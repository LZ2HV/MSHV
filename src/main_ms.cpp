/* MSHV Main Ms UI
 * Copyright 2015-2024 Hrisimir Hristov, LZ2HV
 * May be used under the terms of the GNU General Public License (GPL)
 */

#undef _MOUNTH_H_
#define _SHKY_H_
#define _BANDS_H_
#include "main_ms.h"

#include <QScreen>
#include <QWindow>
#include <QDesktopWidget>
#include <QDesktopServices>
#include <QMimeData>  // For QT5

#define _CONT_NAME_
#include "config_str_con.h"

//#include <QtGui>

//// Translation ////
#include <QTranslator>
QTranslator _translator_;
static int lid = 0;
static int styid = 0;
void _ReadSSAndSet_()
{
    QString app_p = (QCoreApplication::applicationDirPath());
    QFile file(app_p+"/settings/ms_start");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return;
    QString langid = "0";
    QString styleid = "0";
    QTextStream in(&file);
    while (!in.atEnd())
    {
        QString line = in.readLine();
        QRegExp rx;
        rx.setPattern("def_lang=\"?([^\"]*)\"?");
        if (rx.indexIn(line) != -1) langid = rx.cap(1);
        rx.setPattern("def_style=\"?([^\"]*)\"?");
        if (rx.indexIn(line) != -1) styleid = rx.cap(1);
    }
    file.close();
    lid = langid.toInt();
    styid = styleid.toInt();

    if (lid>0 && lid<COUNT_LANGS)
    {
        /*QString istr = "";
        if 		(lid == 1 ) istr = ":HvTranslations/mshv_bg.qm";
        else if (lid == 2 ) istr = ":HvTranslations/mshv_ru.qm";
        else if (lid == 3 ) istr = ":HvTranslations/mshv_zh.qm";
        else if (lid == 4 ) istr = ":HvTranslations/mshv_zhhk.qm";
        else if (lid == 5 ) istr = ":HvTranslations/mshv_eses.qm";
        else if (lid == 6 ) istr = ":HvTranslations/mshv_caes.qm";
        else if (lid == 7 ) istr = ":HvTranslations/mshv_ptpt.qm";
        else if (lid == 8 ) istr = ":HvTranslations/mshv_roro.qm";
        else if (lid == 9 ) istr = ":HvTranslations/mshv_dadk.qm";
        else if (lid == 10) istr = ":HvTranslations/mshv_plpl.qm";
        else if (lid == 11) istr = ":HvTranslations/mshv_frfr.qm";
        else if (lid == 12) istr = ":HvTranslations/mshv_ptbr.qm";
        else if (lid == 13) istr = ":HvTranslations/mshv_nbno.qm";
        else if (lid == 14) istr = ":HvTranslations/mshv_itit.qm";
        if (_translator_.load(istr)) 
        {        
            QApplication::installTranslator(&_translator_);
        }
        else lid = 0;*/

        QString istr = ""; //2.67 -0.5 KB
        const QString str_qm[COUNT_LANGS-1] =
            {
                "bg.qm","ru.qm","zh.qm","zhhk.qm","eses.qm","caes.qm",
                "ptpt.qm","roro.qm","dadk.qm","plpl.qm","frfr.qm",
                "ptbr.qm","nbno.qm","itit.qm","cscz.qm"//,elgr.qm
            };
        for (int i = 0; i < COUNT_LANGS-1; ++i)
        {
            if (lid == i+1)
            {
                istr = ":HvTranslations/mshv_"+str_qm[i];
                break;
            }
        }
        if (_translator_.load(istr))
        {
            QApplication::installTranslator(&_translator_);
        }
        else lid = 0;

        /*QString istr = "";
        QString App_Path = (QCoreApplication::applicationDirPath());
        if 		(lid == 1 ) istr = "mshv_bg.qm";   
        else if (lid == 2 ) istr = "mshv_ru.qm";   
        else if (lid == 3 ) istr = "mshv_zh.qm";   
        else if (lid == 4 ) istr = "mshv_zhhk.qm"; 
        else if (lid == 5 ) istr = "mshv_eses.qm"; 
        else if (lid == 6 ) istr = "mshv_caes.qm";
        else if (lid == 7 ) istr = "mshv_ptpt.qm";
        else if (lid == 8 ) istr = "mshv_roro.qm";
        else if (lid == 9 ) istr = "mshv_dadk.qm";
        else if (lid == 10) istr = "mshv_plpl.qm";
        else if (lid == 11) istr = "mshv_frfr.qm";
        else if (lid == 12) istr = "mshv_ptbr.qm";
        else if (lid == 13) istr = "mshv_nbno.qm";
        else if (lid == 14) istr = "mshv_itit.qm";
        if (_translator_.load(istr,App_Path+"/settings/resources/translations"))
        {        
            QApplication::installTranslator(&_translator_);
        }
        else lid = 0;*/
    }
}
//// end Translation ////
Main_Ms::Main_Ms(QString inst0,QWidget * parent)
        : QWidget(parent)
{
    //printf("--");
    InstName = inst0;
    s_id_set_to_rig = 0;//0<-from App 1<-from Rig
    f_is_d1_data_todec65 = false;
    f_is_d2_data_todec65 = false;
    is_active_astro_w = false;
    g_block_from_close_app_is_active_astro_w = false;
    f_disp_v_h = false;
    last_f_disp_v_h = f_disp_v_h;
    f_onse50 = false;
    s_f_dec50 = false;
    fi_se_changed = -1;
    s_contest_name = "None";
    s_trmN = "Run 1";
    _stcq_ = "";
    s_offset_dt=0;

    for (int i = 0; i < COUNT_MODE; ++i) auto_decode_all[i] = true;
    auto_decode_all[0]  = false;  //MSK144 and MSK40
    auto_decode_all[12] = false; //MSK144ms

    //s_sh_opt = false;
    block_save = true;
    global_start_moni = false;
    f_auto_on = false;
    f_once0s = true;
    f_once_pt_15_to_30 = 0;// 0<-no 1<-30s 2<-15s
    f_rx_only_fi_se = false;
    f_fast_rfresh_only_fi_se = false;

    //+ 100ms za da spre signala kam PA sigurno
    stop_tx_befor_end = (370);//(370 + 100); 4096*1000/11025 stop tx 370ms bifor rx period tx buffer is 4096=370ms
    f_tx_to_rx = false;
    count_tx_to_rx = 0;
    f_rx_glob_ = false;

    setWindowTitle(APP_NAME + InstName);  //qDebug()<<QCoreApplication::applicationName();
    App_Path = (QCoreApplication::applicationDirPath());

    dsty = false;
    if (styid==1) dsty = true;

    HvStylePlastique *THvStylePlastique = new HvStylePlastique(dsty);
    QApplication::setStyle(THvStylePlastique);
    //QApplication::setStyle("windowsvista");
    QPixmap icon = QPixmap(":pic/ms_ico.png");
    setWindowIcon(icon);

    //setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowTitleHint);
    //setStyleSheet("QWidget:titl{background-color: QLinearGradient( x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #ffa02f, stop: 1 #ca0619);}");
    //setWindowFlags(Qt::FramelessWindowHint | windowFlags());

    //// Translation ////
    QMenu *lang_m = new QMenu("Language");
    QActionGroup *ac_gr_lang = new QActionGroup(this);
    const QString str_ln[COUNT_LANGS]=
        {
    		"English","Български","Русский","中文简体","中文繁體","Español","Català","Português","Română",
    		"Dansk","Polski","Français","Português BR","Norsk","Italiano","Czech"//,Greek
        };
    const QString str_lmt[COUNT_LANGS]=
        {
    		"Language","Език","Язык","语言","語言","Idioma","Idioma","Idioma","Limba",
    		"Sprog","Język","Langue","Idioma","Språk","Lingua","Jazyk"//,Ellinka
        };
    for (int i = 0; i < COUNT_LANGS; ++i)
    {
        QAction *ac_tt = new QAction(str_ln[i],this);
        ac_tt->setCheckable(true);
        ac_gr_lang->addAction(ac_tt);
        ac_l[i]=ac_tt;
    }
    //ac_l[14]->setEnabled(false); //itit

    /*lang_m->addAction(ac_l[ 0]); //en
    lang_m->addAction(ac_l[ 1]); //bg
    lang_m->addAction(ac_l[ 2]); //ru
    lang_m->addAction(ac_l[ 3]); //zh   Simplified Chinese
    lang_m->addAction(ac_l[ 4]); //zhhk Traditional Chinese
    lang_m->addAction(ac_l[ 5]); //eses
    lang_m->addAction(ac_l[ 6]); //caes
    lang_m->addAction(ac_l[ 7]); //ptpt
    lang_m->addAction(ac_l[12]); //ptbr
    lang_m->addAction(ac_l[ 8]); //roro
    lang_m->addAction(ac_l[ 9]); //dadk
    lang_m->addAction(ac_l[10]); //plpl
    lang_m->addAction(ac_l[11]); //frfr
    lang_m->addAction(ac_l[13]); //nbno
    lang_m->addAction(ac_l[14]); //itit*/
    for (int i = 0; i < COUNT_LANGS; ++i)
    {
        int pos = i;
        if 		(i>12) pos = i;
        else if (i> 8) pos = i-1;
        else if (i==8) pos = 12;
        lang_m->addAction(ac_l[pos]); //qDebug()<<i<<pos;
    }
    ac_l[0]->setChecked(true);
    for (int i = 1; i < COUNT_LANGS; ++i)
    {
        if (lid == i)
        {
            ac_l[i]->setChecked(true);
            lang_m->setTitle(str_lmt[i]);
            break;
        }
    }
    for (int i = 0; i < COUNT_LANGS; ++i) connect(ac_l[i],SIGNAL(toggled(bool)),this,SLOT(LangChanged(bool)));

    //ru	ru4ng@mail.ru  Igor Yuferev RU4NG
    //zh	871684856@qq.com  SZE-TO Wing VR2UPU   Simplified Chinese
    //zhhk	871684856@qq.com  SZE-TO Wing VR2UPU   Traditional Chinese
    //eses	necktoni@hotmail.com  Toni Olmo EA3KE
    //caes	necktoni@hotmail.com  Toni Olmo EA3KE
    //ptpt	cu3ak@sapo.pt  Jaime Eloy CU3AK
    //roro	fenyo3jw@yahoo.com  Pit Stefan Fenyo YO3JW
    //dadk	5p1kzx@gmail.com  Michael 5P1KZX
    //plpl	sp5qwb@gmail.com  Bartek SP5QWB
    //frfr	on6dp@on6dp.be  Paul ON6DP  /or/  f1rvc.patrice@gmail.com  Patrice F1RVC
    //ptbr	crezivando@gmail.com  Crezivando Junior PP7CJ
    //nbno	kai.gunter.brandt@gmail.com  Kai Gunter Brandt LA3QMA
    //itit	iw4ard@aririmini.it  Gianni Matteini IW4ARD     Italian
    //cscz	ok1abb@email.cz  Miroslav Skoda, OK1ABB Czech
    //elgr  sv2clj@hotmail.com  Thomas, SV2CLJ Greek

    /*ru4ng@mail.ru,871684856@qq.com,necktoni@hotmail.com,cu3ak@sapo.pt,fenyo3jw@yahoo.com,
    5p1kzx@gmail.com,sp5qwb@gmail.com,on6dp@on6dp.be,crezivando@gmail.com,
    kai.gunter.brandt@gmail.com,iw4ard@aririmini.it,ok1abb@email.cz*/

    //// end Translation ////

    TMsCore = new MsCore();
    /*if (!TMsCore->is_select_sound_device) 2.74 stop
    {
    	QMessageBox::critical(nullptr, "MSHV",
    	("Sound Card Error\nPrimary Sound Driver Problem\nApplication Might Stop Working Properly"),
    	QMessageBox::Close);
    //QMessageBox msgBox(this);
        //msgBox.setWindowTitle("MSHV");
        //msgBox.setIcon(QMessageBox::Critical);
        //msgBox.setText("Sound Card Error\nPrimary Sound Driver Problem\nApplication Might Stop Working Properly");
        //msgBox.setStandardButtons(QMessageBox::Close);
        //msgBox.button(QMessageBox::Ok)->animateClick(2000); 
        //msgBox.exec();   	
    }*/

    THvRigControl = new HvRigControl(this);

    TSettingsMs = new SettingsMs(this);
    connect(TSettingsMs, SIGNAL(InDevChanged(QString,int,int,int,int,int,int)),
            this, SLOT(InDevChanged(QString,int,int,int,int,int,int)));
    connect(TSettingsMs, SIGNAL(rejected()), this, SLOT(SaveSettings()));
    connect(TSettingsMs, SIGNAL(EmitTciSelect(int)), THvRigControl, SLOT(SetTciSelect(int)));//2.59
    //connect(TSettingsMs, SIGNAL(StndInLevel(int)), TMsCore, SLOT(SetInLevel(int)));
    //connect(TSettingsMs, SIGNAL(SendSettingsTime(QStringList)), this, SLOT(SetTimeOffset(QStringList)));

    QPixmap p0 = QPixmap(":pic/sld_track_v_si.png");
    QPixmap p1 = QPixmap(":pic/sld_up_v_si.png");
    QPixmap p2 = QPixmap(":pic/sld_down_v_si.png");
    QPixmap p7 = QPixmap(":pic/tumb_v_si_pc.png");
    QPixmap p8 = QPixmap(":pic/tumb_over_v_si_pc.png");
    Slider_Tune_Disp = new HvSlider_V_Identif(0,100,0,0,p1,p0,p2,p7,p8);
    Slider_Tune_Disp->SetValue(50);
    Slider_Cont_Disp = new HvSlider_V_Identif(0,24,0,0,p1,p0,p2,p7,p8);
    Slider_Cont_Disp->SetValue(12);

    //QFont tx_font = font();
    //tx_font.setPointSize(10);
    //tx_font.setBold(true);
    l_tx_text = new QLabel("Txing:");
    l_tx_text->setFixedHeight(20);
    //l_tx_text->setFont(tx_font);
    l_tx_text->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    l_tx_text->setMinimumWidth(202);//215 1.30 from 215 240 zaradi nai dalgia texta na iskat

    MainDisplay = new DisplayMs(0,dsty);
    connect(TMsCore, SIGNAL(Set_Raw(int*,int,bool)), MainDisplay, SLOT(SetRaw(int*,int,bool)));  //1.27 psk rep   fopen bool true    false no file open
    connect(TMsCore, SIGNAL(Set_Graph(double*,int)), MainDisplay, SLOT(SetValue(double*,int)));

    connect(MainDisplay, SIGNAL(EmitFileNameChenged()), this, SLOT(FileNameChengedD1()));
    connect(MainDisplay, SIGNAL(EmitResetToBegin()), TMsCore, SLOT(FastResetSoundCardIn()));

    SecondDisplay = new DisplayMs(1,dsty);

    connect(MainDisplay, SIGNAL(SentData(QList<int*>,QPoint*,int,int*,QString,QString,bool,QLine*,int*,QString*,int*,int,int*,int,bool,QLine*,int,int,bool)),
            SecondDisplay, SLOT(ReciveData(QList<int*>,QPoint*,int,int*,QString,QString,bool,QLine*,int*,QString*,int*,int,int*,int,bool,QLine*,int,int,bool)));

    connect(Slider_Tune_Disp, SIGNAL(SendValue(int,int)), this, SLOT(setTuneDsp(int,int)));//1.55=
    connect(Slider_Cont_Disp, SIGNAL(SendValue(int,int)), this, SLOT(setTuneDsp(int,int)));//1.55=

    connect(SecondDisplay, SIGNAL(EmitFileNameChenged()), this, SLOT(FileNameChengedD2()));
    connect(SecondDisplay, SIGNAL(EmitRriorityDisp(bool)), MainDisplay, SLOT(SetRriorityDisp(bool)));

    TDecodeList1 = new DecodeList(1,dsty);
    TDecodeList2 = new DecodeList(2,dsty);
    FilterDialog = new HvFilterDialog(dsty,this);//(0) and open() no modal
    connect(FilterDialog, SIGNAL(EmitSetFilter(QStringList,bool*,QStringList,QStringList,QStringList,QStringList,QStringList,QStringList)),
            TDecodeList1, SLOT(SetFilter(QStringList,bool*,QStringList,QStringList,QStringList,QStringList,QStringList,QStringList)));
    FilterDialog->SetCountries(TDecodeList1->GetCountries());

    FontDialog = new HvFontDialog(App_Path,this);
    connect(FontDialog, SIGNAL(EmitFontList(QFont)), TDecodeList1, SLOT(SetFontList(QFont)));
    connect(FontDialog, SIGNAL(EmitFontList(QFont)), TDecodeList2, SLOT(SetFontList(QFont)));

    TDecoderMs = new DecoderMs(App_Path);
    connect(MainDisplay, SIGNAL(EmitDataToDecode(int*,int,QString,int,int,bool,bool,bool)), TDecoderMs, SLOT(SetDecode(int*,int,QString,int,int,bool,bool,bool)));//1.27 psk rep   fopen bool true    false no file open
    connect(SecondDisplay, SIGNAL(EmitDataToDecode(int*,int,QString,int,int,bool,bool,bool)), TDecoderMs, SLOT(SetDecode(int*,int,QString,int,int,bool,bool,bool)));//1.27 psk rep   fopen bool true    false no file open

    connect(TDecoderMs, SIGNAL(EmitBackColor(bool)), TDecodeList1, SLOT(SetBackColor(bool)));
    connect(TDecoderMs, SIGNAL(EmitDecodetText(QStringList,bool,bool)), TDecodeList1, SLOT(InsertItem_hv(QStringList,bool,bool))); //1.27 psk rep   fopen bool true    false no file open
    connect(TDecodeList1, SIGNAL(EmitLstNexColor(bool)), TDecoderMs, SLOT(SetNexColor(bool)));
    connect(TDecoderMs, SIGNAL(EmitTimeElapsed(float)), TDecodeList1, SLOT(SetTimeElapsed(float)));//2.33

    connect(TDecoderMs, SIGNAL(EmitDecodetTextRxFreq(QStringList,bool,bool)), TDecodeList2, SLOT(InsertItem_hv(QStringList,bool,bool)));
    TDecodeList2->SetBackColor(true);

    connect(MainDisplay, SIGNAL(EmitZapData(int*,int)), TDecoderMs, SLOT(SetZapData(int*,int)));
    connect(SecondDisplay, SIGNAL(EmitZapData(int*,int)), TDecoderMs, SLOT(SetZapData(int*,int)));
    connect(TDecoderMs, SIGNAL(EmitDecLinesPosToDisplay(int,double,double,QString)), MainDisplay, SLOT(SetDecLinesPosToDisplay(int,double,double,QString)));//1.28 QString p_time for identif perood
    connect(MainDisplay, SIGNAL(EmitDecLinesPosToSecondDisp(int,double,double,QString)), SecondDisplay, SLOT(SetDecLinesPosToDisplay(int,double,double,QString))); //1.28 p_time is not from this period sent to second display
    connect(MainDisplay, SIGNAL(EmitVDMouseDecodeSecondDisp(int)), SecondDisplay, SLOT(SetVDMouseDecodeSecondDisp(int)));

    TMsPlayerHV = new MsPlayerHV(App_Path);
    connect(TMsPlayerHV, SIGNAL(SentFileClarDisplay()), this, SLOT(SetFileToDisplay()));
    connect(TMsPlayerHV, SIGNAL(SentData(int*, int, bool)), TMsCore, SLOT(ReciveData(int*, int, bool))); //1.27 psk rep fopen bool true
    connect(TSettingsMs, SIGNAL(OutDevChanged(QString,int,int)), TMsPlayerHV, SLOT(SetSoundDevice(QString,int,int)));
    connect(MainDisplay, SIGNAL(EmitDataToSaveInFile(int*,int,QString)), TMsPlayerHV, SLOT(SaveFile(int*,int,QString)));
    connect(SecondDisplay, SIGNAL(EmitDataToSaveInFile(int*,int,QString)), TMsPlayerHV, SLOT(SaveFile(int*,int,QString)));
    connect(TMsPlayerHV, SIGNAL(SendTxMsgLabTx(QString)), this, SLOT(SetTxMsgLabTx(QString)));
    connect(TMsPlayerHV, SIGNAL(SendTxMsgAllTxt(QString,double)), this, SLOT(SetTxMsgAllTxt(QString,double)));

    TPicW = new LabW(dsty);
    connect(TDecoderMs, SIGNAL(EmitDecode(bool,int)), this, SLOT(SetDecodeBusy(bool,int)));
    connect(TDecoderMs, SIGNAL(EmitDecode(bool,int)), MainDisplay, SLOT(SetDecBusy(bool,int)));
    connect(TDecoderMs, SIGNAL(EmitDecode(bool,int)), SecondDisplay, SLOT(SetDecBusy(bool,int)));

    TPicW->SetRxMon(false);
    TPicW->SetDecode(false);

    THvSMeter_H = new HvSMeter_H(0,0, QPixmap(":pic/on.png"),QPixmap(":pic/clip.png"),QPixmap(":pic/back.png"),this);
    connect(TMsCore, SIGNAL(Sed_SMeter(int)), THvSMeter_H, SLOT(setValue(int)));

    //2.63
    desktopw = QApplication::desktop();
    screenWidth  = desktopw->screenGeometry(this).width(); 			// Primary Screen width
    screenHeight = desktopw->screenGeometry(this).height();			// Primary Screen height
    //DesktopPriSize = QSize(screenWidth,screenHeight);				// Primary Screen size
    //DesktopAllSize = QSize(desktop->width(),desktop->height());	// Primary + Secondary Screen
    QSize windowSize = size();
    int width  = windowSize.width();
    int height = windowSize.height();
    int x = (screenWidth  - width ) / 2;
    int y = (screenHeight - height) / 2;
    //y -= 50;//linux
    y -= 34;  //34
    x -= 4;  //4
    if (screenWidth<=800) x -= 25;  //25 // testvano
    // end 2.63
    //qDebug()<<"MinDisply="<<screenWidth<<screenHeight<<"Full="<<desktop->width()<<desktop->height();

    THvTxW = new HvTxW(InstName,App_Path,lid,dsty,x,y);

    connect(THvTxW, SIGNAL(EmitDistUnit(bool)),TDecodeList1,SLOT(SetDistUnit(bool)));
    connect(THvTxW, SIGNAL(EmitDistUnit(bool)),TDecodeList2,SLOT(SetDistUnit(bool)));
    connect(THvTxW, SIGNAL(EmitHisCalls(QStringList)),TDecodeList1,SLOT(SetHisCalls(QStringList)));//2.73
    id_mshf = 0; //2.76 for stop TX ?
    f_is_myCstd = true; //2.76.1
    connect(THvTxW,SIGNAL(EmitMshfChanget(uint8_t)),this,SLOT(SetMshf(uint8_t)));
    connect(THvTxW,SIGNAL(EmitSFMATxAll(QString)),TMsPlayerHV,SLOT(SetSFMATxAll(QString)));//2.76sf
    connect(THvTxW,SIGNAL(EmitOtpTxKey(QString)),TMsPlayerHV,SLOT(SetOtpTxKey(QString)));//2.76sf
    connect(THvTxW,SIGNAL(EmitOtpRxMsg(bool)),TDecodeList1,SLOT(SetShowOtpRxMsg(bool)));//2.76sf
    connect(THvTxW,SIGNAL(EmitOtpRxMsg(bool)),TDecodeList2,SLOT(SetShowOtpRxMsg(bool)));//2.76sf
    THvTxW->RefreshOtpKeyMsg();
    connect(THvTxW,SIGNAL(EmitOtpVerif(QString,uint8_t)),TDecodeList1,SLOT(SetOtpVerif(QString,uint8_t)));//2.76sf only to list 1
    //connect(THvTxW,SIGNAL(EmitSFoxVerif(QString)),TDecodeList2,SLOT(SetSFoxVerif(QString)));//2.76sf
    connect(THvTxW,SIGNAL(EmitOffsetDt(int)),this,SLOT(SetOffsetDt(int)));//2.76sf

    QMenuBar *Min_Menu = new QMenuBar();

#if defined _WIN32_
    Start_Cmd = new QProcess;
#endif

    QAction *Test_tones_m = new QAction(QPixmap(":pic/sine.png"),tr("Generate Messages For Test Tones"),this);

#if defined _WIN32_
    QAction *SyncTime_m = new QAction(QPixmap(":pic/sync_st.png"),tr("Time Synchronization"),this);
    connect(SyncTime_m, SIGNAL(triggered()), this, SLOT(SyncTime()));
#endif

    QAction *OCheckTime_m = new QAction(QPixmap(":pic/sync_ot.png"),tr("Online Time Check"),this);
    connect(OCheckTime_m, SIGNAL(triggered()), this, SLOT(OnlineTimeCheck()));

    QAction *Font_m = new QAction(QPixmap(":pic/fonts.png"),tr("Font Settings"),this);
    connect(Font_m, SIGNAL(triggered()), FontDialog, SLOT(exec()));

    THvTxtColor = new HvTxtColor(dsty,this);
    QAction *Txt_colors_m = new QAction(QPixmap(":pic/txt_color.png"),tr("Text Highlight"),this);
    connect(Txt_colors_m, SIGNAL(triggered()), THvTxtColor, SLOT(exec()));

    QMenu *Option_m = new QMenu(tr("Options"));
    Option_m->addAction(QPixmap(":pic/settings.png"),ShKey[lid][4][1], TSettingsMs, SLOT(exec()),QKeySequence(tr(ShKey[lid][4][0],ShKey[lid][4][1])));
    Option_m->addAction(QPixmap(":pic/com_p.png"),ShKey[lid][5][1], THvRigControl, SLOT(exec()),QKeySequence(tr(ShKey[lid][5][0],ShKey[lid][5][1])));
    Option_m->addAction(QPixmap(":pic/macro.png"),ShKey[lid][6][1], THvTxW, SLOT(Macros_exec()),QKeySequence(tr(ShKey[lid][6][0],ShKey[lid][6][1])));

#if defined _WIN32_
    Option_m->addAction(QPixmap(":pic/mixer_play.png"),ShKey[lid][7][1], this, SLOT(WMixPlay()),QKeySequence(tr(ShKey[lid][7][0],ShKey[lid][7][1])));
    Option_m->addAction(QPixmap(":pic/mixer_rec.png"),ShKey[lid][8][1], this, SLOT(WMixRec()),QKeySequence(tr(ShKey[lid][8][0],ShKey[lid][8][1])));
#endif
#if defined _LINUX_
    THvMixerMain = new HvMixerMain();
    Option_m->addAction(QPixmap(":pic/mixer_play.png"),ShKey[lid][7][1], THvMixerMain, SLOT(Start()),QKeySequence(tr(ShKey[lid][7][0],ShKey[lid][7][1])));
    Option_m->addAction(QPixmap(":pic/mixer_rec.png"),ShKey[lid][8][1], THvMixerMain, SLOT(Start()),QKeySequence(tr(ShKey[lid][8][0],ShKey[lid][8][1])));
#endif

    QAction *NetW = new QAction(QPixmap(":pic/radionet.png"),tr("Network Configuration"),this);
    QAction *RadFreqW = new QAction(QPixmap(":pic/rad_freq.png"),tr("Radio And Frequencies Configuration"),this);
    Option_m->addAction(NetW);
    Option_m->addAction(RadFreqW);
    Option_m->addAction(Font_m);
    Option_m->addAction(Txt_colors_m);
    Option_m->addAction(Test_tones_m);

#if defined _WIN32_
    Option_m->addAction(SyncTime_m);
#endif

    //Option_m->addSeparator();
    QMenu *Dlf_m = new QMenu(tr("Decode Lists Options"));
    Dlf_m->setIcon(QPixmap(":pic/smenu.png"));
    Option_m->addMenu(Dlf_m);

    for (int i = 0; i < COUNT_MODE; ++i) two_dec_list_all[i] = false;
    two_dec_list_all[11] = true;  //ft8
    two_dec_list_all[13] = true;  //ft4
    two_dec_list_all[14] = true;  //q65a
    two_dec_list_all[15] = true;  //q65b
    two_dec_list_all[16] = true;  //q65c
    two_dec_list_all[17] = true;  //q65d
    two_dec_list_all[18] = true;  //ft2

    ac_two_dec_list = new QAction(tr("Use Two Decode Lists")+" MSK FT JT65 Q65",this);// Q65 2.55
    ac_two_dec_list->setCheckable(true);
    Dlf_m->addAction(ac_two_dec_list);
    connect(ac_two_dec_list, SIGNAL(toggled(bool)), this, SLOT(SetTwoDecList(bool)));

    ac_2click_list_autu_on = new QAction(tr("Double Click On Call Sets Auto Is On")+" MSK FT JT65 Q65",this);// Q65 2.55
    ac_2click_list_autu_on->setCheckable(true);
    Dlf_m->addAction(ac_2click_list_autu_on);

    ac_new_dec_clr_msg_list = new QAction(tr("New decode period to clear Message List")+" FT",this);
    ac_new_dec_clr_msg_list->setCheckable(true);
    Dlf_m->addAction(ac_new_dec_clr_msg_list);

    ac_click_on_call_show_cty = new QAction(tr("Single Click On Call Shows Country Info")+" MSK FT Q65",this);// Q65 2.55
    ac_click_on_call_show_cty->setCheckable(true);
    Dlf_m->addAction(ac_click_on_call_show_cty);

    ac_show_timec = new QAction(tr("Show/Hide Time Column")+" FT",this);
    ac_show_timec->setCheckable(true);
    ac_show_timec->setChecked(true);
    Dlf_m->addAction(ac_show_timec);
    ac_show_counc = new QAction(tr("Show/Hide Country Column")+" MSK FT Q65",this);
    ac_show_counc->setCheckable(true);
    Dlf_m->addAction(ac_show_counc);
    ac_show_distc = new QAction(tr("Show/Hide Distance Column")+" MSK FT",this);
    ac_show_distc->setCheckable(true);
    Dlf_m->addAction(ac_show_distc);
    ac_show_freqc = new QAction(tr("Show/Hide Frequency Column")+" MSK FT",this);
    ac_show_freqc->setCheckable(true);
    ac_show_freqc->setChecked(true);
    Dlf_m->addAction(ac_show_freqc);
    for (int i = 0; i < COUNT_MODE; ++i)
    {
        s_show_lc[i][0] = true;//time
        s_show_lc[i][1] = false;//dist
        if (i==4 || i==5) s_show_lc[i][2] = false;//iskat-a iskat-b no freq column
        else s_show_lc[i][2] = true;//freq
        s_show_lc[i][3] = false;//Country
    }
    connect(ac_show_timec,SIGNAL(toggled(bool)),this,SLOT(SetSHAllColl(bool)));//2.72
    connect(ac_show_counc,SIGNAL(toggled(bool)),this,SLOT(SetSHAllColl(bool)));//2.73
    connect(ac_show_distc,SIGNAL(toggled(bool)),this,SLOT(SetSHAllColl(bool)));//2.72
    connect(ac_show_freqc,SIGNAL(toggled(bool)),this,SLOT(SetSHAllColl(bool)));//2.72

    ac_filter_list = new QAction(QPixmap(":pic/filters.png"),tr("Decode List Filters")+" FT",this);
    Dlf_m->addAction(ac_filter_list);
    connect(ac_filter_list, SIGNAL(triggered()), FilterDialog, SLOT(exec()));
    connect(TDecodeList1, SIGNAL(EmitHeaderDoubleClicked()), FilterDialog, SLOT(exec()));

    connect(ac_click_on_call_show_cty, SIGNAL(toggled(bool)), TDecodeList1, SLOT(SetStaticClickOnCallShowCty(bool)));//2.66
    //connect(ac_click_on_call_show_cty, SIGNAL(toggled(bool)), TDecodeList2, SLOT(SetClickOnCallShowCty(bool)));
    ac_click_on_call_show_cty->setChecked(true);//2.15

    //Option_m->addSeparator();
    QMenu *Logf_m = new QMenu(tr("Log Options"));
    Logf_m->setIcon(QPixmap(":pic/smenu.png"));
    Option_m->addMenu(Logf_m);
    Direct_log_qso = new QAction(tr("Log Automatically QSO")+" MSK FT Q65",this);// Q65 2.55
    Direct_log_qso->setCheckable(true);
    //Option_m->addAction(Direct_log_qso);
    Logf_m->addAction(Direct_log_qso);
    Prompt_log_qso = new QAction(tr("Prompt Me To Log QSO")+" MSK FT Q65",this);// Q65 2.55
    Prompt_log_qso->setCheckable(true);
    //Option_m->addAction(Prompt_log_qso);
    Logf_m->addAction(Prompt_log_qso);

    Info_dupe_qso = new QAction(tr("Warn Me If QSO Before"),this);
    Info_dupe_qso->setCheckable(true);
    Logf_m->addAction(Info_dupe_qso);

    Log_qso_startdt_eq_enddt = new QAction(tr("Log QSO Start Date,Time = End Date,Time"),this);
    Log_qso_startdt_eq_enddt->setCheckable(true);
    //Option_m->addAction(Log_qso_startdt_eq_enddt);
    Logf_m->addAction(Log_qso_startdt_eq_enddt);

    Log_auto_comm = new QAction(tr("Turn Auto Comments Off"),this);
    Log_auto_comm->setCheckable(true);
    Logf_m->addAction(Log_auto_comm);

    QAction *ac_autologinfo = new QAction(tr("Auto Logging Info Settings"),this);//2.75
    ac_autologinfo->setIcon(QPixmap(":pic/smenu.png"));
    connect(ac_autologinfo,SIGNAL(triggered()),THvTxW,SLOT(SetAutoLogInfo()));
    Logf_m->addAction(ac_autologinfo);

    QMenu *Other_m = new QMenu(tr("Other Options"));
    Other_m->setIcon(QPixmap(":pic/smenu.png"));
    Option_m->addMenu(Other_m);
    ac_start_qso_from_tx2_or_tx1 = new QAction(tr("Skip Tx1")+" MSK FT Q65 ("+tr("Uncheck for DXpedition")+")",this);//+" FT)"
    //ac_start_qso_from_tx2_or_tx1 = new QAction(tr("Skip Tx1")+" MSK FT Q65 ("+tr("Uncheck for DXpedition and No STD Call")+" FT)",this);//2.61
    ac_start_qso_from_tx2_or_tx1->setCheckable(true); // Skip TX1  <- g4hgi Uncheck for DX Expedition
    Other_m->addAction(ac_start_qso_from_tx2_or_tx1);//2.49

    ac_use_queue_cont = new QAction(tr("Use Queue (For Contest Activitiеs Only)")+" MSK FT Q65",this);//Use Queue Call (For Contest Only) Use Queue Call (For Contest Activitiеs Only)
    ac_use_queue_cont->setCheckable(true);
    Other_m->addAction(ac_use_queue_cont);//2.59

    Other_m->addSeparator();

    recognize_tp1 = new QAction(tr("Recognize Period")+" JTMS FSK ISCAT JT6M",this);
    recognize_tp1->setCheckable(true);
    recognize_tp1->setChecked(true);
    Other_m->addAction(recognize_tp1);
    recognize_tp2 = new QAction(tr("Recognize Period")+" MSK FT JT65 Q65",this);// Q65 2.55
    recognize_tp2->setCheckable(true);
    recognize_tp2->setChecked(true);
    Other_m->addAction(recognize_tp2);

    Other_m->addSeparator();

    zero_df_scale_m = new QAction(tr("View JT65 DF Axis On Display"),this);
    zero_df_scale_m->setCheckable(true);
    Other_m->addAction(zero_df_scale_m);
    zero_df_scale_m->setChecked(false);
    connect(zero_df_scale_m, SIGNAL(toggled(bool)), MainDisplay, SLOT(SetZeroDfVScale(bool)));
    zero_df_scale_m->setEnabled(false);
    vd_mouse_lines_draw = new QAction(tr("Turn Off JT65 Display Markers"),this);
    vd_mouse_lines_draw->setCheckable(true);
    Other_m->addAction(vd_mouse_lines_draw);
    connect(vd_mouse_lines_draw, SIGNAL(toggled(bool)), MainDisplay, SLOT(SetVDMouseDrawLines(bool)));
    connect(vd_mouse_lines_draw, SIGNAL(toggled(bool)), SecondDisplay, SLOT(SetVDMouseDrawLines(bool)));

    vd_bw_lines_draw[0] = new QAction(tr("Turn On Mouse Markers")+" FT Q65",this);
    vd_bw_lines_draw[0]->setCheckable(true);
    Other_m->addAction(vd_bw_lines_draw[0]);
    connect(vd_bw_lines_draw[0], SIGNAL(toggled(bool)), MainDisplay, SLOT(SetVDMouseBWDrawLines(bool)));
    vd_bw_lines_draw[1] = new QAction(tr("Turn On RX Markers")+" FT Q65",this);
    vd_bw_lines_draw[1]->setCheckable(true);
    Other_m->addAction(vd_bw_lines_draw[1]);
    vd_bw_lines_draw[1]->setChecked(true);//2.76.1
    connect(vd_bw_lines_draw[1], SIGNAL(toggled(bool)), MainDisplay, SLOT(SetVDMouseBWRXDrawLines(bool)));
    vd_bw_lines_draw[2] = new QAction(tr("Turn On TX Markers")+" FT Q65",this);
    vd_bw_lines_draw[2]->setCheckable(true);
    Other_m->addAction(vd_bw_lines_draw[2]);
    vd_bw_lines_draw[2]->setChecked(true);//2.76.1
    connect(vd_bw_lines_draw[2], SIGNAL(toggled(bool)), MainDisplay, SLOT(SetVDMouseBWTXDrawLines(bool)));

    Other_m->addSeparator();//->setText("Multi Answering Auto Seq Protocol");

    //Other_m->addSection("Multi Answering Auto Seq Protocol");
    Multi_answer_mod = new QAction(QString(ShKey[lid][26][1]+"\t"+ShKey[lid][26][0]),this);
    Other_m->addAction(Multi_answer_mod);
    Multi_answer_mod->setCheckable(true);
    Multi_answer_mod->setEnabled(false);
    Multi_answer_mod_std = new QAction(QString(ShKey[lid][27][1]+"\t"+ShKey[lid][27][0]),this);
    Other_m->addAction(Multi_answer_mod_std);
    Multi_answer_mod_std->setCheckable(true);
    Multi_answer_mod_std->setEnabled(false);
    g_block_mam = false;
    g_ub_m_k = false;
    g_ub_m_k2 = false;
    g_ub_m_k3 = false;
    //Switch Off Automatic Adding Calls To Queue
    MA_man_adding = new QAction(tr("Manually Add Calls To Queue By Double Click"),this);//by double click
    Other_m->addAction(MA_man_adding);//2.74
    /*Other_m->setDefaultAction(MA_man_adding);
    if(dsty) Other_m->setStyleSheet("QMenu::item:default{font-weight:bold;color:rgb(255,150,150);}");
    else Other_m->setStyleSheet("QMenu::item:default{font-weight:bold;color:rgb(180,0,0);}");*/
    MA_man_adding->setCheckable(true);
    MA_man_adding->setEnabled(false);
    connect(MA_man_adding,SIGNAL(toggled(bool)),THvTxW,SLOT(SetMaManAdding(bool)));

    Option_m->addSeparator();

    QString str_bt0[COUNT_MODE];//2.74
    int btid0[COUNT_MODE];
    for (int i = 0; i < COUNT_MODE; ++i)
    {
        str_bt0[i] = ModeStr(pos_mods[i]);
        btid0[i]   = pos_mods[i];
    }
    W_mod_bt_sw = new HvWBtSw(this,COUNT_MODE,btid0,str_bt0,tr("Mode Switcher Buttons"),tr("Choose Modes")+":",
                              tr("USE MODE SWITCHER"),"0#2#18#13#11#14#15#7#8#222",dsty);
    //connect(W_mod_bt_sw,SIGNAL(clicked(int)),this,SLOT(ModBtSwClicked(int)));move down when rb_mode[] is created
    QAction *ac_bt_mode = new QAction(tr("Mode Switcher Buttons"),this);
    ac_bt_mode->setIcon(QPixmap(":pic/smenu.png"));
    connect(ac_bt_mode,SIGNAL(triggered()),W_mod_bt_sw,SLOT(dexec()));
    Option_m->addAction(ac_bt_mode);

    QString str_bt1[COUNT_BANDS];//2.74
    int btid1[COUNT_BANDS];
    for (int i = 0; i < COUNT_BANDS; ++i)
    {
        str_bt1[i] = lst_bands[i];
        btid1[i]   = i;
    }
    W_band_bt_sw = new HvWBtSw(this,COUNT_BANDS,btid1,str_bt1,tr("Band Switcher Buttons"),tr("Choose Bands")+":",
                               tr("USE BAND SWITCHER"),"3#4#5#6#7#8#9#10#11#13#15#18#20#222",dsty);
    //connect(W_band_bt_sw,SIGNAL(clicked(int)),this,SLOT(BandBtSwClicked(int)));move down when ListBands is created
    QAction *ac_bt_band = new QAction(tr("Band Switcher Buttons"),this);
    ac_bt_band->setIcon(QPixmap(":pic/smenu.png"));
    connect(ac_bt_band, SIGNAL(triggered()),W_band_bt_sw,SLOT(dexec()));
    Option_m->addAction(ac_bt_band);

    sh_wf= new QAction(QString(ShKey[lid][32][1]+"\t"+ShKey[lid][32][0]),this);
    sh_wf->setShortcut(QKeySequence(tr(ShKey[lid][32][0],ShKey[lid][32][1])));
    sh_wf->setCheckable(true);
    sh_wf->setChecked(true);
    sh_tx= new QAction(QString(ShKey[lid][33][1]+"\t"+ShKey[lid][33][0]),this);
    sh_tx->setShortcut(QKeySequence(tr(ShKey[lid][33][0],ShKey[lid][33][1])));
    sh_tx->setCheckable(true);
    sh_tx->setChecked(true);
    Option_m->addAction(sh_wf);
    Option_m->addAction(sh_tx);

    Start_astro_m = new QAction(tr("View Astronomical Data"),this);
    Start_astro_m->setCheckable(true);
    Option_m->addAction(Start_astro_m);

    Option_m->addSeparator();

    //ac_aseq_max_dist = new QAction(tr("Use Auto Seq By Max Distance")+" MSK FT Q65",this);
    ac_aseq_max_dist = new QAction(tr("ASeq: Reply to the Most Distant.")+" MSK FT Q65",this);
    //ac_aseq_max_dist = new QAction(tr("ASeq: Reply to the Farthest.")+" MSK FT Q65",this);
    ac_aseq_max_dist->setCheckable(true);
    Option_m->addAction(ac_aseq_max_dist);
    connect(ac_aseq_max_dist, SIGNAL(toggled(bool)), THvTxW, SLOT(SetUseASeqMaxDist(bool)));//2.66

    ac_Cfm73 = new QAction(tr("TX Confirmation If 73 Or RR73")+" FT Q65",this);// Q65 2.55
    ac_Cfm73->setCheckable(true);
    Option_m->addAction(ac_Cfm73);
    ac_Cfm73->setEnabled(false);

    /*ac_ft_df1500 = new QAction(tr("Use Default DF Tolerance 1500 Hz")+" FT",this);
    ac_ft_df1500->setCheckable(true);
    Option_m->addAction(ac_ft_df1500);
    ac_ft_df1500->setEnabled(false);*/

    ac_areset_qso = new QAction(tr("Auto RESET QSO at end")+" MSK FT Q65",this);// Q65 2.55
    ac_areset_qso->setCheckable(true);
    Option_m->addAction(ac_areset_qso);
    ac_areset_qso->setEnabled(false);
    ac_areset_qso->setChecked(true);

    Option_m->addSeparator();

    Mon_start_m = new QAction(tr("Monitor ON At Startup"),this);
    Mon_start_m->setCheckable(true);
    //connect(Mon_start_m, SIGNAL(changed()), this, SLOT(SaveFile()));
    Option_m->addAction(Mon_start_m);

    //THvTxW = new HvTxW(App_Path,lid,dsty);

    connect(ac_Cfm73, SIGNAL(toggled(bool)), THvTxW, SLOT(SetCfm73(bool)));
    ac_Cfm73->setChecked(true);//by default for this moment
    /*connect(ac_ft_df1500, SIGNAL(toggled(bool)), THvTxW, SLOT(SetFtDf1500(bool)));//2.41
    connect(ac_ft_df1500, SIGNAL(toggled(bool)), MainDisplay, SLOT(SetFtDf1500(bool)));//2.41  
    ac_ft_df1500->setChecked(true);//2.76.1*/
    connect(ac_areset_qso, SIGNAL(toggled(bool)), THvTxW, SLOT(SetAResetQsoAtEnd(bool)));//2.49

    connect(Multi_answer_mod, SIGNAL(toggled(bool)), this, SLOT(SetMultiAnswerMod(bool)));
    connect(Multi_answer_mod_std, SIGNAL(toggled(bool)), this, SLOT(SetMultiAnswerModStd(bool)));

    connect(recognize_tp1, SIGNAL(toggled(bool)), THvTxW, SLOT(SetRecognizeTp1(bool)));
    connect(recognize_tp2, SIGNAL(toggled(bool)), THvTxW, SLOT(SetRecognizeTp2(bool)));

    connect(NetW, SIGNAL(triggered()), THvTxW, SLOT(NetW_exec()));
    connect(RadFreqW, SIGNAL(triggered()), THvTxW, SLOT(RadioFreqW_exec()));

    connect(Start_astro_m, SIGNAL(toggled(bool)), this, SLOT(ShowCloseAstroW(bool)));
    connect(THvTxW, SIGNAL(EmitAstroWIsClosed()), this, SLOT(SetAstroWIsClosed()));

    //connect(THvTxW, SIGNAL(EmitLocStInfo(QString,QString,QString)), TRadioAndNetW, SLOT(SetLocalStation(QString,QString,QString)));

    connect(TDecodeList1, SIGNAL(EmitRxStationInfo(QStringList,int,bool,uint8_t)), THvTxW, SLOT(ValidateStationInfo(QStringList,int,bool,uint8_t)));
    //1.78 only for spotting this down
    connect(TDecodeList2, SIGNAL(EmitRxStationInfo(QStringList,int,bool,uint8_t)), THvTxW, SLOT(ValidateStationInfo(QStringList,int,bool,uint8_t)));
    connect(THvTxW , SIGNAL(EmitGBlockListExp(bool)), TDecodeList1, SLOT(SetStaticGBlockListExp(bool)));//2.66
    //connect(THvTxW , SIGNAL(EmitGBlockListExp(bool)), TDecodeList2, SLOT(SetGBlockListExp(bool)));//2.06

    connect(THvTxW, SIGNAL(EmitQrgQSY(QStringList)), this, SLOT(SetQrgQSY(QStringList)));//2.46

    connect(TDecodeList1, SIGNAL(EmitRxTextForAutoSeq(QStringList)), THvTxW, SLOT(SetTextForAutoSeq(QStringList)));

    //connect(THvTxW, SIGNAL(EmitRemoteStation(QString,QString,int,QString,int,int)),
    //TRadioAndNetW, SLOT(AddRemoteStation(QString,QString,int,QString,int,int)));
    connect(TDecoderMs, SIGNAL(EmitDecodeInProgresPskRep(bool)), THvTxW, SLOT(SetDecodeInProgresPskRep(bool)));

    connect(THvTxW, SIGNAL(StndInLevel(int)), TMsCore, SLOT(SetInLevel(int)));
    connect(THvTxtColor, SIGNAL(EmitTextMark(bool*,QString)), THvTxW, SLOT(SetTextMark(bool*,QString)));
    connect(THvTxtColor, SIGNAL(EmitTextMarkColors(QColor*,int,bool,bool)), TDecodeList1, SLOT(SetTextMarkColors(QColor*,int,bool,bool)));
    connect(THvTxtColor, SIGNAL(EmitTextMarkColors(QColor*,int,bool,bool)), TDecodeList2, SLOT(SetTextMarkColors(QColor*,int,bool,bool)));
    connect(THvTxtColor, SIGNAL(EmitTextMark(bool*,QString)),FilterDialog,SLOT(SetTextMark(bool*,QString)));//2.69

    connect(THvTxW, SIGNAL(EmitDListMarkText(QStringList,int,int,int,int,int)), TDecodeList1, SLOT(SetDListMarkText(QStringList,int,int,int,int,int)));
    connect(THvTxW, SIGNAL(EmitDListMarkText(QStringList,int,int,int,int,int)), TDecodeList2, SLOT(SetDListMarkText(QStringList,int,int,int,int,int)));

    ///JTMSK SHORT///
    connect(THvTxW, SIGNAL(EmitListHashCalls(QStringList)), TDecoderMs, SLOT(SetCalsHash(QStringList)));
    connect(THvTxW, SIGNAL(EmitMAMCalls(QStringList)), TDecoderMs, SLOT(SetMAMCalls(QStringList)));

    ///JTMSK SHORT///
    connect(THvTxW, SIGNAL(EmitRxOnlyFiSe(bool)), this, SLOT(SetRxOnlyFiSe(bool)));

    /// FT8 ///
    connect(THvTxW, SIGNAL(EmitTxToRx(bool)), MainDisplay, SLOT(SetTxToRx(bool)));
    connect(THvTxW, SIGNAL(EmitRxToTx(bool)), MainDisplay, SLOT(SetRxToTx(bool)));//2.63

    s_v_disp_tx_frq = 1200.0;//2.76.1 = 1700??? 2.16 for ft8 only
    s_static_tx_frq = 1500;//2.16
    f_static_tx = false;//2.16
    connect(MainDisplay, SIGNAL(EmitVDTxFreq(double)), TDecoderMs, SLOT(SetTxFreq(double)));
    connect(THvRigControl, SIGNAL(EmitStaticTxFrq(bool,int)), this, SLOT(SetStaticTxFrq(bool,int)));//2.16
    connect(MainDisplay, SIGNAL(EmitVDTxFreq(double)), this, SLOT(SetTxFreq(double)));//2.16
    connect(MainDisplay, SIGNAL(EmitVDTxFreq(double)), THvRigControl, SLOT(SetTxFreq(double)));//2.16

    connect(THvTxW, SIGNAL(EmitLockTxrx(bool)), MainDisplay, SLOT(SetLockTxrx(bool)));
    connect(THvTxW, SIGNAL(EmitFreqTxW(double)), MainDisplay, SLOT(SetFreqExternal(double)));
    connect(TDecodeList1, SIGNAL(EmitFreqDecListClick(double)), THvTxW, SLOT(SetFreqTxW(double)));
    connect(TDecodeList2, SIGNAL(EmitFreqDecListClick(double)), THvTxW, SLOT(SetFreqTxW(double)));

    connect(THvTxW, SIGNAL(EmitUdpCmdDl(QStringList)), TDecodeList1, SLOT(SetUdpCmdDl(QStringList)));//2.22
    connect(THvTxW, SIGNAL(EmitUdpCmdDl(QStringList)), TDecodeList2, SLOT(SetUdpCmdDl(QStringList)));//2.22
    connect(THvTxW, SIGNAL(EmitUdpCmdStop(bool)), this, SLOT(SetUdpCmdStop(bool)));//2.22
    connect(TDecodeList1, SIGNAL(EmitUdpDecClr()), THvTxW, SLOT(SetUdpDecClr()));//2.22

    connect(TDecodeList1, SIGNAL(EmitDetectTextInMsg(QString,QString &)), THvTxW, SLOT(DlDetectTextInMsg(QString,QString &)));//for tooltip 2.27
    connect(TDecodeList2, SIGNAL(EmitDetectTextInMsg(QString,QString &)), THvTxW, SLOT(DlDetectTextInMsg(QString,QString &)));//for tooltip 2.27

    connect(Direct_log_qso, SIGNAL(toggled(bool)), this, SLOT(SetDLogQso(bool)));
    connect(Prompt_log_qso, SIGNAL(toggled(bool)), this, SLOT(SetPLogQso(bool)));
    Direct_log_qso->setChecked(true);//2.51

    connect(Info_dupe_qso, SIGNAL(toggled(bool)), THvTxW, SLOT(SetInfoDupeQso(bool)));
    Info_dupe_qso->setChecked(true);//2.51

    connect(Log_qso_startdt_eq_enddt, SIGNAL(toggled(bool)), THvTxW, SLOT(SetLogQsoStartDtEqEndDt(bool)));
    connect(Log_auto_comm, SIGNAL(toggled(bool)), THvTxW, SLOT(SetLogAutoComm(bool)));
    //THvTxW->HisLocChanged(); // for ft8

    connect(THvRigControl, SIGNAL(EmitTxWatchdogParms(int,int,int)), THvTxW, SLOT(SetTxWatchdogParms(int,int,int)));
    connect(THvRigControl, SIGNAL(EmitModSetFrqToRig(bool)), THvTxW, SLOT(SetModSetFrqToRig(bool)));

    //connect(THvTxW, SIGNAL(EmitRigBandFromFreq(int)), this, SLOT(SetBandFromRigFreq(int)));//1.61= down is started
    connect(THvTxW, SIGNAL(EmitFreqGlobalToRig(QString,int)), THvRigControl, SLOT(SetFreq(QString,int)));//2.74
    //connect(THvTxW, SIGNAL(EmitModeGlobalToRig(QString)), THvRigControl, SLOT(SetMode(QString)));//1.61= stop for the moment
    connect(THvRigControl, SIGNAL(EmitGetedFreq(QString)), THvTxW, SLOT(SetFreqGlobalFromRigCat(QString)));//1.61=
    connect(THvRigControl, SIGNAL(EmitGetedMode(QString)), THvTxW, SLOT(SetModeGlobalFromRigCat(QString)));//1.61=
    connect(THvRigControl, SIGNAL(EmitTxActive(int)), THvTxW, SLOT(SetTxActive(int)));//2.21
    connect(THvRigControl, SIGNAL(EmitRigCatActiveAndRead(bool,QString)), THvTxW, SLOT(SetRigCatActiveAndRead(bool,QString)));//2.53 //2.76.1

    connect(THvTxW, SIGNAL(EmitQSOProgress(int)), TDecoderMs, SLOT(SetQSOProgress(int)));
    connect(THvTxW, SIGNAL(EmitQSOProgress(int)), THvRigControl, SLOT(SetQSOProgress(int)));//2.45
    connect(THvTxW, SIGNAL(EmitFreqGlobalToDec(QString)),TDecoderMs,SLOT(SetFreqGlobal(QString)));//2.76.5

    connect(THvTxW, SIGNAL(EmitQrgParms(QString,bool)), THvRigControl, SLOT(SetQrgParms(QString,bool)));//2.45
    connect(THvRigControl, SIGNAL(EmitQrgActive(int)), THvTxW, SLOT(SetQrgActive(int)));//2.45
    connect(THvRigControl, SIGNAL(EmitQrgFromRig(QString)), THvTxW, SLOT(SetQrgFromRig(QString)));//2.45
    connect(THvRigControl, SIGNAL(EmitQrgInfoFromCat(QString)), THvTxW, SLOT(SetQrgInfoFromCat(QString)));//2.45

    connect(THvTxW, SIGNAL(Emit65DeepSearchDb(QStringList)), TDecoderMs, SLOT(Set65DeepSearchDb(QStringList)));//1.49 deep search 65
    THvTxW->Refr65DeepSearchDb();//1.49 deep search 65

    QDateTime utc_tt = getDateTime();
    THvTxW->SetDataTime(utc_tt);//calculate hotA right

    QMenu *File_m = new QMenu(tr("File"));
    //File_m->addAction(Open_m);
    File_m->addAction(QPixmap(":pic/fileopen.png"),ShKey[lid][2][1], this, SLOT(Open()),QKeySequence(tr(ShKey[lid][2][0],ShKey[lid][2][1])));
    File_m->addAction(QPixmap(":pic/log.png"),ShKey[lid][9][1], THvTxW, SLOT(ShowLog()),QKeySequence(tr(ShKey[lid][9][0],ShKey[lid][9][1])));

    QAction *SetDb = new QAction(QPixmap(":pic/setdb.png"),tr("Overwrite Locator Database"),this);
    File_m->addAction(SetDb);
    connect(SetDb, SIGNAL(triggered()), THvTxW, SLOT(SetNewDb()));

    File_m->addAction(QPixmap(":pic/exit.png"),ShKey[lid][3][1], this, SLOT(close()),QKeySequence(tr(ShKey[lid][3][0],ShKey[lid][3][1])));

    TCustomPalW = new CustomPalW(x,y);
    connect(TCustomPalW, SIGNAL(EmitCustomPalette(QPixmap,QColor,QColor)), MainDisplay, SLOT(SetCustomPalette(QPixmap,QColor,QColor)));
    connect(TCustomPalW, SIGNAL(EmitCustomPalette(QPixmap,QColor,QColor)), SecondDisplay, SLOT(SetCustomPalette(QPixmap,QColor,QColor)));
    TCustomPalW->SetPaletteToDisplay();

    QMenu *Palette_m = new QMenu(tr("Palette"));
    //Palette_m->setContentsMargins(2,0,2,0);
    rb_palette[0] = new QAction(tr("Default BW"),this);
    rb_palette[0]->setCheckable(true);
    rb_palette[1] = new QAction(tr("Default Color"),this);
    rb_palette[1]->setCheckable(true);
    rb_palette[1]->setChecked(true);
    for (int i = 2; i < 8; ++i)
    {
        rb_palette[i] = new QAction(QString("%1").arg(i-1)+" "+tr("Palette"),this);
        rb_palette[i]->setCheckable(true);
    }
    rb_palette[8] = new QAction(tr("Custom Palette"),this);
    rb_palette[8]->setCheckable(true);

    QActionGroup *ac_gr_pal = new QActionGroup(this);
    for (int i = 0; i < 9; ++i) ac_gr_pal->addAction(rb_palette[i]);//2.65
    for (int i = 0; i < 9; ++i) Palette_m->addAction(rb_palette[i]);//2.65

    Palette_m->addSeparator();
    //QAction *a_start_pal_edit = new QAction("Custom Palette Editor",this);
    QAction *a_start_pal_edit = new QAction(QPixmap(":pic/pal_edit.png"),tr("Custom Palette Editor"),this);
    Palette_m->addAction(a_start_pal_edit);
    connect(a_start_pal_edit, SIGNAL(triggered()), TCustomPalW, SLOT(show()));

    //Dark style
    ac_dark_st = new QAction("MSHV "+tr("Dark Style"),this);
    ac_dark_st->setCheckable(true);
    Palette_m->addSeparator();
    Palette_m->addAction(ac_dark_st);
    if (styid==1) ac_dark_st->setChecked(true);
    connect(ac_dark_st, SIGNAL(toggled(bool)), this, SLOT(StyleChanged(bool)));

    //identif_only_one = 1;
    for (int i = 0; i < 9; ++i) connect(rb_palette[i], SIGNAL(toggled(bool)), this, SLOT(PaletteChanged(bool)));//2.65

    Mode_m = new QMenu(tr("Mode"));

    allq65 = false;
    s_mode = 2; //HV important set to default mode fsk441
    QActionGroup *ac_gr_mod = new QActionGroup(this);
    for (int i = 0; i < COUNT_MODE; ++i)
    {
        if (i==7) rb_mode[i] = new QAction(ModeStr(i)+"   HF/50MHz",this);
        else if (i==8 || i==9) rb_mode[i] = new QAction(ModeStr(i)+"   VHF/UHF",this);
        else rb_mode[i] = new QAction(ModeStr(i),this);
        rb_mode[i]->setCheckable(true);
        ac_gr_mod->addAction(rb_mode[i]);
    }
    QMenu *MQ65 = new QMenu("Q65");
    MQ65->setIcon(QPixmap(":pic/smenu.png"));
    for (int i = 0; i < COUNT_MODE; ++i)
    {
        int p = pos_mods[i];
        if (p==14 || p==15  || p==16 || p==17)
        {
            MQ65->addAction(rb_mode[p]);
            if (p==17) Mode_m->addMenu(MQ65);
        }
        else Mode_m->addAction(rb_mode[p]);
        if (p==1  || p==3 || p==5 || p==6 || p==9 || p==12) Mode_m->addSeparator();
        connect(rb_mode[i], SIGNAL(toggled(bool)), this, SLOT(ModeChanged(bool)));
    }
    connect(W_mod_bt_sw,SIGNAL(clicked(int)),this,SLOT(ModBtSwClicked(int)));//2.74

    Band_m = new QMenu(tr("Band"));// HF/VHF 1.8-50MHz, VHF/UHF 70-2300MHz SHF/EHF 3.3-228G
    QMenu *B0_m = new QMenu("VLF      ");
    B0_m->setIcon(QPixmap(":pic/smenu.png"));
    Band_m->addMenu(B0_m);
    QMenu *B1_m = new QMenu("HF/VHF   ");
    B1_m->setIcon(QPixmap(":pic/smenu.png"));
    Band_m->addMenu(B1_m);
    QMenu *B2_m = new QMenu("VHF/UHF  ");
    B2_m->setIcon(QPixmap(":pic/smenu.png"));
    Band_m->addMenu(B2_m);
    QMenu *B3_m = new QMenu("SHF/EHF  ");
    B3_m->setIcon(QPixmap(":pic/smenu.png"));
    Band_m->addMenu(B3_m);
    QActionGroup *ac_gr_band = new QActionGroup(this);
    for (int i = 0; i<COUNT_BANDS; ++i)
    {
        QAction *rb_t = new QAction(lst_bands[i],this);
        rb_t->setCheckable(true);
        if (i==17) rb_t->setChecked(true); //2.76.5 17 default HV 70 MHz
        ac_gr_band->addAction(rb_t);
        ListBands.append(rb_t);
        connect(rb_t, SIGNAL(toggled(bool)), this, SLOT(BandChanged(bool)));
        if (i<3)       B0_m->addAction(rb_t);
        else if (i<16) B1_m->addAction(rb_t);
        else if (i<24) B2_m->addAction(rb_t);
        else B3_m->addAction(rb_t);
        //Band_m->addAction(rb_t);
    }
    BandChanged(true); //default set HV 70 MHz
    connect(W_band_bt_sw,SIGNAL(clicked(int)),this,SLOT(BandBtSwClicked(int)));//2.74

    connect(THvTxW, SIGNAL(EmitRigBandFromFreq(int)), this, SLOT(SetBandFromRigFreq(int)));//1.61= ListBands needed
    THvTxW->SendIdMshf();//2.76 at start ListBands needed

    THvHelpMs = new HvHelpMs(tr("Help"),APP_NAME,App_Path,lid,x,y);
    QMenu *Help_m = new QMenu(tr("Help"));

    Help_m->addAction(QPixmap(":pic/help.png"),ShKey[lid][0][1], THvHelpMs, SLOT(show()),QKeySequence(tr(ShKey[lid][0][0],ShKey[lid][0][1])));

    //THvHelpSkMs = new HvHelpSkMs(tr("Keyboard Shortcuts"),APP_NAME,lid,x,y); //2.65
    THvHelpSkMs = new HvHelpSkMs(ShKey[lid][1][1],APP_NAME,lid,x,y);
    Help_m->addAction(QPixmap(":pic/key.png"),ShKey[lid][1][1], THvHelpSkMs, SLOT(show()),QKeySequence(tr(ShKey[lid][1][0],ShKey[lid][1][1])));

    Help_m->addAction(QPixmap(":pic/spot_dx.png"),ShKey[lid][25][1], THvTxW, SLOT(StartEmptySpotDialog()),QKeySequence(tr(ShKey[lid][25][0],ShKey[lid][25][1])));

    Help_m->addAction(OCheckTime_m);

    QAction *BcnList = new QAction(QPixmap(":pic/bcn_list.png"),tr("Beacon List"),this);
    Help_m->addAction(BcnList);
    connect(BcnList, SIGNAL(triggered()), THvTxW, SLOT(ShowBcnList()));

    THvMsProc = new HvMsProc(tr("MS Procedures"),APP_NAME,App_Path,lid,x,y);
    QAction *A_MsProc;
    A_MsProc = new QAction(QPixmap(":pic/ms_proc.png"),tr("MS Procedures"),this);
    connect(A_MsProc, SIGNAL(triggered()), THvMsProc, SLOT(show()));
    Help_m->addAction(A_MsProc);

    HvAboutMsHv *THvAboutMsHv = new HvAboutMsHv(tr("About"),APP_NAME,App_Path,lid,this);
    QAction *Help_abaut;
    Help_abaut = new QAction(QPixmap(":pic/ms_ico.png"),tr("About")+" MSHV",this);
    connect(Help_abaut, SIGNAL(triggered()), THvAboutMsHv, SLOT(exec()));
    Help_m->addAction(Help_abaut);

    for (int i = 0; i < COUNT_MODE; ++i) decoder_depth_all[i] = 1;
    decoder_depth_all[0]  = 2;  //MSK144 and MSK40
    decoder_depth_all[11] = 3;  //ft8
    decoder_depth_all[12] = 2;  //MSK144ms
    decoder_depth_all[13] = 3;  //ft4
    decoder_depth_all[14] = 3;  //q65a
    decoder_depth_all[15] = 3;  //q65b
    decoder_depth_all[16] = 3;  //q65c
    decoder_depth_all[17] = 3;  //q65d
    decoder_depth_all[18] = 3;  //ft2

    QMenu *Decode_m = new QMenu(tr("Decode"));
    //Decode_m->setContentsMargins(2,0,2,0);

    TCpuWudget = new CpuWudget();
    TCpuWudget->setContentsMargins(0,2,0,0);// da se izrawni s Min_Menu

    cthr = TCpuWudget->GetThrCount();
    QMenu *ThrM = new QMenu("FT "+tr("Threads")); //2.69
    ThrM->setIcon(QPixmap(":pic/smenu.png"));
    rb_thr[0] = new QAction(tr("Only 1"),this);
    rb_thr[0]->setCheckable(true);
    rb_thr[1] = new QAction(tr("Max")+" 2",this);
    rb_thr[1]->setCheckable(true);
    rb_thr[2] = new QAction(tr("Max")+" 3",this);
    rb_thr[2]->setCheckable(true);
    rb_thr[3] = new QAction(tr("Max")+" 4",this);
    rb_thr[3]->setCheckable(true);
    rb_thr[4] = new QAction(tr("Max")+" 5",this);
    rb_thr[4]->setCheckable(true);
    rb_thr[5] = new QAction(tr("Max")+" 6",this);
    rb_thr[5]->setCheckable(true);
    QActionGroup *ac_gr_thr = new QActionGroup(this);
    for (int i = 0; i < 6; ++i) ac_gr_thr->addAction(rb_thr[i]);
    for (int i = 0; i < 6; ++i) ThrM->addAction(rb_thr[i]);
    for (int i = 0; i < 6; ++i) connect(rb_thr[i], SIGNAL(toggled(bool)), this, SLOT(SetThrLevel(bool)));
    for (int i = 0; i < COUNT_MODE; ++i) thr_all[i] = 1; //2.69
    rb_thr[0]->setChecked(true);
    if (cthr==2) rb_thr[1]->setText(tr("Max")+" 2 "+tr("Do Not Use If TXing"));
    if (cthr==3) rb_thr[2]->setText(tr("Max")+" 3 "+tr("Do Not Use If TXing"));
    if (cthr==4) rb_thr[3]->setText(tr("Max")+" 4 "+tr("Do Not Use If TXing"));
    if (cthr==5) rb_thr[4]->setText(tr("Max")+" 5 "+tr("Do Not Use If TXing"));
    if (cthr==6) rb_thr[5]->setText(tr("Max")+" 6 "+tr("Do Not Use If TXing"));
    RbThrSetEnabled(false);

    rb_dec_depth[0] = new QAction(tr("Fast")+" MSK FT Q65",this);// Q65 2.55
    rb_dec_depth[0]->setCheckable(true);
    rb_dec_depth[1] = new QAction(tr("Normal")+" MSK FT Q65",this);// Q65 2.55
    rb_dec_depth[1]->setCheckable(true);
    rb_dec_depth[2] = new QAction(tr("Deep")+" MSK FT Q65",this);// Q65 2.55
    rb_dec_depth[2]->setCheckable(true);
    for (int i = 0; i < 3; ++i) rb_dec_depth[i]->setEnabled(false);//HV important set to default mode fsk441
    QActionGroup *ac_gr_dec = new QActionGroup(this);
    for (int i = 0; i < 3; ++i) ac_gr_dec->addAction(rb_dec_depth[i]);

    //2.39 remm
    cb_3intFt_d = new QAction(tr("Use Three-stage Decoding")+" FT8",this);//Use Three Intervals For Decode FT8 (High Speed PCs)
    cb_3intFt_d->setCheckable(true);
    cb_3intFt_d->setEnabled(false);

    cb_UseVarDecFt = new QAction(tr("Use")+" SD Decoder FT8",this);
    cb_UseVarDecFt->setCheckable(true);
    cb_UseVarDecFt->setEnabled(false);
    MVDecFtPar = new QMenu(tr("Parameters")+" SD Decoder FT8"); //2.69
    ThrM->setIcon(QPixmap(":pic/smenu.png"));
    MVDecFtPar->setEnabled(false);
    QActionGroup *ac_gr_vdec_cyc = new QActionGroup(this);
    rb_vdec_cyc[0] = new QAction(tr("Decoder Cycles 1"),this);
    rb_vdec_cyc[0]->setCheckable(true);
    rb_vdec_cyc[1] = new QAction(tr("Decoder Cycles 2"),this);
    rb_vdec_cyc[1]->setCheckable(true);
    rb_vdec_cyc[2] = new QAction(tr("Decoder Cycles 3"),this);
    rb_vdec_cyc[2]->setCheckable(true);
    for (int i = 0; i < 3; ++i) ac_gr_vdec_cyc->addAction(rb_vdec_cyc[i]);
    QActionGroup *ac_gr_vdec_sens = new QActionGroup(this);
    rb_vdec_sens[0] = new QAction(tr("Sensitivity Minimum"),this);
    rb_vdec_sens[0]->setCheckable(true);
    rb_vdec_sens[1] = new QAction(tr("Sensitivity Use Low Thresholds"),this);
    rb_vdec_sens[1]->setCheckable(true);
    rb_vdec_sens[2] = new QAction(tr("Sensitivity Use Subpass"),this);
    rb_vdec_sens[2]->setCheckable(true);
    for (int i = 0; i < 3; ++i) ac_gr_vdec_sens->addAction(rb_vdec_sens[i]);


    cb_msk144rxequal_[0] = new QAction(tr("MSK RX Equalization Off"),this);
    cb_msk144rxequal_[0]->setCheckable(true);
    cb_msk144rxequal_[1] = new QAction(tr("MSK RX Equalization Static"),this);
    cb_msk144rxequal_[1]->setCheckable(true);
    cb_msk144rxequal_[2] = new QAction(tr("MSK RX Equalization Dynamic"),this);
    cb_msk144rxequal_[2]->setCheckable(true);
    cb_msk144rxequal_[3] = new QAction(tr("MSK RX Equalization S And D"),this);
    cb_msk144rxequal_[3]->setCheckable(true);
    for (int i = 0; i < 4; ++i) cb_msk144rxequal_[i]->setEnabled(false);
    QActionGroup *ac_gr_eq = new QActionGroup(this);
    for (int i = 0; i < 4; ++i) ac_gr_eq->addAction(cb_msk144rxequal_[i]);

    TAggressiveDialog = new AggressiveDialog(dsty,this);
    connect(TAggressiveDialog, SIGNAL(EmitLevelAggres(int)), TDecoderMs, SLOT(SetAggresLevFtd(int)));
    connect(TAggressiveDialog, SIGNAL(EmitLevelDeepS(int)), TDecoderMs, SLOT(SetAggresLevDeepS(int)));

    ac_aggressive = new QAction(QPixmap(":pic/slid_agres.png"),tr("Aggressive Levels")+" JT65",this);
    connect(ac_aggressive, SIGNAL(triggered()), TAggressiveDialog, SLOT(exec()));

    /*vhf_uhf_decode_fac_all[0] = false;  //MSK144 and MSK40
    vhf_uhf_decode_fac_all[1] = false;  //no JTMS
    vhf_uhf_decode_fac_all[2] = false;  //no FSK441
    vhf_uhf_decode_fac_all[3] = false;  //no FSK314
    vhf_uhf_decode_fac_all[4] = false;  //no ISCAT-A
    vhf_uhf_decode_fac_all[5] = false;  //no ISCAT-B
    vhf_uhf_decode_fac_all[6] = false;  //no JT6M
    vhf_uhf_decode_fac_all[7] = true;   //no jt56a
    vhf_uhf_decode_fac_all[8] = true;   //no jt56b
    vhf_uhf_decode_fac_all[9] = true;   //no jt56c
    vhf_uhf_decode_fac_all[10] = false; //no pi4
    vhf_uhf_decode_fac_all[11] = false; //ft8
    vhf_uhf_decode_fac_all[12] = false; //MSK144ms
    vhf_uhf_decode_fac_all[13] = false; //ft4
    vhf_uhf_decode_fac_all[13] = false; //q65*/
    for (int i=0; i<COUNT_MODE; ++i)
    {
        if (i>6 && i<10) vhf_uhf_decode_fac_all[i] = true; // jt65
        else vhf_uhf_decode_fac_all[i] = false;
    }

    //cb_vhf_uhf_decode_fac = new QAction("Enable VHF/UHF Features JT65",this);
    cb_vhf_uhf_decode_fac = new QAction(tr("Check for VHF/UHF Uncheck for HF Features")+" JT65",this);
    cb_vhf_uhf_decode_fac->setCheckable(true);
    cb_vhf_uhf_decode_fac->setChecked(false);//1.60= def fsk441


    for (int i = 0; i < COUNT_MODE; ++i) avg_dec_all[i] = false;
    avg_dec_all[8]  = true;  //no jt56b
    avg_dec_all[9]  = true;  //no jt56c
    avg_dec_all[10] = true;  //no pi4 fiktive all time on
    avg_dec_all[18] = true;  //ft2 default ON=true in Decoder init need to be =false OK

    cb_avg_decode = new QAction(tr("Enable Averaging")+" JT65 Q65 FT2",this);// Q65 2.55
    cb_avg_decode->setCheckable(true);
    cb_avg_decode->setChecked(false);

    cb_auto_clr_avg_afdec = new QAction(tr("Auto Clear Averaging After Decode")+" Q65",this);
    cb_auto_clr_avg_afdec->setCheckable(true);
    cb_auto_clr_avg_afdec->setChecked(false);
    //cb_auto_clr_avg_afdec->setVisible(false);// Q65 2.55

    for (int i=0; i<COUNT_MODE; ++i) deep_search_dec_all[i] = false;

    cb_deep_search_decode = new QAction(tr("Enable Deep Search")+" JT65",this);
    cb_deep_search_decode->setCheckable(true);
    cb_deep_search_decode->setChecked(false);

    for (int i = 0; i < COUNT_MODE; ++i) decoder_ap_all[i] = false;
    decoder_ap_all[11] = true;  //ft8
    decoder_ap_all[13] = true;  //ft4
    decoder_ap_all[14] = true;  //q65a ???
    decoder_ap_all[15] = true;  //q65b ???
    decoder_ap_all[16] = true;  //q65c ???
    decoder_ap_all[17] = true;  //q65d ???
    decoder_ap_all[18] = true;  //ft2

    cb_ap_decode = new QAction(tr("Enable AP")+" FT JT65 Q65",this);// Q65 2.55

    cb_ap_decode->setCheckable(true);
    cb_ap_decode->setChecked(false);//default
    cb_ap_decode->setEnabled(false);


    for (int i=0; i<COUNT_MODE; ++i)
    {
        max65_cand_all[i] = 1;
    }

    cb_1_dec_sig_q65 = new QAction(tr("Single Decoded Signal")+" Q65",this);
    cb_1_dec_sig_q65->setCheckable(true);
    cb_1_dec_sig_q65->setEnabled(false);
    connect(cb_1_dec_sig_q65, SIGNAL(toggled(bool)), TDecoderMs, SLOT(SetSingleDecQ65(bool)));
    //cb_1_dec_sig_q65->setVisible(false);// Q65 2.55

    cb_max_drift = new QAction(tr("Use Drift Correction")+" +/- 100 Hz Q65",this);//2.57  +/- 100 Hz
    cb_max_drift->setCheckable(true);
    cb_max_drift->setEnabled(false);
    connect(cb_max_drift, SIGNAL(toggled(bool)), TDecoderMs, SLOT(SetMaxDrift(bool)));
    //cb_max_drift->setVisible(false);// Q65 2.57

    cb_dec_aft_eme_delay = new QAction(tr("Decode After EME Delay")+" Q65",this);
    cb_dec_aft_eme_delay->setCheckable(true);
    cb_dec_aft_eme_delay->setEnabled(false);
    connect(cb_dec_aft_eme_delay, SIGNAL(toggled(bool)), TDecoderMs, SLOT(SetDecAftEMEDelay(bool)));
    //cb_dec_aft_eme_delay->setVisible(false);// Q65 2.55

    cb_max65_cand[0] = new QAction(tr("Single Decoded Signal")+" JT65",this);
    cb_max65_cand[0]->setCheckable(true);
    cb_max65_cand[1] = new QAction(tr("Max 4 Decoded Signals")+" JT65",this);
    cb_max65_cand[1]->setCheckable(true);
    cb_max65_cand[2] = new QAction(tr("Max 8 Decoded Signals")+" JT65",this);
    cb_max65_cand[2]->setCheckable(true);
    cb_max65_cand[3] = new QAction(tr("Max 16 Decoded Signals")+" JT65",this);
    cb_max65_cand[3]->setCheckable(true);
    cb_max65_cand[4] = new QAction(tr("Max 32 Decoded Signals")+" JT65",this);
    cb_max65_cand[4]->setCheckable(true);
    QActionGroup *ac_gr_c65 = new QActionGroup(this);
    for (int i = 0; i < 5; ++i) ac_gr_c65->addAction(cb_max65_cand[i]);
    for (int i = 0; i < 5; ++i) cb_max65_cand[i]->setEnabled(false);
    for (int i = 0; i < 5; ++i) connect(cb_max65_cand[i], SIGNAL(toggled(bool)), this, SLOT(SetMaxCandidats65(bool)));
    cb_max65_cand[1]->setChecked(true);

    Decode_m->addMenu(ThrM);
    Decode_m->addSeparator();
    for (int i = 0; i < 3; ++i) Decode_m->addAction(rb_dec_depth[i]);
    Decode_m->addAction(cb_3intFt_d);//2.39 remm
    Decode_m->addSeparator();

    Decode_m->addAction(cb_UseVarDecFt);
    Decode_m->addMenu(MVDecFtPar);
    for (int i = 0; i < 3; ++i) MVDecFtPar->addAction(rb_vdec_cyc[i]);
    MVDecFtPar->addSeparator();
    for (int i = 0; i < 3; ++i) MVDecFtPar->addAction(rb_vdec_sens[i]);

    Decode_m->addSeparator();
    for (int i = 0; i < 4; ++i) Decode_m->addAction(cb_msk144rxequal_[i]);
    Decode_m->addSeparator();
    Decode_m->addAction(cb_1_dec_sig_q65);
    Decode_m->addAction(cb_max_drift);
    for (int i = 0; i < 5; ++i) Decode_m->addAction(cb_max65_cand[i]);
    Decode_m->addSeparator();
    Decode_m->addAction(ac_aggressive);
    Decode_m->addAction(cb_vhf_uhf_decode_fac);
    Decode_m->addAction(cb_avg_decode);
    Decode_m->addAction(cb_auto_clr_avg_afdec);
    Decode_m->addAction(cb_deep_search_decode);
    Decode_m->addAction(cb_dec_aft_eme_delay);
    Decode_m->addSeparator();
    Decode_m->addAction(cb_ap_decode);

    for (int i = 0; i < 3; ++i) connect(rb_dec_depth[i], SIGNAL(toggled(bool)), this, SLOT(DecodeDeept(bool)));
    rb_dec_depth[0]->setChecked(true);//fast dec=0
    connect(cb_3intFt_d, SIGNAL(toggled(bool)), MainDisplay, SLOT(Decode3intFt(bool)));//2.39 remm
    connect(cb_3intFt_d, SIGNAL(toggled(bool)), TDecoderMs,  SLOT(Decode3intFt(bool)));//2.39 remm
    cb_3intFt_d->setChecked(true);//2.51  default

    connect(cb_UseVarDecFt,SIGNAL(toggled(bool)),this,SLOT(VarDecodeFtPar(bool)));
    for (int i = 0; i < 3; ++i) connect(rb_vdec_cyc[i],SIGNAL(toggled(bool)),this,SLOT(VarDecodeFtPar(bool)));
    for (int i = 0; i < 2; ++i) connect(rb_vdec_sens[i],SIGNAL(toggled(bool)),this,SLOT(VarDecodeFtPar(bool)));
    rb_vdec_cyc[1]->setChecked(true);
    rb_vdec_sens[1]->setChecked(true);

    for (int i = 0; i < 4; ++i) connect(cb_msk144rxequal_[i], SIGNAL(toggled(bool)), this, SLOT(Msk144RxEqual(bool)));
    cb_msk144rxequal_[0]->setChecked(true);//default Equalization msk144 and decoderms.cpp

    connect(cb_vhf_uhf_decode_fac, SIGNAL(toggled(bool)), this, SLOT(SetVhfUhfFeatures(bool)));//1.60=

    connect(cb_avg_decode, SIGNAL(toggled(bool)), this, SLOT(AvgDecodeChanged(bool)));
    connect(cb_auto_clr_avg_afdec, SIGNAL(toggled(bool)), TDecoderMs, SLOT(AutoClrAvgChanged(bool)));
    connect(cb_deep_search_decode, SIGNAL(toggled(bool)), this, SLOT(DeepSearchChanged(bool)));// 1.49 deep search

    //connect(cb_ap_decode, SIGNAL(toggled(bool)), TDecoderMs, SLOT(SetApDecode(bool)));
    connect(cb_ap_decode, SIGNAL(toggled(bool)), this, SLOT(DecodeAp(bool)));

    Min_Menu->addMenu(File_m);
    Min_Menu->addMenu(Option_m);
    Min_Menu->addMenu(Palette_m);
    Min_Menu->addMenu(Mode_m);
    Min_Menu->addMenu(Decode_m);
    Min_Menu->addMenu(Band_m);
    //// Translation ////
    Min_Menu->addMenu(lang_m);
    //// end Translation ////
    Min_Menu->addMenu(Help_m);

    Min_Menu->setContentsMargins(4,0,4,0);//4,0,4,0 ( qreal left, qreal top, qreal right, qreal bottom )
    //Min_Menu->setFixedHeight(23);//1.56=off for +150%
    /*QLabel *lsuperfox = new QLabel("S-u-p-e-r.F-o-x");
    lsuperfox->setContentsMargins(10,0,0,0);
    lsuperfox->setStyleSheet("QLabel{font-weight:bold;color:rgb(255,0,0);}");*/
    QHBoxLayout *H_l = new QHBoxLayout();
    H_l->setContentsMargins(0,0,0,0);
    H_l->addWidget(Min_Menu);
    H_l->setAlignment(Min_Menu,Qt::AlignLeft);
    //H_l->addWidget(lsuperfox);
    //H_l->setAlignment(lsuperfox,Qt::AlignCenter);
    H_l->addWidget(TCpuWudget);
    H_l->setAlignment(TCpuWudget,Qt::AlignRight | Qt::AlignHCenter);
    //H_l->setAlignment(TPicW,Qt::AlignHCenter);
    //H_l->addWidget(THvSMeter_H);

    int but_height=20;//21
    //QFont b_font = font();
    //b_font.setPointSize(8);
    QPushButton *pb_stop_rx = new QPushButton(tr("STOP MONITOR"));
    pb_stop_rx->setFixedHeight(but_height);
    //pb_stop_rx->setFont(b_font);
    connect(pb_stop_rx, SIGNAL(clicked(bool)), this, SLOT(StopRxGlobal()));
    pb_start_rx = new QPushButton(tr("MONITOR"));
    pb_start_rx->setStyleSheet("QPushButton{background-color:palette(Button);}");
    pb_start_rx->setFixedHeight(but_height);
    //pb_start_rx->setFont(b_font);
    connect(pb_start_rx, SIGNAL(clicked(bool)), this, SLOT(StartRxGlobal()));

    QPushButton *pb_stop_tx = new QPushButton(tr("STOP TX"));
    pb_stop_tx->setFixedHeight(but_height);
    connect(pb_stop_tx, SIGNAL(clicked(bool)), this, SLOT(StopTxGlobal()));

    pb_clar_list1 = new QPushButton(tr("CLEAR MESSAGES"));
    pb_clar_list1->setFixedHeight(but_height);
    //pb_clar_list->setFont(b_font);
    connect(pb_clar_list1, SIGNAL(clicked(bool)), TDecodeList1, SLOT(Clear_List()));

    //pb_clar_list2 = new QPushButton(" CLEAR RX MESSAGES ");
    pb_clar_list2 = new QPushButton(tr("CLR RX FREQ MSG"));
    pb_clar_list2->setFixedHeight(but_height);
    connect(pb_clar_list2, SIGNAL(clicked(bool)), TDecodeList2, SLOT(Clear_List()));

    QPushButton *pb_rst_qso = new QPushButton(tr("RESET QSO"));
    pb_rst_qso->setFixedHeight(but_height);
    //pb_rst_qso->setFont(b_font);
    connect(pb_rst_qso, SIGNAL(clicked(bool)), THvTxW, SLOT(ResetQSO()));

    pb_tune = new QPushButton(tr("TUNE"));
    pb_tune->setStyleSheet("QPushButton{background-color:palette(Button);}");
    pb_tune->setFixedSize(65,but_height);
    //pb_tune->setFont(b_font);
    connect(pb_tune, SIGNAL(clicked(bool)), this, SLOT(Tune()));
    f_tune = false;

    //connect(FontDialog, SIGNAL(EmitFontApp(QFont)), this, SLOT(SetFont(QFont)));

    pb_clear_avg65 = new QPushButton(tr("CLEAR AVG")+" 0/0 | 0/0");
    pb_clear_avg65->setFixedHeight(but_height);
    pb_clear_avg65->setHidden(true);
    connect(pb_clear_avg65, SIGNAL(clicked(bool)), TDecoderMs, SLOT(SetClearAvg65()));

    pb_dec_65 = new QPushButton(tr("DECODE")+" NA");//1.49
    pb_dec_65->setFixedHeight(but_height);
    pb_dec_65->setHidden(true);
    pb_dec_65->setEnabled(false);
    //pb_dec_65->setFixedWidth(40);

    connect(MainDisplay,SIGNAL(EmitIsDispHaveDataForDec65(int,bool)),this,SLOT(IsDispHaveDataForDec65(int,bool)));//1.49
    connect(SecondDisplay,SIGNAL(EmitIsDispHaveDataForDec65(int,bool)),this,SLOT(IsDispHaveDataForDec65(int,bool)));//1.49
    connect(pb_dec_65,SIGNAL(clicked(bool)),this,SLOT(SetButtonDecodeAll65()));//1.49

    connect(pb_rst_qso, SIGNAL(clicked(bool)), TDecoderMs, SLOT(SetClearAvg65()));//1.36
    connect(pb_clar_list1, SIGNAL(clicked(bool)), TDecoderMs, SLOT(SetClearAvg65()));//1.37

    connect(TDecoderMs, SIGNAL(EmitAvgSaves(int,int,int,int)), this, SLOT(SetAvg65CountToButtonTxt(int,int,int,int)));
    //connect(MainDisplay, SIGNAL(EmitVDRxFreq(double)), TDecoderMs, SLOT(SetRxFreq65(double)));

    pb_clear_avgPi4 = new QPushButton(tr("CLEAR AVG")+" 0/0");
    pb_clear_avgPi4->setFixedHeight(but_height);
    pb_clear_avgPi4->setHidden(true);
    connect(pb_clear_avgPi4, SIGNAL(clicked(bool)), TDecoderMs, SLOT(SetClearAvgPi4()));
    connect(TDecoderMs, SIGNAL(EmitAvgSavesPi4(int,int)), this, SLOT(SetAvgPi4CountToButtonTxt(int,int)));

    pb_clear_avgQ65 = new QPushButton(tr("CLEAR AVG")+" 0 | 0");
    pb_clear_avgQ65->setFixedHeight(but_height);
    pb_clear_avgQ65->setHidden(true);
    connect(pb_clear_avgQ65, SIGNAL(clicked(bool)), TDecoderMs, SLOT(SetClearAvgQ65()));
    connect(TDecoderMs, SIGNAL(EmitAvgSavesQ65(int,int)), this, SLOT(SetAvgQ65CountToButtonTxt(int,int)));

    //QPushButton *pb_fltr = new QPushButton(tr("FLTR IS OFF"));
    FilterDialog->pb_fltrOnOff->setFixedHeight(but_height);
    //FilterDialog->SetHidFLBtOnOff(true);//FilterDialog->pb_fltrOnOff->setHidden(true);

    QHBoxLayout *H_butons = new QHBoxLayout();
    H_butons->setContentsMargins(0,0,0,0);
    H_butons->addWidget(pb_start_rx);
    H_butons->addWidget(pb_stop_rx);
    H_butons->addWidget(pb_clar_list1);
    H_butons->addWidget(pb_clar_list2);
    H_butons->addWidget(pb_clear_avg65);
    H_butons->addWidget(pb_clear_avgQ65);
    H_butons->addWidget(pb_clear_avgPi4);
    H_butons->addWidget(pb_dec_65);
    H_butons->addWidget(pb_rst_qso);
    H_butons->addWidget(FilterDialog->pb_fltrOnOff);
    H_butons->addWidget(pb_stop_tx);
    H_butons->addWidget(pb_tune);

    connect(THvTxW, SIGNAL(EmitMessageS(QString,bool,bool)), this, SLOT(TxMessageS(QString,bool,bool)));
    connect(THvTxW, SIGNAL(EmitAuto()), this, SLOT(SetAuto()));

    //connect(THvMakros, SIGNAL(EmitRptRsq(bool)), THvTxW, SLOT(SetRptRsq(bool)));
    connect(THvTxW, SIGNAL(EmitMacros(int,QString)),this,SLOT(SetMacros(int,QString)));
    //connect(THvMakros, SIGNAL(EmitMacros(QStringList,int,QString,QString)),
    //this, SLOT(SetMacros(QStringList,int,QString,QString)));//2.32
    //connect(THvMakros, SIGNAL(EmitDistUnit(bool)), THvTxW, SLOT(SetDistUnit(bool)));

    connect(THvTxW, SIGNAL(EmitFileNameChenged()), this, SLOT(FileNameChengedD1()));
    connect(THvTxW, SIGNAL(EmitFileNameChenged()), this, SLOT(FileNameChengedD2()));

    connect(MainDisplay, SIGNAL(EmitVDRxFreqF0F1(double,double,double)), TDecoderMs, SLOT(SetRxFreqF0F1(double,double,double)));
    connect(THvTxW, SIGNAL(EmitDfSdbChanged(int,int)), TDecoderMs, SLOT(SetDfSdb(int,int)));
    connect(THvTxW, SIGNAL(EmitDfChanged(int,int)), MainDisplay, SLOT(SetVDdf(int,int)));
    connect(MainDisplay, SIGNAL(EmitVDRxDf(int)), THvTxW, SLOT(SetRxDf(int)));

    //connect(THvTxW, SIGNAL(EmitShOptChenged(bool)), this, SLOT(SetShOpt(bool)));
    connect(THvTxW, SIGNAL(EmitShOptChenged(bool)), TDecoderMs, SLOT(SetShOpt(bool)));
    connect(THvTxW, SIGNAL(EmitSwlOptChenged(bool)), TDecoderMs, SLOT(SetSwlOpt(bool)));

    connect(TDecodeList1, SIGNAL(ListSelectedTextAll(QString,QString,QString,QString,QString)),
            THvTxW, SLOT(DecListTextAll(QString,QString,QString,QString,QString)));
    connect(TDecodeList1, SIGNAL(ListSelectedRpt(QString)), THvTxW, SLOT(DecListTextRpt(QString)));
    connect(TDecodeList2, SIGNAL(ListSelectedTextAll(QString,QString,QString,QString,QString)),
            THvTxW, SLOT(DecListTextAll(QString,QString,QString,QString,QString)));
    connect(TDecodeList2, SIGNAL(ListSelectedRpt(QString)), THvTxW, SLOT(DecListTextRpt(QString)));
    connect(ac_2click_list_autu_on, SIGNAL(toggled(bool)), THvTxW, SLOT(Set2ClickDecListAutoOn(bool)));
    ac_2click_list_autu_on->setChecked(true);//2.51
    connect(ac_start_qso_from_tx2_or_tx1, SIGNAL(toggled(bool)), THvTxW, SLOT(SetStartFromTx2Tx1(bool)));
    connect(ac_new_dec_clr_msg_list, SIGNAL(toggled(bool)), TDecodeList1, SLOT(SetNewDecClrMsgListFlag(bool)));
    ac_new_dec_clr_msg_list->setChecked(false);
    connect(ac_use_queue_cont, SIGNAL(toggled(bool)), THvTxW, SLOT(SetUseQueueCont(bool)));//2.59

    //for format message
    connect(THvTxW, SIGNAL(EmitWords(QStringList,int,int)), TDecoderMs, SLOT(SetWords(QStringList,int,int)));
    connect(Test_tones_m, SIGNAL(triggered()), THvTxW, SLOT(GenTestTones()));
    connect(THvTxW, SIGNAL(EmitZap(bool)), TDecoderMs, SLOT(SetZap(bool)));
    connect(THvTxW, SIGNAL(EmitZap(bool)), MainDisplay, SLOT(SetZap(bool)));
    connect(THvTxW, SIGNAL(EmitZap(bool)), SecondDisplay, SLOT(SetZap(bool)));

    s_msg = "";
    //s_gen = false;
    //s_imidi = false;
    THvTxW->GetCurrentMsg();
    THvTxW->SetMacrosFirstStart();//THvMakros->SetMacros();
    THvTxW->ReadEDILog();//2.57 after SetMacros for may locator
    //"RT Dec"
    cb_auto_decode_all = new QCheckBox(tr("Auto Dec"));//Auto Decode
    cb_auto_decode_all->setFixedWidth(85);// tested 125%
    //cb_auto_decode_all->setStyleSheet("QCheckBox::indicator{width:13px; height:13px; }");
    connect(cb_auto_decode_all, SIGNAL(toggled(bool)), this, SLOT(CbSetAutoDecodeAll(bool)));
    //connect(cb_auto_decode_all, SIGNAL(toggled(bool)), SecondDisplay, SLOT(SetAutoDecodeAll(bool)));
    cb_auto_decode_all->setChecked(auto_decode_all[2]);// fsk441

    cb_rtd_decode = new QCheckBox(tr("RT Dec"));//Real-time Decode
    cb_rtd_decode->setFixedWidth(75);//tested 125%
    connect(cb_rtd_decode, SIGNAL(toggled(bool)), this, SLOT(CbSetStartStopRtd(bool)));
    cb_rtd_decode->setChecked(true);
    cb_rtd_decode->setEnabled(false);

    l_mode = new QLabel(ModeStr(s_mode)); //inportent fsk441
    //if (dsty) l_mode->setStyleSheet("QLabel {color: rgb(0, 0, 0)}");
    //l_mode->setStyleSheet(ModeColorStr(s_mode));
    if (dsty) l_mode->setStyleSheet(ModeColorStr(s_mode)+"color:rgb(0,0,0)}");
    else l_mode->setStyleSheet(ModeColorStr(s_mode)+"}");

    l_mode->setAlignment(Qt::AlignCenter);
    l_mode->setFixedSize(60,20);//1.30 from 70x20 +125%
    l_mode->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    QHBoxLayout *H_status = new QHBoxLayout();
    H_status->setContentsMargins ( 4, 0, 4, 0);
    H_status->setSpacing(4);
    H_status->addWidget(l_mode);
    H_status->addWidget(cb_auto_decode_all);
    H_status->addWidget(cb_rtd_decode);
    H_status->addWidget(TPicW);
    //H_status->setAlignment(TPicW,Qt::AlignLeft);
    H_status->addWidget(l_tx_text);
    //H_status->setAlignment(l_tx_text,Qt::AlignHCenter);
    H_status->addWidget(THvSMeter_H);
    //H_centr->setAlignment(THvTxW, Qt::AlignRight);

    // this->setMinimumWidth(MainDisplay->width()+Slider_Tune_Disp->width()+2);

    pb_save_disp1 = new QPushButton(tr("SAVE DISPLAY")+" 1");
    //pb_save_disp1->setFixedWidth(150);
    pb_save_disp1->setFixedHeight(19);
    //pb_save_disp1->setFont(b_font);
    //pb_save_disp1->setStyleSheet("background-color:rgb(236, 233, 216);");
    pb_save_disp1->setStyleSheet("QPushButton{background-color:palette(Button);}");
    connect(pb_save_disp1, SIGNAL(clicked(bool)), this, SLOT(SaveFileDisplay1()));
    pb_save_disp2 = new QPushButton(tr("SAVE DISPLAY")+" 2");
    //pb_save_disp2->setFixedWidth(150);
    pb_save_disp2->setFixedHeight(19);
    //pb_save_disp2->setFont(b_font);
    //pb_save_disp2->setStyleSheet("background-color:rgb(236, 233, 216);");
    pb_save_disp2->setStyleSheet("QPushButton{background-color:palette(Button);}");
    connect(pb_save_disp2, SIGNAL(clicked(bool)), this, SLOT(SaveFileDisplay2()));

    cb_flat_dsp = new QCheckBox("FD");
    cb_flat_dsp->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
    cb_flat_dsp->setFixedHeight(17);
    connect(cb_flat_dsp, SIGNAL(toggled(bool)), this, SLOT(SetFlatDisplay_VD(bool)));
    cb_flat_dsp->setChecked(false);
    cb_flat_dsp->setToolTip(tr("Flatten Display"));

    //cb_flat_dsp->setStyleSheet("QCheckBox {color: white; background-color: rgb(80,80,80); selection-color: white; selection-background-color: rgb(80,80,80);}"
    //"QToolTip { color: #000000; background-color: #ffffff; }");
    if (dsty) cb_flat_dsp->setStyleSheet("QCheckBox {color: white; background-color: rgb(80,80,80);selection-color: white; selection-background-color: rgb(80,80,80);}"
                                             " QToolTip {color: #000000; background-color: #ffffe1;}");
    else cb_flat_dsp->setStyleSheet("QCheckBox {background-color: white; selection-color: black; selection-background-color: white;}"
                                        " QToolTip {color: #000000; background-color: #ffffe1;}");

    cb_adle_dsp = new QCheckBox("AF");
    cb_adle_dsp->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
    cb_adle_dsp->setFixedHeight(17);
    connect(cb_adle_dsp, SIGNAL(toggled(bool)), this, SLOT(SetAdleDisplay_VD(bool)));
    cb_adle_dsp->setChecked(true);//2.51 default
    cb_adle_dsp->setToolTip(tr("Auto Flatten Display"));// Auto Adjust Brightnes and Contrast \nIs Not Recommended
    if (dsty) cb_adle_dsp->setStyleSheet("QCheckBox {color: white; background-color: rgb(80,80,80); selection-color: white; selection-background-color: rgb(80,80,80);}"
                                             " QToolTip {color: #000000; background-color: #ffffe1;}");
    else cb_adle_dsp->setStyleSheet("QCheckBox {background-color: white; selection-color: black; selection-background-color: white;}"
                                        " QToolTip {color: #000000; background-color: #ffffe1;}");

    for (int i = 0; i < COUNT_MODE; ++i) s_vdisp_all_speed[i] = 8;
    s_vdisp_all_speed[11] = 9;//5; //ft8 fictive
    s_vdisp_all_speed[13] = 9;//5; //ft4 fictive
    s_vdisp_all_speed[18] = 9;//5; //ft2 fictive

    SB_VDispSpeed = new HvSpinBox();
    SB_VDispSpeed->setRange(1,9);//1.51 1,9//1,5
    SB_VDispSpeed->setValue(s_vdisp_all_speed[s_mode]);
    SB_VDispSpeed->setFixedHeight(19);//problem hv
    //SB_VDispSpeed->setFixedWidth(79);//125% problem hv
    SB_VDispSpeed->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
    SB_VDispSpeed->setPrefix(tr("Speed")+" ");
    //SB_VDispSpeed->setSuffix("  Hz");
    SB_VDispSpeed->findChild<QLineEdit*>()->setReadOnly(true);
    SB_VDispSpeed->setContextMenuPolicy(Qt::NoContextMenu);
    if (dsty) SB_VDispSpeed->setStyleSheet("QSpinBox{background-color:rgb(80,80,80);selection-color:white;selection-background-color:rgb(80,80,80);}");
    else SB_VDispSpeed->setStyleSheet("QSpinBox{background-color:white;selection-color:black;selection-background-color:white;}");

    SB_VDispStartFreq = new HvSpinBox();
    SB_VDispStartFreq->setRange(0,3000);//s3000+b2000=5000 max
    SB_VDispStartFreq->setValue(100);//old 200
    SB_VDispStartFreq->setFixedHeight(19);//problem hv
    //SB_VDispStartFreq->setFixedWidth(113);//125% problem hv
    SB_VDispStartFreq->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
    SB_VDispStartFreq->setPrefix(tr("Start")+" ");
    SB_VDispStartFreq->setSuffix(" Hz");
    SB_VDispStartFreq->findChild<QLineEdit*>()->setReadOnly(true);
    SB_VDispStartFreq->setContextMenuPolicy(Qt::NoContextMenu);
    if (dsty) SB_VDispStartFreq->setStyleSheet("QSpinBox{background-color:rgb(80,80,80);selection-color:white;selection-background-color:rgb(80,80,80);}");
    else SB_VDispStartFreq->setStyleSheet("QSpinBox{background-color:white;selection-color:black;selection-background-color:white;}");
    SB_VDispStartFreq->setSingleStep(100);

    SB_VDispBandwidth = new HvSpinBox();
    SB_VDispBandwidth->setRange(2000,5000);//s3000+b2000=5000 max
    SB_VDispBandwidth->setValue(3200);//old 2000
    SB_VDispBandwidth->setFixedHeight(19);//problem hv
    //SB_VDispBandwidth->setFixedWidth(107);//125% problem hv
    SB_VDispBandwidth->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
    SB_VDispBandwidth->setPrefix("BW ");
    SB_VDispBandwidth->setSuffix(" Hz");
    SB_VDispBandwidth->findChild<QLineEdit*>()->setReadOnly(true);
    SB_VDispBandwidth->setContextMenuPolicy(Qt::NoContextMenu);
    if (dsty) SB_VDispBandwidth->setStyleSheet("QSpinBox{background-color:rgb(80,80,80);selection-color:white;selection-background-color:rgb(80,80,80);}");
    else SB_VDispBandwidth->setStyleSheet("QSpinBox{background-color:white;selection-color:black;selection-background-color:white;}");
    SB_VDispBandwidth->setSingleStep(100);

    QHBoxLayout *H_bsave = new QHBoxLayout();
    H_bsave->setContentsMargins ( 0, 0, 0, 0);
    H_bsave->setSpacing(0);//1.51 0
    H_bsave->addWidget(SB_VDispSpeed);
    H_bsave->addWidget(SB_VDispStartFreq);
    H_bsave->addWidget(SB_VDispBandwidth);
    H_bsave->addWidget(cb_flat_dsp);
    H_bsave->addWidget(cb_adle_dsp);
    H_bsave->addWidget(pb_save_disp1);
    H_bsave->addWidget(pb_save_disp2);
    SB_VDispSpeed->setHidden(true);
    SB_VDispStartFreq->setHidden(true);
    SB_VDispBandwidth->setHidden(true);
    cb_flat_dsp->setHidden(true);
    cb_adle_dsp->setHidden(true);

    connect(SB_VDispStartFreq, SIGNAL(valueChanged(int)), this, SLOT(VDispStartBandChanged(int)));
    connect(SB_VDispBandwidth, SIGNAL(valueChanged(int)), this, SLOT(VDispStartBandChanged(int)));

    //connect(SB_VDispSpeed, SIGNAL(valueChanged(int)), TMsCore, SLOT(SetVDispSpeed(int)));
    connect(SB_VDispSpeed, SIGNAL(valueChanged(int)), this, SLOT(SetVDispSpeed(int)));

    QVBoxLayout *V_disp = new QVBoxLayout();
    V_disp->setContentsMargins ( 0, 0, 0, 0);
    V_disp->setSpacing(0);

    V_disp->addWidget(MainDisplay);
    V_disp->addWidget(SecondDisplay);
    V_disp->addLayout(H_bsave);
    //V_disp->setAlignment(SecondDisplay, Qt::AlignHCenter | Qt::AlignTop);

    QFont f_t = font();
    f_t.setPointSize(7);
    QString strr = "TUNE DISP";//"TUNE DISPLAYS";//TUNE DISPLAYS      TUNE DSPLs  TUNE DISP
    QVBoxLayout *V_btns_txt_sldr = new QVBoxLayout();
    V_btns_txt_sldr->setContentsMargins ( 0, 0, 0, 0);
    V_btns_txt_sldr->setSpacing(0);

    //// 2/1 Display //////////
    QPixmap pixmap1(":pic/2d_release.png");    //1d_release.png
    QPixmap pixmap2(":pic/2d_press.png");      //1d_press.png
    QPixmap pixmap3(":pic/1d_release.png");    //2d_release.png
    QPixmap pixmap4(":pic/1d_press.png");      //2d_press.png
    pb_2D_1D = new HvButton_Left4();
    pb_2D_1D->SetupButton_hv(pixmap1, pixmap2, pixmap3, pixmap4, 0, 0);
    connect(pb_2D_1D, SIGNAL(Release_Lift_Button_hv()), this, SLOT(BtSet2D1D()));
    V_btns_txt_sldr->addWidget(pb_2D_1D);
    pb_2D_1D->setToolTip(tr("Change Waterfall Size"));//Change Display Size //Switches number of displays 2D/1D
    pb_2D_1D->setStyleSheet("QToolTip {color: #000000; background-color: #ffffe1;}");

    QPixmap pixmap5(":pic/d1_release.png");    //1d_release.png
    QPixmap pixmap6(":pic/d1_press.png");      //1d_press.png
    QPixmap pixmap7(":pic/d2_release.png");    //2d_release.png
    QPixmap pixmap8(":pic/d2_press.png");      //2d_press.png
    pb_D1_D2 = new HvButton_Left4();
    pb_D1_D2->SetupButton_hv(pixmap5, pixmap6, pixmap7, pixmap8, 0, 0);
    connect(pb_D1_D2, SIGNAL(Release_Lift_Button_hv()), this, SLOT(BtSetD1D2()));
    pb_D1_D2->setHidden(true);
    ////END 2/1 Display //////////

    QVBoxLayout *V_txt_ = new QVBoxLayout();
    V_txt_->setContentsMargins(0,0,0,0);
    V_txt_->setSpacing(0);
    for (int i = 0; i<strr.count(); i++)
    {
        QLabel *l_ct = new QLabel((QString)"<font color=white>"+strr.at(i));
        l_ct->setFont(f_t);
        txt_tunedsp.append(l_ct);
        V_txt_->addWidget(txt_tunedsp.at(i));
        V_txt_->setAlignment(txt_tunedsp.at(i),Qt::AlignCenter);// po horizontal
    }

    connect(FontDialog, SIGNAL(EmitFontApp(QFont)), TDecodeList1, SLOT(SetFontHeader(QFont)));
    connect(FontDialog, SIGNAL(EmitFontApp(QFont)), TDecodeList2, SLOT(SetFontHeader(QFont)));
    connect(FontDialog, SIGNAL(EmitFontApp(QFont)), THvTxW, SLOT(SetFont(QFont)));
    connect(FontDialog, SIGNAL(EmitFontApp(QFont)), this, SLOT(SetFont(QFont)));
    FontDialog->SetDefFont();

    //V_txt_sldr->addWidget(Slider_Tune_Disp);
    //V_txt_sldr->addWidget(Slider_Cont_Disp);
    V_txt_->setAlignment(Qt::AlignVCenter);
    QVBoxLayout *V_sldr_ = new QVBoxLayout();
    V_sldr_->setContentsMargins ( 0, 0, 0, 0);
    V_sldr_->setSpacing(0);
    V_sldr_->addWidget(Slider_Tune_Disp);
    V_sldr_->addWidget(Slider_Cont_Disp);
    V_sldr_->setAlignment(Qt::AlignVCenter);
    //V_sld_->setAlignment(Qt::AlignBottom);

    V_btns_txt_sldr->addLayout(V_txt_);
    V_btns_txt_sldr->addLayout(V_sldr_);

    //V_sldr->setAlignment(Slider_Tune_Disp,Qt::AlignVCenter);
    V_btns_txt_sldr->addWidget(pb_D1_D2);
    //V_sldr->setAlignment(pb_D1_D2,Qt::AlignBottom);
    //V_btns_txt_sldr->setAlignment(Qt::AlignJustify);

    QHBoxLayout *H_disp_btn_txt_sld = new QHBoxLayout();
    H_disp_btn_txt_sld->setContentsMargins ( 0, 0, 0, 0);
    H_disp_btn_txt_sld->setSpacing(0);

    //QDockWidget *DW = new QDockWidget("Display");
    //QWidget *Box_dspl = new QWidget();
    Box_dspl = new QWidget();
    //if (dsty) Box_dspl->setStyleSheet("background-color:rgb(30,30,30)");
    Box_dspl->setStyleSheet("background-color:black;");
    //Box_dspl->setFrameShape(QFrame::WinPanel);
    //Box_dspl->setFrameShadow(QFrame::Raised);
    Box_dspl->setLayout(H_disp_btn_txt_sld);
    Box_dspl->setContentsMargins(0,0,0,0);

    H_disp_btn_txt_sld->addLayout(V_disp);
    //H_disp_sld->setAlignment(V_disp, Qt::AlignRight);
    H_disp_btn_txt_sld->addLayout(V_btns_txt_sldr);
    //H_disp_sld->setAlignment(Slider_Tune_Disp, Qt::AlignLeft);
    //H_disp_sld->setAlignment(Qt::AlignHCenter);
    //DW->setWidget(Box_dspl);
    //DW->setContentsMargins(0,0,0,0);
    //DW->setFixedHeight(160);

    QHBoxLayout *H_dlist = new QHBoxLayout();
    H_dlist->setContentsMargins ( 0, 0, 0, 0);
    H_dlist->setSpacing(0);
    H_dlist->addWidget(TDecodeList1);
    H_dlist->addWidget(TDecodeList2);

    QVBoxLayout *V_l = new QVBoxLayout(this);
    setLayout(V_l);
    V_l->setContentsMargins(1,0,1,0);// ( qreal left, qreal top, qreal right, qreal bottom )
    V_l->setSpacing(0);
    V_l->addLayout(H_l);
    //V_l->addWidget(DW);
    V_l->addWidget(Box_dspl);
    V_l->addLayout(H_status);
    V_l->addWidget(W_mod_bt_sw);
    V_l->addWidget(W_band_bt_sw);
    V_l->addLayout(H_dlist);
    V_l->addLayout(H_butons);
    V_l->addWidget(THvTxW);

    connect(sh_wf, SIGNAL(toggled(bool)), this, SLOT(SetShowHideWf(bool)));
    connect(sh_tx, SIGNAL(toggled(bool)), this, SLOT(SetShowHideTx(bool)));

    /*offset_hour = 0;
    offset_min = 0;
    offset_t = (offset_hour*3600)+(60*offset_min);*/
    connect(TMsCore, SIGNAL(Refresh_time()), this, SLOT(Refresh()));

    period_time_sec = 30.0;

    time_pos_1period = 0;
    time_pos_2period = 0;
    fast_find_period = false;
    connect(THvTxW, SIGNAL(EmitReriodTime(float)), this, SLOT(SetPeriodTime(float)));

    connect(THvTxW, SIGNAL(StndOutLevel(int)), TMsPlayerHV, SLOT(SetVolume(int)));
    THvTxW->SetInLevel("50");//corect sliders from 100 to 50%
    THvTxW->SetOutLevel("95");//corect sliders from 100 to 95%

    f_is_moved_to_prev_desk_pos = false;

    Read_Settings(App_Path+"/settings/ms_settings");

    //2.76sf for remove
    //if(g_ub_m_k3) THvTxW->setVisibleSf(true);
    //else THvTxW->setVisibleSf(false);
    //end 2.76sf for remove
    /*if(g_ub_m_k3)// Q65 2.57 for drift
    {
    	//Mode_m->insertMenu(rb_mode_jt65a,MQ65);   
    //cb_1_dec_sig_q65->setVisible(true);
    //cb_auto_clr_avg_afdec->setVisible(true);
    //cb_dec_aft_eme_delay->setVisible(true);
    //cb_max_drift->setVisible(true);  
    }*/

    TAllTxt = new AllTxt(App_Path);
    QDateTime utc_t = getDateTime();
    TAllTxt->ReadAllTxt(utc_t.toString("yyyy_MM"));
    connect(TDecodeList1, SIGNAL(EmitRxAllTxt(QString)), this, SLOT(SetRxAllTxt(QString)));

    if (!f_is_moved_to_prev_desk_pos) move(x,y); //2.63

    setAcceptDrops(true);
    f_decoder_busy = false;
    f_tx_busy = false;
    f_de_active = false;//special flag not same as f_decoder_busy
    //qDebug()<<"W_MAIN START";
}
Main_Ms::~Main_Ms()
{
    //// Translation ////
    SaveSS();
    //// end Translation ////
    THvTxW->StopAuto();
    TMsPlayerHV->Stop();
    StopRxGlobal();
    TMsCore->close_sound();
    SaveSettings();//Save_Settings(App_Path+"/settings/ms_settings");
    SetRigTxRx(false);
    THvRigControl->CatStopPttIfClose();//2.38
    usleep(120000);//2.57 =120000 HDSDR=50ms ft991a=10ms
    THvTxW->SetBlockEmitFreqToRig(true); //2.69 no show MA freq restrict QMessageBox at close (DestroyPort())
    THvRigControl->DestroyPort();
}
void Main_Ms::StyleChanged(bool)
{
    QString slang = tr("To change the Style, you need to MANUALLY RESTART MSHV");
    QMessageBox::information(this,"MSHV",slang,QMessageBox::Close);
}
//// Translation ////
void Main_Ms::LangChanged(bool)
{
    static bool one = false;
    if (one)
    {
        one = false;
        return;
    }
    one = true;
 
    const QString slang[COUNT_LANGS] =
        {
    		"To change the Language, you need to MANUALLY RESTART MSHV",
    		"За да промените Езика, трябва РЪЧНО ДА РЕСТАРТИРАТЕ MSHV",
    		"Для смены Языка, необходимо ВРУЧНУЮ ПЕРЕЗАПУСК MSHV",
    		"要更改语言, 您需要手动重新启动 MSHV",
    		"要更改語言, 您需要手動重新啟動 MSHV",
    		"Para cambiar la Lengua, has de REINICIAR MSHV MANUALMENTE",			//eses
    		"Per canviar l'idioma, has de REINICIAR MSHV MANUALMENT",				//caes
    		"Para alterar o idioma, terá que REINICIAR MANUALMENTE o MSHV",			//ptpt
    		"Pentru a schimba limba, trebuie să reporniți manual programul MSHV",	//roro 
    		"For at skifte sprog skal MSHV genstartes MANUELT",						//dadk
    		"Język zostanie zmieniony po RESTARCIE PROGRAMU",						//plpl
    		"Pour changer la langue, vous devez REDEMARRER MANUELLEMENT MSHV !",	//frfr
    		"Para mudar o idioma, é necessário reiniciar manualmente o MSHV",		//ptbr
    		"For å endre språket må du starte MSHV manuelt på nytt",				//nbno
    		"Per cambiare il Linguaggio, devi fare un RESTART MANUALE DI MSHV",		//itit
    		"Pro změnu jazyka musíte restartovat MSHV"								//cscz
    		//"Pro změnu jazyka musíte restartovat MSHV"							//elgr
        };
    int z = 0;
    for (int i = 0; i < COUNT_LANGS; ++i)
    {
        if (ac_l[i]->isChecked())
        {
            z = i;
            break;
        }
    }
    QMessageBox::information(this,"MSHV",slang[z],QMessageBox::Close);
}
void Main_Ms::SaveSS()
{
    QString langid = "0";
    for (int i = 0; i < COUNT_LANGS; ++i)
    {
        if (ac_l[i]->isChecked())
        {
            langid = QString("%1").arg(i);
            break;
        }
    }
    QFile file(App_Path+"/settings/ms_start");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return;
    QTextStream out(&file);
    out << "def_lang="+langid << "\n";
    out << "def_style=" << QString("%1").arg(ac_dark_st->isChecked()) << "\n";
    file.close();
}
//// end Translation ////
void Main_Ms::SetShowHideWf(bool f)
{
    if (f) Box_dspl->setHidden(false);
    else Box_dspl->setHidden(true);
    TDecodeList1->HideShowWfTxRefreshList();
    TDecodeList2->HideShowWfTxRefreshList();
}
void Main_Ms::SetShowHideTx(bool f)
{
    if (f) THvTxW->setHidden(false);
    else THvTxW->setHidden(true);
    TDecodeList1->HideShowWfTxRefreshList();
    TDecodeList2->HideShowWfTxRefreshList();
}
void Main_Ms::SetFlatDisplay_VD(bool f)
{
    if (f && cb_adle_dsp->isChecked()) cb_adle_dsp->setChecked(false);
    MainDisplay->SetFlatDisplay_VD(f);
}
void Main_Ms::SetAdleDisplay_VD(bool f)
{
    if (f && cb_flat_dsp->isChecked()) cb_flat_dsp->setChecked(false);
    MainDisplay->SetAdleDisplay_VD(f);
}
void Main_Ms::SetThrLevel(bool)
{
    //static int p_rb_id = -1;
    for (int i = 0; i < 6; ++i)
    {
        if (rb_thr[i]->isChecked())
        {
            //if (p_rb_id == i) break;
            //p_rb_id = i;
            TDecoderMs->SetThrLevel(i+1); //qDebug()<<"Mode="<<s_mode<<"Threads="<<i+1;
            thr_all[s_mode] = i+1;
            break;
        }
    }
}
void Main_Ms::SetDLogQso(bool)
{
    if (Direct_log_qso->isChecked()) Prompt_log_qso->setChecked(false);
    THvTxW->SetDPLogQso(Direct_log_qso->isChecked(),Prompt_log_qso->isChecked());
}
void Main_Ms::SetPLogQso(bool)
{
    if (Prompt_log_qso->isChecked()) Direct_log_qso->setChecked(false);
    THvTxW->SetDPLogQso(Direct_log_qso->isChecked(),Prompt_log_qso->isChecked());
}
void Main_Ms::SetStaticTxFrq(bool f,int val) //from rig control
{
    s_static_tx_frq = val; //qDebug()<<"1= rig contr"<<s_mode<<f<<val;
    f_static_tx = f;
    SetTxFreq_p();
}
void Main_Ms::SetTxFreq(double val) //from main disp
{
    //qDebug()<<"1 in="<<s_mode<<val;
    s_v_disp_tx_frq = val; //qDebug()<<"2= from main disp"<<s_mode<<val;
    SetTxFreq_p();
}
void Main_Ms::SetTxFreq_p() //tx freq from display and static
{
    static double prv_f0 = -1.0;
    if (s_mode==11 || s_mode==13 || s_mode==18 || allq65) //modes with static and variable TX freq
    {
        double f = s_v_disp_tx_frq;
        if (f_static_tx) f = (double)s_static_tx_frq;
        if (prv_f0 != f)
        {
            prv_f0 = f;
            TMsPlayerHV->SetTxFreq(f); //qDebug()<<"Main_TMsPlayerHV->SetTxFreq="<<s_mode<<f_static_tx<<f;
        }
    }
}
void Main_Ms::RefreshWindowTitle()
{
    if (ListBands.count() < COUNT_BANDS) return; //2.76 protection ListBands needed
    QString band; //qDebug()<<"Main_TMsPlayerHV->SetTxFreq=================="<<s_mode;
    for (int i = 0; i<COUNT_BANDS; ++i)
    {
        if (ListBands.at(i)->isChecked())
        {
            band = lst_bands[i];
            break;
        }
    }
    QString str_t = APP_NAME + InstName;
    str_t.insert(4," "+band);
    bool jt65emecon = false;//2.74
    if ((s_mode==7 || s_mode==8 || s_mode==9) && s_contest_name =="ARRL Inter. EME Contest") jt65emecon = true;
    if (s_mode==0 || s_mode==11 || s_mode==13 || s_mode==18 || allq65 || jt65emecon)
    {
        QString str_t2 = "";
        if (s_contest_name != "None" && s_contest_name != "EU RSQ And Serial Number") str_t2 = "  - "+s_contest_name+s_trmN+" -";
        if (s_mode==11 || s_mode==13 || s_mode==18 || allq65)
        {
            if (Multi_answer_mod->isChecked()) str_t2 = "  - "+tr("MA DXpedition")+" -";
            else if (Multi_answer_mod_std->isChecked() && str_t2.isEmpty()) str_t2 = "  - "+tr("MA Standard")+" -";
        }
        if (id_mshf==2 && s_mode==11) str_t2.append(" SF");
        if (id_mshf==1 && s_mode==11) str_t2.append(" SH");
        str_t.append(str_t2);
    }
    setWindowTitle(str_t);
}
void Main_Ms::RefreshCbCfm73()
{
    if ((s_mode==11 || s_mode==13 || s_mode==18 || allq65) && !Multi_answer_mod->isChecked()) ac_Cfm73->setEnabled(true);
    else ac_Cfm73->setEnabled(false);
}
void Main_Ms::RefreshStartQsoTX2orTx1()//2.76.1
{
    if (s_mode==12 || s_mode==0 || s_mode==11 || s_mode==13 || s_mode==18 || allq65)
    {
        if (!f_is_myCstd || id_mshf==1)
        {
            ac_start_qso_from_tx2_or_tx1->setEnabled(false);
            ac_start_qso_from_tx2_or_tx1->setChecked(false);
        }
        else ac_start_qso_from_tx2_or_tx1->setEnabled(true);
    }
    else ac_start_qso_from_tx2_or_tx1->setEnabled(false);
}
void Main_Ms::SetMshf(uint8_t id)//2.76
{
    id_mshf = id;//0=0,0 1=1,0 2=0,1
    //printf("mode=%d id_mshf=%d\n",s_mode,id_mshf);
    if (id_mshf == 2) THvRigControl->SetMsf(true);
    else  THvRigControl->SetMsf(false);
    MainDisplay->SetMsh(id_mshf);//for disp BW rx tx
    TDecoderMs->SetMsh(id_mshf);
    if (id_mshf == 2) TMsPlayerHV->SetMsf(true);
    else  TMsPlayerHV->SetMsf(false);
    RefreshWindowTitle();
    RefreshStartQsoTX2orTx1();//2.76.1 //if (id_mshf==1 && ac_start_qso_from_tx2_or_tx1->isChecked()) ac_start_qso_from_tx2_or_tx1->setChecked(false);
}
void Main_Ms::SetMultiAnswerMod(bool f)
{
    if (f) Multi_answer_mod_std->setChecked(false);
    if (Multi_answer_mod->isChecked()) TDecoderMs->SetMultiAnswerMod(true);
    else TDecoderMs->SetMultiAnswerMod(false);
    THvTxW->SetMultiAnswerMod(Multi_answer_mod->isChecked(),Multi_answer_mod_std->isChecked());
    RefreshWindowTitle();
    RefreshCbCfm73();
}
void Main_Ms::SetMultiAnswerModStd(bool f)
{
    if (f) Multi_answer_mod->setChecked(false);
    if (Multi_answer_mod->isChecked()) TDecoderMs->SetMultiAnswerMod(true);
    else TDecoderMs->SetMultiAnswerMod(false);
    THvTxW->SetMultiAnswerMod(Multi_answer_mod->isChecked(),Multi_answer_mod_std->isChecked());
    RefreshWindowTitle();
    RefreshCbCfm73();
}
void Main_Ms::SetMacros(int contest_id,QString trmN_stdC)//2.15
{
    if (contest_id == 0)//0 standard
    {
        g_block_mam = false;
        if (allq65 || s_mode==11 || s_mode==13 || s_mode==18)//2.74 if (!g_block_mam)
        {
            MA_man_adding->setEnabled(true);
            Multi_answer_mod->setEnabled(true);
            Multi_answer_mod_std->setEnabled(true);
        }
    }
    else//for future if(contest_id == 2 || contest_id == 3 || contest_id == 4 || contest_id == 5)
    {
        if (!g_ub_m_k)
        {
            g_block_mam = true;
            MA_man_adding->setEnabled(false);
            Multi_answer_mod->setEnabled(false);
            Multi_answer_mod->setChecked(false);
            Multi_answer_mod_std->setEnabled(false);
            Multi_answer_mod_std->setChecked(false);
        }
    }//THvTxW->SetMacros(l,contest_id,s,trmN);
    s_contest_name = s_cont_name[contest_id];
    s_trmN = "";
    if (trmN_stdC.at(0)=='1') s_trmN = ", Run 1"; //2.61
    if (trmN_stdC.at(0)=='2') s_trmN = ", Run 2";
    if (trmN_stdC.at(1)=='0') f_is_myCstd = false;
    else f_is_myCstd = true;
    RefreshStartQsoTX2orTx1();//2.76.1
    /*if (s_mode==12 || s_mode==0 || s_mode==11 || s_mode==13 || s_mode==18 || allq65)
    {
    	if (trmN_stdC.at(1)=='0')
    	{
    		ac_start_qso_from_tx2_or_tx1->setEnabled(false);
    		ac_start_qso_from_tx2_or_tx1->setChecked(false);    
    	}
    	else ac_start_qso_from_tx2_or_tx1->setEnabled(true);
    }		
    else ac_start_qso_from_tx2_or_tx1->setEnabled(false);*/
    RefreshWindowTitle();
    TDecodeList1->SetActivityId(trmN_stdC.mid(2,trmN_stdC.count()-2));
    TDecodeList2->SetActivityId(trmN_stdC.mid(2,trmN_stdC.count()-2));
}
void Main_Ms::SetFont(QFont f)
{
    QFont t_font = f;
    t_font.setPointSize(f.pointSize()-1);//8
    pb_save_disp1->setFont(t_font);
    pb_save_disp2->setFont(t_font);
    pb_start_rx->setFont(f);
    pb_tune->setFont(f);

    cb_flat_dsp->setFont(f);
    cb_adle_dsp->setFont(f);
    SB_VDispSpeed->setFont(f);
    SB_VDispStartFreq->setFont(f);
    SB_VDispBandwidth->setFont(f);

    t_font.setPointSize(7);//7
    for (int i = 0; i<txt_tunedsp.count(); i++)
        txt_tunedsp.at(i)->setFont(t_font);

    TPicW->SetFont(f);
    l_mode->setFont(f);
    t_font.setPointSize(f.pointSize()+1);//tx_font.setPointSize(10);
    t_font.setBold(true);
    l_tx_text->setFont(t_font);

    QFontMetrics fm(f);
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)//2.56
    int fcor = fm.horizontalAdvance("ISCAT-B")+8;
