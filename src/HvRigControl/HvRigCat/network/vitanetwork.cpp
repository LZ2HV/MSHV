/* MSHV Part from RigControl
 * Copyright 2019 Hrisimir Hristov, LZ2HV
 * May be used under the terms of the GNU General Public License (GPL)
 */
/*
MSHV Native FlexRadio VITA-49 audio and control backend, Copyright (C) 2026 Manoj Ramawarrier, VU2CPL
*/
#include "network.h"

//#include <QtGui>

#include <QCoreApplication>//flex native vita-49: aboutToQuit
#include <QMutex>          //flex native vita-49
//#include <QMutexLocker>  //flex native vita-49
//#include <QRegExp>       //flex native vita-49
//#include <QMetaObject>   //flex native vita-49
//#include <QAtomicInt>    //flex native vita-49
//#include <math.h>        //flex native vita-49
//#include <stdlib.h>      //flex native vita-49: getenv
//flex native vita-49: the live Network, for the free-function hooks below.
//There is one Network per selected rig, made and destroyed with it.
static Network *g_vita_net = NULL;
static void VitaRegister(Network *n)
{
    g_vita_net = n;
}
//flex native vita-49: MSHV_FLEX_TRACE=1 in the environment prints every vita
//command, reply and state change to stderr.
//static const bool flex_trace = (getenv("MSHV_FLEX_TRACE") != NULL);   // protocol trace to stderr
//#define DEBUG_FLEX_TRACE

static bool    s_flex_tx_active = false;
bool _FlexVitaTxActive_() { return s_flex_tx_active; }
bool _SetTxAudioFlex_(int *raw)
{
    // Rawplayer thread.  PushTxBlock() touches only the atomic ring, so it
    // is safe to call on an object that lives on another thread.
    if (!g_vita_net || !s_flex_tx_active) return true;
    FlexVita *v = g_vita_net->VitaAudio();
    return v ? v->PushTxBlock(raw) : true;
}
// Native FlexRadio DAX audio over VITA-49, with no SmartSDR / DAX driver in
// the path.  Layout, as agreed with LZ2HV 2026-09-08:
//
//   control -> Network, on `socket`, the SAME session as the CAT.  Commands
//              leave through writeData() framed "C<seq>|", replies return
//              through readNet() -> VitaLine() -> VitaReply(), matched on
//              the sequence number.  Nothing here blocks: the start-up is a
//              step machine (VITA_*), each step armed by the previous reply
//              and guarded by vita_timer.
//   audio   -> FlexVita below, UDP only, on its own thread vThread, as the
//              TCI client's HvWebSocket lives on mThread.
//
// One session is the point.  The earlier backend opened a second API client
// for the audio, and the radio clicked its T/R relays twice per key: the CAT
// session's "slice set <n> tx=1" re-designated the transmit slice a few ms
// after the audio session had already keyed.  With one session, PTT is
// set_ptt() and nothing else.
//
// Protocol, measured on a FLEX-6600 rather than taken on trust:
//  * On connect the radio volunteers "V<version>" and "H<handle>"; the handle
//    identifies ownership of everything created afterwards.
//  * A bare API client owns no slice and sees no slice status at all.
//    "client gui" opens that door, and receive needs it as much as transmit.
//  * "client udpport <port>" must name a socket that is ALREADY bound.
//  * DAX RX is 24 kHz float32 big-endian, stereo-interleaved with L == R:
//    every second float is the true 24 kHz mono stream -- one of MSHV's own
//    input rates, so nothing is resampled here (k_res = 1).
//  * DAX TX is NOT the mirror of DAX RX; see FlexVita::SendTxPacket().
//  * Only a slice this session created is ever removed: "client gui" may
//    hand us a pre-existing one, and removing it kills whoever was using it.
enum
{
    VITA_IDLE = 0,
    VITA_GUI,           // "client gui" sent
    VITA_UDPPORT,       // "client udpport" sent
    VITA_DRAIN,         // collecting slice status before choosing one
    VITA_SLICE_CREATE,  // "slice create" sent, waiting to see it owned by us
    VITA_DAX,           // "dax audio set" sent
    VITA_RX_CREATE,     // "stream create type=dax_rx" sent
    VITA_TX_CREATE,     // "stream create type=dax_tx" sent
    VITA_RUN,           // streams up
    VITA_RETRY,         // failed; try again when vita_timer fires
    VITA_MISC,          // a reply that only needs consuming
    VITA_MIXER_ON,      // "mixer front_speaker mute on"
    VITA_MIXER_OFF,     // "mixer front_speaker mute off"
    VITA_ATU            // "atu start" / "atu bypass" / "atu set memories_enabled="
};
// ---- state the hooks read.  Written on the GUI thread by Network, except
//      the meter values, which FlexVita writes on vThread -- hence the mutex
//      around everything the two threads share.
static bool    s_flex_rx_active = false;
//static bool    s_flex_tx_active = false;
static QString s_flex_status("Flex Native: idle");
struct FlexMeterDef
{
    QString nam, unit, src;
    int num;
    int slice;                  // the slice this meter belongs to, -1 = the whole radio
    FlexMeterDef() : num(-1), slice(-1) {}
};
static QMutex  flex_mutex;
static QHash<int, FlexMeterDef> flex_meter_defs;   // id -> name/unit, from "S...|meter"
static QHash<int, double>       flex_meter_raw;    // id -> scaled value, from UDP
static int     flex_meter_fwd_id = -1, flex_meter_ref_id = -1, flex_meter_swr_id = -1;
static int     flex_meter_slc = -1;   // slice of the "SLC" meters announced last
static int     flex_my_slice = -1;    // vita_slice, for the meters every slice has its own of
static bool    flex_meters_ok = false;
static double  flex_fwd_watts = 0.0, flex_ref_watts = 0.0, flex_swr = 0.0;
static QString flex_rxant, flex_txant, flex_mode, flex_radio_model;
static QStringList flex_ant_list, flex_tx_ant_list, flex_mode_list;
static bool    flex_front_spkr_mute = false, flex_front_spkr_supported = false;
// Radio-global transmit settings, mirrored from the "transmit" status line.
// -1 means "the radio has not told us yet", which the panel shows as blank
// rather than as 0 -- displaying 0 W would be a lie the operator might act on.
static int     flex_rfpower = -1, flex_tunepower = -1, flex_maxpower = -1;
static bool    flex_hwalc = false;
// The radio's antenna tuner.  flex_atu_present is -1 until the "info" reply has
// said, so the panel keeps the tuner group hidden rather than guess.  The rest
// is mirrored from "atu status=... atu_enabled= memories_enabled= using_mem="
// after "sub atu all".
static int     flex_atu_present = -1;
static QString flex_atu_status;                 // the radio's own word: TUNE_SUCCESSFUL, TUNE_MANUAL_BYPASS, ...
static bool    flex_atu_enabled = false, flex_atu_memories = false, flex_atu_using_mem = false;
static QString flex_atu_refused;                // hex code of the last atu command the radio refused
// True when the current status followed TUNE_IN_PROGRESS, i.e. it is the result
// of a tune cycle rather than a Memories recall or a Bypass click.  Kept here,
// where every status line passes: a cycle can be over in 0.7 s, inside one
// 500 ms panel poll.
static bool    flex_atu_after_cycle = false;
// ---- TX ring.  Rawplayer's thread writes 4096-int blocks (stereo 48 kHz,
//      24-bit scale); vThread reads them out at 24 kHz mono.  One producer,
//      one consumer, so two atomic indices are all the locking it needs.
#define FLEX_STREAM_C  4096
#define FLEX_TX_STRIDE 4                 // stereo 48k -> mono 24k: L of every 2nd frame
#define FLEX_BUF_MAX   (50*FLEX_STREAM_C)
static int        flex_txbuff[FLEX_BUF_MAX];
static QAtomicInt flex_txbuff_pos(0);    // producer
static QAtomicInt flex_tx_pos(0);        // consumer
static QAtomicInt flex_tx_primed(0);
static void FlexTxBufferReset()
{
    for (int i = 0; i < FLEX_BUF_MAX; ++i) flex_txbuff[i] = 0;
    flex_txbuff_pos.storeRelease(0);
    flex_tx_pos.storeRelease(0);
    flex_tx_primed.storeRelease(0);
}
// ---- hooks
bool _FlexVitaRxActive_() { return s_flex_rx_active; }
QString _FlexVitaStatus_() { return s_flex_status; }
bool _FlexVitaMeters_(double *fwd_w, double *ref_w, double *swr)
{
    QMutexLocker lk(&flex_mutex);
    if (!flex_meters_ok) return false;
    if (fwd_w) *fwd_w = flex_fwd_watts;
    if (ref_w) *ref_w = flex_ref_watts;
    if (swr)   *swr   = flex_swr;
    return true;
}
// A name can be on more than one meter: every slice has its own TX chain,
// so with two slices on the radio (two MSHV, or MSHV + SmartSDR) there are two
// "ALC" meters.  Take the one of OUR slice -- never another slice's.
bool _FlexVitaMeterByName_(const char *name, double *out)
{
    QMutexLocker lk(&flex_mutex);
    const QString want = QString(name).toUpper();
    int only = -1, mine = -1, radio = -1, count = 0;
    for (QHash<int, FlexMeterDef>::const_iterator it = flex_meter_defs.constBegin();
         it != flex_meter_defs.constEnd(); ++it)
    {
        if (it.value().nam.toUpper() != want) continue;
        count++;
        only = it.key();
        if (it.value().slice >= 0 && it.value().slice == flex_my_slice && (mine < 0 || it.key() < mine)) mine = it.key();
        if (it.value().slice < 0 && (radio < 0 || it.key() < radio)) radio = it.key();
    }
    int best = (mine >= 0) ? mine : radio;
    if (count == 1) best = only;          // one slice, or a meter of the whole radio    
    if (best < 0 || !flex_meter_raw.contains(best)) return false;
    if (out) *out = flex_meter_raw.value(best);
    return true;
}
QStringList _FlexVitaAntList_(bool tx) { QMutexLocker lk(&flex_mutex); return tx ? flex_tx_ant_list : flex_ant_list; }
QString     _FlexVitaAnt_(bool tx)     { QMutexLocker lk(&flex_mutex); return tx ? flex_txant : flex_rxant; }
QStringList _FlexVitaModeList_()       { QMutexLocker lk(&flex_mutex); return flex_mode_list; }
QString     _FlexVitaMode_()           { QMutexLocker lk(&flex_mutex); return flex_mode; }
QString     _FlexVitaRadioModel_()     { QMutexLocker lk(&flex_mutex); return flex_radio_model; }
bool        _FlexVitaFrontSpeakerMute_()  { QMutexLocker lk(&flex_mutex); return flex_front_spkr_mute; }
bool        _FlexVitaHasFrontSpeaker_()   { QMutexLocker lk(&flex_mutex); return flex_front_spkr_supported; }
void _FlexVitaSetAnt_(bool tx, QString ant) { if (g_vita_net) g_vita_net->VitaSetSlice(tx ? "txant" : "rxant", ant); }
void _FlexVitaSetMode_(QString mode)        { if (g_vita_net) g_vita_net->VitaSetSlice("mode", mode); }
void _FlexVitaSetFrontSpeakerMute_(bool on) { if (g_vita_net) g_vita_net->VitaSetFrontSpeaker(on); }
// Transmit settings.  Each getter returns -1 (or leaves the flag alone) until
// the radio has actually reported the value, so the panel can tell "not known
// yet" from "zero" and show a blank instead of an authoritative-looking 0.
int  _FlexVitaRfPower_()   { QMutexLocker lk(&flex_mutex); return flex_rfpower;   }
int  _FlexVitaTunePower_() { QMutexLocker lk(&flex_mutex); return flex_tunepower; }
int  _FlexVitaMaxPower_()  { QMutexLocker lk(&flex_mutex); return flex_maxpower;  }
bool _FlexVitaHwAlc_()     { QMutexLocker lk(&flex_mutex); return flex_hwalc;     }
void _FlexVitaSetTransmit_(QString key, int value)
{
    if (g_vita_net) g_vita_net->VitaSetTransmit(key, value);
}
// The radio's antenna tuner: the panel's hooks.
// Antenna tuner.  A tune cycle keys the radio, so nothing here is ever called
// except from an operator's click in the panel.
int     _FlexVitaAtuPresent_()  { QMutexLocker lk(&flex_mutex); return flex_atu_present;   }
QString _FlexVitaAtuStatus_()   { QMutexLocker lk(&flex_mutex); return flex_atu_status;    }
bool    _FlexVitaAtuEnabled_()  { QMutexLocker lk(&flex_mutex); return flex_atu_enabled;   }
bool    _FlexVitaAtuMemories_() { QMutexLocker lk(&flex_mutex); return flex_atu_memories;  }
bool    _FlexVitaAtuUsingMem_() { QMutexLocker lk(&flex_mutex); return flex_atu_using_mem; }
QString _FlexVitaAtuRefused_()  { QMutexLocker lk(&flex_mutex); return flex_atu_refused;   }
bool    _FlexVitaAtuAfterCycle_() { QMutexLocker lk(&flex_mutex); return flex_atu_after_cycle; }
void _FlexVitaAtuTune_()   { if (g_vita_net) g_vita_net->VitaAtu("start");  }
void _FlexVitaAtuBypass_() { if (g_vita_net) g_vita_net->VitaAtu("bypass"); }
void _FlexVitaSetAtuMemories_(bool on)
{
    if (g_vita_net) g_vita_net->VitaAtu(QString("set memories_enabled=%1").arg(on ? 1 : 0));
}

