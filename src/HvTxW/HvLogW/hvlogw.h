/* MSHV HvLogW for Log program
 * Copyright 2015 Hrisimir Hristov, LZ2HV
 * May be used under the terms of the GNU General Public License (GPL)
 */

#ifndef HVLOGW_H
#define HVLOGW_H

#include <QWidget>
#include <QVBoxLayout>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QMenu>
#include <QMouseEvent>
#include <QCheckBox>
#include <QFileDialog>
#include <QMenuBar>
//#include <QDesktopWidget>
#include <QTimer>
#include <QLineEdit>
#include <QProgressDialog>
#include <QGroupBox>
//#include <QSpinBox>
//#include "hvloglist.h"
#include "../../config.h"
//#include "hveditw.h"


//#include "HvLogDetiles/hvlogdetiles.h"
//#include "HvDbView/hvdbview.h"
//#include "HvMemBut/mem_butt.h"
//#include "hveditw.h"
 
//#include <QtGui>

class HvProgressD : public QProgressDialog
{
    //Q_OBJECT
public:
    HvProgressD(QWidget *parent = 0);
    virtual ~HvProgressD();
	void Show(QString title,QString label,int range);
	void SetValue(int i);
	void Finish(int i);

//signals:

//public slots:    

private:
	int c_barr;
	int max_range;
	QString s_label;
	
//protected:

};

#include <QTreeView>
#include <QHeaderView>
#include <QClipboard>
#include "../hvstditmmod.h"
class HvLogList : public QTreeView
{
    Q_OBJECT
public:
    HvLogList(bool,QWidget *parent = 0);
    virtual ~HvLogList();

	//void SetFocus();
    HvStandardItemModel model;
    void Clear_List();
    void SetItem_hv(QStringList,int);
    void SetEditRow(int);
    void InsertItem_hv(QStringList);
    int GetListCount()
    {
        return 	model.rowCount();
    };

signals:
	//void SendRightClick();
	void EmitDoubleClick();
	void SortClicked(int);

public slots:
    void DeleteSel();
    
private slots:
	void HideSections();
	
private:
    //HvHeaderLog *THvHeader;
    QHeaderView *THvHeader;
    int ActiveIndex;
    QColor ActiveRowText;
    QColor ActiveRowBackg;
    QClipboard *clipboard;
    QString last_logged_dt;   
    int enum_sec_logged;

protected:
	//void mousePressEvent (QMouseEvent*);
	void drawRow(QPainter *painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const;
	void paintEvent(QPaintEvent *);
	void mouseDoubleClickEvent(QMouseEvent * event);
	void keyPressEvent(QKeyEvent * event);
};

#include "../hvqthloc.h"
#include <QComboBox>
#include "../hvinle.h" 
class HvEditW : public QWidget
{
    Q_OBJECT
public:
    HvEditW(QString name, QString bt_cancel_txt,QString bt_corr_tx,bool,QWidget *parent = 0);
    virtual ~HvEditW();

	void SetEdit(QStringList,int);
	//oid SetSettings(QString);
	//void SetDupe(bool,int duped_qso);
	void SetFont(QFont f);
	void SetSatIdNames(QStringList,QStringList);
	
signals:
    void EndEdit();
    //void CheckCall(QString,int);
    void SendCorrContact(QStringList,int);

private slots:
    void CorrContact();
    void Check(QString s_type);
    void ExchangeChanged(QString);
    void PropChanged(QString);

private:
    HvQthLoc THvQthLoc;
    QPushButton *bt_corr;
    QPushButton *bt_cancel;
    //QDateEdit *de_bdate_s;
	//QTimeEdit *te_time_s;
	QDateTimeEdit *dte_s;
	//QDateEdit *de_bdate_e;
	//QTimeEdit *te_time_e;
	QDateTimeEdit *dte_e;
	QComboBox *Cb_mode;
	QComboBox *Cb_band;
	//QLabel *l_dist;
	//QLabel *l_beam;
	HvInLe *le_call;
    HvInLe *le_txrst;
    //HvInLe *le_txsn;
    HvInLe *le_rxrst;
    //HvInLe *le_rxsn;
    HvInLe *le_loc;
    QLineEdit *le_comment;
    HvInLe *le_freq;
    QComboBox *cb_prop;
    QComboBox *cb_sat_mod;
    QComboBox *cb_sat_nam;
    QStringList s_id_sat_nam; 
    QLineEdit *le_rx_freq;   
    
    QLineEdit *le_txsn;
    QLineEdit *le_rxsn;
    HvLeWithSpace *le_txex;
    HvLeWithSpace *le_rxex;
    
