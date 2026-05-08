/* MSHV TxWidget
 * Copyright 2015 Hrisimir Hristov, LZ2HV
 * May be used under the terms of the GNU General Public License (GPL)
 */
#define _BANDS_H_
#define _FREQTOBAND_H_
#include "hvtxw.h"
#include "../config_str_color.h"
#include "config_rpt_all.h"
//#include <QtGui>

HvLabAutoSeq::HvLabAutoSeq(bool f,QWidget * parent )
        : QLabel(parent)
{
    dsty = f;
    setFrameStyle(QFrame::Panel | QFrame::Sunken);
    setAlignment(Qt::AlignCenter);
    setFixedHeight(19);
    setText("ASeq");

    for (int i =0; i<COUNT_MODE; i++) s_autoseq[i] = false;
    s_autoseq[0]  = true; //msk144 2.15  default
    s_autoseq[11] = true; //ft8 2.15  default
    s_autoseq[12] = true; //mskms 2.15  default
    s_autoseq[13] = true; //ft4 2.15  default
    s_autoseq[14] = true; //q65a  default
    s_autoseq[15] = true; //q65b  default
    s_autoseq[16] = true; //q65c  default
    s_autoseq[17] = true; //q65d  default
    s_autoseq[18] = true; //ft2   default

    s_mode = 2;//fsk441

    /*QString sss;
    for (int i = 0; i<COUNT_BANDS; ++i)
    {
    	sss =  "{"+QString("%1").arg(freq_min_max[i].min-2000)+", ";
    	sss += QString("%1").arg(freq_min_max[i].max+2000)+"},"; 
    	qDebug()<<sss;  	
    }*/
}
HvLabAutoSeq::~HvLabAutoSeq()
{}
void HvLabAutoSeq::SetAutoSeqMode(int mod, bool f_enab)
{
    s_mode = mod;

    if (f_enab)
    {
        setEnabled(true);
        if (s_autoseq[s_mode])
        {
            if (dsty) setStyleSheet("QLabel{background-color:rgb(150,0,0);}");
            else setStyleSheet("QLabel{background-color :rgb(255,0,0);}");
        }
        else
            setStyleSheet("QLabel{background-color:palette(Button);}");
    }
    else
    {
        if (dsty) setStyleSheet("QLabel{background-color:rgb(165,120,120);color:rgb(50,50,50);}");
        else setStyleSheet("QLabel{background-color:rgb(255,210,210);color:rgb(50,50,50);}");
        setEnabled(false);
    }
    emit EmitLabAutoSeqPress();
}
bool HvLabAutoSeq::GetAutoSeq()
{
    return s_autoseq[s_mode];
}
void HvLabAutoSeq::SetAutoSeqAll(QString s)
{
    QStringList ls=s.split("#");

    for (int i=0; i<COUNT_MODE; i++)
    {
        QString tstr = ModeStr(i)+"=";
        for (int j=0; j<ls.count(); j++)
        {
            if (ls[j].contains(tstr))
            {
                ls[j].remove(tstr);
                if (!ls[j].isEmpty())
                    s_autoseq[i] = ls[j].toInt();
                break;
            }
        }
    }
}
void HvLabAutoSeq::mousePressEvent(QMouseEvent *)
{
    if (s_autoseq[s_mode])
        s_autoseq[s_mode]=false;
    else
        s_autoseq[s_mode]=true;

    if (s_autoseq[s_mode])
    {
        if (dsty) setStyleSheet("QLabel{background-color:rgb(150,0,0);}");
        else setStyleSheet("QLabel{background-color:rgb(255,0,0);}");
    }
    else
    {
        setStyleSheet("QLabel{background-color:palette(Button);}");
    }
    emit EmitLabAutoSeqPress();
}
HvLabRxOnlyFiSe::HvLabRxOnlyFiSe(QWidget * parent )
        : QLabel(parent)
{
    //setAlignment(Qt::AlignCenter);
    setFrameStyle(QFrame::Panel | QFrame::Sunken);
    setAlignment(Qt::AlignCenter);
}
HvLabRxOnlyFiSe::~HvLabRxOnlyFiSe()
{}
void HvLabRxOnlyFiSe::mousePressEvent( QMouseEvent *)
{
    emit EmitLabPress();
}

HvTxIn::HvTxIn(int ident,bool f,QWidget * parent)
        : QWidget(parent)
{
    dsty = f;
    s_ident = ident;
    line_txt = new HvLeWithSpace();
    //line_txt->setContextMenuPolicy(Qt::NoContextMenu);// no rithclick menu
    line_txt->setMaxLength(36);
    rb_tx = new QRadioButton();
    connect(rb_tx, SIGNAL(clicked(bool)), this, SLOT(rb_clicked()));
    connect(rb_tx, SIGNAL(toggled(bool)), this, SLOT(rb_toggled(bool)));
    b_tx= new QPushButton();
    b_tx->setFixedWidth(32);
    b_tx->setFixedHeight(21);//2.15 from 20 to 21
    connect(b_tx, SIGNAL(clicked(bool)), this, SLOT(b_released()));
    QHBoxLayout *H_h = new QHBoxLayout(this);
    H_h->setContentsMargins ( 2, 0, 1, 0);
    H_h->setSpacing(4);//2.15 from 4 to
    H_h->addWidget(line_txt);
    H_h->addWidget(rb_tx);
    H_h->addWidget(b_tx);
    setLayout(H_h);
}
HvTxIn::~HvTxIn()
{}
void HvTxIn::SetFont(QFont f)
{
    rb_tx->setFont(f);
    line_txt->setFont(f);
    QFontMetrics fm(f);//2.15
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)//2.56
    int fwdd = fm.horizontalAdvance("TX8")+8;
#else
    int fwdd = fm.width("TX8")+8;
#endif
    b_tx->setFixedWidth(fwdd);
}
void HvTxIn::SetEnabledRbBtTx(bool f)
{
    rb_tx->setEnabled(f);
    b_tx->setEnabled(f);
    setFocus(); //no select line edit
}
void HvTxIn::SetEnabledBtTx(bool f)
{
    b_tx->setEnabled(f);
    setFocus(); //no select line edit
}
void HvTxIn::SetReadOnly(bool f)
{
    line_txt->setReadOnly(f);
    SetBackground();
}
void HvTxIn::rb_toggled(bool /*f*/)
{
    SetBackground();
}
void HvTxIn::SetBackground()
{
    if (rb_tx->isChecked())
    {
        if (line_txt->isReadOnly())
        {
            if (dsty) line_txt->setStyleSheet("QLineEdit{background-color:rgb(55,10,10);color:rgb(220,220,255)}");
            else line_txt->setStyleSheet("QLineEdit{background-color:"+ColorStr_[4]+";color:"+ColorStr_[5]+"}");
        }
        else
        {
            if (dsty) line_txt->setStyleSheet("QLineEdit{background-color:rgb(55,10,10)}");
            else line_txt->setStyleSheet("QLineEdit{background-color:"+ColorStr_[4]+"}");
        }
    }
    else
    {
        if (line_txt->isReadOnly())
        {
            if (dsty) line_txt->setStyleSheet("QLineEdit{background-color:palette(Base);color:rgb(220,220,255)}");
            else line_txt->setStyleSheet("QLineEdit{background-color:palette(Base);color:"+ColorStr_[5]+"}");
        }
        else
        {
            line_txt->setStyleSheet("QLineEdit{background-color:palette(Base)}");
        }
    }
}
void HvTxIn::rb_clicked()
{
    emit EmitRbPress(s_ident);
}
void HvTxIn::b_released()
{
    emit EmitBReleased(s_ident,line_txt->text());
}
//////////////////////////////////////////////////////////////////////

TWDialog::TWDialog(QWidget *m,QWidget *n,QWidget *r,QWidget * parent)
        : QDialog(parent)
{
    setMinimumWidth(520);//510 480 
    //setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum);
    setWindowTitle("MSHV");
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    TW = new QTabWidget();
    QVBoxLayout *V_dwt = new QVBoxLayout();
    V_dwt->setContentsMargins(0,0,0,0);
    V_dwt->setSpacing(0);
    V_dwt->addWidget(TW);
    setLayout(V_dwt);
    TW->addTab(m,m->windowTitle());
    TW->addTab(n,n->windowTitle());
    TW->addTab(r,r->windowTitle());
    connect(TW,SIGNAL(tabBarClicked(int)),m,SLOT(TwTabClicked(int)));
}
TWDialog::~TWDialog()
{}
void TWDialog::SetExec(int i)
{
    TW->setCurrentIndex(i);
    exec();
}

#define _CONT_NAME_
#include "../config_str_con.h"

//#undef _LAMBDA_H_
//#undef _BCNBAND_H_
//#undef _BANDTOFREQ_H_
//#define _BANDS_H_
//#define _FREQTOBAND_H_
//#include "../config_band_all.h"

#undef _SHKY_H_
#define _MOUNTH_H_
#include "../config_str_sk.h"
//#include <QCommonStyle>

