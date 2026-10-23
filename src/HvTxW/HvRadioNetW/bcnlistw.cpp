/* MSHV BcnList
 * Copyright 2015 Hrisimir Hristov, LZ2HV
 * May be used under the terms of the GNU General Public License (GPL)
 */
#include "bcnlistw.h"
#include <QHeaderView>
//#include <QtGui>

/*HvHeaderBcnList::HvHeaderBcnList( Qt::Orientation orientation, QWidget *parent )
        : QHeaderView(orientation, parent)
{

    //setResizeMode(QHeaderView::Interactive);
}

HvHeaderBcnList::~HvHeaderBcnList()
{}*/
HvBcnList::HvBcnList(bool f,QWidget *parent)
        : QTreeView(parent)
{
    dsty = f;
    setMinimumSize(100, 150);
    //this->set

    setRootIsDecorated(false);
    setModel(&model);
    QHeaderView *THvHeader = new QHeaderView(Qt::Horizontal);

    //connect(THvHeader, SIGNAL(sectionPressed(int)),this,SIGNAL(SortClicked(int)));
    //THvHeader->setre

    setHeader(THvHeader);
    setAllColumnsShowFocus(true);  // za da o4ertae delia row

    //setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    //setSelectionMode(QAbstractItemView::ExtendedSelection);
    //setSelectionMode(QAbstractItemView::ExtendedSelection);//QAbstractItemView::SingleSelection
    //setSelectionMode(QAbstractItemView::SingleSelection);

    setDragDropMode(QAbstractItemView::NoDragDrop);//QAbstractItemView::InternalMove
    setDragEnabled (false);
    // setAcceptDrops(true);
    setVerticalScrollMode(QAbstractItemView::ScrollPerItem);// tova e za na mi6kata whella i dragdrop QAbstractItemView::ScrollPerPixel
    setAutoScroll(false); // s tova tarsi samo v 1 colona i za scroll pri drag
    /*m_file = new QMenu(this);
    QAction *file_info = new QAction("File Info", this);
    m_file->addAction(file_info);
    connect(file_info, SIGNAL(triggered()), this, SLOT(File_Info()));
    m_file->addSeparator();
    QAction *clr_sel = new QAction("Clear Selected", this);
    m_file->addAction(clr_sel);
    connect(clr_sel, SIGNAL(triggered()), this, SLOT(DellBtSelItems()));
    TFileEditInfoHV = new FileEditInfoHV(this);
    connect(TFileEditInfoHV, SIGNAL(TagChange(QString)), this, SLOT(TagChange_lis(QString)));*/

    QPalette PaletteL = palette();
    PaletteL.setColor(QPalette::Shadow, QColor(120,120,120));// zaradi bordera okantovaneto na lista
    setPalette(PaletteL);

    QStringList list_A;
    list_A <<"N"<<tr("Band")<<tr("Beacon")<<tr("Frequency")<<tr("Locator");
    model.setHorizontalHeaderLabels(list_A);

    THvHeader->setSectionResizeMode(QHeaderView::Fixed);//qt5

    //THvHeader->setResizeMode(2, QHeaderView::Fixed);
    THvHeader->resizeSection(0, 50);//40
    THvHeader->resizeSection(1, 80);//50
    THvHeader->resizeSection(2, 100);//100
    THvHeader->resizeSection(3, 130);//80
    THvHeader->resizeSection(4, 100);//80

    //bavi THvHeader->setSectionResizeMode(0, QHeaderView::Stretch);//1.81 bavi for font
    //bavi THvHeader->setSectionResizeMode(1, QHeaderView::Stretch);//1.81 bavi for font
    THvHeader->setSectionResizeMode(2, QHeaderView::Stretch);//qt5
    //bavi THvHeader->setSectionResizeMode(3, QHeaderView::Stretch);//1.81 bavi for font
    //bavi THvHeader->setSectionResizeMode(4, QHeaderView::Stretch);//1.81 bavi for font

    //THvHeader->hideSection(12);
    setSortingEnabled(true);
    //2.31 false for "\n" jt6m=6 jtms=1
    setUniformRowHeights(true);//2.30 fast wiev
    //setSelectionMode(QAbstractItemView::SingleSelection);
    //setSelectionMode(QAbstractItemView::ExtendedSelection);

    //QFont font("arial", 10);  // times new roman // slaga se tuk zaradi resize coll
    //setFont(font);

    //QFont f_t = font();
    //f_t.setPointSize(10);
    //setFont(f_t);

    //ActiveIndex = -1;
    //ActiveRowText = QColor(255,0,0);
    //ActiveRowBackg = QColor(0,255,0);
    //clipboard = QApplication::clipboard();
    // DrawDropLinePosY = -1;

    //this->setAlternatingRowColors(true);
}
HvBcnList::~HvBcnList()
{}
/*
void HvLogList::SetFocus()
{
	this->show();
}
*/
void HvBcnList::paintEvent(QPaintEvent *event)
{
    QPainter painter(viewport());
    //QModelIndex index = model.index(ActiveIndex, 1, QModelIndex());
    //painter.fillRect(QRect(0,visualRect(index).y(),width(),visualRect(index).height()), ActiveRowBackg);
    QTreeView::paintEvent(event);

    //QTreeView::paintEvent(event);
    //QPainter painter(viewport());

    painter.save();
    QColor cc = QColor(200,200,200);
    if (dsty) cc = QColor(80,80,80);
    painter.setPen(QPen(cc, 1, Qt::DashLine));

    //qDebug()<<model.columnCount();
    for (int i = 1; i<model.columnCount(); ++i)
        painter.drawLine(header()->sectionPosition(i),1,header()->sectionPosition(i),height());

    painter.restore();
}
/*void HvBcnList::drawRow(QPainter *painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const
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
}*/
/*
void HvBcnList::SetEditRow(int numbr)
{
    ActiveIndex = numbr;
    viewport()->update();
}
*/
void HvBcnList::InsertItem_hv(QStringList list)
{
    if (!list.isEmpty())
    {
        /*item = new QStandardItem(QString("%1").arg(model.rowCount() + 1));
        item->setEditable(false);
        item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        qlsi.append(item);*/
        QList<QStandardItem *> qlsi;
        int k = 0;
        for (QStringList::iterator it =  list.begin(); it != list.end(); ++it)
        {
            QStandardItem *item = new QStandardItem(QString(*it));

            if (k==0 || k==1)
                item->setTextAlignment(Qt::AlignCenter);
            item->setEditable(false);

            qlsi.append(item);

            k++;
        }
        model.insertRow(model.rowCount(), qlsi);
        //this->setCurrentIndex(model.index(model.rowCount()-1,0));
        //this->scrollTo(model.index(model.rowCount()-1,0));
    }
    //this->setCurrentIndex(model.index(model.rowCount(), 1, QModelIndex()));
}
HvBcnListW::HvBcnListW(QString path,bool indsty,int x,int y,QWidget * parent )
        : QWidget(parent)
{
    setMinimumSize(500,400);
    //setFixedSize(555,460);

    setWindowTitle(tr("Beacon List"));
    setWindowFlags(windowFlags() &~ Qt::WindowMaximizeButtonHint &~ Qt::WindowContextHelpButtonHint);
    setWindowIcon(QPixmap(":pic/ms_ico.png"));

    THvBcnList = new HvBcnList(indsty);

    QVBoxLayout *layout_v = new QVBoxLayout(this);
    layout_v->setContentsMargins ( 4, 4, 4, 4);
    layout_v->addWidget(THvBcnList);

    this->setLayout(layout_v);

    ReadBcnList(path+"/settings/database/msbcn_db.dbbn");

    move(x,y);

}
HvBcnListW::~HvBcnListW()
{
    //SaveSettings();
}
void HvBcnListW::ReadBcnList(QString path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return;
    }
    int count = 1;
    QTextStream in(&file);
    while (!in.atEnd())
    {
        QString line = in.readLine();
        QStringList list_t;
        list_t = line.split(",");

        if (list_t.count()==4)
        {
            QStringList list_tx;
            //int frq = list_t[2].toiInt();
            list_t[2].insert(list_t[2].count()-3,".");

            list_tx <<QString("%1").arg(count, 3, 'g', -1, QChar('0'))<<list_t[1]<<list_t[0]<<list_t[2]<<list_t[3];
            THvBcnList->InsertItem_hv(list_tx);
            count++;
        }
    }
    file.close();
}