#else
    int fcor = fm.width("ISCAT-B")+8;
#endif
    l_mode->setFixedWidth(fcor);
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0) //2.56
    fcor = fm.horizontalAdvance("Auto Dec")+28;// to150% +28<-cb
#else
    fcor = fm.width("Auto Dec")+28;//horizontalAdvance to150% +28<-cb
#endif
    cb_auto_decode_all->setFixedWidth(fcor);
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0) //2.56
    fcor = fm.horizontalAdvance("RT Dec")+28;// to150% +28<-cb
#else
    fcor = fm.width("RT Dec")+28;//horizontalAdvance to150% +28<-cb
#endif
    cb_rtd_decode->setFixedWidth(fcor);

    MainDisplay->SetFont(f);
    SecondDisplay->SetFont(f);
    //THvMakros->SetFont(f);
    TAggressiveDialog->SetFont(f);
    THvTxtColor->SetFont(f);
    //THvRigControl->SetFont(f);
    FilterDialog->SetFont(f);
    W_mod_bt_sw->SetFont(f);
    W_band_bt_sw->SetFont(f);
}
void Main_Ms::setTuneDsp(int,int)//1.55=
{
    int offset = Slider_Tune_Disp->get_value();
    int contr  = Slider_Cont_Disp->get_value();
    MainDisplay->setTune(offset,contr);
    SecondDisplay->setTune(offset,contr);
}
void Main_Ms::IsDispHaveDataForDec65(int diden_a, bool fss_a)//1.49
{
    if (diden_a == 0)
        f_is_d1_data_todec65 = fss_a;
    if (diden_a == 1)
        f_is_d2_data_todec65 = fss_a;

    if (f_is_d1_data_todec65)
    {
        pb_dec_65->setText(tr("DECODE")+" D1");
        pb_dec_65->setEnabled(true);
    }
    else if (f_is_d2_data_todec65)
    {
        pb_dec_65->setText(tr("DECODE")+" D2");
        pb_dec_65->setEnabled(true);
    }
    else
    {
        pb_dec_65->setText(tr("DECODE")+" NA");
        pb_dec_65->setEnabled(false);
    }
}
void Main_Ms::SetButtonDecodeAll65()//1.49
{
    if (pb_dec_65->text()==tr("DECODE")+" D1")
        MainDisplay->SetButtonDecodeAll65(0);//1.49 inportent need to main dsp to drow pos
    if (pb_dec_65->text()==tr("DECODE")+" D2")
        MainDisplay->SetButtonDecodeAll65(1);//1.49 inportent need to main dsp to drow pos
    //SecondDisplay->SetButtonDecodeAll65();
}
#define G_UL_K_72 0xb0285
void Main_Ms::SetVDispSpeed(int val)
{
    //1.52 stop exeption pazi go save nestva s promiana na defaulta s_vdisp_all_speed[mod] to max=9
    //tova ne stava -> all no Vdisplay(msk144 to jt6m) only fom (jt65 to ft8) set spead
    /*qDebug()<<"1"<<val<<s_mode;
       if (s_mode<=6)
           return;
       qDebug()<<"2"<<val<<s_mode;*/

    s_vdisp_all_speed[s_mode] = val;//save
    TMsCore->SetVDispSpeed(val);
    //SB_VDispSpeed->setValue(s_vdisp_all_speed[s_mode]);
}
QString Main_Ms::get_settings_allI(int ident)
{
    // 0=vdist_speads 1=decoder_depth_all 2=max_cand65
    QString s;
    for (int i =0; i<COUNT_MODE; i++)
    {
        s.append(ModeStr(i)+"=");

        if (ident==0) s.append(QString("%1").arg(s_vdisp_all_speed[i]));
        else if (ident==1) s.append(QString("%1").arg(decoder_depth_all[i]));
        else if (ident==2) s.append(QString("%1").arg(max65_cand_all[i]));
        if (i<COUNT_MODE-1) s.append("#");
    }
    return s;
}
void Main_Ms::SetAllSettingsI(QString s, int ident)
{
    QStringList ls=s.split("#"); // 0=vdist_speads 1=decoder_depth_all 2=max_cand65
    for (int i=0; i<COUNT_MODE; i++)
    {
        QString tstr = ModeStr(i)+"=";
        for (int j=0; j<ls.count(); j++)
        {
            if (ls[j].contains(tstr))
            {
                ls[j].remove(tstr);

                if (ident==0)
                {
                    if (!ls[j].isEmpty() && (ls[j].toInt()>0 && ls[j].toInt()<10))//from 1 do 9
                        s_vdisp_all_speed[i] = ls[j].toInt();
                }
                else if (ident==1)
                {
                    if (!ls[j].isEmpty() && (ls[j].toInt()>0 && ls[j].toInt()<4))// from 1 do 3
                        decoder_depth_all[i] = ls[j].toInt();
                }
                else if (ident==2)
                {
                    if (!ls[j].isEmpty() && (ls[j].toInt()>-1 && ls[j].toInt()<5))//1.51 10,  from 0 do 4
                        max65_cand_all[i] = ls[j].toInt();
                }
                break;
            }
        }
    }
}
void Main_Ms::SetMaxCandidats65Mod(int in_mod)
{
    int val = max65_cand_all[in_mod];
    for (int i = 0; i < 5; ++i)
    {
        if (val==i)
        {
            cb_max65_cand[i]->setChecked(true);
            break;
        }
    }
}
void Main_Ms::SetMaxCandidats65(bool)
{
    int val = 0;
    for (int i = 0; i < 5; ++i)
    {
        if (cb_max65_cand[i]->isChecked())
        {
            TDecoderMs->SetMaxCandidats65(i); //qDebug()<<i;
            val = i;
            break;
        }
    }
    max65_cand_all[s_mode] = val;
}
#define G_UL_K_73 0xafe28
void Main_Ms::ShowCloseAstroW(bool f)
{
    if (f)
    {
        THvTxW->ShowAstroW();//1.58=stop this for ub9ocf
        if (!g_block_from_close_app_is_active_astro_w) is_active_astro_w = true;
    }
    else
    {
        THvTxW->CloseAstroW();//1.58=stop this for ub9ocf
        if (!g_block_from_close_app_is_active_astro_w) is_active_astro_w = false;
    }
}
void Main_Ms::SetAstroWIsClosed()
{
    Start_astro_m->setChecked(false);
}
void Main_Ms::SetAvg65CountToButtonTxt(int iu1,int ia1,int iu2,int ia2)
{
    pb_clear_avg65->setText(tr("CLEAR AVG")+" "+QString("%1").arg(iu1)+"/"+QString("%1").arg(ia1)
                            +" | "+QString("%1").arg(iu2)+"/"+QString("%1").arg(ia2));
}
void Main_Ms::SetAvgPi4CountToButtonTxt(int iu1,int ia1)
{
    pb_clear_avgPi4->setText(tr("CLEAR AVG")+" "+QString("%1").arg(iu1)+"/"+QString("%1").arg(ia1));
}
void Main_Ms::SetAvgQ65CountToButtonTxt(int iu1,int ia1)
{
    pb_clear_avgQ65->setText(tr("CLEAR AVG")+" "+QString("%1").arg(iu1)+" | "+QString("%1").arg(ia1));
}
void Main_Ms::Msk144RxEqual(bool)
{
    for (int i = 0; i < 4; ++i)
    {
        if (cb_msk144rxequal_[i]->isChecked())
        {
            TDecoderMs->SetMsk144RxEqual(i); //qDebug()<<i;
            break;
        }
    }
}
void Main_Ms::VDispStartBandChanged(int)
{
    int start = SB_VDispStartFreq->value();
    int band = SB_VDispBandwidth->value();

    //SB_VDispStartFreq->setRange(0,3000);//s3000+b2000=5000 max
    //SB_VDispBandwidth->setRange(2000,5000);//s3000+b2000=5000 max
    if (start+band>5000)
    {
        //SB_VDispStartFreq->setValue(start-100);
        SB_VDispStartFreq->setValue(start-((start+band)-5000));
        //SB_VDispStartFreq->setValue(band-5000);
        return;
    }

    MainDisplay->setVDispFreqScale(start,start+band);//bool <- change start stop and df
    TMsCore->setVDFftwStartStopFreq(start,start+band);
    //qDebug()<<"Start="<<start<<"Stop="<<start+band<<"Bandwidth"<<band;
}
#define G_UL_K_74 0x58a
void Main_Ms::SetDispVH(bool f)
{
    //qDebug()<<"d0="<<height();
    //return;
    f_disp_v_h = f;

    if (f_disp_v_h==last_f_disp_v_h)
        return;
    last_f_disp_v_h=f_disp_v_h;

    if (f_disp_v_h)
    {// vertical disp
        if (!pb_D1_D2->Button_Stop_b)//only if on one display
            pb_D1_D2->ExtrnalRelease(); //qDebug()<<"F11";

        if (pb_2D_1D->Button_Stop_b)
            MainDisplay->SetVDisplayHight(2);
        else
            MainDisplay->SetVDisplayHight(1);

        SecondDisplay->setHidden(true);
        pb_save_disp2->setHidden(false);
        pb_D1_D2->setHidden(true);

        SB_VDispSpeed->setHidden(false);
        SB_VDispStartFreq->setHidden(false);
        SB_VDispBandwidth->setHidden(false);
        cb_flat_dsp->setHidden(false);
        cb_adle_dsp->setHidden(false);
        SetBS1Text("");
        SetBS2Text("");
    }
    else
    {// horizontal disp
        SB_VDispSpeed->setHidden(true);
        SB_VDispStartFreq->setHidden(true);
        SB_VDispBandwidth->setHidden(true);
        cb_flat_dsp->setHidden(true);
        cb_adle_dsp->setHidden(true);

        MainDisplay->SetVDisplayHight(1);

        if (pb_2D_1D->Button_Stop_b)
        {
            SecondDisplay->setHidden(false);
            pb_save_disp2->setHidden(false);
        }
        else
        {
            pb_save_disp2->setHidden(true);
            pb_D1_D2->setHidden(false);
        }
        SetBS1Text("");
        SetBS2Text("");
        /*if (global_start_moni)
        {
            QDateTime utc_t = getDateTime();
            MainDisplay->SetSyncPosition(time_pos_1period,getHHMin(),utc_t.toString("yyMMdd"));
        }*/
    }
}
void Main_Ms::BtSet2D1D()
{
    TDecodeList1->StaticIgnoreResizeEvent(true);//stop scrolling
    //TDecodeList2->IgnoreResizeEvent(true);
    if (pb_2D_1D->Button_Stop_b)
    {//two disp ili dvoen display  horizontal/vartical
        int hai = height();
        MainDisplay->setHidden(false);
        pb_save_disp1->setHidden(false);

        if (f_disp_v_h)
            MainDisplay->SetVDisplayHight(2);
        else
        {
            SecondDisplay->setHidden(false);
            pb_save_disp2->setHidden(false);
        }

        for (int i = 0; i<txt_tunedsp.count(); i++)
            txt_tunedsp.at(i)->setHidden(false);
        pb_D1_D2->setHidden(true);

        if (!pb_D1_D2->Button_Stop_b)//korect if last is second disp
            pb_D1_D2->Release_Button_hv();

        //hai += (SecondDisplay->height() + pb_save_disp2->height());
        hai += SecondDisplay->height();
        resize(width(), hai);
    }
    else
    { //one disp  horizontal/vartical
        int hai = height();
        SecondDisplay->setHidden(true);

        if (!f_disp_v_h)
            pb_save_disp2->setHidden(true);

        for (int i = 0; i<txt_tunedsp.count(); i++)
            txt_tunedsp.at(i)->setHidden(true);

        if (f_disp_v_h)//1-3 vazno da e tuk ina4e nne se resaizva pravilno
        {
            MainDisplay->SetVDisplayHight(1);
            //pb_D1_D2->setHidden(true);
        }

        pb_D1_D2->setHidden(false);//2-3 vazno da e tuk ina4e nne se resaizva pravilno

        //hai -= (SecondDisplay->height() + pb_save_disp2->height());
        hai -= SecondDisplay->height();
        resize(width(), hai);

        if (f_disp_v_h) //3-3 vazno da e tuk ina4e nne se resaizva pravilno
            pb_D1_D2->setHidden(true);
    }
    TDecodeList1->StaticIgnoreResizeEvent(false);//start scrolling
    //TDecodeList2->IgnoreResizeEvent(false);//start scrolling
}
void Main_Ms::BtSetD1D2()
{
    if (pb_D1_D2->Button_Stop_b)
    {
        // inportent in this order hv
        SecondDisplay->setHidden(true);
        MainDisplay->setHidden(false);
        pb_save_disp2->setHidden(true);
        pb_save_disp1->setHidden(false);
    }
    else
    {
        // inportent in this order hv
        MainDisplay->setHidden(true);
        SecondDisplay->setHidden(false);
        pb_save_disp1->setHidden(true);
        pb_save_disp2->setHidden(false);
    }
    //TDecodeList->scrollToBottom();
}
QString Main_Ms::get_dec_settings_allB(int ident)
{
    QString s;//0=vhf_hf 1=avg 2=deeps 3=ap 4=set_two_dec_lists 5=auto_dec_all
    for (int i =0; i<COUNT_MODE; i++)
    {
        s.append(ModeStr(i)+"=");
        if 		(ident==0) s.append(QString("%1").arg(vhf_uhf_decode_fac_all[i]));
        else if (ident==1) s.append(QString("%1").arg(avg_dec_all[i]));
        else if (ident==2) s.append(QString("%1").arg(deep_search_dec_all[i]));
        else if (ident==3) s.append(QString("%1").arg(decoder_ap_all[i]));
        else if (ident==4) s.append(QString("%1").arg(two_dec_list_all[i]));
        else if (ident==5) s.append(QString("%1").arg(auto_decode_all[i]));
        if (i<COUNT_MODE-1) s.append("#");
    }
    return s;
}
void Main_Ms::SetDecodeAllSettingsB(QString s,int ident)
{
    QStringList ls=s.split("#");//0=vhf_hf 1=avg 2=deeps 3=ap 4=set_two_dec_lists 5=auto_dec_all
    for (int i=0; i<COUNT_MODE; i++)
    {
        QString tstr = ModeStr(i)+"=";
        for (int j=0; j<ls.count(); j++)
        {
            if (ls[j].contains(tstr))
            {
                ls[j].remove(tstr);
                if (!ls[j].isEmpty() && (ls[j].toInt()>-1 && ls[j].toInt()<2))// from 1 do 3
                {
                    if 		(ident==0) vhf_uhf_decode_fac_all[i] = (bool)ls[j].toInt();
                    else if (ident==1) avg_dec_all[i] = (bool)ls[j].toInt();
                    else if (ident==2) deep_search_dec_all[i] = (bool)ls[j].toInt();
                    else if (ident==3) decoder_ap_all[i] = (bool)ls[j].toInt();
                    else if (ident==4) two_dec_list_all[i] = (bool)ls[j].toInt();
                    else if (ident==5) auto_decode_all[i] = (bool)ls[j].toInt();
                }
                break;
            }
        }
    }
}
void Main_Ms::AvgDecodeChanged(bool f)//activate from cb_vhf_uhf_decode_fac->setChecked
{
    avg_dec_all[s_mode] = f;//save
    TDecoderMs->AvgDecodeChanged(f);
}
void Main_Ms::DeepSearchChanged(bool f)//activate from cb_vhf_uhf_decode_fac->setChecked
{
    deep_search_dec_all[s_mode] = f;//save
    TDecoderMs->DeepSearchChanged(f);
}
void Main_Ms::SetVhfUhfFeatures(bool f)//activate from cb_vhf_uhf_decode_fac->setChecked
{
    vhf_uhf_decode_fac_all[s_mode] = f;//save
    TDecoderMs->SetVhfUhfFeatures(f);
    THvTxW->SetVhfUhfFeatures(f);
}
void Main_Ms::DecodeAp(bool f)//activate from cb_ap_decode->setChecked
{
    decoder_ap_all[s_mode] = f;//save
    TDecoderMs->SetApDecode(f);
}
void Main_Ms::SetDecodeDeeptFromMod(int md)
{
    int val = decoder_depth_all[md]; //1-fast 2-normal 3-deep
    for (int i = 0; i < 3; ++i)
    {
        if (val==i+1)
        {
            rb_dec_depth[i]->setChecked(true);
            break;
        }
    }
}
void Main_Ms::DecodeDeept(bool)
{
    int val = 1; //1-fast 2-normal 3-deep
    for (int i = 0; i < 3; ++i)
    {
        if (rb_dec_depth[i]->isChecked())
        {
            val = i+1;
            break;
        }
    }
    decoder_depth_all[s_mode] = val;//save
    TDecoderMs->SetDecoderDeep(val); //qDebug()<<val;
}
void Main_Ms::VarDecodeFtPar(bool)
{
    bool udec = cb_UseVarDecFt->isChecked();
    int tcyc = 2; //1-senc 2-senc 3-senc
    for (int i = 0; i < 3; ++i)
    {
        if (rb_vdec_cyc[i]->isChecked())
        {
            tcyc = i+1;
            break;
        }
    }
    int tsens = 2; //min-1 2-lowth 3-passes
    for (int i = 0; i < 3; ++i)
    {
        if (rb_vdec_sens[i]->isChecked())
        {
            tsens = i+1;
            break;
        }
    }
    TDecoderMs->SetVarDecodeFtPar(udec,tcyc,tsens);
}
/////////////DragDrop/////////////////////////////////
bool Main_Ms::SupportedDrags(QString data)
{
    bool support = false;

    if (f_decoder_busy)
        return support;

    data.remove("\n");
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)//2.56
    QStringList pieces = data.split(QRegExp("file://"), Qt::SkipEmptyParts);
