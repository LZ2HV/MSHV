/* MSHV HvMsProc
 * Copyright 2015 Hrisimir Hristov, LZ2HV
 * May be used under the terms of the GNU General Public License (GPL)
 */

#include "hvmsproc.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QTextBrowser>
#include <QIcon>
//#include <QPixmap>
//#include <QApplication>
//#include <QDesktopWidget>

HvMsProc::HvMsProc(QString title,QString app_name,QString path,int lid,int x,int y,QWidget * parent)
        : QWidget(parent)
{
	setMinimumSize(400,300);  //https://www.youtube.com/watch?v=4QNxcHJxHsA
	//setFixedSize(555,460);
	
    //setWindowTitle("About "+app_name);
    //setWindowFlags(windowFlags() ^ Qt::WindowContextHelpButtonHint);
    setWindowTitle(title+" "+app_name);
    setWindowFlags(windowFlags() &~ Qt::WindowMaximizeButtonHint &~ Qt::WindowContextHelpButtonHint);
    setWindowIcon(QPixmap(":pic/ms_ico.png"));
	
    QVBoxLayout *layout_v = new QVBoxLayout(this);
    layout_v->setContentsMargins ( 4, 4, 4, 4);
    //QString app_name = (QString)APP_NAME;

    QLabel *lab_title;
    lab_title = new QLabel();
    lab_title->setTextInteractionFlags(Qt::LinksAccessibleByMouse);
    lab_title->setOpenExternalLinks(true);
    //<program>  Copyright (C) <year>  <name of author>
    
    //QString c = "©";0xA9
    lab_title->setText(app_name);
    lab_title->setAlignment(Qt::AlignHCenter);
    layout_v->addWidget(lab_title);

    QTextBrowser *text_browser = new  QTextBrowser();
    //text_browser->setContentsMargins(5,5,5,5);
    text_browser->setReadOnly(true);     
    text_browser->setOpenExternalLinks(true);// otvaria linkove ako ima
    
    //QPixmap gpl3_icon = QPixmap(":pic/gplv3-88x31.png");
    
    layout_v->addWidget(text_browser);
        
    //QString path = (QCoreApplication::applicationDirPath());
	//if 		(lid==0 ) text_browser->setSource(QUrl::fromLocalFile(path + "/settings/resources/url_proc/hvmsproc_en.html"));
	if		(lid==1 ) text_browser->setSource(QUrl::fromLocalFile(path + "/settings/resources/url_proc/hvmsproc_en.html"));
	else if (lid==2 ) text_browser->setSource(QUrl::fromLocalFile(path + "/settings/resources/url_proc/hvmsproc_ru.html"));
	/*else if (lid==3 ) text_browser->setSource(QUrl::fromLocalFile(path + "/settings/resources/url_proc/hvmsproc_en.html"));//ZH 
	else if (lid==4 ) text_browser->setSource(QUrl::fromLocalFile(path + "/settings/resources/url_proc/hvmsproc_en.html"));//ZHHK 
	else if (lid==5 ) text_browser->setSource(QUrl::fromLocalFile(path + "/settings/resources/url_proc/hvmsproc_en.html"));//ESES 
	else if (lid==6 ) text_browser->setSource(QUrl::fromLocalFile(path + "/settings/resources/url_proc/hvmsproc_en.html"));//CAES
	else if (lid==7 ) text_browser->setSource(QUrl::fromLocalFile(path + "/settings/resources/url_proc/hvmsproc_en.html"));//PTPT
	else if (lid==8 ) text_browser->setSource(QUrl::fromLocalFile(path + "/settings/resources/url_proc/hvmsproc_en.html"));//RORO
	else if (lid==9 ) text_browser->setSource(QUrl::fromLocalFile(path + "/settings/resources/url_proc/hvmsproc_en.html"));//DADK
	else if (lid==10) text_browser->setSource(QUrl::fromLocalFile(path + "/settings/resources/url_proc/hvmsproc_en.html"));//PLPL
	else if (lid==11) text_browser->setSource(QUrl::fromLocalFile(path + "/settings/resources/url_proc/hvmsproc_en.html"));//FRFR
	else if (lid==12) text_browser->setSource(QUrl::fromLocalFile(path + "/settings/resources/url_proc/hvmsproc_en.html"));//PTBR	
	else if (lid==13) text_browser->setSource(QUrl::fromLocalFile(path + "/settings/resources/url_proc/hvmsproc_en.html"));//NBNO
	else if (lid==14) text_browser->setSource(QUrl::fromLocalFile(path + "/settings/resources/url_proc/hvmsproc_en.html"));//ITIT
	*/
	else			  text_browser->setSource(QUrl::fromLocalFile(path + "/settings/resources/url_proc/hvmsproc_en.html"));//any other

	this->setLayout(layout_v);
	
	this->resize(740,450);
	
    move(x,y);
}
HvMsProc::~HvMsProc()
{}

