/* MSHV
 * By Hrisimir Hristov - LZ2HV
 * May be used under the terms of the GNU General Public License (GPL)
 */
/* 
MSHV TCI Client
Copyright (c) 2017 Expert Electronics
Distributed under the MIT software license, see the accompanying
file COPYING or http://www.opensource.org/licenses/mit-license.php.
TCI Client modified by Hrisimir Hristov, LZ2HV 2021
MSHV Native FlexRadio VITA-49 audio and control backend, Copyright (C) 2026 Manoj Ramawarrier, VU2CPL
*/

#ifndef NETWORK_H
#define NETWORK_H

#include <QWidget>
#include <QTimer>

#include "../rigdef.h"
#define QT_NO_DEBUG_STREAM // remove qdebug socet
#include <QTcpSocket>
#include <QtWebSockets/QWebSocket> //tci
#include <QThread>

//----- vita49 ---------------------------------------------------
#include <QUdpSocket>    //flex native vita-49
#include <QHostAddress>  //flex native vita-49
#include <QElapsedTimer> //flex native vita-49
#include <QHash>         //flex native vita-49
#include <QStringList>   //flex native vita-49
//----- end vita49 -----------------------------------------------

class ThreadRefr : public QThread
{
    Q_OBJECT
signals:
    void refresh();
private:
    //QMutex m_mutex;
    void run()
    {
        while (1)
        {
            //QMutexLocker locker(&m_mutex);
            msleep(11);
            emit refresh();         
        }
    }
};
extern bool _SetTxAudioTci_(int *raw);//, int size
extern void _SetTciBuffReset_();
#define DSMAX 32768 //32768
#define BCMAX 30
class HvWebSocket : public QWebSocket
{
    Q_OBJECT
public:
    HvWebSocket(quint32);
    virtual ~HvWebSocket();
	void SetBufferCommand(QString);	
	//void SetSamplerate(int);

//signals:
 
//public slots:

private slots:
	void ReadCommand();
	void mBinaryMessageReceived(const QByteArray &);
	//void StateChanged(QAbstractSocket::SocketState);
	//void onError(QAbstractSocket::SocketError errorCode);//tci

private:
	quint32 tci_trx;
	int sample_rate;//tci
	QByteArray t_txAudioData; //tci
	int rawrxm[DSMAX+4096];  //tci=8192<-max]; 16384
	uint8_t pos_writ;
	uint8_t pos_read;
	QString wcommands[BCMAX+10];
};

//----- vita49 ---------------------------------------------------
// ---------------------------------------------------------------- flex native vita-49
//
// Hooks for the rest of MSHV.  All are safe to call at any time, on any
// rig model; they answer "idle" / false / empty until the Flex backend is up.
// Audio comes and goes through the same seam the TCI client uses:
// _SetRxAudioTci_() for receive, _SetTxAudioTci_() for transmit (which hands
// the block to _SetTxAudioFlex_() when Flex owns the output device).
//extern bool        _SetTxAudioFlex_(int *raw);
extern bool        _FlexVitaRxActive_();
extern bool        _FlexVitaTxActive_();
extern QString     _FlexVitaStatus_();
// Live metering off the radio.  False until a meter packet has been seen.
extern bool        _FlexVitaMeters_(double *fwd_w, double *ref_w, double *swr);
extern bool        _FlexVitaMeterByName_(const char *name, double *value);
// Antenna / mode, mirrored from the slice so the UI offers only what the
// radio accepts.  Empty until the slice status has arrived.
extern QStringList _FlexVitaAntList_(bool tx);
extern QString     _FlexVitaAnt_(bool tx);
extern void        _FlexVitaSetAnt_(bool tx, QString ant);
extern QStringList _FlexVitaModeList_();
extern QString     _FlexVitaMode_();
extern void        _FlexVitaSetMode_(QString mode);
// The M-series front-panel speaker (and only that -- lineout and headphone
// are left alone).  HasFrontSpeaker is false on a radio without one.
extern bool        _FlexVitaFrontSpeakerMute_();
extern void        _FlexVitaSetFrontSpeakerMute_(bool on);
extern bool        _FlexVitaHasFrontSpeaker_();
extern QString     _FlexVitaRadioModel_();   // e.g. "FLEX-6600M"
extern int         _FlexVitaRfPower_();
extern int         _FlexVitaTunePower_();
extern int         _FlexVitaMaxPower_();
extern bool        _FlexVitaHwAlc_();
extern void        _FlexVitaSetTransmit_(QString key, int value);
// The radio's antenna tuner.  AtuPresent is -1 until the radio has said, then
// 0 or 1.  AtuStatus is the radio's own word (TUNE_SUCCESSFUL, TUNE_BYPASS, ...).
// AtuRefused is the hex code of the last atu command refused, else empty.
extern int         _FlexVitaAtuPresent_();
extern QString     _FlexVitaAtuStatus_();
extern bool        _FlexVitaAtuEnabled_();
extern bool        _FlexVitaAtuMemories_();
extern bool        _FlexVitaAtuUsingMem_();
extern QString     _FlexVitaAtuRefused_();
extern bool        _FlexVitaAtuAfterCycle_();  // the current status is the result of a tune cycle
extern void        _FlexVitaAtuTune_();      // TRANSMITS while the radio tunes
extern void        _FlexVitaAtuBypass_();
extern void        _FlexVitaSetAtuMemories_(bool on);

