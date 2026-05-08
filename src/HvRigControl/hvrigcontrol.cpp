/* MSHV HvRigControl
 * Copyright 2015 Hrisimir Hristov, LZ2HV
 * May be used under the terms of the GNU General Public License (GPL)
 */
#define _BANDS_H_
#include "hvrigcontrol.h"
#include <unistd.h>   // uslepp Linux and windows
#include <math.h> /* round, floor, ceil, trunc */
#include "qexsp_1_2rc/qextserialenumerator.h"
#include "qexsp_1_2rc/qextserialport.h"
#if defined _WIN32_
#include <windows.h>
#include <vector>
struct ComEnumSort
{
    bool operator()(const QString &s1, const QString &s2) const
    {
        QString st1 = s1.left(s1.indexOf('M'));
        int it1 = s1.rightRef(s1.size() - s1.indexOf('M') - 1).toInt();
        QString st2 = s2.left(s2.indexOf('M'));
        int it2 = s2.rightRef(s2.size() - s2.indexOf('M') - 1).toInt();
        if (st1 < st2)
            return true;
        if (st1 > st2)
            return false;
        if (it1 < it2)
            return true;
        return false;
    }
};
#endif

//#include <QtGui>

struct netSP
{
    QString serv;
    QString port;
};
//#include "HvRigCat/network/network_def.h"
//#define COU_NET_PS NETWORK_COUNT//5 //=NETWORK_COUNT
#include "HvRigCat/network/network_def.h"
//#define COU_NET_PS NETWORK_COUNT
static netSP netServPort[NETWORK_COUNT];
struct netTciCS
{
	QString srate;
	int txbuff;
    uint8_t channels;
    QString samples;
    uint8_t type;
};
static netTciCS netTciChSmp[2];

