/* MSHV RadioAndNetW for Log program
 * Copyright 2015 Hrisimir Hristov, LZ2HV
 * May be used under the terms of the GNU General Public License (GPL)
 */
#ifndef RADIONETW_H
#define RADIONETW_H

#include "../../config.h"
#include <QObject>
#include <QString>
//#include <QHostAddress>
//#include <QHostInfo>
#include <QQueue>
#include <QHash>

class MessageClient;
class QTimer;
class QHostInfo; //HV is here QtGui

#include "../hvqthloc.h"
#define QT_NO_DEBUG_STREAM // remove qdebug from tcp socet
#include <QTcpSocket>

//explicit PSK_Reporter(MessageClient *, QObject *parent = nullptr);
class HvTcpClient : public QObject
{
    Q_OBJECT
public:
    explicit HvTcpClient(QObject *parent = nullptr);

    void DisconectOnly();
    bool writeData(QString);

signals:
    void ConectionInfo(QString);

public slots:
    void Connect(QString host,QString port,QString login_name,QString);
    //void SetData(QString data);

private slots:
    void connectToHost();
    void readTelnet();
    void connected_s();
    void disconnected_s();

private:
    //uint8_t stat_disp;
	QRegExp rx_login;
	QRegExp rx_passw;
    HvQthLoc THvQthLoc;
    bool tray_once_agen;
    bool is_pfx(QString s);
    QString FindBaseCallRemAllSlash(QString str);
    //bool f_conect;
    //QTimer *t_tcp_connect;
    QString s_tcphost;
    QString s_tcpport;
    //QString s_tcpdata;
    QString s_login_name;
    QString s_pass;
    QTcpSocket *socket;
    QString itcp_call_err;

protected:

};

#include <QDialog>
#include <QComboBox>
#include <QPushButton>
#include "../hvinle.h"
class SpotDialog : public QDialog
{
    Q_OBJECT
public:
    SpotDialog(bool,QWidget * parent = 0 );
    ~SpotDialog();

    void setAllEdit(QStringList list,int id,int offset);//id 0=psk 1=dx_spot  10=empty
    QStringList getAllEdit();
    bool send_spot_result;
    void SetTcpconInfo(QString);
    void SetDistUnit(bool f_);
    void SetLocFromDB(QString);
    void SetFont(QFont);

signals:
    void EmitReconnect();
    void FindLocFromDB(QString);

private:
    int s_f_offset;
    QString s_mode;
    bool block_loc;
    bool f_km_mi;
    QString CalcDistance(QString myl, QString hisl);
    HvQthLoc THvQthLoc;
    QLabel *l_tcpcon_info;
    QPushButton *b_recon_host;
    QString s_end_info;
    HvInLe *le_my_call;
    HvInLe *le_my_loc;
    HvInLe *le_freq;
    HvInLe *le_dx_call;
    HvInLe *le_dx_loc;
    QComboBox *Cb_prop;
    QStringList id_prop;
    QLineEdit *le_remarks;
    QPushButton *b_send_spot;
    QPushButton *b_cancel_spot;
    QLabel *l_loc_from_db;
    //bool
    //QPushButton *pb_lookup_loc;

private slots:
    void CheckForWalidSpot(QString);
    void LookupLocFromDb();
    void SendSpot();
    void CancelSpot();
    void InfoChanged(QString);
    void PropChanged(QString);//for set fokus
    void Check(QString);
    //void InfoChanged2(QString);
    //void MyLocChang(QString);
    void FreqEntered();
    void DxCallEntered();
    //void DxLocEntered();
    void RemarksEntered();

};

class EditRadInfo : public QDialog
{
    Q_OBJECT
public:
    EditRadInfo(QWidget *parent = 0);
    virtual ~EditRadInfo();

signals:
    void EmitSetInfo(QStringList,int);
	
public slots:
    void SetEditStInfo(QStringList,int);
	    
private slots:
   void ApplayInfo();
    
private:
   QLineEdit *ant;
   //HvInLeFreq *frq;
   QVBoxLayout *VB_Freq = new QVBoxLayout();
   QLabel *l_band;
   QPushButton *pb_applay_info;
   QPushButton *pb_cancel_info;
   int s_edit_index;

};
 
#include <QWidget>
#include <QDialog>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QGroupBox>
#include <QLabel>
#include <QCheckBox>
#include <QFile>
#include <QMessageBox>
#include <QSslSocket>
#include <QUdpSocket>
//#include "radlist.h"

