/* MSHV
 * By Hrisimir Hristov - LZ2HV
 * May be used under the terms of the GNU General Public License (GPL)
 */
#ifndef DECODELIST_H
#define DECODELIST_H

#include <QHeaderView>
#include <QTreeView>
#include <QCoreApplication>
#include <QStandardItemModel>
#include <QMouseEvent>
#include <QPainter>
#include <QLineEdit>
#include <QClipboard>
#include <QApplication>
#include <QFontDatabase>
#include <QItemDelegate>
#include <QMenu>
//#include "../../../DirectX90c/include/dsound.h"// zaradi DWORD in pathred create
//#include <unistd.h> // zaradi usleep
//#include <QtGui>

//Delegate rools for QStandardItem in qtreeview like QLineEdit
class MyDelegate: public QItemDelegate
{
	Q_OBJECT
public:
    MyDelegate (QObject* parent = 0):QItemDelegate(parent)
    {}
    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const;
signals:
    void EmitSelectedText(QString,int,int);
private slots:
    void SelectionChanged();
    //void closeEvent(QCloseEvent*);
    //void commitAndCloseEditor();
    //bool eventFilter(QObject *object, QEvent *event);   
};

/*class HvHeader : public QHeaderView
{
    Q_OBJECT
public:
    HvHeader(Qt::Orientation orientation, QWidget *parent = 0);
    virtual ~HvHeader();

};*/
#include "hvtooltip.h"
#include "hvcty.h"
#include "../HvTxW/hvqthloc.h"

class DecodeList : public QTreeView
{
    Q_OBJECT
public:
    DecodeList(int ident,bool f,QWidget *parent = 0);
    virtual ~DecodeList();

    //void SetFocus();
    QStandardItemModel model;
    //void Clear_List();
    //void SetItem_hv(QStringList,int,int,int);
    //void SetEditRow(int);
    void SetMode(int,bool,bool,bool,bool,bool); 
    /*int GetListCount()
    {
        return 	model.rowCount();
    };*/
    void StaticIgnoreResizeEvent(bool);//1.30 stop scroling
    QStringList GetCountries();
    void SetBackColorTxQsy(bool f);//2.46
    void HideShowWfTxRefreshList();//2.46
    void SetActivityId(QString);
    //void SetSHAllColl(bool,bool,bool);

signals:
    //void SendRightClick();
    void ListSelectedTextAll(QString,QString,QString,QString,QString);
    void ListSelectedRpt(QString);
    void EmitRxAllTxt(QString);
    void EmitLstNexColor(bool);
    void EmitRxStationInfo(QStringList,int,bool,uint8_t);
    void EmitRxTextForAutoSeq(QStringList);
    void EmitFreqDecListClick(double);
    void EmitUdpDecClr();
    void EmitDetectTextInMsg(QString,QString &);
    void EmitHeaderDoubleClicked();//2.44

public slots:
    void InsertItem_hv(QStringList,bool,bool);
    void Clear_List();
    void SetBackColor(bool f);
    void SetDListMarkText(QStringList,int,int,int,int,int);
    void SetFontList(QFont);
    void SetFontHeader(QFont f);
    void SetTextMarkColors(QColor*,int,bool,bool);
    void SetStaticGBlockListExp(bool);
    void SetNewDecClrMsgListFlag(bool);
    void SetUdpCmdDl(QStringList l); 
    void SetStaticClickOnCallShowCty(bool);
    void SetTimeElapsed(float);//2.33
    void SetFilter(QStringList,bool*,QStringList,QStringList,QStringList,QStringList,QStringList,QStringList);//2.43   
    void SetDistUnit(bool); 
    void SetHisCalls(QStringList);
    void SetOtpVerif(QString,uint8_t);//2.76sf
    void SetShowOtpRxMsg(bool f);//2.76sf
    
private slots:
    void RefreshListTimer();
    void ItemSelectedText(QString,int,int);
    void ac_spot();
    //void SetHeaderSingleClicked();
    void SetHeaderDoubleClicked();//2.44

private:
    bool f_otp_show_msg;
	bool dsty;
	bool is_filters_active;	
	bool f_hide_c[10];
	bool hide_filter_list;
	QStringList list_filter;
    bool is_filter_dec;
	bool f_bgcolor_clr_lst;
	bool show_filter_list;	
	bool show_customf_list;
	QStringList list_customf;
	bool show_cufspec_list;
	QStringList list_cufspec;
	bool show_cufend_list;
	QStringList list_cufend;
	bool show_cnyf_list;
	QStringList list_cnyf;
	bool show_pfxf_list;
	QStringList list_pfxf;	
	bool hide_cnyf_list;
	QStringList list_hidcnyf;
	QStringList list_b4qso;
	//bool f_p_hide_b4qso;
	
