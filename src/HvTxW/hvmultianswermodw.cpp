/* MSHV MultiAnswerModW
 * Copyright 2018 Hrisimir Hristov, LZ2HV
 * May be used under the terms of the GNU General Public License (GPL)
 */
#include "hvmultianswermodw.h"
#include "../config.h"
#include <QVBoxLayout>
#include <QPainter>
#include <QDateTime>
#include "../config_str_color.h"
//#include <QtGui>

ListA::ListA(int ident,bool f,QWidget *parent)
        : QTreeView(parent)
{
    dsty = f;
    setMinimumSize(50,50);
    this->setMaximumWidth(440);
    //setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Ignored);
    //qDebug()<<this->sizePolicy().horizontalPolicy()<<this->sizePolicy().verticalPolicy();

    setRootIsDecorated(false);

    setModel(&model);
    THvHeader = new QHeaderView(Qt::Horizontal);
    s_auto_sort = 0;
    s_order_1 = Qt::DescendingOrder;
    s_order_3 = Qt::DescendingOrder;
    model.SetValidSortColumns(1,3);
    if (ident==0) connect(THvHeader,SIGNAL(sortIndicatorChanged(int,Qt::SortOrder)),this,SLOT(SortChanged(int,Qt::SortOrder)));

    setHeader(THvHeader);
    setAllColumnsShowFocus(true);  // za da o4ertae delia row

    //setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    if (ident==0)//Queue   for LNow 5 slots no need scrollbar
        setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    setSelectionBehavior(QAbstractItemView::SelectRows);

    setDragDropMode(QAbstractItemView::NoDragDrop);//QAbstractItemView::InternalMove
    setDragEnabled (false);
    // setAcceptDrops(true);
    setVerticalScrollMode(QAbstractItemView::ScrollPerItem);// tova e za na mi6kata whella i dragdrop QAbstractItemView::ScrollPerPixel
    setAutoScroll(false); // s tova tarsi samo v 1 colona i za scroll pri drag

    QPalette PaletteL = palette();
    PaletteL.setColor(QPalette::Shadow, QColor(120,120,120));// zaradi bordera okantovaneto na lista
    setPalette(PaletteL);

    QStringList list_A;
    if (ident==0)//Queue
        list_A <<tr("Call")<<"dB"<<"Rx dB"<<tr("Dist")<<"Grid"<<"Freq"<<"Time"<<"IDrpt"<<"TTry"<<"GinTxT"<<"s"<<"e";
    else
        list_A <<tr("Call")<<"dB"<<"Rx dB"<<"Dist"<<"Grid"<<"Freq"<<"Time"<<"IDrpt"<<"TTry"<<"GinTxT"<<"s"<<"e";

    model.setHorizontalHeaderLabels(list_A);

    THvHeader->setSectionResizeMode(QHeaderView::Fixed);//qt5

    //THvHeader->setResizeMode(2, QHeaderView::Fixed);
    THvHeader->resizeSection(0, 50);//call
    THvHeader->resizeSection(1, 55);//Tx db
    THvHeader->resizeSection(2, 35);//Rx db
    THvHeader->resizeSection(3, 60);//Dist
    THvHeader->resizeSection(4, 50);//Grid

    if (ident==0)//Queue
        THvHeader->resizeSection(5, 55);//Freq
    else  // Now
        THvHeader->resizeSection(5, 60);//Freq

    THvHeader->resizeSection(6, 50);//Time
    THvHeader->resizeSection(7, 25);//Nrpt
    THvHeader->resizeSection(8, 40);//TTry
    THvHeader->resizeSection(9, 20);//GinTxT
    THvHeader->resizeSection(10, 20);//s
    THvHeader->resizeSection(11, 20);//e

    if (ident==0)//Queue
    {
        //HideSections();
        //THvHeader->resizeSection(0, 50);//call
        THvHeader->hideSection(1);//Tx db
        THvHeader->hideSection(2);//Rx db
        //THvHeader->hideSection(3);//Dist
        THvHeader->hideSection(4);//Grid
        THvHeader->hideSection(5);//Freq
        THvHeader->hideSection(6);//Time
        THvHeader->hideSection(7);//Nrpt
        THvHeader->hideSection(8);//TTry
        THvHeader->hideSection(9);//GinTxT
        THvHeader->hideSection(10);//s
        THvHeader->hideSection(11);//e
    }
    else
    {
        //THvHeader->resizeSection(0, 50);//call
        THvHeader->hideSection(1);//Tx db
        THvHeader->hideSection(2);//Rx db
        THvHeader->hideSection(3);//Dist
        THvHeader->hideSection(4);//Grid
        //THvHeader->hideSection(5);//Freq
        THvHeader->hideSection(6);//Time
        THvHeader->hideSection(7);//Nrpt
        THvHeader->hideSection(8);//TTry
        THvHeader->hideSection(9);//GinTxT
        THvHeader->hideSection(10);//s
        THvHeader->hideSection(11);//e
    }

    THvHeader->setSectionResizeMode(0, QHeaderView::Stretch);//qt5

    m_right_c = new QMenu(this);
    //QAction *ac_spot = new QAction(QPixmap(":pic/spot_dx.png"),"Spot Receiving Text", this);//"Spot DX Cluster" Spot Receiving Text

    QAction *ac_clrall = new QAction(QPixmap(":pic/clr_all.png"),tr("CLR All"), this);
    if (ident==0)//ls Queue
    {
        QAction *ac_clrsel = new QAction(QPixmap(":pic/clr_sel.png"),tr("CLR Selected"), this);
        m_right_c->addAction(ac_clrsel);
        m_right_c->addAction(ac_clrall);
        connect(ac_clrsel, SIGNAL(triggered()), this, SLOT(DeleteSel()));
    }
    if (ident==1)//ls Now
        m_right_c->addAction(ac_clrall);
    connect(ac_clrall, SIGNAL(triggered()), this, SIGNAL(EmitClear_List()));
    //connect(ac_clrall, SIGNAL(triggered()), this, SLOT(Clear_List()));

    if (ident==0)//ls Queue
    {
        setSelectionMode(QAbstractItemView::ExtendedSelection);
        setSortingEnabled(true);
        connect(&model,SIGNAL(EmitSortEnd()),this,SLOT(HideSections()));//2.57
    }
    else         //ls Now
    {
        setSelectionMode(QAbstractItemView::NoSelection);
    }
    //2.31 false for "\n" jt6m=6 jtms=1
    setUniformRowHeights(true);//2.30 fast wiev
    //qDebug()<<"BBBAutoSortDist="<<ident<<THvHeader->sortIndicatorOrder();
}
ListA::~ListA()
{}
void ListA::SortChanged(int col,Qt::SortOrder ord)
{
    if (col==1) s_order_1 = ord;
    if (col==3) s_order_3 = ord;
    //qDebug()<<s_order_1<<s_order_3;
}
void ListA::SetAutoSort(int i)
{
    s_auto_sort = i;
    if (s_auto_sort==1)//dist
    {
        sortByColumn(3,s_order_3);//move sort inicator
        model.sort(3,s_order_3);
        THvHeader->hideSection(1);
        THvHeader->showSection(3);
    }
    else if (s_auto_sort==2)//snr
    {
        sortByColumn(1,s_order_1);//move sort inicator
        model.sort(1,s_order_1);
        THvHeader->hideSection(3);
        THvHeader->showSection(1);
    }
    else//0 sort off
    {
        //sortByColumn(-1,s_order_1);//Note that not all models support this and may even crash in this case.
        THvHeader->hideSection(1);
        THvHeader->showSection(3);
    }   //qDebug()<<"SetAutoSort"<<s_auto_sort;
}
void ListA::HideSections()
{
    if (s_auto_sort==0 || s_auto_sort==1) THvHeader->hideSection(1);//Tx db
    if (s_auto_sort==2) THvHeader->hideSection(3);//Dist
    THvHeader->hideSection(2);//Rx db
    THvHeader->hideSection(4);//Grid
    THvHeader->hideSection(5);//Freq
    THvHeader->hideSection(6);//Time
    THvHeader->hideSection(7);//Nrpt
    THvHeader->hideSection(8);//TTry
    THvHeader->hideSection(9);//GinTxT
    THvHeader->hideSection(10);//s
    THvHeader->hideSection(11);//e //qDebug()<<"HideSections"<<s_auto_sort;
}
void ListA::SetFont(QFont f)
{
    setFont(f);
    THvHeader->setFont(f);
}
void ListA::AutoSortDist(int i)
{
    if (i==1) model.sort(i,s_order_1);
    if (i==3) model.sort(i,s_order_3); //model.sort(i,THvHeader->sortIndicatorOrder());
}
void ListA::SetItem(int row,int col,QString str)//2.57 remove new QStandardItem
{
    // end bug qt5.7 if setItem 0 row show hiden columns
    bool isHidenCol = THvHeader->isSectionHidden(col);
    QStandardItem *item = model.itemFromIndex(model.index(row,col));
    item->setText(str);

    if (col==3 || col==5 || col==1) item->setTextAlignment(Qt::AlignCenter);//freq and dist

    model.setItem(row,col,item);
    // end bug qt5.7 if setItem 0 row show hiden columns
    if (isHidenCol) THvHeader->hideSection(col);

    if 		(s_auto_sort==1 && THvHeader->sortIndicatorSection()==3 && col==3) AutoSortDist(3);//only LsQueue and distance
    else if (s_auto_sort==2 && THvHeader->sortIndicatorSection()==1 && col==1) AutoSortDist(1);
}
void ListA::InsertItem_hv(QStringList list)
{
    if (!list.isEmpty())
    {
        QList<QStandardItem *> qlsi;
        int k = 0;
        for (QStringList::iterator it =  list.begin(); it != list.end(); ++it)
        {
            QStandardItem *item = new QStandardItem(QString(*it));
            if (k==3 || k==5 || k==1) item->setTextAlignment(Qt::AlignCenter);//freq and dist
            item->setEditable(false);
            /*if (!f_row_color)
                item->setBackground(QColor(255,255,255));
            else
                item->setBackground(QColor(212,222,242)); //rgb*/
            qlsi.append(item);
            k++;
        }
        //model.insertRow(model.rowCount(), qlsi);
        model.appendRow(qlsi);

        emit ListCountChange(model.rowCount());

        if 		(s_auto_sort==1 && THvHeader->sortIndicatorSection()==3) AutoSortDist(3);//only LsQueue and distance
        else if (s_auto_sort==2 && THvHeader->sortIndicatorSection()==1) AutoSortDist(1);
    }
    //this->setCurrentIndex(model.index(model.rowCount(), 1, QModelIndex()));
}
int ListA::FindCallOrBaseCallRow(QString c)
{
    int row = -1;
    for (int i = 0; i < model.rowCount(); ++i)
    {
        QString call = model.item(i, 0)->text();
        QStringList	calls = call.split("/");
        for (int j = 0; j < calls.count(); ++j)
        {
            if (c==calls.at(j))
            {
                row = i;
                return row;//break;
            }
        }
    }
    return row;
}
void ListA::RemoveRow(int i)
{
    model.removeRow(i);
    emit ListCountChange(model.rowCount());
}
void ListA::RemoveRows(int pos,int cou)// no need to save temp all go to Now List
{
    model.removeRows(pos,cou);
    emit ListCountChange(model.rowCount());
}
void ListA::Clear_List()
{
    for (int i = model.rowCount()-1; i >= 0; --i)
        model.removeRow(i);
    emit ListCountChange(model.rowCount());
}
void ListA::DeleteSel()
{
    if (!selectionModel()->selection().empty())
    {
        //setAutoScroll(false); // za da ne skrolira pri iztrivane kam poslednia markiran
        QItemSelection selection( selectionModel()->selection() );

        QList<int> rows;
        foreach( const QModelIndex & index, selection.indexes() )
        {
            rows.append( index.row() );
        }
        //q_Sort( rows );
        std::sort(rows.begin(),rows.end());
        int prev = -1;
        for ( int i = rows.count() - 1; i >= 0; i -= 1 )
        {
            int current = rows[i];
            if ( current != prev )
            {
                model.removeRows(current,1);
                prev = current;
            }
        }
        //setAutoScroll(true);  // za da ne skrolira pri iztrivane kam poslednia markiran
        emit ListCountChange(model.rowCount());
    }
}
void ListA::paintEvent(QPaintEvent *event)
{
    QPainter painter(viewport());
    //QModelIndex index = model.index(ActiveIndex, 1, QModelIndex());
    //painter.fillRect(QRect(0,visualRect(index).y(),width(),visualRect(index).height()), ActiveRowBackg);
    QTreeView::paintEvent(event);

    painter.save();
    QColor cc = QColor(200,200,200);
    if (dsty) cc = QColor(80,80,80);
    painter.setPen(QPen(cc, 1, Qt::DashLine));

    //qDebug()<<model.columnCount();
    for (int i = 1; i<model.columnCount(); ++i)
        painter.drawLine(header()->sectionPosition(i),1,header()->sectionPosition(i),height());

    painter.restore();
}
void ListA::mousePressEvent(QMouseEvent * event)
{
    QTreeView::mousePressEvent(event);
    //if (event->button() == Qt::RightButton)
    //SendRightClick();

    //if (s_ident==0)//ls Queue
    //{
    QModelIndex index = indexAt(event->pos());
    if (event->button() == Qt::RightButton && !(index.row() == -1))
        m_right_c->exec(QCursor::pos());
    //}
    /*if (s_ident==1)//ls now
    {
        QModelIndex index = indexAt(event->pos());
        if (event->button() == Qt::RightButton && !(index.row() == -1))
            m_right_cn->exec(QCursor::pos());
    }*/
}

HvSpinLE::HvSpinLE(QWidget * parent )
        : QLineEdit(parent)
{}
HvSpinLE::~HvSpinLE()
{}
void HvSpinLE::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        emit EmitDoubleClick();
        return;
    }
    QLineEdit::mouseDoubleClickEvent(event);
}
HvSpinBoxMTP::HvSpinBoxMTP(QWidget * parent )
        : QSpinBox(parent)
{
    f_sb_mtp = false;
    HvSpinLE *le = new HvSpinLE();
    setLineEdit(le);
    le->setReadOnly(true);
    setContextMenuPolicy(Qt::NoContextMenu);
    setSingleStep(1);
    setValue(2);
    setRange(1,30);//2.73 old=10
    setPrefix(tr("Max Time")+":  ");
    setSuffix("  "+tr("min"));
    setStyleSheet("QSpinBox{selection-color:black;selection-background-color:white;}");
    setToolTip(tr("Double Click for changing to\nMax Time or Max Periods"));
    connect(le, SIGNAL(EmitDoubleClick()), this, SLOT(SetDoubleClick()));
}
HvSpinBoxMTP::~HvSpinBoxMTP()
{}
void HvSpinBoxMTP::SetDoubleClick()
{
    if (f_sb_mtp)
    {
        f_sb_mtp = false;
        setPrefix(tr("Max Time")+":  ");
        setSuffix("  "+tr("min"));
    }
    else
    {
        f_sb_mtp = true;
        setPrefix(tr("Max Periods")+":  ");
        setSuffix(" ");
    }
}
HvSpinBoxSlots::HvSpinBoxSlots(QWidget * parent )//2.76
        :
        QSpinBox(parent)
{
    s_msf = false;
}
HvSpinBoxSlots::~HvSpinBoxSlots()
{}
void HvSpinBoxSlots::SetMsfS5(bool f)
{
    s_msf = f;
}
int HvSpinBoxSlots::valueS()
{
    if (s_msf) return 5;
    else return value();
}
int HvSpinBoxSlots::maximumS()
{
    if (s_msf) return 5;
    else return maximum();
}

#define _CONT_NAME_
#include "../config_str_con.h"

#include "config_rpt_all.h"

