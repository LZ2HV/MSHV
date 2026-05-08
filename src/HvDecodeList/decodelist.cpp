/* MSHV DecodeList
 * Copyright 2015 Hrisimir Hristov, LZ2HV
 * May be used under the terms of the GNU General Public License (GPL)
 */
#include "decodelist.h"
#include "../config.h"
#include "../config_str_all.h"
//#include <QtGui>

static QLineEdit* st_lineEdit_hv;
static int ss_column_;
static int ss_row_;
QWidget* MyDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QWidget *w = QItemDelegate::createEditor(parent,option,index);
    st_lineEdit_hv = qobject_cast<QLineEdit*>(w);
    //2.72
    st_lineEdit_hv->setContentsMargins(3,0,0,0);
    QVariant v = index.data(Qt::BackgroundColorRole);
    QColor c = v.value<QColor>();
    st_lineEdit_hv->setStyleSheet("QLineEdit{background-color:"+c.name()+";}");
    //2.72 end
    st_lineEdit_hv->setReadOnly(true);
    st_lineEdit_hv->setMouseTracking(true);
    st_lineEdit_hv->setCursor(Qt::IBeamCursor);
    ss_column_ = index.column();
    ss_row_ = index.row();
    //if (st_lineEdit_hv && index.data(Qt::TextAlignmentRole) == Qt::AlignLeft)
    //st_lineEdit_hv->setAlignment(Qt::AlignLeft);
    if (st_lineEdit_hv && index.data(Qt::TextAlignmentRole) == Qt::AlignCenter) st_lineEdit_hv->setAlignment(Qt::AlignCenter);
    //else if (st_lineEdit_hv)
    //st_lineEdit_hv->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    connect(st_lineEdit_hv,SIGNAL(selectionChanged()),this,SLOT(SelectionChanged()));    	
    return w;
}
void MyDelegate::SelectionChanged()
{
    QString str = st_lineEdit_hv->selectedText();
    if (!str.isEmpty()) emit EmitSelectedText(st_lineEdit_hv->selectedText(),ss_column_,ss_row_);
}
/*
bool MyDelegate::eventFilter(QObject *object, QEvent *event)
{
    QWidget *editor = qobject_cast<QWidget*>(object);
    if (!editor)
        return false;
    if (event->type() == QEvent::FocusOut || (event->type() == QEvent::Hide && editor->isWindow()))
    {
        //the Hide event will take care of he editors that are in fact complete dialogs
        if (!editor->isActiveWindow() || (QApplication::focusWidget() != editor))
        {
            QWidget *w = QApplication::focusWidget();
            while (w)
            { // don't worry about focus changes internally in the editor
                if (w == editor)
                    return false;
                w = w->parentWidget();
            }
            //#ifndef QT_NO_DRAGANDDROP
                        // The window may lose focus during an drag operation.
                        // i.e when dragging involves the taskbar on Windows.
            //			if (QDragManager::self() && QDragManager::self()->object != 0)
            //			    return false;
            //#endif
            qDebug()<<"DleteEditor"<<event->type();
            emit commitData(editor);//?? <-crash in hvtxw.cpp HvTxW::DialogIsCallDupeInLog
            emit closeEditor(editor, NoHint);//?? <-crash in hvtxw.cpp HvTxW::DialogIsCallDupeInLog
            return false;
        }
    }
    return QItemDelegate::eventFilter(object,event);
}
void MyDelegate::closeEvent(QCloseEvent*e)
{
	 qDebug()<<"DeleteEditor";
}
void MyDelegate::commitAndCloseEditor()
{
     QWidget *w = qobject_cast<QWidget *>(sender());
     //emit commitData(w);
     //emit closeEditor(w);
     st_lineEdit_hv = qobject_cast<QLineEdit*>(w);
     connect(st_lineEdit_hv, SIGNAL(selectionChanged()),this, SLOT(SelectionChanged()));
     qDebug()<<"DeleteEditor";
}
*/
/*HvHeader::HvHeader( Qt::Orientation orientation, QWidget *parent )
        : QHeaderView(orientation, parent)
{
    QString App_Path = (QCoreApplication::applicationDirPath());
    //int nFont = QFontDatabase::addApplicationFont (App_Path+"/settings/font/arial.ttf");//registriram fonta//  xsuni
       //int nFont = QFontDatabase::addApplicationFont (App_Path+"/settings/font/times.ttf");
       int nFont = QFontDatabase::addApplicationFont (App_Path+"/settings/font/xsuni.ttf");
       QStringList fontList = QFontDatabase::applicationFontFamilies(nFont);
       QString szFont = fontList[0];
       QFont font_k(szFont, 10);
       setFont(font_k);
    //setResizeMode(QHeaderView::Interactive);
    //setSectionsMovable(true);
    //moveSection(int from, int to)
    //sectionPosition();
}
HvHeader::~HvHeader()
{}*/
//#include <QGraphicsDropShadowEffect>
#define COL_CY 104//to +175%
DecodeList::DecodeList(int ident,bool f,QWidget *parent)
        : QTreeView(parent)
{
    /*QGraphicsDropShadowEffect* effect = new QGraphicsDropShadowEffect();
    effect->setBlurRadius(0); effect->setColor(QColor(255, 0, 0));
    effect->setOffset(1,1); 
    setGraphicsEffect(effect);*/
    dsty = f;
    f_otp_show_msg = false;
    is_filters_active = false;
    f_bgcolor_clr_lst = true;//2.43
    show_filter_list = false;
    list_filter.clear();
    hide_filter_list = false;
    f_hide_c[0]=false;
    f_hide_c[1]=false;
    f_hide_c[2]=false;
    f_hide_c[3]=false;
    f_hide_c[4]=false;
    f_hide_c[5]=false;
    f_hide_c[6]=false;
    f_hide_c[7]=false;//usefudpdectxt
    f_hide_c[8]=false;//b4qso
    f_hide_c[9]=false;//filtered_answer
    show_customf_list = false;
    list_customf.clear();
    show_cufspec_list = false;
    list_cufspec.clear();
    show_cufend_list = false;
    list_cufend.clear();
    show_cnyf_list = false;
    list_cnyf.clear();
    show_pfxf_list = false;
    list_pfxf.clear();
    hide_cnyf_list = false;
    list_hidcnyf.clear();
    list_b4qso.clear();
    //f_show_timec = true;
    s_show_timec = true;
    //f_show_distc = false;
    s_show_distc = false;
    //f_show_freqc = true;
    s_show_freqc = true;
    s_show_counc = false;
    f_km_mi = false;//km
    s_my_loc = "";
    is_only_cqrr73_active = false;
    l_hiscals.clear();

    count_dec = 0;
    is_filter_dec = false;
    THvCty = new HvCty(ident);

    THvToolTip = new HvToolTip(this);
    //f_click_on_call_show_cty = true;//2.51 fefault

    //g_block_list_exp = false;
    c_mark_txt[0] = QColor(100,254,100);
    c_mark_txt[1] = QColor(254,110,110);//QSO B4
    c_mark_txt[2] = QColor(130,255,255);//loc
    c_mark_txt[3] = QColor(255,210,210);//tx in list2
    c_mark_txt[4] = QColor(255,255,150);//qsy in list2
    c_mark_txt[5] = QColor(255,180,100);//2.56 my call
    c_mark_txt[6] = QColor(100,210,255);//2.63 his call
    f_mark_tx = true;
    f_mark_qsy = true;
    f_mark_txqsy = false;//tx=0 qsy=1;
    id_mark_otp_verif = 0;
    s_list_ident = ident;
    f_new_dec_clr_msg_list = false;
    last_pt_dec_clr_msg_list = "NONE";
    s_fast_dec_clr_msg_list = false;
    m_spot = new QMenu(this);
    QAction *ac_spot = new QAction(QPixmap(":pic/spot_dx.png"),tr("Spot Receiving Text"), this);//"Spot DX Cluster" Spot Receiving Text
    m_spot->addAction(ac_spot);
    connect(ac_spot, SIGNAL(triggered()), this, SLOT(ac_spot()));

    //f_resize_event = false;
    s_mark_txt.clear();
    s_mark_r12_pos = 6;
    s_mark_hisc_pos = 8;
    s_mark_b4q_pos = 10;
    s_mark_loc_pos = 20;
    s_mark_myc_pos = 30;//2.65

    setMinimumSize(100, 127);//127 be6e 530 na 1 display

    setRootIsDecorated(false);

    //THvHeader = new HvHeader(Qt::Horizontal);
    THvHeader = new QHeaderView(Qt::Horizontal);

    setHeader(THvHeader);

    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    setAllColumnsShowFocus(true);  // za da o4ertae delia row
    //setSelectionBehavior(QAbstractItemView::SelectRows);

    setSelectionBehavior(QAbstractItemView::SelectRows);
    //setSelectionMode(QAbstractItemView::ExtendedSelection);

    //setItemDelegate(new MyDelegate(this));
    MyDelegate *TMyDelegate = new MyDelegate(this);
    setItemDelegate(TMyDelegate);
    connect(TMyDelegate, SIGNAL(EmitSelectedText(QString,int,int)), this, SLOT(ItemSelectedText(QString,int,int)));

    setModel(&model);

    //setSelectionBehavior(QAbstractItemView::SelectItems);
    //setSelectionMode(QAbstractItemView::ExtendedSelection);//QAbstractItemView::SingleSelection
    //setAllColumnsShowFocus(true);
    //setSelectionMode(QAbstractItemView::MultiSelection);
    //setSelectionBehavior(QAbstractItemView::SelectRows);

    setDragDropMode(QAbstractItemView::NoDragDrop);//QAbstractItemView::InternalMove
    setDragEnabled (false);
    // setAcceptDrops(true);
    //setVerticalScrollMode(QAbstractItemView::ScrollPerItem);//1.30 for 2D/1D no e poblem s jt6m mnogo text da vidi6 kraia
    setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);//1.30 vazno MSHV v0.96 na mnogo text pri JT6M da vidi6 kraia
    setAutoScroll(false); // s tova tarsi samo v 1 colona i za scroll pri drag
    /*   m_file = new QMenu(this);
       QAction *file_info = new QAction("File Info", this);
       m_file->addAction(file_info);
       connect(file_info, SIGNAL(triggered()), this, SLOT(File_Info()));
       m_file->addSeparator();
       QAction *clr_sel = new QAction("Clear Selected", this);
       m_file->addAction(clr_sel);
       connect(clr_sel, SIGNAL(triggered()), this, SLOT(DellBtSelItems()));
       TFileEditInfoHV = new FileEditInfoHV(this);
       connect(TFileEditInfoHV, SIGNAL(TagChange(QString)), this, SLOT(TagChange_lis(QString)));*/
    //this->setFixedHeight(150);
    //setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
    QStringList list_A;
    //list_A <<"Time"<<"T"<<"Width"<<"dB"<<"Rpt"<<"DF"<<"Message"<<"Freq";
    list_A<<tr("Time")<<tr("Sync")<<"dB"<<"DT"<<"DF"<<"F1"<<tr("Message")<<tr("Country")<<tr("Dist")<<"Ml"<<"Nw"<<"Na"<<"Ta";//setup max count hv->11
    model.setHorizontalHeaderLabels(list_A);
    //THvHeader->setResizeMode(QHeaderView::Fixed);
    //THvHeader->setResizeMode(2, QHeaderView::Fixed);
    // Default ISCAT-A ISCAT-B max columns
    THvHeader->resizeSection(0, 60);
    THvHeader->resizeSection(1, 45);
    THvHeader->resizeSection(2, 40);
    THvHeader->resizeSection(3, 50);
    THvHeader->resizeSection(4, 50);
    THvHeader->resizeSection(5, 50);
    THvHeader->resizeSection(6, 150);
    THvHeader->resizeSection(7, COL_CY);//2.73 Country
    THvHeader->resizeSection(8, 55);
    THvHeader->resizeSection(9, 45);// hv v095 * + ML
    THvHeader->resizeSection(10, 35);
    THvHeader->resizeSection(11, 35);
    THvHeader->resizeSection(12, 40);

    //2.10 max 20 int def_section_sizes[20]; from model.columnCount()
    for (int i = 0; i < model.columnCount(); ++i) def_section_sizes[i]=THvHeader->sectionSize(i);

    connect(THvHeader, SIGNAL(sectionDoubleClicked(int)), this, SLOT(SetHeaderDoubleClicked()));
    //connect(THvHeader, SIGNAL(sectionClicked(int)), this, SLOT(SetHeaderSingleClicked()));
    //THvHeader->setSectionsClickable(true);

    allq65 = false;
    coded_mods = false;
    s_mode = 222;//2.10 202 <-for start refresh    HV important set to default mode fsk441
    msg_column = 6;//HV important set to default mode fsk441
    s_flag_two = false;

    SetMode(s_mode,s_flag_two,s_show_timec,s_show_distc,s_show_freqc,s_show_counc);//HV important set to default mode fsk441 //da skrie sekciite;

    //2.31 false for "\n" jt6m jtms
    setUniformRowHeights(true);//2.30 fast wiev

    //QFont font("arial", 10);  // times new roman // slaga se tuk zaradi resize coll
    //setFont(font);
    //ActiveIndex = -1;
    //ActiveRowText = QColor(255,0,0);
    //ActiveRowBackg = QColor(0,255,0);
    // DrawDropLinePosY = -1;

    //this->setStyleSheet("QTreeView { border-style: outset; border-width: 4px; border-color: black; }");
    //this->setStyleSheet("QTreeView {border-style: outset; border-width: 2px; }");

    clipboard = QApplication::clipboard();
    //QString originalText = clipboard->text();

    //clipboard->setText("newText");
    //createEditor(parent);
    f_row_color = false;

    //qlsi_st_c = 0;
    list_rfresh_timer = new QTimer();
    connect(list_rfresh_timer, SIGNAL(timeout()), this, SLOT(RefreshListTimer()));

}
DecodeList::~DecodeList()
{
    //qDebug()<<"close";
}
QStringList DecodeList::GetCountries()
{
    return THvCty->GetCountries();
}
/*void DecodeList::SetHeaderSingleClicked()
{
    //if (s_mode==11 || s_mode==13|| s_mode==18) //emit EmitHeaderDoubleClicked();
    qDebug()<<"Click";
}*/
void DecodeList::SetHeaderDoubleClicked()//2.44
{
    if (s_mode==11 || s_mode==13 || s_mode==18) emit EmitHeaderDoubleClicked();
}
void DecodeList::SaveAndCorrSectionSize()
{
    QFont fnn = font();
    fnn.setPointSize(10);
    QFontMetrics fm(fnn);
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)//2.56
    int fcor = (fm.horizontalAdvance("8")+2)-10;//2pix<-Bearing 8 K W
