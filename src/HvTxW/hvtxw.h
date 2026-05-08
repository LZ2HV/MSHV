/* MSHV TxWidget
 * Copyright 2015 Hrisimir Hristov, LZ2HV
 * May be used under the terms of the GNU General Public License (GPL)
 */
#ifndef HVTXW_H
#define HVTXW_H

#include <QWidget>

#include <QLabel>
#include "../config_str_all.h"
class HvLabAutoSeq : public QLabel
{
    Q_OBJECT
public:
    HvLabAutoSeq(bool,QWidget * parent = 0);
    virtual ~HvLabAutoSeq();

    QString getautoseq_all()
    {
        QString s;
        for (int i =0; i<COUNT_MODE; i++)
        {
            s.append(ModeStr(i)+"=");
            s.append(QString("%1").arg(s_autoseq[i]));
            if (i<COUNT_MODE-1)
                s.append("#");
        }
        return s;
    };
    void SetAutoSeqAll(QString);
    void SetAutoSeqMode(int,bool);
    bool GetAutoSeq();
    //void SetEnabled(bool);

//public slots:

signals:
    void EmitLabAutoSeqPress();

//private slots:

private:
	bool dsty;
    bool s_autoseq[COUNT_MODE];
    int s_mode;

protected:
    void mousePressEvent( QMouseEvent * ev );
};

class HvLabRxOnlyFiSe : public QLabel
{
    Q_OBJECT
public:
    HvLabRxOnlyFiSe(QWidget * parent = 0);
    virtual ~HvLabRxOnlyFiSe();

//public slots:

signals:
    void EmitLabPress();

//private slots:

//private:

protected:
    void mousePressEvent( QMouseEvent * ev );
};

#include <QRadioButton>
#include <QPushButton>
#include "hvinle.h"
class HvTxIn : public QWidget
{ 
    Q_OBJECT
public:
    HvTxIn(int ident,bool,QWidget * parent = 0);
    virtual ~HvTxIn();

    //QLineEdit *line_txt;
    HvLeWithSpace *line_txt;
    QRadioButton *rb_tx;
    QPushButton *b_tx;
    int s_ident;
    void SetReadOnly(bool);
    void SetEnabledRbBtTx(bool f);
    void SetEnabledBtTx(bool f);
    void SetFont(QFont);

public slots:
    void rb_clicked();
    void b_released();

signals:
    void EmitRbPress(int);
    void EmitBReleased(int,QString);

private slots:
    //void TextChanged(QString);
    void rb_toggled(bool);
    //void SetBackground();

private:
	bool dsty;
	void SetBackground();
    //void keyPressEvent(QKeyEvent*);

protected:

};

#include <QDialog>
#include <QTabWidget>
class TWDialog : public QDialog
{
    Q_OBJECT
public:
    explicit TWDialog(QWidget *,QWidget *,QWidget *,QWidget * parent = 0);
    virtual ~TWDialog();
    void SetExec(int);   
signals:
	void EmitClose();
private:
	QTabWidget *TW;    
    void closeEvent(QCloseEvent*)
    { 
    	emit EmitClose(); 
    };
};

//////////////////////////////////////
#include "HvMakros/hvmakros.h"
#include "hvqthloc.h"
#include "hvmsdb.h"
#include "HvLogW/hvlogw.h"
#define  TEX_MARK_C 31
#include "../config.h"
#include "hvcustomw.h" 
#include "hvmultianswermodw.h" 
#include "HvRadioNetW/radionetw.h"
#include "../HvSlider_V_Identif/hvslider_v_identif.h"
#include "HvAstroDataW/hvastrodataw.h"
#include "hvspinbox.h"
#include "../HvButtons/hvbutton_lrc.h"

//#define _BANDS_H_
//#define _FREQTOBAND_H_
//#undef _LAMBDA_H_
//#undef _BCNBAND_H_
//#undef _BANDTOFREQ_H_
#include "../config_band_all.h"