// ---- byte order
static inline quint32 FlexBE32(const char *p)
{
    const unsigned char *u = (const unsigned char *)p;
    return ((quint32)u[0] << 24) | ((quint32)u[1] << 16) | ((quint32)u[2] << 8) | (quint32)u[3];
}
static inline float FlexBEFloat(const char *p)
{
    union { quint32 i;
        float f;
    } u;
    u.i = FlexBE32(p);
    return u.f;
}
static inline void FlexPutBE32(char *p, quint32 v)
{
    p[0] = (char)((v >> 24) & 0xff);
    p[1] = (char)((v >> 16) & 0xff);
    p[2] = (char)((v >>  8) & 0xff);
    p[3] = (char)( v        & 0xff);
}
#define FLEX_METER_CLASS 0x534C8002u   // meter packets share the audio socket
// ============================================================ FlexVita (UDP, vThread)
FlexVita::FlexVita()
        : QObject(0)
{
    audio_    = new QUdpSocket(this);
    tx_timer_ = new QTimer(this);
    tx_timer_->setTimerType(Qt::PreciseTimer);
    rx_stream_ = 0;
    tx_stream_ = 0;
    tx_packet_count_ = 0;
    tx_sent_frames_  = 0;
    tx_keyed_        = false;
    // Bound here, on the creating thread, so Network can tell the radio the
    // port before this object moves to vThread.  The notifier follows.
    audio_->bind(QHostAddress(QHostAddress::AnyIPv4), 0);
    connect(audio_, SIGNAL(readyRead()), this, SLOT(ReadAudio()));
    connect(tx_timer_, SIGNAL(timeout()), this, SLOT(SendTxPacket()));
}
FlexVita::~FlexVita()
{}
quint16 FlexVita::LocalPort() const
{
    return audio_->localPort();
}
void FlexVita::SetRadio(QString host)
{
    radio_addr_ = QHostAddress(host);
}
void FlexVita::SetRxStream(uint id)
{
    rx_stream_ = id;
}
void FlexVita::SetTxStream(uint id)
{
    // Registers the stream id (TX configured) or, with 0, tears TX down.
    // Audio does NOT flow yet -- SetTxKeyed() starts and stops it, so the
    // radio is fed only between key and unkey.  Feeding it while unkeyed is
    // what held the transmitter up in UNKEY_REQUESTED until MSHV quit.
    tx_stream_ = id;
    if (!id)
    {
        tx_keyed_ = false;
        tx_timer_->stop();
    }
}
void FlexVita::SetTxKeyed(bool on)
{
    if (on)
    {
        if (!tx_stream_) return;             // TX not set up; nothing to key
        if (tx_keyed_)  return;              // ALREADY keyed -- see below
        // Only a genuine unkeyed->keyed transition may reset the TX side.
        // set_ptt() can be asked to key twice with no unkey between (seen on
        // air 2026-09-08, one slot in sixteen: xmit 1 at C172 and again at
        // C176).  Without this guard the second call empties the ring the
        // Rawplayer thread has been filling and restarts tx_packet_count_ and
        // tx_clock_ mid-transmission -- audio already queued is discarded and
        // the VITA packet-count sequence jumps under the radio.  Harmless when
        // the two keys are milliseconds apart, a real discontinuity if the
        // second one lands later in the slot.
        // set_ptt() still traces both keys deliberately -- "TX audio ON"
        // twice with no "OFF" between is how a double PTT shows up in
        // MSHV_FLEX_TRACE; only the audio side ignores the second.
        FlexTxBufferReset();
        tx_packet_count_ = 0;
        tx_sent_frames_  = 0;
        tx_clock_.start();
        tx_keyed_ = true;
        // 128 frames at 24 kHz = 5.33 ms.  The slot sends as many packets as
        // the elapsed time calls for, so timer granularity cannot drift it.
        tx_timer_->start(4);
    }
    else
    {
        tx_keyed_ = false;
        tx_timer_->stop();
    }
}
void FlexVita::Reset()
{
    SetTxKeyed(false);
    SetTxStream(0);
    rx_stream_ = 0;
}
bool FlexVita::ParseVita(const char *data, int len, int *payload_offset, int *payload_bytes)
{
    if (len < 8) return false;
    const quint32 header = FlexBE32(data);
    const int  ptype    = (header >> 28) & 0x0F;
    const bool class_id = (header & 0x08000000) != 0;
    const bool trailer  = (header & 0x04000000) != 0;
    const int  tsi      = (header >> 22) & 0x03;
    const int  tsf      = (header >> 20) & 0x03;
    const int  words    =  header & 0xFFFF;
    const int packet_bytes = words * 4;
    if (packet_bytes <= 0 || packet_bytes > len) return false;
    if (ptype != 1 && ptype != 3) return false;   // IF data, with stream ID
    int idx = 8;                                   // header word + stream ID
    if (class_id) idx += 8;
    if (tsi)      idx += 4;
    if (tsf)      idx += 8;                        // fractional timestamp is 64-bit
    if (idx > packet_bytes) return false;
    int end = packet_bytes;
    if (trailer) end -= 4;
    if (end <= idx) return false;
    *payload_offset = idx;
    *payload_bytes  = end - idx;
    return true;
}
#include "../../../HvMsCore/mscore.h"
void FlexVita::ReadAudio()
{
    static char buf[65536];
    while (audio_->hasPendingDatagrams())
    {
        const qint64 n = audio_->readDatagram(buf, sizeof(buf));
        if (n < 8) continue;

        // Meters share this socket with the audio but are independent of it,
        // so decode them before any audio-path guard -- in particular before
        // _GetRxAudioReadTci_(), which is 0 whenever the decoder is idle and
        // would otherwise drop the metering exactly while it is being watched.
        if (n >= 16 && FlexBE32(buf + 12) == FLEX_METER_CLASS)
        {
            int off = 0, bytes = 0;
            if (ParseVita(buf, (int)n, &off, &bytes))
            {
                QMutexLocker lk(&flex_mutex);
                for (int i = off; i + 4 <= off + bytes; i += 4)
                {
                    const int id = (int)((quint8)buf[i] << 8 | (quint8)buf[i + 1]);
                    const qint16 raw = (qint16)((quint8)buf[i + 2] << 8 | (quint8)buf[i + 3]);
                    // The scale depends on the UNIT.  Verified against the
                    // radio's own low/high bounds:
                    //   dBm / dBFS / SWR -> /128   (SWR raw 128 = 1.00)
                    //   Volts / Amps     -> /256   (raw 3119 = 12.18 V)
                    //   degC             -> /64    (raw 2643 = 41.3 C)
                    //   RPM              -> /1
                    const QString u = flex_meter_defs.value(id).unit;
                    double div = 128.0;
                    if      (u == "Volts" || u == "Amps") div = 256.0;
                    else if (u == "degC")                 div = 64.0;
                    else if (u == "RPM")                  div = 1.0;
                    flex_meter_raw[id] = raw / div;
                    if (id == flex_meter_fwd_id)
                    {
                        const double dbm = raw / 128.0;
                        flex_fwd_watts = (dbm > 0.0) ? pow(10.0, (dbm - 30.0) / 10.0) : 0.0;
                        flex_meters_ok = true;
                    }
                    else if (id == flex_meter_ref_id)
                    {
                        const double dbm = raw / 128.0;
                        flex_ref_watts = (dbm > 0.0) ? pow(10.0, (dbm - 30.0) / 10.0) : 0.0;
                    }
                    else if (id == flex_meter_swr_id)
                    {
                        flex_swr = raw / 128.0;
                    }
                }
            }
            continue;
        }

        if (!rx_stream_) continue;
        if (_GetRxAudioReadTci_() == 0) continue;     // decoder not reading yet
        if (FlexBE32(buf + 4) != rx_stream_) continue;
        int off = 0, bytes = 0;
        if (!ParseVita(buf, (int)n, &off, &bytes)) continue;

        // Stereo-interleaved with L == R: every second float is the true
        // 24 kHz mono stream, scaled to MSHV's 24-bit convention.
        const int floats = bytes / 4;
        int mono = 0;
        for (int i = 0; i + 1 < floats && mono < 8192; i += 2)
        {
            const float f = FlexBEFloat(buf + off + i * 4);
            rx_scratch_[mono++] = (int)(f * 8388607.0f);
        }
        if (mono > 0) _SetRxAudioTci_(rx_scratch_, mono, 1);   // k_res 1 = 24000
    }
}
bool FlexVita::PushTxBlock(int *raw)
{
    // Rawplayer's thread.  Wait until the sender has drained enough that this
    // block cannot overrun it -- the same flow control the TCI path applies.
    if (!tx_stream_) return true;
    int guard = 900;                                  // ~4.5 s ceiling
    while (guard-- > 0)
    {
        int pending = flex_txbuff_pos.loadAcquire() - flex_tx_pos.loadAcquire();
        if (pending < 0) pending += FLEX_BUF_MAX;
        if (pending < FLEX_BUF_MAX - FLEX_STREAM_C * 2) break;
        if (!tx_stream_) return true;
        QThread::usleep(5000);
    }
    int wp = flex_txbuff_pos.loadAcquire();
    for (int i = 0; i < FLEX_STREAM_C; ++i)
    {
        flex_txbuff[wp] = raw[i];
        if (++wp >= FLEX_BUF_MAX) wp = 0;
    }
    flex_txbuff_pos.storeRelease(wp);
    if (flex_tx_primed.loadAcquire() < 4) flex_tx_primed.fetchAndAddRelease(1);
    return true;
}
void FlexVita::SendTxPacket()
{
    if (!tx_keyed_ || !tx_stream_ || radio_addr_.isNull()) return;
    if (flex_tx_primed.loadAcquire() < 2) return;     // let a little audio accumulate

    // Catch up in whole packets against the clock, so a late timer cannot
    // starve the radio's transmit buffer.
    const qint64 due = (tx_clock_.elapsed() * RadioAudioRate) / 1000;
    int packets = (int)((due - tx_sent_frames_) / FramesPerPacket);
    if (packets <= 0) return;
    if (packets > 8) packets = 8;                      // no burst after a stall

    // DAX TX is NOT the mirror image of DAX RX.  Verified against the radio's
    // own FWDPWR meter (37.4 dBm / 5.5 W where every other form gave 0 W):
    //
    //                 RX (radio -> us)        TX (us -> radio)
    //   packet type   3                       1
    //   class code    0x534C03E3              0x534C0123
    //   TSI / TSF     1 / 1                   3 / 1
    //   total size    1052 bytes              284 bytes  (71 words)
    //   payload       128 x stereo float32    128 x MONO int16 BIG-ENDIAN
    //
    // Sending the RX form back at the radio is silently discarded: it keys
    // happily and emits nothing.  Measured constants -- do not "tidy" them.
    //
    //   [0]  header 0x18D00047 | (count << 16)
    //   [4]  stream id      [8] OUI 0x00001C2D   [12] class 0x534C0123
    //   [16] TSI (0)        [20] TSF, 64-bit (0)
    //   [28] payload: FramesPerPacket x int16 BE
    const int payload = FramesPerPacket * 2;
    QByteArray pkt;
    pkt.resize(28 + payload);
    int rp = flex_tx_pos.loadAcquire();
    for (int p = 0; p < packets; ++p)
    {
        char *d = pkt.data();
        const quint32 header = 0x18D00047u | ((quint32)(tx_packet_count_ & 0x0F) << 16);
        FlexPutBE32(d,      header);
        FlexPutBE32(d + 4,  tx_stream_);
        FlexPutBE32(d + 8,  0x00001C2D);              // FlexRadio OUI
        FlexPutBE32(d + 12, 0x534C0123);              // DAX *TX* packet class
        FlexPutBE32(d + 16, 0);                       // TSI
        FlexPutBE32(d + 20, 0);                       // TSF high
        FlexPutBE32(d + 24, 0);                       // TSF low
        for (int i = 0; i < FramesPerPacket; ++i)
        {
            // Rawplayer hands over stereo 48 kHz at 24-bit scale; stride 4
            // takes the left channel at 24 kHz, /256 brings 24-bit to 16.
            int s = flex_txbuff[rp] / 256;
            if (s >  32767) s =  32767;
            if (s < -32768) s = -32768;
            const quint16 w = (quint16)(qint16)s;
            d[28 + i * 2]     = (char)((w >> 8) & 0xff);
            d[28 + i * 2 + 1] = (char)( w       & 0xff);
            rp += FLEX_TX_STRIDE;
            if (rp >= FLEX_BUF_MAX) rp -= FLEX_BUF_MAX;
        }
        flex_tx_pos.storeRelease(rp);
        audio_->writeDatagram(pkt, radio_addr_, VitaUdpPort);
        tx_packet_count_ = (quint8)((tx_packet_count_ + 1) & 0x0F);
        tx_sent_frames_ += FramesPerPacket;
    }
}
// ============================================================ Network side (control)
FlexVita *Network::VitaAudio()
{
    return is_vita ? vita_ : NULL;
}
// Called from SetTciSelect() (device chosen), from VitaTimeout() once the CAT
// init has completed, and on the retry.  Idempotent: an unchanged, running
// backend is left alone; a changed channel, direction or slice rebuilds it.
void Network::VitaCreate()
{
	vita_last_freq_hz = 0;
	vita_rx = 0;
    vita_tx = false;
    vita_ = NULL;
    is_vita = false;
    vita_step = 0;//VITA_IDLE
    vita_handle = 0;
    vita_ch = 1;
    vita_slice = -1;
    vita_slice_created = false;
    vita_rx_stream = 0;
    vita_tx_stream = 0;
    vita_rx_active = false;
    vita_tx_active = false;
    vita_want_tx = false;
    vita_rig_slice = -1;
    vita_gui_done = false;
    vita_status = "Flex Native: idle";
    vita_timer = new QTimer(this);
    vita_timer->setSingleShot(true);
    connect(vita_timer, SIGNAL(timeout()), this, SLOT(VitaTimeout()));
    //This object is created without a parent and is not deleted at exit, so
    //the destructor is not a place to release the radio from.  aboutToQuit
    //is: it fires while the socket is still open.
    connect(QCoreApplication::instance(), SIGNAL(aboutToQuit()), this, SLOT(VitaQuit()));
    VitaRegister(this);
}
void Network::VitaDestroy()
{
#if defined DEBUG_FLEX_TRACE //if (flex_trace) fprintf(stderr, "[MSHV Flex] ~Network: tearing down\n");
    fprintf(stderr, "[MSHV Flex] ~Network: tearing down\n");
#endif    
    //flex native vita-49: release the streams while the socket still exists,
    //then bring the audio thread down the way the TCI one comes down below.
    VitaStop(true);
    if (is_vita)
    {
        vThread.quit();
        vThread.wait(2000);
        delete vita_;
        vita_ = NULL;
        is_vita = false;
    }
    VitaRegister(NULL);
}
void Network::VitaDisconnect()
{
    //flex native vita-49: the control socket is gone, so there is nobody to
    //tell; drop the streams locally and forget the handle.  A reconnect runs
    //the CAT init again, and its completion restarts the audio.
    VitaStop(false);
    vita_gui_done = false;
    vita_handle = 0;
    // The radio removed our slice and its meters while we were not
    // listening; "sub meter all" on the next connection lists what is left.
    QMutexLocker lk(&flex_mutex);
    flex_meter_defs.clear();
    flex_meter_raw.clear();
    flex_meter_fwd_id = flex_meter_ref_id = flex_meter_swr_id = -1;
    flex_meter_slc = -1;    
    vita_buf.clear();
}
void Network::SetVitaSelect(int vr,bool vt)
{
	vita_rx = vr;//RX vita 1,2,3,4..8
    vita_tx = vt;//TX vita false,true
    UpdateFlexVita();//flex native vita-49
}
void Network::VitaReadNet(QByteArray ba,QString &sba)
{
	//QString sba = QString(ba.data());
	//flex native vita-49: the control channel shares this socket.  Every
    //complete line goes to the vita parser first -- stream ids and reply
    //codes are line-exact, whereas the CAT logic below flattens the chunk.
    //Partial lines wait in vita_buf for the rest to arrive.
    vita_last_slice_line.clear();
    vita_buf += ba;
    int vnl;
    while ((vnl = vita_buf.indexOf('\n')) >= 0)
    {
       VitaLine(QString::fromLatin1(vita_buf.left(vnl)));
       vita_buf.remove(0, vnl + 1);
    }
    //With "sub slice all" in effect a chunk can carry several slices'
    //status lines, and indexOf("RF_frequency=") below would take the
    //first one whatever slice it belongs to.  Hand the CAT parser this
    //chunk's line for OUR slice instead, when there is one.
    if (!vita_last_slice_line.isEmpty()) sba = vita_last_slice_line;
}
void Network::VitaStartTimer()
{
    //flex native vita-49: the CAT session is up; the audio may
    //follow.  Deferred a tick rather than started from inside
    //readNet(), since writeData() can re-enter it.
	if (vita_step==0) vita_timer->start(50);	
}
void Network::VitaSetPTTON()
{
	// Start the DAX TX audio just BEFORE keying, so the radio has a
    // little buffered when xmit 1 lands (it wants a few primed
    // packets first).
    if (is_vita && vita_ && vita_tx_active)
    {               
#if defined DEBUG_FLEX_TRACE //if (flex_trace) fprintf(stderr, "[MSHV Flex] TX audio ON (keyed)\n");
		fprintf(stderr, "[MSHV Flex] TX audio ON (keyed)\n");
#endif                
        QMetaObject::invokeMethod(vita_, "SetTxKeyed", Qt::QueuedConnection, Q_ARG(bool, true));
    }
}
void Network::VitaSetPTTOFF()
{
	// Stop the DAX TX audio on unkey.  Without this the radio keeps
    // transmitting what we send and stays in UNKEY_REQUESTED at full
    // power until the stream is torn down -- the "PTT does not release"
    // bug (measured on a FLEX-6600, 2026-09-08).
    if (is_vita && vita_ && vita_tx_active)
    {            
#if defined DEBUG_FLEX_TRACE //if (flex_trace) fprintf(stderr, "[MSHV Flex] TX audio OFF (unkey)\n");
		fprintf(stderr, "[MSHV Flex] TX audio OFF (unkey)\n");
#endif            
        QMetaObject::invokeMethod(vita_, "SetTxKeyed", Qt::QueuedConnection, Q_ARG(bool, false));
    }	
}
void Network::SetVitaLastFreq(unsigned long long f)
{
	vita_last_freq_hz = f;
}
void Network::UpdateFlexVita()
{   
    const bool want = fsdrs && (vita_rx>0 || vita_tx);
    if (!want)
    {
        if (vita_step!=VITA_IDLE || vita_rx_active || vita_tx_active) VitaStop(true);
        return;
    }      
    // The CAT session must be initialised first (cretr_nt == -2).  Until then
    // there is nothing to do: the init-complete branch in readNet() arms
    // vita_timer, which lands back here.
    if (!socket || socket->state()!=QAbstractSocket::ConnectedState || cretr_nt!=-2) return;
    const int ch = (vita_rx>0) ? vita_rx : 1;   // TX-only has no channel of its own
    if (vita_step!=VITA_IDLE && vita_step!=VITA_RETRY)
    {
        if (ch==vita_ch && vita_tx==vita_want_tx && slicenum.toInt()==vita_rig_slice) return;
        VitaStop(true);
    }
    VitaStart();
}
void Network::VitaStart()
{
    if (!is_vita)
    {
        // Same shape as the TCI client: made here, then moved to its thread.
        vita_ = new FlexVita();
        vita_->moveToThread(&vThread);
        vThread.start();
        is_vita = true;
    }
    vita_ch        = (vita_rx>0) ? vita_rx : 1;
    vita_want_tx   = vita_tx;
    vita_rig_slice = slicenum.toInt();
    vita_slice     = -1;
    vita_slice_created = false;
    vita_mismatch.clear();
    vita_log.clear();
    vita_pend.clear();
    // TX datagrams go where the CAT socket is connected: one radio, one address.
    QHostAddress peer = socket->peerAddress();
    bool ok4 = false;
    const quint32 v4 = peer.toIPv4Address(&ok4);
    if (ok4) peer = QHostAddress(v4);
    QMetaObject::invokeMethod(vita_, "SetRadio", Qt::QueuedConnection, Q_ARG(QString, peer.toString()));
    vita_status = "Flex Native: starting";
    VitaMirror();
    if (!vita_gui_done)
    {
        vita_step = VITA_GUI;
        VitaSend("client gui", VITA_GUI);
        vita_timer->start(3000);
    }
    else VitaReply(VITA_GUI, 0, QString());   // already a GUI client on this connection
}
// One command out, remembered by sequence number so its "R<seq>|" reply can
// be routed to the step that sent it.  writeData() does the framing.
int Network::VitaSend(QString cmd, int step)
{
    if (!socket || socket->state()!=QAbstractSocket::ConnectedState) return -1;
    if (seqnum > 999999) seqnum = 0;
    const int seq = seqnum;
#if defined DEBUG_FLEX_TRACE //if (flex_trace) fprintf(stderr, "[MSHV Flex] > C%d|%s  (step %d)\n", seq, qPrintable(cmd), step);
	fprintf(stderr, "[MSHV Flex] > C%d|%s  (step %d)\n", seq, qPrintable(cmd), step);
#endif    
    writeData(cmd,false,NULL);
    vita_pend[seq] = step;
    return seq;
}
// Every complete line off the control socket, before the CAT parser sees the
// chunk.  H = our handle, R = a reply, S = status.
void Network::VitaLine(QString line)
{
    line = line.trimmed();
    if (line.isEmpty()) return;
    const QChar tag = line.at(0);
    if (tag=='H')
    {
        bool ok = false;
        const quint32 h = line.mid(1).toUInt(&ok, 16);
        if (ok) vita_handle = h;
        return;
    }
    if (tag=='R')
    {
        // The model rides on the CAT's own "info" reply, so take it from any
        // reply that carries it; no second "info" needed.  "M" on the end of
        // the model is the only way to know there is a front speaker to mute.
        if (line.contains("model=\""))
        {
            QRegExp mo("model=\"([^\"]+)\"");
            if (mo.indexIn(line) >= 0)
            {
                QMutexLocker lk(&flex_mutex);
                flex_radio_model = mo.cap(1);
                flex_front_spkr_supported = flex_radio_model.trimmed().toUpper().endsWith("M");
            }
        }
        // The same reply says whether there is an antenna tuner to offer.
        if (line.contains("atu_present="))
        {
            QRegExp ap("\\batu_present=([01])\\b");
            if (ap.indexIn(line) >= 0)
            {
                QMutexLocker lk(&flex_mutex);
                flex_atu_present = ap.cap(1).toInt();
            }
        }        
        const int bar = line.indexOf('|');
        if (bar < 0) return;
        bool ok = false;
        const int seq = line.mid(1, bar-1).toInt(&ok);
        if (!ok || !vita_pend.contains(seq)) return;   // a CAT reply, not ours
        const int step = vita_pend.take(seq);        
#if defined DEBUG_FLEX_TRACE //if (flex_trace) fprintf(stderr, "[MSHV Flex] < %s  (step %d)\n", qPrintable(line), step);
		fprintf(stderr, "[MSHV Flex] < %s  (step %d)\n", qPrintable(line), step);
#endif        
        const QString rest = line.mid(bar+1);
        const int bar2 = rest.indexOf('|');
        const quint32 code = rest.left(bar2 < 0 ? rest.size() : bar2).toUInt(0, 16);
        const QString body = (bar2 < 0) ? QString() : rest.mid(bar2+1);
        VitaReply(step, code, body);
        return;
    }
    if (tag!='S') return;

    vita_log.append(line);
    if (vita_log.size() > 400) vita_log.removeFirst();
    if (line.contains(QString("|slice %1 ").arg(slicenum))) vita_last_slice_line = line;

    // Mirror our slice's antenna / mode state and the lists of what the
    // radio accepts, so the UI never hardcodes them.
    if (vita_slice >= 0 && line.contains(QString("|slice %1 ").arg(vita_slice)))
    {
        QRegExp kv("\\b(rxant|txant|mode|ant_list|tx_ant_list|mode_list)=([^\\s]+)");
        QMutexLocker lk(&flex_mutex);
        int at = 0;
        while ((at = kv.indexIn(line, at)) >= 0)
        {
            const QString k = kv.cap(1), v = kv.cap(2);
            if      (k == "rxant")       flex_rxant       = v;
            else if (k == "txant")       flex_txant       = v;
            else if (k == "mode")        flex_mode        = v;
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
            else if (k == "ant_list")    flex_ant_list    = v.split(',', Qt::SkipEmptyParts);
            else if (k == "tx_ant_list") flex_tx_ant_list = v.split(',', Qt::SkipEmptyParts);
            else if (k == "mode_list")   flex_mode_list   = v.split(',', Qt::SkipEmptyParts);
#else
            else if (k == "ant_list")    flex_ant_list    = v.split(',', QString::SkipEmptyParts);
            else if (k == "tx_ant_list") flex_tx_ant_list = v.split(',', QString::SkipEmptyParts);
            else if (k == "mode_list")   flex_mode_list   = v.split(',', QString::SkipEmptyParts);
#endif
            at += kv.matchedLength();
        }
    }    
    // Radio-global transmit settings: "transmit rfpower=50 tunepower=10 ...".
    // These are NOT slice properties -- one transmitter, one set of values --
    // so they are matched on the "|transmit " line rather than the slice line.
    if (line.contains("|transmit "))
    {
        QRegExp kv("\\b(rfpower|tunepower|max_power_level|hwalc_enabled)=(\\d+)");
        QMutexLocker lk(&flex_mutex);
        int at = 0;
        while ((at = kv.indexIn(line, at)) >= 0)
        {
            const QString k = kv.cap(1);
            const int     v = kv.cap(2).toInt();
            if      (k == "rfpower")         flex_rfpower   = v;
            else if (k == "tunepower")       flex_tunepower = v;
            else if (k == "max_power_level") flex_maxpower  = v;
            else if (k == "hwalc_enabled")   flex_hwalc     = (v != 0);
            at += kv.matchedLength();
        }
    }  
    // Antenna tuner: "atu status=TUNE_MANUAL_BYPASS atu_enabled=1 memories_enabled=0
    // using_mem=0" (verified on the 6600).  A fresh status also means the radio
    // has moved on from any command it refused.
    if (line.contains("|atu "))
    {
        QRegExp kv("\\b(status|atu_enabled|memories_enabled|using_mem)=([^\\s]+)");
        QMutexLocker lk(&flex_mutex);
        int at = 0;
        while ((at = kv.indexIn(line, at)) >= 0)
        {
            const QString k = kv.cap(1), v = kv.cap(2);
            if (k == "status")
            {
                // The radio repeats a status in several lines; only a CHANGE says
                // where this one came from.
                if (v != flex_atu_status)
                {
                    flex_atu_after_cycle = (flex_atu_status == "TUNE_IN_PROGRESS");
                    flex_atu_status = v;
                }
                flex_atu_refused.clear();
            }
            else if (k == "atu_enabled")      flex_atu_enabled   = (v == "1");
            else if (k == "memories_enabled") flex_atu_memories  = (v == "1");
            else if (k == "using_mem")        flex_atu_using_mem = (v == "1");
            at += kv.matchedLength();
        }
    }      
    // The front speaker is not in the status stream on a 6600; if a firmware
    // does report it, believe that over our own shadow.
    if (line.contains("|radio slices="))
    {
        QRegExp fs("\\bfront_speaker_mute=([01])\\b");
        if (fs.indexIn(line) >= 0)
        {
            QMutexLocker lk(&flex_mutex);
            flex_front_spkr_mute      = (fs.cap(1) == "1");
            flex_front_spkr_supported = true;
        }
    }
    // Meter definitions: "meter 7.src=TX-#7.num=1#7.nam=FWDPWR#7.unit=dBm#..."
    // and "meter 20 removed".
    // Every slice has its own TX chain, so its own "TX-" meters (CODEC .. ALC
    // .. GAIN).  The radio lists a slice's "SLC" meters and then its TX chain,
    // so a "TX-" meter belongs to the slice of the "SLC" meters before it
    // (FLEX-6600: TX chain num=8 for slice 0, num=9 for slice 1).    
    if (line.contains("|meter "))
    {
        QMutexLocker lk(&flex_mutex);
        QRegExp rem("\\|meter (\\d+) removed");
        if (rem.indexIn(line) >= 0)
        {   
            const int id = rem.cap(1).toInt();
            flex_meter_defs.remove(id);
            flex_meter_raw.remove(id);
            if (id == flex_meter_fwd_id) flex_meter_fwd_id = -1;
            if (id == flex_meter_ref_id) flex_meter_ref_id = -1;
            if (id == flex_meter_swr_id) flex_meter_swr_id = -1;
            flex_meter_slc = -1;
        }
        QList<int> ids;                       // in the order the radio lists them
        QHash<int, FlexMeterDef> got;
        QRegExp kv("(\\d+)\\.(src|num|nam|unit)=([^#\\s]+)");        
        int at = 0;
        while ((at = kv.indexIn(line, at)) >= 0)
        {
            const int id = kv.cap(1).toInt();
            if (!got.contains(id))
            {
                ids.append(id);
                got[id] = flex_meter_defs.value(id);
            }
            const QString k = kv.cap(2), v = kv.cap(3);
            if      (k == "src")  got[id].src  = v;
            else if (k == "num")  got[id].num  = v.toInt();
            else if (k == "nam")  got[id].nam  = v.toUpper();
            else if (k == "unit") got[id].unit = v;
            at += kv.matchedLength();
        }
        for (int i = 0; i < ids.count(); ++i)        
        {
            const int id = ids.at(i);
            const FlexMeterDef old = flex_meter_defs.value(id);
            FlexMeterDef d = got.value(id);
            if (d.src == "SLC")
            {
                d.slice = d.num;
                flex_meter_slc = d.num;
            }
            else if (d.src.startsWith("TX"))
            {
                // a meter listed again keeps its slice
                if (d.nam != old.nam || d.src != old.src || d.num != old.num) d.slice = flex_meter_slc;
            }
            else
            {
                d.slice = -1;
                flex_meter_slc = -1;          // a meter of the whole radio ends a slice's list
            }
            flex_meter_defs[id] = d;
            const QString nam = d.nam;            
            if      (nam == "FWDPWR")
            {
                if (flex_meter_fwd_id < 0) flex_meter_fwd_id = id;
            }
            else if (nam == "REFPWR")
            {
                if (flex_meter_ref_id < 0) flex_meter_ref_id = id;
            }
            else if (nam == "SWR")
            {
                if (flex_meter_swr_id < 0) flex_meter_swr_id = id;
            }
        }
    }
    // The create reply does not always carry the stream id; the status does.
    if (vita_step==VITA_RX_CREATE && vita_rx_stream==0 && line.contains("type=dax_rx")
            && line.toLower().contains(QString("client_handle=0x%1").arg(vita_handle, 8, 16, QChar('0'))))
    {
        QRegExp rx("\\|stream 0x([0-9A-Fa-f]+)");
        if (rx.indexIn(line) >= 0)
        {
            vita_rx_stream = rx.cap(1).toUInt(0, 16);
            if (vita_rx_stream) VitaRxUp();
        }
    }
    if (vita_step==VITA_TX_CREATE && vita_tx_stream==0 && line.contains("type=dax_tx")
            && line.toLower().contains(QString("client_handle=0x%1").arg(vita_handle, 8, 16, QChar('0'))))
    {
        QRegExp tx("\\|stream 0x([0-9A-Fa-f]+)");
        if (tx.indexIn(line) >= 0)
        {
            vita_tx_stream = tx.cap(1).toUInt(0, 16);
            if (vita_tx_stream) VitaTxUp();
        }
    }
    // A slice we asked for has appeared under our handle.
    if (vita_step==VITA_SLICE_CREATE)
    {
        const int made = VitaOwnedSlice();
        if (made >= 0)
        {
            vita_slice = made;
            vita_slice_created = true;
            // The radio assigns the index -- it may not be the one the rig
            // model names, and with other GUI clients present it usually is
            // not.  The slice is ours either way, so take the CAT half to it
            // rather than asking the operator to re-pick a letter; see the
            // note in VitaChooseSlice().  The panel line shows the number.
            if (vita_rig_slice >= 0 && made != vita_rig_slice)
            {
                slicenum = QString::number(made);
                vita_rig_slice = made;
            }
            VitaSliceReady();
        }
    }
}
void Network::VitaReply(int step, quint32 code, QString body)
{
    body = body.trimmed();
    switch (step)
    {
    case VITA_GUI:
        if (code != 0)
        {
            VitaFail("client gui refused - radio granted no slice rights");
            return;
        }
        vita_gui_done = true;
        VitaSend("sub slice all", VITA_MISC);
        VitaSend("sub audio_stream all", VITA_MISC);
        VitaSend("sub dax all", VITA_MISC);
        VitaSend("sub meter all", VITA_MISC);
        VitaSend("sub radio all", VITA_MISC);
        VitaSend("sub atu all", VITA_MISC);   // read-only; a radio without a tuner just never reports        
        if (vita_want_tx) VitaSend("sub tx all", VITA_MISC);
        vita_step = VITA_UDPPORT;
        VitaSend("client udpport " + QString::number(vita_->LocalPort()), VITA_UDPPORT);
        vita_timer->start(3000);
        return;
    case VITA_UDPPORT:
        if (code != 0)
        {
            VitaFail("client udpport refused");
            return;
        }
        vita_step = VITA_DRAIN;              // let the slice status arrive first
        vita_timer->start(1200);
        return;
    case VITA_DAX:
        vita_rx_stream = 0;                  // "dax audio set" is advisory; the stream is the truth
        vita_step = VITA_RX_CREATE;
        VitaSend("stream create type=dax_rx dax_channel=" + QString::number(vita_ch), VITA_RX_CREATE);
        vita_timer->start(3000);
        return;
    case VITA_RX_CREATE:
        if (code != 0)
        {
            VitaFail("DAX RX stream refused");
            return;
        }
        if (!body.isEmpty())
        {
            bool ok = false;
            const quint32 id = body.toUInt(&ok, 16);
            if (ok && id) vita_rx_stream = id;
        }
        if (vita_rx_stream) VitaRxUp();      // else VitaLine() will see it in the status
        return;
    case VITA_TX_CREATE:
        if (code != 0)
        {
            // Receive works without it; say so rather than fail the lot.
            vita_mismatch += " [!] DAX TX stream refused";
            vita_step = VITA_RUN;
            VitaSetStatus();
            return;
        }
        if (!body.isEmpty())
        {
            bool ok = false;
            const quint32 id = body.toUInt(&ok, 16);
            if (ok && id) vita_tx_stream = id;
        }
        if (vita_tx_stream) VitaTxUp();      // else VitaLine() will see it in the status
        return;
    case VITA_MIXER_ON:
    case VITA_MIXER_OFF:
    	{
        	// A radio without a front panel answers 0x500000B7 (verified on a
        	// 6600); remember that so the UI greys the control, not lies.
        	QMutexLocker lk(&flex_mutex);
        	flex_front_spkr_supported = (code == 0);
        	if (code == 0) flex_front_spkr_mute = (step == VITA_MIXER_ON);
    	}
    	return;
    case VITA_ATU:
        // "atu start" needs a transmit slice and the interlock READY.  Keep the
        // code so the panel says the radio refused, rather than a click that
        // seems to do nothing.
        if (code != 0)
        {
            QMutexLocker lk(&flex_mutex);
            flex_atu_refused = QString::number(code, 16).toUpper();
        }
        return;    	
    default:
        return;
    }
}
void Network::VitaTimeout()
{
    switch (vita_step)
    {
    case VITA_IDLE:
    case VITA_RETRY:
        UpdateFlexVita();
        return;
    case VITA_DRAIN:
        VitaChooseSlice();
        return;
    case VITA_SLICE_CREATE:
        {
            const int made = VitaOwnedSlice();
            if (made < 0) 
            { 
            	VitaFail("slice create not answered"); 
            	return; 
            }
            vita_slice = made;
            vita_slice_created = true;
        	// Same as the status-stream path above: the slice we just created is
        	// ours whatever index it landed on, so move the CAT half onto it.
        	if (vita_rig_slice >= 0 && made != vita_rig_slice)
        	{
            	slicenum = QString::number(made);
            	vita_rig_slice = made;
        	}
            VitaSliceReady();
            return;
        }
    case VITA_RX_CREATE:
        if (vita_rx_stream) VitaRxUp();
        else VitaFail("DAX RX stream not granted");
        return;
    case VITA_TX_CREATE:
        if (vita_tx_stream)
        {
            VitaTxUp();
            return;
        }
        vita_mismatch += " [!] DAX TX stream not granted";
        vita_step = VITA_RUN;
        VitaSetStatus();
        return;
    case VITA_GUI:
    case VITA_UDPPORT:
    case VITA_DAX:
        VitaFail("no reply from radio");
        return;
    default:
        return;
    }
}
// Which slice feeds DAX, and is transmitted on.  It MUST be the slice the CAT
// tunes and reads, or MSHV shows one frequency and works another.  Order:
//   1. Rig Control's slice, if it exists -- whoever owns it; the CAT
//      addresses it by number regardless, and following it is what keeps the
//      two halves on one frequency
//   2. a slice this session already owns
//   3. create one
// Only a created slice is ever removed again.
void Network::VitaChooseSlice()
{
    const int want = vita_rig_slice;
    // ONE slice must carry both halves: the audio backend listens to it and the
    // CAT side tunes, keys and reads frequency from it.  If they disagree, MSHV
    // shows one frequency and works another.
    //
    // `want` comes from the rig model letter ("Slice A TCP" -> 0), i.e. it is a
    // GLOBAL slice index.  But the radio's `index_letter` is numbered PER
    // CLIENT: with another GUI client (SmartSDR) connected first, that client
    // holds slice 0 and calls it "A", and the radio hands US slice 1 -- which it
    // ALSO calls "A".  Taking the letter as a global index then adopts the other
    // client's slice: MSHV tunes and keys theirs while the radio, under
    // multiFLEX, feeds our DAX channel from OUR slice, which nobody has moved
    // off the default 14.100 USB.  Waterfall alive, zero decodes on every band,
    // and the other operator watches their slice being dragged around the bands.
    // Measured 2026-09-10, and verified fixed against a second GUI client.
    //
    // So resolve the letter against slices we OWN, and fall back to the global
    // index only when we own none -- which is the single-client case, i.e.
    // exactly the behaviour this had before.
    int pick = VitaOwnedSliceByLetter(want);
    if (pick < 0) pick = VitaOwnedSlice();
    if (pick < 0 && want >= 0 && VitaSliceExists(want)) pick = want;
    if (pick >= 0)
    {
        vita_slice = pick;
        vita_slice_created = false;
        vita_mismatch.clear();
        // Point the CAT half at the same slice.  vita_rig_slice moves with it or
        // UpdateFlexVita()'s restart test (slicenum vs vita_rig_slice) would see
        // a change on every call and rebuild the session in a loop.																						   
        if (pick != want)
        {
            slicenum = QString::number(pick);
            vita_rig_slice = pick;
        }
        VitaSliceReady();
        return;
    }
    vita_step = VITA_SLICE_CREATE;
    // The API takes a frequency at creation -- "slice create [freq=<MHz>]
    // [pan=][ant=][mode=][clone_slice=]" -- so ask for the last one
    // set_freq() sent, when there is one, and the slice appears where MSHV is
    // rather than at the radio's default.  The app re-asserts on
    // VitaSliceReady() regardless, which is what covers the adopt path and a
    // first run with nothing sent yet.
    QString create = "slice create mode=digu";
    if (vita_last_freq_hz > 0)
        create += QString(" freq=%1").arg((double)vita_last_freq_hz / 1000000.0, 0, 'f', 6);
    VitaSend(create, VITA_MISC);
    vita_timer->start(4000);
}
void Network::VitaSliceReady()
{
    // Route the slice to our DAX channel -- receive needs this as much as
    // transmit; a DAX RX stream carries audio only while a slice feeds it.
    vita_step = VITA_DAX;
    VitaSend(QString("dax audio set %1 slice=%2").arg(vita_ch).arg(vita_slice), VITA_DAX);
    vita_timer->start(3000);
    // The slice exists and is ours: tell the app to put it on MSHV's
    // frequency.  A fresh GUI client's slice sits at the radio's default
    // (14.100 USB on the 6600), and MSHV otherwise FOLLOWS the rig, so without
    // this it adopts 20m from the radio and never tunes -- measured 2026-09-09,
    // three minutes untouched.  Deferred a tick: the app's answer comes back
    // down as set_freq() -> writeData(), and writeData() can re-enter
    // readNet(), which is not something to invite from inside the step machine.
    //QTimer::singleShot(0, this, [this]{ emit EmitFlexSliceReady(); });
}
void Network::VitaRxUp()
{
    // The stream id can arrive twice -- in the status stream AND in the
    // create reply, in either order -- so this must be safe to reach twice.
    // Without the guard it once created two DAX TX streams on the radio.
    if (vita_rx_active) return;
    vita_timer->stop();
    vita_rx_active = true;
    QMetaObject::invokeMethod(vita_, "SetRxStream", Qt::QueuedConnection, Q_ARG(uint, (uint)vita_rx_stream));
    if (vita_want_tx)
    {
        vita_tx_stream = 0;
        vita_step = VITA_TX_CREATE;
        VitaSend("stream create type=dax_tx", VITA_TX_CREATE);
        vita_timer->start(3000);
        return;
    }
    vita_step = VITA_RUN;
    VitaSetStatus();
}
void Network::VitaTxUp()
{
    if (vita_tx_active) return;          // same double-arrival as VitaRxUp()
    vita_timer->stop();
    // DAX becomes the transmit audio source, fed from our channel on our slice.
    VitaSend("transmit set dax=1", VITA_MISC);
    VitaSend(QString("dax audio set %1 slice=%2 tx=1").arg(vita_ch).arg(vita_slice), VITA_MISC);
    vita_tx_active = true;
    QMetaObject::invokeMethod(vita_, "SetTxStream", Qt::QueuedConnection, Q_ARG(uint, (uint)vita_tx_stream));
    vita_step = VITA_RUN;
    VitaSetStatus();
}
bool Network::VitaSliceExists(int n)
{
    // Slices come and go and the status reports both, so the LAST line wins.
    const QString want = QString("|slice %1 ").arg(n);
    bool exists = false;
    for (int i = 0; i < vita_log.size(); ++i)
    {
        const QString l = vita_log.at(i);
        if (!l.contains(want)) continue;
        const QString low = l.toLower();
        if (low.contains("in_use=1"))      exists = true;
        else if (low.contains("in_use=0")) exists = false;
    }
    return exists;
}
int Network::VitaOwnedSlice()
{
    const QString want = QString("client_handle=0x%1").arg(vita_handle, 8, 16, QChar('0')).toLower();
    int found = -1;
    for (int i = 0; i < vita_log.size(); ++i)
    {
        const QString low = vita_log.at(i).toLower();
        if (!low.contains("|slice ") || !low.contains("in_use=1") || !low.contains(want)) continue;
        QRegExp rx("\\|slice (\\d+)");
        if (rx.indexIn(vita_log.at(i)) >= 0) found = rx.cap(1).toInt();
    }
    return found;
}
// The slice WE own that the radio labels with this letter (0 = A).
// index_letter is numbered PER CLIENT, so it is only meaningful together with
// client_handle -- see the note in VitaChooseSlice().
int Network::VitaOwnedSliceByLetter(int letter)
{
    if (letter < 0) return -1;
    const QString handle = QString("client_handle=0x%1").arg(vita_handle, 8, 16, QChar('0')).toLower();
    QRegExp isletter(QString("index_letter=%1(\\s|$)").arg(QChar('a' + letter)));
    int found = -1;
    for (int i = 0; i < vita_log.size(); ++i)
    {
        const QString low = vita_log.at(i).toLower();
        if (!low.contains("|slice ") || !low.contains("in_use=1") || !low.contains(handle)) continue;
        if (isletter.indexIn(low) < 0) continue;
        QRegExp rx("\\|slice (\\d+)");
        if (rx.indexIn(vita_log.at(i)) >= 0) found = rx.cap(1).toInt();
    }
    return found;
}
// Tear down.  tell_radio is false when the socket is already gone.
void Network::VitaStop(bool tell_radio)
{
    vita_timer->stop();
    vita_pend.clear();            // anything older is moot; the release below is what may still be pending
    if (is_vita && vThread.isRunning())
        QMetaObject::invokeMethod(vita_, "Reset", Qt::BlockingQueuedConnection);
    if (tell_radio && socket && socket->state()==QAbstractSocket::ConnectedState)
    {
        if (vita_tx_stream) VitaSend(QString("stream remove 0x%1").arg(vita_tx_stream, 8, 16, QChar('0')), VITA_MISC);
        if (vita_rx_stream) VitaSend(QString("stream remove 0x%1").arg(vita_rx_stream, 8, 16, QChar('0')), VITA_MISC);
        if (vita_tx_active) VitaSend("transmit set dax=0", VITA_MISC);
        if (vita_slice >= 0 && vita_slice_created) VitaSend(QString("slice remove %1").arg(vita_slice), VITA_MISC);
    }
    vita_step = VITA_IDLE;
    vita_rx_stream = 0;
    vita_tx_stream = 0;
    vita_rx_active = false;
    vita_tx_active = false;
    vita_slice = -1;
    vita_slice_created = false;
    vita_mismatch.clear();
    {
        QMutexLocker lk(&flex_mutex);
        flex_meters_ok = false;
        flex_ant_list.clear();
        flex_tx_ant_list.clear();
        flex_mode_list.clear();
        flex_rxant.clear();
        flex_txant.clear();
        flex_mode.clear();
        flex_atu_status.clear();      // "sub atu all" on the next start reports afresh
        flex_atu_after_cycle = false;
        flex_atu_refused.clear();        
    }
    vita_status = "Flex Native: idle";
    VitaMirror();
}
void Network::VitaFail(QString why)
{
    VitaStop(true);
    vita_status = "Flex Native: " + why + " - retrying";
    VitaMirror();
    vita_step = VITA_RETRY;
    vita_timer->start(10000);
}
void Network::VitaQuit()
{
    VitaStop(true);
    // Let the release complete on the wire before the process goes: the
    // radio answers each command, and we leave once it has (or 500 ms).
    QElapsedTimer t;
    t.start();
    while (socket && socket->state()==QAbstractSocket::ConnectedState && !vita_pend.isEmpty() && t.elapsed() < 500)
    {
        if (socket->waitForReadyRead(50)) readNet();
    }
}
void Network::VitaSetStatus()
{
    vita_status = QString("Flex Native: RX ch%1%2 slice %3%4")
                  .arg(vita_ch).arg(vita_tx_active ? " + TX" : "").arg(vita_slice).arg(vita_mismatch);
    VitaMirror();
}
void Network::VitaMirror()
{
    s_flex_rx_active = vita_rx_active;
    s_flex_tx_active = vita_tx_active;
    QMutexLocker lk(&flex_mutex);
    flex_my_slice = vita_slice;
#if defined DEBUG_FLEX_TRACE    
    if (s_flex_status != vita_status) fprintf(stderr, "[MSHV Flex] %s\n", qPrintable(vita_status));
#endif    
    s_flex_status    = vita_status;
}
void Network::VitaSetSlice(QString key, QString value)
{
    if (vita_slice < 0 || value.isEmpty()) return;
    VitaSend(QString("slice s %1 %2=%3").arg(vita_slice).arg(key).arg(value), VITA_MISC);
}
// Mute the M-series front-panel speaker, and ONLY that -- lineout and
// headphone are left alone so someone monitoring on phones keeps hearing.
// Per the SmartSDR API ("TCPIP mixer"): the argument is the WORD on/off and
// the command is space separated, although the radio REPORTS the state as
// "lineout_mute=0".  Do not reach for the slice's own audio_mute: that one is
// per client, returns success, and changes nothing audible.
void Network::VitaSetFrontSpeaker(bool mute)
{
    VitaSend(QString("mixer front_speaker mute %1").arg(mute ? "on" : "off"),
             mute ? VITA_MIXER_ON : VITA_MIXER_OFF);
}
// Radio-global transmit settings: rfpower, tunepower, max_power_level,
// hwalc_enabled -- all 0..100 except the last, which is a flag.
//
// NOTE THE SYNTAX IS NOT THE MIXER'S.  "transmit set" takes key=value with an
// EQUALS sign; the mixer command immediately above takes space-separated
// words and REJECTS the equals form.  Two neighbouring commands, two
// conventions -- per the SmartSDR API reference, pages "TCPIP transmit" and
// "TCPIP mixer".  Read the reference before adding a third; guessing
// spellings against live hardware has already cost this project a day.
//
// No clamp here beyond the radio's own: max_power_level is the ceiling and it
// is the radio that enforces it, which is where an operator expects to find it.
void Network::VitaSetTransmit(QString key, int value)
{
    if (key.isEmpty()) return;
    VitaSend(QString("transmit set %1=%2").arg(key).arg(value), VITA_MISC);
}
// The radio's antenna tuner.  "atu start" and "atu bypass" are in the SmartSDR
// API reference, page "TCPIP atu"; "atu set memories_enabled=0|1" is not on that
// page, but it is what AetherSDR sends for its MEM button.  "atu start" runs a
// tune cycle, and the radio TRANSMITS while it tunes.
void Network::VitaAtu(QString args)
{
    if (args.isEmpty()) return;
    {
        QMutexLocker lk(&flex_mutex);
        flex_atu_refused.clear();     // this command's own answer decides now
    }
    VitaSend("atu " + args, VITA_ATU);
}