#else
    int fcor = (fm.width("8")+2)-10;//2pix<-Bearing 8 K W
#endif
    int fps = font().pointSize()+fcor;
    for (int i = 0; i < model.columnCount(); ++i)
    {
        int inc_sz = (def_section_sizes[i]*fps)/10;// in %
        THvHeader->resizeSection(i,inc_sz);
    }
    //qDebug()<<"2 IncSize="<<fps<<fcor; //<<fm.leftBearing('8')<<fm.rightBearing('8');
}
void DecodeList::SetFontList(QFont f)
{
    setFont(f);
    SaveAndCorrSectionSize(); //qDebug()<<"SetFontList=";
}
void DecodeList::SetFontHeader(QFont f)
{
    THvHeader->setFont(f);
}
/*
void DecodeList::ScrollToBottom()//1.30 exsternal if switch dispalys 2D/1D
{
    scrollToBottom();
}
int DecodeList::GetCurrentScrollRow() //1.30 for 2D/1D triabva ScrollPerItem no e poblem s jt6m mnogo text da vidi6 kraia
{
    int crow = 0;
    //viewport()->update();

    if (model.rowCount()>0)
    {
        QModelIndex  index = indexAt(viewport()->rect().topLeft());
        crow = index.row();
    }

    return crow;
}
*/
//int alignment=0;
/*
int DecodeList::FindSelPosForTP(QString str,QString all_txt)//1.56=
{
    //1.56= find msg_pos 0,1,2 call
    QStringList list_all_txt = all_txt.split(" ");
    int msg_pos = 0;
    for (msg_pos = 0; msg_pos < list_all_txt.count(); msg_pos++)
    {
        QString tmp_s = list_all_txt.at(msg_pos);
        if (tmp_s.contains(str))
            break;
    }
    return msg_pos;
}
*/
void DecodeList::SetUdpCmdDl(QStringList l)
{
    //"CLR", "1" 0=1, 1=2, 2=3->all
    //"062100", "18", "0.0", "FT8", "LZ2HV SP9HWY JO90"   s_list_ident
    if (l.at(0) == "CLR")
    {
        if (s_list_ident == l.at(1).toInt()+1) Clear_List();
        else if (l.at(1).toInt()+1==3) Clear_List();
    }
    else if (s_list_ident == 1)
    {
        if (ModeStr(s_mode)!= l.at(3)) return;

        int rc = model.rowCount()-1;
        int msg_col = 0;
        int dt_col  = 0;
        int snr_col = 0;

        if (s_mode==11 || s_mode==13 || s_mode==18 || allq65)//ft8 ft4	msg=4
        {
            msg_col = 4;
            dt_col  = 2;
            snr_col = 1;
        }
        else if (s_mode==0 || s_mode==12)//msk144 msk144ms  msg=6
        {
            msg_col = 6;
            dt_col  = 1;
            snr_col = 3;
        }

        for (int i = rc; i >= 0; --i)
        {
            QString str = model.item(i, msg_col)->text();         //msg
            if (l.at(4) == str)
            {
                if (l.at(2) == model.item(i, dt_col)->text())     //dt
                {
                    if (l.at(1) == model.item(i, snr_col)->text())//snr
                    {
                        if (l.at(0) == model.item(i, 0)->text())  //time
                        {
                            ItemSelectedText(str,msg_col,i);
                            break;
                        }
                    }
                }
            }
        }
    } //qDebug()<<"WWWWW="<<l<<in_column;
}
void DecodeList::ItemSelectedText(QString str,int in_column,int in_row)
{	//qDebug()<<str<<in_column<<in_row;
    if (s_mode==0 || s_mode==12)//msk144 msk144ms
    {
        if (in_column==3 || in_column==4)
        {
            if (s_mode==12)
            {
                if (!model.item(in_row, 4)->text().isEmpty())// vavno moze i da e prazno
                    emit ListSelectedRpt(model.item(in_row, 4)->text());
            }
            else emit ListSelectedRpt(model.item(in_row, 3)->text());
        }
        if (in_column==6)
        {
            QString tx_rpt = "+00";
            if (s_mode==12)
            {
                if (!model.item(in_row, 4)->text().isEmpty())// vavno moze i da e prazno
                    tx_rpt = model.item(in_row, 4)->text();
            }
            else tx_rpt = model.item(in_row, 3)->text();

            //2.02 need first for Queued
            emit ListSelectedTextAll(model.item(in_row, 6)->text(),str,model.item(in_row, 0)->text(),
                                     tx_rpt,model.item(in_row, 12)->text());//2.73 Dist Country

            if (s_mode==12)
            {
                if (!model.item(in_row, 4)->text().isEmpty())// vavno moze i da e prazno
                    emit ListSelectedRpt(tx_rpt);
            }
            else emit ListSelectedRpt(tx_rpt);
        }
    }
    if (s_mode==1 || s_mode==2 || s_mode==3 )//jtms fsk441 fsk315
    {
        if (str.contains(' '))//1.56= for ListSelectedRpt
            return;
        if (in_column==4)
            emit ListSelectedRpt(str);
        if (in_column==6)
        {
            emit ListSelectedRpt(model.item(in_row, 4)->text());
            emit ListSelectedTextAll(model.item(in_row, 6)->text(),str,model.item(in_row, 0)->text(),
                                     model.item(in_row, 4)->text(),model.item(in_row, 7)->text());
            //ListSelectedRpt(model.item(in_row, 4)->text());
        }
    }
    if (s_mode==4 || s_mode==5)//iskat-a iskat-b
    {
        if (str.contains(' '))//1.56= for ListSelectedRpt
            return;
        if (in_column==2)
            emit ListSelectedRpt(str);
        if (in_column==6)
        {
            emit ListSelectedRpt(model.item(in_row, 2)->text());
            emit ListSelectedTextAll(model.item(in_row, 6)->text(),str,model.item(in_row, 0)->text(),
                                     model.item(in_row, 2)->text(),"?");
            //ListSelectedRpt(model.item(in_row, 2)->text());
        }
    }
    if (s_mode==6)//JT6M
    {
        //if (in_column==2) // no raport 1.30 hv
        //ListSelectedRpt(str);
        if (str.contains(' '))//1.56= for ListSelectedRpt
            return;
        if (in_column==5)
        {
            emit ListSelectedTextAll(model.item(in_row, 5)->text(),str,model.item(in_row, 0)->text(),"?","?");
        }
    }
    if (s_mode==7 || s_mode==8 || s_mode==9)//jt65abc
    {
        //if (model.item(in_row, 2)->text()=="TX")//1.60=moved to insert my tx
        //return;
        if (in_column==2)
            emit ListSelectedRpt(str);
        if (in_column==9)
            emit EmitFreqDecListClick((double)str.toInt());
        if (in_column==6)
        {
            double his_freq = (double)model.item(in_row, 9)->text().toInt();
            emit EmitFreqDecListClick(his_freq);
            emit ListSelectedRpt(model.item(in_row, 2)->text());
            emit ListSelectedTextAll(model.item(in_row, 6)->text(),str,model.item(in_row, 0)->text(),
                                     model.item(in_row, 2)->text(),model.item(in_row, 9)->text());
        }
    }
    if (s_mode==10)//pi4
    {
        if (str.contains(' '))//1.56= for ListSelectedRpt
            return;
        if (in_column==2)
            emit ListSelectedRpt(str);
        if (in_column==9)
            emit EmitFreqDecListClick((double)str.toInt());
        if (in_column==6)
        {
            double his_freq = (double)model.item(in_row, 9)->text().toInt();
            emit EmitFreqDecListClick(his_freq);
            emit ListSelectedRpt(model.item(in_row, 2)->text());
            emit ListSelectedTextAll(model.item(in_row, 6)->text(),str,model.item(in_row, 0)->text(),
                                     model.item(in_row, 2)->text(),model.item(in_row, 9)->text());
        }
    }
    if (s_mode==11 || s_mode==13 || s_mode==18 || allq65)//ft8 ft4
    {
        //if (model.item(in_row, 1)->text()=="TX")//1.60=moved to insert my tx
        //return;
        //QTime ttt;

        if (in_column==1)
            emit ListSelectedRpt(str);
        if (in_column==9)
            emit EmitFreqDecListClick((double)str.toInt());//2.73 Dist Country
        if (in_column==4)
        {
            //2.02 need first for Queued
            emit ListSelectedTextAll(model.item(in_row, 4)->text(),str,model.item(in_row, 0)->text(),
                                     model.item(in_row, 1)->text(),model.item(in_row, 9)->text());//2.73 Dist Country
            double his_freq = (double)model.item(in_row, 9)->text().toInt();//2.73 Dist Country
            emit EmitFreqDecListClick(his_freq);
            emit ListSelectedRpt(model.item(in_row, 1)->text());
        }
        //qDebug()<<"Time="<<ttt.elapsed();
    }

    if (s_mode == 0  && in_column==3)// msk144
        st_lineEdit_hv->setCursor(Qt::ArrowCursor);
    if (s_mode == 12  && in_column==4)// msk144ms
        st_lineEdit_hv->setCursor(Qt::ArrowCursor);

    if ((s_mode == 4 || s_mode == 5) && in_column==2)//iskat-a iskat-b jt65abc pi4 and rpt column
        st_lineEdit_hv->setCursor(Qt::ArrowCursor);

    if ((s_mode == 7 || s_mode == 8 || s_mode == 9  || s_mode==10) && (in_column==2 || in_column==9))//jt65 pi4
        st_lineEdit_hv->setCursor(Qt::ArrowCursor);

    if ((s_mode == 1 || s_mode == 2 || s_mode == 3) && in_column==4 ) // jtms fsk441 fsk315  and rpt column
        st_lineEdit_hv->setCursor(Qt::ArrowCursor);

    if ((s_mode == 11 || s_mode==13 || s_mode==18 || allq65) && (in_column==1 || in_column==8)) // ft8 ft4  and rpt and freq column
        st_lineEdit_hv->setCursor(Qt::ArrowCursor);

    // JT6M no rpt column for selection
    //this->setFocus();
    //QString strr = st_lineEdit_hv->text();
    //st_lineEdit_hv->setText(strr);
    //st_lineEdit_hv->clearFocus();
}
static float s_ts_ = -1.0;
void DecodeList::CountMsgInPeriod(bool f)
{
    if (s_list_ident!=1) return;
    //only for MSK, JT65, FT8, FT4
    //if (s_mode!=11 && s_mode!=13 && s_mode!=0 && s_mode!=12 && s_mode!=7 && s_mode!=8 && s_mode!=9) return;
    if (s_mode==11 || s_mode==13 || s_mode==18 || s_mode==0 || s_mode==12 || s_mode==7 || s_mode==8 || s_mode==9 || allq65)// || allq65
    {
        if (f) count_dec++;
        else
        {
            count_dec = 0;
            is_filter_dec = false;
            s_ts_ = -1.0;//2.46 protect all modes from /0.0s
        }
        model.setHeaderData(msg_column,Qt::Horizontal,tr("Message")+" "+QString("%1").arg(count_dec));  //  +""
    }
}
static bool g_block_list_exp = false;
void DecodeList::SetStaticGBlockListExp(bool f)
{
    g_block_list_exp = f; //qDebug()<<"List="<<s_list_ident<<"Block="<<f;
}
void DecodeList::SetTimeElapsed(float ts)
{
    if (s_list_ident!=1) return;
    if (g_block_list_exp) return;//2.66
    s_ts_ = ts;
    if (s_ts_ == -1.0)//2.46 protect all modes from /0.0s
        model.setHeaderData(msg_column,Qt::Horizontal,tr("Message")+" "+QString("%1").arg(count_dec));
    else
        model.setHeaderData(msg_column,Qt::Horizontal,tr("Message")+" "+QString("%1").arg(count_dec)+" / "+QString("%1").arg(ts,0,'f',1)+"s");
}
//#include <QToolTip>
void DecodeList::RefreshFiltHeadColor(bool f1,bool f2,bool f3,bool f4,bool f5,bool f6,bool f7,bool f8,bool f9)
{
    if ((s_mode==11 || s_mode==13 || s_mode==18) && (f1 || f2 || f3 || f4 || f5 || f6 || f7 || f8 || f9))//ft8 ft4
    {
        is_filters_active = true;
        if (dsty) header()->setStyleSheet("QHeaderView::section{background-color:rgb(155,90,90);}");
        else header()->setStyleSheet("QHeaderView::section{background-color:rgb(255,190,190);}");
        if (model.rowCount()<1) //2.45
        {
            f_bgcolor_clr_lst = false;// exception for I wanna, I no wanna filter, for Idiots
            emit EmitLstNexColor(true);
        }
        else if (!is_filter_dec) //2.45 && model.rowCount()>0
        {
            if (f_row_color) emit EmitLstNexColor(true);
            else emit EmitLstNexColor(false);
        }
        if (f1 && !f2 && !f3 && !f4 && !f5 && !f6 && !f7 && !f8 && !f9) is_only_cqrr73_active = true;
        else is_only_cqrr73_active = false;
        /*QPoint post = mapToGlobal(QPoint(0,0));
        post+=QPoint(width()/2,-10);
        QToolTip::showText(post,"Decode List Filter Active",this,rect(),1500);*/
    }
    else
    {
        is_filters_active = false;
        header()->setStyleSheet("QHeaderView::section{background-color:palette(Button);}");
        if (s_mode==11 || s_mode==13 || s_mode==18) //2.45
        {
            if (model.rowCount()<1) emit EmitLstNexColor(false);
            else if (!is_filter_dec)
            {
                if (f_row_color) emit EmitLstNexColor(false);
                else emit EmitLstNexColor(true);
            }
        }
        is_only_cqrr73_active = false;
        /*QPoint post = mapToGlobal(QPoint(0,0));
        post+=QPoint(width()/2,-10);
        QToolTip::showText(post,"Decode List Filter Not Active",this,rect(),1500);*/
    }//qDebug()<<"List"<<s_list_ident<<"-> is_filters_active="<<is_filters_active<<"OnlyCQ="<<is_only_cqrr73_active;
}
void DecodeList::CorrListSize()
{
    int wid = 0;
    for (int i = 0; i < model.columnCount(); ++i)//List all column count
    {
        if (THvHeader->isSectionHidden(i)) continue;
        wid+=def_section_sizes[i]; //qDebug()<<i<<wid;
    } //qDebug()<<"-------------------";
    if (wid*2>800) setMinimumWidth(wid);//setMinimumSize(wid,127);//800 = app minimum size
    else setMinimumWidth(100);//setMinimumSize(100,127);
}
void DecodeList::SetMode(int mode,bool flag_two,bool f0,bool f1,bool f2,bool f3) //,bool ftc,bool fdc,bool ffc,bool fcc
{
    //qDebug()<<fc[0]<<fc[1]<<fc[2]<<fc[3];
    if (s_mode != mode) Clear_List();//1.60= for two decode lists
    //bool show_distc = false;
    //if (f_show_distc) show_distc = true;
    /*&& (id_cont == 0 || id_cont == 2 || id_cont == 3 || id_cont == 5 || id_cont == 6 || id_cont == 7 || id_cont == 8 || id_cont == 13)*/
    if (s_mode==mode && s_flag_two==flag_two && s_show_timec==f0 && s_show_distc==f1 && s_show_freqc==f2 && s_show_counc==f3) return;
    bool rfr_cmsginp = true;
    if (s_mode == mode) rfr_cmsginp = false;

    if (mode==14 || mode==15 || mode==16 || mode==17) allq65 = true;
    else allq65 = false;
    //if (s_list_ident==1) qDebug()<<mode<<flag_two<<ftc<<fdc<<ffc;
    //if (mode==11 || mode==13 || mode==0 || mode==12 || allq65) coded_mods = true;
    //else coded_mods = false;
    if (mode>0 && mode<11) coded_mods = false;//old modes + jt65 pi4
    else coded_mods = true;

    s_mode = mode;
    s_flag_two = flag_two;
    s_show_timec = f0;
    s_show_distc = f1;
    s_show_freqc = f2;
    s_show_counc = f3;

    THvHeader->setSectionResizeMode(QHeaderView::Fixed);//qt5

    QStringList list_A;

    THvHeader->showSection(0);//2.72
    THvHeader->showSection(1);//for two lists jt65
    THvHeader->showSection(2);//w
    THvHeader->showSection(3);//for two lists ft8
    THvHeader->showSection(4);//for two lists jt65
    THvHeader->showSection(5);//for two lists ft8 jt65
    THvHeader->showSection(6);//for two lists ft8
    THvHeader->showSection(7);//for two lists jt65
    THvHeader->showSection(8);//for two lists jt65

    //2.31 false for "\n" jt6m=6 jtms=1
    setUniformRowHeights(true);//2.30 fast wiev

    if (mode==0 || mode==12)//msk144
    {
        //list_A <<"Time"<<"T"<<"Width"<<"dB"<<"Rpt"<<"DF"<<"Message"<<"Dist"<<"Navg"<<"Bit err"<<"Eye"<<"Freq";//11
        if (flag_two)//2.46
        {
            if (s_list_ident == 1) list_A <<tr("Time")<<"T"<<tr("Width")<<"dB"<<"Rpt"<<"DF"<<tr("Message")<<tr("Country")<<tr("Dist")<<"Navg"<<"Bit err"<<"Eye"<<"Freq";//12
            if (s_list_ident == 2) list_A <<tr("Time")<<"T"<<tr("Width")<<"dB"<<"Rpt"<<"DF"<<tr("TX & For-Me Message")<<tr("Country")<<tr("Dist")<<"Navg"<<"Bit err"<<"Eye"<<"Freq";//12

            THvHeader->hideSection(1);
            THvHeader->hideSection(2);
            if (mode==12)
            {
                THvHeader->hideSection(3);//db
                THvHeader->showSection(4);//rpt
            }
            else
            {
                THvHeader->showSection(3);//db
                THvHeader->hideSection(4);//rpt
            }
            THvHeader->hideSection(9);//2.73 Dist Country
            THvHeader->hideSection(10);//2.73 Dist Country
            THvHeader->hideSection(11);//2.73 Dist Country
        }
        else
        {
            list_A <<tr("Time")<<"T"<<tr("Width")<<"dB"<<"Rpt"<<"DF"<<tr("Message")<<tr("Country")<<tr("Dist")<<"Navg"<<"Bit err"<<"Eye"<<"Freq";//12
            if (mode==12)
            {
                THvHeader->showSection(2);//w
                THvHeader->showSection(4);//rpt
            }
            else
            {
                THvHeader->hideSection(2);//w
                THvHeader->hideSection(4);//rpt
            }
            THvHeader->showSection(10);//2.73 Dist Country
            THvHeader->showSection(11);//2.73 Dist Country
        }
        //THvHeader->showSection(11);
        //if (!f_show_timec) THvHeader->hideSection(0);
        if (!f3) THvHeader->hideSection(7);//2.73 Dist Country
        if (!f1) THvHeader->hideSection(8);
        if (!f2) THvHeader->hideSection(12);
        else THvHeader->showSection(12);

        model.setHorizontalHeaderLabels(list_A);

        def_section_sizes[0] = 70;//THvHeader->resizeSection(0, 70);//time 65 to 70 -> 125%
        def_section_sizes[1] = 50;//THvHeader->resizeSection(1, 50);//t
        def_section_sizes[2] = 55;//THvHeader->resizeSection(2, 55);//width
        def_section_sizes[3] = 45;//THvHeader->resizeSection(3, 50);//db ?? be6e 55
        def_section_sizes[4] = 40;//THvHeader->resizeSection(4, 40);//rpt
        def_section_sizes[5] = 50;//THvHeader->resizeSection(5, 50);//df
        def_section_sizes[6] = 150;//THvHeader->resizeSection(6, 150);
        def_section_sizes[7] = COL_CY;//2.73 Country
        def_section_sizes[8] = 55;
        def_section_sizes[9] = 50;//THvHeader->resizeSection(7, 50);// Navg
        def_section_sizes[10] = 50;//THvHeader->resizeSection(8, 50);// Bit Err
        def_section_sizes[11] = 58;//THvHeader->resizeSection(9, 58);// Eye 125%
        def_section_sizes[12]= 70;//THvHeader->resizeSection(10, 70);// * $ ^ E freq  65 to 70 -> 125%

        //THvHeader->setSectionResizeMode(0, QHeaderView::ResizeToContents);//bavi no work bug in qt5.7
        THvHeader->setSectionResizeMode(6, QHeaderView::Stretch);//qt5
        msg_column = 6;
        if (rfr_cmsginp) CountMsgInPeriod(false);
        else SetTimeElapsed(s_ts_);
    }
    if (mode==1 || mode==2 || mode==3)//jtms fsk441 fsk315
    {
        list_A <<tr("Time")<<"T"<<tr("Width")<<"dB"<<"Rpt"<<"DF"<<tr("Message")<<"Freq";//7
        model.setHorizontalHeaderLabels(list_A);

        THvHeader->hideSection(8);
        THvHeader->hideSection(9);
        THvHeader->hideSection(10);
        THvHeader->hideSection(11);
        THvHeader->hideSection(12);//2.73 Dist Country

        def_section_sizes[0] = 70;//THvHeader->resizeSection(0, 70);//time 65 to 70 -> 125%
        def_section_sizes[1] = 50;//THvHeader->resizeSection(1, 50);//t
        def_section_sizes[2] = 55;//THvHeader->resizeSection(2, 55);//width
        def_section_sizes[3] = 40;//THvHeader->resizeSection(3, 40);//db
        def_section_sizes[4] = 40;//THvHeader->resizeSection(4, 40);//rpt
        def_section_sizes[5] = 50;//THvHeader->resizeSection(5, 50);
        def_section_sizes[6] = 150;//THvHeader->resizeSection(6, 150);
        def_section_sizes[7] = 70;//THvHeader->resizeSection(7, 70);//v0.95 * + Freq Hz 65 to 70 -> 125%

        //THvHeader->setSectionResizeMode(0, QHeaderView::ResizeToContents);//bavi no work bug in qt5.7
        THvHeader->setSectionResizeMode(6, QHeaderView::Stretch);//qt5

        //2.31 false for "\n" jt6m=6 jtms=1
        if (mode==1) setUniformRowHeights(false);//2.30 fast wiev

        msg_column = 6;
    }
    if (mode==4 || mode==5)//iskat-a iskat-b
    {
        list_A <<tr("Time")<<tr("Sync")<<"dB"<<"DT"<<"DF"<<"F1"<<tr("Message")<<"Ml"<<"Nw"<<"Na"<<"Ta";//10
        model.setHorizontalHeaderLabels(list_A);

        THvHeader->showSection(8);
        THvHeader->showSection(9);
        THvHeader->showSection(10);
        THvHeader->hideSection(11);
        THvHeader->hideSection(12);//2.73 Dist Country

        def_section_sizes[0] = 70;//THvHeader->resizeSection(0, 70);//time 65 to 70 -> 125%
        def_section_sizes[1] = 50;//THvHeader->resizeSection(1, 50);
        def_section_sizes[2] = 45;//THvHeader->resizeSection(2, 45);//db
        def_section_sizes[3] = 50;//THvHeader->resizeSection(3, 50);//dt
        def_section_sizes[4] = 50;//THvHeader->resizeSection(4, 50);
        def_section_sizes[5] = 50;//THvHeader->resizeSection(5, 50);
        def_section_sizes[6] = 150;//THvHeader->resizeSection(6, 150);
        def_section_sizes[7] = 50;//THvHeader->resizeSection(7, 50);// hv v095 * + ML 45 to 50 -> 125%
        def_section_sizes[8] = 35;//THvHeader->resizeSection(8, 35);
        def_section_sizes[9] = 35;//THvHeader->resizeSection(9, 35);
        def_section_sizes[10] = 50;//THvHeader->resizeSection(10, 50);// v1.32

        //THvHeader->setSectionResizeMode(0, QHeaderView::ResizeToContents);//bavi no work bug in qt5.7
        THvHeader->setSectionResizeMode(6, QHeaderView::Stretch);//qt5

        msg_column = 6;
    }
    if (mode==6)//jt6m
    {
        list_A <<tr("Time")<<"T"<<tr("Width")<<"dB"<<"DF"<<tr("Message")<<"MsgLen"<<"Freq";//7
        model.setHorizontalHeaderLabels(list_A);

        THvHeader->hideSection(8);
        THvHeader->hideSection(9);
        THvHeader->hideSection(10);
        THvHeader->hideSection(11);
        THvHeader->hideSection(12);//2.73 Dist Country

        def_section_sizes[0] = 70;//THvHeader->resizeSection(0, 70);//time 65 to 70 -> 125%
        def_section_sizes[1] = 50;//THvHeader->resizeSection(1, 50);//t
        def_section_sizes[2] = 60;//THvHeader->resizeSection(2, 60);
        def_section_sizes[3] = 40;//THvHeader->resizeSection(3, 40);//db
        def_section_sizes[4] = 50;//THvHeader->resizeSection(4, 50);
        def_section_sizes[5] = 150;//THvHeader->resizeSection(5, 150);
        def_section_sizes[6] = 70;//THvHeader->resizeSection(6, 70);//vazno 60 to Part999 i * + MsgLen
        def_section_sizes[7] = 55;//THvHeader->resizeSection(7, 55);// Freq Hz

        //THvHeader->setSectionResizeMode(0, QHeaderView::ResizeToContents);//bavi no work bug in qt5.7
        THvHeader->setSectionResizeMode(5, QHeaderView::Stretch);//qt5

        //2.31 false for "\n" jt6m=6 jtms=1
        setUniformRowHeights(false);//2.30 fast wiev

        msg_column = 5;
    }
    if (mode==7 || mode==8 || mode==9)//jt65abc
    {
        //list_A <<"Time"<<"Sync"<<"dB"<<"DT"<<"DF"<<"W"<<"Message"<<"D Inf"<<"Flags"<<"Freq";//9 Flags
        if (flag_two)
        {
            if (s_list_ident == 1) list_A <<tr("Time")<<tr("Sync")<<"dB"<<"DT"<<"DF"<<"W"<<tr("Message")<<"D Inf"<<"Flags"<<"Freq";//9 Flags
            if (s_list_ident == 2) list_A <<tr("Time")<<tr("Sync")<<"dB"<<"DT"<<"DF"<<"W"<<tr("RX Frequency Message")<<"D Inf"<<"Flags"<<"Freq";//9 Flags
            THvHeader->hideSection(1);
            THvHeader->hideSection(4);
            THvHeader->hideSection(5);
            THvHeader->hideSection(7);
            THvHeader->hideSection(8);
        }
        else list_A <<tr("Time")<<tr("Sync")<<"dB"<<"DT"<<"DF"<<"W"<<tr("Message")<<"D Inf"<<"Flags"<<"Freq";//9 Flags

        THvHeader->showSection(9);
        THvHeader->hideSection(10);
        THvHeader->hideSection(11);
        THvHeader->hideSection(12);//2.73 Dist Country

        model.setHorizontalHeaderLabels(list_A);

        def_section_sizes[0] = 70;//THvHeader->resizeSection(0, 70);//time 65 to 70 -> 125%
        def_section_sizes[1] = 50;//THvHeader->resizeSection(1, 50);//sync
        def_section_sizes[2] = 45;//THvHeader->resizeSection(2, 45);//db 40
        def_section_sizes[3] = 50;//THvHeader->resizeSection(3, 50);//dt
        def_section_sizes[4] = 50;//THvHeader->resizeSection(4, 50);
        def_section_sizes[5] = 50;//THvHeader->resizeSection(5, 50);
        def_section_sizes[6] = 150;//THvHeader->resizeSection(6, 150);
        def_section_sizes[7] = 40;//THvHeader->resizeSection(7, 40);// csync
        def_section_sizes[8] = 90;//THvHeader->resizeSection(8, 90);//1.58=90 +VHF 1.57=60 for AP flag=65 55 cflags +VHF
        def_section_sizes[9] = 55;//THvHeader->resizeSection(9, 55);//freq

        //THvHeader->setSectionResizeMode(0, QHeaderView::ResizeToContents);//bavi no work bug in qt5.7
        THvHeader->setSectionResizeMode(6, QHeaderView::Stretch);//qt5

        msg_column = 6;
        if (rfr_cmsginp) CountMsgInPeriod(false);
        else SetTimeElapsed(s_ts_);
    }
    if (mode==10)//pi4
    {
        list_A <<tr("Time")<<tr("Sync")<<"dB"<<"DT"<<"DF"<<"W"<<tr("Message")<<"D Inf"<<"Dec"<<"Freq";//9 Flags
        model.setHorizontalHeaderLabels(list_A);

        THvHeader->showSection(8);
        THvHeader->showSection(9);
        THvHeader->hideSection(10);
        THvHeader->hideSection(11);
        THvHeader->hideSection(12);//2.73 Dist Country

        def_section_sizes[0] = 70;//THvHeader->resizeSection(0, 70);//time 65 to 70 -> 125%
        def_section_sizes[1] = 50;//THvHeader->resizeSection(1, 50);//sync
        def_section_sizes[2] = 50;//THvHeader->resizeSection(2, 50);//db 40
        def_section_sizes[3] = 50;//THvHeader->resizeSection(3, 50);//dt
        def_section_sizes[4] = 50;//THvHeader->resizeSection(4, 50);
        def_section_sizes[5] = 50;//THvHeader->resizeSection(5, 50);
        def_section_sizes[6] = 150;//THvHeader->resizeSection(6, 150);
        def_section_sizes[7] = 40;//THvHeader->resizeSection(7, 40);// csync
        def_section_sizes[8] = 60;//THvHeader->resizeSection(8, 60);//1.60= cflags
        def_section_sizes[9] = 55;//THvHeader->resizeSection(9, 55);//freq

        //THvHeader->setSectionResizeMode(0, QHeaderView::ResizeToContents);//bavi no work bug in qt5.7
        THvHeader->setSectionResizeMode(6, QHeaderView::Stretch);//qt5

        msg_column = 6;
    }
    if (mode==11 || mode==13 || mode==18 || allq65)//ft8 ft4
    {
        QString qdc = "Qual";
        if (allq65) qdc = "Dec";
        if (flag_two)
        {
            if (s_list_ident == 1) list_A <<tr("Time")<<"dB"<<"DT"<<"DF Of TX"<<tr("Message")<<tr("Country")<<tr("Dist")<<tr("Type")<<qdc<<"Freq";//9 Flags
            if (s_list_ident == 2) list_A <<tr("Time")<<"dB"<<"DT"<<"DF Of TX"<<tr("RX Frequency Message")<<tr("Country")<<tr("Dist")<<"Type"<<qdc<<"Freq";//9 Flags
            THvHeader->hideSection(3);
            THvHeader->hideSection(7);//2.73 Dist Country
            THvHeader->hideSection(8);//2.73 Dist Country
        }
        else list_A <<tr("Time")<<"dB"<<"DT"<<"DF Of TX"<<tr("Message")<<tr("Country")<<tr("Dist")<<tr("Type")<<qdc<<"Freq";//9 Flags

        THvHeader->showSection(9);
        THvHeader->hideSection(10);
        THvHeader->hideSection(11);
        THvHeader->hideSection(12);//2.73 Dist Country
        if (!f0 && !allq65) THvHeader->hideSection(0);
        if (!f3) THvHeader->hideSection(5);//2.73 Dist Country
        if (!f1 ||  allq65) THvHeader->hideSection(6);//2.73 Dist Country
        if (!f2 && !allq65) THvHeader->hideSection(9);//2.73 Dist Country

        model.setHorizontalHeaderLabels(list_A);

        def_section_sizes[0] = 70;//THvHeader->resizeSection(0, 70);//time 65 to 70 -> 125%
        def_section_sizes[1] = 45;//THvHeader->resizeSection(1, 45);//db
        def_section_sizes[2] = 50;//THvHeader->resizeSection(2, 50);//dt
        def_section_sizes[3] = 90;//THvHeader->resizeSection(3, 90);//df +125% 80
        def_section_sizes[4] = 150;//THvHeader->resizeSection(4, 150);
        def_section_sizes[5] = COL_CY;//2.73 Country
        def_section_sizes[6] = 55;
        def_section_sizes[7] = 70;//THvHeader->resizeSection(5, 70);//Type
        def_section_sizes[8] = 50;//THvHeader->resizeSection(6, 50);//Q
        def_section_sizes[9] = 55;//THvHeader->resizeSection(7, 55);//freq

        //THvHeader->setSectionResizeMode(0, QHeaderView::Interactive);//1.81 for font
        THvHeader->setSectionResizeMode(4, QHeaderView::Stretch);//qt5

        msg_column = 4;
        if (rfr_cmsginp) CountMsgInPeriod(false);
        else SetTimeElapsed(s_ts_);
    }

    if (flag_two) CorrListSize();
    else setMinimumWidth(100);//setMinimumSize(100,127);
    SaveAndCorrSectionSize(); //qDebug()<<"uniformRowHeights="<<uniformRowHeights();

    if (s_list_ident==1)
    {
        RefreshFastDcml();
        RefreshFiltHeadColor(show_filter_list,hide_filter_list,show_customf_list,show_cufspec_list,
                             show_cufend_list,show_cnyf_list,show_pfxf_list,hide_cnyf_list,f_hide_c[8]);
    }
}
void DecodeList::SetActivityId(QString myloc)
{
    s_my_loc = myloc; //qDebug()<<"SetActivity"<<myloc;
}
void DecodeList::SetDistUnit(bool f)
{
    f_km_mi = f; //qDebug()<<"SetDistUnit"<<f;
}
void DecodeList::SetHisCalls(QStringList l)
{
    l_hiscals = l; //if (s_list_ident==1) qDebug()<<"HisCalls="<<l_hiscals;
}
void DecodeList::keyPressEvent(QKeyEvent * event)
{
    if (event->matches(QKeySequence::Copy))
    {
        QModelIndex index = selectionModel()->currentIndex();
        QString active;
        QString str;

        active.append(ModeStr(s_mode)+" ");

        if (s_mode==0 || s_mode==12)//MSK144
        {
            active.append(model.item(index.row(),0)->text()+" ");//Time
            active.append(model.item(index.row(),1)->text()+" s ");//T Sec
            if (s_mode==12)
                active.append(model.item(index.row(),2)->text()+" ms ");//width
            active.append(model.item(index.row(),3)->text()+" dB ");//db
            if (s_mode==12)
                active.append(model.item(index.row(),4)->text()+" ");//RPT

            active.append(model.item(index.row(),5)->text()+" Hz ");//DF

            if (s_mode==0)
            {
                QString s_s = model.item(index.row(),12)->text();//2.73 Dist Country
                s_s.remove("*");//(unpackjt:text->'$' codet->'*') msk40->'#' rtd->'^','&'  , eq->'@'
                s_s.remove("$");
                s_s.remove("#");
                s_s.remove("&");// mislia niamam
                s_s.remove("^");
                s_s.remove("@");
                //s_s.remove("!");
                active.append("Freq"+s_s+" Hz > ");//freq
            }
            else
                active.append("> ");

            str = model.item(index.row(),6)->text();
            //str.remove("\n");// zaradi JT6M
            str.replace("\n"," ");// zaradi JT6M jtms
            active.append(str);//MSG
        }
        else if (s_mode==1 || s_mode==2 || s_mode==3) //JTMS FSK441 FSK315
        {
            active.append(model.item(index.row(),0)->text()+" ");//Time
            active.append(model.item(index.row(),1)->text()+" s ");//T Sec
            active.append(model.item(index.row(),2)->text()+" ms ");//Width
            active.append(model.item(index.row(),3)->text()+" dB ");//db
            active.append(model.item(index.row(),4)->text()+" ");//RPT
            active.append(model.item(index.row(),5)->text()+" Hz > ");//DF

            str = model.item(index.row(),6)->text();
            //str.remove("\n");// zaradi JT6M
            str.replace("\n"," ");// zaradi JT6M jtms
            active.append(str);//MSG
        }
        else if (s_mode==4 || s_mode==5) // ISCAT-A ISCAT-B
        {
            active.append(model.item(index.row(),0)->text()+" ");//Time
            active.append(model.item(index.row(),1)->text()+" ");//Sync
            active.append(model.item(index.row(),2)->text()+" dB ");//db
            active.append(model.item(index.row(),3)->text()+" s ");//DT time
            active.append(model.item(index.row(),4)->text()+" Hz ");//DF
            active.append(model.item(index.row(),5)->text()+" Hz > ");//F1

            str = model.item(index.row(),6)->text();
            //str.remove("\n");// zaradi JT6M
            str.replace("\n"," ");// zaradi JT6M jtms
            active.append(str);//MSG
        }
        else if (s_mode==6) // JT6M
        {
            active.append(model.item(index.row(),0)->text()+" ");//Time
            active.append(model.item(index.row(),1)->text()+" s ");//T Sec
            active.append(model.item(index.row(),2)->text()+" s ");//Width
            active.append(model.item(index.row(),3)->text()+" dB ");//db
            active.append(model.item(index.row(),4)->text()+" Hz > ");//DF

            str = model.item(index.row(),5)->text();
            //str.remove("\n");// zaradi JT6M
            str.replace("\n"," ");// zaradi JT6M jtms
            active.append(str);//MSG
        }
        else if (s_mode==7 || s_mode==8 || s_mode==9  || s_mode==10) // jt65abc pi4 neznam za sga
        {
            active.append(model.item(index.row(),0)->text()+" ");//Time
            active.append(model.item(index.row(),1)->text()+" ");//Sync
            active.append(model.item(index.row(),2)->text()+" dB ");//db
            active.append(model.item(index.row(),3)->text()+" s ");//DT time
            active.append(model.item(index.row(),4)->text()+" Hz ");//DF
            active.append(model.item(index.row(),5)->text()+" W ");//W

            QString s_s = model.item(index.row(),9)->text();
            active.append("Freq "+s_s+" Hz > ");//freq

            str = model.item(index.row(),6)->text();
            //str.remove("\n");// zaradi JT6M
            str.replace("\n"," ");// zaradi JT6M jtms
            active.append(str);//MSG
        }
        else if (s_mode==11 || s_mode==13 || s_mode==18 || allq65) // ft8 ft4
        {
            active.append(model.item(index.row(),0)->text()+" ");//Time
            active.append(model.item(index.row(),1)->text()+" dB ");//db
            active.append(model.item(index.row(),2)->text()+" s ");//DT time
            //active.append(model.data(model.index(index.row(),3,QModelIndex()),Qt::DisplayRole).toString()+" Hz ");//DF
            //active.append(model.data(model.index(index.row(),5,QModelIndex()),Qt::DisplayRole).toString()+" W ");//W

            QString s_s = model.item(index.row(),9)->text();//2.73 Dist Country
            active.append("Freq "+s_s+" Hz > ");//freq

            str = model.item(index.row(),4)->text();
            //str.remove("\n");// zaradi JT6M
            str.replace("\n"," ");// zaradi JT6M jtms
            active.append(str);//MSG
        }
        else // Unknown
        {
            for (int i = 0; i<model.columnCount(); i++)
            {
                if (i < model.columnCount() - 1)
                    active.append(model.item(index.row(),i)->text()+" ");
                else
                    active.append(model.item(index.row(),i)->text());
            }
        }
        clipboard->setText(active);
    }
    else if (event->matches(QKeySequence::SelectAll)) event->ignore();//for AutoOn shortkey
    else QTreeView::keyPressEvent(event); // za da raboti keybord shortkeys Application
}
void DecodeList::RefreshListTimer()
{
    //qDebug()<<model.rowCount();
    list_rfresh_timer->stop();
    /*for (int i = 0; i<qlsi_st_c; ++i)
        model.appendRow(qlsi_st[i]);
    qlsi_st_c = 0;*/
    setCurrentIndex(model.index(model.rowCount()-1,6));
    scrollToBottom();
}
void DecodeList::HideShowWfTxRefreshList()//2.64
{
    if (!list_rfresh_timer->isActive() && model.rowCount()>0) list_rfresh_timer->start(80);//have min 1 item
}
void DecodeList::SetBackColor(bool f)
{
    //qDebug()<<"f_row_color="<<is_filter_dec<<f_bgcolor_clr_lst<<model.rowCount();
    //if ((s_mode==11 || s_mode==13|| s_mode==18) && is_filters_active && !is_filter_dec && !f_bgcolor_clr_lst)//2.66 old && model.rowCount()>0
    bool is_fact = is_filters_active; //2.66 //bool is_fdec = is_filter_dec;
    if (!is_fact && g_block_list_exp && s_list_ident==1)//2.66 importent->s_list_ident==1 g_block_list_exp<-problem for list=2
    {
        is_fact = true;
        is_filter_dec = false;
    }

    if ((s_mode==11 || s_mode==13 || s_mode==18) && is_fact && !is_filter_dec && !f_bgcolor_clr_lst) //2.66
    {
        if (f) emit EmitLstNexColor(false);
        else   emit EmitLstNexColor(true);

        if (f) f = false;
        else   f = true;
    }

    f_bgcolor_clr_lst = false;
    f_row_color = f;
    count_dec = 0;
    is_filter_dec = false;
    //s_ts_ = -1.0;//2.46 protect all modes from /0.0s
}
void DecodeList::SetBackColorTxQsy(bool f)//2.46
{
    f_mark_txqsy = f;//tx=0 qsy=1;
}
void DecodeList::RefreshFastDcml()
{
    if (f_new_dec_clr_msg_list && (s_mode==11 || s_mode==13 || s_mode==18)) s_fast_dec_clr_msg_list = true;//ft4
    else s_fast_dec_clr_msg_list = false;
    //qDebug()<<"s_fast_dec_clr_msg_list="<<s_list_ident<<s_fast_dec_clr_msg_list;
}
void DecodeList::SetNewDecClrMsgListFlag(bool f)
{
    if (s_list_ident==1)
    {
        f_new_dec_clr_msg_list = f;
        RefreshFastDcml();
        if (s_mode==11 || s_mode==13 || s_mode==18)//ft4
        {
            if (f)
            {
                //f_bgcolor_clr_lst = true;
                emit EmitLstNexColor(false);
            }
            else
            {
                //f_bgcolor_clr_lst = false;//2.66=old
                //EmitLstNexColor(true);    //2.66=old
                if (f_row_color)//2.66
                {
                    //qDebug()<<"1--false="<<f_bgcolor_clr_lst;
                    f_bgcolor_clr_lst = false;
                    emit EmitLstNexColor(true);
                }
                else
                {
                    //qDebug()<<"2--true="<<f_bgcolor_clr_lst;
                    f_bgcolor_clr_lst = true;
                    emit EmitLstNexColor(false);
                }
            }
        }
    } //qDebug()<<"f_new_dec_clr_msg_lis="<<f_new_dec_clr_msg_list;
}
void DecodeList::SetFilterParms(QStringList l,QStringList &lc,bool &f)
{
    if (l.isEmpty())
    {
        f = false;
        lc.clear();
    }
    else
    {
        lc = l;
        f = true;
    }
}
void DecodeList::SetFilter(QStringList lc,bool *fh,QStringList lc0,QStringList lc1,QStringList lc2,
                           QStringList lc3,QStringList lc4,QStringList lc5)
{
    //if (s_list_ident!=1) return; //2.66 no needed for the moment
    f_hide_c[0]=fh[0];
    f_hide_c[1]=fh[1];
    f_hide_c[2]=fh[2];
    f_hide_c[3]=fh[3];
    f_hide_c[4]=fh[4];
    f_hide_c[5]=fh[5];
    f_hide_c[6]=fh[6];
    f_hide_c[7]=fh[7];//usefudpdectxt
    f_hide_c[8]=fh[8];//b4qso
    f_hide_c[9]=fh[9];//filtered_answer
    if (f_hide_c[0] || f_hide_c[1] || f_hide_c[2] || f_hide_c[3] || f_hide_c[4] || f_hide_c[5] || f_hide_c[6]) hide_filter_list = true;
    else hide_filter_list =false;

    SetFilterParms(lc, list_filter, show_filter_list);
    SetFilterParms(lc0,list_customf,show_customf_list);
    SetFilterParms(lc1,list_cufspec,show_cufspec_list);
    SetFilterParms(lc2,list_cufend, show_cufend_list);
    SetFilterParms(lc3,list_cnyf,   show_cnyf_list);
    SetFilterParms(lc4,list_pfxf,   show_pfxf_list);
    SetFilterParms(lc5,list_hidcnyf,hide_cnyf_list);

    RefreshFiltHeadColor(show_filter_list,hide_filter_list,show_customf_list,show_cufspec_list,
                         show_cufend_list,show_cnyf_list,show_pfxf_list,hide_cnyf_list,f_hide_c[8]);
    /*qDebug()<<is_filters_active<<"F="<<f_hide_c[0]<<f_hide_c[1]<<f_hide_c[2]<<f_hide_c[3]<<f_hide_c[4]<<f_hide_c[5]<<
    f_hide_c[6]<<list_filter<<list_cnyf<<list_pfxf<<list_cufend<<list_customf<<list_cufspec<<f_hide_c[7];*/
    //qDebug()<<"f_hide_c[9]"<<f_hide_c[9];
}
bool DecodeList::AllisDigitOrAllisLetter(QString s)
{
    bool res = false;
    bool alldig = true;
    bool alllet = true;
    for (int j = 0; j < s.count(); ++j)
    {
        if (alldig && s.at(j).isLetter()) alldig = false;
        if (alllet && s.at(j).isDigit() ) alllet = false;
        if (!alldig && !alllet) break;
    }
    if (alldig || alllet) res = true;  //qDebug()<<alldig<<alllet;
    return res;
    /*bool res = true;
    bool alldig = true;
    bool alllet = true;
    for (int j = 0; j < s.count(); ++j)
    {
        if (alldig && s.at(j).isLetter()) alldig = false;
        if (alllet && s.at(j).isDigit() ) alllet = false;
        if (!alldig && !alllet) 
        {
        	res = false;
            break;	
       	}
    }
    return res;*/
}
QString DecodeList::FindHisCall(QString c)
{
    //"Z9Z RR73; A9ZZ <LZ2HVV> -04";
    //"TU; G3AAA LZ2HVV R-09 FN   ";
    //"CQ TEST LZ2HVV KN23"
    //"CQ 001 LZ2HVV KN23"
    //"CQ DX LZ2HVV KN23"
    //"CQ RU LZ2HVV KN23"
    //c="CQ LZ2000HV";
    //c="W0A/B KN23";
    QString res = "";
    QString tstr = c;
    tstr.remove("<");
    tstr.remove(">");
    if (tstr.isEmpty()) return res;//2.44
    //if (tstr.mid(0,4)=="TU; ")
    //tstr.remove("TU; ");
    QStringList lm = tstr.split(" ");
    int clm = lm.count();
    bool clm2 = false;
    if (clm > 2) clm2 = true;

    if (clm == 1)
    {
        if (tstr!="...") res = tstr; //2.44 LZ2HV/IMAGIE;   if (tstr!="...")
    }
    else if (clm > 3 && lm.at(1)=="RR73;")
    {
        if (lm.at(3)!="...") res=lm.at(3); //"ZZ9ZZZ RR73; ZZ9ZZZ <KH1/KH7ZZZZ> -04";
    }
    else if (clm2 && lm.at(0)=="TU;") res=lm.at(2); //"TU; G3AAA K1ABC R-09 FN";
    else if (clm > 1)
    {
        QString t2c = lm.at(1);
        if (t2c!="...") // && t2c!="RR73;"  exception no find in dB prefix <...>
        {
            if (clm2 && t2c.count()<3) res=lm.at(2); //"CQ RU LZ2HVV KN23"
            else
            {
                bool alldorl = AllisDigitOrAllisLetter(t2c);
                if (alldorl && clm2) res=lm.at(2);//"CQ TEST LZ2HVV KN23" "CQ 001 LZ2HVV KN23" "LZ2INT TNX 73"
                else if (!alldorl) res=lm.at(1);  //"CQ LZ2000HV" "CQ LZ2HV KN23" "LZ2HV QSOB4"
                //qDebug()<<"............="<<clm<<alldorl<<res<<c;
            }
            //protection from
            if (res=="73" || res=="RR73" || res=="QSOB4") res=lm.at(0);
        }
    }
    if (res.count()<3) res=""; //"TNX 73 GL"
    else
    {
        bool alldorl = AllisDigitOrAllisLetter(res);
        if (alldorl) res="";
        else if (!THvQthLoc.isValidCallsign(res)) res = "";//2.66
        //2.75 "LZ2HV/B KN23" "W0A/B KN23"
        if (clm == 2/*&& THvQthLoc.isValidCallsign(lm.at(0))*/)
        {
        	int idx = lm.at(0).indexOf("/B"); //qDebug()<<idx<<lm.at(0);
        	if (idx>2 && THvQthLoc.isValidLocator(res)) res = lm.at(0);
       	}
    } //qDebug()<<"out="<<res.leftJustified(13,' ')<<" in="<<tstr; 
    return res;
}
bool DecodeList::ShowDecode(QString s)// show CQ,73,RR73
{
    if (s.isEmpty()) return false;
    bool res = false;

    int sc = s.count();
    for (int i = 0; i<list_filter.count(); ++i)
    {
        QString flt = list_filter.at(i);
        int cf = flt.count();
        if (sc<cf) continue;//exception msg small then filter
        int ss = 0;
        if (flt.at(0)==' ') ss = sc - cf; //qDebug()<<sc<<cf<<"fff="<<ss<<ss+cf;
        if (s.mid(ss,cf)==flt)
        {
            res = true;
            break;
        }
    }
    return res;
}
bool DecodeList::ShowCDecode(QString call)// /P,/QRP
{
    if (call.isEmpty()) return false;
    bool res = false;
    for (int i = 0; i<list_customf.count(); ++i)
    {
        if (call.contains(list_customf.at(i)))
        {
            res = true;
            break;
        }
    }
    return res;
}
bool DecodeList::ShowCENDDecode(QString s)// KN,FN
{
    if (s.isEmpty()) return false;
    QString end = s;
    int ili = s.lastIndexOf(" ")+1;
    if (ili>0) end = s.mid(ili,s.count()-ili);
    bool res = false;
    for (int i = 0; i<list_cufend.count(); ++i)
    {
        if (end.contains(list_cufend.at(i)))
        {
            res = true;
            break;
        }
    }
    return res;
}
bool DecodeList::ShowCSDecode(QString call)
{
    if (call.isEmpty()) return false;
    bool res = false;
    for (int i = 0; i<list_cufspec.count(); ++i)
    {
        if (call==list_cufspec.at(i))
        {
            res = true;
            break;
        }
    }
    return res;
}
bool DecodeList::HideB4Qso(QString call)
{
    if (call.isEmpty()) return true;
    bool res = true;
    for (int i = 0; i<list_b4qso.count(); ++i)
    {
        if (call==list_b4qso.at(i))
        {
            res = false;
            break;
        }
    }
    return res;
}
QString DecodeList::FindCountry(QString sid,QString txt,QString call)
{
    QString res = "";
    if (!s_show_counc) return res;
    if (sid.startsWith("TX") || sid=="QSY") return res;
    if (call.isEmpty()) call = FindHisCall(txt);
    res = THvCty->FindCountry(call,false);
    return res;
}
QString DecodeList::CalcDistance(QString sid,QString txt)
{
    QString res = "";
    if (allq65 || !s_show_distc) return res;
    if (sid.startsWith("TX") || sid=="QSY") return res;
    res = "-";
    QString hloc = "";
    QStringList l = txt.split(" ");
    QString s = l.at(l.count()-1);
    if (l.count()>2 && s!="RR73") hloc = s;
    if (!THvQthLoc.isValidLocator(hloc) || !THvQthLoc.isValidLocator(s_my_loc)) return res;
    QString c_test_loc = THvQthLoc.CorrectLocator(hloc);
    QString c_my_loc = THvQthLoc.CorrectLocator(s_my_loc);
    double dlong1 = THvQthLoc.getLon(c_my_loc);
    double dlat1  = THvQthLoc.getLat(c_my_loc);
    double dlong2 = THvQthLoc.getLon(c_test_loc);
    double dlat2  = THvQthLoc.getLat(c_test_loc);
    if (!f_km_mi) res = QString("%1").arg(THvQthLoc.getDistanceKilometres(dlong1,dlat1,dlong2,dlat2));
    else res = QString("%1").arg(THvQthLoc.getDistanceMilles(dlong1,dlat1,dlong2,dlat2));
    return res;
}
bool DecodeList::isNotTxnQsyCq(QString s)
{
    if (!s.startsWith("TX") && s!="QSY" && s!="CQ") return true;
    else return false;
}
bool DecodeList::isFromHiscals(QString s)
{
    bool f = false;
    s.remove("<");
    s.remove(">");
    QStringList tlist = s.split(" ");
    for (int i = 0; i<l_hiscals.count(); ++i)
    {
        for (int j = 0; j<tlist.count(); ++j)
        {
            if (tlist.at(j)==l_hiscals.at(i))
            {
                f = true;
                break;
            }
            if (f) break;
        }
    }
    return f;
}
void DecodeList::SetOtpVerif(QString s,uint8_t id)
{	
	QStringList l = s.split("#");//id=1,verified id=2,invalid
	id_mark_otp_verif = id;
	InsertItem_hv(l,true,true);
	id_mark_otp_verif = 0;
}
void DecodeList::SetShowOtpRxMsg(bool f)
{
	f_otp_show_msg = f; //qDebug()<<f;
}
/*uint8_t DecodeList::isVerfOrOtpmsg(QString s,bool f)
{
	uint8_t id = 0;//0=normal msg, 1=$VERIFY$, 2=C0ALL.123456, 3=verified
	if (s_mode!=11) return 0;
	if 		(s.startsWith("$VERIFY$ ")) id = 1;
    else if (s.contains(QRegularExpression{"^[A-Z0-9]{2,6}\\.[0-9]{6}"})) id = 2;
    else if (f) id = 3;
    else if (s.endsWith(" verified")) id = 3;  
    return id;	
}*/
void DecodeList::InsertItem_hv(QStringList list,bool ffopen,bool forme)
{
    if (g_block_list_exp) return;
    if (!list.isEmpty())
    {
        bool f_show = true;
        bool freply = true;
        bool f_sho0 = true;
        QString call = "";
        if (is_filters_active && f_hide_c[9] && !is_only_cqrr73_active) freply = isFromHiscals(list.at(4));
        if ((is_filters_active && !forme) || (is_filters_active && !freply))//no needed ->(s_mode == 11 || s_mode == 13|| s_mode==18)
        {
            //QString call = "";
            bool f_b4qso = false;
            if (f_hide_c[8] && !list_b4qso.isEmpty()) f_b4qso = true; //qDebug()<<"f_b4qso="<<f_b4qso;//2.69
            if (hide_filter_list || show_customf_list || show_cufspec_list || show_cnyf_list ||
                    show_pfxf_list || hide_cnyf_list || f_b4qso) call = FindHisCall(list.at(4));

            QString pfx,coy,cot;
            bool ispfx  = false;
            if (hide_filter_list || show_cnyf_list || show_pfxf_list || hide_cnyf_list) ispfx = THvCty->FilndDbPfx(call,pfx,coy,cot);

            /*if (count_dec==0) qDebug()<<"------------------------------------------------------------------";
            qDebug()<<"Call="<<call.leftJustified(13,' ')<<" StdPfx="<<pfx.leftJustified(5,' ')<<" in="<<list.at(4);*/

            bool pshow = false;
            if (show_customf_list)// /P,/QRP
            {
                pshow = ShowCDecode(call);
                f_sho0 = pshow;
            }
            if (show_cufend_list && !pshow)// KN,FN
            {
                pshow = ShowCENDDecode(list.at(4));
                f_sho0 = pshow;
            }
            if (show_cufspec_list && !pshow)// LZ2HV,SP9HWY
            {
                pshow = ShowCSDecode(call);
                f_sho0 = pshow;
            }
            if (show_cnyf_list && !pshow) // Country pfx
            {
                pshow =  THvCty->ShowCNYDecode(call,list_cnyf,ispfx,coy);
                f_sho0 = pshow;
            }
            if (show_pfxf_list && !pshow) // custom pfx
            {
                pshow =  THvCty->ShowPFXDecode(call,list_pfxf,ispfx,pfx);
                f_sho0 = pshow;
            }
            if (!pshow)
            {
                if (show_filter_list) f_sho0 = ShowDecode(list.at(4));//show CQ,73,RR73
                if (hide_filter_list && f_sho0) f_sho0 = THvCty->HideContinent(call,f_hide_c,ispfx,cot);//Hide Continent
                if (hide_cnyf_list && f_sho0) f_sho0 = THvCty->HideCountry(call,list_hidcnyf,coy);//Hide Country //ispfx,
                if (f_b4qso && f_sho0) f_sho0 = HideB4Qso(call);//Hide B4QSO
            }
        }
        if (is_filters_active && !freply) freply = f_sho0;
        //if (is_filters_active && !forme ) freply = false;
        if (is_filters_active && !forme ) f_show = f_sho0;
        //if (s_list_ident==1/* && f_show!=freply*/) qDebug()<<"f_show="<<f_show<<"freply="<<freply<<list.at(4);

        //error counting with filter f_show &&
        if (s_fast_dec_clr_msg_list && last_pt_dec_clr_msg_list!=list.at(0))//2.07 exception for future && new_pt
        {
            last_pt_dec_clr_msg_list=list.at(0);
            Clear_List();
        }

        QString alltxt;
        alltxt.append(ModeStr(s_mode)+"|");
        QList<QStandardItem *> qlsi;
        int k = 0;
        uint8_t falltxt[2];
        falltxt[0] = 100; //no save column
        falltxt[1] = 100; //no save column

        uint8_t id_vomsg = 0;
        if (s_mode==11)//2.76sf 0=normal msg, 1=$VERIFY$, 2=C0ALL.123456, 3=verified
        {
        	if (list.at(4).startsWith("$VERIFY$ "))
        	{
        		if (!f_otp_show_msg) f_show = false;
        		if (s_list_ident == 2 && list.at(1).startsWith("TX")) f_show = true;//only from 2 list my TX
        		id_vomsg = 1;
       		}
       		else if (list.at(4).contains(QRegularExpression{"^[A-Z0-9]{2,6}\\.[0-9]{6}"}))
       		{
        		if (!f_otp_show_msg) f_show = false;
        		if (s_list_ident == 2 && list.at(1).startsWith("TX")) f_show = true;//only from 2 list my TX
        		id_vomsg = 2;       			
      		}
       		else if (id_mark_otp_verif>0) id_vomsg = 3;//verified & invalid & not found
       		//else if (id_mark_otp_verif==2) id_voms = 4;//invalid
       	}//if (s_list_ident==1) qDebug()<<"id_voms="<<id_voms<<"f_show="<<f_show;

        if (s_mode==11 || s_mode==13 || s_mode==18 || allq65)
        {
        	if (id_vomsg>0)//2.76sf 
        	{
        		list.insert(5,"");//list.insert(5, FindCountry("","",list.at(4).split(" ").at(0))); 
        		list.insert(6,"");      		
       		}
            else 
            {
            	list.insert(5, FindCountry(list.at(1),list.at(4),call)); 
            	list.insert(6,CalcDistance(list.at(1),list.at(4)));           	
           	}            
            falltxt[0] = 5;
            falltxt[1] = 6;
        }
        else if (s_mode==0)
        {
            list.insert(7, FindCountry(list.at(3),list.at(6),call));
            list.insert(8,CalcDistance(list.at(3),list.at(6)));
            falltxt[0] = 7;
            falltxt[1] = 8;
        }
        else if (s_mode==12)
        {
            list.insert(7, FindCountry(list.at(4),list.at(6),call));
            list.insert(8,CalcDistance(list.at(4),list.at(6)));
            falltxt[0] = 7;
            falltxt[1] = 8;
        }        

        for (QStringList::iterator it =  list.begin(); it != list.end(); ++it)
        {
            QString txtin = QString(*it);
            if (f_show)
            {
                QStandardItem *item = new QStandardItem(txtin);
                item->setEditable(false);// no double click all and down allow only for call and rpt
                if (s_mode==6)//JT6M
                {
                    if (k==5)
                    {
                        item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
                        item->setEditable(true); // yes double click for select call
                    }
                    else item->setTextAlignment(Qt::AlignCenter);
                }
                else if (s_mode==0)//msk144
                {
                    if (k==3)
                    {
                        if (isNotTxnQsyCq(list.at(3))) item->setEditable(true); // yes double click for select rpt
                    }
                    if (k==6)
                    {
                        item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
                        if (isNotTxnQsyCq(list.at(3))) item->setEditable(true); // yes double click for select call
                    }
                    else if (k==7) item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);//2.73 Dist Country  freq
                    else item->setTextAlignment(Qt::AlignCenter);
                    if (k==12) item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter); //2.73 Dist Country
                }
                else if (s_mode==12)//msk144ms
                {
                    if (k==4)
                    {
                        if (isNotTxnQsyCq(list.at(4))) item->setEditable(true); // yes double click for select rpt
                    }
                    if (k==6)
                    {
                        item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
                        if (isNotTxnQsyCq(list.at(4))) item->setEditable(true); // yes double click for select call
                    }
                    else if (k==7) item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);//2.73 Dist Country  freq
                    else item->setTextAlignment(Qt::AlignCenter);
                    if (k==12) item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);//2.73 Dist Country
                }
                else if (s_mode==11 || s_mode==13 || s_mode==18 || allq65)//ft8 ft4
                {
                    if (k==1)
                    {
                        if (isNotTxnQsyCq(list.at(1))) item->setEditable(true); // yes double click for select rpt
                    }
                    if (k==4)
                    {
                        item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
                        if (isNotTxnQsyCq(list.at(1))) item->setEditable(true); // yes double click for select call
                    }
                    else if (k==5) item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);//2.73 Dist Country  freq
                    else item->setTextAlignment(Qt::AlignCenter);
                    if (k==9)//2.73 Dist Country  freq
                    {
                        if (isNotTxnQsyCq(list.at(1))) item->setEditable(true);//1.60= no if my TX
                    }
                }
                else//jtms fsk315 fsk441 iscat-a iscat-b jt65abc pi4
                {
                    if (k==6)
                    {
                        item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
                        if (isNotTxnQsyCq(list.at(2))) item->setEditable(true); // yes double click for select call //1.60= no if my TX
                    }
                    else item->setTextAlignment(Qt::AlignCenter);
                    if (k==7 && (s_mode==1 || s_mode==4 || s_mode==5)) //jtms iscat-a iscat-b for char ->*
                        item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
                    if (k==4 && (s_mode==1 || s_mode==2 || s_mode==3)) //jtms fsk441 fsk315
                        item->setEditable(true); // yes double click  for select rpt
                    if (k==2 && (s_mode==4 || s_mode==5 )) //iscat-a iscat-b
                        item->setEditable(true); // yes double click for select rpt
                    if ((k==2 || k==9) && (s_mode==7 || s_mode==8 || s_mode==9 || s_mode==10)) //jt65abc pi4
                    {
                        if (isNotTxnQsyCq(list.at(2))) item->setEditable(true); // yes double click  for select rpt  or freq //1.60= no if my TX
                    }
                }
                //qDebug()<<"color="<<f_row_color;
                //QColor cc = palette().color(QPalette::Base);
                QColor cc = QColor(255,255,255);
                if (dsty) cc = QColor(25,25,25);
                if (f_row_color || s_fast_dec_clr_msg_list) item->setBackground(cc);//item->setBackground(QColor(255,255,255));
                else
                {
                    if (s_list_ident == 2)
                    {
                        //f_mark_txqsy;//tx=0 qsy=1;
                        if      (f_mark_tx && !f_mark_txqsy) item->setBackground(c_mark_txt[3]);//255,210,210
                        else if (f_mark_qsy && f_mark_txqsy) item->setBackground(c_mark_txt[4]);//255,255,150
                        else item->setBackground(cc);//item->setBackground(QColor(255,255,255));
                    }
                    else
                    {
                        if (dsty) item->setBackground(QColor(45,45,45));
                        else item->setBackground(QColor(212,222,242));//rgb item->setBackground(QColor(212,222,242));
                    }
                }
                
                if (id_mark_otp_verif==1)//2.76sf verified
                {
                	if (dsty) item->setBackground(QColor(0,100,0));
                	else item->setBackground(QColor(200,250,200));
               	}
                else if (id_mark_otp_verif>1)//2.76sf invalid & not found
                {
                	if (dsty) item->setBackground(QColor(130,10,10));
                	else item->setBackground(QColor(250,200,200));
               	} 
               	              	
                qlsi.append(item);
            }
            if (k!=falltxt[0] && k!=falltxt[1])//2.73
            {
                if (k<list.count()-1) alltxt.append(txtin+"|");
                else alltxt.append(txtin);
            }
            k++;
        }
        if (f_show)
        {
            model.appendRow(qlsi);
            is_filter_dec = true;
        }
        if (id_vomsg==0) CountMsgInPeriod(true);//qDebug()<<id_voms<<f_otp_show_msg;

        alltxt.replace("\n"," ");// for JT6M jtms
        emit EmitRxAllTxt(alltxt);

        if (!ffopen)
        {
            if (s_list_ident == 1)//only from 1 list
            {
                bool emitudpdectxt = true;//2.76sf id_voms -> 0=normal msg, 1=$VERIFY$, 2=C0ALL.123456, 3=verified
                if (f_hide_c[7] && !f_show) emitudpdectxt = false;//qDebug()<<"EmitUdpDecTxt="<<emitudpdectxt;
                if (id_vomsg>0   && !f_show) emitudpdectxt = false;//2.76sf to DecText
                emit EmitRxStationInfo(list,0,emitudpdectxt,id_vomsg);//0->to psk reporter 1->to dx cluster, bool usefudpdectxt
                if (f_show && freply && id_vomsg==0) emit EmitRxTextForAutoSeq(list);//this signal for  2 list no conected  
            }
        }
        /*else if (s_list_ident == 1 && id_voms>0)//2.76.1 For the future from file open
        {
            bool emitudpdectxt = true;//2.76sf id_voms -> 0=normal msg, 1=$VERIFY$, 2=C0ALL.123456, 3=verified
            if (f_hide_c[7] && !f_show) emitudpdectxt = false;//qDebug()<<"EmitUdpDecTxt="<<emitudpdectxt;
            if (id_voms>0   && !f_show) emitudpdectxt = false;//2.76sf to DecText
            emit EmitRxStationInfo(list,100,emitudpdectxt,id_voms);//0->to psk reporter 1->to dx cluster, bool usefudpdectxt        	
       	}*/
        if (model.rowCount() > 250) model.removeRow(0);
        if (!list_rfresh_timer->isActive() && f_show) list_rfresh_timer->start(80);
    }
}
void DecodeList::Clear_List()
{
    for (int i = model.rowCount()-1; i >= 0; --i) model.removeRow(i);
    /*qDebug()<<"Clear_List()"<<s_mode<<show_filter_list<<s_list_ident;
       if ((s_mode==222 || s_mode==11 || s_mode==13|| s_mode==18) && show_filter_list) emit EmitLstNexColor(true);
       else emit EmitLstNexColor(false);*/
    f_bgcolor_clr_lst = true;
    emit EmitLstNexColor(false);
    if (s_list_ident == 1) emit EmitUdpDecClr();
    CountMsgInPeriod(false);
}
/*void DecodeList::SetDListMarkTextB4Qso(QStringList l)
{
	s_mark_txt
}*/
void DecodeList::SetTextMarkColors(QColor *c,int c_c,bool f_c3,bool f_c4)
{
    for (int i =0; i<c_c; ++i) c_mark_txt[i] = c[i];
    f_mark_tx = f_c3;
    f_mark_qsy = f_c4;
    viewport()->update(); //qDebug()<<"Color="<<s_list_ident<<c_mark_txt[6]<<c_mark_txt[5]<<f_mark_tx<<f_mark_qsy;
}
void DecodeList::SetDListMarkText(QStringList l,int r12,int ih,int j,int k,int im)
{
    if (s_mark_txt!=l)
    {
        s_mark_txt = l;//2.65 first this
        s_mark_r12_pos = r12;
        s_mark_hisc_pos = ih;
        s_mark_b4q_pos = j;
        s_mark_loc_pos = k;
        s_mark_myc_pos = im;

        list_b4qso.clear();
        int end = k;
        if (end > im) end = im;
        for (int i = j; i < end; ++i) list_b4qso.append(l.at(i));

        viewport()->update();
        //qDebug()<<l;
        //qDebug()<<s_mark_b4q_pos<<s_mark_loc_pos<<s_mark_myc_pos;
        //qDebug()<<list_b4qso<<list_b4qso.isEmpty();
        //qDebug()<<"--------------------------------";
    }
}
/*bool DecodeList::isSingeWord(QString s,int b,int e) const //separate word
{
    bool res = false;
    if (s.at(b)==' ' && s.at(e)==' ') res = true;
    return res;
}
bool DecodeList::DrawCode(QString s,QString w0,int b,int i) const
{
    bool res = true;
    if (s=="CQ" || s=="QRZ" || s=="RRR" || s=="RR73" || s=="RR73;" || s=="73")
    {
    	QString w = " "+w0+" "; //const QChar c = '=';//no coded characters ; % ^ = ~  RR73;
    	int e = b+s.count()+1;
    	if 		(i<1 && !isSingeWord(w,b,e)) res=false;
    	else if (i>0 &&  isSingeWord(w,b,e)) res=false;
    	//if 		(i<1 && !(w.at(b)==' ' && w.at(e)==' ')) res=false;
        //else if (i>0 &&  (w.at(b)==' ' && w.at(e)==' ')) res=false;
   	}
    return res;
}*/
void DecodeList::drawRow(QPainter *painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const
{
    QStyleOptionViewItem opt = option;

    if (selectionModel()->isSelected(index)) //(index == selectedIndexes()[0])//selectedIndexes()[0]
    {
        opt.font.setBold(true);
        opt.palette.setColor(QPalette::Highlight, model.itemFromIndex(index)->background().color());

        QColor cc = QColor(0,0,0);
        if (dsty) cc = QColor(255,255,255);
        opt.palette.setColor(QPalette::HighlightedText,cc);

        //8QString ss = model.data(model.index(index.row(),msg_column,QModelIndex()),Qt::DisplayRole).toString();
        //QToolTip::showText(QCursor::pos(),THvCty->FindCountry(ss));
        //QPoint pos = QPoint(opt.rect.x(),opt.rect.y());
        //QToolTip::showText(pos,THvCty->FindCountry(ss));
    }
    ///painter->setOpacity(0.5);
    QTreeView::drawRow(painter, opt, index);

    if (!s_mark_txt.isEmpty())
    {
        painter->setFont(opt.font);
        QRect r1;
        QFontMetrics fm(opt.font);
        //int start = 0;
        //int end = 0;
        //int begin = 0;
        //int nex_row = 0;
        QString str_find;
        int i_mark_ = 0;

        //QString str_row = model.data(model.index(index.row(),msg_column,QModelIndex()),Qt::DisplayRole).toString();
        QString str_row = model.item(index.row(),msg_column)->text();//2.48

        for (int i=0; i<s_mark_txt.count(); ++i)
        {
            str_find=s_mark_txt.at(i);

            /*if 		(i>=s_mark_loc_pos)	 i_mark_ = 5;
            else if (i>=s_mark_b4q_pos)	 i_mark_ = 4;
            else if (i>=s_mark_hisc_pos) i_mark_ = 3;
            else if (i>=s_mark_myc_pos)	 i_mark_ = 2;
            else if (i>=s_mark_r12_pos)	 i_mark_ = 1;*/
            if		(i>=s_mark_myc_pos)	 i_mark_ = 5;
            else if (i>=s_mark_loc_pos)	 i_mark_ = 4;
            else if (i>=s_mark_b4q_pos)	 i_mark_ = 3;
            else if (i>=s_mark_hisc_pos) i_mark_ = 2;
            else if (i>=s_mark_r12_pos)	 i_mark_ = 1;

            //if(str_find.isEmpty()) continue;//1.60= no need for the moment
            int start = 0;
            int end = 0;
            int begin = 0;
            int nex_row = 0;

try_again:
            end = str_row.indexOf(str_find,start);

            if (end!=-1)
            {
                /*bool draww = true;
                if (coded_mods) draww = DrawCode(str_find,str_row,end,i_mark_);*/
                bool draww = true;
                if (coded_mods)
                {
                    if (str_find=="CQ" || str_find=="QRZ" || str_find=="RRR" || str_find=="RR73" || str_find=="RR73;" || str_find=="73")
                    {
                        QString w0 = " "+str_row+" ";
                        int e1 = end+str_find.count()+1;
                        if 		(i_mark_<1 && !(w0.at(end)==' ' && w0.at(e1)==' ')) draww=false;
                        else if (i_mark_>0 &&  (w0.at(end)==' ' && w0.at(e1)==' ')) draww=false;
                        //if 		(i_mark_<1 && !isSingeWord(w0,end,e1)) draww=false;
                        //else if (i_mark_>0 &&  isSingeWord(w0,end,e1)) draww=false;
                    }
                }

                //qDebug()<<str_row<<i<<str_find<<s_mark_myc_pos<<start<<end<<i_mark_;
                QString s = str_row.mid(start-1,end-(start-1));
                int count_n = s.count("\n");

                if (count_n>0)
                {
                    for (int x=0; x<count_n; ++x)
                    {
                        nex_row+=fm.height();
                        begin =+ str_row.indexOf("\n",begin)+1;
                    }
                }

#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)//2.56
                int sta_1 = fm.horizontalAdvance(str_row.mid(begin,(end-begin)));
                int end_1 = fm.horizontalAdvance(str_find)+1;
#else
                int sta_1 = fm.width(str_row.mid(begin,(end-begin)));
                int end_1 = fm.width(str_find)+1;
#endif

                if (draww && (header()->sectionSize(msg_column) > sta_1+end_1+opt.rect.x()+5))
                {
                    r1=QRect(sta_1,nex_row,end_1,fm.height()-2);
                    r1.translate(header()->sectionPosition(msg_column)+opt.rect.x()+5,opt.rect.y()+1);

                    /*if 		(i_mark_ == 2) painter->fillRect(r1,c_mark_txt[5]); //2.56 my call:
                    else if (i_mark_ == 3) painter->fillRect(r1,c_mark_txt[6]); //2.63 his call:
                    else if (i_mark_ == 4) painter->fillRect(r1,c_mark_txt[1]);	// calls
                    else if (i_mark_ == 5) painter->fillRect(r1,c_mark_txt[2]);	// locs
                    else				   painter->fillRect(r1,c_mark_txt[0]);	// All others*/
                    if 		(i_mark_ == 2) painter->fillRect(r1,c_mark_txt[6]); //2.63 his call:
                    else if (i_mark_ == 3) painter->fillRect(r1,c_mark_txt[1]);	// calls
                    else if (i_mark_ == 4) painter->fillRect(r1,c_mark_txt[2]);	// locs
                    else if (i_mark_ == 5) painter->fillRect(r1,c_mark_txt[5]); //2.56 my call:
                    else				   painter->fillRect(r1,c_mark_txt[0]);	// All others

                    r1.translate(0 ,-1);

                    painter->drawText(r1,str_find);
                }
                start = end+1;
                goto try_again;
            }
        }
    }
}
void DecodeList::paintEvent(QPaintEvent *event)
{
    QTreeView::paintEvent(event);
    QPainter painter(viewport());
    painter.save();
    QColor cc = QColor(200,200,200);
    if (dsty) cc = QColor(80,80,80);
    painter.setPen(QPen(cc, 1, Qt::DashLine));
    int c_col;
    int istrt = 1;
    if (s_mode==0 || s_mode==12)//msk144
    {
        if (s_flag_two)//<<tr("Time")<<"T"<<tr("Width")<<"dB"<<"Rpt"<<"DF"<<tr("Message")<<tr("Dist")<<"Navg"<<"Bit err"<<"Eye"<<"Freq";
        {
            //if (s_mode==0  && !f_show_timec) istrt = 4;
            //if (s_mode==12 && !f_show_timec) istrt = 5;
            c_col=7;
            if (s_show_counc) c_col=8;//2.73 Dist Country
            if (s_show_distc) c_col=9;
            if (s_show_freqc) c_col++;
        }
        else
        {
            //if (s_mode==0  && !f_show_timec) istrt = 3;
            //if (s_mode==12 && !f_show_timec) istrt = 2;
            c_col=13;//2.73 Dist Country
        }
    }
    else if (s_mode==4 || s_mode==5 ) c_col=11;//iskat-a iskat-b
    else if (s_mode==10) c_col=10;//pi4
    else if (s_mode==11 || s_mode==13 || s_mode==18 || allq65)//<<tr("Time")<<"dB"<<"DT"<<"DF Of TX"<<tr("Message")<<tr("Dist")<<tr("Type")<<qdc<<"Freq";
    {
        if (!s_show_timec && !allq65) istrt = 2;
        if (s_flag_two)
        {
            c_col=6;
            if (!allq65)
            {
                c_col=5;
                if (s_show_counc) c_col=6;//2.73 Dist Country
                if (s_show_distc) c_col=7;
                if (s_show_freqc) c_col++;
            }
            else if (s_show_counc) c_col=7;
        }
        else c_col=10;//2.73 Dist Country
    }
    else if (s_mode==7 || s_mode==8 || s_mode==9)//<<tr("Time")<<tr("Sync")<<"dB"<<"DT"<<"DF"<<"W"<<tr("Message")<<"D Inf"<<"Flags"<<"Freq";
    {
        if (s_flag_two) c_col=8;
        else c_col=10;
    }
    else c_col=8;// jtms fsk441 fsk315 jt6m
    //if (s_list_ident==1) qDebug()<<"istrt="<<istrt<<c_col;
    for (int i = istrt; i<c_col; ++i) painter.drawLine(header()->sectionPosition(i),1,header()->sectionPosition(i),height());
    painter.restore();
    // triabva da e sled QTreeView::paintEvent(event); ina4e ne risuva pravilno
    // Tuk a ne na drawRow za da risuva kogato niama item v listata
}
static bool f_resize_event = false;
void DecodeList::StaticIgnoreResizeEvent(bool f)//1.30 stop scroling
{
    f_resize_event = f;
}
void DecodeList::resizeEvent(QResizeEvent * event)
{
    if (f_resize_event) event->ignore();
    else QTreeView::resizeEvent(event);
}
void DecodeList::ac_spot()
{
    QModelIndex index = selectionModel()->currentIndex();
    QStringList list;
    for (int i = 0; i < model.columnCount(); i++) list << index.sibling(index.row(),i).data().toString();
    // no crash ->model.item(index.row(),1)->text();//2.48    
	//2.76sf last -> 0=normal msg, 1=$VERIFY$, 2=C0ALL.123456, 3=verified
    emit EmitRxStationInfo(list,1,false,3);//0->to psk reporter 1->to dx clusters  2.34, bool usefudpdectxt
}
/*
void DecodeList::mouseDoubleClickEvent(QMouseEvent * event)
{
	//THvToolTip->setTimeToStop(300);
	QTreeView::mouseDoubleClickEvent(event);
}
*/
static bool f_click_on_call_show_cty = true;
void DecodeList::SetStaticClickOnCallShowCty(bool f)
{
    f_click_on_call_show_cty = f;
}
//#include <QToolTip>
void DecodeList::mousePressEvent(QMouseEvent * event)
{
    QTreeView::mousePressEvent(event);
    QModelIndex index = indexAt(event->pos());
    if (event->button() == Qt::RightButton && !(index.row() == -1)) m_spot->exec(QCursor::pos());

    if (f_click_on_call_show_cty && (s_mode==0 || s_mode==12 || s_mode==11 || s_mode==13 || s_mode==18 || allq65) &&
            event->button() == Qt::LeftButton && !(index.row() == -1))
    {
        QString ss = model.item(index.row(),msg_column)->text();//2.48
        QString call = "";
        emit EmitDetectTextInMsg(ss,call);
        QPoint post = event->globalPos();
        post += QPoint(+4,+6);
        THvToolTip->showText(post,THvCty->FindCountry(call,true),2500);//QToolTip::showText(post,THvCty->FindCountry(call),this,rect(),2500);
    }
}
/*
void DecodeList::mouseReleaseEvent(QMouseEvent * event)
{
    QModelIndex index = indexAt(event->pos());
    if (event->button() == Qt::LeftButton && !(index.row() == -1))
	{
		QString ss = index.sibling(index.row(),msg_column).data().toString();
		QPoint post = event->globalPos();
		post += QPoint(0,+5);
		//post += QPoint(0,-30);
		QToolTip::showText(post,THvCty->FindCountry(ss));
	}
	//if(event->type()==QEvent::ToolTip || event->type()==QEvent::ToolTipChange)
		event->ignore();
    qDebug()<<"e="<<event->type();
    //l_tooltip->setHidden(true);
    //l_tooltip->setText("");

    QTreeView::mouseReleaseEvent(event);
}
*/
/*void DecodeList::mouseMoveEvent (QMouseEvent  *event)
{
	QModelIndex index = indexAt(event->pos());
    if (!(index.row() == -1))
	{
		QString ss = index.sibling(index.row(),msg_column).data().toString();
		QPoint post = event->globalPos();
		post += QPoint(0,+5);
		//post += QPoint(0,-30);
		QToolTip::showText(post,THvCty->FindCountry(ss));
	}
    QTreeView::mouseMoveEvent(event);
}*/
/*
bool  DecodeList::event ( QEvent * e )
{
	if(e->type()==8 || e->type()==9 || e->type()==151)
		{
	//qDebug()<<"11mmmmmmm="<<e;
	e->ignore();
	}
qDebug()<<"11mmmmmmm="<<e;
	QTreeView::event(e);
}
*/