HvRigControl::HvRigControl( QWidget *parent )
        : QDialog(parent)
{
    //this->setWindowTitle(APP_NAME + (QString)" Rig Control");
    //this->setWindowTitle("Rig Control");
    this->setWindowTitle(tr("Interface Control"));
    setWindowFlags(windowFlags() ^ Qt::WindowContextHelpButtonHint);
    //QGridLayout *l = new QGridLayout( this, 1, 1, 0, 0, "boardLayout");
    //setStyleSheet("border: 1px solid red");
    //setMinimumSize(612,548);//+125%
    //setMinimumSize(640,556);//2.74 stop->2.76.2

    end_rs0 = 0;
    rig_cat_active_and_read = false;//2.30
    only_one = false;
    s_have_read_data_rts_on = 0;

    THvRigCat = new HvRigCat();

    //cb_off_set_frq_to_rig = new QCheckBox(tr("Tuning Default RIG Freq Only By Pressing Button F"));
    cb_off_set_frq_to_rig = new QCheckBox(tr("TUNE RIG Freq/Mode Only By Pressing Button F"));
    cb_off_set_frq_to_rig->setChecked(false);
    //cb_mod_set_frq_to_rig = new QCheckBox(tr("Tuning Default RIG Freq From Mode"));
    cb_mod_set_frq_to_rig = new QCheckBox(tr("TUNE RIG Freq/Mode From Mode"));
    cb_mod_set_frq_to_rig->setChecked(true);//2.53 default on

    cb_rig_mod = new QComboBox();
    QStringList lst_baud;
    lst_baud<<"RIG Mode: None"<<"RIG Mode: USB"<<"RIG Mode: DIGU";
    cb_rig_mod->addItems(lst_baud);
    cb_rig_mod->setCurrentIndex(0);

    QHBoxLayout *H_frs = new QHBoxLayout();
    H_frs->setContentsMargins(0,0,0,0);
    H_frs->setSpacing(5);//2.74 =10
    H_frs->addWidget(cb_off_set_frq_to_rig);
    H_frs->addWidget(cb_mod_set_frq_to_rig);
    H_frs->addWidget(cb_rig_mod);
    H_frs->setAlignment(Qt::AlignCenter);

    QGroupBox *gb_p1 = new QGroupBox(tr("Port")+" 1:");

    cb_port = new QComboBox();
    cb_port->addItem("None");
    QLabel *l_port = new QLabel(tr("Port")+": ");
    QHBoxLayout *H_port = new QHBoxLayout();
    H_port->setContentsMargins(4,4,4,4);
    H_port->addWidget(l_port);
    H_port->setAlignment(l_port,Qt::AlignRight);
    H_port->addWidget(cb_port);

    cb_baud = new QComboBox();
    lst_baud.clear();
    lst_baud << "300" << "600" << "1200" << "2400" << "4800" << "9600" << "19200" << "38400" << "57600" << "115200";
    /*#if defined(Q_OS_UNIX) || defined(qdoc)
        BAUD50 = 50,                //POSIX ONLY
        BAUD75 = 75,                //POSIX ONLY
        BAUD134 = 134,              //POSIX ONLY
        BAUD150 = 150,              //POSIX ONLY
        BAUD200 = 200,              //POSIX ONLY
        BAUD1800 = 1800,            //POSIX ONLY
    #  if defined(B76800) || defined(qdoc)
        BAUD76800 = 76800,          //POSIX ONLY
    #  endif
    #  if (defined(B230400) && defined(B4000000)) || defined(qdoc)
        BAUD230400 = 230400,        //POSIX ONLY
        BAUD460800 = 460800,        //POSIX ONLY
        BAUD500000 = 500000,        //POSIX ONLY
        BAUD576000 = 576000,        //POSIX ONLY
        BAUD921600 = 921600,        //POSIX ONLY
        BAUD1000000 = 1000000,      //POSIX ONLY
        BAUD1152000 = 1152000,      //POSIX ONLY
        BAUD1500000 = 1500000,      //POSIX ONLY
        BAUD2000000 = 2000000,      //POSIX ONLY
        BAUD2500000 = 2500000,      //POSIX ONLY
        BAUD3000000 = 3000000,      //POSIX ONLY
        BAUD3500000 = 3500000,      //POSIX ONLY
        BAUD4000000 = 4000000,      //POSIX ONLY
    #  endif
    #endif //Q_OS_UNIX
    #if defined(Q_OS_WIN) || defined(qdoc)
        BAUD14400 = 14400,          //WINDOWS ONLY
        BAUD56000 = 56000,          //WINDOWS ONLY
        BAUD128000 = 128000,        //WINDOWS ONLY
        BAUD256000 = 256000,        //WINDOWS ONLY
    #endif  //Q_OS_WIN
        BAUD110 = 110,
        BAUD300 = 300,
        BAUD600 = 600,
        BAUD1200 = 1200,
        BAUD2400 = 2400,
        BAUD4800 = 4800,
        BAUD9600 = 9600,
        BAUD19200 = 19200,
        BAUD38400 = 38400,
        BAUD57600 = 57600,
        BAUD115200 = 115200*/
    /*#if defined(Q_OS_UNIX) || defined(qdoc)
        lst_baud.append("50");//POSIX ONLY
        lst_baud.append("75");//POSIX ONLY
    #endif
        lst_baud.append("110");//ALL
    #if defined(Q_OS_UNIX) || defined(qdoc)
        lst_baud.append("134");//POSIX ONLY
        lst_baud.append("150");//POSIX ONLY
        lst_baud.append("200");//POSIX ONLY
    #endif
        lst_baud.append("300");//ALL
        lst_baud.append("600");//ALL
        lst_baud.append("1200");//ALL
    #if defined(Q_OS_UNIX) || defined(qdoc)
        lst_baud.append("1800");
    #endif
        lst_baud.append("2400");//ALL
        lst_baud.append("4800");//ALL
        lst_baud.append("9600");//ALL
    #if defined(Q_OS_WIN) || defined(qdoc)
        lst_baud.append("14400");//WINDOWS ONLY
    #endif
        lst_baud.append("19200");//ALL
        lst_baud.append("38400");//ALL
    #if defined(Q_OS_WIN) || defined(qdoc)
        lst_baud.append("56000");//WINDOWS ONLY
    #endif
        lst_baud.append("57600");//ALL
    #if defined(Q_OS_UNIX) || defined(qdoc)
    #  if defined(B76800) || defined(qdoc)
        lst_baud.append("76800");//POSIX ONLY
    #  endif
    #endif
        lst_baud.append("115200");
    #if defined(Q_OS_WIN) || defined(qdoc)
        lst_baud.append("128000");//WINDOWS ONLY
    #endif
    #if defined(Q_OS_UNIX) || defined(qdoc)
    #  if (defined(B230400) && defined(B4000000)) || defined(qdoc)
        lst_baud.append("230400");//POSIX ONLY
    #  endif
    #endif
    #if defined(Q_OS_WIN) || defined(qdoc)
        lst_baud.append("256000");//WINDOWS ONLY
    #endif
    #if defined(Q_OS_UNIX) || defined(qdoc)
    #  if (defined(B230400) && defined(B4000000)) || defined(qdoc)
        lst_baud.append("460800"); //POSIX ONLY
        lst_baud.append("500000"); //POSIX ONLY
        lst_baud.append("576000"); //POSIX ONLY
        lst_baud.append("921600"); //POSIX ONLY
        lst_baud.append("1000000");//POSIX ONLY
        lst_baud.append("1152000");//POSIX ONLY
        lst_baud.append("1500000");//POSIX ONLY
        lst_baud.append("2000000");//POSIX ONLY
        lst_baud.append("2500000");//POSIX ONLY
        lst_baud.append("3000000");//POSIX ONLY
        lst_baud.append("3500000");//POSIX ONLY
        lst_baud.append("4000000");//POSIX ONLY
    #  endif
    #endif
    */
    cb_baud->addItems(lst_baud);
    //int i96 = cb_baud->findText("9600",Qt::MatchCaseSensitive);
    //if (i96 >= 0) cb_baud->setCurrentIndex(i96);
    cb_baud->setCurrentIndex(5);
    QLabel *l_baud = new QLabel(tr("Baud Rate")+": ");
    QHBoxLayout *H_baud = new QHBoxLayout();
    H_baud->setContentsMargins(4,4,4,4);
    H_baud->addWidget(l_baud);
    H_baud->setAlignment(l_baud,Qt::AlignRight);
    H_baud->addWidget(cb_baud);

    ////// net //////////
    QVBoxLayout *V_port1 = new QVBoxLayout();
    V_port1->setContentsMargins(0,0,0,0);
    V_port1->addLayout(H_port);
    V_port1->addLayout(H_baud);

    // Network Defaults
    s_rig_name = "None";
    s_net_model_id = -100;
    s_rig_full_name = "None";//2.76.1
    block_save_net = false;
    net_active = false;
    netServPort[0].serv="127.0.0.1";
    netServPort[0].port="7809";			//HRD
    netServPort[1].serv="127.0.0.1";
    netServPort[1].port="12345";		//FlRig
    netServPort[2].serv="127.0.0.1";
    netServPort[2].port="52002";		//DXCommander
    netServPort[3].serv="127.0.0.1";
    netServPort[3].port="50001";		//TCI Client rx1
    netServPort[4].serv="127.0.0.1";
    netServPort[4].port="50001";		//TCI Client rx2
    netServPort[5].serv="127.0.0.1";
    netServPort[5].port="4532";		    //Hamlib NET rigctl
    netServPort[6].serv="127.0.0.1";
    netServPort[6].port="4532";		    //FlexRadio 6xxx
    netServPort[7].serv="RIG IP Addr";
    netServPort[7].port="4992";		    //FlexRadio SmartSDR Slice A
    netServPort[8].serv="RIG IP Addr";
    netServPort[8].port="4992";		    //FlexRadio SmartSDR Slice
    netServPort[9].serv="RIG IP Addr";
    netServPort[9].port="4992";		    //FlexRadio SmartSDR Slice
    netServPort[10].serv="RIG IP Addr";
    netServPort[10].port="4992";		//FlexRadio SmartSDR Slice
    netServPort[11].serv="RIG IP Addr";
    netServPort[11].port="4992";		//FlexRadio SmartSDR Slice
    netServPort[12].serv="RIG IP Addr";
    netServPort[12].port="4992";		//FlexRadio SmartSDR Slice
    netServPort[13].serv="RIG IP Addr";
    netServPort[13].port="4992";		//FlexRadio SmartSDR Slice
    netServPort[14].serv="RIG IP Addr";
    netServPort[14].port="4992";		//FlexRadio SmartSDR Slice H
    //netServPort[15].serv="XXXX";
    //netServPort[15].port="9999";		//XXXX

    TCPServer = new QLineEdit();
    TCPServer->setText(netServPort[0].serv);
    TCPPort = new QLineEdit();
    TCPPort->setText(netServPort[0].port);//HRD=7809, FlRig=12345, DXCommander=52002
    TCPPort->setInputMask("9999999999");
    connect(TCPServer, SIGNAL(textChanged(QString)), this, SLOT(TCPSPChanged(QString)));
    connect(TCPPort, SIGNAL(textChanged(QString)), this, SLOT(TCPSPChanged(QString)));

    l_con_info = new QLabel("<font color='red'>"+tr("Disconnected")+"</font>");
    QLabel *l_server = new QLabel(tr("Server")+":");
    QLabel *l_tcpp = new QLabel(tr("Port")+":");
    pb_re_conect = new QPushButton(tr("Connect"));

    l_tcich = new QLabel("TCI "+tr("Channels")+":");
    tci_channels = new QComboBox();
    QStringList lst_tci;
    lst_tci <<"1"<<"2";
    tci_channels->addItems(lst_tci);
    tci_channels->setCurrentIndex(1);
    l_tcisamp = new QLabel(tr("Samples")+":");
    l_tcisamp->setContentsMargins(4,0,0,0);
    tci_samples = new QComboBox();
    lst_tci.clear();
    lst_tci <<"512"<<"1024"<<"1536"<<"2048";
    tci_samples->addItems(lst_tci);
    tci_samples->setCurrentIndex(3);
    l_tcistype = new QLabel(tr("Type")+":");
    l_tcistype->setContentsMargins(4,0,0,0);
    tci_tcistype = new QComboBox();
    lst_tci.clear();
    lst_tci <<"int16"<<"int24"<<"int32"<<"float32";
    tci_tcistype->addItems(lst_tci);
    tci_tcistype->setCurrentIndex(3);
    
    l_tcisrate = new QLabel(tr("Audio Sample Rate")+":");
    //l_tcisrate->setContentsMargins(4,0,0,0);
    tci_rample_rate = new QComboBox();
    lst_tci.clear();
    lst_tci <<"12000"<<"24000"<<"48000";
    tci_rample_rate->addItems(lst_tci);
    tci_rample_rate->setCurrentIndex(2);
    tci_tx_buff = new QSpinBox();
    tci_tx_buff->setRange(50,400);
    tci_tx_buff->setSingleStep(50);
    tci_tx_buff->setPrefix("Tx Buffer def=50  ");
    tci_tx_buff->setSuffix("  ms");
    tci_tx_buff->setValue(50);  
    tci_tx_buff->findChild<QLineEdit*>()->setReadOnly(true); 

    QHBoxLayout *H_tci0 = new QHBoxLayout();
    H_tci0->setContentsMargins(0,0,0,0);
    H_tci0->setSpacing(2);
    H_tci0->addWidget(l_tcich);
    H_tci0->addWidget(tci_channels);
    H_tci0->addWidget(l_tcisamp);
    H_tci0->addWidget(tci_samples);
    H_tci0->addWidget(l_tcistype);
    H_tci0->addWidget(tci_tcistype);
    H_tci0->setAlignment(Qt::AlignLeft);
    
    QHBoxLayout *H_tci1 = new QHBoxLayout();
    H_tci1->setContentsMargins(0,0,0,0);
    H_tci1->setSpacing(5);
    H_tci1->addWidget(l_tcisrate);
    H_tci1->addWidget(tci_rample_rate);
    H_tci1->addWidget(tci_tx_buff);
    //H_tci1->addWidget(l_tcisamp);
    //H_tci1->addWidget(tci_samples);
    H_tci1->setAlignment(Qt::AlignLeft);  
    
    l_tcisrate->setHidden(true);
    tci_rample_rate->setHidden(true);
    tci_tx_buff->setHidden(true);
    l_tcich->setHidden(true);
    tci_channels->setHidden(true);
    l_tcisamp->setHidden(true);
    tci_samples->setHidden(true);
    l_tcistype->setHidden(true);
    tci_tcistype->setHidden(true);
    netTciChSmp[0].srate = "48000";
    netTciChSmp[0].txbuff = 50;
    netTciChSmp[0].channels = 1;
    netTciChSmp[0].samples  = "2048";
    netTciChSmp[0].type  = 3;
    netTciChSmp[1].srate = "48000";
    netTciChSmp[1].txbuff = 50;
    netTciChSmp[1].channels = 1;
    netTciChSmp[1].samples  = "2048";
    netTciChSmp[1].type  = 3;
    connect(tci_rample_rate, SIGNAL(currentIndexChanged(QString)), this, SLOT(TCPSPChanged(QString)));
    connect(tci_tx_buff, SIGNAL(valueChanged(QString)), this, SLOT(TCPSPChanged(QString)));    
    connect(tci_channels, SIGNAL(currentIndexChanged(QString)), this, SLOT(TCPSPChanged(QString)));
    connect(tci_samples, SIGNAL(currentIndexChanged(QString)), this, SLOT(TCPSPChanged(QString)));
    connect(tci_tcistype, SIGNAL(currentIndexChanged(QString)), this, SLOT(TCPSPChanged(QString)));

    QHBoxLayout *H_addr_port_bt = new QHBoxLayout();
    H_addr_port_bt->setContentsMargins (0,0,0,0);
    H_addr_port_bt->setSpacing(4);
    H_addr_port_bt->addWidget(l_server);
    H_addr_port_bt->setAlignment(l_server,Qt::AlignRight);
    H_addr_port_bt->addWidget(TCPServer);
    H_addr_port_bt->setAlignment(TCPServer,Qt::AlignLeft);
    H_addr_port_bt->addWidget(l_tcpp);
    H_addr_port_bt->setAlignment(l_tcpp,Qt::AlignRight);
    H_addr_port_bt->addWidget(TCPPort);
    H_addr_port_bt->setAlignment(TCPPort,Qt::AlignLeft);
    H_addr_port_bt->addWidget(pb_re_conect);
    H_addr_port_bt->setAlignment(pb_re_conect,Qt::AlignLeft);
    H_addr_port_bt->setAlignment(Qt::AlignLeft);

    QVBoxLayout *V_net = new QVBoxLayout();
    V_net->setContentsMargins(10,0,4,4);
    V_net->addWidget(l_con_info);
    V_net->addLayout(H_addr_port_bt);
    V_net->addLayout(H_tci1);
    V_net->addLayout(H_tci0);
    gb_net = new QGroupBox(tr("Network")+":");
    gb_net->setLayout(V_net);
    gb_net->setEnabled(false);
    connect(pb_re_conect, SIGNAL(clicked(bool)), this, SLOT(SetNetConnect()));
    connect(this, SIGNAL(EmitNetSP(QString)), THvRigCat, SIGNAL(EmitNetSP(QString)));
    connect(THvRigCat, SIGNAL(EmitNetConnInfo(QString,bool,bool)), this, SLOT(SetNetConnInfo(QString,bool,bool)));

    QHBoxLayout *H_net_port = new QHBoxLayout();
    H_net_port->setContentsMargins(8,0,0,0);

    //H_net_port->addLayout(V_net);
    H_net_port->addWidget(gb_net);
    H_net_port->addLayout(V_port1);
    ////// end net //////////

    QGroupBox *gb_method = new QGroupBox(tr("PTT Method")+":");//tr("Ptt Method")//QFrame
    //gb_method->setFrameStyle(QFrame::Panel | QFrame::Sunken);//

    rb_rts = new QRadioButton(tr("PTT Via RTS"));
    rb_rts->setChecked(true);
    rb_dtr = new QRadioButton(tr("PTT Via DTR"));
    rb_cat = new QRadioButton(tr("PTT Via CAT COMMAND"));
    rb_ptt_off = new QRadioButton(tr("PTT OFF"));
    //cb_read_data_kenwood_rts_on = new QCheckBox(tr(" RTS"));
    //cb_read_data_kenwood_dtr_on = new QCheckBox(tr(" DTR"));
    cb_read_data_kenwood_rts_on = new QCheckBox(tr("Enable Read RTS ON"));
    cb_read_data_kenwood_dtr_on = new QCheckBox(tr("Enable Read DTR ON"));
    cb_read_data_kenwood_rts_on->setToolTip("Handshake Via RTS");//2.73
    cb_read_data_kenwood_dtr_on->setToolTip("Handshake Via DTR");//2.73
    cb_read_data_kenwood_rts_on->setEnabled(false);
    cb_read_data_kenwood_dtr_on->setEnabled(false);
    QVBoxLayout *V_via0 = new QVBoxLayout();
    V_via0->setContentsMargins(0,0,0,0);
    V_via0->setSpacing(0);
    V_via0->addWidget(cb_read_data_kenwood_rts_on);
    V_via0->addWidget(cb_read_data_kenwood_dtr_on);

    QHBoxLayout *H_via = new QHBoxLayout();
    H_via->setContentsMargins(0,1,1,1);
    H_via->setSpacing(5);
    H_via->addWidget(rb_ptt_off);
    H_via->addWidget(rb_rts);
    H_via->addWidget(rb_dtr);
    H_via->addWidget(rb_cat);
    H_via->addLayout(V_via0);
    //H_via->addWidget(cb_read_data_kenwood_rts_on);
    //H_via->addWidget(cb_read_data_kenwood_dtr_on);
    H_via->setAlignment(Qt::AlignCenter);
    gb_method->setLayout(H_via);

    /*QVBoxLayout *V_via = new QVBoxLayout();
    V_via->setContentsMargins ( 0, 0, 0, 0);
    V_via->setSpacing(0);
    V_via->addLayout(H_via);
    V_via->addWidget(cb_read_data_kenwood_rts_on);
    V_via->setAlignment(cb_read_data_kenwood_rts_on,Qt::AlignCenter);
    gb_method->setLayout(V_via);*/

    pb_test_port = new QPushButton(" "+tr("START PTT TEST    NO PORT SELECTED")+" ");
    connect(pb_test_port, SIGNAL(clicked(bool)), this, SLOT(TestPtt()));
    pb_test_port->setEnabled(false);
    f_test_port = false;

    /////////////////////////////////////////////////////////////////
    //g_block_rig_st_tx = false;//2.50 block for specific rig
    f_block_display_static_tx = false;
    f_static_tx = false;
    mp_v_disp_tx_frq = 1.0;
    v_disp_tx_frq = 0;
    last_v_disp_tx_frq = 0;
    curr_rig_tx_frq = 0;
    last_curr_rig_tx_frq = 0;
    corr_app_ind_tx_frq = 0;
    corr_ms_static_tx = 0;
    max_try_off_static_tx = 0;
    f_static_tx_active = false;
    s_mode = 2;//default
    all_static_tx_modes = false;//2.64 default
    //gb_static_tx = new QGroupBox("Static TX  Audio Frequency FT8");
    //rb_static_tx = new QRadioButton("Use Static TX  Audio Frequency FT8");
    //gb_static_tx = new QGroupBox("RIG Frequency Shift Relative To Audio Frequency For FT8"
    //" (RIG must be readable && writable via CAT)");
    //rb_static_tx = new QRadioButton("Enable Frequency Shift RIG For FT8");
    gb_static_tx = new QGroupBox(tr("Selected Constant TX  Audio Frequency")+" FT Q65 "+
                                 tr("(RIG frequency must be readable && writable via CAT)"));
    rb_static_tx = new QCheckBox(tr("Use Selected Constant TX  Audio Frequency")+" FT Q65");
    //rb_static_tx->setContentsMargins(0,0,4,0);
    l_static_tx = new QLabel(tr("Select  Audio Frequency")+":");//QLabel *l_static_tx = new QLabel(tr("Select  Audio Frequency")+":");
    sb_static_tx = new QSpinBox();
    sb_static_tx->setPrefix("def=1500  ");
    sb_static_tx->setSuffix("  Hz");
    sb_static_tx->setRange(1200,2500);
    sb_static_tx->setSingleStep(100);
    sb_static_tx->setValue(1500);
    sb_static_tx->findChild<QLineEdit*>()->setReadOnly(true);
    //sb_static_tx->setStyleSheet("QSpinBox {selection-color: black; selection-background-color: white;}");

    QHBoxLayout *H_static_tx = new QHBoxLayout();
    H_static_tx->setContentsMargins(4,4,4,4);
    H_static_tx->setSpacing(4);
    H_static_tx->addWidget(rb_static_tx);
    //H_static_tx->setAlignment(rb_static_tx,Qt::AlignLeft);
    H_static_tx->addWidget(l_static_tx);
    H_static_tx->setAlignment(l_static_tx,Qt::AlignRight);
    H_static_tx->addWidget(sb_static_tx);
    //H_static_tx->setAlignment(Qt::AlignRight);

    gb_static_tx->setEnabled(false);//default
    gb_static_tx->setLayout(H_static_tx);
    connect(rb_static_tx, SIGNAL(toggled(bool)), this, SLOT(RbSetStaticTxF(bool)));
    connect(sb_static_tx, SIGNAL(valueChanged(int)), this, SLOT(SBStaticTxChanged(int)));
    f_txrx_static_tx = false;
    timer_try_static_tx = new QTimer();
    connect(timer_try_static_tx, SIGNAL(timeout()), this, SLOT(TimerTryStaticTxF()));
    timer_try_static_tx->setSingleShot(true);
    timer_try_static_tx_isActive = false;
    /////////////////////////////////////////////////////////////////

    //// qrg /////
    f_qrg_tx = false;
    f_qso7tx = false;
    prev_qrg = "";
    rx_qrg = 0;
    //// end qrg /////

    ///////////////////////////////////////////////////
    s_id_band = 17;//2.76.5 17 default HV 70 MHz

    FreqOffsetTR = "0";
    for (int i = 0; i<COUNT_BANDS; ++i)
    {
        //offset_trsv_rig[i].band = lst_bamds[i];
        offset_trsv_rig[i].rb_id = 0;
        offset_trsv_rig[i].freq = "";
    }
    rb_offset_tr_off = new QRadioButton(tr("Off"));
    rb_offset_tr_off->setChecked(true);
    rb_offset_tr_sum = new QRadioButton(tr("Sum"));
    rb_offset_tr_sub = new QRadioButton(tr("Subtract"));

    //QPushButton *pb_set_offset_trsv_rig = new QPushButton("SET OFFSET");
    //connect(pb_set_offset_trsv_rig, SIGNAL(clicked(bool)), this, SLOT(BBB()));

    QLabel *l_offs_tr = new QLabel(tr("Frequency In")+" Hz:");
    le_offset_trsv_rig = new QLineEdit();
    QRegExp rx("^[1-9][0-9]*$");//if dot -> "^[1-9][0-9.]*$"
    QValidator *validator = new QRegExpValidator(rx, this);
    le_offset_trsv_rig->setValidator(validator);
    le_offset_trsv_rig->setMaxLength(12);
    le_offset_trsv_rig->setEnabled(false);

    le_offset_trsv_rig_res = new QLineEdit();
    le_offset_trsv_rig_res->setReadOnly(true);
    le_offset_trsv_rig_res->setText("RIG: 70.000.000 Hz");

    QHBoxLayout *H_offstr = new QHBoxLayout();
    H_offstr->setContentsMargins( 4, 0, 0, 0);
    H_offstr->setSpacing(4);
    H_offstr->addWidget(l_offs_tr);
    H_offstr->addWidget(le_offset_trsv_rig);
    H_offstr->addWidget(le_offset_trsv_rig_res);

    //QLabel *l_offs_tr_help = new QLabel("How to make for any Band: Switc Menu Band to you band, set Port to None, setup and then choice Port again.");
    QLabel *l_offs_tr_help = new QLabel(
                                 tr("How to set: 1. In Interface Control set Port to None.\n"
                                    "2. Choose your band from the Band Menu.\n"
                                    "3. In Interface Control set up your Transverter or RIG offset.\n"
                                    "4. In Interface Control choose your Port to start communication.")
                             );

    gb_troffs = new QGroupBox(tr("Transverter Local Oscillator Or RIG Offset:   For Band")+" 70 MHz");
    QHBoxLayout *H_troffs = new QHBoxLayout();
    H_troffs->setContentsMargins(4,4,4,2);
    H_troffs->setSpacing(4);
    H_troffs->addWidget(rb_offset_tr_off);
    H_troffs->addWidget(rb_offset_tr_sum);
    H_troffs->addWidget(rb_offset_tr_sub);
    H_troffs->addLayout(H_offstr);
    //H_troffs->addWidget(pb_set_offset_trsv_rig);

    QVBoxLayout *V_troffs = new QVBoxLayout();
    V_troffs->setContentsMargins (2, 2, 2, 2);
    V_troffs->setSpacing(2);
    V_troffs->addWidget(l_offs_tr_help);
    V_troffs->setAlignment(l_offs_tr_help,Qt::AlignCenter);
    V_troffs->addLayout(H_troffs);

    gb_troffs->setLayout(V_troffs);
    connect(rb_offset_tr_off, SIGNAL(toggled(bool)), this, SLOT(RbOffsetTR(bool)));
    connect(rb_offset_tr_sum, SIGNAL(toggled(bool)), this, SLOT(RbOffsetTR(bool)));
    connect(rb_offset_tr_sub, SIGNAL(toggled(bool)), this, SLOT(RbOffsetTR(bool)));
    connect(le_offset_trsv_rig, SIGNAL(textChanged(QString)), this, SLOT(LeOffsetTrsvRigChange(QString)));
    //gb_troffs->setEnabled(false);
    ////////////////////////////////////////////////////////

    //groupBox->setEnabled(false);
    //////////////////////////////////////////////////////////////////////////za mahane
    /*QPushButton *pb_greq = new QPushButton(" SET FREQ ");
    connect(pb_greq, SIGNAL(clicked(bool)), this, SLOT(set_freq()));
    lfrq = new  QLabel();
    lfrq->setText("TRANSCEIVER REAL FREQ: 0");
    lmod = new  QLabel();
    lmod->setText("TRANSCEIVER REAL MODE: NO MODE");
    lefrq = new  QLineEdit();
    QRegExp rx("^[1-9][0-9]*$");
    lefrq = new QLineEdit();
    QValidator *validator = new QRegExpValidator(rx, this);
    lefrq->setValidator(validator);
    lefrq->setMaxLength(14);
    lefrq->setText("14250000");
    QPushButton *pb_mode = new QPushButton(" SET MODE TO USB ");
    connect(pb_mode, SIGNAL(clicked(bool)), this, SLOT(set_mode()));*/
    //////////////////////////////////////////////////////////////////////////za mahane

    cb_port2 = new QComboBox();
    cb_port2->addItem("None");
    QLabel *l_port2 = new QLabel(tr("Port")+": ");
    QHBoxLayout *H_port2 = new QHBoxLayout();
    H_port2->setContentsMargins(4,2,4,2);
    H_port2->addWidget(l_port2);
    H_port2->setAlignment(l_port2,Qt::AlignRight);
    H_port2->addWidget(cb_port2);

    cb_baud2 = new QComboBox();
    cb_baud2->addItems(lst_baud);
    //if (i96 >= 0) cb_baud2->setCurrentIndex(i96);
    cb_baud2->setCurrentIndex(5);
    QLabel *l_baud2 = new QLabel(tr("Baud Rate")+": ");
    QHBoxLayout *H_baud2 = new QHBoxLayout();
    H_baud2->setContentsMargins(4,4,4,4);
    H_baud2->addWidget(l_baud2);
    H_baud2->setAlignment(l_baud2,Qt::AlignRight);
    H_baud2->addWidget(cb_baud2);

    QGroupBox *gb_method2 = new QGroupBox(tr("PTT Method")+":");
    rb_rts2 = new QRadioButton(tr("PTT Via RTS"));
    rb_rts2->setChecked(true);
    rb_dtr2 = new QRadioButton(tr("PTT Via DTR"));

    QHBoxLayout *H_via2 = new QHBoxLayout();
    H_via2->setContentsMargins(1,0,1,1);
    H_via2->setSpacing(5);
    H_via2->addWidget(rb_rts2);
    H_via2->addWidget(rb_dtr2);
    H_via2->setAlignment(Qt::AlignCenter);
    gb_method2->setLayout(H_via2);

    pb_test_port2 = new QPushButton(" "+tr("START PTT TEST    NO PORT SELECTED")+" ");
    connect(pb_test_port2, SIGNAL(clicked(bool)), this, SLOT(TestPtt2()));
    pb_test_port2->setEnabled(false);
    f_test_port2 = false;

    QVBoxLayout *H_portboud2 = new QVBoxLayout();
    H_portboud2->setContentsMargins(0,0,0,0);
    H_portboud2->setSpacing(0);
    H_portboud2->addLayout(H_port2);
    H_portboud2->addLayout(H_baud2);
    QHBoxLayout *H_portvia2 = new QHBoxLayout();
    H_portvia2->setContentsMargins(10,0,0,2);
    H_portvia2->setSpacing(0);
    H_portvia2->addWidget(gb_method2);
    H_portvia2->addLayout(H_portboud2);
    //H_portvia2->addWidget(pb_test_port2);

    QGroupBox *gb_p2 = new QGroupBox(tr("Port")+" 2:");
    QVBoxLayout *V_p2 = new QVBoxLayout();
    V_p2->setContentsMargins(2,0,2,2);
    V_p2->setSpacing(1);
    //V_p2->addLayout(H_port2);
    //V_p2->addLayout(H_baud2);
    //V_p2->addWidget(gb_method2);
    V_p2->addLayout(H_portvia2);
    V_p2->addWidget(pb_test_port2);
    gb_p2->setLayout(V_p2);

///////////////////////////////////////////////////////////////////////////////
    QGroupBox *gb_txwatchdog = new QGroupBox(tr("Tx Watchdog")+":");
    rb_offtxwatch = new QRadioButton(tr("Off"));
    rb_offtxwatch->setChecked(true);// default off
    f_no_time_count_txwatchdog = 0;//by 0=off 1=time 2=count;
    rb_mintxwatch = new QRadioButton(tr("In Time"));
    rb_coutxwatch = new QRadioButton(tr("In Number Of TX Periods"));
    sb_mintxwatch = new QSpinBox();
    sb_mintxwatch->setPrefix("def=20  ");
    sb_mintxwatch->setSuffix("  "+tr("minutes"));
    s_txwatchdog_time = 20;// default 20
    sb_mintxwatch->setValue(s_txwatchdog_time);
    sb_mintxwatch->setRange(2,99);
    sb_mintxwatch->setSingleStep(1);
    sb_mintxwatch->setEnabled(false);
    sb_mintxwatch->setFixedWidth(152);//+125%
    sb_coutxwatch = new QSpinBox();
    sb_coutxwatch->setPrefix("def=10  ");
    sb_coutxwatch->setSuffix("  "+tr("Periods"));
    s_txwatchdog_coun = 10;// default 10
    sb_coutxwatch->setValue(s_txwatchdog_coun);
    sb_coutxwatch->setRange(2,50);
    sb_coutxwatch->setSingleStep(1);
    sb_coutxwatch->setEnabled(false);
    sb_coutxwatch->setFixedWidth(152);//+125%
    connect(rb_offtxwatch, SIGNAL(toggled(bool)), this, SLOT(RbTxwatchChanged(bool)));
    connect(rb_mintxwatch, SIGNAL(toggled(bool)), this, SLOT(RbTxwatchChanged(bool)));
    connect(rb_coutxwatch, SIGNAL(toggled(bool)), this, SLOT(RbTxwatchChanged(bool)));
    connect(sb_mintxwatch, SIGNAL(valueChanged(int)), this, SLOT(SBTxwatchChanged(int)));
    connect(sb_coutxwatch, SIGNAL(valueChanged(int)), this, SLOT(SBTxwatchChanged(int)));
    QHBoxLayout *H_txwatchdog = new QHBoxLayout();
    H_txwatchdog->setContentsMargins ( 2, 2, 2, 2);
    H_txwatchdog->setSpacing(5);
    H_txwatchdog->addWidget(rb_offtxwatch);
    H_txwatchdog->addWidget(rb_mintxwatch);
    H_txwatchdog->addWidget(sb_mintxwatch);
    H_txwatchdog->addWidget(rb_coutxwatch);
    H_txwatchdog->addWidget(sb_coutxwatch);
    H_txwatchdog->setAlignment(Qt::AlignHCenter);
    gb_txwatchdog->setLayout(H_txwatchdog);

    QVBoxLayout *V_p1 = new QVBoxLayout();
    V_p1->setContentsMargins(2,0,2,2);
    V_p1->setSpacing(2);
    ////// net //////////
    //V_p1->addLayout(H_port);
    //V_p1->addLayout(H_baud);
    V_p1->addLayout(H_net_port);
    ////// end net //////////
    V_p1->addWidget(gb_method);
    V_p1->addWidget(THvRigCat);
    V_p1->setAlignment(THvRigCat,Qt::AlignCenter);
    //V_p1->addWidget(cb_off_set_frq_to_rig);
    //V_p1->setAlignment(cb_off_set_frq_to_rig,Qt::AlignCenter);
    //V_p1->addLayout(H_rc);
    V_p1->addLayout(H_frs);
    V_p1->addWidget(pb_test_port);
    V_p1->addWidget(gb_static_tx);//2.16
    V_p1->addWidget(gb_troffs);
    gb_p1->setLayout(V_p1);

    QVBoxLayout *V_l = new QVBoxLayout(this);
    V_l->setContentsMargins(5,2,5,4);
    V_l->setSpacing(2);
    //V_l->addWidget(lbl);
    //    V_l->addLayout(H_port);
    //    V_l->addLayout(H_baud);
    //V_l->addWidget(rb_rts);
    //V_l->addWidget(rb_dtr);
    // V_l->addLayout(H_via);
    //    V_l->addWidget(gb_method);
    //    V_l->addWidget(THvRigCat);
    V_l->addWidget(gb_p1);
    V_l->addWidget(gb_p2);

    //V_l->setAlignment(THvRigCat,Qt::AlignCenter);

    //    V_l->addWidget(pb_test_port);

    /////////////////////////////////////////////////////////////// // za mahane
    /* V_l->addWidget(lfrq);// za mahane
     V_l->setAlignment(lfrq,Qt::AlignCenter); // za mahane
     V_l->addWidget(lmod);// za mahane
     V_l->setAlignment(lmod,Qt::AlignCenter); // za mahane
     V_l->addWidget(lefrq);// za mahane
     V_l->setAlignment(lefrq,Qt::AlignCenter); // za mahane
     V_l->addWidget(pb_greq);// za mahane
     V_l->addWidget(pb_mode);// za mahane*/
    ///////////////////////////////////////////////////////////////

    V_l->addWidget(gb_txwatchdog);
    this->setLayout(V_l);

    ScanComPorts();

    PortSettings settings = {BAUD9600, DATA_8, PAR_NONE, STOP_2, FLOW_OFF, 10};//long Timeout_Millisec;
    //stop port = new QextSerialPort("None", settings, QextSerialPort::Polling);


    ////////////////////////////////////////////////////////new read com
    port = new QextSerialPort("None", settings, QextSerialPort::EventDriven);
    ////////////////////////////////////////////////////////end new read com
    port2 = new QextSerialPort("None", settings, QextSerialPort::Polling);


    connect(cb_port, SIGNAL(currentIndexChanged(QString)), this, SLOT(PortNameChanged(QString)));
    connect(cb_baud, SIGNAL(currentIndexChanged(QString)), this, SLOT(BaudRateChanged(QString)));
    connect(rb_rts, SIGNAL(toggled(bool)), this, SLOT(PttChanget(bool)));
    connect(rb_dtr, SIGNAL(toggled(bool)), this, SLOT(PttChanget(bool)));
    connect(rb_cat, SIGNAL(toggled(bool)), this, SLOT(PttChanget(bool)));
    connect(rb_ptt_off, SIGNAL(toggled(bool)), this, SLOT(PttChanget(bool)));
    connect(cb_read_data_kenwood_rts_on, SIGNAL(toggled(bool)), this, SLOT(ReadDataRtsOnChanget(bool)));
    connect(cb_read_data_kenwood_dtr_on, SIGNAL(toggled(bool)), this, SLOT(ReadDataDtrOnChanget(bool)));

    connect(THvRigCat, SIGNAL(EmitCatAactiveAndRead(bool,bool)), this, SLOT(SetCatAactiveAndRead(bool,bool)));
    connect(THvRigCat, SIGNAL(EmitWriteCmd(char*,int)), this, SLOT(SetWriteCmd(char*,int)));
    connect(THvRigCat, SIGNAL(EmitSetRigSet(RigSet,int,int)), this, SLOT(SetRigSet(RigSet,int,int)));
    connect(THvRigCat, SIGNAL(EmitGetedFreq(QString)), this, SLOT(SetGetedFreq(QString)));
    connect(THvRigCat, SIGNAL(EmitGetedMode(QString)), this, SLOT(SetGetedMode(QString)));
    connect(THvRigCat, SIGNAL(EmitPttDtr(bool)), this, SLOT(SetPttDtr(bool)));// sea-235
    connect(THvRigCat, SIGNAL(EmitFullRigInfo(QString)), this, SLOT(SetFullRigInfo(QString)));//2.76.1 for pskreporter

    ////// omnirig /////////////
    omnirig_active = false;
    omnirig_false_refresh = false;
    omnirig_type = "";
#if defined _WIN32_
    connect(THvRigCat, SIGNAL(EmitOmniRigActive(bool,QString)), this, SLOT(SetOmniRigActive(bool,QString)));
    connect(this, SIGNAL(EmitOmniRigPttMethod(bool*)), THvRigCat, SIGNAL(EmitOmniRigPttMethod(bool*)));
#endif
    ////// omnirig end /////////////

    connect(cb_port2, SIGNAL(currentIndexChanged(QString)), this, SLOT(PortNameChanged2(QString)));
    connect(cb_baud2, SIGNAL(currentIndexChanged(QString)), this, SLOT(BaudRateChanged2(QString)));
    connect(rb_rts2, SIGNAL(toggled(bool)), this, SLOT(PttChanget2(bool)));
    connect(rb_dtr2, SIGNAL(toggled(bool)), this, SLOT(PttChanget2(bool)));

    connect(cb_off_set_frq_to_rig, SIGNAL(toggled(bool)), this, SLOT(CbOffSetFrqToRig(bool)));
    connect(cb_mod_set_frq_to_rig, SIGNAL(toggled(bool)), this, SIGNAL(EmitModSetFrqToRig(bool)));

    //connect(port, SIGNAL(readyRead()), this, SLOT(onReadyRead()));//1.61= read port

    /*port = new QextSerialPort(QLatin1String("COM8"), QextSerialPort::Polling);
    port->setBaudRate(BAUD19200);
    port->setFlowControl(FLOW_OFF);
    port->setParity(PAR_NONE);
    port->setDataBits(DATA_8);
    port->setStopBits(STOP_2);*/
    //set timeouts to 500 ms
    //port->setTimeout(500);

    ////////////////////////////////////////////////////////new read com
    connect(port, SIGNAL(readyRead()), this, SLOT(onReadyRead()));//1.61= read port
    ////////////////////////////////////////////////////////end new read com
    s_set_rig_mode_frq = "";
    set_rig_mode_id = 0;
    timer_set_rig_mode = new QTimer();
    connect(timer_set_rig_mode, SIGNAL(timeout()),this,SLOT(SetRigModeFreq()));
    timer_set_rig_mode->setSingleShot(true);
    f_msf = false;
    f_msf_new = false;
    f_msf_special_tx_rx = 0;
    //only_one = true;//za mahane
    //SetRigName("Yaesu FT-847");//za mahane
}
HvRigControl::~HvRigControl()
{
    //DestroyPort();//za mahane
    //qDebug()<<"CLOSE DestroyPort()";//za mahane
}
void HvRigControl::SetTciSelect(int i)
{
    THvRigCat->SetTciSelect(i);
}
void HvRigControl::SetMsf(bool f)//2.76sf
{
    if (f_msf_new==f) return;//2.76sf protrct from 0,1,2 from main_ms
    f_msf_new = f; //qDebug()<<"SetMsf()="<<s_mode<<f_msf_special_tx_rx<<f;
    if (f_msf_special_tx_rx==0)
    {
        f_msf = f_msf_new;
        RbSetStaticTxF(false);
    }
    else f_msf_special_tx_rx = 2;
}
void HvRigControl::CatStopPttIfClose()
{
    if (omnirig_active || net_active || (rb_cat->isChecked() && port->isOpen()))
    {
        THvRigCat->set_ptt(false,true);//2.38 true=immediately

        if (f_static_tx && all_static_tx_modes && f_static_tx_active)
        {
            usleep(50000);
            int fst = sb_static_tx->value();
            if (s_mode==11 && f_msf) fst = 750;
            long long int static_tx_f = (long long int)fst;
            long long int return_rig_frq = curr_rig_tx_frq + (static_tx_f - last_v_disp_tx_frq);
            SetFreq(QString("%1").arg(return_rig_frq),3);//2.74 0=mybe frq,no mod, 1=mybe frq,mod, 2=frq,mod, 3=frq,no mod
            //qDebug()<<"CLOSE DestroyPort()"<<return_rig_frq<<f_static_tx_active;
        }
    }
    //2.60 remove QRG at close //qDebug()<<"CLOSE DestroyPort()"<<f_qrg_tx;
    if ((s_mode==0 || s_mode==2 || s_mode==3 || s_mode==12) && rig_cat_active_and_read && f_qrg_tx)
    {
        emit EmitQrgActive(1);
    }
}
void HvRigControl::SetMode(int mod)
{
    s_mode = mod; //qDebug()<<mod;
    if (s_mode==11 || s_mode==13 || s_mode==18 || s_mode==14 || s_mode==15 || s_mode==16 || s_mode==17) all_static_tx_modes = true;
    else all_static_tx_modes = false;
    corr_app_ind_tx_frq = 0;//for any case
    if ((s_mode==0 || s_mode==2 || s_mode==3 || s_mode==12) && rig_cat_active_and_read) emit EmitQrgActive(1);
    else emit EmitQrgActive(0);
    RbSetStaticTxF(false);//2.76sf fictive for Q65 FT4 no SF
}
/*void HvRigControl::SetFont(QFont fo)
{
    sb_static_tx->setFont(fo);
    THvRigCat->SetFont(fo);
}*/
void HvRigControl::SBStaticTxChanged(int)
{
    RbSetStaticTxF(false); // fictive
}
void HvRigControl::RbSetStaticTxF(bool)
{
    int fst = sb_static_tx->value();//2.76sf
    if (s_mode==11 && f_msf)
    {
    	fst = 750;
    	sb_static_tx->setHidden(true);
    	l_static_tx->setText(tr("Select  Audio Frequency")+":  def=750  750  Hz   ");    	    	
   	}
    else 
    {
    	l_static_tx->setText(tr("Select  Audio Frequency")+":");
    	sb_static_tx->setHidden(false);    	
   	} 
    if (rb_static_tx->isChecked() && gb_static_tx->isEnabled())//no move RIG static freq -> || rig_cat_active_and_read
    {
        emit EmitStaticTxFrq(true,fst);
        f_static_tx = true;
        corr_app_ind_tx_frq = 0; //qDebug()<<"RbSetStaticTxF()"<<"On"<<fst;
    }
    else
    {
        emit EmitStaticTxFrq(false,fst);
        f_static_tx = false;
        corr_app_ind_tx_frq = 0; //qDebug()<<"RbSetStaticTxF()"<<"Off"<<fst;
    }
}
void HvRigControl::SetTxFreq(double f)
{
    if (mp_v_disp_tx_frq==1.0) v_disp_tx_frq = (long long int)round(f);
    else
    {
        int tx_frq1 = 0;
        if (mp_v_disp_tx_frq==10.0)
            tx_frq1 = (int)(round(f/10.0)*10.0);
        else
        {
            tx_frq1 = (int)round(f);
            tx_frq1 /= mp_v_disp_tx_frq;
            tx_frq1 *= mp_v_disp_tx_frq;
        }
        //int tx_frq2 = (int)(round(f/mp_v_disp_tx_frq)*mp_v_disp_tx_frq);
        v_disp_tx_frq = (long long int)tx_frq1;
        //qDebug()<<"V1="<<mp_v_disp_tx_frq<<"VdispReal="<<f<<"VdispRounded="<<tx_frq1;
        //qDebug()<<"V2="<<mp_v_disp_tx_frq<<"VdispReal="<<f<<"VdispRounded="<<tx_frq2;
    }
    /*value   round   floor   ceil    trunc
    -----   -----   -----   ----    -----
    2.3     2.0     2.0     3.0     2.0
    3.8     4.0     3.0     4.0     3.0
    5.5     6.0     5.0     6.0     5.0
    -2.3    -2.0    -3.0    -2.0    -2.0
    -3.8    -4.0    -4.0    -3.0    -3.0
    -5.5    -6.0    -6.0    -5.0    -5.0*/
}
void HvRigControl::SetStaticTxParms(QString s)
{
    QStringList ls=s.split("#");
    if (ls.count()>1)
    {
        sb_static_tx->setValue(ls.at(1).toInt());
        if (ls.at(0)=="1") rb_static_tx->setChecked(true);
    }
}
void HvRigControl::SetFullRigInfo(QString s)//2.76.1 for pskreporter
{
    s_rig_full_name = s;//qDebug()<<s_rig_full_name;
}
void HvRigControl::SetCatAactiveAndRead(bool f,bool toTxWidget)
{
    //if (net_active) f = false; // if net stop static tx
    //if (g_block_rig_st_tx) f = false; //2.50 if "DX Lab Suite Commander" stop static tx, block for specific rig
    if (toTxWidget)
    {
        emit EmitRigCatActiveAndRead(f,s_rig_full_name);//2.53 //2.76.1 or s_rig_name for pskreporter
        if (!f) end_rs0 = 0;//2.55 reset
    }
    rig_cat_active_and_read = f; //2.30
    gb_static_tx->setEnabled(f); //qDebug()<<f;

    if (f && (s_mode==0 || s_mode==2 || s_mode==3 || s_mode==12)) emit EmitQrgActive(1);
    else emit EmitQrgActive(0);

    if (f) RbSetStaticTxF(false); // fictive
    else
    {
        int fst = sb_static_tx->value();//2.76sf
        if (s_mode==11 && f_msf) fst = 750;
        emit EmitStaticTxFrq(false,fst);
        f_static_tx = false;
        corr_app_ind_tx_frq = 0; //qDebug()<<"Off"<<sb_static_tx->value();
    }
}
////// omnirig /////////////
#if defined _WIN32_
void HvRigControl::SetOmniRigActive(bool f1,QString s)
{
    omnirig_active = f1;

    cb_port->setCurrentIndex(0); //<-garmi
    cb_baud->setEnabled(false);
    cb_port->setEnabled(false);
    omnirig_type = "";

    if (omnirig_active)
    {
        omnirig_type = " "+s;
        pb_test_port->setEnabled(true);
        pb_test_port->setText(" "+tr("START PTT TEST")+" "+omnirig_type);
        THvRigCat->StartStopPollingTimer(true);
        //SetStaticTxF(true);
        bool f[10];
        f[0] = rb_ptt_off->isChecked();
        f[1] = rb_rts->isChecked();
        f[2] = rb_dtr->isChecked();
        f[3] = rb_cat->isChecked();
        f[4] = cb_read_data_kenwood_rts_on->isChecked();
        f[5] = cb_read_data_kenwood_dtr_on->isChecked();
        emit EmitOmniRigPttMethod(f);
        gb_troffs->setEnabled(false);
    }
    else
    {
        SetRigName("None");
        pb_test_port->setEnabled(false);
        pb_test_port->setText(" "+tr("START PTT TEST    FAILED TO START OMNIRIG")+" "+omnirig_type);
        omnirig_false_refresh = true;
        gb_troffs->setEnabled(true);
    }
    //s="OmniRig Rig 1 = A";
    int id0 = s.indexOf(" ");//2.76.1 for pskreporter
    if (id0>-1)
    {
        int id1 = s.indexOf("=");
        if (id1>-1 && s.count()>id1+2)
        {
            s.replace(id0,id1-id0+1,"");
            SetFullRigInfo(s);//qDebug()<<"OmniRigFullName="<<s;
        }
    }
}
#endif
////// omnirig end /////////////
////// net //////////
void HvRigControl::SetNetRigSrvPort(QString s)
{
    QStringList ls = s.split("#");  //HRD=7809, FlRig=12345, DXCommander=52002
    int cls = ls.count();
    if (cls > NETWORK_COUNT) cls = NETWORK_COUNT;
    for (int i = 0; i < cls; ++i)
    {
        QString s0 = ls.at(i);
        QStringList ls0 = s0.split(";");
        if (ls0.count()==2)
        {
            netServPort[i].serv = ls0.at(0);
            netServPort[i].port = ls0.at(1);
        }
        if (ls0.count()==7 && (i==3 || i==4))//tci 1,2
        {
            netServPort[i].serv = ls0.at(0);
            netServPort[i].port = ls0.at(1); //qDebug()<<i<<ls0.at(3)<<ls0.at(3);
            int z = ls0.at(2).toInt();
            if (z>-1 && z<2) netTciChSmp[i-3].channels = z;
            netTciChSmp[i-3].samples = ls0.at(3);
            z = ls0.at(4).toInt();
            if (z>-1 && z<4) netTciChSmp[i-3].type = z;
            netTciChSmp[i-3].srate = ls0.at(5);
            z = ls0.at(6).toInt();
            if (z>49 && z<401) netTciChSmp[i-3].txbuff = z;
        }
    }
}
QString HvRigControl::net_rig_srv_port()
{
    QString out;
    for (int i = 0; i < NETWORK_COUNT; ++i) //HRD=7809, FlRig=12345, DXCommander=52002
    {
        out.append(netServPort[i].serv+";"+netServPort[i].port);
        if (i==3 || i==4) out.append(";"+QString("%1").arg(netTciChSmp[i-3].channels)+";"+
                                         netTciChSmp[i-3].samples+";"+QString("%1").arg(netTciChSmp[i-3].type)+";"+
                                         netTciChSmp[i-3].srate+";"+QString("%1").arg(netTciChSmp[i-3].txbuff));
        if (i<NETWORK_COUNT-1) out.append("#");
    }
    return out;
}
void HvRigControl::TCPSPChanged(QString)
{
    if (block_save_net) return;
    if (s_net_model_id>-1 && s_net_model_id<NETWORK_COUNT)
    {
        netServPort[s_net_model_id].serv = TCPServer->text();
        netServPort[s_net_model_id].port = TCPPort->text();
        if (s_net_model_id==3 || s_net_model_id==4)
        {
            int id = s_net_model_id - 3;
            netTciChSmp[id].channels = tci_channels->currentIndex();
            netTciChSmp[id].samples  = tci_samples->currentText();
            netTciChSmp[id].type = tci_tcistype->currentIndex();
            netTciChSmp[id].srate = tci_rample_rate->currentText();
            netTciChSmp[id].txbuff = tci_tx_buff->value();
        }
    }
}
void HvRigControl::SetNetConnect()
{
    QString s = TCPServer->text()+"#"+TCPPort->text()+"#"+tci_channels->currentText()+"#"+
                tci_samples->currentText()+"#"+tci_tcistype->currentText()+"#"+tci_rample_rate->currentText()+"#"+
                QString("%1").arg(tci_tx_buff->value());
    emit EmitNetSP(s);//TCPServer->text(),TCPPort->text(),tci_channels->currentText(),tci_samples->currentText());
}
void HvRigControl::SetNetConnInfo(QString s,bool f1,bool f2)
{
    l_con_info->setText(s);
    if (f1 && f2)
    {
        pb_re_conect->setText(tr("Disconnect"));
        pb_re_conect->setEnabled(true);
        THvRigCat->StartStopPollingTimer(true);
        pb_test_port->setEnabled(true);
        pb_test_port->setText(" "+tr("START PTT TEST")+" "+omnirig_type);

        l_tcisrate->setEnabled(false);
        tci_rample_rate->setEnabled(false);
        tci_tx_buff->setEnabled(false);
        l_tcich->setEnabled(false);
        tci_channels->setEnabled(false);
        l_tcisamp->setEnabled(false);
        tci_samples->setEnabled(false);
        l_tcistype->setEnabled(false);
        tci_tcistype->setEnabled(false);

        net_active = true;
    }
    else if (f1 && !f2)
    {
        pb_re_conect->setText(tr("Wait"));
        pb_re_conect->setEnabled(false);
        THvRigCat->StartStopPollingTimer(false);
        pb_test_port->setEnabled(false);

        l_tcisrate->setEnabled(false);
        tci_rample_rate->setEnabled(false);
        tci_tx_buff->setEnabled(false);
        l_tcich->setEnabled(false);
        tci_channels->setEnabled(false);
        l_tcisamp->setEnabled(false);
        tci_samples->setEnabled(false);
        l_tcistype->setEnabled(false);
        tci_tcistype->setEnabled(false);

        net_active = true;
    }
    else
    {
        pb_re_conect->setText(tr("Connect"));
        pb_re_conect->setEnabled(true);
        THvRigCat->StartStopPollingTimer(false);
        pb_test_port->setEnabled(false);

        l_tcisrate->setEnabled(true);
        tci_rample_rate->setEnabled(true);
        tci_tx_buff->setEnabled(true);
        l_tcich->setEnabled(true);
        tci_channels->setEnabled(true);
        l_tcisamp->setEnabled(true);
        tci_samples->setEnabled(true);
        l_tcistype->setEnabled(true);
        tci_tcistype->setEnabled(true);

        net_active = false;
    }
}
////// end net //////////
void HvRigControl::onReadyRead()
{
    //QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
    //port->flush();

    QByteArray bytes;
    int a = port->bytesAvailable();
    bytes.resize(a);
    port->read(bytes.data(), bytes.size());
    THvRigCat->SetReadyRead(bytes,bytes.size());

    //sss.append(QString(bytes.toHex()));
    //qDebug()<<"sss"<<sss;
    //while (port->waitForReadyRead(5000))
    //s_bytes.append(port->readAll());
    //qDebug()<<"ReadCmd1"<<(QString(s_bytes.toHex()));
    //s_bytes.append(port->readAll());
    //qDebug()<<"ReadCmd1"<<(QString(s_bytes.toHex()));
}
////////////////////////////////////////////////////////end new read com
void HvRigControl::SetTxWatchdogParms(QString s)
{
    QStringList ls=s.split("#");
    if (ls.count()>2)
    {
        sb_mintxwatch->setValue(ls.at(1).toInt());
        sb_coutxwatch->setValue(ls.at(2).toInt());
        if (ls.at(0)=="0")
            rb_offtxwatch->setChecked(true);
        else if (ls.at(0)=="1")
            rb_mintxwatch->setChecked(true);
        else if (ls.at(0)=="2")
            rb_coutxwatch->setChecked(true);
    }
}
void HvRigControl::SBTxwatchChanged(int)
{
    s_txwatchdog_time = sb_mintxwatch->value();
    s_txwatchdog_coun = sb_coutxwatch->value();
    emit EmitTxWatchdogParms(f_no_time_count_txwatchdog,s_txwatchdog_time,s_txwatchdog_coun);
}
void HvRigControl::RbTxwatchChanged(bool)
{
    if (rb_offtxwatch->isChecked())
    {
        sb_mintxwatch->setEnabled(false);
        sb_coutxwatch->setEnabled(false);
        f_no_time_count_txwatchdog = 0;
    }
    else if (rb_mintxwatch->isChecked())
    {
        sb_mintxwatch->setEnabled(true);
        sb_coutxwatch->setEnabled(false);
        f_no_time_count_txwatchdog = 1;
    }
    else if (rb_coutxwatch->isChecked())
    {
        sb_mintxwatch->setEnabled(false);
        sb_coutxwatch->setEnabled(true);
        f_no_time_count_txwatchdog = 2;
    }
    emit EmitTxWatchdogParms(f_no_time_count_txwatchdog,s_txwatchdog_time,s_txwatchdog_coun);
}
void HvRigControl::TestPtt()
{
    if (!f_test_port)
    {
        f_test_port = true;
        SetPtt(true,1);//id 0=All 1=p1 2=p2
        pb_test_port->setText(" "+tr("STOP PTT TEST")+" "+omnirig_type);
        pb_test_port->setStyleSheet("QPushButton{background-color:rgb(255,128,128);}");
        SetEnabledAll(false);
    }
    else
    {
        f_test_port = false;
        SetPtt(false,1);//id 0=All 1=p1 2=p2
        pb_test_port->setText(" "+tr("START PTT TEST")+" "+omnirig_type);
        pb_test_port->setStyleSheet("QPushButton{background-color:palette(Button);}");
        SetEnabledAll(true);
    }
}
#if defined _WIN32_
QStringList HvRigControl::portNamesFromHardwareDeviceMap()
{
    HKEY hKey = NULL;// = Q_NULLPTR;
    if (::RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"HARDWARE\\DEVICEMAP\\SERIALCOMM", 0, KEY_QUERY_VALUE, &hKey) != ERROR_SUCCESS)
        return QStringList();

    QStringList result;
    DWORD index = 0;

    // This is a maximum length of value name, see:
    // https://msdn.microsoft.com/en-us/library/windows/desktop/ms724872%28v=vs.85%29.aspx
    enum { MaximumValueNameInChars = 16383 };

    std::vector<wchar_t> outputValueName(MaximumValueNameInChars, 0);
    std::vector<wchar_t> outputBuffer(MAX_PATH + 1, 0);
    DWORD bytesRequired = MAX_PATH;
    for (;;)
    {
        DWORD requiredValueNameChars = MaximumValueNameInChars;
        const LONG ret = ::RegEnumValue(hKey, index, &outputValueName[0], &requiredValueNameChars,
                                        NULL, NULL, reinterpret_cast<PBYTE>(&outputBuffer[0]), &bytesRequired);
        if (ret == ERROR_MORE_DATA)
        {
            outputBuffer.resize(bytesRequired / sizeof(wchar_t) + 2, 0);
        }
        else if (ret == ERROR_SUCCESS)
        {
            result.append(QString::fromWCharArray(&outputBuffer[0]));
            ++index;
        }
        else
        {
            break;
        }
    }
    ::RegCloseKey(hKey);
    return result;
}
QStringList HvRigControl::StrListToUpper(QStringList lst)
{
    for (int i = 0; i<lst.count(); i++)
    {
        lst[i] = lst.at(i).toUpper();
    }
    return lst;
}
#endif
void HvRigControl::ScanComPorts()
{
    QStringList lst_ports;
    QList<QextPortInfo> ports = QextSerialEnumerator::getPorts();
    foreach (QextPortInfo info, ports) //for (auto &info : ports)
    {
#if defined _FREEBSDHV_
        QString tn = info.portName;
        //if (!info.portName.isEmpty() && !tn.contains(".init") && !tn.contains(".lock") )
        //lst_ports.append(info.portName);
        if (tn.isEmpty() || tn.endsWith(QLatin1String(".init")) || tn.endsWith(QLatin1String(".lock"))) continue;
        lst_ports.append(tn);
#else
        if (!info.portName.isEmpty())
            lst_ports.append(info.portName);
#endif
        /*qDebug() << "port name:"       << info.portName;
        qDebug() << "friendly name:"   << info.friendName;
        qDebug() << "physical name:"   << info.physName;
        qDebug() << "enumerator name:" << info.enumName;
        qDebug() << "vendor ID:"       << info.vendorID;
        qDebug() << "product ID:"      << info.productID;
        qDebug() << "===================================";*/
    }

#if defined _WIN32_
    QStringList lst_ports_hdm;
    ComEnumSort le;
    lst_ports_hdm = portNamesFromHardwareDeviceMap();

    //lst_ports_hdm << "COM1"<< "cOM6"<< "COm3"<< "COM2";
    //lst_ports     << "COM1"<< "COm2"<< "COM17"<< "COM6";

    if ( lst_ports.count() > 0 )
        lst_ports = StrListToUpper(lst_ports);

    if ( lst_ports_hdm.count() > 0 )
    {
        lst_ports_hdm = StrListToUpper(lst_ports_hdm);
        for (int i = 0; i<lst_ports_hdm.count(); i++)
        {
            if ( lst_ports.contains(lst_ports_hdm.at(i),Qt::CaseInsensitive) != true )
                lst_ports << lst_ports_hdm.at(i);
        }
    }
    if ( lst_ports.count() > 0 )
    {
        //q_Sort(lst_ports.begin(), lst_ports.end(), le);
        std::sort(lst_ports.begin(), lst_ports.end(), le);
        QStringList lst_ports_sort;
        foreach (const QString &str, lst_ports)
        {
            lst_ports_sort<<str;
        }
        lst_ports = lst_ports_sort;
    }
    //qDebug() << "lst_ports=" <<lst_ports;
#endif
    //qDebug() << "lst_ports=" <<lst_ports;
    cb_port->addItems(lst_ports);
    cb_port2->addItems(lst_ports);
    // qDebug() << "===================================";
}
void HvRigControl::SetPttDtr(bool f)//sea-235
{
    if (port->isOpen()) port->setDtr(f); //qDebug()<<"DTR---FROM-SEA------->"<<f;
}
void HvRigControl::SetWriteCmd(char*data,int size)
{
    if (port->isOpen())
    {
        //QByteArray array(data, size);
        //qDebug()<<"WriteCmd="<<(QString(array.toHex()))<<array.data();
        port->write(data,size);
    }
}
void HvRigControl::Set_Rts(bool flag)
{
    if (port->isOpen())
    {
        if (!cb_read_data_kenwood_rts_on->isChecked()) port->setRts(flag);
        else port->setRts(true);
    }
}
void HvRigControl::Set_Dtr(bool flag)
{
    if (port->isOpen())
    {
        if (s_rig_name=="SEA-235" || cb_read_data_kenwood_dtr_on->isChecked()) port->setDtr(true);
        else port->setDtr(flag);
    }
}
void HvRigControl::PortNameChanged(QString str)
{
    //qDebug()<<"str"<<port->isOpen();

    if (port->isOpen())
    {
        THvRigCat->StartStopPollingTimer(false);
        //SetStaticTxF(false);
        //port->reset();
        port->close();
        //qDebug("is open: %d", port->isOpen());
    }

    port->setTimeout(500);//bold note: this function does nothing in event driven mode.

    port->setPortName(str);
    port->open(QIODevice::ReadWrite | QIODevice::Unbuffered);//1.63= important for linux
    //port->open(QIODevice::ReadWrite);
    Set_Dtr(false);
    Set_Rts(false);

    if (port->isOpen())
    {
        if (!rb_ptt_off->isChecked())
            pb_test_port->setEnabled(true);

        pb_test_port->setText(" "+tr("START PTT TEST")+" "+omnirig_type);

        THvRigCat->StartStopPollingTimer(true);//,cb_read_data_kenwood_rts_on->isChecked()
        //SetStaticTxF(true);
        gb_troffs->setEnabled(false);
    }
    else
    {
        pb_test_port->setEnabled(false);
        if (str=="None") pb_test_port->setText(" "+tr("START PTT TEST    NO PORT SELECTED")+" "+omnirig_type);
        else pb_test_port->setText(" "+tr("START PTT TEST    PORT")+" : "+str+" "+tr("IS BUSY")+" "+omnirig_type);
        gb_troffs->setEnabled(true);
    }
    //qDebug()<<str<<"PortNameChanged is open:"<<port->isOpen()<<omnirig_active;
}
void HvRigControl::BaudRateChanged(QString str)
{
    //QString s_port_name = port->portName();
    if (port->isOpen())// important
    {
        THvRigCat->StartStopPollingTimer(false);  //end command important
        //SetStaticTxF(false);
        usleep(200000);
    }

    switch (str.toInt())
    {
    case 300:
        port->setBaudRate(BAUD300);
        break;
    case 600:
        port->setBaudRate(BAUD600);
        break;
    case 1200:
        port->setBaudRate(BAUD1200);
        break;
    case 2400:
        port->setBaudRate(BAUD2400);
        break;
    case 4800:
        port->setBaudRate(BAUD4800);
        break;
    case 9600:
        port->setBaudRate(BAUD9600);
        break;
        //case 14400:
        //port->setBaudRate(BAUD14400);
        //break;
    case 19200:
        port->setBaudRate(BAUD19200);
        break;
    case 38400:
        port->setBaudRate(BAUD38400);
        break;
    case 57600:
        port->setBaudRate(BAUD57600);
        break;
    case 115200:
        port->setBaudRate(BAUD115200);
        break;
    default:
        port->setBaudRate(BAUD9600);
        break;
    }
    Set_Dtr(false);
    Set_Rts(false);

    //PortNameChanged(s_port_name);

    if (port->isOpen())
    {
        THvRigCat->StartStopPollingTimer(true);
        //SetStaticTxF(true);
    }
}
void HvRigControl::SetEnabledAll(bool f)
{
    THvRigCat->setEnabled(f);

    //if (s_rig_name=="SEA-235") rb_dtr->setEnabled(false);//2.73 down sea-235
    //else rb_dtr->setEnabled(f);

    ////// omnirig /////////////
    if (omnirig_active || net_active)////// net //////////
    {
        cb_baud->setEnabled(false);
        cb_port->setEnabled(false);
    }
    else
    {
        cb_baud->setEnabled(f);
        cb_port->setEnabled(f);
    }
    ////// omnirig end /////////////

    ////// net //////////
    if (f && net_active) gb_net->setEnabled(true);
    else gb_net->setEnabled(false);

    rb_ptt_off->setEnabled(f);
    if (cb_read_data_kenwood_rts_on->isChecked()) rb_rts->setEnabled(false);
    else rb_rts->setEnabled(f);
    if (cb_read_data_kenwood_dtr_on->isChecked() || s_rig_name=="SEA-235") rb_dtr->setEnabled(false);
    else rb_dtr->setEnabled(f);
    if (s_have_read_data_rts_on==3)
    {
        cb_read_data_kenwood_rts_on->setEnabled(f);
        cb_read_data_kenwood_dtr_on->setEnabled(f);
    }
    else if (s_have_read_data_rts_on==2) cb_read_data_kenwood_dtr_on->setEnabled(f);
    else if (s_have_read_data_rts_on==1) cb_read_data_kenwood_rts_on->setEnabled(f);
    else
    {
        cb_read_data_kenwood_rts_on->setEnabled(false);
        cb_read_data_kenwood_dtr_on->setEnabled(false);
    }

    //qDebug()<<s_ptt_type_t;
    if (s_ptt_type_t == RIG_PTT_NONE) rb_cat->setEnabled(false);
    else rb_cat->setEnabled(f);
}
void HvRigControl::PttChanget(bool)
{
    Set_Rts(false);
    Set_Dtr(false);

    if (port->isOpen() && !rb_ptt_off->isChecked())
        pb_test_port->setEnabled(true);
    else
    {
        if (!omnirig_active)////// omnirig /////////////
            pb_test_port->setEnabled(false);
    }

    if (f_test_port)
    {
        f_test_port = false;
        SetPtt(false,1);//id 0=All 1=p1 2=p2
        pb_test_port->setText(" "+tr("START PTT TEST")+" "+omnirig_type);
        pb_test_port->setStyleSheet("QPushButton{background-color:palette(Button);}");
        SetEnabledAll(true);
    }
    ////// omnirig /////////////
#if defined _WIN32_
    if (omnirig_active)
    {
        bool f[10];
        f[0] = rb_ptt_off->isChecked();
        f[1] = rb_rts->isChecked();
        f[2] = rb_dtr->isChecked();
        f[3] = rb_cat->isChecked();
        f[4] = cb_read_data_kenwood_rts_on->isChecked();
        f[5] = cb_read_data_kenwood_dtr_on->isChecked();
        emit EmitOmniRigPttMethod(f);
    }
#endif
    ////// omnirig end /////////////
}
void HvRigControl::SetRigSet(RigSet sett, int have_read_data_rts_on,int net_model_id)
{
    //2.50 "DX Lab Suite Commander" resolution is to 1kHz
    //if (sett.name=="DX Lab Suite Commander") g_block_rig_st_tx = true;//2.50 block for specific rig
    //else g_block_rig_st_tx = false;

    s_net_model_id = net_model_id;
    s_rig_name = sett.name;
    ////// net //////////
    block_save_net = true;
    l_tcisrate->setHidden(true);
    tci_rample_rate->setHidden(true);
    tci_tx_buff->setHidden(true);
    l_tcich->setHidden(true);
    tci_channels->setHidden(true);
    l_tcisamp->setHidden(true);
    tci_samples->setHidden(true);
    l_tcistype->setHidden(true);
    tci_tcistype->setHidden(true);
    if (sett.port_type==RIG_PORT_NETWORK)
    {
        if (s_net_model_id>-1 && s_net_model_id<NETWORK_COUNT)
        {
            TCPServer->setText(netServPort[s_net_model_id].serv);
            TCPPort->setText(netServPort[s_net_model_id].port);
            if (s_net_model_id==3 || s_net_model_id==4)
            {
                int id = s_net_model_id - 3;
                tci_channels->setCurrentIndex(netTciChSmp[id].channels);
                int index = tci_samples->findText(netTciChSmp[id].samples,Qt::MatchCaseSensitive);
                if (index >= 0) tci_samples->setCurrentIndex(index);
                tci_tcistype->setCurrentIndex(netTciChSmp[id].type);
                index = tci_rample_rate->findText(netTciChSmp[id].srate,Qt::MatchCaseSensitive);
                if (index >= 0) tci_rample_rate->setCurrentIndex(index); 
                tci_tx_buff->setValue(netTciChSmp[id].txbuff);               
                l_tcisrate->setHidden(false);
                tci_rample_rate->setHidden(false);
                tci_tx_buff->setHidden(false);
                l_tcich->setHidden(false);
                tci_channels->setHidden(false);
                l_tcisamp->setHidden(false);
                tci_samples->setHidden(false);
                l_tcistype->setHidden(false);
                tci_tcistype->setHidden(false);
            }
        }
        net_active = true;
        gb_net->setEnabled(true);
        cb_port->setCurrentIndex(0);
    }
    else
    {
        net_active = false;
        gb_net->setEnabled(false);
    }
    block_save_net = false;
    ////// end net //////////

    ////// omnirig /////////////
    if (omnirig_active)
    {
        omnirig_active = false;
        omnirig_type = "";
        pb_test_port->setEnabled(false);
        pb_test_port->setText(" "+tr("START PTT TEST    NO PORT SELECTED")+" "+omnirig_type);
        gb_troffs->setEnabled(true);
    }
    // refresh button test omnirig exeption
    if (omnirig_false_refresh)
    {
        omnirig_false_refresh = false;
        pb_test_port->setText(" "+tr("START PTT TEST    NO PORT SELECTED")+" "+omnirig_type);
        gb_troffs->setEnabled(true);
    }
    ////// omnirig end /////////////

    s_ptt_type_t = sett.ptt_type;

    SetEnabledAll(true);

    if (sett.port_type==RIG_PORT_NETWORK) SetNetConnect();//2.51 need to be here
    //qDebug()<<"rrrr"<<s_rig_name;
    ////////////////////////////////////////////////////////new read com
    rb_dtr->setEnabled(true);
    rb_rts->setEnabled(true);
    s_have_read_data_rts_on = have_read_data_rts_on;
    if (have_read_data_rts_on==3)//dtr and rts active
    {
        cb_read_data_kenwood_dtr_on->setEnabled(true);
        cb_read_data_kenwood_rts_on->setEnabled(true);
        if (cb_read_data_kenwood_dtr_on->isChecked()) rb_dtr->setEnabled(false);
        if (cb_read_data_kenwood_rts_on->isChecked()) rb_rts->setEnabled(false);
    }
    else if (have_read_data_rts_on==2)//dtr active
    {
        cb_read_data_kenwood_dtr_on->setEnabled(true);
        cb_read_data_kenwood_rts_on->setEnabled(false);
        if (cb_read_data_kenwood_rts_on->isChecked()) cb_read_data_kenwood_rts_on->setChecked(false);
        if (cb_read_data_kenwood_dtr_on->isChecked()) rb_dtr->setEnabled(false);
    }
    else if (have_read_data_rts_on==1)//rts active
    {
        cb_read_data_kenwood_rts_on->setEnabled(true);
        cb_read_data_kenwood_dtr_on->setEnabled(false);
        if (cb_read_data_kenwood_dtr_on->isChecked()) cb_read_data_kenwood_dtr_on->setChecked(false);
        if (cb_read_data_kenwood_rts_on->isChecked()) rb_rts->setEnabled(false);
        if (s_rig_name=="SEA-235")
        {
            rb_dtr->setEnabled(false);
            if (rb_dtr->isChecked()) rb_cat->setChecked(true);
        }
    }
    else
    {
        cb_read_data_kenwood_rts_on->setEnabled(false);
        cb_read_data_kenwood_dtr_on->setEnabled(false);
        if (cb_read_data_kenwood_rts_on->isChecked()) cb_read_data_kenwood_rts_on->setChecked(false);
        if (cb_read_data_kenwood_dtr_on->isChecked()) cb_read_data_kenwood_dtr_on->setChecked(false);
    }
    ////////////////////////////////////////////////////////end new read com
    if (sett.ptt_type == RIG_PTT_NONE && rb_cat->isChecked())
    {
        if (cb_read_data_kenwood_rts_on->isChecked() && cb_read_data_kenwood_dtr_on->isChecked()) rb_ptt_off->setChecked(true);
        else if (cb_read_data_kenwood_rts_on->isChecked()) rb_dtr->setChecked(true);
        else rb_rts->setChecked(true);
    }

    if (only_one)
    {
        int index = cb_baud->findText(QString("%1").arg(sett.serial_rate_max), Qt::MatchCaseSensitive);
        if (index >= 0) cb_baud->setCurrentIndex(index);
        else cb_baud->setCurrentIndex(3);// towa go resetwa na 4800
    }
    only_one = true;

    THvRigCat->StartStopPollingTimer(port->isOpen());
    //SetStaticTxF(port->isOpen());

    //corr precision
    if (sett.name=="FT-847" ||
            sett.name=="FT-990" ||
            sett.name=="FT-857" ||
            sett.name=="FT-1000D" ||
            sett.name=="FT-900" ||
            sett.name=="FT-897" ||
            sett.name=="FT-890" ||
            sett.name=="FT-840" ||
            sett.name=="FT-817" ||
            sett.name=="FT-767GX" ||
            sett.name=="FT-757GX" ||
            sett.name=="FT-980"
            //|| sett.name=="FT-747GX" //2.46 may be need to add FT-747GX
            //|| sett.name=="TS-2000"
       )
        mp_v_disp_tx_frq=10.0;
    else if (sett.name=="FT-1000MP" ||
             sett.name=="MARK-V FT-1000MP" ||
             sett.name=="MARK-V Field FT-1000MP"
            )
        mp_v_disp_tx_frq=0.625;
    else if (sett.name=="FT-100")
        mp_v_disp_tx_frq=1.25;
    else
        mp_v_disp_tx_frq=1.0;

    //qDebug()<<sett.name<<mp_v_disp_tx_frq;
    if (mp_v_disp_tx_frq!=1.0) SetTxFreq((double)v_disp_tx_frq);

}
void HvRigControl::SetupPtt(QString str)
{
    if (str.toInt()==0) rb_dtr->setChecked(true);
    if (str.toInt()==1) rb_rts->setChecked(true);
    if (str.toInt()==2) rb_cat->setChecked(true);
    if (str.toInt()==3) rb_ptt_off->setChecked(true);
}
void HvRigControl::ReadDataRtsOnChanget(bool f)
{
    if (f)
    {
        rb_rts->setEnabled(false);
        if (rb_rts->isChecked())
        {
            if (s_rig_name=="SEA-235") rb_cat->setChecked(true);//sea-235
            else if (cb_read_data_kenwood_dtr_on->isChecked()) rb_ptt_off->setChecked(true);
            else rb_dtr->setChecked(true);
        }
        Set_Rts(true);
    }
    else
    {
        rb_rts->setEnabled(true);
        Set_Rts(false);
    }
    ////// omnirig /////////////
#if defined _WIN32_
    if (omnirig_active)
    {
        bool f[10];
        f[0] = rb_ptt_off->isChecked();
        f[1] = rb_rts->isChecked();
        f[2] = rb_dtr->isChecked();
        f[3] = rb_cat->isChecked();
        f[4] = cb_read_data_kenwood_rts_on->isChecked();
        f[5] = cb_read_data_kenwood_dtr_on->isChecked();
        emit EmitOmniRigPttMethod(f);
    }
#endif
    ////// omnirig end /////////////
}
void HvRigControl::ReadDataDtrOnChanget(bool f)
{
    if (f)
    {
        rb_dtr->setEnabled(false);
        if (rb_dtr->isChecked())
        {
            if (s_rig_name=="SEA-235") rb_cat->setChecked(true);//sea-235
            else if (cb_read_data_kenwood_rts_on->isChecked()) rb_ptt_off->setChecked(true);
            else rb_rts->setChecked(true);
        }
        Set_Dtr(true);
    }
    else
    {
        rb_dtr->setEnabled(true);
        Set_Dtr(false);
    }
    ////// omnirig /////////////
#if defined _WIN32_
    if (omnirig_active)
    {
        bool f[10];
        f[0] = rb_ptt_off->isChecked();
        f[1] = rb_rts->isChecked();
        f[2] = rb_dtr->isChecked();
        f[3] = rb_cat->isChecked();
        f[4] = cb_read_data_kenwood_rts_on->isChecked();
        f[5] = cb_read_data_kenwood_dtr_on->isChecked();
        emit EmitOmniRigPttMethod(f);
    }
#endif
    ////// omnirig end /////////////
}
void HvRigControl::SetupReadDataRtsOn(QString str)// from settings
{
    if (str=="3")
    {
        cb_read_data_kenwood_dtr_on->setChecked(true);
        cb_read_data_kenwood_rts_on->setChecked(true);
    }
    else if (str=="2") cb_read_data_kenwood_dtr_on->setChecked(true);
    else if (str=="1") cb_read_data_kenwood_rts_on->setChecked(true);
    else
    {
        cb_read_data_kenwood_dtr_on->setChecked(false);
        cb_read_data_kenwood_rts_on->setChecked(false);
    }
}
void HvRigControl::SetPortName(QString str)
{
    int index = cb_port->findText(str, Qt::MatchCaseSensitive);
    if (index >= 0)
        cb_port->setCurrentIndex(index);
    else
        cb_port->setCurrentIndex(0);
}
void HvRigControl::SetPortBaud(QString str)
{
    int index = cb_baud->findText(str, Qt::MatchCaseSensitive);
    if (index >= 0)
        cb_baud->setCurrentIndex(index);
}