class HvTxW : public QWidget
{
    Q_OBJECT
public:
    HvTxW(QString,QString path,int lid,bool,int,int,QWidget * parent = 0);
    virtual ~HvTxW();

	void SetMacrosFirstStart();
    void SetBand(QString,int id);//0<-from App 1<-from Rig
    void CloseAllWidget();
    void StartStopZap();
    void SetZap(QString);
    void SetMinsigndb(QString);
    void SetDftolerance(QString);
    void SetDataTime(QDateTime);
    void SetGUbMK(bool);
    void GetCurrentMsg();
    void ModeChanget(int,bool);
    void SetInLevel(QString);
    void SetOutLevel(QString);
    void SetPiriodTimeAllModes(QString);
    void SetLastTxRptAutoSeq(QString);
    void SetTxRxCountAutoSeq(bool);
    void Refr65DeepSearchDb();//1.49 deep search 65
    void SetVhfUhfFeatures(bool);

    bool GetAutoIsOn()
    {
        return f_auto_on;
    };
    bool GetTxFi()
    {
        return rb_tx_fi->isChecked();
    };
    QString get_Calls()
    {
        //return le_his_call->text();
        return le_his_call->getText()+"_"+list_macros.at(0);
    };
    QString getMy_Call()
    {
        //return le_his_call->text();
        return list_macros.at(0);
    };
    QString getsigndb()
    {
        QString s;
        for (int i =0; i<COUNT_MODE; i++)
        {
            s.append(ModeStr(i)+"=");
            s.append(QString("%1").arg(s_minsigndb[i]));
            if (i<COUNT_MODE-1)
                s.append("#");
        }
        return s;
    };
    QString getdftol()
    {
        return SB_DfTolerance1->def_df_all_modes();
    };
    QString getzap()
    {
        return QString("%1").arg(cb_zap->isChecked());
    };
    QString default_in_lev_cor()
    {
        return QString("%1").arg(Slider_Rx_level->get_value());
    };
    QString default_out_lev_cor()
    {
        QString txl;
        for (int i = 0; i<COUNT_BANDS; ++i) 
        {
        	txl.append(QString("%1").arg(s_tx_level[i]));//2.54
        	if (i<COUNT_BANDS-1) txl.append("#");
		}
		return txl; //return QString("%1").arg(Slider_Tx_level->get_value());
    };
    QString def_period_time_all_modes()
    {
        return SB_PeriodTime->def_pt_all_modes();
    };

    void SetOpAllB(QString s,int);
    QString get_op_allB(int);
 
    void SetSwlOp(QString s);
    QString get_swl_op()
    {
        //if(cb_sh_rpt->isChecked())
        return QString("%1").arg((int)cb_swl->isChecked());
    };
    HvAstroDataW *THvAstroDataW;
    QString GetAstroWPos();
    void SetAstroWPos(QString);
    QString GetLogWPosWH();
    void SetLogWPosWH(QString);
    
    void ShowAstroW();
    void CloseAstroW();
    //void SetRaDec(QString);
    //void GetRaDec(QString);

    // for ft8 to decoder
    QString getautoseq_all()
    {
        return AutoSeqLab->getautoseq_all();
    };
    void SetAutoSeqAll(QString);

