/* MSHV Part from RigControl
 * Copyright 2019 Hrisimir Hristov, LZ2HV
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
#define _NETWORK_RIGS_
#include "network_def.h"
#include "network.h"

#include <unistd.h> 
#include <QLocale>

//----- vita49 ---------------------------------------------------
#include <QCoreApplication> //flex native vita-49: aboutToQuit
#include <QMutex>        //flex native vita-49
#include <QMutexLocker>  //flex native vita-49
#include <QRegExp>       //flex native vita-49
#include <QMetaObject>   //flex native vita-49
#include <QAtomicInt>    //flex native vita-49
#include <math.h>        //flex native vita-49
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
//----- end vita49 ---------------------------------------------------

//SdrApplication.exe --serial=EED05101000001
//SdrApplication.exe --serial=EED05231500040

//#include <QtGui>

//tci
HvWebSocket::HvWebSocket(quint32 trxx)
{
    tci_trx = trxx;
    sample_rate = 48000;
    pos_writ = 0;
    pos_read = 0;
    for (int i = 0; i<BCMAX; ++i) wcommands[i]="";
    t_txAudioData.clear();
    connect(this,SIGNAL(binaryMessageReceived(QByteArray)),this,SLOT(mBinaryMessageReceived(QByteArray)));
    //connect(this,SIGNAL(error(QAbstractSocket::SocketError)),this,SLOT(onError(QAbstractSocket::SocketError)));
    ThreadRefr *TThreadRefr = new ThreadRefr;
    connect(TThreadRefr, SIGNAL(refresh()), this, SLOT(ReadCommand()));
    //connect(TThreadRefr, SIGNAL(finished()), TThreadRefr,SLOT(deleteLater()));
    TThreadRefr->start(); //qDebug()<<"NEW===================================RX="<<(quint32)tci_trx<<"WPos="<<pos_writ<<"RPos="<<pos_read;
}
HvWebSocket::~HvWebSocket()
{
    //qDebug()<<"TCI Delete";
}
/*void HvWebSocket::onError(QAbstractSocket::SocketError errorCode)
{
    //std::cout << "Error: " << errorCode << std::endl;
    const QString err[100]=
        {"ConnectionRefused",
         "RemoteHostClosed",
         "HostNotFound",
         "SocketAccess",
         "SocketResource",
         "SocketTimeout",
         "DatagramTooLarge",
         "Network",
         "AddressInUse",
         "SocketAddressNotAvailable",
         "UnsupportedSocketOperation",
         "UnfinishedSocketOperation",
         "ProxyAuthenticationRequired",
         "SslHandshakeFailed",
         "ProxyConnectionRefused",
         "ProxyConnectionClosed",
         "ProxyConnectionTimeout",
         "ProxyNotFound",
         "ProxyProtocol",
         "Operation",
         "SslInternal",
         "SslInvalidUserData",
         "Temporary",
         "UnknownSocket"};
    if (errorCode<=-1)
    {
        qDebug()<<"UnknownSocketError";//QAbstractSocket::UnknownSocketError -1
        return;
    } qDebug()<<"Error="<<err[errorCode];
}*/
void HvWebSocket::SetBufferCommand(QString s)
{
    wcommands[pos_writ]=s; //qDebug()<<"IN Command=------------------------------->"<<s;
    pos_writ++;
    if (pos_writ>BCMAX-1) pos_writ = 0;
}
void HvWebSocket::ReadCommand()
{
    uint8_t twrit = pos_writ;
    int diff = twrit - pos_read;
    if (twrit<pos_read) diff = twrit + BCMAX - pos_read;
    if (diff<1) return;
    for (uint8_t i = 0; i < diff; ++i)
    {
        QString s = wcommands[pos_read]; //uint8_t t_pos_read = pos_read;
        pos_read++;
        if (pos_read>BCMAX-1) pos_read = 0;
        if (s.count()<1) continue;//2.76.2
        QChar c = s.at(0);
        /*if (c=='0')
        {
        	QStringList l = s.mid(1,s.count()-1).split(" ");
        	QUrl url; //QUrl t_url("ws://" + leAddress->text() + ":" + QString::number(sbPort->value()));
        	url.setUrl("ws://" + l.at(0));
        	url.setHost(l.at(0));
        	url.setPort(l.at(1).toInt());
        	open(url);    
        }*/
        if 		(c=='1') close(QWebSocketProtocol::CloseCodeNormal,"end"); //close
        else if (c=='2') flush();
        else sendTextMessage(s);
        //printf("MSHV Send--> ReadAddres= %2d TCI RX= %d Command= %s\n",t_pos_read,tci_trx,qPrintable(s));
    } //printf("Count= %2d WPos= %2d RPos= %2d\n---------------------------------\n",diff,twrit,pos_read);
}
typedef enum
{
    IqInt16 = 0,
    IqInt24,
    IqInt32,
    IqFloat32,
    IqFloat64
}IqDataType;
typedef enum
{
    IqStream = 0,
    RxAudioStream,
    TxAudioStream,
    TxChrono,
}StreamType;
typedef struct
{
    uint32_t receiver; // receiver number
    uint32_t sampleRate; // sampling rate
    uint32_t format; // always equal to 4 (float 32 bit)
    uint32_t codec; // compression algorithm (not implemented), always 0
    uint32_t crc; // checksum (not implemented), always 0
    uint32_t length; // number of samples
    uint32_t type; // stream type
    uint32_t channels; // number of channels
    uint32_t reserv[8]; // reserved
    uint8_t data[DSMAX+4096]; // samples
}
DataStream;
//QElapsedTimer ttt;
#define STREAM_C 4096 //2.70
#define BUF_OFFSET STREAM_C*4	//2.57
#define BUF_MAX 50*STREAM_C		//2.57=50 28=1.1s 40=1.6s 50=2.1s  60=2.5s 72=3.0s 94=4.0s 120=5.1s
#define BUF_MAXOFFSET BUF_MAX - BUF_OFFSET
static int _flush_raw_   = 2;
static int rawtxbuff[BUF_MAX+STREAM_C*4+4096];
static int rawtxbuff_pos = 0;
static int rawtx_pos     = 0;
static int diff_offs     = 0;
static int _reset_sta_   = 0;
static int exit_txaudio  = 0;
void _SetTciBuffReset_()
{
    for (int i = 0; i < BUF_MAX; ++i) rawtxbuff[i] = 0;
    rawtx_pos     = 0;
    rawtxbuff_pos = 0;
    diff_offs     = BUF_MAXOFFSET;//xeception no start
    _reset_sta_   = 0;
    _flush_raw_   = 2;
    //qDebug()<<"_SetTciBuffReset_==============="<<STREAM_C<<BUF_OFFSET<<BUF_MAXOFFSET<<BUF_MAX;
}

//----- vita49 ---------------------------------------------------
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
//----- end vita49 ---------------------------------------------------

bool _SetTxAudioTci_(int *raw)//, int size
{
    //----- vita49 ---------------------------------------------------
    // Rawplayer feeds every network transport through this one entry point;
    // hand the block to the Flex backend when that owns the output device.
    if (_FlexVitaTxActive_()) return _SetTxAudioFlex_(raw);
    //----- end vita49 ---------------------------------------------------

    exit_txaudio = 750;   //2.57 old=800x5000=4,5s old=900x4000=4,5s  max-wait-time ic53=3200 pG10=3900 ic57=3600 ic59=3700
    while (1)  //max-wait-time need to be > max buffer time
    {
        usleep(5000); //2.57 <- not correct experiment needed
        if (_flush_raw_ > 1)
        {
            _flush_raw_--;
            exit_txaudio = _flush_raw_ - 1;
        }
        else if (_flush_raw_==1) exit_txaudio = 0;

        if (exit_txaudio<1) break;
        exit_txaudio--;
    }
    if (_flush_raw_==0)
    {
        //if (exit_txaudio==0) qDebug()<<"Long time SDR no read samples > 4sec";
        return true;
    }

    if (_reset_sta_ == 0) //reset exeption
    {
        _reset_sta_ = 1;
        rawtxbuff_pos = 0;
        rawtx_pos = 0;
        diff_offs = BUF_MAXOFFSET;
        //qDebug()<<" --- RESET ----------------------------------------->"<<diff_offs<<BUF_MAX;
        //ttt.start();
    }
    if (_reset_sta_ < 42) _reset_sta_++; //42

    for (int i = 0; i < STREAM_C; ++i) rawtxbuff[i + rawtxbuff_pos] = raw[i];  //to zero 1,005122699
    //if (_reset_sta_ < 5) qDebug()<<"WRITE POS --->"<<rawtxbuff_pos<<rawtxbuff[rawtxbuff_pos]<<diff_offs;
    //if (_reset_sta_ > 36 && diff_offs>151000) qDebug()<<"buffer offset < 1.5sec"<<diff_offs;
    //if (www>3 && diff_offs>STREAM_C) qDebug()<<"Flush-------------->"<<www;
    //if (_reset_sta_ == 6) qDebug()<<"Start Play="<<ttt.elapsed();
    /*if (diff_offs>STREAM_C*2)
    {
        QString sss;
        for (int i = 0; i < BUF_MAXOFFSET; i+=STREAM_C)
        {
            if (i<diff_offs) sss.append("0");
            else sss.append("*");
        }
        qDebug()<<sss;//<<ttt.elapsed();
        //if (diff_offs>BUF_SPEAD) qDebug()<<sss<<rawtx_pos<<"HIGH";
        //else qDebug()<<sss<<rawtx_pos<<"SLOW";//<<rawtxbuff_pos;
    }*/

    rawtxbuff_pos += STREAM_C;
    if (rawtxbuff_pos > BUF_MAX - STREAM_C) rawtxbuff_pos = 0;

    diff_offs -= STREAM_C;  //if (diff_offs==4096) qDebug()<<"Buffer Full="<<ttt.elapsed();
    if (diff_offs <= 0)
    {
        _flush_raw_ = 0;
        diff_offs = 0;
    }
    //usleep(1000);
    return true;
}
#include "../../../HvMsCore/mscore.h"
static constexpr quint32 IqHeaderSize = 16u*sizeof(quint32); //tci
static bool tci_190 = false;
void HvWebSocket::mBinaryMessageReceived(const QByteArray &ba)
{
    DataStream *pStream = (DataStream*)(ba.data()); //qDebug()<<ba.size()<<pStream->length;
    if (pStream->receiver != tci_trx) return; //if (pStream->receiver != (quint32)tci_trx.toInt()) return;

    int fmt = pStream->format;
    int bit_s = 4;
    if	    (fmt==0) bit_s = 2;
    else if	(fmt==1) bit_s = 3;
    //else if (fmt==2) bit_s = 4;
    //else if (fmt==3) bit_s = 4;
    int chan = 2;
    if (pStream->channels==1) chan = 1;

    if (pStream->type == RxAudioStream)
    {
        if (_GetRxAudioReadTci_() == 0) return;
        int k_rx_resa = 3;//2.76.2 def=48000
        if 		(pStream->sampleRate==24000) k_rx_resa = 1;
        else if (pStream->sampleRate==12000) k_rx_resa = 0;
        int cr2 = (int)pStream->length*bit_s;
        if (cr2 > DSMAX) cr2 = DSMAX; //protection
        int cmono = 0;
        //qDebug()<<DSMAX<<"fmt="<<fmt<<"bits="<<bit_s<<"chan="<<chan<<"length="<<cr2;
        for (int i = 0; i < cr2; i+=chan*bit_s)
        {
            if (fmt==3)
            {	//float32=4
                union {
                    uint8_t a[4];
                    float f;
                } U;
                U.a[0]=pStream->data[i];
                U.a[1]=pStream->data[i+1];
                U.a[2]=pStream->data[i+2];
                U.a[3]=pStream->data[i+3];
                rawrxm[cmono] = (int)(U.f*8388607.0);//rawrxm[cmono] = (int)(U.f*8380000.2);//2.76.2 full=8388607//old rawrxm[cmono] = (int)(U.f*4341632.8);
            }
            else
            {	//For INT types int16=2,int24=3,int32=4
                int z = 0;
                if 		(bit_s==2) z = (pStream->data[i] << 16) + (pStream->data[i+1] << 24);
                else if (bit_s==3) z = (pStream->data[i] <<  8) + (pStream->data[i+1] << 16) + (pStream->data[i+2] << 24);
                else if (bit_s==4) z =  pStream->data[i] + (pStream->data[i+1] <<  8) + (pStream->data[i+2] << 16) + (pStream->data[i+3] << 24);
                z = (z >> 8);//to 24-bit
                rawrxm[cmono] = z;//2.76.2 //old z/2; //2.67 important
                /*if (i==0)
                {
                	int b0,b1,b2,b3;
                	b0  = (z & 0xff);
                	b1 = ((z >>  8) & 0xff);
                	b2 = ((z >> 16) & 0xff);
                	b3 = ((z >> 24) & 0xff);   
                	qDebug()<<b0<<b1<<b2<<b3<<z;                
                }*/
            }
            cmono++;
        }
        _SetRxAudioTci_(rawrxm,cmono,k_rx_resa); //qDebug()<<cmono;
    }
    else if (pStream->type == TxChrono) //else if (pStream->type == IqStream)
    {
        sample_rate = 48000;
        int k_tx_rate = 1;//2.76.2 def=48000
        if 		(pStream->sampleRate==24000)
        {
            sample_rate = 24000;
            k_tx_rate = 2;
        }
        else if (pStream->sampleRate==12000)
        {
            sample_rate = 12000;
            k_tx_rate = 4;
        }
        quint32 cr3 = pStream->length*bit_s;  //plength = plength*pStream->channels*4-bytes
        if (cr3 > DSMAX) cr3 = DSMAX; //protection
        int txasize = IqHeaderSize + cr3*sizeof(uint8_t);
        if (t_txAudioData.size()!=txasize) t_txAudioData.resize(txasize);
        DataStream *t_txStream = (DataStream*)(t_txAudioData.data());
        t_txStream->receiver   = tci_trx;
        t_txStream->sampleRate = sample_rate;
        t_txStream->format     = fmt;
        t_txStream->codec      = 0;
        t_txStream->crc        = 0;
        t_txStream->length     = (cr3/bit_s);	//qDebug()<<pStream->length<<t_txStream->length;
        t_txStream->type       = TxAudioStream;
        if (tci_190) t_txStream->channels = chan;
        for (quint32 i = 0; i < cr3; i+=bit_s)
        {
            if (_reset_sta_ > 5)
            {
                if (fmt==3)
                {	//float32=4
                    union {
                        uint8_t a[4];
                        float f;
                    } U;
                    U.f = (float)rawtxbuff[rawtx_pos]/8380000.0; //2.70 8380000.0 full=8388607
                    t_txStream->data[i]   =	U.a[0];
                    t_txStream->data[i+1] = U.a[1];
                    t_txStream->data[i+2] = U.a[2];
                    t_txStream->data[i+3] = U.a[3];
                    /*static float hh0 = 0.0;
                    float hh = U.f;
                    if (hh>hh0)
                    {
                    	hh0=hh;
                    	printf("Tci MaxOut=%.1f\n",((float)hh0*100.0));///8388607.0);
                    }*/
                }
                else
                {
                    //For INT types int16=2,int24=3,int32=4
                    int z = rawtxbuff[rawtx_pos];//-8388607;
                    if (bit_s==2) z /= 256; // z = z >> 8;
                    if (bit_s==4) z *= 256;
                    t_txStream->data[i]   = (z & 0xff);
                    t_txStream->data[i+1] = ((z >>  8) & 0xff);
                    if (bit_s>2) t_txStream->data[i+2] = ((z >> 16) & 0xff);
                    if (bit_s>3) t_txStream->data[i+3] = ((z >> 24) & 0xff);
                    /*static int hh0 = 0;
                    int hh = z;//rawtxbuff[rawtx_pos];
                    if (hh>hh0)
                    {
                    	hh0=hh;
                    	printf("Tci MaxOut=%d\n",hh0);//qDebug()<<hh0;
                    }*/
                    /*uint8_t a[4];
                    memcpy(a,&z,bit_s*sizeof(uint8_t));
                    for (int j = 0; j < bit_s; ++j) t_txStream->data[i+j]=a[j];*/
                }
                if (chan==1) rawtx_pos+=2*k_tx_rate;
                else rawtx_pos+=k_tx_rate;
                if (rawtx_pos > BUF_MAX - 1) rawtx_pos = 0;
            }
            else
            {
                t_txStream->data[i]   =	0;
                t_txStream->data[i+1] =	0;
                if (bit_s>2) t_txStream->data[i+2] = 0;
                if (bit_s>3) t_txStream->data[i+3] = 0;
            }
        }
        sendBinaryMessage(t_txAudioData);
        if (rawtxbuff_pos > rawtx_pos) diff_offs = BUF_MAXOFFSET - (rawtxbuff_pos - rawtx_pos);
        else diff_offs = (rawtx_pos - rawtxbuff_pos) - BUF_OFFSET;
        if (diff_offs > BUF_MAXOFFSET) diff_offs = BUF_MAXOFFSET;//protect
        if (diff_offs < 0) diff_offs = 0;//protect
        if (diff_offs > STREAM_C - 1) _flush_raw_ = 3;//if have place
        //else qDebug()<<diff_offs;
        //qDebug()<<diff_offs<<_flush_raw_;
    }
}
//end tci

