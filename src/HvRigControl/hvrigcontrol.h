/* MSHV
 *
 * By Hrisimir Hristov - LZ2HV
 * May be used under the terms of the GNU General Public License (GPL)
 */

#ifndef HVRIGCONTROL_H
#define HVRIGCONTROL_H

#include <QDialog>
#include <QLabel>
#include <QVBoxLayout>
#include <QComboBox>
#include <QRadioButton>
#include <QPushButton>
#include <QGroupBox>
#include <QSpinBox>
#include <QLineEdit>

#include "HvRigCat/hvrigcatw.h"

#include "../config.h"
//#include <unistd.h> // uslepp Linux and windows
//#include <windows.h>  // Slepp windows
//#if defined _LINUX_
//#include <unistd.h>
//#endif

#include "../config_band_all.h"
class HvRigControl : public QDialog
{
    Q_OBJECT
public:
    HvRigControl(QWidget *parent = 0);
    virtual ~HvRigControl();

    void SetMode(int mod);
    //void SetFont(QFont f);
    void DestroyPort();
    void SetPtt(bool,int);//id 0=All 1=p1 2=p2
            
    QString get_port_name()
    {
        return cb_port->currentText();
    };
    QString get_port_baud()
    {
        return cb_baud->currentText();
    };
    QString get_ptt_rts_dtr()
    {
        QString s = "0";
        if (rb_rts->isChecked()) s = "1";
        if (rb_cat->isChecked()) s = "2";
        if (rb_ptt_off->isChecked()) s = "3";    
        return s;
    };
    QString get_rig_name()
    {
        return THvRigCat->get_rig_name();
    };
    QString get_ptt_cat_type()
    {
        return THvRigCat->get_ptt_type();
    };
    QString get_tx_watchdog_parms()
    {
        return QString("%1").arg(f_no_time_count_txwatchdog)+"#"+QString("%1").arg(s_txwatchdog_time)+"#"+
               QString("%1").arg(s_txwatchdog_coun);
    };
    QString get_read_data_rts_on()//to settings
    {
    	if (cb_read_data_kenwood_rts_on->isChecked() && cb_read_data_kenwood_dtr_on->isChecked()) return "3";
    	else if (cb_read_data_kenwood_dtr_on->isChecked()) return "2";
    	else if (cb_read_data_kenwood_rts_on->isChecked()) return "1";
    	else return "0";	 
    };
    
    
    QString get_port_name2()
    {
        return cb_port2->currentText();
    };
    QString get_port_baud2()
    {
        return cb_baud2->currentText();
    };
    QString get_ptt_rts_dtr2()
    {
        QString s = "0";
        if (rb_rts2->isChecked()) s = "1";   
        return s;
    };
    QString off_set_frq_to_rig()
    {
        QString s = "0";
        if (cb_off_set_frq_to_rig->isChecked()) s = "1";   
        return s;
    };
    QString static_tx_parms()
    {
        QString s = "0";
        if (rb_static_tx->isChecked()) s = "1";  
        s.append("#");
        s.append(QString("%1").arg(sb_static_tx->value()));          
        return s;
    };
    QString mod_set_frq_to_rig()
    {
        QString s = "0";
        if (cb_mod_set_frq_to_rig->isChecked()) s = "1";
        s.append("#"+QString("%1").arg(cb_rig_mod->currentIndex()));   
        return s;
    };
    QString net_rig_srv_port();
    /*QString net_tci_settings()
    {
    	return (tci_channels->currentText()+"#"+tci_samples->currentText());
   	};
   	void SetNetTciSettings(QString);*/
    
    void SetStaticTxParms(QString);
    void SetPortName(QString);
    void SetPortBaud(QString);
    void SetupPtt(QString);
    void SetRigName(QString);
    void SetPttCatType(QString);
    void SetupReadDataRtsOn(QString str);    
    
    void SetPortName2(QString);
    void SetPortBaud2(QString);
    void SetupPtt2(QString);    
    
    void SetTxWatchdogParms(QString);
    