    bool block_txrst;
    bool block_rxrst;
    bool block_rxsn;
    bool block_call;
    bool block_loc;
    bool block_dupe;
    
    QString myLocator;
    QString s_dist;

    int index_edit;
    int next_dupe;
    bool first_dupe_flag;
    int first_dupe;
    QString s_enum_sec_sta;
    QString s_enum_sec_end;
    QComboBox *cb_cont_id;
    QComboBox *cb_cabrillo_trmN;
    bool FDCheck(QString);
    bool RUCheck(QString);

};

class HvLogW : public QWidget
{
    Q_OBJECT
public:
    HvLogW(QString,QString app_path, bool,int,int,QWidget *, QWidget * parent = 0);
    virtual ~HvLogW();

   	bool Insert(QStringList,bool,bool,int,bool,bool);
   	void SetMyCallGridExchAllCont(QString,QString,QString);	
   	void IsCallDupeInLog(QString,QString,QString,int,bool &);
   	void SetFont(QFont f);
   	void SetSettings(QString); 
   	QString GetSettings();		
   	void SetMarkTextAllRuleChanged(bool *f,int mode,QString band);
   	void SetFreqGlobal(QString);
   	void Show_log();
	QString GetPosXYWH()//2.48
	{
		QString str = QString("%1").arg(pos().x())+"#"+QString("%1").arg(pos().y());
		if (windowState() == Qt::WindowMaximized)
        {
            str.append("#FULL#FULL");
        }
        else
        {
        	str.append("#"+QString("%1").arg(this->width())+"#"+QString("%1").arg(this->height()));
        }
		return str;				
	}
	QString GetUseAdifSave() {return QString("%1").arg(ac_use_adif_save->isChecked());}
	void ReadEDILog();//2.57
	void SetPosXYWH(QString);//2.48
	void SetDistUnit(bool);
	void SetUseAdifSave(QString);
	void SetMaxLogQsoCount(QString);
	QString GetMaxLogQsoCount();
	void SetAutoLogInfo();//2.75
	QString GetPropSettings();//2.75
	void SetPropSettings(QString);//2.75
	void SetLogAutoComm(bool f);//2.76.3

public slots:
   void EditQso();
   void SetUdpBroadLoggedAll(bool,bool);
   void SetUploadClubLogInfo(QString);

signals:
   void EmitLoggedQSO(QStringList);
   void EmitAdifRecord(QString);
   void EmitMarkTextLogAll(QStringList,int);
   void EmitUploadSelected(QByteArray);
   void EmitCBEnableAliChanged(bool);

private slots:
    void DeleteSelected();
    void End_Edit();
    //void SetEditText();
    void SetCorrContact(QStringList list,int index_edit);
    void Find();
    void DefaultSort();
    void SortClicked(int);
    void AddExtrnalAdif();
    void AddExtrnalLog();
    void AddManQso();
    void End_Add_Man_Qso();
    //void SetManQsoText();
    void AddManQsoToList(QStringList list,int index_edit);
    void ExportSelToAdif();
    void ExportAllToAdif();
    void ExportToCabr();
    //void ViewDialogStatstic();    
    void OkAddToLog();
    void CancelAddToLog();
    void RefreshSave();    
    void OkCabr();
    void CancelCabr();
    void CbContNameChanged(int);
    void CreateNewLogAndSaveOld();
    void CreateBackupLog();
    void CbContTrmChanged(int);
    void UploadSelToClubLog();
    void PropChanged(QString);

private:
	//QDialog *DStatistic;
	//QTextBrowser *TBStatistic;
	//QWidget * w_parent;
	QAction *ac_use_adif_save;
	HvQthLoc THvQthLoc;
	bool f_km_mi;
	QString CalcDistance(QString);
	int beg_append;
	bool save_in_new_format;
	bool fsave_busy; 
	QString FREQ_GLOBAL;
	void RefreshCbTrmN(int);
	void CreateNBLog(bool);
	bool isDuplicate(QStringList l,QString s,int b);
    void GetMarkTextAllFromLog_p(QString band,QString mode);
    QString ExtractAdifRecord(QString in,QString str);
    //QString GetFREQall(int,int);
    QString MakeAdifString(int l_row);
    QStringList MakeLoggedQSO(int l_row);
    void ExportToAdif(QString ident,int f_sel_or_all,bool pbarr);
    HvInLe *le_find;
    int find_count_row;
    int find_count_column;
    bool f_found;
    QModelIndex s_index;
    QLabel *l_qsos_in_log;
    QPushButton *b_edit_qso;
    QPushButton *b_add_man_qso;
	QString AppPath;
	QString FullPath;
	QString FileName;
	//QString FullName;
    HvLogList *THvLogList; 
    //HvLogDetiles *THvLogDetiles;
    void SaveEDI(QString,bool);//2.48 append
    void ReadEDI(QString,bool pbarr,bool check_format);
    HvEditW *THvEditW;
    HvEditW *AddManQsoW;
    QString s_my_call;
    QString s_my_grid;
    QString s_my_fd_exch;
    QString s_my_ru_exch;
    bool f_iem_ru_dx;
    QString GetBandInLambda(QString frq);        
    QDialog *add_to_log_dialog;
    QLabel *l_warn;
    QLabel *add_to_log_txt;
    QLabel *l_freq;
    QLineEdit *add_to_log_le_frq;
    QCheckBox *cb_enable_ali;//2.75
    QComboBox *add_to_log_cb_prop;
    QComboBox *cb_sat_mod;
    QComboBox *cb_sat_nam;
    QLineEdit *le_rx_freq;
    QStringList s_id_sat_nam;
    QLabel *l_comment;
    bool f_off_auto_comm;//2.76.3
    QLineEdit *add_to_log_le;
    QPushButton *b_add_to_log_ok;
    QPushButton *b_add_to_log_cacel;
    void StartAddToDialog(QStringList in_lst);
    QString addtolog_comment;
    QString addtolog_freq;
    QString addtolog_prop;
    QString addtolog_txsn;
    QString addtolog_rxsn;
    QString addtolog_txex;
    QString addtolog_rxex;    
    QString addtolog_satn;
    QString addtolog_satm;
    QString addtolog_rxfr;    
    bool f_addtolog;
    QTimer *timer_refr_save;
    QLabel *l_txsn;
    QLabel *l_rxsn;
    QLabel *l_txex;
    QLabel *l_rxex;
    QLineEdit *add_to_log_le_txsn; 
    QLineEdit *add_to_log_le_rxsn;
    HvLeWithSpace *add_to_log_le_txex;
    HvLeWithSpace *add_to_log_le_rxex;    
    bool s_udportcp_broad_logged_adif;
    bool s_udp_broad_logged_qso;    
    bool f_cabr_ok;
    QString GetCabriloMoStr(QString);
    bool StartCabriloDialog();
    QDialog *ExportToCabrilloD;
    QDateTimeEdit *exp_dte_start;
    QDateTimeEdit *exp_dte_end;
    QComboBox *cb_cabrillo_cont;
    HvLeWithSpace *le_cabrillo_cont_id;
    QCheckBox *cb_cabrillo_all_qso;
    QComboBox *cb_cabrillo_band;
    QComboBox *cb_cabrillo_mod;
    QComboBox *cb_cabrillo_cop;
    QComboBox *cb_cabrillo_pwr;
    QComboBox *cb_cabrillo_as;
    QComboBox *cb_cabrillo_cst;
    QComboBox *cb_cabrillo_trm;
    QComboBox *cb_cabrillo_trmN;
    //QSpinBox *sb_cabrillo_trmN;
    QComboBox *cb_cabrillo_tim;
    QComboBox *cb_cabrillo_ovr;
    HvLeWithSpace *le_cabrillo_oper; 
    HvLeWithSpace *le_cabrillo_locat;
    QLineEdit *le_cabrillo_name;
    QLineEdit *le_cabrillo_email;
    QLineEdit *le_cabrillo_club;    
    QLineEdit *le_cabrillo_addr;    
    QLineEdit *le_cabrillo_addr2;
    QLineEdit *le_cabrillo_addr_city;
    QLineEdit *le_cabrillo_addr_spro;
    QLineEdit *le_cabrillo_addr_posc;
    QLineEdit *le_cabrillo_addr_cntr;       
    QPushButton *b_cabrillo_ok;
    QPushButton *b_cabrillo_cacel;
    bool f_isCheck[7];
    bool s_allowed_modes;
    QString s_mode;
    QString s_band;	
    HvProgressD *THvProgrD; 
    QMenuBar *Min_Menu;
    void SetManQsoText();
    void SetEditText();
	void CheckLogCount();
	void SaveAllExternal(bool);
    	
protected:
    pthread_t ths;
    static void *ThreadSaveLog(void *);
	//void mousePressEvent (QMouseEvent*);
	void keyPressEvent(QKeyEvent*);
	void closeEvent(QCloseEvent*)
    {
    	//qDebug()<<"Close-------------";
        End_Edit();
        End_Add_Man_Qso();
    };

};
#endif