#else
    QStringList pieces = data.split(QRegExp("file://"), QString::SkipEmptyParts);
#endif
    if (pieces.count()==1)// only one file
    {
        for (QStringList::iterator it =  pieces.begin(); it != pieces.end(); it++)
        {
            QString path_rem_space = (*it).replace("%20", " ").toUtf8();// qt4.4.0 zaradi " i kirilica
            QFileInfo fi(path_rem_space);
            QString str = fi.suffix().toLower().toUtf8(); //qt4.4.0 zaradi " i kirilica
            if (str == "wav") support = true;
            //QDir dir(path_rem_space);
            //if (dir.exists())
            //support = true;
        }
    }
    return support;
}
void Main_Ms::dragEnterEvent(QDragEnterEvent *event)
{
    if (!f_decoder_busy)
    {
        QString filess;
        QList<QUrl> urls = event->mimeData()->urls();
        for (int i = 0; i < urls.size(); ++i)
        {
            filess.append("file://"+urls.at(i).toLocalFile());
        }
        if (SupportedDrags(filess))
            event->acceptProposedAction();
        //else
        //event->ignore();
    }
    //else
    //event->ignore();
    //QCoreApplication::processEvents();// qt5
    //qDebug()<<QApplication::startDragTime();
}
void Main_Ms::dropEvent(QDropEvent *event)
{
    if (event->mimeData()->hasFormat("text/uri-list"))
    {
        QString filess;
        QList<QUrl> urls = event->mimeData()->urls();

        for (int i = 0; i < urls.size(); ++i)
        {
            filess.append("file://"+urls.at(i).toLocalFile());
        }
        QString instr = filess.remove("\n");
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)//2.56
        QStringList pieces = instr.split(QRegExp("file://"), Qt::SkipEmptyParts);
#else
        QStringList pieces = instr.split(QRegExp("file://"), QString::SkipEmptyParts);
#endif
        if (pieces.count()>0) FileOpen(pieces.at(0));

        event->acceptProposedAction();
    }
}
void Main_Ms::dragMoveEvent(QDragMoveEvent *event)
{
    QRect rec = MainDisplay->rect();

    rec.setY(rec.y()+24);
    rec.setHeight(rec.height()+24);
    if (rec.contains(event->pos()) && !MainDisplay->isHidden())//v1.30 !MainDisplay->isHidden() for one disp work
        event->acceptProposedAction();
    else
        event->ignore();

    //QApplication::processEvents();
    //QCoreApplication::processEvents(QEventLoop::AllEvents);
}
/////////////DragDrop////////////
void Main_Ms::SetBandFromRigFreq(int i)
{
    s_id_set_to_rig = 1;//0<-from App 1<-from Rig //qDebug()<<"SetBandFromRigFreq"<<s_id_set_to_rig<<i;
    ListBands.at(i)->setChecked(true);
}
void Main_Ms::SkUpDownBandChanged(bool f_up_down)
{
    if (f_tx_busy) return;
    int ib = -1;
    for (int i = 0; i<COUNT_BANDS; ++i)
    {
        if (ListBands.at(i)->isChecked())
        {
            ib = i;
            break;
        }
    }
    if (ib>-1)// band is exit
    {
        if (f_up_down && ib+1<COUNT_BANDS) ListBands.at(ib+1)->setChecked(true);
        else if (!f_up_down && ib-1>-1) ListBands.at(ib-1)->setChecked(true);
    }
}
void Main_Ms::BandChanged(bool)
{
    //StopTxGlobal();//2.12
    for (int i = 0; i<COUNT_BANDS; ++i)
    {
        if (ListBands.at(i)->isChecked())
        {
            QString temp_band = lst_bands[i];//2.16
            THvRigControl->SetBand(temp_band);
            THvTxW->SetBand(temp_band,s_id_set_to_rig);//0<-from App 1<-from Rig
            RefreshWindowTitle();
            s_id_set_to_rig = 0;
            W_band_bt_sw->SetActiveBt(i);
            break;
        }
    }
}
void Main_Ms::ModeMenuStatRefresh(bool dea)
{
    bool txa = false;   //ft8 and ft4
    if (s_mode==11 || s_mode==13 || s_mode==18 || allq65) txa = f_tx_busy;
    if (dea || txa)
    {
        Mode_m->setDisabled(true);
        W_mod_bt_sw->setDisabled(true);
    }
    else if (!dea && !txa)
    {
        Mode_m->setDisabled(false);
        W_mod_bt_sw->setDisabled(false);
    }
}
void Main_Ms::SetRigTxRx(bool f)
{
    //qDebug()<<"SetRigTxRx...................."<<f;
    f_tx_busy = f;//2.47
    THvRigControl->SetPtt(f,0);//id 0=All 1=p1 2=p2
    THvTxW->SetTxRxCountAutoSeq(f);
    if (f)
    {
        if (dsty) l_tx_text->setStyleSheet("QLabel{background-color:rgb(170,0,0);}");
        else l_tx_text->setStyleSheet("QLabel{background-color:rgb(255,0,0);}");
        Band_m->setDisabled(true);
        W_band_bt_sw->setDisabled(true);
    }
    else
    {
        l_tx_text->setStyleSheet("QLabel{background-color:palette(Button);}");
        Band_m->setDisabled(false);
        W_band_bt_sw->setDisabled(false); //qDebug()<<g_block_mam<<g_ub_m_k;
    }
    //f_tx_busy = f;
    if (s_mode==11 || s_mode==13 || s_mode==18 || allq65) ModeMenuStatRefresh(f_de_active);//ft8 and ft4
}
void Main_Ms::SetOffsetDt(int dt)//2.76.5
{ 
	s_offset_dt=dt; //qDebug()<<s_offset_dt;
	TMsPlayerHV->SetOffsetDt(dt);
}
QDateTime Main_Ms::getDateTime()
{
    QDateTime utc_t;//QDateTime utc_t = QDateTime::currentDateTimeUtc().addMSecs(-500);
    if (s_offset_dt==0) utc_t = QDateTime::currentDateTimeUtc();
    else utc_t = QDateTime::currentDateTimeUtc().addMSecs(s_offset_dt);
    return utc_t;
}
void Main_Ms::SetPeriodTime(float val)
{
    //StopRxGlobal();
    //qDebug()<<"15===="<<val;
    period_time_sec = (double)val;
    /*if (period_time_sec==7.0)//2.35 correct in spinbox
        period_time_sec=7.5;*/

    TDecoderMs->SetPerodTime((int)period_time_sec);

    //f_once_pt_15_to_30 0<-no 1<-30s 2<-15s
    if (val<=15 && f_once_pt_15_to_30!=2)
    {
        //f_once0s = false;
        MainDisplay->setHDispPeriodTime(15,s_mode);
        SecondDisplay->setHDispPeriodTime(15,s_mode);
        f_once_pt_15_to_30 = 2;
        //qDebug()<<"15="<<val;
    }
    /*&& f_once_pt_15_to_30!=1*/
    if (val>15)
    {
        //qDebug()<<"30="<<val;
        MainDisplay->setHDispPeriodTime(val,s_mode);//
        SecondDisplay->setHDispPeriodTime(val,s_mode);//
        f_once_pt_15_to_30 = 1;
    }

    fast_find_period = true;
}
QString Main_Ms::getHHMin()
{
    /*old no ms 5,6,10,15,30,60
    QString hm = getDateTime().toString("hhmm");
    int ss = getDateTime().toString("ss").toInt();
    int multip_ss = ss/period_time_sec;
    multip_ss = period_time_sec*multip_ss;
    if (multip_ss < 10)
        hm.append("0");
    hm.append(QString("%1").arg(multip_ss));*/
    /*QString hm = getDateTime().toString("hhmm");
    int sz1 = getDateTime().toString("sszzz").toInt();
    sz1 /=100;
    int ssms1 = (int)(period_time_sec*10.0); qDebug()<<"------="<<sz1<<ssms1;
    int multip_ssz = sz1/ssms1;
    multip_ssz = (ssms1*multip_ssz)/10;
    if (multip_ssz < 10) hm.append("0");
    hm.append(QString("%1").arg(multip_ssz)); qDebug()<<"FFFFFF="<<hm<<multip_ssz;
    return hm;*/
    QString hm = getDateTime().toString("hhmm");//2.67.6 to 1000ms FT2=3.750
    int sz1 = getDateTime().toString("sszzz").toInt();//sz1 /=10;
    int ssms1 = (int)(period_time_sec*1000.0); //qDebug()<<"------="<<sz1<<ssms1;
    int multip_ssz = sz1/ssms1;
    multip_ssz = (ssms1*multip_ssz)/1000;
    if (multip_ssz < 10) hm.append("0");
    hm.append(QString("%1").arg(multip_ssz)); //qDebug()<<"FFFFFF="<<hm<<multip_ssz;
    return hm;
}
void Main_Ms::SetRxOnlyFiSe(bool f) // v1.27 for rx only first second period
{
    f_rx_only_fi_se = f;
    //StopRxGlobal();
    //StartRxGlobal();
    f_fast_rfresh_only_fi_se = true;
    //qDebug()<<"RX_ONLY_FS_SE="<<f_rx_only_fi_se<<f_fast_rfresh_only_fi_se;
}
void Main_Ms::Refresh()
{
    bool f_fs_se;
    QDateTime utc_t = getDateTime();//THvTxW->SetDataTime(utc_t.toString("dd.MM.yyyy"),utc_t.toString("hh:mm:ss"));//MMMM
    THvTxW->SetDataTime(utc_t);//MMMM
    int ss = utc_t.toString("ss").toInt();//int ms = utc_t.toString("z").toInt(); //<--- error QT > QT5 v5.9.x
    int ms = utc_t.toString("zzz").toInt();
    int period_time_msec = (int)(period_time_sec*1000.0); //qDebug()<<"RX_ONLY_FS_SE="<<period_time_msec;
    int i_period_time_sec =(int)(period_time_sec);//2.67.5
    static int8_t ss0 = 0;
    if (f_tune)//2.67.5 tune period jtdx=30000mec
    {
        period_time_msec =30000;
        i_period_time_sec=30;
        if (ss0==0) ss0=ss;
        if (ss-ss0<0) ss+=(60-ss0);
        else ss-=ss0;
        /*static int8_t ttt = -1;
        if(ttt!=ss)
        {
        	ttt=ss;
        	printf(" ss= %d\n",ttt);     
        }*/
    } else ss0=0;
    //int min_59 = utc_t.toString("m").toInt(); //<--- error QT > QT5 v5.9.x
    int min_59 = utc_t.toString("mm").toInt();

    time_pos_1period = (min_59*60000)+(ss*1000)+ms; //time_pos_1period = ss+ms;
    time_pos_2period = time_pos_1period;

    //time_pos_2period = time_pos_2period % (period_time_msec*2); // not work correct
    while (time_pos_2period>period_time_msec*2)//time_pos_2period za dva perioda tx+rx
        time_pos_2period -=period_time_msec*2;

    //time_pos_1period = time_pos_1period % period_time_msec; // not work correct
    while (time_pos_1period>period_time_msec)//time_pos_1period za edin priod
        time_pos_1period -=period_time_msec;

    if (time_pos_2period<=period_time_msec) f_fs_se = true;
    else f_fs_se = false;

    int stop_tx50s = 0;
    if 		(s_mode==11)
    {
        stop_tx50s = 600;//stop 14s stop TX//ft8
        //if (id_mshf==2) stop_tx50s = 400;//2.76 ???
    }
    else if (s_mode==13) stop_tx50s = 800;//stop 5s stop TX     //ft4 error 2.18=if(s_mode==13) need to be else if
    else if (s_mode==18) stop_tx50s = 400;//ft2 ???
    else if (allq65)//q65 stop TX
    {
        stop_tx50s = 800; 									// 2,25- 0,5 = 1,75 to end
        if 		(i_period_time_sec== 30) stop_tx50s = 2000;	// 4,5 - 0,5 = 4,0  to end
        else if (i_period_time_sec== 60) stop_tx50s = 6000;  	// 9,0 - 1,0 = 8,0  to end
        else if (i_period_time_sec==120) stop_tx50s = 3600; 	// 6.6 - 1,0 = 5,6  to end
    }
    else if (s_mode==7 || s_mode==8 || s_mode==9)
    {
        stop_tx50s = 11000;// 48s stop 49s stop TX
        if (f_tune) stop_tx50s = 2000;//2.76.5
    }
    else stop_tx50s = 0;

    if (s_mode==10)
    {
        if (!TMsPlayerHV->Is_RealStop() || THvTxW->GetAutoIsOn()) StopTxGlobal();
    }

    bool f_tx_fi_se = THvTxW->GetTxFi();
    if (f_tx_fi_se)
    {
        if (f_fs_se && time_pos_1period<(period_time_msec-stop_tx_befor_end-stop_tx50s)) f_is_period_rx_tx=true;
        else f_is_period_rx_tx=false;
    }
    else
    {
        if (!f_fs_se && time_pos_1period<(period_time_msec-stop_tx_befor_end-stop_tx50s)) f_is_period_rx_tx=true;
        else f_is_period_rx_tx=false;
    }

    //////////////REMUTE/////////////////////////////////
    if (THvTxW->GetAutoIsOn())
    {
        if (global_start_moni && fi_se_changed != (int)f_tx_fi_se)
        {
            fi_se_changed = (int)f_tx_fi_se;
            if (!f_is_period_rx_tx) s_f_dec50 = false;
            else s_f_dec50 = true;
            //qDebug()<<"first second changed"<<fi_se_changed<<f_is_period_rx_tx;
        }

        if (TMsCore->GetSta_Sto() && !f_is_period_rx_tx)//na rx i si period za slu6ane
            f_auto_on = true;//da ne triperi tx-rx pri auto on vavno
        else
            f_auto_on = false;

        if (f_is_period_rx_tx && TMsPlayerHV->Is_RealStop())
        {
            f_auto_on = false;
            TxMessage(s_msg,true);
        }
        else if (!f_is_period_rx_tx && !TMsPlayerHV->Is_RealStop())
        {
            f_tx_to_rx = true; //qDebug()<<"1Stop TX Start RX"<<time_pos_1period;
        }
    }
    if (!THvTxW->GetAutoIsOn() && time_pos_1period>(period_time_msec-stop_tx_befor_end-stop_tx50s)//stop tx 370ms bifor rx period tx buffer is 4096=370ms
            && !TMsPlayerHV->Is_RealStop())
    {
        f_tx_to_rx = true; //qDebug()<<"2Stop TX Start RX"<<time_pos_1period;
    }

    if (s_mode==10)//pi4 rx
    {
        if (time_pos_1period>30000 && f_onse50 && global_start_moni)//s_f_dec50  52000 12000
        {
            f_onse50 = false;
            if (!s_f_dec50)
            {
                MainDisplay->DecodeAllData(true,true,f_fs_se);
                s_f_dec50 = true; //qDebug()<<"Decode50";
            }
        }
    }
    else if (s_mode==11)//ft8 rx to decode  11800 -1700- 13500 -1200- 14700 jt9=50*3456=14.4sec
    {
        int start_dec_t = 14400; //qDebug()<<"f_fs_se"<<f_fs_se; f_fs_se=true=first
        if (cb_3intFt_d->isChecked()) start_dec_t = 11820;//2.71=11810 need 141696 samples   old=11750
        if (id_mshf>0 && f_fs_se) start_dec_t = 14390;//2.76.2 old->2.76=14400
        if (time_pos_1period>start_dec_t && f_onse50 && global_start_moni)//2.38=14400 2.20=14250 2.19=14200 wsjtx=14350
        {
            f_onse50 = false;
            if (!s_f_dec50)
            {
                MainDisplay->DecodeAllData(true,true,f_fs_se); //qDebug()<<f_fs_se;
                s_f_dec50 = true; //qDebug()<<"Decode50"<<s_f_dec50;
            }
        }
    }
    else if (s_mode==13)//ft4 rx to decode
    {
        if (time_pos_1period>6100 && f_onse50 && global_start_moni)//2.21 st=0.350 tx=0.350+5.04  in wsjt=??
        {
            f_onse50 = false;
            if (!s_f_dec50)
            {
                MainDisplay->DecodeAllData(true,true,f_fs_se);
                s_f_dec50 = true; //qDebug()<<"Decode50"<<s_f_dec50;
            }
        }
    }
    else if (s_mode==18)//ft2 rx to decode ??? 3.75
    {
        if (time_pos_1period>3400 && f_onse50 && global_start_moni)//2.21 st=0.350 tx=0.350+5.04  in wsjt=??
        {
            f_onse50 = false;
            if (!s_f_dec50)
            {
                MainDisplay->DecodeAllData(true,true,f_fs_se);
                s_f_dec50 = true; //qDebug()<<"Decode50"<<s_f_dec50;
            }
        }
    }
    else if (allq65) // q65 rx
    {
        int start_dec_t = 13900;    							//wsjt-x  13.8 s
        if		(i_period_time_sec== 30) start_dec_t = 28600;   	//wsjt-x  27.6 s, decode_at_52s= 28.8 s
        else if (i_period_time_sec== 60) start_dec_t = 56500;   	//wsjt-x  56.4 s
        else if (i_period_time_sec==120) start_dec_t = 117600;  	//wsjt-x  117.5 s
        if (time_pos_1period>start_dec_t && f_onse50 && global_start_moni)//s_f_dec50  52000 12000
        {
            f_onse50 = false;
            if (!s_f_dec50)
            {
                MainDisplay->DecodeAllData(true,true,f_fs_se);
                s_f_dec50 = true; //qDebug()<<"Decode50";
            }
        }
    }
    else // jt65 rx if(s_mode==7 || s_mode==8 || s_mode==9)
    {
        if (time_pos_1period>52000 && f_onse50 && global_start_moni)//s_f_dec50  52000 12000
        {
            f_onse50 = false;
            if (!s_f_dec50)
            {
                MainDisplay->DecodeAllData(true,true,f_fs_se);
                s_f_dec50 = true; //qDebug()<<"Decode50";
            }
        }
    }


    if (f_fast_rfresh_only_fi_se && global_start_moni && !THvTxW->GetAutoIsOn()) // v1.27 for rx only first second period
    {
        f_fast_rfresh_only_fi_se = false;
        //s_f_dec50 = false;
        if (!f_rx_only_fi_se) // && !TMsCore->GetSta_Sto()
        {
            if (!TMsCore->GetSta_Sto())
            {
                //MainDisplay->SetSyncPosition(time_pos_1period,getHHMin(),utc_t.toString("yyMMdd"));
                StartRx();  //StartRx() tam ima -> MainDisplay->SetSyncPosition
                s_f_dec50 = false;
                //qDebug()<<"StartRx() If OFF F/S  From STOP";
            }
        }
        else
        {
            if (!f_is_period_rx_tx)
            {
                //MainDisplay->SetSyncPosition(time_pos_1period,getHHMin(),utc_t.toString("yyMMdd"));
                StartRx(); //StartRx() tam ima -> MainDisplay->SetSyncPosition
                s_f_dec50 = false;
                //qDebug()<<"StartRx() If Period F/S  From STOP";
            }
            else
            {
                StopRx();
                if (!s_f_dec50)
                {
                    MainDisplay->DecodeAllData(true,true,f_fs_se);
                    s_f_dec50 = true;
                    //qDebug()<<"DecodeAllData Auto OFF FAST REFRESH FIRST SECOND======="<<s_f_dec50;
                }
            }
        }
    }

    if ((time_pos_1period<(period_time_msec/2-10) && f_once0s) || fast_find_period)
    {
        // kogato se nulira togava  SetSyncPosition
        //qDebug()<<"<<<<<<<<<<<<"<<time_pos_1period<<period_time_msec<<f_once0s;
        fast_find_period = false;
        f_once0s = false;
        if (global_start_moni && !THvTxW->GetAutoIsOn())//pokriza moitor off, a tx start
        {
            if (f_rx_only_fi_se) // v1.27 for rx only first second period
            {
                if (!f_is_period_rx_tx)
                {
                    //MainDisplay->SetSyncPosition(time_pos_1period,getHHMin(),utc_t.toString("yyMMdd"));
                    StartRx();
                    //qDebug()<<"StartRx() Evry F/S period From STOP";
                }
                else
                {
                    StopRx();
                    if (!s_f_dec50)
                    {
                        MainDisplay->DecodeAllData(true,true,f_fs_se);
                        //s_f_dec50 = true;
                        //qDebug()<<"DecodeAllData Auto OFF ======="<<s_f_dec50;
                    }
                }
            }
            else
            {
                MainDisplay->SetSyncPosition(time_pos_1period,getHHMin(),utc_t.toString("yyMMdd"),s_f_dec50);
                StartRx(); // no need this ? but for any case HV 1.27
                //qDebug()<<"11SetSyncPosition + StartRx()";
            }
        }

        if (TMsPlayerHV->Is_RealStop() && THvTxW->GetAutoIsOn())
        {
            MainDisplay->SetSyncPosition(time_pos_1period,getHHMin(),utc_t.toString("yyMMdd"),s_f_dec50);
            //qDebug()<<"22 SetSyncPosition"<<s_f_dec50;
        }

        if (!TMsPlayerHV->Is_RealStop() && THvTxW->GetAutoIsOn())
        {
            if (!s_f_dec50)
            {
                MainDisplay->DecodeAllData(true,true,f_fs_se);
                //s_f_dec50 = true;
                //qDebug()<<"DecodeAllData Auto ON ======="<<s_f_dec50;
            }
        }

        if (global_start_moni)
        {
            if (f_rx_only_fi_se || THvTxW->GetAutoIsOn())
            {
                if (!f_is_period_rx_tx)
                    s_f_dec50 = false;
                //else
                //{
                //qDebug()<<"1s_f_dec50==================s_f_dec50===================="<<s_f_dec50;
                //s_f_dec50 = true;
                //qDebug()<<"2s_f_dec50==================s_f_dec50===================="<<s_f_dec50;
                //}
            }
            else
                s_f_dec50 = false;
            //qDebug()<<"s_f_dec50=================="<<s_f_dec50;
        }
    }

    if (time_pos_1period>(period_time_msec/2+10) && !f_once0s)//
    {
        //qDebug()<<">>>>>>>>>>>>"<<time_pos_1period;
        f_once0s = true;
        f_onse50 = true;
    }

    // TX to RX width delay
    if (f_tx_to_rx)
    {
        f_tx_to_rx = false;
        TMsPlayerHV->Stop(); //qDebug()<<"MAIN====== TMsPlayerHV->Stop()"<<TMsPlayerHV->Is_RealStop();
        count_tx_to_rx = stop_tx_befor_end + 25;//0.96->50ms vazno +25ms zaradi auto decode ina4e iztriva displea sled tx
        f_rx_glob_ = true;

        if (f_tune)// za da raboti i na stop
        {
            f_tune = false;
            pb_tune->setStyleSheet("QPushButton{background-color:palette(Button);}");
            THvTxW->GetCurrentMsg();
        }
    }
    if ( count_tx_to_rx > 0)
    {
        count_tx_to_rx -= 5;//5ms refresh time in mscore.cpp ThreadRefr msleep(5);<-5ms or timer_ref_->start(10);
        //qDebug()<<"CD="<<count_tx_to_rx;
    }
    else
        count_tx_to_rx = 0;
    if (f_rx_glob_ && count_tx_to_rx==0)
    {
        f_rx_glob_ = false; //qDebug()<<"RXXXXXXXX";
        SetRigTxRx(false);
        if (THvTxW->GetAutoIsOn())
            StartRxGlobal();
        else //if (!f_rx_only_fi_se) //off auto and only -> v1.27 for rx only first second period
        {
            if (!f_rx_only_fi_se)  // v1.27 for rx only first second period
                StartRx();
            if (f_rx_only_fi_se && !f_is_period_rx_tx) // v1.27 for rx only first second period
                StartRx();
        }
    }
    //////////////END REMUTE/////////////////////////////////
    //qDebug()<<"s_f_dec50"<<s_f_dec50;
}
void Main_Ms::SetAuto()
{
    if (!THvTxW->GetAutoIsOn())
    {
        f_auto_on = false;
        f_tx_to_rx = true; //qDebug()<<"3Stop TX Start RX";// ako izklu4a auto da spre i tx ako e na tx

        //qDebug()<<"s_f_dec50"<<s_f_dec50;
        //if (s_mode!=11)//ft8 no good for aoto_seq "stop auto" make decodings my be need remove for all modes
        //s_f_dec50 = false; //vazno pri spirane AUTO da ne go smiata za decodirano i decodira nowoto my be need to stop it

        //v1.47 no decode if data is < p_time*2/3 and auto is off from tx
        //v1.47 this is importent for AutoSeq for ues only one 73 to stop TX other case make one more decode
        int period_time_msec_23 = (((int)(period_time_sec*1000.0))*2/3);
        if (time_pos_1period < period_time_msec_23)
        {
            s_f_dec50 = false;
            //qDebug()<<"RESET s_f_dec50"<<time_pos_1period<<period_time_msec_23<<s_f_dec50;
        }
        //else
        //qDebug()<<"NO RESET s_f_dec50"<<time_pos_1period<<period_time_msec_23<<s_f_dec50;
        //s_f_dec50 = true;
    }
    else
    {
        f_auto_on = true;
        //if (!f_rx_only_fi_se || !global_start_moni) // v1.27 for rx only first second period
        //StartRxGlobal();// ima smisal ne vklu4en auto on za parwi pat

        // s postoianen refresh na f_is_period_rx_tx v void Refresh() pokriva vsi4ki iziskania
        if (!f_is_period_rx_tx) // v1.27 for rx only first second period
            StartRxGlobal(); // ima smisal ne vklu4en auto on za parwi pat
        else
        { // ostava za6toto taka e na drugite versii 1.27 HV
            global_start_moni = true;
            if (dsty) pb_start_rx->setStyleSheet("QPushButton{background-color:rgb(64,150,0);}");
            else pb_start_rx->setStyleSheet("QPushButton{background-color:rgb(50,240,50);}");//150, 150, 255
        }

        if (f_tune)//stop tune if auto is on and send current msg
        {
            f_tune = false;
            pb_tune->setStyleSheet("QPushButton{background-color:palette(Button);}");
            THvTxW->GetCurrentMsg();
            f_auto_on = false;
            TxMessage(s_msg,true);
        }
    }
}
void Main_Ms::SetRxAllTxt(QString rx_str)
{
    QDateTime utc_t = getDateTime();
    QString freq_g = THvTxW->GetFreqGlobal();
    freq_g = freq_g.mid(0,freq_g.size()-3);//to khz
    //QString mshf = "";//2.76sf
    if (s_mode==11)
    {
        if (id_mshf==1) rx_str.replace(0,3,"FT8_SH");//rx_str.replace("FT8|","FT8_SH|");
        if (id_mshf==2) rx_str.replace(0,3,"FT8_SF");
    }
    TAllTxt->SetTxt(utc_t.toString("yyyy_MM_dd"),"RX "+freq_g+" "+rx_str); //qDebug()<<rx_str;
}
void Main_Ms::SetQrgQSY(QStringList l)//2.46
{
    QString s = l.at(0);
    int sc = s.count();
    s = s.mid(0,sc-3);
    s.insert(sc-6,".");
    s = " - "+s+" -";
    _stcq_ = l.at(1);
    SetTxMsgAllTxt(s,-10.0);//ID QSY = -10.0
}
void Main_Ms::SetTxMsgLabTx(QString str)
{
    //qDebug()<<"SetTxMsgLabTx"<<s_mode<<Multi_answer_mod->isChecked();
    //no update if msg is same
    THvTxW->SetLastTxRptAutoSeq(str);
    if ((s_mode==11 || s_mode==13 || s_mode==18 || allq65) && (Multi_answer_mod->isChecked() || Multi_answer_mod_std->isChecked()))
    {
        int ch = str.count("#");
        if (ch>0) l_tx_text->setText("Txing: "+QString("%1").arg(ch+1)+" Slot Messages");
        else l_tx_text->setText("Txing: "+str);
    }
    else l_tx_text->setText("Txing: "+str);
}
void Main_Ms::SetTxMsgAllTxt(QString str,double tx_freq)
{
    QDateTime utc_t = getDateTime();
    QString freq_g = THvTxW->GetFreqGlobal();
    freq_g = freq_g.mid(0,freq_g.size()-3);//to khz

    QString mshf = "";//2.76sf
    if (s_mode==11)
    {
        if (id_mshf==1) mshf = "_SH";
        if (id_mshf==2) mshf = "_SF";
    }
    if (tx_freq==-10.0)//ID QSY = -10.0
        TAllTxt->SetTxt(utc_t.toString("yyyy_MM_dd"),"QSY "+freq_g+" "+ModeStr(s_mode)+mshf+
                        utc_t.toString("|hhmmss|")+str);
    else
        TAllTxt->SetTxt(utc_t.toString("yyyy_MM_dd"),"TX "+freq_g+" "+ModeStr(s_mode)+mshf+
                        utc_t.toString("|hhmmss|")+str);
    /*TDecodeList2->isVisible() && */
    if (s_mode==0 || s_mode==12 || s_mode==7 || s_mode==8 || s_mode==9 || s_mode==11 || s_mode==13 || s_mode==18 || allq65)//ft8 jt65 ft4
    {
        QString tx_time_ss = utc_t.toString("hhmmss");
        TDecodeList2->SetBackColor(false);//ID QSY = -10.0
        QStringList ltxx;
        QStringList lcqq;
        if (s_mode==11 || s_mode==13 || s_mode==18)//ft8 ft4
        {
            if (tx_freq==-10.0)//ID QSY = -10.0
            {
                TDecodeList2->SetBackColorTxQsy(true);//2.46 tx=0 qsy=1;
                lcqq<<""<<"CQ"<<""<<""<<_stcq_<<""<<""<<"";
                ltxx<<""<<"QSY"<<""<<""<<str<<""<<""<<"";
                TDecodeList2->InsertItem_hv(lcqq,true,true);
                TDecodeList2->InsertItem_hv(ltxx,true,true);
            }
            else
            {
                TDecodeList2->SetBackColorTxQsy(false);//2.46
                QStringList lmam;
                lmam = str.split("#");
                int cl = lmam.count();
                int nslt = 1;
                int itx_freq = (int)tx_freq;
                if (f_static_tx) itx_freq = (int)s_v_disp_tx_frq; //2.16  ft8 static tx
                for (int i = 0; i < cl; ++i)
                {
                    QString TXt = "TX";
                    if (cl > 1) TXt.append(QString("%1").arg(nslt));
                    //"Time"<<"dB"<<"DT"<<"DF From TX"<<"Message"<<"Type"<<"Qual"<<"Freq"; //7 Flags ft8
                    ltxx<<tx_time_ss<<TXt<<""<<""<<lmam.at(i)<<""<<""<<QString("%1").arg(itx_freq);
                    TDecodeList2->InsertItem_hv(ltxx,true,true);
                    ltxx.clear();
                    nslt++;
                    if (s_mode==11 && id_mshf != 2) itx_freq+=60;//2.76sf 1.70 60Hz interval
                    else if (s_mode==13)itx_freq+=100; //1.70 100Hz interval
                    else if (s_mode==18)itx_freq+=250;
                }
            }
        }
        else if (s_mode==7 || s_mode==8 || s_mode==9)//jt65
        {
            TDecodeList2->SetBackColorTxQsy(false);//2.46
            //"Time"<<"Sync"<<"dB"<<"DT"<<"DF"<<"W"<<"Message"<<"D Inf"<<"Flags"<<"Freq"; //jt65 9 Flags
            //ltxx<<tx_time_ss<<""<<"TX"<<""<<""<<""<<str<<""<<""<<QString("%1").arg((int)tx_freq);
            ltxx<<tx_time_ss<<""<<"TX"<<""<<""<<""<<str<<""<<""<<"1270";//2.76.3
            TDecodeList2->InsertItem_hv(ltxx,true,true);
        }
        else if (s_mode==0 || s_mode==12)//2.46 MSK
        {
            //"Time"<<"T"<<"Width"<<"dB"<<"Rpt"<<"DF"<<"Message"<<"Navg"<<"Bit err"<<"Eye"<<"Freq";//10 flags MSK
            if (tx_freq==-10.0)//ID QSY = -10.0
            {
                TDecodeList2->SetBackColorTxQsy(true);//2.46
                if (s_mode==0)
                {
                    lcqq<<""<<""<<""<<"CQ"<<""<<""<<_stcq_<<""<<""<<""<<"";
                    ltxx<<""<<""<<""<<"QSY"<<""<<""<<str<<""<<""<<""<<"";
                }
                if (s_mode==12)
                {
                    lcqq<<""<<""<<""<<""<<"CQ"<<""<<_stcq_<<""<<""<<""<<"";
                    ltxx<<""<<""<<""<<""<<"QSY"<<""<<str<<""<<""<<""<<"";
                }
                TDecodeList2->InsertItem_hv(lcqq,true,true);
                TDecodeList2->InsertItem_hv(ltxx,true,true);
            }
            else
            {
                TDecodeList2->SetBackColorTxQsy(false);//2.46
                if (s_mode==0)  ltxx<<tx_time_ss<<""<<""<<"TX"<<""<<""<<str<<""<<""<<""<<"1500";
                if (s_mode==12) ltxx<<tx_time_ss<<""<<""<<""<<"TX"<<""<<str<<""<<""<<""<<"1500";
                TDecodeList2->InsertItem_hv(ltxx,true,true);
            }
        }
        else if (allq65)//q65 no have QSY, but have static_tx
        {
            int itx_freq = (int)tx_freq;
            if (f_static_tx) itx_freq = (int)s_v_disp_tx_frq;
            TDecodeList2->SetBackColorTxQsy(false);
            ltxx<<tx_time_ss<<"TX"<<""<<""<<str<<""<<""<<QString("%1").arg(itx_freq);
            TDecodeList2->InsertItem_hv(ltxx,true,true);
        }
        TDecodeList2->SetBackColor(true);
    }
}
void Main_Ms::SetFilePlay()//hv inportent start play only from here
{
    //QTime ctt = QTime::currentTime();
    //qDebug()<<"1 SetFilePlay()....................";
    //TMsPlayerHV->setfile_play(q-strdup(qPrintable(s_msg)),true,s_mode);//2.47 s_gen removed
    static char c_msg_sent[430];//2.50 same is in genmesage_ms.cpp
    strncpy(c_msg_sent,s_msg.toUtf8(),420);
    TMsPlayerHV->setfile_play(c_msg_sent,true,s_mode,period_time_sec);

    if (!s_msg.contains("@")) THvTxW->SetStartQsoDateTime(); //EmitForStartQsoDateTime();
    if (f_tune && s_msg!="@TUNE")//smenia cweta na butona kogato ne e tune
    {
        f_tune = false;
        pb_tune->setStyleSheet("QPushButton{background-color:palette(Button);}");
    }
}
void Main_Ms::TxMessage(QString msg,bool immediately)//2.47 s_gen removed
{
    s_msg = msg.toUpper(); //qDebug()<<imidiatly<<msg; //2.47 stot s_imidi = immediately;
    if (immediately)
    {
        if (!f_auto_on) //da ne triperi tx-rx pri auto_on vavno
        {
            StopRx();
            SetRigTxRx(true); //qDebug()<<"UP_TX="<<QTime::currentTime().toString("ss:zzz");
            SetFilePlay();
        }
    }
}
void Main_Ms::TxMessageS(QString msg,bool immediately,bool iftximid)//2.47
{
    if (f_tx_busy && iftximid) TxMessage(msg,true);
    else TxMessage(msg,immediately);
}
void Main_Ms::SetFileToDisplay()// from player file open
{
    if (TMsCore->GetSta_Sto())
    {
        StopRx();
    }
    MainDisplay->ReciveClarDisplay();
}
void Main_Ms::Tune()
{
    if (!f_auto_on)//da ne tragva tune pri auto on v perioda za slu6ane //na rx i si period za slu6ane f_auto_on=true
    {
        if (f_tune)
        {
            if (!THvTxW->GetAutoIsOn())
            {
                f_tx_to_rx = true; //qDebug()<<"4Stop TX Start RX";
            }
            else
            {
                //if auto on period for tx -> tune off and send current mesg
                f_tune = false;
                pb_tune->setStyleSheet("QPushButton{background-color:palette(Button);}");
                THvTxW->GetCurrentMsg();
                TxMessage(s_msg,true);
            }
        }
        else
        {
            f_tune = true;
            if (dsty) pb_tune->setStyleSheet("QPushButton{background-color:rgb(170,10,10);}");
            else pb_tune->setStyleSheet("QPushButton{background-color:rgb(255,110,110);}");//150, 150, 255
            TxMessage("@TUNE",true);
        }
    }
}
void Main_Ms::SetUdpCmdStop(bool)// f
{
    StopTxGlobal();
}
void Main_Ms::StopTxGlobal()
{
    THvTxW->StopAuto();
    f_tx_to_rx = true; //qDebug()<<"5Stop TX Start RX";
}
void Main_Ms::StartRxGlobal()
{
    global_start_moni = true;
    if (dsty) pb_start_rx->setStyleSheet("QPushButton{background-color:rgb(64,150,0);}");
    else pb_start_rx->setStyleSheet("QPushButton{background-color:rgb(50,240,50);}");//150, 150, 255

    StartRx();
}
void Main_Ms::StartRx()
{
    //qDebug()<<"PLAY=====";
    //TMsCore->FastResetSound();
    if (!TMsCore->GetSta_Sto() && global_start_moni)
    {
        //qDebug()<<"1PLAY=====";

        //if(s_mode==0) // msk144 & msk40 za po natatak ako ima oplakwane 1.31
        //TDecoderMs->ResetCalsHashFileOpen();

        //f_open_file = false;
        //qDebug()<<"startppp";
        TMsCore->SetStr_Sto(true);
        TPicW->SetRxMon(true);
        QDateTime utc_t = getDateTime();
        MainDisplay->SetSyncPosition(time_pos_1period,getHHMin(),utc_t.toString("yyMMdd"),s_f_dec50);//true vinagi e decodirano
        //qDebug()<<"FFFFFF="<<time_pos_1period<<getHHMin()<<s_f_dec50;
    }
}
void Main_Ms::StopRx()
{
    TMsCore->SetStr_Sto(false);
    TPicW->SetRxMon(false);
}
void Main_Ms::StopRxGlobal()
{
    StopRx();
    global_start_moni = false;
    pb_start_rx->setStyleSheet("QPushButton{background-color:palette(Button);}");
}
/////////////////////////////////////////////////////
/*
void Main_Ms::SetAutoDecodeAll()
{
    cb_auto_decode_all->setChecked(auto_decode_all[s_mode]);
}
*/
void Main_Ms::CbSetAutoDecodeAll(bool f)
{
    if (f && (s_mode==0 || s_mode==12))//msk144 + msk40
        cb_rtd_decode->setChecked(false);

    auto_decode_all[s_mode] = f;

    MainDisplay->SetAutoDecodeAll(f);
    SecondDisplay->SetAutoDecodeAll(f);
}
void Main_Ms::CbSetStartStopRtd(bool f)
{
    if (s_mode==0 || s_mode==12)//msk144 + msk40
    {
        MainDisplay->SetStartStopRtd(f);
        SecondDisplay->SetStartStopRtd(f);//v1.29 decode to end
        if (f) cb_auto_decode_all->setChecked(false);
    }
}
void Main_Ms::SetDecodeBusy(bool f,int dec_state)//dec_state no=0 dec=1 rtddec=2
{
    f_decoder_busy = f;// for drag drop funktion

    TMsCore->SetDecBusy(f_decoder_busy);

    TPicW->SetDecode(dec_state);

    if ( dec_state == 0 )  //|| dec_state==2
    {
        f_de_active = false;//special flag not same as f_decoder_busy
        SB_VDispSpeed->setReadOnly(false);
        SB_VDispStartFreq->setReadOnly(false);
        SB_VDispBandwidth->setReadOnly(false);//setReadOnly  setDisabled
    }
    else
    {
        f_de_active = true;//special flag not same as f_decoder_busy
        SB_VDispSpeed->setReadOnly(true);
        SB_VDispStartFreq->setReadOnly(true);
        SB_VDispBandwidth->setReadOnly(true);
    }
    //2.69
    SB_VDispSpeed->findChild<QLineEdit*>()->setReadOnly(true);
    SB_VDispStartFreq->findChild<QLineEdit*>()->setReadOnly(true);
    SB_VDispBandwidth->findChild<QLineEdit*>()->setReadOnly(true);

    ModeMenuStatRefresh(f_de_active);
    //THvTxW->SetDecode(f_de_active);
}
void Main_Ms::SetSHAllColl(bool)
{
    //if (s_mode==0 || s_mode==12 || s_mode==11 || s_mode==13  || s_mode==18)
    s_show_lc[s_mode][0] = ac_show_timec->isChecked();
    s_show_lc[s_mode][1] = ac_show_distc->isChecked();
    s_show_lc[s_mode][2] = ac_show_freqc->isChecked();
    s_show_lc[s_mode][3] = ac_show_counc->isChecked();
    SetModeDecodeListS(two_dec_list_all[s_mode],s_show_lc[s_mode][0],s_show_lc[s_mode][1],s_show_lc[s_mode][2],s_show_lc[s_mode][3]);
}
void Main_Ms::SetTwoDecList(bool f)
{
    two_dec_list_all[s_mode] = f;
    SetModeDecodeListS(f,s_show_lc[s_mode][0],s_show_lc[s_mode][1],s_show_lc[s_mode][2],s_show_lc[s_mode][3]);
}
void Main_Ms::SetModeDecodeListS(bool f,bool f0,bool f1,bool f2,bool f3)//bool ftc,bool fdc,bool ffc,bool fcc
{
    ac_two_dec_list->setChecked(f);
    ac_show_timec->setChecked(f0);
    ac_show_distc->setChecked(f1);
    ac_show_freqc->setChecked(f2);
    ac_show_counc->setChecked(f3);
    TDecodeList1->SetMode(s_mode,f,f0,f1,f2,f3);
    TDecodeList2->SetMode(s_mode,f,f0,f1,f2,f3);
    if ((s_mode==0 || s_mode==12  || s_mode==7 ||s_mode==8 || s_mode==9 ||
            s_mode==11 || s_mode==13  || s_mode==18 || allq65) && f)//jt65 ft8 ft4 q65
    {
        pb_clar_list1->setText(tr("CLR MSG"));
        TDecodeList2->setHidden(false);
        pb_clar_list2->setHidden(false);
    }
    else
    {
        TDecodeList2->setHidden(true);
        pb_clar_list2->setHidden(true);
        pb_clar_list1->setText(tr("CLEAR MESSAGES"));
    }
}
void Main_Ms::ModBtSwClicked(int i)
{
    if (i == 220)
    {
        TDecodeList1->HideShowWfTxRefreshList();
        TDecodeList2->HideShowWfTxRefreshList();
        return;
    }
    if (i == s_mode) return;
    //StopRxGlobal(); // is not a good idea
    //if (!TMsPlayerHV->Is_RealStop() || THvTxW->GetAutoIsOn()) StopTxGlobal();
    rb_mode[i]->setChecked(true);
}
void Main_Ms::BandBtSwClicked(int i)
{
    if (i == 220)
    {
        TDecodeList1->HideShowWfTxRefreshList();
        TDecodeList2->HideShowWfTxRefreshList();
        return;
    }
    for (int x = 0; x<COUNT_BANDS; ++x)//if (i == s_band_ident) return;
    {
        if (ListBands.at(x)->isChecked())
        {
            if (x == i) return;
        }
    }
    //StopRxGlobal(); // is not a good idea
    //if (!TMsPlayerHV->Is_RealStop() || THvTxW->GetAutoIsOn()) StopTxGlobal();
    ListBands.at(i)->setChecked(true);
}
void Main_Ms::RbThrSetEnabled(bool f)
{
    for (int i = 1; i < 6; ++i)
    {
        if (f)
        {
            if (cthr<i+1) rb_thr[i]->setEnabled(false);
            else rb_thr[i]->setEnabled(true);
        }
        else rb_thr[i]->setEnabled(false);
    }
}
void Main_Ms::ModeChanged(bool fg)
{
    if (!fg) return; //qDebug()<<"Mode"<<fg;
    for (int i = 0; i < 3; ++i) rb_dec_depth[i]->setEnabled(false);
    RbThrSetEnabled(false);
    cb_3intFt_d->setEnabled(false);
    cb_UseVarDecFt->setEnabled(false);
    MVDecFtPar->setEnabled(false);
    for (int i = 0; i < 4; ++i) cb_msk144rxequal_[i]->setEnabled(false);
    cb_rtd_decode->setEnabled(false);
    MainDisplay->SetStartStopRtd(false);
    SecondDisplay->SetStartStopRtd(false);
    ac_aggressive->setEnabled(false);
    cb_vhf_uhf_decode_fac->setEnabled(false);
    cb_avg_decode->setEnabled(false);
    cb_auto_clr_avg_afdec->setEnabled(false);
    cb_deep_search_decode->setEnabled(false);
    pb_clear_avg65->setHidden(true);
    pb_clear_avgPi4->setHidden(true);
    ac_two_dec_list->setEnabled(false);
    ac_2click_list_autu_on->setEnabled(false);
    Direct_log_qso->setEnabled(false);
    Prompt_log_qso->setEnabled(false);
    cb_ap_decode->setEnabled(false);
    pb_dec_65->setHidden(true);
    MA_man_adding->setEnabled(false);
    Multi_answer_mod->setEnabled(false);
    Multi_answer_mod_std->setEnabled(false);
    ac_new_dec_clr_msg_list->setEnabled(false);
    ac_click_on_call_show_cty->setEnabled(false);
    ac_show_timec->setEnabled(false);
    ac_show_counc->setEnabled(false);
    ac_show_distc->setEnabled(false);
    ac_show_freqc->setEnabled(false);
    //ac_ft_df1500->setEnabled(false);
    ac_filter_list->setEnabled(false);
    ac_areset_qso->setEnabled(false);
    allq65 = false;
    cb_1_dec_sig_q65->setEnabled(false);
    cb_max_drift->setEnabled(false);
    cb_dec_aft_eme_delay->setEnabled(false);
    pb_clear_avgQ65->setHidden(true);
    FilterDialog->SetHidFLBtOnOff(true);
    vd_bw_lines_draw[0]->setEnabled(false);
    vd_bw_lines_draw[1]->setEnabled(false);
    vd_bw_lines_draw[2]->setEnabled(false);

    if (rb_mode[12]->isChecked())
    {   //MSKMS ??? cps.
        s_mode = 12;
        MainDisplay->setArrayInPxel(12);//12 26 v1.20 kakwo prostranstwo da decodira
        SecondDisplay->setArrayInPxel(12);//12 26 kakwo prostranstwo da decodira

        for (int i = 0; i < 3; ++i) rb_dec_depth[i]->setEnabled(true);

        for (int i = 0; i < 4; ++i) cb_msk144rxequal_[i]->setEnabled(true);

        cb_rtd_decode->setEnabled(true);
        MainDisplay->SetStartStopRtd(cb_rtd_decode->isChecked());
        SecondDisplay->SetStartStopRtd(cb_rtd_decode->isChecked());//v1.29 decode to end
        Direct_log_qso->setEnabled(true);
        Prompt_log_qso->setEnabled(true);
        ac_2click_list_autu_on->setEnabled(true);
        //ac_start_qso_from_tx2_or_tx1->setEnabled(true);//2.61 in SetMactos
        ac_two_dec_list->setEnabled(true);
        ac_click_on_call_show_cty->setEnabled(true);
        //ac_show_timec->setEnabled(true);
        ac_show_counc->setEnabled(true);
        ac_show_distc->setEnabled(true);
        ac_show_freqc->setEnabled(true);
    }
    if (rb_mode[0]->isChecked())
    {   //MSK144 ??? cps.
        s_mode = 0;
        MainDisplay->setArrayInPxel(12);//12 26 v1.20 kakwo prostranstwo da decodira
        SecondDisplay->setArrayInPxel(12);//12 26 kakwo prostranstwo da decodira

        for (int i = 0; i < 3; ++i) rb_dec_depth[i]->setEnabled(true);

        for (int i = 0; i < 4; ++i) cb_msk144rxequal_[i]->setEnabled(true);

        cb_rtd_decode->setEnabled(true);
        MainDisplay->SetStartStopRtd(cb_rtd_decode->isChecked());
        SecondDisplay->SetStartStopRtd(cb_rtd_decode->isChecked());//v1.29 decode to end
        Direct_log_qso->setEnabled(true);
        Prompt_log_qso->setEnabled(true);
        ac_2click_list_autu_on->setEnabled(true);
        //ac_start_qso_from_tx2_or_tx1->setEnabled(true);//2.61 in SetMactos
        ac_two_dec_list->setEnabled(true);
        ac_areset_qso->setEnabled(true);
        ac_click_on_call_show_cty->setEnabled(true);
        //ac_show_timec->setEnabled(true);
        ac_show_counc->setEnabled(true);
        ac_show_distc->setEnabled(true);
        ac_show_freqc->setEnabled(true);
    }
    if (rb_mode[1]->isChecked())
    {   //JTMS 197 cps.
        s_mode = 1;
        MainDisplay->setArrayInPxel(14);// kakwo prostranstwo da decodira
        SecondDisplay->setArrayInPxel(14);// kakwo prostranstwo da decodira
    }
    if (rb_mode[2]->isChecked())
    {    //FSK441 147 cps
        s_mode = 2;
        MainDisplay->setArrayInPxel(16);// kakwo prostranstwo da decodira
        SecondDisplay->setArrayInPxel(16);// kakwo prostranstwo da decodira
    }
    if (rb_mode[3]->isChecked())
    {    //FSK315 105 cps
        s_mode = 3;
        MainDisplay->setArrayInPxel(20);// kakwo prostranstwo da decodira
        SecondDisplay->setArrayInPxel(20);// kakwo prostranstwo da decodira
    }
    if (rb_mode[4]->isChecked())
    {   // ISCAT-A 16.15 cps.
        s_mode = 4;
        MainDisplay->setArrayInPxel(68);//68 v1.16 80 kakwo prostranstwo da decodira
        SecondDisplay->setArrayInPxel(68);//80 kakwo prostranstwo da decodira
    }
    if (rb_mode[5]->isChecked())
    {   //ISCAT-B 32.3 cps.
        s_mode = 5;
        MainDisplay->setArrayInPxel(50);//50 1.32 60 kakwo prostranstwo da decodira
        SecondDisplay->setArrayInPxel(50);//50 1.32 60 kakwo prostranstwo da decodira
    }
    if (rb_mode[6]->isChecked())
    {   //JT6M 14.4 cps.
        s_mode = 6;
        MainDisplay->setArrayInPxel(53);  // 1.39=53 fult->1.35=12  ok 53->v1.15 tested 52-75 kakwo prostranstwo da decodira
        SecondDisplay->setArrayInPxel(53);//1.39=53  fult->1.35=12   53->v1.15 tested 52-75 kakwo prostranstwo da decodira
    }
    if (rb_mode[7]->isChecked())
    {   // cps.
        s_mode = 7;
        MainDisplay->setArrayInPxel(50);  //1.40 no used in jt65->50
        SecondDisplay->setArrayInPxel(50);//1.40 no used in jt65->50

        ac_aggressive->setEnabled(true);
        cb_vhf_uhf_decode_fac->setEnabled(true);
        cb_avg_decode->setEnabled(true);
        cb_deep_search_decode->setEnabled(true);
        pb_clear_avg65->setHidden(false);
        pb_dec_65->setHidden(false);

        //1.57=!!!
        cb_ap_decode->setEnabled(true);
        ac_two_dec_list->setEnabled(true);
        ac_2click_list_autu_on->setEnabled(true);
    }
    if (rb_mode[8]->isChecked())
    {   // cps.
        s_mode = 8;
        MainDisplay->setArrayInPxel(50);  //1.40 no used in jt65->50
        SecondDisplay->setArrayInPxel(50);//1.40 no used in jt65->50

        ac_aggressive->setEnabled(true);
        cb_vhf_uhf_decode_fac->setEnabled(true);
        cb_avg_decode->setEnabled(true);
        cb_deep_search_decode->setEnabled(true);
        pb_clear_avg65->setHidden(false);
        pb_dec_65->setHidden(false);

        //1.57=!!!
        cb_ap_decode->setEnabled(true);
        ac_two_dec_list->setEnabled(true);
        ac_2click_list_autu_on->setEnabled(true);
    }
    if (rb_mode[9]->isChecked())
    {   // cps.
        s_mode = 9;
        MainDisplay->setArrayInPxel(50);  //1.40 no used in jt65->50
        SecondDisplay->setArrayInPxel(50);//1.40 no used in jt65->50

        ac_aggressive->setEnabled(true);
        cb_vhf_uhf_decode_fac->setEnabled(true);
        cb_avg_decode->setEnabled(true);
        cb_deep_search_decode->setEnabled(true);
        pb_clear_avg65->setHidden(false);
        pb_dec_65->setHidden(false);

        //1.57=!!!
        cb_ap_decode->setEnabled(true);
        ac_two_dec_list->setEnabled(true);
        ac_2click_list_autu_on->setEnabled(true);
    }
    if (rb_mode[10]->isChecked())
    {   // cps.
        s_mode = 10;
        MainDisplay->setArrayInPxel(50);  //1.40 no used in pi4
        SecondDisplay->setArrayInPxel(50);//1.40 no used in pi4
        cb_avg_decode->setEnabled(false);
        pb_clear_avgPi4->setHidden(false);
    }
    if (rb_mode[11]->isChecked())
    {   // cps.
        s_mode = 11;
        MainDisplay->setArrayInPxel(50);  //1.40 no used in ft8
        SecondDisplay->setArrayInPxel(50);//1.40 no used in ft8

        for (int i = 0; i < 3; ++i) rb_dec_depth[i]->setEnabled(true);
        RbThrSetEnabled(true);

        cb_3intFt_d->setEnabled(true); //2.39 remm
        cb_UseVarDecFt->setEnabled(true);
        MVDecFtPar->setEnabled(true);

        ac_two_dec_list->setEnabled(true);
        Direct_log_qso->setEnabled(true);
        Prompt_log_qso->setEnabled(true);
        cb_ap_decode->setEnabled(true);
        ac_2click_list_autu_on->setEnabled(true);
        if (!g_block_mam)
        {
            MA_man_adding->setEnabled(true);
            Multi_answer_mod->setEnabled(true);
            Multi_answer_mod_std->setEnabled(true);
        }
        //ac_start_qso_from_tx2_or_tx1->setEnabled(true);//2.61 in SetMactos
        ac_new_dec_clr_msg_list->setEnabled(true);
        ac_click_on_call_show_cty->setEnabled(true);
        //ac_ft_df1500->setEnabled(true);
        ac_filter_list->setEnabled(true);
        ac_areset_qso->setEnabled(true);
        FilterDialog->SetHidFLBtOnOff(false);
        ac_show_timec->setEnabled(true);
        ac_show_counc->setEnabled(true);
        ac_show_distc->setEnabled(true);
        ac_show_freqc->setEnabled(true);
    }
    if (rb_mode[13]->isChecked())
    {   // cps.
        s_mode = 13;
        MainDisplay->setArrayInPxel(50);  //1.40 no used in ft4
        SecondDisplay->setArrayInPxel(50);//1.40 no used in ft4

        for (int i = 0; i < 3; ++i) rb_dec_depth[i]->setEnabled(true);
        RbThrSetEnabled(true);

        ac_two_dec_list->setEnabled(true);
        Direct_log_qso->setEnabled(true);
        Prompt_log_qso->setEnabled(true);
        cb_ap_decode->setEnabled(true);
        ac_2click_list_autu_on->setEnabled(true);
        if (!g_block_mam)
        {
            MA_man_adding->setEnabled(true);
            Multi_answer_mod->setEnabled(true);
            Multi_answer_mod_std->setEnabled(true);
        }
        //ac_start_qso_from_tx2_or_tx1->setEnabled(true);//2.61 in SetMactos
        ac_new_dec_clr_msg_list->setEnabled(true);
        ac_click_on_call_show_cty->setEnabled(true);
        //ac_ft_df1500->setEnabled(true);
        ac_filter_list->setEnabled(true);
        ac_areset_qso->setEnabled(true);
        FilterDialog->SetHidFLBtOnOff(false);
        ac_show_timec->setEnabled(true);
        ac_show_counc->setEnabled(true);
        ac_show_distc->setEnabled(true);
        ac_show_freqc->setEnabled(true);
    }
    if (rb_mode[18]->isChecked())
    {   // cps.
        s_mode = 18;
        MainDisplay->setArrayInPxel(50);  //1.40 no used in ft2
        SecondDisplay->setArrayInPxel(50);//1.40 no used in ft2

        for (int i = 0; i < 3; ++i) rb_dec_depth[i]->setEnabled(true);
        RbThrSetEnabled(true);
        cb_avg_decode->setEnabled(true);

        ac_two_dec_list->setEnabled(true);
        Direct_log_qso->setEnabled(true);
        Prompt_log_qso->setEnabled(true);
        cb_ap_decode->setEnabled(true);
        ac_2click_list_autu_on->setEnabled(true);
        if (!g_block_mam)
        {
            MA_man_adding->setEnabled(true);
            Multi_answer_mod->setEnabled(true);
            Multi_answer_mod_std->setEnabled(true);
        }
        //ac_start_qso_from_tx2_or_tx1->setEnabled(true);
        ac_new_dec_clr_msg_list->setEnabled(true);
        ac_click_on_call_show_cty->setEnabled(true);
        //ac_ft_df1500->setEnabled(true);
        ac_filter_list->setEnabled(true);
        ac_areset_qso->setEnabled(true);
        FilterDialog->SetHidFLBtOnOff(false);
        ac_show_timec->setEnabled(true);
        ac_show_counc->setEnabled(true);
        ac_show_distc->setEnabled(true);
        ac_show_freqc->setEnabled(true);
    }
    if (rb_mode[14]->isChecked() || rb_mode[15]->isChecked() || rb_mode[16]->isChecked() || rb_mode[17]->isChecked())
    {   // cps.
        allq65 = true;
        if (rb_mode[14]->isChecked()) s_mode = 14;
        if (rb_mode[15]->isChecked()) s_mode = 15;
        if (rb_mode[16]->isChecked()) s_mode = 16;
        if (rb_mode[17]->isChecked()) s_mode = 17;

        MainDisplay->setArrayInPxel(50);  //1.40 no used in ft4
        SecondDisplay->setArrayInPxel(50);//1.40 no used in ft4

        for (int i = 0; i < 3; ++i) rb_dec_depth[i]->setEnabled(true);
        cb_auto_clr_avg_afdec->setEnabled(true);
        cb_avg_decode->setEnabled(true);
        pb_clear_avgQ65->setHidden(false);

        ac_two_dec_list->setEnabled(true);
        Direct_log_qso->setEnabled(true);
        Prompt_log_qso->setEnabled(true);
        cb_ap_decode->setEnabled(true);
        ac_2click_list_autu_on->setEnabled(true);
        if (!g_block_mam)
        {
            MA_man_adding->setEnabled(true);
            Multi_answer_mod->setEnabled(true);
            Multi_answer_mod_std->setEnabled(true);
        }
        //ac_start_qso_from_tx2_or_tx1->setEnabled(true);//2.61 in SetMactos
        ac_new_dec_clr_msg_list->setEnabled(true);
        ac_click_on_call_show_cty->setEnabled(true);
        //ac_ft_df1500->setEnabled(true);
        ac_filter_list->setEnabled(true);
        ac_areset_qso->setEnabled(true);
        ac_show_counc->setEnabled(true);
    }
    rb_thr[thr_all[s_mode]-1]->setChecked(true); //2.69
    THvRigControl->SetMode(s_mode);//2.16
    cb_ap_decode->setChecked(decoder_ap_all[s_mode]);
    SetDecodeDeeptFromMod(s_mode);
    SB_VDispSpeed->setValue(s_vdisp_all_speed[s_mode]);

    if (s_mode==10)
        pb_tune->setEnabled(false);
    else
        pb_tune->setEnabled(true);

    //SetAutoDecodeAll();
    cb_auto_decode_all->setChecked(auto_decode_all[s_mode]);

    if (dsty) l_mode->setStyleSheet(ModeColorStr(s_mode)+"color:rgb(0,0,0)}");
    else l_mode->setStyleSheet(ModeColorStr(s_mode)+"}");
    l_mode->setText(ModeStr(s_mode));

    TMsPlayerHV->SetModeForWavSaves(s_mode);

    //MainDisplay->SetMode(s_mode);//2.53 need to be here
    //SecondDisplay->SetMode(s_mode);//2.53 need to be here
    THvTxW->ModeChanget(s_mode,vhf_uhf_decode_fac_all[s_mode]);
    //THvMakros->ModeChanget(s_mode);//2.33

    MainDisplay->SetMode(s_mode);//2.41 need to be here
    SecondDisplay->SetMode(s_mode);//2.41 need to be here

    if (s_mode<7 || s_mode==12)// fast msk144-to-jt6m  modes vazno da e tuk sled  MainDisplay SecondDisplay
    {
        SetDispVH(false);
        for (int i = 0; i < 5; ++i) cb_max65_cand[i]->setEnabled(false);
        zero_df_scale_m->setEnabled(false);
        vd_mouse_lines_draw->setEnabled(false);
    }
    else if (s_mode==10) //pi4
    {
        SetDispVH(true);
        for (int i = 0; i < 5; ++i) cb_max65_cand[i]->setEnabled(false);
        zero_df_scale_m->setEnabled(false);
        vd_mouse_lines_draw->setEnabled(false);
    }
    else if (s_mode==11) // ft8
    {
        SetDispVH(true);
        for (int i = 0; i < 5; ++i) cb_max65_cand[i]->setEnabled(false);
        zero_df_scale_m->setEnabled(false);
        vd_mouse_lines_draw->setEnabled(false);
        vd_bw_lines_draw[0]->setEnabled(true);
        vd_bw_lines_draw[1]->setEnabled(true);
        vd_bw_lines_draw[2]->setEnabled(true);
    }
    else if (s_mode==13) // ft4
    {
        SetDispVH(true);
        for (int i = 0; i < 5; ++i) cb_max65_cand[i]->setEnabled(false);
        zero_df_scale_m->setEnabled(false);
        vd_mouse_lines_draw->setEnabled(false);
        vd_bw_lines_draw[0]->setEnabled(true);
        vd_bw_lines_draw[1]->setEnabled(true);
        vd_bw_lines_draw[2]->setEnabled(true);
    }
    else if (s_mode==18) // ft2
    {
        SetDispVH(true);
        for (int i = 0; i < 5; ++i) cb_max65_cand[i]->setEnabled(false);
        zero_df_scale_m->setEnabled(false);
        vd_mouse_lines_draw->setEnabled(false);
        vd_bw_lines_draw[0]->setEnabled(true);
        vd_bw_lines_draw[1]->setEnabled(true);
        vd_bw_lines_draw[2]->setEnabled(true);
    }
    else if (allq65) // q65
    {
        SetDispVH(true);
        for (int i = 0; i < 5; ++i) cb_max65_cand[i]->setEnabled(false);
        zero_df_scale_m->setEnabled(false);
        vd_mouse_lines_draw->setEnabled(false);
        cb_1_dec_sig_q65->setEnabled(true);
        cb_dec_aft_eme_delay->setEnabled(true);
        cb_max_drift->setEnabled(true);
        vd_bw_lines_draw[0]->setEnabled(true);
        vd_bw_lines_draw[1]->setEnabled(true);
        vd_bw_lines_draw[2]->setEnabled(true);
    }
    else
    {
        SetDispVH(true);
        for (int i = 0; i < 5; ++i) cb_max65_cand[i]->setEnabled(true);
        zero_df_scale_m->setEnabled(true);
        vd_mouse_lines_draw->setEnabled(true);
    }

    TMsCore->SetMode(s_mode);
    TDecoderMs->setMode(s_mode);
    //THvMakros->ModeChanget(s_mode);//stop 2.33
    //THvTxW->ModeChanget(s_mode,vhf_uhf_decode_fac_all[s_mode]);//stop 2.33
    cb_vhf_uhf_decode_fac->setChecked(vhf_uhf_decode_fac_all[s_mode]);
    cb_avg_decode->setChecked(avg_dec_all[s_mode]);
    cb_deep_search_decode->setChecked(deep_search_dec_all[s_mode]);
    SetModeDecodeListS(two_dec_list_all[s_mode],s_show_lc[s_mode][0],s_show_lc[s_mode][1],s_show_lc[s_mode][2],s_show_lc[s_mode][3]);
    SetMaxCandidats65Mod(s_mode);
    //ModeMenuStatRefresh(f_de_active);//2.21 no needet for the monemt small exception

    if (!TMsPlayerHV->Is_RealStop()) SetFilePlay();

    FileNameChengedD1();
    FileNameChengedD2();
    RefreshCbCfm73();
    TSettingsMs->SetMode(s_mode);//tci
    W_mod_bt_sw->SetActiveBt(s_mode);//2.74
}
void Main_Ms::PaletteChanged(bool) //2.65
{
    static int id0_only_one = 1;
    for (int i = 0; i < 9; ++i)
    {
        if (rb_palette[i]->isChecked())
        {
            if (id0_only_one == i) break;
            id0_only_one = i;
            MainDisplay->SetPalette(i);
            SecondDisplay->SetPalette(i);
        }
    }
    SaveSettings();
}
void Main_Ms::InDevChanged(QString dev,int bpsampl,int latency, int card_buffer_polls,int channel,int disp_refresh, int lm_refresh)
{	/* QString rate, */
    TMsCore->SetupSettings(dev,bpsampl,latency,card_buffer_polls,channel);//rate,
    MainDisplay->SetDisplayRefr(disp_refresh);
    THvSMeter_H->SetLMRefr(lm_refresh);
    SaveSettings();
}
bool Main_Ms::isFindId(QString id,QString line,QString &res)
{
    bool fres = false;
    QRegExp rx;
    rx.setPattern(id+"=\"?([^\"]*)\"?");
    if (rx.indexIn(line) != -1)
    {
        res = rx.cap(1);
        fres = true;
    }
    return fres;
}
void Main_Ms::SetQActionCb(QString s, bool idp, QAction *ac)//idp priority of procedure
{
    if (s.isEmpty()) return;
    if (idp)
    {
        if (s=="1") ac->setChecked(true);
        else ac->setChecked(false);
    }
    else
    {
        if (s=="0") ac->setChecked(false);
        else ac->setChecked(true);
    }
}
void Main_Ms::Read_Settings(QString path)
{
    const int c_st_id = 110;//92  89
    //dopalva se tuk v kraia
    const QString st_id[c_st_id]=
        {
            "default_w","default_h","default_device_alsa","default_bitpersample","default_card_latency",
            "default_card_buffer_polls","default_in_channel","default_palette","default_scan_refresh",
            "default_lm_refresh","out_dev","default_out_buf","mod_identifaer","com_port_name","com_port_baud",
            "com_port_ptt_rts_dtr","default_rig_name","default_ptt_cat_type","default_read_data_rts_on",
            "com_port_name2","com_port_baud2","com_port_ptt_rts_dtr2","off_set_frq_to_rig","tune_display",
            "tune_display_cont","auto_decode_all","signdb_all","getdftol_all","defaut_zap",
            "default_in_lev_cor","default_out_lev_cor","default_period_time_all","default_txt_mark",
            "default_band","default_mon_startup","default_rtd","def_dec_depth","def_sh_op_all",
            "def_2d1d","def_msk144rxequal","def_swl_op","def_vhf_uhf_decode_fac","def_avg_decode",
            "def_vdisp_speed","def_vdisp_start_freq","def_vdisp_band_width","def_astro_data",
            "def_max65_cand","def_zero_df_scale","def_cust_pall","auto_seq_all","lock_txrx_all",
            "prompt_log_qso","info_dupe_qso","log_qso_startdt_eq_enddt","tx_watchdog_parms",
            "def_ap_decode","def_deep_search_decode","def_vd_mouse_lines_draw","def_desk_size_wh",
            "def_desk_pos_xy","def_flat_vdsp","def_recognize_tp1","def_recognize_tp2","def_aggressive_lev",
            "def_two_dec_list","def_2click_list_autu_on","offset_trsv_rig_parms","multi_answer_mod",
            "def_gbk_t","start_qso_from_tx2_or_tx1","def_app_fonts","default_txt_mark_color",
            "new_dec_clr_msg_list","vd_rx_tx_freq","static_tx_parms","mod_set_frq_to_rig","direct_log_qso",
            "click_on_call_show_cty","def_desk_pos_help_xy","def_desk_pos_astr_xy","def_desk_pos_logw_xywh",
            "net_rig_srv_port","def_confirm73","def_thr_lev","off_auto_comments","def_df1500_Ft","def_dec_3int_Ft",
            "def_filter_list","def_filter_list0","def_filter_list1","def_filter_list2","def_filter_list3",
            "def_filter_list4","def_filter_list5","def_adle_vdsp","def_areset_qso","def_1_dec_sig_q65",
            "def_auto_clr_avg_afdec","def_dec_aft_eme_delay","def_max_drift","def_use_queue_cont","def_filter_list6",
            "use_aseq_max_dist","def_mod_bt_sw","def_show_lcols","vd_bw_lines_draw","def_band_bt_sw",
            "def_show_hide_wf_tx","def_var_dec_parr"
        };

    QString st_res[c_st_id];
    for (int i = 0; i < c_st_id; ++i)
        st_res[i]="";

    QDir testD(path.mid(0,path.count()-12)); // /ms_settings = 12
    if (!testD.exists())
    {
        QMessageBox::critical(nullptr, "MSHV Critical Error",
                              ("MSHV cannot find the required sub-directories and files.\n"
                               "Software is not installed properly.\n"
                               "Please:\n"
                               "1. Close MSHV.\n"
                               "2. Reinstall and use desktop shortcut to stat MSHV." ),
                              QMessageBox::Close);
    }

    block_save = true;

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        rb_mode[2]->setChecked(true);//1.26 HV important set to default mode fsk441
        //v1.30 inportent minimal sizes need to change if changes min sizes
        int t_width = 801;
        int t_height = 680;
        if (screenHeight<=600)
        {
            if (pb_2D_1D->Button_Stop_b)//only if on one display
                pb_2D_1D->ExtrnalRelease();
            t_height = 530;
        }
        resize(t_width, t_height);
        FontDialog->SetDefFont();//2.16 no settings

        block_save = false;
        return;
    }

    QTextStream in(&file);
    while (!in.atEnd())
    {
        QString line = in.readLine();

        if (line=="[ms_settings]") //continue;
            line = in.readLine();//next line

        for (int i = 0; i < c_st_id; ++i)
        {
            if (isFindId(st_id[i],line,st_res[i]))
            {
                //qDebug()<<i<<st_id[i];
                line = in.readLine();
                //qDebug()<<i<<"idfind="<<st_id[i]<<"id="<<st_res[i];
            }
            //else
            //qDebug()<<i<<"FALSE READ --------------------------------  idfind="<<st_id[i];
        }
    }
    file.close();
    //qDebug()<<"1Time="<<ttt.elapsed();//2297 ms   down 2813 ms

    if (!st_res[84].isEmpty())
    {
        int sthr = 1;
        QStringList l0 = st_res[84].split("#");
        bool exeptt = false;
        //remove after 1 year 16 desember 2022
        if (l0.count()==1) //old variant
        {
            sthr = l0.at(0).toInt();
            if (sthr<1) sthr = 1;	//protection
            if (sthr>cthr)
            {
                exeptt = true;
                sthr = cthr;
            }
            if (sthr>6) sthr = 6; //protection
            if (exeptt)
            {
                if (sthr==1)
                {
                    thr_all[11] = 1;
                    thr_all[13] = 1;
                    thr_all[18] = 1;
                }
                else
                {
                    thr_all[11] = sthr - 1;
                    thr_all[13] = sthr - 1;
                    thr_all[18] = sthr - 1;
                }
            }
            else
            {
                thr_all[11] = sthr;
                thr_all[13] = sthr;
                thr_all[18] = sthr;
            }
        }
        //END remove after 1 year 16 desember 2022
        else
        {
            int cmd_0 = l0.count();
            if (cmd_0>COUNT_MODE) cmd_0 = COUNT_MODE; //protection
            for (int i = 0; i<cmd_0; ++i)
            {
                exeptt = false;
                sthr = l0.at(i).toInt();
                if (i!=11 && i!=13 && i!=18) sthr = 1;//only modes with threads
                if (sthr<1) sthr = 1;	//protection
                if (sthr>cthr)
                {
                    exeptt = true;
                    sthr = cthr;
                }
                if (sthr>6) sthr = 6; //protection
                if (exeptt)
                {
                    if 	(sthr==1) thr_all[i] = 1;
                    else thr_all[i] = sthr - 1;
                }
                else thr_all[i] = sthr;
            }
        }
    }

    if (!st_res[69].isEmpty())
    {
        QStringList lt=st_res[69].split("#");
        if (lt.count()>2)
        {
            if (lt.at(0).toInt()==G_UL_K_72)
            {
                g_ub_m_k = true;
                g_block_mam = false;
            }
            else g_ub_m_k = false;
            if (lt.at(1).toInt()==G_UL_K_73)
            {
                g_ub_m_k2 = true;
                THvTxW->SetGUbMK(true);
            }
            else g_ub_m_k2 = false;
            if (lt.at(2).toInt()==G_UL_K_74) g_ub_m_k3 = true;
            else g_ub_m_k3 = false;
        }
    }

    if (!st_res[74].isEmpty()) MainDisplay->SetFreqRxTxVD(st_res[74]);//vd_rx_tx_freq

    if (!st_res[68].isEmpty())
    {
        QStringList lt=st_res[68].split("#");
        lt<<"0"<<"0";
        if (lt.at(0) == "1" && !g_block_mam) Multi_answer_mod->setChecked(true);
        else if (lt.at(0) == "2" && !g_block_mam) Multi_answer_mod_std->setChecked(true);
        else
        {
            Multi_answer_mod->setChecked(false);
            Multi_answer_mod_std->setChecked(false);
        }
        SetQActionCb(lt.at(1),true,MA_man_adding);//def false
    }

    if (!st_res[67].isEmpty()) THvRigControl->SetOffsetTrsvRigParms(st_res[67]);
    if (!st_res[82].isEmpty()) THvRigControl->SetNetRigSrvPort(st_res[82]);

    if (!st_res[33].isEmpty())
    {
        if (st_res[33].toInt()>=0 || st_res[33].toInt()<COUNT_BANDS)
        {
            ListBands.at(st_res[33].toInt())->setChecked(true);
        }
    }

    if (!st_res[31].isEmpty()) THvTxW->SetPiriodTimeAllModes(st_res[31]);

    if (!st_res[32].isEmpty()) THvTxtColor->SetTextMark(st_res[32]);
    if (!st_res[72].isEmpty()) THvTxtColor->SetTextColor(st_res[72]);

    if (!st_res[38].isEmpty())
    {
        if (st_res[38].toInt() == 0) pb_2D_1D->ExtrnalRelease();
    }

    if (screenHeight<=600)
    {
        if (pb_2D_1D->Button_Stop_b) pb_2D_1D->ExtrnalRelease();//only if on one display
    }
    if (!st_res[1].isEmpty() && !st_res[0].isEmpty())
    {
        if (st_res[1] == "FULL" || st_res[0] == "FULL" ) setWindowState(Qt::WindowMaximized);
        else
        {
            int t_width = st_res[0].toInt();
            int t_height = st_res[1].toInt();
            if (t_width>screenWidth) t_width=screenWidth-50;// 50 borders
            if (t_height>screenHeight) t_height=screenHeight-100;// 100 2 times taskbar
            resize(t_width, t_height);
        }
    }
    else
    {
        int t_width = 801;
        int t_height = 680;
        if (screenHeight<=600) t_height = 530;
        resize(t_width, t_height);
    }
    if (!st_res[59].isEmpty() && !st_res[60].isEmpty())
    {
        QStringList list_wh = st_res[59].split("#");
        QStringList list_xy = st_res[60].split("#");
        if (list_wh.count()==2 && list_xy.count()==2)
        {
            int w_sz = list_wh[0].toInt();
            int h_sz = list_wh[1].toInt();
            if (w_sz==desktopw->width() && h_sz==desktopw->height())
            {
                int x_pos1 = list_xy[0].toInt();
                int y_pos1 = list_xy[1].toInt();
                move(x_pos1,y_pos1);
                f_is_moved_to_prev_desk_pos = true;
                if (!st_res[79].isEmpty()) THvHelpMs->SetPosXY(st_res[79]);
                if (!st_res[80].isEmpty()) THvTxW->SetAstroWPos(st_res[80]);
                if (!st_res[81].isEmpty()) THvTxW->SetLogWPosWH(st_res[81]);
            }
        }
    }

    if (!st_res[7].isEmpty())
    {
        bool found_p = false; //2.65
        for (int i = 0; i < 9; ++i)
        {
            if (st_res[7].toInt()==i)
            {
                rb_palette[i]->setChecked(true);
                MainDisplay->SetPalette(i);
                SecondDisplay->SetPalette(i);
                found_p = true;
                break;
            }
        }
        if (!found_p)
        {
            rb_palette[1]->setChecked(true);
            MainDisplay->SetPalette(1);
            SecondDisplay->SetPalette(1);
        }
    }

    if (!st_res[26].isEmpty()) THvTxW->SetMinsigndb(st_res[26]);
    if (!st_res[27].isEmpty()) THvTxW->SetDftolerance(st_res[27]);
    if (!st_res[28].isEmpty()) THvTxW->SetZap(st_res[28]);

    //1.61= CAT ot tuk se refre6va CAT only ont time
    if (!st_res[76].isEmpty()) THvRigControl->SetModSetFrqToRig(st_res[76]);
    if (!st_res[22].isEmpty()) THvRigControl->SetOffSetFrqToRig(st_res[22]);
    if (!st_res[18].isEmpty()) THvRigControl->SetupReadDataRtsOn(st_res[18]);
    if (!st_res[15].isEmpty()) THvRigControl->SetupPtt(st_res[15]);
    if (!st_res[13].isEmpty()) THvRigControl->SetPortName(st_res[13]);
    if (!st_res[14].isEmpty()) THvRigControl->SetPortBaud(st_res[14]);
    if (!st_res[17].isEmpty()) THvRigControl->SetPttCatType(st_res[17]);//tova da e parvo vazno
    if (!st_res[16].isEmpty()) THvRigControl->SetRigName(st_res[16]);
    if (!st_res[55].isEmpty()) THvRigControl->SetTxWatchdogParms(st_res[55]);
    if (!st_res[21].isEmpty()) THvRigControl->SetupPtt2(st_res[21]);
    if (!st_res[19].isEmpty()) THvRigControl->SetPortName2(st_res[19]);
    if (!st_res[20].isEmpty()) THvRigControl->SetPortBaud2(st_res[20]);
    //1.61= CAT ot tuk se refre6va CAT only ont time

    if (!st_res[23].isEmpty() && !st_res[24].isEmpty())
    {
        Slider_Tune_Disp->SetValue(st_res[23].toInt());
        Slider_Cont_Disp->SetValue(st_res[24].toInt());
    }

    if (!st_res[2].isEmpty() && !st_res[3].isEmpty()
            && !st_res[4].isEmpty() && !st_res[5].isEmpty() && !st_res[8].isEmpty()
            && !st_res[10].isEmpty() && !st_res[11].isEmpty() && !st_res[6].isEmpty())
    {
        if (st_res[9].isEmpty()) st_res[9]="2";
        TSettingsMs->SetDevices_Drv(st_res[2],st_res[3],/*st_res[3], rate = not used*/
                                    st_res[4],st_res[5],st_res[6],st_res[8],
                                    st_res[9],st_res[10],st_res[11]);
    }

    if (!st_res[29].isEmpty()) THvTxW->SetInLevel(st_res[29]);
    if (!st_res[30].isEmpty()) THvTxW->SetOutLevel(st_res[30]);

    if (!st_res[39].isEmpty())
    {
        for (int i = 0; i < 4; ++i)
        {
            if (st_res[39].toInt()==i)
            {
                cb_msk144rxequal_[i]->setChecked(true);
                break;
            }
        }
    }

    if (!st_res[35].isEmpty())
    {
        if (st_res[35].toInt()==0) cb_rtd_decode->setChecked(false);
        else cb_rtd_decode->setChecked(true);
    }

    //0=vhf_hf 1=avg 2=deeps 3=ap 4=set_two_dec_lists 5=auto_dec_all
    if (!st_res[25].isEmpty()) SetDecodeAllSettingsB(st_res[25],5);

    if (!st_res[40].isEmpty()) THvTxW->SetSwlOp(st_res[40]);
    if (!st_res[37].isEmpty()) THvTxW->SetOpAllB(st_res[37],0);//sh=0
    if (!st_res[50].isEmpty()) THvTxW->SetAutoSeqAll(st_res[50]);
    if (!st_res[51].isEmpty()) THvTxW->SetOpAllB(st_res[51],1);//lock_txrx=1

    if (!st_res[43].isEmpty()) SetAllSettingsI(st_res[43],0);// 0=vdist_speads
    if (!st_res[44].isEmpty()) SB_VDispStartFreq->setValue(st_res[44].toInt());
    if (!st_res[45].isEmpty()) SB_VDispBandwidth->setValue(st_res[45].toInt());

    if (!st_res[61].isEmpty())
    {
        if (st_res[61]=="1") cb_flat_dsp->setChecked(true);
        else cb_flat_dsp->setChecked(false);
    }
    if (!st_res[95].isEmpty())
    {
        if (st_res[95]=="0") cb_adle_dsp->setChecked(false);
        else cb_adle_dsp->setChecked(true);
    }

    if (!st_res[65].isEmpty()) SetDecodeAllSettingsB(st_res[65],4);//0=vhf_hf 1=avg 2=deeps 3=ap 4=set_two_dec_lists
    if (!st_res[41].isEmpty()) SetDecodeAllSettingsB(st_res[41],0);//0=vhf_hf 1=avg 2=deeps 3=ap
    if (!st_res[57].isEmpty()) SetDecodeAllSettingsB(st_res[57],2);//0=vhf_hf 1=avg 2=deeps 3=ap
    if (!st_res[42].isEmpty()) SetDecodeAllSettingsB(st_res[42],1);//0=vhf_hf 1=avg 2=deeps 3=ap
    if (!st_res[56].isEmpty()) SetDecodeAllSettingsB(st_res[56],3);//0=vhf_hf 1=avg 2=deeps 3=ap
    if (!st_res[47].isEmpty()) SetAllSettingsI(st_res[47],2);// 2=max_cand65
    if (!st_res[36].isEmpty()) SetAllSettingsI(st_res[36],1);//1=decoder_depth_all

    //SetQActionCb(st_res[86], false, ac_ft_df1500);//2.67.1  SetQActionCb(st_res[86], true, ac_ft_df1500);//2.41
    SetQActionCb(st_res[96], false, ac_areset_qso);//2.49

    if (!st_res[105].isEmpty())
    {
        QStringList l = st_res[105].split("|");
        int cm = l.count();
        if (cm > COUNT_MODE) cm = COUNT_MODE;
        for (int i = 0; i < cm; ++i)
        {
            QStringList l0 = l.at(i).split("#");
            int cc = l0.count();
            if (cc > 4) cc = 4;
            for (int j = 0; j < cc; ++j) s_show_lc[i][j] = (bool)l0.at(j).toInt();
        }
    }

    if (!st_res[12].isEmpty())
    {
        int idm = st_res[12].toInt();
        s_mode = idm;
        if (idm<COUNT_MODE) rb_mode[idm]->setChecked(true);
        else
        {
            s_mode = 2;
            rb_mode[2]->setChecked(true);
        }
    }
    else
    {
        s_mode = 2;
        rb_mode[2]->setChecked(true);
    }

    if (!st_res[89].isEmpty()) FilterDialog->SetSettings0(st_res[89]);//2.44
    if (!st_res[90].isEmpty()) FilterDialog->SetSettings1(st_res[90]);//2.44
    if (!st_res[91].isEmpty()) FilterDialog->SetSettings2(st_res[91]);//2.44
    if (!st_res[92].isEmpty()) FilterDialog->SetSettings3(st_res[92]);//2.44
    if (!st_res[93].isEmpty()) FilterDialog->SetSettings4(st_res[93]);//2.44
    if (!st_res[94].isEmpty()) FilterDialog->SetSettings5(st_res[94]);//2.44
    if (!st_res[102].isEmpty()) FilterDialog->SetSettings6(st_res[102]);//2.62
    if (!st_res[88].isEmpty()) FilterDialog->SetSettings(st_res[88]);//2.43

    if (!st_res[75].isEmpty())//static_tx_parms
        THvRigControl->SetStaticTxParms(st_res[75]);
    if (!st_res[34].isEmpty())
    {
        if (st_res[34] == "1")
        {
            Mon_start_m->setChecked(true);
            StartRxGlobal();
        }
        else Mon_start_m->setChecked(false);
    }
    if (!st_res[46].isEmpty())
    {
        if (st_res[46].toInt()==1) is_active_astro_w = true;
    }
    SetQActionCb(st_res[48], true, zero_df_scale_m);
    SetQActionCb(st_res[58], true, vd_mouse_lines_draw);
    if (!st_res[49].isEmpty()) TCustomPalW->SetPalSettings(st_res[49]);
    SetQActionCb(st_res[77], false, Direct_log_qso);// 2.51 default checked
    SetQActionCb(st_res[52], true, Prompt_log_qso);
    SetQActionCb(st_res[53], false, Info_dupe_qso);// 2.51 default checked
    SetQActionCb(st_res[54], true, Log_qso_startdt_eq_enddt);
    SetQActionCb(st_res[62], false, recognize_tp1);
    SetQActionCb(st_res[63], false, recognize_tp2);
    if (!st_res[64].isEmpty()) TAggressiveDialog->SetAggresLevels(st_res[64]);
    SetQActionCb(st_res[66], false, ac_2click_list_autu_on);// 2.51 default checked
    SetQActionCb(st_res[70], true, ac_start_qso_from_tx2_or_tx1);
    if (!st_res[71].isEmpty()) FontDialog->SetFontsAll(st_res[71]);
    SetQActionCb(st_res[73], true, ac_new_dec_clr_msg_list);
    SetQActionCb(st_res[78], false, ac_click_on_call_show_cty);// 2.51 default checked
    SetQActionCb(st_res[83], false, ac_Cfm73);
    SetQActionCb(st_res[85], true, Log_auto_comm);
    SetQActionCb(st_res[87], false, cb_3intFt_d);//2.51 default checked
    SetQActionCb(st_res[97], true, cb_1_dec_sig_q65);
    SetQActionCb(st_res[98], true, cb_auto_clr_avg_afdec);
    SetQActionCb(st_res[99], true, cb_dec_aft_eme_delay);
    SetQActionCb(st_res[100], true, cb_max_drift);//2.57 def false
    SetQActionCb(st_res[101], true, ac_use_queue_cont);//2.59 def false
    SetQActionCb(st_res[103], true, ac_aseq_max_dist);//2.66 def false
    if (!st_res[104].isEmpty()) W_mod_bt_sw->SetSettings(st_res[104]);
    if (!st_res[106].isEmpty())
    {
        QStringList l = st_res[106].split("#");
        if (l.count()>2)
        {
            SetQActionCb(l.at(0), true,  vd_bw_lines_draw[0]);
            SetQActionCb(l.at(1), false, vd_bw_lines_draw[1]);//2.76.1 SetQActionCb(l.at(1), true, vd_bw_lines_draw[1]);
            SetQActionCb(l.at(2), false, vd_bw_lines_draw[2]);//2.76.1 SetQActionCb(l.at(2), true, vd_bw_lines_draw[2]);
        }
    }
    if (!st_res[107].isEmpty()) W_band_bt_sw->SetSettings(st_res[107]);
    if (!st_res[108].isEmpty())
    {
        QStringList l = st_res[108].split("#");
        if (l.count()>1)
        {
            SetQActionCb(l.at(0),false,sh_wf);//2.76.5 def true if (l.at(0)=="0") sh_wf->setChecked(false);
            SetQActionCb(l.at(1),false,sh_tx);//2.76.5 def true if (l.at(1)=="0") sh_tx->setChecked(false);
        }
    }
    if (!st_res[109].isEmpty())//def_var_dec_parr=1#1#1
    {
        QStringList l = st_res[109].split("#");
        if (l.count()>2)
        {
            SetQActionCb(l.at(0),true,cb_UseVarDecFt);//default false
            if 		(l.at(1)=="0") rb_vdec_cyc[0]->setChecked(true);
            else if (l.at(1)=="1") rb_vdec_cyc[1]->setChecked(true);
            else if (l.at(1)=="2") rb_vdec_cyc[2]->setChecked(true);
            if 		(l.at(2)=="0") rb_vdec_sens[0]->setChecked(true);
            else if (l.at(2)=="1") rb_vdec_sens[1]->setChecked(true);
            else if (l.at(2)=="2") rb_vdec_sens[2]->setChecked(true);
        }
    }

    THvTxW->SetBlockEmitFreqToRig(false);

    //qDebug()<<"W_MAIN START";//<<ttt.elapsed();//2813 ms new 513ms
    block_save = false;
}
void Main_Ms::SaveSettings()
{
    if (!block_save) Save_Settings(App_Path+"/settings/ms_settings");
}
void Main_Ms::Save_Settings(QString path)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;
    QTextStream out(&file);
    out << "[ms_settings]" << "\n";

    if (windowState() == Qt::WindowMaximized)
    {
        out << "default_w=" << "FULL" << "\n";
        out << "default_h=" << "FULL" << "\n";
    }
    else
    {
        out << "default_w=" << QString("%1").arg(this->width()) << "\n";
        out << "default_h=" << QString("%1").arg(this->height()) << "\n";
    }
    out << "default_device_alsa=" << TSettingsMs->default_in_dev() << "\n";
    out << "default_bitpersample=" << TSettingsMs->default_bitpersample() << "\n";//out << "default_sample_rate=" << "48000" << "\n";//free TSettingsMs->default_sample_rate() << "\n";
    out << "default_card_latency=" << TSettingsMs->default_card_latency() << "\n";
    out << "default_card_buffer_polls=" << TSettingsMs->default_card_buffer_polls() << "\n";
    out << "default_in_channel=" << TSettingsMs->default_in_channel() << "\n";
    out << "default_palette=" << MainDisplay->default_color() << "\n";
    out << "default_scan_refresh=" << TSettingsMs->default_scan_refresh() << "\n";
    out << "default_lm_refresh=" << TSettingsMs->default_lm_refresh() << "\n";
    out << "default_out_dev=" << TSettingsMs->default_out_dev() << "\n";
    out << "default_out_buf=" << TSettingsMs->default_out_buf() << "\n";
    out << "mod_identifaer=" <<  QString("%1").arg(s_mode) << "\n";

    out << "com_port_name=" <<  THvRigControl->get_port_name() << "\n";
    out << "com_port_baud=" <<  THvRigControl->get_port_baud() << "\n";
    out << "com_port_ptt_rts_dtr=" <<  THvRigControl->get_ptt_rts_dtr() << "\n";
    out << "default_rig_name=" <<  THvRigControl->get_rig_name() << "\n";
    out << "default_ptt_cat_type=" <<  THvRigControl->get_ptt_cat_type() << "\n";
    out << "default_read_data_rts_on=" <<  THvRigControl->get_read_data_rts_on() << "\n";

    out << "com_port_name2=" <<  THvRigControl->get_port_name2() << "\n";
    out << "com_port_baud2=" <<  THvRigControl->get_port_baud2() << "\n";
    out << "com_port_ptt_rts_dtr2=" <<  THvRigControl->get_ptt_rts_dtr2() << "\n";
    out << "off_set_frq_to_rig=" <<  THvRigControl->off_set_frq_to_rig() << "\n";

    //out << "tx_fs=" << QString("%1").arg(THvTxW->GetTxFs()) << "\n";
    out << "tune_display=" << QString("%1").arg(Slider_Tune_Disp->get_value()) << "\n";
    out << "tune_display_cont=" << QString("%1").arg(Slider_Cont_Disp->get_value()) << "\n";

    out << "auto_decode_all=" << get_dec_settings_allB(5) << "\n";//0=vhf_hf 1=avg 2=deeps 3=ap 4=set_two_dec_lists 5=auto_dec_all

    out << "signdb_all=" << THvTxW->getsigndb() << "\n";
    out << "getdftol_all=" << THvTxW->getdftol() << "\n";
    out << "defaut_zap=" << THvTxW->getzap() << "\n";
    out << "default_in_lev_cor=" << THvTxW->default_in_lev_cor() << "\n";
    out << "default_out_lev_cor=" << THvTxW->default_out_lev_cor() << "\n";
    out << "default_period_time_all=" << THvTxW->def_period_time_all_modes() << "\n";
    out << "default_txt_mark=" << THvTxtColor->GetTextMark() << "\n";
    ///out << "default_mon_r=" << THvTxW->get_mon_r12 << "\n";

    int ik=0;
    for (ik = 0; ik<COUNT_BANDS; ++ik)
    {
        //QRadioButton *bt = (QRadioButton*)Vl_band->itemAt(ik)->widget();
        if (ListBands.at(ik)->isChecked())
            break;
    }
    out << "default_band=" <<  QString("%1").arg(ik) << "\n";

    out << "default_mon_startup=" << QString("%1").arg(Mon_start_m->isChecked()) << "\n";
    out << "default_rtd=" << QString("%1").arg(cb_rtd_decode->isChecked()) << "\n";

    out << "def_dec_depth=" << get_settings_allI(1) << "\n";
    out << "def_sh_op_all=" << THvTxW->get_op_allB(0) << "\n";

    out << "def_2d1d=" << QString("%1").arg(pb_2D_1D->Button_Stop_b) << "\n";

    QString dd = "0";
    for (int i = 1; i < 4; ++i)
    {
        if (cb_msk144rxequal_[i]->isChecked())
        {
            dd = QString("%1").arg(i);
            break;
        }
    }
    out << "def_msk144rxequal=" << dd << "\n";
    out << "def_swl_op=" << THvTxW->get_swl_op() << "\n";
    out << "def_vhf_uhf_decode_fac=" << get_dec_settings_allB(0) << "\n";//0=vhf_hf 1=avg 2=deeps 3=ap
    out << "def_avg_decode=" << get_dec_settings_allB(1) << "\n";//0=vhf_hf 1=avg 2=deeps 3=ap

    out << "def_vdisp_speed=" << get_settings_allI(0) << "\n";// 0=vdist_speads
    out << "def_vdisp_start_freq=" << QString("%1").arg(SB_VDispStartFreq->value()) << "\n";
    out << "def_vdisp_band_width=" << QString("%1").arg(SB_VDispBandwidth->value()) << "\n";
    out << "def_astro_data=" << QString("%1").arg(is_active_astro_w) << "\n";

    out << "def_max65_cand=" << get_settings_allI(2) << "\n";// 2=max_cand65
    out << "def_zero_df_scale=" << QString("%1").arg(zero_df_scale_m->isChecked()) << "\n";
    out << "def_cust_pall=" << TCustomPalW->GetPalSettings() << "\n";

    out << "auto_seq_all=" << THvTxW->getautoseq_all() << "\n";
    out << "lock_txrx_all=" << THvTxW->get_op_allB(1) << "\n";

    out << "prompt_log_qso=" << THvTxW->GetPromptLogQso() << "\n";
    out << "info_dupe_qso=" << QString("%1").arg(Info_dupe_qso->isChecked()) << "\n";
    out << "log_qso_startdt_eq_enddt=" << QString("%1").arg(Log_qso_startdt_eq_enddt->isChecked()) << "\n";

    out << "tx_watchdog_parms=" << THvRigControl->get_tx_watchdog_parms() << "\n";
    out << "def_ap_decode=" << get_dec_settings_allB(3) << "\n";//0=vhf_hf 1=avg 2=deeps 3=ap
    out << "def_deep_search_decode=" << get_dec_settings_allB(2) << "\n";//0=vhf_hf 1=avg 2=deeps 3=ap
    out << "def_vd_mouse_lines_draw=" << QString("%1").arg(vd_mouse_lines_draw->isChecked()) << "\n";
    /// 1.51
    out << "def_desk_size_wh=" << QString("%1").arg(desktopw->width())<<"#"<<QString("%1").arg(desktopw->height()) << "\n";
    out << "def_desk_pos_xy=" << QString("%1").arg(pos().x())<<"#"<<QString("%1").arg(pos().y()) << "\n";
    out << "def_flat_vdsp=" << QString("%1").arg(cb_flat_dsp->isChecked()) << "\n";

    out << "def_recognize_tp1=" << QString("%1").arg(recognize_tp1->isChecked()) << "\n";
    out << "def_recognize_tp2=" << QString("%1").arg(recognize_tp2->isChecked()) << "\n";

    out << "def_aggressive_lev=" << TAggressiveDialog->GetAggresLevels() << "\n";
    out << "def_two_dec_list=" << get_dec_settings_allB(4) << "\n";//4=set_two_dec_lists
    out << "def_2click_list_autu_on=" << QString("%1").arg(ac_2click_list_autu_on->isChecked()) << "\n";
    out << "offset_trsv_rig_parms=" << THvRigControl->get_offset_trsv_rig_parms() << "\n";

    QString mass = "0";
    if (Multi_answer_mod->isChecked()) 			mass = "1";
    else if (Multi_answer_mod_std->isChecked()) mass = "2";
    out << "multi_answer_mod=" << mass <<"#"<< QString("%1").arg(MA_man_adding->isChecked()) << "\n";
    int gbk_tt  = 0xa;
    int gbk_tt2 = 0xf;
    int gbk_tt3 = 0xd;
    if (g_ub_m_k) gbk_tt   = G_UL_K_72;
    if (g_ub_m_k2) gbk_tt2 = G_UL_K_73;
    if (g_ub_m_k3) gbk_tt3 = G_UL_K_74;
    out << "def_gbk_t="<<QString("%1").arg(gbk_tt)<<"#"<<QString("%1").arg(gbk_tt2)<<"#"<<QString("%1").arg(gbk_tt3)<<"#"<<QString("%1").arg(0xe)<<"\n";
    out << "start_qso_from_tx2_or_tx1=" << QString("%1").arg(ac_start_qso_from_tx2_or_tx1->isChecked()) << "\n";
    out << "def_app_fonts=" << FontDialog->GetFontsAll() << "\n";
    out << "default_txt_mark_color=" << THvTxtColor->GetTextColor() << "\n";
    out << "new_dec_clr_msg_list=" << QString("%1").arg(ac_new_dec_clr_msg_list->isChecked()) << "\n";
    out << "vd_rx_tx_freq=" << MainDisplay->GetFreqRxTxVD() << "\n";
    out << "static_tx_parms=" << THvRigControl->static_tx_parms() << "\n";
    out << "mod_set_frq_to_rig=" <<  THvRigControl->mod_set_frq_to_rig() << "\n";
    out << "direct_log_qso=" << THvTxW->GetDirectLogQso() << "\n";
    out << "click_on_call_show_cty=" << QString("%1").arg(ac_click_on_call_show_cty->isChecked()) << "\n";
    out << "def_desk_pos_help_xy=" << THvHelpMs->GetPosXY() << "\n";
    out << "def_desk_pos_astr_xy=" << THvTxW->GetAstroWPos() << "\n";
    out << "def_desk_pos_logw_xywh=" << THvTxW->GetLogWPosWH() << "\n";
    out << "net_rig_srv_port=" <<  THvRigControl->net_rig_srv_port() << "\n";
    out << "def_confirm73=" <<  QString("%1").arg(ac_Cfm73->isChecked()) << "\n";
    dd.clear(); //2.69
    for (int i = 0; i < COUNT_MODE; ++i)
    {
        dd.append(QString("%1").arg(thr_all[i]));
        if (i<COUNT_MODE-1) dd.append("#");
    }
    out << "def_thr_lev=" << dd << "\n";

    out << "off_auto_comments=" << QString("%1").arg(Log_auto_comm->isChecked()) << "\n";
    out << "def_df1500_Ft=" << "1"/*QString("%1").arg(ac_ft_df1500->isChecked())*/ << "\n";
    out << "def_dec_3int_Ft=" << QString("%1").arg(cb_3intFt_d->isChecked()) << "\n";//2.39 remm
    out << "def_filter_list=" << FilterDialog->GetSettings() << "\n";
    out << "def_filter_list0=" << FilterDialog->GetSettings0() << "\n";
    out << "def_filter_list1=" << FilterDialog->GetSettings1() << "\n";
    out << "def_filter_list2=" << FilterDialog->GetSettings2() << "\n";
    out << "def_filter_list3=" << FilterDialog->GetSettings3() << "\n";
    out << "def_filter_list4=" << FilterDialog->GetSettings4() << "\n";
    out << "def_filter_list5=" << FilterDialog->GetSettings5() << "\n";
    out << "def_adle_vdsp=" << QString("%1").arg(cb_adle_dsp->isChecked()) << "\n";
    out << "def_areset_qso=" << QString("%1").arg(ac_areset_qso->isChecked()) << "\n";

    out << "def_1_dec_sig_q65=" << QString("%1").arg(cb_1_dec_sig_q65->isChecked()) << "\n";
    out << "def_auto_clr_avg_afdec=" << QString("%1").arg(cb_auto_clr_avg_afdec->isChecked()) << "\n";
    out << "def_dec_aft_eme_delay=" << QString("%1").arg(cb_dec_aft_eme_delay->isChecked()) << "\n";
    out << "def_max_drift=" << QString("%1").arg(cb_max_drift->isChecked()) << "\n";
    out << "def_use_queue_cont=" << QString("%1").arg(ac_use_queue_cont->isChecked()) << "\n";
    out << "def_filter_list6=" << FilterDialog->GetSettings6() << "\n";
    out << "use_aseq_max_dist=" << QString("%1").arg(ac_aseq_max_dist->isChecked()) << "\n";
    out << "def_mod_bt_sw=" << W_mod_bt_sw->GetSettings() << "\n";
    dd.clear();
    for (int i = 0; i < COUNT_MODE; ++i)
    {
        for (int j = 0; j < 4; ++j)
        {
            dd.append(QString("%1").arg(s_show_lc[i][j]));
            if (j<4-1) dd.append("#");
        }
        if (i<COUNT_MODE-1) dd.append("|");
    }
    out << "def_show_lcols=" << dd << "\n";
    out << "vd_bw_lines_draw=" << QString("%1").arg(vd_bw_lines_draw[0]->isChecked()) << "#"
    << QString("%1").arg(vd_bw_lines_draw[1]->isChecked()) << "#"
    << QString("%1").arg(vd_bw_lines_draw[2]->isChecked()) << "\n";
    out << "def_band_bt_sw=" << W_band_bt_sw->GetSettings() << "\n";
    out << "def_show_hide_wf_tx="<<QString("%1").arg(sh_wf->isChecked())<<"#"<<QString("%1").arg(sh_tx->isChecked())<<"\n";

    dd = QString("%1").arg(cb_UseVarDecFt->isChecked());
    dd.append("#");
    for (int i = 0; i < 3; ++i)
    {
        if (rb_vdec_cyc[i]->isChecked())
        {
            dd.append(QString("%1").arg(i));
            break;
        }
    }
    dd.append("#");
    for (int i = 0; i < 3; ++i)
    {
        if (rb_vdec_sens[i]->isChecked())
        {
            dd.append(QString("%1").arg(i));
            break;
        }
    }
    out << "def_var_dec_parr=" << dd << "\n";

    file.close();
}
void Main_Ms::SetBS1Text(QString s)
{
    if (f_disp_v_h)
    {
        if (s.isEmpty())
            pb_save_disp1->setText(tr("SAVE THIS"));
        else
            pb_save_disp1->setText(s);
    }
    else
    {
        if (s.isEmpty())
            pb_save_disp1->setText(tr("SAVE DISPLAY")+" 1");
        else
            pb_save_disp1->setText(s);
    }
}
void Main_Ms::SetBS2Text(QString s)
{
    if (f_disp_v_h)
    {
        if (s.isEmpty())
            pb_save_disp2->setText(tr("SAVE PREVIOUS"));
        else
            pb_save_disp2->setText(s);
    }
    else
    {
        if (s.isEmpty())
            pb_save_disp2->setText(tr("SAVE DISPLAY")+" 2");
        else
            pb_save_disp2->setText(s);
    }
}
void Main_Ms::FileNameChengedD1()
{
    if (TMsCore->GetSta_Sto())
        SetBS1Text("");//pb_save_disp1->setText("SAVE DISPLAY 1");
}
void Main_Ms::FileNameChengedD2()
{
    SetBS2Text("");//pb_save_disp2->setText("SAVE DISPLAY 2");
}
void Main_Ms::SaveFileDisplay1()
{
    //QString mode = "UNKNOWN";
    QString call = "NOCALL";

    if (!THvTxW->get_Calls().isEmpty())
        call = THvTxW->get_Calls();

    if (call.contains("/"))
    {
        if (call.at(call.count()-1)=='/')
            call.replace("/","_SLASH");
        else
            call.replace("/","_SLASH_");
    }

    MainDisplay->SaveFile(call+"_"+ModeStr(s_mode));
    SetBS1Text(tr("File Name")+": "+MainDisplay->getFullFileName());//pb_save_disp1->setText("SAVE DISPLAY 1 File Name: "+MainDisplay->getFullFileName());
}
void Main_Ms::SaveFileDisplay2()
{
    //QString mode = "UNKNOWN";
    QString call = "NOCALL";

    if (!THvTxW->get_Calls().isEmpty()) call = THvTxW->get_Calls();

    if (call.contains("/"))
    {
        if (call.at(call.count()-1)=='/') call.replace("/","_SLASH");
        else call.replace("/","_SLASH_");
    }

    SecondDisplay->SaveFile(call+"_"+ModeStr(s_mode));
    SetBS2Text(tr("File Name")+": "+SecondDisplay->getFullFileName());//pb_save_disp2->setText("SAVE DISPLAY 2 File Name: "+SecondDisplay->getFullFileName());
}
void Main_Ms::Open()
{
    QString path = App_Path+"/RxWavs";
    /*QFileDialog dialog(this,tr("Open"), path+"/", tr("Files WAV (*.wav)"));
    dialog.setLabelText(QFileDialog::Reject, tr("Eaioae"));
    dialog.setLabelText(QFileDialog::Accept, tr("iia?i"));
    QString fileName; 
    if (dialog.exec()) fileName =  dialog.selectedFiles()[0];*/
    QString fileName = QFileDialog::getOpenFileName(this,tr("Open"), path+"/", tr("Files")+" WAV (*.wav)");
    FileOpen(fileName);
}
void Main_Ms::FileOpen(QString fileName)
{
    if (fileName.isEmpty()) return;
    QString path_rem_space = fileName.replace("%20", " ").toUtf8();//qt4.4.0 zaradi " i kirilica
    QFileInfo fi(path_rem_space);
    QString name = fi.baseName().trimmed();//2.76sf
    QString time_t;
    int to;

    if (name.count()<6) to = name.count();
    else to = 6;
    for (int i = name.count()-1; i>=name.count()-to; i--) time_t.insert(0,name.at(i));

    TDecoderMs->SetCalsHashFileOpen(name);    //qDebug()<<name<<time_t;
    MainDisplay->setDisplayTime(time_t,"REWRITED");// for jt65 v1.35 and 2.43
    //TMsPlayerHV->OpenFile(qstrdup(qPrintable(path_rem_space)));
    int c_fn=path_rem_space.count()+128;
    char *c_msg_sent = new char[c_fn];//2.50 same is in genmesage_ms.cpp
    strncpy(c_msg_sent,path_rem_space.toUtf8(),c_fn); //printf("%s\n",c_msg_sent);
    TMsPlayerHV->OpenFile(c_msg_sent); //qDebug()<<sizeof(c_msg_sent)/sizeof(char)<<strlen(c_msg_sent)<<c_fn<<c_msg_sent;
    //MainDisplay->setDisplayTime(time_t,"REWRITED");//rem for jt65 v1.35
    SetBS1Text(tr("Open File")+": "+fi.fileName());//pb_save_disp1->setText("SAVE DISPLAY 1 Open File: "+fi.fileName());
    delete [] c_msg_sent;

    bool res = true;//2.76 res=1 (even first), or res=0 (odd second)
    if (time_t.count()>5 && s_mode==11)
    {
        uint8_t time_ss =  time_t.midRef(4,2).toInt();//get seconds 120023
        uint8_t time_mm =  time_t.midRef(2,2).toInt();//get min 120023
        int time_p = ((int)time_mm*60)+time_ss; //int ntrperiod = 15;
        time_p = time_p % (15*2); //j=mod(nutc/5,2)
        if (time_p<15) res = true;
        else res = false;
        //uint8_t time_ss =  time_t.midRef(4,2).toInt();
        //uint8_t time_mm =  time_t.midRef(2,2).toInt();
        //float time_p = ((int)time_mm*60)+time_ss;
        //if (time_ss == 7 || time_ss == 22 || time_ss == 37 || time_ss == 52) time_p += 0.5;
        //time_p = fmod(time_p,period_time_sec*2.0);
        //if (time_p<period_time_sec) res = true;
        //else res = false;
    } //qDebug()<<time_t<<(int)res<<period_time_sec;
    MainDisplay->DecodeAllData(true,true,res);
}
#if defined _WIN32_
//////////// WINDOWS MIXERS ////////////////////////////////////////////
float Main_Ms::GetOSVersion()
{
    float str = 0.0;
    int osmj = 0;
    int osmi = 0;
    int osbn = 0;
    int ospi = 0;

    //other method
    /*typedef NTSTATUS (WINAPI fRtlGetVersion) (PRTL_OSVERSIONINFOEXW);
     	OSVERSIONINFOEXW osverinfo;
     	fRtlGetVersion *RtlGetVersion1;
     	RtlGetVersion1 = (fRtlGetVersion *)GetProcAddress (GetModuleHandleA("ntdll"),"RtlGetVersion");
     	memset (&osverinfo, 0, sizeof (OSVERSIONINFOEXW));
     	osverinfo.dwOSVersionInfoSize = sizeof (OSVERSIONINFOEXW);
     	RtlGetVersion1 (&osverinfo);*/

    //NTSTATUS(WINAPI *RtlGetVersion)(LPOSVERSIONINFOEXW);//64bit warning: dereferencing type-punned pointer will break strict-aliasing rules [-Wstrict-aliasing]
    typedef ULONG (WINAPI *RtlGetVersionFn)(LPOSVERSIONINFOEXW);
    RtlGetVersionFn RtlGetVersion;
    OSVERSIONINFOEXW osvi_new;
    //*(FARPROC*)&RtlGetVersion = GetProcAddress(GetModuleHandleA("ntdll"),"RtlGetVersion");//64bit warning: dereferencing type-punned pointer will break strict-aliasing rules [-Wstrict-aliasing]
    RtlGetVersion = (RtlGetVersionFn)(GetProcAddress(GetModuleHandleA("ntdll"),"RtlGetVersion"));
    if (NULL != RtlGetVersion)
    {
        osvi_new.dwOSVersionInfoSize = sizeof(osvi_new);
        RtlGetVersion(&osvi_new);
        osmj = osvi_new.dwMajorVersion;
        osmi = osvi_new.dwMinorVersion;
        osbn = osvi_new.dwBuildNumber;
        ospi = osvi_new.dwPlatformId;
    }

    //old method to 2.71
    /*OSVERSIONINFOEX osvi_old;
    SYSTEM_INFO si;
    PGNSI pGNSI;
    BOOL bOsVersionInfoEx;
    ZeroMemory(&si, sizeof(SYSTEM_INFO));
    ZeroMemory(&osvi_old, sizeof(OSVERSIONINFOEX));
    osvi_old.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    if (!(bOsVersionInfoEx = GetVersionEx((OSVERSIONINFO *)&osvi_old))) return str;
    pGNSI = (PGNSI) GetProcAddress(GetModuleHandle((WCHAR*)TEXT("kernel32.dll")),"GetNativeSystemInfo");
    if (NULL != pGNSI) pGNSI(&si);
    else GetSystemInfo(&si);
    //end old method to 2.71
    if (osmj==0 || ospi!=VER_PLATFORM_WIN32_NT)
    {
        osmj = osvi_old.dwMajorVersion;
        osmi = osvi_old.dwMinorVersion;
        osbn = osvi_old.dwBuildNumber;
        ospi = osvi_old.dwPlatformId;    
    }*/

    if (VER_PLATFORM_WIN32_NT==ospi && osmj > 4)
    {
        if (osmj >= 10)//osInfo.dwBuildNumber >= 22000
        {
            //24780<-12.0 ???
            if (osbn >= 22000) str = 11.0;//"Windows11.0";
            else str = 10.0;//"Windows10.0";
        }
        if (osmj == 6)
        {
            if (osmi == 0) str = 6.0;//"Vista";
            if (osmi == 1) str = 6.1;//"Windows7";
            if (osmi == 2) str = 6.2;//"Windows8";
            if (osmi == 3) str = 6.3;//"Windows8.1";
        }
        if (osmj == 5 && osmi == 2) str = 5.2;//"ServersWin";
        if (osmj == 5 && osmi == 1) str = 5.1;//"XP";
        if (osmj == 5 && osmi == 0) str = 5.0;//"Win2000";
    }//else str = "0.0";//"No Windows";
    //qDebug()<<"NEW="<<osvi_new.dwMajorVersion<<osvi_new.dwMinorVersion<<osvi_new.dwBuildNumber<<VER_PLATFORM_WIN32_NT<<osvi_new.dwPlatformId;
    //qDebug()<<"OLD="<<osvi_old.dwMajorVersion<<osvi_old.dwMinorVersion<<osvi_old.dwBuildNumber<<VER_PLATFORM_WIN32_NT<<osvi_old.dwPlatformId;
    //qDebug()<<"RES="<<osmj<<osmi<<osbn<<VER_PLATFORM_WIN32_NT<<ospi<<"Win="<<str;
    return str;
}
void Main_Ms::WMixRec()
{
    if (Start_Cmd)
    {
        Start_Cmd->close();
        Start_Cmd->kill();
    }
    Start_Cmd->setProcessChannelMode(QProcess::MergedChannels);
    float str = GetOSVersion();
    if (str >= 6.0)
        Start_Cmd->start("control.exe mmsys.cpl,,1");//recordev-> control.exe mmsys.cpl,,1 //SndVol.exe /r
    else
        Start_Cmd->start("sndvol32.exe -r");
    connect(Start_Cmd, SIGNAL(finished(int)), this, SLOT(End_Cmd(int)));
    //vista windows7 Playback Devices: control.exe mmsys.cpl,,0
    //vista windows7 Recording Devices: control.exe mmsys.cpl,,1
    //Win Xp Recording Devices: sndvol32.exe -r
    //Win Xp Playback Devices: sndvol32.exe -p
}
void Main_Ms::WMixPlay()
{
    if (Start_Cmd)
    {
        Start_Cmd->close();
        Start_Cmd->kill();
    }
    Start_Cmd->setProcessChannelMode(QProcess::MergedChannels);
    float str = GetOSVersion();
    if (str >= 6.0)
        Start_Cmd->start("control.exe mmsys.cpl,,0");//recordev-> control.exe mmsys.cpl,,1 //SndVol.exe /p
    else
        Start_Cmd->start("sndvol32.exe -p");
    connect(Start_Cmd, SIGNAL(finished(int)), this, SLOT(End_Cmd(int)));
    //vista windows7 Playback Devices: control.exe mmsys.cpl,,0
    //vista windows7 Recording Devices: control.exe mmsys.cpl,,1
    //Win Xp Recording Devices: sndvol32.exe -r
    //Win Xp Playback Devices: sndvol32.exe -p
}
void Main_Ms::SyncTime()
{
    if (Start_Cmd)
    {
        Start_Cmd->close();
        Start_Cmd->kill();
    }
    Start_Cmd->setProcessChannelMode(QProcess::MergedChannels);
    Start_Cmd->start("control.exe timedate.cpl,,0");//"W32tm /resync" RUNDLL32 SHELL32.DLL,Control_RunDLL %1,@0
    //Start_Cmd->start("http://time.is/");
    //QDesktopServices::openUrl(QUrl("http://time.is/", QUrl::TolerantMode));
    QMessageBox::information(this, "MSHV",
                             (tr("Windows Xp:\n"
                                 "1.Click on the Internet Time tab.\n"
                                 "2.Click on the Update Now button.\n"
                                 "Windows 7,8,8.1,10:\n"
                                 "1.Click on the Internet Time tab.\n"
                                 "2.Click on the Change Settings button.\n"
                                 "3.Click on the Update Now button.")),
                             QMessageBox::Close);

    //QDesktopServices::openUrl(QUrl("http://time.is/", QUrl::TolerantMode));
    connect(Start_Cmd, SIGNAL(finished(int)), this, SLOT(End_Cmd(int)));
}
void Main_Ms::End_Cmd(int)
{   //qDebug()<<"dsfsdds";
    QString cmd_err = Start_Cmd->readAllStandardError();
    QString cmd_output = Start_Cmd->readAllStandardOutput();

    if (!cmd_output.isEmpty())
        QMessageBox::critical(this, "MSHV",
                              ("CMD Error\n"+cmd_err+"\n"+cmd_output),
                              QMessageBox::Close);
    disconnect(Start_Cmd, SIGNAL(finished(int)), this, SLOT(End_Cmd(int)));
}
#endif