// Native FlexRadio VITA-49 DAX audio -- the UDP half only.
//
// The control half (SmartSDR "C<seq>|" commands: client gui, subscriptions,
// client udpport, stream create/remove, slice choice, meters, mixer) lives in
// Network and rides the SAME TCP socket as the CAT session, so MSHV is one
// API client to the radio, not two.  This object owns nothing but the UDP
// socket, and runs in its own thread (Network::vThread) exactly as the TCI
// HvWebSocket does in mThread: DAX is ~190 packets a second each way and
// neither direction may ever wait on the GUI thread.
//
// Everything that crosses a thread is one of three things: a queued slot
// call (Network -> here), a mutex-guarded copy (meters: here -> Network), or
// the atomic index pair of the TX ring (Rawplayer thread writes, this reads).
//
// Wire format, measured against a FLEX-6600 -- DAX TX is NOT the mirror of
// DAX RX, see SendTxPacket():
//   RX  radio -> us   type 3, class 0x534C03E3, 24 kHz stereo float32 BE, L==R
//   TX  us -> radio   type 1, class 0x534C0123, 24 kHz MONO int16 BE, 284 bytes
class FlexVita : public QObject
{
    Q_OBJECT
public:
    static const int RadioAudioRate  = 24000; // DAX sample rate, both ways
    static const int FramesPerPacket = 128;   // 5.33 ms per packet
    static const int VitaUdpPort     = 4991;  // the radio's inbound VITA port

    FlexVita();
    virtual ~FlexVita();
    quint16 LocalPort() const;          // bound in the ctor; give it to "client udpport"
    bool PushTxBlock(int *raw);         // Rawplayer thread: 4096 ints, stereo 48 kHz

public slots:
    // Invoked queued from Network's thread; plain types so no metatype
    // registration is needed for the queued arguments.
    void SetRadio(QString);             // where TX datagrams go = the CAT socket's peer
    void SetRxStream(uint);             // DAX RX stream id; 0 stops receive
    void SetTxStream(uint);             // DAX TX stream id; 0 = TX torn down
    void SetTxKeyed(bool);              // PTT: stream audio only while keyed
    void Reset();

private slots:
    void ReadAudio();
    void SendTxPacket();

private:
    bool ParseVita(const char *data, int len, int *payload_offset, int *payload_bytes);

    QUdpSocket   *audio_;
    QTimer       *tx_timer_;
    QHostAddress  radio_addr_;
    quint32       rx_stream_;
    quint32       tx_stream_;
    quint8        tx_packet_count_;     // VITA 4-bit sequence
    bool          tx_keyed_;            // audio flows only between key and unkey
    QElapsedTimer tx_clock_;
    qint64        tx_sent_frames_;
    int           rx_scratch_[8192];
};
//----- end vita49 ---------------------------------------------------

class Network : public QWidget
{
    Q_OBJECT
public:
    Network(int ModelID,QWidget *parent = 0);
    virtual ~Network();

signals:
    void EmitRigSet(RigSet);
    void EmitWriteCmd(char*data,int size);
    void EmitReadedInfo(CmdID,QString);
    void EmitNetConnInfo(QString,bool,bool);//info,connect,ready to use
    void EmitFullRigInfo(QString);//2.76.1 for pskreporter
    //void EmitFlexSliceReady();     //flex native vita-49: our slice exists and is ours -- app asserts frequency
 
public slots:
	void ConnectNet(QString);	
	//void SetMode(int);//tci
	void SetOnOffCatCommand(bool,int,int);//tci
	//void SetTciRig(int);//tci

private slots:
    void initAll();
    void readNet();
    void connected_s();
    void disconnected_s();
    void SetCmd(CmdID,ptt_t,QString);
    //void SetReadyRead(QByteArray,int); 
    void wTextMessageReceived(const QString &);//tci
    void SetTciTxOnRX2();//tci
    void SetTciSelect(int,int,bool);//tci
    
	//----- vita49 ---------------------------------------------------    
    void VitaTimeout();//flex native vita-49
    void VitaQuit();//flex native vita-49: release the radio on the way out
	//----- end vita49 --------------------------------------------------- 
	   