    QString GetDirectLogQso()
    {
        return QString("%1").arg(direct_log_qso);
    };
    QString GetPromptLogQso()
    {
        return QString("%1").arg(prompt_log_qso);
    };
    void CloseBcnList();
    void SetBlockEmitFreqToRig(bool f);  
    QString GetFreqGlobal()
    {
        return FREQ_GLOBAL;
    };      
    void ExpandShrinkDf(bool f);//2.05
    //void SetMacros(QStringList,int,QString,QString); //2.32
    void SetDPLogQso(bool,bool);
    void SetMultiAnswerMod(bool,bool);
    void SetStartQsoDateTime(); //2.49
    void SetTxMessage(int); //2.49
    void ReadEDILog();//2.57
    void SendIdMshf();//2.76
    void RefreshOtpKeyMsg();//2.76    

public slots:
	//void SetFtDf1500(bool);
	void SetAResetQsoAtEnd(bool); //2.49
	void SetCfm73(bool);
    void StopAuto();
	void SetTxActive(int);
    void SetFont(QFont);
    void SetFreqTxW(double f);
    void SetLogQsoStartDtEqEndDt(bool f);
    void SetFreqGlobalFromRigCat(QString);
    void SetModeGlobalFromRigCat(QString);
    
    void SetTxWatchdogParms(int,int,int);
    //void ExternalFindLocFromDB(QString call);
    //void SetMacros(QStringList,bool,int,QString);//2.15 stoped
    
    //void SetMultiAnswerMod(bool,bool);
    void SetRecognizeTp1(bool f);
    void SetRecognizeTp2(bool f);
    void Set2ClickDecListAutoOn(bool f);
    
    void SetButClrQueuedCall(); 
    void SetDoQRG(QString,QString);
    void DecListTextAll(QString,QString,QString,QString,QString);
    void DecListTextRpt(QString);
    //void DecListTimePeriod(QString,int,QString);
    void GenTestTones();

    void AddToLogButton();

    void ShowLog();
    void SetNewDb();
    void SetDxParm(QString dc,QString re,QString dxg);
    void gen_msg();
    void bt_gen_msg();//for SWL
    void auto_on();
    void ResetQSO();
    void SetTextMark(bool*,QString);
    void SetRxOnlyFiSe();

    void ValidateStationInfo(QStringList list, int id, bool emitudpdectxt,uint8_t);//2.43 emitudpdectxt
    void SetDecodeInProgresPskRep(bool);
    void SetRxDf(int);