void HvRigControl::SetRigName(QString s)//from read settings
{
    THvRigCat->SetRigName(s);
}
void HvRigControl::SetPttCatType(QString s)
{
    THvRigCat->SetPttCatType(s);
}
void HvRigControl::DestroyPort()
{
    if (port->isOpen() || net_active)//tci=net_active  || net_active
    {
        //SetPtt_p(false, 1);//id 0=All 1=p1 2=p2
        THvRigCat->StartStopPollingTimer(false);
        //SetStaticTxF(false);
        //port->reset();
        port->close();
        //qDebug("is open: %d", port->isOpen());
    }
    if (port2->isOpen())
    {
        //port2->reset();
        port2->close();
    }

    usleep(100000);

    delete port;
    //port = NULL;
    delete port2;
    //port2 = NULL;
}

////////////////////////////////////////////////////////new read com
/*void HvRigControl::set_freq()// za mahane
{
    SetFreq(lefrq->text());// za mahane
}
void HvRigControl::set_mode()// za mahane
{
    SetMode("USB");// za mahane //only USB
}*/
void HvRigControl::SetPtt_p(bool flag,int id)//2.17 id 0=All 1=p1 2=p2
{
    if (id==0 || id==1)//id 0=All 1=p1 2=p2
    {
        ////// omnirig /////////////
        if (omnirig_active || net_active)////// net //////////
            THvRigCat->set_ptt(flag,false);
        else
        {
            if (rb_rts->isChecked())
                Set_Rts(flag);
            if (rb_dtr->isChecked())
                Set_Dtr(flag);
            if (rb_cat->isChecked() && port->isOpen())
                THvRigCat->set_ptt(flag,false);
        }
        ////// omnirig end /////////////
    }
    if (id==0 || id==2)//id 0=All 1=p1 2=p2
    {
        if (rb_rts2->isChecked())
            Set_Rts2(flag);
        if (rb_dtr2->isChecked())
            Set_Dtr2(flag);
    }
}
void HvRigControl::SetQSOProgress(int i) //2.45
{
    if (i==6) f_qso7tx = true;
    else f_qso7tx = false;     //qDebug()<<"SetQSOProgress="<<i<<f_qso7tx;
}
void HvRigControl::SetQrgParms(QString s,bool f)//2.45
{
    static bool _fprevqrg_ = false;
    f_qrg_tx = f; //qDebug()<<f;

    QString _gfreq_ = QString("%1").arg(curr_rig_tx_frq);
    if (f)
    {
        if (_fprevqrg_ != f)
        {
            prev_qrg = _gfreq_;
            emit EmitQrgInfoFromCat(prev_qrg);
        }
        QString newqrg = _gfreq_;
        QString qrg = QString("%1").arg(s.toInt(),3,10,QChar('0'));
        int beg = newqrg.count() - 6;
        newqrg.replace(beg,3,qrg);
        SetFreq(newqrg,3);//2.74 0=mybe frq,no mod, 1=mybe frq,mod, 2=frq,mod, 3=frq,no mod//2.51 true= ignore only button F flag from Interface Control
    }
    else if (!prev_qrg.isEmpty() && prev_qrg!=_gfreq_)//else if (!_prevqrg_.isEmpty() && _prevqrg_!=FREQ_GLOBAL)
        SetFreq(prev_qrg,3);//2.74 0=mybe frq,no mod, 1=mybe frq,mod, 2=frq,mod, 3=frq,no mod//2.51 true= ignore only button F flag from Interface Control
    _fprevqrg_ = f;
    //qDebug()<<"SetQrgParms="<<s<<f<<prev_qrg<<_gfreq_;
}
//int prevt = 0;
//int currt = 0;
void HvRigControl::TimerTryStaticTxF()
{
    if (f_txrx_static_tx)//try go to TX
    {
        timer_try_static_tx->stop();
        SetPtt_p(true,ss_id);
        //qDebug()<<"--> TX FREQ OK TRY="<<"1"<<QTime::currentTime().toString("ss:zzz");
        //qDebug()<<"------------------------------";
        curr_rig_tx_frq = last_curr_rig_tx_frq;//for any case if no succsess ft857 exception
        f_block_display_static_tx = false;
        timer_try_static_tx_isActive = false;
    }
    else //try go to RX
    {
        int increase = 50;
        //int try_max=4;   //try to stop   4=/srart=70ms inc=50ms try=4 All=1136ms/
        int try_max = 3;   //try to stop   3=/srart=70ms inc=50ms try=3 All=892ms /
        if (curr_rig_tx_frq==last_curr_rig_tx_frq || max_try_off_static_tx>=try_max)
        {
            timer_try_static_tx->stop();
            if (curr_rig_tx_frq!=last_curr_rig_tx_frq && max_try_off_static_tx>=try_max)//if no no succsess last SET
            {
                usleep(increase*1000);
                SetFreq(QString("%1").arg(last_curr_rig_tx_frq),3);
                curr_rig_tx_frq = last_curr_rig_tx_frq;//for any case if no succsess ft857 exception
                //currt = QTime::currentTime().toString("sszzz").toInt();
                //qDebug()<<"<-1 TRY TO RX SET"<<currt-prevt; prevt=currt;
            }
            //currt = QTime::currentTime().toString("sszzz").toInt();  currt-=prevt;
            //if (max_try_off_static_tx>=try_max)//no succsess   exeption ???
            //qDebug()<<"<-- RX FREQ OK MAX="<<max_try_off_static_tx<<QTime::currentTime().toString("ss:zzz");//<<currt;
            //else
            //qDebug()<<"<-- RX FREQ OK TRY="<<max_try_off_static_tx<<QTime::currentTime().toString("ss:zzz");//<<currt;
            //qDebug()<<"------------------------------";
            max_try_off_static_tx = 0;
            corr_app_ind_tx_frq = 0;//for any case
            gb_static_tx->setEnabled(true);
            emit EmitTxActive(0);//0=off TX, 1=normal TX, 2=Static TX
            f_static_tx_active = false;//important need to be here for fast tx/rx
            f_block_display_static_tx = false;
            timer_try_static_tx_isActive = false;
            corr_ms_static_tx = 0;
            f_msf = f_msf_new; //qDebug()<<"TimerTryStaticTxF()="<<f_msf_special_tx_rx;
            if (f_msf_special_tx_rx==2) RbSetStaticTxF(false);
            f_msf_special_tx_rx=0;
        }
        else
        {
            f_block_display_static_tx = true;// need to be here
            max_try_off_static_tx++;
            int ms_100 = increase + 50;
            if (max_try_off_static_tx % try_max != 0)//+1 takt rezerve tested ft991 i ft857  5
            {
                ms_100 =  50 + corr_ms_static_tx;
                SetFreq(QString("%1").arg(last_curr_rig_tx_frq),3);
                //currt = QTime::currentTime().toString("sszzz").toInt();
                //qDebug()<<"<-2 TRY TO RX SET"<<currt-prevt; prevt=currt;
            }
            usleep(ms_100*1000);
            corr_app_ind_tx_frq = 0;
            THvRigCat->get_freq();
            //currt = QTime::currentTime().toString("sszzz").toInt();
            //qDebug()<<"<-- TRY TO RX GET"<<currt-prevt; prevt=currt;
            //qDebug()<<"--------------------";
            if (max_try_off_static_tx % try_max == 0) corr_ms_static_tx += increase;
            timer_try_static_tx->start(corr_ms_static_tx);
        }
    }
}
void HvRigControl::SetPtt(bool flag, int id)//id 0=All 1=p1 2=p2
{
    bool all_sttx = false;  //ft8  ft4 q65
    if (f_static_tx && all_static_tx_modes) all_sttx = true;
    bool all_qrg = false;  //msk fsk
    if (f_qrg_tx && f_qso7tx) all_qrg = true; //qDebug()<<f_static_tx<<f_qrg_tx;

    if (f_msf_special_tx_rx<2 || !f_static_tx)//f_msf_special_tx_rx 0=RX, 1=TX, 2=TXtoRX after end TX
    {
        if (flag) f_msf_special_tx_rx=1;
        else f_msf_special_tx_rx=0; //qDebug()<<"SetPtt()="<<f_msf_special_tx_rx<<"f_static_tx="<<f_static_tx;
    }

    if (all_sttx || all_qrg)//to TX
    {
        if (flag && !f_static_tx_active)// && timer_off_static_tx->->isActive()
        {
            //qDebug()<<"--> TX PTT==-START-=="<<QTime::currentTime().toString("ss:zzz");
            //prevt = QTime::currentTime().toString("sszzz").toInt();

            f_static_tx_active = true;
            timer_try_static_tx_isActive = true;
            gb_static_tx->setEnabled(false);
            emit EmitTxActive(2);//0=off TX, 1=normal TX, 2=Static TX

            f_block_display_static_tx = true;
            if (all_sttx)
            {
                int fst = sb_static_tx->value();//2.76sf
                if (s_mode==11 && f_msf) fst = 750;
                long long int static_tx_f = (long long int)fst;
                last_curr_rig_tx_frq = curr_rig_tx_frq - (static_tx_f - v_disp_tx_frq);
                last_v_disp_tx_frq = v_disp_tx_frq;
                corr_app_ind_tx_frq = (static_tx_f - v_disp_tx_frq);
            }
            if (all_qrg)
            {
                rx_qrg = curr_rig_tx_frq;
                last_curr_rig_tx_frq = prev_qrg.toLong();
                corr_app_ind_tx_frq = (rx_qrg - last_curr_rig_tx_frq);
            }

            SetFreq(QString("%1").arg(last_curr_rig_tx_frq),3);
            //currt = QTime::currentTime().toString("sszzz").toInt();
            //qDebug()<<"--> TRY TO TX SET"<<QTime::currentTime().toString("ss:zzz")<<currt-prevt; prevt=currt;

            ss_id = id;
            f_txrx_static_tx = true;//try go to TX
            timer_try_static_tx->start(70);//1 loop only 70ms    12 - 30
            return;
        }
    }

    SetPtt_p(flag,id);//2.17

    if (all_sttx || f_qrg_tx) //to RX
    {
        //for only once protection !timer_off_static_tx->isActive()
        if (!flag && f_static_tx_active && !timer_try_static_tx_isActive)//!timer_try_static_tx->isActive()
        {
            //qDebug()<<"<-- RX PTT==-START-=="<<QTime::currentTime().toString("ss:zzz");
            //prevt = QTime::currentTime().toString("sszzz").toInt();

            timer_try_static_tx_isActive = true;

            if (all_sttx)
            {
                int fst = sb_static_tx->value();//2.76sf
                if (s_mode==11 && f_msf) fst = 750;
                long long int static_tx_f = (long long int)fst;
                last_curr_rig_tx_frq = curr_rig_tx_frq + (static_tx_f - last_v_disp_tx_frq);
            }
            if (f_qrg_tx)
            {
                last_curr_rig_tx_frq = rx_qrg;
            }

            f_txrx_static_tx = false;//try go to RX
            corr_ms_static_tx = 70;//start at 70+50->SetFreq()=120
            timer_try_static_tx->start(corr_ms_static_tx);
        }
        else if (!flag && !timer_try_static_tx_isActive)
        {//exception from fsk441 to ft8/4 on TX with f_static_tx and !f_static_tx_active
            gb_static_tx->setEnabled(true);
            emit EmitTxActive(0);//0=off TX, 1=normal TX, 2=Static TX
        }
    }
    else
    {
        if (flag)
        {
            gb_static_tx->setEnabled(false);
            emit EmitTxActive(1);//0=off TX, 1=normal TX, 2=Static TX
        }
        else
        {
            if (rig_cat_active_and_read) gb_static_tx->setEnabled(true);//2.30
            emit EmitTxActive(0);//0=off TX, 1=normal TX, 2=Static TX
        }
    }
}
//////////////////////////////////////////////////////////////////
void HvRigControl::SetOffsetTrsvRigParms(QString s)
{
    QStringList ls=s.split("#");

    QStringList lsm;
    for (int i=0; i<COUNT_BANDS; i++) lsm << lst_bands[i]+"=";

    for (int i=0; i<COUNT_BANDS; i++)
    {
        QString tstr = lsm[i];
        for (int j=0; j<ls.count(); j++)
        {
            if (ls[j].contains(tstr))
            {
                ls[j].remove(tstr);
                QStringList lss = ls[j].split("|");
                if (lss.count()==2)
                {
                    offset_trsv_rig[i].rb_id = lss.at(0).toInt();
                    long long int ioff = lss.at(1).toLongLong();//2.74
                    if (ioff==0) offset_trsv_rig[i].freq = "";
                    else offset_trsv_rig[i].freq = QString("%1").arg(ioff);//2.74
                }
                break;
            }
        }
    } //qDebug()<<"offset_trsv_rig="<<offset_trsv_rig[11].freq<<s_id_band;
    if (s_id_band==17)//2.76.5 17 default HV 70 MHz 
    {
        s_id_band=-1;
        SetBand("70 MHz");//for exception
    }
}
QString HvRigControl::get_offset_trsv_rig_parms()
{
    QString sada;
    for (int i=0;  i<COUNT_BANDS; i++)
    {
        //sada.append(offset_trsv_rig[i].band);
        sada.append(lst_bands[i]);
        sada.append("=");
        sada.append(QString("%1").arg(offset_trsv_rig[i].rb_id));
        sada.append("|");
        if (offset_trsv_rig[i].freq.isEmpty())
            sada.append("0");
        else
            sada.append(offset_trsv_rig[i].freq);
        if (i<COUNT_BANDS-1)
            sada.append("#");
    }
    return sada;
}
void HvRigControl::LeOffsetTrsvRigChange(QString s)
{
    offset_trsv_rig[s_id_band].freq = s;
    //qDebug()<<"LE Change";
    SetFreqOffsetTR();
}
void HvRigControl::RbOffsetTR(bool)
{
    int id = 0;
    if (rb_offset_tr_off->isChecked())
        id = 0;
    if (rb_offset_tr_sum->isChecked())
        id = 1;
    if (rb_offset_tr_sub->isChecked())
        id = 2;

    if (id==0)
        le_offset_trsv_rig->setEnabled(false);
    else
        le_offset_trsv_rig->setEnabled(true);

    offset_trsv_rig[s_id_band].rb_id = id;
    //qDebug()<<"RB Change";
    SetFreqOffsetTR();
}
void HvRigControl::SetBand(QString band)
{
    //qDebug()<<"FFFFFF="<<offset_trsv_rig[11].freq<<s_id_band;
    for (int i = 0; i<COUNT_BANDS; ++i)
    {
        if (band == lst_bands[i])
        {
            if (s_id_band==i) return;

            s_id_band = i;
            int trb_id = offset_trsv_rig[i].rb_id;

            le_offset_trsv_rig->setText(offset_trsv_rig[i].freq);
            if (trb_id==0) rb_offset_tr_off->setChecked(true);
            if (trb_id==1) rb_offset_tr_sum->setChecked(true);
            if (trb_id==2) rb_offset_tr_sub->setChecked(true);
            break;
        }
    }
    gb_troffs->setTitle(tr("Transverter Local Oscillator Or RIG Offset:   For Band")+" "+band);
    //qDebug()<<"Band Change";
    SetFreqOffsetTR();
}
void HvRigControl::SetFreqOffsetTR()
{
    if (rb_offset_tr_off->isChecked())
        FreqOffsetTR = "";
    else
        FreqOffsetTR = offset_trsv_rig[s_id_band].freq;

    QString frq_b = lst_bands[s_id_band];
    long long int inf = 0;
    long long int lof = FreqOffsetTR.toLongLong();
    //double t_frq = 0.0;

    if (frq_b.contains(" kHz"))
    {
        frq_b.remove(" kHz");
        double t_frq = frq_b.toDouble();
        inf = t_frq * 1000;
    }
    else if (frq_b.contains(" MHz"))
    {
        frq_b.remove(" MHz");
        double t_frq = frq_b.toDouble();
        inf = t_frq * 1000000;
    }
    else if (frq_b.contains(" GHz"))
    {
        frq_b.remove(" GHz");
        double t_frq = frq_b.toDouble();
        inf = t_frq * 1000000000;
    }
    if (rb_offset_tr_sum->isChecked())
    {
        if (lof>inf)
            inf = lof - inf;
        else
            inf = inf - lof;
    }
    else if (rb_offset_tr_sub->isChecked())
    {
        inf = lof + inf;
    }
    frq_b = (QString("%1").arg(inf));
    int k = frq_b.count();
    k -= 3;
    if (k>0)
        frq_b.insert(k,".");
    k -= 3;
    if (k>0)
        frq_b.insert(k,".");
    le_offset_trsv_rig_res->setText("RIG: "+frq_b+" Hz");
    //qDebug()<<"FreqOffsetTR="<<FreqOffsetTR<<
    //rb_offset_tr_off->isChecked()<<rb_offset_tr_sum->isChecked()<<rb_offset_tr_sub->isChecked();
}
//////////////////////////////////////////////////////////////
void HvRigControl::CbOffSetFrqToRig(bool f)
{
    if (f)
    {
        cb_mod_set_frq_to_rig->setEnabled(false);
        cb_mod_set_frq_to_rig->setChecked(false);
    }
    else
    {
        cb_mod_set_frq_to_rig->setEnabled(true);
    }
}
void HvRigControl::SetModSetFrqToRig(QString str)
{
    QStringList l = str.split("#");
    l<<"0";
    if 		(l.at(0)=="0") cb_mod_set_frq_to_rig->setChecked(false);//2.53 default on
    if 		(l.at(1)=="1") cb_rig_mod->setCurrentIndex(1);
    else if (l.at(1)=="2") cb_rig_mod->setCurrentIndex(2);
    //qDebug()<<"mode"<<str;
}
void HvRigControl::SetOffSetFrqToRig(QString str)
{
    if (str=="1") cb_off_set_frq_to_rig->setChecked(true);
    //qDebug()<<"off mode"<<str;
}
#define TSRMI 80
#define TSTCI 300
void HvRigControl::SetRigModeFreq()
{
    if (set_rig_mode_id==1)
    {
        set_rig_mode_id=2;
        int mo = cb_rig_mod->currentIndex();
        if (mo==1) THvRigCat->set_mode("USB");
        if (mo==2) THvRigCat->set_mode("DIGU");
        if (s_net_model_id==3 || s_net_model_id==4) timer_set_rig_mode->start(TSTCI);
        else timer_set_rig_mode->start(TSRMI); //qDebug()<<"SetModeToRIG"<<cb_rig_mod->currentText();
    }
    else if (set_rig_mode_id==2)
    {
        set_rig_mode_id=0;
        timer_set_rig_mode->stop();//2.74 0=mybe frq,no mod, 1=mybe frq,mod, 2=frq,mod, 3=frq,no mod
        SetFreq(s_set_rig_mode_frq,3); //qDebug()<<"SetFrqToRIG 2="<<s_set_rig_mode_frq;
    }
}
void HvRigControl::SetFreq(QString str,int id)
{
    //qDebug()<<"SetFreq-------->"<<str<<cb_off_set_frq_to_rig->isChecked()<<id;
    //if (cb_off_set_frq_to_rig->isChecked() && !f) return;//2.16 f true set from button F or static_tx
    if (cb_off_set_frq_to_rig->isChecked() && id<2) return; //2.74 0=mybe frq,no mod 1=mybe frq,mod 2=frq,mod 3=frq,no mod

    //no if TX false and timer_off_static_tx->isActive() false
    if (!f_static_tx_active && !timer_try_static_tx_isActive) curr_rig_tx_frq = str.toLongLong();//!timer_try_static_tx->isActive()

    if (rb_offset_tr_off->isChecked())
    {
        THvRigCat->set_freq(str);
        //qDebug()<<"APP -> RIG SET FREQ="<<str;
    }
    else
    {
        ///offset from APP to RIG /////////////////
        long long int res = 0;
        long long int lof = FreqOffsetTR.toLongLong();
        long long int inf = str.toLongLong();
        if (rb_offset_tr_sum->isChecked())
        {
            if (lof>inf) res = lof - inf;
            else res = inf - lof;
        }
        else if (rb_offset_tr_sub->isChecked()) res = lof + inf;

        THvRigCat->set_freq(QString("%1").arg(res));
        //qDebug()<<"APP -> RIG SET FREQ="<<res;
    }
    if (rig_cat_active_and_read && (id==1 || id==2) && cb_rig_mod->currentIndex()>0)
    {	//2.74 0=mybe frq,no mod, 1=mybe frq,mod, 2=frq,mod, 3=frq,no mod
        set_rig_mode_id=1;
        s_set_rig_mode_frq = str;
        if (s_net_model_id==3 || s_net_model_id==4) timer_set_rig_mode->start(TSTCI);
        else timer_set_rig_mode->start(TSRMI); //qDebug()<<"SetFrqToRIG 1="<<s_set_rig_mode_frq;
    }
}
void HvRigControl::SetGetedFreq(QString str)
{
    long long int res = 0;
    if (rb_offset_tr_off->isChecked()) res = str.toLongLong();
    else
    {
        //offset from RIG -> APP
        long long int lof = FreqOffsetTR.toLongLong();
        long long int inf = str.toLongLong();
        if (rb_offset_tr_sum->isChecked())
            res = lof + inf;
        else if (rb_offset_tr_sub->isChecked())
        {
            if (lof>inf)
                res = lof - inf;
            else
                res = inf - lof;
        }
    }

    curr_rig_tx_frq = res;

    if (!f_block_display_static_tx)
    {
        long long int end_rs = curr_rig_tx_frq;
        long long int end_rs_rig = str.toLongLong();//2.68
        //if (f_static_tx) // no need
        end_rs += corr_app_ind_tx_frq;  //corr APP indication for spot and pskrep
        end_rs_rig += corr_app_ind_tx_frq;//2.68
        if (end_rs0 != end_rs)//2.55
        {
            end_rs0 = end_rs;
            emit EmitGetedFreq(QString("%1").arg(end_rs)+"#"+QString("%1").arg(end_rs_rig));//2.68 +"#"+QString("%1").arg(end_rs_rig)
        }
    }
    if (!f_static_tx_active && f_qrg_tx) emit EmitQrgFromRig(QString("%1").arg(curr_rig_tx_frq));
}
/*void HvRigControl::SetMode(QString str)//stop for the moment
{
    THvRigCat->set_mode(str);//only USB
}*/
void HvRigControl::SetGetedMode(QString s)
{
    //transceiver real freq
    //lmod->setText("TRANSCEIVER REAL MODE: "+s);//za mahane
    emit EmitGetedMode(s);
}
////////////////////////////////////////////////////////end new read com