    QString get_offset_trsv_rig_parms();
    void SetOffsetTrsvRigParms(QString);
    void SetBand(QString);
    void SetOffSetFrqToRig(QString);
    void SetModSetFrqToRig(QString);
	void SetNetRigSrvPort(QString s);
	void CatStopPttIfClose();
	void SetMsf(bool f);

//public slots:	
 
signals:
	void EmitStaticTxFrq(bool,int);
	void EmitTxActive(int);
////// omnirig /////////////
#if defined _WIN32_    
    void EmitOmniRigPttMethod(bool*);     
#endif
////// omnirig end /////////////
	void EmitNetSP(QString); 
    void EmitTxWatchdogParms(int,int,int);
    void EmitGetedFreq(QString s);
    void EmitGetedMode(QString s); 
    void EmitModSetFrqToRig(bool);
    void EmitQrgActive(int i);//2.45
    void EmitQrgFromRig(QString);//2.45
    void EmitQrgInfoFromCat(QString);//2.45
    void EmitRigCatActiveAndRead(bool f,QString);//2.53 //2.76.1
    
private slots:
    void SetCatAactiveAndRead(bool,bool);
    void SBStaticTxChanged(int);
    void RbSetStaticTxF(bool f);
    void TimerTryStaticTxF();
    void TCPSPChanged(QString);
    
////// omnirig /////////////
#if defined _WIN32_
    void SetOmniRigActive(bool,QString);
#endif
    void SetNetConnect();
    void SetNetConnInfo(QString,bool,bool);
////// omnirig end /////////////
    ////////////////////////////////////////////////////new read com    
    void SetTxFreq(double);   
    void SetFreq(QString,int);
    
    void SetQrgParms(QString,bool); //2.45
    void SetQSOProgress(int); //2.45
    //void SetMode(QString);//stop for the moment
    ////////////////////////////////////////////////////end new read com 
    void PortNameChanged(QString);
    void PortNameChanged2(QString);
    void BaudRateChanged(QString);
    void BaudRateChanged2(QString);
    void PttChanget(bool);
    void PttChanget2(bool);
    void TestPtt();
    void TestPtt2();
    void SetWriteCmd(char*data,int size);
    void SetRigSet(RigSet,int,int);
    void RbTxwatchChanged(bool);
    void SBTxwatchChanged(int);
 
    
    ////////////////////////////////////////////////////////new read com
    //void set_freq();//za mahane posle
    //void set_mode();//za mahane posle
    void onReadyRead();
    void SetGetedFreq(QString);
    void SetGetedMode(QString);
    void ReadDataRtsOnChanget(bool);
    void ReadDataDtrOnChanget(bool);
    ////////////////////////////////////////////////////////end new read com 
    void RbOffsetTR(bool);
    void LeOffsetTrsvRigChange(QString);
    void CbOffSetFrqToRig(bool);
    void SetPttDtr(bool);// sea-235
    void SetTciSelect(int);
    void SetRigModeFreq();
    void SetFullRigInfo(QString);//2.76.1
    
private:
	bool f_msf;//2.76sf
	bool f_msf_new;//2.76sf
	uint8_t f_msf_special_tx_rx;//2.76sf
	int s_net_model_id;
	QString s_rig_name;
	QString s_rig_full_name;//2.76.1
	bool rig_cat_active_and_read;
    void SetPtt_p(bool,int);//2.17 id 0=All 1=p1 2=p2
	////// omnirig /////////////
    bool omnirig_active;
    bool omnirig_false_refresh;
    QString omnirig_type;
	////// omnirig end /////////////
    QComboBox *cb_port;
    QComboBox *cb_baud;
    QComboBox *cb_port2;
    QComboBox *cb_baud2;
    bool block_save_net;
	bool net_active;
    QLineEdit *TCPServer;
    QLineEdit *TCPPort;
#if defined _WIN32_
    QStringList StrListToUpper(QStringList lst);
    QStringList portNamesFromHardwareDeviceMap();
#endif
    QLabel *l_con_info;
    QGroupBox *gb_net;
    QPushButton *pb_re_conect;
    //QGroupBox *gb_tci;
    QLabel *l_tcich;
    QComboBox *tci_channels;
    QLabel *l_tcisamp;
    QComboBox *tci_samples;
    QLabel *l_tcistype;
    QComboBox *tci_tcistype;
    QLabel *l_tcisrate;
    QComboBox *tci_rample_rate;
    QSpinBox *tci_tx_buff;