    void SetTextForAutoSeq(QStringList);
    void SetInfoDupeQso(bool);
    void Macros_exec();
    void NetW_exec();
    void RadioFreqW_exec();
    void StartEmptySpotDialog();
    void ShowBcnList();
    void SetStartFromTx2Tx1(bool);
    void SetModSetFrqToRig(bool);
    void SetUdpDecClr();
    void DlDetectTextInMsg(QString,QString &);
    void SetLogAutoComm(bool f);
    void SetQrgActive(int);//2.45
    void SetQrgFromRig(QString);//2.45
    void SetQrgInfoFromCat(QString);//2.45
    void SetQSOProgressAll(int,bool);//2.51
    void SetUseQueueCont(bool);//2.59
    void SetUseASeqMaxDist(bool);//2.66
    void SetMAFirstTX(bool);
    void SetMaManAdding(bool);
    void CBEnableAliChanged(bool);//2.75
    void SetAutoLogInfo();//2.75

signals:
	void EmitMacros(int,QString);
    void EmitGBlockListExp(bool);
    void EmitFreqGlobalToRig(QString,int);
    //void EmitModeGlobalToRig(QString);
    void EmitRigBandFromFreq(int);
    void Emit65DeepSearchDb(QStringList);//1.49 deep search 65
    //void EmitLocFromDB(QString);
    void EmitAstroWIsClosed();
    void EmitMessageS(QString msg,bool imidiatly,bool iftximid);
    void EmitAuto();
    void EmitFileNameChenged();
    void EmitDfSdbChanged(int,int);
    void EmitDfChanged(int,int);
    void EmitWords(QStringList,int,int);
    void EmitZap(bool);
    void StndInLevel(int);
    void StndOutLevel(int);
    void EmitReriodTime(float);
    void EmitDListMarkText(QStringList l,int,int,int,int,int);
    void EmitListHashCalls(QStringList l);
    void EmitShOptChenged(bool f);
    void EmitSwlOptChenged(bool f);
    void EmitRxOnlyFiSe(bool); 
    void EmitMyGridMsk144ContM(QString,bool);
    // for ft8
    void EmitTxToRx(bool);
    void EmitRxToTx(bool);//2.63
    void EmitLockTxrx(bool);
    void EmitFreqTxW(double);
    void EmitQSOProgress(int);
    void EmitFreqGlobalToDec(QString);//2.76.5
    void EmitMAMCalls(QStringList);
    void EmitUdpCmdDl(QStringList);
    void EmitUdpCmdStop(bool);
    void EmitQrgParms(QString s,bool f);//2.45 
    void EmitQrgQSY(QStringList);//2.46 
    void EmitDistUnit(bool);
    void EmitHisCalls(QStringList);
    void EmitMshfChanget(uint8_t);//2.76
    void EmitSFMATxAll(QString);//2.76
    void EmitOtpTxKey(QString);//2.76
    void EmitOtpRxMsg(bool);//2.76
    void EmitOtpVerif(QString,uint8_t);//2.76
    void EmitOffsetDt(int);//2.76.5
 
private slots:
    void SetRptRsq(bool);
	void SetMacros(QStringList,int,QString,QString); //2.32	
    void SetDistUnit(bool);
    void SetEmitMessage(QString msg,bool imidiatly,bool iftximid,bool id_mam);
	void SetRigCatActiveAndRead(bool,QString);//2.76.1
    void ExternalFindLocFromDB(QString call);
    void LockTxrxChanged(bool);
    void RbPress(int);
    void BReleased(int,QString);
    void SetTxSnV2(int);
    void DfSdbChanged(int);
    void Check(QString);
    void CheckBD();
    void AddDb();
    void StndInLevel_s(int,int);
    void StndOutLevel_s(int,int);
    void Sh_Rpt_Changet(bool);
    void Swl_Changet(bool);
    void FormatRxRst();
    void RbTxFiSeChange(bool);
    void SetAstroData(double my_za,double my_el,int his_dop,double dgrd);
    void AutoSeqLabPress();
    void SetDefFreqGlobal(int,QString);
    void AddToLogMultiAnswerQSO(QStringList);
    void IsCallDupeInLog(QString,int,bool &);
    void SetDoubleClickFromAllAutoOn();//2.45
    void SetMarkTextLogAll(QStringList,int);
    void RefreshLRestrict();
    void SetQrgActiveId(int);//2.60
    void SetHisCalls(QStringList);
    void MshfChanget(bool);//2.76

private:
	uint8_t id_mshf;//2.76
	bool f_ma_first_tx;
	bool dsty;
	bool allq65;
	bool alljt65;
	int slid;
	bool fpsk_restrict;
	bool f_areset_qso;//2.49
	void ResetQSO_p(bool);
	bool f_off_auto_comm;
	bool f_tx_rx;
	QString s_last_call_for_BDB;
    void RefreshBackupDB();
	bool f_cfm73;
	bool sf_cfm73;
	void RefreshCfm73();
	bool g_ub_m_k;	
	bool prev_frest_;
    bool f_mod_set_frq_to_rig;
	QFrame *Box_in_tx;
	int s_list_log_mark_txt_p1;
	QStringList s_list_log_mark_txt;
	QStringList s_list_mark_txt;
	QStringList s_list_mark_myc;
	QString s_calls_mark;
	int s_mark_r12_pos;
	//int s_mark_myc_pos;
	int s_mark_hisc_pos;
	void SetMarkTextAll();
	QString s_last_bccall_tolog_excp;
	HvButtonLeftRightClick *b_add_to_log;//QPushButton *b_add_to_log;	
    void AddToLog_p(bool direct_save_to_log); 
    bool s_start_qso_from_tx2;
    //bool DialogIsCallDupeInLog(QString hisCall_inmsg);
    bool f_multi_answer_mod;
    bool f_multi_answer_mod_std;
    MultiAnswerModW *MultiAnswerMod;
    void SetTxTextsHiden(bool f);
    void RefreshMultiAnswerModAndASeq();
    