//////// Port2 ////////////////////////////////////////////////////////
void HvRigControl::SetEnabledAll2(bool f)
{
    rb_dtr2->setEnabled(f);
    cb_baud2->setEnabled(f);
    cb_port2->setEnabled(f);
    rb_rts2->setEnabled(f);
}
void HvRigControl::TestPtt2()
{
    if (!f_test_port2)
    {
        f_test_port2 = true;
        SetPtt(true,2);//id 0=All 1=p1 2=p2
        pb_test_port2->setText(" "+tr("STOP PTT TEST")+" ");
        pb_test_port2->setStyleSheet("QPushButton{background-color:rgb(255,128,128);}");
        SetEnabledAll2(false);
    }
    else
    {
        f_test_port2 = false;
        SetPtt(false,2);//id 0=All 1=p1 2=p2
        pb_test_port2->setText(" "+tr("START PTT TEST")+" ");
        pb_test_port2->setStyleSheet("QPushButton{background-color:palette(Button);}");
        SetEnabledAll2(true);
    }
}
void HvRigControl::Set_Rts2(bool flag)
{
    if (port2->isOpen()) port2->setRts(flag);//inportant its protected from port->isOpen()
}
void HvRigControl::Set_Dtr2(bool flag)
{
    if (port2->isOpen()) port2->setDtr(flag);//inportant its protected from port->isOpen()
}
void HvRigControl::PortNameChanged2(QString str)
{
    if (port2->isOpen()) port2->close();

    port2->setTimeout(500);

    port2->setPortName(str);
    port2->open(QIODevice::ReadWrite);
    Set_Dtr2(false);
    Set_Rts2(false);

    if (port2->isOpen())
    {
        pb_test_port2->setEnabled(true);
        pb_test_port2->setText(" "+tr("START PTT TEST")+" ");
    }
    else
    {
        pb_test_port2->setEnabled(false);
        if (str=="None") pb_test_port2->setText(" "+tr("START PTT TEST    NO PORT SELECTED")+" ");
        else pb_test_port2->setText(" "+tr("START PTT TEST    PORT")+" : "+str+" "+tr("IS BUSY")+" ");
    }
}
void HvRigControl::BaudRateChanged2(QString str)
{
    //QString s_port_name = port->portName();
    if (port2->isOpen())// important
    {
        usleep(200000);
    }

    switch (str.toInt())
    {
    case 300:
        port2->setBaudRate(BAUD300);
        break;
    case 600:
        port2->setBaudRate(BAUD600);
        break;
    case 1200:
        port2->setBaudRate(BAUD1200);
        break;
    case 2400:
        port2->setBaudRate(BAUD2400);
        break;
    case 4800:
        port2->setBaudRate(BAUD4800);
        break;
    case 9600:
        port2->setBaudRate(BAUD9600);
        break;
        //case 14400:
        //port2->setBaudRate(BAUD14400);
        //break;
    case 19200:
        port2->setBaudRate(BAUD19200);
        break;
    case 38400:
        port2->setBaudRate(BAUD38400);
        break;
    case 57600:
        port2->setBaudRate(BAUD57600);
        break;
    case 115200:
        port2->setBaudRate(BAUD115200);
        break;
    default:
        port2->setBaudRate(BAUD9600);
        break;
    }
    Set_Dtr2(false);
    Set_Rts2(false);
}
void HvRigControl::PttChanget2(bool)
{
    Set_Rts2(false);
    Set_Dtr2(false);
    if (f_test_port2)
    {
        f_test_port2 = false;
        SetPtt(false,2);//id 0=All 1=p1 2=p2
        pb_test_port2->setText(" "+tr("START PTT TEST")+" ");
        pb_test_port2->setStyleSheet("QPushButton{background-color:palette(Button);}");
        SetEnabledAll2(true);
    }
}
void HvRigControl::SetPortName2(QString str)
{
    int index = cb_port2->findText(str, Qt::MatchCaseSensitive);
    if (index >= 0) cb_port2->setCurrentIndex(index);
    else cb_port2->setCurrentIndex(0);
}
void HvRigControl::SetPortBaud2(QString str)
{
    int index = cb_baud2->findText(str, Qt::MatchCaseSensitive);
    if (index >= 0) cb_baud2->setCurrentIndex(index);
}
void HvRigControl::SetupPtt2(QString str)
{
    if (str.toInt()==0) rb_dtr2->setChecked(true);
    if (str.toInt()==1) rb_rts2->setChecked(true);
}