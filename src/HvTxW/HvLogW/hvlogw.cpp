/* MSHV HvLogW for Log program
 * Copyright 2015 Hrisimir Hristov, LZ2HV
 * May be used under the terms of the GNU General Public License (GPL)
 */
#include "hvlogw.h"
//#define _S0SATIDN_
#include "config_prop_all.h"
#define _BANDS_H_
#define _LAMBDA_H_
#define _FREQTOBAND_H_
#define _BANDTOFREQ_H_
#include "../../config_band_all.h"
#include "../../config_str_all.h"

//#include <QtGui>
//#include <limits>
#include <QCoreApplication>
HvProgressD::HvProgressD(QWidget * parent)
        : QProgressDialog(parent)
{
    //QProgressDialog *pd = new QProgressDialog("Operation in progress.", "Cancel", 0, 100);
    c_barr = 0;
    max_range = 510;
    //setWindowModality(Qt::WindowModal);// very slow
    setWindowModality(Qt::NonModal);// fast ok
    //setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::CustomizeWindowHint);// no buttons
    setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::CustomizeWindowHint | Qt::WindowCloseButtonHint);// + close
    setCancelButton(0);
    setRange(0,max_range);
    s_label = "Wait";
    //setStyleSheet( "QProgressBar{border:2px solid grey; border-radius:5px; text-align: center;}"
    //"QProgressBar::chunk {background-color: #d9d9d9; width: 15px;}"); //bcc9e6 05B8CC margin: 0.5px;<-nakysva go*/

    /*QString pbs ="QProgressBar{background: #bdc1c9; border: 1px solid #b6b6b6; text-align: center; padding: 1px; border-radius: 4px;}"
    	"QProgressBar::chunk{background-color: qlineargradient(spread:pad, x1:1, y1:0.545, x2:1, y2:0, stop:0 #3874f2, stop:1 #5e90fa);border-radius: 3px;}";*/
    /*QString pbs ="QProgressBar{background: #f3f2ff; border: 1px solid #b6b6b6; text-align: center; padding: 1px; border-radius: 4px;}"
    			"QProgressBar::chunk{background-color: qlineargradient(spread:pad, x1:1, y1:0.545, x2:1, y2:0, stop:0 #3874f2, stop:1 #5e90fa);border-radius: 3px;}";	
    setStyleSheet(pbs);	*/
}
HvProgressD::~HvProgressD()
{}
void HvProgressD::Show(QString title,QString label,int range)
{
    c_barr = 0;
    s_label = label;
    max_range = range;
    setRange(0,max_range);
    setWindowTitle(title);
    setLabelText(s_label);
    show();
    QCoreApplication::processEvents();// qt5
}
void HvProgressD::SetValue(int i)
{
    if (c_barr>max_range-5)
        c_barr = 0;
    else
        c_barr++;
    setLabelText(s_label+QString("%1").arg(i));
    setValue(c_barr);
}
void HvProgressD::Finish(int i)
{
    setLabelText(s_label+QString("%1").arg(i));
    setValue(max_range-2);//2.68 -2
}