    bool s_2click_list_autu_on;
    bool log_qso_startdt_eq_enddt;
    bool f_block_emit_freq_to_rig;
    bool rig_cat_active_and_read;
    
    HvMakros *THvMakros;
    TWDialog *TWD;
    QString FREQ_GLOBAL;
    //void SetFreqGlobal();
    bool FindRigBandFromFreq(QString);
    HvCatDispW *THvCatDispW;
    bool s_bvhf_jt65;
    bool f_recognize_tp1;
    bool f_recognize_tp2;
    RadioAndNetW *TRadioAndNetW;
    bool TrySetQueuedCall(bool);
    void CountTx73_p(bool);
    void SetLockTxrxMode(int);
    void TryFindCallLocForSpot(QString in,QString &call,QString &loc);
    //bool isAstroWActive;
    bool sh_op_all[COUNT_MODE];
    bool DetectCallSufix(QString &call, bool f_sep_n);
    QString DetectSufix(QString call_all, bool f_sep_n);
    QStringList l_mam_hiscals_;
    void MarkTextChanged(bool);//boll norefresh b4 qso
    bool s_txt_mark[TEX_MARK_C]; 
    bool s_f_rpt_rsq;
    bool f_km_mi;
    int count_tx_widget;
    QVBoxLayout *V_l;
    QLabel *l_time;
    QLabel *l_dey;
    HvSpinBoxDt *sb_dt;
    QLabel *l_mycall_loc;

    QString AppPath;
    void SaveSettings();
    bool isFindId(QString id,QString line,QString &res);
    void ReadSettings();
    QString sr_path;
    //QLineEdit *le_his_call;
    
    bool SetTimePeriod_p(QString time_1,int msg_pos);//1.56=,QString str
    //void DetectTextInMsg(QString str, QString &hisCall_inmsg,QString &hisLoc_inmsg,QString &myCall_inmsg,
                         //QString &rpt_inmsg,QString &contest_mode_inmsg,QString &rr73_inmsg);//1.56=
    int FindSelPosForTP(QString str,QString all_txt);//1.56=                     
    HvInLe *le_his_call;
    HvRptLe *le_rst_tx;
    HvRptLe *le_rst_rx;

    //HvInLe *le_txsn;
    //QLabel *l_txsn;
    HvSpinBoxSn *sb_txsn;
    HvSpinBoxSn *sb_txsn_v2;
    QPushButton *b_auto_on;
    QPushButton *b_gen_msg;
    QRadioButton *rb_tx_fi;
    QRadioButton *rb_tx_se;

    bool f_rx_only_fi_se;
    HvLabRxOnlyFiSe *l_rx_only_fi_se;

    bool f_auto_on;
    QStringList list_macros;
    QString s_my_base_call;
    QString s_my_contest_exch; 
    QString s_his_contest_exch;
    QString s_his_contest_sn;
    QString s_trmN;
    int s_cont_type;
    int s_cont_id;
    bool f_cont5_ru_dx;
    //bool s_my_call_is_std;
    void RefreshCntestOnlySdtc();
    bool f_cntest_only_sdtc;
    //void RefreshTwoNoSdtc();
    bool f_two_no_sdtc;

    //bool is_pfx(QString s);
    //QString FindBaseCallRemAllSlash(QString);

    //bool isStandardCall(QString w);
    QString DecodeMacros(QString,bool,int,bool,bool,uint8_t);
    HvSpinBox *SB_MinSigdB;
    //bool f_last_from_queued;
    HvQueuedCallW *TQueuedCall; 
    //QCheckBox *cb_rpt_db_msk;
    
    HvSpinBoxDf *SB_DfTolerance1;

    HvSpinBox4per30s *SB_PeriodTime;

    int s_minsigndb[COUNT_MODE];
    //int s_dftolerance[COUNT_MODE];
    int s_mode; 
    bool f_nosave;

