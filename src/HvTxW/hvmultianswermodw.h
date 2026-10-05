/* MSHV MultiAnswerModW
 * Copyright 2018 Hrisimir Hristov, LZ2HV
 * May be used under the terms of the GNU General Public License (GPL)
 */
#ifndef HVMULTIANSWERMOD_H
#define HVMULTIANSWERMOD_H

#include <QWidget>
#include <QTabWidget>
#include <QTreeView>
#include <QHeaderView>
#include <QStandardItemModel>
#include <QMenu>
#include <QMouseEvent>
#include "hvstditmmod.h"

class ListA : public QTreeView
{
    Q_OBJECT 
public:
    ListA(int ident,bool,QWidget *parent = 0);
    virtual ~ListA();

    HvStandardItemModel model;
    Qt::SortOrder s_order_1;
    Qt::SortOrder s_order_3;
    int GetRowCount()
    {
        return 	model.rowCount();
    };
    int GetColumnCount()
    {
        return 	model.columnCount();
    };
    /*QString GetColumnOrders()
    {
    	int i1=0;
    	int i3=0;
    	if (s_order_1==Qt::DescendingOrder) i1=1;
    	if (s_order_3==Qt::DescendingOrder) i3=1;
        return 	QString("%1").arg(i1)+"#"+QString("%1").arg(i3);
    };*/
    void SetColumnOrders(QString);
    int FindCallOrBaseCallRow(QString c);
    void RemoveRow(int);
    void RemoveRows(int,int);
    void SetFont(QFont f);
    void SetItem(int row,int col,QString str);
    void InsertItem_hv(QStringList);
    void Clear_List();

signals:
    void ListCountChange(int);
    void EmitClear_List();

public slots:
    //void SetItem(int row,int col,QString str);
    //void InsertItem_hv(QStringList);
    //void Clear_List();
    void DeleteSel();
    void SetAutoSort(int);
    //void SetItem_hv(QStringList,int);
    void HideSections();
    void SortChanged(int,Qt::SortOrder);

private:
	bool dsty;
    int s_auto_sort;
    void AutoSortDist(int);
    QHeaderView *THvHeader;
    //int ActiveIndex;
    //QColor ActiveRowText;
    //QColor ActiveRowBackg;
    QMenu *m_right_c;
    //Qt::SortOrder s_order_1;
    //Qt::SortOrder s_order_3;

protected:
    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent*);

};

#include <QSpinBox>
#include <QComboBox>
#include <QPushButton>
#include <QCheckBox>
#include "hvqthloc.h"
#include "hvinle.h"

class HvSpinLE : public QLineEdit
{
    Q_OBJECT
public:
    HvSpinLE(QWidget *parent = 0);
    virtual ~HvSpinLE();    
signals:
	void EmitDoubleClick();
//public slots:
//private:
protected:
	void mouseDoubleClickEvent(QMouseEvent *);
};
class HvSpinBoxMTP : public QSpinBox
{
    Q_OBJECT
public:
    HvSpinBoxMTP(QWidget *parent = 0);
    virtual ~HvSpinBoxMTP(); 
    bool GetMTP() 
    {
    	return f_sb_mtp;
   	};
//signals:
public slots:
	void SetDoubleClick();
private:
	bool f_sb_mtp;
//protected:
};
class HvSpinBoxSlots : public QSpinBox//2.76
{
    Q_OBJECT
public:
    HvSpinBoxSlots(QWidget *parent = 0);
    virtual ~HvSpinBoxSlots(); 
    void SetMsfS5(bool);
    int valueS();
    int maximumS();
//signals:
//public slots:
private:
	bool s_msf;
//protected:
};