    void ScanComPorts();
    QextSerialPort *port;
    QextSerialPort *port2;
    void Set_Rts(bool);
    void Set_Dtr(bool);
    void Set_Rts2(bool);
    void Set_Dtr2(bool);
    //QGroupBox *gb_method;
    QCheckBox *cb_read_data_kenwood_rts_on;
    QCheckBox *cb_read_data_kenwood_dtr_on;
    QRadioButton *rb_rts;
    QRadioButton *rb_dtr;
    QRadioButton *rb_rts2;
    QRadioButton *rb_dtr2;
    QRadioButton *rb_cat;
    QRadioButton *rb_ptt_off;
    QPushButton *pb_test_port;
    bool f_test_port;
    QPushButton *pb_test_port2;
    bool f_test_port2;
    HvRigCat *THvRigCat;
    QCheckBox *cb_off_set_frq_to_rig;
    QCheckBox *cb_mod_set_frq_to_rig;
    QComboBox *cb_rig_mod;
    void SetEnabledAll(bool);   
    void SetEnabledAll2(bool);
    ptt_type_t s_ptt_type_t;
    bool only_one;

    QRadioButton *rb_offtxwatch;
    QRadioButton *rb_mintxwatch;
    QRadioButton *rb_coutxwatch;
    QSpinBox *sb_mintxwatch;
    QSpinBox *sb_coutxwatch;
    int s_txwatchdog_time;
    int s_txwatchdog_coun;
    int f_no_time_count_txwatchdog;

    /*QLabel *lfrq;//za mahane
    QLabel *lmod;//za mahane
    QLineEdit *lefrq;//za mahane*/
    int s_have_read_data_rts_on;
    
    //void EnableGbStaticTX(bool);
    QGroupBox *gb_static_tx;
    QSpinBox *sb_static_tx;
    QLabel *l_static_tx;//2.76.1
    QCheckBox *rb_static_tx;//QRadioButton *rb_static_tx;//2.76.4
    bool f_static_tx;
    bool f_static_tx_active;
    double mp_v_disp_tx_frq;
    long long int v_disp_tx_frq;
    long long int last_v_disp_tx_frq;
    long long int end_rs0;//2.55
    long long int curr_rig_tx_frq;
    long long int last_curr_rig_tx_frq;
    long long int corr_app_ind_tx_frq;
    int s_mode;
    bool all_static_tx_modes;
    int ss_id;
    bool f_txrx_static_tx;
    bool timer_try_static_tx_isActive;
    QTimer *timer_try_static_tx;
    int corr_ms_static_tx;
    int max_try_off_static_tx; 
    bool f_block_display_static_tx;
    //bool g_block_rig_st_tx;//2.50 block for specific rig
    QString s_set_rig_mode_frq;
    int set_rig_mode_id;
    QTimer *timer_set_rig_mode;
    
    bool f_qrg_tx;
	bool f_qso7tx;
	QString prev_qrg;
	long long int rx_qrg; 
    
    QGroupBox *gb_troffs;
    int s_id_band;
    //QString s_band;
    QString FreqOffsetTR;
    void SetFreqOffsetTR();
    typedef struct//Transceiver  Transverter
    {
        int rb_id;
        QString freq;
    }
    offset_trrig;
    offset_trrig offset_trsv_rig[COUNT_BANDS];   
    QRadioButton *rb_offset_tr_off;
    QRadioButton *rb_offset_tr_sum;
    QRadioButton *rb_offset_tr_sub;
    QLineEdit *le_offset_trsv_rig; 
    QLineEdit *le_offset_trsv_rig_res; 
    
protected: 
    void closeEvent(QCloseEvent*)
    {
        if (f_test_port)
        {
            f_test_port = false;
            SetPtt(false,1);//id 0=All 1=p1 2=p2
            pb_test_port->setText(" START PTT TEST ");
            pb_test_port->setStyleSheet("QPushButton{background-color :palette(Button);}");
            SetEnabledAll(true);
        }     
        if (f_test_port2)
        {
            f_test_port2 = false;
            SetPtt(false,2);//id 0=All 1=p1 2=p2
            pb_test_port2->setText(" START PTT TEST ");
            pb_test_port2->setStyleSheet("QPushButton{background-color :palette(Button);}");
            SetEnabledAll2(true);
        }             
        //DestroyPort();
    }
};
#endif