    //void onError(QAbstractSocket::SocketError errorCode);//tci

private:
	QString GetModeStr(QString);
	QString GetModeStrKenwood(QChar);
    QString s_nethost;
    QString s_netport;
     
    QString s_tcich;
    QString s_tcismp;
    QString s_tcityp;
    QString s_tcitxbuff;
    QString s_tcisamprate;
    
	QTcpSocket *socket;
	bool is_wsocket;//tci
	QThread mThread;//tci
	HvWebSocket *wsocket;//tci
	void connectToHost();
	bool writeData(QString str,bool id,char*);
	
	bool tci_start_stop_state;//tci
	bool is_tci_trx; //tci
	QString tci_trx; //tci
	//bool tci_tx_enable[2];//tci
	//bool tci_rx_enable[2];//tci
	bool tci_tx_enable;//tci
	bool tci_rx_enable;//tci
	bool tci_rx_mute;//tci
	int tci_drive;//tci
	bool tci_split_enable;//tci
	QString wdemanf;//tci
	QString wdevice;//tci
	QStringList lsV012;
	int id_tci_prot;//tci
	//int sample_rate;//tci
	int tci_select;//tci
	bool isMyTCICommand(QString);//tci
	void SetTciStrtStopAudio(bool);
	
	//----- vita49 --------------------------------------------------- 	
	int vita_rx;//flex native vita-49: RX DAX channel, 0 = off
	bool vita_tx;//flex native vita-49: TX enabled
	//flex native vita-49: control on `socket` (one API session with the CAT),
	//UDP audio in vita_ on vThread.  See the block in network.cpp.
	void UpdateFlexVita();
	void VitaStart();
	void VitaStop(bool tell_radio);
	void VitaFail(QString why);
	int  VitaSend(QString cmd,int step);
	void VitaLine(QString line);
	void VitaReply(int step,quint32 code,QString body);
	void VitaChooseSlice();
	void VitaSliceReady();
	void VitaRxUp();
	void VitaTxUp();
	void VitaSetStatus();
	void VitaMirror();
	bool VitaSliceExists(int n);
	int  VitaOwnedSlice();
	int  VitaOwnedSliceByLetter(int letter);//index_letter is per client -- see VitaChooseSlice()
	FlexVita *vita_;
	QThread vThread;
	bool is_vita;                //vita_ exists and vThread runs
	int  vita_step;              //where the start-up sequence is, VITA_* below
	QHash<int,int> vita_pend;    //command seq -> step, to route "R<seq>|" replies
	QByteArray vita_buf;         //line assembly: readNet() hands over raw chunks
	QStringList vita_log;        //recent status lines, for the slice search
	quint32 vita_handle;         //our client handle, from the "H" line
	int  vita_ch;                //DAX channel in use
	int  vita_slice;             //slice feeding DAX, -1 = none
	bool vita_slice_created;     //only ever remove a slice we made ourselves
	quint32 vita_rx_stream;
	quint32 vita_tx_stream;
	bool vita_rx_active;
	bool vita_tx_active;
	bool vita_want_tx;
	unsigned long long vita_last_freq_hz; //last set_freq() for this rig, for "slice create freq="; 0 = none yet
	int  vita_rig_slice;         //Rig Control's slice when we started, for the restart test
	bool vita_gui_done;          //"client gui" already sent on this connection
	QString vita_last_slice_line;//this chunk's status line for OUR slice, for the CAT parser
	QString vita_status;
	QString vita_mismatch;       //non-empty when audio and Rig Control disagree
	QTimer *vita_timer;          //step time-outs and the retry
public:
	void VitaSetSlice(QString key,QString value);//flex native vita-49: rxant/txant/mode from the panel
	void VitaSetTransmit(QString key,int value);  //flex native vita-49: rfpower/tunepower/max_power_level/hwalc from the panel
	void VitaSetFrontSpeaker(bool mute);          //flex native vita-49: M-series speaker
	void VitaAtu(QString args);                   //flex native vita-49: antenna tuner from the panel
	FlexVita *VitaAudio();                        //flex native vita-49: the UDP side, for _SetTxAudioFlex_()
	//----- end vita49 --------------------------------------------------- 
	
private:
	//flrig
	char *FLRig_xml_build(char *cmd, char *value, char *xmlbuf,int xmllen);
	QString FLRig_get_value(QString);
	int seqnum;
	QString slicenum;
	bool fsdrs;
	bool fsdrs_poll;

    int  s_ModelID;
    void set_ptt(ptt_t);    
    int s_CmdID;
    void set_freq(unsigned long long);
    void get_freq();
    void set_mode(QString);
    void get_mode();
protected:
    QTimer *timer_init;

};
#endif
