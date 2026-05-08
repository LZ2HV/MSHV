/* MSHV Main
 * Copyright 2015 Hrisimir Hristov, LZ2HV
 * May be used under the terms of the GNU General Public License (GPL)
 */ 
#ifndef MAIN_MS_H
#define MAIN_MS_H

#include "config.h"

#include <QWidget>
#include "DisplayMs/display_ms.h"
#include "DisplayMs/HvCustomPalW/custompalw.h"
#include "HvStylePlastique/hvstyleplastique.h"
#include "HvStylePlastique/hvfontdialog.h"
#include "HvMsCore/mscore.h"
#include "SettingsMs/settings_ms.h"
#include "HvSMeter/hvsmeter_h.h"
#include "HvMsPlayer/msplayerhv.h"
#include "HvTxW/hvtxw.h"
#include "HvDecodeList/decodelist.h"
#include "HvDecoderMs/decoderms.h"
#include "HvRigControl/hvrigcontrol.h"
#include "LabWidget/labw.h"
#include "HvSlider_V_Identif/hvslider_v_identif.h"
#include "HvAllTxt/hvalltxt.h"
#include "HvHelpMs/hvhelpms.h"
#include "HvHelpSkMs/hvhelpskms.h"
#include "HvAboutMsHv/hvaboutmshv.h"
#include "HvTextColor/hvtxtcolor.h"
#include "HvAggressiveW/aggressiv_d.h"
#include "HvTxW/hvspinbox.h"

#if defined _LINUX_
#include "HvAlsaMixer/hvmixermain.h"
#endif
#include "HvButtons/hvbutton_left4.h"
#include "HvButtons/hvmodbtsw.h"


#if defined _WIN32_
typedef void (WINAPI *PGNSI)(LPSYSTEM_INFO);
#endif

#include "CpuWidget/cpuwudget.h"
#include "HvMsProc/hvmsproc.h"
#include "HvDecodeList/hvfilterdialog.h"
void _ReadSSAndSet_();

#include "config_str_sk.h" 
class Main_Ms : public QWidget
{
    Q_OBJECT
public:
    Main_Ms(QString,QWidget * parent = 0);
    ~Main_Ms();

    bool is_active_astro_w;
    QAction *Start_astro_m;
//signals:
        
private slots:
	void SetTxFreq(double);//2.16
	void SetStaticTxFrq(bool,int);//2.16
    void SetMacros(int,QString);//2.32
    void SetBandFromRigFreq(int i);
    void SetSHAllColl(bool);
    void SetTwoDecList(bool f);
    void ShowCloseAstroW(bool);
    void SetAstroWIsClosed();
    void Open();
    void InDevChanged(QString,int,int,int,int,int,int);//QString rate
    void SaveSettings();
    void PaletteChanged(bool);
    void Refresh();
    void TxMessageS(QString,bool,bool);
    void StopTxGlobal();
    void StartRxGlobal();
    void ModeChanged(bool);
    void SetAuto();
    void StopRxGlobal();
    void SetFileToDisplay();
    void SaveFileDisplay1();
    void SaveFileDisplay2();
    void FileNameChengedD1();
    void FileNameChengedD2();
    void Tune();
    void SetTxMsgLabTx(QString);
    void SetTxMsgAllTxt(QString,double);
    void SetAvg65CountToButtonTxt(int,int,int,int);
    void SetAvgPi4CountToButtonTxt(int,int);
    void SetAvgQ65CountToButtonTxt(int,int);
    void SetButtonDecodeAll65();
    void SetUdpCmdStop(bool);
    void SetDLogQso(bool);
    void SetPLogQso(bool);
    void SetQrgQSY(QStringList);//2.46 
#if defined _WIN32_
    void WMixRec();
    void WMixPlay();
    void SyncTime();
    void End_Cmd(int);
#endif
    void SetRxAllTxt(QString);
    void OnlineTimeCheck();
    void BandChanged(bool);
    //void End_Cmd_SyncTime(int);
    //void SyncTimeClose(int);
    //void BSyncTimeClose();
    void SetDecodeBusy(bool,int dec_state);//dec_state no=0 dec=1 rtddec=2
    void SetPeriodTime(float);
    void CbSetStartStopRtd(bool);
    //void SetShOpt(bool);
    void CbSetAutoDecodeAll(bool);
    void DecodeAp(bool);
    void DecodeDeept(bool);
    void VarDecodeFtPar(bool);//2.76.5
    void SetRxOnlyFiSe(bool);
    void BtSet2D1D();
    void BtSetD1D2();
    void Msk144RxEqual(bool);
    void SetVhfUhfFeatures(bool);
    void DeepSearchChanged(bool);
    void AvgDecodeChanged(bool);
    void VDispStartBandChanged(int);
    void SetMaxCandidats65(bool);
    void SetVDispSpeed(int);
    void IsDispHaveDataForDec65(int,bool);
    void setTuneDsp(int,int);
    //void SetRptDbMsk(bool);
    void SetFont(QFont);
    void SetMultiAnswerMod(bool);
    void SetMultiAnswerModStd(bool);
    void SetThrLevel(bool);
    void SetFlatDisplay_VD(bool);
    void SetAdleDisplay_VD(bool);
    void SetShowHideWf(bool);
    void SetShowHideTx(bool);
    void LangChanged(bool);
    void StyleChanged(bool);
    void ModBtSwClicked(int);
    void BandBtSwClicked(int);
    void SetMshf(uint8_t);//2.76
    void SetOffsetDt(int);//2.76.5
    
private:
	int s_offset_dt;//2.76.5
    QString InstName;
	QString _stcq_;
	void SkUpDownBandChanged(bool);
    void SetTxFreq_p();//2.64
    double s_v_disp_tx_frq;
    int s_static_tx_frq;
    bool f_static_tx;
    