HvTxW::HvTxW(QString inst,QString path,int lid,bool f,int x,int y,QWidget * parent )
        : QWidget(parent)
{
    dsty = f;
    AppPath = path;
    l_mam_hiscals_.clear();
    id_mshf = 10;//2.76

    THvMakros = new HvMakros(AppPath,dsty,this);
    connect(THvMakros,SIGNAL(EmitRptRsq(bool)),this,SLOT(SetRptRsq(bool)));
    connect(THvMakros,SIGNAL(EmitMacros(QStringList,int,QString,QString)),this,SLOT(SetMacros(QStringList,int,QString,QString)));
    connect(THvMakros,SIGNAL(EmitDistUnit(bool)),this,SLOT(SetDistUnit(bool)));

    TRadioAndNetW = new RadioAndNetW(inst,AppPath,dsty,x,y,this);
    connect(TRadioAndNetW,SIGNAL(FindLocFromDB(QString)),this,SLOT(ExternalFindLocFromDB(QString)));
    connect(TRadioAndNetW,SIGNAL(EmitUdpCmdDl(QStringList)),this,SIGNAL(EmitUdpCmdDl(QStringList)));
    connect(TRadioAndNetW,SIGNAL(EmitUdpCmdStop(bool)),this,SIGNAL(EmitUdpCmdStop(bool)));
    connect(TRadioAndNetW,SIGNAL(EmitOpenRadNetWToRecon()),this,SLOT(NetW_exec()));
    connect(TRadioAndNetW,SIGNAL(EmitOtpTxKey(QString)),this,SIGNAL(EmitOtpTxKey(QString)));//2.76sf
    connect(TRadioAndNetW,SIGNAL(EmitOtpRxMsg(bool)),this,SIGNAL(EmitOtpRxMsg(bool)));//2.76sf
    connect(TRadioAndNetW,SIGNAL(EmitOtpVerif(QString,uint8_t)),this,SIGNAL(EmitOtpVerif(QString,uint8_t)));//2.76sf

    TWD = new TWDialog(THvMakros,TRadioAndNetW,TRadioAndNetW->GetRadListW(),this);
    connect(TWD,SIGNAL(EmitClose()),THvMakros,SLOT(SetClose()));

    slid = lid;
    allq65 = false;
    alljt65 = false;
    s_mark_r12_pos = 0;
    //s_mark_myc_pos = 0;
    s_mark_hisc_pos = 0;

    f_off_auto_comm = false;
    s_last_call_for_BDB = "_";//full reset
    f_cfm73 = true; //default ft8/4
    sf_cfm73 = true;//default ft8/4
    f_tx_rx = false;
    f_areset_qso = true;//2.49 default
    fpsk_restrict = false;

    f_mod_set_frq_to_rig = true;//2.53 by default true
    s_start_qso_from_tx2 = false;
    f_multi_answer_mod = false;
    f_multi_answer_mod_std = false;
    f_block_emit_freq_to_rig = true;
    log_qso_startdt_eq_enddt = false;
    f_recognize_tp1 = true;
    f_recognize_tp2 = true;

    s_bvhf_jt65 = false;//1.60= def fsk441

    //TX watchdog
    s_PrevTxRpt = "_";
    s_last_txwatchdog_time = 0;
    s_last_txwatchdog_coun = 0;
    s_txwatchdog_time = 20*60;//default=20min
    s_txwatchdog_coun = 10;//default=10 perions;
    f_no_time_count_txwatchdog = 0;//default=1 0=off 1=time 2=count;
    //TX watchdog

    direct_log_qso = true;//2.51 default
    prompt_log_qso = false;
    info_dupe_qso = true;//2.51 default
    for (int i =0; i<COUNT_MODE; i++) s_locktxrx[i]=false;
    s_locktxrx[11]=true; //2.51 ft8 default
    s_locktxrx[13]=true; //2.51 ft4 default

    is_LastTxRptCq = false;
    count_73_auto_seq = 0;
    one_addtolog_auto_seq = false;
    f_add_to_log_started = false;
    is_locked_tx_rst_msk_aseq = false;//2.59

    THvAstroDataW = new HvAstroDataW(x,y);
    connect(THvAstroDataW, SIGNAL(EmitAstroWIsClosed()), this, SIGNAL(EmitAstroWIsClosed()));
    connect(THvAstroDataW, SIGNAL(EmitAstroData(double,double,int,double)),this,SLOT(SetAstroData(double,double,int,double)));

    g_no_block_tx = 101;//101=noblock 100=blockall 6=blocktx7 1=blocktx2
    g_ub_m_k = false;
    prev_frest_ = false;
    //s_msk144_contest_mode = false;
    TRadioAndNetW->GetFtFr(_ftfr_);
    s_cont_type = 0; // default
    s_cont_id = 0;
    s_my_contest_exch = "";// default
    s_his_contest_exch ="";// default
    s_his_contest_sn   ="";// default
    f_cont5_ru_dx = false;// default
    s_trmN = "0";// default
    max_snr_psk_rep = -200;
    hisCall_pskrep = "";
    hisLoc_pskrep = "";
    prev_max_snr_psk_rep = -200;
    prev_hisCall_pskrep = "";
    prev_hisLoc_pskrep = "";
    prev_mode_pskrep = "";
    //s_freq_offset = 0;

    f_rx_only_fi_se = false;
    list_macros<<""<<""<<""<<""<<""<<""<<""<<""<<""<<""<<""<<""<<""<<"";
    s_my_base_call = "_NONE_";
    s_last_bccall_tolog_excp = "NONE_";
    //s_my_call_is_std = true;
    f_cntest_only_sdtc = false;
    f_two_no_sdtc = false;

    for (int i = 0; i < TEX_MARK_C; i++)
        s_txt_mark[i]=true;// inportet default all is marked HV 1.26 here and in hvtxtcolor.cpp

    block_mon_call1 = true;
    block_mon_call2 = true;
    block_loc = true;
    block_call = true;

    s_f_rpt_rsq = false;
    f_auto_on = false;
    f_km_mi = false;//km
    s_dist_points = -1;
    f_aseqmaxdist = false;

    s_log_time_now = "00:00";
    QDateTime utc_t = QDateTime::currentDateTimeUtc();
    s_log_data_start=utc_t.toString("yyyyMMdd");
    s_log_time_start=utc_t.toString("hh:mm");
    s_last_call_for_log = "NO_CALL";

    TMsDb = new MsDb(AppPath);
    connect(TMsDb, SIGNAL(Emit65DeepSearchDb(QStringList)), this, SIGNAL(Emit65DeepSearchDb(QStringList)));

    V_l = new QVBoxLayout();
    V_l->setContentsMargins(1, 0, 1, 0); //2.15 (1, 1, 1, 1);
    V_l->setSpacing(0);//2.15 1 to 0

    QPixmap p0,p1,p2,p7,p8;
    if (dsty)
    {
        p0 = QPixmap(":pic/big/sld_track_v_si.png");
        p1 = QPixmap(":pic/big/sld_up_v_si.png");
        p2 = QPixmap(":pic/big/sld_down_v_si.png");
        p7 = QPixmap(":pic/big/tumb_v_si_p.png");
        p8 = QPixmap(":pic/big/tumb_over_v_si_p.png");
    }
    else
    {
        p0 = QPixmap(":pic/bw/sl_v_track.png");
        p1 = QPixmap(":pic/bw/sl_v_up.png");
        p2 = QPixmap(":pic/bw/sl_v_down.png");
        p7 = QPixmap(":pic/bw/tumb_v_play.png");
        p8 = QPixmap(":pic/bw/tumb_v_over_play.png");
    }
    Slider_Tx_level = new HvSlider_V_Identif(0,100,0,0,p1,p0,p2,p7,p8);
    Slider_Tx_level->SetValue(100);
    QLabel *l_txt = new QLabel("TX");
    l_trxmx = new QLabel("MAX");//dB
    if (dsty) l_trxmx->setStyleSheet("QLabel{color:rgb(255,200,200);}");
    else l_trxmx->setStyleSheet("QLabel{color:rgb(150,0,0);}");// font: 7pt;
    l_trxmi = new QLabel("MIN");
    if (dsty) l_trxmi->setStyleSheet("QLabel{color:rgb(220,220,255);}");
    else l_trxmi->setStyleSheet("QLabel{color:rgb(0,0,200);}");

    QVBoxLayout *V_tx_1 = new QVBoxLayout();
    V_tx_1->setContentsMargins ( 0, 0, 0, 0);
    V_tx_1->setSpacing(0);
    V_tx_1->addWidget(l_txt);
    V_tx_1->setAlignment(l_txt,Qt::AlignCenter);
    V_tx_1->addWidget(l_trxmx);
    V_tx_1->setAlignment(l_trxmx,Qt::AlignCenter);
    V_tx_1->addWidget(Slider_Tx_level);
    V_tx_1->setAlignment(Slider_Tx_level,Qt::AlignCenter);
    V_tx_1->addWidget(l_trxmi);
    V_tx_1->setAlignment(l_trxmi,Qt::AlignCenter);
    V_tx_1->setAlignment(Qt::AlignVCenter);
    connect(Slider_Tx_level, SIGNAL(SendValue(int,int)), this, SLOT(StndOutLevel_s(int,int)));
    for (int i = 0; i<COUNT_BANDS; ++i) s_tx_level[i]=95;//2.54

    QPixmap p9,p10;
    if (dsty)
    {
        p9 = QPixmap(":pic/big/tumb_v_si_c.png");
        p10 = QPixmap(":pic/big/tumb_over_v_si_c.png");
    }
    else
    {
        p9 = QPixmap(":pic/bw/tumb_v_rec.png");
        p10 = QPixmap(":pic/bw/tumb_v_over_rec.png");
    }
    Slider_Rx_level = new HvSlider_V_Identif(0,100,0,0,p1,p0,p2,p9,p10);
    Slider_Rx_level->SetValue(50);
    QLabel *l_trx = new QLabel("RX");
    l_trxdp = new QLabel("+20");//+20dB 1.78
    l_trxdm = new QLabel("-20");//-20dB 1.78
    if (dsty) l_trxdp->setStyleSheet("QLabel{color:rgb(255,200,200);}");
    else l_trxdp->setStyleSheet("QLabel{color:rgb(150,0,0);}");
    if (dsty) l_trxdm->setStyleSheet("QLabel{color:rgb(220,220,255);}");
    else l_trxdm->setStyleSheet("QLabel{color:rgb(0,0,200);}");

    QVBoxLayout *V_rx_1 = new QVBoxLayout();
    V_rx_1->setContentsMargins (0, 0, 0, 0);
    V_rx_1->setSpacing(0);
    V_rx_1->addWidget(l_trx);
    V_rx_1->setAlignment(l_trx,Qt::AlignCenter);
    V_rx_1->addWidget(l_trxdp);
    V_rx_1->setAlignment(l_trxdp,Qt::AlignCenter);
    V_rx_1->addWidget(Slider_Rx_level);
    V_rx_1->setAlignment(Slider_Rx_level,Qt::AlignCenter);
    V_rx_1->addWidget(l_trxdm);
    V_rx_1->setAlignment(l_trxdm,Qt::AlignCenter);
    V_rx_1->setAlignment(Qt::AlignVCenter);
    connect(Slider_Rx_level, SIGNAL(SendValue(int,int)), this, SLOT(StndInLevel_s(int,int)));

    count_tx_widget = 7;

    for (int i = 0; i<count_tx_widget; i++)
    {
        HvTxIn *TempHvTxIn = new HvTxIn(i,dsty);
        TempHvTxIn->b_tx->setText("Tx"+QString("%1").arg(i+1));
        V_l->addWidget(TempHvTxIn);

        if (i==count_tx_widget-2)
        {
            TempHvTxIn->rb_tx->setChecked(true);
            s_b_identif = i;
        }

        connect(TempHvTxIn, SIGNAL(EmitRbPress(int)), this, SLOT(RbPress(int)));
        connect(TempHvTxIn, SIGNAL(EmitBReleased(int,QString)), this, SLOT(BReleased(int,QString)));
    }

    MultiAnswerMod = new MultiAnswerModW(dsty);
    MultiAnswerMod->setHidden(true);//by default
    V_l->addWidget(MultiAnswerMod);
    V_l->setAlignment(MultiAnswerMod, Qt::AlignBottom);
    connect(MultiAnswerMod,SIGNAL(MamEmitMessage(QString,bool,bool,bool)),this,SLOT(SetEmitMessage(QString,bool,bool,bool)));
    connect(MultiAnswerMod,SIGNAL(EmitMAMCalls(QStringList)), this, SIGNAL(EmitMAMCalls(QStringList)));
    connect(MultiAnswerMod,SIGNAL(EmitHisCalls(QStringList)), this, SLOT(SetHisCalls(QStringList)));//2.71
    connect(MultiAnswerMod,SIGNAL(EmitDxParm(QString,QString,QString)), this, SLOT(SetDxParm(QString,QString,QString)));
    connect(MultiAnswerMod,SIGNAL(EmitGBlockListExp(bool)), this, SIGNAL(EmitGBlockListExp(bool)));
    connect(MultiAnswerMod,SIGNAL(EmitStopAuto()), this, SLOT(StopAuto()));//,Qt::DirectConnection
    connect(MultiAnswerMod,SIGNAL(EmitQSOProgressMAM(int,bool)), this, SLOT(SetQSOProgressAll(int,bool)));//2.51
    connect(MultiAnswerMod,SIGNAL(EmitDoQRG(QString,QString)),this,SLOT(SetDoQRG(QString,QString)));//2.71
    connect(MultiAnswerMod,SIGNAL(EmitMAFirstTX(bool)),this,SLOT(SetMAFirstTX(bool)));//2.71
    connect(MultiAnswerMod,SIGNAL(EmitSFMATxAll(QString)),this,SIGNAL(EmitSFMATxAll(QString)));//2.76sf

    QHBoxLayout *H_tx = new QHBoxLayout();
    H_tx->setContentsMargins ( 0, 0, 0, 0);
    H_tx->setSpacing(0);
    H_tx->addLayout(V_rx_1);
    H_tx->addLayout(V_l);
    H_tx->addLayout(V_tx_1);
    //H_tx->setAlignment(Qt::AlignHCenter);

    //QFrame *Box_in_tx = new QFrame();
    Box_in_tx = new QFrame();
    Box_in_tx->setMaximumWidth(440);//1.81 400    <- towa i towa po gore praviat idealniq slu4ai v1.11->line_txt->setMaximumWidth(300);
    Box_in_tx->setMinimumWidth(263);//2.15 263 - 264
    //Box_in_tx->setMaximumHeight(160);
    //Box_in_tx->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
    Box_in_tx->setFrameShape(QFrame::WinPanel);
    Box_in_tx->setFrameShadow(QFrame::Raised);
    Box_in_tx->setLayout(H_tx);
    Box_in_tx->setContentsMargins(3,2,3,3);

    /////////////////////////////////////////////////////
    sb_txsn_v2 = new HvSpinBoxSn("SN :",dsty);
    sb_txsn_v2->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
    connect(sb_txsn_v2, SIGNAL(EmitValueChanged(int)),this, SLOT(SetTxSnV2(int)));
    sb_txsn_v2->SetHidden(true);

    f_nosave = false;
    s_mode = 2;               //HV important set to default mode fsk441
    /*s_minsigndb[0] = 1;//msk144 not used
    s_minsigndb[1] = 1;//jtms
    s_minsigndb[2] = 1;
    s_minsigndb[3] = 1;
    s_minsigndb[4] = 1;
    s_minsigndb[5] = 1;
    s_minsigndb[6] = -10;//jt6m
    s_minsigndb[7] = 1;//jt65a
    s_minsigndb[8] = 1;//jt65b
    s_minsigndb[9] = 1;//jt65c
    s_minsigndb[10] = 1;//pi4
    s_minsigndb[11] = 1;//ft8
    s_minsigndb[12] = 1;//msk144ms
    s_minsigndb[13] = 1;//ft4
    s_minsigndb[14] = 1;//q65a
    s_minsigndb[15] = 1;//q65b
    s_minsigndb[16] = 1;//q65c
    s_minsigndb[17] = 1;//q65d*/
    for (int i=0; i<COUNT_MODE; ++i)
    {
        if (i==6) s_minsigndb[i]=-10;
        else s_minsigndb[i]=1;
    }

    SB_MinSigdB = new HvSpinBox();
    //SB_MinSigdB->setContentsMargins(2,4,2,1);
    SB_MinSigdB->setRange(-30,10);
    SB_MinSigdB->setValue(1);
    //SB_MinSigdB->setFixedWidth(152);//problem hv
    //SB_MinSigdB->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
    SB_MinSigdB->setPrefix("S Limit def=1  ");
    SB_MinSigdB->setSuffix("  dB");
    SB_MinSigdB->findChild<QLineEdit*>()->setReadOnly(true);
    SB_MinSigdB->setContextMenuPolicy(Qt::NoContextMenu);
    if (dsty) SB_MinSigdB->setStyleSheet("QSpinBox{selection-color:white;selection-background-color:rgb(36,33,16);}");
    else SB_MinSigdB->setStyleSheet("QSpinBox{selection-color:black;selection-background-color:white;}");
    //SB_MinSigdB->setCursor(Qt::ArrowCursor);

    TQueuedCall = new HvQueuedCallW(dsty);
    //TQueuedCall->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
    TQueuedCall->setHidden(true);
    connect(TQueuedCall, SIGNAL(EmidButClrQueuedCall()), this, SLOT(SetButClrQueuedCall()));

    cb_zap = new QCheckBox("ZAP");
    cb_zap->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
    //cb_zap->setFixedWidth(48);//42 stop v 1.24 hv hide end zap for w10
    connect(cb_zap, SIGNAL(toggled(bool)), this, SIGNAL(EmitZap(bool)));
    cb_zap->setChecked(false);

    //cb_msh = new QCheckBox("MSHxxxxxxx");//2.76
    cb_msh = new QCheckBox(tr("Super Hound"));//2.76
    cb_msh->setChecked(false);
    cb_msh->setHidden(true);
    //cb_msf = new QCheckBox("MSFxxxxx");
    cb_msf = new QCheckBox(tr("Super Fox"));
    cb_msf->setChecked(false);
    cb_msf->setHidden(true);
    cb_msf->setEnabled(false);
    connect(cb_msh,SIGNAL(toggled(bool)),this,SLOT(MshfChanget(bool)));
    connect(cb_msf,SIGNAL(toggled(bool)),this,SLOT(MshfChanget(bool)));
    
    QHBoxLayout *H_lsz = new QHBoxLayout();
    H_lsz->setContentsMargins(0,0,0,0);
    H_lsz->setSpacing(1);
    H_lsz->addWidget(SB_MinSigdB);
    H_lsz->addWidget(TQueuedCall);
    H_lsz->addWidget(sb_txsn_v2);        
    H_lsz->addWidget(cb_msh);
    H_lsz->addWidget(cb_msf);    
    //f_vsf_ = false;//2.76sf for remove
    
    H_lsz->addWidget(cb_zap);
    //H_lsz->setAlignment(cb_zap, Qt::AlignRight);

    SB_DfTolerance1 = new HvSpinBoxDf(dsty);
    SB_DfTolerance1->SetMode(s_mode);

    connect(SB_MinSigdB, SIGNAL(valueChanged(int)), this, SLOT(DfSdbChanged(int)));
    connect(SB_DfTolerance1, SIGNAL(EmitValueChanged(int)), this, SLOT(DfSdbChanged(int)));

    //2.45
    le_qrg = new HvQrg(dsty);
    connect(le_qrg, SIGNAL(EmitEnter()),this, SLOT(gen_msg()));
    connect(le_qrg, SIGNAL(EmitTextChanged(QString)), MultiAnswerMod, SLOT(SetQRG(QString)));
    connect(le_qrg, SIGNAL(EmitQgrParms(QString,bool)), this, SIGNAL(EmitQrgParms(QString,bool)));
    connect(le_qrg, SIGNAL(EmitActiveId(int)), this, SLOT(SetQrgActiveId(int)));//2.60

    cb_sh_rpt = new QCheckBox("Sh");
    //cb_sh_rpt->setFixedWidth(38);//38 stop v 1.24 hv hide end cb_sh_rpt for w10
    connect(cb_sh_rpt, SIGNAL(toggled(bool)), this, SLOT(Sh_Rpt_Changet(bool)));
    cb_sh_rpt->setEnabled(false);// default
    cb_sh_rpt->setChecked(false);// default
    for (int i=0; i<COUNT_MODE; ++i) sh_op_all[i]=false;

    cb_swl = new QCheckBox("SWL");
    //cb_swl->setToolTip("Check to monitor Sh messages.");
    connect(cb_swl, SIGNAL(toggled(bool)), this, SLOT(Swl_Changet(bool)));
    cb_swl->setEnabled(false);// default
    cb_swl->setChecked(false);// default

    SB_PeriodTime = new HvSpinBox4per30s(dsty);
    //SB_PeriodTime->setSizePolicy(QSizePolicy::Maximum,QSizePolicy::Fixed);

    SB_PeriodTime->SetMode(s_mode);

    connect(SB_PeriodTime, SIGNAL(EmitValueChanged(float)), this, SIGNAL(EmitReriodTime(float)));
    connect(SB_PeriodTime, SIGNAL(EmitValueChanged(float)), MultiAnswerMod, SLOT(SetReriodTime(float)));

    AutoSeqLab = new HvLabAutoSeq(dsty);
    AutoSeqLab->setToolTip(tr("Automatic Sequencing"));
    connect(AutoSeqLab, SIGNAL(EmitLabAutoSeqPress()), this, SLOT(AutoSeqLabPress()));

    QHBoxLayout *H_qrg = new QHBoxLayout();
    H_qrg->setContentsMargins (0,1,0,1);//1.31 ContentsMargins left 35 za mahane ako ima drug widget
    H_qrg->setSpacing(3);
    //H_qrg->addWidget(cb_sh_rpt);
    H_qrg->addWidget(AutoSeqLab);
    //H_qrg->setAlignment(AutoSeqLab, Qt::AlignLeft);
    H_qrg->addWidget(SB_PeriodTime);
    H_qrg->setAlignment(SB_PeriodTime, Qt::AlignRight);

    /*H_qrg->addWidget(l_qrg);
    H_qrg->setAlignment(l_qrg, Qt::AlignRight);
    H_qrg->addWidget(le_qrg);
    H_qrg->setAlignment(le_qrg, Qt::AlignRight);*/
    H_qrg->addWidget(le_qrg);

    //H_qrg->setAlignment(Qt::AlignRight);

    THvLogW = new HvLogW(inst,AppPath,dsty,x,y,this);//2.71
    connect(MultiAnswerMod, SIGNAL(AddToLog(QStringList)), this, SLOT(AddToLogMultiAnswerQSO(QStringList)));
    connect(MultiAnswerMod, SIGNAL(IsCallDupeInLog(QString,int,bool &)), this, SLOT(IsCallDupeInLog(QString,int,bool &)));
    connect(MultiAnswerMod, SIGNAL(EmitDoubleClick()), this, SLOT(SetDoubleClickFromAllAutoOn()));//2.45

    connect(TRadioAndNetW, SIGNAL(EmitUdpBroadLoggedAll(bool,bool)), THvLogW, SLOT(SetUdpBroadLoggedAll(bool,bool)));
    /*bool a_brod_log_adif = false;
    if (TRadioAndNetW->GetUdpBroadLoggedAdif() || TRadioAndNetW->GetTcpBroadLoggedAdif() ||
            TRadioAndNetW->GetClubLogAdif() || TRadioAndNetW->GetUdp2BroadLoggedAdif() || 
            TRadioAndNetW->GetQRZLogAdif()) a_brod_log_adif = true;*/
    THvLogW->SetUdpBroadLoggedAll(TRadioAndNetW->GetUdpBroadLoggedQso(),TRadioAndNetW->GetLogAdifAll()); // need for slot exeption

    connect(THvLogW, SIGNAL(EmitLoggedQSO(QStringList)), TRadioAndNetW, SLOT(SendLoggedQSO(QStringList)));
    connect(THvLogW, SIGNAL(EmitAdifRecord(QString)), TRadioAndNetW, SLOT(SendAdifRecord(QString)));
    connect(THvLogW, SIGNAL(EmitUploadSelected(QByteArray)), TRadioAndNetW, SLOT(SetUploadSelected(QByteArray)));
    connect(TRadioAndNetW, SIGNAL(EmitUploadClubLogInfo(QString)), THvLogW, SLOT(SetUploadClubLogInfo(QString)));

    connect(THvLogW, SIGNAL(EmitMarkTextLogAll(QStringList,int)), this, SLOT(SetMarkTextLogAll(QStringList,int)));
    //connect(THvLogW, SIGNAL(EmitMarkTextLoc(QStringList)), this, SLOT(SetMarkTextLoc(QStringList)));
    s_list_log_mark_txt.clear();
    s_list_log_mark_txt_p1 = 50;
    s_list_mark_txt.clear();
    s_list_mark_myc.clear();
    s_calls_mark = "";

    b_add_to_log = new HvButtonLeftRightClick(tr("ADD TO LOG"));//2.75
    b_add_to_log->setFixedHeight(21);
    b_add_to_log->setStyleSheet("QPushButton{background-color:palette(Button);}");
    b_add_to_log->setToolTip(tr("Left Click, ADD TO LOG")+"\n"+tr("Right Click, Logging Settings"));
    connect(b_add_to_log,SIGNAL(lclicked()),this,SLOT(AddToLogButton()));
    connect(b_add_to_log,SIGNAL(rclicked()),this,SLOT(SetAutoLogInfo()));
    connect(THvLogW,SIGNAL(EmitCBEnableAliChanged(bool)),this,SLOT(CBEnableAliChanged(bool)));

    le_his_call = new HvInLe("call", tr("TO RADIO")+":",dsty);
    //le_his_call->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
    //le_his_call->setFixedWidthLine(120);//95
    connect(le_his_call, SIGNAL(EmitSndCheck(QString)), this, SLOT(Check(QString)));
    connect(le_his_call, SIGNAL(EmitTextChanged(QString)), MultiAnswerMod, SLOT(SetHisCallChanged(QString)));// for DetectTextInMsg
    connect(le_his_call, SIGNAL(EmitEntered()),this, SLOT(gen_msg()));
    le_his_call->setMinimumWidth(126);//1.40 be6e->128 122forOOO
    le_his_call->setMaxLength(15);
    le_his_call->setMaximumWidth(230);//1.81 230 1.30 220 +125%
    //le_his_call->setFixedWidth(126);

    le_rst_rx = new HvRptLe("rstrx","RX RPT:",dsty);// RPT
    connect(le_rst_rx, SIGNAL(EmitEntered()),this, SLOT(FormatRxRst()));
    //connect(le_rst_rx, SIGNAL(SndCheck(QString)), this, SLOT(Check(QString)));

    QHBoxLayout *H_call_log= new QHBoxLayout();
    H_call_log->setContentsMargins (0, 0, 0, 0);
    H_call_log->setSpacing(2);
    //H_call_log->addWidget(b_view_log);
    H_call_log->addWidget(b_add_to_log);
    H_call_log->setAlignment(b_add_to_log,Qt::AlignRight);
    H_call_log->addWidget(le_his_call);
    //H_call_log->setAlignment(le_his_call,Qt::AlignCenter);
    H_call_log->addWidget(le_rst_rx);
    H_call_log->setAlignment(le_rst_rx,Qt::AlignLeft);
    //H_call_log->setAlignment(le_his_call,Qt::AlignRight);
    H_call_log->setAlignment(Qt::AlignCenter);
    //H_locdb->setAlignment(Qt::AlignCenter);

    s_2click_list_autu_on = true;//2.51
    b_auto_on= new QPushButton(tr("AUTO IS OFF"));
    b_auto_on->setStyleSheet("QPushButton{background-color:palette(Button);}");
    b_auto_on->setFixedHeight(21);
    //b_auto_on->setToolTip("frrrrr");
    //b_auto_on->setFixedSize(90,21);//90,21 stop v 1.24 hv hide end b_auto_on for w10
    connect(b_auto_on, SIGNAL(clicked(bool)), this, SLOT(auto_on()));
    b_gen_msg= new QPushButton(tr("GEN MSG"));
    b_gen_msg->setFixedHeight(21);
    //b_gen_msg->setFixedSize(90,21);//90,21 stop v 1.24 hv hide end b_gen_msg for w10
    connect(b_gen_msg, SIGNAL(clicked(bool)), this, SLOT(bt_gen_msg()));//1.64= for SWL stations

    //File_m->addAction(QPixmap(":pic/exit.png"),ShKey[3][1], this, SLOT(close()),QKeySequence(tr(ShKey[3][0].toUtf8(),ShKey[3][1].toUtf8())));

    QHBoxLayout *H_bu = new QHBoxLayout();
    H_bu->setContentsMargins ( 1, 1, 1, 1);
    H_bu->setSpacing(1);
    H_bu->addWidget(b_gen_msg);
    //H_bu->setAlignment(b_gen_msg,Qt::AlignCenter);
    H_bu->addWidget(b_auto_on);
    //H_bu->setAlignment(b_auto_on,Qt::AlignCenter);

    le_rst_tx = new HvRptLe("rsttx","TX RPT:",dsty);//le_rst_tx->SetErrotText
    connect(le_rst_tx, SIGNAL(EmitSndCheck(QString)), this, SLOT(Check(QString)));//2.66
    connect(le_rst_tx, SIGNAL(EmitEntered()),this, SLOT(gen_msg()));

    //l_txsn = new QLabel("");
    sb_txsn = new HvSpinBoxSn("SN :",dsty);
    //sb_txsn->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
    sb_txsn->SetHidden(true);
    connect(sb_txsn, SIGNAL(EmitValueChanged(int)),this, SLOT(gen_msg()));
    //connect(le_txsn, SIGNAL(SndCheck(QString)), this, SLOT(Check(QString)));

    pb_tx_to_rx = new HvTxRxEqual("TX",tr("TX To RX"),true);
    pb_tx_to_rx->setHidden(true);//default
    connect(pb_tx_to_rx, SIGNAL(clicked(bool)), this, SIGNAL(EmitTxToRx(bool)));
    pb_rx_to_tx = new HvTxRxEqual("RX",tr("RX To TX"),false);//2.63
    pb_rx_to_tx->setHidden(true);//default
    connect(pb_rx_to_tx, SIGNAL(clicked(bool)), this, SIGNAL(EmitRxToTx(bool)));
    QHBoxLayout *H_txrx0 = new QHBoxLayout();
    H_txrx0->setContentsMargins(0,0,0,0);
    H_txrx0->setSpacing(0);
    H_txrx0->addWidget(pb_tx_to_rx);
    H_txrx0->addWidget(pb_rx_to_tx);

    cb_lock_txrx = new QCheckBox("LTR");
    cb_lock_txrx->setToolTip(tr("Lock TX & RX"));
    cb_lock_txrx->setHidden(true);//default
    connect(cb_lock_txrx, SIGNAL(toggled(bool)), this, SLOT(LockTxrxChanged(bool)));

    QHBoxLayout *H_rst = new QHBoxLayout();
    H_rst->setContentsMargins (0,5,0,2);
    H_rst->setSpacing(2);//2.63 old=3
    H_rst->addWidget(cb_lock_txrx);
    H_rst->addLayout(H_txrx0);
    H_rst->addWidget(cb_sh_rpt);
    H_rst->setAlignment(cb_sh_rpt, Qt::AlignBottom);//1.31
    H_rst->addWidget(cb_swl);
    H_rst->setAlignment(cb_swl, Qt::AlignBottom);//1.31
    H_rst->addWidget(sb_txsn);
    H_rst->setAlignment(sb_txsn, Qt::AlignLeft);
    H_rst->addWidget(le_rst_tx);
    H_rst->setAlignment(le_rst_tx, Qt::AlignRight);

    l_rx_only_fi_se = new HvLabRxOnlyFiSe();
    l_rx_only_fi_se->setText("RXF");    //default
    l_rx_only_fi_se->setToolTip(tr("RX Only First/Second Period"));
    connect(l_rx_only_fi_se, SIGNAL(EmitLabPress()), this, SLOT(SetRxOnlyFiSe()));

    rb_tx_fi = new QRadioButton(tr("TX FIRST"));
    rb_tx_fi->setFixedHeight(19);//2.15 16 to 19 +150%
    rb_tx_se = new QRadioButton(tr("TX SECOND"));
    rb_tx_se->setFixedHeight(19);//2.15 16 to 19 +150%
    connect(rb_tx_fi, SIGNAL(toggled(bool)), this, SLOT(RbTxFiSeChange(bool)));
    connect(rb_tx_se, SIGNAL(toggled(bool)), this, SLOT(RbTxFiSeChange(bool)));
    rb_tx_se->setChecked(true);
    f_ma_first_tx = false;

    QHBoxLayout *H_txx = new QHBoxLayout();
    H_txx->setContentsMargins ( 0, 2, 0, 2);
    H_txx->setSpacing(1);
    H_txx->addWidget(l_rx_only_fi_se);
    //H_txx->setAlignment(l_rx_only_fi_se,Qt::AlignLeft);
    H_txx->addWidget(rb_tx_fi);
    //H_txx->setAlignment(rb_tx_fi,Qt::AlignRight);
    H_txx->addWidget(rb_tx_se);
    //H_txx->setAlignment(Qt::AlignRight);

    QVBoxLayout *V_tx_b = new QVBoxLayout();
    V_tx_b->setContentsMargins ( 1, 2, 1, 1);
    V_tx_b->setSpacing(3);
    //V_tx_b->addWidget(SB_MinSigdB);
    V_tx_b->addLayout(H_lsz);
    V_tx_b->addWidget(SB_DfTolerance1);
    //V_tx_b->addLayout(H_dfsh);
    //V_tx_b->setAlignment(H_lle,Qt::AlignRight);

    //V_tx_b->addWidget(le_his_call);
    //V_tx_b->setAlignment(le_his_call,Qt::AlignRight);
    V_tx_b->addLayout(H_rst);

    V_tx_b->addLayout(H_qrg);
    //V_tx_b->setAlignment(H_qrg,Qt::AlignRight);

    V_tx_b->addLayout(H_txx);
    V_tx_b->addLayout(H_bu);
    V_tx_b->setAlignment(H_bu, Qt::AlignTop);
    //V_tx_b->setAlignment(Qt::AlignRight);

    //V_tx_b->setAlignment(Qt::AlignRight);
    QFrame *Box_in_tx_b = new QFrame();
    Box_in_tx_b->setMaximumWidth(295);//1.81 for font 14pt
    //Box_in_tx_b->setFixedWidth(210);
    Box_in_tx_b->setFrameShape(Box_in_tx->frameShape());
    Box_in_tx_b->setFrameShadow(Box_in_tx->frameShadow());
    Box_in_tx_b->setLayout(V_tx_b);
    //Box_in_tx_b->setLayout(H_tx_bb);
    Box_in_tx_b->setContentsMargins(3,2,3,3);

    /////////////////////////////////////////////////////
    l_time = new QLabel();
    l_dey = new QLabel();
    //l_dey->setFont(f_t);
    l_dey->setContentsMargins(0,0,0,0);
    //l_dey->setFixedWidth(85);//110 stop v 1.24 hv hide end dey for w10
    sb_dt = new HvSpinBoxDt(dsty);
    connect(sb_dt,SIGNAL(EmitValueChanged(int)),this,SIGNAL(EmitOffsetDt(int)));

    QHBoxLayout *H_detm = new QHBoxLayout();
    H_detm->setContentsMargins (1,1,1,1);
    H_detm->setSpacing(10);// v1.24 hv no hide time and dey w10
    H_detm->addWidget(l_dey);
    H_detm->setAlignment(l_dey, Qt::AlignVCenter);
    H_detm->addWidget(l_time);
    H_detm->setAlignment(l_time,Qt::AlignCenter);
    H_detm->addWidget(sb_dt);
    H_detm->setAlignment(Qt::AlignCenter);
    sb_dt->SetHidden(true);

    QVBoxLayout *V_dt = new QVBoxLayout();
    V_dt->setContentsMargins (1,1,1,1);
    V_dt->setSpacing(3);
    QSpacerItem *item = new QSpacerItem(1,1, QSizePolicy::Expanding, QSizePolicy::Fixed);
    V_dt->addSpacerItem(item);

    l_mycall_loc = new QLabel();
    //l_mycall_loc->setFont(f_t);
    s_band = "70 MHz";
    s_iband = 17;//2.76.5 17 default HV 70 MHz 

    rig_cat_active_and_read = false;  //no read rig

    FREQ_GLOBAL = "70230000";
    THvCatDispW = new HvCatDispW(dsty);
    connect(THvCatDispW, SIGNAL(EmitSetDefFreqGlobal(int,QString)), this, SLOT(SetDefFreqGlobal(int,QString)));
    //QPushButton *pb_set_def_freq_to_rig = new QPushButton("DEF");
    //pb_set_def_freq_to_rig->set

    QHBoxLayout *H_clf = new QHBoxLayout();
    H_clf->setContentsMargins ( 0, 0, 0, 0); //( int left, int top, int right, int bottom )
    H_clf->setSpacing(12);
    H_clf->addWidget(l_mycall_loc);
    H_clf->addWidget(THvCatDispW);
    //H_clf->setAlignment(Qt::AlignCenter);

    //H_clf->addWidget(pb_set_def_freq_to_rig);

    LeHisLoc = new HvInLe("locator", tr("LOCATOR")+":",dsty);//HIS LOCATOR:
    LeHisLoc->SetMask(">AA99AA");
    LeHisLoc->setMinimumWidth(120);//120 1.30
    connect(LeHisLoc, SIGNAL(EmitSndCheck(QString)), this, SLOT(Check(QString)));

    QLabel *l_dst = new QLabel();
    l_dst->setText(tr("Dist")+":");//Dstc:Distance:
    l_dst->setContentsMargins(6,0,0,0);
    //l_dst->setFixedWidth(60);
    l_dist = new QLabel();
    //l_dist->setFixedWidth(65);
    QLabel *l_bea = new QLabel();
    l_bea->setText(tr("Azimuth")+":");
    l_bea->setContentsMargins(6,0,0,0);
    //l_bea->setFixedWidth(60);
    l_beam = new QLabel();
    //l_beam->setFixedWidth(60);
    QLabel *l_e = new QLabel();
    l_e->setText(tr("Elevation")+":");
    l_e->setContentsMargins(6,0,0,0);
    //l_e->setFixedWidth(15);
    l_el = new QLabel();
    l_haz = new QLabel();
    l_loc_from_db =  new QLabel("DB: NA");

    s_dist = "NA";
    s_beam = "NA";

    pb_checkdb = new QPushButton(tr("LOOKUP"));
    pb_checkdb->setFixedHeight(21);
    pb_checkdb->setMaximumWidth(80);//r5wm=70  org=80

    connect(pb_checkdb, SIGNAL(clicked(bool)),this,SLOT(CheckBD()));
    if (dsty) pb_checkdb->setStyleSheet("QPushButton{color:rgb(255,200,0)}");
    else pb_checkdb->setStyleSheet("QPushButton{color:"+ColorStr_[1]+"}");

    pb_adddb = new QPushButton(tr("ADD"));
    pb_adddb->setFixedHeight(21);
    //pb_adddb->setFixedWidth(35);//1.30 +125%
    pb_adddb->setMaximumWidth(47);//r5wm=40  org=45

    if (dsty) pb_adddb->setStyleSheet("QPushButton{color:rgb(255,200,0)}");
    else pb_adddb->setStyleSheet("QPushButton{color:"+ColorStr_[1]+"}");

    connect(pb_adddb, SIGNAL(clicked(bool)), this, SLOT(AddDb()));
    QHBoxLayout *H_locdb = new QHBoxLayout();
    H_locdb->setContentsMargins(0, 1, 0, 1);
    H_locdb->setSpacing(2);
    H_locdb->addWidget(LeHisLoc);
    H_locdb->addWidget(l_loc_from_db);
    H_locdb->addWidget(pb_checkdb);
    H_locdb->addWidget(pb_adddb);
    //H_locdb->addWidget(LeHisLoc);
    H_locdb->setAlignment(LeHisLoc,Qt::AlignRight);
    H_locdb->setAlignment(Qt::AlignCenter);
    //H_locdb->setAlignment(pb_adddb,Qt::AlignLeft);

    QHBoxLayout *H_lc = new QHBoxLayout();
    H_lc->setContentsMargins (1, 1, 1, 1);
    H_lc->setSpacing(3);
    H_lc->addWidget(l_haz);
    H_lc->addWidget(l_bea);
    H_lc->addWidget(l_beam);
    H_lc->addWidget(l_e);
    H_lc->addWidget(l_el);
    H_lc->addWidget(l_dst);
    H_lc->addWidget(l_dist);
    //H_lc->setAlignment(Qt::AlignCenter);
    //H_lc->addWidget(l_loc_from_db);

    le_mon_call1 = new HvInLe("call_mon_1", tr("R1")+":",dsty);
    //le_his_call->setFixedWidthLine(120);//95
    connect(le_mon_call1, SIGNAL(EmitSndCheck(QString)), this, SLOT(Check(QString)));
    le_mon_call1->setMinimumWidth(122);//120 1.30 granitsa
    le_mon_call1->setMaxLength(15);
    //le_mon_call1->setContentsMargins(0,0,0,0);

    l_monit = new QLabel(tr("MONITOR")); //MONITOR
    le_mon_call2 = new HvInLe("call_mon_2", tr("R2")+":",dsty);
    //le_his_call->setFixedWidthLine(120);//95
    connect(le_mon_call2, SIGNAL(EmitSndCheck(QString)), this, SLOT(Check(QString)));
    le_mon_call2->setMinimumWidth(122);//120 1.30 granitsa
    le_mon_call2->setMaxLength(15);
    //le_mon_call2->setContentsMargins(0,0,0,0);

    l_moont = new QLabel(tr("MOON"));
    l_moont->setContentsMargins(0,3,4,3);//1.62= (0,0,4,0);
    //l_moont->setFont(f_tl);//l_moont->setStyleSheet("QLabel{font:bold;};");
    l_azm = new QLabel(tr("Az")+":");
    l_azm->setContentsMargins(0,3,4,3);//1.62= (0,0,4,0);
    //l_azm->setFont(f_tl);
    l_elm = new QLabel(tr("El")+":");
    l_elm->setContentsMargins(0,3,4,3);//1.62= (0,0,4,0);
    //l_elm->setFont(f_tl);
    l_dopm = new QLabel(tr("Dop")+":");
    l_dopm->setContentsMargins(0,3,4,3);//1.62= (0,0,4,0);
    //l_dopm->setFont(f_tl);
    l_dgrdm = new QLabel(tr("Dgrd")+":");
    l_dgrdm->setContentsMargins(0,3,4,3);//1.62= (0,0,4,0);
    //l_dgrdm->setFont(f_tl);
    l_moont->setHidden(true);
    l_azm->setHidden(true);
    l_elm->setHidden(true);
    l_dopm->setHidden(true);
    l_dgrdm->setHidden(true);
    //SetShowMoni12MoonD(true);

    QHBoxLayout *H_mon_calls = new QHBoxLayout();
    H_mon_calls->setContentsMargins ( 0, 0, 0, 0);
    H_mon_calls->setSpacing(4);

    H_mon_calls->addWidget(l_monit);
    H_mon_calls->addWidget(le_mon_call1);
    H_mon_calls->setAlignment(le_mon_call1,Qt::AlignCenter);
    H_mon_calls->addWidget(le_mon_call2);
    H_mon_calls->setAlignment(le_mon_call2,Qt::AlignCenter);

    H_mon_calls->addWidget(l_moont);
    H_mon_calls->addWidget(l_azm);
    H_mon_calls->addWidget(l_elm);
    H_mon_calls->addWidget(l_dopm);
    H_mon_calls->addWidget(l_dgrdm);
    //H_mon_calls->setAlignment(Qt::AlignCenter);

    // HV inportent here is a place for Check
    LeHisLoc->SndCheck_s("");// proverka v na4aloto
    le_his_call->SndCheck_s("");// proverka v na4aloto
    le_mon_call1->SndCheck_s("");// proverka v na4aloto
    le_mon_call2->SndCheck_s("");// proverka v na4aloto

    // V_dt->setAlignment(H_dtt,Qt::AlignRight)
    //V_dt->addWidget(l_mycall_loc);
    //V_dt->setAlignment(l_mycall_loc,Qt::AlignCenter);
    V_dt->addLayout(H_clf);
    V_dt->setAlignment(H_clf,Qt::AlignCenter);

    //V_dt->addWidget(le_his_call);
    V_dt->addLayout(H_call_log);
    V_dt->setAlignment(H_call_log,Qt::AlignCenter);

    //V_dt->addWidget(b_add_to_log);

    V_dt->addLayout(H_locdb);
    V_dt->setAlignment(LeHisLoc,Qt::AlignCenter);
    V_dt->addLayout(H_lc);
    V_dt->setAlignment(H_lc,Qt::AlignCenter);

    V_dt->addLayout(H_mon_calls);
    V_dt->setAlignment(H_mon_calls,Qt::AlignCenter);
    /*V_dt->addWidget(l_dey);
    V_dt->setAlignment(l_dey,Qt::AlignCenter);
    V_dt->addWidget(l_time);
    V_dt->setAlignment(l_time, Qt::AlignCenter);*/
    V_dt->addLayout(H_detm);

    //V_dt->setAlignment(l_time, Qt::AlignTop);
    V_dt->setAlignment(Qt::AlignVCenter);
    //V_dt->setAlignment(Qt::AlignLeft);

    QFrame *Box_dt = new QFrame();
    //Box_dt->setFixedWidth(340);
    Box_dt->setFrameShape(Box_in_tx->frameShape());
    Box_dt->setFrameShadow(Box_in_tx->frameShadow());
    Box_dt->setLayout(V_dt);
    Box_dt->setContentsMargins(3,2,3,3);

    //TX watchdog reset 2.49 stop
    //connect(this, SIGNAL(EmitMessageS(QString,bool,bool)), this, SLOT(ResetTxWatchdog(QString,bool,bool)));
    ///////////////////////////////////////////////////////

    QHBoxLayout *H_l = new QHBoxLayout(this);
    H_l->setContentsMargins ( 1, 1, 1, 1);
    H_l->setSpacing(1);
    //H_l->addWidget(status_frame);
    H_l->addWidget(Box_dt);
    //H_l->setAlignment(Box_dt,Qt::AlignRight);
    H_l->addWidget(Box_in_tx_b);
    //H_l->setAlignment(Box_in_tx_b,Qt::AlignRight);
    H_l->addWidget(Box_in_tx);
    //H_l->setAlignment(Box_in_tx,Qt::AlignLeft);
    //H_l->setAlignment(Qt::AlignCenter);

    setLayout(H_l);

    sr_path = AppPath+"/settings/ms_mesages";

    f_makros_ready = false;

    //1.61= CAT
    //s_mode = -1;//2.71 stop s_mode = -1; <- crash ,old ->? ot tuk se refre6va CAT only ont time
    s_band = "70 MHz";
    //end 1.61= CAT
    ReadSettings();
}
HvTxW::~HvTxW()
{
    SaveSettings();
    TRadioAndNetW->SaveSettings();//2.68 for TAB "Radio And Frequencies Configuration"
    //TMsDb->SaveDb();  2.35 stop close error no needed
    //THvLogW->SaveAllExternal(); 2.35 stop close error no needed qDebug()<<"DELETE HvTxW";
}
void HvTxW::RefreshOtpKeyMsg()
{
	TRadioAndNetW->RefreshOtpKeyMsg();
}
void HvTxW::SetMaManAdding(bool f)
{
    MultiAnswerMod->SetMaManAdding(f);
}
void HvTxW::Macros_exec()
{
    TWD->SetExec(0);
}
void HvTxW::NetW_exec()
{
    TWD->SetExec(1);
}
void HvTxW::RadioFreqW_exec()
{
    TWD->SetExec(2);
}
void HvTxW::SetMacrosFirstStart()
{
    THvMakros->SetMacros();
}
void HvTxW::SetUseQueueCont(bool f)//2.59
{
    TQueuedCall->SetUseQueueCont(f);
}
void HvTxW::ReadEDILog()//2.57
{
    THvLogW->ReadEDILog();
}
void HvTxW::SetQrgActive(int i)//2.45
{
    le_qrg->SetQrgActive(i);
}
void HvTxW::SetQrgFromRig(QString s)//2.45
{
    le_qrg->SetQrgFromRig(s); //qDebug()<<"SetQrgFromRig="<<s;
}
void HvTxW::SetQrgInfoFromCat(QString s)
{
    le_qrg->SetQrgInfoFromCat(s);
}
/*void HvTxW::SetFtDf1500(bool f)
{
    SB_DfTolerance1->SetFtDf1500(f);
}*/
void HvTxW::SetAResetQsoAtEnd(bool f)//2.49
{
    f_areset_qso = f;
}
QString HvTxW::GetAstroWPos()
{
    return THvAstroDataW->GetPosXY();
}
void HvTxW::SetAstroWPos(QString s)
{
    THvAstroDataW->SetPosXY(s);
}
QString HvTxW::GetLogWPosWH()//2.48
{
    return THvLogW->GetPosXYWH();
}
void HvTxW::SetLogWPosWH(QString s)//2.48
{
    THvLogW->SetPosXYWH(s);
}
void HvTxW::SetFont(QFont f)
{
    QFont f_t = f;

    f_t.setPointSize(f.pointSize()-2);//7
    l_trxmx->setFont(f_t);
    l_trxmi->setFont(f_t);
    l_trxdp->setFont(f_t);
    l_trxdm->setFont(f_t);

    f_t.setPointSize(f.pointSize()-1);//f_t.setPointSize(8);
    AutoSeqLab->setFont(f_t);
    l_rx_only_fi_se->setFont(f_t);//8
    //pb_tx_to_rx->setFont(f_t);
    f_t.setPointSize(f.pointSize()+7);//f_t.setPointSize(16);
    f_t.setBold(true);// f_t.setBold(true);
    l_time->setFont(f_t);
    f_t.setPointSize(f.pointSize()+3);//f_t.setPointSize(12);
    l_dey->setFont(f_t);
    sb_dt->setFont(f);
    l_mycall_loc->setFont(f_t);

    //QFont f_tl = font();
    //f_tl.setBold(true);
    //f_tl.setPointSize(9);
    f_t.setPointSize(f.pointSize());
    l_moont->setFont(f_t);
    l_azm->setFont(f_t);
    l_elm->setFont(f_t);
    l_dopm->setFont(f_t);
    l_dgrdm->setFont(f_t);
    THvCatDispW->SetFont(f);

    le_his_call->SetFont(f);
    LeHisLoc->SetFont(f);
    le_mon_call1->SetFont(f);
    le_mon_call2->SetFont(f);
    sb_txsn->SetFont(f);
    sb_txsn_v2->SetFont(f);
    le_rst_tx->SetFont(f);
    le_rst_rx->SetFont(f);
    SB_MinSigdB->setFont(f);
    SB_DfTolerance1->setFont(f);
    SB_PeriodTime->setFont(f);
    b_auto_on->setFont(f);
    pb_checkdb->setFont(f);
    pb_adddb->setFont(f);

    int bint =0;
    int rbint=0;
    //int txtt =0;
    for (int i = 0; i<count_tx_widget; i++)
    {
        HvTxIn *TempHvTxIn = (HvTxIn*)V_l->itemAt(i)->widget();
        TempHvTxIn->SetFont(f);
        if (i==0)//only once
        {
            bint = TempHvTxIn->b_tx->width();
            rbint = TempHvTxIn->rb_tx->width();
        }
    }
    MultiAnswerMod->SetFont(f);

    if (rbint>80) rbint=13;// exception no settings to default
    QFontMetrics fm(f);
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)//2.56
    int fwdd = fm.horizontalAdvance("SP9HWY LZ2HVV R+599 0002TXXRXX")+bint+rbint; //qDebug()<<"2SetMode="<<fwdd;
#else
    int fwdd = fm.width("SP9HWY LZ2HVV R+599 0002TXXRXX")+bint+rbint; //qDebug()<<"2SetMode="<<fwdd;
