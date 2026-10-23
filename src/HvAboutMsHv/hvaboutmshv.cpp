/* MSHV AboutMsHv
 * Copyright 2015 Hrisimir Hristov, LZ2HV
 * May be used under the terms of the GNU General Public License (GPL)
 */
#include "hvaboutmshv.h"
//#include <QApplication> 
#include <QVBoxLayout>
#include <QDialog>
#include <QLabel>
#include <QTextBrowser>
 
HvAboutMsHv::HvAboutMsHv(QString title,QString app_name,QString path,int lid,QWidget * parent)
        : QDialog(parent)
{
	//setMinimumSize(510,330);  
	//setFixedSize(605,600);
	//this->setFixedWidth(700);
	setMinimumSize(710,600);
    setWindowTitle(title+" "+app_name);
    setWindowFlags(windowFlags() ^ Qt::WindowContextHelpButtonHint);
	
    QVBoxLayout *layout_v = new QVBoxLayout(this);
    layout_v->setContentsMargins (4,4,4,4);
    //QString app_name = (QString)APP_NAME;
 
    QLabel *lab_title;
    lab_title = new QLabel();
    lab_title->setTextInteractionFlags(Qt::LinksAccessibleByMouse);
    lab_title->setOpenExternalLinks(true);
    //<program>  Copyright (C) <year>  <name of author>
    
    //QString c = "©";0xA9
    lab_title->setText(app_name+" Copyright "+QChar(0xA9)+" 2015-2025 Hrisimir Hristov - LZ2HV<br><a href=\"mailto:lz2hv@abv.bg\">lz2hv@abv.bg</a>"+" & "+"<a href=\"http://lz2hv.org/mshv\">MSHV Web Site</a>");
    lab_title->setAlignment(Qt::AlignHCenter);
    layout_v->addWidget(lab_title);

    QTextBrowser *text_browser = new  QTextBrowser();
    text_browser->setReadOnly(true);     
    text_browser->setOpenExternalLinks(true);// otvaria linkove ako ima
    
    //QPixmap gpl3_icon = QPixmap(":pic/gplv3-88x31.png");
     
    layout_v->addWidget(text_browser);
        
    //QString path = (QCoreApplication::applicationDirPath());
	//if 		(lid==0 ) text_browser->setSource(QUrl::fromLocalFile(path + "/settings/resources/url_about/about_en.html"));
	if 		(lid==1 ) text_browser->setSource(QUrl::fromLocalFile(path + "/settings/resources/url_about/about_bg.html"));
	else if (lid==2 ) text_browser->setSource(QUrl::fromLocalFile(path + "/settings/resources/url_about/about_ru.html"));
	/*else if (lid==3 ) text_browser->setSource(QUrl::fromLocalFile(path + "/settings/resources/url_about/about_en.html"));//ZH
	else if (lid==4 ) text_browser->setSource(QUrl::fromLocalFile(path + "/settings/resources/url_about/about_en.html"));//ZHHK 
	else if (lid==5 ) text_browser->setSource(QUrl::fromLocalFile(path + "/settings/resources/url_about/about_en.html"));//ESES 
	else if (lid==6 ) text_browser->setSource(QUrl::fromLocalFile(path + "/settings/resources/url_about/about_en.html"));//CAES
	else if (lid==7 ) text_browser->setSource(QUrl::fromLocalFile(path + "/settings/resources/url_about/about_en.html"));//PTPT
	else if (lid==8 ) text_browser->setSource(QUrl::fromLocalFile(path + "/settings/resources/url_about/about_en.html"));//RORO
	else if (lid==9 ) text_browser->setSource(QUrl::fromLocalFile(path + "/settings/resources/url_about/about_en.html"));//DADK
	else if (lid==10) text_browser->setSource(QUrl::fromLocalFile(path + "/settings/resources/url_about/about_en.html"));//PLPL
	else if (lid==11) text_browser->setSource(QUrl::fromLocalFile(path + "/settings/resources/url_about/about_en.html"));//FRFR
	else if (lid==12) text_browser->setSource(QUrl::fromLocalFile(path + "/settings/resources/url_about/about_en.html"));//PTBR
	else if (lid==13) text_browser->setSource(QUrl::fromLocalFile(path + "/settings/resources/url_about/about_en.html"));//NBNO
	else if (lid==14) text_browser->setSource(QUrl::fromLocalFile(path + "/settings/resources/url_about/about_en.html"));//ITIT
	*/
	else 			  text_browser->setSource(QUrl::fromLocalFile(path + "/settings/resources/url_about/about_en.html"));//any other

	this->setLayout(layout_v);
}
HvAboutMsHv::~HvAboutMsHv()
{}










