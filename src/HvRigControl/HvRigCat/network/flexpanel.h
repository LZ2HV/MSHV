/* MSHV
 * By Hrisimir Hristov - LZ2HV
 * May be used under the terms of the GNU General Public License (GPL)
 */
/*
 * Native Flex VITA-49 control / monitor panel.
 *
 * A small non-modal window showing the radio's own meters and offering the
 * two controls that are awkward to reach otherwise: antenna and mode.
 *
 * Everything here is driven from what the radio reports. The antenna and mode
 * dropdowns are populated from the slice's own ant_list / tx_ant_list /
 * mode_list, so MSHV never offers a choice the radio would reject, and the
 * lists follow the hardware rather than a table that goes stale.
 *
 * The panel is inert when the Flex backend is not running: it simply shows
 * that it is not connected.
 * MSHV Flex Panel, Copyright (C) 2026 Manoj Ramawarrier, VU2CPL
 * MSHV Native FlexRadio VITA-49 audio and control backend, Copyright (C) 2026 Manoj Ramawarrier, VU2CPL
 */
 
#ifndef FLEXPANEL_H
#define FLEXPANEL_H

#include <QDialog>
#include <QMap>

class QLabel;
class QComboBox;
class QCheckBox;
class QSpinBox;
class QGroupBox;
class QPushButton;
class QTimer;

class FlexPanel : public QDialog
{
    Q_OBJECT
public:
    explicit FlexPanel(bool dark, QWidget *parent = 0);
    
    void FlexTrackFreq(QString);
   	void StopFlexPushPending()
   	{
   		flex_push_pending = false;
   		// The frequency has just gone to the radio, so the slice is about to
   		// retune and the radio may change the TX antenna itself.  Put the
   		// remembered pair back a tick or two after that, not before it.
   		//flex_ant_restore = 2;  
   		flex_ant_band = BandFromFreq(GetFlexLastFreq());
		flex_ant_restore = AntRestoreTicks; 		
  	}
  	QString GetFlexLastFreq()
    {
    	if (flex_native_last_freq.toLongLong() < 100000) flex_native_last_freq = "14074000";   // nothing remembered: 20m FT8
        return flex_native_last_freq;
    };  
  	QString GetFlexLastAll();
	void SetFlexLastAll(QString s);
    
public slots:
	//void ShowFlexPanel();
private slots:
    void Refresh();
    void RxAntChanged(int);
    void TxAntChanged(int);
    void ModeChanged(int);
    void LocalMuteToggled(bool);
    void RfPowerEdited();
    void TunePowerEdited();
    void MaxPowerEdited();
    void HwAlcToggled(bool);
    void AtuTuneClicked();
    void AtuBypassClicked();
    void AtuMemoriesToggled(bool);
signals:
	void EmitUpdateFlexMeter(bool,bool,double,double,bool);

private:
    void FillCombo(QComboBox *box, QStringList items, QString current);
    // Show the radio's value without the spinbox echoing it straight back, and
    // without stamping on a number the operator is part-way through typing.
    // `shown` records what we last displayed, which is how an edit by the
    // operator is told apart from a value the radio gave us.
    void FillSpin(QSpinBox *box, int value, int &shown);
    // Common guard for the three power spinboxes: send only a value the
    // OPERATOR changed, never one we merely displayed.  See the comment at
    // RfPowerEdited() -- getting this wrong transmits at the wrong power.
    void SendIfEdited(QSpinBox *box, int &shown, int radio_value, const char *key);

    QLabel    *l_conn;
    QLabel    *l_fwd;
    QLabel    *l_ref;
    QLabel    *l_swr;
    QLabel    *l_alc;
    QLabel    *l_patemp;
    QLabel    *l_volts;
    QComboBox *cb_rxant;
    QComboBox *cb_txant;
    QComboBox *cb_mode;
    QCheckBox *cb_localmute;
    QSpinBox  *sb_rfpower;
    QSpinBox  *sb_tunepower;
    QSpinBox  *sb_maxpower;
    QCheckBox *cb_hwalc;
    QLabel    *l_model;
    QGroupBox *gb_atu;
    QLabel    *l_atu;
    QPushButton *pb_atu_tune;
    QPushButton *pb_atu_bypass;
    QCheckBox *cb_atu_mem;    
    bool 	  timer_speed_one;
    QTimer    *timer;
    bool      filling;   // suppress the change signals while repopulating
    // What we last DISPLAYED in each power box, -1 = nothing yet.  A box whose
    // value still equals this was not touched by the operator, so it must not
    // be sent back to the radio.
    int        shown_rf;
    int        shown_tune;
    int        shown_max;
    bool	   flex_up;
    bool       flex_push_pending;
    QString    flex_native_last_freq; // Hz, persisted in ms_settings; empty = nothing remembered
    // The antennas that went with it, saved on the same line.  The radio picks
    // the TX antenna itself when a slice is tuned to a transverter band, but
    // leaves the RX antenna where it was -- which is silence on that band.
    QString    flex_native_last_rxant;
    QString    flex_native_last_txant;
    //int        flex_ant_restore; // Refresh ticks left before they go back, 0 = nothing to do
    int        flex_ant_restore; // Refresh ticks left in the restore, 0 = nothing to do
    // One pair per band -- "rxant:txant" under MSHV's own band index, saved
    // under the band's NAME -- because one pair cannot serve a station with
    // transverters: the antennas that make 2m work are the wrong ones on 20m,
    // and the radio only ever changes half of them by itself.  A band nobody
    // has used is simply not in here, and then nothing is sent and the radio's
    // own choice stands.
    QMap<int,QString> flex_band_ant;
    int        flex_ant_band;    // band the slice is on, -1 = not known / not a band
    // Ticks of the 500 ms panel timer.  The restore is armed with the first and
    // sends at the other two: one second to let the radio's own antenna change
    // land, then a second attempt a second later in case the new band's antenna
    // lists had not arrived yet.
    enum { AntRestoreTicks = 6, AntSendFirst = 4, AntSendAgain = 1 };
    static int BandFromFreq(QString hz);
    static int BandFromName(QString name);
    QString    BandAnt(int band, bool tx) const;
};

#endif
