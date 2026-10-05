/* MSHV HelpSkMs
 * Copyright 2015 Hrisimir Hristov, LZ2HV
 * May be used under the terms of the GNU General Public License (GPL)
 */
#include "hvhelpskms.h"

#include <QVBoxLayout>
#include <QLabel>
//#include <QDesktopWidget>
//#include <QApplication>
#include <QIcon>
//#include <QByteArray>

#undef _MOUNTH_H_
#define _SHKY_H_
#include "../config_str_sk.h"

//#include <QTextBrowser>
//#include <QTextTable>

//#include <QtGui>

HvHelpSkMs::HvHelpSkMs(QString title,QString app_name,int lid,int x,int y,QWidget *parent )
        : QWidget(parent)
{
    //setMinimumSize(190,280);
    setWindowTitle(title+" "+app_name);
    //setWindowFlags(windowFlags() ^ Qt::WindowContextHelpButtonHint);
    setWindowFlags(windowFlags() &~ Qt::WindowMaximizeButtonHint &~ Qt::WindowContextHelpButtonHint);
    setWindowIcon(QPixmap(":pic/ms_ico.png"));
    //setFixedSize(412,432);// setFixedSize(410,420); 

    move(x,y);

    /*QHBoxLayout *lv_h1 = new QHBoxLayout();
    QLabel *l_helpw1 = new QLabel;
    QLabel *l_helpw2 = new QLabel;
    QString s1;
    QString s2;
    for (int i = 0; i<COUNT_SK; ++i)
    {
       s1.append(ShKey[lid][i][0]+"\n");
       s2.append(ShKey[lid][i][1]+"\n");
    }
    l_helpw1->setText(s1);
    l_helpw2->setText(s2);
    QVBoxLayout *la_helpw1 = new QVBoxLayout();
    la_helpw1->setContentsMargins ( 1, 1, 1, 1);
    la_helpw1->addWidget(l_helpw1);
    la_helpw1->setAlignment(l_helpw1,Qt::AlignLeft);
    QVBoxLayout *la_helpw2 = new QVBoxLayout();
    la_helpw2->setContentsMargins ( 1, 1, 1, 1);
    la_helpw2->addWidget(l_helpw2);
    la_helpw2->setAlignment(l_helpw2,Qt::AlignLeft);
    lv_h1->addLayout(la_helpw1);
    lv_h1->addLayout(la_helpw2);*/

    QVBoxLayout *lv_h1 = new QVBoxLayout();
    lv_h1->setContentsMargins(10,10,10,10);
    lv_h1->setSpacing(1);
       
    for (int i = 0; i<COUNT_SK; ++i)
    {
        QLabel *l_t1 = new QLabel;
        QLabel *l_t2 = new QLabel;
        QHBoxLayout *lh_h1 = new QHBoxLayout();
        lh_h1->setContentsMargins(0,0,0,0);
        lh_h1->setSpacing(1);
        l_t1->setText(ShKey[lid][i][0]);
        l_t1->setFixedWidth(100);
        l_t2->setText(ShKey[lid][i][1]);
        lh_h1->addWidget(l_t1);
        //lh_h1->setAlignment(l_t1,Qt::AlignLeft);
        lh_h1->addWidget(l_t2);
        //lh_h1->setAlignment(l_t2,Qt::AlignCenter);
        //lh_h1->setAlignment(Qt::AlignHCenter);
        lv_h1->addLayout(lh_h1);
    }
     
    /*QTextBrowser *TBStatistic = new  QTextBrowser();
    TBStatistic->setReadOnly(true);     	 
    lv_h1->addWidget(TBStatistic);	
	QTextCursor cursor(TBStatistic->textCursor());
	cursor.movePosition(QTextCursor::Start);	
	QTextTableFormat tableFormat;
    tableFormat.setBorder(0);
    tableFormat.setCellPadding(0); 
    tableFormat.setCellSpacing(0);
    tableFormat.setWidth(QTextLength(QTextLength::PercentageLength,100));
	QTextTable *table = cursor.insertTable(COUNT_SK,2,tableFormat);	
    for (int i = 0; i<COUNT_SK; ++i) 
    {
    	table->cellAt(i,0).firstCursorPosition().insertText(ShKey[lid][i][0]);
    	table->cellAt(i,1).firstCursorPosition().insertText(ShKey[lid][i][1]);
   	}*/   	     
    
    //lv_h1->setAlignment(Qt::AlignTop);
    setLayout(lv_h1);
}
HvHelpSkMs::~HvHelpSkMs()
{}