#include <QTreeView>
#include <QStandardItemModel>
class HvRadList : public QTreeView
{
    Q_OBJECT
public:
    HvRadList(bool,QWidget *parent = 0);
    virtual ~HvRadList();

    QStandardItemModel model;
    void Clear_List();
    void InsertItem_hv(QStringList);
    void SetItem_hv(QStringList,int);

    int GetListCount()
    {
        return 	model.rowCount();
    };

signals:
	void EmitDoubleClick(QStringList,int);

private:
	bool dsty;
    bool f_row_color;

protected:
	//void mousePressEvent (QMouseEvent*);
	//void drawRow(QPainter *painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const;
	void paintEvent(QPaintEvent *);
	void mouseDoubleClickEvent(QMouseEvent * event);
	//void resizeEvent(QResizeEvent * event);
	//void keyPressEvent(QKeyEvent * event);
};

#include "bcnlistw.h"
#include "pskreporterudptcp.h"
class RadioAndNetW : public QWidget
{
    Q_OBJECT
public:
    RadioAndNetW(QString,QString path,bool,int,int,QWidget *parent = 0);
    ~RadioAndNetW();

    //void SetFileOpen();
    void CloseBcnList();
    //void CreateDialogSpot(QWidget*);
    //void StartEmptySpotDialog();
    void SetFullRigInfo(QString);
    void SetLocalStation(QString myCall,QString myLoc,QString band,int con_id);
    void AddRemoteStation(QString hisCall,QString hisLoc,int frq,QString mode,int snr,int id,QString pwidth,QString);
    void SetLocFromDB(QString);
    void SetDistUnit(bool);
    void SetModeForFreqFromMode(int i);
    void SetFreqGlobal(QString);
    void SetFont(QFont f);
    void SendDecodTxt(QString tim,int sn,QString dt,int frq,QString msg);///*,QString mod*/
    void SetAuto(bool);
    void SetTx(bool);
    void SetDxParm(QString,QString,QString);
    void SetUdpDecClr();
    void FindFreqRadList(int id,QString &sfrq);
    void SetTxMsg(QString);
    void StartEmptySpotDialog();
    void ShowBcnList();
    void FindFreqLocFromBcnList(QString hisCall,int id,QString &hisLoc,QString &sfrq);
    void SaveSettings();
    void SendOtpCheck(QString s);

    bool GetUdpBroadLoggedQso()
    {
        return cb_udp_broad_log_qso->isChecked();
    };
    bool GetUdpBroadDecod()
    {
        return cb_udp_broad_decod->isChecked();
    };
    /*bool GetUdpBroadLoggedAdif()
    {
        return cb_udp_broad_log_adif->isChecked();
    };
    bool GetTcpBroadLoggedAdif()
    {
        return cb_tcp_broad_log_adif->isChecked();
    };
    bool GetClubLogAdif()
    {
        return cb_clublog->isChecked();
    };
    bool GetUdp2BroadLoggedAdif()
    {
        return cb_udp2adif->isChecked();
    };
    bool GetQRZLogAdif()
    {
        return cb_qrzlog->isChecked();
    };*/
    bool GetLogAdifAll()
    {
        bool res = false;
        if 	(cb_udp_broad_log_adif->isChecked() ||
             cb_tcp_broad_log_adif->isChecked() ||
             cb_clublog->isChecked() ||
             cb_udp2adif->isChecked() ||
             cb_qrzlog->isChecked() ||
             cb_eqsl->isChecked()) res = true;
        return res;
    }
    void GetFtFr(long long int *a);
    QWidget *GetRadListW()
    {
        return TRadListW;
    };
    void RefreshOtpKeyMsg();    

signals:
    void FindLocFromDB(QString);
    void EmitUdpBroadLoggedAll(bool,bool);
    void EmitUdpCmdDl(QStringList);
    void EmitUdpCmdStop(bool);
    void EmitOpenRadNetWToRecon();
    void EmitUploadClubLogInfo(QString);
    void EmitOtpTxKey(QString);
    void EmitOtpRxMsg(bool);
    void EmitOtpVerif(QString,uint8_t);
    

public slots:
    void ConDiscon();
    void TCPServPortChanged(QString);
    //void SetLocFromDB(QString);
    //void ReadHttpRequest(QNetworkReply*);
    //void readTelnet();
    //void displayError(QAbstractSocket::SocketError socketError);
    //void ConectToTelnetServer();
    //void FindFreqRadList(int id,QString &sfrq);
    //void FindFreqLocFromBcnList(QString hisCall,int id,QString &hisLoc,QString &sfrq);
    void SendLoggedQSO(QStringList ls);
    void SendAdifRecord(QString);
    void replayDecodes();
    void networkErrorUDPBrodcast(QString const&);
    void SetUploadSelected(QByteArray);
    
private:
    QWidget *w_parent;
    int s_cont_id;
    bool f_mods_accept_cmd;
    QString s_mode;
    QString s_smode;
    //int prev_pos_dec;
    int id_activ_upd;
    int pos_dec;
    int pos_upd;
    QString s_dx_call;
    QString s_report;
    QString s_dx_grid;
    bool s_auto;
    bool s_tx;
    QString s_tx_msg;
    void SendStatus(int);