	void SetFilterParms(QStringList l,QStringList &lc,bool &f);
	void RefreshFiltHeadColor(bool,bool,bool,bool,bool,bool,bool,bool,bool);
	bool AllisDigitOrAllisLetter(QString);
	HvQthLoc THvQthLoc;
	QString FindHisCall(QString);
	bool ShowDecode(QString);
	bool ShowCDecode(QString);
	bool ShowCSDecode(QString);
	bool ShowCENDDecode(QString);
	bool HideB4Qso(QString);
	QString FindCountry(QString,QString,QString);	
	QString CalcDistance(QString,QString);
	bool isNotTxnQsyCq(QString);
	
	//bool f_click_on_call_show_cty;
	HvToolTip *THvToolTip;
	HvCty *THvCty;
    int s_list_ident;
    bool s_fast_dec_clr_msg_list;
	QString last_pt_dec_clr_msg_list;
    bool f_new_dec_clr_msg_list;
    void RefreshFastDcml();
    //bool g_block_list_exp;
    QTimer *list_rfresh_timer;    
    QMenu *m_spot; 
    //HvHeader *THvHeader;
    QHeaderView *THvHeader;
    //int ActiveIndex;
    //QColor ActiveRowText;
    //QColor ActiveRowBackg;
    bool allq65;
    bool coded_mods;
    int s_mode;
    QClipboard *clipboard;
    bool f_row_color;
    int msg_column;
    QStringList s_mark_txt;
    int s_mark_b4q_pos; 
    int s_mark_loc_pos;
    int s_mark_myc_pos;
    int s_mark_hisc_pos;
    int s_mark_r12_pos;
    bool s_flag_two;
    bool f_mark_tx;
    bool f_mark_qsy;
    bool f_mark_txqsy;//2.46
    uint8_t id_mark_otp_verif;//2.76
    QColor c_mark_txt[11];
    int def_section_sizes[22];
    void SaveAndCorrSectionSize();//2.10 max 20 int def_section_sizes[20]; from model.columnCount()
    int count_dec;
	void CountMsgInPeriod(bool);
	bool s_show_timec;	
	bool s_show_distc;
	bool s_show_freqc;
	bool s_show_counc;
	bool f_km_mi;
	QString s_my_loc;
	bool is_only_cqrr73_active;
	QStringList l_hiscals;
	bool isFromHiscals(QString);
	void CorrListSize();
	//uint8_t isVerfOrOtpmsg(QString,bool);
 
protected:
    void mousePressEvent (QMouseEvent*);
    //bool isSingeWord(QString s,int b,int e) const;
    //bool DrawCode(QString,QString,int,int) const;
    void drawRow(QPainter *painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const;
	//void mouseMoveEvent (QMouseEvent*);
    //void drawMarks(QString find,QString alltxt,QPainter *painter, QRect rect_row);
    //void mouseDoubleClickEvent(QMouseEvent * event);
	//void mouseReleaseEvent(QMouseEvent * event);
    
    void paintEvent(QPaintEvent *);
    void keyPressEvent(QKeyEvent * event);
    void resizeEvent(QResizeEvent * event);
    //bool f_resize_event;
	//bool event ( QEvent * e );
    //QFont font_items;
};
#endif
