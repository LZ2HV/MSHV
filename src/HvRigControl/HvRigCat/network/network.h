/* MSHV
 * By Hrisimir Hristov - LZ2HV
 * May be used under the terms of the GNU General Public License (GPL)
 */
/* 
MSHV TCI Client
Copyright (c) 2017 Expert Electronics
Distributed under the MIT software license, see the accompanying
file COPYING or http://www.opensource.org/licenses/mit-license.php.
TCI Client modified by Hrisimir Hristov, LZ2HV 2021
*/

#ifndef NETWORK_H
#define NETWORK_H

#include <QWidget>
#include <QTimer>

#include "../rigdef.h"
#define QT_NO_DEBUG_STREAM // remove qdebug socet
#include <QTcpSocket>
#include <QtWebSockets/QWebSocket> //tci
#include <QThread>

class ThreadRefr : public QThread
{
    Q_OBJECT
signals:
    void refresh();
private:
    //QMutex m_mutex;
    void run()
    {
        while (1)
        {
            //QMutexLocker locker(&m_mutex);
            msleep(11);
            emit refresh();         
        }
    }
};
extern bool _SetTxAudioTci_(int *raw);//, int size
extern void _SetTciBuffReset_();
#define DSMAX 32768 //32768
#define BCMAX 30
class HvWebSocket : public QWebSocket
{
    Q_OBJECT
public:
    HvWebSocket(quint32);
    virtual ~HvWebSocket();
	void SetBufferCommand(QString);	
	//void SetSamplerate(int);

//signals:
 
//public slots:

private slots:
	void ReadCommand();
	void mBinaryMessageReceived(const QByteArray &);
	//void StateChanged(QAbstractSocket::SocketState);
	//void onError(QAbstractSocket::SocketError errorCode);//tci

private:
	quint32 tci_trx;
	int sample_rate;//tci
	QByteArray t_txAudioData; //tci
	int rawrxm[DSMAX+4096];  //tci=8192<-max]; 16384
	uint8_t pos_writ;
	uint8_t pos_read;
	QString wcommands[BCMAX+10];
};

class Network : public QWidget
{
    Q_OBJECT
public:
    Network(int ModelID,QWidget *parent = 0);
    virtual ~Network();

signals:
    void EmitRigSet(RigSet);
    void EmitWriteCmd(char*data,int size);
    void EmitReadedInfo(CmdID,QString);
    void EmitNetConnInfo(QString,bool,bool);//info,connect,ready to use
    void EmitFullRigInfo(QString);//2.76.1 for pskreporter
 
public slots:
	void ConnectNet(QString);	
	//void SetMode(int);//tci
	void SetOnOffCatCommand(bool,int,int);//tci
	//void SetTciRig(int);//tci

private slots:
    void initAll();
    void readNet();
    void connected_s();
    void disconnected_s();
    void SetCmd(CmdID,ptt_t,QString);
    //void SetReadyRead(QByteArray,int); 
    void wTextMessageReceived(const QString &);//tci
    void SetTciTxOnRX2();//tci
    void SetTciSelect(int);//tci
    //void onError(QAbstractSocket::SocketError errorCode);//tci

private:
	QString GetModeStr(QString);
	QString GetModeStrKenwood(QChar);
    QString s_nethost;
    QString s_netport;
     
    QString s_tcich;
    QString s_tcismp;
    QString s_tcityp;
    QString s_tcitxbuff;
    QString s_tcisamprate;
    
	QTcpSocket *socket;
	bool is_wsocket;//tci
	QThread mThread;//tci
	HvWebSocket *wsocket;//tci
	void connectToHost();
	bool writeData(QString str,bool id,char*);
	
	bool tci_start_stop_state;//tci
	bool is_tci_trx; //tci
	QString tci_trx; //tci
	//bool tci_tx_enable[2];//tci
	//bool tci_rx_enable[2];//tci
	bool tci_tx_enable;//tci
	bool tci_rx_enable;//tci
	bool tci_rx_mute;//tci
	int tci_drive;//tci
	bool tci_split_enable;//tci
	QString wdemanf;//tci
	QString wdevice;//tci
	QStringList lsV012;
	int id_tci_prot;//tci
	//int sample_rate;//tci
	int tci_select;//tci
	bool isMyTCICommand(QString);//tci
	void SetTciStrtStopAudio(bool);
	
	//flrig
	char *FLRig_xml_build(char *cmd, char *value, char *xmlbuf,int xmllen);
	QString FLRig_get_value(QString);
	//
	int seqnum;
	QString slicenum;
	bool fsdrs;
	bool fsdrs_poll;

    int  s_ModelID;
    void set_ptt(ptt_t);    
    int s_CmdID;
    void set_freq(unsigned long long);
    void get_freq();
    void set_mode(QString);
    void get_mode();
protected:
    QTimer *timer_init;

};
#endif