#include <QApplication>
HvLogList::HvLogList(bool indsty,QWidget *parent)
        : QTreeView(parent)
{
    setMinimumSize(795, 200);
    //this->set
    last_logged_dt = "_";
    enum_sec_logged = 0;

    setRootIsDecorated(false);
    setModel(&model);
    model.SetValidSortColumns(21,-12);
    //THvHeader = new HvHeaderLog(Qt::Horizontal);
    THvHeader = new QHeaderView(Qt::Horizontal);


    connect(THvHeader, SIGNAL(sectionPressed(int)),this,SIGNAL(SortClicked(int)));

    setHeader(THvHeader);
    setAllColumnsShowFocus(true);  // za da o4ertae delia row

    //setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    //setSelectionMode(QAbstractItemView::ExtendedSelection);//QAbstractItemView::SingleSelection


    setDragDropMode(QAbstractItemView::NoDragDrop);//QAbstractItemView::InternalMove
    setDragEnabled (false);
    // setAcceptDrops(true);
    setVerticalScrollMode(QAbstractItemView::ScrollPerItem);// tova e za na mi6kata whella i dragdrop QAbstractItemView::ScrollPerPixel
    setAutoScroll(true); //for lz2pr s tova tarsi samo v 1 colona i za scroll pri drag
    //setAutoScroll(false);

    QPalette PaletteL = palette();
    PaletteL.setColor(QPalette::Shadow, QColor(120,120,120));// zaradi bordera okantovaneto na lista
    setPalette(PaletteL);

    QStringList list_A;
    //list_A <<"Date"<<"UTC Start"<<"Date End"<<"UTC End"<<"Callsign"<<"Locator"<<"TXRpt"<<"RXRpt"<<"Mode"
    //<<"Band"<<"Freq kHz"<<"Prop"<<"Comment";
    list_A <<tr("Date")<<tr("Start")<<"Date End"<<tr("End")<<tr("Callsign")<<tr("Locator")<<"TXRpt"<<"RXRpt"<<tr("Mode")
    <<tr("Band")<<"Freq kHz"<<tr("Prop")<<tr("Comment")<<"secSta"<<"TXSn"<<"RXSn"<<"TXExch"<<"RXExch"<<"Cid"
    <<"Cmt"<<"secDnd"<<tr("Distance")<<tr("Satellite")<<tr("Sat Mode")<<"RX Freq MHz";
    model.setHorizontalHeaderLabels(list_A);

    THvHeader->setSectionResizeMode(QHeaderView::Fixed);//qt5

    HideSections();
    /*THvHeader->hideSection(2);  //date end  hiden
    THvHeader->hideSection(13); //enum seconds 00-59 hiden
    THvHeader->hideSection(18); //contest id hiden
    THvHeader->hideSection(19); //contest MULTI-TWO hiden
    THvHeader->hideSection(20); //enum seconds end 00-59 hiden*/

    //THvHeader->setResizeMode(2, QHeaderView::Fixed);
    THvHeader->resizeSection(0, 80);//0  //78 date start 125%
    THvHeader->resizeSection(1, 60);//1  //  70 utc time start 125%
    THvHeader->resizeSection(2, 78);//2  //78 date end  hiden
    THvHeader->resizeSection(3, 60);//3  //  65 utc time end 125%
    THvHeader->resizeSection(4, 80);//4  // call
    THvHeader->resizeSection(5, 75);//5  //70 loc
    THvHeader->resizeSection(6, 50);//6  // txrpt
    THvHeader->resizeSection(7, 50);//7  // rxrpt
    THvHeader->resizeSection(8, 68);//8  // mode 125%
    THvHeader->resizeSection(9, 75);//9  //100 75 band 125%
    THvHeader->resizeSection(10, 70);//10  //freq
    THvHeader->resizeSection(11, 50);//11 // Propagation
    THvHeader->resizeSection(12, 100);//12 // commemt
    THvHeader->resizeSection(13, 25);//enum seconds 00-59 hiden      		13
    THvHeader->resizeSection(14, 45);//my tx sn contest               	    14
    THvHeader->resizeSection(15, 45);//my rx sn contest               	    15
    THvHeader->resizeSection(16, 55);//my tx exch contest            		16
    THvHeader->resizeSection(17, 55);//my rx exch contest           		17
    THvHeader->resizeSection(18, 20); //contest id hiden                    18
    THvHeader->resizeSection(19, 20); //contest MULTI-TWO hiden             19
    THvHeader->resizeSection(20, 20); //enum seconds end 00-59 hiden        20
    THvHeader->resizeSection(21, 75); //distance					        21
    THvHeader->resizeSection(22, 74); //Satellite					        22
    THvHeader->resizeSection(23, 72); //Sat Mode					        23
    THvHeader->resizeSection(24, 90); //RX Freq MHz					        24
    //THvHeader->resizeSection(25, 75); //reserve					        25
    //THvHeader->resizeSection(26, 75); //reserve					        26
    //THvHeader->resizeSection(27, 75); //reserve					        27
    //THvHeader->resizeSection(28, 75); //reserve					        28

    //THvHeader->moveSection(21,6);
    //THvHeader->moveSection(21,11);
    THvHeader->moveSection(21,8);
    THvHeader->moveSection(22,14);
    THvHeader->moveSection(23,15);
    THvHeader->moveSection(24,16);  //qDebug()<<model.columnCount();

    //THvHeader->setSectionResizeMode(4, QHeaderView::ResizeToContents);//no work bug in qt5.7
    /*THvHeader->setSectionResizeMode(0, QHeaderView::Stretch);//1.81 for font
       THvHeader->setSectionResizeMode(5, QHeaderView::Stretch);//1.81 for font
       THvHeader->setSectionResizeMode(8, QHeaderView::Stretch);//1.81 for font
       THvHeader->setSectionResizeMode(9, QHeaderView::Stretch);//band 1.81 for font
       THvHeader->setSectionResizeMode(10, QHeaderView::Stretch);//1.81 for font
       //THvHeader->setSectionResizeMode(11, QHeaderView::Stretch);//Propagation 1.81 for font

       THvHeader->setSectionResizeMode(4, QHeaderView::Stretch);// qt5.7*/
    //THvHeader->setSectionResizeMode(12, QHeaderView::Stretch);//qt5 comment
    THvHeader->setSectionResizeMode(QHeaderView::Interactive);//2.01
    //THvHeader->setSectionResizeMode(QHeaderView::Stretch);//2.01

    //THvHeader->resizeSection(8, 35);
    //THvHeader->resizeSection(9, 46);
    //THvHeader->resizeSection(10, 70);
    //THvHeader->resizeSection(11, 70);

    setSortingEnabled(true);
    connect(&model,SIGNAL(EmitSortEnd()),this,SLOT(HideSections()));//2.57

    //setSelectionMode(QAbstractItemView::SingleSelection);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    //2.31 false for "\n" jt6m=6 jtms=1
    setUniformRowHeights(true);//2.30 fast wiev

    ActiveIndex = -1;

    if (indsty)
    {
        ActiveRowText = QColor(255,250,250);
        ActiveRowBackg = QColor(0,150,0);
    }
    else
    {
        ActiveRowText = QColor(255,0,0);
        ActiveRowBackg = QColor(0,255,0);
    }
    clipboard = QApplication::clipboard();
}
HvLogList::~HvLogList()
{}
void HvLogList::HideSections()
{
    THvHeader->hideSection(2);  //date end  hiden
    THvHeader->hideSection(13); //enum seconds 00-59 hiden
    THvHeader->hideSection(18); //contest id hiden
    THvHeader->hideSection(19); //contest MULTI-TWO hiden
    THvHeader->hideSection(20); //enum seconds end 00-59 hiden
}
void HvLogList::paintEvent(QPaintEvent *event)
{
    QPainter painter(viewport());
    QModelIndex index = model.index(ActiveIndex, 1, QModelIndex());
    painter.fillRect(QRect(0,visualRect(index).y(),width(),visualRect(index).height()), ActiveRowBackg);
    QTreeView::paintEvent(event);
}
void HvLogList::drawRow(QPainter *painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const
{
    QStyleOptionViewItem opt = option;

    if (index.row() == ActiveIndex)
        opt.palette.setColor(QPalette::HighlightedText, ActiveRowText);
    else
        opt.palette.setBrush(QPalette::HighlightedText, palette().highlightedText());

    if (index.row() == ActiveIndex)
        opt.palette.setColor(QPalette::Text, ActiveRowText);
    else
        opt.palette.setBrush(QPalette::Text, palette().text());

    if (index.row() == ActiveIndex)
        opt.palette.setColor(QPalette::Highlight, ActiveRowBackg);
    else
        opt.palette.setBrush(QPalette::Highlight, palette().highlight());

    QTreeView::drawRow(painter, opt, index);
}
void HvLogList::SetEditRow(int numbr)
{
    ActiveIndex = numbr;
    viewport()->update();
}
void HvLogList::mouseDoubleClickEvent(QMouseEvent * event)
{
    if (event->button() == Qt::LeftButton)
        emit EmitDoubleClick();
    QTreeView::mousePressEvent(event);
}
void HvLogList::InsertItem_hv(QStringList list)
{
    if (!list.isEmpty())
    {
        QList<QStandardItem *> qlsi;
        int collumn = 0;
        for (QStringList::iterator it =  list.begin(); it != list.end(); ++it)
        {
            QString itxt = QString(*it);
            if (!itxt.isEmpty()) itxt = itxt.trimmed();//2.63 protection

            if (collumn==13 && itxt.isEmpty())//1.82 enum seconds 00-59;
            {
                QString dt_start = list.at(0)+list.at(1);
                if (dt_start == last_logged_dt)
                {
                    if (enum_sec_logged<59)
                        enum_sec_logged++;
                }
                else
                    enum_sec_logged = 0;
                itxt = QString("%1").arg(enum_sec_logged,2,10,QChar('0'));
                last_logged_dt = dt_start;
            }
            if (collumn==20 && itxt.isEmpty())//2.41 enum seconds end 00-59;
            {
                itxt = QString("%1").arg(enum_sec_logged,2,10,QChar('0'));
            }
            if (collumn==10)//freq 1.50
            {
                int k = itxt.count();
                k -= 3;
                if (k>0) itxt.insert(k,".");//KHz 1.50
                else if (!itxt.isEmpty()) itxt.prepend("0.");//2.31
            }
            QStandardItem *item = new QStandardItem(itxt);
            //if (collumn==21) item->setTextAlignment(Qt::AlignRight); //res=res.rightJustified(5,' ');
            item->setEditable(false);
            qlsi.append(item);
            collumn++;
        }
        //model.insertRow(model.rowCount(), qlsi);
        model.appendRow(qlsi);
        //this->setCurrentIndex(model.index(model.rowCount()-1,0));
        //this->scrollTo(model.index(model.rowCount()-1,0));
    }
}
void HvLogList::SetItem_hv(QStringList list,int index_edit)
{
    //int column_numb = 0;
    //for (QStringList::iterator it =  list.begin(); it != list.end(); ++it)
    for (int j = 0; j < list.count(); ++j)
    {
        // end bug qt5.7 if setItem 0 row show hiden columns
        bool isHidenCol = THvHeader->isSectionHidden(j);
        QString itxt = list.at(j);//QString(*it);
        if (!itxt.isEmpty()) itxt = itxt.trimmed();//2.63 protection

        if (j==10)//freq 1.50
        {
            int k = itxt.count();
            k -= 3;
            if (k>0) itxt.insert(k,".");//KHz 1.50
            else if (!itxt.isEmpty()) itxt.prepend("0.");//2.31
        }

        //QStandardItem *item = new QStandardItem(itxt);
        QStandardItem *item = model.itemFromIndex(model.index(index_edit, j));
        item->setText(itxt);
        //item->setEditable(false);
        //item->setTextAlignment(Qt::AlignLeft);
        model.setItem(index_edit,j,item);
        // end bug qt5.7 if setItem 0 row show hiden columns
        if (isHidenCol) THvHeader->hideSection(j);
        //column_numb++;
    }
    setCurrentIndex(model.index(index_edit, 1, QModelIndex()));// ina4e ne refreshva 1 row
}
void HvLogList::Clear_List()
{
    for (int i = model.rowCount()-1; i >= 0; --i) model.removeRow(i);// ok pravilno hv
}
void HvLogList::DeleteSel()
{
    if (!selectionModel()->selection().empty())
    {
        setAutoScroll(false); //2.21 bez towa bavi za da ne skrolira pri iztrivane kam poslednia markiran
        QItemSelection selection( selectionModel()->selection() );

        QList<int> rows;
        foreach( const QModelIndex & index, selection.indexes() )
        {
            rows.append( index.row() );
        }

        //q_Sort(rows);
        std::sort(rows.begin(),rows.end());

        int prev = -1;
        for ( int i = rows.count() - 1; i >= 0; i -= 1 )
        {
            int current = rows[i];
            if ( current != prev )
            {
                model.removeRows( current, 1 );
                prev = current;
            }
        }
        setAutoScroll(true);  //2.21 bez towa bavi za da ne skrolira pri iztrivane kam poslednia markiran
    }
}
void HvLogList::keyPressEvent(QKeyEvent * event)
{
    if (event->matches(QKeySequence::Copy) && !selectionModel()->selection().empty() )
    {
        QModelIndexList lst = selectionModel()->selectedRows(0);
        QList<int> rows;
        foreach( const QModelIndex & index, lst )
        {
            rows.append( index.row() );
        }
        //q_Sort(rows);
        std::sort(rows.begin(),rows.end());
        QString str_n;
        for (int j = 0; j < rows.count(); j++)
        {

            for (int i = 0; i < model.columnCount(); i++)
            {
                str_n.append(model.item(rows[j], i)->text());
                if (i < model.columnCount()-1)
                    str_n.append(" ");
            }
            if (j < rows.count()-1)
                str_n.append("\n");
        }
        clipboard->setText(str_n);
    }
    else //vazno da e else ina4e kopira samo edna kletka
        QTreeView::keyPressEvent(event); // za da raboti keybord shortkeys Application
}

#include <QDateEdit>
#include <QPushButton>
#define _POS_CONT_
#define _CONT_NAME_
#define _CONT_NAMEID_
#include "../../config_str_con.h"
#include "../../config_str_exc.h"
HvEditW::HvEditW(QString name,QString bt_cancel_txt,QString bt_corr_txt,bool indsty,QWidget * parent )
        : QWidget(parent)
{
    s_dist = "";
    s_enum_sec_sta = "";
    s_enum_sec_end = "";
    index_edit = -1;
    next_dupe = -1;
    block_txrst = true;
    block_rxrst = true;
    block_rxsn = true;
    block_call = true;
    block_loc = true;
    block_dupe = false;// ina4e vednaga pokazva dupe
    first_dupe_flag = false;
    first_dupe = -1;

    QLabel *l_e = new QLabel();
    if (indsty) l_e->setText("<h3><font color='#00c800'>"+name);//EDIT QSO
    else l_e->setText("<h3><font color=green>"+name);//EDIT QSO
    //l_e->setFixedWidth(140);

    QLabel *l_d_s = new QLabel();
    l_d_s->setText(tr("Date UTC Start")+":");
    //l_d_s->setFixedWidth(102);//125%
    dte_s = new QDateTimeEdit();
    dte_s->setDisplayFormat("yyyy-MM-dd  hh:mm");
    dte_s->setMinimumWidth(90);

    QLabel *l_d_e = new QLabel();
    l_d_e->setText(tr("End")+":");
    //l_d_e->setFixedWidth(28);//125%
    dte_e = new QDateTimeEdit();
    dte_e->setDisplayFormat("yyyy-MM-dd  hh:mm");
    dte_e->setMinimumWidth(90);

    QLabel *l_m = new QLabel();
    l_m->setText(tr("Mode")+":");
    //l_m->setFixedWidth(35);
    Cb_mode = new QComboBox();
    QStringList lst_m;

    for (int i = 0; i < COUNT_MODE; ++i) lst_m<<ModeStr(pos_mods[i]);//2.67
    //NO DG MODES
    lst_m<<"SSB"<<"CW"<<"FM";

    Cb_mode->addItems(lst_m);
    Cb_mode->setCurrentIndex(0);
    Cb_mode->setFixedWidth(95);//1.81  1.50 85 75
    QLabel *l_ba = new QLabel();
    l_ba->setText(tr("Band")+":");
    //l_ba->setFixedWidth(35);//1.50 38
    Cb_band = new QComboBox();
    QStringList lst_b;
    for (int i = 0; i<COUNT_BANDS; ++i) lst_b << lst_bands[i];
    Cb_band->addItems(lst_b);
    Cb_band->setCurrentIndex(0);
    Cb_band->setFixedWidth(105);//1.81   1.50 95

    QHBoxLayout *H_dtm = new QHBoxLayout();
    H_dtm->setContentsMargins(1,1,1,1);
    H_dtm->setSpacing(3);
    H_dtm->addWidget(l_d_s);
    //H_dtm->setAlignment(l_d_s,Qt::AlignLeft);
    H_dtm->addWidget(dte_s);
    //H_dtm->setAlignment(dte_s,Qt::AlignLeft);
    //H_dtm->addWidget(de_bdate_s);
    //H_dtm->addWidget(l_c_s);
    //H_dtm->addWidget(te_time_s);
    H_dtm->addWidget(l_d_e);
    H_dtm->addWidget(dte_e);
    //H_dtm->addWidget(de_bdate_e);
    //H_dtm->addWidget(l_c_e);
    //H_dtm->addWidget(te_time_e);
    H_dtm->addWidget(l_m);
    //H_dtm->setAlignment(l_m,Qt::AlignLeft);
    H_dtm->addWidget(Cb_mode);
    //H_dtm->setAlignment(Cb_mode,Qt::AlignLeft);
    H_dtm->addWidget(l_ba);
    //H_dtm->setAlignment(l_ba,Qt::AlignLeft);
    H_dtm->addWidget(Cb_band);

    QSpacerItem *space = new QSpacerItem(1,1,QSizePolicy::Expanding);
    H_dtm->addSpacerItem(space);
    //H_dtm->setAlignment(Cb_band,Qt::AlignLeft);

    le_call = new HvInLe("call", tr("CALLSIGN"),indsty);
    connect(le_call, SIGNAL(EmitSndCheck(QString)), this, SLOT(Check(QString)));
    //le_call->setFixedWidthLine(95);
    //le_call->SetMask(">XXXXXXXXXXXXXXXXXXXXXXXX");
    //connect(le_call, SIGNAL(SndCheck(QString)), this, SLOT(Check(QString)));
    //le_call->SndCheck_s("");// proverka v na4aloto
    le_txrst = new HvInLe("txrst","TXRpt",indsty);
    le_txrst->setMaxLength(3);
    //le_txrst->SetMask("#99");//1.41 stop for OOO in jt65 + sh
    le_txrst->setFixedWidth(92);//125%
    //connect(le_txrst, SIGNAL(SndCheck(QString)), this, SLOT(Check(QString)));
    //le_txsn = new HvInLe("txsn", "TxSn");
    //le_txsn->SetMask("99999");
    //le_txsn->setFixedWidth(55);
    //le_txsn->setReadOnly(true);
    le_rxrst = new HvInLe("rxrst","RXRpt",indsty);
    le_rxrst->setMaxLength(3);
    //le_rxrst->SetMask("#99");//1.41 stop for OOO in jt65 + sh
    le_rxrst->setFixedWidth(92);//125%
    //connect(le_rxrst, SIGNAL(SndCheck(QString)), this, SLOT(Check(QString)));
    //le_rxsn = new HvInLe("rxsn", "RxSn");
    //le_rxsn->SetMask("99999");
    //le_rxsn->setFixedWidth(55);
    //connect(le_rxsn, SIGNAL(SndCheck(QString)), this, SLOT(Check(QString)));
    //le_rxsn->SndCheck_s("");// proverka v na4aloto
    le_loc = new HvInLe("locator", tr("LOCATOR"),indsty);
    le_loc->SetMask(">AA99AA");
    le_loc->setFixedWidth(150);//145 125%
    connect(le_loc, SIGNAL(EmitSndCheck(QString)), this, SLOT(Check(QString)));
    //le_loc->SndCheck_s("");// proverka v na4aloto

    QLabel *l_prop = new QLabel();
    l_prop->setText(tr("Propagation")+":");
    //l_prop->setFixedWidth(34);
    cb_prop = new QComboBox();
    cb_prop->setFixedWidth(160);
    QStringList lst_prop;
    for (int i = 0; i<COUNT_PROP; ++i) lst_prop<<s_prop_mod[i];
    cb_prop->addItems(lst_prop);
    cb_prop->setCurrentIndex(0);
    connect(cb_prop, SIGNAL(currentIndexChanged(QString)), this, SLOT(PropChanged(QString)));

    QLabel *l_satmod = new QLabel();
    l_satmod->setText(tr("Sat Mode")+":");
    //l_satmod->setFixedWidth(80);
    cb_sat_mod = new QComboBox();
    cb_sat_mod->setMinimumWidth(80);
    //cb_sat_mod->setFixedWidth(160);
    lst_prop.clear();
    lst_prop<<"None";
    for (int i = 1; i<COUNT_SATM; ++i) lst_prop<<s_id_sat_mod[i];
    cb_sat_mod->addItems(lst_prop);
    cb_sat_mod->setCurrentIndex(0);
    QLabel *l_satmodn = new QLabel();
    l_satmodn->setText(tr("Satellite")+":");
    //l_satmodn->setFixedWidth(80);
    cb_sat_nam = new QComboBox();
    //cb_sat_nam->setFixedWidth(160);
    cb_sat_nam->setMinimumWidth(250);
    //lst_prop.clear();
    //s_id_sat_nam.clear();
    //lst_prop << "None";
    //s_id_sat_nam << "";
    //cb_sat_nam->addItems(lst_prop);
    //cb_sat_nam->setCurrentIndex(0);
    QLabel *l_rxfreq = new QLabel();
    l_rxfreq->setText("RX "+tr("Frequency In")+" MHz:");
    le_rx_freq = new QLineEdit();
    QRegExp rxd("^[0-9][0-9.]*$");//+dot+0
    QValidator *validatorD = new QRegExpValidator(rxd, this); 
    le_rx_freq->setValidator(validatorD);

    QHBoxLayout *H_sat= new QHBoxLayout();
    H_sat->setContentsMargins(0,0,0,0);
    H_sat->setSpacing(4);
    H_sat->addWidget(l_satmodn);
    H_sat->addWidget(cb_sat_nam);
    H_sat->addWidget(l_satmod);
    H_sat->addWidget(cb_sat_mod);
    H_sat->addWidget(l_rxfreq);
    H_sat->addWidget(le_rx_freq);
    //H_sat->setAlignment(Qt::AlignLeft);
    cb_sat_nam->setEnabled(false);
    cb_sat_mod->setEnabled(false);
    le_rx_freq->setEnabled(false);//2.75

    QLabel *l_comment = new QLabel(tr("COMMENT")+":");
    //l_comment->setFixedWidth(50);
    le_comment = new QLineEdit();
    //le_comment->setFixedWidth(140);
    QRegExp rx2("^[^;]*$");//2.72 exclude -> ;
    QValidator *validator2 = new QRegExpValidator(rx2,this);
    le_comment->setValidator(validator2);

    /*QLabel *l_eqslmsg = new QLabel(tr("eQSL MSG")+":");
    //l_comment->setFixedWidth(50);
    QLineEdit *le_eqslmsg = new QLineEdit();
    //le_comment->setFixedWidth(140);
    //QRegExp rx2("^[^;]*$");//2.72 exclude -> ;
    //QValidator *validator2 = new QRegExpValidator(rx2,this); 
    le_eqslmsg->setValidator(validator2);*/

    le_freq = new HvInLe("freq", tr("Frequency In")+" kHz:",indsty);
    QRegExp rx("^[1-9][0-9]*$");//^[2-9][0-9]{6}$ Out of 7 digits 1 is consumed by first position 2-9 and then next 6 digits can be from 0-9
    QValidator *validator = new QRegExpValidator(rx, this);
    le_freq->SetValidatorHv(validator);// only digits
    //le_freq->setMinimumWidth(126);
    le_freq->setFixedWidth(220);
    le_freq->setMaxLength(9);
    QHBoxLayout *H_c = new QHBoxLayout();
    H_c->setContentsMargins(1,1,1,1);
    H_c->setSpacing(3);
    H_c->addWidget(le_freq);
    H_c->addWidget(l_prop);
    H_c->addWidget(cb_prop);
    H_c->addWidget(l_comment);
    H_c->setAlignment(l_comment,Qt::AlignRight);
    H_c->addWidget(le_comment);
    //H_c->addWidget(l_eqslmsg);
    //H_c->addWidget(le_eqslmsg);

    QRegExp rx1("^[1-9][0-9]*$");//^[2-9][0-9]{6}$ Out of 7 digits 1 is consumed by first position 2-9 and then next 6 digits can be from 0-9
    QValidator *validator1 = new QRegExpValidator(rx1, this);
    QLabel *l_txsn = new QLabel("TXSn:");
    QLabel *l_rxsn = new QLabel("RXSn:");
    QLabel *l_txex = new QLabel("TXExch:");
    QLabel *l_rxex = new QLabel("RXExch:");
    le_txsn = new QLineEdit();
    le_txsn->setValidator(validator1);
    le_rxsn = new QLineEdit();
    le_rxsn->setValidator(validator1);
    connect(le_rxsn, SIGNAL(textChanged(QString)), this, SLOT(ExchangeChanged(QString)));
    le_txex = new HvLeWithSpace();
    le_rxex = new HvLeWithSpace();
    connect(le_txex, SIGNAL(textChanged(QString)), this, SLOT(ExchangeChanged(QString)));
    connect(le_rxex, SIGNAL(textChanged(QString)), this, SLOT(ExchangeChanged(QString)));

    QLabel *l_bac = new QLabel();
    l_bac->setText(tr("Contest Name")+":");
    cb_cont_id = new QComboBox();
    QStringList lst_bc;
    for (int i = 0; i<COUNT_CONTEST; ++i) lst_bc << s_cont_name[pos_cont[i]];
    cb_cont_id->addItems(lst_bc);
    cb_cont_id->setMaxVisibleItems(COUNT_CONTEST);//2.50
    cb_cont_id->setCurrentIndex(0);
    connect(cb_cont_id, SIGNAL(currentIndexChanged(QString)), this, SLOT(ExchangeChanged(QString)));

    QLabel *l_trmn = new QLabel();
    //l_trmn->setText("Multi-Two Station:");//Transmitter
    l_trmn->setText(tr("Transmitter")+":");
    l_trmn->setContentsMargins(0,0,0,0);
    cb_cabrillo_trmN = new QComboBox();
    QStringList lst_trmn;
    lst_trmn <<"None"<<"Run 1"<<"Run 2";
    cb_cabrillo_trmN->addItems(lst_trmn);
    cb_cabrillo_trmN->setCurrentIndex(0);
    cb_cabrillo_trmN->setMinimumWidth(80);

    QHBoxLayout *H_cont = new QHBoxLayout();
    H_cont->setContentsMargins(1,1,1,1);
    H_cont->setSpacing(3);
    H_cont->addWidget(l_txsn);
    H_cont->addWidget(le_txsn);
    H_cont->addWidget(l_rxsn);
    H_cont->addWidget(le_rxsn);
    H_cont->addWidget(l_txex);
    H_cont->addWidget(le_txex);
    H_cont->addWidget(l_rxex);
    H_cont->addWidget(le_rxex);
    H_cont->addWidget(l_bac);
    H_cont->addWidget(cb_cont_id);
    H_cont->addWidget(l_trmn);
    H_cont->addWidget(cb_cabrillo_trmN);

    bt_corr = new QPushButton(bt_corr_txt);//" Apply Changes "
    //bt_corr->setFixedHeight(le_loc->sizeHint().height());
    connect(bt_corr, SIGNAL(clicked(bool)), this, SLOT(CorrContact()));
    bt_cancel = new QPushButton(bt_cancel_txt);//" Exit Edit Mode "
    //bt_cancel->setFixedHeight(le_loc->sizeHint().height());
    connect(bt_cancel, SIGNAL(clicked(bool)), this, SIGNAL(EndEdit()));
    QHBoxLayout *H_b = new QHBoxLayout();
    H_b->setContentsMargins(1,1,1,1);
    H_b->setSpacing(3);
    H_b->addWidget(le_call);
    //H_b->setAlignment(le_call,Qt::AlignCenter);
    H_b->addWidget(le_txrst);
    //H_b->setAlignment(le_txrst,Qt::AlignRight);
    //H_b->addWidget(le_txsn);
    H_b->addWidget(le_rxrst);
    //H_b->setAlignment(le_rxrst,Qt::AlignRight);
    //H_b->addWidget(le_rxsn);
    H_b->addWidget(le_loc);
    //H_b->setAlignment(le_loc,Qt::AlignRight);

    //H_b->addWidget(l_comment);
    //H_b->setAlignment(l_comment,Qt::AlignRight);
    //H_b->addWidget(le_comment);

    H_b->addWidget(bt_cancel);
    //H_b->addWidget(bt_corr);
    H_dtm->addWidget(bt_corr);
    //H_dtm->setAlignment(bt_corr,Qt::AlignRight);

    QVBoxLayout *V_l = new QVBoxLayout(this);
    V_l->setContentsMargins(1,1,1,1);
    V_l->setSpacing(2);
    V_l->addWidget(l_e);
    V_l->setAlignment(l_e,Qt::AlignCenter);
    V_l->addLayout(H_dtm);
    //V_l->setAlignment(H_dtm,Qt::AlignLeft);
    V_l->addLayout(H_b);
    V_l->addLayout(H_c);
    V_l->addLayout(H_sat);
    V_l->addLayout(H_cont);
    setLayout(V_l);
}
HvEditW::~HvEditW()
{}
void HvEditW::SetSatIdNames(QStringList l0,QStringList l1)
{
    cb_sat_nam->addItems(l0);
    cb_sat_nam->setCurrentIndex(0);
    s_id_sat_nam = l1;
}
void HvEditW::PropChanged(QString s)
{
    if (s=="Satellite")
    {
        cb_sat_nam->setEnabled(true);
        cb_sat_mod->setEnabled(true);
        le_rx_freq->setEnabled(true);//2.75
    }
    else
    {
        /*cb_sat_nam->setCurrentIndex(0);
        cb_sat_mod->setCurrentIndex(0);
        le_rx_freq->setText("");*/
        cb_sat_nam->setEnabled(false);
        cb_sat_mod->setEnabled(false);
        le_rx_freq->setEnabled(false);//2.75
    }
}
void HvEditW::SetFont(QFont f)
{
    le_call->SetFont(f);
    le_txrst->SetFont(f);
    le_rxrst->SetFont(f);
    le_loc->SetFont(f);
    le_freq->SetFont(f);
}
bool HvEditW::FDCheck(QString fdt)
{
    bool f_fderr = true;
    QStringList lfd = fdt.split(" ");
    if (lfd.count()==2)//le_in->setStyleSheet("QLineEdit {background-color :palette(Base);}");
    {
        QString cat = lfd.at(0);//1A-32F
        QString sec = lfd.at(1);//sec
        int icatl = (int)cat.at(cat.count()-1).toLatin1();
        if (icatl>=(int)'A' && icatl<=(int)'F')
        {
            int icat = cat.midRef(0,cat.count()-1).toInt(); //qDebug()<<cat<<sec<<icat<<sec<<cat.mid(0,cat.count()-1);
            //if (icat>0 && icat<33)//need to be 1-32  pack77 no problem ???
            if (icat>0 && icat<33)
            {
                for (int i = 0; i < NSEC; ++i)
                {
                    if (sec==csec_77[i].trimmed())
                    {
                        f_fderr = false;
                        break;
                    }
                }
            }
        }
    }
    return f_fderr;
}
bool HvEditW::RUCheck(QString s)
{
    bool f_ruerr = true;
    int irxsn = le_rxsn->text().toInt();
    for (int i = 0; i < NUSCAN; ++i)
    {
        if (s==cmult_77[i].trimmed() || s=="DX" || irxsn>0)
        {
            f_ruerr = false;
            break;
        }
    }
    if (f_ruerr) //2.74 270rc3  //&& le_exchru->text().count()==4
    {
        bool all_dig = true;
        for (int j = 0; j < s.count(); ++j)
        {
            if (s.at(j).isLetter())
            {
                all_dig = false;
                break;
            }
        }
        if (all_dig && s.toInt()>0 && s.toInt()<8000) f_ruerr = false;
    }
    return f_ruerr;
}
void HvEditW::ExchangeChanged(QString)
{
    //FD 2B EMA
    //RU MA
    QString txstr = le_txex->text();
    QString rxstr = le_rxex->text();
    int cont_id = pos_cont[cb_cont_id->currentIndex()]; //qDebug()<<cont_id;
    //QString fdt = le_exchfd->text();
    if (cont_id==4)// FD
    {
        if (FDCheck(txstr))
            le_txex->setStyleSheet("QLineEdit{background-color:rgb(255,255,200);color:rgb(200,0,0)}");
        else
            le_txex->setStyleSheet("QLineEdit{background-color:palette(Base);}");

        if (FDCheck(rxstr))
            le_rxex->setStyleSheet("QLineEdit{background-color:rgb(255,255,200);color:rgb(200,0,0)}");
        else
            le_rxex->setStyleSheet("QLineEdit{background-color:palette(Base);}");
    }
    else if (cont_id==9 || cont_id==10 || cont_id==11 || cont_id==12)//2.64 ALL RU
    {
        if (RUCheck(txstr))
            le_txex->setStyleSheet("QLineEdit{background-color:rgb(255,255,200);color:rgb(200,0,0)}");
        else
            le_txex->setStyleSheet("QLineEdit{background-color:palette(Base);}");

        if (RUCheck(rxstr))
            le_rxex->setStyleSheet("QLineEdit{background-color:rgb(255,255,200);color:rgb(200,0,0)}");
        else
            le_rxex->setStyleSheet("QLineEdit{background-color:palette(Base);}");
    }
    else
    {
        le_txex->setStyleSheet("QLineEdit{background-color:palette(Base);}");
        le_rxex->setStyleSheet("QLineEdit{background-color:palette(Base);}");
    }
}
void HvEditW::Check(QString s_type)
{
    if (s_type == "call")
    {
        //if (le_call->getText().count()>1)
        if (THvQthLoc.isValidCallsign(le_call->getText()))
        {
            le_call->setError(false);
            le_call->setErrorColorLe(false);
        }
        else
        {
            le_call->setError(true);
            le_call->setErrorColorLe(true);
        }
    }

    if (s_type == "locator")
    {
        if (!le_loc->getText().isEmpty() && THvQthLoc.isValidLocator(le_loc->getText()))
        {
            le_loc->setError(false);
            le_loc->setErrorColorLe(false);
        }
        else
        {
            le_loc->setError(true);
            le_loc->setErrorColorLe(true);
        }
    }
}
void HvEditW::CorrContact()
{
    /*if (!THvQthLoc.isValidCallsign(le_call->getText()))
    {
        QString text = tr("Please set valid call sign");
        QMessageBox::critical(this, "MSHV", text, QMessageBox::Ok);
        //le_call->SetFocus();//za da otiva fokusa na call
        return;
    } 2.55 allow adding manually SWL Calls with "-" */
    //QDateTime dateTime2 = QDateTime::fromString("M1d1y9800:01:02","'M'M'd'd'y'yyhh:mm:ss");
    //QDateTime dateTime_s = QDateTime::fromString(de_bdate_s->text()+te_time_s->text(),"yyyyMMddhh:mm");
    //QDateTime dateTime_e = QDateTime::fromString(de_bdate_e->text()+te_time_e->text(),"yyyyMMddhh:mm");
    //QDateTime dateTime2 = dte_s->
    //dateTime_s.setDate(de_bdate_s->date)

    if (dte_s->dateTime().secsTo(dte_e->dateTime())<0)
    {
        QString text = tr("The starting Date and Time have to be earlier than the End Date and Time");
        QMessageBox::critical(this, "MSHV", text, QMessageBox::Ok);
        //le_call->SetFocus();//za da otiva fokusa na call
        return;
    }
    //qDebug()<<dateTime_s.secsTo(dateTime_e);

    QStringList list;
    /*QString str_rxsn = le_rxsn->getText();
    int rxn = str_rxsn.toInt();
    int d100 = rxn/100;
    int d10 = rxn/10%10;
    int d1 = rxn%10;
    str_rxsn = QString("%1%2%3").arg(d100).arg(d10).arg(d1);*/

    int id = cb_prop->currentIndex();
    QString propt = s_id_prop_mod[id];//None

    QString satn = "";
    QString satm = "";
    QString satr = "";
    if (cb_sat_nam->isEnabled())
    {
        id = cb_sat_nam->currentIndex();
        satn = s_id_sat_nam[id];
    }
    if (cb_sat_mod->isEnabled())
    {
        id = cb_sat_mod->currentIndex();
        satm = s_id_sat_mod[id];
    }
    if (le_rx_freq->isEnabled()) satr = le_rx_freq->text();//2.75

    list<<dte_s->date().toString("yyyyMMdd")<<dte_s->time().toString("hh:mm")
    <<dte_e->date().toString("yyyyMMdd")<<dte_e->time().toString("hh:mm")<<le_call->getText()<<le_loc->getText()
    <<le_txrst->getText()<<le_rxrst->getText()<<Cb_mode->currentText()<<Cb_band->currentText()<<le_freq->getText()
    <<propt<<le_comment->text()<<s_enum_sec_sta<<le_txsn->text()<<le_rxsn->text()<<le_txex->text()<<le_rxex->text()
    <<QString("%1").arg(pos_cont[cb_cont_id->currentIndex()])<<QString("%1").arg(cb_cabrillo_trmN->currentIndex())
    <<s_enum_sec_end<<s_dist<<satn<<satm<<satr;

    emit SendCorrContact(list, index_edit);

    le_call->SetFocus();//za da otiva fokusa na call

}
void HvEditW::SetEdit(QStringList list, int index)
{
    index_edit = index; //qDebug()<<list.count(); return;
    le_call->SetText("");//zaradi duble qso
    first_dupe_flag = true;
    /*1          SSB         SSB
      2          CW          CW
      3          SSB         CW
      4          CW          SSB
      5          AM          AM
      6          FM          FM
      7          RTTY        RTTY
      8          SSTV        SSTV
      9          ATV         ATV*/

    dte_s->setDateTime(QDateTime::fromString(list.at(0)+list.at(1), "yyyyMMddhh:mm"));
    dte_e->setDateTime(QDateTime::fromString(list.at(2)+list.at(3), "yyyyMMddhh:mm"));
    //dte_s->setDate(QDate::fromString(list.at(0), "yyyyMMdd"));
    //dte_s->setTime(QTime::fromString(list.at(1), "hh:mm"));
    //dte_e->setDate(QDate::fromString(list.at(2), "yyyyMMdd"));
    //dte_e->setTime(QTime::fromString(list.at(3), "hh:mm"));

    //{"MSK144","JTMS","FSK441","FSK315","JT6M","ISCAT-A","ISCAT-B"};
    for (int i = 0; i<COUNT_MODE; ++i)//2.67
    {
        if (list.at(8) == ModeStr(pos_mods[i]))
        {
            Cb_mode->setCurrentIndex(i);
            break;
        }
    }
    if 		(list.at(8) == "SSB") Cb_mode->setCurrentIndex(COUNT_MODE);
    else if (list.at(8) == "CW" ) Cb_mode->setCurrentIndex(COUNT_MODE+1);
    else if (list.at(8) == "FM" ) Cb_mode->setCurrentIndex(COUNT_MODE+2);


    int index1 = Cb_band->findText(list.at(9), Qt::MatchCaseSensitive);
    if (index1 >= 0) Cb_band->setCurrentIndex(index1);

    le_call->SetText(list.at(4));
    if (le_call->getText().isEmpty()) Check("call");

    le_txrst->SetText(list.at(6));
    le_rxrst->SetText(list.at(7));

    le_loc->SetText(list.at(5));

    QString tmpf = list.at(10);// freq 1.50
    tmpf.remove(".");
    tmpf.remove(",");
    if (tmpf.at(0) == '0') tmpf = tmpf.mid(1,tmpf.count()-1);//2.31 remove first zero 0.455.000
    le_freq->SetText(tmpf);// freq 1.50
    //le_comment->setText("frq rezerve list.at(10)");

    index1 = 0;
    for (int i = 0; i<COUNT_PROP; ++i)//prop
    {
        if (list.at(11)==s_id_prop_mod[i])
        {
            index1 = i;
            break;
        }
    }
    cb_prop->setCurrentIndex(index1);

    le_comment->setText(list.at(12));//comment
    s_enum_sec_sta = list.at(13);        //enum seconds 00-59;
    s_enum_sec_end = list.at(20);        //enum seconds end 00-59;

    le_txsn->setText(list.at(14));
    le_rxsn->setText(list.at(15));
    le_txex->setText(list.at(16));
    le_rxex->setText(list.at(17));
    //s_cont_id = list.at(18);//contest id hiden
    int index2 = list.at(18).toInt();//qDebug()<<s_cont_name[index2];
    if (index2 >= 0)
    {
    	index1 = cb_cont_id->findText(s_cont_name[index2],Qt::MatchCaseSensitive);//2.76
   		if (index1>-1) cb_cont_id->setCurrentIndex(index1); 
   	}

    //contest MULTI-TWO hiden
    index2 = list.at(19).toInt(); //qDebug()<<index2;
    if (index2 >= 0) cb_cabrillo_trmN->setCurrentIndex(index2);

    s_dist = list.at(21);        //distance

    index1 = 0;//Satellite
    for (int i = 0; i<cb_sat_nam->count(); ++i)
    {
        if (list.at(22)==s_id_sat_nam[i])
        {
            index1 = i;
            break;
        }
    }
    cb_sat_nam->setCurrentIndex(index1);
    index1 = cb_sat_mod->findText(list.at(23),Qt::MatchCaseSensitive);
    if (index1 >= 0) cb_sat_mod->setCurrentIndex(index1);//Sat Mode
    else cb_sat_mod->setCurrentIndex(0);
    le_rx_freq->setText(list.at(24));//RX Freq
}

//#define _CONT_NAME_
//#define _CONT_NAMEID_
//#include "../../config_str_con.h"

#define COUNT_NO_DG_MODE 3
static const QString no_dg_mode[COUNT_NO_DG_MODE] =
    {"SSB","CW","FM"
    };
#define COUNT_BANDS_CABR 31
static const QString lst_bands_cabr[COUNT_BANDS_CABR] =
    {"ALL","160M","80M","60M","40M","30M","20M","17M","15M","12M","10M","6M","4M","2M","222","432","902","1.2G","2.3G","3.4G","5.7G","10G",
     "24G","47G","75G","123G","134G","241G","Light","VHF-3-BAND","VHF-FM-ONLY"
    };
#define COUNT_COP_CABR 3
static const QString lst_cop_cabr[COUNT_COP_CABR] =
    {"SINGLE-OP","MULTI-OP","CHECKLOG"
    };
#define COUNT_PWR_CABR 4
static const QString lst_pwr_cabr[COUNT_PWR_CABR] =
    {"HIGH","MEDIUM","LOW","QRP"
    };
#define COUNT_AS_CABR 2
static const QString lst_as_cabr[COUNT_AS_CABR] =
    {"ASSISTED","NON-ASSISTED"
    };
#define COUNT_CST_CABR 9
static const QString lst_cst_cabr[COUNT_CST_CABR] =
    {"FIXED","MOBILE","PORTABLE","ROVER","ROVER-LIMITED","ROVER-UNLIMITED","EXPEDITION","HQ","SCHOOL"
    };
#define COUNT_TRM_CABR 5
static const QString lst_trm_cabr[COUNT_TRM_CABR] =
    {"ONE","TWO","LIMITED","UNLIMITED","SWL"
    };
#define COUNT_TIM_CABR 4
static const QString lst_tim_cabr[COUNT_TRM_CABR] =
    {"N/A","6-HOURS","12-HOURS","24-HOURS"
    };
#define COUNT_OVR_CABR 6
static const QString lst_ovr_cabr[COUNT_OVR_CABR] =
    {"N/A","CLASSIC","ROOKIE","TB-WIRES","NOVICE-TECH","OVER-50"
    };
#define COUNT_MOD_CABR 15
static const QString lst_mod_cabr[COUNT_MOD_CABR] =
    {"DIGI","FT2","FT4","FT8","MSK144","JTMS","FSK441","FSK315","ISCAT-A","ISCAT-B","JT6M","DIGITAL","SSB","RTTY","MIXED"
    };
HvLogW::HvLogW(QString inst,QString app_path, bool indsty,int x,int y,QWidget *wp_,QWidget * parent )
        : QWidget(parent)
{
    //w_parent = wp_;
    f_km_mi = false;
    THvProgrD = new HvProgressD(this);
    THvProgrD->close();
    s_udportcp_broad_logged_adif = false;
    s_udp_broad_logged_qso = false;
    beg_append = 0;
    save_in_new_format = false;
    fsave_busy = false;
    //c_sec_broad_logged = 0;
    //last_broad_logged_dt = "yyyyMMdd hh:mm";
    f_isCheck[0] = true;
    f_isCheck[1] = true;
    f_isCheck[2] = true;
    f_isCheck[3] = true;
    f_isCheck[4] = true;
    f_isCheck[5] = true;
    f_isCheck[6] = true;
    s_allowed_modes = false;
    s_mode = "";
    s_band = "";
    FREQ_GLOBAL = "70230000";
    f_off_auto_comm = false;//2.76.3

    addtolog_comment = "";
    addtolog_freq = "";
    addtolog_prop = "";
    addtolog_txsn = "";
    addtolog_rxsn = "";
    addtolog_txex = "";
    addtolog_rxex = "";
    addtolog_satn = "";
    addtolog_satm = "";
    addtolog_rxfr = "";

    f_addtolog = false;
    add_to_log_dialog = new QDialog(wp_); //2.71 old=add_to_log_dialog = new QDialog(this);
    //add_to_log_dialog->setWindowFlags(windowFlags() ^ Qt::WindowContextHelpButtonHint);// maha help butona
    add_to_log_dialog->setWindowFlags( Qt::Dialog | Qt::WindowTitleHint );
    //add_to_log_dialog->setFixedSize(450,110);
    add_to_log_dialog->setWindowTitle(tr("ADD TO LOG"));

    QHBoxLayout *H_add_log_up= new QHBoxLayout();
    H_add_log_up->setContentsMargins(0,0,0,0);
    H_add_log_up->setSpacing(10);
    l_warn = new QLabel("");
    l_warn->setPixmap(QPixmap(":pic/warning_icon.png"));
    add_to_log_txt = new QLabel("");
    add_to_log_txt->setAlignment(Qt::AlignCenter);
    H_add_log_up->addWidget(l_warn);
    //H_add_log_up->setAlignment(l_warn,Qt::AlignTop);
    H_add_log_up->addWidget(add_to_log_txt);
    H_add_log_up->setAlignment(Qt::AlignCenter);

    l_freq = new QLabel(tr("Frequency In")+" kHz:");
    //l_freq->setFixedWidth(110);
    add_to_log_le_frq = new QLineEdit();
    //add_to_log_le_frq->setMinimumWidth(150);
    //add_to_log_le_frq->setFixedWidth(150);
    QRegExp rx("^[1-9][0-9]*$");//("^[1-9][0-9.]*$");
    QValidator *validator = new QRegExpValidator(rx, this);
    add_to_log_le_frq->setValidator(validator);// only digits
    //add_to_log_le_frq->setMinimumWidth(126);
    //add_to_log_le_frq->setFixedWidth(90);//100
    add_to_log_le_frq->setMinimumWidth(90);
    add_to_log_le_frq->setMaxLength(9);

    QLabel *l_prop = new QLabel();
    l_prop->setText(tr("Propagation")+":");
    add_to_log_cb_prop = new QComboBox();
    add_to_log_cb_prop->setFixedWidth(160);
    QStringList lst_prop;
    for (int i = 0; i<COUNT_PROP; ++i) lst_prop<<s_prop_mod[i];
    add_to_log_cb_prop->addItems(lst_prop);
    add_to_log_cb_prop->setCurrentIndex(0);
    connect(add_to_log_cb_prop, SIGNAL(currentIndexChanged(QString)), this, SLOT(PropChanged(QString)));

    QLabel *l_satmod = new QLabel();
    l_satmod->setText(tr("Sat Mode")+":");
    //l_satmod->setFixedWidth(80);
    cb_sat_mod = new QComboBox();
    cb_sat_mod->setMinimumWidth(80);
    //cb_sat_mod->setFixedWidth(160);
    lst_prop.clear();
    lst_prop << "None";
    for (int i = 1; i<COUNT_SATM; ++i) lst_prop<<s_id_sat_mod[i];
    cb_sat_mod->addItems(lst_prop);
    cb_sat_mod->setCurrentIndex(0);
    QLabel *l_satmodn = new QLabel();
    l_satmodn->setText(tr("Satellite")+":");
    //l_satmodn->setFixedWidth(80);
    cb_sat_nam = new QComboBox();
    //cb_sat_nam->setFixedWidth(160);
    cb_sat_nam->setMinimumWidth(250);
    lst_prop.clear();
    s_id_sat_nam.clear();
    lst_prop<<"None";
    s_id_sat_nam<<"";
    QString path0 = app_path;
    path0.append("/settings/database/sat.dat");
    QFile file(path0);
    if (file.open(QIODevice::ReadOnly))
    {
        QTextStream in(&file);
        while (!in.atEnd())
        {
            QString line = in.readLine();
            QStringList lst = line.split("|");//qDebug()<<lst.count()<<lst;
            if (lst.count()<2) continue;
            if (lst.at(0).isEmpty() || lst.at(1).isEmpty()) continue;
            s_id_sat_nam.append(lst.at(0));
            lst_prop.append(lst.at(1));
        }
        file.close();
    }
    else//if sat.dat missing
    {
        for (int i = 0; i<COUNT_SATIDN; ++i)
        {
            s_id_sat_nam.append(s0_sat_id_mod[i][0]);
            lst_prop.append(s0_sat_id_mod[i][1]);
        }
    }
    //Test and copy to default -> config_prop_all.h
    /*qDebug()<<"COUNT_SATIDN ="<<lst_prop.count()-1;
    for (int i = 1; i<lst_prop.count(); ++i)
    {
    	if (i==1) qDebug().nospace()<<"{\n{"<<s_id_sat_nam.at(i)<<","<<lst_prop.at(i)<<"},";    
    	else if (i==lst_prop.count()-1) qDebug().nospace()<<"{"<<s_id_sat_nam.at(i)<<","<<lst_prop.at(i)<<"}\n};";
    	else qDebug().nospace()<<"{"<<s_id_sat_nam.at(i)<<","<<lst_prop.at(i)<<"},";
    }*/
    cb_sat_nam->addItems(lst_prop);
    cb_sat_nam->setCurrentIndex(0);
    QLabel *l_rxfreq = new QLabel();
    l_rxfreq->setText("RX "+tr("Frequency In")+" MHz:");
    le_rx_freq = new QLineEdit();
    QRegExp rxd("^[0-9][0-9.]*$");//+dot+0
    QValidator *validatorD = new QRegExpValidator(rxd, this);    
    le_rx_freq->setValidator(validatorD);//only digits

    QHBoxLayout *H_sat= new QHBoxLayout();
    H_sat->setContentsMargins(0,0,0,0);
    H_sat->setSpacing(5);
    H_sat->addWidget(l_satmodn);//H_sat->setAlignment(l_satmodn,Qt::AlignRight);
    H_sat->addWidget(cb_sat_nam);
    H_sat->addWidget(l_satmod);//H_sat->setAlignment(l_satmod,Qt::AlignRight);
    H_sat->addWidget(cb_sat_mod);
    //H_sat->setAlignment(Qt::AlignCenter);
    cb_sat_nam->setEnabled(false);
    cb_sat_mod->setEnabled(false);
    le_rx_freq->setEnabled(false);//2.75

    QHBoxLayout *H_freq= new QHBoxLayout();
    H_freq->setContentsMargins(0,0,0,0);
    H_freq->setSpacing(4);
    H_freq->addWidget(l_rxfreq);
    H_freq->addWidget(le_rx_freq);

    cb_enable_ali = new QCheckBox(tr("Enable Auto Logging Info"));
    connect(cb_enable_ali,SIGNAL(toggled(bool)),this,SIGNAL(EmitCBEnableAliChanged(bool)));//clicked 2.75

    QHBoxLayout *H_p= new QHBoxLayout();
    H_p->setContentsMargins(0,0,0,0);
    H_p->setSpacing(4);
    H_p->addWidget(cb_enable_ali);
    H_p->addWidget(l_prop);
    H_p->setAlignment(l_prop,Qt::AlignRight);
    H_p->addWidget(add_to_log_cb_prop);
    //H_p->setAlignment(Qt::AlignLegt);

    l_comment = new QLabel(tr("COMMENT")+":");
    //l_comment->setFixedWidth(70);
    add_to_log_le = new QLineEdit();
    add_to_log_le->setMinimumWidth(110);
    //add_to_log_le->setFixedWidth(120);
    QRegExp rx2("^[^;]*$");//2.72 exclude -> ;
    QValidator *validator2 = new QRegExpValidator(rx2,this);
    add_to_log_le->setValidator(validator2);
    QHBoxLayout *H_add_log_comm= new QHBoxLayout();
    H_add_log_comm->setContentsMargins(0,0,0,0);
    H_add_log_comm->setSpacing(4);
    H_add_log_comm->addWidget(l_comment);
    H_add_log_comm->addWidget(add_to_log_le);

    QGroupBox *GB_001 = new QGroupBox(tr("Auto Logging Info")+":");
    QVBoxLayout *lgb001 = new QVBoxLayout();
    lgb001->setContentsMargins(8,5,8,5);
    lgb001->setSpacing(4);
    lgb001->addLayout(H_p);
    lgb001->addLayout(H_sat);
    lgb001->addLayout(H_freq);
    lgb001->addLayout(H_add_log_comm);
    GB_001->setLayout(lgb001);

    QHBoxLayout *H_add_log_fp_prop= new QHBoxLayout();
    H_add_log_fp_prop->setContentsMargins(0,0,0,0);
    H_add_log_fp_prop->setSpacing(4);
    H_add_log_fp_prop->addWidget(l_freq);
    H_add_log_fp_prop->addWidget(add_to_log_le_frq);
    H_add_log_fp_prop->setAlignment(Qt::AlignCenter);

    //QRegExp rx1("^[1-9][0-9]*$");//^[2-9][0-9]{6}$ Out of 7 digits 1 is consumed by first position 2-9 and then next 6 digits can be from 0-9
    QValidator *validator1 = new QRegExpValidator(rx, this);
    l_txsn = new QLabel("TXSn :");
    add_to_log_le_txsn = new QLineEdit();
    add_to_log_le_txsn->setValidator(validator1);
    l_rxsn = new QLabel("RXSn :");
    add_to_log_le_rxsn = new QLineEdit();
    add_to_log_le_rxsn->setValidator(validator1);
    l_txex = new QLabel("TXExch :");
    add_to_log_le_txex = new HvLeWithSpace();
    l_rxex = new QLabel("RXExch :");
    add_to_log_le_rxex = new HvLeWithSpace();
    QHBoxLayout *H_add_log_cont= new QHBoxLayout();
    H_add_log_cont->setContentsMargins(0,0,0,0);
    H_add_log_cont->setSpacing(4);
    H_add_log_cont->addWidget(l_txsn);
    H_add_log_cont->addWidget(add_to_log_le_txsn);
    H_add_log_cont->addWidget(l_rxsn);
    H_add_log_cont->addWidget(add_to_log_le_rxsn);
    H_add_log_cont->addWidget(l_txex);
    H_add_log_cont->addWidget(add_to_log_le_txex);
    H_add_log_cont->addWidget(l_rxex);
    H_add_log_cont->addWidget(add_to_log_le_rxex);

    b_add_to_log_ok = new QPushButton(tr("Add QSO"));
    b_add_to_log_cacel = new QPushButton(tr("Cancel"));
    QHBoxLayout *H_add_log_bt= new QHBoxLayout();
    H_add_log_bt->setContentsMargins(0,0,0,0);
    H_add_log_bt->setSpacing(10);
    H_add_log_bt->addWidget(b_add_to_log_ok);
    H_add_log_bt->addWidget(b_add_to_log_cacel);
    H_add_log_bt->setAlignment(Qt::AlignCenter);
    connect(b_add_to_log_ok, SIGNAL(released()), this, SLOT(OkAddToLog()));
    connect(b_add_to_log_cacel, SIGNAL(released()), this, SLOT(CancelAddToLog()));

    QVBoxLayout *V_add_log= new QVBoxLayout(add_to_log_dialog);
    V_add_log->setContentsMargins(6,5,6,5);
    V_add_log->setSpacing(5);
    V_add_log->addLayout(H_add_log_up);
    //V_add_log->setAlignment(add_to_log_txt,Qt::AlignCenter);
    V_add_log->addLayout(H_add_log_fp_prop);
    V_add_log->addWidget(GB_001);
    //V_add_log->addLayout(H_sat);
    //V_add_log->addLayout(H_freq);
    //V_add_log->addLayout(H_add_log_comm);
    V_add_log->addLayout(H_add_log_cont);
    V_add_log->addLayout(H_add_log_bt);
    V_add_log->setAlignment(Qt::AlignCenter);
    add_to_log_dialog->setLayout(V_add_log);

    QLabel *dial_cabr_txt = new QLabel(tr("Please Choose"));
    QLabel *l_warn2 = new QLabel("");
    l_warn2->setPixmap(QPixmap(":pic/warning_icon.png"));
    QHBoxLayout *H_cabr_up= new QHBoxLayout();
    H_cabr_up->setContentsMargins(0,0,0,0);
    H_cabr_up->setSpacing(20);
    dial_cabr_txt->setAlignment(Qt::AlignCenter);
    H_cabr_up->addWidget(l_warn2);
    //H_cabr_up->setAlignment(l_warn2,Qt::AlignLeft);
    H_cabr_up->addWidget(dial_cabr_txt);
    //H_cabr_up->setAlignment(dial_cabr_txt,Qt::AlignHCenter);
    H_cabr_up->setAlignment(Qt::AlignCenter);

    ExportToCabrilloD = new QDialog(this);
    //ExportToCabrilloD->setWindowFlags( Qt::Dialog | Qt::WindowTitleHint );//qt4
    ExportToCabrilloD->setWindowFlags(ExportToCabrilloD->windowFlags() & ~Qt::WindowContextHelpButtonHint);//qt5
    ExportToCabrilloD->setWindowTitle(tr("Export In Cabrillo"));

    QLabel *l_cn = new QLabel();
    l_cn->setText(tr("Contest Name")+":");
    cb_cabrillo_cont = new QComboBox();
    //cb_cabrillo_cont->setFixedWidth(180);
    QStringList lst_cont;
    for (int i = 0; i<COUNT_CONTEST; ++i) lst_cont << s_cont_name[pos_cont[i]];
    cb_cabrillo_cont->addItems(lst_cont);
    cb_cabrillo_cont->setMaxVisibleItems(COUNT_CONTEST);
    QLabel *l_cn_id = new QLabel(tr("Contest ID")+":");
    le_cabrillo_cont_id = new HvLeWithSpace();
    cb_cabrillo_cont->setCurrentIndex(0);
    le_cabrillo_cont_id->setText("NONE");
    le_cabrillo_cont_id->setMinimumWidth(135);
    connect(cb_cabrillo_cont, SIGNAL(currentIndexChanged(int)), this, SLOT(CbContNameChanged(int)));
    QHBoxLayout *H_01_contn= new QHBoxLayout();
    H_01_contn->setContentsMargins(0,0,0,0);
    H_01_contn->setSpacing(4);
    H_01_contn->addWidget(l_cn);
    H_01_contn->addWidget(cb_cabrillo_cont);
    H_01_contn->addWidget(l_cn_id);
    H_01_contn->addWidget(le_cabrillo_cont_id);
    H_01_contn->setAlignment(Qt::AlignCenter);

    cb_cabrillo_all_qso = new QCheckBox(tr("Do not filter QSOs by Contest ID (It is not recommended)"));

    QLabel *l_cnb = new QLabel();
    l_cnb->setText(tr("Band")+":");
    cb_cabrillo_band = new QComboBox();
    QStringList lst_contb;
    for (int i = 0; i<COUNT_BANDS_CABR; ++i)
        lst_contb << lst_bands_cabr[i];
    cb_cabrillo_band->addItems(lst_contb);
    cb_cabrillo_band->setCurrentIndex(0);

    QLabel *l_cop = new QLabel();
    l_cop->setText(tr("Operator")+":");
    cb_cabrillo_cop = new QComboBox();
    QStringList lst_contop;
    for (int i = 0; i<COUNT_COP_CABR; ++i)
        lst_contop << lst_cop_cabr[i];
    cb_cabrillo_cop->addItems(lst_contop);
    cb_cabrillo_cop->setCurrentIndex(0);

    QLabel *l_pwr = new QLabel();
    l_pwr->setText(tr("Power")+":");
    cb_cabrillo_pwr = new QComboBox();
    QStringList lst_pwr;
    for (int i = 0; i<COUNT_PWR_CABR; ++i)
        lst_pwr << lst_pwr_cabr[i];
    cb_cabrillo_pwr->addItems(lst_pwr);
    cb_cabrillo_pwr->setCurrentIndex(2);

    QLabel *l_mod = new QLabel();
    l_mod->setText(tr("Mode")+":");
    cb_cabrillo_mod = new QComboBox();
    QStringList lst_mod;
    for (int i = 0; i<COUNT_MOD_CABR; ++i)
        lst_mod << lst_mod_cabr[i];
    cb_cabrillo_mod->addItems(lst_mod);
    cb_cabrillo_mod->setCurrentIndex(0);
    cb_cabrillo_mod->setMinimumWidth(100);

    QLabel *l_as = new QLabel();
    l_as->setText(tr("Assisted")+":");
    cb_cabrillo_as = new QComboBox();
    QStringList lst_as;
    for (int i = 0; i<COUNT_AS_CABR; ++i)
        lst_as << lst_as_cabr[i];
    cb_cabrillo_as->addItems(lst_as);
    cb_cabrillo_as->setCurrentIndex(0);// ASSISTED

    QLabel *l_ovr = new QLabel();
    l_ovr->setText(tr("Overlay")+":");
    cb_cabrillo_ovr = new QComboBox();
    QStringList lst_ovr;
    for (int i = 0; i<COUNT_OVR_CABR; ++i)
        lst_ovr << lst_ovr_cabr[i];
    cb_cabrillo_ovr->addItems(lst_ovr);
    cb_cabrillo_ovr->setCurrentIndex(0);

    QLabel *l_cst = new QLabel();
    l_cst->setText((tr("Station")+":"));
    cb_cabrillo_cst = new QComboBox();
    QStringList lst_cst;
    for (int i = 0; i<COUNT_CST_CABR; ++i)
        lst_cst << lst_cst_cabr[i];
    cb_cabrillo_cst->addItems(lst_cst);
    cb_cabrillo_cst->setCurrentIndex(0);

    QLabel *l_tim = new QLabel();
    l_tim->setText(tr("Time Category")+":");
    cb_cabrillo_tim = new QComboBox();
    QStringList lst_tim;
    for (int i = 0; i<COUNT_TIM_CABR; ++i)
        lst_tim << lst_tim_cabr[i];
    cb_cabrillo_tim->addItems(lst_tim);
    cb_cabrillo_tim->setCurrentIndex(0);

    QLabel *l_trm = new QLabel();
    l_trm->setText(tr("Transmitter")+":");
    cb_cabrillo_trm = new QComboBox();
    QStringList lst_trm;
    for (int i = 0; i<COUNT_TRM_CABR; ++i)
        lst_trm << lst_trm_cabr[i];
    cb_cabrillo_trm->addItems(lst_trm);
    cb_cabrillo_trm->setCurrentIndex(0);
    QLabel *l_trmn = new QLabel();
    //l_trmn->setText("Multi-Two Station:");//Transmitter
    l_trmn->setText("Multi-Two "+tr("Transmitter")+":");
    l_trmn->setContentsMargins(10,0,0,0);
    cb_cabrillo_trmN = new QComboBox();
    QStringList lst_trmn;
    lst_trmn <<"From Log"<<"Run 1"<<"Run 2";
    cb_cabrillo_trmN->addItems(lst_trmn);
    cb_cabrillo_trmN->setCurrentIndex(0);
    cb_cabrillo_trmN->setMinimumWidth(40);
    /*sb_cabrillo_trmN = new QSpinBox();
    sb_cabrillo_trmN->setPrefix("Transmitter Number:  ");
    sb_cabrillo_trmN->setRange(0,1);
    sb_cabrillo_trmN->setStyleSheet("QSpinBox {selection-color: black; selection-background-color: white;}");*/
    QHBoxLayout *H_cabr_trn= new QHBoxLayout();
    H_cabr_trn->setContentsMargins(0, 0, 0, 0);
    H_cabr_trn->setSpacing(10);
    H_cabr_trn->addWidget(cb_cabrillo_trm);
    //H_cabr_trn->setAlignment(cb_cabrillo_trm,Qt::AlignLeft);
    H_cabr_trn->addWidget(l_trmn);
    H_cabr_trn->addWidget(cb_cabrillo_trmN);
    //H_cabr_trn->addWidget(sb_cabrillo_trmN);
    H_cabr_trn->setAlignment(Qt::AlignLeft);
    cb_cabrillo_trmN->setEnabled(false);
    //sb_cabrillo_trmN->setEnabled(false);
    connect(cb_cabrillo_trm, SIGNAL(currentIndexChanged(int)), this, SLOT(CbContTrmChanged(int)));

    QLabel *l_locat = new QLabel();
    l_locat->setText(tr("Location")+":");// ARRL Sect. or DX
    le_cabrillo_locat = new HvLeWithSpace();
    le_cabrillo_locat->setMinimumWidth(120);
    QLabel *l_locat1 = new QLabel();
    l_locat1->setText(tr("ARRL Sect. or DX"));
    QHBoxLayout *H_cabr_11= new QHBoxLayout();
    H_cabr_11->setContentsMargins(0, 0, 0, 0);
    H_cabr_11->setSpacing(10);
    H_cabr_11->addWidget(le_cabrillo_locat);
    H_cabr_11->addWidget(l_locat1);

    QLabel *l_oper = new QLabel();
    l_oper->setText(tr("Operators")+":");
    le_cabrillo_oper = new HvLeWithSpace();
    //le_cabrillo_oper->setMinimumWidth(220);

    QLabel *l_name = new QLabel();
    l_name->setText(tr("Name")+":");
    le_cabrillo_name = new QLineEdit();
    le_cabrillo_name->setMaxLength(75);
    //le_cabrillo_name->setMinimumWidth(220);

    QLabel *l_email = new QLabel();
    l_email->setText(tr("Email")+":");
    le_cabrillo_email = new QLineEdit();
    //le_cabrillo_email->setMinimumWidth(170);

    QLabel *l_club = new QLabel();
    l_club->setText(tr("Club")+":");
    le_cabrillo_club = new QLineEdit();
    //le_cabrillo_club->setMinimumWidth(170);
    le_cabrillo_club->setText("None");

    QLabel *l_addr = new QLabel();
    l_addr->setText(tr("Address")+":");
    le_cabrillo_addr = new QLineEdit();
    le_cabrillo_addr->setMaxLength(45);
    //le_cabrillo_addr->setMinimumWidth(220);

    QLabel *l_addr2 = new QLabel();
    l_addr2->setText(tr("Address")+":");
    le_cabrillo_addr2 = new QLineEdit();
    le_cabrillo_addr2->setMaxLength(45);
    //le_cabrillo_addr->setMinimumWidth(220);

    QLabel *l_city = new QLabel();
    l_city->setText(tr("City")+":");
    le_cabrillo_addr_city = new QLineEdit();
    //le_cabrillo_addr_city->setMaxLength(45);
    //le_cabrillo_addr_city->setMinimumWidth(170);
    //le_cabrillo_addr_city->setText("None");
    QLabel *l_spro = new QLabel();
    l_spro->setText(tr("State")+":");
    le_cabrillo_addr_spro = new QLineEdit();
    //le_cabrillo_addr_spro->setMaxLength(45);
    //le_cabrillo_addr_spro->setMinimumWidth(170);
    //le_cabrillo_addr_spro->setText("None");

    QLabel *l_posc = new QLabel();
    l_posc->setText(tr("Zip")+":");
    le_cabrillo_addr_posc = new QLineEdit();
    //le_cabrillo_addr_posc->setMaxLength(45);
    //le_cabrillo_addr_posc->setMinimumWidth(170);
    //le_cabrillo_addr_posc->setText("None");

    QHBoxLayout *H_cabr_12= new QHBoxLayout();
    H_cabr_12->setContentsMargins(0,0,0,0);
    H_cabr_12->setSpacing(2);
    H_cabr_12->addWidget(le_cabrillo_addr_city);
    H_cabr_12->addWidget(l_spro);
    H_cabr_12->addWidget(le_cabrillo_addr_spro);
    H_cabr_12->addWidget(l_posc);
    H_cabr_12->addWidget(le_cabrillo_addr_posc);

    QLabel *l_cntr = new QLabel();
    l_cntr->setText(tr("Country")+":");
    le_cabrillo_addr_cntr = new QLineEdit();
    //le_cabrillo_addr_cntr->setMaxLength(45);
    //le_cabrillo_addr_cntr->setMinimumWidth(170);
    //le_cabrillo_addr_cntr->setText("None");

    QVBoxLayout *V_01= new QVBoxLayout();
    V_01->setContentsMargins ( 5, 5, 5, 5);
    V_01->setSpacing(5);
    V_01->addWidget(l_locat);
    V_01->setAlignment(l_locat,Qt::AlignRight);
    V_01->addWidget(l_cnb);
    V_01->setAlignment(l_cnb,Qt::AlignRight);
    V_01->addWidget(l_cop);
    V_01->setAlignment(l_cop,Qt::AlignRight);
    V_01->addWidget(l_mod);
    V_01->setAlignment(l_mod,Qt::AlignRight);
    V_01->addWidget(l_pwr);
    V_01->setAlignment(l_pwr,Qt::AlignRight);
    V_01->addWidget(l_ovr);
    V_01->setAlignment(l_ovr,Qt::AlignRight);
    V_01->addWidget(l_cst);
    V_01->setAlignment(l_cst,Qt::AlignRight);
    V_01->addWidget(l_as);
    V_01->setAlignment(l_as,Qt::AlignRight);
    V_01->addWidget(l_tim);
    V_01->setAlignment(l_tim,Qt::AlignRight);
    V_01->addWidget(l_trm);
    V_01->setAlignment(l_trm,Qt::AlignRight);
    V_01->addWidget(l_club);
    V_01->setAlignment(l_club,Qt::AlignRight);
    V_01->addWidget(l_oper);
    V_01->setAlignment(l_oper,Qt::AlignRight);
    V_01->addWidget(l_name);
    V_01->setAlignment(l_name,Qt::AlignRight);
    V_01->addWidget(l_email);
    V_01->setAlignment(l_email,Qt::AlignRight);
    V_01->addWidget(l_addr);
    V_01->setAlignment(l_addr,Qt::AlignRight);
    V_01->addWidget(l_addr2);
    V_01->setAlignment(l_addr2,Qt::AlignRight);
    V_01->addWidget(l_city);
    V_01->setAlignment(l_city,Qt::AlignRight);
    //V_01->addWidget(l_spro);
    //V_01->setAlignment(l_spro,Qt::AlignRight);
    //V_01->addWidget(l_posc);
    //V_01->setAlignment(l_posc,Qt::AlignRight);
    V_01->addWidget(l_cntr);
    V_01->setAlignment(l_cntr,Qt::AlignRight);

    //V_01->setAlignment(Qt::AlignRight);

    QVBoxLayout *V_02= new QVBoxLayout();
    V_02->setContentsMargins(5,5,5,5);
    V_02->setSpacing(5);
    //V_02->addWidget(le_cabrillo_locat);
    //V_02->setAlignment(le_cabrillo_locat,Qt::AlignLeft);
    V_02->addLayout(H_cabr_11);
    V_02->addWidget(cb_cabrillo_band);
    V_02->setAlignment(cb_cabrillo_band,Qt::AlignLeft);
    V_02->addWidget(cb_cabrillo_cop);
    V_02->setAlignment(cb_cabrillo_cop,Qt::AlignLeft);
    V_02->addWidget(cb_cabrillo_mod);
    V_02->setAlignment(cb_cabrillo_mod,Qt::AlignLeft);
    V_02->addWidget(cb_cabrillo_pwr);
    V_02->setAlignment(cb_cabrillo_pwr,Qt::AlignLeft);
    V_02->addWidget(cb_cabrillo_ovr);
    V_02->setAlignment(cb_cabrillo_ovr,Qt::AlignLeft);
    V_02->addWidget(cb_cabrillo_cst);
    V_02->setAlignment(cb_cabrillo_cst,Qt::AlignLeft);
    V_02->addWidget(cb_cabrillo_as);
    V_02->setAlignment(cb_cabrillo_as,Qt::AlignLeft);
    V_02->addWidget(cb_cabrillo_tim);
    V_02->setAlignment(cb_cabrillo_tim,Qt::AlignLeft);

    //V_02->addWidget(cb_cabrillo_trm);
    V_02->addLayout(H_cabr_trn);
    //V_02->setAlignment(cb_cabrillo_trm,Qt::AlignLeft);

    V_02->addWidget(le_cabrillo_club);
    //V_02->setAlignment(le_cabrillo_club,Qt::AlignLeft);
    V_02->addWidget(le_cabrillo_oper);
    //V_02->setAlignment(le_cabrillo_oper,Qt::AlignLeft);
    V_02->addWidget(le_cabrillo_name);
    //V_02->setAlignment(le_cabrillo_name,Qt::AlignLeft);
    V_02->addWidget(le_cabrillo_email);
    //V_02->setAlignment(le_cabrillo_email,Qt::AlignLeft);
    V_02->addWidget(le_cabrillo_addr);
    V_02->addWidget(le_cabrillo_addr2);
    //V_02->setAlignment(le_cabrillo_addr,Qt::AlignLeft);
    //V_02->addWidget(le_cabrillo_addr_city);
    //V_02->setAlignment(le_cabrillo_addr_city,Qt::AlignLeft);
    //V_02->addWidget(le_cabrillo_addr_spro);
    //V_02->setAlignment(le_cabrillo_addr_spro,Qt::AlignLeft);
    //V_02->addWidget(le_cabrillo_addr_posc);
    //V_02->setAlignment(le_cabrillo_addr_posc,Qt::AlignLeft);
    V_02->addLayout(H_cabr_12);
    V_02->addWidget(le_cabrillo_addr_cntr);
    //V_02->setAlignment(le_cabrillo_addr_cntr,Qt::AlignLeft);

    //V_02->setAlignment(Qt::AlignLeft);

    QHBoxLayout *H_01_hhh= new QHBoxLayout();
    H_01_hhh->setContentsMargins(0,0,0,0);
    H_01_hhh->setSpacing(2);
    H_01_hhh->addLayout(V_01);
    H_01_hhh->addLayout(V_02);
    //H_01_hhh->setAlignment(Qt::AlignHCenter);

    QLabel *l_d_s = new QLabel();
    l_d_s->setText(tr("Date UTC Start")+":");
    exp_dte_start = new QDateTimeEdit();
    exp_dte_start->setDisplayFormat("yyyy-MM-dd  hh:mm");
    exp_dte_start->setMinimumWidth(90);
    QLabel *l_d_e = new QLabel();
    l_d_e->setText(tr("End")+":");
    exp_dte_end = new QDateTimeEdit();
    exp_dte_end->setDisplayFormat("yyyy-MM-dd  hh:mm");
    exp_dte_end->setMinimumWidth(90);
    QHBoxLayout *H_dtm = new QHBoxLayout();
    H_dtm->setContentsMargins(1,1,1,1);
    H_dtm->setSpacing(3);
    H_dtm->addWidget(l_d_s);
    H_dtm->addWidget(exp_dte_start);
    H_dtm->addWidget(l_d_e);
    H_dtm->addWidget(exp_dte_end);
    H_dtm->setAlignment(Qt::AlignCenter);

    b_cabrillo_ok = new QPushButton(tr("OK"));
    b_cabrillo_ok->setFixedWidth(80);
    b_cabrillo_cacel = new QPushButton(tr("Cancel"));
    b_cabrillo_cacel->setFixedWidth(80);
    QHBoxLayout *H_b_exp_cab= new QHBoxLayout();
    H_b_exp_cab->setContentsMargins(0,0,0,0);
    H_b_exp_cab->setSpacing(10);
    H_b_exp_cab->addWidget(b_cabrillo_ok);
    H_b_exp_cab->addWidget(b_cabrillo_cacel);
    H_b_exp_cab->setAlignment(Qt::AlignCenter);
    connect(b_cabrillo_ok, SIGNAL(released()), this, SLOT(OkCabr()));
    connect(b_cabrillo_cacel, SIGNAL(released()), this, SLOT(CancelCabr()));

    QVBoxLayout *V_exp_cab= new QVBoxLayout(ExportToCabrilloD);
    V_exp_cab->setContentsMargins ( 5, 5, 5, 5);
    V_exp_cab->setSpacing(5);
    V_exp_cab->addLayout(H_cabr_up);
    V_exp_cab->addLayout(H_01_contn);
    V_exp_cab->addWidget(cb_cabrillo_all_qso);
    V_exp_cab->setAlignment(cb_cabrillo_all_qso,Qt::AlignCenter);
    V_exp_cab->addLayout(H_dtm);
    V_exp_cab->addLayout(H_01_hhh);
    V_exp_cab->setAlignment(Qt::AlignTop);
    V_exp_cab->addLayout(H_b_exp_cab);

    ExportToCabrilloD->setLayout(V_exp_cab);

    //blosk_from_other_list = false;
    AppPath = app_path;
    s_my_call = "NONE";
    s_my_grid = "NONE";
    s_my_fd_exch = "NONE";
    s_my_ru_exch = "NONE";
    f_iem_ru_dx = false;
    this->setWindowTitle(tr("Log")+" "+(QString)APP_NAME + inst);
    //setWindowFlags(windowFlags() ^ Qt::WindowContextHelpButtonHint);
    setWindowFlags(windowFlags() &~ Qt::WindowMaximizeButtonHint &~ Qt::WindowContextHelpButtonHint);
    setWindowIcon(QPixmap(":pic/ms_ico.png"));

    THvLogList = new HvLogList(indsty);//indsty
    connect(THvLogList, SIGNAL(EmitDoubleClick()), this, SLOT(EditQso()));
    connect(THvLogList, SIGNAL(SortClicked(int)), this, SLOT(SortClicked(int)));

    Min_Menu = new QMenuBar();

    //Min_Menu->setMinimumWidth(100);
    //Min_Menu->setContentsMargins(0,0,0,0);
    QMenu *Menu_m = new QMenu(tr("   Menu   "));
    //Menu_m->setContentsMargins(0,0,0,0);

    QAction *NewLog_m;
    NewLog_m = new QAction(QPixmap(":pic/new_log.png"),tr("Create New Log  And Back Up Old"),this);
    connect(NewLog_m, SIGNAL(triggered()), this, SLOT(CreateNewLogAndSaveOld()));

    QAction *BackupLog_m;
    BackupLog_m = new QAction(QPixmap(":pic/new_log_b.png"),tr("Create Log Backup"),this);
    connect(BackupLog_m, SIGNAL(triggered()), this, SLOT(CreateBackupLog()));

    QAction *AddLog_m;
    AddLog_m = new QAction(QPixmap(":pic/addlog.png"),tr("Add Log"),this);
    connect(AddLog_m, SIGNAL(triggered()), this, SLOT(AddExtrnalLog()));

    QAction *AddLog_madif;
    AddLog_madif = new QAction(QPixmap(":pic/addlog.png"),tr("Add ADIF To Log"),this);
    connect(AddLog_madif, SIGNAL(triggered()), this, SLOT(AddExtrnalAdif()));

    QAction *ExportSelToAdif_m;
    ExportSelToAdif_m = new QAction(QPixmap(":pic/exportsel.png"),tr("Export Selected In ADIF"),this);
    connect(ExportSelToAdif_m, SIGNAL(triggered()), this, SLOT(ExportSelToAdif()));
    QAction *ExportAllToAdif_m;
    ExportAllToAdif_m = new QAction(QPixmap(":pic/exportall.png"),tr("Export All In ADIF"),this);
    connect(ExportAllToAdif_m, SIGNAL(triggered()), this, SLOT(ExportAllToAdif()));

    QAction *ExportToCabr_m;
    ExportToCabr_m = new QAction(QPixmap(":pic/exportsel.png"),tr("Export In Cabrillo"),this);
    connect(ExportToCabr_m, SIGNAL(triggered()), this, SLOT(ExportToCabr()));

    QAction *UploadSelToClubLog_m;
    UploadSelToClubLog_m = new QAction(QPixmap(":pic/uploadsel.png"),tr("Upload Selected To")+" Club Log",this);
    connect(UploadSelToClubLog_m, SIGNAL(triggered()), this, SLOT(UploadSelToClubLog()));

    ac_use_adif_save = new QAction(tr("Use Save QSOs In ADIF Log"),this);//2.66
    ac_use_adif_save->setCheckable(true);
    ac_use_adif_save->setChecked(true);

    /*QAction *Statistic_m; //for resurce file <file>pic/logstat.png</file>
    Statistic_m = new QAction(QPixmap(":pic/logstat.png"),tr("Log Statistic By Modes"),this);
    connect(Statistic_m, SIGNAL(triggered()), this, SLOT(ViewDialogStaostic()));*/

    Menu_m->addAction(NewLog_m);
    Menu_m->addAction(BackupLog_m);
    Menu_m->addAction(AddLog_m);
    Menu_m->addAction(AddLog_madif);
    Menu_m->addAction(ExportSelToAdif_m);
    Menu_m->addAction(ExportAllToAdif_m);
    Menu_m->addAction(ExportToCabr_m);
    Menu_m->addAction(UploadSelToClubLog_m);
    Menu_m->addAction(ac_use_adif_save);
    //Menu_m->addAction(Statistic_m);
    Min_Menu->addMenu(Menu_m);

    /*DStatistic = new QDialog(this);
    DStatistic->setWindowTitle("MSHV "+tr("Log Statistic"));// By Modes
    DStatistic->setMinimumSize(150,380);
    DStatistic->setWindowFlags(DStatistic->windowFlags() & ~Qt::WindowContextHelpButtonHint);    
    TBStatistic = new  QTextBrowser();
    TBStatistic->setReadOnly(true); 
    //TBStatistic->setAlignment(Qt::AlignVCenter);    
    QVBoxLayout *lv_h1 = new QVBoxLayout();
    lv_h1->setContentsMargins(0,0,0,0);
    lv_h1->setSpacing(0); 
    lv_h1->addWidget(TBStatistic);
    DStatistic->setLayout(lv_h1);*/

    QPushButton *b_dell_selected = new QPushButton(tr("DELETE SELECTED"));
    b_dell_selected->setFixedHeight(22);
    //b_dell_selected->setFixedSize(100,22);
    connect(b_dell_selected, SIGNAL(clicked(bool)), this, SLOT(DeleteSelected()));

    QPushButton *b_defaut_sort = new QPushButton(tr("DEFAULT SORT"));//default formatting
    b_defaut_sort->setFixedHeight(22);
    connect(b_defaut_sort, SIGNAL(clicked(bool)), this, SLOT(DefaultSort()));

    b_edit_qso = new QPushButton(tr("EDIT QSO"));
    b_edit_qso->setFixedHeight(22);
    connect(b_edit_qso, SIGNAL(clicked(bool)), this, SLOT(EditQso()));
    b_add_man_qso = new QPushButton(tr("ADD QSO MANUALLY"));
    b_add_man_qso->setFixedHeight(22);
    connect(b_add_man_qso, SIGNAL(clicked(bool)), this, SLOT(AddManQso()));
    QHBoxLayout *H_l_qso_oper= new QHBoxLayout();
    H_l_qso_oper->setContentsMargins ( 0, 0, 0, 0);
    H_l_qso_oper->setSpacing(0);
    H_l_qso_oper->addWidget(b_add_man_qso);
    H_l_qso_oper->addWidget(b_edit_qso);
    //H_l_qso_oper->set

    THvEditW = new HvEditW(tr("EDIT QSO")," "+tr("Exit Edit Mode")+" "," "+tr("Apply Changes")+" ",indsty,this);
    //connect(THvEditW, SIGNAL(CheckCall(QString,int)), this, SLOT(CheckCall_W(QString,int)));
    connect(THvEditW, SIGNAL(SendCorrContact(QStringList,int)), this, SLOT(SetCorrContact(QStringList,int)));
    THvEditW->hide();
    connect(THvEditW, SIGNAL(EndEdit()), this, SLOT(End_Edit()));
    THvEditW->SetSatIdNames(lst_prop,s_id_sat_nam);

    AddManQsoW = new HvEditW(tr("ADD QSO MANUALLY")," "+tr("Exit Add Mode")+" "," "+tr("Add QSO")+" ",indsty,this);
    //connect(THvEditW, SIGNAL(CheckCall(QString,int)), this, SLOT(CheckCall_W(QString,int)));
    connect(AddManQsoW, SIGNAL(SendCorrContact(QStringList,int)), this, SLOT(AddManQsoToList(QStringList,int)));
    AddManQsoW->hide();
    connect(AddManQsoW, SIGNAL(EndEdit()), this, SLOT(End_Add_Man_Qso()));
    AddManQsoW->SetSatIdNames(lst_prop,s_id_sat_nam);

    le_find = new HvInLe("find", tr("Find")+":",indsty);
    QPushButton *b_find = new QPushButton(tr("Find"));
    b_find->setFixedHeight(22);
    connect(b_find, SIGNAL(clicked(bool)), this, SLOT(Find()));
    connect(le_find, SIGNAL(EmitEntered()), this, SLOT(Find()));

    QHBoxLayout *H_mm = new QHBoxLayout();
    H_mm->setContentsMargins (0, 1, 1, 0);
    H_mm->setSpacing(0);
    H_mm->addWidget(Min_Menu);
    //H_mm->setAlignment(Qt::AlignCenter);
    QFrame *Box_mm = new QFrame();
    Box_mm->setFrameShape(QFrame::Box);
    Box_mm->setFrameShadow(QFrame::Raised);//Sunken Plain Raised
    Box_mm->setLineWidth(1);
    Box_mm->setMidLineWidth(0);
    Box_mm->setStyleSheet("QFrame{background-color:rgb(190,190,255);}");
    Box_mm->setLayout(H_mm);

    QLabel *l_qsos_in_log1 = new QLabel(tr("QSOs In Log")+":");
    l_qsos_in_log = new QLabel("0");
    QHBoxLayout *H_l_top= new QHBoxLayout();
    H_l_top->setContentsMargins ( 2, 2, 2, 2);
    H_l_top->setSpacing(8);
    //H_l_top->addWidget(Min_Menu);
    //H_l_top->setAlignment(Min_Menu,Qt::AlignLeft);
    H_l_top->addWidget(Box_mm);
    H_l_top->setAlignment(Box_mm,Qt::AlignLeft);
    H_l_top->addWidget(b_dell_selected);
    H_l_top->addWidget(l_qsos_in_log1);
    H_l_top->addWidget(l_qsos_in_log);
    H_l_top->addWidget(b_defaut_sort);
    H_l_top->addWidget(le_find);
    H_l_top->addWidget(b_find);

    //HvLogList *THvLogList1 = new HvLogList();
    QVBoxLayout *V_l = new QVBoxLayout(this);
    V_l->setContentsMargins ( 1, 1, 1, 1);
    V_l->setSpacing(1);
    //V_l->addWidget(b_dell_selected);
    V_l->addLayout(H_l_top);
    V_l->addWidget(THvLogList);
    //V_l->addWidget(THvLogList1);
    //V_l->addWidget(b_edit_qso);
    V_l->addLayout(H_l_qso_oper);
    V_l->addWidget(THvEditW);
    V_l->addWidget(AddManQsoW);
    //V_l->addWidget(TransmitTxt);
    //V_l->setAlignment(TransmitTxt, Qt::AlignHCenter);
    //V_l->addWidget(THvInputW);
    //V_l->addWidget(THvEditW);
    setLayout(V_l);

    find_count_row = 0;
    find_count_column = 0;
    f_found = false;
    //timer_canc_add_to_log_dialog = new QTimer();
    //connect(timer_canc_add_to_log_dialog, SIGNAL(timeout()), this, SLOT(CancelAddToLog()));

    resize(this->width(),this->height());// triabva zaradi patvia start pat

    timer_refr_save = new QTimer();//1.81 inportent for big logs
    connect(timer_refr_save, SIGNAL(timeout()), this, SLOT(RefreshSave()));//,Qt::QueuedConnection

    //ReadEDI(AppPath+"/log/mshvlog.edim",false,true);//2.57 stop bool check for old format

    move(x,y);

    //THvLogList->setCurrentIndex(THvLogList->model.index(THvLogList->model.rowCount()-1,0));
    //THvLogList->scrollToBottom();
    //THvLogList->update();

    /*unsigned long long int max_ll_int_size = std::numeric_limits<unsigned long long int>::max();
    quint64 max_unsigned_int_size = std::numeric_limits<quint64>::max();
    //int max_int_size = std::numeric_limits<int>::max();
    QString ttry = QString("%1").arg(max_unsigned_int_size);
    unsigned int kkk = ttry.toUInt();
    bool max_quint64 = std::numeric_limits<bool>::max();
    qDebug()<<max_ll_int_size<<max_unsigned_int_size<<ttry<<kkk<<max_quint64;*/
}
HvLogW::~HvLogW()
{
    //SaveAllExternal(); 2.35 stop close error no needed
}
/*#include <QTextTable>
void HvLogW::ViewDialogStatstic()
{
	QString info;
	if (THvLogList->model.rowCount()<1)
        return;
    //////// v1 /////////////////////////////////////////
    QDialog *DStatistic = new QDialog(this);
    DStatistic->setWindowTitle("MSHV "+tr("Log Statistic By Modes"));// By Modes
    DStatistic->setMinimumSize(150,380);
    DStatistic->setWindowFlags(DStatistic->windowFlags() & ~Qt::WindowContextHelpButtonHint);
    QTextBrowser *TBStatistic = new  QTextBrowser();
    TBStatistic->setReadOnly(true);
    //TBStatistic->setAlignment(Qt::AlignVCenter);
	QVBoxLayout *lv_h1 = new QVBoxLayout();
    lv_h1->setContentsMargins(0,0,0,0);
    lv_h1->setSpacing(0);
    lv_h1->addWidget(TBStatistic);
    DStatistic->setLayout(lv_h1);

	QTextCursor cursor(TBStatistic->textCursor());
	cursor.movePosition(QTextCursor::Start);

	QTextBlockFormat bfAlignment;
	bfAlignment.setAlignment(Qt::AlignCenter); //Qt::AlignHCenter AlignRight
	//QTextBlockFormat bflAlignment;
	//bflAlignment.setAlignment(Qt::AlignLeft);

    QTextTableFormat tableFormat;
    tableFormat.setBorder(0);
    tableFormat.setCellPadding(2);
    tableFormat.setCellSpacing(0);
    tableFormat.setWidth(QTextLength(QTextLength::PercentageLength,100));
    //tableFormat.setAlignment(Qt::AlignCenter);
	QTextTable *table = cursor.insertTable(COUNT_MODE+1,2,tableFormat);
	//QTextTableCell fromCell = table->cellAt(0, 0);
  	//fromCell.setFormat(nickFormat);

    table->cellAt(0,0).firstCursorPosition().setBlockFormat(bfAlignment);
   	table->cellAt(0,0).firstCursorPosition().insertText(tr("QSOs In Log"));
   	int cmodea = 0;
	for (int i = 0; i < COUNT_MODE; ++i)
    {
    	table->cellAt(i+1,0).firstCursorPosition().setBlockFormat(bfAlignment);
    	table->cellAt(i+1,0).firstCursorPosition().insertText(ModeStr(i));
    	QString mode = ModeStr(i);
    	int cmode = 0;
        for (int i = 0; i < THvLogList->model.rowCount(); ++i)
        {
        	if (THvLogList->model.item(i,8)->text() == mode) cmode++;
       	}
       	//table->cellAt(i,1).firstCursorPosition().setBlockFormat(bfAlignment);
       	table->cellAt(i+1,1).firstCursorPosition().insertText(QString("%1").arg(cmode));
       	cmodea+=cmode;
    }
    table->cellAt(0,1).firstCursorPosition().insertText(QString("%1").arg(cmodea));
    TBStatistic->scrollToAnchor("MSK144");
    DStatistic->exec();

    delete lv_h1;
    delete TBStatistic;
    delete DStatistic;
    //////// end v1 /////////////////////////////////////////

    //////// v2 /////////////////////////////////////////
    QDialog *DStatistic = new QDialog(this);
    DStatistic->setWindowTitle("MSHV "+tr("Log Statistic"));
    //DStatistic->setMinimumWidth(200);
    DStatistic->setMinimumSize(800,380);
    DStatistic->setWindowFlags(DStatistic->windowFlags() & ~Qt::WindowContextHelpButtonHint);
    QTextBrowser *TBStatistic = new  QTextBrowser();
    TBStatistic->setReadOnly(true);
    QTextCursor cursor(TBStatistic->textCursor());
	cursor.movePosition(QTextCursor::Start);
	//TBStatistic->setLineWrapMode(QTextEdit::NoWrap);
	QVBoxLayout *lv_h1 = new QVBoxLayout();
    //lv_h1->setContentsMargins(20,10,20,10);
    lv_h1->setContentsMargins(0,0,0,0);
    lv_h1->setSpacing(0);
    lv_h1->addWidget(TBStatistic);
    DStatistic->setLayout(lv_h1);

	QTextBlockFormat bfAlignment;
	bfAlignment.setAlignment(Qt::AlignHCenter); //Qt::AlignHCenter
	//cursor.setBlockFormat(centerAlignment);

	//QTextCharFormat nickFormat;
 	//nickFormat.setForeground(Qt::darkGreen);
  	//nickFormat.setFontWeight(QFont::Bold);
  	//nickFormat.setFontUnderline(true);
  	//nickFormat.setUnderlineColor(Qt::gray);
    QTextTableFormat tableFormat;
    //tableFormat.setBorderStyle(QTextFrameFormat::BorderStyle_Dotted);
	//tableFormat.setBorderBrush(QBrush(Qt::CrossPattern));
    tableFormat.setBorder(1);
    tableFormat.setCellPadding(0);
    tableFormat.setCellSpacing(0);
    //tableFormat.setWidth(QTextLength(QTextLength::PercentageLength, 100));
    tableFormat.setAlignment(Qt::AlignCenter);

	QVector<QTextLength> columnWidth;
	QFontMetrics fm(this->font());
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)//2.56
       int fcor = fm.horizontalAdvance("2320 GHz")+8;
#else
       int fcor = fm.width("2320 GHz")+8;
#endif
	for(int i = 0; i < COUNT_MODE+1; ++i)
	columnWidth.append(QTextLength(QTextLength::FixedLength, fcor));
    tableFormat.setColumnWidthConstraints(columnWidth);

	QTextTable *table = cursor.insertTable(COUNT_BANDS+2,COUNT_MODE+1,tableFormat);
	//QTextTableCell fromCell = table->cellAt(0, 0);
  	//fromCell.setFormat(nickFormat);

    table->cellAt(0,0).firstCursorPosition().setBlockFormat(bfAlignment);
    table->cellAt(0,0).firstCursorPosition().insertText(tr("Band"));
   	table->cellAt(1,0).firstCursorPosition().setBlockFormat(bfAlignment);
   	table->cellAt(1,0).firstCursorPosition().insertText(tr("Totals"));

   	int perbands[COUNT_BANDS];
   	for (int i = 0; i < COUNT_BANDS; ++i)
    {
    	table->cellAt(i+2,0).firstCursorPosition().setBlockFormat(bfAlignment);
    	table->cellAt(i+2,0).firstCursorPosition().insertText(lst_bands[i]);
   	}

	for (int i = 0; i < COUNT_MODE; ++i)
    {
    	for (int x = 0; x < COUNT_BANDS; ++x) perbands[x] = 0;
    	table->cellAt(0,i+1).firstCursorPosition().setBlockFormat(bfAlignment);
    	table->cellAt(0,i+1).firstCursorPosition().insertText(ModeStr(i));
    	QString mode = ModeStr(i);
    	int cmode = 0;
        for (int i = 0; i < THvLogList->model.rowCount(); ++i)
        {
        	if (THvLogList->model.item(i,8)->text() == mode)
        	{
        		cmode++;
        		QString band = THvLogList->model.item(i,9)->text();
        		for (int j = 0; j < COUNT_BANDS; ++j)
        		{
        			if (lst_bands[j]==band)
        			{
        				perbands[j]++;
    					break;
       				}
       			}
       		}
       	}
       	table->cellAt(1,i+1).firstCursorPosition().setBlockFormat(bfAlignment);
       	table->cellAt(1,i+1).firstCursorPosition().insertText(QString("%1").arg(cmode));
       	for (int x = 0; x < COUNT_BANDS; ++x)
       	{
			table->cellAt(x+2,i+1).firstCursorPosition().setBlockFormat(bfAlignment);
    		table->cellAt(x+2,i+1).firstCursorPosition().insertText(QString("%1").arg(perbands[x]));
      	}
    }
    //cursor.endEditBlock();
    TBStatistic->scrollToAnchor("Band");
    DStatistic->exec();
    delete lv_h1;
    delete TBStatistic;
    delete DStatistic;
    //////// end v2 /////////////////////////////////////////
}*/
void HvLogW::PropChanged(QString s)
{
    if (s=="Satellite")
    {
        cb_sat_nam->setEnabled(true);
        cb_sat_mod->setEnabled(true);
        le_rx_freq->setEnabled(true);//2.75
    }
    else
    {
        /*cb_sat_nam->setCurrentIndex(0);
        cb_sat_mod->setCurrentIndex(0);
        le_rx_freq->setText("");*/
        cb_sat_nam->setEnabled(false);
        cb_sat_mod->setEnabled(false);
        le_rx_freq->setEnabled(false);//2.75
    }
}
void HvLogW::ReadEDILog()//2.57
{
    //qDebug()<<s_my_grid;
    ReadEDI(AppPath+"/log/mshvlog.edim",false,true);// bool check for old format
    //DefaultSort();//2.57  if sorted return to normal
    //for (int i = 0; i < 10; ++i) MakeLoggedQSO(i);
}
void HvLogW::Show_log()// 2.28
{
    //if(!isVisible()) THvLogList->scrollToBottom();
    show();
    //slow down first open HV
    //THvLogList->setCurrentIndex(THvLogList->model.index(THvLogList->model.rowCount()-1,0));
    THvLogList->scrollToBottom();//2.52
}
void HvLogW::SetPosXYWH(QString s)//2.48
{
    QStringList list_s = s.split("#");
    if (list_s.count()==4)
    {
        move(list_s[0].toInt(),list_s[1].toInt());
        if (!list_s[2].isEmpty() && !list_s[3].isEmpty())
        {
            if (list_s[2] == "FULL" || list_s[3] == "FULL" )
                setWindowState(Qt::WindowMaximized);
            else
                resize(list_s[2].toInt(), list_s[3].toInt());
        }
    }
}
void HvLogW::SetFont(QFont f)
{
    le_find->SetFont(f);
    THvEditW->SetFont(f);
    AddManQsoW->SetFont(f);
    //Min_Menu->setFont(f);
    //ExportToCabriloD->setFont(f);
}
QString HvLogW::ExtractAdifRecord(QString in,QString str)
{
    //2.21 mode Adif 3.1.0 <mode:4>MFSK <submode:3>FT4 <-submode exeption ????
    QString out;
    int i = in.indexOf(str,0,Qt::CaseInsensitive); //2.09 ,0,Qt::CaseInsensitive
    int j = in.indexOf(">",i);
    QString w = in.mid(i,j-i+1);

    //LOGGER32 exception
    if (str=="<QSO_DATE:" || str=="<QSO_DATE_OFF:") w.remove(":D",Qt::CaseInsensitive);

    w.remove(str,Qt::CaseInsensitive);
    w.remove(">");
    int cw = w.toInt();
    out = in.mid(j+1,cw);
    out.remove("<");
    out.remove(">");
    out=out.trimmed();
    return out;
}
//#define MAX_LOG_COUNT 15000
//#define MAX_LOG_COUNT 25000 //2.48
//#define MAX_LOG_COUNT 45000 //2.59
static int MAX_LOG_COUNT = 50000; //2.69
bool d_max_log_count = false;
void HvLogW::CheckLogCount()
{
    if (d_max_log_count) return;
    d_max_log_count = true;
    if (THvLogList->model.rowCount()>MAX_LOG_COUNT)
    {
        int c_del = (THvLogList->model.rowCount() - MAX_LOG_COUNT) + 999;//2.69=999 old=499
        QMessageBox::information(this, "MSHV",
                                 tr("The maximum number of QSOs in Log is")+": "+QString("%1").arg(MAX_LOG_COUNT)+".\n"+
                                 tr("Your Log contains")+": "+QString("%1").arg(THvLogList->model.rowCount())+" QSOs.\n"+
                                 tr("Please make backup of your Log and then delete a minimum of")+" "+QString("%1").arg(c_del)+" "+tr("of your old QSOs."),
                                 QMessageBox::Ok);
    }
    d_max_log_count = false;
}
void HvLogW::SetMaxLogQsoCount(QString s)
{
    int i = s.toInt();
    if (i>9999 && i<500001) MAX_LOG_COUNT = i; //qDebug()<<MAX_LOG_COUNT;
    /*MAX_LOG_COUNT = s.toInt();
    if (MAX_LOG_COUNT<10000 ) MAX_LOG_COUNT = 10000;	
    if (MAX_LOG_COUNT>500000) MAX_LOG_COUNT = 500000;*/
}
QString HvLogW::GetMaxLogQsoCount()
{
    return QString("%1").arg(MAX_LOG_COUNT);
}
void HvLogW::AddExtrnalAdif()
{
    End_Edit();
    End_Add_Man_Qso();
    QFileDialog dialog;

    QString path = QFileDialog::getOpenFileName(this,
                   "Add ADIF To Log", AppPath, "LOG File (*.adi)");

    if (path.isEmpty())
        return;

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    int added_qso = 0;
    int err_mod_qso = 0;

    THvProgrD->Show(tr("Adding QSOs"),tr("Please Wait, Adding QSOs")+" ",510);

    QString line;
    QTextStream in(&file);
    while (!in.atEnd())
    {
        QString line0 = in.readLine();
        QStringList t_list;
        //int i =0;
        //<STATION_CALLSIGN:5>LZ2HV<MY_GRIDSQUARE:6>KN23SF<CALL:6>SP9HWY

        ////////////////////////////////////
        line.append(line0);
        //qDebug()<<"1 line="<<line;
        int ieoh = line.indexOf("<EOH>",0,Qt::CaseInsensitive);//2.09 ,0,Qt::CaseInsensitive
        if (ieoh>-1) //rmove head
        {
            line.clear();
            continue;
        }
        ///////////////////////////////////

        int ieor = line.indexOf("<EOR>",0,Qt::CaseInsensitive);//2.09 ,0,Qt::CaseInsensitive
        if (ieor==-1) continue; //2.06 if no end char no my line -> continue

        //2.21 mode Adif 3.1.0 <mode:4>MFSK <submode:3>FT4 <-submode exeption ????
        QString mode_disp = ExtractAdifRecord(line,"<MODE:");
        mode_disp=mode_disp.toUpper();
        QString smode_disp;
        bool fmode  = false;
        ////// my existing modes /////////////////////////////////////////////////
        for (int i = 0; i < COUNT_MODE; ++i)
        {
            if (ModeStr(i)==mode_disp)
            {
                fmode = true;
                break;
            }
        }
        if (!fmode)// try as submode
        {
            smode_disp = ExtractAdifRecord(line,"<SUBMODE:");
            smode_disp=smode_disp.toUpper();
            bool fsmode = false;
            for (int i = 0; i < COUNT_MODE; ++i)
            {
                if (ModeStr(i)==smode_disp)
                {
                    fsmode = true;
                    break;
                }
            }
            if (fsmode)
            {
                fmode = true;
                mode_disp = smode_disp;
            }
        }
        ////// end my existing modes //////////////////////////////////////////////
        if (!fmode) //submode correction form other softs
        {
            if (mode_disp=="JT65")      //wsjtx or N1MM
            {
                fmode = true;
                mode_disp="JT65B";
            }
            else if (mode_disp=="ISCAT")//wsjtx or N1MM
            {
                fmode = true;
                mode_disp="ISCAT-B";
            }
            else if (mode_disp=="MFSK" || mode_disp=="Q65") //wsjtx or N1MM
            {
            	if (smode_disp=="FT2")
                {
                    fmode = true;
                    mode_disp="FT2";
                }
                else if (smode_disp=="FT4")
                {
                    fmode = true;
                    mode_disp="FT4";
                }
                else if (smode_disp=="Q65" || mode_disp=="Q65")
                {
                    fmode = true;
                    mode_disp="Q65A";
                }
            }
        }
        if (!fmode)//NO DG MODES
        {
            for (int i = 0; i < COUNT_NO_DG_MODE; ++i)
            {
                if (no_dg_mode[i]==mode_disp)
                {
                    fmode = true;
                    break;
                }
            }
        }
        if (!fmode)// mode no exist
        {
            err_mod_qso++;
            line.clear();
            continue;
        }

        QString calls = ExtractAdifRecord(line,"<CALL:");
        calls=calls.toUpper();
        QString hgrid = ExtractAdifRecord(line,"<GRIDSQUARE:");
        hgrid=hgrid.toUpper();

        QString dist = ExtractAdifRecord(line,"<DISTANCE:");//distance

        QString tx_rst = ExtractAdifRecord(line,"<RST_SENT:");
        QString rx_rst = ExtractAdifRecord(line,"<RST_RCVD:");

        QString str_date = ExtractAdifRecord(line,"<QSO_DATE:");
        QString enum_sec_sta = "00"; //enum seconds 00-59
        QString enum_sec_end = "00"; //enum seconds 00-59
        QString tss_s = ExtractAdifRecord(line,"<TIME_ON:");
        if (tss_s.count()>5) enum_sec_sta=tss_s.mid(4,2);//2.41 <time_off:6>182810
        tss_s=tss_s.mid(0,4);
        tss_s.insert(2,":");
        QString str_date_end = ExtractAdifRecord(line,"<QSO_DATE_OFF:");
        if (str_date_end.isEmpty())
            str_date_end=str_date;
        QString tss_end = ExtractAdifRecord(line,"<TIME_OFF:");
        if (tss_end.isEmpty())
        {
            tss_end=tss_s;
            enum_sec_end = enum_sec_sta;
        }
        else
        {
            if (tss_end.count()>5)  enum_sec_end=tss_end.mid(4,2); //2.41 <time_off:6>182810
            tss_end=tss_end.mid(0,4);
            tss_end.insert(2,":");
        }

        QString str_frq = ExtractAdifRecord(line,"<FREQ:");
        if (!str_frq.isEmpty())//many exceptions 50.313000  71.23  14.07000 50.1 600 .600 0.600
        {
            int idot = str_frq.indexOf(".");
            if (idot==-1)
            {
                idot=str_frq.count();
                str_frq.append(".0");
            }
            else if (idot==0)
            {
                str_frq.prepend("0");
                idot++;
            }
            int iend = str_frq.count();
            QString fw = str_frq.mid(0,idot);
            for (int i = idot+1; i < idot+4; ++i)
            {
                if (i<iend)
                    fw.append(str_frq[i]);
                else
                    fw.append("0");
            }
            str_frq=fw; //qDebug()<<str_frq;
        }
        QString str_band = ExtractAdifRecord(line,"<BAND:");
        str_band=str_band.toUpper();
        for (int i = 0; i < COUNT_BANDS; ++i)
        {
            if (str_band==lst_lambda[i])
            {
                str_band=lst_bands[i];
                break;
            }
        }
        if (str_band.isEmpty() && !str_frq.isEmpty())//exception band missing from -> HB9OAB - WLOG2000
        {
            unsigned long long f_int = str_frq.toLongLong();
            f_int = f_int*1000;
            for (int i = 0; i<COUNT_BANDS; ++i)
            {
                if (f_int>=freq_min_max[i].min && f_int<=freq_min_max[i].max)
                {
                    str_band = lst_bands[i]; //qDebug()<<"ADIF Export Band Missing Exception="<<str_band;
                    break;
                }
            }
        }
        if (!str_band.isEmpty() && str_frq.isEmpty())
        {
            for (int i = 0; i<COUNT_BANDS; ++i)
            {
                if (str_band==lst_bands[i])
                {
                    str_frq = lst_bandtofrq[i]; //qDebug()<<"ADIF Export Freq Missing Exception="<<str_frq;
                    break;
                }
            }
        }

        QString prop_m = ExtractAdifRecord(line,"<PROP_MODE:");
        QString comm = ExtractAdifRecord(line,"<COMMENT:");

        QString satn = ExtractAdifRecord(line,"<SAT_NAME:");
        QString satm = ExtractAdifRecord(line,"<SAT_MODE:");
        QString frrx = ExtractAdifRecord(line,"<FREQ_RX:");

        QString tx_sn = ExtractAdifRecord(line,"<STX:");
        if (tx_sn.isEmpty()) tx_sn = ExtractAdifRecord(line,"<STX_STRING:");//SRX_STRING LogHX exception
        if (tx_sn.toInt()>0) tx_sn=QString("%1").arg(tx_sn.toInt());//remove zeros no neded
        QString rx_sn = ExtractAdifRecord(line,"<SRX:");
        if (rx_sn.isEmpty()) rx_sn = ExtractAdifRecord(line,"<SRX_STRING:");//SRX_STRING LogHX exception
        if (rx_sn.toInt()>0) rx_sn=QString("%1").arg(rx_sn.toInt());//remove zeros no neded

        QString log_rx_exch;
        QString rx_n1mm_exch = ExtractAdifRecord(line,"<APP_N1MM_EXCHANGE1:");
        QString rx_exch = ExtractAdifRecord(line,"<ARRL_SECT:");
        if (rx_exch.isEmpty()) rx_exch = ExtractAdifRecord(line,"<STATE:");//wsjt-x exception STATE:
        if (!rx_n1mm_exch.isEmpty()) log_rx_exch.append(rx_n1mm_exch);
        if (!rx_exch.isEmpty()) log_rx_exch.append(" "+rx_exch);
        //wsjt-x category exception
        if (rx_rst.count()==2 || rx_rst.count()==3)
        {
            int dg = 0;
            if (rx_rst.count()==3) dg = rx_rst.midRef(0,2).toInt();
            else dg = rx_rst.midRef(0,1).toInt();
            QString rx_rst1 = rx_rst.at(rx_rst.count()-1);
            rx_rst1=rx_rst1.toUpper();
            int t1 = (int)rx_rst1.at(rx_rst1.count()-1).toLatin1();
            if (t1>=(int)'A' && t1<=(int)'F' && dg>=1 && dg<=32)
            {
                log_rx_exch = rx_rst+" "+rx_exch;
                rx_rst = "+00";
            }
        }
        log_rx_exch=log_rx_exch.trimmed();
        log_rx_exch=log_rx_exch.toUpper();

        QString cont_id = ExtractAdifRecord(line,"<CONTEST_ID:");
        if (!cont_id.isEmpty()) cont_id = cont_id.toUpper();
        QString log_cont_id = "0";
        for (int i = 0; i < COUNT_CONTEST; ++i)
        {
            if (s_cont_name_id[i]==cont_id)
            {
                log_cont_id = QString("%1").arg(i);
                break;
            }
        }

        QString log_tx_exch;
        QString tx_mshv_exch = ExtractAdifRecord(line,"<APP_MSHV_EXCHANGE_TX:");
        if (!tx_mshv_exch.isEmpty()) log_tx_exch = tx_mshv_exch;
        if (log_tx_exch.isEmpty())
        {
            int id_cont_t = log_cont_id.toInt(); //2.63
            //all RU 2.64
            if ((id_cont_t==9 || id_cont_t==10 || id_cont_t==11 || id_cont_t==12)
                    && !s_my_ru_exch.isEmpty() && s_my_ru_exch!="NONE")
                log_tx_exch = s_my_ru_exch;
            else if (id_cont_t==4 && !s_my_fd_exch.isEmpty() && s_my_fd_exch!="NONE")// 4="ARRL-FIELD-DAY"
                log_tx_exch = s_my_fd_exch;
        }
        //wsjt-x category exception
        if (tx_rst.count()==2 || tx_rst.count()==3)
        {
            int dg = 0;
            if (tx_rst.count()==3) dg = tx_rst.midRef(0,2).toInt();
            else dg = tx_rst.midRef(0,1).toInt();
            QString tx_rst1 = tx_rst.at(tx_rst.count()-1);
            tx_rst1=tx_rst1.toUpper();
            int t1 = (int)tx_rst1.at(tx_rst1.count()-1).toLatin1();
            if (t1>=(int)'A' && t1<=(int)'F' && dg>=1 && dg<=32)
            {
                log_tx_exch = tx_rst;
                tx_rst = "+00";
            }
        }
        log_tx_exch=log_tx_exch.trimmed();
        log_tx_exch=log_tx_exch.toUpper();

        QString trmN = ExtractAdifRecord(line,"<APP_N1MM_RUN1RUN2:");//<APP_N1MM_RUN1RUN2:1>0
        if (trmN.isEmpty()) trmN="0";
        //QString qslmsg = ExtractAdifRecord(line,"<QSLMSG:");

        //qDebug()<<calls<<hgrid<<mode_disp<<str_date<<tss_s<<str_date_end<<tss_end<<str_band<<
        //str_frq<<comm<<tx_rst<<rx_rst<<tx_sn<<rx_sn<<log_rx_exch<<cont_id;
        t_list<<str_date<<tss_s<<str_date_end<<tss_end<<calls<<hgrid<<tx_rst
        <<rx_rst<<mode_disp<<str_band<<str_frq<<prop_m<<comm<<enum_sec_sta
        <<tx_sn<<rx_sn<<log_tx_exch<<log_rx_exch<<log_cont_id<<trmN<<enum_sec_end<<dist
        <<satn<<satm<<frrx;

        Insert(t_list,false,false,0,false,false);
        added_qso++;
        line.clear();

        THvProgrD->SetValue(added_qso);
    }
    THvProgrD->Finish(added_qso);
    file.close();
    THvProgrD->close();

    DefaultSort(); //2.24 need for cabrilo and adif export

    SaveAllExternal(false);
    THvLogList->setCurrentIndex(THvLogList->model.index(THvLogList->model.rowCount()-1,0));
    THvLogList->scrollToBottom();
    l_qsos_in_log->setText(QString("%1").arg(THvLogList->model.rowCount()));

    QMessageBox::information(this, "MSHV", tr("Added")+" "+QString("%1").arg(added_qso)+" "+tr("QSOs from ADIF to Log.")+"\n"+
                             tr("Non-supported modes of QSOs")+" "+QString("%1").arg(err_mod_qso), QMessageBox::Ok);
    //qDebug()<<"All Lines Count="<<allc<<"Exported QSOs="<<THvLogList->model.rowCount()<<"Error no supported modes="<<ccc<<"Eror is: "<<err;

    if (s_allowed_modes && (f_isCheck[0] || f_isCheck[3])) GetMarkTextAllFromLog_p(s_band,s_mode);

    CheckLogCount();
}
void HvLogW::AddExtrnalLog()
{
    End_Edit();
    End_Add_Man_Qso();
    QFileDialog dialog;
    QString file = dialog.getOpenFileName(this,"Add Log", AppPath, "LOG File (*.edim)");
    if (!file.isEmpty())
    {
        ReadEDI(file,true,false);//bool check for old format
        DefaultSort();//2.24 need for cabrilo and adif export
        SaveAllExternal(false);
        //all_exp=0 sel_exp=1  save_all_for_ext_log_prog=2 append_all_for_ext_log_prog=3
        //ExportToAdif("",2);//save_all_for_ext_log_prog=2
        THvLogList->setCurrentIndex(THvLogList->model.index(THvLogList->model.rowCount()-1,0));
        THvLogList->scrollToBottom();
        l_qsos_in_log->setText(QString("%1").arg(THvLogList->model.rowCount()));
    }
}
void HvLogW::SortClicked(int)
{
    if (s_index.isValid())
    {
        QFont f_t = font();//vra6ta staria font
        QStandardItem *item = THvLogList->model.itemFromIndex(s_index);
        item->setFont(f_t);//vra6ta staria font
    }
    End_Edit();
    End_Add_Man_Qso();
}
void HvLogW::DefaultSort()
{
    if (s_index.isValid())
    {
        QFont f_t = font();//vra6ta staria font
        QStandardItem *item = THvLogList->model.itemFromIndex(s_index);
        item->setFont(f_t);//vra6ta staria font
    }

    THvLogList->sortByColumn(1,Qt::AscendingOrder);
    THvLogList->sortByColumn(0,Qt::AscendingOrder);
    //THvLogList->model.sort(1,Qt::AscendingOrder); //2.60
    //THvLogList->model.sort(0,Qt::AscendingOrder); //2.60

    End_Edit();
    End_Add_Man_Qso();
}
void HvLogW::SetMarkTextAllRuleChanged(bool *f,int mode,QString band)
{
    f_isCheck[0] = f[19];//qso b4
    f_isCheck[1] = f[20];//qso b4 band
    f_isCheck[2] = f[21];//qso b4 mode
    f_isCheck[3] = f[22];//loc
    f_isCheck[4] = f[23];//loc 46
    f_isCheck[5] = f[24];//loc b
    f_isCheck[6] = f[25];//loc m
    s_mode = ModeStr(mode);
    s_band = band;
    if (mode==0 || mode==7 || mode==8 || mode==9 || mode==11 || mode==12 || mode==13 || mode==18 ||
            mode==14 || mode==15 || mode==16 || mode==17)//ft4 q65
        s_allowed_modes = true;
    else
        s_allowed_modes = false;


    if (s_allowed_modes && (f_isCheck[0] || f_isCheck[3])) GetMarkTextAllFromLog_p(s_band,s_mode);
    else
    {
        QStringList l;
        l.clear();
        emit EmitMarkTextLogAll(l,0);//emit empty list
    }
}
bool HvLogW::isDuplicate(QStringList l,QString s,int b)//2.63
{
    bool res = false;
    for (int j = b; j<l.count(); ++j)
    {
        if (l.at(j)==s)
        {
            res = true;
            break;
        }
    }
    return res;
}
void HvLogW::GetMarkTextAllFromLog_p(QString band,QString mode)
{
    QStringList l;
    int p1 = 0;
    int l_count = THvLogList->GetListCount();
    l.clear();

    if (f_isCheck[0]) //qso b4 calls
    {
        for (int i = 0; i<l_count; ++i)
        {
            if (f_isCheck[2])
            {
                if (THvLogList->model.item(i,8)->text()!=mode) continue;
            }
            if (f_isCheck[1])
            {
                if (THvLogList->model.item(i,9)->text()!=band) continue;
            }
            QString s = THvLogList->model.item(i,4)->text();
            if (s.isEmpty()) continue;
            if (isDuplicate(l,s,0)) continue;
            l.append(s);
        }
    } //l.removeDuplicates();
    p1 = l.count();
    if (f_isCheck[3]) //qso b4 loc
    {
        for (int i = 0; i<l_count; ++i)
        {
            if (f_isCheck[6])
            {
                if (THvLogList->model.item(i,8)->text()!=mode) continue;
            }
            if (f_isCheck[5])
            {
                if (THvLogList->model.item(i,9)->text()!=band) continue;
            }
            QString s = THvLogList->model.item(i,5)->text();
            if (s.isEmpty()) continue;

            if (!f_isCheck[4])//only loc 6
            {
                if (s.count()>5)//2.63  ==6
                {
                    if (isDuplicate(l,s.mid(0,6),p1)) continue;
                    l.append(s.mid(0,6));
                } //else continue;
            }
            else //loc 4 and 6
            {
                if (s.count()>3)
                {
                    if (!isDuplicate(l,s.mid(0,4),p1)) l.append(s.mid(0,4));
                    if (s.count()>5)
                    {
                        if (isDuplicate(l,s.mid(0,6),p1)) continue;
                        l.append(s.mid(0,6));
                    }
                } //else continue;
            }
        }
    } //l.removeDuplicates();
    emit EmitMarkTextLogAll(l,p1); //qDebug()<<l<<l.count();
}
void HvLogW::IsCallDupeInLog(QString call,QString mod,QString band,int maxqso,bool &is_exist)
{
    is_exist=false;
    int l_count = THvLogList->GetListCount();
    for (int i = 0; i<l_count; ++i)
    {
        if (THvLogList->model.item(i,4)->text()!=call) continue;
        if (f_isCheck[2]) // mod
        {
            if (THvLogList->model.item(i,8)->text()!=mod)  continue;
        }
        if (f_isCheck[1]) // band
        {
            if (THvLogList->model.item(i,9)->text()!=band) continue;
        }
        if (maxqso>1)//2.66 (0 or 1)=1-QSO 2=2-QSO 3=3-QSO
        {
            maxqso--;
            continue;
        }
        is_exist=true;
        return;
    }
}
void HvLogW::Find()
{
    //THvLogList->setRowHidden(1,QModelIndex(),true);
    if (THvLogList->model.rowCount()<1)
        return;

    QFont f_t = font();//vra6ta staria font
    if (s_index.isValid())
    {
        QStandardItem *item = THvLogList->model.itemFromIndex(s_index);
        item->setFont(f_t);//vra6ta staria font
    }
    THvLogList->setCurrentIndex(THvLogList->model.index(-1, 0));

    if (!le_find->getText().isEmpty())
    {
c100:
        if (find_count_row > THvLogList->model.rowCount()-1)
            find_count_row = 0;

        for (int i = find_count_row; i<THvLogList->model.rowCount(); i++)
        {
            for (int j = find_count_column; j<THvLogList->model.columnCount(); j++)
            {
                find_count_column++;
                if (find_count_column > THvLogList->model.columnCount()-1)
                {
                    find_count_column = 0;
                    find_count_row++;
                }

                if (THvLogList->model.item(i, j)->text().contains(le_find->getText(),Qt::CaseInsensitive))
                {
                    s_index = THvLogList->model.index(i, j);
                    QStandardItem *item = THvLogList->model.itemFromIndex(s_index);
                    f_t.setBold(true);
                    //f_t.setPointSize(11);
                    f_t.setPointSize(f_t.pointSize()+1);
                    item->setFont(f_t);
                    THvLogList->setCurrentIndex(s_index);
                    THvLogList->scrollTo(s_index);
                    f_found = true;
                    return;
                }
            }
        }
        if (f_found)
        {
            f_found = false;
            goto c100;
        }
        else f_found = false;//niama nzda ana za wseki slu4ei
    }
}
void HvLogW::End_Add_Man_Qso()
{
    b_add_man_qso->show();
    b_edit_qso->show();
    AddManQsoW->hide();
}
void HvLogW::AddManQso()
{
    b_edit_qso->hide();
    b_add_man_qso->hide();
    AddManQsoW->show();
    SetManQsoText();
}
void HvLogW::SetFreqGlobal(QString s)
{
    FREQ_GLOBAL = s;   //qDebug()<<s;
}
void HvLogW::SetManQsoText()
{
    //QModelIndex index = THvLogList->selectionModel()->currentIndex();
    int index_n = THvLogList->model.rowCount();

    QDateTime utc_t = QDateTime::currentDateTimeUtc();
    //QString band = "";
    QString mode = "";
    //QString c_id = "";
    //QString c_trmn = "";

    if (index_n>0)
    {
        //band = THvLogList->model.item(THvLogList->model.rowCount()-1, 9)->text();
        mode = THvLogList->model.item(THvLogList->model.rowCount()-1, 8)->text();
        //c_id = THvLogList->model.item(THvLogList->model.rowCount()-1, 18)->text();
    }
    QString tsfrq = FREQ_GLOBAL.mid(0,FREQ_GLOBAL.size()-3);

    QStringList list;
    list << utc_t.toString("yyyyMMdd"); //date start 0
    list << utc_t.toString("hh:mm"); //time start    1
    list << utc_t.toString("yyyyMMdd"); //date end   2 hiden
    list << utc_t.toString("hh:mm"); //time end      3
    list << ""; //1.40=empty CALL? 12 to 11 call     4
    list << ""; //loc                                5
    list << ""; //txrpt                              6
    list << ""; //rxrpt                              7
    list << mode; //mode                             8
    list << s_band;//band                            9
    list << tsfrq;//freq                             10
    list << ""; //prop                               11
    list << ""; //comment                            12
    list << ""; //enum seconds 00-59                 13
    list << "";//txsn							  	 14
    list << "";//rxsn							  	 15
    list << "";//txex							  	 16
    list << "";//rxex							  	 17
    list << "0";//cont id						  	 18 hiden
    list << "0";//contest MULTI-TWO hiden            19 hiden
    list << ""; //enum seconds end 00-59             20 hiden
    list << ""; //distance				             21
    list << ""; //Satellite				             22
    list << ""; //Sat Mode				             23
    list << ""; //RX Freq				             24

    AddManQsoW->SetEdit(list,index_n);
    THvLogList->scrollToBottom();
}
void HvLogW::End_Edit()
{
    b_add_man_qso->show();
    b_edit_qso->show();
    THvEditW->hide();
    THvLogList->SetEditRow(-1);
}
void HvLogW::EditQso()
{
    if (THvLogList->model.rowCount() < 1) return;
    if (THvLogList->selectionModel()->selection().empty())
    {
        QMessageBox::critical(this, "MSHV", tr("Please select QSO."), QMessageBox::Ok);
        return;
    }
    if (THvLogList->selectionModel()->selectedRows(0).count()>1)
    {
        QMessageBox::critical(this, "MSHV", tr("Please select only one QSO to edit."), QMessageBox::Ok);
        return;
    }

    if (AddManQsoW->isVisible())
        End_Add_Man_Qso();

    if (b_edit_qso->isVisible())
    {
        b_edit_qso->hide();
        b_add_man_qso->hide();
        THvEditW->show();
    }
    SetEditText();
    //THvLogList->model
}

void HvLogW::SetEditText()
{
    QModelIndex index = THvLogList->selectionModel()->currentIndex();
    int index_n = index.row();
    QStringList list;

    /*list << index.sibling(index.row(),0).data().toString(); //date start                0
    list << index.sibling(index.row(),1).data().toString(); //time start                 1
    list << index.sibling(index.row(),2).data().toString(); //date end  hiden               0
    list << index.sibling(index.row(),3).data().toString(); //time end                1
    list << index.sibling(index.row(),4).data().toString(); // 12 to 11 call
    list << index.sibling(index.row(),5).data().toString(); //loc
    list << index.sibling(index.row(),6).data().toString(); //txrpt
    list << index.sibling(index.row(),7).data().toString(); //rxrpt
    list << index.sibling(index.row(),8).data().toString(); //mode
    list << index.sibling(index.row(),9).data().toString(); //band
    list << index.sibling(index.row(),10).data().toString(); //frq reserved hiden 1.31
    list << index.sibling(index.row(),11).data().toString(); //prop
    list << index.sibling(index.row(),12).data().toString(); //comment
    list << index.sibling(index.row(),13).data().toString(); //enum seconds 00-59
    list << index.sibling(index.row(),14).data().toString();//txsn
    list << index.sibling(index.row(),15).data().toString();//rxsn
    list << index.sibling(index.row(),16).data().toString();//txex
    list << index.sibling(index.row(),17).data().toString();//rxex
    list << index.sibling(index.row(),18).data().toString();//cont id hisen
    list << index.sibling(index.row(),19).data().toString();//cont MULTI-TWO hiden
    list << index.sibling(index.row(),20).data().toString();//enum seconds end 00-59
    list << index.sibling(index.row(),21).data().toString();//distance*/
    //2.65 count list columns
    int ccount = THvLogList->model.columnCount();
    for (int i = 0; i < ccount; ++i) list << THvLogList->model.item(index_n,i)->text();//2.65 no crash ?
    //old for (int i = 0; i < ccount; ++i) list << index.sibling(index_n,i).data().toString();

    THvEditW->SetEdit(list,index_n);
    THvLogList->SetEditRow(index_n);
    //THvLogList->scrollToBottom(); no no move
}
void HvLogW::DeleteSelected()
{
    if (!THvLogList->selectionModel()->selection().empty())
    {
        //QItemSelection selection( THvLogList->selectionModel()->selection() );
        QModelIndexList lst = THvLogList->selectionModel()->selectedRows(0);
        QList<int> rows;
        foreach( const QModelIndex & index, lst )
        {
            rows.append( index.row() );
        }
        //q_Sort( rows );
        std::sort(rows.begin(),rows.end());

        QString str_n;
        if (rows.count()<11)
        {
            str_n.append(tr("Do you want to delete the following entries from the log?")+"\n");
            for (int j = 0; j < rows.count(); j++)
            {
                for (int i = 0; i<THvLogList->model.columnCount(); i++)
                {
                    //2.73 no show, end-date,enum sec,cont-ID,distance,Satellite,SatMode,RX Freq
                    if (i==2 || i==13 || i==18 || i==19  || i==20 || i==21 || i==22 || i==23 || i==24) continue;
                    str_n.append(THvLogList->model.item(rows[j], i)->text());
                    str_n.append(" ");
                }
                str_n.append("\n");
            }
        }
        else
        {
            str_n.append(tr("Do you want to delete")+" "+QString("%1").arg(rows.count())+" "+tr("QSOs from the log?")+"\n");
        }

        QMessageBox msgBox(this);
        msgBox.setWindowTitle("MSHV");
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText(str_n);
        //Do you want to add the following entry in the log?\n
        //msgBox.setInformativeText("Do you want to save DUPLICATE QSO?");
        msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Cancel);
        //msgBox.setButtonText(QMessageBox::Ok, tr("Aa"));
        //msgBox.setButtonText(QMessageBox::Cancel, tr("Iao"));
        int ret = msgBox.exec();
        switch (ret)
        {
        case QMessageBox::Ok:
            // dupe = "DUPE";
            break;
        case QMessageBox::Cancel:
            return;
            break;
        default:
            return;
            break;
        }

        if (s_index.isValid())
        {
            QFont f_t = font();//vra6ta staria font
            QStandardItem *item = THvLogList->model.itemFromIndex(s_index);
            item->setFont(f_t);//vra6ta staria font
        }

        THvLogList->DeleteSel();

        SaveAllExternal(false);
        //all_exp=0 sel_exp=1  save_all_for_ext_log_prog=2 append_all_for_ext_log_prog=3
        //ExportToAdif("",2);//save_all_for_ext_log_prog=2

        l_qsos_in_log->setText(QString("%1").arg(THvLogList->model.rowCount()));

        End_Edit();
        End_Add_Man_Qso();

        if (s_allowed_modes && (f_isCheck[0] || f_isCheck[3])) GetMarkTextAllFromLog_p(s_band,s_mode);

        CheckLogCount();
    }
}
/*void HvLogW::mousePressEvent ( QMouseEvent * event)
{
    //if (event->button() == Qt::RightButton)
    {
        m_opt->removeAction(edit_qso);
        m_opt->exec(QCursor::pos());
    }//
    QWidget::mousePressEvent(event);
}*/
void HvLogW::keyPressEvent(QKeyEvent *event)
{
    if (event->key()==Qt::Key_Delete && !THvLogList->selectionModel()->selection().empty())
    {
        End_Edit();
        End_Add_Man_Qso();
        DeleteSelected();
    }
    else QWidget::keyPressEvent(event);
}
#define MIN_CCOUNT 25
//const QString _LOG_ID_ = "[QSORecords; MSHV LOG FILE ID=20201024-248]";//count=25
//const QString _LOG_ID_ = "[QSORecords; MSHV LOG FILE ID=20210617-257]";//count=25 add distance need full log refresh
const QString _LOG_ID_ = "[QSORecords; MSHV LOG FILE ID=20231018-273]";//2.73 count=29
void HvLogW::ReadEDI(QString path,bool pbarr,bool check_format)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return;
    QStringList list;
    QString line;
    bool f_qsos = false;
    int added_qso = 0;

    if (pbarr) THvProgrD->Show(tr("Adding QSOs"),tr("Please Wait, Adding QSOs")+" ",510);
    QTextStream in(&file);
    while (!in.atEnd())
    {
        line = in.readLine();
        if (line.isEmpty()) continue;
        if (line.contains("[END;", Qt::CaseInsensitive)) f_qsos = false;//for old version
        if (check_format && !line.contains(_LOG_ID_,Qt::CaseInsensitive)) save_in_new_format = true;//old log
        check_format = false;
        if (f_qsos)
        {
            bool fcount25 = false;
            list = line.split(";");
            if (list.count() == 25) fcount25 = true;//old count=25 and 24column="24"
            if (list.count() < MIN_CCOUNT)
            {
                for (int i = list.count(); i < MIN_CCOUNT; ++i) list.append("");
            }
            QStringList t_list;
            QString str_date = list.at(0);
            QString str_date_end = list.at(2);
            QString tss_s = list.at(1);
            tss_s.insert(2,":");
            QString tss_end = list.at(3);
            tss_end.insert(2,":");
            QString mode_disp = list.at(5);
            QString str_band = list.at(9);

            if      (mode_disp=="0") mode_disp="NON";
            else if (mode_disp=="1") mode_disp="SSB";
            else if (mode_disp=="2") mode_disp="CW";
            //else if (mode_disp=="3") mode_disp="SSB/CW";
            //else if (mode_disp=="4") mode_disp="CW/SSB";
            //else if (mode_disp=="5") mode_disp="AM";
            else if (mode_disp=="6") mode_disp="FM";
            //else if (mode_disp=="7") mode_disp="RTTY";
            //else if (mode_disp=="8") mode_disp="SSTV";
            //else if (mode_disp=="9") mode_disp="ATV";
            else if (mode_disp=="10") mode_disp="JTMS";
            else if (mode_disp=="11") mode_disp="FSK441";
            else if (mode_disp=="12") mode_disp="ISCAT-A";
            else if (mode_disp=="13") mode_disp="ISCAT-B";
            else if (mode_disp=="14") mode_disp="JT6M";
            else if (mode_disp=="15") mode_disp="FSK315";
            //else if (mode_disp=="16") mode_disp="JTMSK";
            else if (mode_disp=="17") mode_disp="MSK144";
            else if (mode_disp=="18") mode_disp="JT65A";
            else if (mode_disp=="19") mode_disp="JT65B";
            else if (mode_disp=="20") mode_disp="JT65C";
            else if (mode_disp=="21") mode_disp="PI4";
            else if (mode_disp=="22") mode_disp="FT8";
            else if (mode_disp=="23") mode_disp="MSKMS";
            else if (mode_disp=="24") mode_disp="FT4";
            else if (mode_disp=="25") mode_disp="Q65A";
            else if (mode_disp=="26") mode_disp="Q65B";
            else if (mode_disp=="27") mode_disp="Q65C";
            else if (mode_disp=="28") mode_disp="Q65D";
            else if (mode_disp=="29") mode_disp="FT2";

            str_band.replace("k"," kHz",Qt::CaseInsensitive);
            str_band.replace("M"," MHz",Qt::CaseInsensitive);
            str_band.replace("G"," GHz",Qt::CaseInsensitive);
            //Za Ma MANE  lz2hv may e ve4e end

            QString enum_sec_sta = list.at(18);
            if (enum_sec_sta.isEmpty()) enum_sec_sta = "00";
            QString enum_sec_end = list.at(20);
            if (enum_sec_end.isEmpty()) enum_sec_end = enum_sec_sta;
            QString cont_id = list.at(17);
            if (list.at(17).isEmpty()) cont_id = "0";
            QString trmN = list.at(19);
            if (trmN.isEmpty()) trmN = "0";
            QString dist = list.at(21);
            QString rxfrq = list.at(24);
            if (fcount25 && rxfrq=="24") rxfrq = "";

            t_list<<str_date<<tss_s<<str_date_end<<tss_end<<list.at(4)<<list.at(8)<<list.at(6)
            <<list.at(7)<<mode_disp<<str_band<<list.at(10)<<list.at(12)<<list.at(11)<<enum_sec_sta
            <<list.at(13)<<list.at(14)<<list.at(15)<<list.at(16)<<cont_id<<trmN<<enum_sec_end<<dist
            <<list.at(22)<<list.at(23)<<rxfrq;

            Insert(t_list,false,false,0,false,false);//0=show_cont_id 0=no, 1=sn, 2=exch 3=sn+exch
            list.clear();
            t_list.clear();

            if (pbarr)
            {
                added_qso++;
                THvProgrD->SetValue(added_qso);
            }
        }
        if (line.contains("[QSORecords;", Qt::CaseInsensitive))
        {
            f_qsos = true;
        }
    }
    if (pbarr) THvProgrD->Finish(added_qso);
    file.close();
    if (pbarr) THvProgrD->close();
    if (s_allowed_modes && (f_isCheck[0] || f_isCheck[3])) GetMarkTextAllFromLog_p(s_band,s_mode);
    if (pbarr) CheckLogCount();//only from add external
}
void HvLogW::SetLogAutoComm(bool f)//2.76.3
{
    f_off_auto_comm = f; 
	if (!f_off_auto_comm) add_to_log_le->clear();//2.76.3 
}
void HvLogW::CancelAddToLog()
{
    add_to_log_dialog->close();
	if (!f_off_auto_comm) add_to_log_le->clear();//2.76.3
}
void HvLogW::OkAddToLog()
{
    f_addtolog = true;
    addtolog_comment = add_to_log_le->text();
	if (!f_off_auto_comm) add_to_log_le->clear();//2.76.3
    addtolog_freq = add_to_log_le_frq->text();

    int id = add_to_log_cb_prop->currentIndex();
    addtolog_prop = s_id_prop_mod[id];//None
    if (cb_sat_nam->isEnabled())
    {
        id = cb_sat_nam->currentIndex();
        addtolog_satn = s_id_sat_nam[id];
    }
    else addtolog_satn = "";
    if (cb_sat_mod->isEnabled())
    {
        id = cb_sat_mod->currentIndex();
        addtolog_satm = s_id_sat_mod[id];
    }
    else addtolog_satm = "";
    if (le_rx_freq->isEnabled()) addtolog_rxfr = le_rx_freq->text();//2.75
    else addtolog_rxfr = "";

    addtolog_txsn = add_to_log_le_txsn->text();
    addtolog_rxsn = add_to_log_le_rxsn->text();
    addtolog_txex = add_to_log_le_txex->text();
    addtolog_rxex = add_to_log_le_rxex->text();

    add_to_log_dialog->close();
}
void HvLogW::StartAddToDialog(QStringList in_lst)
{
    f_addtolog = false;
    //add_to_log_le->clear();//2.76.3 stop
    add_to_log_le_frq->clear();
    
    add_to_log_le_txsn->clear();
    add_to_log_le_rxsn->clear();
    add_to_log_le_txex->clear();
    add_to_log_le_rxex->clear();

    QString str_n;
    for (int i = 0; i < in_lst.count(); i++)
    {
        if (i==10)//freq
        {
            add_to_log_le_frq->setText(in_lst.at(i));
            continue;
        }
        else if (i==12)//comment
        {
        	if (!f_off_auto_comm) add_to_log_le->setText(in_lst.at(i));//2.76.3           
            continue;
        }
        else if (i==14)//
        {
            add_to_log_le_txsn->setText(in_lst.at(i));
            continue;
        }
        else if (i==15)//
        {
            add_to_log_le_rxsn->setText(in_lst.at(i));
            continue;
        }
        else if (i==16)//
        {
            add_to_log_le_txex->setText(in_lst.at(i));
            continue;
        }
        else if (i==17)//
        {
            add_to_log_le_rxex->setText(in_lst.at(i));
            continue;
        }

        // no problem 13, 20, empty enum sec start end
        if (i!=2 && i!=18 && i!=19)//stop date end hiden 2 and 18 cont id and 19contest MULTI-TWO hiden
        {
            if (i==1) str_n.append(tr("From")+" "+in_lst.at(i));// UTC Start                
            else if (i==3) str_n.append(tr("To")+" "+in_lst.at(i));// UTC End                
            else str_n.append(in_lst.at(i));                
            //if (i==0 || i==2)
            //str_n.append("-");
            //else
            str_n.append(" ");
        }
    }
    //qDebug()<<in_lst;
    add_to_log_txt->setText(tr("Do you want to add the following entry in the log?")+"\n"+str_n);
    // b_add_to_log_cacel->setFocus();
    add_to_log_le->setFocus();

    //timer_canc_add_to_log_dialog->start(60000);//wait 1min
    /*QPoint post = w_parent->mapToGlobal(QPoint(0,0));
    post+=QPoint(w_parent->width()/2-240,(-w_parent->pos().y()/2+20));
    add_to_log_dialog->move(post);*/

    add_to_log_dialog->exec();

    //b_add_to_log_cacel->setFocus();
}
void HvLogW::SetAutoLogInfo()//2.75
{
    l_txsn->setHidden(true);//f_addtolog = false; HV no use for this function
    add_to_log_le_txsn->setHidden(true);
    l_rxsn->setHidden(true);
    add_to_log_le_rxsn->setHidden(true);
    l_txex->setHidden(true);
    add_to_log_le_txex->setHidden(true);
    l_rxex->setHidden(true);
    add_to_log_le_rxex->setHidden(true);

    add_to_log_dialog->setWindowTitle(tr("Auto Logging Info Settings"));
    l_warn->setHidden(true);
    add_to_log_txt->setHidden(true);
    l_freq->setHidden(true);
    add_to_log_le_frq->setHidden(true);
    if (!f_off_auto_comm)//2.76.3
    {
    	l_comment->setHidden(true);
    	add_to_log_le->setHidden(true);    	
   	}
    b_add_to_log_ok->setHidden(true);
    b_add_to_log_cacel->setText(tr("Close"));

    add_to_log_dialog->exec();

    add_to_log_dialog->setWindowTitle(tr("ADD TO LOG"));
    l_warn->setHidden(false);
    add_to_log_txt->setHidden(false);
    l_freq->setHidden(false);
    add_to_log_le_frq->setHidden(false);
    l_comment->setHidden(false);
    add_to_log_le->setHidden(false);
    b_add_to_log_ok->setHidden(false);
    b_add_to_log_cacel->setText(tr("Cancel"));//f_addtolog = true; HV no use for this function
}
/*QString HvLogW::GetFREQall(int id, int l_row) //ID 0=ADIF, 1=LOGEDQSO, 2=Cabrillo
{
	QString s = "";
	QString aHz = "000";
	if (id==2) aHz = "";

	//QString a10[6]={"0.475", "3.573", "14.074", "3400.065", "5650.065", "10368.200"};
	//QString a10[6]={"", "", "", "", "", ""};
	//QString a9[6] ={"472 kHz", "3.5 MHz", "14 MHz", "3.4 GHz", "5.65 GHz", "10 GHz"};
	//for (int i=0; i<6; ++i)
	//{

	//0.475, 3.573, 14.074, 3400.065
	s = THvLogList->model.item(l_row, 10)->text();//s = a10[i];
    if (!s.isEmpty())
    {
        s.append(aHz);
        if (s.at(0) == '0') s = s.mid(1,s.count()-1);
    }
    if (s.isEmpty())
    {
		//472 kHZ, 3.5 MHz, 14 MHz, 3.4 GHz
        s = THvLogList->model.item(l_row, 9)->text();//s = a9[i]; FREQ for the moment form band 9 -> 10 reserved for freq
        if (s.contains("GHz"))
        {
            s.remove(" GHz");
            double d = s.toDouble()*1000.0;
            s = QString("%1").arg(d)+".000"+aHz;
        }
        else if (s.contains("MHz"))
        {
            s.remove(" MHz");
            s = QString("%1").arg(s.toDouble(),0,'f',3)+aHz;
        }
        else if (s.contains("kHz"))
        {
            s.replace(" kHz",aHz);
            if (id==0) s.prepend(".");
        }
    }
    if (id!=0) s.remove(".");

    // output = adif 		.136000 .472000 1.800000	in MHz to Hz
	// output = loged qso 	 136000 1800000 to Hz
	// output = cabrillo     136 1800 to kHz
	//qDebug()<<s;
	//}

	return s;
}*/
QString HvLogW::MakeAdifString(int l_row)
{
    QString s;
    QString out;

    s = s_my_call;//STATION_CALLSIGN
    out.append("<STATION_CALLSIGN:"+QString("%1").arg(s.count())+">"+s);
    s = s_my_grid;//MY_GRIDSQUARE
    out.append("<MY_GRIDSQUARE:"+QString("%1").arg(s.count())+">"+s);

    s = THvLogList->model.item(l_row, 4)->text();            //call
    out.append("<CALL:"+QString("%1").arg(s.count())+">"+s);
    s = THvLogList->model.item(l_row, 5)->text();            //gridsquare loc
    out.append("<GRIDSQUARE:"+QString("%1").arg(s.count())+">"+s);

    s = THvLogList->model.item(l_row, 21)->text();//distance
    out.append("<DISTANCE:"+QString("%1").arg(s.count())+">"+s);

    /*s = THvLogList->model.item(l_row, 8)->text();          //old method mode
    out.append("<MODE:"+QString("%1").arg(s.count())+">"+s);*/

    //2.21 mode Adif 3.1.0 <mode:4>MFSK <submode:3>FT4 <-submode exeption ????
    s = THvLogList->model.item(l_row, 8)->text();
    if (s=="FT2")
    {
        out.append("<MODE:4>MFSK");
        out.append("<SUBMODE:"+QString("%1").arg(s.count())+">"+s);
    }
    else if (s=="FT4")
    {
        out.append("<MODE:4>MFSK");
        out.append("<SUBMODE:"+QString("%1").arg(s.count())+">"+s);
    }
    else if (s=="ISCAT-A" || s=="ISCAT-B")
    {
        out.append("<MODE:5>ISCAT");
        out.append("<SUBMODE:"+QString("%1").arg(s.count())+">"+s);
    }
    else if (s=="JT65A" || s=="JT65B" || s=="JT65C")
    {
        out.append("<MODE:4>JT65");
        out.append("<SUBMODE:"+QString("%1").arg(s.count())+">"+s);
    }
    else if (s=="Q65A" || s=="Q65B" || s=="Q65C" || s=="Q65D")//2.55
    {
        out.append("<MODE:4>MFSK");  //test -> s = "Q65";
        out.append("<SUBMODE:"+QString("%1").arg(s.count())+">"+s);
    }
    else out.append("<MODE:"+QString("%1").arg(s.count())+">"+s);
    //qDebug()<<out;

    s = THvLogList->model.item(l_row, 6)->text();            //rst_sent
    out.append("<RST_SENT:"+QString("%1").arg(s.count())+">"+s);
    s = THvLogList->model.item(l_row, 7)->text();            //rst_rcvd
    out.append("<RST_RCVD:"+QString("%1").arg(s.count())+">"+s);
    s = THvLogList->model.item(l_row, 0)->text();            //qso_date on
    out.append("<QSO_DATE:"+QString("%1").arg(s.count())+">"+s);
    s = THvLogList->model.item(l_row, 1)->text();            //time_on
    s.remove(":");
    s.append(THvLogList->model.item(l_row, 13)->text());//1.82 enum sec
    out.append("<TIME_ON:"+QString("%1").arg(s.count())+">"+s);

    s = THvLogList->model.item(l_row, 2)->text();            //qso_date off
    out.append("<QSO_DATE_OFF:"+QString("%1").arg(s.count())+">"+s);
    s = THvLogList->model.item(l_row, 3)->text();            //time_off
    s.remove(":");
    s.append(THvLogList->model.item(l_row, 20)->text());//1.82 enum sec end
    out.append("<TIME_OFF:"+QString("%1").arg(s.count())+">"+s);

    s = THvLogList->model.item(l_row, 9)->text();            //band
    s = GetBandInLambda(s); //qDebug()<<s;
    out.append("<BAND:"+QString("%1").arg(s.count())+">"+s);

    s = THvLogList->model.item(l_row, 10)->text();
    if (!s.isEmpty())
    {
        s.append("000");
        if (s.at(0) == '0') s = s.mid(1,s.count()-1);//2.31
    }
    if (s.isEmpty())
    {
        s = THvLogList->model.item(l_row, 9)->text();  //FREQ for the moment form band 9 -> 10 reserved for freq
        if (s.contains("GHz"))
        {
            s.remove(" GHz"); //s = QString("%1").arg((int)(s.toDouble()*1000.0))+".000000";
            double d = s.toDouble()*1000.0;
            s = QString("%1").arg(d)+".000000";
        }
        else if (s.contains("MHz"))
        {
            s.remove(" MHz");
            s = QString("%1").arg(s.toDouble(),0,'f',3)+"000";
        }
        else if (s.contains("kHz"))
        {
            s.replace(" kHz","000");
            s.prepend(".");
        }
    }
    //s = GetFREQall(0,l_row);
    //output = "adif 3.1" .136000 .472000 1.800000	in MHz to Hz
    //qDebug()<<"ADIF="<<s;
    out.append("<FREQ:"+QString("%1").arg(s.count())+">"+s);
    //za sega fiktivno in mhz end

    s = THvLogList->model.item(l_row, 11)->text();// prop
    if (!s.isEmpty()) out.append("<PROP_MODE:"+QString("%1").arg(s.count())+">"+s);//2.65

    s = THvLogList->model.item(l_row, 12)->text();    //comment
    if (!s.isEmpty()) out.append("<COMMENT:"+QString("%1").arg(s.count())+">"+s);//2.65

    s = THvLogList->model.item(l_row,22)->text();
    if (!s.isEmpty()) out.append("<SAT_NAME:"+QString("%1").arg(s.count())+">"+s);
    s = THvLogList->model.item(l_row,23)->text();
    if (!s.isEmpty()) out.append("<SAT_MODE:"+QString("%1").arg(s.count())+">"+s);
    s = THvLogList->model.item(l_row,24)->text();
    if (!s.isEmpty()) out.append("<FREQ_RX:"+QString("%1").arg(s.count())+">"+s);

    //sn = THvLogList->model.item(l_row, 14)->text().toInt();//TXSn
    s = THvLogList->model.item(l_row, 14)->text();//QString("%1").arg(sn);
    if (!s.isEmpty()) out.append("<STX:"+QString("%1").arg(s.count())+">"+s);//2.65
    //int sn = THvLogList->model.item(l_row, 15)->text().toInt();//RXSn  //or <STX_STRING:3>044 <SRX_STRING:3>045
    s = THvLogList->model.item(l_row, 15)->text();//QString("%1").arg(sn);
    if (!s.isEmpty()) out.append("<SRX:"+QString("%1").arg(s.count())+">"+s);//2.65

    QString rxexch_ = THvLogList->model.item(l_row, 17)->text();
    QStringList l_rxexch_ = rxexch_.split(" ");
    l_rxexch_.append("");
    l_rxexch_.append("");
    s = l_rxexch_.at(0);
    if (!s.isEmpty()) out.append("<APP_N1MM_EXCHANGE1:"+QString("%1").arg(s.count())+">"+s); //2.65 adif -> CLASS   ARRL_CLASS
    s = l_rxexch_.at(1);
    if (!s.isEmpty()) out.append("<ARRL_SECT:"+QString("%1").arg(s.count())+">"+s);//2.65
    //out.append("<STATE:"+QString("%1").arg(s.count())+">"+s); //???   STATE:

    s = THvLogList->model.item(l_row, 18)->text();//contest id
    int id_cont_t = s.toInt();
    if (id_cont_t>0)
    {
        s = s_cont_name_id[id_cont_t];
        out.append("<CONTEST_ID:"+QString("%1").arg(s.count())+">"+s);
    }

    // Activity Type                id	type	dec-id       dec-type	dec-cq
    //"Standard"					0	0		0 = CQ		 0			0
    //"EU RSQ And Serial Number"	1	NONE	1  NONE		 NONE		NONE
    //"NA VHF Contest"				2	2		2  CQ TEST	 1			3 = CQ TEST
    //"EU VHF Contest"				3 	3		3  CQ TEST	 2			3 = CQ TEST
    //"ARRL Field Day"				4	4		4  CQ FD	 3			2 = CQ FD
    //"ARRL Inter. Digital Contest"	5	2		5  CQ TEST   1 			3 = CQ TEST
    //"WW Digi DX Contest"			6	2		6  CQ WW	 1			4 = CQ WW
    //"FT4 DX Contest"				7	2		7  CQ WW	 1			4 = CQ WW
    //"FT8 DX Contest"				8	2		8  CQ WW	 1			4 = CQ WW
    //"FT Roundup Contest"			9	5		9  CQ RU	 4			1 = CQ RU
    //"Bucuresti Digital Contest"	10 	5		10 CQ BU 	 4			5 = CQ BU
    //"FT4 SPRINT Fast Training"	11 	5		11 CQ FT 	 4			6 = CQ FT
    //"PRO DIGI Contest"			12  5		12 CQ PDC 	 4			7 = CQ PDC
    //"CQ WW VHF Contest"			13	2		13 CQ TEST	 1			3 = CQ TEST
    //"Q65 Pileup" or "Pileup"		14	2		14 CQ 		 1			0 = CQ
    //"NCCC Sprint"					15	2		15 CQ NCCC	 1			8 = CQ NCCC
    //"ARRL Inter. EME Contest"		16	6		16 CQ 		 0			0 = CQ
    //"FT Challenge Contest"		17  6       17 CQ FTC    0          9 = CQ FTC

    //2.64 All RU+FD  id_cont_t==7  id_cont_t==8
    if (id_cont_t==4 || id_cont_t==9 || id_cont_t==10 || id_cont_t==11 || id_cont_t==12)
    {
        s = THvLogList->model.item(l_row, 16)->text();//my tx exchange
        if (!s.isEmpty()) out.append("<APP_MSHV_EXCHANGE_TX:"+QString("%1").arg(s.count())+">"+s);
    }

    s = THvLogList->model.item(l_row, 19)->text();//contest MULTI-TWO hiden
    if (s.toInt()>0) out.append("<APP_N1MM_RUN1RUN2:"+QString("%1").arg(s.count())+">"+s); //2.65 <APP_N1MM_RUN1RUN2:1>0

    //n1mm rx-><APP_N1MM_EXCHANGE1:2>2A  rx-><ARRL_SECT:2>AL  <CONTEST_ID:14>ARRL-FIELD-DAY ????
    //s = qslmsg; if (!s.isEmpty()) out.append("<QSLMSG:"+QString("%1").arg(s.count())+">"+s);

    out.append("<EOR>\n");

    return out;
}
QStringList HvLogW::MakeLoggedQSO(int l_row)
{
    QStringList ls;
    QString dt_start = THvLogList->model.item(l_row, 0)->text()+" "+THvLogList->model.item(l_row, 1)->text();
    QString dt_end   = THvLogList->model.item(l_row, 2)->text()+" "+THvLogList->model.item(l_row, 3)->text();
    dt_start.append(":"+THvLogList->model.item(l_row, 13)->text());//1.82 enum seconds 00-59;
    dt_end.append(":"+THvLogList->model.item(l_row, 20)->text());  //1.82 enum seconds end 00-59;

    ///QString("%1").arg(nrpt,2,10,QChar('0'));
    ls << dt_end;                                                //0 qso_date off time_off
    ls << THvLogList->model.item(l_row, 4)->text();//.toUtf8();  //1 call
    ls << THvLogList->model.item(l_row, 5)->text();//.toUtf8();  //2 gridsquare loc
    QString s;												     //3 freq
    s = THvLogList->model.item(l_row, 10)->text();
    if (!s.isEmpty())
    {
        s.append("000");
    }
    if (s.isEmpty())
    {
        s = THvLogList->model.item(l_row, 9)->text();  //FREQ for the moment form band 9 -> 10 reserved for freq
        if (s.contains("GHz"))
        {
            s.remove(" GHz");
            double d = s.toDouble()*1000.0;
            s = QString("%1").arg(d)+"000000";
        }
        else if (s.contains("MHz"))
        {
            s.remove(" MHz");
            s = QString("%1").arg((int)(s.toDouble()*1000.0))+"000";
        }
        else if (s.contains("kHz")) s.replace(" kHz","000");
    }
    s = s.remove(".");
    if (s.at(0) == '0') s = s.mid(1,s.count()-1);//2.31
    //s = GetFREQall(1,l_row);
    //output= 136000 1800000 to Hz
    //qDebug()<<"LoggedQSO="<<s;
    ls << s;													 //3 freq
    ls << THvLogList->model.item(l_row, 8)->text();//.toUtf8();  //4 mode
    ls << THvLogList->model.item(l_row, 6)->text();//.toUtf8();  //5 rst_sent
    ls << THvLogList->model.item(l_row, 7)->text();//.toUtf8();  //6 rst_rcvd
    ls << "";												     //7 tx pwr
    ls << THvLogList->model.item(l_row, 12)->text();//.toUtf8(); //8 comment
    ls << "";												     //9 name
    ls << dt_start;                                              //10 qso_date on  time_on
    ls << "";												     //11 operator_call
    ls << s_my_call;//.toUtf8(); 								 //12 my_call
    ls << s_my_grid;//.toUtf8(); 								 //13 my_grid
    ls << THvLogList->model.item(l_row, 16)->text();			 //14 16-my tx exch contest exchange_sent
    ls << THvLogList->model.item(l_row, 17)->text();			 //15 17-my rx exch contest exchange_rcvd
    ls << THvLogList->model.item(l_row, 11)->text();			 //16 //11  Propagation //2.46
    ls << THvLogList->model.item(l_row, 22)->text();			 //17 Satellite
    ls << THvLogList->model.item(l_row, 23)->text();			 //18 Sat Mode
    ls << THvLogList->model.item(l_row, 24)->text();			 //19 RX Freq MHz

    return ls;
}
/*void HvLogW::SetTcpBroadLoggedAdif(bool f)
{
	s_tcp_broad_logged_adif = f;
}*/
void HvLogW::SetUdpBroadLoggedAll(bool fqso,bool fadif)
{
    s_udp_broad_logged_qso = fqso;  //qDebug()<<fqso<<fadif;
    s_udportcp_broad_logged_adif = fadif;
}
void HvLogW::SetDistUnit(bool f)
{
    f_km_mi = f;
}
QString HvLogW::CalcDistance(QString hloc)
{
    QString res = "";
    if (!THvQthLoc.isValidLocator(hloc) || !THvQthLoc.isValidLocator(s_my_grid)) return res;

    QString c_test_loc = THvQthLoc.CorrectLocator(hloc);
    QString c_my_loc = THvQthLoc.CorrectLocator(s_my_grid);

    double dlong1 = THvQthLoc.getLon(c_my_loc);
    double dlat1  = THvQthLoc.getLat(c_my_loc);
    double dlong2 = THvQthLoc.getLon(c_test_loc);
    double dlat2 = THvQthLoc.getLat(c_test_loc);

    if (!f_km_mi)
    {
        res = QString("%1").arg(THvQthLoc.getDistanceKilometres(dlong1,dlat1,dlong2,dlat2));
    }
    else
    {
        res = QString("%1").arg(THvQthLoc.getDistanceMilles(dlong1,dlat1,dlong2,dlat2));
    }

    return res;
}
bool HvLogW::Insert(QStringList lst, bool save_changes,bool warning_msg,int show_cont_id,bool f_maxqsoc,bool add_prop_info)
{
    if (lst.count() < MIN_CCOUNT) return false;//2.73 21to25 w10 protection    		    	
    if (!lst.empty() && warning_msg)
    {
        if (show_cont_id==0)//show_cont_id 0=no, 1=sn, 2=exch 3=sn+exch
        {
            l_txsn->setHidden(true);
            add_to_log_le_txsn->setHidden(true);
            l_rxsn->setHidden(true);
            add_to_log_le_rxsn->setHidden(true);
            l_txex->setHidden(true);
            add_to_log_le_txex->setHidden(true);
            l_rxex->setHidden(true);
            add_to_log_le_rxex->setHidden(true);
        }
        else if (show_cont_id==1)
        {
            l_txsn->setHidden(false);
            add_to_log_le_txsn->setHidden(false);
            l_rxsn->setHidden(false);
            add_to_log_le_rxsn->setHidden(false);
            l_txex->setHidden(true);
            add_to_log_le_txex->setHidden(true);
            l_rxex->setHidden(true);
            add_to_log_le_rxex->setHidden(true);
        }
        else if (show_cont_id==2)
        {
            l_txsn->setHidden(true);
            add_to_log_le_txsn->setHidden(true);
            l_rxsn->setHidden(true);
            add_to_log_le_rxsn->setHidden(true);
            l_txex->setHidden(false);
            add_to_log_le_txex->setHidden(false);
            l_rxex->setHidden(false);
            add_to_log_le_rxex->setHidden(false);
        }
        else if (show_cont_id==3)
        {
            l_txsn->setHidden(false);
            add_to_log_le_txsn->setHidden(false);
            l_rxsn->setHidden(false);
            add_to_log_le_rxsn->setHidden(false);
            l_txex->setHidden(false);
            add_to_log_le_txex->setHidden(false);
            l_rxex->setHidden(false);
            add_to_log_le_rxex->setHidden(false);
        }

        StartAddToDialog(lst);

        if (f_addtolog)
        {
            lst[12] = addtolog_comment;// comment           
            lst[11] = addtolog_prop;   // prop
            lst[10] = addtolog_freq;
            lst[14] = addtolog_txsn;
            lst[15] = addtolog_rxsn;
            lst[16] = addtolog_txex;
            lst[17] = addtolog_rxex;
            lst[22] = addtolog_satn;
            lst[23] = addtolog_satm;
            lst[24] = addtolog_rxfr;
        }
        else return false;
    }

    if (add_prop_info && cb_enable_ali->isChecked() && !warning_msg)//2.75
    {
        int id = add_to_log_cb_prop->currentIndex();
        lst[11] = s_id_prop_mod[id];//prop
        if (cb_sat_nam->isEnabled())
        {
            id = cb_sat_nam->currentIndex();
            lst[22] = s_id_sat_nam[id];//sat
        }
        else lst[22] = "";
        if (cb_sat_mod->isEnabled())
        {
            id = cb_sat_mod->currentIndex();
            lst[23] = s_id_sat_mod[id];//sat mod
        }
        else lst[23] = "";
        if (le_rx_freq->isEnabled()) lst[24] = le_rx_freq->text();//rx freq 2.75
        else lst[24] = "";  
        if (f_off_auto_comm) lst[12] = add_to_log_le->text();//2.76.3         
    }

    if (lst.at(21).isEmpty())//readEDI readADIF ....
    {
        QString distt = CalcDistance(lst.at(5)); //qDebug()<<s_my_grid<<lst.at(5)<<distt;
        lst.replace(21,distt);
    }
    THvLogList->InsertItem_hv(lst);

    if (save_changes)
    {
        int rowt = THvLogList->model.rowCount()-1;
        if (!timer_refr_save->isActive())//2.48
        {
            beg_append = rowt;
            timer_refr_save->start(500);
        }

        //int rowt = THvLogList->model.rowCount()-1;
        if (isVisible())//1.81 app frize
        {
            THvLogList->setCurrentIndex(THvLogList->model.index(rowt,0));
            THvLogList->scrollToBottom();
        }

        End_Edit();
        End_Add_Man_Qso();

        if (s_udp_broad_logged_qso) emit EmitLoggedQSO(MakeLoggedQSO(rowt));
        if (s_udportcp_broad_logged_adif) emit EmitAdifRecord(MakeAdifString(rowt));
    }

    l_qsos_in_log->setText(QString("%1").arg(THvLogList->model.rowCount()));

    if (f_maxqsoc) CheckLogCount();

    return true;
}
void HvLogW::AddManQsoToList(QStringList list,int)
{
    QString distt = CalcDistance(list.at(5));
    list.replace(21,distt);
    THvLogList->InsertItem_hv(list);

    SaveAllExternal(false);
    //all_exp=0 sel_exp=1  save_all_for_ext_log_prog=2 append_all_for_ext_log_prog=3
    if (ac_use_adif_save->isChecked()) ExportToAdif("",3,false);//append_all_for_ext_log_prog=3

    int rowt = THvLogList->model.rowCount()-1;
    THvLogList->setCurrentIndex(THvLogList->model.index(rowt,0));
    THvLogList->scrollToBottom();
    l_qsos_in_log->setText(QString("%1").arg(THvLogList->model.rowCount()));

    if (s_udp_broad_logged_qso) emit EmitLoggedQSO(MakeLoggedQSO(rowt));
    if (s_udportcp_broad_logged_adif) emit EmitAdifRecord(MakeAdifString(rowt));

    if (s_allowed_modes && (f_isCheck[0] || f_isCheck[3])) GetMarkTextAllFromLog_p(s_band,s_mode);

    CheckLogCount();
}
void HvLogW::SetCorrContact(QStringList list,int index_edit)
{
    QString distt = CalcDistance(list.at(5));
    list.replace(21,distt);
    THvLogList->SetItem_hv(list,index_edit);

    SaveAllExternal(false);
    //all_exp=0 sel_exp=1  save_all_for_ext_log_prog=2 append_all_for_ext_log_prog=3
    //ExportToAdif("",2);//save_all_for_ext_log_prog=2

    if (s_allowed_modes && (f_isCheck[0] || f_isCheck[3])) GetMarkTextAllFromLog_p(s_band,s_mode);
}
void HvLogW::SaveAllExternal(bool f)//  f true=append false=save all
{
    fsave_busy = true;  //qDebug()<<"start save"<<fsave_busy;
    SaveEDI(AppPath+"/log/mshvlog.edim",f); //qDebug()<<"stop save"<<fsave_busy;
}
#include <unistd.h>
void *HvLogW::ThreadSaveLog(void *argom)
{
    HvLogW* pt = (HvLogW*)argom;
    //all_exp=0 sel_exp=1  save_all_for_ext_log_prog=2 append_all_for_ext_log_prog=3
    if (pt->ac_use_adif_save->isChecked()) pt->ExportToAdif("",3,false);//2.65 save_all_for_ext_log_prog=3
    usleep(20000);//2.67 ???
    pt->SaveAllExternal(true);
    pthread_detach(pt->ths);
    pthread_exit(NULL);
    return NULL;
}
void HvLogW::RefreshSave()//1.81 inportent for big logs
{
    if (fsave_busy) return;
    timer_refr_save->stop();
    fsave_busy = true;
    pthread_create(&ths,NULL,HvLogW::ThreadSaveLog,(void*)this);
}
void HvLogW::CreateNewLogAndSaveOld()
{
    CreateNBLog(true);
}
void HvLogW::CreateBackupLog()
{
    CreateNBLog(false);
}
void HvLogW::CreateNBLog(bool f)
{
    End_Edit();
    End_Add_Man_Qso();

    QString tstr = tr("Save QSOs In Backup File");
    if (f) tstr = tr("Save QSOs And Create New Log");

    THvProgrD->Show(tstr,tr("Please Wait")+" ",7);
    int exp_qso = 1;
    THvProgrD->SetValue(exp_qso);

    QString nqsos = QString("%1").arg(THvLogList->model.rowCount());
    QDateTime utc_t = QDateTime::currentDateTimeUtc();
    QString str = QString(AppPath+"/log/"+"backup_log_"+s_my_call+"_"+utc_t.toString("yyyyMMddhhmmss")+".edim");
    fsave_busy = true;//2.48
    SaveEDI(str,false);

    if (f)
    {
        exp_qso++;
        THvProgrD->SetValue(exp_qso);
        THvLogList->selectAll();
        exp_qso++;
        THvProgrD->SetValue(exp_qso);
        THvLogList->DeleteSel();
        exp_qso++;
        THvProgrD->SetValue(exp_qso);
        SaveAllExternal(false);
    }
    exp_qso++;
    THvProgrD->SetValue(exp_qso);

    if (f) l_qsos_in_log->setText(QString("%1").arg(THvLogList->model.rowCount()));

    if (f && s_allowed_modes && (f_isCheck[0] || f_isCheck[3])) GetMarkTextAllFromLog_p(s_band,s_mode);

    THvProgrD->close();

    tstr = nqsos+" "+tr("QSOs Have Been Successfully Saved In MSHV Log Format.")+"\n";
    if (f) tstr = nqsos+" "+tr("QSOs Have Been Successfully Saved In MSHV Log Format  And A New Log Has Been Created.")+"\n";

    QMessageBox::information(this, "MSHV", tstr+tr("Full Path And File Name is")+":\n"+str, QMessageBox::Ok);
}
//QTime ppp;
void HvLogW::SaveEDI(QString path,bool fappend)
{
    if (save_in_new_format) //old logs
    {
        save_in_new_format = false;
        fappend = false; //save all
    }

    QFile file(path);
    if (fappend)
    {
        if (!file.open(QIODevice::Append | QIODevice::Text))
            return;
    }
    else
    {
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
            return;
    }

    QStringList out;
    int b_app = beg_append;
    if (!fappend)
    {
        b_app = 0; //out << "["<< APP_NAME << " LOG FILE]" << "\n";
        out << _LOG_ID_ << "\n";//no crash old versions
    }
    //qDebug()<<"Save="<<fappend<<b_app<<THvLogList->model.rowCount();
    for (int j = b_app;  j < THvLogList->model.rowCount(); ++j)
    {
        //out << QDate::fromString(THvLogList->model.item(j, 10)->text(), "dd/MM/yy").toString("yyMMdd") <<";";//date
        // "Date"<<"UTC"<<"Callsign"<<"Locator"<<"Mode"<<"Band"<<"RptTx"<<"RptRX";
        out << THvLogList->model.item(j, 0)->text()<<";";//date start
        out << THvLogList->model.item(j, 1)->text().remove(":")<<";";//time start
        out << THvLogList->model.item(j, 2)->text()<<";";//date end hiden
        out << THvLogList->model.item(j, 3)->text().remove(":")<<";";//time end
        out << THvLogList->model.item(j, 4)->text()<<";";//call

        QString mode_code=THvLogList->model.item(j, 8)->text();//mode
        if      (mode_code=="NON") mode_code="0";
        else if (mode_code=="SSB") mode_code="1";
        else if (mode_code=="CW") mode_code="2";
        //else if (mode_code=="SSB/CW") mode_code="3";
        //else if (mode_code=="CW/SSB") mode_code="4";
        //else if (mode_code=="AM") mode_code="5";
        else if (mode_code=="FM") mode_code="6";
        //else if (mode_code=="RTTY") mode_code="7";
        //else if (mode_code=="SSTV") mode_code="8";
        //else if (mode_code=="ATV") mode_code="9";
        else if (mode_code=="JTMS") mode_code="10";
        else if (mode_code=="FSK441") mode_code="11";
        else if (mode_code=="ISCAT-A") mode_code="12";
        else if (mode_code=="ISCAT-B") mode_code="13";
        else if (mode_code=="JT6M") mode_code="14";
        else if (mode_code=="FSK315") mode_code="15";
        //else if (mode_code=="JTMSK") mode_code="16";
        else if (mode_code=="MSK144") mode_code="17";
        else if (mode_code=="JT65A") mode_code="18";
        else if (mode_code=="JT65B") mode_code="19";
        else if (mode_code=="JT65C") mode_code="20";
        else if (mode_code=="PI4") mode_code="21";
        else if (mode_code=="FT8") mode_code="22";
        else if (mode_code=="MSKMS") mode_code="23";
        else if (mode_code=="FT4") mode_code="24";
        else if (mode_code=="Q65A") mode_code="25";
        else if (mode_code=="Q65B") mode_code="26";
        else if (mode_code=="Q65C") mode_code="27";
        else if (mode_code=="Q65D") mode_code="28";
        else if (mode_code=="FT2") mode_code="29";

        out << mode_code<<";";//Mode code

        out << THvLogList->model.item(j, 6)->text()<<";";//Sent-RST       tx
        //out << ""<<";";//Sent QSO number
        out << THvLogList->model.item(j, 7)->text()<<";";//Received-RST   rx
        //out << ""<<";";//Received QSO number
        //out << "" <<";";//Received exchange
        out << THvLogList->model.item(j, 5)->text()<<";";//Received-WWL   loc
        //out << ""<<";";//QSO-Points
        //out << "" <<";";//New-Exchange-(N)

        //check for new New-WWL-
        //QString strWWL = Ceck_New_WWL(j);
        //if (strWWL=="N")
        //    count_WWLs++;
        //out << "" <<";";//New-WWL-(N)

        //out << "" <<";";//New-DXCC-(N)
        //out << "";//Duplicate-QSO-(D)
        QString str_band = THvLogList->model.item(j, 9)->text();  //      band
        str_band.remove(" ",Qt::CaseInsensitive);
        str_band.remove("Hz",Qt::CaseInsensitive);

        out << str_band <<";";//Band HV                                               9

        QString fnodot = THvLogList->model.item(j, 10)->text();
        fnodot.remove(".");
        fnodot.remove(",");
        if (fnodot.at(0) == '0') fnodot = fnodot.mid(1,fnodot.count()-1);//2.31
        out  <<fnodot<<";";//freq reserve from 1.31 HV                                10
        out  <<THvLogList->model.item(j, 12)->text()<<";";//comment                   11
        out  <<THvLogList->model.item(j, 11)->text()<<";";//prop                      12
        out  <<THvLogList->model.item(j, 14)->text()<<";";//my tx sn contest          13
        out  <<THvLogList->model.item(j, 15)->text()<<";";//my rx sn contest          14
        out  <<THvLogList->model.item(j, 16)->text()<<";";//my tx exch contest        15
        out  <<THvLogList->model.item(j, 17)->text()<<";";//my rx exch contest        16
        out  <<THvLogList->model.item(j, 18)->text()<<";";//2.01 contest id           17
        out  <<THvLogList->model.item(j, 13)->text()<<";";//enum seconds 00-59        18
        out  <<THvLogList->model.item(j, 19)->text()<<";";//contest MULTI-TWO hiden   19
        out  <<THvLogList->model.item(j, 20)->text()<<";";//enum seconds end 00-59    20
        out  <<THvLogList->model.item(j, 21)->text()<<";";//distance                  21
        out  <<THvLogList->model.item(j, 22)->text()<<";";//Satellite                 22
        out  <<THvLogList->model.item(j, 23)->text()<<";";//Sat Mode                  23
        out  <<THvLogList->model.item(j, 24)->text()<<";";//RX Freq                   24
        out  <<";";  //free reserve from 2.73 HV                                      25
        out  <<";";  //free reserve from 2.73 HV                                      26
        out  <<";"; //free reserve from 2.73 HV                                       27
        //out  <<""; //free reserve from 2.73 HV                                      28
        out<<"\n";
    }
    //out << "[END; LOG FILE " << APP_NAME << "]" << "\n";
    QTextStream out_s(&file);
    for (QStringList::iterator it =  out.begin(); it != out.end(); ++it)
    {
        QString sss = (*it);
        out_s<<sss;
    }
    file.close();
    fsave_busy = false;
    //qDebug()<<ppp.elapsed();
}
void HvLogW::SetMyCallGridExchAllCont(QString s,QString gr,QString cont_exch)
{
    s_my_call = s;
    s_my_grid = gr;
    QStringList l_exch = cont_exch.split("#");
    //if(l_exch.count()>=2)
    //{
    s_my_fd_exch = l_exch.at(0);//FD
    s_my_ru_exch = l_exch.at(1);//RU
    //}

    /*QStringList l_exchfd = s_my_fd_exch.split(" ");
    if (l_exchfd.count()>=2 && !l_exchfd.at(1).isEmpty())
        le_cabrillo_locat->setText(l_exchfd.at(1));
    else
        le_cabrillo_locat->setText("");*/

    if (s_my_ru_exch=="DX") f_iem_ru_dx = true;
    else f_iem_ru_dx = false;
    //qDebug()<<s_my_fd_exch<<s_my_ru_exch;
}

