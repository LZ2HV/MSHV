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
*/
#define _NETWORK_RIGS_
#include "network_def.h"
#include "network.h"
#include <unistd.h>
#include <QLocale>

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
bool _SetTxAudioTci_(int *raw)//, int size
{
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
                    	qDebug()<<hh0;
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
                    int hh = rawtxbuff[rawtx_pos];
                    if (hh>hh0)
                    {
                    	hh0=hh;
                    	qDebug()<<hh0;
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
void Network::SetTciSelect(int i)//tci 0=non 1=rx 2=tx 3=rx,tx
{
    tci_select = i;
    if (s_ModelID==3 || s_ModelID==4)//2.76.1 tci
    {
        if (tci_select>0) SetTciStrtStopAudio(true);
        else SetTciStrtStopAudio(false);
    }
}
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
            wsocket->open(url); //qDebug()<<tci_19<<s_nethost;
            wsocket->moveToThread(&mThread);
            mThread.start();
        }   //else wsocket->SetCommand("0"+s_nethost+" "+s_netport); // without delete on disconnect ???
    }
    else
    {
        if (socket->state() == QAbstractSocket::ConnectedState)
        {
            socket->disconnectFromHost(); //qDebug()<<"Cnect to Disconect=";
            //socket->waitForDisconnected(300);
            return;
        }
        int p1 = s_netport.toInt();
        socket->connectToHost(s_nethost, p1);
        socket->waitForConnected(350);//importent for TCP hv
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
int cretr_nt = -1;
void Network::connected_s()
{
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
    emit EmitNetConnInfo("<font color='red'>"+tr("Disconnected")+"</font>",false,false);
    if (is_wsocket)//  && wsocket
    {
        mThread.exit();
        usleep(22000);//2.76.2 old=usleep(4000);
        delete wsocket; //wsocket->deleteLater();
        is_wsocket = false;
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
        if (cretr_nt>4)
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
        writeData("info",false,NULL);//writeData("sub slice "+slicenum,false,NULL);C16|info
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
        ls1 <<""<<"";
		//qDebug()<<"MSHV Receive <-"<<ls1;
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
            R16|0|model="FLEX-6700",S67319A86|slice 0 in_use=1 sample_rate=24000 RF_frequency=10.137000 mode=DIGU client_handle=0x76AF7C73
            */
            sba.replace(',',' ');
            isGetRadio = false;
            int id0 = sba.indexOf("model=");
            if (id0>-1)
            {
                timer_init->stop();
                cretr_nt = -2;
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
        int iddax = slicenum.toInt()+1;
        if (ptt==RIG_PTT_ON)
        {
            fsdrs_poll = true;
            get_freq();
            ptt_cmd = "1";
            writeData("dax audio set "+QString("%1").arg(iddax)+" tx=1",false,NULL);
        }
        else ptt_cmd = "0";
        writeData("slice set "+slicenum+" tx=1",false,NULL);
        writeData("xmit "+ptt_cmd,false,NULL);
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