#define MAXSL 6 //2.71 old=5
class MultiAnswerModW : public QTabWidget
{
    Q_OBJECT
public:
    MultiAnswerModW(bool,QWidget *parent = 0);
    virtual ~MultiAnswerModW();
    void SetMode(int);
    void SetMacros(QStringList list,int,int);//,int
    bool FormatTxIfSlash6CharCall(QString &s,int id,bool &f_pfx_sfx);
    QString FindBase6CharCallRemAllSlash(QString str);
    QString FindBaseFullCallRemAllSlash(QString str);//2.00
    void DetectTextInMsg(QString str, QString &hisCall_inmsg,QString &hisLoc_inmsg,QString &myCall_inmsg,
                         QString &rpt_inmsg,QString &contest_r_inmsg,QString &rr73_inmsg,int&,int&,//1.69=   QString hcle,
                         QString &sn_inmsg,QString &arrl_exch_imsg);//v2.01
    void DecListTextAll(QString tx_rpt,QString str,QString freq,bool,QString &hcap,QString &hloc);//1.73 hisCall_inmsg_for_ap
    void SetDistUnit(bool f_);
    void SetTextForAutoSeq(QStringList list_in);
    void GetCurrentMsg();
    void SetTxRxMsg(bool);
    void SetSettings(QString);
    QString GetSettings()
    {
        QString out;
        out.append(QString("%1").arg(SBqueueLimit->value()));
        out.append("#");
        out.append(QString("%1").arg(SBslots->value()));
        out.append("#");
        out.append(QString("%1").arg(SBmaxTP->value()));
        out.append("#");
        out.append(QString("%1").arg(Cb_dupes->currentIndex()));
        out.append("#");
        out.append(QString("%1").arg(cb_tx_sm->isChecked()));
        out.append("#");        
        out.append(QString("%1").arg(Cb_sort->currentIndex()));
        out.append("#");        
        out.append(QString("%1").arg(cb_tx_cq_on_free_slot->isChecked()));
        out.append("#");
        out.append(QString("%1").arg(cb_cont_ns->isChecked()));
        out.append("#");
        out.append(QString("%1").arg(SBmaxTP->GetMTP()));  
        out.append("#");
    	int i1=1;
    	int i3=1;
    	if (LsQueue->s_order_1==Qt::AscendingOrder) i1=0;
    	if (LsQueue->s_order_3==Qt::AscendingOrder) i3=0;
        out.append(QString("%1").arg(i1)+"#"+QString("%1").arg(i3)); 
        out.append("#");
        out.append(QString("%1").arg(cb_otp_mamd_key->isChecked()));//2.76sf        
        out.append("##"); //2.71 2.70 reserved            
        return out;
    }
    void SetFont(QFont f);
    bool isStandardCall(QString w);//,bool nobc = false
    void isStandardCalls(QString c1,QString c2,bool &fc1,bool &fc2,uint8_t &noQSO);
    void SetTextMark(bool b,bool m);
    void SetInfoDupeQso(bool f);
    bool DialogIsCallDupeInLog(QString hisCall_inmsg);
    void SetMAStd(bool);
    void SetStartFromTx2Tx1(bool f);
    void SetCntestOnlySdtc(bool f);
    void SetTxSnV2(int);
    QString TryFormatRPTtoRST(QString);
    void SetCfm73(bool f);
    void SetLastBcCallToLog(QString);
	void SetBand();
	int FindCallBackupDB(QString call);
	void SetBackupDB(QString call,QString txr,QString rxr,QString g,QString s,QString e);
	void GetBackupDB(int pos,QString &txr,QString &rxr,QString &g,QString &s,QString &e);
	void RefreshLRestrict_pub(bool);
	void SetAuto(bool);
	QString CalcDistance(QString,bool);//2.66
	void SetIsLastTxRptCq(bool);//2.66
	void SetUseASeqMaxDist(bool);//2.66
	void RestrictSeTX(bool);
	void setHfBand(bool f);
	QString DetectCQTypeFromMacros(QString);
	void SetMaManAdding(bool);
	void SetMsfS5SMsg(uint8_t);//2.76	

signals:
    void MamEmitMessage(QString,bool,bool,bool);
    void AddToLog(QStringList);
    void IsCallDupeInLog(QString,int,bool &);
    void EmitDoubleClick();
    void EmitMAMCalls(QStringList);
    void EmitHisCalls(QStringList);
    void EmitDxParm(QString dc,QString re,QString dxg);
    void EmitGBlockListExp(bool);
    void EmitStopAuto();
    void EmitQSOProgressMAM(int,bool);//2.51
    void EmitDoQRG(QString,QString);
    void EmitMAFirstTX(bool);
    void EmitSFMATxAll(QString);//2.76
    
public slots:
	void SetHisCallChanged(QString); 
	void SetQRG(QString);
	void SetReriodTime(float);
	
private slots:
    void LQueueCountChange(int);
    void LNowCountChange(int);
    void ClrQueue();
    void ClrNow();
    void CbCQChanged(QString);
    void SBslotsValueChanged(int);
    void LeFreeCQtextChanged(QString);
    void UseFreeCq();
    void CbTxCqOnFreeSlotChanged(bool);
    void cb_otp_mamd_key_toggled();//2.76sf  

private:
    //PomAll pomAll;
    uint8_t id_mshf;
    bool s_msf_ftmsg;//2.76
    uint8_t c_sf_rpt;
    uint8_t c_sf_r73;
	bool dsty;
	QString s_qrg;
	QString s_last_bccall_tolog_excp;
    bool f_cfm73;
    bool s_txt_mark_b;
    bool s_txt_mark_m;
	bool info_dupe_qso;
	bool f_multi_answer_mod_std;
	bool  s_start_qso_from_tx2;	
    int s_co_type;
    int s_co_id;
    bool f_con_only_sdtc;   
    void SetTxMsgEnd();
    QString gen_in_tx_time;
    bool f_tx_rx;
    int s_count_nw_i3b;//2.13
    int s_pos_nw_i3b;
    QString t_list_i3b[MAXSL+5]; //max slots 5