struct HRDMessage
{
    // Placement style new overload for outgoing messages that does the
    // construction too.
    static void * operator new (size_t size, QString const& payload)
    {
        size += sizeof (QChar) * (payload.size () + 1); // space for terminator too
        HRDMessage * storage (reinterpret_cast<HRDMessage *> (new char[size]));
        storage->size_ = size;
        ushort const * pl (payload.utf16());
        //q_Copy (pl, pl + payload.size () + 1, storage->payload_); // copy terminator too
        std::copy (pl, pl + payload.size () + 1, storage->payload_); //2.56 copy terminator too
        storage->magic_1_ = magic_1_value_;
        storage->magic_2_ = magic_2_value_;
        storage->checksum_ = 0;  //qDebug()<<"Bites="<<storage->payload_[0];
        return storage;
    }
    // Placement style new overload for incoming messages that does the
    // construction too.
    //
    // No memory allocation here.
    static void * operator new (size_t /* size */, QByteArray const& message)
    {
        // Nasty const_cast here to avoid copying the message buffer.
        return const_cast<HRDMessage *> (reinterpret_cast<HRDMessage const *> (message.data ()));
    }

    void operator delete (void * p, size_t)
    {
        delete [] reinterpret_cast<char *> (p); // Mirror allocation in operator new above.
    }

    quint32 size_;
    qint32 magic_1_;
    qint32 magic_2_;
    qint32 checksum_;            // Apparently not used.
    //QChar payload_[1];         // UTF-16 (which is wchar_t on Windows)
    wchar_t payload_[1];

    static qint32 constexpr magic_1_value_ = 0x1234ABCD;
    static qint32 constexpr magic_2_value_ = 0xABCD1234;
};
static int n_ModelID = -1;
Network::Network(int ModelID,QWidget *parent)
        : QWidget(parent)
{
    //qDebug()<<"Create"<<ModelID;
    n_ModelID = ModelID;
    s_ModelID = ModelID;
    ////////////////////////////////////////////////////////new read com
    //s_rig_name = rigs_network[s_ModelID].name;
    //qDebug()<<"fffffffff"<<s_ModelID;
    s_CmdID = -1;
    //s_read_array.clear();
    ////////////////////////////////////////////////////////end new read com

    socket = new QTcpSocket(this);
    connect(socket, SIGNAL(readyRead()), this, SLOT(readNet()));
    connect(socket, SIGNAL(connected()), this, SLOT(connected_s()));
    connect(socket, SIGNAL(disconnected()), this, SLOT(disconnected_s()));

    //tci
    //is_my_trx = false;
    tci_select = 0;

    //----- vita49 ---------------------------------------------------
    vita_rx = 0;
    vita_tx = false;
    //flex native vita-49
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
    //----- end vita49 ---------------------------------------------------

    tci_trx = "0";//RX1
    if (ModelID==4) tci_trx = "1";//rx2
    is_tci_trx = false;
    tci_start_stop_state = false;
    tci_tx_enable = false;
    tci_rx_enable = false;
    tci_rx_mute = true;
    tci_drive=25;
    tci_split_enable = false;
    //sample_rate = 48000;
    wdevice = "NONE";
    wdemanf = "ExpertSDR3";
    id_tci_prot = 0;//0, 1=(1.5.0)andUP, 2=(1.9.0)andUp
    lsV012.clear();
    lsV012<<"1"<<"0"<<"0";

    //qRegisterMetaType<QAbstractSocket::SocketState>();
    //qRegisterMetaType<CmdID>("CmdID");
    is_wsocket = false;
    wsocket = NULL;
    /*wsocket = new HvWebSocket();//Q_NULLPTR QString(), QWebSocketProtocol::VersionLatest, this
    connect(wsocket,SIGNAL(connected_m()),this,SLOT(connected_s()));
    connect(wsocket,SIGNAL(disconnected_m()),this,SLOT(disconnected_s()));
    connect(wsocket,SIGNAL(mBinaryMessageReceived(QByteArray)),this,SLOT(wBinaryMessageReceived(QByteArray)),Qt::DirectConnection);//,Qt::DirectConnection
    connect(wsocket,SIGNAL(mTextMessageReceived(QString)),this,SLOT(wTextMessageReceived(QString)),Qt::DirectConnection);
    //connect(wsocket,SIGNAL(error(QAbstractSocket::SocketError)),this,SLOT(onError(QAbstractSocket::SocketError)));*/
    //end tci

    s_nethost = "";
    s_netport = "";
    s_tcich = "2";
    s_tcismp = "2048";
    s_tcityp = "float32";
    s_tcisamprate = "48000";
    s_tcitxbuff = "50";
    //s_tcisrate = "48000";
    //f_conected = false;

    seqnum = 0;
    fsdrs_poll = true;
    vita_last_freq_hz = 0;
    if (s_ModelID>6 && s_ModelID<15)
    {
        fsdrs = true;
        slicenum = QString("%1").arg(s_ModelID - 7);
    }
    else
    {
        fsdrs = false;
        slicenum = "0";
    }

    timer_init = new QTimer();
    connect(timer_init, SIGNAL(timeout()), this, SLOT(initAll()));

    //QTimer::singleShot(5000,this,SLOT(disconnected_s()));
    //QTimer::singleShot(6000,this,SLOT(connected_s()));
    //long long int freq = 10137000;
    //double freqd = ((double)freq / 1000000.0); qDebug()<<freqd;
    //qDebug()<<"slice tune "+slicenum+" "+QString("%1").arg(freqd,0,'f',6)+" autopan=1";
    //ConnectNet("192.168.8.82","7809");//test
}
Network::~Network()
{
    //qDebug()<<"Delete"<<rigs_network[s_ModelID].name;

    //----- vita49 ---------------------------------------------------
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
    //----- end vita49 ---------------------------------------------------

    if (socket)
    {
        if (socket->state() == QAbstractSocket::ConnectedState)
            socket->disconnectFromHost();
        //socket->waitForDisconnected(300);
        socket->close();
    }
    if (is_wsocket)//tci wsocket
    {
        SetTciStrtStopAudio(false);
        writeData("1",false,NULL);//wsocket->close(QWebSocketProtocol::CloseCodeNormal,"end");
        writeData("2",false,NULL);//flush
        usleep(22000);
        mThread.exit();
        usleep(11000);
        delete wsocket;  //wsocket->deleteLater();
        is_wsocket = false; //qDebug()<<"deleteLater()";
    }
}
void Network::SetOnOffCatCommand(bool f, int model_id, int fact_id)//tci
{
    if (model_id!=s_ModelID || fact_id!=NETWORK_ID) return;

    //----- vita49 ---------------------------------------------------
    if (!f && fsdrs) VitaStop(true);//flex native vita-49: CAT off -> release the radio
    //----- end vita49 ---------------------------------------------------

    if (!f && (s_ModelID==3 || s_ModelID==4))
    {
        SetTciStrtStopAudio(false);
        writeData("2",false,NULL); //wsocket->flush();//immediately //qDebug()<<"END-Tci=";
        usleep(22000);//2.76.2
    }
    if (f && s_ModelID==6)
    {
        writeData("AI;",false,NULL);
        writeData("AI0;",false,NULL);
    }
    //else if (f && s_ModelID==5) writeData("\\start\n",false,NULL);//???
}
void Network::SetTciStrtStopAudio(bool f)
{
    static bool start_stop_audio = false; //qDebug()<<"audio_???"<<f;
    if (start_stop_audio == f) return;
    start_stop_audio = f;
    if (f)
    {
        //qDebug()<<"audio_start--------------------"<<tci_protocol;
        writeData("audio_samplerate:"+s_tcisamprate+";",false,NULL);
        if (id_tci_prot>1)//0, 1=(1.5.0)andUP, 2=(1.9.0)andUp if (tci_protocol>1.8)
        {
            writeData("audio_stream_sample_type:"+s_tcityp+";",false,NULL);
            writeData("audio_stream_channels:"+s_tcich+";",false,NULL);
            writeData("audio_stream_samples:"+s_tcismp+";",false,NULL);
            writeData("tx_stream_audio_buffering:"+s_tcitxbuff+";",false,NULL);
        }
        writeData("audio_start:"+tci_trx+";",false,NULL);
    }
    else
    {
        writeData("audio_stop:"+tci_trx+";",false,NULL); //qDebug()<<"audio_stop";
    }
}
void Network::SetTciSelect(int i,int vr,bool vt)//tci 0=non 1=rx 2=tx 3=rx,tx
{
    tci_select = i;
    if (s_ModelID==3 || s_ModelID==4)//2.76.1 tci
    {
        if (tci_select>0) SetTciStrtStopAudio(true);
        else SetTciStrtStopAudio(false);
    }

    //----- vita49 ---------------------------------------------------
    vita_rx = vr;//RX vita 1,2,3,4..8
    vita_tx = vt;//TX vita false,true
    UpdateFlexVita();//flex native vita-49
    //----- end vita49 ---------------------------------------------------
}
int cretr_nt = -1;