void Main_Ms::OnlineTimeCheck()
{
    //Online timecheck
    QDesktopServices::openUrl(QUrl("http://time.is/", QUrl::TolerantMode));
}
int Main_Ms::titleBarHeight()
{
    QStyleOptionTitleBar options;
    options.initFrom(this);

    return this->style()->pixelMetric(
               QStyle::PM_TitleBarHeight,
               &options,
               this)-2;//-4 remove 4pix up
}
/*void Main_Ms::Screenshot()//to old 2.55
{
    //QWidget *w = QApplication::activeWindow();
    QDesktopWidget dw;
    QWidget *w=dw.screen(dw.screenNumber(this));
    if (w)
    {
        QString call = "NOCALL";

        if (!THvTxW->getMy_Call().isEmpty())
            call = THvTxW->getMy_Call();

        if (call.contains("/"))
        {
            if (call.at(call.count()-1)=='/')
                call.replace("/","_SLASH");
            else
                call.replace("/","_SLASH_");
        }

        QString str = getDateTime().toString("_yyMMdd_hhmmss");

        //QPixmap p = QPixmap::grabWidget(w);//samo widgeta
        //QPixmap p = QPixmap::grabWindow(QApplication::desktop()->winId()); // full desktop

        QRect rect=geometry();
        int tbh=titleBarHeight()+12;//qt5 +12
        QPixmap p;

        QScreen *screen = QGuiApplication::primaryScreen();

        if (const QWindow *window = windowHandle())
            screen = window->screen();
        if (!screen)
            return;

        p=screen->grabWindow(
              w->winId(),
              rect.x()-2,//-2 border left
              rect.y()-tbh,
              rect.width()+4,//-4 border right
              rect.height()+tbh+2);//-2 border down

        p.save(QString(App_Path+"/Screenshots/"+call+str+".png"));
    }
}*/
void Main_Ms::Screenshot()//new 2.56
{
    QString call = "NOCALL";

    if (!THvTxW->getMy_Call().isEmpty()) call = THvTxW->getMy_Call();

    if (call.contains("/"))
    {
        if (call.at(call.count()-1)=='/')
            call.replace("/","_SLASH");
        else
            call.replace("/","_SLASH_");
    }

    QString str = getDateTime().toString("_yyMMdd_hhmmss");

    //QPixmap p = QPixmap::grabWidget(w);//samo widgeta
    //QPixmap p = QPixmap::grabWindow(QApplication::desktop()->winId()); // full desktop

    QRect rect=geometry();
    int tbh=titleBarHeight()+13;//qt5 +12
    QPixmap p;

    QDesktopWidget dw;
    QScreen* screen = QGuiApplication::screens()[dw.screenNumber(this)];
    /*  tested no needed for the moment 2.61
    #if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
        QScreen* screen0 = QGuiApplication::screenAt(this->mapToGlobal({this->width()/2,0}));
    #else
        QDesktopWidget dw;
        QScreen* screen = QGuiApplication::screens()[dw.screenNumber(this->mapToGlobal({this->width()/2,0}))];
    #endif
    */
    if (!screen) return;

    if (const QWindow *window = windowHandle()) screen = window->screen();
    if (!screen) return;

    p=screen->grabWindow(
          0,
          rect.x()-1,//-2 border left
          rect.y()-tbh,
          rect.width()+2,//-4 border right
          rect.height()+tbh+1);//-2 border down

    p.save(QString(App_Path+"/Screenshots/"+call+str+".png"));
}
void Main_Ms::keyPressEvent(QKeyEvent* event)
{
    /*    Qt::Key_NumberSign-># Qt::Key_Bar->| Qt::Key_Semicolon->;
    if (event->key() == Qt::Key_Space || event->key() == Qt::Key_NumberSign
            || event->key() == Qt::Key_Bar || event->key() == Qt::Key_Semicolon)
        return;*/
    //qDebug()<<QString("0x%1").arg(event->key(), 8, 16, QLatin1Char( '0' ));
    switch (event->key())
    {
    case Qt::Key_Escape:
        StopTxGlobal();
        break;
    case Qt::Key_F1:
        THvTxW->SetTxMessage(0);
        break;
    case Qt::Key_F2:
        THvTxW->SetTxMessage(1);
        break;
    case Qt::Key_F3:
        THvTxW->SetTxMessage(2);
        break;
    case Qt::Key_F4:
        THvTxW->SetTxMessage(3);
        break;
    case Qt::Key_F5:
        THvTxW->SetTxMessage(4);
        break;
    case Qt::Key_F6:
        THvTxW->SetTxMessage(5);
        break;
    case Qt::Key_F7:
        THvTxW->SetTxMessage(6);
        break;
    case Qt::Key_F9:
        THvTxW->SetRxOnlyFiSe();
        break;
    case Qt::Key_F10:
        if (!Box_dspl->isHidden())
        {
            pb_2D_1D->ExtrnalRelease(); //qDebug()<<"F10";
        }
        break;
    case Qt::Key_F11:
        //if (!Box_dspl->isHidden())
        //{
        if (!pb_2D_1D->Button_Stop_b && !f_disp_v_h)//only if on one display and horizontal display
            pb_D1_D2->ExtrnalRelease(); //qDebug()<<"F11";
        //}
        break;
    case Qt::Key_Z:
        if (event->modifiers()==Qt::ControlModifier)//ControlModifier
            THvTxW->StartStopZap();
        break;
    case Qt::Key_A:
        if (event->modifiers()==Qt::ControlModifier)
            THvTxW->auto_on();
        break;
    case Qt::Key_G:
        if (event->modifiers()==Qt::ControlModifier)
            THvTxW->gen_msg();
        break;
    case Qt::Key_1:
        if (event->modifiers()==Qt::ControlModifier)
            SaveFileDisplay1();
        break;
    case Qt::Key_2:
        if (event->modifiers()==Qt::ControlModifier)
            SaveFileDisplay2();
        break;
    case Qt::Key_L:
        if (event->modifiers()==Qt::AltModifier)
            THvTxW->AddToLogButton();
        break;
    case Qt::Key_M:
        if (event->modifiers()==Qt::AltModifier)
            StartRxGlobal();
        break;
    case Qt::Key_S:
        if (event->modifiers()==Qt::AltModifier)
            StopRxGlobal();
        break;
    case Qt::Key_C:
        if (event->modifiers()==Qt::AltModifier)
            TDecodeList1->Clear_List();
        break;
    case Qt::Key_F12:
        Screenshot();
        break;
    case Qt::Key_F8:
        THvTxW->StartEmptySpotDialog();
        break;
    case Qt::Key_QuoteLeft:  //US  //Qt::Key_Apostrophe  Qt::Key_W  Qt::Key_QuoteLeft;->`~  || Qt::Key_AsciiCircum ^
    case Qt::Key_AsciiCircum://German
    case Qt::Key_Q:		     //MA Standard
        if (event->modifiers()==Qt::ControlModifier && (s_mode==11 || s_mode==13 || s_mode==18 || allq65) && !g_block_mam)//"Multi Answering Auto Sequence Protocol FT8"
        {
            if (event->key()==Qt::Key_QuoteLeft || event->key()==Qt::Key_AsciiCircum)//MA DXpedition
            {
                if (Multi_answer_mod->isChecked())
                    Multi_answer_mod->setChecked(false);
                else
                    Multi_answer_mod->setChecked(true);
            }
            if (event->key()==Qt::Key_Q)//MA Standard
            {
                if (Multi_answer_mod_std->isChecked())
                    Multi_answer_mod_std->setChecked(false);
                else
                    Multi_answer_mod_std->setChecked(true);
            }
        }
        if (event->modifiers()==Qt::ControlModifier && (s_mode==11 || s_mode==13 || s_mode==18 || allq65) && g_block_mam)
        {
            QString text = tr("Multi Answering Auto Seq Protocol\n"
                              "can be used only in Standard Activity Type\n"
                              "Go to Options Macros and correct.");
            QMessageBox::warning(this, "MSHV", text, QMessageBox::Ok);
        }
        break;
    case Qt::Key_Period:
        if (event->modifiers()==Qt::ControlModifier)
            THvTxW->ExpandShrinkDf(true);//2.05 +
        break;
    case Qt::Key_Comma:
        if (event->modifiers()==Qt::ControlModifier)
            THvTxW->ExpandShrinkDf(false);//2.05 -
        break;
    case Qt::Key_Equal://0x0000003d:
        if (event->modifiers()==Qt::ControlModifier)
            SkUpDownBandChanged(true);//Band UP
        break;
    case Qt::Key_Minus:
        if (event->modifiers()==Qt::ControlModifier)
            SkUpDownBandChanged(false);//Band DOWN
        break;
        /*case Qt::Key_F:
            if (event->modifiers()==Qt::AltModifier && (s_mode==11 || s_mode==13 || s_mode==18))
                FilterDialog->SetOnOff();//Filter on/off
            break;*/
    default:
        QWidget::keyPressEvent(event);
    }
    //qDebug()<<"KeyHex="<<QString::number(event->key(),16);
}