    QString FREQ_GLOBAL;
    QString s_mode_str_for_ferq;
    HvQthLoc THvQthLoc;
    bool tcp_conect_stat;
    void SetGlobalTcpStat(QString);
    bool global_tcp_stat;

    HvTcpClient *TelnetClient;
    //QTcpSocket *tcpSocket;
    //QString currentFortune;
    //quint16 blockSize;
    //void ConectToTelnetServer();
    //QHttp *http_post;
    //QNetworkAccessManager *networkManager;
    //void SendHttpMsg(QString my_c, QString dx_c, QString freq QString info);
    SpotDialog *TSpotDialog;
    HvBcnListW *THvBcnListW;
    //QList<QStringList> Bcnlist;
    //void ReadBcnList(QString);

    QWidget *TRadListW;
    HvRadList *THvRadList;
    EditRadInfo *TEditRadInfo;
    QString s_band;//vremenno
    QString sr_path;
    //void SaveSettings();
    bool isFindId(QString id,QString line,QString &res);
    void ReadSettings();

    PSKReporter *psk_Reporter;

    QCheckBox *cb_start_stop_psk_rpt;
    QCheckBox *cb_udp_tcp_psk_rpt;
    QLineEdit *UDPServer;
    QLineEdit *UDPPort;
    QLabel *l_con_info;
    QLabel *l_tcpcon_info;

    QString s_myCall;
    QString s_myLoc;
    QString s_rigInfo;

    QPushButton *pb_re_conect;

    //bool block_emit_tcp;
    void ReadTelnetList(QString path);
    QLineEdit *TCPServer;
    QLineEdit *TCPPort;
    QLineEdit *TCPPass;
    QPushButton *pb_tcp_conect;
    QComboBox *Cb_telnet;

    //void FindFreqLocFromBcnList(QString hisCall,int id,QString &hisLoc,QString &sfrq);
    //void FindFreqRadList(int id,QString &sfrq);
    void StartSpotDialod(QStringList,int f_offset,int id);
    QString RemBegEndWSpaces(QString str);

    MessageClient *m_messageClientBroad;
    QLabel *l_udp_broad_info;
    QCheckBox *cb_udp_broad_log_qso;
    QCheckBox *cb_udp_broad_log_adif;
    QCheckBox *cb_udp_broad_decod;
    QLineEdit *UDPServerBroad;
    QLineEdit *UDPPortBroad;
    QPushButton *pb_re_conect_udp_broad;
    QTimer *decod_upd_timer;

    QUdpSocket *socet_udp2_broad;
    QCheckBox *cb_udp2adif;
    QLineEdit *udp2_Server;
    QLineEdit *udp2_Port;

    QTcpSocket *socet_tcp_broad;
    QLineEdit *TCPServerBroad;
    QLineEdit *TCPPortBroad;
    QCheckBox *cb_tcp_broad_log_adif;
    QPushButton *b_reset_default_freqs_cont;

    bool cl_send_file;
    bool cl_err_flag;
    bool cl_once_data_err_msg;
    int cl_pos_ul_writ;
    int cl_pos_ul_read;
    int cl_is_act_send_check;
    QTimer *cl_upd_timer;
    QString file_cl_error;
    QSslSocket *socet_tcp_clublog;
    //QLabel *l_tcp_clublog_info;
    QLineEdit *LeClubLogServer;
    QLineEdit *LeClubLogPort;
    QLineEdit *LeClubLogPost0;
    QLineEdit *LeClubLogPost1;
    QCheckBox *cb_clublog;
    QLineEdit *LeClubLogMail;
    QLineEdit *LeClubLogPass;
    QLabel *LClubLogCall;
    void RefreshUdpOrTcpBroadLoggedAll();
    bool isUnsafe(QChar compareChar);
    QString decToHex(QChar num, int radix);
    QString convert(QChar val);
    QString URLEncode(QString strEncode);
    void CheckClubLogError(int,int);
    void StartStopClubLog(bool);
    void SendClubLogAdif(QString);