#define MAXB_DB 60 //2.70 old=20
static int backup_db_pos_write;
static QString backup_db_[MAXB_DB+5][6+4];
static QString str_macros_mam_[7];//2.47 for win10
MultiAnswerModW::MultiAnswerModW(bool f,QWidget * parent )
        : QTabWidget(parent)
{
    dsty = f;
    id_mshf = 0;//2.76sf
    s_msf_ftmsg = false;//2.76sf Freetext msg;
    c_sf_rpt=0;
    c_sf_r73=0;
    g_block_stop_auto = false;//2.52
    f_auto_on = false;
    gg_frest = false;
    for (int i = 0; i<MAXB_DB; ++i)
    {
        for (int j = 0; j<6; ++j) backup_db_[i][j] = "";
    }
    backup_db_pos_write = 0;
    current_msg = "";
    s_man_adding = false;

    s_qrg = "237";
    s_last_bccall_tolog_excp = "NONE_";
    f_cfm73 = true;
    s_le_his_call = "";
    //this->setFixedHeight(160);
    //this->setFixedHeight(158);
    setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Ignored);// inportent 1.81
    //setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Ignored);
    this->setStyleSheet("QTabBar::tab{height:22px}");
    s_co_type = 0;
    s_co_id = 0;
    //lis_macros_mam_<<"%T %M %G4"<<"%T %M %R"<<"%T %M R%R"<<"%T %M RR73"<<"%T %M RR73"<<"CQ %M %G4";
    str_macros_mam_[0]="%T %M %G4";//2.47
    str_macros_mam_[1]="%T %M %R";
    str_macros_mam_[2]="%T %M R%R";
    str_macros_mam_[3]="%T %M RR73";
    str_macros_mam_[4]="%T %M RR73";
    str_macros_mam_[5]="CQ %M %G4";
    s_txsn_v2 = 1;
    QWidget *queue =new QWidget();
    LsQueue = new ListA(0,dsty);
    QVBoxLayout *V_lq = new QVBoxLayout();
    V_lq->setContentsMargins (0,0,0,0);
    V_lq->setSpacing(0);
    queue->setLayout(V_lq);
    V_lq->addWidget(LsQueue);

    QWidget *now_work =new QWidget();
    LsNow = new ListA(1,dsty);
    QVBoxLayout *V_ln = new QVBoxLayout();
    V_ln->setContentsMargins (0, 0, 0, 0);
    V_ln->setSpacing(0);
    now_work->setLayout(V_ln);
    V_ln->addWidget(LsNow);

    QWidget *settings =new QWidget();

    SBqueueLimit = new QSpinBox();
    SBqueueLimit->setSingleStep(1);
    SBqueueLimit->setValue(5);
    //SBqueueLimit->setRange(1,50);//1.78 100
    SBqueueLimit->setRange(0,50);//2.39 0
    SBqueueLimit->setPrefix(tr("Queue Limit")+":  ");
    //SBqueueLimit->setReadOnly(true);
    SBqueueLimit->findChild<QLineEdit*>()->setReadOnly(true);
    SBqueueLimit->setContextMenuPolicy(Qt::NoContextMenu);
    SBqueueLimit->setStyleSheet("QSpinBox{selection-color:black;selection-background-color:white;}");

    //SBslots = new QSpinBox();
    SBslots = new HvSpinBoxSlots();
    SBslots->setSingleStep(1);
    SBslots->setValue(1);
    SBslots->setRange(1,MAXSL); //max slots 5
    SBslots->setPrefix(tr("TX Slots")+":  ");
    //SBslots->setReadOnly(true);
    SBslots->findChild<QLineEdit*>()->setReadOnly(true);
    SBslots->setContextMenuPolicy(Qt::NoContextMenu);
    SBslots->setStyleSheet("QSpinBox{selection-color:black;selection-background-color:white;}");

    /*SBmaxtime = new QSpinBox();
    SBmaxtime->setSingleStep(1);
    SBmaxtime->setValue(2);
    SBmaxtime->setRange(1,10);
    SBmaxtime->setPrefix(tr("Max Time")+":  ");
    SBmaxtime->setSuffix("  "+tr("min"));
    SBmaxtime->findChild<QLineEdit*>()->setReadOnly(true);
    SBmaxtime->setStyleSheet("QSpinBox {selection-color: black; selection-background-color: white;}");*/
    period_time_sec = 15.0;
    SBmaxTP = new HvSpinBoxMTP();

    QVBoxLayout *V_sb = new QVBoxLayout();
    V_sb->setContentsMargins(0, 0, 0, 0);
    V_sb->setSpacing(1);
    V_sb->addWidget(SBqueueLimit);
    V_sb->addWidget(SBslots);
    V_sb->addWidget(SBmaxTP);

    Cbcqtype = new QComboBox();
    QStringList lst_m;  //2.68 "CQ MDX" = MADX
    /*lst_m<<"CQ"<<"CQ MDX"<<"CQ DX"<<"CQ UP"<<"CQ AF"<<"CQ AN"<<"CQ AS"<<"CQ EU"<<"CQ NA"<<"CQ OC"
    <<"CQ SA"<<"CQ JA"//<<"CQ 0"<<"CQ 1"<<"CQ 2"<<"CQ 3"<<"CQ 4"<<"CQ 5"<<"CQ 6"<<"CQ 7"<<"CQ 8"<<"CQ 9"<<"CQ MACQ"
    <<"CQ QRG"<<"CQ END"<<"TIME"<<"Free Msg";*/
    lst_m<<"CQ"<<"CQ MDX"<<"CQ DX"<<"CQ UP"
    <<"CQ IOTA"<<"CQ POTA"<<"CQ SOTA"<<"CQ BOTA"<<"CQ WWFF"//<<"CQ COTA"<<"CQ LOTA"<<"CQ AOTA"//<<"CQ GOTA"<<"CQ JOTA"<<"CQ YOTA"
    <<"CQ AF"<<"CQ AN"<<"CQ AS"<<"CQ EU"<<"CQ NA"<<"CQ OC"<<"CQ SA"<<"CQ JA"
    //<<"CQ 0"<<"CQ 1"<<"CQ 2"<<"CQ 3"<<"CQ 4"<<"CQ 5"<<"CQ 6"<<"CQ 7"<<"CQ 8"<<"CQ 9"<<"CQ MACQ"
    <<"CQ QRG"<<"CQ END"<<"TIME"<<"Free Msg";

    Cbcqtype->setFocusPolicy(Qt::NoFocus);
    int fHeight = 21;
    Cbcqtype->setFixedHeight(fHeight);

    LeFreeCQ = new HvLeWithSpace();
    //LeFreeCQ->setMaximumWidth(165);
    //LeFreeCQ->setMinimumWidth(165);
    //LeFreeCQ->setFixedWidth(165);
    LeFreeCQ->setFixedHeight(fHeight);
    LeFreeCQ->setMaxLength(26);
    //LeFreeCQ->setText("CQ");
    LeFreeCQ->setEnabled(false);
    //LeFreeCQ->setContentsMargins(1, 1, 0, 1);
    //LeFreeCQ->setFocusPolicy(Qt::TabFocus);
    //LeFreeCQ->setFocusPolicy(Qt::NoFocus);
    //Cbcqtype->setEditable(true);
    f_block_free_cq = false;
    pb_use_free_cq =new QPushButton(tr("USE"));
    QHBoxLayout *H_fmsg = new QHBoxLayout();
    H_fmsg->setContentsMargins(1, 0, 0, 0);
    H_fmsg->setSpacing(2);
    H_fmsg->addWidget(LeFreeCQ);
    H_fmsg->addWidget(pb_use_free_cq);
    pb_use_free_cq->setEnabled(false);
    pb_use_free_cq->setFixedSize(45,20);

    Cbcqtype->addItems(lst_m);
    Cbcqtype->setCurrentIndex(0);
    QString tip1 = "CQ MDX, MSHV "+tr("recommended\nidentification for DXpeditions");
    Cbcqtype->setItemData(1,tip1,Qt::ToolTipRole);
    tip1.prepend(tr("CQ Type")+":\n");
    Cbcqtype->setToolTip(tip1);

    CbcqtypeSF = new QComboBox();
    lst_m.clear();
    lst_m<<"CQ"<<"QSY To"<<"QRM QSY To"<<"Call From To"<<"QRX"<<"QRT"<<"Free Msg";
    CbcqtypeSF->addItems(lst_m);
    CbcqtypeSF->setCurrentIndex(0);
    CbcqtypeSF->setFixedHeight(fHeight);
    CbcqtypeSF->setHidden(true);

    cb_tx_cq_on_free_slot = new QCheckBox(tr("CQ on free slot"));
    cb_tx_cq_on_free_slot->setToolTip(tr("TX CQ on free slot"));//2.13 special msg
    cb_tx_cq_on_free_slot->setChecked(false);
    QHBoxLayout *H_l4 = new QHBoxLayout();
    H_l4->setContentsMargins(1, 0, 1, 0);
    H_l4->setSpacing(2);
    H_l4->addWidget(Cbcqtype);
    H_l4->addWidget(CbcqtypeSF);
    H_l4->addWidget(cb_tx_cq_on_free_slot);

    //cb_no_dupes = new QCheckBox(tr("No Dupe"));
    Cb_dupes = new QComboBox();
    Cb_dupes->setFixedHeight(fHeight);
    //Cb_dupes->setToolTip(tr("Dupe function\nMax QSOs per Call in Log"));
    //Cb_dupes->setToolTip(tr("Maximum QSOs per Call in the log"));
    //Cb_dupes->setToolTip(tr("Maximum QSOs per Call\nFiltered by Call, Band, Mode"));
    s_txt_mark_b = true;//2.66 inportet default all is marked HV 1.26 here and in hvtxtcolor.cpp
    s_txt_mark_m = true;//2.66 inportet default all is marked HV 1.26 here and in hvtxtcolor.cpp
    SetTextMark(s_txt_mark_b,s_txt_mark_m);
    QStringList lst_d;
    //lst_d<<tr("No Dupe")<<tr("No Dupe")<<tr("One Dupe")<<tr("Two Dupe");
    lst_d<<tr("No Limit")<<tr("1 QSO")<<tr("2 QSOs")<<tr("3 QSOs");
    Cb_dupes->addItems(lst_d);
    /*Cb_dupes->addItem("No Limit");
    Cb_dupes->addItem("1 QSO");
    Cb_dupes->addItem("2 QSOs");
    Cb_dupes->addItem("3 QSOs");*/

    cb_tx_sm = new QCheckBox("SM");//cb_tx_sm = new QCheckBox("SMsg");//2.10 special msg
    cb_tx_sm->setStyleSheet("QCheckBox{spacing:3px;}");
    cb_cont_ns = new QCheckBox(tr("CNS"));
    cb_cont_ns->setStyleSheet("QCheckBox{spacing:3px;}");
    cb_cont_ns->setHidden(true);
    /*cb_cont_ns = new QCheckBox();//cb_cont_ns->setToolTip("Continue non-stop");
    cb_cont_ns->setToolTip("CNS");   
    cb_cont_ns->setHidden(true);    
    cb_tx_sm_std = new QCheckBox("SMsg");//2.10 special msg
    cb_tx_sm_std->setHidden(true);
    QHBoxLayout *H_ns_smsg = new QHBoxLayout();
    H_ns_smsg->setContentsMargins(0, 0, 0, 0);
    H_ns_smsg->setSpacing(0);
    H_ns_smsg->addWidget(cb_tx_sm_std);
    H_ns_smsg->addWidget(cb_cont_ns);*/

    //cb_tx_sm->setToolTip("No TX special msgs");//2.10 special msg
    cb_tx_sm->setToolTip(tr("TX Special MSG:\nA2AA RR73; B2BB <C2CC> +05"));//2.10 special msg
    cb_tx_sm->setChecked(false);//default no emit
    //cb_tx_sm_std->setToolTip(tr("TX Special MSG:\nA2AA RR73; B2BB <C2CC> +05"));//2.10 special msg
    //cb_tx_sm_std->setChecked(false);//default no emit

    /*cb_a_sort = new QCheckBox(tr("Auto Sort"));
    cb_a_sort->setToolTip(tr("Auto sorting of Queued is activated\nif Dist column header is marked"));//2.10 special msg
    cb_a_sort->setChecked(false);//default no emit*/
    Cb_sort = new QComboBox();
    //Cb_sort->setToolTip(tr("Methods for automatic sorting of Queued list\nif Dist or dB column header is marked"));
    Cb_sort->setToolTip(tr("Auto sorting of Queued is activated\nif Dist or dB column header is marked"));
    Cb_sort->setFixedHeight(fHeight);
    lst_d.clear();
    lst_d<<tr("Sort Off")<<tr("Distance")<<tr("S/N (dB)");
    Cb_sort->addItems(lst_d);
    //Cb_sort->setItemData(1,tr("Auto sorting of Queued is activated\nif Dist column header is marked"),Qt::ToolTipRole);
    //Cb_sort->setItemData(2,tr("Auto sorting of Queued is activated\nif dB column header is marked"),Qt::ToolTipRole);

    cb_otp_mamd_key = new QCheckBox("OTP");
    cb_otp_mamd_key->setStyleSheet("QCheckBox{spacing:3px;}");
    //cb_otp_mamd_key->setToolTip(tr("TX on free slot\nOTP Key MA DXpedition"));
    cb_otp_mamd_key->setToolTip(tr("TX OTP Key on free slot\nMA DXpedition"));
    cb_otp_mamd_key->setChecked(false);
    connect(cb_otp_mamd_key,SIGNAL(toggled(bool)),this,SLOT(cb_otp_mamd_key_toggled()));
    QHBoxLayout *H_l3 = new QHBoxLayout();
    H_l3->setContentsMargins(0,0,0,0);
    H_l3->setSpacing(0);
    H_l3->addWidget(cb_cont_ns);
    H_l3->addWidget(cb_tx_sm);
    H_l3->addWidget(cb_otp_mamd_key);
    //f_vsf_ = false;//2.76sf for remove

    QVBoxLayout *V_cb = new QVBoxLayout();
    V_cb->setContentsMargins(0,0,0,0);
    V_cb->setSpacing(2);
    V_cb->addWidget(Cb_dupes);
    V_cb->addLayout(H_l3);
    //V_cb->addWidget(cb_tx_sm);
    //V_cb->addWidget(cb_cont_ns);
    //V_cb->addLayout(H_ns_smsg);
    V_cb->addWidget(Cb_sort);

    QHBoxLayout *H_l2 = new QHBoxLayout();
    H_l2->setContentsMargins(1,0,1,0);
    H_l2->setSpacing(4);
    H_l2->addLayout(V_sb);
    H_l2->addLayout(V_cb);

    pb_clr_queue = new QPushButton(tr("CLR QUEUE"));
    pb_clr_queue->setFixedHeight(fHeight);
    pb_clr_now = new QPushButton(tr("CLR NOW"));
    pb_clr_now->setFixedHeight(fHeight);
    QHBoxLayout *H_lb = new QHBoxLayout();
    H_lb->setContentsMargins(1,0,0,0);
    H_lb->setSpacing(4);
    H_lb->addWidget(pb_clr_queue);
    H_lb->addWidget(pb_clr_now);
    //H_lb->setAlignment(Qt::AlignCenter);
    //V_lq->addWidget(pb_clr_queue);
    //V_ln->addWidget(pb_clr_now);

    QVBoxLayout *V_ls = new QVBoxLayout();
    V_ls->setContentsMargins(2, 2, 2, 2);
    //V_ls->setContentsMargins(0, 0, 0, 0);
    V_ls->setSpacing(1);
    settings->setLayout(V_ls);

    V_ls->addLayout(H_l2);
    //V_ls->addWidget(Cbcqtype);
    V_ls->addLayout(H_l4);

    V_ls->addLayout(H_fmsg);
    //V_ls->setAlignment(LeFreeCQ,Qt::AlignCenter);
    V_ls->addLayout(H_lb);
    //V_ls->setAlignment(H_lb,Qt::AlignCenter);
    V_ls->setAlignment(Qt::AlignTop | Qt::AlignHCenter); //setAlignment(Box_dt,Qt::AlignRight);

    addTab(queue, tr("Queue")+" 0");
    addTab(now_work, tr("Now")+" 0");
    addTab(settings, tr("Settings"));//Settings
    setCurrentIndex(1);//2.11

    /*QString tabStyle = "QTabBar::tab:top:!selected{margin-top:2px;}"
    				   "QTabBar::tab:top:selected{margin-top:2px;}"
    				   "QTabBar::tab:left:!selected{margin-right:2px;}"
    				   "QTabBar::tab:left:selected{margin-right:2px;}";
    setStyleSheet(tabStyle);*/
    allq65 = false;
    s_mode = 2;
    info_dupe_qso = true;//2.51 default
    //s_txt_mark_b = true;//2.66 inportet default all is marked HV 1.26 here and in hvtxtcolor.cpp
    //s_txt_mark_m = true;//2.66 inportet default all is marked HV 1.26 here and in hvtxtcolor.cpp
    f_multi_answer_mod_std = false;
    s_start_qso_from_tx2 = false;

    list_macros<<""<<""<<""<<""<<""<<""<<""<<""<<""<<""<<""<<""<<""<<""<<""<<"";
    s_my_base_call = "_NONE_";
    //s_my_call_is_std = true;
    f_con_only_sdtc = false;
    my_call_have_slash = false;
    f_km_mi = false;//km

    f_tx_rx = false;
    s_count_nw_i3b = 0;
    for (int i = 0; i <MAXSL; ++i) t_list_i3b[i]=""; //max slots 5
    s_pos_nw_i3b = 0;
    gen_in_tx_time = "0";//1tx 0rx

    f_aseqmaxdist = false;//2.66
    is_LastTxRptCq = false;//2.66
    s_dist_points = -1;//2.66

    connect(pb_clr_queue, SIGNAL(clicked(bool)), this, SLOT(ClrQueue()));
    connect(pb_clr_now, SIGNAL(clicked(bool)), this, SLOT(ClrNow()));
    connect(LsQueue, SIGNAL(ListCountChange(int)), this, SLOT(LQueueCountChange(int)));
    connect(LsNow, SIGNAL(ListCountChange(int)), this, SLOT(LNowCountChange(int)));
    connect(Cbcqtype, SIGNAL(currentIndexChanged(QString)), this, SLOT(CbCQChanged(QString)));
    connect(CbcqtypeSF, SIGNAL(currentIndexChanged(QString)), this, SLOT(CbCQChanged(QString)));
    connect(LsQueue, SIGNAL(EmitClear_List()), this, SLOT(ClrQueue()));
    connect(LsNow, SIGNAL(EmitClear_List()), this, SLOT(ClrNow()));
    connect(SBslots, SIGNAL(valueChanged(int)), this, SLOT(SBslotsValueChanged(int)));
    connect(LeFreeCQ, SIGNAL(textChanged(QString)), this, SLOT(LeFreeCQtextChanged(QString)));
    connect(pb_use_free_cq, SIGNAL(clicked(bool)), this, SLOT(UseFreeCq()));
    connect(Cb_sort, SIGNAL(currentIndexChanged(int)), LsQueue, SLOT(SetAutoSort(int)));
    connect(cb_tx_cq_on_free_slot, SIGNAL(toggled(bool)), this, SLOT(CbTxCqOnFreeSlotChanged(bool)));

    //SetDefaultMacros();
}
MultiAnswerModW::~MultiAnswerModW()
{
    // destroy
}
void MultiAnswerModW::SetFont(QFont f)
{
    setFont(f);
    LsNow->SetFont(f);
    LsQueue->SetFont(f);
    SBqueueLimit->setFont(f);
    SBslots->setFont(f);
    SBmaxTP->setFont(f);
    Cbcqtype->setFont(f);
    CbcqtypeSF->setFont(f);
    //2.16 no need for the moment
    //for (int i = 0; i < Cbcqtype->count(); ++i)
    //Cbcqtype->setItemData(i,f,Qt::FontRole);
    //qDebug()<<Cbcqtype->count();
    //cb_no_dupes->setFont(f);
    Cb_dupes->setFont(f);
    cb_tx_sm->setFont(f);
    cb_otp_mamd_key->setFont(f);
    cb_cont_ns->setFont(f);
    Cb_sort->setFont(f);
    cb_tx_cq_on_free_slot->setFont(f);
    LeFreeCQ->setFont(f);
    pb_use_free_cq->setFont(f);
    pb_clr_queue->setFont(f);
    pb_clr_now->setFont(f);
    //LsQueue->setFixedHeight(height());
    //LsNow->setFixedHeight(height());
}
void MultiAnswerModW::SetReriodTime(float d)
{
    period_time_sec = d;
}
void MultiAnswerModW::SetSettings(QString s)
{
    QStringList ls = s.split("#");
    if (ls.count()<12) return;  //ls<<"0"<<"1"<<"1"<<"0"; //not possyble -> support for old ver
    SBqueueLimit->setValue(ls.at(0).toInt());
    SBslots->setValue(ls.at(1).toInt());
    SBmaxTP->setValue(ls.at(2).toInt());
    Cb_dupes->setCurrentIndex(ls.at(3).toInt());
    if (ls.at(4)=="1") cb_tx_sm->setChecked(true);
    else cb_tx_sm->setChecked(false);
    //if (ls.at(11)=="1") cb_tx_sm_std->setChecked(true);
    //else cb_tx_sm_std->setChecked(false);
    if (ls.at(9) =="0") LsQueue->s_order_1 = Qt::AscendingOrder;
    if (ls.at(10)=="0") LsQueue->s_order_3 = Qt::AscendingOrder;
    Cb_sort->setCurrentIndex(ls.at(5).toInt());
    if (ls.at(6)=="1") cb_tx_cq_on_free_slot->setChecked(true);
    else cb_tx_cq_on_free_slot->setChecked(false);
    if (ls.at(7)=="1") cb_cont_ns->setChecked(true);
    else cb_cont_ns->setChecked(false);
    if (ls.at(8)=="1") SBmaxTP->SetDoubleClick();
    if (ls.at(11)=="1") cb_otp_mamd_key->setChecked(true);
}
void MultiAnswerModW::LQueueCountChange(int n)
{
    setTabText(0,tr("Queue")+" "+QString("%1").arg(n));
}
void MultiAnswerModW::LNowCountChange(int n)
{
    setTabText(1,tr("Now")+" "+QString("%1").arg(n));
}
#include "../pfx_sfx.h"
bool MultiAnswerModW::is_pfx(QString s)
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
}
bool MultiAnswerModW::is_sfx(QString s)
{
    bool res = false;
    for (int i = 0; i < NZ2; i++)
    {
        if (s==(QString)sfx[i])
        {
            res = true;
            break;
        }
    }
    return res;
}
bool MultiAnswerModW::FormatTxIfSlash6CharCall(QString &s,int tx_id,bool &f_pfx_sfx)
{
    bool have_slash = false;
    f_pfx_sfx = false;
    if (s.contains("/"))
    {
        bool call_0 = false;
        bool call_1 = false;
        bool fpfx   = false;
        QStringList l_s = s.split("/"); //qDebug()<<l_my.count();
        int cls0 = l_s.at(0).count();
        int cls1 = l_s.at(1).count();
        if (cls0>6) cls0 = 6;
        if (cls1>6) cls1 = 6;

        if (is_pfx(l_s.at(0))) fpfx   = true;
        if (!is_pfx(l_s.at(0)) && THvQthLoc.isValidCallsign(l_s.at(0))) call_0 = true;
        if (!is_pfx(l_s.at(1)) && THvQthLoc.isValidCallsign(l_s.at(1))) call_1 = true;

        if (call_0 && call_1) s = l_s.at(1).mid(0,cls1);
        else if (call_0 && !call_1)
        {
            if (tx_id==0 || tx_id==6)
            {
                if (is_sfx(l_s.at(1)))
                {
                    have_slash= true;
                    s = l_s.at(0).mid(0,cls0)+"/"+l_s.at(1);
                }
                else
                    s = l_s.at(0).mid(0,cls0);
            }
            else
            {
                if (is_sfx(l_s.at(1))) f_pfx_sfx = true;
                have_slash= true;
                s = l_s.at(0).mid(0,cls0)+"/"+l_s.at(1).mid(0,cls1);
            }
        }
        else if (fpfx && !call_0 && call_1)
        {
            f_pfx_sfx = true;
            have_slash= true;
            s = l_s.at(0).mid(0,cls0)+"/"+l_s.at(1).mid(0,cls1);
        }
        else if (!call_0 && call_1)//1.74 w1/kh7z corr
        {
            //s = s;
            s = l_s.at(1).mid(0,cls1);
            //have_slash= true;
        }
        else
            s = l_s.at(0).mid(0,cls0);
    }
    else
    {
        int cs = s.count();
        if (cs>6) cs = 6;
        s = s.mid(0,cs);
    } //qDebug()<<s;

    return have_slash;
}
QString MultiAnswerModW::FindBase6CharCallRemAllSlash(QString str)// nay be need flag only 6
{
    int cres = str.count();
    if (cres>6) cres = 6;
    QString res = str.mid(0,cres);
    if (str.contains("/"))
    {
        QStringList list_str = str.split("/");
        int cls0 = list_str.at(0).count();
        int cls1 = list_str.at(1).count();
        if (cls0>6) cls0 = 6;
        if (cls1>6) cls1 = 6;

        if (list_str.count() >= 3) res = list_str.at(1).mid(0,cls1); //2.00//more then 2 shlashes SP9/LZ2HV/QRP
        else
        {// only 2 count
            bool call_0 = false;
            bool call_1 = false;
            if (!is_pfx(list_str.at(0)) && THvQthLoc.isValidCallsign(list_str.at(0))) call_0 = true;
            if (!is_pfx(list_str.at(1)) && THvQthLoc.isValidCallsign(list_str.at(1))) call_1 = true;

            if (call_0 && call_1)       //bin=11 int=3
                res = list_str.at(1).mid(0,cls1);
            else if (call_0 && !call_1) //bin=10 int=2
                res = list_str.at(0).mid(0,cls0);
            else if (!call_0 && call_1) //bin=01 int=1
                res = list_str.at(1).mid(0,cls1);
            else
                res = list_str.at(0).mid(0,cls0);   //bin=00 int=0
        }
    }
    return res;
}
QString MultiAnswerModW::FindBaseFullCallRemAllSlash(QString str)
{
    QString res = str;
    if (str.contains("/"))
    {
        QStringList list_str = str.split("/");

        if (list_str.count() >= 3) //more then 2 shlashes SP9/LZ2HV/QRP
            res = list_str.at(1);
        else
        {// only 2 count
            bool call_0 = false;
            bool call_1 = false;

            if (THvQthLoc.isValidCallsign(list_str.at(0)))//if (!is_pfx(list_str.at(0)) && isValidCallsign(list_str.at(0)))
                call_0 = true;
            if (THvQthLoc.isValidCallsign(list_str.at(1)))//if (!is_pfx(list_str.at(1)) && isValidCallsign(list_str.at(1)))
                call_1 = true;

            if (call_0 && call_1)    //bin=11 int=3
                res = list_str.at(1);//res = list_str.at(1).mid(0,6);
            else if (call_0 && !call_1) //bin=10 int=2
                res = list_str.at(0);//res = list_str.at(0).mid(0,6);
            else if (!call_0 && call_1) //bin=01 int=1
                res = list_str.at(1);//res = list_str.at(1).mid(0,6);
            else
                res = list_str.at(0);//res = list_str.at(0).mid(0,6);   //bin=00 int=0
        }
    }
    return res;
}
void MultiAnswerModW::SetHisCallChanged(QString s)
{
    s_le_his_call = s; //qDebug()<<"e="<<s_le_his_call;
}
void MultiAnswerModW::DetectTextInMsg(QString str, QString &hisCall_inmsg,QString &hisLoc_inmsg,QString &myCall_inmsg,
                                      QString &rpt_inmsg,QString &cont_r_inmsg,QString &rr73_inmsg,
                                      int &row_queue,int &row_now,QString &sn_inmsg,QString &arrl_exch_imsg)
{
    //str="LZ2HVG RR73; LZ2HVV <...> +06"; //fox exception
    //str="TU; <...> LZ2HV R 589 NY";      //RU exception
    //str="TU; LZ2HV <...> R 589 0041";    //RU exception
    //str="<...> LZ33MM +05";              //v2 exception
    //str="LZ33MM <...> +07";              //v2 exception
    QString hisCallFromLe = "";//inportent only one slot needed     nowcall,//"" <- corr for eu VHF
    if (f_multi_answer_mod_std)//no problem bez tova-> && s_id_con==3
    {
        if (LsNow->GetRowCount()>0) hisCallFromLe = LsNow->model.item(0,0)->text();// <- importent
    }
    else hisCallFromLe = s_le_his_call;// || f_multi_answer_mod is a problem for eu vhf contest
    //qDebug()<<"e="<<str<<hisCallFromLe;

    bool is_fox_msg_for_me_as2call_ft8 = false;
    //??? We're in RTTY contest and have "nextCall" queued up: send a "TU; ..." message
    QString str0 = str+"xxxx";
    if (str0.mid(0,4)=="TU; ") str.remove("TU; ");
    int cst0 = str0.count(); //bool tu0 = false;
    if (cst0>6)
    {
        if (str0.mid(cst0-7,3)==" TU") str.remove(cst0-7,3);//2.76.5 str.remove(" TU");
    }
    str.remove("<");
    str.remove(">");
    str.remove(";");

    bool some_digits = false;
    //QString his_call_23 = ""; //2.71 LZ2HV 4UNR KN23, CQ 095 4UNR
    int pos_loc_call_as_grig = 100; //2.13 exeprion call_as_grig PA70, PA70X, PA70XX

    QStringList ls_text = str.split(" ");
    for (int i = ls_text.count()-1; i>=0; --i)
    {
        //"TA2NC RR73 SP9HWY <...> +00"
        //<...> LZ33MM +00              v2 exception
        //LZ33MM <...> +00              v2 exception
        //"TU; LZ2HV <...> R 589 NY"    TU;<-removed
        //"TU; <...> LZ2HV R 589 0001"  TU;<-removed
        //str="<...> +07";              msk144 +sh
        //ft8+msk144 + fox exeption for call no exist and v2 non-standard calls
        if ((s_mode==0 || s_mode==11 || s_mode==13 || s_mode==18 || allq65) && i>0 && ls_text.at(i)=="...") hisCall_inmsg="...";//ft4
        if (rr73_inmsg.isEmpty() && ls_text.at(i)=="RR73") rr73_inmsg = "RR73";//2.75
        else if (i!=0 && i!=1 && hisLoc_inmsg.isEmpty() && THvQthLoc.isValidLocator(ls_text.at(i)))//2.75 (i!=0 && i!=1)
        {
            hisLoc_inmsg = ls_text.at(i);
            pos_loc_call_as_grig = i;//2.13 exeprion call_as_grig PA70, PA70X, PA70XX
        }
        else if (hisCall_inmsg.isEmpty() && THvQthLoc.isValidCallsign(ls_text.at(i)))
            /*2.75 not good idea click on my TX mycall=hiscall->&& FindBaseFullCallRemAllSlash(ls_text.at(i))!=s_my_base_call*/
        {
            hisCall_inmsg = ls_text.at(i);
            if (!hisCall_inmsg.isEmpty())
            {   //my be all is digits
                bool all_dig = true;
                for (int j = 0; j < hisCall_inmsg.count(); ++j)
                {
                    if (hisCall_inmsg.at(j).isLetter())
                    {
                        all_dig = false;
                        break;
                    }
                }
                if (all_dig)// all is digits
                {
                    int rptsn = hisCall_inmsg.toInt(); //qDebug()<<hisCall_inmsg<<hisCall_inmsg.count()<<rptsn;
                    if (rptsn>=520001 && rptsn<=592047)//new ->2047  sn_inmsg.isEmpty() &&
                    {
                        rpt_inmsg = hisCall_inmsg.mid(0,2);
                        sn_inmsg = hisCall_inmsg.mid(2,4);//2.39 old EU VHF -> hisCall_inmsg = hisCallFromLe;
                        hisCall_inmsg = "";//2.39 new EU VHF
                    }
                    else if (rptsn>=1000 && rptsn<=7999)//1000-4095  RU DX SN     hisCall_inmsg.count() == 4 &&
                    {
                        sn_inmsg = hisCall_inmsg;
                        hisCall_inmsg = "";
                    }
                    else if (rptsn>=529 && rptsn<=599)//hisCall_inmsg.count() == 3 &&
                    {
                        rpt_inmsg = hisCall_inmsg;
                        hisCall_inmsg = "";
                    }
                    else
                    {
                        some_digits = true;//100-10000000000 exception
                        hisCall_inmsg = "";
                    }
                }
                else//2.01 exception no call frm to 10A-32A ARRL Field Day category
                {
                    if (hisCall_inmsg.count() == 3)//10A-32F  t1>="A" and t1<="F"
                    {
                        int t1 = (int)hisCall_inmsg.at(2).toLatin1();
                        int dg = hisCall_inmsg.midRef(0,2).toInt();
                        if (t1>=(int)'A' && t1<=(int)'F' && dg>=10 && dg<=32)//no need 01a-nocall -> && hisCall_inmsg.mid(0,2).toInt()>0
                        {
                            arrl_exch_imsg = hisCall_inmsg;
                            hisCall_inmsg = "";
                        }
                    }
                }
            }
            if (i==0 && !hisCall_inmsg.isEmpty())//1.73 exception lz2hv/qrp 73
            {
                if (FindBaseFullCallRemAllSlash(hisCall_inmsg)==s_my_base_call)
                {
                    if (my_call_have_slash) hisCall_inmsg = hisCallFromLe;//no_pfx_sfx_have_slash problem is-> no pfx no sfx but have slash     && !hisCallFromLe.isEmpty()
                    else hisCall_inmsg = "";
                    myCall_inmsg = ls_text.at(i);//qDebug()<<"1.73 exception lz2hv/qrp 73"<<"myCall_inmsg="<<myCall_inmsg<<"hisCall_inmsg="<<hisCall_inmsg;
                }
            }//end 1.73 exception lz2hv/qrp 73
            if (s_mode==12)// rpt_ms hisCall exception //////////////////// new MSK144MS //////////////////
            {
                bool is_rpt_ms = false;
                for (int x = 0; x < COUNT_RPT_MS; ++x)
                {
                    QString r_rpt = "R"+rpt_ms_p[x];
                    if (r_rpt == hisCall_inmsg)
                    {
                        is_rpt_ms = true;
                        break;
                    }
                }
                if (is_rpt_ms)
                {
                    rpt_inmsg = hisCall_inmsg;//no needed
                    hisCall_inmsg = "";
                }
            }////////////////////end new MSK144MS //////////////////
            /*if (ls_text.count()==3 && i==2 && !hisCall_inmsg.isEmpty())//2.71 LZ2HV 4UNR KN23, CQ 095 4UNR
            {
            	his_call_23 = hisCall_inmsg;
            	hisCall_inmsg="";            
            }*/
        }
        else if (myCall_inmsg.isEmpty() &&  THvQthLoc.isValidCallsign(ls_text.at(i)) &&
                 FindBaseFullCallRemAllSlash(ls_text.at(i))==s_my_base_call)//1.70
        {
            myCall_inmsg = ls_text.at(i);
            if ((s_mode==11 || s_mode==13 || s_mode==18 || allq65) && i>0) is_fox_msg_for_me_as2call_ft8 = true;//ft8 ft4 + fox exeption my call no first 1.69
        }
        else if (cont_r_inmsg.isEmpty() && ls_text.at(i)=="R") cont_r_inmsg = ls_text.at(i);//msk144 ft8 contest mode
        else if (/*i>1 && i<4 && */ls_text.at(i).count()==2)//ARRL Field Day category 1A-9F  t1>="A" and t1<="F"
        {
            QString s0 = ls_text.at(i);
            int t1 = (int)s0.at(1).toLatin1();
            int dg = s0.midRef(0,1).toInt();
            if (t1>=(int)'A' && t1<=(int)'F' && dg>=1 && dg<=9) arrl_exch_imsg = s0;//(int)s0.at(0).toLatin1()>0
        }
        else if (ls_text.at(i).count()==4)//RU DX SN 0001-0999
        {
            int rptsn = ls_text.at(i).toInt();
            if (rptsn>=1 && rptsn<=999) sn_inmsg = ls_text.at(i);
        }
        /*else if (ls_text.at(i)=="DE" && myCall_inmsg.isEmpty())//exeption DE SP9HWY/QRP R-09 and DE SP9HWY/QRP 73
             myCall_inmsg = list_macros.at(0);*/
    }
    /*if (!his_call_23.isEmpty() && hisCall_inmsg.isEmpty())//2.71 LZ2HV 4UNR KN23, CQ 095 4UNR
    {
    	hisCall_inmsg = his_call_23;	
    	his_call_23 = "";	
    }*/
    //qDebug().nospace()<<"hisCall="<<hisCall_inmsg<<" hisLoc="<<hisLoc_inmsg<<" myCall="<<myCall_inmsg<<" pos_loc="<<pos_loc_call_as_grig;
    /*if (pos_loc_call_as_grig==1)//2.14 exeption fox to me as pos=1 call msg //2.13 exeprion call_as_grig PA70, PA70X, PA70XX
       {
           hisCall_inmsg = hisLoc_inmsg;
           hisLoc_inmsg = "";
       }
       else if (pos_loc_call_as_grig==0)//2.14 exeption fox to me as pos=0 call msg
       {
           if (FindBaseFullCallRemAllSlash(hisLoc_inmsg)==s_my_base_call) myCall_inmsg = hisLoc_inmsg;
           hisLoc_inmsg = "";
       }
       else*/
    if (pos_loc_call_as_grig==3 && ls_text.count()==5)//2.75
    {   //SMsg->"KK2HV RR73; LZ2HV <RP79GD> +06", LZ2HV <...> R 589 NY", <...> LZ2HV R 589 0001", WA9XYZ KA1ABC R 16A EMA
        //<RP9DD> <RP79GD> R 590057 KN23SF, <RP9DD> <RP79GD> 590057 KN23SF
        if (FindBaseFullCallRemAllSlash(hisCall_inmsg)==s_my_base_call)
        {
            myCall_inmsg = hisCall_inmsg;//pos->hisCall_inmsg>0, pos myCall_inmsg is >0, no need->"&& i>0"
            if (s_mode==11 || s_mode==13 || s_mode==18 || allq65) is_fox_msg_for_me_as2call_ft8 = true;
        }
        hisCall_inmsg = hisLoc_inmsg;//Double click or RX fox=call_as_grig
        hisLoc_inmsg = "";
    }
    //2.13 END exeprion call_as_grig PA70, PA70X, PA70XX

    //1.73 exeption DE SP9HWY/QRP R-09 and DE SP9HWY/QRP 73
    if (row_queue==-1)// -1=ident find mam -2=no find mam
    {
        QString his_base_call = FindBaseFullCallRemAllSlash(hisCall_inmsg);
        row_queue = LsQueue->FindCallOrBaseCallRow(his_base_call);
        row_now   = LsNow->FindCallOrBaseCallRow(his_base_call);//qDebug()<<"MAMP== FIND"<<hisCall_inmsg<<row_queue<<row_now;
    }
    if (ls_text.at(0)=="DE" && myCall_inmsg.isEmpty() && !hisCall_inmsg.isEmpty())
    {
        if (row_queue==-2)// -1=ident find mam -2=no find mam
        {
            if (FindBaseFullCallRemAllSlash(hisCall_inmsg)==FindBaseFullCallRemAllSlash(hisCallFromLe))
                myCall_inmsg = list_macros.at(0);
            //qDebug()<<"NORM== exeption DE SP9HWY/QRP R-09 and DE SP9HWY/QRP 73";
        }
        else if (row_queue>-1 || row_now>-1)
        {
            myCall_inmsg = list_macros.at(0);
            //qDebug()<<"MAMP== exeption DE SP9HWY/QRP R-09 and DE SP9HWY/QRP 73";
        }
    }
    //qDebug()<<"EEE xeption DE SP9HWY/QRP R-09 and DE SP9HWY/QRP 73"<<row_queue<<row_now;
    //end 1.73 exeption DE SP9HWY/QRP R-09 and DE SP9HWY/QRP 73

    //qDebug()<<"1111="<<rpt_inmsg<<str<<myCall_inmsg<<hisCall_inmsg;
    QString trpt = ls_text.at(ls_text.count()-1); //2.71 LZ2HV 4UNR kn23, CQ 095 4UNR
    //old if (hisLoc_inmsg.isEmpty() && trpt!=hisCall_inmsg && trpt!=myCall_inmsg && !some_digits/* && his_call_23.isEmpty()*/)//1.70 for slash only calls
    if (hisLoc_inmsg.isEmpty() && rr73_inmsg.isEmpty() && trpt!=hisCall_inmsg && trpt!=myCall_inmsg && !some_digits)//2.75
    {   //rpt= +12,-12,R+12,R-12,R26,26,73,RRR,599,59
        bool arrl_sec = false;
        if (trpt!="RRR" && trpt!="73" && trpt.count()>1 && trpt.count()<4)
        {
            arrl_sec = true;
            for (int j = 0; j < trpt.count(); ++j)
            {
                if (trpt.at(j).isDigit())
                {
                    arrl_sec = false;
                    break;
                }
            }
            if (arrl_sec)//arrl section
            {
                if (arrl_exch_imsg.isEmpty()) arrl_exch_imsg = trpt;//arrl section Roundup
                else arrl_exch_imsg.append(" "+trpt);//arrl section Field Day
            }
        }
        if (!arrl_sec && rpt_inmsg.isEmpty()) rpt_inmsg = trpt;
        //if (tu0) rr73_inmsg = "RR73";
    }
    if (s_mode==0 || s_mode==11 || s_mode==13 || s_mode==18 || allq65)//ft4 ft8+msk144 + fox exeption for call no exist and v2 non-standard calls
    {	//"TA2NC RR73 SP9HWY <...> +00", <...> LZ2HV +00, LZ33MM <...> +00   v2 exception
        if (is_fox_msg_for_me_as2call_ft8)
        {
            rpt_inmsg = ls_text.at(ls_text.count()-1);
            rr73_inmsg = "";//RR73 not for me
        }
        if (hisCall_inmsg=="...") hisCall_inmsg="";//1.69 big exeption for fox msg
    }
    //2.75 "LZ2HV/B KN23" "W0A/B KN23"
    if (ls_text.count() == 2/*&& THvQthLoc.isValidCallsign(ls_text.at(0))*/)
    {
        int idx = ls_text.at(0).indexOf("/B");
        if (idx>2 && THvQthLoc.isValidLocator(hisCall_inmsg))
        {
            hisLoc_inmsg = hisCall_inmsg;
            hisCall_inmsg = ls_text.at(0);
        }
    }
    /*qDebug().nospace()<<"hisCall="<<hisCall_inmsg<<" hisLoc="<<hisLoc_inmsg<<" myCall="<<myCall_inmsg<<
    " rpt="<<rpt_inmsg<<" contest_mode="<<cont_r_inmsg<<" rr73="<<rr73_inmsg<<" sn_inmsg="<<sn_inmsg<<
    " arrl_exch_imsg="<<arrl_exch_imsg;*/
}
QString MultiAnswerModW::format_rpt_ma(QString s)
{
    QString ss;
    if (s_mode==0 || s_mode==11 || s_mode==13 || s_mode==18 || allq65)//msk144 ft8 ft4
    {
        if (f_multi_answer_mod_std && s_co_type==3)////52-59 for  v2
        {
            if (s[0].toLatin1()=='-' || s[0].toLatin1()=='+')
                ss="59";
            else
                ss=s;
        }
        else if (f_multi_answer_mod_std && s_co_type==5)//529-599
        {
            if (s[0].toLatin1()=='-' || s[0].toLatin1()=='+')
                ss="599";
            else
                ss=s;
        }
        else
        {
            if (s[0].toLatin1()!='-' && s[0].toLatin1()!='+')
            {
                s="+"+s;
            }
            if (s.toInt()==0)
                s.replace('-','+');
            ss=s[0];
            ss.append(QString("%1").arg(abs(s.toInt()),2,10,QChar('0')));
        }
    }
    else
        ss=s;
    return ss;
}
void MultiAnswerModW::SetDistUnit(bool f_)
{
    f_km_mi = f_;
}
QString MultiAnswerModW::CalcDistance(QString h, bool fkmmi)
{
    QString dist;
    QString c_test_loc = THvQthLoc.CorrectLocator(h);
    QString c_my_loc = THvQthLoc.CorrectLocator(list_macros.at(1));
    double dlong1 = THvQthLoc.getLon(c_my_loc);
    double dlat1  = THvQthLoc.getLat(c_my_loc);
    double dlong2 = THvQthLoc.getLon(c_test_loc);
    double dlat2 = THvQthLoc.getLat(c_test_loc);
    int dist_km = THvQthLoc.getDistanceKilometres(dlong1,dlat1,dlong2,dlat2);
    /*if (!fkmmi) dist = QString("%1").arg(dist_km,5,10,QChar(' '));//for sorting
    else dist = QString("%1").arg(THvQthLoc.getDistanceMilles(dlong1,dlat1,dlong2,dlat2),4,10,QChar(' '));//for sorting*/
    if (!fkmmi) dist = QString("%1").arg(dist_km);//2.70 no need for sorting ->' '
    else dist = QString("%1").arg(THvQthLoc.getDistanceMilles(dlong1,dlat1,dlong2,dlat2));//2.70 no need for sorting ->' '
    return dist;
}
bool MultiAnswerModW::isStandardCall(QString w)//,bool nobc
{
    //static QRegularExpression standard_call_re {
    //  R"(
    //      ^\s*				# optional leading spaces
    //      ( [A-Z]{0,2} | [A-Z][0-9] | [0-9][A-Z] )  # part 1
    //      ( [0-9][A-Z]{0,3} )                       # part 2
    //      (/R | /P)?			# optional suffix
    //      \s*$				# optional trailing spaces
    //  )", QRegularExpression::CaseInsensitiveOption | QRegularExpression::ExtendedPatternSyntaxOption};
    //return standard_call_re.match (w).hasMatch ();
    QRegExp rx("^\\s*([A-Z]{0,2}|[A-Z][0-9]|[0-9][A-Z])([0-9][A-Z]{0,3})(/R|/P)?\\s*$");
    rx.setCaseSensitivity(Qt::CaseInsensitive);
    bool res0 = rx.exactMatch(w); //qDebug()<<w<<res0;
    if (id_mshf==2 && !res0)//2.76.2    && !nobc
    {
        QString bc = FindBaseFullCallRemAllSlash(w);
        res0 = rx.exactMatch(bc);
    } //printf(" id_mshf= %d Call= %s isStd= %s\n",id_mshf,qPrintable(w),res0 ? "true" : "false");
    return res0; //return pomAll.isStandardCall(w);
}
void MultiAnswerModW::isStandardCalls(QString c1,QString c2,bool &fc1,bool &fc2,uint8_t &noQSO)
{
    fc1 = isStandardCall(c1);
    fc2 = isStandardCall(c2); //qDebug()<<"-------->"<<"MyC="<<fc1<<c1<<"HisC="<<fc2<<c2;
    noQSO = 0;
    if (s_co_type==0 || s_co_id==16 || s_co_id==17)//2.74 || s_co_type==6 s_co_id==16 ?
    {
        if (id_mshf!=2)
        {
            int c1c = c1.count();
            int c2c = c2.count();
            QString c1end = c1.mid(c1c-2,2);
            QString c2end = c2.mid(c2c-2,2);
            if (fc1 && fc2)// lz2hv/r  (lz2020hv lz2hv/p)
            {
                if ((c1end=="/P" && c2end=="/R") || (c1end=="/R" && c2end=="/P"))
                {
                    fc1 = false;
                    fc2 = false;
                }
            }
            else if (fc1 && !fc2) //LZ2HV LZ2HV/P  NOSTD->  LZ2HV/QRP LZ2020HV LZ2020HV/P, R5WM/R LZ2HV/QRP, R5WM/R LZ2222HV, R5WM LZ2HV/QRP
            {
                if (c1end=="/P" || c1end=="/R")
                {
                    QString bc2 = FindBaseFullCallRemAllSlash(c2);
                    if (!isStandardCall(bc2)) noQSO = 1;//lz2hv/p lz22hv
                    else noQSO = 2; //lz2hv/p lz2hv/mm
                    fc1 = false;
                }
            }
            else if (!fc1 && fc2) // LZ2HV/QRP R5WM/R, LZ2222HV R5WM/R, LZ2HV/QRP R5WM
            {
                if (c2end=="/P" || c2end=="/R")
                {
                    QString bc1 = FindBaseFullCallRemAllSlash(c1);
                    if (!isStandardCall(bc1)) noQSO = 3;//lz22hv lz2hv/p
                    else  noQSO = 4;//lz2hv/mm lz2hv/p
                    fc2 = false;
                }
            }
            else if (!fc1 && !fc2)// NOSTD->  LZ2HV/QRP LZ2020HV LZ2020HV/P
            {
                QString bc1 = FindBaseFullCallRemAllSlash(c1);
                QString bc2 = FindBaseFullCallRemAllSlash(c2);
                if 		( isStandardCall(bc1) &&  isStandardCall(bc2)) noQSO = 0;//lz2hv/mm lz3hv/gg
                else if (!isStandardCall(bc1) &&  isStandardCall(bc2)) noQSO = 5;//lz22hv   lz2hv/gg
                else if ( isStandardCall(bc1) && !isStandardCall(bc2)) noQSO = 6;//lz2hv/gg	lz22hv
                else if (!isStandardCall(bc1) && !isStandardCall(bc2)) noQSO = 7;//lz22hv	lz33hv
                //else noQSO = true;
            }
        }
        if (id_mshf>0)//2.76.2
        {
            if (id_mshf==2)//2.76.2 me_sfox his Basecall need to be std // && !fc2
            {
                QString bc2 = FindBaseFullCallRemAllSlash(c2);
                if (!isStandardCall(bc2)) noQSO = 100;
                else noQSO = 0;
            }
            if (id_mshf==1)//2.76.2 me_shound my Basecall need to be std // && !fc1
            {
                QString bc1 = FindBaseFullCallRemAllSlash(c1);
                if (!isStandardCall(bc1)) noQSO = 100;
                else noQSO = 0;
            }
        }
    }
    else
    {// if (!fc1 && !fc2) // for contests no use new logic eventual exception  //2.39 old EU VHF
        if (!fc1 && !fc2 && s_co_type!=3) noQSO = 100; //2.39 for new EU VHF
    } //qDebug()<<"MyC="<<fc1<<c1<<"HisC="<<fc2<<c2<<"BlockQSO="<<noQSO;
    //printf("id_mshf= %d\nMyCall= %s isStd= %s\nHiCall= %s isStd= %s\nnoQSO= %s\n--------\n",id_mshf,qPrintable(c1),fc1 ? "true" : "false",qPrintable(c2),fc2 ? "true" : "false",noQSO ? "true" : "false");
}
void MultiAnswerModW::SetTxSnV2(int v)
{
    s_txsn_v2 = v; //qDebug()<<"s_txsn_v2"<<s_txsn_v2;
}
QString MultiAnswerModW::MakeSMsg(QString hc0,QString hc1,QString mc,QString rpt)//2.76.2
{
    QString res;
    QString hbc0 = FindBaseFullCallRemAllSlash(hc0);
    QString hbc1 = FindBaseFullCallRemAllSlash(hc1);
    QString mbc  = mc;
    QString rp0  = rpt;
    if (rpt.isEmpty()) rp0 = "-10";
    if (id_mshf==2) mbc = FindBaseFullCallRemAllSlash(mc);
    res = hbc0+" RR73; "+hbc1+" <"+mbc+"> "+rp0;
    return res;
}
QString MultiAnswerModW::DecodeMacros(int row, QString id)//row from listNow id msg id
{
    int tx_id = id.toInt();//"Call"<<"dB"<<"Rx dB"<<"Dist"<<"Grid"<<"Freq"<<"Time"<<"IDrpt"<<"TTry"<<"GinTxT";
    QString str_out = str_macros_mam_[tx_id];
    //V2
    //9 click to someone else             %H %M %G4   //2.22
    //0 SP9HWY LZ2HV +00                  %H %M %R
    //1 SP9HWY LZ2HV R+00                 %H %M R%R   5b4aif
    //2 SP9HWY LZ2HV RR73                 %H %M RR73
    //3 TA2NC RR73; SP9HWY <LZ2HV> +00
    //4 CQ +??+ LZ2HV KN23				  CQ %M %G4
    QString my_call = list_macros.at(0);
    QString his_call = LsNow->model.item(row,0)->text();
    //bool his_call_is_std = true;//isStandardCall(his_call);
    bool my_call_is_std,his_call_is_std;
    uint8_t noQSO;
    isStandardCalls(my_call,his_call,my_call_is_std,his_call_is_std,noQSO);

    bool f_sf4r = false;//2.76sf
    //bool f_sfft = false;//2.76sf Free Text msg
    uint8_t nslot = SBslots->valueS();//2.76sf
    if (id_mshf==2)
    {
        nslot = 4;
        if (c_sf_rpt>=4) f_sf4r = true;//2.76sf
        if (s_msf_ftmsg) f_sf4r = true;//CbcqtypeSF->currentText()=="Free Msg" && !f_block_free_cq
    }

    if (tx_id==0) //click to someone else
    {
        if (!his_call_is_std && !my_call_is_std && noQSO!=100) str_out ="<"+his_call+"> "+my_call;
        else if (!his_call_is_std) his_call = "<"+his_call+">";
        else if (!my_call_is_std) str_out ="<"+his_call+"> "+my_call;
    }
    else if (tx_id==1 || tx_id==2) //1=rpt 2=R+rpt
    {
        if (s_co_type==3)//2.39
        {
            //his_call = "<"+FindBaseFullCallRemAllSlash(his_call)+">";
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
                his_call = FindBaseFullCallRemAllSlash(his_call);
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
        c_sf_rpt++;
    }
    else if (tx_id==3 || tx_id==4) //3=RR73    id=="4" <- importent need to be here
    {
        QString next_queued = "";
        QString next_queued_tx_id = "";
        if (LsQueue->GetRowCount()>0)
        {
            next_queued = LsQueue->model.item(0,0)->text();
            next_queued_tx_id = LsQueue->model.item(0,7)->text();//2.69
        } //qDebug()<<next_queued<<next_queued_tx_id;
        bool f_tx_sm = false;//2.70
        if (!f_multi_answer_mod_std && (cb_tx_sm->isChecked() || id_mshf==2)) f_tx_sm = true; //2.70 //2.76
        //if (f_multi_answer_mod_std && cb_tx_sm_std->isChecked() && s_co_type==0) f_tx_sm = true; //2.71
        if (f_tx_sm && !f_tx_rx && !next_queued.isEmpty() && his_call_is_std && //2.70 added f_tx_sm, SBslots->valueS()>1 &&
                isStandardCall(next_queued) && LsNow->GetRowCount() >= nslot/*SBslots->valueS()-sfoxslot*/ &&
                (next_queued_tx_id == "1" || next_queued_tx_id == "2") && !f_sf4r) //not a good idea-> || next_queued_tx_id == "2"
        {
            bool exist = false;
            for (int i = 0; i<MAXSL; ++i)//max slots 5
            {
                if (t_list_i3b[i]==his_call)
                {
                    exist = true;
                    break;
                }
            }
            if (!exist)
            {
                t_list_i3b[s_count_nw_i3b]=his_call; //qDebug()<<"1-t_list_i3b Need Only ?="<<his_call;
                s_count_nw_i3b++;
                RefreshLists(s_count_nw_i3b);
            }
            int nw_i3b = LsNow->GetRowCount()-s_count_nw_i3b+s_pos_nw_i3b; //qDebug()<<"2------"<<row<<id<<nw_i3b<<LsNow->GetRowCount();
            //QString rptt = LsNow->model.item(nw_i3b,1)->text();
            //if (rptt.isEmpty()) rptt = "-10";//2.69 protection
            //str_out = his_call+" RR73; "+LsNow->model.item(nw_i3b,0)->text()+" <"+my_call+"> "+rptt;//2.76.2 stop
            str_out = MakeSMsg(his_call,LsNow->model.item(nw_i3b,0)->text(),my_call,LsNow->model.item(nw_i3b,1)->text());

            //qDebug()<<"2="<<s_count_nw_i3b<<LsNow->GetRowCount()<<nw_i3b<<next_queued<<str_out;
            s_pos_nw_i3b++;
            c_sf_rpt++;
        }//else if (LsNow->GetRowCount()-s_count_nw_i3b+s_pos_nw_i3b<LsNow->GetRowCount())
        else if (s_pos_nw_i3b-s_count_nw_i3b<0 && !f_sf4r && his_call_is_std)//2.76.2 added->his_call_is_std /*&& isStandardCall(next_queued)*/
        {
            int nw_i3b = LsNow->GetRowCount()-s_count_nw_i3b+s_pos_nw_i3b;//qDebug()<<"3000------"<<his_call<<his_call_is_std<<next_queued<<isStandardCall(next_queued);
            //QString rptt = LsNow->model.item(nw_i3b,1)->text();
            //if (rptt.isEmpty()) rptt = "-10";//2.69 protection
            //str_out = his_call+" RR73; "+LsNow->model.item(nw_i3b,0)->text()+" <"+my_call+"> "+rptt;//2.76.2 stop
            str_out = MakeSMsg(his_call,LsNow->model.item(nw_i3b,0)->text(),my_call,LsNow->model.item(nw_i3b,1)->text());

            //qDebug()<<"several more"<<s_count_nw_i3b<<LsNow->GetRowCount()<<nw_i3b;
            s_pos_nw_i3b++;
            c_sf_rpt++; //qDebug()<<"3------"<<row<<id<<his_call<<his_call_is_std<<next_queued<<LsNow->model.item(nw_i3b,0)->text();
        }
        /*else if (his_call_is_std && s_my_call_is_std)
            res = his_call+" "+s_my_base_call+" RR73";*/
        /*else if (!his_call_is_std && !my_call_is_std)
        {
            his_call = FindBaseFullCallRemAllSlash(his_call);
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
                his_call = FindBaseFullCallRemAllSlash(his_call);
                my_call = s_my_base_call;
            }
        }
        else if (!his_call_is_std && my_call_is_std) my_call  = "<"+my_call+">";
        else if (his_call_is_std && !my_call_is_std) his_call  = "<"+his_call+">";

        c_sf_r73++;//2.76sf 5=RR73 and 4=RPT
        if (id_mshf==2 && c_sf_rpt>=4 && c_sf_r73<5 && !s_msf_ftmsg)//exception 9-slots
        {
            if (LsQueue->GetRowCount()>0)
            {
                next_queued = LsQueue->model.item(0,0)->text();
                next_queued_tx_id = LsQueue->model.item(0,7)->text();
            }
            if (!f_tx_rx && LsQueue->GetRowCount()>0 && !next_queued.isEmpty() && isStandardCall(next_queued) &&
                    LsNow->GetRowCount() >= 8/*SBslots->valueS()+3 <-8*/ && next_queued_tx_id=="3")
            {
                bool exist = false;
                for (int i = 0; i<MAXSL; ++i)//max slots 5-6
                {
                    if (t_list_i3b[i]==next_queued)
                    {
                        exist = true;
                        break;
                    }
                } //qDebug()<<"exist---exeption------------->"<<next_queued<<exist;
                if (!exist)
                {
                    t_list_i3b[s_count_nw_i3b]=next_queued; //qDebug()<<"2-t_list_i3b Need Only ?="<<next_queued;
                    s_count_nw_i3b++;
                    RefreshLists(s_count_nw_i3b);
                    int nw_i3b = LsNow->GetRowCount()-s_count_nw_i3b+s_pos_nw_i3b;
                    str_out.append("#"+LsNow->model.item(nw_i3b,0)->text()+" "+my_call+" RR73");
                    s_pos_nw_i3b++;
                    c_sf_r73++;
                    //s_count_nw_i3b--;//2.76sf ???
                    if (LsNow->model.item(nw_i3b, 9)->text().isEmpty()) LsNow->SetItem(nw_i3b,9,gen_in_tx_time);//2.76sf
                    //qDebug()<<"Prev Call="<<his_call<<row<<"Nex NEW Msg="<<next_queued<<nw_i3b<<"c_rpt="<<c_sf_rpt<<"c_r73="<<c_sf_r73;
                    //for (int i = 0; i<MAXSL; ++i) qDebug()<<t_list_i3b[i];
                }
            }
        }
        //gen_in_tx_time->find decoded in tx period,and gen in tx period and no emited yet,important for RR73 removing msgs
        if (LsNow->model.item(row, 9)->text().isEmpty()) LsNow->SetItem(row,9,gen_in_tx_time);
    }

    str_out.replace(QString("%T"), his_call);
    str_out.replace(QString("%M"), my_call);
    /* ???
    str_out.replace(QString("%H"), DetectSufix(his_call,false));
    str_out.replace(QString("%O"), DetectSufix(my_call,false));
    str_out.replace(QString("%SH"), DetectSufix(his_call,true));
    str_out.replace(QString("%SO"), DetectSufix(my_call,true));
    */
    str_out.replace(QString("%R"), LsNow->model.item(row,1)->text());//my tx rpt
    str_out.replace(QString("%G4"), list_macros.at(1).mid(0,4));//my loc4
    str_out.replace(QString("%G6"), list_macros.at(1));//my loc6
    str_out.replace(QString("%N"), QString("%1").arg(s_txsn_v2,4,10,QChar('0')));//new s 0001

    //bool f = false; //str_out = "K3TT RR73; K7TT <LZ2HV> -04#<K8TT/QRP> <LZ2HV/QRP> RR73";
    //if (str_out.contains('/') || str_out.contains('<')) {qDebug()<<"IN ="<<str_out; f=true;}
    //qDebug()<<"IN ="<<str_out;
    if (id_mshf==2)//2.76.2 CQ +??+ LZ2HV KN23 //1 SP9HWY LZ2HV R+00
    {
        QStringList l0 = str_out.split('#');
        str_out.clear();
        for (int i = 0; i < l0.count(); ++i)//IN = "K3TT RR73; K7TT <LZ2HV> -04#K8TT LZ2HV RR73"
        {
            QString s0 = l0.at(i);//IN = "K1TT/P RR73; K6TT/R <SV2/LZ2HV> +14"
            if (s0.indexOf("RR73;")<0)
            {
                if (s0.indexOf('<')>-1)
                {
                    s0.remove('<');
                    s0.remove('>');
                }
                if (s0.indexOf('/')>-1)//K8TT/P LZ2HV RR73, K8TT/R LZ2HV RR73, <K8TT/QRP> LZ2HV RR73
                {
                    QStringList l = s0.split(' ');
                    if (l.count()>1)
                    {
                        s0.replace(l.at(0),FindBaseFullCallRemAllSlash(l.at(0)));
                        s0.replace(l.at(1),FindBaseFullCallRemAllSlash(l.at(1)));
                    }
                }
            }
            str_out.append(s0);
            if (i<l0.count()-1) str_out.append("#");
        }
    } //qDebug()<<"OUT="<<str_out; qDebug()<<"--------------------------";
    //qDebug()<<"c_rpt="<<c_sf_rpt<<"c_r73="<<c_sf_r73<<his_call<<tx_id<<str_out;
    return str_out;
}
void MultiAnswerModW::SetAuto(bool f) //2.35
{
    f_auto_on = f;
}
void MultiAnswerModW::gen_msg()
{
    int rwc = LsNow->GetRowCount() - s_count_nw_i3b;
    /*if (s_msf)//???
    {
    	if ((c_sf_rpt + c_sf_r73)>=9) rwc++;    
    }*/  //qDebug()<<"rwc="<<rwc;
    //if (SBslots->valueS()==1 && rwc>1) rwc = 1;//2.71 stop for hiscalls 2.69 ? protection for exeption 1-slot but emitting 2
    //if (rwc>MAXSL) rwc = MAXSL;//2.71 stop for hiscalls 2.69 protection never emit more than 5 slots  //max slots 5
    QString prev_current_msg = current_msg; //qDebug()<<SBslots->valueS();
    current_msg = "";
    s_pos_nw_i3b = 0;

    if (rwc<=0)//start cq
    {
        if (!g_block_stop_auto && f_multi_answer_mod_std && (!cb_cont_ns->isChecked() || id_mshf==1))//2.52 ->!g_block_stop_auto eventual->!f_tx_rx &&
            emit EmitStopAuto(); // from set_macros stop auto and clar lists
        if (f_block_free_cq)
        {
            QString Myloc4 = list_macros.at(1).mid(0,4);
            current_msg = "CQ "+list_macros.at(0)+" "+Myloc4;
        }
        else current_msg = LeFreeCQ->text();
        if (id_mshf==2)//2.76sf ok
        {
            if (s_msf_ftmsg)//!f_block_free_cq && CbcqtypeSF->currentText()=="Free Msg" current_msg = "";
            {
                QString Myloc4 = list_macros.at(1).mid(0,4);
                current_msg = "CQ "+list_macros.at(0)+" "+Myloc4;
            }
        } //qDebug()<<"s_msf_ftmsg="<<s_msf_ftmsg<<current_msg;
        emit EmitQSOProgressMAM(5,true);//2.51
    }
    else
    {
        c_sf_rpt=0;
        c_sf_r73=0;
        for (int i = 0; i < rwc; ++i)
        {
            //only first  "Call"<<"dB"<<"Rx dB"<<"Dist"<<"Grid"<<"Freq"<<"Time"<<"IDrpt"<<"TTry"<<"GinTxT";
            current_msg.append(DecodeMacros(i,LsNow->model.item(i,7)->text()));
            if (i<rwc-1) current_msg.append("#");
            if (i==0) emit EmitDxParm(LsNow->model.item(i,0)->text(),LsNow->model.item(i,1)->text(),LsNow->model.item(i,4)->text());
        } //qDebug()<<"gen_msg() end-"<<"c_rpt="<<c_sf_rpt<<"c_r73="<<c_sf_r73;
        if (cb_tx_cq_on_free_slot->isChecked() && rwc < 2 && rwc < SBslots->valueS() && id_mshf!=2)//2.76(&& !s_msf) add cq if have place and only one candidat in LsNow
        {
            current_msg.append("#"); //QString Myloc4 = list_macros.at(1).mid(0,4);
            if (f_block_free_cq)
            {
                QString Myloc4 = list_macros.at(1).mid(0,4);
                current_msg.append("CQ "+list_macros.at(0)+" "+Myloc4);
            }
            else current_msg.append(LeFreeCQ->text());
        }
        int identif = LsNow->model.item(0,7)->text().toInt();//2.51
        emit EmitQSOProgressMAM(identif,true);//2.51
    }
    //V1
    //0 SP9HWY LZ2HV +00                  %H %M %R
    //1 SP9HWY LZ2HV R+00                 %H %M R%R   5b4aif
    //2 SP9HWY LZ2HV RR73                 %H %M RR73
    //3 "TA2NC RR73;" SP9HWY <LZ2HV> +00
    //4 CQ +??+ LZ2HV KN23				  CQ %M %G4
    if (prev_current_msg != current_msg)//2.35
    {
        //qDebug()<<"Msg="<<current_msg<<"Count="<<current_msg.count("#")+1;//2.59 +ft4 f_tx_rx &&
        if ((s_mode==11 || s_mode==13 || s_mode==18 || allq65) && f_auto_on && SBslots->valueS()==1) emit MamEmitMessage(current_msg,false,true,true);
        else emit MamEmitMessage(current_msg,false,false,true); //qDebug()<<"Msg="<<current_msg.count();
    }
}
void MultiAnswerModW::SetSFMATxAll()//2.76sf
{
    static QString s0 = "-";
    QString s;
    s = LeFreeCQ->text();
    if (CbcqtypeSF->currentIndex()>0 && !f_block_free_cq)//if (CbcqtypeSF->currentText()=="Free Msg" && !f_block_free_cq)
    {
        s.append("#1");
        s_msf_ftmsg = true;
    }
    else
    {
        s.append("#0");
        s_msf_ftmsg = false;
    } //qDebug()<<"s_msf_ftmsg="<<s_msf_ftmsg;
    if (cb_tx_cq_on_free_slot->isChecked()) s.append("#1");//s.append("#"+QString("%1").arg(cb_tx_cq_on_free_slot->isChecked()));
    else s.append("#0");

    QString call6 = s_my_base_call;
    call6 = call6.left(6);
    call6 = call6.trimmed();
    s.append("#"+call6);
    bool madx = false;
    if (!f_multi_answer_mod_std && !isHidden()) madx = true;
    s.append("#"+QString("%1").arg(madx)); //qDebug()<<"MAM="<<f_multi_answer_mod_std<<isHidden();

    if (cb_otp_mamd_key->isChecked()) s.append("#1");//s.append("#"+QString("%1").arg(cb_otp_mamd_key->isChecked()));
    else s.append("#0");
    s.append("#"+QString("%1").arg(SBslots->valueS()));//for mamdx no TX OTP if slots is full

    if (s0==s) return;
    s0=s; //qDebug()<<s0.split("#")<<s0.split("#").count()<<"MADX Slots Limit="<<s0.split("#").at(6);
    s.append("#0#0");//protect
    emit EmitSFMATxAll(s);
}
void MultiAnswerModW::cb_otp_mamd_key_toggled()
{
    SetSFMATxAll();//2.76sf
}
void MultiAnswerModW::CbTxCqOnFreeSlotChanged(bool)
{
    SetSFMATxAll();//2.76sf, if SF no need refresh->!s_msf, otherwise it makes a bad message
    if (SBslots->valueS()>1 && id_mshf!=2) gen_msg();//need to upd CQ on free slot`1
}
void MultiAnswerModW::GetCurrentMsg()
{
    emit MamEmitMessage(current_msg,false,false,true);
}
void MultiAnswerModW::SBslotsValueChanged(int)
{
    c_sf_rpt=0; //qDebug()<<"SBslotsValueChanged RefreshLists0";
    RefreshLists(0);
    SetSFMATxAll();//2.76sf for mamdx no TX OTP if slots is full
}
int MultiAnswerModW::FindCallBackupDB(QString call)
{
    int res = -1;
    if (call.isEmpty()) return res;
    for (int i = 0; i<MAXB_DB; ++i)
    {
        if (backup_db_[i][0] == call)
        {
            res = i;
            break;
        }
    }
    return res;
}
void MultiAnswerModW::SetBackupDB(QString call,QString txr,QString rxr,QString g,QString s,QString e)
{
    //"Call"<<"Tx dB"<<"Rx dB"<<"Dist"<<"Grid"<<"Freq"<<"Time"<<"IDrpt"<<"TTry"<<"GinTxT"<<"s"<<"e";
    //"Call"<<"Tx dB"<<"Rx dB"        <<"Grid"                                           <<"s"<<"e";
    //"Call"=0
    //"Tx dB"=1
    //"Rx dB"=2
    //"Grid"=3
    //"s"=4
    //"e"=5
    if (call.isEmpty()) return;
    if (s_mode==0 || s_mode==11 || s_mode==13 || s_mode==18 || allq65)
    {
        if (txr.isEmpty() && rxr.isEmpty() && g.isEmpty() && s.isEmpty() && e.isEmpty()) return;
    }
    else
    {
        if (g.isEmpty()) return;
    }

    int pos_c = FindCallBackupDB(call);
    if (!s.isEmpty())
    {
        int sn = s.toInt();
        s = QString("%1").arg(sn);
    }
    if (pos_c != -1)
    {		// existing call
        if (!g.isEmpty()) backup_db_[pos_c][3] = g;
        if (s_mode==0 || s_mode==11 || s_mode==13 || s_mode==18 || allq65)
        {
            if (!txr.isEmpty()) backup_db_[pos_c][1] = txr;
            if (!rxr.isEmpty()) backup_db_[pos_c][2] = rxr;
            if (!s.isEmpty())   backup_db_[pos_c][4] = s;
            if (!e.isEmpty())   backup_db_[pos_c][5] = e;
        }
    }
    else
    {		// new call
        backup_db_[backup_db_pos_write][0] = call;
        backup_db_[backup_db_pos_write][3] = g;
        if (s_mode==0 || s_mode==11 || s_mode==13 || s_mode==18 || allq65)
        {
            backup_db_[backup_db_pos_write][1] = txr;
            backup_db_[backup_db_pos_write][2] = rxr;
            backup_db_[backup_db_pos_write][4] = s;
            backup_db_[backup_db_pos_write][5] = e;
        }
        else
        {   //overwrite some existing call (reset)
            backup_db_[backup_db_pos_write][1] = "";
            backup_db_[backup_db_pos_write][2] = "";
            backup_db_[backup_db_pos_write][4] = "";
            backup_db_[backup_db_pos_write][5] = "";
        }
        backup_db_pos_write++;
        if (backup_db_pos_write > (MAXB_DB-1)) backup_db_pos_write = 0;
    }

    /*for (int i = 0; i<backup_db_pos_write; ++i)
    {
        QString sss;
        for (int j = 0; j<6; ++j)
        {
            if (j==0) sss.append(" "+backup_db_[i][j].leftJustified(10,' '));
            else if (j==3) sss.append(" "+backup_db_[i][j].leftJustified(6,' '));
            else sss.append(" "+backup_db_[i][j].leftJustified(4,' '));
            if (j<5)sss.append("|");
        }
        qDebug()<<sss;
    }
    qDebug()<<"---------------------------------------------"<<backup_db_pos_write;*/
    //printf("backup_db_pos_writec=%d\n",backup_db_pos_write);
}
void MultiAnswerModW::GetBackupDB(int pos,QString &txr,QString &rxr,QString &g,QString &s,QString &e)
{
    g = backup_db_[pos][3];
    if (s_mode==0 || s_mode==11 || s_mode==13 || s_mode==18 || allq65)
    {
        txr = backup_db_[pos][1];
        rxr = backup_db_[pos][2];
        s   = backup_db_[pos][4];
        e   = backup_db_[pos][5];
    }
    else
    {
        txr = "";
        rxr = "";
        s   = "";
        e   = "";
    }
}
void MultiAnswerModW::ClrQueue()
{
    LsQueue->Clear_List();
}
void MultiAnswerModW::RefreshBackupDbAll()//2.59
{
    for (int i = LsNow->model.rowCount()-1; i >= 0; --i)
    {
        SetBackupDB(LsNow->model.item(i,0)->text(),
                    LsNow->model.item(i,1)->text(),
                    LsNow->model.item(i,2)->text(),
                    LsNow->model.item(i,4)->text(),
                    LsNow->model.item(i,10)->text(),
                    LsNow->model.item(i,11)->text());
    }
}
void MultiAnswerModW::ClrNow()
{
    bool f_clr = false;
    if (LsNow->GetRowCount()>0) f_clr = true;
    RefreshBackupDbAll();
    LsNow->Clear_List();
    if (LsQueue->GetRowCount()<=0)
    {
        gen_msg();//need to upd CQ
        QStringList l;
        l.clear();
        if (f_clr) emit EmitHisCalls(l);//2.71  2.72 no needed -> g_hf_b
        if (f_multi_answer_mod_std && f_clr && SBslots->valueS()<2) emit EmitMAFirstTX(false);//2.71
    }
    else
    {
        c_sf_rpt=0; //qDebug()<<"ClrNow RefreshLists0";
        RefreshLists(0);
    }
}
void MultiAnswerModW::ClrNowUserDoubleClick()
{
    RefreshBackupDbAll();
    LsNow->Clear_List();
}
void MultiAnswerModW::BlockFrreCq(bool f)
{
    f_block_free_cq = f;

    if (f_block_free_cq)
    {
        if (dsty)
        {
            LeFreeCQ->setStyleSheet("QLineEdit{background-color:rgb(55,55,55);color:rgb(255,200,0)}");
            pb_use_free_cq->setStyleSheet("QPushButton{color:rgb(255,200,0)}");
        }
        else
        {
            LeFreeCQ->setStyleSheet("QLineEdit{background-color:"+ColorStr_[3]+";color:"+ColorStr_[1]+"}");
            pb_use_free_cq->setStyleSheet("QPushButton{color:"+ColorStr_[1]+"}");
        }
    }
    else
    {
        LeFreeCQ->setStyleSheet("QLineEdit{background-color:palette(Base);}");
        pb_use_free_cq->setStyleSheet("QPushButton{background-color:palette(Button);}");
    }
}
void MultiAnswerModW::SetStartFromTx2Tx1(bool f)
{
    s_start_qso_from_tx2 = f;
}
static bool g_block_setx = false;
static bool g_hf_b = false;
void MultiAnswerModW::ConfigRestrictW()
{
    if (f_multi_answer_mod_std)//(s_mode==11 || s_mode==13 || s_mode==18) &&
    {
        if (gg_frest || allq65) SBslots->setRange(1,1);// || s_mode==18
        else
        {
            if (s_co_type==0) SBslots->setRange(1,2);
            else SBslots->setRange(1,1);//<- importent eu vhf only 1 slot needed
        }
        cb_tx_sm->setHidden(true);
        cb_otp_mamd_key->setHidden(true);
        cb_cont_ns->setHidden(false);
        if (id_mshf==1) cb_cont_ns->setEnabled(false);
        else cb_cont_ns->setEnabled(true);
        SBslots->setHidden(false);//2.76
    }
    else
    {
        if (gg_frest || allq65) SBslots->setRange(1,1);// || s_mode==18
        else
        {
            if (s_mode==13) SBslots->setRange(1,4);//2.71 old=3
            else if (s_mode==18) SBslots->setRange(1,3);//2.71 old=3
            else SBslots->setRange(1,MAXSL); //2.71 max slots 5
        }
        cb_cont_ns->setHidden(true);
        //cb_tx_sm->setHidden(false);
        if (id_mshf==2)//2.76
        {
            SBslots->setHidden(true);
            cb_tx_sm->setHidden(true);
            cb_otp_mamd_key->setHidden(true);
        }
        else
        {
            SBslots->setHidden(false);
            cb_tx_sm->setHidden(false);
            if (s_mode==11)cb_otp_mamd_key->setHidden(false); //2.76sf for remove un comment
            //if (f_vsf_ && s_mode==11) cb_otp_mamd_key->setHidden(false);//2.76sf for remove
            else cb_otp_mamd_key->setHidden(true);
        }
    }

    if (SBslots->maximumS()==1) SBslots->setEnabled(false);
    else if (!g_block_setx || !g_hf_b) SBslots->setEnabled(true);//2.71 added g_block_setx
    else if (g_hf_b && g_block_setx) SBslots->setEnabled(false);

    if (s_mode!=11 && s_mode!=13 && s_mode!=18 && !allq65) emit EmitMAFirstTX(false);//2.76.4
    else
    {//2.71
        if (g_hf_b && g_block_setx && f_multi_answer_mod_std && SBslots->valueS()>1) SBslots->setValue(1);
        else if (g_hf_b && f_multi_answer_mod_std && SBslots->valueS()>1/*||LsNow->GetRowCount()>1*/) emit EmitMAFirstTX(true);
        else if (f_multi_answer_mod_std && LsNow->GetRowCount()<2) emit EmitMAFirstTX(false);
        else emit EmitMAFirstTX(true);
    }
    //qDebug()<<g_hf_b<<g_block_setx<<f_multi_answer_mod_std<<SBslots->valueS();
    //qDebug()<<"valueS="<<SBslots->valueS()<<"value="<<SBslots->value();
    //printf("valueS=%d maximumS=%d\n",SBslots->valueS(),SBslots->maximumS());
}
void MultiAnswerModW::SetMsfS5SMsg(uint8_t mshf)//2.76
{
    if (mshf > 0 && s_mode==11)
    {
        id_mshf = mshf;
        if (mshf == 2) SBslots->SetMsfS5(true);
        else SBslots->SetMsfS5(false);
    }
    else
    {
        id_mshf = 0;
        SBslots->SetMsfS5(false);
    } //qDebug()<<"mode="<<s_mode<<"id_mshf="<<id_mshf;
    ConfigRestrictW();
    if (id_mshf==2)
    {
        Cbcqtype->setHidden(true);
        CbcqtypeSF->setHidden(false);
    }
    else
    {
        CbcqtypeSF->setHidden(true);
        Cbcqtype->setHidden(false);
    }
    CbCQChanged("");//2.76 for reset CQ to default CQ DX not possyble //printf("id_mshf=%d\n",s_msf);
    SetSFMATxAll();//2.76sf madx use isHidden() for sending OTP key, in MADX mod only, to msplayer
}
void MultiAnswerModW::RestrictSeTX(bool f)//2.71
{
    g_block_setx = f;
    if (f)
    {
        if (g_hf_b) SBslots->setEnabled(false); //SBslots->setValue(1);//moved up to ConfigRestrictW()
    }
    else
    {
        if (g_hf_b && SBslots->maximumS()>1) SBslots->setEnabled(true);
    }
}
void MultiAnswerModW::setHfBand(bool f)
{
    g_hf_b = f;
    if (s_mode==11 || s_mode==13 || s_mode==18 || allq65) ConfigRestrictW();
    //ClrQueue();
    //ClrNow();
    //printf(" Hf Band =%s Mode =%d\n",f ? "true " : "false",s_mode);
}
void MultiAnswerModW::RefreshLRestrict_pub(bool f)
{
    gg_frest = f;
    if (f)
    {
        bool f_msg = false;
        if (SBslots->valueS()>1) f_msg = true;
        ConfigRestrictW();
        if (f_msg && !isHidden())
        {
            QMessageBox msgBox(this);
            msgBox.setWindowTitle("MSHV");
            msgBox.setIcon(QMessageBox::Information);
            msgBox.setText(tr("In Settings MA, TX Slots: It Is Set To ONE.\n"
                              "Please choose another dial frequency.\n"
                              "MSHV will not operate on more than one slot\n"
                              "in the standard FT8 and FT4 HF sub-bands."));
            msgBox.setStandardButtons(QMessageBox::Ok);
            msgBox.button(QMessageBox::Ok)->animateClick(8000);//after 15sec auto close
            msgBox.button(QMessageBox::Ok)->setFixedWidth(85);
            msgBox.exec();
        }
    }
    else ConfigRestrictW();
}
void MultiAnswerModW::RefreshLRestrict()//2.23
{
    if (s_mode==11 || s_mode==13 || s_mode==18 || allq65)//clr for ft8 ft4
    {
        ClrQueue();
        ClrNow();
        ConfigRestrictW();
        s_last_bccall_tolog_excp = "_NONE_";//reset
    }
}
void MultiAnswerModW::SetMAStd(bool f)
{
    f_multi_answer_mod_std = f;
    RefreshLRestrict();
    CbCQChanged("");
    QStringList l;
    l.clear();
    emit EmitHisCalls(l);//2.71
}
void MultiAnswerModW::SetBand()//for reset
{
    RefreshLRestrict();
}
void MultiAnswerModW::SetMode(int m)
{
    if (m == 14 || m == 15 || m == 16 || m == 17) allq65 = true;
    else allq65 = false;
    s_mode = m;
    RefreshLRestrict();
    if (s_mode!=11 && s_mode!=13 && s_mode!=18 && !allq65) emit EmitMAFirstTX(false);//2.71
}
void MultiAnswerModW::SetMacros(QStringList listm,int id_c0,int type_c0)
{
    //2.65 if Makros and RadioAndNet is closed and no changes
    //2.65 problem is if I TXing msg will be resetted
    if (list_macros==listm && s_co_type==type_c0 && s_co_id==id_c0) return;

    g_block_stop_auto = true;//2.52
    s_co_type = type_c0;
    s_co_id = id_c0;
    RefreshLRestrict();
    list_macros = listm; //qDebug()<<list_macros;

    QString my_c = list_macros.at(0);
    s_my_base_call = FindBaseFullCallRemAllSlash(my_c);
    //s_my_call_is_std = true;//isStandardCall(my_c);
    if (my_c.contains("/"))
    {
        my_call_have_slash = true;
    }
    else
    {
        //list_macros.replace(0, my_c.mid(0,6));//2.00 strip to 6char
        my_call_have_slash = false;
    }//qDebug()<<"TX RX ====="<<s_my_base_call<<my_call_have_slash;

    CbCQChanged("");
    BlockFrreCq(false);
    SetSFMATxAll();//2.76sf for my call
    gen_msg();
    g_block_stop_auto = false;//2.52
}
void MultiAnswerModW::SetQRG(QString s)
{
    if (s.isEmpty() || s.at(0).isLetter() || s.at(1).isLetter() || s.at(2).isLetter()) s_qrg = s;
    else s_qrg = QString("%1").arg(s.toInt(),3,10,QChar('0'));
    CbCQChanged("");
}
QString MultiAnswerModW::DetectCQTypeFromMacros(QString s)
{
    QString cqq = "CQ";
    int mci = s.indexOf(" %M");//my call  CQ XXXX %M
    QString cqq0 = s+"xxx";
    if (cqq0.mid(0,3)=="CQ " && mci > 3 && mci < 8)
    {
        bool islet = true;
        for (int i = 3; i < mci; ++i)
        {
            if (!s.at(i).isLetter()) islet = false;
        }
        if (islet) cqq = s.mid(0,mci);
    }
    return cqq;
}
void MultiAnswerModW::CbCQChanged(QString)
{
    if (s_co_type > 0 && f_multi_answer_mod_std)
    {
        for (int i = 0; i <6; ++i)
        {
            if (i==3) str_macros_mam_[i]="%T %M RR73";
            else
            {	//2.66 protection from sufix Macros
                QString s = list_macros.at(i+2);
                s.replace(QString("%H"),"%T");
                s.replace(QString("%O"),"%M");
                s.replace(QString("%SH"),"%T");
                s.replace(QString("%SO"),"%M");
                str_macros_mam_[i]=s;
            }
        }
    }
    else if (s_co_type == 0 && f_multi_answer_mod_std)//2.72
    {
        str_macros_mam_[0]="%T %M %G4";
        str_macros_mam_[1]="%T %M %R";
        str_macros_mam_[2]="%T %M R%R";
        str_macros_mam_[3]="%T %M RR73";
        str_macros_mam_[4]="%T %M 73";//2.72 new
        str_macros_mam_[5]="CQ %M %G4";
    }
    else//DXpedition no report -> 73
    {
        str_macros_mam_[0]="%T %M %G4";//2.47
        str_macros_mam_[1]="%T %M %R";
        str_macros_mam_[2]="%T %M R%R";
        str_macros_mam_[3]="%T %M RR73";
        str_macros_mam_[4]="%T %M RR73";
        str_macros_mam_[5]="CQ %M %G4";
    }//qDebug()<<str_macros_mam_[0]<<str_macros_mam_[1]<<str_macros_mam_[2]<<str_macros_mam_[3]<<str_macros_mam_[4]<<str_macros_mam_[5];

    if (s_co_type == 0 || !f_multi_answer_mod_std) Cbcqtype->setItemText(0,"CQ");//2.76
    else Cbcqtype->setItemText(0,DetectCQTypeFromMacros(str_macros_mam_[5]));

    if (!Cbcqtype->isHidden())
    {
        QString sc = Cbcqtype->currentText();//QString sr = Cbcqtype->itemText(0);//reset
        if (sc=="Free Msg")
        {
            LeFreeCQ->setEnabled(true);
            pb_use_free_cq->setEnabled(true);
            if (LeFreeCQ->text()=="PSE SYNC TIME")
            {
                QString sr = Cbcqtype->itemText(0);//reset
                QString Myloc4 = list_macros.at(1).mid(0,4);
                LeFreeCQ->setText(sr+" "+list_macros.at(0)+" "+Myloc4);//reset
                BlockFrreCq(false);
            }
        }
        else
        {
            LeFreeCQ->setEnabled(false);
            pb_use_free_cq->setEnabled(false);
            QString Myloc4 = list_macros.at(1).mid(0,4);

            if (sc=="TIME") LeFreeCQ->setText("PSE SYNC TIME");
            else if (!isStandardCall(list_macros.at(0))) LeFreeCQ->setText("CQ "+list_macros.at(0));
            else
            {
                sc.replace(QString("QRG"),s_qrg);//2.33
                LeFreeCQ->setText(sc+" "+list_macros.at(0)+" "+Myloc4);
            }
        }
    }
    if (!CbcqtypeSF->isHidden())//2.76
    {
        QString sc = CbcqtypeSF->currentText();//QString sr = CbcqtypeSF->itemText(0);
        if (CbcqtypeSF->currentIndex()>0)
        {
            LeFreeCQ->setEnabled(true);
            pb_use_free_cq->setEnabled(true);
            if 		(sc=="QSY To") 		 LeFreeCQ->setText("QSY TO 14092 KHZ");
            else if (sc=="QRM QSY To") 	 LeFreeCQ->setText("QRM QSY TO 14092 KHZ");
            else if (sc=="Call From To") LeFreeCQ->setText("CALL FROM 200 TO 3200 HZ");
            else if (sc=="QRX") 		 LeFreeCQ->setText("QRX 10 MIN");
            else if (sc=="QRT") 		 LeFreeCQ->setText("QRT 73 TO ALL");
            else if (sc=="Free Msg") 	 LeFreeCQ->setText("TYPE MESSAGE");
        }
        else
        {
            LeFreeCQ->setEnabled(false);
            pb_use_free_cq->setEnabled(false);
            QString Myloc4 = list_macros.at(1).mid(0,4);
            if (!isStandardCall(list_macros.at(0))) LeFreeCQ->setText("CQ "+list_macros.at(0));//2.76.2
            else LeFreeCQ->setText(sc+" "+list_macros.at(0)+" "+Myloc4);
        }
        SetSFMATxAll();//2.76sf
    }
    //ok Cbcqtype->clearFocus(); //ok->Qt::NoFocus if cq box noting happend Ctrl+` kyboard shortcut
    //setFocus();//if cq box noting happend Ctrl+` kyboard shortcut
}
void MultiAnswerModW::LeFreeCQtextChanged(QString)
{
    if (!Cbcqtype->isHidden())
    {
        if (Cbcqtype->currentText()=="Free Msg") BlockFrreCq(true);
        else BlockFrreCq(false);
    }
    if (!CbcqtypeSF->isHidden())//2.76sf
    {
        if (CbcqtypeSF->currentIndex()>0) BlockFrreCq(true);//if (CbcqtypeSF->currentText()=="Free Msg") BlockFrreCq(true);
        else BlockFrreCq(false);
        SetSFMATxAll();//2.76sf
    }
    if (LsNow->GetRowCount()<=0) gen_msg();//need to upd CQ
}
QString MultiAnswerModW::RemWSpacesInsideAndBegEnd(QString str)
{
    QString s;
    //int msg_count = 0;
    //RemWSpacesInside
    for (int i = 0; i<str.count(); i++) str.replace("  "," ");
    //RemBegEndWSpaces //2.64 stop
    /*for (msg_count = str.count()-1; msg_count>=0; msg_count--)
    {
        if (str.at(msg_count)!=' ')
            break;
    }
    s = str.mid(0,msg_count+1);
    msg_count = 0;
    for (msg_count = 0; msg_count<s.count(); msg_count++)
    {
        if (s.at(msg_count)!=' ')
            break;
    }
    s = s.mid(msg_count,(s.count()-msg_count));*/
    s = str.trimmed();
    return s;
}
/*bool MultiAnswerModW::IsMsgHaveValidCall(QString s)
{
    QStringList	msg = s.split(" ");
    bool loc_once = false;
    for (int i = msg.count()-1; i >= 0; --i)
    {
        if (!loc_once && THvQthLoc.isValidLocator(msg.at(i)))
        {
            loc_once = true;
        }
        else if (THvQthLoc.isValidCallsign(msg.at(i)))
            return true;
    }

    return false;
}*/
void MultiAnswerModW::UseFreeCq()
{
    /*if (!IsMsgHaveValidCall(LeFreeCQ->text()))//no emit empty text
    {
        QString text = "Message does not contains with valid call sign";
        QMessageBox::critical(this, "  Warning  ", text, QMessageBox::Ok);
        return;
    }*/
    QString str = LeFreeCQ->text();
    str = RemWSpacesInsideAndBegEnd(str);
    LeFreeCQ->setText(str);
    BlockFrreCq(false);
    SetSFMATxAll();//2.76sf
    if (LsNow->GetRowCount()<=0) gen_msg();//need to upd CQ
}
void MultiAnswerModW::SetTxRxMsg(bool f)
{
    f_tx_rx = f;
    if (f)
    {
        gen_in_tx_time = "1";//1 tx 0 rx
        bool is_change = false;
        unsigned int ttry_now = QDateTime::currentDateTimeUtc().toTime_t();
        for (int i = LsNow->GetRowCount()-1; i >=0; --i)
        {
            unsigned int ttry = LsNow->model.item(i,8)->text().toUInt();
            /*bool res = false;
            if (!SBmaxTP->GetMTP()) res = (ttry_now-ttry)>=(unsigned int)SBmaxTP->value()*60;
            else res = (ttry_now-ttry)>=(unsigned int)((period_time_sec*2.0)*(float)SBmaxTP->value());
            if (res)*/
            unsigned int res = 0;
            if (!SBmaxTP->GetMTP()) res = (unsigned int)SBmaxTP->value()*60;
            else res = (unsigned int)((period_time_sec*2.0)*(float)SBmaxTP->value());
            if ((ttry_now-ttry)>=res)
            {
                SetBackupDB(LsNow->model.item(i,0)->text(),
                            LsNow->model.item(i,1)->text(),
                            LsNow->model.item(i,2)->text(),
                            LsNow->model.item(i,4)->text(),
                            LsNow->model.item(i,10)->text(),
                            LsNow->model.item(i,11)->text());
                LsNow->RemoveRow(i);
                is_change = true;
            }
        }
        if (is_change)
        {
            c_sf_rpt=0; //qDebug()<<"SetTxRxMsg RefreshLists0";
            RefreshLists(0);
        }
    }
    else
    {
        gen_in_tx_time = "0";
        SetTxMsgEnd();
    }
}
void MultiAnswerModW::SetLastBcCallToLog(QString call)
{
    s_last_bccall_tolog_excp = call;
}
void MultiAnswerModW::SetTxMsgEnd()
{
    bool is_change = false;
    //qDebug()<<"TXing END====="<<f_tx_rx<<f_cfm73;
    //if (!f_tx_rx)//2.69 ???
    //{
    s_count_nw_i3b = 0;
    for (int i = 0; i <MAXSL; ++i) t_list_i3b[i]=""; //max slots 5
    //}

    for (int i = LsNow->GetRowCount()-1; i >=0; --i)
    {
        QString id_rpt = LsNow->model.item(i,7)->text();
        QString g_tx_t = LsNow->model.item(i,9)->text();
        bool f_id_rpt = false;//2.72
        if (id_rpt=="3" || id_rpt=="4") f_id_rpt = true;//2.72
        if (f_multi_answer_mod_std && s_co_id==15 && id_rpt=="2")
        {
            g_tx_t = "0";
            f_id_rpt = true;
        }
        //qDebug()<<"TXing-END== "<<"id_rpt="<<id_rpt<<"g_tx_t="<<g_tx_t<<f_id_rpt;
        if (f_id_rpt && g_tx_t=="0")//2.72
        {
            //gen_in_tx_time->find decoded in tx period,and gen in tx period and no emited yet,important for RR73 removing msgs
            //log_data_start & log_time_start, his_call, his_loc, rst_tx,  rst_rx
            //"Call"<<"dB"<<"Rx dB"<<"Dist"<<"Grid"<<"Freq"<<"Time"<<"IDrpt"<<"TTry"<<"GinTxT"
            QString comm = "MADX QSO";
            QString cont_id = "0";
            QString rxsn = "";
            QString rxex = "";
            bool f_addtolog = true;
            if (f_multi_answer_mod_std)
            {
                QString hisBaseCall_inmsg = FindBaseFullCallRemAllSlash(LsNow->model.item(i,0)->text());
                if (hisBaseCall_inmsg == s_last_bccall_tolog_excp) f_addtolog = false;
                if (f_addtolog)
                {
                    //s_last_bccall_tolog_excp = hisBaseCall_inmsg;
                    comm = "MASTD QSO";
                    if (s_co_type>1)
                    {
                        //All=2 and up
                        comm = s_cont_name[s_co_id];
                        cont_id = QString("%1").arg(s_co_id);
                        if (s_co_type==3)
                        {
                            QString s = LsNow->model.item(i,10)->text();
                            if (!s.isEmpty())
                            {
                                int sn = s.toInt();
                                rxsn = QString("%1").arg(sn);
                            }
                        }
                        else if (s_co_type==4)
                        {
                            rxex = LsNow->model.item(i,11)->text();
                        }
                        else if (s_co_type==5)
                        {
                            rxex = LsNow->model.item(i,11)->text();
                            QString s = LsNow->model.item(i,10)->text();
                            if (!s.isEmpty())
                            {
                                int sn = s.toInt();
                                rxsn = QString("%1").arg(sn);
                            }
                        }
                    }
                }
            }
            if (f_addtolog)
            {
                QStringList l;
                l<<LsNow->model.item(i,6)->text()<<LsNow->model.item(i,0)->text()
                <<LsNow->model.item(i,4)->text()<<LsNow->model.item(i,1)->text()
                <<LsNow->model.item(i,2)->text()
                <<comm<<rxsn<<rxex<<cont_id; //qDebug()<<"AddtoLog"<<LsNow->model.item(i,0)->text();
                emit AddToLog(l);
            }
            LsNow->RemoveRow(i);
            is_change = true;
        }
        //else if (id_rpt==wait_for && g_tx_t=="1")//exeption
        //else if (id_rpt=="3" && g_tx_t=="1")//exeption
        else if (f_id_rpt && g_tx_t=="1")//2.72 exeption
        {
            //gen_in_tx_time->find decoded in tx period,and gen in tx period and no emited yet,important for RR73 removing msgs
            LsNow->SetItem(i,9,"0");
        }
    }
    if (is_change)
    {
        c_sf_rpt=0; //qDebug()<<"SetTxMsgEnd RefreshLists0";
        RefreshLists(0);
    }
}
void MultiAnswerModW::RefreshLists(int c_plus)
{
    bool is_need_rfresh_lists = true; //qDebug()<<"RefreshLists==="<<c_plus<<LsNow->GetRowCount()<<LsQueue->GetRowCount();
    if (LsNow->GetRowCount() >= SBslots->valueS() + c_plus || LsQueue->GetRowCount()<=0) is_need_rfresh_lists = false;

    static int prevnowc = 0;//2.71
    bool is_need_rfresh_hiscalls = true;
    if (!is_need_rfresh_lists && prevnowc == LsNow->GetRowCount()) is_need_rfresh_hiscalls = false;
    bool f_fr_msg_sf = false;
    if (id_mshf==2 && s_msf_ftmsg) f_fr_msg_sf = true;//CbcqtypeSF->currentText()=="Free Msg" && !f_block_free_cq

    if (is_need_rfresh_lists)
    {
        int remc = 0;
        for (int i = 0; i < LsQueue->GetRowCount(); ++i)
        {
            if (f_fr_msg_sf)//2.76sf max 4slots if free msg
            {
                if (LsNow->GetRowCount() >= 4/*SBslots->valueS()-1 <-4*/) break;
            }

            QStringList ls; //"Call"<<"dB"<<"Rx dB"<<"Dist"<<"Grid"<<"Freq"<<"Time"<<"IDrpt"<<"TTry"<<"GinTxT";
            for (int j = 0; j < LsQueue->GetColumnCount(); ++j) ls << LsQueue->model.item(i,j)->text();

            if (id_mshf==2 && ls.at(7)=="1")//2.76sf
            {
                if (c_sf_rpt>=4) continue;
                if (c_plus==0) c_sf_rpt++;//2.76sf count in gen_msg
            }

            QDateTime utc_t = QDateTime::currentDateTimeUtc();
            if (ls.at(6).isEmpty())//no start time
            {
                QString d=utc_t.toString("yyyyMMdd");
                QString t=utc_t.toString("hh:mm");
                ls.replace(6,d+" "+t);
            }
            QString ttry = QString("%1").arg(utc_t.toTime_t());//moze +15s
            ls.replace(8,ttry);  //qDebug()<<"ls.replace(8,ttry);"<<ttry;

            LsNow->InsertItem_hv(ls);
            remc++;
            if (LsNow->GetRowCount() >= SBslots->valueS()) break;
        }
        LsQueue->RemoveRows(0,remc);

        QStringList lout;//2.00 for hash
        for (int i = 0; i < LsNow->GetRowCount(); ++i) lout<<LsNow->model.item(i,0)->text();
        emit EmitMAMCalls(lout);
    }
    if (c_plus == 0) gen_msg();

    prevnowc = LsNow->GetRowCount(); //qDebug()<<"MA= "<<is_need_rfresh_lists<<is_need_rfresh_hiscalls;
    if (is_need_rfresh_hiscalls)//2.71
    {
        QStringList lhc;
        for (int i = 0; i < LsNow->GetRowCount(); ++i) lhc<<LsNow->model.item(i,0)->text();
        if (LsNow->GetRowCount()<=0) lhc.clear();
        emit EmitHisCalls(lhc);
    }
    if (g_hf_b && f_multi_answer_mod_std)//2.71
    {
        if (SBslots->valueS()>1) emit EmitMAFirstTX(true);
        else if (LsNow->GetRowCount()<2) emit EmitMAFirstTX(false);
    }
    if (id_mshf==2 && LsNow->GetRowCount() >= 5/*SBslots->valueS() <-5*/) CbcqtypeSF->setEnabled(false);//s2.76sf
    else if (!CbcqtypeSF->isEnabled()) CbcqtypeSF->setEnabled(true);
}
static QString _s00_;
void MultiAnswerModW::SetTextMark(bool b,bool m)
{
    s_txt_mark_b = b;
    s_txt_mark_m = m;
    _s00_ = tr("Filtered By")+": "+tr("Call");
    if (s_txt_mark_b) _s00_.append(", "+tr("Band"));// band
    if (s_txt_mark_m) _s00_.append(", "+tr("Mode"));// mode
    Cb_dupes->setToolTip(tr("Maximum QSOs per Call")+"\n"+_s00_);
}
void MultiAnswerModW::SetInfoDupeQso(bool f)
{
    info_dupe_qso = f;
}
bool MultiAnswerModW::DialogIsCallDupeInLog(QString hisCall_inmsg)
{
    bool res = false;
    bool is_dupe = false;

    emit IsCallDupeInLog(hisCall_inmsg,1,is_dupe);//2.66 (0 or 1)=1-QSO 2=2-QSO 3=3-QSO

    if (is_dupe)
    {
        emit EmitGBlockListExp(true);//2.06
        setFocus(Qt::MouseFocusReason);//?? <-crash in decodelist.cpp bool MyDelegate::eventFilter
        //b_gen_msg->setFocus();//?? <-crash in decodelist.cpp QWidget* bool MyDelegate::eventFilter
        this->setFocus();

        //QString criter = "  Criteria are: By Call"; //Mode "+ModeStr(s_mode)+", Band "+s_band
        /*QString criter = " "+tr("Filtered By")+": "+tr("Call");
        if (s_txt_mark_b) criter.append(", "+tr("Band"));// band
        if (s_txt_mark_m) criter.append(", "+tr("Mode"));// mode*/

        QString text = "<p align='center'>"+tr("QSO Before")+": "+tr("Call")+" "+hisCall_inmsg//+"<br>"
                       +" "+_s00_+"<br>"+tr("Do You Want To Continue QSO?")+"</p>";//ModeStr(s_mode),s_band

        int ret = QMessageBox::warning(this,"  MSHV Log Warning  ",text,QMessageBox::Ok | QMessageBox::Cancel);

        switch (ret)
        {
        case QMessageBox::Cancel:
            res = true;
            break;
        case QMessageBox::Ok:
            res = false;
            break;
        default:
            res = true;
            break;
        }
        emit EmitGBlockListExp(false);//2.06
    }
    return res;
}
void MultiAnswerModW::SetCntestOnlySdtc(bool f)
{
    f_con_only_sdtc = f; //qDebug()<<"f_cntest_only_sdtc="<<f_cntest_only_sdtc;
}
QString MultiAnswerModW::TryFormatRPTtoRST(QString rpt)//2.59
{
    QString rst = rpt;
    if ((s_mode==0 || s_mode==11 || s_mode==13 || s_mode==18 || allq65) && (s_co_type==3 || s_co_type==5))//ft4  3=52-59 for Contest v2  5=//529-599
    {
        int n = rst.toInt();
        int nn=(n+36)/6;
        if (nn<2) nn=2;
        if (nn>9) nn=9;
        rst = "5"+QString("%1").arg(nn)+"9";
        if (s_co_type==3) rst = rst.mid(0,2);
    }
    return rst;
}
void MultiAnswerModW::SetCfm73(bool f)
{
    f_cfm73 = f; //qDebug()<<"SetCfm73"<<f;
}
void MultiAnswerModW::SetIsLastTxRptCq(bool f)//2.66
{
    is_LastTxRptCq = f;
    if (!is_LastTxRptCq) s_dist_points = -1;//2.66 reset
}
void MultiAnswerModW::SetUseASeqMaxDist(bool f)//2.66
{
    f_aseqmaxdist = f;
}
void MultiAnswerModW::SetMaManAdding(bool f)
{
    s_man_adding = f;
}
void MultiAnswerModW::DecListTextAll(QString tx_rpt,QString str,QString freq,bool f_double_click,
                                     QString &his_call_for_ap,QString &his_loc_for_dist)
{
    //QDateTime utc_t = QDateTime::currentDateTimeUtc();
    //s_log_data_start=utc_t.toString("yyyyMMdd");
    //s_log_time_start=utc_t.toString("hh:mm");
    QString hisCall_inmsg = "";
    QString hisLoc_inmsg  = "";
    QString myCall_inmsg  = "";
    QString rpt_inmsg  = "";
    QString cont_r_inmsg  = ""; //msk144 ft8 contest mode R fictive
    QString rr73_inmsg  = "";   ////1.54=RR73 msk144 ft8 rr73
    int row_queue = -1;// -1=ident find mam   -2=no find mam
    int row_now = -1;  // -1=ident find mam   -2=no find mam
    QString sn_inmsg = "";//v.2.0 fictive
    QString arrl_exch_imsg = "";//v.2.01 fictive

    //QString nowcall = "";//inportent only one slot needed     nowcall,//"" <- corr for eu VHF
    //if (f_multi_answer_mod_std && LsNow->GetRowCount()>0)//no problem bez tova-> && s_id_con==3
    //nowcall = LsNow->model.item(0,0)->text();//call
    //qDebug()<<"nowcall"<<f_multi_answer_mod_std<<s_id_con<<LsNow->GetRowCount()<<nowcall;

    DetectTextInMsg(str, hisCall_inmsg,hisLoc_inmsg,myCall_inmsg,rpt_inmsg,cont_r_inmsg,rr73_inmsg,
                    row_queue,row_now,sn_inmsg,arrl_exch_imsg);
    his_call_for_ap = hisCall_inmsg;
    his_loc_for_dist = hisLoc_inmsg;
    //for now and=&&   or=|| if msg=DE SP9HWY/QRP R-15

    if ((!myCall_inmsg.isEmpty() || f_multi_answer_mod_std) && !hisCall_inmsg.isEmpty())
    {
        bool fc1,fc2;
        uint8_t noQSO;
        isStandardCalls(list_macros.at(0),hisCall_inmsg,fc1,fc2,noQSO);
        if (f_multi_answer_mod_std && f_con_only_sdtc)
        {
            if (!fc1 || !fc2) return;
        }
        if (noQSO==100) return; //two_no_sdtc

        int id_0qe_1nw_2ne = 2;//0=Queue,1=Now,2=New //2.47 moved here
        if (row_queue>-1) id_0qe_1nw_2ne = 0;
        if (row_now  >-1) id_0qe_1nw_2ne = 1;
        if (s_man_adding && id_0qe_1nw_2ne==2 && !f_double_click) return;//2.73

        //if (cb_no_dupes->isChecked() && id_0qe_1nw_2ne == 2) //2.47 kp4hf problem
        if (Cb_dupes->currentIndex()>0 && id_0qe_1nw_2ne == 2 && (!f_multi_answer_mod_std || !f_double_click))//2.66=?
        {
            bool is_dupe = false;
            emit IsCallDupeInLog(hisCall_inmsg,Cb_dupes->currentIndex(),is_dupe);
            if (is_dupe) return;
        }
        /*if (f_multi_answer_mod_std && info_dupe_qso && f_double_click)//2.66=stop
        {
            if (DialogIsCallDupeInLog(hisCall_inmsg)) return;
        }*/

        ////////// ORG only for MSK, FT8/4 and not for contests and MA ///////////////////
        if (f_double_click && s_co_id==0 && f_multi_answer_mod_std && SBslots->valueS()==1 && !allq65) //2.71
        {
            QStringList lstr = str.split(" ");
            if (lstr.count() > 2) //CQ 236 LZ2HV
            {
                QString s = lstr.at(1);
                if (s.count()<4 && lstr.at(0)=="CQ") //only kHz 236
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
                    if (alldig) emit EmitDoQRG(s,str);
                }
            }
        }
        ///////////////////////////// END ORG ////////////////////////////////////////////

        //qDebug()<<LsQueue->FindCallRow(hisCall_inmsg)<<LsNow->FindCallRow(hisCall_inmsg);
        QString id_rpt = "0";
        bool block_rpt_all = true;
        if (!myCall_inmsg.isEmpty())//<-- && f_double_click
        {
            if (rpt_inmsg.isEmpty() && hisLoc_inmsg.isEmpty() && cont_r_inmsg.isEmpty() && rr73_inmsg.isEmpty() && arrl_exch_imsg.isEmpty())
            {//msg=0 only calls exeption if have slashs
                id_rpt = "1"; //qDebug()<<"MAM msg=0 only calls===================";
                block_rpt_all = false;
            }
            else if (rpt_inmsg.isEmpty() && (!hisLoc_inmsg.isEmpty() || !arrl_exch_imsg.isEmpty()))//1.84 down my be for later
            {//msg=0 KN23
                if (!cont_r_inmsg.isEmpty())  //msk144 ft8 c mode
                    id_rpt = "3";
                else
                {
                    if (f_multi_answer_mod_std && (s_co_type==2 || s_co_type==4)) id_rpt = "2";//msk144 ft8
                    else id_rpt = "1";
                }
                block_rpt_all = false;
            }
            else if (!rpt_inmsg.isEmpty() && rpt_inmsg.at(0)!='R' && rpt_inmsg.at(rpt_inmsg.count()-1).isDigit() && rpt_inmsg!="73")
            {//msg=1 +00
                if (!cont_r_inmsg.isEmpty()) id_rpt = "3";
                else id_rpt = "2";
                rpt_inmsg = format_rpt_ma(rpt_inmsg);
                block_rpt_all = false;
            }
            else if (!rpt_inmsg.isEmpty() && rpt_inmsg.at(0)=='R' && rpt_inmsg.at(rpt_inmsg.count()-1).isDigit())
            {//msg=2 R+00
                id_rpt = "3";
                rpt_inmsg.remove("R");
                rpt_inmsg = format_rpt_ma(rpt_inmsg);

                //if(f_multi_answer_mod_std && s_id_con>0)
                block_rpt_all = false;// i ot tx2=1 i ot tx3=2 se gubi lokatora pri std taka 4e puskaj gi
                //else
                //block_rpt_all = true;
            }
            else if (!rpt_inmsg.isEmpty() && rpt_inmsg.at(0)=='R' && !rpt_inmsg.at(rpt_inmsg.count()-1).isDigit() &&
                     rpt_inmsg.at(rpt_inmsg.count()-1)=='R')
            {//msg=3 RRR
                id_rpt = "3";
                block_rpt_all = true;
            }
            else if ((!rpt_inmsg.isEmpty() && rpt_inmsg=="73") || rr73_inmsg=="RR73")//1.54=RR73
            {//msg=4 73 or RR73
                //if (f_multi_answer_mod_std) id_rpt = "4";//2.72 stop
                //else id_rpt = "3";//2.72 stop
                id_rpt = "4";//2.72
                block_rpt_all = true;
            }
        }
        else if (f_multi_answer_mod_std && myCall_inmsg.isEmpty() && f_double_click)//2.23
        {//msg=9 click to someone else
            if (s_start_qso_from_tx2 && s_co_type!=3) id_rpt = "1";//2.31 no in eu vhf
            else id_rpt = "0";
            block_rpt_all = false;
            if (!rpt_inmsg.isEmpty() && rpt_inmsg.at(0)=='R' && rpt_inmsg.at(rpt_inmsg.count()-1).isDigit())
                rpt_inmsg.remove("R");
        }
        else return;

        if (id_mshf==2 && id_rpt=="2") return;//2.76sf no answer to, mycall call +02
        if (id_0qe_1nw_2ne == 2 && block_rpt_all) return; //MAM no posible newcand with rpt-> RRR RR73 73
        /*QString tr0 = "";
        if (id_0qe_1nw_2ne == 2 && block_rpt_all) 
        {
        	//if (FindCallBackupDB(hisCall_inmsg)==-1) return;
        	if (!f_double_click || !f_multi_answer_mod_std) return;
        	id_rpt = "3";
        	tr0 = gen_in_tx_time;
        	//if (f_tx_rx) tr0 = "1";
        }*/

        bool is_checked = false;//2.66
        int c_que_limit = SBqueueLimit->value(); //2.39 no Queue
        if (c_que_limit == 0)//2.39 no Queue   id_0qe_1nw_2ne -> 0=Queue,1=Now,2=New
        {
            bool f_aseqmd = false;
            if (f_aseqmaxdist && LsQueue->GetRowCount()==0 && SBslots->valueS()==1 && id_0qe_1nw_2ne==2)//2.66  && !TQueuedCall->haveQueuedCall() ???
            {
                if (!myCall_inmsg.isEmpty() && !hisCall_inmsg.isEmpty() && is_LastTxRptCq)//&& !hisLoc_inmsg.isEmpty()
                {
                    //int ds = MultiAnswerMod->CalcDistance(hisLoc_inmsg.mid(0,4),false).toInt();//false=kilometers
                    //int po=ds/500;  //ARRL Inter. Contest rules points, bad for VHF
                    //if (ds>500*po) po++;
                    //po++;
                    int po = -1;
                    if (hisLoc_inmsg.isEmpty()) po = 0; //<mycall> hiscall no loc
                    else po = CalcDistance(hisLoc_inmsg,false).toInt();//good for VHF false=kilometers
                    if (po > s_dist_points)
                    {
                        /*QString s;
                        if (LsNow->GetRowCount()>0) s = LsNow->model.item(0,0)->text();
                         		qDebug()<<"Change="<<s<<s_dist_points<<"to"<<hisCall_inmsg<<po;*/
                        s_dist_points = po;
                        if (LsNow->GetRowCount()==1) f_aseqmd = true;
                    }
                }
            }
            if ((f_double_click || f_aseqmd) && id_0qe_1nw_2ne!=1 && id_0qe_1nw_2ne!=0 && LsQueue->GetRowCount()==0
                    && SBslots->valueS()==1 && LsNow->GetRowCount()==1) //2.59 all on the fly
            {
                if (f_multi_answer_mod_std && info_dupe_qso && f_double_click) //2.66=?
                {
                    is_checked = true;
                    if (DialogIsCallDupeInLog(hisCall_inmsg)) return;
                }
                ClrNowUserDoubleClick();
            }
            if (LsNow->GetRowCount() < SBslots->valueS()) c_que_limit = 1;
        }
        if (id_0qe_1nw_2ne == 2 && LsQueue->GetRowCount() >= c_que_limit) return; //2.39 no Queue, new and no have place

        if (!is_checked && f_multi_answer_mod_std && info_dupe_qso && f_double_click) //2.66=?
        {
            if (DialogIsCallDupeInLog(hisCall_inmsg)) return;
        }

        QString dist = "?";
        if (!hisLoc_inmsg.isEmpty()) dist = CalcDistance(hisLoc_inmsg,f_km_mi);

        if (rpt_inmsg=="73" || rpt_inmsg=="RRR" || rpt_inmsg=="RR73") rpt_inmsg = "";//no update HisRxRptForMe

        //if (id_rpt == "9" || id_rpt == "0" || id_rpt == "1")//2.22  1.80 no update MyTxRptForHim
        if (id_rpt == "0" || id_rpt == "1" || id_rpt == "2")//2.22  1.80 no update MyTxRptForHim
        {
            /*if (f_multi_answer_mod_std && (s_co_type==3 || s_co_type==5))
                tx_rpt = FormatRPTtoRST(tx_rpt);*/
            if (f_multi_answer_mod_std) tx_rpt = TryFormatRPTtoRST(tx_rpt); //2.59
            tx_rpt = format_rpt_ma(tx_rpt);
        }
        else tx_rpt = "";
        //qDebug()<<"2="<<hisCall_inmsg<<"tx_rpt="<<tx_rpt;

        //if (id_0qe_1nw_2ne == 1) tx_rpt = "";//exist no update

        if (id_0qe_1nw_2ne == 1)     //first upd if Now 1=Now
        {
            QString prev_id_rpt = LsNow->model.item(row_now,7)->text();//qDebug()<<prev_id_rpt<<id_rpt;
            if (hisCall_inmsg.contains("/")) LsNow->SetItem(row_now,0,hisCall_inmsg);//1.73 update if have slash during QSO
            if (!tx_rpt.isEmpty()) LsNow->SetItem(row_now,1,tx_rpt);//1.80 no update MyTxRptForHim
            if (!rpt_inmsg.isEmpty()) LsNow->SetItem(row_now,2,rpt_inmsg);
            if (dist!="?") LsNow->SetItem(row_now,3,dist);
            if (!hisLoc_inmsg.isEmpty()) LsNow->SetItem(row_now,4,hisLoc_inmsg);
            LsNow->SetItem(row_now,5,freq);
            LsNow->SetItem(row_now,7,id_rpt);
            QString ttry = QString("%1").arg(QDateTime::currentDateTimeUtc().toTime_t());
            LsNow->SetItem(row_now,8,ttry);//reset time
            if (!sn_inmsg.isEmpty()) LsNow->SetItem(row_now,10,sn_inmsg);
            if (!arrl_exch_imsg.isEmpty()) LsNow->SetItem(row_now,11,arrl_exch_imsg);

            //qDebug()<<"C_msg="<<current_msg<<current_msg.mid(current_msg.count()-4,4);
            //qDebug()<<"RX= "<<LsNow->model.item(row_now,0)->text()<<"prev_id_rpt"<<prev_id_rpt<<"id_rpt"<<id_rpt;
            if (f_multi_answer_mod_std && s_co_id==15 && prev_id_rpt == "0" && id_rpt == "3")
            {
                LsNow->SetItem(row_now,9,"0");//1=tx 0=rx
                SetTxMsgEnd();
            }
            else if (f_tx_rx && SBslots->valueS()==1 && (id_rpt == "4" || id_rpt == "3"))//2.66
            {//if (f_tx_rx && (s_mode==11 || s_mode==13 || s_mode==18 || allq65) && SBslots->valueS()==1 && (id_rpt == "4" || id_rpt == "3"))
                //if (f_tx_rx && f_auto_on && SBslots->valueS()==1 && s_mode==11 && (id_rpt == "4" || id_rpt == "3"))
                //LsNow->SetItem(row_now,7,"3");//2.72 stop
                LsNow->SetItem(row_now,9,"0");//1 tx 0 rx gen_in_tx_time
                if (!f_cfm73 && id_rpt == "4") SetTxMsgEnd();
                else gen_msg();
                //qDebug()<<"000SetTxMsgEnd()"<<gen_in_tx_time<<id_rpt;
            }
            //else if (!f_cfm73 && prev_id_rpt == "2" && id_rpt == "4")//old 2.70  no needed -> f_multi_answer_mod_std &&
            else if (!f_cfm73 && (prev_id_rpt == "2" || prev_id_rpt == "3") && id_rpt == "4")//2.70
            {
                //LsNow->SetItem(row_now,7,"3");//2.72 stop out id for SetTxMsgEnd();
                LsNow->SetItem(row_now,9,"0");//1 tx 0 rx
                SetTxMsgEnd();
                //qDebug()<<"111SetTxMsgEnd()"<<gen_in_tx_time;
            }
            //else if (f_cfm73 && prev_id_rpt == "2" && id_rpt == "4")//old 2.47
            else if (f_cfm73 && (prev_id_rpt == "2" || prev_id_rpt == "3") && id_rpt == "4")//2.48 no needed -> f_multi_answer_mod_std &&
            {
                //LsNow->SetItem(row_now,7,"3");//2.72 stop out id for SetTxMsgEnd();
                gen_msg();
                //qDebug()<<"222SetTxMsgEnd()"<<gen_in_tx_time<<prev_id_rpt<<id_rpt;
            }
            else gen_msg();
        }
        else if (id_0qe_1nw_2ne == 0)//second upd if Queue 0=Queue
        {
            if (hisCall_inmsg.contains("/")) LsQueue->SetItem(row_queue,0,hisCall_inmsg);//1.73 update if have slash during QSO
            if (!tx_rpt.isEmpty()) LsQueue->SetItem(row_queue,1,tx_rpt);//1.80 no update MyTxRptForHim
            if (!rpt_inmsg.isEmpty()) LsQueue->SetItem(row_queue,2,rpt_inmsg);
            if (dist!="?") LsQueue->SetItem(row_queue,3,dist);
            if (!hisLoc_inmsg.isEmpty()) LsQueue->SetItem(row_queue,4,hisLoc_inmsg);
            LsQueue->SetItem(row_queue,5,freq);
            LsQueue->SetItem(row_queue,7,id_rpt);
            if (!sn_inmsg.isEmpty()) LsQueue->SetItem(row_queue,10,sn_inmsg);
            if (!arrl_exch_imsg.isEmpty()) LsQueue->SetItem(row_queue,11,arrl_exch_imsg);
            if (id_mshf==2 && c_sf_rpt>=4 && c_sf_r73<5 && id_rpt=="3" && !s_msf_ftmsg) gen_msg();//2.76sf 9-slots
        }
        else if (id_0qe_1nw_2ne == 2)//thirt upd  2=New
        {
            //"Call"<<"dB"<<"Rx dB"<<"Dist"<<"Grid"<<"Freq"<<"Time"<<"IDrpt"<<"Try"<<"GinTxT"<<s<<e;
            QStringList lss;
            if (!rpt_inmsg.isEmpty()) rpt_inmsg = format_rpt_ma(rpt_inmsg);
            //QString g_tx_t = "";
            //if (f_multi_answer_mod_std && s_co_id==15 && id_rpt=="2") g_tx_t = "0";
            lss<<hisCall_inmsg<<tx_rpt<<rpt_inmsg<<dist<<hisLoc_inmsg<<freq<<""<<id_rpt<<""<<""<<sn_inmsg<<arrl_exch_imsg;
            LsQueue->InsertItem_hv(lss); //qDebug()<<"IN= "<<hisCall_inmsg<<"id_rpt="<<id_rpt<<"g_tx_t="<<g_tx_t;
            RefreshLists(0);//qDebug()<<"Aa start RefreshLists0";
        }
        if (f_double_click)
        {
            s_last_bccall_tolog_excp = "_NONE_";//reset add to log if I want to make QSO again
            emit EmitDoubleClick();
        }
    }
}
//#define _TEST_MASF_
#if defined _TEST_MASF_
#include <unistd.h>//for usleep
#endif
void MultiAnswerModW::SetTextForAutoSeq(QStringList list_in)
{
#if !defined _TEST_MASF_
    if (!list_in.isEmpty() && (s_mode==11 || s_mode==13 || s_mode==18 || allq65) && list_in.count()>9)//2.73 Dist Country
    {
        QString text_msg = list_in.at(4);
        QString tx_rpt = list_in.at(1);
        QString freq = list_in.at(9);//2.73 Dist Country
        QString hcap;//fictive
        QString hloc;//fictive
        DecListTextAll(tx_rpt,text_msg,freq,false,hcap,hloc);//false f_double_click
    }
#else
    static int uuu = 0;
    QStringList l;
    QString mc  = list_macros.at(0);//"LZ2HV";
    QString pf0 = "AA";
    QString pf1 = "BB";
    QString sf  = "TT";
    QString loc = "JO66";
    QString rpt = "R+02";
    int crpt = 10;
    for (int i = 0; i < crpt; ++i)
    {
        int i0 = i;
        sf = "TT";
        if ((i % 2)==0) sf = "TT";
        if (i>9)
        {
            i0 = i - 10;
            sf = "VV";
        }
        if (uuu==0) l<<mc+" "+pf0+QString("%1").arg(i0)+sf+" "+loc;
        if (uuu==1)
        {
            if (i<crpt/2) l<<mc+" "+pf0+QString("%1").arg(i0)+sf+" "+rpt;
            else l<<mc+" "+pf0+QString("%1").arg(i0)+sf+" "+loc;
        }
        if (uuu==2)
        {
            if (i<crpt/2) l<<mc+" "+pf1+QString("%1").arg(i0)+sf+" "+loc;
            else l<<mc+" "+pf0+QString("%1").arg(i0)+sf+" "+rpt;
        }
        if (uuu==3)
        {
            if (i<crpt/2) l<<mc+" "+pf1+QString("%1").arg(i0)+sf+" "+rpt;
            else l<<mc+" "+pf1+QString("%1").arg(i0)+sf+" "+loc;
        }
        if (uuu==4)
        {
            if (i<crpt/2)
            {}
            else l<<mc+" "+pf1+QString("%1").arg(i0)+sf+" "+rpt;
        }
    }
    if (!list_in.isEmpty() && (s_mode==11 || s_mode==13 || s_mode==18 || allq65) && list_in.count()>9)//2.73 Dist Country
    {
        QString text_msg = list_in.at(4);
        QString tx_rpt = list_in.at(1);
        QString freq = list_in.at(9);//2.73 Dist Country
        QString hcap;//fictive
        QString hloc;//fictive
        for (int i = 0; i < l.count(); ++i)
        {
            text_msg = l.at(i);//qDebug()<<"RX="<<text_msg;
            DecListTextAll(tx_rpt,text_msg,freq,false,hcap,hloc);//false f_double_click
            usleep(10000);
        } //qDebug()<<"-------------------------------------------------------------------------";
    }
    if (uuu<4) uuu++;
    else uuu=0;
#endif
}