    int s_id_set_to_rig;//0<-from App 1<-from Rig
    void SetModeDecodeListS(bool,bool,bool,bool,bool);
    bool f_is_moved_to_prev_desk_pos;
    bool f_is_d1_data_todec65;
    bool f_is_d2_data_todec65;
    bool fast_find_period;
    bool f_disp_v_h;
    bool last_f_disp_v_h;
    void SetDispVH(bool);
    HvFontDialog *FontDialog;
    CpuWudget *TCpuWudget;
    void SetFilePlay();
    //QDialog *SyncTimeDialog;
    //int band_count;
    QMenu *Band_m;
    QList<QAction*> ListBands;
    QLabel *l_sync_t;
    //QPushButton *pb_sesync_d;
    //void StopTx();
    void StartRx();
    void SetRigTxRx(bool);

    HvTxW *THvTxW;
    QString App_Path;
    QString s_contest_name;
    QString s_trmN;
    void RefreshWindowTitle();
    DisplayMs *MainDisplay;
    DisplayMs *SecondDisplay;
    QWidget *Box_dspl;
    MsCore *TMsCore;
    SettingsMs *TSettingsMs;
    HvSMeter_H *THvSMeter_H;
    MsPlayerHV *TMsPlayerHV;
    void Save_Settings(QString path);
    bool isFindId(QString id,QString line,QString &res);
	void SetQActionCb(QString s, bool idp, QAction *ac);
    void Read_Settings(QString path);
	void TxMessage(QString,bool);

    QAction *rb_palette[9];//2.65
    CustomPalW *TCustomPalW;
    bool dsty;
    QAction *ac_dark_st;

    //int identif_only_one;
    int time_pos_1period;
    int time_pos_2period;
    //int period_time_msec;
    double period_time_sec;
    QString s_msg;
    //bool s_gen;
    //bool s_imidi;
    QAction *rb_mode[COUNT_MODE];
    QMenu *Mode_m;
    bool allq65;
    int s_mode;
    DecodeList *TDecodeList1;
    DecodeList *TDecodeList2;
    HvFilterDialog *FilterDialog;
    DecoderMs *TDecoderMs;
    HvRigControl *THvRigControl;
    bool global_start_moni;
    LabW *TPicW;
    bool f_auto_on;
    QLabel *l_mode;
    HvSlider_V_Identif *Slider_Tune_Disp;
    HvSlider_V_Identif *Slider_Cont_Disp;
    QString getHHMin();