//----- vita49 ---------------------------------------------------
// ============================================================ flex native vita-49
//
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
//extern void _SetRxAudioTci_(int *, int, int);
//extern int  _GetRxAudioReadTci_();
//extern int  cretr_nt;   // the CAT init state, defined further down: -2 = initialised
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
    QString nam, unit;
};
static QMutex  flex_mutex;
static QHash<int, FlexMeterDef> flex_meter_defs;   // id -> name/unit, from "S...|meter"
static QHash<int, double>       flex_meter_raw;    // id -> scaled value, from UDP
static int     flex_meter_fwd_id = -1, flex_meter_ref_id = -1, flex_meter_swr_id = -1;
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
bool _FlexVitaMeterByName_(const char *name, double *out)
{
    QMutexLocker lk(&flex_mutex);
    const QString want = QString(name).toUpper();
    int best = -1;
    for (QHash<int, FlexMeterDef>::const_iterator it = flex_meter_defs.constBegin();
         it != flex_meter_defs.constEnd(); ++it)
    {
        if (it.value().nam.toUpper() != want) continue;
        if (best < 0 || it.key() < best) best = it.key();
    }
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
    if (line.contains("|meter "))
    {
        QMutexLocker lk(&flex_mutex);
        QRegExp un("(\\d+)\\.unit=([^#\\s]+)");
        int ua = 0;
        while ((ua = un.indexIn(line, ua)) >= 0)
        {
            flex_meter_defs[un.cap(1).toInt()].unit = un.cap(2);
            ua += un.matchedLength();
        }
        QRegExp def("(\\d+)\\.nam=([^#\\s]+)");
        int at = 0;
        while ((at = def.indexIn(line, at)) >= 0)
        {
            const int id = def.cap(1).toInt();
            const QString nam = def.cap(2).toUpper();
            flex_meter_defs[id].nam = nam;
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
            at += def.matchedLength();
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
// ============================================================ end flex native vita-49
//----- end vita49 ---------------------------------------------------

void Network::connectToHost()
{
    if (s_ModelID!=n_ModelID) return;//protection 2.57 //qDebug()<<"111"<<s_ModelID<<n_ModelID;
    if (s_ModelID==3 || s_ModelID==4)//tci
    {
        if (is_wsocket)
        {
            if (wsocket->state() == QAbstractSocket::ConnectedState)
            {
                SetTciStrtStopAudio(false);
                writeData("1",false,NULL); //wsocket->close(QWebSocketProtocol::CloseCodeNormal,"end");
                //writeData("2",false,NULL);
                //usleep(42000);
                //qDebug()<<"TCI Disconnect";
                return;
            }
        }
        if (!is_wsocket)
        {
            //qDebug()<<"TCI Socket Create"<<tci_19;
            wsocket = new HvWebSocket((quint32)tci_trx.toInt());//Q_NULLPTR QString(), QWebSocketProtocol::VersionLatest, this
            connect(wsocket,SIGNAL(connected()),this,SLOT(connected_s()));
            connect(wsocket,SIGNAL(disconnected()),this,SLOT(disconnected_s()));
            connect(wsocket,SIGNAL(textMessageReceived(QString)),this,SLOT(wTextMessageReceived(QString)));
            //connect(wsocket,SIGNAL(error(QAbstractSocket::SocketError)),this,SLOT(onError(QAbstractSocket::SocketError)));
            is_wsocket = true;
            QUrl url; //QUrl t_url("ws://" + leAddress->text() + ":" + QString::number(sbPort->value()));
            url.setUrl("ws://" + s_nethost);
            url.setHost(s_nethost);
            url.setPort(s_netport.toInt());
#if defined _MACOS_
            // Mac fix: skip the moveToThread/mThread dance entirely. The
            // upstream worker-thread setup raced on Mac — connected() was
            // firing into a worker that didn't yet have a running event
            // loop, so connected_s() never fired and the TCI init never
            // armed. WebSocket I/O is async on Qt's main event loop;
            // running it there is fine and matches what wsjt-x and most
            // other Qt-based digital mode apps do.
            fprintf(stderr, "[MSHV TCI] connecting to ws://%s:%d (main thread)\n",
                    qPrintable(s_nethost), s_netport.toInt());
            wsocket->open(url);
#else            
            wsocket->open(url); //qDebug()<<tci_19<<s_nethost;
            wsocket->moveToThread(&mThread);
            mThread.start();
#endif            
        }   //else wsocket->SetCommand("0"+s_nethost+" "+s_netport); // without delete on disconnect ???
    }
    else
    {
        if (socket->state() == QAbstractSocket::ConnectedState)
        {
            //----- vita49 ---------------------------------------------------
            VitaStop(true);//flex native vita-49: release the radio while we can still tell it
            //----- end vita49 ---------------------------------------------------

            socket->disconnectFromHost(); //qDebug()<<"Cnect to Disconect=";
            //socket->waitForDisconnected(300);
            return;
        }
        int p1 = s_netport.toInt();
        socket->connectToHost(s_nethost, p1);
        socket->waitForConnected(350);//importent for TCP hv

    	//----- vita49 ---------------------------------------------------
    	UpdateFlexVita();//flex native vita-49: no-op until the CAT init completes
    	//----- end vita49 ---------------------------------------------------        
    }
}
void Network::ConnectNet(QString all)
{
    QStringList l = all.split("#");
    s_nethost = l.at(0);
    s_netport = l.at(1);
    s_tcich = l.at(2);
    s_tcismp = l.at(3);
    s_tcityp = l.at(4);
    s_tcisamprate = l.at(5);
    s_tcitxbuff = l.at(6);
    connectToHost(); //qDebug()<<l.at(0)<<l.at(1)<<l.at(2)<<l.at(3)<<l.at(4)<<l.at(5);
}
int cinit_id_nt = -1;
//int cretr_nt = -1;//2.76.7 move up
void Network::connected_s()
{
#if defined _MACOS_
    fprintf(stderr, "[MSHV TCI] connected_s fired (WebSocket connected to %s:%s)\n",
            qPrintable(s_nethost), qPrintable(s_netport));
#endif
    emit EmitNetConnInfo("<font color='red'>"+tr("Connecting to")+" "+s_nethost+" "+tr("Port")+" "+s_netport+"</font>",true,false);
    cretr_nt = -1;
    cinit_id_nt = -1;
    wdevice = "NONE";  //tci
    wdemanf = "ExpertSDR3";
    tci_190 = false;
    id_tci_prot = 0; //0, 1=(1.5.0)andUP, 2=(1.9.0)andUp
    lsV012.clear();
    lsV012<<"1"<<"0"<<"0";
    seqnum = 0;
    timer_init->start(500); //qDebug()<<"connected_s"<<"Connected to "+s_nethost+" Port "+s_netport+"";
}
void Network::disconnected_s()
{
#if defined _MACOS_
    fprintf(stderr, "[MSHV TCI] disconnected_s fired\n");
#endif
    //----- vita49 ---------------------------------------------------
    //flex native vita-49: the control socket is gone, so there is nobody to
    //tell; drop the streams locally and forget the handle.  A reconnect runs
    //the CAT init again, and its completion restarts the audio.
    VitaStop(false);
    vita_gui_done = false;
    vita_handle = 0;
    vita_buf.clear();
    //----- end vita49 ---------------------------------------------------

    emit EmitNetConnInfo("<font color='red'>"+tr("Disconnected")+"</font>",false,false);
    if (is_wsocket)//  && wsocket
    {
        mThread.exit();
#if defined _MACOS_
        // wsocket lives on the main thread on Mac (we skip moveToThread to
        // avoid the connected_s race), so a synchronous `delete wsocket`
        // can run while the WebSocket still has pending queued events
        // referencing it — crash. deleteLater drains the event queue first.
        is_wsocket = false;
        QWebSocket *dying = wsocket;
        wsocket = NULL;
        dying->deleteLater();
#else        
        usleep(22000);//2.76.2 old=usleep(4000);
        delete wsocket; //wsocket->deleteLater();
        is_wsocket = false;
#endif
    } //qDebug()<<"----------------------------------disconnected_s";
}
/*#include <iostream>
void Network::onError(QAbstractSocket::SocketError errorCode)
{
        std::cout << "Error: " << errorCode << std::endl;
}*/
QStringList rigs_nam_nt;
QStringList rigs_ids_nt;
QString rig_but_ptt_nt[4];
QString rig_mode_str_nt[3];
int cur_rig_id_nt = 0;
QString cur_rig_name_nt;
#define CMD_C_NT 5
QString command_nt[CMD_C_NT] = {"get radios","get radio","get vfo-count","get buttons","get dropdowns"};
//flrig
#define MAXXMLLEN 8192
bool isGetRadio = false;
#include <QCoreApplication>
#include "../../../config.h"
char *Network::FLRig_xml_build(char *cmd, char *value, char *xmlbuf,int xmllen)
{
    char xml[MAXXMLLEN]={0};//2.62
    char tmp[32];
    // We want at least a 4K buf to play with
    if (xmllen < 4096) return NULL; //rig_debug(RIG_DEBUG_ERR, "%s: xmllen < 4096\n");

    QString ts = "POST /RPC2 HTTP/1.1\r\n";
    ts.append("User-Agent: XMLRPC++ 0.8\r\n");
    ts.append("Host: "+s_nethost+":"+s_netport+"\r\n");
    ts.append("Content-type: text/xml\r\n");

    //sprintf(xmlbuf,hed1);  OM2AGC
    //strcat(xmlbuf,hed1);//2.59 OM2AGC
    //strcpy(xmlbuf,hed1);//2.62 KZ1O ???
    strncpy(xmlbuf,ts.toUtf8(),MAXXMLLEN-1);

    //sprintf(xml,"<?xml version=\"1.0\"?>\r\n"); //OM2AGC
    //strcat(xml,"<?xml version=\"1.0\"?>\r\n");//2.59 OM2AGC
    //strcpy(xml,"<?xml version=\"1.0\"?>\r\n");//2.62 KZ1O ???
    ts = "<?xml version=\"1.0\"?>\r\n";
    QString nam_ext_pid = QCoreApplication::applicationName();
#if defined _WIN32_
    nam_ext_pid.append(".exe");
#endif
    nam_ext_pid.append(" "+QString("%1").arg(QCoreApplication::applicationPid()));
    ts.append("<?clientid=\""+nam_ext_pid+"\"?>\r\n"); //<?clientid="fldigi.exe 2996"?>
    strncpy(xml,ts.toUtf8(),MAXXMLLEN-1);

    strcat(xml,"<methodCall><methodName>");
    strcat(xml,cmd);
    strcat(xml,"</methodName>\r\n");
    if (value && strlen(value) > 0) strcat(xml,value);
    strcat(xml,"</methodCall>\r\n");
    strcat(xmlbuf,"Content-length: ");
    sprintf(tmp,"%d\r\n\r\n",(int)strlen(xml));
    strcat(xmlbuf,tmp);
    strcat(xmlbuf,xml);

    /*strncat(xml, "<methodCall><methodName>", sizeof(xml) - 1);
    strncat(xml, cmd, sizeof(xml) - strlen(xml) - 1);
    strncat(xml, "</methodName>\r\n", sizeof(xml) - strlen(xml) - 1);
    if (value && strlen(value) > 0) strncat(xml, value, sizeof(xml) - 1);
    strncat(xml, "</methodCall>\r\n", sizeof(xml) - 1);
    strncat(xmlbuf, "Content-length: ", xmllen - 1);
    snprintf(tmp, sizeof(tmp), "%d\r\n\r\n", (int)strlen(xml));
    strncat(xmlbuf, tmp, xmllen - 1);
    strncat(xmlbuf, xml, xmllen - 1);*/

    //qDebug()<<xmlbuf;
    return xmlbuf;
}
QString Network::FLRig_get_value(QString s)
{
    QString res = "";
    int ibeg = s.indexOf("<value>");
    int iend = s.indexOf("</value>",ibeg);
    int cou = iend-ibeg;
    res = s.mid(ibeg,cou);
    res.remove("<value>");
    res = res.trimmed();
    /*if (res.isEmpty())
    {
    	int ibeg2 = s.indexOf("<value>",iend);
    	int iend2 = s.indexOf("</value>",ibeg2);
    	int cou2 = iend2-ibeg2;
    	res = s.mid(ibeg2,cou2); qDebug()<<"res2======"<<res;
    	res.remove("<value>");
    	res = res.trimmed(); 
    }*/
    return res;
}
bool Network::writeData(QString str,bool id,char *cmdFLRig)
{
    //qDebug()<<str;
    if (s_ModelID==3 || s_ModelID==4)//tci
    {
        if (is_wsocket)
        {
            if (wsocket->state() == QAbstractSocket::ConnectedState)
            {
                wsocket->SetBufferCommand(str);
                return true;
            }
            else
                return false;
        }
        else
            return false;
    }
    else
    {
        if (socket->state() == QAbstractSocket::ConnectedState)
        {
            if (s_ModelID==0)
            {
                //bool prepend_context = false;
                auto context = '[' + QString::number (cur_rig_id_nt) + "] ";
                auto string = id ? context + str : str;
                QScopedPointer<HRDMessage> message {new (string) HRDMessage};
                socket->write(reinterpret_cast<char const *> (message.data()), message->size_);
            }
            else if (s_ModelID==1)
            {
                char *cmd = qstrdup(qPrintable(str));
                char xml[MAXXMLLEN]={0};//2.62
                char *pxml = FLRig_xml_build(cmd,cmdFLRig,xml,sizeof(xml)); //qDebug()<<"XML= "<<strlen(pxml)<<pxml;
                socket->write(pxml,strlen(pxml));
            }
            else if (s_ModelID==2 || s_ModelID==5)
            {
                char *cmd = qstrdup(qPrintable(str)); //qDebug()<<"2<- "<<str;
                socket->write(cmd,strlen(cmd));
            }
            else if (s_ModelID==6)//FlexRadio 6xxx
            {
                char *cmd = qstrdup(qPrintable(str));
                socket->write(cmd,strlen(cmd));
            }
            else if (fsdrs)//FlexRadio SmartSDR Slice A-H TCP
            {
                if (seqnum > 999999) seqnum = 0;
#if defined DEBUG_FLEX_TRACE                 
				fprintf(stderr, "[MSHV Flex] >> C%d|%s\n", seqnum, qPrintable(str));//every CAT command too  
#endif				              
                QString str0 = "C"+QString("%1").arg(seqnum)+"|"+str+QChar(0x0a);
                char *cmd = qstrdup(qPrintable(str0));
                socket->write(cmd,strlen(cmd));
                seqnum++;
            }
            return socket->waitForBytesWritten(50);//500
        }
        else
        {
            //qDebug()<<"Error writeData="<<str;
            return false;
        }
    }
}
void Network::initAll()
{
    if (s_ModelID==0)
    {
        cretr_nt++;
        if (cretr_nt>3)
        {
            cretr_nt = 0;
            cinit_id_nt++;
        }
        if (cinit_id_nt>CMD_C_NT-1)
        {
            timer_init->stop();
            cretr_nt = -1;
            cinit_id_nt = -1;
            //emit EmitNetConnInfo("<font color='red'>"+tr("Error Initializing RIG")+"</font>",true,true);//old
            emit EmitNetConnInfo("<font color='red'>"+tr("Error To Initialize")+" RIG</font>",true,true);//2.76.1
            //qDebug()<<"END-----Loop";//<<ttt.elapsed();
            return;
        }
        if (cinit_id_nt==-1) cinit_id_nt=0;
        if (cretr_nt % 2 == 0)
        {
            //ttt.start();
            if (cinit_id_nt<2)
                writeData(command_nt[cinit_id_nt],false,NULL);//"get radios"=0,"get radio"=1  <2
            else
            {
                writeData(command_nt[cinit_id_nt],true,NULL);//all others with ID current radio
            }

            emit EmitNetConnInfo("<font color='red'> "+tr("Connecting to")+" "+s_nethost+" "+tr("Port")+" "+s_netport+"  "+tr("Try")+": "+QString("%1").arg(cinit_id_nt)+"</font>",true,false);
            //qDebug()<<"COMAND"<<command[cinit_id]<<c_retr<<ttt.elapsed();
        }
        //else
        //qDebug()<<"Listen"<<command[cinit_id]<<c_retr<<ttt.elapsed();
    }
    else if (s_ModelID==1)
    {
        cretr_nt++;
        if (cretr_nt>4)
        {
            timer_init->stop();
            cretr_nt = -1;//?
            emit EmitNetConnInfo("<font color='red'>"+tr("Error To Initialize")+" RIG</font>",true,true);
            return;
        }
        isGetRadio = true; //qDebug()<<cretr_nt;
        writeData("rig.get_xcvr",false,NULL);//false fictive
        emit EmitNetConnInfo("<font color='red'> "+tr("Connecting to")+" "+s_nethost+" "+tr("Port")+" "+s_netport+"  "+tr("Try")+": "+QString("%1").arg(cretr_nt)+"</font>",true,false);
    }
    else if (s_ModelID==2)
    {
        cretr_nt++;
        if (cretr_nt>4)
        {
            timer_init->stop();
            cretr_nt = -1;//?
            emit EmitNetConnInfo("<font color='red'>"+tr("Error To Initialize")+" DX Commander</font>",true,true);
            return;
        }
        isGetRadio = true;
        writeData("<command:10>CmdGetFreq<parameters:0>",false,NULL);
        emit EmitNetConnInfo("<font color='red'>"+tr("Connecting to")+" "+s_nethost+" "+tr("Port")+" "+s_netport+"  "+tr("Try")+": "+QString("%1").arg(cretr_nt)+"</font>",true,false);
    }
    else if (s_ModelID==3 || s_ModelID==4)//tci
    {
        cretr_nt++;
#if defined _MACOS_
        // AetherSDR's TCI server on Mac frequently takes longer than the
        // upstream 5-retry / 2.5 s budget to respond to the initial
        // `vfo:0,0;` query — especially on a fresh MSHV launch while the
        // SDR is still cleaning up the previous session. 20 retries × 500 ms
        // = 10 s makes the auto-connect reliable without user action.
        const int tci_max_retry = 20;
#else
        const int tci_max_retry = 4;
#endif
        if (cretr_nt > tci_max_retry)        
        {
            timer_init->stop();
            cretr_nt = -1;
            emit EmitNetConnInfo("<font color='red'>"+tr("Error To Initialize")+" TCI Server</font>",true,true);
            return;
        }
        isGetRadio = true; //qDebug()<<"SRAT COMMAND TX ---->";
        writeData("vfo:"+tci_trx+",0;",false,NULL); //writeData("vfo:"+tci_trx+",0;",false,NULL);
        emit EmitNetConnInfo("<font color='red'>"+tr("Connecting to")+" "+s_nethost+" "+tr("Port")+" "+s_netport+"  "+tr("Try")+": "+QString("%1").arg(cretr_nt)+"</font>",true,false);
    }
    else if (s_ModelID==5)
    {
        cretr_nt++;
        if (cretr_nt>4)
        {
            timer_init->stop();
            cretr_nt = -1;//?
            emit EmitNetConnInfo("<font color='red'>"+tr("Error To Initialize")+" NET RigCtl</font>",true,true);
            return;
        }
        isGetRadio = true;//2.76.1
        if (cretr_nt == 0 || cretr_nt == 1) writeData("\\get_rig_info\n",false,NULL);
        else writeData("\\chk_vfo\n",false,NULL);
        emit EmitNetConnInfo("<font color='red'>"+tr("Connecting to")+" "+s_nethost+" "+tr("Port")+" "+s_netport+"  "+tr("Try")+": "+QString("%1").arg(cretr_nt)+"</font>",true,false);
    }
    else if (s_ModelID==6)//FlexRadio 6xxx
    {
        cretr_nt++;
        if (cretr_nt>4)
        {
            timer_init->stop();
            cretr_nt = -1;
            emit EmitNetConnInfo("<font color='red'>"+tr("Error To Initialize")+" FlexRadio 6xxx</font>",true,true);
            return;
        }
        isGetRadio = true;//2.76.1
        writeData("ID;",false,NULL);//or ZZID
        emit EmitNetConnInfo("<font color='red'>"+tr("Connecting to")+" "+s_nethost+" "+tr("Port")+" "+s_netport+"  "+tr("Try")+": "+QString("%1").arg(cretr_nt)+"</font>",true,false);
    }
    else if (fsdrs)//FlexRadio SmartSDR Slice A-H TCP
    {
        cretr_nt++;
        if (cretr_nt>4)
        {
            timer_init->stop();
            cretr_nt = -1;
            emit EmitNetConnInfo("<font color='red'>"+tr("Error To Initialize")+" FlexRadio SmartSDR Slice</font>",true,true);// "+slicenum+"
            return;
        }
        isGetRadio = true;
        writeData("info",false,NULL);//writeData("sub slice "+slicenum,false,NULL);C16|info qDebug()<<"cretr_nt="<<cretr_nt; 
        if (cretr_nt == -2) return;//init completed inside writeData() (it can re-enter readNet): "Connecting" would stop polling and VitaStop() the start      
        emit EmitNetConnInfo("<font color='red'>"+tr("Connecting to")+" "+s_nethost+" "+tr("Port")+" "+s_netport+"  "+tr("Try")+": "+QString("%1").arg(cretr_nt)+"</font>",true,false);
    }
}
QString Network::GetModeStr(QString mod)
{
    QString smode = "WRONG_MODE";
    mod = mod.trimmed();
    mod = mod.toUpper();
    if 		(mod == "LSB") smode = "LSB";
    else if (mod == "USB") smode = "USB";
    else if (mod == "CW") smode = "CW";    //2.68 tci flrig
    else if (mod == "CW-L") smode = "CWL";//2.68 flrig
    else if (mod == "CW-U") smode = "CWU";//2.68 flrig
    else if (mod == "AM") smode = "AM";
    else if (mod == "FM") smode = "FM";
    else if (mod == "NFM") smode = "NFM";//tci
    //All DIG U/L
    else if (mod == "DIGL" || mod == "DATA-L") smode = "DIGL";//tci flrig
    else if (mod == "FT8" || mod == "FT4" || mod == "FT2" || mod == "JT65") smode = "DIGU";//tci
    else if (mod == "D-USB") smode = "DIGU";
    else if (mod == "DATA") smode = "DIGU";
    else if (mod == "DATA-USB") smode = "DIGU";
    else if (mod == "DATA-U") smode = "DIGU";//flrig
    else if (mod == "DIG") smode = "DIGU";
    else if (mod == "DIGU") smode = "DIGU";//tci
    else if (mod == "DIGI") smode = "DIGU";
    else if (mod == "FSK") smode = "DIGU";//DIGL ?
    else if (mod == "FSK-R") smode = "DIGU";//flrig
    else if (mod == "PKT") smode = "DIGU";//DIGL ?
    else if (mod == "PKT-U") smode = "DIGU";
    else if (mod == "PKT(U)") smode = "DIGU";
    else if (mod == "PSK") smode = "DIGU";//DIGL ?
    else if (mod == "PSK-U") smode = "DIGU";
    else if (mod == "USB-D") smode = "DIGU";
    else if (mod == "USB-D1") smode = "DIGU";
    else if (mod == "USB-D2") smode = "DIGU";
    else if (mod == "USB-D3") smode = "DIGU";
    else if (mod == "RTTY-R") smode = "DIGU";
    else if (mod == "RTTYR") smode = "DIGU";
    else if (mod == "PKTUSB") smode = "DIGU";// rigctl
    //"D-USB","DATA","DATA-USB","DATA-U","DIG","DIGI","FSK",
    //"PKT","PKT-U","PKT(U)","PSK","PSK-U","USB-D","USB-D1","USB-D2","USB-D3"
    return smode;
}
bool Network::isMyTCICommand(QString sc0)//2.64 tci
{
    const int cs = 12;//+3;
    const QString sc[cs] =
        {
            "vfo","modulation","trx","tx_enable","rx_enable","rx_mute","protocol","device",
            "start","stop","split_enable","drive"//,"audio_stream_sample_type","audio_stream_channels","audio_stream_samples"
        };
    bool res = false;
    for (int i = 0; i < cs; ++i)
    {
        if (sc[i]==sc0)
        {
            res = true;
            break;
        }
    }
    return res;
}
void Network::wTextMessageReceived(const QString &s0)//tci
{
    if (s0.isEmpty()) return;
    QStringList ls0;  //qDebug()<<"MSHV str="<<s;
    QString s = s0;

    if (wdevice!="NONE") s = s.toLower();//2.76.1
    //if (wdevice=="NONE") qDebug()<<"NO       "<<s;
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    ls0 = s.split(";",Qt::SkipEmptyParts);
#else
    ls0 = s.split(";",QString::SkipEmptyParts);
#endif
    /*
    #if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
        ls0 = s.toLower().split(";",Qt::SkipEmptyParts);
    #else
        ls0 = s.toLower().split(";",QString::SkipEmptyParts);
    #endif
    */
    for (int i = 0; i < ls0.count(); ++i)
    {
        QString cmdd = ls0.at(i);
        QStringList ls1;
        QStringList ls2;

#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
        ls1 = cmdd.split(":",Qt::SkipEmptyParts);
#else
        ls1 = cmdd.split(":",QString::SkipEmptyParts);
#endif
        ls1 <<""<<""; //qDebug()<<"MSHV Receive <-"<<ls1;
        if (!isMyTCICommand(ls1.at(0))) continue;//2.64
        //if (ls1.at(0)=="if" || ls1.at(0)=="dds") continue; //2.64 HPSDR not needed return from vfo: get set command
        //if (ls1.at(0)=="tx_power" || ls1.at(0)=="tx_swr") continue; //2.64
        if (ls1.at(0)=="device" && wdevice=="NONE") wdevice = ls1.at(1);//2.76.1
        /*{
        	wdevice = ls1.at(1).toUpper();
        	wdevice.replace("SUN","Sun"); 
        	wdevice.replace("COLIBRY","Colibri");
        	wdevice.replace("TRANSCEIVER","Transceiver");
        }*/
        if (ls1.at(0)=="start") tci_start_stop_state = true;
        if (ls1.at(0)=="stop")  tci_start_stop_state = false;        
        /*if (ls1.at(0)=="start" || ls1.at(0)=="stop")
        {
            bool prev = tci_start_stop_state;
            tci_start_stop_state = (ls1.at(0)=="start");
            // Refresh the "SDR ON/OFF" indicator once init has finished.
            // Upstream only paints it during the init-complete branch, so
            // a "start;" that arrives in response to MSHV's audio_start
            // (the normal first-connect path) was leaving the indicator
            // stuck on red OFF until the user manually disconnected and
            // reconnected.
            if (prev != tci_start_stop_state && cretr_nt == -2 && wdevice != "NONE")
            {
                QString sss = tci_start_stop_state
                              ? QString("SDR ON")
                              : QString("SDR <font color='red'>OFF</font>");
                emit EmitNetConnInfo("<font color='#00b300'>"+tr("Connected to")+" "+s_nethost+" "+tr("Port")+
                                     " "+s_netport+" - "+lsV012[0]+"."+lsV012[1]+"."+lsV012[2]+", "+wdevice+", "+sss+" -</font>",true,true);
            }
        }*/
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
        ls2 = ls1[1].split(",",Qt::SkipEmptyParts);
#else
        ls2 = ls1[1].split(",",QString::SkipEmptyParts);
#endif
        ls2 <<""<<""<<"";

        if (ls1.at(0)=="protocol") //protocol = 1.5 trx:0,true,tci;, 1.6=???
        {
            QString s = ls2.at(1);
            /*float pf = s.toFloat();
            tci_protocol = pf; //qDebug()<<"protocol="<<tci_protocol;
            if (tci_protocol>1.8) tci_19 = true; //???  tci_protocol>=1.9 = ???
            else tci_19 = false; //qDebug()<<s<<tci_19<<tci_protocol;*/
            lsV012.clear();
            lsV012 = s.split(".");
            lsV012<<"0"<<"0"<<"0";
            int v00 = lsV012[0].toInt();
            int v01 = lsV012[1].toInt();
            int v02 = lsV012[2].toInt();
            tci_190 = true;
            id_tci_prot = 2;//0, 1=(1.5.0)andUP, 2=(1.9.0)andUp
            if (v00<2 && v01<9 && v02<1000)//1.8.999
            {
                tci_190 = false;
                id_tci_prot = 1;//0, 1=(1.5.0)andUP, 2=(1.9.0)andUp
            }
            if (v00<2 && v01<5 && v02<1000)//1.4.999
            {
                tci_190 = false;
                id_tci_prot = 0;//0, 1=(1.5.0)andUP, 2=(1.9.0)andUp
            }
            wdemanf = ls2.at(0);//2.76.1
            //printf("Manufacturer= %s\n",qPrintable(wdemanf));
        } //qDebug()<<lsV012[0]<<lsV012[1]<<lsV012[2]<<tci_190<<id_tci_prot;
        //qDebug()<<"MSHV Receive <-"<<ls1.at(0)<<ls2.at(0)<<ls2.at(1)<<ls2.at(2);
        if (ls2.at(0)!=tci_trx) continue; //2.64
        //qDebug()<<"MSHV Receive <-"<<ls1.at(0)<<ls2.at(0)<<ls2.at(1)<<ls2.at(2);
#if defined _MACOS_
        // AetherSDR (FlexRadio TCI bridge on Mac) sends "vfo:trx,0,FREQ;"
        // proactively right after WebSocket connect — before initAll's first
        // query fires. Upstream's check at the next line gates init-complete
        // on isGetRadio (set only by initAll), so the proactive notification
        // is missed and our subsequent vfo:0,0; queries hit AetherSDR's
        // pub/sub model with no response → 5/20-retry timeout. Hand the
        // proactive notification to the upstream init-complete path by
        // raising isGetRadio here.
        if (!isGetRadio && cretr_nt != -2 && ls1.at(0) == "vfo" && ls2.at(1) == "0") isGetRadio = true;
#endif        
        if (isGetRadio)
        {
            isGetRadio = false; //qDebug()<<"First Start---------------------------------------------";
            if (ls1.at(0)=="vfo" && ls2.at(1)=="0")//2.64 && ls2.at(0)==tci_trx
            {
                timer_init->stop();
                cretr_nt = -2;
                QString sss = "SDR <font color='red'>OFF</font>";
                if (tci_start_stop_state) sss = "SDR ON";
                //emit EmitNetConnInfo("<font color='#00b300'>"+tr("Connected to")+" "+s_nethost+" "+tr("Port")+
                //" "+s_netport+" - "+QString("%1").arg(tci_protocol,0,'f',1)+", "+wdevice+", "+sss+" -</font>",true,true);
                emit EmitNetConnInfo("<font color='#00b300'>"+tr("Connected to")+" "+s_nethost+" "+tr("Port")+
                                     " "+s_netport+" - "+lsV012[0]+"."+lsV012[1]+"."+lsV012[2]+", "+wdevice+", "+sss+" -</font>",true,true);
                emit EmitFullRigInfo(wdemanf+" "+wdevice);//2.76.1 for pskreporter
                //printf("Manufacturer= %s Device= %s\n",qPrintable(wdemanf),qPrintable(wdevice));
                if (tci_select>0) SetTciStrtStopAudio(true);
                if (!tci_tx_enable)   writeData("tx_enable:"+tci_trx+",true;",false,NULL);
                if (!tci_rx_enable)   writeData("rx_enable:"+tci_trx+",true;",false,NULL);
                if (tci_rx_mute)      writeData("rx_mute:"+tci_trx+",false;",false,NULL);
                if (tci_split_enable) writeData("split_enable:"+tci_trx+",false;",false,NULL);//2.76.6
                if (tci_drive<=0)     writeData("drive:"+tci_trx+",25;",false,NULL);//2.76.6
                writeData("tx_sensors_enable:false,500;",false,NULL);//2.76.6
                writeData("rx_sensors_enable:false,500;",false,NULL);//2.76.6
                //writeData("stop;",false,NULL);
            }
        }
        else if (ls1.at(0)=="vfo" && ls2.at(1)=="0")//2.64 && ls2.at(0)==tci_trx
        {
            //if (s_CmdID == GET_FREQ)
            //{
            emit EmitReadedInfo(GET_FREQ,ls2.at(2));
            //}
            s_CmdID = -1;
        }
        else if (ls1.at(0)=="modulation")//2.64 && ls2.at(0)==tci_trx
        {
            //if (s_CmdID == GET_MODE)
            //{
            QString smode = GetModeStr(ls2.at(1));
            emit EmitReadedInfo(GET_MODE,smode);
            //}
            s_CmdID = -1;
        }
        else if (ls1.at(0)=="trx")//2.64 && ls2.at(0)==tci_trx
        {
            if (ls2.at(1)=="true") is_tci_trx = true;
            else is_tci_trx = false;
        }
        else if (ls1.at(0)=="tx_enable")//2.64 && ls2.at(0)==tci_trx
        {
            if (ls2.at(1)=="true") tci_tx_enable = true;
            else tci_tx_enable = false;
        }
        else if (ls1.at(0)=="rx_enable")//2.64 && ls2.at(0)==tci_trx
        {
            if (ls2.at(1)=="true") tci_rx_enable = true;
            else tci_rx_enable = false;
        }
        else if (ls1.at(0)=="rx_mute")//2.64
        {
            if (ls2.at(1)=="true") tci_rx_mute = true;
            else tci_rx_mute = false;
        }
        else if (ls1.at(0)=="split_enable")//2.76.6
        {
            if (ls2.at(1)=="true") tci_split_enable = true;
            else tci_split_enable = false;
        }
        else if (ls1.at(0)=="drive")//2.76.6
        {
            tci_drive = ls2.at(1).toInt(); //qDebug()<<"0 tci_drive="<<tci_drive;
            //if (tci_drive<=0) QTimer::singleShot(1000,this,SLOT(SetTciDrive()));//;writeData("drive:"+tci_trx+",25;",false,NULL);//2.76.6
        }
    }
}
QString Network::GetModeStrKenwood(QChar c)
{
    QString smode = "WRONG_MODE";
    if 		(c==(char)0x31) smode = "LSB"; //0x31=LSB=1
    else if (c==(char)0x32) smode = "USB"; //0x32=USB=2
    else if (c==(char)0x33) smode = "CWU";//0x33=CW-USB=3
    else if (c==(char)0x34) smode = "FM";  //0x34=FM=4
    else if (c==(char)0x35) smode = "AM";  //0x35=AM=5
    else if (c==(char)0x36) smode = "DIGL";//0x36=DATA-LSB=6
    else if (c==(char)0x37) smode = "CWL";//0x37=CW-LSB=7
    else if (c==(char)0x39) smode = "DIGU";//0x39=DATA-USB=9
    return smode;
}
void Network::readNet()
{
    QByteArray ba = socket->readAll();

    if (s_ModelID==0)//Ham Radio Deluxe
    {
        auto buffer = ba;
        HRDMessage const * reply
        {
            new (buffer) HRDMessage
        };
        if (reply->magic_1_value_ != reply->magic_1_ && reply->magic_2_value_ != reply->magic_2_) return;// no for me

        while (buffer.size () - offsetof (HRDMessage, size_) < reply->size_)
        {
            buffer += ba;
            reply = new (buffer) HRDMessage;
        }
        QString str = QString {(QChar*)reply->payload_};

        if (str.isEmpty ()) return;
        if (str=="OK" || str=="0" || str=="1") return; //qDebug()<<"NO NEEDED"<<str;

        if (s_CmdID == GET_FREQ && cinit_id_nt==-1)
        {
            if (str.count()>1)//2.76.2
            {
                if (str.at(0).isDigit() && str.at(1).isDigit())//protect freq
                {
                    emit EmitReadedInfo(GET_FREQ,str); //printf("FREQ =%s\n",qPrintable(str));
                }
            } //else qDebug()<<"FreqError="<<str;
            s_CmdID = -1;//I Find my answer no need more
        }
        else if (s_CmdID == GET_MODE && cinit_id_nt==-1)
        {
            if (str.at(0)=='M')//protect mode "Mode" "Main Mode" "Main A"
            {
                QStringList lstr = str.split(":");//printf("MODE =%s\n",qPrintable(str));
                if (lstr.count()>1)
                {
                    QString smode = GetModeStr(lstr.at(1));
                    emit EmitReadedInfo(GET_MODE,smode);
                }
                else emit EmitReadedInfo(GET_MODE,"WRONG_MODE");// Mode no -> : error
            } //else qDebug()<<"ModeError="<<str;
            s_CmdID = -1;//I Find my answer no need more
        }

        if (cinit_id_nt==-1) return; // not for me
        if (cinit_id_nt==0)
        {
            QStringList ltmp;
            rigs_ids_nt.clear();
            rigs_nam_nt.clear();
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)//2.56
            ltmp << str.trimmed().split (',', Qt::SkipEmptyParts);
#else
            ltmp << str.trimmed().split (',', QString::SkipEmptyParts);
#endif
            for (int i = 0; i<ltmp.count(); ++i)
            {
                QString stmp = ltmp.at(i);
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)//2.56
                QStringList isrig = stmp.trimmed().split(':', Qt::SkipEmptyParts);
#else
                QStringList isrig = stmp.trimmed().split(':', QString::SkipEmptyParts);
#endif
                if (isrig.count()==2)
                {
                    rigs_ids_nt.append(isrig.at(0));
                    rigs_nam_nt.append(isrig.at(1));
                }
            }
            if (rigs_ids_nt.count()<1) return;
            //qDebug()<<"radios="<<rigs_ids_nt<<rigs_nam_nt;
            cretr_nt = -1;
            cinit_id_nt++;
        }
        else if (cinit_id_nt==1)
        {
            cur_rig_name_nt = "No Radio";
            cur_rig_id_nt = 0;
            for (int i = 0; i<rigs_nam_nt.count(); ++i)
            {
                if (str==rigs_nam_nt.at(i))
                {
                    cur_rig_id_nt = rigs_ids_nt.at(i).toInt();
                    cur_rig_name_nt = str;
                    break;
                }
            }
            //qDebug()<<cur_rig_id_nt<<"radio="<<str;
            cretr_nt = -1;
            cinit_id_nt++;
        }
        else if (cinit_id_nt==2)
        {
            //qDebug()<<"vfo-count="<<str;
            cretr_nt = -1;
            cinit_id_nt++;
        }
        else if (cinit_id_nt==3)
        {
            rig_but_ptt_nt[0]="";
            rig_but_ptt_nt[1]="";
            rig_but_ptt_nt[2]="";
            rig_but_ptt_nt[3]="";
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)//2.56
            QStringList tbut = str.trimmed().split(',', Qt::SkipEmptyParts).replaceInStrings(" ", "~");
#else
            QStringList tbut = str.trimmed().split(',', QString::SkipEmptyParts).replaceInStrings(" ", "~");
#endif
            for (int i = 0; i<tbut.count(); ++i)//TX|TX~main|TX~-~A|TX~A
            {
                QString tstt = tbut.at(i);
                if (tstt=="TX") rig_but_ptt_nt[0]=tstt;
                if (tstt=="TX~main") rig_but_ptt_nt[1]=tstt;
                if (tstt=="TX~-~A") rig_but_ptt_nt[2]=tstt;
                if (tstt=="TX~A") rig_but_ptt_nt[3]=tstt;
            }
            //qDebug()<<"get buttonsAll="<<str;
            //qDebug()<<"get buttons="<<rig_but_ptt_nt[0]<<rig_but_ptt_nt[1]<<rig_but_ptt_nt[2]<<rig_but_ptt_nt[3];
            cretr_nt = -1;
            cinit_id_nt++;
        }
        else if (cinit_id_nt==4)
        {
            rig_mode_str_nt[0]="";
            rig_mode_str_nt[1]="";
            rig_mode_str_nt[2]="";
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)//2.56
            QStringList tbut = str.trimmed ().split (',', Qt::SkipEmptyParts);
#else
            QStringList tbut = str.trimmed ().split (',', QString::SkipEmptyParts);
#endif
            for (int i = 0; i<tbut.count(); ++i)//Main Mode|Mode|Mode A
            {
                QString tstt = tbut.at(i);
                if (tstt=="Main Mode") rig_mode_str_nt[0]=tstt;
                if (tstt=="Mode") rig_mode_str_nt[1]=tstt;
                if (tstt=="Main A") rig_mode_str_nt[2]=tstt;
            }
            //qDebug()<<"get dropdownsALL="<<str;
            //qDebug()<<"get dropdowns="<<rig_mode_str_nt[0]<<rig_mode_str_nt[1]<<rig_mode_str_nt[2];
            timer_init->stop();
            cretr_nt = -1;
            cinit_id_nt = -1;
            emit EmitNetConnInfo("<font color='#00b300'>"+tr("Connected to")+" "+s_nethost+" "+tr("Port")+" "+s_netport+" - RIG:"+cur_rig_name_nt+" -</font>",true,true);
            emit EmitFullRigInfo("Ham Radio Deluxe "+cur_rig_name_nt);//2.76.1 for pskreporter or HRD
        }
    }
    else if (s_ModelID==1)//FLRig
    {
        QString sba = QString(ba.data());
        if (sba.isEmpty ()) return;
        int ifl = sba.indexOf(" 200 OK");//" 200 OK"
        if (ifl < 0) return;
        QString value = FLRig_get_value(sba);
        if (value.isEmpty())
        {
            //qDebug()<<"---ERROR---"<<QString("%1").arg(s_CmdID).rightJustified(2,' ')<<"Count ignor="<<sba.count(" 200 OK");
            /*QString ss = sba;
            ss.remove("HTTP/1.1 200 OK\r\nServer: XMLRPC++ 0.8\r\nContent-Type: text/xml\r\nContent-length: ");
            ss = ss.remove("\r\n\r\n<?xml version=\"1.0\"?>\r\n<methodResponse><params><param>\r\n\t");
            ss = ss.remove("\r\n</param></params></methodResponse>\r\n");
            qDebug()<<"STR="<<ss<<s_CmdID;*/
            return;
        }   //qDebug()<<value<<s_CmdID;

        if (isGetRadio)
        {
            //cur_rig_name_nt = "No Radio";
            isGetRadio = false;
            timer_init->stop();
            cretr_nt = -2;
            emit EmitNetConnInfo("<font color='#00b300'>"+tr("Connected to")+" "+s_nethost+" "+tr("Port")+" "+s_netport+" - RIG:"+value+" -</font>",true,true);
            emit EmitFullRigInfo("FLRig "+value);//2.76.1 for pskreporter
        }
        else if (s_CmdID == GET_FREQ)
        {
            if (value.toInt()>12000) emit EmitReadedInfo(GET_FREQ,value);//135000
            //else EmitReadedInfo(GET_FREQ,"0");
            s_CmdID = -1;
        }
        else if (s_CmdID == GET_MODE)
        {
            if (value.at(0).isLetter())//2.62
            {
                QString smode = GetModeStr(value);
                emit EmitReadedInfo(GET_MODE,smode);
            }
            //else EmitReadedInfo(GET_MODE,"WRONG_MODE");
            s_CmdID = -1;
        }
    }
    else if (s_ModelID==2)//DX Lab Suite Commander
    {
        QString sba = QString(ba.data());
        if (sba.isEmpty ()) return;
        if (sba.at(0)!='<') return;

        if (isGetRadio)
        {
            isGetRadio = false;
            if (sba.mid(0,9) == "<CmdFreq:")
            {
                timer_init->stop();
                cretr_nt = -2;
                emit EmitNetConnInfo("<font color='#00b300'>"+tr("Connected to")+" "+s_nethost+" "+tr("Port")+" "+s_netport+" - DX Commander -</font>",true,true);
            }
        }
        else if (s_CmdID == GET_FREQ)
        {
            if (sba.mid(0,9) == "<CmdFreq:")
            {
                sba = sba.mid(sba.indexOf ('>') + 1);
                QLocale loc;
                sba.replace(QChar{QChar::ReplacementCharacter},loc.groupSeparator());
                bool f;
                long long int frq = (QLocale::c().toDouble(sba,&f)+1e-4)*1e3;
                //qDebug()<<"connected_s"<<sba<<frq;
                /*sba.remove(",");
                sba.remove(".");
                QString freq = sba;
                if (freq.toInt()==0) freq = "0";*/
                QString freq = QString("%1").arg(frq);
                emit EmitReadedInfo(GET_FREQ,freq);
            }
            s_CmdID = -1;
        }
        else if (s_CmdID == GET_MODE)
        {
            if (sba.mid(0,9) == "<CmdMode:")  //<CmdMode:2>CW
            {
                sba = sba.mid(sba.indexOf ('>') + 1);
                QString smode = GetModeStr(sba);
                emit EmitReadedInfo(GET_MODE,smode);
            }
            s_CmdID = -1;
        }   //qDebug()<<"RX From DXLab="<<sba;
    }
    else if (s_ModelID==5)//Hamlib NET rigctl
    {
        QString sba = QString(ba.data()); //qDebug()<<cretr_nt<<"IN> "<<sba;
        if (sba.isEmpty ()) return;
        if (!sba.endsWith('\n')) return;
        if (sba.startsWith("RPRT ")) return;//"RPRT -5\n"
        /*sba.remove("RPRT 0\n");
        sba.remove("RPRT 1\n");
        if (sba.isEmpty()) return;*/
        //qDebug()<<cretr_nt<<"----------> "<<sba;
        QStringList ls = sba.split("\n");
        ls<<" "<<" "<<" ";
        if (isGetRadio)
        {
            isGetRadio = false; //bool ok01 = false;
            if (cretr_nt == 0 || cretr_nt == 1)
            {
                for (int i = 0; i < ls.count(); ++i)//2.76.1
                {
                    if (ls.at(i).startsWith("Rig="))
                    {
                        timer_init->stop();
                        cretr_nt = -2;
                        QString na0 = ls.at(i);
                        na0.remove("Rig=");
                        emit EmitNetConnInfo("<font color='#00b300'>"+tr("Connected to")+" "+s_nethost+" "+tr("Port")+" "+s_netport+" - RIG:"+na0+" -</font>",true,true);
                        emit EmitFullRigInfo("Hamlib NET rigctl "+na0);//2.76.1 for pskreporter or HamLib NETRigCtl   Hamlib NET rigctl
                        //ok01 = true; qDebug()<<cretr_nt<<"1-CONNECT"<<sba;
                    }
                }
            }
            else//if (!ok01)
            {
                if (sba.count()>1)
                {
                    QChar dd = sba.at(sba.count()-2);
                    if (dd.isDigit())//"CHKVFO 0\n" or "CHKVFO 1\n"
                    {
                        timer_init->stop(); //qDebug()<<cretr_nt<<"2-CONNECT"<<sba;
                        cretr_nt = -2;
                        emit EmitNetConnInfo("<font color='#00b300'>"+tr("Connected to")+" "+s_nethost+" "+tr("Port")+" "+s_netport+" - NET RigCtl -</font>",true,true);
                    }
                }
            }
        }
        else if (s_CmdID == GET_FREQ)
        {
            if (ls.at(0).toInt()>12000) emit EmitReadedInfo(GET_FREQ,ls.at(0));//qDebug()<<"Efrq===================="<<ls.at(0);
            s_CmdID = -1;
        }
        else if (s_CmdID == GET_MODE)
        {
            sba = ls.at(0);//qDebug()<<"Mod====================="<<ls.at(0);
            QString smode = GetModeStr(sba);
            emit EmitReadedInfo(GET_MODE,smode);
            s_CmdID = -1;
        }
    }
    else if (s_ModelID==6)//FlexRadio 6xxx
    {
        QString sba = QString(ba.data()); //qDebug()<<"----------> "<<sba;
        if (sba.isEmpty ()) return;
        if (sba.count()<3) return;
        if (sba.at(sba.count()-1)!=';') return;//if (!sba.endsWith(';')) return;
        sba.remove(";");
        if (isGetRadio)
        {
            //"ID019"    TS-2000
            //"IDID900"  DDUtil in TS-2000 mode
            //ID900      PowerSDR after ZZID; command
            //ID904 	 SmartSDR Flex-6700 "ZZAI0;ZZDX1","","ZZTX1","ZZTX0","ZZFA","ZZMD01","ZZMD07","ZZIF","ZZIF"
            //ID905 	 PowerSDR Flex-6500 "AI0"        ,"","TX"   ,"RX"   ,"FA"  ,"MD2"   ,"MD9"   ,"IF"  ,"IF"
            //ID906 	 PowerSDR Flex-6700R
            //ID907 	 PowerSDR Flex-6300
            //ID908 	 PowerSDR Flex-6400
            //ID909 	 PowerSDR Flex-6600
            isGetRadio = false;
            if (sba.mid(0,2) == "ID" && sba.count()<6)//ID904
            {
                timer_init->stop();
                cretr_nt = -2;
                QString idn = sba;
                idn.remove("ID"); //idn = "906";
                if 		(idn=="019") idn = "as TS-2000";
                else if (idn=="900") idn = "as PowerSDR";
                else if (idn=="904") idn = "Flex-6700";
                else if (idn=="905") idn = "Flex-6500";
                else if (idn=="906") idn = "Flex-6700R";
                else if (idn=="907") idn = "Flex-6300";
                else if (idn=="908") idn = "Flex-6400";
                else if (idn=="909") idn = "Flex-6600";
                else idn.prepend("ID");
                emit EmitNetConnInfo("<font color='#00b300'>"+tr("Connected to")+" "+s_nethost+" "+tr("Port")+" "+s_netport+" - RIG:"+idn+" -</font>",true,true);
                if (idn.startsWith("Flex-")) emit EmitFullRigInfo("FlexRadio "+idn);
            }
        }
        else if ((s_CmdID==GET_FREQ || s_CmdID==GET_MODE) && sba.count()>36 && sba.mid(0,2) == "IF")
        {
            QByteArray tfreq;
            QString sba0 = sba.mid(2,11);
            tfreq.append(sba0.toLatin1());
            unsigned long long f = tfreq.toLongLong();
            emit EmitReadedInfo(GET_FREQ,QString("%1").arg(f));
            QString smode = GetModeStrKenwood(sba.at(29)); //qDebug()<<(sba.at(29));
            emit EmitReadedInfo(GET_MODE,smode);
            s_CmdID = -1;
        }
        else if (s_CmdID == GET_FREQ && sba.count()>12 && sba.mid(0,2) == "FA")
        {
            QByteArray tfreq;
            QString sba0 = sba.mid(2,11);
            tfreq.append(sba0.toLatin1());
            unsigned long long f = tfreq.toLongLong();
            emit EmitReadedInfo(GET_FREQ,QString("%1").arg(f));
            s_CmdID = -1;
        }
        else if (s_CmdID == GET_MODE && sba.mid(0,2) == "MD")
        {
            QString smode = GetModeStrKenwood(sba.at(2));
            emit EmitReadedInfo(GET_MODE,smode);
            s_CmdID = -1;
        }
    }
    else if (fsdrs)//FlexRadio SmartSDR Slice A-H TCP
    {
        QString sba = QString(ba.data()); //qDebug()<<"IN-> "<<sba;
        if (sba.isEmpty ()) return;//if (seqnum > 999999) seqnum = 0; R999999|0. = count=10
        //if (sba.count()<10) return;//???

        //----- vita49 ---------------------------------------------------
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
        //----- end vita49 ---------------------------------------------------

        sba.replace('\r',' ');
        sba.replace('\n',' ');
        /*if (wdevice == "NONE")
        {
        	via UDP
        	//UDP ----->        
        //discovery_protocol_version=2.0.0.0 model=Flex-6600 khhll lllll
        //<----- UDP 
        }*/
        /* Test
        V1.2.0.0
        H545A4ACD
        M10000001|Client connected from IP 192.168.0.4
        */
        if (isGetRadio)
        {
            /*R16|0|model="FLEX-6700",chassis_serial="K00000000",mbtrx_pcb="PC-0062",mbtrx_pcbrev="E",
            mbtrx_bom="B-0064",mbtrx_bomrev="A",mbtrx_serial="B28154-07 LF",mbpa_pcb="",mbpa_rev="",
            mbpa_bom="",mbpa_bomrev="",mbpa_serial="",name="",callsign="N5AC",gps="Warming Up",
            scu=2,slice=8,software_ver=0.0.3,mac=00:18:31:78:4F:B4,ip=192.168.30.136,
            netmask=255.255.255.0,gateway=192.168.30.1,location=""*/
            /* Test
            R16|0|model="FLEX-6700",chassis_serial="K00000000",mbtrx_pcb="PC-0062",mbtrx_pcbrev="E"
            S67319A86|slice 0 in_use=1 sample_rate=24000 RF_frequency=10.137000 client_handle=0x76AF7C73 index_letter=A rxant=ANT2 mode=DIGU wide=0
            R16|0|model="FLEX-6700",S67319A86|slice 0 in_use=1 sample_rate=24000 RF_frequency=10.137000 mode=DIGU client_handle=0x76AF7C73 rxant=ANT2 txant=XVTA
            */
            sba.replace(',',' ');
            isGetRadio = false;
            int id0 = sba.indexOf("model=");
            if (id0>-1)
            {
                timer_init->stop();
                cretr_nt = -2;

                //----- vita49 ---------------------------------------------------
                //flex native vita-49: the CAT session is up; the audio may
                //follow.  Deferred a tick rather than started from inside
                //readNet(), since writeData() can re-enter it.
                if (vita_step==0) vita_timer->start(50);
                //----- end vita49 ---------------------------------------------------

                QString mod0 = "Flex-XXXX";
                int id1 = sba.indexOf(' ',id0);
                id0 = id0+6;
                if (id1>-1)
                {
                    mod0 = sba.mid(id0,id1-id0);
                    mod0.remove('\"');
                    mod0.replace("FLEX","Flex");//to lower case name
                    if 		(slicenum=="0") mod0 = "A "+mod0;
                    else if (slicenum=="1") mod0 = "B "+mod0;
                    else if (slicenum=="2") mod0 = "C "+mod0;
                    else if (slicenum=="3") mod0 = "D "+mod0;
                    else if (slicenum=="4") mod0 = "E "+mod0;
                    else if (slicenum=="5") mod0 = "F "+mod0;
                    else if (slicenum=="6") mod0 = "G "+mod0;
                    else if (slicenum=="7") mod0 = "H "+mod0;
                }
                emit EmitNetConnInfo("<font color='#00b300'>"+tr("Connected to")+" "+s_nethost+" "+tr("Port")+" "+s_netport+" - RIG:"+mod0+" -</font>",true,true);
                if (!mod0.startsWith("Flex-XXXX")) emit EmitFullRigInfo("FlexRadio SmartSDR Slice "+mod0);
                fsdrs_poll = true;
            }
        }
        else if (sba.contains("slice "+slicenum)) //or if (!sba.contains("slice "+slicenum)) return;
        {
            /* Example response to "sub slice 0"
            511+511+35
            S67319A86|slice 0 in_use=1 sample_rate=24000 RF_frequency=10.137000 client_handle=0x76AF7C73 index_letter=A rit_on=0 rit_freq=0 xit_on=0 xit_freq=0 rxant=ANT2 mode=DIGU wide=0 filter_lo=0 filter_hi=3510 step=10 step_list=1,5,10,20,100,250,500,1000 agc_mode=fast agc_threshold=65 agc_off_level=10 pan=0x40000000 txant=ANT2 loopa=0 loopb=0 qsk=0 dax=1 dax_clients=1 lock=0 tx=1 active=1 audio_level=100 audio_pan=51 audio_mute=1 record=0 play=disabled record_time=0.0 anf=0 anf_level=0 nr=0 nr_level=0 nb=0 nb_lev direct=1 el=50 wnb=0 wnb_level=100 apf=0 apf_level=0 squelch=1 squelch_level=20 diversity=0 diversity_parent=0 diversity_child=0 diversity_index=1342177293 ant_list=ANT1,ANT2,RX_A,RX_B,XVTA,XVTB mode_list=LSB,USB,AM,CW,DIGL,DIGU,SAM,FM,NFM,DFM,RTTY fm_tone_mode=OFF fm_tone_value=67.0 fm_repeater_offset_freq=0.000000 tx_offset_freq=0.000000 repeater_offset_dir=SIMPLEX fm_tone_burst=0 fm_deviation=5000 dfm_pre_de_emphasis=0 post_demod_low=300 post_demod_high=3300 rtty_mark=2125 rtty_shift=170 digl_offset=2210 digu_offset=1500 post_demod_bypass=0 rfgain=24  tx_ant_list=ANT1,ANT2,XVTA,XVTB
            S67319A86|waveform installed_list=
            R0|0|*/
            //if (s_CmdID == GET_FREQ || s_CmdID == GET_MODE)//RF_frequency=10.137000
            //{
            int id0 = sba.indexOf("RF_frequency=");
            if (id0>-1)
            {
                int id1 = sba.indexOf(' ',id0);
                id0 = id0+13;
                if (id1>-1)
                {
                    QByteArray tfreq;
                    QString sba0 = sba.mid(id0,id1-id0);
                    tfreq.append(sba0.toLatin1());
                    double frq = tfreq.toDouble();
                    unsigned long long f = (frq * 1e6);
                    emit EmitReadedInfo(GET_FREQ,QString("%1").arg(f));
                    s_CmdID = -1;
                }
            }
            id0 = sba.indexOf("mode=");//mode=DIGU
            if (id0>-1)
            {
                int id1 = sba.indexOf(' ',id0);
                id0 = id0+5;
                if (id1>-1)
                {
                    QString smode = GetModeStr(sba.mid(id0,id1-id0));
                    emit EmitReadedInfo(GET_MODE,smode);
                    s_CmdID = -1;
                }
            }
            //}
        }
    }
}
void Network::SetCmd(CmdID i,ptt_t ptt,QString str)
{
    switch (i)
    {
    case GET_SETT:
        emit EmitRigSet(rigs_network[s_ModelID]);
        break;
    case SET_PTT:
        set_ptt(ptt);
        break;
    case SET_FREQ:
        set_freq(str.toLongLong());
        break;
    case GET_FREQ:
        s_CmdID = GET_FREQ;
        get_freq();
        break;
    case SET_MODE:
        set_mode(str);
        break;
    case GET_MODE:
        s_CmdID = GET_MODE;
        get_mode();
        break;
    }
}
void Network::SetTciTxOnRX2()
{
    if (!tci_tx_enable) return;
    QString tci_trx150 = "";//0, 1=(1.5.0)andUP, 2=(1.9.0)andUp
    if (tci_select>1 && id_tci_prot>0) tci_trx150 = ",tci";//if (tci_select>1 && tci_protocol>=1.5) tci_trx15 = ",tci";
    writeData("trx:"+tci_trx+",true"+tci_trx150+";",false,NULL);
    //if (tci_drive<=0) writeData("drive:"+tci_trx+",25;",false,NULL);//2.76.6
}
void Network::set_ptt(ptt_t ptt)
{
    char *a = (char*)"1221";
    emit EmitWriteCmd(a,4);//fictive stop poping timer
    if (s_ModelID==0)
    {
        QString res = "TX";
        if (!rig_but_ptt_nt[0].isEmpty()) rig_but_ptt_nt[0]=res;
        else if (!rig_but_ptt_nt[1].isEmpty()) rig_but_ptt_nt[1]=res;
        else if (!rig_but_ptt_nt[2].isEmpty()) rig_but_ptt_nt[2]=res;
        else if (!rig_but_ptt_nt[3].isEmpty()) rig_but_ptt_nt[3]=res;

        if (ptt==RIG_PTT_ON)
            writeData("set button-select "+res+" 1",true,NULL);//ok
        else
            writeData("set button-select "+res+" 0",true,NULL);//ok
    }
    else if (s_ModelID==1)
    {
        if (ptt==RIG_PTT_ON)
            writeData("rig.set_ptt",false,(char*)"<params><param><value><i4>1</i4></value></param></params>");
        else
            writeData("rig.set_ptt",false,(char*)"<params><param><value><i4>0</i4></value></param></params>");
    }
    else if (s_ModelID==2)
    {
        if (ptt==RIG_PTT_ON)
            writeData("<command:5>CmdTX<parameters:0>",false,NULL);
        else
            writeData("<command:5>CmdRX<parameters:0>",false,NULL);
    }
    else if (s_ModelID==3 || s_ModelID==4)//tci
    {
        if (!tci_tx_enable) return;
        QString tci_trx150 = "";//0, 1=(1.5.0)andUP, 2=(1.9.0)andUp
        if (tci_select>1 && id_tci_prot>0) tci_trx150 = ",tci";//if (tci_select>1 && tci_protocol>=1.5) tci_trx15 = ",tci";
        if (ptt==RIG_PTT_ON)
        {
            exit_txaudio = -1; //exit immediately from while and no flush raw
            if (!is_tci_trx)
            {
                if (tci_trx=="1") QTimer::singleShot(100,this,SLOT(SetTciTxOnRX2()));//protection
                else
                {
                    writeData("trx:"+tci_trx+",true"+tci_trx150+";",false,NULL);
                    //if (tci_drive<=0) writeData("drive:"+tci_trx+",25;",false,NULL);//2.76.6
                }
            }
            //is_my_trx = true;
        }
        else
        {
            //is_my_trx = false;
            writeData("trx:"+tci_trx+",false"+tci_trx150+";",false,NULL); //writeData("trx:"+tci_trx+",false;",false,NULL);
            //writeData("trx:"+tci_trx+",false;",false,NULL);
            exit_txaudio = -1; //exit immediately from while and no flush raw
        }
    }
    else if (s_ModelID==5)
    {
        /*if (ptt==RIG_PTT_ON) writeData("\\set_ptt 1\n",false,NULL);// \\set_ptt <- extended command
        else writeData("\\set_ptt 0\n",false,NULL);*/
        QString ptt_cmd = "0";
        if 		(ptt==RIG_PTT_ON     ) ptt_cmd = "1";
        else if (ptt==RIG_PTT_ON_MIC ) ptt_cmd = "2";
        else if (ptt==RIG_PTT_ON_DATA) ptt_cmd = "3";
        writeData("\\set_ptt "+ptt_cmd+"\n",false,NULL); //writeData("T "+ptt_cmd+"\n",false,NULL);
    }
    else if (s_ModelID==6)
    {
        QString ptt_cmd = "RX;";
        if (ptt==RIG_PTT_ON) ptt_cmd = "TX;";
        else 				 ptt_cmd = "RX;";
        writeData(ptt_cmd,false,NULL);
    }
    else if (fsdrs)//FlexRadio SmartSDR Slice A-H
    {
        QString ptt_cmd = "0";//"dax audio set %d tx=1"  "slice set %d tx=1" "xmit %d"

        //----- vita49 ---------------------------------------------------
        //flex native vita-49: the DAX channel is independent of the slice
        //number (channel 3 can be fed by slice A).  While the audio backend is
        //up, key the channel it actually created; slice+1 is only the default.
        int iddax = vita_tx_active ? vita_ch : slicenum.toInt()+1;
        //----- end vita49 ---------------------------------------------------
        //old= int iddax = slicenum.toInt()+1;//2.76.6

        if (ptt==RIG_PTT_ON)
        {
            fsdrs_poll = true;
            get_freq();
            ptt_cmd = "1";

            //----- vita49 ---------------------------------------------------
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
            //----- end vita49 ---------------------------------------------------

            writeData("dax audio set "+QString("%1").arg(iddax)+" tx=1",false,NULL);
        }
        else ptt_cmd = "0";
        writeData("slice set "+slicenum+" tx=1",false,NULL);
        writeData("xmit "+ptt_cmd,false,NULL);

        //----- vita49 ---------------------------------------------------
        // Stop the DAX TX audio on unkey.  Without this the radio keeps
        // transmitting what we send and stays in UNKEY_REQUESTED at full
        // power until the stream is torn down -- the "PTT does not release"
        // bug (measured on a FLEX-6600, 2026-09-08).
        if (ptt!=RIG_PTT_ON && is_vita && vita_ && vita_tx_active)
        {            
#if defined DEBUG_FLEX_TRACE //if (flex_trace) fprintf(stderr, "[MSHV Flex] TX audio OFF (unkey)\n");
			fprintf(stderr, "[MSHV Flex] TX audio OFF (unkey)\n");
#endif            
            QMetaObject::invokeMethod(vita_, "SetTxKeyed", Qt::QueuedConnection, Q_ARG(bool, false));
        }
        //----- end vita49 ---------------------------------------------------
    }
}
void Network::set_freq(unsigned long long freq)
{
    char *a = (char*)"1221";
    emit EmitWriteCmd(a,4);//fictive stop poping timer
    if (s_ModelID==0) writeData("set frequency-hz "+QString("%1").arg(freq),true,NULL);
    else if (s_ModelID==1)
    {
        char value[MAXXMLLEN];
        sprintf(value, "<params><param><value><double>%.0f</double></value></param></params>", (double)freq);
        writeData("rig.set_vfoA",false,value);
    }
    else if (s_ModelID==2)
    {
        //<command:10>CmdSetFreq<parameters:17><xcvrfreq:5>21230
        //freq = (int)(round(freq/1000.0));
        //freq = freq/1000;
        //QString s = QString("%1").arg(freq);
        QString s = QString {"%L1"}.arg (freq/1e3+1e-4,10,'f',3);
        s = ("<xcvrfreq:"+QString("%1").arg(s.count())+">"+s);
        s = ("<command:10>CmdSetFreq<parameters:"+QString("%1").arg(s.count())+">"+s);
        writeData(s,false,NULL); //qDebug()<<s;
    }
    else if (s_ModelID==3 || s_ModelID==4)//tci
    {
        //no needed HV -> if (!tci_tx_enable) return;
        writeData("vfo:"+tci_trx+",0,"+QString("%1").arg(freq)+";",false,NULL); //writeData("vfo:"+tci_trx+",0,"+QString("%1").arg(freq)+";",false,NULL);
    }
    else if (s_ModelID==5)
    {
        //F, set_freq 'Frequency'
        writeData("\\set_freq "+QString("%1").arg(freq)+"\n",false,NULL); //writeData("F "+QString("%1").arg(freq)+"\n",false,NULL);
    }//qDebug()<<"freqbuf="<<"set frequency-hz "+QString("%1").arg(freq);
    else if (s_ModelID==6)
    {
        QString frq = "FA";
        frq.append(QString("%1").arg(freq,11,10,QChar('0')));
        frq.append(";");
        writeData(frq,false,NULL); //writeData("F "+QString("%1").arg(freq)+"\n",false,NULL);
    }
    else if (fsdrs)//FlexRadio SmartSDR Slice A-H
    {
        double freqd = ((double)freq / 1000000.0);//"slice tune %d %.6f autopan=1"
        writeData("slice tune "+slicenum+" "+QString("%1").arg(freqd,0,'f',6)+" autopan=1",false,NULL);
        fsdrs_poll = true;
        vita_last_freq_hz = freq;   //flex native vita-49: so a slice we CREATE starts here
    }
}
void Network::set_mode(QString str)
{
    char *a = (char*)"1221";
    emit EmitWriteCmd(a,4);//fictive stop poping timer
    if (s_ModelID==0)//Ham Radio Deluxe
    {
        QString res = "Mode";//get dropdown-text {Main Mode} res-> Main Mode: LSB
        if (!rig_mode_str_nt[0].isEmpty()) res=rig_mode_str_nt[0];
        else if (!rig_mode_str_nt[1].isEmpty()) res=rig_mode_str_nt[1];
        else if (!rig_mode_str_nt[2].isEmpty()) res=rig_mode_str_nt[2];
        res.replace(" ","~");
        if 		(str=="USB" ) writeData("set dropdown "+res+" USB 1",true,NULL);
        else if (str=="DIGU") writeData("set dropdown "+res+" DATA-U 11",true,NULL);//2.74 DATA-U 11=?
        else if (str=="LSB" ) writeData("set dropdown "+res+" LSB 0",true,NULL);
    }
    else if (s_ModelID==1)//FLRig
    {
        if 		(str=="USB" ) writeData("rig.set_modeA",false,(char*)"<params><param><value>USB</value></param></params>");
        else if (str=="DIGU") writeData("rig.set_modeA",false,(char*)"<params><param><value>DATA-U</value></param></params>");
        else if (str=="LSB" ) writeData("rig.set_modeA",false,(char*)"<params><param><value>LSB</value></param></params>");
    }
    else if (s_ModelID==2)//DX Lab Suite Commander
    {
        QString s = "USB";//<command:10>CmdSetMode<parameters:7><1:2>CW
        if (str=="LSB") s = "LSB";
        if (str=="DIGU") s = "DATA-U";
        s = ("<1:"+QString("%1").arg(s.count())+">"+s);
        s = ("<command:10>CmdSetMode<parameters:"+QString("%1").arg(s.count())+">"+s);
        writeData(s,false,NULL); //qDebug()<<s;
    }
    else if (s_ModelID==3 || s_ModelID==4)//tci
    {
        QString s = "usb";
        if (str=="LSB") s = "lsb";
        if (str=="DIGU") s = "digu";
        writeData("modulation:"+tci_trx+","+s+";",false,NULL); //writeData("modulation:"+tci_trx+","+s+";",false,NULL);
    }
    else if (s_ModelID==5)//NET RigCtl
    {
        QString s = "USB";
        if 		(str=="LSB" ) s = "LSB";
        else if (str=="DIGU") s = "PKTUSB";
        writeData("\\set_mode "+s+" 3000\n",false,NULL);//writeData("M "+s+"\n",false,NULL);
    }//qDebug()<<"MODE="<<str;
    else if (s_ModelID==6)
    {
        QString s = "MD2;";//usb
        if 		(str=="LSB" ) s = "MD1;";
        else if (str=="DIGU") s = "MD9;";
        writeData(s,false,NULL);
    }
    else if (fsdrs)//FlexRadio SmartSDR Slice A-H
    {
        QString s = "USB";//"slice set %d mode=%s"
        if 		(str=="LSB" ) s = "LSB";
        else if (str=="DIGU") s = "DIGU";
        writeData("slice set "+slicenum+" mode="+s,false,NULL);
        //writeData("filt "+slicenum+" 100 3300",false,NULL); //for bandwidth "filt %d 0 %ld"   C19|filt 0 100 2800
        fsdrs_poll = true;
    }
}
void Network::get_freq()
{
    if (s_ModelID==0)
    {
        if (cinit_id_nt>-1) return;
        writeData("get frequency",true,NULL);
    }
    else if (s_ModelID==1)
    {
        if (cretr_nt !=-2) return;
        writeData("rig.get_vfoA",false,NULL);//false fictive
    }
    else if (s_ModelID==2)
    {
        if (cretr_nt !=-2) return;
        writeData("<command:10>CmdGetFreq<parameters:0>",false,NULL);//false fictive
    }
    else if (s_ModelID==3 || s_ModelID==4)//tci
    {
        if (cretr_nt !=-2) return;
        //if (is_my_trx) return; //stop plling on tx    is_tci_trx    is_my_trx
        writeData("vfo:"+tci_trx+",0;",false,NULL); //writeData("vfo:"+tci_trx+",0;",false,NULL);
    }
    else if (s_ModelID==5)
    {
        if (cretr_nt !=-2) return;
        writeData("\\get_freq\n",false,NULL);  //writeData("f\n",false,NULL);
    }
    else if (s_ModelID==6)
    {
        if (cretr_nt !=-2) return;
        writeData("IF;",false,NULL);//writeData("FA;",false,NULL);//or ZZFA
    }
    else if (fsdrs)//FlexRadio SmartSDR Slice A-H TCP
    {
        if (cretr_nt !=-2) return;
        if (!fsdrs_poll) return;
        fsdrs_poll = false;
        writeData("sub slice "+slicenum,false,NULL);//"sub slice 0"*/
    }
}
void Network::get_mode()
{
    if (s_ModelID==0)
    {
        if (cinit_id_nt>-1) return;
        QString res = "Mode";//get dropdown-text {Main Mode} res-> Main Mode: LSB
        if (!rig_mode_str_nt[0].isEmpty()) res=rig_mode_str_nt[0];
        else if (!rig_mode_str_nt[1].isEmpty()) res=rig_mode_str_nt[1];
        else if (!rig_mode_str_nt[2].isEmpty()) res=rig_mode_str_nt[2];
        writeData("get dropdown-text {"+res+"}",true,NULL);
    }
    else if (s_ModelID==1)
    {
        if (cretr_nt !=-2) return;
        writeData("rig.get_modeA",false,NULL);//false fictive
    }
    else if (s_ModelID==2)
    {
        if (cretr_nt !=-2) return;
        writeData("<command:11>CmdSendMode<parameters:0>",false,NULL);//false fictive
    }
    else if (s_ModelID==3 || s_ModelID==4)//tci
    {
        if (cretr_nt !=-2) return;
        //if (is_my_trx) return;//stop plling on tx
        writeData("modulation:"+tci_trx+";",false,NULL); //writeData("modulation:"+tci_trx+";",false,NULL);
    }
    else if (s_ModelID==5)
    {
        if (cretr_nt !=-2) return;
        writeData("\\get_mode\n",false,NULL); //writeData("m\n",false,NULL);
    }
    else if (s_ModelID==6)
    {
        if (cretr_nt !=-2) return;
        writeData("IF;",false,NULL);//writeData("MD;",false,NULL);//or ZZMD
    }
    else if (fsdrs)//FlexRadio SmartSDR Slice A-H TCP
    {
        if (cretr_nt !=-2) return;
        if (!fsdrs_poll) return;
        fsdrs_poll = false;
        writeData("sub slice "+slicenum,false,NULL);//"sub slice 0"*/
    }
}
/*void Network::SetReadyRead(QByteArray,int)
{}*/
////////////////////////////////////////////////////////end new read com