void HvLogW::ExportSelToAdif()
{
    if (THvLogList->selectionModel()->selection().empty())
    {
        QMessageBox::critical(this, "MSHV", tr("Please select QSO."), QMessageBox::Ok);
        return;
    }
    //all_exp=0 sel_exp=1  save_all_for_ext_log_prog=2 append_all_for_ext_log_prog=3
    ExportToAdif("SELECTED_QSO",1,true);
}
void HvLogW::ExportAllToAdif()
{
    if (THvLogList->model.rowCount()<1)
    {
        QMessageBox::critical(this, "MSHV", tr("No QSO In List."), QMessageBox::Ok);
        return;
    }
    //all_exp=0 sel_exp=1  save_all_for_ext_log_prog=2 append_all_for_ext_log_prog=3
    ExportToAdif("ALL_QSO",0,true);
}
QString HvLogW::GetBandInLambda(QString frq)
{
    QString res = "";
    for (int i = 0; i < COUNT_BANDS; i++)
    {
        if (frq == lst_bands[i])
        {
            res=lst_lambda[i];
            break;
        }
    }
    return res;
}
void HvLogW::ExportToAdif(QString ident,int f_sel_or_all,bool pbarr)
{
    //DefaultSort();
    //export_all=0 export_sel=1  save_all_for_ext_log_prog=2 append_all_for_ext_log_prog=3
    QDateTime utc_t = QDateTime::currentDateTimeUtc();
    QString file_Path_Name;// <- s towa za VQLog HV

    //all_exp=0 sel_exp=1  save_all_for_ext_log_prog=2 append_all_for_ext_log_prog=3
    if (f_sel_or_all==2 || f_sel_or_all==3)//1.53 for external progs
    {
        //Tova ne e prilovimo za momenta
        //problemi s LogHX3 triabva da se dobavia samo po edno qso->f_sel_or_all=3
        //I triabva sled close MSHV da se iztrivat vsi4ki
        //Ne podarza funkciata edit i delete qso->f_sel_or_all=2
        file_Path_Name = QString(AppPath+"/log/mshvlog.adi");
    }
    else
    {
        QString call = s_my_call;
        if (call.contains("/"))
        {
            if (call.at(call.count()-1)=='/')
                call.replace("/","_SLASH");
            else
                call.replace("/","_SLASH_");
        }
        QString dir_path_name = QString(AppPath+"/ExportLog/");
        QString file_name = QString("/"+call+"_"+utc_t.toString("yyyyMMdd")+"_"+ident);
        //qDebug()<<dir_path_name+file_name;

        QFileDialog fileDialog(this, "Save File",dir_path_name+file_name, "ADI File (*.adi)");
        fileDialog.setDefaultSuffix("adi");
        fileDialog.setAcceptMode(QFileDialog::AcceptSave);
        //fileDialog.setLabelText(QFileDialog::Reject, tr("Eaioae"));
        //fileDialog.setLabelText(QFileDialog::Accept, tr("Naaa"));
        //fileDialog.setLabelText(QMessageBox::Ok, tr("Aa"));
        //fileDialog.setLabelText(QMessageBox::Cancel, tr("Iao"));

        QStringList list;
        if (fileDialog.exec()) list = fileDialog.selectedFiles();

        if (list.isEmpty()) return;
        else file_Path_Name = *list.begin();

        if (file_Path_Name.isEmpty()) return;
    }

    //all_exp=0 sel_exp=1  save_all_for_ext_log_prog=2 append_all_for_ext_log_prog=3
    QFile file(file_Path_Name);
    if (f_sel_or_all==3)
    {
        if (!file.open(QIODevice::Text | QIODevice::Append)) return;
    }
    else
    {
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return;
    }

    int to = 0;
    int from = 0;
    QList<int> rows;
    //all_exp=0 sel_exp=1  save_all_for_ext_log_prog=2 append_all_for_ext_log_prog=3
    if (f_sel_or_all==1)
    {
        QModelIndexList lst = THvLogList->selectionModel()->selectedRows(0);
        foreach( const QModelIndex & index, lst )
        {
            rows.append( index.row() );
        }
        //q_Sort(rows);
        std::sort(rows.begin(),rows.end());
        /*QString str_n;
        for (int j = 0; j < rows.count(); j++)
        {
            for (int i = 0; i<THvLogList->model.columnCount(); i++)
            {
                str_n.append(THvLogList->model.item(rows[j], i)->text());
                str_n.append(" ");
            }
            str_n.append("\n");
        }*/
        to = rows.count();
        from = 0;
    }
    else if (f_sel_or_all==3)
    {
        //all_exp=0 sel_exp=1  save_all_for_ext_log_prog=2 append_all_for_ext_log_prog=3
        to = THvLogList->model.rowCount();
        from = beg_append;//2.65 THvLogList->model.rowCount()-1;
    }
    else
    {
        //all_exp=0 sel_exp=1  save_all_for_ext_log_prog=2 append_all_for_ext_log_prog=3
        to = THvLogList->model.rowCount();
        from = 0;
    }

    if (pbarr) THvProgrD->Show(tr("Export QSOs"),tr("Please Wait, Export QSOs")+" ",510);
    QTextStream out(&file);

    //all_exp=0 sel_exp=1  save_all_for_ext_log_prog=2 append_all_for_ext_log_prog=3
    QString verr  = (QString)VER_MS;
    QString headd = (QString)APP_NAME+" ADIF Export\n<ADIF_VER:5>3.1.0\n<PROGRAMID:4>MSHV\n"
                    "<PROGRAMVERSION:"+QString("%1").arg(verr.count())+">"+verr+"\n<EOH>\n";

    if (f_sel_or_all==3)
    {
        if (file.size()==0) out << headd;
        //if (file.size()>2048)
        if (file.size()>10666900) //5333450~20000-QSOs 10666900~40000-QSOs, 21333800~80000-QSOs
        {
            file.close();
            QFile filer(file_Path_Name);
            if (!filer.open(QIODevice::ReadOnly | QIODevice::Text)) return; //ReadWrite
            QTextStream in(&filer);
            QStringList lst0;
            while (!in.atEnd()) lst0 << in.readLine();
            filer.close();
            QFile filew(file_Path_Name);
            if (!filew.open(QIODevice::WriteOnly | QIODevice::Text)) return;
            QTextStream outw(&filew);
            outw << headd; // 5=count head
            for (int z = 5+((lst0.count()-5)/4); z < lst0.count(); ++z) outw << lst0.at(z) << "\n";
            for (int x=from;  x < to; x++) outw << MakeAdifString(x);
            filew.close();
            return;
        } //qDebug()<<file.size();
    }
    else out << headd;

    int j;
    int exp_qso = 0;
    for (j=from;  j < to; j++)
    {
        int l_row;

        //all_exp=0 sel_exp=1  save_all_for_ext_log_prog=2 append_all_for_ext_log_prog=3
        if (f_sel_or_all==1) l_row = rows[j];
        else l_row = j;

        out << MakeAdifString(l_row);
        exp_qso++;
        if (pbarr) THvProgrD->SetValue(exp_qso);
    }
    if (pbarr) THvProgrD->Finish(exp_qso);
    file.close();
    if (pbarr) THvProgrD->close();

    //all_exp=0 sel_exp=1  save_all_for_ext_log_prog=2 append_all_for_ext_log_prog=3
    if (f_sel_or_all==0 || f_sel_or_all==1)
        QMessageBox::information(this, "MSHV", tr("Successfully Exported")+" "+QString("%1").arg(exp_qso)+" "+tr("QSOs In ADIF Format.")+"\n"+
                                 tr("Full Path And File Name is")+":\n"+file_Path_Name, QMessageBox::Ok);
}
static int upld_qso_ = 0;
void HvLogW::SetUploadClubLogInfo(QString s)
{
    if (s=="-End-") THvProgrD->Finish(upld_qso_);
    else
    {
        THvProgrD->close();
        QMessageBox msgBox(this);
        msgBox.setWindowTitle("MSHV");
        msgBox.setIcon(QMessageBox::Information);
        msgBox.setText(s);
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.button(QMessageBox::Ok)->animateClick(10000);//after 10sec auto close
        msgBox.exec();
    }
}
void HvLogW::UploadSelToClubLog()
{
    QByteArray out;
    out.clear();
    QList<int> rows;
    if (THvLogList->selectionModel()->selection().empty())
    {
        QMessageBox::critical(this, "MSHV", tr("Please select QSO."), QMessageBox::Ok);
        return;
    }
    QModelIndexList lst = THvLogList->selectionModel()->selectedRows(0);
    foreach( const QModelIndex & index, lst )
    {
        rows.append( index.row() );
    }
    std::sort(rows.begin(),rows.end());//q_Sort(rows);
    THvProgrD->Show(tr("Upload QSOs"),tr("Please Wait, Uploading QSOs")+" ",510);
    upld_qso_ = 0;
    for (int j=0;  j < rows.count(); j++)
    {
        int l_row = rows[j];
        out.append(MakeAdifString(l_row).toUtf8());
        upld_qso_++;
        THvProgrD->SetValue(upld_qso_);
    }
    out.remove(out.size()-1,1);//remove last \n
    emit EmitUploadSelected(out);
}
void HvLogW::SetSettings(QString s)
{
    if (!s.isEmpty())
    {
        QStringList ls = s.split("#");
        if (ls.count()>9)
        {
            le_cabrillo_club->setText(ls.at(0));
            le_cabrillo_oper->setText(ls.at(1));
            le_cabrillo_name->setText(ls.at(2));
            le_cabrillo_email->setText(ls.at(3));
            le_cabrillo_addr->setText(ls.at(4));
            le_cabrillo_addr2->setText(ls.at(5));
            le_cabrillo_addr_city->setText(ls.at(6));
            le_cabrillo_addr_spro->setText(ls.at(7));
            le_cabrillo_addr_posc->setText(ls.at(8));
            le_cabrillo_addr_cntr->setText(ls.at(9));
        }
    }
}
void HvLogW::SetUseAdifSave(QString s)
{
    if (!s.isEmpty())
    {
        if (s=="0") ac_use_adif_save->setChecked(false);
    }
}
QString HvLogW::GetSettings()
{
    QString res;
    res.append(le_cabrillo_club->text()); //0
    res.append("#");
    res.append(le_cabrillo_oper->text()); //1
    res.append("#");
    res.append(le_cabrillo_name->text()); //2
    res.append("#");
    res.append(le_cabrillo_email->text()); //3
    res.append("#");
    res.append(le_cabrillo_addr->text()); //4
    res.append("#");
    res.append(le_cabrillo_addr2->text()); //5
    res.append("#");
    res.append(le_cabrillo_addr_city->text()); //6
    res.append("#");
    res.append(le_cabrillo_addr_spro->text()); //7
    res.append("#");
    res.append(le_cabrillo_addr_posc->text()); //8
    res.append("#");
    res.append(le_cabrillo_addr_cntr->text()); //9
    return res;
}
QString HvLogW::GetPropSettings()//2.75
{
    QString str;
    str.append(add_to_log_cb_prop->currentText());
    str.append("#");
    str.append(cb_sat_nam->currentText());
    str.append("#");
    str.append(cb_sat_mod->currentText());
    str.append("#");
    str.append(le_rx_freq->text());
    str.append("#");
    str.append(QString("%1").arg(cb_enable_ali->isChecked()));
    str.append("#");
    str.append(add_to_log_le->text());//2.76.3
    return str;
}
void HvLogW::SetPropSettings(QString s)
{
    QStringList ls = s.split("#");
    if (ls.count()>5)
    {
        int index = add_to_log_cb_prop->findText(ls.at(0),Qt::MatchCaseSensitive);
    	if (index >= 0) add_to_log_cb_prop->setCurrentIndex(index);
    	index = cb_sat_nam->findText(ls.at(1),Qt::MatchCaseSensitive);
    	if (index >= 0) cb_sat_nam->setCurrentIndex(index);
    	index = cb_sat_mod->findText(ls.at(2),Qt::MatchCaseSensitive);
    	if (index >= 0) cb_sat_mod->setCurrentIndex(index);
        le_rx_freq->setText(ls.at(3));
        if (ls.at(4)=="1") cb_enable_ali->setChecked(true);
        add_to_log_le->setText(ls.at(5));//2.76.3 
    }
}
void HvLogW::RefreshCbTrmN(int i)
{
    if ((i==2 || i==4 || i==5 || i==6 || i==17) && cb_cabrillo_trm->currentText()=="TWO") cb_cabrillo_trmN->setEnabled(true);
    else
    {
        cb_cabrillo_trmN->setEnabled(false);
        cb_cabrillo_trmN->setCurrentIndex(0);
    }
}
void HvLogW::CbContNameChanged(int i)
{
	int i1 = pos_cont[i];
    le_cabrillo_cont_id->setText(s_cont_name_id[i1]);

    if (i1==4)//FD
    {
        QStringList l_exchfd = s_my_fd_exch.split(" ");
        if (l_exchfd.count()>=2 && !l_exchfd.at(1).isEmpty()) le_cabrillo_locat->setText(l_exchfd.at(1));
        else le_cabrillo_locat->setText("");
    }
    //else if (i==5 || i==6 || i==7 || i==8 || i==9 || i==10 || i==11 || i==12 || i==13 || i==14 || i==15)//RU, WW Digi DX Contest, CQ WW VHF Contest
    else if (i1>4) le_cabrillo_locat->setText(s_my_ru_exch);
    else le_cabrillo_locat->setText("");

    RefreshCbTrmN(i1);
}
void HvLogW::CbContTrmChanged(int)
{
    RefreshCbTrmN(pos_cont[cb_cabrillo_cont->currentIndex()]);
}
bool HvLogW::StartCabriloDialog()
{
    f_cabr_ok = false;

    QString s_cdt = QDateTime::currentDateTimeUtc().toString("yyyyMMddhhmm");//HV 2.01 need to lost miliseconds
    exp_dte_start->setDateTime(QDateTime::fromString(s_cdt,"yyyyMMddhhmm"));//addMSecs addDays(-365) .addDays(-1)
    exp_dte_end->setDateTime(QDateTime::fromString(s_cdt,"yyyyMMddhhmm"));
    //le_cabrillo_oper->setText(s_my_call);

    ExportToCabrilloD->exec();
    return f_cabr_ok;
}
void HvLogW::OkCabr()
{
    if (pos_cont[cb_cabrillo_cont->currentIndex()]<1)
    {
        QMessageBox::critical(this, "MSHV", tr("Please select Contest Name."), QMessageBox::Ok);
        return;
    }
    int id_cont = pos_cont[cb_cabrillo_cont->currentIndex()];//0=my old 1=na vhf 2=eu vhf 3=arrl fd 4=arrl rtty roundup
    if (exp_dte_start->dateTime().secsTo(exp_dte_end->dateTime())<0)//<=0
    {
        QString text = tr("The starting Date and Time have to be earlier than the End Date and Time");
        QMessageBox::critical(this, "MSHV", text, QMessageBox::Ok);
        return;
    }
    unsigned int dte_start = exp_dte_start->dateTime().toTime_t();
    unsigned int dte_end   = exp_dte_end->dateTime().toTime_t();
    int rows =0;
    for (int i = 0; i < THvLogList->model.rowCount(); ++i)
    {
        int id_cont_t = THvLogList->model.item(i, 18)->text().toInt();
        if (id_cont_t==id_cont || cb_cabrillo_all_qso->isChecked())
        {
            QString dt = THvLogList->model.item(i, 2)->text();//date end
            QString tm = THvLogList->model.item(i, 3)->text();//time end
            QDateTime tdm = QDateTime::fromString(dt+tm,"yyyyMMddhh:mm");
            unsigned int idtm = tdm.toTime_t();
            if (dte_end>=idtm && dte_start<=idtm)
            {
                rows++;
                break; // min 1 QSO
            }
        }
    }
    if (rows<1)
    {
        QString sdt = "From: "+exp_dte_start->dateTime().toString("yyyyMMdd hh:mm");
        QString edt = " - To: "+exp_dte_end->dateTime().toString("yyyyMMdd hh:mm");
        QMessageBox::critical(this, "MSHV", tr("No QSOs In Log For")+" "+s_cont_name[id_cont]+"\n"
                              +sdt+edt+" .", QMessageBox::Ok);
        return;
    }

    f_cabr_ok = true;
    /*QString dt = THvLogList->model.item(0, 2)->text();//date end
    QString tm = THvLogList->model.item(0, 3)->text();//time end
    QDateTime tdm = QDateTime::fromString(dt+tm,"yyyyMMddhh:mm");
    int idtm = tdm.toTime_t();
    qDebug()<<dte_start;
    qDebug()<<idtm;
    qDebug()<<dte_end;*/

    ExportToCabrilloD->close();
}
void HvLogW::CancelCabr()
{
    ExportToCabrilloD->close();
}
QString HvLogW::GetCabriloMoStr(QString s) //for any qso mo= CW,PH,FM,RY,DG
{
    QString res = "DG";
    if 		(s=="CW"  ) res = "CW";
    else if (s=="SSB" ) res = "PH";
    else if (s=="FM"  ) res = "FM";
    else if (s=="RTTY") res = "RY";
    return res;
}
void HvLogW::ExportToCabr()
{
    if (!StartCabriloDialog()) return;

    int id_cont = pos_cont[cb_cabrillo_cont->currentIndex()];//0=none 1=my old 2=na vhf 3=eu vhf 4=arrl fd 5=arrl rtty roundup
    unsigned int dte_start = exp_dte_start->dateTime().toTime_t(); //qDebug()<<id_cont;
    unsigned int dte_end   = exp_dte_end->dateTime().toTime_t();
    QList<int> rows;
    for (int i = 0; i < THvLogList->model.rowCount(); ++i)
    {
        int id_cont_t = THvLogList->model.item(i, 18)->text().toInt();
        if (id_cont_t==id_cont || cb_cabrillo_all_qso->isChecked())// || cb_cabrillo_all_qso->isChecked()
        {
            QString dt = THvLogList->model.item(i, 2)->text();//date end
            QString tm = THvLogList->model.item(i, 3)->text();//time end
            QDateTime tdm = QDateTime::fromString(dt+tm,"yyyyMMddhh:mm");
            unsigned int idtm = tdm.toTime_t();
            if (dte_end>=idtm && dte_start<=idtm)
                rows.append(i);
        }
    }
    //qDebug()<<rows.count();

    QDateTime utc_t = QDateTime::currentDateTimeUtc();
    QString file_Path_Name;
    QString dir_path_name = QString(AppPath+"/ExportLog/");
    QString call = s_my_call;
    if (call.contains("/"))
    {
        if (call.at(call.count()-1)=='/') call.replace("/","_SLASH");
        else call.replace("/","_SLASH_");
    }
    QString c_name =s_cont_name[id_cont];
    c_name.replace(" ","_");
    c_name.replace(".","");//2.74 for Inter.
    QString file_name = QString("/"+call+"_"+c_name+"_"+utc_t.toString("yyyyMMdd")+"_Cabrillo");//+ident
    //qDebug()<<dir_path_name+file_name;

    //QFileDialog fileDialog(this, tr("Save Cabrillo Log File"),
    //dir_path_name+file_name, tr("Log File (*.log)"));//*.cbr, *.log
    //fileDialog.setDefaultSuffix("log");//cbr, log
    QFileDialog fileDialog(this, "Save Cabrillo Log File",
                           dir_path_name+file_name, "Cabrillo Log (*.cbr)");//*.cbr, *.log
    fileDialog.setDefaultSuffix("cbr");

    fileDialog.setAcceptMode(QFileDialog::AcceptSave);
    QStringList list;
    if (fileDialog.exec()) list =  fileDialog.selectedFiles();

    if (list.isEmpty()) return;
    else file_Path_Name = *list.begin();

    if (file_Path_Name.isEmpty()) return;

    QFile file(file_Path_Name);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return;

    THvProgrD->Show(tr("Export QSOs"),tr("Please Wait, Export QSOs")+" ",510);
    QTextStream out(&file);

    ///////////// cabrillo head ///////////////////
    out << "START-OF-LOG: 3.0" << "\n";					                   // START-OF-LOG: 3.0
    out << "LOCATION: "+le_cabrillo_locat->text() << "\n";				   // LOCATION: DX
    out << "CALLSIGN: "+s_my_call << "\n";			                       // CALLSIGN: LZ2HV
    out << "CLUB: "+le_cabrillo_club->text() << "\n";					   // CLUB: None le_cabrillo_club
    //out << "CONTEST: "+s_cont_name_id[id_cont] << "\n"; 			       // CONTEST: ARRL-FIELD-DAY
    out << "CONTEST: "+le_cabrillo_cont_id->text() << "\n";				   // CONTEST: ARRL-FIELD-DAY
    out << "CATEGORY-OPERATOR: "+cb_cabrillo_cop->currentText() << "\n";   // CATEGORY-OPERATOR: MULTI-OP
    out << "CATEGORY-ASSISTED: "+cb_cabrillo_as->currentText() << "\n";    // CATEGORY-ASSISTED: ASSISTED
    out << "CATEGORY-BAND: "+cb_cabrillo_band->currentText() << "\n";      // CATEGORY-BAND: ALL
    out << "CATEGORY-MODE: "+cb_cabrillo_mod->currentText() << "\n";	   // CATEGORY-MODE: SSB,CW,RTTY,FM,MIXED
    out << "CATEGORY-POWER: "+cb_cabrillo_pwr->currentText() << "\n";      // CATEGORY-POWER: HIGH
    out << "CATEGORY-STATION: "+cb_cabrillo_cst->currentText() << "\n";    // CATEGORY-STATION: FIXED
    if (cb_cabrillo_tim->currentIndex()>0)
        out << "CATEGORY-TIME: "+cb_cabrillo_tim->currentText() << "\n";    //CATEGORY-TIME:
    out << "CATEGORY-TRANSMITTER: "+cb_cabrillo_trm->currentText() << "\n"; // CATEGORY-TRANSMITTER: UNLIMITED
    if (cb_cabrillo_ovr->currentIndex()>0)
        out << "CATEGORY-OVERLAY: "+cb_cabrillo_ovr->currentText() << "\n"; //CATEGORY-OVERLAY:
    // CLAIMED-SCORE: 0 ???
    out << "OPERATORS: "+le_cabrillo_oper->text() << "\n";				    // OPERATORS: LZ2HV
    out << "NAME: "+le_cabrillo_name->text() << "\n";					    // NAME:
    out << "EMAIL: "+le_cabrillo_email->text() << "\n";					    // EMAIL:
    out << "ADDRESS: "+le_cabrillo_addr->text() << "\n";					// ADDRESS:   1
    out << "ADDRESS: "+le_cabrillo_addr2->text() << "\n";					// ADDRESS:   2
    out << "ADDRESS-CITY: "+le_cabrillo_addr_city->text() << "\n";			// ADDRESS-CITY: Tur
    out << "ADDRESS-STATE-PROVINCE: "+le_cabrillo_addr_spro->text() << "\n";// ADDRESS-STATE-PROVINCE: TURBB
    out << "ADDRESS-POSTALCODE: "+le_cabrillo_addr_posc->text() << "\n";	// ADDRESS-POSTALCODE: 500
    out << "ADDRESS-COUNTRY: "+le_cabrillo_addr_cntr->text() << "\n";		// ADDRESS-COUNTRY: BULGARIA
    out << "CREATED-BY: "+(QString)APP_NAME << "\n";
    /////////// end cabrillo head /////////////////

    int j = 0;
    for (j = 0; j < rows.count(); j++)
    {
        out << "QSO: ";
        QString s;
        s = THvLogList->model.item(rows[j], 10)->text();//3 freq
        if (!s.isEmpty())
        {
            s.remove(".");
            if (s.at(0) == '0') s = s.mid(1,s.count()-1);//2.31
        }
        if (s.isEmpty())
        {
            s = THvLogList->model.item(rows[j], 9)->text();  //FREQ for the moment form band 9 -> 10 reserved for freq
            if (s.contains("GHz"))
            {
                s.remove(" GHz");
                double d = s.toDouble()*1000.0;
                s = QString("%1").arg(d)+"000";
            }
            else if (s.contains("MHz"))
            {
                s.remove(" MHz");
                s = QString("%1").arg((int)(s.toDouble()*1000.0));
            }
            else if (s.contains("kHz")) s.remove(" kHz");
        }
        //s = GetFREQall(2,rows[j]);
        //qDebug()<<"Cabr="<<s;
        //output= 136 1800 to kHz
        double d00 = s.toDouble();
        if (d00>=1000000.0)//10000000 G
        {
            double d01=d00/1000000.0;
            if (d00<10000000.0)
            {
                s = QString("%1").arg(d01);
                s = s.mid(0,3)+"G"; //s = QString("%1").arg(d01,0,'f',1);
            }
            else s = QString("%1").arg((int)d01)+"G";
        }
        else if (d00>=50000.0)
        {
            d00=d00/1000.0;
            s = QString("%1").arg((int)d00);
        }
        //qDebug()<<"CabrADD="<<s;
        out << s.rightJustified(5,' ')+" ";
        out << GetCabriloMoStr(THvLogList->model.item(rows[j], 8)->text())+" ";//2.24 old=out << "DG "; <-mode
        s = THvLogList->model.item(rows[j], 2)->text();            //qso_date off
        s.insert(4,"-");
        s.insert(7,"-");
        out << s+" ";
        s = THvLogList->model.item(rows[j], 3)->text();            //time_off
        s.remove(":");
        out << s+" ";

        QString trmN = "0";
        if (cb_cabrillo_trmN->isEnabled())
        {
            trmN = cb_cabrillo_trmN->currentText();
            QString trmN_log = THvLogList->model.item(rows[j], 19)->text();//19 contest MULTI-TWO hiden
            if (trmN=="From Log")//0=none, 1=0, 2=1
            {
                if (trmN_log=="2")
                    trmN = "1";
                else
                    trmN = "0";
            }
            else
            {
                if (trmN=="Run 1")
                    trmN = "0";
                else if (trmN=="Run 2")
                    trmN = "1";
            }
        }

        if (id_cont==1)//MY OLD MODES VHF Contest  ???
        {
            out << s_my_call.leftJustified(13,' ')+" ";
            QString st = THvLogList->model.item(rows[j], 6)->text();//6 my tx rpt
            out << st.leftJustified(3,' ')+" ";
            out << s_my_grid.leftJustified(6,' ')+" ";//my grid

            QString stf = THvLogList->model.item(rows[j], 14)->text();//14 my tx sn contest
            int istf = stf.toInt();
            st = QString("%1").arg(istf,3,10,QChar('0'));
            out << st.leftJustified(3,' ')+" ";

            st = THvLogList->model.item(rows[j], 4)->text();//4 his call
            out << st.leftJustified(13,' ')+" ";
            st = THvLogList->model.item(rows[j], 7)->text();//7 my rx rpt
            out << st.leftJustified(3,' ')+" ";
            st = THvLogList->model.item(rows[j], 5)->text();//5 his grid
            if (st.isEmpty()) st = "ZZ00ZZ";//6
            out << st.leftJustified(6,' ')+" ";

            stf = THvLogList->model.item(rows[j], 15)->text();//15 my rx sn contest
            istf = stf.toInt();
            st= QString("%1").arg(istf,3,10,QChar('0'));
            out << st.leftJustified(3,' ');//last no need to be leftJustified
            //end 87 char
        }
        else if (id_cont==2 || id_cont==13 || id_cont==14)//NA VHF Contest and CQ WW VHF Contest
        {
            out << s_my_call.leftJustified(13,' ')+" ";
            QString st = s_my_grid.mid(0,4);//my grid 4char
            out << st.leftJustified(10,' ')+" ";

            st = THvLogList->model.item(rows[j], 4)->text();//4 his call
            out << st.leftJustified(13,' ')+" ";
            st = THvLogList->model.item(rows[j], 5)->text();//5 his grid
            if (st.isEmpty()) st = "ZZ00";
            st = st.mid(0,4);//his grid 4char
            out << st.leftJustified(10,' ');//last no need to be leftJustified
            //out << "  "; //+2 to 81 char
            out << " "+trmN;
        }
        else if (id_cont==3)//EU VHF Contest ???
        {
            out << s_my_call.leftJustified(13,' ')+" ";
            QString st = THvLogList->model.item(rows[j], 6)->text();//6 my tx rpt
            QString stf = THvLogList->model.item(rows[j], 14)->text();//14 my tx sn contest
            int istf = stf.toInt();
            st.append(QString("%1").arg(istf,4,10,QChar('0')));
            out << st.leftJustified(6,' ')+" ";
            out << s_my_grid.leftJustified(6,' ')+" ";//my grid

            st = THvLogList->model.item(rows[j], 4)->text();//4 his call
            out << st.leftJustified(13,' ')+" ";
            st = THvLogList->model.item(rows[j], 7)->text();//7 my rx rpt
            stf = THvLogList->model.item(rows[j], 15)->text();//15 my rx sn contest
            istf = stf.toInt();
            st.append(QString("%1").arg(istf,4,10,QChar('0')));
            out << st.leftJustified(6,' ')+" ";
            st = THvLogList->model.item(rows[j], 5)->text();//5 his grid
            if (st.isEmpty()) st = "ZZ00ZZ";//6
            out << st.leftJustified(6,' ');//last no need to be leftJustified
            //end 85 char
        }
        else if (id_cont==4)//ARRL Field Day
        {
            out << s_my_call.leftJustified(13,' ')+" ";
            QString st = THvLogList->model.item(rows[j], 16)->text();//16 my tx exch
            QStringList l_st = st.split(" ");
            l_st.append("");
            l_st.append("");
            out << l_st.at(0).leftJustified(3,' ')+" ";
            out << l_st.at(1).leftJustified(6,' ')+" ";

            st = THvLogList->model.item(rows[j], 4)->text();//4 his call
            out << st.leftJustified(13,' ')+" ";
            st = THvLogList->model.item(rows[j], 17)->text();//17 my rx  exch
            l_st = st.split(" ");
            l_st.append("");
            l_st.append("");
            out << l_st.at(0).leftJustified(3,' ')+" ";
            out << l_st.at(1).leftJustified(6,' ');//last no need to be leftJustified
            //out << "  "; //+2 to 81 char
            out << " "+trmN;
        }
        else if (id_cont == 9 || id_cont == 10 || id_cont == 11 || id_cont == 12)//ARRL RTTY Contest and ARRL RTTY Roundup
        {
            out << s_my_call.leftJustified(13,' ')+" ";
            QString st = THvLogList->model.item(rows[j], 6)->text();//my txrpt
            out << st.leftJustified(3,' ')+" ";

            if (f_iem_ru_dx)
            {
                QString stf = THvLogList->model.item(rows[j], 14)->text();//14 my tx sn contest "my txsn for dx"
                int istf = stf.toInt();
                if (istf>0) st = QString("%1").arg(istf,4,10,QChar('0'));
                else st = ""; //2.65 exception ?
            }
            else
            {
                st = THvLogList->model.item(rows[j], 16)->text();//16 my tx exch
                //if (st.isEmpty()) st = "ZZZ";  //2.65 exception ?
            }
            out << st.leftJustified(6,' ')+" ";//"my tx exch" or "my txsn for dx"

            ///////////////////// his ///////////////////
            st = THvLogList->model.item(rows[j], 4)->text();//4 his call
            out << st.leftJustified(13,' ')+" ";
            st = THvLogList->model.item(rows[j], 7)->text();//my rxrpt
            out << st.leftJustified(3,' ')+" ";

            QString stf1 = THvLogList->model.item(rows[j], 15)->text();//15 my rx sn contest "my rxsn for dx"
            QString stf2 = THvLogList->model.item(rows[j], 17)->text();//17 my rx exch
            if (!stf1.isEmpty())// && stf2.isEmpty() exception sn + exch ->stf2.isEmpty()
            {
                int istf = stf1.toInt();
                if (istf>0) st = QString("%1").arg(istf,4,10,QChar('0'));
                else st = ""; //2.65 exception ?
            }
            else
            {
                if (stf2.isEmpty()) st = "0000"; //2.65 exception not in a contest
                else st = stf2;
            }
            out << st.leftJustified(6,' ');//last no need to be leftJustified  "my rx exch" or "my rxsn for dx"
            //out << "  "; //+2 to 81 char
            out << " "+trmN;
        }
        else if (id_cont==5 || id_cont==6 || id_cont == 7 || id_cont == 8 || id_cont==15)//WW Digi DX Contest  "ARRL Inter. Digital Contest"  FT4/8 DX
        {
            out << s_my_call.leftJustified(13,' ')+" ";
            QString st = s_my_grid.mid(0,4);//my grid 4char
            out << st.leftJustified(8,' ')+" ";

            st = THvLogList->model.item(rows[j], 4)->text();//4 his call
            out << st.leftJustified(13,' ')+" ";
            QString stg = THvLogList->model.item(rows[j], 5)->text();//5 his grid
            if (stg.isEmpty()) stg = "ZZ00";
            st = stg.mid(0,4);
            out << st.leftJustified(8,' ');//last no need to be leftJustified
            out << "     "+trmN;//+4 to 81 char
        }
        else if (id_cont==16)//"ARRL Inter. EME Contest"
        {
            out << s_my_call.leftJustified(13,' ')+" ";
            QString st = THvLogList->model.item(rows[j], 6)->text();//my txrpt
            if (st.isEmpty()) st = "-15";
            out << st.leftJustified(3,' ')+" ";
            //QString st = s_my_grid.mid(0,4);//my grid 4char
            //out << st.leftJustified(8,' ')+" ";

            st = THvLogList->model.item(rows[j], 4)->text();//4 his call
            out << st.leftJustified(13,' ')+" ";
            st = THvLogList->model.item(rows[j], 7)->text();//my rxrpt
            if (st.isEmpty()) st = "-15";
            out << st.leftJustified(3,' ')+" ";
            //QString stg = THvLogList->model.item(rows[j], 5)->text();//5 his grid
            //if (stg.isEmpty()) stg = "ZZ00";
            //st = stg.mid(0,4);
            //out << st.leftJustified(8,' ');//last no need to be leftJustified
            out << "     "+trmN;//+4 to 81 char
        }
        else if (id_cont==17)//"FT Challenge"
        {
            out << s_my_call.leftJustified(13,' ')+" ";
            QString st = THvLogList->model.item(rows[j], 6)->text();//my txrpt
            if (st.isEmpty()) st = "+99";
            out << st.leftJustified(3,' ')+" ";
            st = s_my_grid.mid(0,4);//my grid 4char
            out << st.leftJustified(4,' ')+" ";

            st = THvLogList->model.item(rows[j], 4)->text();//4 his call
            out << st.leftJustified(13,' ')+" ";
            st = THvLogList->model.item(rows[j], 7)->text();//my rxrpt
            if (st.isEmpty()) st = "+99";
            out << st.leftJustified(3,' ')+" ";
            QString stg = THvLogList->model.item(rows[j], 5)->text();//5 his grid
            if (stg.isEmpty()) stg = "ZZ00";
            st = stg.mid(0,4);
            out << st.leftJustified(4,' ')+" ";//last no need to be leftJustified
            out << "   "+trmN;//+4 to 81 char
        }        

        out <<"\n";
        THvProgrD->SetValue(j);
    }
    out << "END-OF-LOG:";
    THvProgrD->Finish(j);
    file.close();
    THvProgrD->close();

    QMessageBox::information(this, "MSHV", tr("Successfully Exported")+" "+QString("%1").arg(j)+" "+tr("QSOs In Cabrillo Format.")+"\n"+
                             tr("Full Path And File Name is")+":\n"+file_Path_Name, QMessageBox::Ok);
}