    //QHBoxLayout *H_bsave;
    void SetBS1Text(QString);
    void SetBS2Text(QString);

    QPushButton *pb_save_disp1;
    QPushButton *pb_save_disp2;
  
    QCheckBox *cb_flat_dsp;
    QCheckBox *cb_adle_dsp;
    
    void SetAllSettingsI(QString s,int ident);
    QString get_settings_allI(int ident);
    
    int s_vdisp_all_speed[COUNT_MODE];
    HvSpinBox *SB_VDispSpeed;
    //QString get_vdisp_all_speed();
    //void SetVdispAllSpeed(QString s);

    HvSpinBox *SB_VDispStartFreq;
    HvSpinBox *SB_VDispBandwidth;
    bool f_onse50;
    bool s_f_dec50;
    int fi_se_changed;

    QPushButton *pb_clar_list1;
    QPushButton *pb_clar_list2;
    QPushButton *pb_start_rx;
    QPushButton *pb_tune;
    QPushButton *pb_clear_avg65;
    QPushButton *pb_dec_65;
    QPushButton *pb_clear_avgPi4;
    QPushButton *pb_clear_avgQ65;
    bool f_tune;
    QLabel *l_tx_text;

    bool auto_decode_all[COUNT_MODE];
    QCheckBox *cb_auto_decode_all;

    QCheckBox *cb_rtd_decode;
    //bool s_rtd_decode;
    //bool s_sh_opt;

    int decoder_depth_all[COUNT_MODE];
    //void SetDecodeDepthAll(QString s);
    //QString get_decoder_depth_all();
    void SetDecodeDeeptFromMod(int md);
    
    AggressiveDialog *TAggressiveDialog;
    
    //QMenu *ThrM;
    int cthr;
    QAction *rb_thr[6];
    int thr_all[COUNT_MODE];
    void RbThrSetEnabled(bool);
    
    QAction *rb_dec_depth[3];    
    QAction *cb_3intFt_d;//2.39 remm 
    QAction *cb_UseVarDecFt;
    QMenu *MVDecFtPar;
    QAction *rb_vdec_cyc[3]; 
    QAction *rb_vdec_sens[3];   
       
    QAction *cb_msk144rxequal_[4];
    QAction *ac_aggressive;
     
    void SetDecodeAllSettingsB(QString s,int ident);
    QString get_dec_settings_allB(int ident);          
    bool vhf_uhf_decode_fac_all[COUNT_MODE];
    QAction *cb_vhf_uhf_decode_fac;
    bool avg_dec_all[COUNT_MODE];   
    QAction *cb_avg_decode;    
    bool deep_search_dec_all[COUNT_MODE];
    QAction *cb_deep_search_decode;    
    bool decoder_ap_all[COUNT_MODE];
    QAction *cb_ap_decode;
    QAction *cb_auto_clr_avg_afdec; 
    QAction *cb_1_dec_sig_q65;
    QAction *cb_dec_aft_eme_delay;
    QAction *cb_max_drift;

	int max65_cand_all[COUNT_MODE];
	//void SetMax65CandAll(QString s);
    //QString get_max65_cand_all();
    void SetMaxCandidats65Mod(int);
    QAction *cb_max65_cand[5];

#if defined _WIN32_
    float GetOSVersion();
    QProcess *Start_Cmd;
#endif
#if defined _LINUX_
    HvMixerMain *THvMixerMain;
#endif

    AllTxt *TAllTxt;
    void StopRx();
    HvHelpSkMs *THvHelpSkMs;
    HvHelpMs *THvHelpMs;
    HvMsProc *THvMsProc;
    int titleBarHeight();
    void Screenshot();
    void FileOpen(QString);
    HvTxtColor *THvTxtColor;
    