#endif
    if (fwdd < 263) fwdd = 263;
    Box_in_tx->setMinimumWidth(fwdd);//2.15 263 - 264
    //qDebug()<<"3SetMode="<<bint<<rbint<<fwdd;//<<MultiAnswerMod->contentsRect().width();

    THvAstroDataW->SetFont(f);
    TRadioAndNetW->SetFont(f);
    THvLogW->SetFont(f);
    TQueuedCall->SetFont(f);
    le_qrg->SetFont(f);
    THvMakros->SetFont(f);
    b_add_to_log->setFont(f);//2.75
    cb_msh->setFont(f);//2.76
    cb_msf->setFont(f);
}
void HvTxW::SetUdpDecClr()
{
    TRadioAndNetW->SetUdpDecClr();
}
void HvTxW::SetTxActive(int i)
{
    THvCatDispW->SetTxActive(i);
}
void HvTxW::CloseBcnList()
{
    TRadioAndNetW->CloseBcnList();
}
void HvTxW::StartEmptySpotDialog()
{
    TRadioAndNetW->StartEmptySpotDialog();
}
void HvTxW::ShowBcnList()
{
    TRadioAndNetW->ShowBcnList();
}
void HvTxW::Refr65DeepSearchDb()//1.49 deep search 65
{
    TMsDb->Refr65DeepSearchDb();
}
void HvTxW::SetAutoSeqAll(QString s)
{
    AutoSeqLab->SetAutoSeqAll(s);
}
void HvTxW::SetLockTxrxMode(int mod)
{
    cb_lock_txrx->setChecked(s_locktxrx[mod]);
}
void HvTxW::LockTxrxChanged(bool f)
{
    s_locktxrx[s_mode] = f;
    emit EmitLockTxrx(f);
}
void HvTxW::ShowAstroW()//1.58=stop this for ub9ocf
{
    THvAstroDataW->show();
    THvAstroDataW->StartStopTimer(true); //qDebug()<<"ShowAstroW()="<<true;
}
void HvTxW::CloseAstroW()//1.58=stop this for ub9ocf
{
    THvAstroDataW->close();//2.74=(&& !allq65) if not jt65abc allq65
    if (s_mode!=7 && s_mode!=8 && s_mode!=9 && !allq65) THvAstroDataW->StartStopTimer(false);
}
void HvTxW::SetShowMoni12MoonD(bool f)
{
    if (f)
    {
        l_moont->setHidden(true);
        l_azm->setHidden(true);
        l_elm->setHidden(true);
        l_dopm->setHidden(true);
        l_dgrdm->setHidden(true);
        if (!THvAstroDataW->isVisible()) THvAstroDataW->StartStopTimer(false);//1.58=stop this for ub9ocf
        l_monit->setHidden(false);
        le_mon_call1->setHidden(false);
        le_mon_call2->setHidden(false);
    }
    else
    {
        l_monit->setHidden(true);
        le_mon_call1->setHidden(true);
        le_mon_call2->setHidden(true);
        THvAstroDataW->StartStopTimer(true);//1.58=stop this for ub9ocf
        l_moont->setHidden(false);
        l_azm->setHidden(false);
        l_elm->setHidden(false);
        l_dopm->setHidden(false);
        l_dgrdm->setHidden(false);
    }
}
void HvTxW::SetAstroData(double my_za,double my_el,int his_dop,double dgrd)
{
    l_azm->setText(tr("Az")+": "+QString("%1").arg(my_za,0,'f',2));
    l_elm->setText(tr("El")+": "+QString("%1").arg(my_el,0,'f',2));
    l_dopm->setText(tr("Dop")+": "+QString("%1").arg(his_dop));
    l_dgrdm->setText(tr("Dgrd")+": "+QString("%1").arg(dgrd,0,'f',1));
}
void HvTxW::SetMAFirstTX(bool f)//2.71
{
    if (f && f_multi_answer_mod)
    {
        f_ma_first_tx = true;
        rb_tx_fi->setChecked(true);
        rb_tx_se->setChecked(false);
        rb_tx_se->setEnabled(false);
    }
    else
    {
        f_ma_first_tx = false;
        rb_tx_se->setEnabled(true);
    }
    //system("cls");
    //printf(" MA TxOnlyFirst=%s Mode=%d\n",f_ma_first_tx ? "true " : "false",s_mode);
    //printf(" MA First TX=%s  MA Flag=%s\n",f_ma_first_tx ? "true " : "false",f_multi_answer_mod ? "true " : "false");
}
void HvTxW::RbTxFiSeChange(bool)
{
    if (rb_tx_fi->isChecked())
    {
        if (f_rx_only_fi_se)
        {
            if (dsty) l_rx_only_fi_se->setStyleSheet("QLabel{background-color:rgb(150,0,0);}");
            else l_rx_only_fi_se->setStyleSheet("QLabel{background-color:rgb(255,0,0);}");
            l_rx_only_fi_se->setText("RXS");
            emit EmitRxOnlyFiSe(true);
        }
        else
        {
            l_rx_only_fi_se->setStyleSheet("QLabel{background-color:palette(Button);}");
            l_rx_only_fi_se->setText("RXS");
            emit EmitRxOnlyFiSe(false);
        }
        MultiAnswerMod->RestrictSeTX(false);//2.71
    }
    else
    {
        if (f_rx_only_fi_se)
        {
            if (dsty) l_rx_only_fi_se->setStyleSheet("QLabel{background-color:rgb(150,0,0);}");
            else l_rx_only_fi_se->setStyleSheet("QLabel{background-color:rgb(255,0,0);}");
            l_rx_only_fi_se->setText("RXF");
            emit EmitRxOnlyFiSe(true);
        }
        else
        {
            l_rx_only_fi_se->setStyleSheet("QLabel{background-color:palette(Button);}");
            l_rx_only_fi_se->setText("RXF");
            emit EmitRxOnlyFiSe(false);
        }
        MultiAnswerMod->RestrictSeTX(true);//2.71
    }
}
void HvTxW::SetRxOnlyFiSe()
{
    if (f_rx_only_fi_se) f_rx_only_fi_se = false;
    else f_rx_only_fi_se = true;
    RbTxFiSeChange(true);
}
QString HvTxW::get_op_allB(int ident)
{
    //0=sh_op 1=lock_txrx
    QString s;
    for (int i =0; i<COUNT_MODE; i++)
    {
        s.append(ModeStr(i)+"=");

        if (ident==0)
            s.append(QString("%1").arg(sh_op_all[i]));
        else if (ident==1)
            s.append(QString("%1").arg(s_locktxrx[i]));

        if (i<COUNT_MODE-1)
            s.append("#");
    }
    return s;
}
void HvTxW::SetOpAllB(QString s,int ident)
{
    //0=sh_op 1=lock_txrx
    QStringList ls=s.split("#");

    for (int i=0; i<COUNT_MODE; i++)
    {
        QString tstr = ModeStr(i)+"=";
        for (int j=0; j<ls.count(); j++)
        {
            if (ls[j].contains(tstr))
            {
                ls[j].remove(tstr);

                if (!ls[j].isEmpty())// only lial values hv
                {
                    if (ident==0)
                        sh_op_all[i] = (bool)ls[j].toInt();
                    else if (ident==1)
                        s_locktxrx[i] = (bool)ls[j].toInt();
                }

                break;
            }
        }
    }
}
void HvTxW::SetSwlOp(QString s)
{
    if (s.toInt()==1) cb_swl->setChecked(true);
    else cb_swl->setChecked(false);
}
void HvTxW::Swl_Changet(bool f)
{
    emit EmitSwlOptChenged(f);
    if (cb_swl->isChecked()) cb_sh_rpt->setChecked(false);
}
void HvTxW::SetVhfUhfFeatures(bool f)//1.60= def
{
    s_bvhf_jt65 = f;
    if (s_mode==7 || s_mode==8 || s_mode==9)//only for jt65
    {
        if (!f)
        {
            cb_sh_rpt->setChecked(false);
            cb_sh_rpt->setEnabled(false);
        }
        else
        {
            cb_sh_rpt->setEnabled(true);
            cb_sh_rpt->setChecked(sh_op_all[s_mode]);
        }
    } //qDebug()<<"VHF/HF="<<s_bvhf_jt65;
}
void HvTxW::Sh_Rpt_Changet(bool f)
{
    if (s_mode==7 || s_mode==8 || s_mode==9)
    {
        if (s_bvhf_jt65)
        {
            sh_op_all[s_mode]=f;
            //qDebug()<<"SSSSSSSSSSSSSSAVE sh_op_all="<<f;
        }
    }
    else
        sh_op_all[s_mode]=f;

    //sh_op_all[s_mode]=f;
    emit EmitShOptChenged(f);
    SetRptRsqSettings();
    gen_msg();

    if (cb_sh_rpt->isChecked() && cb_swl->isEnabled())//criticle  && cb_swl->isEnabled()
        cb_swl->setChecked(false);
}
void HvTxW::SetMarkTextAll()
{
    /*QStringList ls;
    ls = s_list_mark_txt;
    int pos_qso_b4 = ls.count();
    ls.append(s_list_log_mark_txt);
    int pos_loc = pos_qso_b4 + s_list_log_mark_txt_p1;
    emit EmitDListMarkText(ls,s_mark_r12_pos,s_mark_myc_pos,s_mark_hisc_pos,pos_qso_b4,pos_loc);*/
    QStringList ls;
    ls = s_list_mark_txt;
    int pos_qso_b4 = ls.count();
    ls.append(s_list_log_mark_txt);
    int pos_loc = pos_qso_b4 + s_list_log_mark_txt_p1;
    int mark_myc_pos = ls.count();
    ls.append(s_list_mark_myc);
    emit EmitDListMarkText(ls,s_mark_r12_pos,s_mark_hisc_pos,pos_qso_b4,pos_loc,mark_myc_pos);
    //qDebug()<<"LS="<<ls<<s_mark_myc_pos<<pos_qso_b4<<pos_loc<<ls.count();
}
void HvTxW::SetMarkTextLogAll(QStringList l,int p1)
{
    s_list_log_mark_txt = l;
    s_list_log_mark_txt_p1 = p1;
    SetMarkTextAll(); //qDebug()<<"QsoB4======"<<l.count();
}
void HvTxW::SetTextMark(bool *f,QString scm)
{
    //2.06 no used here s_txt_mark[25]<-in decodelist
    //QString sss;
    s_calls_mark = scm;
    for (int i = 0; i < TEX_MARK_C; i++)
    {
        s_txt_mark[i]=f[i];
        //sss.append(QString("%1").arg(i)+"=");
        //sss.append(QString("%1").arg((int)f[i]));
        //sss.append(",");
    }
    MultiAnswerMod->SetTextMark(s_txt_mark[20],s_txt_mark[21]);
    MarkTextChanged(true);//bool false  norefresh b4 qso
}
void HvTxW::SetHisCalls(QStringList l)//2.71
{
    //2.71 many signals, from: band, mod, Text Highlight, mod std or MA, SetMacros, ...
    l_mam_hiscals_ = l; //qDebug()<<" LIST="<<l_mam_hiscals_<<"MAM="<<f_multi_answer_mod<<"Mode="<<s_mode;
    MarkTextChanged(false);
    /*char c[20] = "CLR";
    if (l_mam_hiscals_.count()>0)
    {
    	int cc = l_mam_hiscals_.at(0).count();
    	if (cc>19) cc=19;
    	for (int i = 0; i < cc; i++) c[i] = l_mam_hiscals_.at(0)[i].toLatin1();    
    }
    printf(" LIST=%s   MAM=%s Mode=%d\n",c,f_multi_answer_mod ? "true " : "false",s_mode);*/
}
void HvTxW::MarkTextChanged(bool f_b4qso)//bool false  norefresh b4 qso
{
    //qDebug()<<" LIST="<<l_mam_hiscals_<<"MAM="<<f_multi_answer_mod<<"Mode="<<s_mode;
    QStringList ls;
    QStringList ls_hsah;
    ls_hsah<<""<<""<<""<<""<<""<<"";
    ls.clear();

    QString smycall = list_macros.at(0);//1.60=
    ls_hsah.replace(0,smycall);    //ls_hsah.replace(0,MultiAnswerMod->FindBaseCallRemAllSlash(smycall));

    if (s_txt_mark[0]) ls<<"CQ";
    if (s_txt_mark[7]) ls<<"73";
    if (s_txt_mark[8]) ls<<"QRZ";
    if (s_txt_mark[5]) ls<<"RRR";
    if (s_txt_mark[6]) ls<<"RR73"<<"RR73;";//2.63   "RR73;"
    if (s_txt_mark[3] && !block_loc) ls<<LeHisLoc->getText().mid(0,4);//loc4
    if (s_txt_mark[4] && !block_loc) ls<<LeHisLoc->getText();//loc6

    s_mark_r12_pos = ls.count();
    if (!block_mon_call1)
    {
        if (s_txt_mark[15]) ls<<DetectSufix(le_mon_call1->getText(),false);
        if (s_txt_mark[16]) ls<<DetectSufix(le_mon_call1->getText(),true);
        if (s_txt_mark[9] ) ls<<le_mon_call1->getText();
        ls_hsah.replace(2,le_mon_call1->getText());
    }
    if (!block_mon_call2)
    {
        if (s_txt_mark[17]) ls<<DetectSufix(le_mon_call2->getText(),false);
        if (s_txt_mark[18]) ls<<DetectSufix(le_mon_call2->getText(),true);
        if (s_txt_mark[10]) ls<<le_mon_call2->getText();
        ls_hsah.replace(3,le_mon_call2->getText());
    }
    if (s_txt_mark[28] && !s_calls_mark.isEmpty())//2.66
    {
        QStringList ssl = s_calls_mark.split(",");
        for (int i = 0; i < ssl.count(); ++i) ls<<ssl.at(i);
    }

    s_mark_hisc_pos = ls.count();//2.63
    QStringList l_hiscals_;
    if (f_multi_answer_mod && (s_mode==11 || s_mode==13 || s_mode==18 || allq65))//2.71
    {
        if (s_txt_mark[2] && l_mam_hiscals_.count()>0) ls.append(l_mam_hiscals_);
        for (int i = 0; i<l_mam_hiscals_.count(); ++i)
        {
            l_hiscals_.append(l_mam_hiscals_.at(i));
            QString c0 = MultiAnswerMod->FindBaseFullCallRemAllSlash(l_mam_hiscals_.at(i));
            if (c0!=l_mam_hiscals_.at(i)) l_hiscals_.append(c0);
        }
    }
    else if (!block_call)
    {
        l_hiscals_<<le_his_call->getText();
        QString c0 = MultiAnswerMod->FindBaseFullCallRemAllSlash(le_his_call->getText());
        if (c0!=le_his_call->getText()) l_hiscals_<<c0;
    }
    emit EmitHisCalls(l_hiscals_);//qDebug()<<" LIST="<<l_hiscals_;
    if (!block_call)
    {
        if (s_txt_mark[13])
        {
            QString sf = DetectSufix(le_his_call->getText(),false);
            if (sf.count()>1) ls<<sf;//2.52 minimum 2 letters
        }
        if (s_txt_mark[14]) ls<<DetectSufix(le_his_call->getText(),true);
        if (s_txt_mark[2] ) ls<<le_his_call->getText();
        ls_hsah.replace(1,le_his_call->getText());//2.72 stop set down, error in 2.71
        ls_hsah.replace(5,MultiAnswerMod->FindBaseFullCallRemAllSlash(le_his_call->getText()));//2.76.5
    }
    //if (!block_call) ls_hsah.replace(1,le_his_call->getText());//2.72

    s_list_mark_txt = ls;
    //2.65 need to be last, exception for my call in log
    ls.clear();
    if (!smycall.isEmpty())
    {
        if (s_txt_mark[11])
        {
            QString sf = DetectSufix(smycall,false);
            if (sf.count()>1) ls<<sf; //2.52 minimum 2 letters
        }
        if (s_txt_mark[12]) ls<<DetectSufix(smycall,true);
        if (s_txt_mark[1] ) ls<<smycall;
    }
    s_list_mark_myc = ls;
    //2.65 END need to be last, exception for my call in log

    if (!block_loc) ls_hsah.replace(4,LeHisLoc->getText());//2.53 for jt65 q65
    	
    emit EmitListHashCalls(ls_hsah);//ls_hsah[4]   for ft8 his locator

    if (f_b4qso)//bool false  norefresh b4 qso
    {
        THvLogW->SetMarkTextAllRuleChanged(s_txt_mark,s_mode,s_band); //qDebug()<<"SetMarkTextQsoB4====== Markers";
    }
    else
    {
        SetMarkTextAll(); //qDebug()<<"MarkText======"<<ls.count();
    }
}
void HvTxW::SetPiriodTimeAllModes(QString s)
{
    SB_PeriodTime->SetPiriodTimeAllModes(s);
}
void HvTxW::StndOutLevel_s(int val,int)
{
    s_tx_level[s_iband] = val;
    emit StndOutLevel(val);
}
void HvTxW::SetOutLevel(QString out_level_cor)
{
    QStringList ltxl = out_level_cor.split("#");
    for (int i = 0; i<ltxl.count(); ++i)
    {
        if (i<COUNT_BANDS) s_tx_level[i] = ltxl.at(i).toInt();
    }
    Slider_Tx_level->SetValue(s_tx_level[s_iband]);
}
void HvTxW::SetInLevel(QString in_level_cor)
{
    Slider_Rx_level->SetValue(in_level_cor.toInt());
}
void HvTxW::StndInLevel_s(int val,int)
{
    emit StndInLevel(val);
}
void HvTxW::RefreshBackupDB()
{
    if (MultiAnswerMod->isHidden())
    {
        if (s_last_call_for_BDB==le_his_call->getText()) return;
        s_last_call_for_BDB = le_his_call->getText();
        MultiAnswerMod->SetBackupDB(le_his_call->getText(),//call
                                    le_rst_tx->getText(),//txr
                                    le_rst_rx->getText(),//rxr
                                    LeHisLoc->getText(),//g
                                    s_his_contest_sn,//s
                                    s_his_contest_exch);//e
        //qDebug()<<le_his_call->getText()<<LeHisLoc->getText();
    }
}
void HvTxW::ResetQSO_p(bool full_rst)//2.43
{
    if (s_cont_type>1 && (s_mode==0 || s_mode==11  || s_mode==13 || s_mode==18 || allq65))//ft4
    {
        if (TrySetQueuedCall(true)) return;//2.59 true = on the flay if on TX reset, old=no flag
        else TQueuedCall->ClrQueuedCall(false);
    }

    RefreshBackupDB();

    LeHisLoc->SetText("");
    le_his_call->SetText("");

    if (full_rst)//2.43
    {
        le_mon_call1->SetText("");
        le_mon_call2->SetText("");
    }

    le_rst_tx->SetRptRsqSettings("TX",s_mode,s_f_rpt_rsq,cb_sh_rpt->isChecked(),s_cont_type);
    le_rst_rx->SetRptRsqSettings("RX",s_mode,s_f_rpt_rsq,cb_sh_rpt->isChecked(),s_cont_type);

    //if (sb_txsn->isEnabled())//2.01 stoped napomnia da smeni6 nomera v staria wariant
    //sb_txsn->SetText("");

    if (s_mode==0 || s_mode==11 || s_mode==12 || s_mode==13 || s_mode==18 || alljt65 || allq65)//ft8 ft4 msk144 mskms 2.51 + jt65
    {
        //if (s_mode==0 || s_mode==2 || s_mode==3 || s_mode==12) qrg modes
        if (le_qrg->GetQrgActive()==2) SetTxRaportToRB(6,false);//2.60= goto CQ QRG
        else SetTxRaportToRB(5,false);//1.60= goto CQ
    }

    gen_msg();

    s_his_contest_exch = "";//reset
    s_his_contest_sn   = "";//reset
}
void HvTxW::ResetQSO()//2.43 only from button reset qso
{
    ResetQSO_p(true);
}
void HvTxW::SetNewDb()
{
    QString dir_in = AppPath;
    int i;
    for (i = dir_in.count()-1; i>=0; i--)
    {
        if (dir_in.at(i)=='/')
            break;
    }
    dir_in.remove(i,(dir_in.count()-i));
    //qDebug()<<dir_in;

    QFileDialog dialog;

    QString file = QFileDialog::getOpenFileName(this,
                   tr("Overwrite Locator Database"), dir_in, "BDMH or CALL3.TXT (*.dbmh CALL3.TXT)");
    if (!file.isEmpty())
    {
        TMsDb->SetNewDb(file);
        CheckBD();
        //TMsDb->CheckBD(le_his_call->getText());
    }
}
void HvTxW::CloseAllWidget()
{
    if (THvLogW->isVisible())
        THvLogW->close();
}
void HvTxW::ShowLog()
{
    THvLogW->Show_log();
    THvLogW->activateWindow();
}
void HvTxW::FormatRxRst()
{
    le_rst_rx->SetText(le_rst_rx->format_rpt(le_rst_rx->getText()),false);
}
void HvTxW::SetLogQsoStartDtEqEndDt(bool f)
{
    log_qso_startdt_eq_enddt = f; //qDebug()<<"log_qso_startdt_eq_enddt="<<log_qso_startdt_eq_enddt;
}
void HvTxW::SetAutoLogInfo()//2.75
{
    THvLogW->SetAutoLogInfo();
}
void HvTxW::CBEnableAliChanged(bool f)//2.75
{
    if (f)
    {
        if (dsty) b_add_to_log->setStyleSheet("QPushButton{background-color:rgb(155,90,90);}");
        else b_add_to_log->setStyleSheet("QPushButton{background-color:rgb(255,190,190);}");
    }
    else b_add_to_log->setStyleSheet("QPushButton{background-color:palette(Button);}");
}
void HvTxW::AddToLogButton()
{
    AddToLog_p(false);
}
void HvTxW::SetLogAutoComm(bool f)
{
    f_off_auto_comm = f;
    THvLogW->SetLogAutoComm(f);
}
void HvTxW::AddToLog_p(bool direct_save_to_log)//false direct save to log off;  true direct save to log on
{
    FormatRxRst();
    if (block_call && !direct_save_to_log)
    {
        QString text = tr("Please set valid call sign");
        QMessageBox::critical(this, "MSHV", text, QMessageBox::Ok);//le_call->SetFocus();//za da otiva fokusa na call       
        return;
    }
    if (le_his_call->getText().isEmpty()) return;//2.50 protection call 100% need

    QString his_loc_t = LeHisLoc->getText();
    QString rsttxx = le_rst_tx->format_rpt(le_rst_tx->getText());
    QString rstrxx = le_rst_tx->format_rpt(le_rst_rx->getText());
    int pos_bcup = MultiAnswerMod->FindCallBackupDB(le_his_call->getText());
    if (pos_bcup != -1)
    {
        QString bctxr,bcrxr,bcg,bcs,bce;
        MultiAnswerMod->GetBackupDB(pos_bcup,bctxr,bcrxr,bcg,bcs,bce);
        if (!le_rst_tx->isRealRpt() && !bctxr.isEmpty()) rsttxx = bctxr;
        if (!le_rst_rx->isRealRpt() && !bcrxr.isEmpty()) rstrxx = bcrxr;
        if (his_loc_t.isEmpty()) his_loc_t = bcg;
        if (s_his_contest_sn.isEmpty()) s_his_contest_sn = bcs;
        if (s_his_contest_exch.isEmpty()) s_his_contest_exch = bce;
    }

    QStringList lst;
    QString tfrq = "0";
    if (s_mode==10)//for pi4   && his_loc_t->isEmpty()
        TRadioAndNetW->FindFreqLocFromBcnList(le_his_call->getText(),1,his_loc_t,tfrq);

    if (tfrq=="0")//1.61=
    {
        tfrq = FREQ_GLOBAL.mid(0,FREQ_GLOBAL.size()-2);
        double round0 = tfrq.toDouble();//2.74
        double roundi = round0/10.0;
        roundi = round(roundi);
        tfrq = QString("%1").arg((int)roundi); //2^32 =4294967296
    }

    QString comm = "";//2.01 Standard
    QString txsn = "";
    QString rxsn = "";
    QString txex = "";
    QString rxex = "";
    int show_cont_id = 0;//0=no, 1=sn, 2=exch 3=sn+exch
    QString cont_id = "0";
    if (s_cont_type>1 && (s_mode==0 || s_mode==11 || s_mode==13 || s_mode==18 || allq65 || (alljt65 && s_cont_id==16)))//ft4 !f_multi_answer_mod &&
    {
        //All=2 and up
        comm = s_cont_name[s_cont_id];
        cont_id = QString("%1").arg(s_cont_id);
        if (s_cont_type==3)//EU VHF Contest
        {
            txsn = QString("%1").arg(sb_txsn_v2->Value());
            if (!s_his_contest_sn.isEmpty())
            {
                int sn = s_his_contest_sn.toInt();
                rxsn = QString("%1").arg(sn);
            }
            show_cont_id = 1;
        }
        else if (s_cont_type==4)//ARRL Field Day
        {
            txex = s_my_contest_exch;
            rxex = s_his_contest_exch;
            show_cont_id = 2;
        }
        else if (s_cont_type==5)//All RU
        {
            txex = s_my_contest_exch;
            rxex = s_his_contest_exch;
            if (f_cont5_ru_dx) txsn = QString("%1").arg(sb_txsn_v2->Value());
            if (!s_his_contest_sn.isEmpty())
            {
                int sn = s_his_contest_sn.toInt();
                rxsn = QString("%1").arg(sn);
            }
            show_cont_id = 3;
        }
    }
    //if (s_f_rpt_rsq && (s_mode > 0 || s_mode < 7))//jtms,fsk,iscat,jt6m
    if (s_f_rpt_rsq && (s_mode > 0 && s_mode < 7))//2.12 jtms,fsk,iscat,jt6m
    {
        comm = s_cont_name[1];
        txsn = QString("%1").arg(sb_txsn->Value());//QString("%1").arg(sb_txsn->Value(),3,10,QChar('0'));
        rxsn = "";
        show_cont_id = 1;
        cont_id = "1";
    }

    if (direct_save_to_log && (s_mode==0 || s_mode==11 || s_mode==12 || s_mode==13 || s_mode==18 || allq65) && s_cont_type==0) comm = "Auto login";//log Automatically        
    //qDebug()<<direct_save_to_log<<s_mode<<s_id_contest_mode<<comm;

    if (s_mode==11 && id_mshf>0)//2.76fs
    {    	
    	if (id_mshf==1) 
    	{
    		if (!comm.isEmpty()) comm.append(" ");
    		comm.append("SH");    		
   		}
        if (id_mshf==2)
        {
        	if (!comm.isEmpty()) comm.append(" ");
        	comm.append("SF");        	
       	}	
   	}
    if (f_off_auto_comm) comm = "";

    if (log_qso_startdt_eq_enddt)
    {
        lst<<s_log_data_now<<s_log_time_now<<s_log_data_now<<s_log_time_now<<le_his_call->getText()<<his_loc_t
        <<rsttxx<<rstrxx<<ModeStr(s_mode)
        <<s_band<<tfrq<<""<<comm<<""<<txsn<<rxsn<<txex<<rxex<<cont_id<<s_trmN<<""<<""<<""<<""<<"";
    }
    else
    {
        lst<<s_log_data_start<<s_log_time_start<<s_log_data_now<<s_log_time_now<<le_his_call->getText()<<his_loc_t
        <<rsttxx<<rstrxx<<ModeStr(s_mode)
        <<s_band<<tfrq<<""<<comm<<""<<txsn<<rxsn<<txex<<rxex<<cont_id<<s_trmN<<""<<""<<""<<""<<"";
    }

    f_add_to_log_started = true;

    bool show_add_dialog = true;
    if (direct_save_to_log)
    {
        show_cont_id = 0;
        show_add_dialog = false;
    }

    if (THvLogW->Insert(lst,true,show_add_dialog,show_cont_id,true,true))//show_cont_id 0=no, 1=sn, 2=exch 3=sn+exch
    {
        s_last_call_for_BDB = le_his_call->getText();
        MultiAnswerMod->SetBackupDB(le_his_call->getText(),//call
                                    rsttxx,//txr
                                    rstrxx,//rxr
                                    his_loc_t,//g
                                    rxsn,//s
                                    rxex);//e

        s_last_call_for_log = "NO_CALL"; //for reset time
        s_last_bccall_tolog_excp = MultiAnswerMod->FindBaseFullCallRemAllSlash(lst.at(4));//2.02 from Queued exception
        MultiAnswerMod->SetLastBcCallToLog(s_last_bccall_tolog_excp);

        if ((s_cont_type==3 || f_cont5_ru_dx) && (s_mode==0 || s_mode==11 || s_mode==13 || s_mode==18 || allq65))//ft4 f_cont5_ru_dx
        {
            int sn = sb_txsn_v2->Value();
            sb_txsn_v2->SetValue(sn+1);
        }
        //if (s_f_rpt_rsq && (s_mode>0 || s_mode<7))//Old Contest
        if (s_f_rpt_rsq && (s_mode>0 && s_mode<7))//2.12 Old Contest
        {
            int sn = sb_txsn->Value();
            sb_txsn->SetValue(sn+1);
        }

        if (s_txt_mark[19] || s_txt_mark[22])
        {
            THvLogW->SetMarkTextAllRuleChanged(s_txt_mark,s_mode,s_band);
        }
    }
    f_add_to_log_started = false;
}
void HvTxW::AddToLogMultiAnswerQSO(QStringList l)
{
    //log_data_start & log_time_start, his_call, his_loc, rst_tx,  rst_rx
    //l.at(0),                         l.at(1),  l.at(2), l.at(3), l.at(4)
    if (l.count()<9) return;//2.47 for any case
    if (l.at(1).isEmpty()) return;//2.50 protection call 100% need

    QStringList ldt = l.at(0).split(" ");
    QString data_start = "";
    QString time_start = "";
    if (ldt.count()>1)
    {
        data_start = ldt.at(0);
        time_start = ldt.at(1);
    }

    QString his_loc_t = l.at(2);
    int pos_bcup = MultiAnswerMod->FindCallBackupDB(l.at(1));
    if (pos_bcup != -1)
    {
        QString bctxr,bcrxr,bcg,bcs,bce;
        MultiAnswerMod->GetBackupDB(pos_bcup,bctxr,bcrxr,bcg,bcs,bce);
        if (l.at(3).isEmpty()) l.replace(3,bctxr);
        if (l.at(4).isEmpty()) l.replace(4,bcrxr);
        if (his_loc_t.isEmpty()) his_loc_t = bcg;
        if (l.at(6).isEmpty()) l.replace(6,bcs);
        if (l.at(7).isEmpty()) l.replace(7,bce);
    }

    if (his_loc_t.isEmpty()) his_loc_t = TMsDb->CheckBD(l.at(1));// 1.73 if no loc try to find from DB        

    QStringList lst;
    QString tfrq = FREQ_GLOBAL.mid(0,FREQ_GLOBAL.size()-2);
    double round0 = tfrq.toDouble();//2.74
    double roundi = round0/10.0;
    roundi = round(roundi);
    tfrq = QString("%1").arg((int)roundi); //2^32 =4294967296

    QString txsn = "";
    QString txex = "";
    if (f_multi_answer_mod_std && s_cont_type>1 && (s_mode==0 || s_mode==11 || s_mode==13 || s_mode==18 || allq65))//ft4 !f_multi_answer_mod &&
    {
        if (s_cont_type==3) txsn = QString("%1").arg(sb_txsn_v2->Value());
        else if (s_cont_type==4) txex = s_my_contest_exch;
        else if (s_cont_type==5)
        {
            txex = s_my_contest_exch;
            if (f_cont5_ru_dx) txsn = QString("%1").arg(sb_txsn_v2->Value());
        }
    }

    QString comm = l.at(5);
    if (s_mode==11 && id_mshf>0)//2.76fs
    {
    	if (id_mshf==1) comm.append(" SH");
        if (id_mshf==2) comm.append(" SF");	
   	}
    if (f_off_auto_comm) comm = "";
    //QString rstrxx = l.at(4); //s_cont_type=2,4 no rx rpt
    //if (rstrxx.isEmpty() && (s_cont_type==2 || s_cont_type==4)) rstrxx = "+00";

    if (log_qso_startdt_eq_enddt)
    {
        //MA have format rpt
        lst<<s_log_data_now<<s_log_time_now<<s_log_data_now<<s_log_time_now<<l.at(1)<<his_loc_t
        <<l.at(3)<<l.at(4)<<ModeStr(s_mode)
        <<s_band<<tfrq<<""<<comm<<""<<txsn<<l.at(6)<<txex<<l.at(7)<<l.at(8)<<s_trmN<<""<<""<<""<<""<<"";
    }
    else
    {
        //MA have format rpt
        lst<<data_start<<time_start<<s_log_data_now<<s_log_time_now<<l.at(1)<<his_loc_t
        <<l.at(3)<<l.at(4)<<ModeStr(s_mode)
        <<s_band<<tfrq<<""<<comm<<""<<txsn<<l.at(6)<<txex<<l.at(7)<<l.at(8)<<s_trmN<<""<<""<<""<<""<<"";
    }

    if (THvLogW->Insert(lst,true,false,0,true,true))//0=show_cont_id 0=no, 1=sn, 2=exch 3=sn+exch
    {
        MultiAnswerMod->SetBackupDB(l.at(1),//call
                                    l.at(3),//txr
                                    l.at(4),//rxr
                                    his_loc_t,//g
                                    l.at(6),//s
                                    l.at(7));//e

        //s_last_call_for_log = "NO_CALL"; //reset
        //qDebug()<<"RESET";
        s_last_bccall_tolog_excp = MultiAnswerMod->FindBaseFullCallRemAllSlash(lst.at(4));//2.02 from Queued exception
        MultiAnswerMod->SetLastBcCallToLog(s_last_bccall_tolog_excp);

        //																		  s_mode==0 || <- not exist
        if (f_multi_answer_mod_std && (s_cont_type==3 || f_cont5_ru_dx) && (s_mode==11 || s_mode==13 || s_mode==18 || allq65))
        {
            int sn = sb_txsn_v2->Value();
            sb_txsn_v2->SetValue(sn+1);
        }

        if (s_txt_mark[19] || s_txt_mark[22])
        {
            THvLogW->SetMarkTextAllRuleChanged(s_txt_mark,s_mode,s_band);
        }
    }
}
bool HvTxW::SetTimePeriod_p(QString time_1,int msg_pos)//1.56=,QString str
{
    bool f_fs_se;//for MSK144,JT65,FT8  msg_pos if 0<-v obratnia period 1,2...100<-normal logic
    int period_time_msec = (int)(SB_PeriodTime->get_period_time()*100.0);
    int time_ss =  time_1.midRef(4,2).toInt();//get seconds 120023
    int time_mm =  time_1.midRef(2,2).toInt();//get min 120023
    int time_ms = 0;
    if (s_mode==13)//ft4
    {
        if (time_ss == 7 || time_ss == 22 || time_ss == 37 || time_ss == 52) time_ms = 50;
        //if (time_ss % 15 == 7)  time_ms = 50;
    }
    if (s_mode==18)//ft2
    {
    	if      (time_ss == 3 || time_ss == 18 || time_ss == 33 || time_ss == 48) time_ms = 75;
    	else if (time_ss == 7 || time_ss == 22 || time_ss == 37 || time_ss == 52) time_ms = 50;
    	else if (time_ss ==11 || time_ss == 26 || time_ss == 41 || time_ss == 56) time_ms = 25;
    	/*if      (time_ss % 15 == 3)  time_ms = 75; // 0.75s -> 3 units
		else if (time_ss % 15 == 7)  time_ms = 50; // 0.50s -> 2 units
		else if (time_ss % 15 == 11) time_ms = 25; // 0.25s -> 1 unit*/               
    }
    int time_pos_2period = (time_mm*6000)+(time_ss*100)+time_ms; //qDebug()<<time_mm<<time_ss<<time_pos_2period<<period_time_msec;

    //time_pos_2period = time_pos_2period % (period_time_msec*2);// not work correct
    while (time_pos_2period>=period_time_msec*2) time_pos_2period-=period_time_msec*2;

    if (time_pos_2period<period_time_msec)
    {
        if (msg_pos>0) f_fs_se = true;
        else f_fs_se = false;
    }
    else
    {
        if (msg_pos>0) f_fs_se = false;
        else f_fs_se = true;
    }
    //qDebug()<<"To30s="<<time_1<<time_pos_2period<<msg_pos<<f_fs_se;

    if (f_ma_first_tx && f_fs_se) return false; //2.71
    if (f_fs_se)
    {
        rb_tx_se->setChecked(true);
        rb_tx_fi->setChecked(false);
    }
    else
    {
        rb_tx_fi->setChecked(true);
        rb_tx_se->setChecked(false);
    }
    return true;
}
int HvTxW::FindSelPosForTP(QString str,QString all_txt)//1.56=
{
    //1.56= find msg_pos 0,1,2 call
    QStringList list_all_txt = all_txt.split(" ");
    int msg_pos = 0;
    for (msg_pos = 0; msg_pos < list_all_txt.count(); msg_pos++)
    {
        QString tmp_s = list_all_txt.at(msg_pos);
        if (tmp_s.contains(str)) break;
    }
    return msg_pos;
}
void HvTxW::SetRecognizeTp1(bool f)
{
    f_recognize_tp1 = f;//jtms, fsk, iscats, jt6m
    //qDebug()<<"f_recognize_tp1="<<f_recognize_tp1;
}
void HvTxW::SetRecognizeTp2(bool f)
{
    f_recognize_tp2 = f;//msk144, jt65, ft8
    //qDebug()<<"f_recognize_tp2="<<f_recognize_tp2;
}
void HvTxW::Set2ClickDecListAutoOn(bool f)
{
    s_2click_list_autu_on = f;
}
void HvTxW::SetDoubleClickFromAllAutoOn()//2.45
{
    if ((s_mode==0 || s_mode==11 || s_mode==12 || s_mode==13 || s_mode==18 || alljt65 || allq65) &&
            !f_auto_on && s_2click_list_autu_on)//2.51 msk144, ft8, mskms, ft4 +jt65
        auto_on();
}
void HvTxW::IsCallDupeInLog(QString call,int maxqso,bool &f)
{
    //qDebug()<<"ISSSS Dupe="<<call<<s_band<<ModeStr(s_mode);
    THvLogW->IsCallDupeInLog(call,ModeStr(s_mode),s_band,maxqso,f);
}
void HvTxW::SetInfoDupeQso(bool f)
{
    info_dupe_qso = f;
    MultiAnswerMod->SetInfoDupeQso(f);
}
void HvTxW::SetStartFromTx2Tx1(bool f)
{
    s_start_qso_from_tx2 = f;
    MultiAnswerMod->SetStartFromTx2Tx1(f);
}
void HvTxW::SetButClrQueuedCall()
{
    gen_msg();
}
void HvTxW::DlDetectTextInMsg(QString str ,QString &hisCall_inmsg)
{
    QString hisLoc_inmsg  = "";
    QString myCall_inmsg  = "";
    QString rpt_inmsg  = "";
    QString cont_r_inmsg  = ""; //msk144 ft8 contest mode R fictive
    QString rr73_inmsg  = "";   ////1.54=RR73 msk144 ft8 rr73
    int row_queue = -1;// -1=ident find mam   -2=no find mam
    int row_now = -1;  // -1=ident find mam   -2=no find mam
    QString sn_inmsg = "";//v.2.0 fictive
    QString arrl_exch_imsg = "";//v.2.01 fictive

    MultiAnswerMod->DetectTextInMsg(str, hisCall_inmsg,hisLoc_inmsg,myCall_inmsg,rpt_inmsg,
                                    cont_r_inmsg,rr73_inmsg,row_queue,row_now,sn_inmsg,arrl_exch_imsg);
}
void HvTxW::SetDoQRG(QString s,QString all_txt)//2.71
{
    QString newqrg = FREQ_GLOBAL;
    QString qrg = QString("%1").arg(s.toInt(),3,10,QChar('0'));
    int beg = newqrg.count() - 6;
    newqrg.replace(beg,3,qrg); //14.285.000 to Hz
    QStringList lt;
    lt<<newqrg<<all_txt;
    emit EmitQrgQSY(lt);
    SetDefFreqGlobal(3,newqrg);//2.74 0=mybe frq,no mod, 1=mybe frq,mod, 2=frq,mod, 3=frq,no mod
}
//#include <unistd.h>//usleep x86_64
void HvTxW::DecListTextAll(QString all_txt,QString str,QString tp,QString tx_rpt,QString freq)
{
    //qDebug()<<all_txt<<str<<tp<<tx_rpt<<freq;
    if ((s_mode==11 || s_mode==13 || s_mode==18 || allq65) && f_multi_answer_mod)//ft8 ft4 q65
    {
        if (!f_auto_on && !str.isEmpty() && f_recognize_tp2)// && !f_ma_first_tx
        {
            int msg_pos = FindSelPosForTP(str,all_txt);
            //SetTimePeriod_p(tp,msg_pos);
            if (!SetTimePeriod_p(tp,msg_pos)) //2.71
            {
                if (!f_multi_answer_mod_std) return;
                QLabel *lb = new QLabel(this,Qt::ToolTip | Qt::BypassGraphicsProxyWidget);
                lb->setStyleSheet("QFrame, QLabel {border: 2px solid rgb(10,70,10); border-radius: 0px;"
                                  "padding-left: 2px; padding-top: 1px; padding-right: 2px; padding-bottom: 1px;"
                                  "color: black; background-color: #ffffe1;}");
                lb->setAlignment(Qt::AlignCenter);
                lb->setText(tr("In Multi Answering Auto Seq Protocol Standard")+"\n"+
                            tr("In the HF bands")+"\n"+
                            tr("Not possible to TX in second period if TX Slots is more then one"));
                QPoint post = this->mapToGlobal(QPoint(0,0));
                post+=QPoint(this->width()/2-160,(-this->pos().y()/2));
                lb->move(post);
                lb->show();
                QTimer::singleShot(5000,lb,SLOT(deleteLater()));
                return;
            }
        }

        QString hisCall_inmsg_for_ap = "";
        QString hisLoc_inmsg_for_dist = "";
        MultiAnswerMod->DecListTextAll(tx_rpt,str,freq,true,hisCall_inmsg_for_ap,hisLoc_inmsg_for_dist);//true double click

        if (!hisCall_inmsg_for_ap.isEmpty())//1.73
        {
            bool fc1,fc2; uint8_t noQSO;
            MultiAnswerMod->isStandardCalls(list_macros.at(0),hisCall_inmsg_for_ap,fc1,fc2,noQSO);
            if (noQSO==100)
            {
                emit EmitGBlockListExp(true);//2.15
                setFocus(Qt::MouseFocusReason);
                b_gen_msg->setFocus();
                QString tex0 = tr("There were two non-standard callsigns,");//2.76.2
                if (id_mshf==2) tex0 = tr("There were non-standard callsign,");
                QString text = tex0+"\n"+
                               tr("My Call")+": "+list_macros.at(0)+"\n"+
                               tr("His Call")+": "+hisCall_inmsg_for_ap+"\n"+
                               tr("so this QSO is not possible in this protocol");
                QMessageBox::warning(this, "MSHV", text, QMessageBox::Ok);
                emit EmitGBlockListExp(false);//2.15
                return;
            }
            if (f_cntest_only_sdtc)// no contest here
            {
                if (!fc1 || !fc2)
                {
                    emit EmitGBlockListExp(true);//2.15
                    setFocus(Qt::MouseFocusReason);
                    b_gen_msg->setFocus();
                    QString text = tr("There were non-standard callsign,")+"\n"+
                                   tr("My Call")+": "+list_macros.at(0)+"\n"+
                                   tr("His Call")+": "+hisCall_inmsg_for_ap+"\n"+
                                   tr("so this QSO is not possible in this protocol");
                    QMessageBox::warning(this, "MSHV", text, QMessageBox::Ok);
                    emit EmitGBlockListExp(false);//2.15
                    return;
                }
            }

            s_last_call_for_BDB = hisCall_inmsg_for_ap;//protect no update from ModeChanget and ResetQSO

            le_his_call->SetText(hisCall_inmsg_for_ap);
            gen_msg();// for any case
        }
        if (!hisLoc_inmsg_for_dist.isEmpty()) LeHisLoc->SetText(hisLoc_inmsg_for_dist);
        return;
    }

    //bool f_have_changes = false;
    //if (s_mode==0 || s_mode==7 || s_mode==8 || s_mode==9 || s_mode==11 || s_mode==12 || s_mode==13|| s_mode==18)//msk144, jt65, ft8 ft4
    //bool f_jt65 = false;
    //if (s_mode==7 || s_mode==8 || s_mode==9) f_jt65 = true;//2.51 jt65
    if (s_mode==0 || alljt65 || s_mode==11 || s_mode==12 || s_mode==13 || s_mode==18 || allq65)//msk144, jt65, ft8, mskms, ft4
    {
        QString hisCall_inmsg = "";
        QString hisLoc_inmsg  = "";
        QString myCall_inmsg  = "";
        QString rpt_inmsg  = "";
        QString cont_r_inmsg  = ""; //msk144 ft8 contest mode
        QString rr73_inmsg  = ""; ////1.54=RR73 msk144 ft8 rr73
        //QString hisCallFromLe = le_his_call->getText(); hisCallFromLe,
        int row_queue = -2;// -1=ident find mam   -2=no find mam
        int row_now = -2;  // -1=ident find mam   -2=no find mam
        QString sn_inmsg = "";//v.2.01
        QString arrl_exch_imsg = "";//v.2.01

        MultiAnswerMod->DetectTextInMsg(str, hisCall_inmsg,hisLoc_inmsg,myCall_inmsg,rpt_inmsg,
                                        cont_r_inmsg,rr73_inmsg,row_queue,row_now,sn_inmsg,arrl_exch_imsg);
        /*qDebug()<<"msg="<<str;
        qDebug()<<"hisCall="<<hisCall_inmsg<<"hisLoc="<<hisLoc_inmsg<<"myCall="<<myCall_inmsg<<
        "rpt="<<rpt_inmsg<<"cont_r_inmsg="<<cont_r_inmsg<<"rr73="<<rr73_inmsg<<"sn_inmsg="<<sn_inmsg<<
        "arrl_exch_imsg="<<arrl_exch_imsg;*/

        if ((s_mode==0 || s_mode==11 || s_mode==13 || s_mode==18 || allq65) && hisCall_inmsg.isEmpty()) return;//2.75 for <...>

        int no0_find1_ooo2 = 1;//2.51 0=no, find rpt=1, ooo_jt65=2
        if (alljt65)//2.51 MSGs -> RO, RRR, 73, LZ2HV SP9HWY JO90 OOO
        {
            QStringList ljt65 = str.split(" ");
            if (ljt65.count()==1)
            {
                if (ljt65.at(0)=="RO" || ljt65.at(0)=="RRR" || ljt65.at(0)=="73") no0_find1_ooo2 = 0;
            }
            else if (ljt65.count()==4)
            {
                if (ljt65.at(3)=="OOO") no0_find1_ooo2 = 2;
            }
        }

        if (!hisCall_inmsg.isEmpty())
        {
            bool fc1,fc2; uint8_t noQSO;
            MultiAnswerMod->isStandardCalls(list_macros.at(0),hisCall_inmsg,fc1,fc2,noQSO);
            if (f_two_no_sdtc && noQSO==100)//if (f_two_no_sdtc)
            {
                emit EmitGBlockListExp(true);//2.15
                setFocus(Qt::MouseFocusReason);
                b_gen_msg->setFocus();
                QString tex0 = tr("There were two non-standard callsigns,");//2.76.2
                if (id_mshf==1) tex0 = tr("There were non-standard callsign,");
                QString text = tex0+"\n"+
                               tr("My Call")+": "+list_macros.at(0)+"\n"+
                               tr("His Call")+": "+hisCall_inmsg+"\n"+
                               tr("so this QSO is not possible in this protocol");
                QMessageBox::warning(this, "MSHV", text, QMessageBox::Ok);
                emit EmitGBlockListExp(false);//2.15
                return;
            }
            //qDebug()<<"f_cntest_only_sdtc"<<f_cntest_only_sdtc<<fc1<<fc2;
            if (f_cntest_only_sdtc)
            {
                if (!fc1 || !fc2)
                {
                    emit EmitGBlockListExp(true);//2.15
                    setFocus(Qt::MouseFocusReason);
                    b_gen_msg->setFocus();
                    QString text = tr("There were non-standard callsign,")+"\n"+
                                   tr("My Call")+": "+list_macros.at(0)+"\n"+
                                   tr("His Call")+": "+hisCall_inmsg+"\n"+
                                   tr("so this QSO is not possible in this protocol");
                    QMessageBox::warning(this, "MSHV", text, QMessageBox::Ok);
                    emit EmitGBlockListExp(false);//2.15
                    return;
                }
            }

            if (info_dupe_qso)
            {
                if (MultiAnswerMod->DialogIsCallDupeInLog(hisCall_inmsg)) return;
            }

            //qDebug()<<"hisCall="<<hisCall_inmsg<<myCall_inmsg<<rpt_inmsg<<s_id_contest_mode;
            if (TQueuedCall->isUseQueueCont() && s_cont_type>1 && (s_mode==0 || s_mode==11 || s_mode==13 || s_mode==18 || allq65))//2.02 Queued ft4
            {
                if (TQueuedCall->haveQueuedCall())
                {
                    emit EmitGBlockListExp(true);//2.15
                    setFocus(Qt::MouseFocusReason);
                    b_gen_msg->setFocus();
                    QString text = tr("You have a Call in the queue already")+"\n"+TQueuedCall->GetQueuedCallData().at(0);
                    QMessageBox::warning(this, "MSHV", text, QMessageBox::Ok);
                    emit EmitGBlockListExp(false);//2.15
                    return;//2.59 no place have Queued
                }
                if (!myCall_inmsg.isEmpty())
                {
                    if (le_his_call->getText()!="" && (!rpt_inmsg.isEmpty() || !arrl_exch_imsg.isEmpty() || !hisLoc_inmsg.isEmpty()))
                    {
                        QString c1 = MultiAnswerMod->FindBaseFullCallRemAllSlash(le_his_call->getText());
                        QString c2 = MultiAnswerMod->FindBaseFullCallRemAllSlash(hisCall_inmsg);
                        //qDebug()<<"222hisCall="<<c1<<c2<<hisCall_inmsg<<myCall_inmsg<<rpt_inmsg<<s_id_contest_mode;
                        if (c1!=c2)
                        {
                            tx_rpt = MultiAnswerMod->TryFormatRPTtoRST(tx_rpt);//2.59
                            QString my_rpt = le_rst_tx->format_rpt_tx(tx_rpt);

                            TQueuedCall->SetQueuedCall(hisCall_inmsg,my_rpt,rpt_inmsg,arrl_exch_imsg,sn_inmsg,hisLoc_inmsg,freq);
                            gen_msg();
                            return;
                        }
                    }
                }
            }

            ////////// ORG_QSY only for MSK, FT8/4 and not for contests and MA ///////////////
            if (s_cont_type==0 && (s_mode==0 || s_mode==12 || s_mode==11 || s_mode==13 || s_mode==18))// || allq65=?
            {
                QStringList lstr = str.split(" ");
                if (lstr.count() > 2) //CQ 236 LZ2HV
                {
                    QString s = lstr.at(1);
                    if (s.count()<4 && lstr.at(0)=="CQ")//only kHz 236
                    {
                        bool alldig = true;
                        for (int j = 0; j < s.count(); ++j)
                        {
                            if (s.at(j).isLetter())
                            {
                                alldig = false;
                                break;
                            }
                        }
                        if (alldig) SetDoQRG(s,all_txt);
                    }
                }
            }
            ///////////////////////////// END ORG_QSY ////////////////////////////////////////

            RefreshBackupDB();
            s_last_call_for_BDB = "_";

            le_his_call->SetText(hisCall_inmsg);

            //2.59
            tx_rpt = MultiAnswerMod->TryFormatRPTtoRST(tx_rpt);
            QString s = le_rst_tx->format_rpt_tx(tx_rpt);
            le_rst_tx->SetText(s,true);
            if (TQueuedCall->isLastFromQueued()) TQueuedCall->ClrQueuedCall(false);//2.59
            gen_msg();  //emit EmitFreqTxW(freq.toDouble());

            //if (s_mode==0 || s_mode==11 || s_mode==12 || s_mode==13|| s_mode==18)//ft8 ft4
            if (no0_find1_ooo2>0)//2.51 msk ft8 ft4 +jt65    0=no, find rpt=1, ooo_jt65=2
            {
                //qDebug()<<"hisCall="<<hisCall_inmsg<<myCall_inmsg;
                if (!myCall_inmsg.isEmpty())//1.78
                {
                    if (rpt_inmsg.isEmpty() && hisLoc_inmsg.isEmpty() && cont_r_inmsg.isEmpty() && rr73_inmsg.isEmpty() && arrl_exch_imsg.isEmpty())
                    {//msg=0 only calls exeption if have slashs     for ARRL Field Day && arrl_exch_imsg.isEmpty()
                        SetTxRaportToRB(1,true);
                    }
                    //else if (rpt_inmsg.isEmpty() && !hisLoc_inmsg.isEmpty())//1.84
                    //else if (rpt_inmsg.isEmpty() && (!hisLoc_inmsg.isEmpty() || s_id_contest_mode==4))//2.01
                    else if (rpt_inmsg.isEmpty() && (!hisLoc_inmsg.isEmpty() || !arrl_exch_imsg.isEmpty() /*&& rr73_inmsg!="RR73"*/ ))//2.01
                    {//msg=0 KN23    contest 1 and 3 no report
                        if (no0_find1_ooo2==2)//2.15 jt65=OOO msg   0=no, find rpt=1, ooo_jt65=2
                            SetTxRaportToRB(2,true);
                        else if (!cont_r_inmsg.isEmpty())  //msk144 ft8 contest mode
                            SetTxRaportToRB(3,true);
                        else
                        {
                            if (s_cont_type==2 || s_cont_type==4) //msk144 ft8 contest mode
                                SetTxRaportToRB(2,true);
                            else
                                SetTxRaportToRB(1,true);
                        }
                        if (!arrl_exch_imsg.isEmpty())
                            s_his_contest_exch = arrl_exch_imsg;
                    }
                    else if (!rpt_inmsg.isEmpty() && rpt_inmsg.at(0)!='R' && rpt_inmsg.at(rpt_inmsg.count()-1).isDigit() && rpt_inmsg!="73")
                    {//msg=1 +00      contest 2 and 4 have teport
                        le_rst_rx->SetText(le_rst_rx->format_rpt(rpt_inmsg),true);
                        if (!cont_r_inmsg.isEmpty())
                            SetTxRaportToRB(3,true);
                        else
                            SetTxRaportToRB(2,true);

                        //qDebug()<<"msg=1 +00"<<s_id_contest_mode;
                        //SetTxRaportToRB(2);
                        //no need down update if (!hisLoc_inmsg.isEmpty())        //2.01 update EU C LOC
                        //LeHisLoc->SetText(hisLoc_inmsg);
                        if (!sn_inmsg.isEmpty())
                            s_his_contest_sn = sn_inmsg;
                        if (!arrl_exch_imsg.isEmpty())
                            s_his_contest_exch = arrl_exch_imsg;
                    }
                    else if (!rpt_inmsg.isEmpty() && rpt_inmsg.at(0)=='R' && rpt_inmsg.at(rpt_inmsg.count()-1).isDigit())
                    {//msg=2 R+00
                        rpt_inmsg.remove("R");
                        le_rst_rx->SetText(le_rst_rx->format_rpt(rpt_inmsg),true);
                        SetTxRaportToRB(3,true);

                        //no need down update if (!hisLoc_inmsg.isEmpty())        //2.01 update EU C LOC
                        //LeHisLoc->SetText(hisLoc_inmsg);
                        if (!sn_inmsg.isEmpty())
                            s_his_contest_sn = sn_inmsg;
                        if (!arrl_exch_imsg.isEmpty())
                            s_his_contest_exch = arrl_exch_imsg;
                    }
                    else if (!rpt_inmsg.isEmpty() && rpt_inmsg.at(0)=='R' && !rpt_inmsg.at(rpt_inmsg.count()-1).isDigit() &&
                             rpt_inmsg.at(rpt_inmsg.count()-1)=='R')
                    {//msg=3 RRR
                        SetTxRaportToRB(4,true);
                        count_73_auto_seq = 1;//2.50 if (!f_cfm73)
                    }
                    else if ((!rpt_inmsg.isEmpty() && rpt_inmsg=="73") || rr73_inmsg=="RR73")
                    {//msg=4 73 RR73
                        ////1.47 logic tx 73 several times and one
                        SetTxRaportToRB(4,true);
                        count_73_auto_seq = 1;//2.50 if (!f_cfm73)
                    }
                    else
                    {   // rpt no exist and for me
                        if (s_start_qso_from_tx2 && s_cont_type!=3)//2.31 no in eu vhf
                        {
                            SetTxRaportToRB(1,true);
                            one_addtolog_auto_seq = false;//2.31 start from TX2 and TX4=RR73 exception
                        }
                        else
                            SetTxRaportToRB(0,true);
                    } //qDebug()<<"count_73_auto_seq="<<count_73_auto_seq;
                }
                else
                {   // rpt no exist and not for me
                    if (s_start_qso_from_tx2 && s_cont_type!=3)//2.31 no in eu vhf
                    {
                        SetTxRaportToRB(1,true);//2.59 remove old start new on the flay, old=false
                        one_addtolog_auto_seq = false;//2.31 start from TX2 and TX4=RR73 exception
                    }
                    else SetTxRaportToRB(0,true);//2.59 remove old start new on the flay, old=false
                }
            }

            if (s_mode!=10 && f_recognize_tp2)////1.56= no need bit for any case tp no need in PI4
            {
                int msg_pos = FindSelPosForTP(str,all_txt);//qDebug()<<"msg_pos="<<str<<all_txt<<msg_pos;
                SetTimePeriod_p(tp,msg_pos);
            }
            //f_have_changes = true;
        }
        if (!hisLoc_inmsg.isEmpty()) LeHisLoc->SetText(hisLoc_inmsg);
        //if (f_have_changes)
        //gen_msg();
        if (no0_find1_ooo2>0) SetDoubleClickFromAllAutoOn();//2.51 0=no, find rpt=1, ooo_jt65=2
    }
    else //jtms, fsk, iscats, jt6m, pi4
    {
        if (str.contains(' ')) return;//1.56= no need bit for any case
        if (THvQthLoc.isValidLocator(str)) LeHisLoc->SetText(str);
        else if (THvQthLoc.isValidCallsign(str))
        {
            if (info_dupe_qso)
            {
                if (MultiAnswerMod->DialogIsCallDupeInLog(str))
                    return;
            }

            le_his_call->SetText(str);

            if (s_mode==0 || s_mode==11 || s_mode==12 || s_mode==13 || s_mode==18 || allq65)//ft8 ft4 no need but for any case
            {
                if (s_start_qso_from_tx2 && s_cont_type!=3)//2.31 no in eu vhf
                {
                    SetTxRaportToRB(1,false);
                    one_addtolog_auto_seq = false;//2.31 start from TX2 and TX4=RR73 exception
                }
                else SetTxRaportToRB(0,false);
            }

            if (s_mode!=10 && f_recognize_tp1)//tp no need in PI4
            {
                //int msg_pos = FindSelPosForTP(str,all_txt);
                SetTimePeriod_p(tp,100);
            }
            //f_have_changes = true;
            gen_msg();
        }
        //if (f_have_changes)
        //gen_msg();
    }
    //if (!f_have_changes)//1.57= only if call change
    //return;
    //qDebug()<<"gen_msg();"<<str;
    //gen_msg();
}
void HvTxW::DecListTextRpt(QString str)//RX RPT
{
    if (s_cont_type>1 && (s_mode==0 || s_mode==11 || s_mode==13 || s_mode==18 || allq65) && TQueuedCall->haveQueuedCall())//ft4 !f_multi_answer_mod &&
        return;
    str = MultiAnswerMod->TryFormatRPTtoRST(str);//2.59
    QString s = le_rst_tx->format_rpt_tx(str);  //qDebug()<<"msg=1 +00"<<s<<str;
    le_rst_tx->SetText(s,true);                 //qDebug()<<"DecListTextRpt="<<le_rst_tx->getText();
    gen_msg();
}
void HvTxW::AddDb()
{
    if (block_call)
    {
        QString text = tr("Please set valid call sign");
        QMessageBox::critical(this, "MSHV", text, QMessageBox::Ok);
        //le_call->SetFocus();//za da otiva fokusa na call
        return;
    }
    if (block_loc)
    {
        QString text = tr("Please set valid locator");
        QMessageBox::critical(this, "MSHV", text, QMessageBox::Ok);
        //le_call->SetFocus();//za da otiva fokusa na call
        return;
    }
    if (!block_loc && !block_call)
    {
        QStringList list;
        list << le_his_call->getText() << LeHisLoc->getText();
        TMsDb->SetToDbNew(list);
        //LeHisLoc->setErrorColorLe(true);
        CheckBD();
    }
}
void HvTxW::CheckBD()
{
    //if(!block_loc)
    //LeHisLoc->SetText(TMsDb->CheckBD(le_his_call->getText()));
    Check("call");
    //LeHisLoc->setErrorColorLe(false);
}
void HvTxW::ExpandShrinkDf(bool f)//2.05
{
    SB_DfTolerance1->ExpandShrinkDf(f);
}
void HvTxW::SetRxDf(int dft)
{
    SB_DfTolerance1->setValue(dft);
}
void HvTxW::DfSdbChanged(int)
{
    emit EmitDfSdbChanged(SB_MinSigdB->value(),SB_DfTolerance1->value());

    //all vertikal displais onlyyyyy jt65abc pi4 ft8
    if (alljt65  || s_mode==10 || s_mode==11 || s_mode==13 || s_mode==18 || allq65)//all vertikal displais only jt65abc pi4 ft8 ft4
        emit EmitDfChanged(SB_DfTolerance1->value(),s_mode);

    if (!f_nosave)
    {
        s_minsigndb[s_mode] = SB_MinSigdB->value();
        //s_dftolerance[s_mode] = SB_DfTolerance1->value();
    } //qDebug()<<s_dftolerance[0]<<s_dftolerance[1]<<s_dftolerance[2];
}
void HvTxW::StartStopZap()
{
    // no zap in msk ft8 jt65abc pi4 ft4 q65
    //if (s_mode==0 || s_mode==7 || s_mode==8 || s_mode==9  || s_mode==10 || s_mode==11 || s_mode==12 || s_mode==13 || s_mode==18 || allq65)
    //return;
    if (s_mode==0 || s_mode>6) return; //2.65

    if (cb_zap->isChecked()) cb_zap->setChecked(false);
    else cb_zap->setChecked(true);
}
void HvTxW::SetZap(QString s)
{
    if (s.toInt()==1) cb_zap->setChecked(true);
    else cb_zap->setChecked(false);
}
void HvTxW::SetMinsigndb(QString s)
{
    QStringList ls=s.split("#");

    for (int i=0; i<COUNT_MODE; i++)
    {
        QString tstr = ModeStr(i)+"=";
        for (int j=0; j<ls.count(); j++)
        {
            if (ls[j].contains(tstr))
            {
                ls[j].remove(tstr);
                if (!ls[j].isEmpty())
                    s_minsigndb[i] = ls[j].toInt();
                break;
            }
        }
    }
}
void HvTxW::SetDftolerance(QString s)
{
    SB_DfTolerance1->SetDfAllModes(s);
}
void HvTxW::SetRptRsqSettings()
{    
    QSizePolicy spol_w = sizePolicy();//qDebug()<<"SetRptRsqSettings()"<<le_rst_tx->getText();
    //setSizePolicy(QSizePolicy::Ignored,QSizePolicy::Ignored);
    setSizePolicy(QSizePolicy::Ignored,spol_w.verticalPolicy());//2.51 only horizontal, vertical move list scroller
    //cb_zap->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum);//Ignored

    le_rst_tx->SetRptRsqSettings("TX",s_mode,s_f_rpt_rsq,cb_sh_rpt->isChecked(),s_cont_type);
    le_rst_rx->SetRptRsqSettings("RX",s_mode,s_f_rpt_rsq,cb_sh_rpt->isChecked(),s_cont_type);
    cb_sh_rpt->setText("Sh");//1.49

    if (s_mode==11 || s_mode==13 || s_mode==18 || allq65) //ft8 ft4 no rsq
    {
        if (s_cont_type==3 || f_cont5_ru_dx)
        {            
            if (f_multi_answer_mod && !f_multi_answer_mod_std) 
            {
            	sb_txsn_v2->SetHidden(true);
            	if (s_mode==11) cb_zap->setHidden(true);
            	else cb_zap->setHidden(false);
           	}
            else 
            {
            	cb_zap->setHidden(true);
            	sb_txsn_v2->SetHidden(false);            	
           	}
        }
        else
        {
            sb_txsn_v2->SetHidden(true);
			if      (s_mode==11 && f_multi_answer_mod && !f_multi_answer_mod_std) cb_zap->setHidden(true);//2.76
			else if (s_mode==11 && s_cont_type==0) cb_zap->setHidden(true);
			else cb_zap->setHidden(false);
        }
        if (s_cont_type>1)
        {           
            if (s_mode==11 && f_multi_answer_mod && !f_multi_answer_mod_std)
            {
            	SB_MinSigdB->setHidden(true);//2.76sf for remove only ->if (f_vsf_)
            	//else SB_MinSigdB->setHidden(false);//2.76sf for remove all
            	TQueuedCall->setHidden(true);
            	//if (f_vsf_)//2.76sf for remove only ->if (f_vsf_)
            	//{
            	cb_msh->setHidden(false);
            	cb_msf->setHidden(false);            		
           		//} 
           	}
            else
            {
            	cb_msh->setHidden(true);
            	cb_msf->setHidden(true);
            	if (f_multi_answer_mod && !f_multi_answer_mod_std)//2.76
            	{
            		TQueuedCall->setHidden(true);
            		SB_MinSigdB->setHidden(false);
           		}
           		else
           		{
           			SB_MinSigdB->setHidden(true);
           			TQueuedCall->setHidden(false);
          		}            	            	
           	}
        }
        else
        {
            TQueuedCall->setHidden(true);
            if (s_mode==11)
            {
            	SB_MinSigdB->setHidden(true);//2.76sf for remove only ->if (f_vsf_)
                //else SB_MinSigdB->setHidden(false);//2.76sf for remove all
                //if (f_vsf_)//2.76sf for remove only ->if (f_vsf_)
            	//{
                cb_msh->setHidden(false);
                cb_msf->setHidden(false);
                //}
            }
            else
            {
                cb_msh->setHidden(true);
                cb_msf->setHidden(true);
                SB_MinSigdB->setHidden(false);
            }
        }
        sb_txsn->SetHidden(true);
        cb_swl->setHidden(true);
        cb_sh_rpt->setHidden(true);
        pb_tx_to_rx->setHidden(false);
        pb_rx_to_tx->setHidden(false);
        cb_lock_txrx->setHidden(false);
    }
    else if (alljt65) //1.49 jt65abc
    {
        sb_txsn_v2->SetHidden(true);
        cb_zap->setHidden(false);
        TQueuedCall->setHidden(true);//ru quined
        cb_msh->setHidden(true);
        cb_msf->setHidden(true);
        SB_MinSigdB->setHidden(false);//ru quined
        pb_tx_to_rx->setHidden(true);
        pb_rx_to_tx->setHidden(true);
        cb_lock_txrx->setHidden(true);
        sb_txsn->SetHidden(true);
        cb_sh_rpt->setHidden(false);
        cb_swl->setHidden(true);
        cb_sh_rpt->setText("Sh HF/VHF");
    }
    else
    {
        if (s_mode==0 && (s_cont_type==3 || f_cont5_ru_dx))
        {
            cb_zap->setHidden(true);
            sb_txsn_v2->SetHidden(false);
        }
        else
        {
            sb_txsn_v2->SetHidden(true);
            cb_zap->setHidden(false);
        }    
        cb_msh->setHidden(true);
        cb_msf->setHidden(true);    
        if (s_mode==0 && s_cont_type>1)
        {
            SB_MinSigdB->setHidden(true);
            TQueuedCall->setHidden(false);
        }
        else
        {
            TQueuedCall->setHidden(true);
            SB_MinSigdB->setHidden(false);
        }        
        if (!s_f_rpt_rsq)
        {
            pb_tx_to_rx->setHidden(true);
            pb_rx_to_tx->setHidden(true);
            cb_lock_txrx->setHidden(true);
            sb_txsn->SetHidden(true);
            cb_sh_rpt->setHidden(false);
            cb_swl->setHidden(false);
        }
        else
        {
            pb_tx_to_rx->setHidden(true);
            pb_rx_to_tx->setHidden(true);
            cb_lock_txrx->setHidden(true);
            cb_sh_rpt->setHidden(true);
            cb_swl->setHidden(true);
            sb_txsn->SetHidden(false);
        }        
    } //default is -> QSizePolicy::MinimumExpanding      GrowFlag | ExpandFlag
    setSizePolicy(spol_w); //cb_zap->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
}
void HvTxW::SetEnabledRbBtTx(bool f)
{
    if (f) g_no_block_tx = 101;//101=noblock 100=blockall 6=blocktx7 1=blocktx2
    else   g_no_block_tx = 100;
    for (int i = 0; i<count_tx_widget; i++)
    {
        HvTxIn *TempHvTxIn = (HvTxIn*)V_l->itemAt(i)->widget();
        TempHvTxIn->SetEnabledRbBtTx(f);
    }
}
void HvTxW::SetEnabledBtTx(bool f)
{
    int b_str = 0;
    int b_end = count_tx_widget;
    if (f) g_no_block_tx = 101;//101=noblock 100=blockall 6=blocktx7 1=blocktx2
    else
    {
        int b_activ = -1;
        for (int i = 0; i<count_tx_widget; i++)
        {
            HvTxIn *TempHvTxIn = (HvTxIn*)V_l->itemAt(i)->widget();
            if (TempHvTxIn->rb_tx->isChecked())
            {
                b_activ = i;
                break;
            }
        }
        if (b_activ!=-1)
        {
            if (b_activ==6) g_no_block_tx = 100;//101=noblock 100=blockall 6=blocktx7 1=blocktx2//QRG raport
            else
            {
                b_str = 6;
                b_end = 7;
                g_no_block_tx = 6;//101=noblock 100=blockall 6=blocktx7 1=blocktx2
            }
        }
    }

    for (int i = b_str; i<b_end; ++i)
    {
        HvTxIn *TempHvTxIn = (HvTxIn*)V_l->itemAt(i)->widget();
        TempHvTxIn->SetEnabledBtTx(f);
    }
}
void HvTxW::SetTxTextsReadOnly(bool f)
{
    //for (int i = 3; i<count_tx_widget; i++)//2.00
    //return;
    for (int i = 0; i<count_tx_widget; i++)
    {
        HvTxIn *TempHvTxIn = (HvTxIn*)V_l->itemAt(i)->widget();
        TempHvTxIn->SetReadOnly(f);
    }
}
////////RIG CONTROL/////////////////////////////////
bool HvTxW::FindRigBandFromFreq(QString f)
{
    /*
    "135 kHz","472 kHz","501 kHz",
    "1.8 MHz","3.5 MHz","5 MHz","7 MHz","10 MHz","14 MHz","18 MHz","21 MHz","24 MHz","27 MHz","28 MHz","40 MHz","50 MHz","60 MHz",
     "70 MHz","144 MHz","222 MHz","432 MHz","902 MHz","1296 MHz","2320 MHz","3.4 GHz","5.65 GHz","10 GHz",
     "24 GHz","47 GHz","76 GHz","120 GHz","144 GHz","248 GHz"
    */
    bool isStdBand = false;
    unsigned long long f_int = f.toLongLong();
    QString band;
    int idband = -1;
    for (int i = 0; i<COUNT_BANDS; ++i)
    {
        if (f_int>=freq_min_max[i].min && f_int<=freq_min_max[i].max)
        {
            idband = i;
            isStdBand = true;
            break;
        }
    }
    if (!isStdBand) //2.68 new
    {
        for (int i = 1; i<COUNT_BANDS; ++i)//find closer std band
        {
            if (f_int<freq_min_max[i].min)
            {
                unsigned long long i1 = freq_min_max[i].min - f_int;
                unsigned long long i0;
                if (f_int<freq_min_max[i-1].max) i0 = freq_min_max[i-1].max - f_int;
                else i0 = f_int - freq_min_max[i-1].max;
                if (i1<i0) idband = i;
                else idband = i-1;
                //qDebug()<<freq_min_max[i-1].max<<i0<<"|"<<f_int<<"|"<<i1<<freq_min_max[i].min;
                break;
            }
        }
        if (idband<0) idband = COUNT_BANDS-1;//very hight
    }
    band = lst_bands[idband];
    if (s_band!=band && idband>=0) emit EmitRigBandFromFreq(idband);
    return isStdBand;
}
void HvTxW::SetModeGlobalFromRigCat(QString s)
{
    THvCatDispW->SetMode(s);
}
void HvTxW::SetGUbMK(bool f)
{
    g_ub_m_k = f;
}
void HvTxW::RefreshLRestrict()
{
    static QString prev_mfrq_  = "2"; //static bool prev_frest_ = false;
    static bool prev_hf_ = false;
    if (prev_mfrq_ == FREQ_GLOBAL) return;
    prev_mfrq_ = FREQ_GLOBAL;
    long long int mfrq = FREQ_GLOBAL.toLongLong();
    bool hf = false;
    if (mfrq<0x213C4D1) hf = true;//213C4D1 34.850.001 midle  29702000+39998000=69700000/2=34850000
    if (hf != prev_hf_) MultiAnswerMod->setHfBand(hf);
    prev_hf_ = hf; 
    emit EmitFreqGlobalToDec(FREQ_GLOBAL);//2.76.5
    if (g_ub_m_k) return;
    if (s_mode!=11 && s_mode!=13 && s_mode!=18 && !allq65) return; 
    if (mfrq<1000000) return;
    bool frest = false;
    for (int i = 0; i < 27; ++i)//all=27 +FT2=36
    {
        long long int Hzd = 0;
        Hzd = mfrq - _ftfr_[i];
        if (i<17)//all=17 +FT2=26
        {
        	if (qAbs(Hzd) < 3000)
			{
				frest = true;
				break;        		
			}
        }
        else//wspr
        {
        	if(Hzd > -3500 && Hzd < 300) 
        	{
            	frest = true;
            	break;
        	} 
       	}
    }
    if (prev_frest_ == frest) return;
    prev_frest_ = frest;
    MshfChanget(false);//2.76
    MultiAnswerMod->RefreshLRestrict_pub(frest);
}
static bool _freq_from_app_rig_ = false;
void HvTxW::SetFreqGlobalFromRigCat(QString s)
{
    QStringList l = s.split("#");
    bool isStd = FindRigBandFromFreq(l.at(0));

    if (!isStd && _freq_from_app_rig_ && l.at(0)!=l.at(1))
    {
        FindRigBandFromFreq(l.at(1)); //qDebug()<<"Find:"<<ar<<l.at(1)<<"isStd="<<isStd<<rtsv;
    } //else qDebug()<<ar<<l.at(0)<<l.at(1)<<"isStd="<<isStd<<rtsv;
    _freq_from_app_rig_ = true;

    FREQ_GLOBAL = l.at(0);
    THvCatDispW->SetFreq(FREQ_GLOBAL);
    TRadioAndNetW->SetFreqGlobal(FREQ_GLOBAL);
    THvLogW->SetFreqGlobal(FREQ_GLOBAL);
    RefreshLRestrict();
}
void HvTxW::SetBlockEmitFreqToRig(bool f)
{
    f_block_emit_freq_to_rig = f; //2.69 set to true, no show MA freq restrict QMessageBox at close (DestroyPort())
    if (!f) QTimer::singleShot(4000, this, SLOT(RefreshLRestrict()));//first time refresh
}
void HvTxW::SetRigCatActiveAndRead(bool f,QString s)//2.53
{
    rig_cat_active_and_read = f; //qDebug()<<"IN="<<s;
    if (!f && !f_block_emit_freq_to_rig) SetDefFreqGlobal(0,"default");//2.74 0=mybe frq,no mod, 1=mybe frq,mod, 2=frq,mod, 3=frq,no mod
    if (f) TRadioAndNetW->SetFullRigInfo(s);//2.76.1 for pskreporter
    else TRadioAndNetW->SetFullRigInfo("None");//2.76.1	for pskreporter
}
void HvTxW::SetDefFreqGlobal(int id,QString user_frq)
{
    QString tfrq = user_frq; //qDebug()<<"SetDefFreqGlobal="<<f<<user_frq<<f_block_emit_freq_to_rig;
    if (tfrq=="default") TRadioAndNetW->FindFreqRadList(0,tfrq);//0 to hz
    else FindRigBandFromFreq(tfrq);  //if user_frq
    _freq_from_app_rig_ = false;//2.68
    if (!rig_cat_active_and_read)//no read rig default settings
    {
        FREQ_GLOBAL = tfrq;
        TRadioAndNetW->SetFreqGlobal(FREQ_GLOBAL);
        THvCatDispW->SetFreq(FREQ_GLOBAL);
        THvLogW->SetFreqGlobal(FREQ_GLOBAL);
        THvCatDispW->SetMode("USB");
    }
    if (!f_block_emit_freq_to_rig)
    {
        emit EmitFreqGlobalToRig(tfrq,id);//da go setva dori da ne go 4ete
        RefreshLRestrict(); //emit EmitFreqGlobalToDec(tfrq);
    } 
}
////////END RIG CONTROL/////////////////////////////////
void HvTxW::RefreshCntestOnlySdtc()//2.15
{
    // EU VHF CONTEST, ARRL FIELD DAY, ARRL-RTTY
    //if ((s_id_contest_mode==3 || s_id_contest_mode==4 || s_id_contest_mode==5) &&  //2.39 old EU VHF
    if ((s_cont_type==4 || s_cont_type==5) &&  //2.39 for new EU VHF
            (s_mode==0 || s_mode==11 || s_mode==13 || s_mode==18 || allq65))//msk144 ft8 ft4
        f_cntest_only_sdtc = true;
    else
        f_cntest_only_sdtc = false;

    MultiAnswerMod->SetCntestOnlySdtc(f_cntest_only_sdtc);
    //qDebug()<<"f_cntest_only_sdtc="<<f_cntest_only_sdtc;
}
void HvTxW::SetModSetFrqToRig(bool f)
{
    f_mod_set_frq_to_rig = f; //qDebug()<<"SetModSetFrqToRig="<<f_mod_set_frq_to_rig;
}
void HvTxW::RefreshCfm73()
{
    if ((s_mode==11 || s_mode==13 || s_mode==18 || allq65) && !sf_cfm73) f_cfm73 = false;
    else f_cfm73 = true;
    MultiAnswerMod->SetCfm73(f_cfm73); //qDebug()<<"RefreshCfm73()"<<f_cfm73;
}
void HvTxW::SetCfm73(bool f)
{
    sf_cfm73 = f;
    RefreshCfm73();
}
void HvTxW::SetLockTxRstMskASeq(bool f)
{
    is_locked_tx_rst_msk_aseq = f;
    //if (f) qDebug()<<"RPT MSK Lock ASeq ====== "<<f;
    //else   qDebug()<<"RPT MSK Lock ASeq = "<<f;
}
void HvTxW::ModeChanget(int mode,bool f)
{
    //qDebug()<<"ModeChanget"<<mode<<s_mode;
    static int sss_mode = -1;//2.71 s_mode = -1 <- crash
    if (sss_mode==mode) return;//only once because is radio buttons
    sss_mode = mode; //qDebug()<<"ModeChanget"<<mode;

    RefreshBackupDB(); //2.33 work here or from SetMacros

    s_mode = mode;
    allq65 = false;
    alljt65 = false;
    s_bvhf_jt65 = f;
    f_nosave = true;
    cb_sh_rpt->setEnabled(false);
    cb_swl->setEnabled(false);
    AutoSeqLab->setHidden(true);
    TQueuedCall->ClrQueuedCall(false);//2.02 Queued
    f_two_no_sdtc = false;
    sb_dt->SetHidden(true);

    if (mode == 0)
    {//Default values for MSK144
        cb_sh_rpt->setEnabled(true);
        cb_swl->setEnabled(true);
        SB_MinSigdB->setPrefix("S Limit - N/A - ");//not used
        SB_MinSigdB->setEnabled(false);
        SB_MinSigdB->setRange(-30,10);//1.79 jt6m problem
        SB_MinSigdB->setValue(1);//s_minsigndb[mode] not used
        SetRptRsqSettings();
        cb_zap->setChecked(false);
        cb_zap->setEnabled(false);
        SetTxTextsReadOnly(true);//2.00 all avaleable
        SetEnabledRbBtTx(true);
        SetShowMoni12MoonD(true);
        AutoSeqLab->setHidden(false);
        f_two_no_sdtc = true;
    }
    if ( mode == 1 || mode == 2 || mode == 3)
    {//Default values for JTMS FSK441 FSK315
        SB_MinSigdB->setPrefix("S Limit def=1  ");
        SB_MinSigdB->setEnabled(true);
        SB_MinSigdB->setRange(-3,10);//1.79 jt6m problem
        SB_MinSigdB->setValue(s_minsigndb[mode]);
        SetRptRsqSettings();
        cb_zap->setEnabled(true);
        SetTxTextsReadOnly(false);
        SetEnabledRbBtTx(true);
        SetShowMoni12MoonD(true);
    }
    if (mode == 4 || mode == 5)
    {//Default values for ISCAT-A ISCAT-B
        SB_MinSigdB->setPrefix("Sync def=1  ");
        SB_MinSigdB->setEnabled(true);
        SB_MinSigdB->setRange(-30,10);//1.79 jt6m problem
        SB_MinSigdB->setValue(s_minsigndb[mode]);
        SetRptRsqSettings();
        cb_zap->setEnabled(true);
        SetTxTextsReadOnly(false);
        SetEnabledRbBtTx(true);
        SetShowMoni12MoonD(true);
    }
    if (mode == 6)
    {//Default values for JT6M
        SB_MinSigdB->setPrefix("S Limit def=-10  ");
        SB_MinSigdB->setEnabled(true);
        SB_MinSigdB->setRange(-30,10);//1.79 jt6m problem
        SB_MinSigdB->setValue(s_minsigndb[mode]);
        SetRptRsqSettings();
        cb_zap->setEnabled(true);
        SetTxTextsReadOnly(false);
        SetEnabledRbBtTx(true);
        SetShowMoni12MoonD(true);
    }
    if (mode == 7 || mode == 8 || mode == 9 || mode == 10)
    {//Default values for JT65ABC
        pb_tx_to_rx->setHidden(true);
        pb_rx_to_tx->setHidden(true);
        if (mode != 10)// no need in pi4
        {
            if (!s_bvhf_jt65) cb_sh_rpt->setEnabled(false);
            else cb_sh_rpt->setEnabled(true);
            alljt65 = true;
        }
        SB_MinSigdB->setPrefix("Sync def=1  ");
        SB_MinSigdB->setEnabled(true);
        SB_MinSigdB->setRange(-1,10);//1.79 jt6m problem
        SB_MinSigdB->setValue(s_minsigndb[mode]);
        SetRptRsqSettings();
        cb_zap->setChecked(false);//1.52 no zap in jt65 pi4
        cb_zap->setEnabled(false);//1.52 no zap in jt65 pi4
        if (mode == 10)// no need in pi4
        {
            SetShowMoni12MoonD(true);
            SetTxTextsReadOnly(true);
            SetEnabledRbBtTx(false);
        }
        else
        {
            SetShowMoni12MoonD(false);
            SetTxTextsReadOnly(false);
            SetEnabledRbBtTx(true);
        }
    }
    if (mode == 11 || mode == 13 || mode == 18)
    {//Default values for ft8 ft4
        pb_tx_to_rx->setHidden(true);
        pb_rx_to_tx->setHidden(true);
        cb_sh_rpt->setEnabled(false);
        cb_swl->setEnabled(false);
        SB_MinSigdB->setPrefix("S Limit - N/A - ");//not used
        SB_MinSigdB->setEnabled(false);
        SB_MinSigdB->setRange(-30,10);//1.79 jt6m problem
        SB_MinSigdB->setValue(1);//s_minsigndb[mode] not used
        SetRptRsqSettings();
        cb_zap->setChecked(false);
        cb_zap->setEnabled(false);
        SetTxTextsReadOnly(true);//2.00 all avaleable
        SetEnabledRbBtTx(true);
        SetShowMoni12MoonD(true);
        AutoSeqLab->setHidden(false);
        f_two_no_sdtc = true;
        sb_dt->SetHidden(false);
    }
    if (mode == 12)
    {//Default values for MSKMS
        cb_sh_rpt->setEnabled(false);
        cb_swl->setEnabled(false);
        SB_MinSigdB->setPrefix("S Limit - N/A - ");//not used
        SB_MinSigdB->setEnabled(false);
        SB_MinSigdB->setRange(-30,10);//1.79 jt6m problem
        SB_MinSigdB->setValue(1);//s_minsigndb[mode] not used
        SetRptRsqSettings();
        cb_zap->setChecked(false);
        cb_zap->setEnabled(false);
        SetTxTextsReadOnly(true);
        SetEnabledRbBtTx(true);
        SetShowMoni12MoonD(true);
        AutoSeqLab->setHidden(false);
    }
    if (mode == 14 || mode == 15  || mode == 16 || mode == 17)
    {//Default values for q65
        allq65 = true;
        pb_tx_to_rx->setHidden(true);
        pb_rx_to_tx->setHidden(true);
        cb_sh_rpt->setEnabled(false);
        cb_swl->setEnabled(false);
        SB_MinSigdB->setPrefix("S Limit - N/A - ");//not used
        SB_MinSigdB->setEnabled(false);
        SB_MinSigdB->setRange(-30,10);//1.79 jt6m problem
        SB_MinSigdB->setValue(1);//s_minsigndb[mode] not used
        SetRptRsqSettings();
        cb_zap->setChecked(false);
        cb_zap->setEnabled(false);
        SetTxTextsReadOnly(true);//2.00 all avaleable
        SetEnabledRbBtTx(true);
        SetShowMoni12MoonD(false);//2.74 0ld=true
        AutoSeqLab->setHidden(false);
        f_two_no_sdtc = true;
    }

	sb_dt->SetMode(s_mode);
    TRadioAndNetW->SetModeForFreqFromMode(s_mode);
    SB_DfTolerance1->SetMode(mode);

    SetLockTxrxMode(s_mode);

    MultiAnswerMod->SetMode(s_mode);
    RefreshMultiAnswerModAndASeq();

    RefreshCntestOnlySdtc();

    if (!s_bvhf_jt65 && (mode == 7 || mode == 8 || mode == 9)) cb_sh_rpt->setChecked(false);
    else cb_sh_rpt->setChecked(sh_op_all[s_mode]);

    //qDebug()<<"SB_PeriodTime->SetMode"<<mode;
    SB_PeriodTime->SetMode(mode);

    gen_msg();

    if (!rig_cat_active_and_read || f_mod_set_frq_to_rig)//no read rig
    {
        //qDebug()<<"Mode SET  100"<<s_band;
        SetDefFreqGlobal(1,"default");//2.74 0=mybe frq,no mod, 1=mybe frq,mod, 2=frq,mod, 3=frq,no mod
    }

    if (s_txt_mark[19] || s_txt_mark[22])
    {
        THvLogW->SetMarkTextAllRuleChanged(s_txt_mark,s_mode,s_band);
        //qDebug()<<"SetMarkTextQsoB4====== Mode"<<s_mode;
    }
    RefreshCfm73();
    SetLockTxRstMskASeq(false);//2.59
    f_nosave = false;
    THvMakros->ModeChanget(s_mode);
}
void HvTxW::SetBand(QString s,int id)
{
    //qDebug()<<"SetBand"<<s<<s_band<<id;
    if (s_band==s) return;//only once because is radio buttons

    s_band = s;
    if (s_mode==11 || s_mode==13 || s_mode==18 || allq65) fpsk_restrict = true; //only ft8 ft4  || allq65

    //l_mycall_loc->setText(list_macros.at(0)+" "+list_macros.at(1)+"   Band "+s_band);
    QString loc4 = list_macros.at(1);
    l_mycall_loc->setText(list_macros.at(0)+" "+loc4.mid(0,4));

    //emit EmitLocStInfo(list_macros.at(0),list_macros.at(1),s_band);

    TRadioAndNetW->SetLocalStation(list_macros.at(0),list_macros.at(1),s_band,s_cont_id);
    THvAstroDataW->SetMyLocHisLocBand(list_macros.at(1),LeHisLoc->getText(),s_band);

    if (id==0)
    {
        //qDebug()<<"BAnd SET  id"<<id<<s_band;
        //SetDefFreqGlobal(false,"default");//0<-from App 1<-from Rig // false omly F button true
        SetDefFreqGlobal(2,"default");//2.74 0=mybe frq,no mod, 1=mybe frq,mod, 2=frq,mod, 3=frq,no mod  0<-from App 1<-from Rig
    }
    //else
    //qDebug()<<"NO SET id"<<id<<s_band;
    if (s_txt_mark[19] || s_txt_mark[22])
    {
        THvLogW->SetMarkTextAllRuleChanged(s_txt_mark,s_mode,s_band);
    }
    MultiAnswerMod->SetBand();//for reset

    s_iband = 17;//2.76.5 17 default HV 70 MHz 
    for (int i = 0; i<COUNT_BANDS; ++i)
    {
        if (lst_bands[i]==s_band)
        {
            s_iband = i;
            break;
        }
    }
    Slider_Tx_level->SetValue(s_tx_level[s_iband]);
    SetLockTxRstMskASeq(false);//2.59
    //qDebug()<<"SetMarkTextQsoB4====== Band"<<s_band<<FREQ_GLOBAL;
}
void HvTxW::SetRptRsq(bool f)
{
    s_f_rpt_rsq = f; //qDebug()<<"GGGGGGG"<<s_f_rpt_rsq;
}
void HvTxW::SetMacros(QStringList list,int id_c0,QString cont_exch,QString trmN)//imidiatly//2.32 stop  ,bool msk144contm
{
    //qDebug()<<"SetMacros"<<s_mode<<id_c0;
    //2.51 New Contest
    // Activity Type                id	type	dec-id       dec-type	dec-cq
    //"Standard"					0	0		0 = CQ		 0			0 = CQ
    //"EU RSQ And Serial Number"	1	NONE	1  NONE		 NONE		NONE
    //"NA VHF Contest"				2	2		2  CQ TEST	 1			3 = CQ TEST
    //"EU VHF Contest"				3 	3		3  CQ TEST	 2			3 = CQ TEST
    //"ARRL Field Day"				4	4		4  CQ FD	 3			2 = CQ FD
    //"ARRL Inter. Digital Contest"	5	2		5  CQ TEST   1 			3 = CQ TEST
    //"WW Digi DX Contest"			6	2		6  CQ WW	 1			4 = CQ WW
    //"FT4 DX Contest"				7	2		7  CQ WW	 1			4 = CQ WW
    //"FT8 DX Contest"				8	2		8  CQ WW	 1			4 = CQ WW
    //"FT Roundup Contest"			9	5		9  CQ RU	 4			1 = CQ RU
    //"Bucuresti Digital Contest"	10 	5		10 CQ BU 	 4			5 = CQ BU
    //"FT4 SPRINT Fast Training"	11 	5		11 CQ FT 	 4			6 = CQ FT
    //"PRO DIGI Contest"			12  5		12 CQ PDC 	 4			7 = CQ PDC
    //"CQ WW VHF Contest"			13	2		13 CQ TEST	 1			3 = CQ TEST
    //"Q65 Pileup" or "Pileup"		14	2		14 CQ 		 1			0 = CQ
    //"NCCC Sprint"					15	2		15 CQ NCCC	 1			8 = CQ NCCC
    //"ARRL Inter. EME Contest"		16	6		16 CQ 		 0			0 = CQ
    //"FT Challenge Contest"		17  6       17 CQ FTC    0          9 = CQ FTC

    s_trmN = trmN;//2.61
    QString trmN_stdC = trmN;
    if (MultiAnswerMod->isStandardCall(list.at(0))) trmN_stdC.append("1"); //2.61
    else trmN_stdC.append("0");
    trmN_stdC.append(list.at(1));//+ myloc
    emit EmitMacros(id_c0,trmN_stdC);//+ myloc

    int decode_type = 0; // 0 = CQ
    s_cont_type = 0; 	 // 0 = CQ
    //typ0as_cont = false; <-for the future
    //int decoder_cq = 0;	 // 0 = CQ
    switch (id_c0)
    {
    case 2:  // 2  = CQ TEST NA VHF
    case 5:  // 5  = CQ TEST ARRL Inter. Digital Contest
    case 13: // 13 = CQ TEST WW VHF Contest
        s_cont_type = 2;
        decode_type = 1;
        //decoder_cq  = 3; //CQ TEST
        break;
    case 6:  // 6  = CQ WW WWROF
    case 7:  // 7  = CQ WW FT4 DX
    case 8:  // 8  = CQ WW FT8 DX
        s_cont_type = 2;
        decode_type = 1;
        //decoder_cq  = 4; //CQ WW
        break;
    case 3:  // 3  = CQ TEST EU VHF
        s_cont_type = 3;
        decode_type = 2;
        //decoder_cq  = 3; //CQ TEST
        break;
    case 4:  // 4  = CQ FD ARRL
        s_cont_type = 4;
        decode_type = 3;
        //decoder_cq  = 2; //CQ FD
        break;
    case 9:  // 9  = CQ RU FT Roundup
        s_cont_type = 5;
        decode_type = 4;
        //decoder_cq  = 1; //CQ RU
        break;
    case 10: // 10 = CQ BU Bucuresti Digital
        s_cont_type = 5;
        decode_type = 4;
        //decoder_cq  = 5; //CQ BU
        break;
    case 11: // 11 = CQ FT FT4 SPRINT Fast Training
        s_cont_type = 5;
        decode_type = 4;
        //decoder_cq  = 6; //CQ FT
        break;
    case 12: // 12 = PRO DIGI Contest
        s_cont_type = 5;
        decode_type = 4;
        //decoder_cq  = 7; //CQ PDC
        break;
    case 14: // 14 = Pileup
        s_cont_type = 2;
        decode_type = 1;
        //decoder_cq  = 0; //CQ
        break;
    case 15: // 15 = NCCC Sprint
        s_cont_type = 2;
        decode_type = 1;
        //decoder_cq  = 8; //CQ NCCC
        break;
    case 16: // 16 = ARRL Inter. EME Contest
        s_cont_type = 6;
        decode_type = 0;
        //typ0as_cont = true;
        //decoder_cq  = 0; //CQ
        break;
    case 17: // 17 = FT Challenge Contest
        s_cont_type = 6;
        decode_type = 0;
        //decoder_cq  = 0; //CQ FTC
        break;        
    default:
        s_cont_type = 0;
        decode_type = 0;
        //decoder_cq  = 0; //CQ
        break;
    }
    s_cont_id = id_c0;
    //qDebug()<<s_cont_name[s_cont_id]<<"ID="<<s_cont_id<<"TXtype="<<s_cont_type<<"DECtype="<<decode_type<<"typ0as_cont="<<typ0as_cont;

    if (s_cont_type==3) sb_txsn_v2->SetRange(2047);//eu vhf contest//new 2.39 sb_txsn_v2->SetRange(4095);
    else if (s_cont_type==5) sb_txsn_v2->SetRange(7999);//ru ARRL RTTY Roundup

    TQueuedCall->ClrQueuedCall(false);//2.02 Queued

    f_cont5_ru_dx = false;
    QStringList l_exch = cont_exch.split("#");
    if (s_cont_type==4) s_my_contest_exch = l_exch.at(0);//FD
    else if (s_cont_type==5) //RU
    {
        s_my_contest_exch = l_exch.at(1);
        if (s_my_contest_exch=="DX") f_cont5_ru_dx = true;
    }
    else s_my_contest_exch = "";
    //qDebug()<<cont_exch<<s_my_contest_exch<<s_trmN;

    list_macros = list;
    s_my_base_call = MultiAnswerMod->FindBaseFullCallRemAllSlash(list_macros.at(0));
    //s_my_call_is_std = MultiAnswerMod->isStandardCall(list_macros.at(0));
    MultiAnswerMod->SetMacros(list,s_cont_id,s_cont_type);//2.23

    RefreshCntestOnlySdtc();

    //l_mycall_loc->setText(list_macros.at(0)+" "+list_macros.at(1)+"   Band "+s_band);
    QString loc4 = list_macros.at(1);
    l_mycall_loc->setText(list_macros.at(0)+" "+loc4.mid(0,4));

    f_makros_ready = true;

    SetRptRsqSettings();

    //if (imidiatly)//2.32 stop
    gen_msg();

    QStringList ls;
    ls << list_macros.at(0);
    ls << s_my_base_call;//1.70
    QString cqq = "CQ";
    int decoder_cq = 0;
    if (s_cont_type != 0) cqq = MultiAnswerMod->DetectCQTypeFromMacros(list[7]);
    if (cqq != "CQ")
    {
        for (int i = 0; i < cqq.count(); ++i) decoder_cq += (int)cqq.at(i).toLatin1();
    }
    ls << cqq; //qDebug()<<cqq<<decoder_cq;
    emit EmitWords(ls,decoder_cq,decode_type);// to decoder

    MarkTextChanged(false);//bool false  norefresh b4 qso for my call and loc
    THvLogW->SetMyCallGridExchAllCont(list_macros.at(0),list_macros.at(1),cont_exch);

    //emit EmitLocStInfo(list_macros.at(0),list_macros.at(1),s_band);

    TRadioAndNetW->SetLocalStation(list_macros.at(0),list_macros.at(1),s_band,s_cont_id);
    //emit EmitMyGridMsk144ContM(list_macros.at(1),msk144contm);
    THvAstroDataW->SetMyLocHisLocBand(list_macros.at(1),LeHisLoc->getText(),s_band);
    //qDebug()<<"TX RX ====="<<s_my_base_call;
    MshfChanget(false);//2.76
}
bool HvTxW::DetectCallSufix(QString &call, bool f_sep_n)
{
    bool isSfx = false;
    QString sfx_a = call;
    int i = 0;
    int dig1=0;
    int let1=0;
    int dig2=0;
    int let2=0;
    int sep_n = 1;

    if (f_sep_n) sep_n = 0;

    for (i = 0; i < call.count(); i++)
    {
        if (call.at(i).isDigit())
        {
            dig1=i;
            break;
        }
    }
    for (i = dig1+1; i < call.count(); i++)
    {
        if (!call.at(i).isDigit())
        {
            let1=i;
            break;
        }
    }
    for (i = let1+1; i < call.count(); i++)
    {
        if (call.at(i).isDigit())
        {
            dig2=i;
            break;
        }
    }
    for (i = dig2+1; i < call.count(); i++)
    {
        if (!call.at(i).isDigit())
        {
            let2=i;
            break;
        }
    }

    /*if (dig2!=0 && let2!=0)// (let1!=0 && dig2!=0 && let2!=0)
        sfx=call.mid(dig2,call.count()-dig2);
    else if (let1!=0)
        sfx=call.mid(dig1,call.count()-dig1);*/

    if (dig2!=0 && let2!=0)// (let1!=0 && dig2!=0 && let2!=0)
        sfx_a=call.mid(dig2+sep_n,call.count()-dig2+sep_n);
    else if (dig1!=0 && let1!=0)
        sfx_a=call.mid(dig1+sep_n,call.count()-dig1+sep_n);

    if (sfx_a!=call)
        isSfx = true;

    call=sfx_a;
    //qDebug()<<"CallSuf="<<suf;
    return isSfx;
}
QString HvTxW::DetectSufix(QString call_all, bool f_sep_n)
{
    QString sfx_a = call_all;
    QString sfx1;
    QString sfx2;
    int slash1 = -1;

    slash1 = call_all.indexOf("/");//qDebug()<<"slash1="<<slash1;

    if (slash1==-1)
    {
        DetectCallSufix(sfx_a,f_sep_n);
        return sfx_a;
    }

    sfx1 = call_all.mid(0,slash1);
    sfx2 = call_all.mid(slash1+1,call_all.count()-(slash1+1));

    if (DetectCallSufix(sfx2,f_sep_n))
        sfx_a = sfx2;
    else if (DetectCallSufix(sfx1,f_sep_n))
        sfx_a = sfx1+"/"+sfx2;

    return sfx_a;
}
/*#include "../_pfx_sfx.h"
bool HvTxW::is_pfx(QString s)
{
    bool res = false;
    for (int i = 0; i < NZ; i++)
    {
        if (s==pfx[i])
        {
            res = true;
            break;
        }
    }
    return res;
}*/
void HvTxW::StopAuto()
{
    f_auto_on = false;
    b_auto_on->setText(tr("AUTO IS OFF"));
    b_auto_on->setStyleSheet("QPushButton{background-color:palette(Button);}");
    TRadioAndNetW->SetAuto(false);
    MultiAnswerMod->SetAuto(false);
    count_73_auto_seq = 0;//2.50 reset 73 for sop from watchdog or manual stop TX with 73 or RR73
    //one_addtolog_auto_seq = false;//reset add to log
    emit EmitAuto();
}
void HvTxW::auto_on()
{
    if (f_auto_on)
    {
        f_auto_on = false;
        b_auto_on->setText(tr("AUTO IS OFF"));
        b_auto_on->setStyleSheet("QPushButton{background-color:palette(Button);}");
        TRadioAndNetW->SetAuto(false);
        MultiAnswerMod->SetAuto(false);
        count_73_auto_seq = 0; //2.50 reset 73 for manual stop TX with 73 or RR73
    }
    else
    {
        f_auto_on = true;
        b_auto_on->setText(tr("AUTO IS ON"));
        if (dsty) b_auto_on->setStyleSheet("QPushButton{background-color:rgb(170,28,28);}");
        else b_auto_on->setStyleSheet("QPushButton{background-color:rgb(255,128,128);}");

        s_last_txwatchdog_time = QDateTime::currentDateTimeUtc().toTime_t();// reset watchdog time
        s_last_txwatchdog_coun = 0;
        TRadioAndNetW->SetAuto(true);
        MultiAnswerMod->SetAuto(true);
    }
    emit EmitAuto();
    //one_addtolog_auto_seq = false;//reset add to log
}
void HvTxW::SaveSettings()
{
    QFile file(sr_path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return;
    QTextStream out(&file);

    out << "his_call_tx=" << le_his_call->getText() << "\n";
    out << "my_qrg_tx=" << le_qrg->text() << "\n";
    out << "tx_fi=" << QString("%1").arg(rb_tx_fi->isChecked()) << "\n";
    out << "mon_call1=" << le_mon_call1->getText() << "\n";
    out << "mon_call2=" << le_mon_call2->getText() << "\n";
    out << "def_radec=" << THvAstroDataW->GetRaDec() << "\n";
    out << "def_multi_answer=" << MultiAnswerMod->GetSettings() << "\n";
    out << "def_cont_v1_txsn=" << QString("%1").arg(sb_txsn->Value()) << "\n";
    out << "def_cont_v2_txsn=" << QString("%1").arg(sb_txsn_v2->Value()) << "\n";
    out << "def_cabrillo_log_set=" << THvLogW->GetSettings() << "\n";
    out << "def_use_adif_save=" << THvLogW->GetUseAdifSave() << "\n";
    out << "log_qsos_limit_gt9999_lt500001=" << THvLogW->GetMaxLogQsoCount() << "\n";//500001
    out << "add_to_log_prop_all=" << THvLogW->GetPropSettings() << "\n";//2.75
    out << "mam_shf="<<QString("%1").arg(cb_msh->isChecked())<<"#"<<QString("%1").arg(cb_msf->isChecked())<<"\n";//2.76

    //out << "auto_seq_all=" << AutoSeqLab->getautoseq_all() << "\n";
    //out << "lock_txrx_all=" << get_lock_txrx_all() << "\n";

    file.close();
}
bool HvTxW::isFindId(QString id,QString line,QString &res)
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
void HvTxW::ReadSettings()
{
    const int c_st_id = 14;//dopalva se tuk v kraia    
    const QString st_id[c_st_id]=
        {
            "his_call_tx","my_qrg_tx","tx_fi","mon_call1","mon_call2","def_radec","def_multi_answer",
            "cont_v1_txsn","cont_v2_txsn","def_cabrillo_log_set","def_use_adif_save",
            "log_qsos_limit_gt9999_lt500001","add_to_log_prop_all","mam_shf"
        };
    QString st_res[c_st_id];
    for (int i = 0; i < c_st_id; ++i) st_res[i]="";

    QFile file(sr_path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        le_his_call->SetText("CALL");   //SP9HWY ako ne pro4ete ms_mesages
        le_qrg->setText("237");
        le_mon_call1->SetText("MONR1"); //G0LFF
        le_mon_call2->SetText("MONR2"); //LZ2PG
        return;
    }
    //QTime ttt;
    //ttt.start();
    QTextStream in(&file);
    while (!in.atEnd())
    {
        QString line = in.readLine();

        for (int i = 0; i < c_st_id; ++i)
        {
            if (isFindId(st_id[i],line,st_res[i]))
            {
                line = in.readLine();
                //qDebug()<<i<<"idfind="<<st_id[i]<<"id="<<st_res[i];
            }
            //else qDebug()<<i<<"FALSE READ --------------------------------  idfind="<<st_id[i];
        } //qDebug()<<"ffffffffffffffff===============";
    }
    file.close();
    //qDebug()<<"2Time="<<ttt.elapsed();
    if (!st_res[6].isEmpty()) MultiAnswerMod->SetSettings(st_res[6]);
    if (!st_res[0].isEmpty()) le_his_call->SetText(st_res[0]);
    if (!st_res[1].isEmpty()) le_qrg->setText(st_res[1]);
    if (!st_res[2].isEmpty())
    {
        if (st_res[2].toInt()==0) rb_tx_se->setChecked(true);
        else rb_tx_fi->setChecked(true);
    }
    if (!st_res[3].isEmpty()) le_mon_call1->SetText(st_res[3]);
    if (!st_res[4].isEmpty()) le_mon_call2->SetText(st_res[4]);
    if (!st_res[5].isEmpty()) THvAstroDataW->SetRaDec(st_res[5]);
    if (!st_res[7].isEmpty()) sb_txsn->SetValue(st_res[7].toInt());
    if (!st_res[8].isEmpty()) sb_txsn_v2->SetValue(st_res[8].toInt());
    if (!st_res[9].isEmpty()) THvLogW->SetSettings(st_res[9]);
    if (!st_res[10].isEmpty()) THvLogW->SetUseAdifSave(st_res[10]);
    if (!st_res[11].isEmpty()) THvLogW->SetMaxLogQsoCount(st_res[11]);
    if (!st_res[12].isEmpty()) THvLogW->SetPropSettings(st_res[12]);//2.75
    if (!st_res[13].isEmpty())//2.76
    {
    	QStringList l = st_res[13].split("#");
    	if (l.count()>1)
    	{
    		if (l.at(0)=="1") cb_msh->setChecked(true);
    		if (l.at(1)=="1") cb_msf->setChecked(true);
   		}
   	}
}
void HvTxW::SetTxTextsHiden(bool f)
{
    for (int i = 0; i<count_tx_widget; i++)
    {
        HvTxIn *TempHvTxIn = (HvTxIn*)V_l->itemAt(i)->widget();
        TempHvTxIn->setHidden(f);
    }
}
void HvTxW::RefreshMultiAnswerModAndASeq()
{
    if ((s_mode==11 || s_mode==13 || s_mode==18 || allq65) && f_multi_answer_mod)
    {
        AutoSeqLab->SetAutoSeqMode(s_mode,false);
        SetTxTextsHiden(true);
        MultiAnswerMod->setHidden(false);
        MultiAnswerMod->GetCurrentMsg();
    }
    else
    {
        AutoSeqLab->SetAutoSeqMode(s_mode,true);
        MultiAnswerMod->setHidden(true);
        SetTxTextsHiden(false);//false
        GetCurrentMsg();
    }
    RfreshDxParm();//2.66  from mode and MAM flag=f_multi_answer_mod
}
void HvTxW::MshfChanget(bool)//2.76
{    	
    uint8_t mshf = 0;//static uint8_t id_mshf = 10; //0=0,0 1=1,0 2=0,1
    if (s_cont_type<2 && (f_multi_answer_mod_std || !f_multi_answer_mod) && cb_msh->isChecked()) mshf = 1;
    if (f_multi_answer_mod && !f_multi_answer_mod_std && cb_msf->isChecked() && !prev_frest_) mshf = 2; //rb_tx_fi->isChecked() &&
    
    if (f_multi_answer_mod && !f_multi_answer_mod_std)
    {
    	cb_msh->setEnabled(false);
    	if (!prev_frest_) cb_msf->setEnabled(true); 
    	else cb_msf->setEnabled(false); //qDebug()<<prev_frest_<<f_tx_rx<<cb_msf->isEnabled();   	
   	}
    else 
    {
    	cb_msh->setEnabled(true);
    	cb_msf->setEnabled(false);
   	} 
   	
   	//2.76 need to be here  printf("mode=%d id_mshf=%d\n",s_mode,mshf);
   	if (mshf == 1 && s_mode==11) rb_tx_se->setChecked(true);   	
    MultiAnswerMod->SetMsfS5SMsg(mshf);

    if (mshf == 1 && s_mode==11)
    {
    	rb_tx_fi->setEnabled(false);
    	HvTxIn *TempHvTxIn = (HvTxIn*)V_l->itemAt(1)->widget();//2.76.2
    	TempHvTxIn->SetEnabledRbBtTx(false);
    	g_no_block_tx = 1;//101=noblock 100=blockall 6=blocktx7 1=blocktx2
    	if (TempHvTxIn->rb_tx->isChecked()) SetTxRaportToRB(0,false);
   	}
    else
    {
    	rb_tx_fi->setEnabled(true);
    	if (g_no_block_tx!=100)//2.76.2 or s_mode!=10
    	{
    		HvTxIn *TempHvTxIn = (HvTxIn*)V_l->itemAt(1)->widget();
    		TempHvTxIn->SetEnabledRbBtTx(true);
    		g_no_block_tx = 101;//101=noblock 100=blockall 6=blocktx7 1=blocktx2
   		}	
   	} //printf("g_no_block_tx=%d\n",g_no_block_tx);

    if (id_mshf == mshf) return;
    id_mshf = mshf;  
    
    if (mshf == 0 || mshf == 2) MultiAnswerMod->SetBand();//for reset ???
    if (mshf == 0)
    {
    	cb_msh->setStyleSheet("");//"QCheckBox{font-weight:normal;color:palette(WindowText);}"  
    	cb_msf->setStyleSheet("");  	
   	}
    else if (mshf == 1) 
    {
    	if (dsty) cb_msh->setStyleSheet("QCheckBox{font-weight:bold;color:rgb(0,220,0);}");
    	else      cb_msh->setStyleSheet("QCheckBox{font-weight:bold;color:rgb(0,120,0);}");
    	cb_msf->setStyleSheet("");    	
   	} 
   	else if (mshf == 2) 
    {
    	cb_msh->setStyleSheet("");
    	if (dsty) cb_msf->setStyleSheet("QCheckBox{font-weight:bold;color:rgb(0,220,0);}");
    	else      cb_msf->setStyleSheet("QCheckBox{font-weight:bold;color:rgb(0,120,0);}");
   	}               
    /*if 		(mshf==0) qDebug()<<"RX="<<false<<"TX="<<false;
    else if (mshf==1) qDebug()<<"RX="<<true<<" TX="<<false;
    else if (mshf==2) qDebug()<<"RX="<<false<<"TX="<<true;
    else 			  qDebug()<<"ERROR--------------------";*/
    emit EmitMshfChanget(id_mshf); //printf("HvTxW id_mshf=%d\n",id_mshf);
}
void HvTxW::SendIdMshf()
{
	emit EmitMshfChanget(id_mshf);
}
void HvTxW::SetMultiAnswerMod(bool fmadx, bool fmastd)
{
    if (fmadx || fmastd) f_multi_answer_mod = true;
    else f_multi_answer_mod = false;
    f_multi_answer_mod_std = fmastd;//for recorgnize tp
    MultiAnswerMod->SetMAStd(fmastd);
    RefreshBackupDB();
    RefreshMultiAnswerModAndASeq();
    
    MshfChanget(false);//2.76sf use MultiAnswerMod->isHidden() for sending OTP key, in MADX mod only, to msplayer
    SetRptRsqSettings();
}
void HvTxW::SetEmitMessage(QString msg,bool immediately,bool iftximid,bool id_mam)
{
    if ((s_mode==11 || s_mode==13 || s_mode==18 || allq65) && f_multi_answer_mod && !id_mam)//only ft8  ft4
    {
        return;
    }
    if ((s_mode==11 || s_mode==13 || s_mode==18 || allq65) && f_multi_answer_mod && id_mam)//only ft8  ft4
    {
        emit EmitMessageS(msg,immediately,iftximid);
        ResetTxWatchdog(msg,immediately,iftximid);//qDebug()<<"MultiAnswerMod"<<msg;
    }
    else if (!id_mam)// exception 2.35 for on the fly /auto on + ma to std
    {
        emit EmitMessageS(msg,immediately,iftximid);
        ResetTxWatchdog(msg,immediately,iftximid);//qDebug()<<"NormalMod"<<msg;
    }
}
void HvTxW::GetCurrentMsg()
{
    //qDebug()<<"GetCurrentMsg()";
    if ((s_mode==11 || s_mode==13 || s_mode==18 || allq65) && f_multi_answer_mod)
    {
        MultiAnswerMod->GetCurrentMsg();
    }
    else
    {
        for (int i = 0; i<count_tx_widget; i++)
        {
            HvTxIn *TempHvTxIn = (HvTxIn*)V_l->itemAt(i)->widget();
            if (TempHvTxIn->rb_tx->isChecked())
            {
                SetEmitMessage(TempHvTxIn->line_txt->text(),false,false,false);
            }
        }
    }
}
void HvTxW::SetStartQsoDateTime()
{
    for (int i = 0; i<count_tx_widget; i++)
    {
        HvTxIn *TempHvTxIn = (HvTxIn*)V_l->itemAt(i)->widget();
        if (TempHvTxIn->rb_tx->isChecked() && (i==0 || i==1 || i==2)) //only starting qso -> tx1 or tx2 or tx3
        {
            if (s_last_call_for_log != le_his_call->getText())
            {
                QDateTime utc_t = QDateTime::currentDateTimeUtc();
                s_log_data_start=utc_t.toString("yyyyMMdd");
                s_log_time_start=utc_t.toString("hh:mm");
                s_last_call_for_log=le_his_call->getText();
                //qDebug()<<"save start data time call="<<s_log_data_start<<s_log_time_start<<s_last_call_for_log;
            }
        }// v1.52
        else if (TempHvTxIn->rb_tx->isChecked() && (i==3 || i==4 || i==5 || i==6))//2.63 tx4 or tx5 or tx6 or tx7
        {
            s_last_call_for_log = "NO_CALL"; //reset for new qso 1.52
        }
    }
}
void HvTxW::SetDataTime(QDateTime dt)
{
    //QDateTime utc_t = getDateTime();  Translation
    l_time->setText(dt.toString("hh:mm:ss"));
    l_dey->setText(dt.toString("d ")+IntToMonth[slid][dt.toString("M").toInt()]+dt.toString(" yyyy"));
    s_log_data_now = dt.toString("yyyyMMdd");
    s_log_time_now = dt.toString("hh:mm");
}
void HvTxW::GenTestTones()
{
    QStringList lst;
    lst << "@A" << "@B" << "@C" << "@D" << "@500" << "@1000" << "@2000";
    for (int i = 0; i<count_tx_widget; i++)
    {
        HvTxIn *TempHvTxIn = (HvTxIn*)V_l->itemAt(i)->widget();
        TempHvTxIn->line_txt->setText(lst.at(i));
        if (i!=s_b_identif) TempHvTxIn->rb_tx->setChecked(false);
        else SetEmitMessage(TempHvTxIn->line_txt->text(),false,false,false);
    }
}
QString HvTxW::DecodeMacros(QString str,bool f_sh,int tx_id,bool my_call_is_std,bool his_call_is_std,uint8_t noQSO)
{
    QString str_out = str;
    QString his_call = le_his_call->getText();
    QString my_call = list_macros.at(0);
    //qDebug()<<"000="<< his_call<<str_out;
    bool f_his_call_empty = false;
    if (his_call.isEmpty()) f_his_call_empty = true;

    //jt65abc mskms pi4
    if (alljt65 || s_mode==10 || s_mode==12)
    {
        if (tx_id==1 || tx_id==2 || tx_id==3 || tx_id==4)//1.70 remove slash in calls
        {
            // in WSJT+sh and nostd call TX2=R5WM J6/LZ2HV OOO in MSHV TX2=R5WM LZ2HV KN23 OOO
            // in WSJT-X+sh TX2=mishmash
            his_call = MultiAnswerMod->FindBase6CharCallRemAllSlash(his_call);
            my_call  = MultiAnswerMod->FindBase6CharCallRemAllSlash(my_call);//KH1/KH7Z
        }
        else
        {
            //restrictions if slash tx_id==0 || tx_id==5 || tx_id==6
            bool f_pfx_sfx = false;
            if (MultiAnswerMod->FormatTxIfSlash6CharCall(my_call,tx_id,f_pfx_sfx))
            {
                his_call = MultiAnswerMod->FindBase6CharCallRemAllSlash(his_call);
                QStringList l_s = str_out.split(" ");
                if (tx_id==0)
                {
                    str_out = l_s.at(0)+" "+l_s.at(1);
                }
                if (tx_id==5)
                {
                    if (my_call.count()>9)
                    {
                        my_call = my_call.mid(0,10);
                        str_out = l_s.at(0)+" "+l_s.at(1);
                    }
                    if (f_pfx_sfx)
                        str_out = l_s.at(0)+" "+l_s.at(1);
                }
            }
            else if (MultiAnswerMod->FormatTxIfSlash6CharCall(his_call,tx_id,f_pfx_sfx))
            {
                if (tx_id==0)
                {
                    QStringList l_s = str_out.split(" ");
                    str_out = l_s.at(0)+" "+l_s.at(1);
                    //qDebug()<<"1111="<< his_call<<str_out;
                }
            }
        } //qDebug()<<"1111_V"<<ModeStr(s_mode);
    }

    if (s_mode==0 || s_mode==11 || s_mode==13 || s_mode==18 || allq65)//ft4
    { 	//qDebug()<<noQSO;
        if (tx_id==0)//2.02
        {
            /*if (!his_call_is_std)
                his_call = "<"+his_call+">";
            else if (!my_call_is_std)
                str_out ="<"+his_call+"> "+my_call;*/
            if (!his_call_is_std && !my_call_is_std && noQSO!=100) str_out ="<"+his_call+"> "+my_call;
            else if (!his_call_is_std) his_call = "<"+his_call+">";
            else if (!my_call_is_std) str_out ="<"+his_call+"> "+my_call;
        }
        else if (tx_id==1 || tx_id==2)
        {
            /*if ((!his_call_is_std && !s_my_call_is_std) || (!his_call_is_std && my_call_is_std))
                his_call = "<"+his_call+">";
            else if (his_call_is_std && !s_my_call_is_std)
                my_call  = "<"+my_call+">";*/
            if (s_cont_type==3)//2.39 new EU Contest
            {
                //his_call = "<"+MultiAnswerMod->FindBaseFullCallRemAllSlash(his_call)+">";
                //my_call  = "<"+s_my_base_call+">";
                //2.41 new new EU Contest, no base calls
                his_call = "<"+his_call+">";
                my_call  = "<"+my_call+">";
            }
            else if (!his_call_is_std && !my_call_is_std)
            {              
                if (noQSO==0 || noQSO==1 || noQSO==2 || noQSO==6) 
                {
                	his_call = "<"+his_call+">";
                	my_call = s_my_base_call;                	
               	}
            	if (noQSO==3 || noQSO==4 || noQSO==5) 
                {
                	his_call = MultiAnswerMod->FindBaseFullCallRemAllSlash(his_call);
                	my_call = "<"+my_call+">";                	
               	}
                if (noQSO==7) 
                {
                	his_call = "<"+his_call+">";
                	my_call  = "<"+my_call+">";                	
               	}
            }
            else if (!his_call_is_std && my_call_is_std) his_call = "<"+his_call+">";
            else if (his_call_is_std && !my_call_is_std) my_call  = "<"+my_call+">";
        }
        else if (tx_id==3 || tx_id==4)//rrr, 73
        {
            /*if (!his_call_is_std && !my_call_is_std)
            {
                his_call = MultiAnswerMod->FindBaseFullCallRemAllSlash(his_call);
                my_call = s_my_base_call;
            }
            else if (!his_call_is_std && my_call_is_std)
                my_call  = "<"+my_call+">";
            else if (his_call_is_std && !my_call_is_std)
                his_call  = "<"+his_call+">";*/
            if (!his_call_is_std && !my_call_is_std)
            {
                if (noQSO==0 || noQSO==1 || noQSO==2 || noQSO==5 || noQSO==6 || noQSO==7) my_call  = "<"+my_call+">";
                else if (noQSO==3 || noQSO==4) his_call = "<"+his_call+">";
                else
                {
                    his_call = MultiAnswerMod->FindBaseFullCallRemAllSlash(his_call);
                    my_call = s_my_base_call;
                }
            }
            else if (!his_call_is_std && my_call_is_std) my_call  = "<"+my_call+">";
            else if (his_call_is_std && !my_call_is_std) his_call  = "<"+his_call+">";
        }
        else if (tx_id==5)//cq
        {
            //if (!my_call_is_std)
            if (!MultiAnswerMod->isStandardCall(my_call))
            {
                QStringList l_s = str_out.split(" ");
                str_out = l_s.at(0)+" "+ my_call;
            }
        }

        if (s_cont_type>1)//all contesst !f_multi_answer_mod &&
        {
            if (TQueuedCall->haveQueuedCall())// RU Queued
            {
                if (tx_id==3) str_out.replace(" RRR"," RR73");
                //else if (tx_id==4)
                //str_out.replace(" 73"," RR73");
            }
            if (TQueuedCall->isLastFromQueued())
            {
                if (tx_id==2 && s_cont_type==5) str_out = "TU; "+str_out;//else if(s_id_contest_mode>1)                    
                else if (tx_id==3) str_out.replace(" RRR"," RR73");                    
                //else if (tx_id==4)
                //str_out.replace(" 73"," RR73");
            }
        }
        //if (tx_id==6)//?? cq test, cq 234
        //qDebug()<<"2222_V"<<ModeStr(s_mode)<<his_call<<his_call_is_std<<my_call<<my_call_is_std;
    }

    //if (f_sh && ((!his_call_is_std && !my_call_is_std) || (his_call_is_std && my_call_is_std) || tx_id==2))
    if (f_sh)
    {
        if (s_mode==0)//2.16 msk40 no possible emit RR73 bad message
        {
            if (tx_id==3) str_out.replace(" RR73"," RRR");//RRR
            else if (tx_id==4) str_out.replace(" RR73"," 73");//73
        }

        his_call = his_call.remove("<");
        his_call = his_call.remove(">");
        my_call = my_call.remove("<");
        my_call = my_call.remove(">");
        his_call = le_his_call->getText();//MultiAnswerMod->FindBaseFullCallRemAllSlash(his_call);/2.02 from rc3
        my_call = list_macros.at(0);//s_my_base_call;

        str_out.replace(QString("%T"), "<"+his_call);
        str_out.replace(QString("%M"), my_call+">");
        str_out.replace(QString("%H"), "<"+DetectSufix(his_call,false));
        str_out.replace(QString("%O"), DetectSufix(my_call,false)+">");
        str_out.replace(QString("%SH"), "<"+DetectSufix(his_call,true));
        str_out.replace(QString("%SO"), DetectSufix(my_call,true)+">");
    }
    else
    {
        str_out.replace(QString("%T"), his_call);
        str_out.replace(QString("%M"), my_call);
        str_out.replace(QString("%H"), DetectSufix(his_call,false));
        str_out.replace(QString("%O"), DetectSufix(my_call,false));
        str_out.replace(QString("%SH"), DetectSufix(his_call,true));
        str_out.replace(QString("%SO"), DetectSufix(my_call,true));
    }

    if (f_his_call_empty)//2.00 HV exeption reset msg no show <,>
    {
        str_out = str_out.remove("<");
        str_out = str_out.remove(">");
    }

    str_out.replace(QString("%R"), le_rst_tx->format_rpt(le_rst_tx->getText()));

    str_out.replace(QString("%G4"), list_macros.at(1).mid(0,4));

    //msk144 jt65abc mskms ft8 no %G6 only %G4  1.74
    //qDebug()<<s_id_contest_mode;
    if ((tx_id==1 || tx_id==2) && (s_mode==0 || s_mode==11 || s_mode==13 || s_mode==18 || allq65) && s_cont_type==3)//ft4 HV 2.00 for contest need to update
        str_out.replace(QString("%G6"), list_macros.at(1));
    else if (s_mode==0 || s_mode==11 || alljt65 || s_mode==10 || s_mode==12 || s_mode==13 || s_mode==18 || allq65)//ft4
        str_out.replace(QString("%G6"), list_macros.at(1).mid(0,4));
    else
        str_out.replace(QString("%G6"), list_macros.at(1));

    int sn = 0;
    QString qrgg = le_qrg->text();
    if (s_mode==0 || s_mode==11 || s_mode==13 || s_mode==18 || allq65)//msk144 ft8 ft4
    {
        if (qrgg.isEmpty() || qrgg.at(0).isLetter() || qrgg.at(1).isLetter() || qrgg.at(2).isLetter())
            str_out.replace(QString("%QRG"), qrgg);
        else
            str_out.replace(QString("%QRG"), QString("%1").arg(qrgg.toInt(),3,10,QChar('0')));//2.33

        sn = sb_txsn_v2->Value();
        str_out.replace(QString("%N"), QString("%1").arg(sn,4,10,QChar('0')));  //new contest sn 0001
    }
    else
    {
        str_out.replace(QString("%QRG"), qrgg);//2.33
        sn = sb_txsn->Value();
        str_out.replace(QString("%N"), QString("%1").arg(sn,3,10,QChar('0')));  // old modes rsq sn 001
    }
    //qDebug()<<"SWL"<<str_out;
    return str_out;
}
void HvTxW::bt_gen_msg()//for SWL
{
    QDateTime utc_t = QDateTime::currentDateTimeUtc();
    s_log_data_start=utc_t.toString("yyyyMMdd");
    s_log_time_start=utc_t.toString("hh:mm");
    le_qrg->SetEnter();//2.45
    gen_msg();
}
void HvTxW::SetDxParm(QString dc,QString re,QString dxg)//2.66
{
    static QString dc0 = ""; //qDebug()<<"-------------";
    static int irptt0 = 100; //100 = rpt not exist -50 +49 599
    static QString dxg0 = "";
    QString rptt = re;
    rptt = rptt.remove("+");
    int irptt = rptt.toInt();
    if (dc0==dc && irptt0==irptt && dxg0==dxg) return;
    dc0 = dc;
    irptt0 = irptt;
    dxg0 = dxg;
    TRadioAndNetW->SetDxParm(dc,QString("%1").arg(irptt),dxg); //qDebug()<<"SetDxParm="<<dc<<re<<dxg;
}
void HvTxW::RfreshDxParm() //2.66
{
    if (f_multi_answer_mod && (s_mode==11 || s_mode==13 || s_mode==18 || allq65)) return;//MAM and MAM Modes
    SetDxParm(le_his_call->getText(),le_rst_tx->getText(),LeHisLoc->getText());
    /*if (!f_multi_answer_mod || (s_mode!=11 && s_mode!=13 && s_mode!=18 && !allq65))//MAM Modes
    {	
    	QString dc = le_his_call->getText();
    	QString re = le_rst_tx->getText();
    	QString dxg = LeHisLoc->getText();
    	SetDxParm(dc,re,dxg); 
    }*/
}
void HvTxW::gen_msg()
{
    //qDebug()<<le_his_call->getText()<<le_rst_tx->getText();
    //le_rst->text();
    /*%T %M
    %T %M %R
    %T %M R%R
    %M RRR
    %M 73    
    CQ %M*/
    le_rst_tx->SetText(le_rst_tx->format_rpt(le_rst_tx->getText()),false);
    bool c1std = true;
    bool c2std = true;
    uint8_t noQSO = 0;
    if (s_mode==0 || s_mode==11 || s_mode==13 || s_mode==18 || allq65) MultiAnswerMod->isStandardCalls(list_macros.at(0),le_his_call->getText(),c1std,c2std,noQSO);
    for (int i = 0; i<count_tx_widget; i++)
    {
        HvTxIn *TempHvTxIn = (HvTxIn*)V_l->itemAt(i)->widget();

        if ((s_mode==0 || s_mode==11 || s_mode==13 || s_mode==18 || allq65) && cb_sh_rpt->isChecked() && (i>=2 && i<=4) && s_cont_type==0)//ft4 && !s_msk144_contest_mode msk144 ft8 v1.42
            TempHvTxIn->line_txt->setText(DecodeMacros(list_macros.at(i+2),true,i,c1std,c2std,noQSO));
        else if ((s_mode==0 || s_mode==11 || s_mode==13 || s_mode==18 || allq65) && cb_sh_rpt->isChecked() && s_cont_type>1 && (i>=3 && i<=4))
            TempHvTxIn->line_txt->setText(DecodeMacros(list_macros.at(i+2),true,i,c1std,c2std,noQSO));
        //////////////////Correct "JT65ABC + SH" from makros-> msk144 + SH/////////////////////////
        else if ((alljt65 || s_mode==10) && cb_sh_rpt->isChecked() && (i>=0 && i<=4))
        {
            if 		(i==0) TempHvTxIn->line_txt->setText(DecodeMacros("%T %M %G4",false,i,c1std,c2std,noQSO));
            else if (i==1) TempHvTxIn->line_txt->setText(DecodeMacros("%T %M %G4",false,i,c1std,c2std,noQSO)+" OOO");
            else if (i==2) TempHvTxIn->line_txt->setText("RO");
            else if (i==3) TempHvTxIn->line_txt->setText("RRR");
            else if (i==4) TempHvTxIn->line_txt->setText("73");
        }
        /////////////////Correct "JT65ABC MSKMS NORMAL" from makros-> msk144 contest mode////////////////
        else if ((alljt65 || s_mode==10 || s_mode==12) && s_cont_type>1 && (i==0 || i==1 || i==2 || i==3 || i==5))
        {
            if 		(i==0) TempHvTxIn->line_txt->setText(DecodeMacros("%T %M %G4",false,i,c1std,c2std,noQSO));
            else if (i==1) TempHvTxIn->line_txt->setText(DecodeMacros("%T %M %R",false,i,c1std,c2std,noQSO));
            else if (i==2) TempHvTxIn->line_txt->setText(DecodeMacros("%T %M R%R",false,i,c1std,c2std,noQSO));
            else if (i==3) TempHvTxIn->line_txt->setText(DecodeMacros("%T %M RR73",false,i,c1std,c2std,noQSO));//2.31 =RR73 2.02 =RRR
            else if (i==5) TempHvTxIn->line_txt->setText(DecodeMacros("CQ %M %G4",false,i,c1std,c2std,noQSO));
        }
        ///////////////END JT65ABC MSKMS /////////////////////////////////////////////////////////////////
        else TempHvTxIn->line_txt->setText(DecodeMacros(list_macros.at(i+2),false,i,c1std,c2std,noQSO));

        if (i!=s_b_identif) TempHvTxIn->rb_tx->setChecked(false);
        else SetEmitMessage(TempHvTxIn->line_txt->text(),false,false,false);
    }
    SaveSettings();
    //if (!f_multi_answer_mod) SetDxParm(le_his_call->getText(),le_rst_tx->getText(),LeHisLoc->getText()); //2.66 stop
}
void HvTxW::SetTxSnV2(int v)
{
    gen_msg();
    MultiAnswerMod->SetTxSnV2(v);
}
void HvTxW::SetQSOProgressAll(int idd,bool from_mam)//2.51 imoprtent form MultiAnswerMod ClrNow()
{
    static int prew_idd = -1;//or 5 default
    int idd_out = idd;
    if (from_mam && (!f_multi_answer_mod || (s_mode!=11 && s_mode!=13 && s_mode!=18 && !allq65)))//MAM modes here ft8,ft4  || allq65
    {
        for (int i = 0; i<count_tx_widget; i++)
        {
            HvTxIn *TempHvTxIn = (HvTxIn*)V_l->itemAt(i)->widget();
            if (TempHvTxIn->rb_tx->isChecked())
            {
                idd_out = i;
                break;
            }
        }
    }
    if (idd_out != prew_idd)
    {
        /*QString fff = "FALSE"; if (f_multi_answer_mod) fff = "TRUE ";
        if(from_mam) qDebug()<<"->MAM="<<fff<<s_mode<<"TX="<<idd_out+1;
        else 		 qDebug()<<"  TXW="<<fff<<s_mode<<"TX="<<idd_out+1;*/
        prew_idd = idd_out;
        emit EmitQSOProgress(idd_out);

        if (s_mode==0 || s_mode==12)
        {
            if (idd_out==0 || idd_out==5 || idd_out==6) SetLockTxRstMskASeq(false);//2.59
            /*if (s_cont_type==5) //Roundup type = 5 for the future
            {
            if (idd_out==5 || idd_out==6) SetLockTxRstMskASeq(false);		
            }
            else
            {
            if (idd_out==0 || idd_out==5 || idd_out==6) SetLockTxRstMskASeq(false);		
            }*/
        }
    }
    if (from_mam && f_multi_answer_mod && idd==5 && (s_mode==11 || s_mode==13 || s_mode==18 || allq65))//2.66 5=CQ MAM modes
    {
        //QString rpt = MultiAnswerMod->format_rpt_ma("+00");
        QString rpt = "+00";
        if (f_multi_answer_mod_std)
        {
            if (s_cont_type==5) rpt = "599";
            if (s_cont_type==3) rpt = "59";
        }
        SetDxParm("",rpt,""); //qDebug()<<"7777"<<rpt;
    }
}
void HvTxW::RbPress(int identif)
{
    //qDebug()<<identif<<s_b_identif;
    s_b_identif = identif;
    for (int i = 0; i<count_tx_widget; i++)
    {
        HvTxIn *TempHvTxIn = (HvTxIn*)V_l->itemAt(i)->widget();
        if (i!=identif)
            TempHvTxIn->rb_tx->setChecked(false);
        else
        {
            SetQSOProgressAll(identif,false);//2.51
            SetEmitMessage(TempHvTxIn->line_txt->text(),false,false,false);
        }
    }
}
void HvTxW::BReleased(int identif,QString str)
{
    s_b_identif = identif;
    for (int i = 0; i<count_tx_widget; i++)
    {
        HvTxIn *TempHvTxIn = (HvTxIn*)V_l->itemAt(i)->widget();
        if (i!=identif)
        {
            TempHvTxIn->rb_tx->setChecked(false);
        }
        else
        {
            SetQSOProgressAll(identif,false);//2.51
            TempHvTxIn->rb_tx->setChecked(true);
            SetEmitMessage(str,true,false,false);
            //EmitMessage("SP9HWY RR73; TA2NC <LZ2HV> +04",true,true);
            //qDebug()<<i<<TempHvTxIn->line_txt->text();
        }
    }
}
void HvTxW::SetTxMessage(int identif)
{   
    if (g_no_block_tx!=100)//101=noblock 100=blockall 6=blocktx7 1=blocktx2
    {
        if (g_no_block_tx!=identif)
        {
            HvTxIn *TempHvTxIn = (HvTxIn*)V_l->itemAt(identif)->widget();
            TempHvTxIn->b_released();
        }
    }
}
void HvTxW::Check(QString s_type)
{	//qDebug()<<s_type;
    if (s_type == "call")
    {
        //if (le_his_call->getText().count()>1) //qDebug()<<"e="<<le_his_call->getText();
        if (THvQthLoc.isValidCallsign(le_his_call->getText()))
        {
            le_his_call->setError(false);
            le_his_call->setErrorColorLe(false);
            block_call = false;
        }
        else
        {
            le_his_call->setError(true);
            le_his_call->setErrorColorLe(true);
            block_call = true;
        }
        MarkTextChanged(false);//bool false  norefresh b4 qso

        QString dbloc = TMsDb->CheckBD(le_his_call->getText());
        if (dbloc!="")
        {
            if (dsty) pb_checkdb->setStyleSheet("QPushButton{color:rgb(255,255,255)}");
            else pb_checkdb->setStyleSheet("QPushButton{color:"+ColorStr_[0]+"}");
            l_loc_from_db->setText("DB:"+dbloc);

            if (LeHisLoc->getText()==dbloc)
                //LeHisLoc->setErrorColorLe(false);
                LeHisLoc->setError(false);
            else
                //LeHisLoc->setErrorColorLe(true);
                LeHisLoc->setError(true);
            LeHisLoc->SetText(dbloc);
        }
        else
        {
            if (dsty) pb_checkdb->setStyleSheet("QPushButton{color:rgb(255,200,0)}");
            else pb_checkdb->setStyleSheet("QPushButton{color:"+ColorStr_[1]+"}");
            l_loc_from_db->setText("DB:NA");
            //LeHisLoc->setErrorColorLe(true);
            LeHisLoc->setError(true);
            LeHisLoc->SetText("");
        }
        emit EmitFileNameChenged();
        RfreshDxParm();//2.66
    }

    if (s_type == "call_mon_1")
    {
        if (THvQthLoc.isValidCallsign(le_mon_call1->getText()))
        {
            le_mon_call1->setError(false);
            le_mon_call1->setErrorColorLe(false);
            block_mon_call1 = false;
        }
        else
        {
            le_mon_call1->setError(true);
            le_mon_call1->setErrorColorLe(true);
            block_mon_call1 = true;
        }
        MarkTextChanged(false);//bool false  norefresh b4 qso
    }
    if (s_type == "call_mon_2")
    {
        if (THvQthLoc.isValidCallsign(le_mon_call2->getText()))
        {
            le_mon_call2->setError(false);
            le_mon_call2->setErrorColorLe(false);
            block_mon_call2 = false;
        }
        else
        {
            le_mon_call2->setError(true);
            le_mon_call2->setErrorColorLe(true);
            block_mon_call2 = true;
        }
        MarkTextChanged(false);//bool false  norefresh b4 qso
    }

    if (s_type == "locator")
    {
        if (!LeHisLoc->getText().isEmpty() && THvQthLoc.isValidLocator(LeHisLoc->getText()))
        {
            //LeHisLoc->setError(false);
            LeHisLoc->setErrorColorLe(false);
            block_loc = false;

            THvAstroDataW->SetMyLocHisLocBand(list_macros.at(1),LeHisLoc->getText(),s_band);
        }
        else
        {
            //LeHisLoc->setError(true);
            LeHisLoc->setErrorColorLe(true);
            block_loc = true;

            THvAstroDataW->SetMyLocHisLocBand(list_macros.at(1),list_macros.at(1),s_band);
        }
        MarkTextChanged(false);//bool false  norefresh b4 qso

        QString dbloc = TMsDb->CheckBD(le_his_call->getText());
        if (dbloc!="")
        {
            if (LeHisLoc->getText()==dbloc)
                //LeHisLoc->setErrorColorLe(false);
                LeHisLoc->setError(false);
            else
                //LeHisLoc->setErrorColorLe(true);
                LeHisLoc->setError(true);
        }
        else
        {
            //LeHisLoc->setErrorColorLe(true);
            LeHisLoc->setError(true);
        }
        CalcDistance();
        RfreshDxParm();//2.66
    }
    if (s_type == "rsttx") RfreshDxParm();//2.66

    if (!le_his_call->getError() && !LeHisLoc->getError())
    {
        if (dsty) pb_adddb->setStyleSheet("QPushButton{color:rgb(255,255,255)}");
        else pb_adddb->setStyleSheet("QPushButton{color:"+ColorStr_[0]+"}");
    }
    else
    {
        if (dsty) pb_adddb->setStyleSheet("QPushButton{color: rgb(255,200,0)}");
        else pb_adddb->setStyleSheet("QPushButton{color:"+ColorStr_[1]+"}");
    }
}
void HvTxW::SetDistUnit(bool f_)
{
    f_km_mi = f_;
    TRadioAndNetW->SetDistUnit(f_);
    MultiAnswerMod->SetDistUnit(f_);
    THvLogW->SetDistUnit(f_);
    CalcDistance();
    emit EmitDistUnit(f_);
}
void HvTxW::CalcDistance()
{
    if (f_makros_ready)
    {
        if (block_loc)
        {
            s_dist = "NA";
            s_beam = "NA";
            if (!f_km_mi)
                l_dist->setText("NA km");
            else
                l_dist->setText("NA mi");

            QString nondeg = (QString("NA")+QChar(0xB0));
            l_beam->setText(nondeg);
            l_el->setText(nondeg);
            l_haz->setText("Hot : "+nondeg);
        }
        else
        {
            QString c_test_loc = THvQthLoc.CorrectLocator(LeHisLoc->getText());
            QString c_my_loc = THvQthLoc.CorrectLocator(list_macros.at(1));

            double dlong1 = THvQthLoc.getLon(c_my_loc);
            double dlat1  = THvQthLoc.getLat(c_my_loc);
            double dlong2 = THvQthLoc.getLon(c_test_loc);
            double dlat2 = THvQthLoc.getLat(c_test_loc);

            int dist_km = THvQthLoc.getDistanceKilometres(dlong1,dlat1,dlong2,dlat2);

            if (!f_km_mi)
            {
                s_dist = QString("%1").arg(dist_km);
                l_dist->setText(s_dist+" km");
            }
            else
            {
                s_dist = QString("%1").arg(THvQthLoc.getDistanceMilles(dlong1,dlat1,dlong2,dlat2));
                l_dist->setText(s_dist+" mi");
            }
            int az = THvQthLoc.getBeam(dlong1,dlat1,dlong2,dlat2);
            s_beam = QString("%1").arg(az);
            l_beam->setText(s_beam+QChar(0xB0));

            //  ("El:NA Deg");

            QStringList t = s_log_time_now.split(":");
            double utch=t.at(0).toInt()+t.at(1).toInt()/60.0;// + utc[5]/3600.0

            QStringList elhot = THvQthLoc.getElAndHot(utch,dist_km,az,dlong1,dlat1,dlong2,dlat2);
            l_el->setText(elhot.at(0)+QChar(0xB0));
            l_haz->setText(elhot.at(1)+QChar(0xB0));
            /*QString sout = elhot.at(1).mid(0,7)+elhot.at(1).mid(7,elhot.at(1).count()).rightJustified(3,' ')+", "
                           +"Azimuth: "+QString("%1").arg(az).rightJustified(3,' ')
                           +", Elevation: "+elhot.at(0).rightJustified(3,' ');
            qDebug()<<sout;*/
        }
    }
}
void HvTxW::SetDecodeInProgresPskRep(bool f)
{
    //only JTMS FSK441 FSK315 ISCAT-A ISCAT-B JT6M JT65ABC PI4
    if (s_mode==1 || s_mode==2 || s_mode==3 || s_mode==4 || s_mode==5 || s_mode==6 ||
            s_mode==7 || s_mode==8 || s_mode==9 || s_mode==10)
    {
        if (f)
        {
            max_snr_psk_rep = -200;
            hisCall_pskrep = "";
            hisLoc_pskrep = "";
            //s_freq_offset = 0;
        }
        else
        {
            if (prev_hisCall_pskrep == hisCall_pskrep && prev_hisLoc_pskrep == hisLoc_pskrep
                    && prev_max_snr_psk_rep == max_snr_psk_rep && prev_mode_pskrep == ModeStr(s_mode))
                return;
            else
            {
                prev_max_snr_psk_rep = max_snr_psk_rep;
                prev_hisCall_pskrep = hisCall_pskrep;
                prev_hisLoc_pskrep = hisLoc_pskrep;
                prev_mode_pskrep = ModeStr(s_mode);
            }

            //qDebug()<<"ACUMULATED="<<"hisCall_pskrep="<<hisCall_pskrep<<"hisLoc_pskrep="<<hisLoc_pskrep<<"Mode="<<ModeStr(s_mode)<<"MaxSNR="<<max_snr_psk_rep;

            if (!hisCall_pskrep.isEmpty() && s_mode==10)// pi4
            {
                int freq_offset = 0;//fictive
                TRadioAndNetW->AddRemoteStation(hisCall_pskrep,hisLoc_pskrep,freq_offset,ModeStr(s_mode),max_snr_psk_rep,0,"NA","NA#15");//id 0->to psk reporter 1->to dx clusters spot
                //qDebug()<<"ACUMULATED="<<"hisCall_pskrep="<<hisCall_pskrep<<"hisLoc_pskrep="<<hisLoc_pskrep<<"Mode="<<ModeStr(s_mode)<<"MaxSNR="<<max_snr_psk_rep;
                max_snr_psk_rep = -200;
                hisCall_pskrep = "";
                hisLoc_pskrep = "";
                //s_freq_offset = 0;
            }
            else if (!hisCall_pskrep.isEmpty() && !hisLoc_pskrep.isEmpty())
            {
                int freq_offset = 0;//fictive
                TRadioAndNetW->AddRemoteStation(hisCall_pskrep,hisLoc_pskrep,freq_offset,ModeStr(s_mode),max_snr_psk_rep,0,"NA","NA#15");
                //qDebug()<<"ACUMULATED="<<"hisCall_pskrep="<<hisCall_pskrep<<"hisLoc_pskrep="<<hisLoc_pskrep<<"Mode="<<ModeStr(s_mode)<<"MaxSNR="<<max_snr_psk_rep;
                max_snr_psk_rep = -200;
                hisCall_pskrep = "";
                hisLoc_pskrep = "";
                //s_freq_offset = 0;
            }
        }
    }
    if (!f)
    {
        //qDebug()<<"PSK RESTRICT ============= FALSE ====>"<<fpsk_restrict;
        fpsk_restrict = false;
    }
}
void HvTxW::TryFindCallLocForSpot(QString in,QString &call,QString &loc)
{
    //in = in+"\n";
    //qDebug()<<"11TryFindCallLocForSpot="<<in;
    QString in_tmp = in.replace("\n"," ");//1.54=tested ok need      zaradi JT6M jtms
    //qDebug()<<"2222222222222TryFindCallLocForSpot="<<in_tmp;
    QStringList ls_text = in_tmp.split(" ");
    for (int i = ls_text.count()-1; i >=0 ; i--)
    {
        if (loc.isEmpty() && THvQthLoc.isValidLocator(ls_text.at(i)) && ls_text.at(i)!="RR73")//1.54=RR73 if loc is missing
        {
            loc = ls_text.at(i);
            continue;
        }
        //1.54=RR73 list_macros.at(0) no my call
        if (call.isEmpty() && THvQthLoc.isValidCallsign(ls_text.at(i)) && list_macros.at(0)!=ls_text.at(i) && ls_text.at(i)!="RR73")
        {
            call=ls_text.at(i);
            if (loc.isEmpty())
                loc = TMsDb->CheckBD(call);
            break;
        }
    }
}
void HvTxW::ValidateStationInfo(QStringList list_in, int id, bool emitudpdectxt,uint8_t id_voms)//id 0->to psk reporter 1->to dx clusters spot,  bool emitudpdectxt
{
    if (!list_in.isEmpty())
    {
        QString hisCall_spot = "";
        QString hisLoc_spot  = "";
        int snr_spot = 0;
        int freq_offset = 0;//1.61= only for ft8 for the moment
        QString sdt  = "";
        QString smsg  = "";
        static QString psk_restrict_t = "_";
        bool fpsk_block = false;
        
        if (s_mode==11 || s_mode==13 || s_mode==18 || allq65)//only ft8 ft4 allq65
        {
            if (fpsk_restrict)
            {
                fpsk_restrict = false;
                psk_restrict_t = list_in.at(0);
            }
            if (list_in.at(0) == psk_restrict_t)
            {
                fpsk_block = true;
            }
            else
            {
                fpsk_block = false;
                psk_restrict_t = "_";
            }
        }//qDebug()<<"list_in.at(0)="<<list_in.at(0)<<fpsk_block;
        //2.73 Dist Country
        if (((s_mode==0 || s_mode==12) && list_in.count()>6) || ((s_mode==11 || s_mode==13 || s_mode==18 || allq65) && list_in.count()>9))//MSK144 MSK40 FT8 ft4 have column 4
        {
            hisCall_pskrep = "";
            hisLoc_pskrep  = "";

            QStringList ls_text;
            QString strt;
            if (s_mode==0 || s_mode==12) strt = list_in.at(6);// MSK144  MSK144msk
            if (s_mode==11 || s_mode==13 || s_mode==18 || allq65) strt = list_in.at(4);// ft8 ft4

            smsg = strt;
            //for ft8+fox and msk144+sh
            QString strt0 = strt+"xxxx"; //qDebug()<<strt;
            if (strt0.mid(0,4)=="TU; ") strt.remove("TU; ");
            int cst0 = strt0.count();
            if (cst0>6) 
            {
            	if (strt0.mid(cst0-7,3)==" TU") strt.remove(cst0-7,3);//2.76.5 strt.remove(" TU");
           	}
            strt.remove("<"); //qDebug()<<strt;
            strt.remove(">");
            strt.remove(";");

            int pos_loc_call_as_grig = 100;//2.13 exeprion call_as_grig PA70, PA70X, PA70XX
            ls_text = strt.split(" ");
            //"TA2NC RR73 SP9HWY <...> +00"
            //<...> LZ33MM +00              v2 exception
            //LZ33MM <...> +00              v2 exception
            //"TU; LZ2HV <...> R 589 NY"    TU;<-removed
            //"TU; <...> LZ2HV R 589 0001"  TU;<-removed
            //str="<...> +07";              msk144 +sh
            //ft8+msk144 + fox exeption for call no exist and v2 non-standard calls
            for (int i = ls_text.count()-1; i >=0 ; i--)
            {
                //ft8 ft4 + fox exeption for call no exist 1.69
                if ((s_mode==11 || s_mode==0 || s_mode==13 || s_mode==18 || allq65) && i>0 && ls_text.at(i)=="...")//2.20
                {
                    hisCall_pskrep="...";
                    hisCall_spot="...";
                }
                if (ls_text.at(i)=="RR73") continue;//{}//2.75
                else if (i!=0 && i!=1 && hisLoc_pskrep.isEmpty() && THvQthLoc.isValidLocator(ls_text.at(i)))//2.75 (i!=0 && i!=1)
                {
                    hisLoc_pskrep = ls_text.at(i);
                    hisLoc_spot =   ls_text.at(i);
                    pos_loc_call_as_grig = i;//2.13 exeprion call_as_grig PA70, PA70X, PA70XX
                }
                else if (hisCall_pskrep.isEmpty() && THvQthLoc.isValidCallsign(ls_text.at(i)))
                {
                    hisCall_pskrep = ls_text.at(i);
                    hisCall_spot =   ls_text.at(i);
                    //////////////////// new MSK144MS ////////////////////
                    if (s_mode==12)// rpt_ms hisCall exception
                    {
                        bool is_rpt_ms = false;
                        for (int x = 0; x < COUNT_RPT_MS; ++x)
                        {
                            QString r_rpt = "R"+rpt_ms_p[x];
                            if (r_rpt == hisCall_pskrep)
                            {
                                is_rpt_ms = true;
                                break;
                            }
                        }
                        if (is_rpt_ms)
                        {
                            hisCall_pskrep = "";
                            hisCall_spot = "";
                        }
                    }
                    ////////////////////end new MSK144MS //////////////////
                }//if (hisCall_pskrep == list_macros.at(0) && id == 0) return;//2.75 move down for SMsg no emit if is my call
            }
            //2.13 exeprion call_as_grig PA70, PA70X, PA70XX
            /*if (pos_loc_call_as_grig==1)//2.14 exeption fox to me as 1 call msg
            {
                if (hisLoc_pskrep!="RR73")
                {
                    hisCall_pskrep = hisLoc_pskrep;
                    hisCall_spot = hisLoc_spot;
                    hisLoc_pskrep = "";
                    hisLoc_spot = "";
                }
            }
            else if (pos_loc_call_as_grig==0)//2.14 exeption fox to me as 1 call msg
            {
                if (hisLoc_pskrep!="RR73")
                {
                    //if(FindBaseFullCallRemAllSlash(hisLoc_inmsg)==s_my_base_call)
                    //myCall_inmsg = hisLoc_inmsg;
                    hisLoc_pskrep = "";
                    hisLoc_spot = "";
                }
            }*/
            if (pos_loc_call_as_grig==3 && ls_text.count()==5)//2.75
            {   //SMsg->"KK2HV RR73; LZ2HV <RP79GD> +06", LZ2HV <...> R 589 NY", <...> LZ2HV R 589 0001", WA9XYZ KA1ABC R 16A EMA
                //<RP9DD> <RP79GD> R 590057 KN23SF, <RP9DD> <RP79GD> 590057 KN23SF
                /*if (FindBaseFullCallRemAllSlash(hisCall_inmsg)==s_my_base_call)
                {
                    myCall_inmsg = hisCall_inmsg;//pos->hisCall_inmsg>0, pos myCall_inmsg is >0, no need->"&& i>0"
                    if (s_mode==11 || s_mode==13 || s_mode==18|| allq65) is_fox_msg_for_me_as2call_ft8 = true;
                }*/
                hisCall_pskrep = hisLoc_pskrep;
                hisCall_spot = hisLoc_spot;
                hisLoc_pskrep = "";
                hisLoc_spot = "";
            }
            //2.75 "LZ2HV/B KN23" "W0A/B KN23"
            if (ls_text.count() == 2/*&& THvQthLoc.isValidCallsign(ls_text.at(0))*/)
            {
                int idx = ls_text.at(0).indexOf("/B");
                if (idx>2 && THvQthLoc.isValidLocator(hisCall_pskrep))
                {
                    hisLoc_pskrep = hisCall_pskrep;
                    hisLoc_spot = hisCall_spot;
                    hisCall_pskrep = ls_text.at(0);
                    hisCall_spot = ls_text.at(0);
                }
            } //qDebug()<<hisCall_pskrep<<hisLoc_pskrep<<hisCall_spot<<hisLoc_spot;
            //2.13 END exeprion call_as_grig PA70, PA70X, PA70XX
            
            bool fpsk_block2 = false;//2.76sf           
            if (s_mode==11)
        	{   //id_voms -> 0=normal msg, 1=$VERIFY$, 2=C0ALL.123456, 3=verified & invalid //4=invalid
                if (id_voms==2) 
                {
        			QTime t = QTime::fromString(list_in.at(0),"hhmmss");
        			if (t.isValid()) 
        			{
        				QString st0 = strt;
        				st0.replace("."," ");
        				TRadioAndNetW->SendOtpCheck(list_in.at(0)+" $VERIFY$ "+st0+" "+list_in.at(9));
       				}
        			fpsk_block2 = true;                	
               	}
        		if (id_voms==1 && id_mshf>0)
        		{
        			QTime t = QTime::fromString(list_in.at(0),"hhmmss");
        			if (t.isValid()) TRadioAndNetW->SendOtpCheck(list_in.at(0)+" "+strt+" "+list_in.at(9));
        			fpsk_block2 = true;        			
       			}
       			if (id_voms>2) fpsk_block2 = true;
       		}//qDebug()<<"sfoxvfy="<<sfoxvfy;
       		                        
            if (hisCall_pskrep == list_macros.at(0) && id == 0) fpsk_block2 = true;//2.76 no -> return;//2.75 no emit if is my call
            if (s_mode==0 || s_mode==11 || s_mode==13 || s_mode==18 || allq65)//ft8 ft4 + fox big exeption for fox msg 1.69
            {
                if (hisCall_pskrep=="...")//big exeption for fox msg 1.69
                {
                    hisCall_pskrep="";
                    hisCall_spot = "";
                }
            }
            if (hisCall_pskrep==le_his_call->getText())// only shure they my partner
            {
                if (LeHisLoc->getText().contains(hisLoc_pskrep))//if shure to full locator
                {
                    hisLoc_pskrep = LeHisLoc->getText();
                    hisLoc_spot =   LeHisLoc->getText();
                }
                else if (hisLoc_pskrep.isEmpty())
                {
                    hisLoc_pskrep = LeHisLoc->getText(); //no loc but they my partner to full locator
                    hisLoc_spot =   LeHisLoc->getText();
                }
            }
            if (s_mode==0 || s_mode==12)//MSK144 MSK40 msk144ms
            {
                /*if (cb_rpt_db_msk->isChecked())
                {
                    if (list_in.at(4).isEmpty())// vavno moze i da e prazno
                        snr_spot = 26;
                    else
                        snr_spot = list_in.at(4).toInt();
                }
                else*/
                snr_spot = list_in.at(3).toInt();//vinagi db za spot i pska reporter
                //freq_offset = 0;//list_in.at(11).toInt();
                sdt = list_in.at(1); //qDebug()<<"ALL="<<list_in<<list_in.at(10);
            }
            if (s_mode==11 || s_mode==13 || s_mode==18 || allq65) // FT8 ft4
            {
                snr_spot = list_in.at(1).toInt();
                freq_offset = list_in.at(9).toInt();//only for ft8 //2.73 Dist Country
                sdt = list_in.at(2);
            }
            //to here no my partner but have loc emit
            //if no my partner no loc no emit
            //qDebug()<<"hisCall="<<hisCall_pskrep<<"hisLoc="<<hisLoc_pskrep<<"pos_loc="<<pos_loc_call_as_grig<<"Ls_count="<<ls_text.count();
            //qDebug()<<"hisCall="<<hisCall_pskrep<<"hisLoc="<<hisLoc_pskrep<<"Mode="<<ModeStr(s_mode)<<"MaxSNR="<<snr_spot<<fpsk_block;
            if (!hisCall_pskrep.isEmpty() && !hisLoc_pskrep.isEmpty() && id==0 && !fpsk_block && !fpsk_block2)//v1.41 id 0->to psk reporter 1->to dx clusters
            {
            	QString hhmmsspt = "NA#15";//2.76.4
            	if (s_mode==11 || s_mode==13 || s_mode==18 || allq65)
            	{
            		hhmmsspt = list_in.at(0);
            		hhmmsspt.append("#"+QString("%1").arg(SB_PeriodTime->get_period_time(),0,'f',1));            		
           		}
                TRadioAndNetW->AddRemoteStation(hisCall_pskrep,hisLoc_pskrep,freq_offset,ModeStr(s_mode),snr_spot,id,"NA",hhmmsspt);
                //qDebug()<<"hisCall="<<hisCall_pskrep<<"hisLoc="<<hisLoc_pskrep<<"Mode="<<ModeStr(s_mode)<<"MaxSNR="<<snr_spot;
            }
        }
        else if ((s_mode==1 || s_mode==2 || s_mode==3) && list_in.count()>6)//JTMS FSK441 FSK315  have column 6
        {
            if (list_in.at(6).contains(le_his_call->getText()))
            {
                hisCall_pskrep = le_his_call->getText();
                hisLoc_pskrep =  LeHisLoc->getText();
                hisCall_spot =   le_his_call->getText();
                hisLoc_spot =    LeHisLoc->getText();
                if (list_in.at(3).toInt()>max_snr_psk_rep) max_snr_psk_rep = list_in.at(3).toInt();
                //qDebug()<<"snr1="<<list.at(3).toInt();
            }
            TryFindCallLocForSpot(list_in.at(6),hisCall_spot,hisLoc_spot);
            snr_spot = list_in.at(3).toInt();
            //freq_offset = 0;
            smsg = list_in.at(6);
            sdt = list_in.at(1);
        }
        else if ((s_mode==4 || s_mode==5) && list_in.count()>6)//ISCAT-A ISCAT-B
        {
            if (list_in.at(6).contains(le_his_call->getText()))
            {
                hisCall_pskrep = le_his_call->getText();
                hisLoc_pskrep =  LeHisLoc->getText();
                hisCall_spot =   le_his_call->getText();
                hisLoc_spot =    LeHisLoc->getText();
                if (list_in.at(2).toInt()>max_snr_psk_rep) max_snr_psk_rep = list_in.at(2).toInt();
                //qDebug()<<"snr2="<<list.at(2).toInt();
            }
            TryFindCallLocForSpot(list_in.at(6),hisCall_spot,hisLoc_spot);
            snr_spot = list_in.at(2).toInt();
            //freq_offset = 0;
            smsg = list_in.at(6);
            sdt = list_in.at(3);
        }
        else if (s_mode==6 && list_in.count()>5)//JT6M
        {
            if (list_in.at(5).contains(le_his_call->getText()))
            {
                hisCall_pskrep = le_his_call->getText();
                hisLoc_pskrep =  LeHisLoc->getText();
                hisCall_spot =   le_his_call->getText();
                hisLoc_spot =    LeHisLoc->getText();
                if (list_in.at(3).toInt()>max_snr_psk_rep) max_snr_psk_rep = list_in.at(3).toInt();
                //qDebug()<<"snr3="<<list.at(3).toInt();
            }
            TryFindCallLocForSpot(list_in.at(5),hisCall_spot,hisLoc_spot);
            snr_spot = list_in.at(3).toInt();
            //freq_offset = 0;
            smsg = list_in.at(5);
            sdt = list_in.at(1);
        }
        else if ((s_mode==7 || s_mode==8 || s_mode==9) && list_in.count()>6)//jt65abc
        {
            if (list_in.at(6).contains(le_his_call->getText()))
            {
                hisCall_pskrep = le_his_call->getText();
                hisLoc_pskrep =  LeHisLoc->getText();
                hisCall_spot =   le_his_call->getText();
                hisLoc_spot =    LeHisLoc->getText();
                if (list_in.at(2).toInt()>max_snr_psk_rep) max_snr_psk_rep = list_in.at(2).toInt();
                //qDebug()<<"snr2="<<list.at(2).toInt();
            }
            TryFindCallLocForSpot(list_in.at(6),hisCall_spot,hisLoc_spot);
            snr_spot = list_in.at(2).toInt();
            //freq_offset = 0;
            smsg = list_in.at(6);
            sdt = list_in.at(3);
        }
        else if (s_mode==10 && list_in.count()>6)//pi4
        {
            if (THvQthLoc.isValidCallsign(list_in.at(6)))
            {
                hisCall_pskrep = list_in.at(6);
                hisLoc_pskrep = "";
                hisCall_spot =   list_in.at(6);
                hisLoc_spot =   "";
                if (list_in.at(2).toInt()>max_snr_psk_rep) max_snr_psk_rep = list_in.at(2).toInt();
                snr_spot = list_in.at(2).toInt();
                //freq_offset = 0;
                smsg = list_in.at(6);
                sdt = list_in.at(3);
            }
        }

        if (id==1)//v1.41 id 0->to psk reporter 1->to dx clusters
        {
            QString pwidth = "NA";
            if (s_mode==1 || s_mode==2 || s_mode==3 || s_mode==12) pwidth = list_in.at(2);//all JTMS FSK441/315 and MSK144MS
            bool spot_res = true;
            QString c1 = MultiAnswerMod->FindBaseFullCallRemAllSlash(hisCall_spot);//0 full
            QString c2 = MultiAnswerMod->FindBaseFullCallRemAllSlash(list_macros.at(0));//0 full
            if (c1==c2)
            {
                QString text = "<p align='center'>"+tr("Are you sure you want to spot your call")+" "+hisCall_spot+" ?</p>";
                int ret = QMessageBox::warning(this, "MSHV", text, QMessageBox::Ok | QMessageBox::Cancel);
                switch (ret)
                {
                case QMessageBox::Cancel:
                    spot_res = false;
                    break;
                case QMessageBox::Ok:
                    spot_res = true;
                    break;
                default:
                    spot_res = false;
                    break;
                }
            }
            if (spot_res) TRadioAndNetW->AddRemoteStation(hisCall_spot,hisLoc_spot,freq_offset,ModeStr(s_mode),snr_spot,id,pwidth,"NA#15");
        }
        //qDebug()<<"Status start"<<smsg;
        //2.43  && id==0 no emit if to DX Cluster  //v1.41 id 0->to psk reporter 1->to dx clusters ,  bool emitudpdectxt
        if (TRadioAndNetW->GetUdpBroadDecod() && id==0 && emitudpdectxt && !fpsk_block)
        {
            smsg.remove("\n");//
            TRadioAndNetW->SendDecodTxt(list_in.at(0),snr_spot,sdt,freq_offset,smsg);
        }
        //if (!fpsk_block) qDebug()<<"Emit="<<smsg;
        //else qDebug()<<"BLOCK="<<smsg;
    }
}
void HvTxW::SetTxWatchdogParms(int f_no_time_txcount,int txwatchdog_time,int txwatchdog_coun)
{
    f_no_time_count_txwatchdog = f_no_time_txcount;
    s_txwatchdog_time = (unsigned int)txwatchdog_time*60;//x*60 to sec
    s_txwatchdog_coun = txwatchdog_coun;
    //qDebug()<<"SetTxWatchdogParms"<<f_no_time_count_txwatchdog<<s_txwatchdog_time<<s_txwatchdog_coun;
}
void HvTxW::ResetTxWatchdog(QString newmsg,bool,bool)// if tx msg is changed
{
    //TX watchdog reset TUNE no signal here
    //qDebug()<<"TX kkk"<<newmsg;
    if (s_PrevTxRpt!=newmsg)
    {
        s_PrevTxRpt = newmsg;
        s_last_txwatchdog_time = QDateTime::currentDateTimeUtc().toTime_t();
        s_last_txwatchdog_coun = 0;
        //qDebug()<<"TX watchdog reset"<<s_PrevTxRpt;
    }
}
bool HvTxW::TrySetQueuedCall(bool f)//2.59 true = on the flay if on TX reset, old=no flag
{
    bool res = false;
    if (!f_multi_answer_mod && TQueuedCall->haveQueuedCall())// RU Queued
    {
        RefreshBackupDB();
        s_last_call_for_BDB = "_";

        QStringList llqq = TQueuedCall->GetQueuedCallData();
        le_his_call->SetText(llqq.at(0));
        le_rst_tx->SetText(llqq.at(1),true);//my tx rpt
        le_rst_rx->SetText(le_rst_rx->format_rpt(llqq.at(2)),true);//his rpt my rx rpt
        s_his_contest_exch = llqq.at(3);
        s_his_contest_sn = llqq.at(4);
        LeHisLoc->SetText(llqq.at(5));
        double freq = llqq.at(6).toDouble();
        TQueuedCall->ClrQueuedCall(true);
        SetFreqTxW(freq);

        gen_msg();
        if (s_cont_type==3)//eu vhf cont
            SetTxRaportToRB(1,f);//2.59 true = on the flay if on TX reset, old=no flag
        else
            SetTxRaportToRB(2,f);//2.59 true = on the flay if on TX reset, old=no flag

        res = true;
    }
    return res;
}
void HvTxW::CountTx73_p(bool svlog)
{
    int c_cfm73 = 0;
    int max_c = 1+c_cfm73;
    if (s_mode==0 || s_mode==12) //for msk144 2-times=73
    {
        max_c = 1+2; //1+2;
        c_cfm73 = 1;
    }
    //qDebug()<<count_73_auto_seq;
    if (AutoSeqLab->GetAutoSeq() && f_auto_on && count_73_auto_seq > c_cfm73)// after shure rx 73 identif=2
    {
        if (count_73_auto_seq>=max_c)
        {
            count_73_auto_seq = 0;
            if (s_cont_type>1 && (s_mode==0 || s_mode==11 || s_mode==13 || s_mode==18 || allq65) && TrySetQueuedCall(false))
                return;// nill
            else
            {
                StopAuto();
                //if (s_mode==0 || s_mode==2 || s_mode==3 || s_mode==12) qrg modes
                if (le_qrg->GetQrgActive()==2) SetTxRaportToRB(6,false);//2.60= goto CQ QRG
                else SetTxRaportToRB(5,false);//1.60= goto CQ
                if (svlog)//2.59
                {
                    //qDebug()<<"NEW->> ASK= LOG UP --------- 73 RR73 ---------->";
                    one_addtolog_auto_seq = true;//importent
                    if (direct_log_qso) AddToLog_p(true);
                    else if (prompt_log_qso) AddToLog_p(false);//false direct save to log off
                }
                //stop 2.43 if (!f_multi_answer_mod && s_id_contest_mode>1 && (s_mode == 0 || s_mode == 11 || s_mode==13 || s_mode==18))//ft4
                bool farst = false;
                if (f_areset_qso && s_cont_type==0) farst = true;
                if (!f_multi_answer_mod && (farst || s_cont_type>1) && (s_mode==0 || s_mode==11 || s_mode==13 || s_mode==18 || allq65))//2.43
                    ResetQSO_p(false);//2.43
            }
        }
        else count_73_auto_seq++;
    }
}
void HvTxW::SetTxRxCountAutoSeq(bool f)
{
    f_tx_rx = f;
    if ((s_mode==11 || s_mode==13 || s_mode==18 || allq65) && f_multi_answer_mod) MultiAnswerMod->SetTxRxMsg(f);//eventual ft4

    TRadioAndNetW->SetTx(f);
    THvAstroDataW->SetTx(f);
    le_qrg->SetReadOnly(f);

    if (le_qrg->GetQrgActive()==2)//2.51
    {
        if (f) SetEnabledBtTx(false);
        else SetEnabledBtTx(true);
    }

    if (f && (s_mode==0 || s_mode==12))//2.59 start TX
    {
        int id_tx = -1;
        for (int i = 0; i<count_tx_widget; i++)
        {
            HvTxIn *TempHvTxIn = (HvTxIn*)V_l->itemAt(i)->widget();
            if (TempHvTxIn->rb_tx->isChecked())
            {
                id_tx = i;
                break;
            }
        }
        if (id_tx==1 || id_tx==2) SetLockTxRstMskASeq(true);
        /*if (s_cont_type==5) //Roundup type = 5 for the future
        {
        	if (id_tx==0 || id_tx==1 || id_tx==2) SetLockTxRstMskASeq(true);		
        }
        else
        {
        	if (id_tx==1 || id_tx==2)) SetLockTxRstMskASeq(true);		
        }*/
    }

    if (!f)	//only end tx
    {
        CountTx73_p(false); // v1.47 2.59=false

        //TX watchdog
        if (f_no_time_count_txwatchdog == 0)    //by 0=no 1=time 2=count;
            return;
        else if (f_no_time_count_txwatchdog == 1) //by 0=no 1=time 2=count;
        {
            unsigned int hh = QDateTime::currentDateTimeUtc().toTime_t();
            if (hh < s_last_txwatchdog_time + s_txwatchdog_time)//1min 1*60=60s
                return;
            s_last_txwatchdog_time = hh;
        }
        else if (f_no_time_count_txwatchdog == 2) //by 0=no 1=time 2=count;
        {
            s_last_txwatchdog_coun++;
            if (s_txwatchdog_coun > s_last_txwatchdog_coun)//1min 1*60=60s      1802=30:02  1800+2  2=seconds
                return;
            s_last_txwatchdog_coun = 0;
        }
        StopAuto();
        //END TX watchdog
    }
}
void HvTxW::SetLastTxRptAutoSeq(QString t)
{
    QString t0 = t+"xxx";
    if (t0.mid(0,3)=="CQ ") is_LastTxRptCq = true;
    else is_LastTxRptCq = false;
    MultiAnswerMod->SetIsLastTxRptCq(is_LastTxRptCq);
    TRadioAndNetW->SetTxMsg(t);
    if (!is_LastTxRptCq) s_dist_points = -1;//2.66 reset
    //qDebug()<<"CQ"<<is_LastTxRptCq<<t;
}
void HvTxW::SetTxRaportToRB(int identif,bool immediately)
{
    //if(s_b_identif==identif)// for many times decode from msk144
    //return;
    s_b_identif = identif;
    for (int i = 0; i<count_tx_widget; i++)
    {
        HvTxIn *TempHvTxIn = (HvTxIn*)V_l->itemAt(i)->widget();
        if (i!=identif)
        {
            TempHvTxIn->rb_tx->setChecked(false);
        }
        else
        {
            SetQSOProgressAll(identif,false);//2.51
            TempHvTxIn->rb_tx->setChecked(true);
            if ((s_mode==11 || s_mode==13 || s_mode==18 || allq65) && f_auto_on && immediately)//2.59 +ft44 f_tx_rx &&
                SetEmitMessage(TempHvTxIn->line_txt->text(),false,true,false);//iftximid
            else SetEmitMessage(TempHvTxIn->line_txt->text(),false,false,false);
            //qDebug()<<"msg="<<s_b_identif<<TempHvTxIn->line_txt->text();
        }
    }
}
void HvTxW::SetQrgActiveId(int i)//2.60
{
    //if (s_mode != 0 && s_mode != 12) return;
    int b_activ = -1;
    for (int i = 0; i<count_tx_widget; i++)
    {
        HvTxIn *TempHvTxIn = (HvTxIn*)V_l->itemAt(i)->widget();
        if (TempHvTxIn->rb_tx->isChecked())
        {
            b_activ = i;
            break;
        }
    }
    //qDebug()<<"SetQrgActiveId="<<i<<"b_activ="<<b_activ;
    if 		(i==2 && b_activ==5) SetTxRaportToRB(6,false);
    else if (i!=2 && b_activ==6) SetTxRaportToRB(5,false);
}
void HvTxW::SetDPLogQso(bool f1,bool f2)
{    
    direct_log_qso = f1; //qDebug()<<f1<<f2;
    prompt_log_qso = f2;
    one_addtolog_auto_seq = false; // reset one add to log
}
void HvTxW::AutoSeqLabPress()
{
    count_73_auto_seq = 0;// reset 73
}
void HvTxW::SetFreqTxW(double f)
{
    if ((s_mode==11 || s_mode==13 || s_mode==18 || allq65) && f_multi_answer_mod) return;
    // no change freq if    haveQueuedCall //eventual ft4 if (s_id_contest_mode>1 && (s_mode == 0 || s_mode == 11) && TQueuedCall->haveQueuedCall())
    if (s_cont_type>1 && (s_mode==0 || s_mode==11 || s_mode==13 || s_mode==18 || allq65) && TQueuedCall->haveQueuedCall()) return;//ft4        
    emit EmitFreqTxW(f);
}
void HvTxW::SetTxRstASeq(QString s)//2.59
{
    if (s_mode==0 || s_mode==12)
    {
        if (!is_locked_tx_rst_msk_aseq)
        {
            QString ss = le_rst_tx->format_rpt_tx(s);
            le_rst_tx->SetText(ss,true);
        }
    }
    else
    {
        QString ss = le_rst_tx->format_rpt_tx(s);
        le_rst_tx->SetText(ss,true);
    }
}
void HvTxW::SetUseASeqMaxDist(bool f)
{
    f_aseqmaxdist = f; //qDebug()<<"SetUseASeqMaxDist="<<f;
    MultiAnswerMod->SetUseASeqMaxDist(f);
}
bool HvTxW::isAddToLog(QString hisBaseCall_inmsg)//2.76.1
{
	bool f = false;
	if ((!f_add_to_log_started && !one_addtolog_auto_seq) || (!f_add_to_log_started && hisBaseCall_inmsg != s_last_bccall_tolog_excp)) f = true;
	return f;
}
void HvTxW::SetTextForAutoSeq(QStringList list_in)
{
    if ((s_mode==11 || s_mode==13 || s_mode==18 || allq65) && f_multi_answer_mod)//ft8 only  eventual ft4
    {
        //if (f_auto_on && AutoSeqLab->GetAutoSeq())
        if (f_auto_on) MultiAnswerMod->SetTextForAutoSeq(list_in);
        return;
    }
    //2.73 Dist Country
    if (!list_in.isEmpty() && (s_mode==0 || s_mode==11 || s_mode==12 || s_mode==13 || s_mode==18 || allq65) && list_in.count()>9 && f_auto_on)//FT8 ft4 have column 7 for freq msk144 have column 8 for freq
    {
        if (direct_log_qso || prompt_log_qso || AutoSeqLab->GetAutoSeq())
        {
            QString hisCall_inmsg = "";
            QString hisLoc_inmsg  = "";
            QString myCall_inmsg  = "";
            QString rpt_inmsg  = "";
            QString cont_r_inmsg  = ""; //msk144 ft8 contest mode
            QString rr73_inmsg  = ""; ////1.54=RR73 msk144 ft8 rr73
            QString text_msg;

            if (s_mode==0 || s_mode==12) text_msg = list_in.at(6);//msk144 and msk144ms
            else text_msg = list_in.at(4);//ft8 ft4
            
            //QString hisCallFromLe = le_his_call->getText();  hisCallFromLe,
            int row_queue = -2;// -1=ident find mam   -2=no find mam
            int row_now = -2;  // -1=ident find mam   -2=no find mam
            QString sn_inmsg = "";//v.2.0
            QString arrl_exch_imsg = "";//v.2.01            
            //bool tu0=false;
            //if (text_msg.endsWith(" TU")) tu0=true;
            MultiAnswerMod->DetectTextInMsg(text_msg,hisCall_inmsg,hisLoc_inmsg,myCall_inmsg,rpt_inmsg,
                                            cont_r_inmsg,rr73_inmsg,row_queue,row_now,sn_inmsg,arrl_exch_imsg);
            //qDebug()<<"hisCall="<<hisCall_inmsg<<"hisLoc="<<hisLoc_inmsg<<"myCall="<<myCall_inmsg<<
            //"rpt="<<rpt_inmsg<<"contest_mode="/*<<contest_mode_inmsg*/<<"rr73="<<rr73_inmsg<<text_msg;

            bool fc1 = true;
            bool fc2 = true;
            uint8_t noQSO = 0;
            if (!hisCall_inmsg.isEmpty()) MultiAnswerMod->isStandardCalls(list_macros.at(0),hisCall_inmsg,fc1,fc2,noQSO);
            if (f_two_no_sdtc && noQSO==100)//if (f_two_no_sdtc)
            {
                return;
            }
            if (f_cntest_only_sdtc)
            {
                if (!fc1 || !fc2)
                {
                    return;
                }
            }
            //if (s_man_adding && le_his_call->getText()=="") return;

            QString le_his_base_call = MultiAnswerMod->FindBaseFullCallRemAllSlash(le_his_call->getText());//1.73
            QString hisBaseCall_inmsg = "_NONE_";//1.73
            if (!hisCall_inmsg.isEmpty()) hisBaseCall_inmsg = MultiAnswerMod->FindBaseFullCallRemAllSlash(hisCall_inmsg);//1.70

            if (AutoSeqLab->GetAutoSeq())
            {
                if (f_aseqmaxdist && (le_his_call->getText()=="" || s_dist_points>-1))//2.66  && !TQueuedCall->haveQueuedCall() ???
                {	// (le_his_call->getText()=="" || s_dist_points>-1)=logic is, if no reset no start CQ-QSO
                    if (!myCall_inmsg.isEmpty() && !hisCall_inmsg.isEmpty() && is_LastTxRptCq)//&& !hisLoc_inmsg.isEmpty()
                    {
                        //int ds = MultiAnswerMod->CalcDistance(hisLoc_inmsg.mid(0,4),false).toInt();//false=kilometers
                        //int po=ds/500;  //ARRL Inter. Contest rules points, bad for VHF
                        //if (ds>500*po) po++;
                        //po++;
                        int po = -1;
                        if (hisLoc_inmsg.isEmpty()) po = 0; //<mycall> hiscall no loc
                        else po = MultiAnswerMod->CalcDistance(hisLoc_inmsg,false).toInt();//good for VHF false=kilometers
                        if (po > s_dist_points)
                        {
                            //qDebug()<<"Change="<<le_his_call->getText()<<s_dist_points<<"to"<<hisCall_inmsg<<po;
                            s_dist_points = po;
                            le_his_call->SetText("");
                        }
                    }
                }
                //no good idea if (!hisCall_inmsg.isEmpty() && !myCall_inmsg.isEmpty() && tis_LastTxRptCq)
                if (le_his_call->getText()=="" && !myCall_inmsg.isEmpty() && is_LastTxRptCq)// cq no call in his call
                {
                    le_his_call->SetText(hisCall_inmsg);
                    if (!hisLoc_inmsg.isEmpty()) LeHisLoc->SetText(hisLoc_inmsg);
                    gen_msg();
                    le_his_base_call = MultiAnswerMod->FindBaseFullCallRemAllSlash(le_his_call->getText());//1.70 refresh base call importent to be here
                    if (!arrl_exch_imsg.isEmpty()) s_his_contest_exch = arrl_exch_imsg;
                }

                if (le_his_base_call==hisBaseCall_inmsg && !myCall_inmsg.isEmpty())//msg=0 my corespondent and mycall
                {
                    if (hisCall_inmsg.contains("/"))//1.73 update if have slash during QSO
                    {
                        if (le_his_call->getText()!=hisCall_inmsg)
                        {
                            le_his_call->SetText(hisCall_inmsg);
                            gen_msg();// no need but for any case
                        }
                    }

                    QString my_tx_rpt;
                    if (s_mode==0)//msk144
                        my_tx_rpt = list_in.at(3);
                    if (s_mode==12)
                    {
                        if (list_in.at(4).isEmpty())// vavno moze i da e prazno
                            my_tx_rpt = "26";
                        else
                            my_tx_rpt = list_in.at(4);
                    }
                    if (s_mode==11 || s_mode==13 || s_mode==18 || allq65)//ft8 ft4
                    {
                        my_tx_rpt = list_in.at(1);
                        double his_freq = (double)list_in.at(9).toInt();//2.73 Dist Country
                        emit EmitFreqTxW(his_freq);
                    }
                    my_tx_rpt = MultiAnswerMod->TryFormatRPTtoRST(my_tx_rpt);//2.59

                    //if (LastTxRpt.mid(0,3)=="CQ ")
                    if (rpt_inmsg.isEmpty() && hisLoc_inmsg.isEmpty() && cont_r_inmsg.isEmpty() && rr73_inmsg.isEmpty() && arrl_exch_imsg.isEmpty())
                    {//msg=0 only calls exeption if have slashs    for ARRL Field Day && arrl_exch_imsg.isEmpty()
                        //qDebug()<<"Nor msg=0 only calls===================";
                        //QString ss = le_rst_tx->format_rpt_tx(my_tx_rpt);
                        //le_rst_tx->SetText(ss,true);
                        SetTxRstASeq(my_tx_rpt);//2,59
                        gen_msg();
                        SetTxRaportToRB(1,true);
                        count_73_auto_seq = 0;//inportent
                        one_addtolog_auto_seq = false;//inportent
                    }
                    //else if (rpt_inmsg.isEmpty() && !hisLoc_inmsg.isEmpty())
                    //else if (rpt_inmsg.isEmpty() && (!hisLoc_inmsg.isEmpty() || s_id_contest_mode==4))//2.01
                    else if (rpt_inmsg.isEmpty() && (!hisLoc_inmsg.isEmpty() || !arrl_exch_imsg.isEmpty()))//2.01  /*&& rr73_inmsg!="RR73"*/
                    {//msg=0 KN23
                        //QString ss = le_rst_tx->format_rpt_tx(my_tx_rpt);
                        //le_rst_tx->SetText(ss,true);
                        SetTxRstASeq(my_tx_rpt);//2,59
                        gen_msg();

                        if (!cont_r_inmsg.isEmpty()) SetTxRaportToRB(3,true);//msk144 ft8 contest mode
                        else
                        {
                            if (/*s_mode != 12 &&*/ (s_cont_type==2 || s_cont_type==4)) //msk144 ft8 contest mode
                                SetTxRaportToRB(2,true);
                            else
                                SetTxRaportToRB(1,true);
                        }
                        //SetTxRaportToRB(1);
                        //qDebug()<<"msg0="<<s_cont_type<<cont_r_inmsg<<sn_inmsg<<arrl_exch_imsg<<hisLoc_inmsg;

                        count_73_auto_seq = 0;//importent
                        one_addtolog_auto_seq = false;//importent
                        //2.13 update NA contest no standard call
                        if (!hisLoc_inmsg.isEmpty()) LeHisLoc->SetText(hisLoc_inmsg);
                        //2.13 END update NA contest no standard call
                        if (!arrl_exch_imsg.isEmpty()) s_his_contest_exch = arrl_exch_imsg;
                    }
                    else if (!rpt_inmsg.isEmpty() && rpt_inmsg.at(0)!='R' && rpt_inmsg.at(rpt_inmsg.count()-1).isDigit() && rpt_inmsg!="73")
                    {//msg=1 +00
                        //QString ss = le_rst_tx->format_rpt_tx(my_tx_rpt);
                        //le_rst_tx->SetText(ss,true); //qDebug()<<"msg=1 +00";
                        SetTxRstASeq(my_tx_rpt);//2,59
                        gen_msg();
                        le_rst_rx->SetText(le_rst_rx->format_rpt(rpt_inmsg),true);

                        if (!cont_r_inmsg.isEmpty()) SetTxRaportToRB(3,true);
                        else SetTxRaportToRB(2,true);
                        //SetTxRaportToRB(2);

                        count_73_auto_seq = 0;//importent
                        one_addtolog_auto_seq = false;//importent
                        if (!hisLoc_inmsg.isEmpty()) LeHisLoc->SetText(hisLoc_inmsg);//2.01 update EU C LOC
                        if (!sn_inmsg.isEmpty()) s_his_contest_sn = sn_inmsg;
                        if (!arrl_exch_imsg.isEmpty()) s_his_contest_exch = arrl_exch_imsg;
                        //qDebug()<<"msg1="<<sn_inmsg<<arrl_exch_imsg<<hisLoc_inmsg;
                    }
                    else if (!rpt_inmsg.isEmpty() && rpt_inmsg.at(0)=='R' && rpt_inmsg.at(rpt_inmsg.count()-1).isDigit())
                    {//msg=2 R+00
                        rpt_inmsg.remove("R");
                        le_rst_rx->SetText(le_rst_rx->format_rpt(rpt_inmsg),true);
                        SetTxRaportToRB(3,true);
                        count_73_auto_seq = 0;//importent
                        //one_addtolog_auto_seq = false; //2.31 stopped

                        if (!hisLoc_inmsg.isEmpty()) LeHisLoc->SetText(hisLoc_inmsg);//2.01 update EU C LOC                            
                        if (!sn_inmsg.isEmpty()) s_his_contest_sn = sn_inmsg;
                        if (!arrl_exch_imsg.isEmpty()) s_his_contest_exch = arrl_exch_imsg;

                        //2.31 importent
                        if (isAddToLog(hisBaseCall_inmsg))//2.76.1
                        {
                            one_addtolog_auto_seq = true;//importent
                            if (s_cont_type>1 && s_mode != 12) AddToLog_p(true);//true direct save to log on
                            else if (direct_log_qso) AddToLog_p(true);
                            else if (prompt_log_qso) AddToLog_p(false);//false direct save to log off
                        }
                        //qDebug()<<"msg2="<<sn_inmsg<<arrl_exch_imsg<<hisLoc_inmsg;
                    }
                    else if (!rpt_inmsg.isEmpty() && rpt_inmsg.at(0)=='R' && !rpt_inmsg.at(rpt_inmsg.count()-1).isDigit() &&
                             rpt_inmsg.at(rpt_inmsg.count()-1)=='R')
                    {//msg=3 RRR
                        SetTxRaportToRB(4,true);
                        count_73_auto_seq = 1;//importent no 73 only RRR no stop TX from SetTxCountAutoSeq()
                        if (isAddToLog(hisBaseCall_inmsg))//2.76.1       
                        {
                            //qDebug()<<"msg3 000= LOG UP --------- RRR R ---------->";
                            one_addtolog_auto_seq = true;//importent
                            if (s_cont_type>1 && s_mode != 12) AddToLog_p(true);//true direct save to log on
                            else if (direct_log_qso) AddToLog_p(true);
                            else if (prompt_log_qso) AddToLog_p(false);//false direct save to log off
                        }
                    }
                    else if ((!rpt_inmsg.isEmpty() && rpt_inmsg=="73") || rr73_inmsg=="RR73")//1.54=RR73
                    {//msg=4 73
                        SetTxRaportToRB(4,true);//2.02 first this

                        if (!f_cfm73) count_73_auto_seq = 1;//2.31

                        if (s_cont_type>1 && s_mode != 12)//2.02  2.59 or->if (((!f_cfm73 && f_areset_qso) || s_cont_type>1) && s_mode != 12)
                        {
                            if (isAddToLog(hisBaseCall_inmsg))//2.76.1        
                            {
                                //qDebug()<<"msg4 DIRECT QueuedCall= LOG UP --------- 73 RR73 ---------->";
                                one_addtolog_auto_seq = true;//importent
                                AddToLog_p(true);//true direct save to log on
                            }
                        }

                        if (count_73_auto_seq<2)
                        {
                            if (count_73_auto_seq==0) count_73_auto_seq = 2;// min 2 73
                            else
                            {
                                count_73_auto_seq = 2;
                                bool svlog = false;//2.59 exeption from -> (!f_cfm73 && f_areset_qso) effect no save QSO
                                if (isAddToLog(hisBaseCall_inmsg)) svlog = true;//2.76.1        
                                CountTx73_p(svlog);
                            }
                        }
                        if (isAddToLog(hisBaseCall_inmsg))//2.76.1        
                        {
                            //qDebug()<<"msg4 ASK= LOG UP --------- 73 RR73 ---------->";
                            one_addtolog_auto_seq = true;//importent
                            if (direct_log_qso) AddToLog_p(true);
                            else if (prompt_log_qso) AddToLog_p(false);//false direct save to log off
                        }
                    }
                    else // any other msg reset rrr and 73
                    {
                        //qDebug()<<"msg4="<<count_73_auto_seq<<text_msg;
                        count_73_auto_seq = 0;//importent
                        one_addtolog_auto_seq = false;//importent
                    }
                    //my TX RR73 2.59 tested
                    for (int i = 0; i<count_tx_widget; i++)
                    {
                        HvTxIn *TempHvTxIn = (HvTxIn*)V_l->itemAt(i)->widget();
                        if (TempHvTxIn->rb_tx->isChecked())
                        {
                            //if (TempHvTxIn->line_txt->text().endsWith(" RR73") || TempHvTxIn->line_txt->text().endsWith(" RRR"))//2.31
                            //QString tss = TempHvTxIn->line_txt->text();
                            if (TempHvTxIn->line_txt->text().endsWith(" RR73"))
                            {
                                /*if (f_cfm73 && TempHvTxIn->line_txt->text().endsWith(" RRR")) count_73_auto_seq = 0;
                                else count_73_auto_seq = 1;*/
                                count_73_auto_seq = 1;//1 <- 2.59 tested ok, go from R+12 = 0
                                //qDebug()<<"11=="<<count_73_auto_seq;
                                if (s_cont_id==15)
                                {
                                    count_73_auto_seq = 2;
                                    bool svlog = false;//2.59 exeption from -> (!f_cfm73 && f_areset_qso) effect no save QSO
                                    if (isAddToLog(hisBaseCall_inmsg)) svlog = true;//2.76.1        
                                    CountTx73_p(svlog);
                                }
                                //else count_73_auto_seq = 1;
                                //2.59 for NA Contest msg-> R KN23
                                if (isAddToLog(hisBaseCall_inmsg))//2.76.1        
                                {
                                    one_addtolog_auto_seq = true;//importent
                                    if (s_cont_type>1 && s_mode != 12) AddToLog_p(true);//true direct save to log on
                                    else if (direct_log_qso) AddToLog_p(true);
                                    else if (prompt_log_qso) AddToLog_p(false);//false direct save to log off
                                }
                            }
                            if (s_cont_id==15 && TempHvTxIn->line_txt->text().indexOf(" R ")>-1)
                                //if (s_cont_id==15 && tss.mid(tss.count()-7,3)==" R ")//" R KN23"
                            {
                                //qDebug()<<"22=="<<count_73_auto_seq;
                                count_73_auto_seq = 1;
                                if (isAddToLog(hisBaseCall_inmsg))//2.76.1        
                                {
                                    one_addtolog_auto_seq = true;//importent
                                    if (s_cont_type>1 && s_mode != 12) AddToLog_p(true);//true direct save to log on
                                    else if (direct_log_qso) AddToLog_p(true);
                                    else if (prompt_log_qso) AddToLog_p(false);//false direct save to log off
                                }
                            }
                        }
                    }
                }
            }
            else //"auto sq off" but have "prompt for log qso"
            {
                if (le_his_base_call==hisBaseCall_inmsg && !myCall_inmsg.isEmpty())
                {
                    if ((!rpt_inmsg.isEmpty() && ((rpt_inmsg.at(0)=='R' && !rpt_inmsg.at(rpt_inmsg.count()-1).isDigit() &&
                                                   rpt_inmsg.at(rpt_inmsg.count()-1)=='R') || rpt_inmsg=="73")) ||
                            rr73_inmsg=="RR73")//1.54=RR73 msg=3-4 RRR 73 RR73
                    {
                        if (isAddToLog(hisBaseCall_inmsg))//2.76.1        
                        {
                            one_addtolog_auto_seq = true;//importent
                            if (direct_log_qso) AddToLog_p(true);
                            else AddToLog_p(false);//false direct save to log off
                            /*if (prompt_log_qso) AddToLog_p(false);//false direct save to log off
                            else if(s_id_contest_mode>1) AddToLog_p(true);//true direct save to log on*/
                        }
                    }
                    else one_addtolog_auto_seq = false;//importent // any other msg reset rrr and 73
                }
            }
        }
    }
}
void HvTxW::ExternalFindLocFromDB(QString call)
{
    QString loc = TMsDb->CheckBD(call); //emit EmitLocFromDB(loc);
    TRadioAndNetW->SetLocFromDB(loc);
}


