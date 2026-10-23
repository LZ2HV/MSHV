/* MSHV
 *
 * By Hrisimir Hristov - LZ2HV
 * May be used under the terms of the GNU General Public License (GPL)
 */

#ifndef BCNLISTW_H
#define BCNLISTW_H

//#include "../config.h"

//#include <QHeaderView>
#include <QTreeView>
//#include <QCoreApplication>
#include <QStandardItemModel>
//#include <QMouseEvent>
#include <QPainter>
//#include <QFont>
//#include <QMessageBox>
//#include <QClipboard>
//#include <QApplication>
//#include <QtGui>

/*class HvHeaderBcnList : public QHeaderView
{
    Q_OBJECT
public:
    HvHeaderBcnList(Qt::Orientation orientation, QWidget *parent = 0);
    virtual ~HvHeaderBcnList();

};*/
class HvBcnList : public QTreeView
{
    Q_OBJECT  //2.65 <- for tr() Q_OBJECT
public:
    HvBcnList(bool,QWidget *parent = 0);
    virtual ~HvBcnList();

	//void SetFocus();
    QStandardItemModel model;
    //void Clear_List();
    //void SetItem_hv(QStringList,int);
    //void SetEditRow(int);

    /*int GetListCount()
    {
        return 	model.rowCount();
    };*/
	void InsertItem_hv(QStringList);

//signals:
	//void SendRightClick();
	//void EmitDoubleClick(QStringList,int);
	//void SortClicked(int);

//public slots:
    //void InsertItem_hv(QStringList);
    //void DeleteSel();
    //void SetItem_hv(QStringList,int);

private:
	bool dsty;
    //HvHeaderBcnList *THvHeader;
    //int ActiveIndex;
    //QColor ActiveRowText;
    //QColor ActiveRowBackg;
    //QClipboard *clipboard;

protected:
	//void mousePressEvent (QMouseEvent*);
	//void drawRow(QPainter *painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const;
	void paintEvent(QPaintEvent *);
	//void mouseDoubleClickEvent(QMouseEvent * event);
	//void keyPressEvent(QKeyEvent * event);

};

//#include <QApplication>
//#include <QDesktopWidget>
#include <QVBoxLayout>
#include <QTextStream>
#include <QFile>
#include "../../config.h"

class HvBcnListW : public QWidget
{
    Q_OBJECT  //2.65 <- for tr() Q_OBJECT
public:
    HvBcnListW(QString path,bool,int,int,QWidget *parent = 0);
    virtual ~HvBcnListW();
    
    HvBcnList *THvBcnList;
    
private:    
    void ReadBcnList(QString);
    
};    
#endif