    bool two_dec_list_all[COUNT_MODE];
    QAction *ac_two_dec_list; 
    QAction *ac_2click_list_autu_on;  
    QAction *ac_start_qso_from_tx2_or_tx1; 
    QAction *ac_use_queue_cont; 
    QAction *ac_new_dec_clr_msg_list; 
    QAction *ac_click_on_call_show_cty;
    QAction *ac_show_timec;
    QAction *ac_show_counc;
    QAction *ac_show_distc;
    QAction *ac_show_freqc;
    bool s_show_lc[COUNT_MODE][4];
    QAction *ac_filter_list;
      
    QAction *Direct_log_qso;    
    QAction *Prompt_log_qso;
    QAction *Info_dupe_qso;
    QAction *Log_qso_startdt_eq_enddt;
    QAction *Log_auto_comm;
    bool g_block_mam;
    bool g_ub_m_k;
    bool g_ub_m_k2;
    bool g_ub_m_k3;
    QAction *MA_man_adding;
    QAction *Multi_answer_mod;
    QAction *Multi_answer_mod_std;
    QAction *Mon_start_m;
    QAction *sh_wf;
    QAction *sh_tx;

    QAction *ac_Cfm73;
	void RefreshCbCfm73();
	uint8_t id_mshf;//2.76
	bool f_is_myCstd;//2.76.1
    void RefreshStartQsoTX2orTx1();//2.76.1 
	//QAction *ac_ft_df1500;    
	QAction *ac_areset_qso; 
	QAction *ac_aseq_max_dist; 
    QAction *recognize_tp1;
    QAction *recognize_tp2;
    
    QAction *zero_df_scale_m;
    QAction *vd_mouse_lines_draw;
    QAction *vd_bw_lines_draw[3];
    bool block_save;
    int f_once_pt_15_to_30;
    bool f_rx_only_fi_se;
    bool f_fast_rfresh_only_fi_se;

    //// 2/1 Display //////////
    QList<QLabel*>txt_tunedsp;
    HvButton_Left4 *pb_2D_1D;
    HvButton_Left4 *pb_D1_D2;
    //// END 2/1 Display //////////

    bool g_block_from_close_app_is_active_astro_w;

	//// Translation ////
    QAction *ac_l[COUNT_LANGS]; 
    void SaveSS();
    //// end Translation ////
    HvWBtSw *W_mod_bt_sw;
    HvWBtSw *W_band_bt_sw;
        
protected:
    QDateTime getDateTime();
    bool f_once0s;
    int stop_tx_befor_end;//stop tx 370ms bifor rx period tx buffer is 4096=370ms
    bool f_tx_to_rx;
    int count_tx_to_rx;
    bool f_rx_glob_;
    bool f_is_period_rx_tx;
    int screenWidth;
    int screenHeight;
    QDesktopWidget *desktopw;
    //QSize DesktopAllSize;

    void keyPressEvent(QKeyEvent*);
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);
    bool SupportedDrags(QString data);
    bool f_decoder_busy;
    bool f_tx_busy;
    bool f_de_active;
    void ModeMenuStatRefresh(bool);
    void dragMoveEvent(QDragMoveEvent *event);
    //void mouseMoveEvent(QMouseEvent *event);
    //void paintEvent(QPaintEvent *);

    void closeEvent(QCloseEvent*)
    { 
        if (THvHelpSkMs->isVisible()) THvHelpSkMs->close();
        if (THvHelpMs->isVisible()) THvHelpMs->close();
        THvTxW->CloseAllWidget();
#if defined _LINUX_
        if (THvMixerMain->isVisible()) THvMixerMain->close();
#endif
        if (THvTxW->THvAstroDataW->isVisible())
        {
            g_block_from_close_app_is_active_astro_w = true;
            Start_astro_m->setChecked(false);
        }
        if (TCustomPalW->isVisible()) TCustomPalW->close();
        if (THvMsProc->isVisible()) THvMsProc->close();              
        THvTxW->CloseBcnList();           
    };
};
#endif