    QString current_msg;
    bool f_km_mi;
    bool allq65;
    int s_mode;
    QStringList list_macros;
    HvQthLoc THvQthLoc;
    ListA *LsQueue;
    ListA *LsNow;
    QSpinBox *SBqueueLimit;
    //QSpinBox *SBslots;
    HvSpinBoxSlots *SBslots;
    
    float period_time_sec;
    HvSpinBoxMTP *SBmaxTP;
    
    QComboBox *Cbcqtype;
    QComboBox *CbcqtypeSF;
    HvLeWithSpace *LeFreeCQ;
    //QCheckBox *cb_no_dupes;
    //QString filt_txt;
    QComboBox *Cb_dupes;
    QCheckBox *cb_tx_sm;//2.10 tx/notx special msg
    QCheckBox *cb_otp_mamd_key;//2.76sf
    //QCheckBox *cb_tx_sm_std;//2.71
    //QCheckBox *cb_a_sort;//2.13 sort
    QComboBox *Cb_sort;
    QCheckBox *cb_tx_cq_on_free_slot;//2.13
    QCheckBox *cb_cont_ns;
    QString format_rpt_ma(QString s);
    //QString CalcDistance(QString,bool);
    bool is_pfx(QString s);
    bool is_sfx(QString s);
	bool g_block_stop_auto;//2.52
	QString MakeSMsg(QString,QString,QString,QString);
    QString DecodeMacros(int row,QString id);

    bool f_auto_on;
    void gen_msg();
    void RefreshLists(int);//2.13
    QPushButton *pb_clr_queue;
    QPushButton *pb_clr_now;
    QString s_my_base_call;
    //bool s_my_call_is_std;
    bool my_call_have_slash;
    QPushButton *pb_use_free_cq;
    bool f_block_free_cq;
    void BlockFrreCq(bool);
    //bool IsMsgHaveValidCall(QString s);
    QString RemWSpacesInsideAndBegEnd(QString str);
    bool gg_frest;
    void ConfigRestrictW();
    void RefreshLRestrict();
    int s_txsn_v2;
    QString s_le_his_call;
    void RefreshBackupDbAll();//2.59
    void ClrNowUserDoubleClick();//2.59    
    bool f_aseqmaxdist;//2.66
    bool is_LastTxRptCq;//2.66
    int s_dist_points;//2.66
    bool s_man_adding;
    void SetSFMATxAll();//2.76

protected:

};
#endif