    void CalcDistance();
    HvQthLoc THvQthLoc;
    bool block_loc;
    bool block_call;
    HvInLe *LeHisLoc;
    QLabel *l_dist;
    QLabel *l_beam;
    QLabel *l_el;
    QLabel *l_haz;
    QLabel *l_loc_from_db;
    void RfreshDxParm();

    QString s_dist;
    QString s_beam;
    //QString s_beam;
    MsDb *TMsDb;
    QPushButton *pb_checkdb;
    QPushButton *pb_adddb;
    //bool f_atstart_check_db;
    bool f_makros_ready;
    int s_b_identif;
    HvQrg *le_qrg;
    QCheckBox *cb_zap;
    QCheckBox *cb_sh_rpt;
    QCheckBox *cb_swl;
    QCheckBox *cb_msh;//2.76
    QCheckBox *cb_msf;//2.76
    HvTxRxEqual *pb_tx_to_rx;
    HvTxRxEqual *pb_rx_to_tx;//2.63
    HvLabAutoSeq *AutoSeqLab;
    QCheckBox *cb_lock_txrx;

    HvLogW *THvLogW;
    QString s_log_data_now;
    QString s_log_time_now;
    QString s_log_data_start;
    QString s_log_time_start;
    QString s_last_call_for_log;

    QString s_band;
    void SetRptRsqSettings();
    int s_iband;
    int s_tx_level[COUNT_BANDS];//2.54
    HvSlider_V_Identif *Slider_Tx_level;
    HvSlider_V_Identif *Slider_Rx_level;
    //QString format_rst(QString s);
    //QLabel *l_rst_rx;
    //QLabel *l_rst_tx;
    QLabel *l_monit;
    HvInLe *le_mon_call1;
    HvInLe *le_mon_call2;
    bool block_mon_call1;
    bool block_mon_call2;

    QLabel *l_moont;
    QLabel *l_azm;
    QLabel *l_elm;
    QLabel *l_dopm;
    QLabel *l_dgrdm;

    void SetShowMoni12MoonD(bool f);
    int g_no_block_tx;
    void SetEnabledRbBtTx(bool f);
    void SetEnabledBtTx(bool f);
    void SetTxTextsReadOnly(bool);

    QLabel *l_my_az;

    int max_snr_psk_rep;
    QString hisCall_pskrep;
    QString hisLoc_pskrep;
    //int s_freq_offset;
    int prev_max_snr_psk_rep;
    QString prev_hisCall_pskrep;
    QString prev_hisLoc_pskrep;
    QString prev_mode_pskrep;

    int count_73_auto_seq;
    bool one_addtolog_auto_seq;
    bool f_add_to_log_started;
    bool is_LastTxRptCq;
    void SetTxRaportToRB(int,bool);// bool immediately
    bool s_locktxrx[COUNT_MODE];
    bool prompt_log_qso;
    bool direct_log_qso;
    bool info_dupe_qso;
    bool is_locked_tx_rst_msk_aseq;
    void SetLockTxRstMskASeq(bool);
    void SetTxRstASeq(QString);
    bool isAddToLog(QString);//2.76.1 
       
	long long int _ftfr_[40];//+FT2
    //TX watchdog
    QString s_PrevTxRpt;
    unsigned int s_last_txwatchdog_time;
    int s_last_txwatchdog_coun;
    unsigned int s_txwatchdog_time;
    int s_txwatchdog_coun;
    int f_no_time_count_txwatchdog;//by 0=no 1=time 2=count;
    void ResetTxWatchdog(QString,bool,bool);
    //HvAstroDataW *THvAstroDataW;
    QLabel *l_trxmx;
    QLabel *l_trxmi;
    QLabel *l_trxdp;
    QLabel *l_trxdm;
    int s_dist_points;
    bool f_aseqmaxdist;

protected:



};
#endif