/* MSHV
 * By Hrisimir Hristov - LZ2HV
 * May be used under the terms of the GNU General Public License (GPL)
 * MSHV Flex Panel, Copyright (C) 2026 Manoj Ramawarrier, VU2CPL
 * MSHV Native FlexRadio VITA-49 audio and control backend, Copyright (C) 2026 Manoj Ramawarrier, VU2CPL
 */
#include "flexpanel.h"
#include "network.h"
// The band a frequency falls in, and the band's name, MSHV's own tables -- the
// same freq_min_max[] HvTxW::FindRigBandFromFreq() uses and the same lst_bands[]
// the band menu shows, so "the band" means here what it means in the rest of
// the program.  This unit gets its own static copy of the tables, which is what
// the `static` in that header means.
#define _BANDS_H_
#define _FREQTOBAND_H_
#include "../../../config_band_all.h"

#include <QCheckBox>
#include <QComboBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QTimer>
#include <QVBoxLayout>
//#include <QtGui>

static QLabel *ValueLabel()
{
    QLabel *l = new QLabel("--");
    l->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    l->setAlignment(Qt::AlignCenter);
    l->setMinimumWidth(84);
    return l;
}
FlexPanel::FlexPanel(bool dark, QWidget *parent)
    : QDialog(parent)
{
    Q_UNUSED(dark)
    setWindowTitle(tr("FlexRadio Native"));
    setWindowFlags(windowFlags() ^ Qt::WindowContextHelpButtonHint);
    // Non-modal: the operator keeps working with this open.
    setModal(false);
    filling = false;
    // -1 = the radio has told us nothing yet, so nothing may be sent back.
    shown_rf = shown_tune = shown_max = -1;
    
    flex_native_last_freq = "0";
    flex_native_last_rxant = "";
    flex_native_last_txant = "";
    flex_ant_restore = 0; 
    flex_ant_band = -1;       
    flex_up = false;
    flex_push_pending = false;

    QVBoxLayout *V = new QVBoxLayout(this);
    V->setContentsMargins(8, 8, 8, 8);
    V->setSpacing(6);

    l_conn = new QLabel();
    l_conn->setAlignment(Qt::AlignCenter);
    V->addWidget(l_conn);

    QGroupBox *gb_m = new QGroupBox(tr("Radio meters"));
    QGridLayout *G = new QGridLayout();
    G->setContentsMargins(8, 6, 8, 6);
    G->setSpacing(5);
    int r = 0;
    l_fwd    = ValueLabel(); G->addWidget(new QLabel(tr("Forward power")), r, 0); G->addWidget(l_fwd,    r++, 1);
    l_swr    = ValueLabel(); G->addWidget(new QLabel(tr("SWR")),           r, 0); G->addWidget(l_swr,    r++, 1);
    l_ref    = ValueLabel(); G->addWidget(new QLabel(tr("Reflected")),     r, 0); G->addWidget(l_ref,    r++, 1);
    l_alc    = ValueLabel(); G->addWidget(new QLabel(tr("ALC")),           r, 0); G->addWidget(l_alc,    r++, 1);
    l_patemp = ValueLabel(); G->addWidget(new QLabel(tr("PA temperature")),r, 0); G->addWidget(l_patemp, r++, 1);
    l_volts  = ValueLabel(); G->addWidget(new QLabel(tr("Supply")),        r, 0); G->addWidget(l_volts,  r++, 1);
    gb_m->setLayout(G);
    V->addWidget(gb_m);

    QGroupBox *gb_c = new QGroupBox(tr("Slice"));
    QGridLayout *C = new QGridLayout();
    C->setContentsMargins(8, 6, 8, 6);
    C->setSpacing(5);
    cb_rxant = new QComboBox();
    cb_txant = new QComboBox();
    cb_mode  = new QComboBox();
    C->addWidget(new QLabel(tr("RX antenna")), 0, 0); C->addWidget(cb_rxant, 0, 1);
    C->addWidget(new QLabel(tr("TX antenna")), 1, 0); C->addWidget(cb_txant, 1, 1);
    C->addWidget(new QLabel(tr("Mode")),       2, 0); C->addWidget(cb_mode,  2, 1);
    gb_c->setLayout(C);
    V->addWidget(gb_c);

    // Transmit power.  These are radio-global, not slice properties, hence a
    // group of their own rather than sitting under "Slice" above.
    //
    // Spinboxes, committing on Enter or focus-out (editingFinished) rather
    // than on every keystroke or arrow-click: an amplifier is downstream, and
    // a control that fires on valueChanged would both spam the radio and let a
    // stray drag walk the drive up while you watched.
    QGroupBox *gb_p = new QGroupBox(tr("Transmit power"));
    QGridLayout *P = new QGridLayout();
    P->setContentsMargins(8, 6, 8, 6);
    P->setSpacing(5);
    sb_rfpower   = new QSpinBox();
    sb_tunepower = new QSpinBox();
    sb_maxpower  = new QSpinBox();
    QSpinBox *pw[3] = { sb_rfpower, sb_tunepower, sb_maxpower };
    for (int i = 0; i < 3; i++)
    {
        pw[i]->setRange(0, 100);
        pw[i]->setSuffix(" %");
        pw[i]->setKeyboardTracking(false);// do not emit per keystroke
    }
    sb_rfpower->setToolTip(tr("RF drive for normal transmit (transmit set rfpower).\n"
                              "Takes effect on Enter or when the field loses focus."));
    sb_tunepower->setToolTip(tr("RF drive for TUNE only (transmit set tunepower).\n"
                                "Kept separate so a tune-up does not hit the\n"
                                "amplifier at full transmit drive."));
    sb_maxpower->setToolTip(tr("Ceiling the radio enforces on the two above\n"
                               "(transmit set max_power_level)."));
    cb_hwalc = new QCheckBox(tr("Hardware ALC"));
    cb_hwalc->setToolTip(tr("Let an external amplifier's ALC line control drive\n"
                            "(transmit set hwalc_enabled)."));
    int pr = 0;
    P->addWidget(new QLabel(tr("RF power")),   pr, 0); P->addWidget(sb_rfpower,   pr++, 1);
    P->addWidget(new QLabel(tr("Tune power")), pr, 0); P->addWidget(sb_tunepower, pr++, 1);
    P->addWidget(new QLabel(tr("Max power")),  pr, 0); P->addWidget(sb_maxpower,  pr++, 1);
    P->addWidget(cb_hwalc, pr, 0, 1, 2);
    gb_p->setLayout(P);
    V->addWidget(gb_p);

    // The radio's antenna tuner.  Hidden until the radio says it has one
    // (atu_present in "info"), so nobody without a tuner sees controls that do
    // nothing.  Two buttons rather than SmartSDR's single toggle: the button
    // you press says what happens, with no "was it tuned at this frequency?"
    // guess in between.
    gb_atu = new QGroupBox(tr("Antenna tuner"));
    QGridLayout *T = new QGridLayout();
    T->setContentsMargins(8, 6, 8, 6);
    T->setSpacing(5);
    l_atu = ValueLabel();
    pb_atu_tune   = new QPushButton(tr("Tune"));
    pb_atu_bypass = new QPushButton(tr("Bypass"));
    cb_atu_mem    = new QCheckBox(tr("Memories"));
    pb_atu_tune->setToolTip(tr("Runs the radio's antenna tuner (atu start).\n"
                               "The radio TRANSMITS while it tunes -\n"
                               "put an amplifier in standby first."));
    pb_atu_bypass->setToolTip(tr("Takes the tuner out of the antenna path (atu bypass)."));
    cb_atu_mem->setToolTip(tr("Use the tuner's stored settings\n"
                              "(atu set memories enabled)."));
    T->addWidget(new QLabel(tr("Status")), 0, 0); T->addWidget(l_atu, 0, 1);
    T->addWidget(pb_atu_tune, 1, 0);              T->addWidget(pb_atu_bypass, 1, 1);
    T->addWidget(cb_atu_mem, 2, 0, 1, 2);
    gb_atu->setLayout(T);
    gb_atu->hide();
    V->addWidget(gb_atu);

    // Working the radio through MSHV otherwise means hearing it twice: once
    // from the decoded DAX stream and once out of the radio's own speaker.
    QGroupBox *gb_a = new QGroupBox(tr("Local audio"));
    QVBoxLayout *A = new QVBoxLayout();
    A->setContentsMargins(8, 6, 8, 6);
    A->setSpacing(5);
    // Keep this label SHORT: it is the widest thing in the group and the
    // panel sizes itself to it. The detail belongs in the tooltip.
    cb_localmute = new QCheckBox(tr("Mute front speaker"));
    cb_localmute->setToolTip(tr("Mutes the speaker in the radio's front panel.\n"
                                "M series only - other models have no front speaker.\n"
                                "Line-out and headphones are left alone, and the\n"
                                "DAX audio MSHV decodes is unaffected."));
    A->addWidget(cb_localmute);
    l_model = new QLabel();
    l_model->setAlignment(Qt::AlignLeft);
    A->addWidget(l_model);
    gb_a->setLayout(A);
    V->addWidget(gb_a);

    connect(cb_rxant, SIGNAL(currentIndexChanged(int)), this, SLOT(RxAntChanged(int)));
    connect(cb_txant, SIGNAL(currentIndexChanged(int)), this, SLOT(TxAntChanged(int)));
    connect(cb_mode,  SIGNAL(currentIndexChanged(int)), this, SLOT(ModeChanged(int)));
    connect(cb_localmute, SIGNAL(toggled(bool)), this, SLOT(LocalMuteToggled(bool)));
    connect(sb_rfpower,   SIGNAL(editingFinished()), this, SLOT(RfPowerEdited()));
    connect(sb_tunepower, SIGNAL(editingFinished()), this, SLOT(TunePowerEdited()));
    connect(sb_maxpower,  SIGNAL(editingFinished()), this, SLOT(MaxPowerEdited()));
    connect(cb_hwalc,     SIGNAL(toggled(bool)),     this, SLOT(HwAlcToggled(bool)));
    connect(pb_atu_tune,   SIGNAL(clicked()),     this, SLOT(AtuTuneClicked()));
    connect(pb_atu_bypass, SIGNAL(clicked()),     this, SLOT(AtuBypassClicked()));
    connect(cb_atu_mem,    SIGNAL(toggled(bool)), this, SLOT(AtuMemoriesToggled(bool)));    

    timer_speed_one = false;
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(Refresh()));
    timer->start(2000); //qDebug()<<"FlexPanel::Create Speed=2000";
    Refresh();     
}
// The band a frequency belongs to, numbered as MSHV numbers bands everywhere
// else, or -1 when it is outside every band.  The panel files antennas under
// this index and leaves a band it cannot name alone rather than guessing.
int FlexPanel::BandFromFreq(QString hz)
{
    const unsigned long long f = hz.toULongLong();
    for (int i = 0; i < COUNT_BANDS; ++i)
        if (f >= freq_min_max[i].min && f <= freq_min_max[i].max) return i;
    return -1;
}
// The band with this name, or -1 for a name this build does not have.  An
// exact match, not contains(): "5 MHz" is inside "3.5 MHz".  An empty name is
// an unused slot and matches nothing.
int FlexPanel::BandFromName(QString name)
{
    if (name.isEmpty()) return -1;
    for (int i = 0; i < COUNT_BANDS; ++i)
        if (lst_bands[i] == name) return i;
    return -1;
}
// The pair remembered for one band, "" when that band has never been used.
QString FlexPanel::BandAnt(int band, bool tx) const
{
    if (band < 0) return "";
    const QStringList l = flex_band_ant.value(band).split(":");
    if (l.count() != 2) return "";
    return tx ? l.at(1) : l.at(0);
}
void FlexPanel::FlexTrackFreq(QString hz)
{
    if (!flex_up || flex_push_pending) return;
    if (hz.toLongLong() < 100000) return;
    flex_native_last_freq = hz;
    const int band = BandFromFreq(hz);
    if (band != flex_ant_band)
    {
        // A BAND CHANGE, and the radio does only half the job: tuning the slice
        // to a transverter band moves the TX antenna itself and leaves the RX
        // antenna on the HF one, so there is no receive; coming back to HF it
        // moves nothing at all, so TX is left on the transverter port (MM0CEZ,
        // 2026-09-18, on all three of his bands).  Put THIS band's own pair back
        // a moment after the retune, the same way the start-up push does.
        //
        // This is ahead of the restore check below on purpose: a band change
        // arriving while a restore is still outstanding must re-aim it, or the
        // pair for the band just left would be sent to the new one -- which on a
        // transverter band means transmitting into the wrong antenna.
        flex_ant_band = band;
        flex_ant_restore = AntRestoreTicks;
        // Drop the pair the panel is carrying: it belongs to the band just left,
        // and a quit inside this window would otherwise save it against the new
        // one.  The next Refresh() reads the radio's own answer into the gap.
        flex_native_last_rxant = "";
        flex_native_last_txant = "";
        return;
    }
    // Still the same band -- a move inside it, or the start-up push landing on
    // the band StopFlexPushPending() aimed the restore at.  Nothing to do: the
    // antennas of this band still apply, and the pair must NOT be dropped here.
    // That was the whole of the 2026-09-18 trap: the push arrives as an ordinary
    // frequency change (StopFlexPushPending() clears the pending flag and
    // SetDefFreqGlobal() reaches RefreshLRestrict() immediately after), and a
    // clear at this point wiped the very pair the restore was about to put back
    // -- measured on the 6600, the pair was gone one tick before the antenna
    // lists arrived.
}
// One settings line carries all of it -- so the frequency, the antennas and the
// per-band pairs travel together and nothing outside this panel has to know
// about antennas:
//
//     14074000#ANT2#ANT2#14 MHz:ANT2:ANT2#144 MHz:XVTA:XVTA#432 MHz:RX_B:XVTB
//     ^freq    ^rx  ^tx  ^ one per band used: bandname:rxant:txant
//
// Fields 0..2 keep the meaning they had when there was only one pair, so a line
// written by a build that knows just the triple still reads here, and a build
// that knows just the triple still reads a line written here -- it takes the
// three fields it understands and ignores the rest.
//
// Each band is filed under its NAME, as offset_trsv_rig_parms= files the
// transverter offsets (LZ2HV, 2026-09-19): a band added to the table later
// moves every index after it, and would move everybody's antennas with it,
// while a name stays what it is.  A name this build does not have is dropped.
QString FlexPanel::GetFlexLastAll()
{
    const QString hz = GetFlexLastFreq();
    const int band = BandFromFreq(hz);
    QString rx = BandAnt(band, false);
    QString tx = BandAnt(band, true);
    if (rx.isEmpty()) rx = flex_native_last_rxant;
    if (tx.isEmpty()) tx = flex_native_last_txant;
    QString s = hz+"#"+rx+"#"+tx;
    QMapIterator<int,QString> it(flex_band_ant);
    while (it.hasNext())
    {
        it.next();
        s += "#"+lst_bands[it.key()]+":"+it.value();
    }
    return s;
}
void FlexPanel::SetFlexLastAll(QString s)
{
    QStringList l = s.split("#");
    flex_native_last_freq = l.at(0).trimmed();
    if (l.count()>1) flex_native_last_rxant = l.at(1).trimmed();
    if (l.count()>2) flex_native_last_txant = l.at(2).trimmed();
    flex_band_ant.clear();
    for (int i = 3; i < l.count(); ++i)
    {
        const QStringList e = l.at(i).trimmed().split(":");
        if (e.count()!=3) continue;
        // "14 MHz:ANT2:ANT2".  The first per-band build (2026-09-18) put the band
        // INDEX there; such a line is read once and saved back under the name.
        int band = BandFromName(e.at(0));
        if (band<0)
        {
            bool ok = false;
            band = e.at(0).toInt(&ok);
            if (!ok || band>=COUNT_BANDS) band = -1;
        }
        // A band this build does not have is dropped rather than misfiled.
        if (band<0) continue;        	
        if (e.at(1).isEmpty() || e.at(2).isEmpty()) continue;
        flex_band_ant[band] = e.at(1)+":"+e.at(2);
    }
    // A line from a build that knew only the one pair: file it under the band its
    // frequency belongs to, so the start-up restore still happens and the
    // operator loses nothing by updating.
    const int band0 = BandFromFreq(flex_native_last_freq);
    if (band0>=0 && !flex_band_ant.contains(band0)
        && !flex_native_last_rxant.isEmpty() && !flex_native_last_txant.isEmpty())
        flex_band_ant[band0] = flex_native_last_rxant+":"+flex_native_last_txant;
}
// Repopulate without the combo's own change signal firing back at the radio.
void FlexPanel::FillCombo(QComboBox *box, QStringList items, QString current)
{
    if (items.isEmpty()) return;
    // Leave a combo the operator is currently using alone.
    if (box->count() == items.count() && box->currentText() == current) return;

    filling = true;
    box->clear();
    box->addItems(items);
    const int at = items.indexOf(current);
    if (at >= 0) box->setCurrentIndex(at);
    filling = false;
}
// Mirror the radio's value into a spinbox without it echoing straight back,
// and without overwriting a number the operator is in the middle of typing --
// the poll runs every 400 ms, which is easily inside a two-keystroke edit.
void FlexPanel::FillSpin(QSpinBox *box, int value, int &shown)
{
    if (value < 0) return;              // radio has not reported it yet
    if (box->hasFocus()) return;        // being edited right now: leave alone
    shown = value;                      // remember what the radio told us
    if (box->value() == value) return;
    filling = true;
    box->setValue(value);
    filling = false;
}
// Send a power setting ONLY when the operator actually changed the number.
//
// editingFinished() fires on plain focus-out with nothing edited, so the test
// for "did this change?" carries the whole weight.  The first version compared
// the box against the RADIO's current value, which is the wrong reference in
// two reachable cases:
//
//  * at startup the box reads 0 while the radio has reported nothing (-1), so
//    "different" is true and a focus-out would push 0 at the radio;
//  * Flex RF power is PER BAND.  Change band while the box has focus and the
//    radio reports a new rfpower that FillSpin declines to write; the box then
//    holds the PREVIOUS band's number, looks "different", and a focus-out
//    sends it -- silently retuning the operator's power on the new band.
//
// Neither has been observed on the air; both fall straight out of reading the
// code, and with an amplifier downstream an unasked-for power change is worth
// designing out rather than arguing about the odds.
//
// Comparing against `shown` -- the last value WE put in the box -- makes an
// untouched box a no-op by construction, whatever the radio does meanwhile.
void FlexPanel::SendIfEdited(QSpinBox *box, int &shown, int radio_value, const char *key)
{
    if (filling) return;
    if (radio_value < 0) return;        // radio has not reported: never push a guess
    if (shown < 0) return;              // we have never displayed a real value
    if (box->value() == shown) return;  // operator did not change it
    shown = box->value();               // so a second focus-out does not resend
    _FlexVitaSetTransmit_(QString(key), shown);
}
void FlexPanel::Refresh()
{
    bool is_meter = false;
    double fwd = 0.0, ref = 0.0, swr = 0.0, v = 0.0;
    flex_up = (_FlexVitaRxActive_() || _FlexVitaTxActive_());
    /*bool r = false;// For test only
    bool t = false;
    flex_up = (r || t);
    static int c = 0;
    if (c>6 && c<=20)
    {
    	r = true;
    	t = true;    	
    	flex_up = (r || t);
    	if (c>=20) c=0;
   	}
   	c++;*/
    if (flex_up && !timer_speed_one)
    {
        timer_speed_one = true;
        flex_push_pending = true;
        flex_ant_restore = 0;// the push arms it, see StopFlexPushPending()        
        timer->start(500); //qDebug()<<"FlexPanel::Refresh Speed=500"<<flex_push_pending;
    }
    else if (!flex_up && timer_speed_one)
    {
        timer_speed_one = false;
        flex_push_pending = false;
        flex_ant_restore = 0;// backend gone: nothing left to put back        
        timer->start(2000); //qDebug()<<"FlexPanel::Refresh Speed=2000"<<flex_push_pending;
        emit EmitUpdateFlexMeter(false,false,0.0,0.0,flex_push_pending);//reset only one shot
    }
    if (!flex_up)
    {
        l_conn->setText("<b>" + tr("Flex Native: not connected") + "</b>");
        return;
    }
    // The backend's own status line carries the DAX channel and, more
    // importantly, the slice it is actually using -- plus a warning when that
    // is not the slice selected in Rig Control.  A mismatch means MSHV
    // displays one frequency and works another, so it must be visible rather
    // than buried: red, and it is the only thing on this line that changes.
    const QString st = _FlexVitaStatus_();
    if (st.contains("[!]")) l_conn->setText("<b><font color='#ff5050'>" + st + "</font></b>");
    else l_conn->setText("<b>" + st + "</b>");

    is_meter = _FlexVitaMeters_(&fwd, &ref, &swr);
    if (is_meter)
    {
        l_fwd->setText(QString("%1 W").arg(fwd, 0, 'f', (fwd < 10.0) ? 2 : 1));
        l_ref->setText(QString("%1 W").arg(ref, 0, 'f', 2));
        l_swr->setText(QString::number(swr, 'f', 2));
        // Only meaningful under load -- an unkeyed radio reads exactly 1.00.
        if (fwd > 0.5 && swr >= 3.0)      l_swr->setStyleSheet("QLabel{color:rgb(255,80,80);}");
        else if (fwd > 0.5 && swr >= 2.0) l_swr->setStyleSheet("QLabel{color:rgb(255,180,60);}");
        else                              l_swr->setStyleSheet("");
    }
    if (_FlexVitaMeterByName_("ALC", &v))    l_alc->setText(QString("%1 dBFS").arg(v, 0, 'f', 1));
    if (_FlexVitaMeterByName_("PATEMP", &v)) l_patemp->setText(QString("%1 C").arg(v, 0, 'f', 1));
    if (_FlexVitaMeterByName_("+13.8A", &v)) l_volts->setText(QString("%1 V").arg(v, 0, 'f', 2));

    const QStringList rx_list = _FlexVitaAntList_(false);
    const QStringList tx_list = _FlexVitaAntList_(true);
    const QString rx_ant = _FlexVitaAnt_(false);
    const QString tx_ant = _FlexVitaAnt_(true); //QString rx_ant = "XVTA"; QString tx_ant = "XVTA";
    FillCombo(cb_rxant, rx_list, rx_ant);
    FillCombo(cb_txant, tx_list, tx_ant);    
    FillCombo(cb_mode,  _FlexVitaModeList_(),     _FlexVitaMode_());
    // The antennas the operator last used ON THIS BAND: remembered while the
    // backend runs, put back after the start-up frequency push and after every
    // band change.
    //
    // Tuning a slice to a transverter band makes the radio change the TX
    // antenna on its own and leave the RX antenna where it was, so the
    // operator hears nothing until they set it by hand (MM0CEZ, 2026-09-17).
    // The restore waits a tick or two after the push because the radio's own
    // antenna change arrives WITH the retune, and anything sent before it
    // would simply be overwritten.
    //
    // Nothing is remembered while a restore is outstanding: at start-up the
    // radio is reporting the FRESH slice's antennas, and letting those land in
    // the memory would erase the pair that is about to go back -- the same
    // reason FlexTrackFreq() ignores the 14.100 a fresh slice starts on.
    if (flex_ant_restore > 0)
    {
        if (!rx_list.isEmpty() && !tx_list.isEmpty())
        {
            flex_ant_restore--;
            // Two attempts, a second apart: the first once the radio's own
            // change has landed, the second to see it through, because on a
            // transverter band the new ant_list can arrive a tick after the
            // frequency and a pair sent against the old list goes nowhere.  A
            // second attempt with nothing left to do sends nothing at all.
            if (flex_ant_restore == AntSendFirst || flex_ant_restore == AntSendAgain)
            {
                const QString want_rx = BandAnt(flex_ant_band, false);
                const QString want_tx = BandAnt(flex_ant_band, true);
                // contains(): an antenna the radio no longer offers is not sent
                // back at it, and neither is an empty one -- a band with nothing
                // remembered is left exactly as the radio set it up.
                if (rx_list.contains(want_rx) && want_rx != rx_ant)
                    _FlexVitaSetAnt_(false, want_rx);
                if (tx_list.contains(want_tx) && want_tx != tx_ant)
                    _FlexVitaSetAnt_(true, want_tx);                
            }
        }
    }
    else if (!flex_push_pending)
    {
        if (!rx_ant.isEmpty()) flex_native_last_rxant = rx_ant;
        if (!tx_ant.isEmpty()) flex_native_last_txant = tx_ant;
        // File the pair under the band it is being used on, which is what makes
        // the next visit to that band get ITS antennas back rather than the ones
        // the last band left behind.
        if (flex_ant_band < 0) flex_ant_band = BandFromFreq(flex_native_last_freq);
        if (flex_ant_band >= 0 && !rx_ant.isEmpty() && !tx_ant.isEmpty())
            flex_band_ant[flex_ant_band] = rx_ant+":"+tx_ant;        
    }

    FillSpin(sb_rfpower,   _FlexVitaRfPower_(),   shown_rf);
    FillSpin(sb_tunepower, _FlexVitaTunePower_(), shown_tune);
    FillSpin(sb_maxpower,  _FlexVitaMaxPower_(),  shown_max);
    const bool hwalc = _FlexVitaHwAlc_();
    if (cb_hwalc->isChecked() != hwalc)
    {
        filling = true;
        cb_hwalc->setChecked(hwalc);
        filling = false;
    }
    // Antenna tuner: shown only on a radio that reports one, usable only while
    // MSHV owns the transmitter (a tune keys the radio) and the radio says the
    // tuner is enabled.  isHidden(), not isVisible(): the panel itself is
    // created hidden, and isVisible() would be false for that reason alone.
    const bool atu_here = (_FlexVitaAtuPresent_() == 1);
    if (gb_atu->isHidden() == atu_here) gb_atu->setHidden(!atu_here);
    if (atu_here)
    {
        const QString ast = _FlexVitaAtuStatus_();
        QString text, color;
        if      (ast == "TUNE_SUCCESSFUL")    { text = tr("Tuned");            color = "rgb(80,200,80)"; }
        else if (ast == "TUNE_OK")            { text = tr("OK");               color = "rgb(80,200,80)"; }
        else if (ast == "TUNE_IN_PROGRESS")   { text = tr("Tuning...");        }
        // TUNE_BYPASS straight after a cycle means the antenna was already
        // matched and the tuner left itself out: a good result, which FlexRadio
        // counts as success.  From a Memories recall it is just "Bypass".
        else if (ast == "TUNE_BYPASS" && _FlexVitaAtuAfterCycle_())
                                              { text = tr("No tuning required"); color = "rgb(80,200,80)"; }
        else if (ast == "TUNE_BYPASS")        { text = tr("Bypass");           color = "rgb(255,180,60)"; }
        else if (ast == "TUNE_MANUAL_BYPASS") { text = tr("Bypass (manual)");  color = "rgb(255,180,60)"; }
        else if (ast == "TUNE_FAIL_BYPASS")   { text = tr("Failed, bypassed"); color = "rgb(255,80,80)"; }
        else if (ast == "TUNE_FAIL")          { text = tr("Failed");           color = "rgb(255,80,80)"; }
        else if (ast == "TUNE_ABORTED")       { text = tr("Aborted");          color = "rgb(255,80,80)"; }
        else if (ast == "TUNE_NOT_STARTED")   { text = tr("Not tuned");        }
        else if (ast.isEmpty() || ast == "NONE") text = "--";
        else                                     text = ast;   // a word this build does not know: show the radio's own
        if (_FlexVitaAtuUsingMem_()) text += " (" + tr("memory") + ")";
        const QString refused = _FlexVitaAtuRefused_();
        if (!refused.isEmpty())
        {
            text  = tr("Refused by the radio") + " 0x" + refused;
            color = "rgb(255,80,80)";
        }
        if (l_atu->text() != text) l_atu->setText(text);
        const QString css = color.isEmpty() ? QString() : "QLabel{color:" + color + ";}";
        if (l_atu->styleSheet() != css) l_atu->setStyleSheet(css);

        const bool usable = _FlexVitaTxActive_() && _FlexVitaAtuEnabled_();
        pb_atu_tune->setEnabled(usable && ast != "TUNE_IN_PROGRESS");
        pb_atu_bypass->setEnabled(usable);
        cb_atu_mem->setEnabled(usable);
        const bool mem = _FlexVitaAtuMemories_();
        if (cb_atu_mem->isChecked() != mem)
        {
            filling = true;
            cb_atu_mem->setChecked(mem);
            filling = false;
        }
    }
    // Only an M series radio has a front speaker; on anything else the
    // command does not exist, so grey the control rather than offer one that
    // silently does nothing.
    const bool has_spkr = _FlexVitaHasFrontSpeaker_();
    if (cb_localmute->isEnabled() != has_spkr) cb_localmute->setEnabled(has_spkr);

    const bool muted = _FlexVitaFrontSpeakerMute_();
    if (cb_localmute->isChecked() != muted)
    {
        filling = true;
        cb_localmute->setChecked(muted);
        filling = false;
    }
    // Model only -- no explanatory suffix. Why the box is greyed goes in the
    // tooltip, which costs no width; a sentence here stretched the whole
    // panel to fit it.
    const QString model = _FlexVitaRadioModel_();
    if (!model.isEmpty() && l_model->text().isEmpty())
    {
        l_model->setText(QString("Radio: %1").arg(model));
        cb_localmute->setToolTip(has_spkr
                                 ? tr("Mutes the speaker in the radio's front panel.\n"
                                      "Line-out and headphones are left alone, and the\n"
                                      "DAX audio MSHV decodes is unaffected.")
                                 : QString("%1 ").arg(model) + tr("has no front panel speaker.\n"
                                      "This control is for M series radios."));
    }
    emit EmitUpdateFlexMeter(flex_up,is_meter,fwd,swr,flex_push_pending); //qDebug()<<rx_and_tx<<has_met<<fwd<<swr;//emit EmitUpdateFlexMeter(true,true,1.0,1.8);
}
// The operator's own pick wins over a restore that has not fired yet -- a band
// change arms one for the next couple of seconds, and setting an antenna by hand
// inside that window must not be undone a moment later.  What they chose is
// learned back into this band's pair on the next tick, from the radio.
void FlexPanel::RxAntChanged(int)
{
    if (filling) return;
    flex_ant_restore = 0;    
    _FlexVitaSetAnt_(false, cb_rxant->currentText());
}
void FlexPanel::TxAntChanged(int)
{
    if (filling) return;
    flex_ant_restore = 0;    
    _FlexVitaSetAnt_(true, cb_txant->currentText());
}
void FlexPanel::ModeChanged(int)
{
    if (filling) return;
    _FlexVitaSetMode_(cb_mode->currentText());
}
// editingFinished() also fires on focus-out with the value unchanged, so each
// of these would re-send the radio its own setting every time the panel lost
// focus. Harmless but noisy on the CAT link, so send only a real change.
void FlexPanel::RfPowerEdited()
{
    SendIfEdited(sb_rfpower, shown_rf, _FlexVitaRfPower_(), "rfpower");
}
void FlexPanel::TunePowerEdited()
{
    SendIfEdited(sb_tunepower, shown_tune, _FlexVitaTunePower_(), "tunepower");
}

void FlexPanel::MaxPowerEdited()
{
    SendIfEdited(sb_maxpower, shown_max, _FlexVitaMaxPower_(), "max_power_level");
}
void FlexPanel::HwAlcToggled(bool on)
{
    if (filling) return;
    _FlexVitaSetTransmit_("hwalc_enabled", on ? 1 : 0);
}
void FlexPanel::LocalMuteToggled(bool on)
{
    // Refresh() sets this box from the backend's own state; without the guard
    // that would bounce straight back at the radio as a fresh command.
    if (filling) return;
    _FlexVitaSetFrontSpeakerMute_(on);
}
// A tune cycle keys the radio, so it goes out only from this click -- never
// from Refresh() -- and the button is enabled only while MSHV owns the
// transmitter.
void FlexPanel::AtuTuneClicked()
{
    _FlexVitaAtuTune_();
}
void FlexPanel::AtuBypassClicked()
{
    _FlexVitaAtuBypass_();
}
void FlexPanel::AtuMemoriesToggled(bool on)
{
    // Refresh() mirrors the radio's own setting into this box; don't echo it.
    if (filling) return;
    _FlexVitaSetAtuMemories_(on);
}