    int qrz_pos_ul_writ;
    int qrz_pos_ul_read;
    int qrz_is_act_send_check;
    bool qrz_once_data_err_msg;
    QString qrz_error_str;
    bool qrz_err_flag;
    QTimer *qrz_upd_timer;
    QSslSocket *socet_tcp_qrzlog;
    QLineEdit *LeQRZLogServer;
    QLineEdit *LeQRZLogPort;
    QLineEdit *LeQRZLogApi;
    QLineEdit *LeQRZLogPost;
    QCheckBox *cb_qrzlog;
    void CheckQRZLogError();
    void StartStopQRZLog(bool);
    void SendQRZLogAdif(QString);
    
    int eqsl_pos_ul_writ;
    int eqsl_pos_ul_read;
    bool eqsl_once_data_err_msg;
    bool eqsl_err_flag;
    int eqsl_is_act_send_check;
    QString eqsl_error_str;
    QLineEdit *LeEQSLServer;
    QLineEdit *LeEQSLPort;
    QLineEdit *LeEQSLPost;
    QLineEdit *LeEQSLUser;
    QLineEdit *LeEQSLPass;
    QLineEdit *LeEQSLQTHNick;
    QLineEdit *LeEQSLmsg;
    QTimer *eqsl_upd_timer;
    QSslSocket *socet_tcp_eqsl;//QSslSocket QTcpSocket
    QCheckBox *cb_eqsl;
    void CheckEQSLError();
    void StartStopEQSL(bool);
    void SendEQSLAdif(QString); 
    
    uint8_t otp_pos_ch_writ;
    uint8_t otp_pos_ch_read;
    uint8_t otp_is_act_send_check;
    QTimer *otp_check_timer;
    QSslSocket *socet_tcp_otp;
    QLineEdit *LeOtpServer;
    QLineEdit *LeOtpPort;
    QComboBox *CbOTPServers;
    QLineEdit *LeOtpKey;
    QCheckBox *cb_otp_key; 
    //QCheckBox *cb_otp_mamd_key;
    QCheckBox *cb_otp_msg;
    void StartStopOtp(bool);
    //bool StartStopOtp(bool);
    void SendOtpTxKey();
    void ParseOtpMsg(QString,uint8_t);
    //void SendSFoxCheckDataGram();
    //void SendSFoxCheck(QString s);
    
    QString AppPath;
    QCheckBox *cb_wr_status;
    QTimer *write_status_timer;

private slots:
    void StartStopTCPBroad(bool);
    void DecodUpdTimer();
    //void OpenRadNetWToRecon();
    void CbTelnetChanged(QString);
    void ServTextChanged(QString);
    void PortTextChanged(QString);
    void SetConectionInfo(QString);
    void SetConectionTcpInfo(QString);
    void StInfoChanged(QStringList,int);
    void Reconnect();
    void StartStopReport(bool);
    void UdpTcpChangedPsk(bool);
    void StartStopUdpBroad(bool);
    void ReconnectUdpBroad();
    void UDPSrvPortBroadChanged(QString);
    void ConectionInfoBroad(QString);
    void SetDefaultFreqsActType(int id);
    void SetDefaultFreqsActTypeBut();
    void ResetDefaultFreqsBut();
    void SetDefaultFreqs(bool f);
    void set_reply_clr(QStringList);
    void set_halt_tx(bool);
    //void connected_clublog();
    //void disconnected_clublog();
    void readClubLog();
    void cb_clublog_toggled();
    void UplClubLogAdif();
    void StartStopUdp2Broad(bool);
  	//void connected_qrzlog();
  	//void disconnected_qrzlog();
    void cb_qrzlog_toggled();
    void readQRZLog();
    void UplQRZLogAdif();    
    //void connected_eqsl();
    //void disconnected_eqsl();    
    void cb_eqsl_toggled();
    void readEQSL();
    void UplEQSLAdif();   
    void WriteStatusTimer();
    
	//void connected_sfox();
	//void disconnected_sfox(); 
	//void sslError( QList<QSslError> errors );   
	void cb_otp_key_toggled();
	void cb_otp_msg_toggled(bool);
    void LeOtpKeyEdited(QString);
    void readOtp();
    void SendOtpCheckDataGram();
    void CbOTPServersChanged(QString);

};

#endif
