/* MSHV HelpMs
 * Copyright 2015 Hrisimir Hristov, LZ2HV
 * May be used under the terms of the GNU General Public License (GPL)
 */
#include "hvhelpms.h"
#include <QIcon>
#include <QTextBrowser>

HvHelpMs::HvHelpMs(QString title,QString app_name,QString path,int lid,int x,int y,QWidget *parent) 
	: QWidget(parent)
{  
	//setFixedSize(700,500);//1.30 800x513pix
	//this->setFixedWidth(750);
	//setMinimumWidth(750);
	setMinimumSize(790,525);
	//this->setMinimumHeight(450);//1.30 800x513pix
	//setWindowFlags(windowFlags() ^ Qt::WindowContextHelpButtonHint);// otva e da go ima
	setWindowFlags(windowFlags() &~ Qt::WindowMaximizeButtonHint &~ Qt::WindowContextHelpButtonHint);
	
    //setWindowIcon(QPixmap(":pic_main/icon_sdr.png"));
    setWindowIcon(QPixmap(":pic/ms_ico.png"));
    setWindowTitle(title+" "+app_name); 
    QTextBrowser *text_browser = new  QTextBrowser();
    text_browser->setReadOnly(true);
   
    //QUrl
    //text_browser->setFixedSize(this->width(), this->height());
    
    QLabel *lab_title;
    lab_title = new QLabel();
    lab_title->setTextInteractionFlags(Qt::LinksAccessibleByMouse);
    lab_title->setOpenExternalLinks(true);
    lab_title->setText(app_name+"<br><a href=\"mailto:lz2hv@abv.bg\">lz2hv@abv.bg</a>"+" & "+"<a href=\"http://lz2hv.org/mshv\">MSHV Web Site</a>");
	lab_title->setAlignment(Qt::AlignHCenter); 
	
	/*QPixmap pixmap1(":pic/en_press.png");
    QPixmap pixmap2(":pic/en_release.png");
    Bt_En = new HvButton_Left2(this);
    Bt_En->SetupButton_hv(pixmap2, pixmap1, 0, 0);
    connect(Bt_En, SIGNAL(Release_Lift_Button_hv()), this, SLOT(SetText_en()));
    QPixmap pixmap3(":pic/bg_press.png");
    QPixmap pixmap4(":pic/bg_release.png");
    Bt_Bg = new HvButton_Left2(this);
    Bt_Bg->SetupButton_hv(pixmap4, pixmap3, 0, 0);
    connect(Bt_Bg, SIGNAL(Release_Lift_Button_hv()), this, SLOT(SetText_bg()));*/     
    QHBoxLayout *layout_h = new QHBoxLayout();
    layout_h->setContentsMargins ( 0, 0, 0, 0);  
    layout_h->setAlignment(Qt::AlignCenter);
    layout_h->setSpacing(40);
	//layout_h->addWidget(Bt_En); 
	layout_h->addWidget(lab_title);
	//layout_h->addWidget(Bt_Bg);	 
    
    QVBoxLayout *layout_v = new QVBoxLayout();
    layout_v->setContentsMargins ( 4, 4, 4, 4);  
	this->setLayout(layout_v);
	layout_v->addLayout(layout_h);
	layout_v->addWidget(text_browser);

	//setStyleSheet("QTextBrowser { background: rgb(60, 60, 60); }");

	//url_hv = new QUrl();
		
    //if 		(lid==0 ) text_browser->setSource(QUrl::fromLocalFile(path + "/settings/resources/url_help/en/help_en.html"));
    if 		(lid==1 ) text_browser->setSource(QUrl::fromLocalFile(path + "/settings/resources/url_help/en/help_en.html"));
    else if (lid==2 ) text_browser->setSource(QUrl::fromLocalFile(path + "/settings/resources/url_help/ru/help_ru.html"));
    /*else if (lid==3 ) text_browser->setSource(QUrl::fromLocalFile(path + "/settings/resources/url_help/en/help_en.html"));//ZH 
    else if (lid==4 ) text_browser->setSource(QUrl::fromLocalFile(path + "/settings/resources/url_help/en/help_en.html"));//ZHHK 
    else if (lid==5 ) text_browser->setSource(QUrl::fromLocalFile(path + "/settings/resources/url_help/en/help_en.html"));//ESES 
    else if (lid==6 ) text_browser->setSource(QUrl::fromLocalFile(path + "/settings/resources/url_help/en/help_en.html"));//CAES
    else if (lid==7 ) text_browser->setSource(QUrl::fromLocalFile(path + "/settings/resources/url_help/en/help_en.html"));//PTPT
    else if (lid==8 ) text_browser->setSource(QUrl::fromLocalFile(path + "/settings/resources/url_help/en/help_en.html"));//RORO
    else if (lid==9 ) text_browser->setSource(QUrl::fromLocalFile(path + "/settings/resources/url_help/en/help_en.html"));//DADK
    else if (lid==10) text_browser->setSource(QUrl::fromLocalFile(path + "/settings/resources/url_help/en/help_en.html"));//PLPL
    else if (lid==11) text_browser->setSource(QUrl::fromLocalFile(path + "/settings/resources/url_help/en/help_en.html"));//FRFR
    else if (lid==12) text_browser->setSource(QUrl::fromLocalFile(path + "/settings/resources/url_help/en/help_en.html"));//PTBR
    else if (lid==13) text_browser->setSource(QUrl::fromLocalFile(path + "/settings/resources/url_help/en/help_en.html"));//NBNO
    else if (lid==14) text_browser->setSource(QUrl::fromLocalFile(path + "/settings/resources/url_help/en/help_en.html"));//ITIT
    */
    else 			  text_browser->setSource(QUrl::fromLocalFile(path + "/settings/resources/url_help/en/help_en.html"));//any other
        
    move(x,y);
}
HvHelpMs::~HvHelpMs()
{}
void HvHelpMs::SetPosXY(QString s)
{
	QStringList list_s = s.split("#");
	if(list_s.count()==2)
	{
		move(list_s[0].toInt(),list_s[1].toInt());
	}	
} 
/*
void HvHelpMs::SetText_en()
{
	QString path;// = (QCoreApplication::applicationDirPath());
	text_browser->setSource(QUrl::fromLocalFile(path + "/Resources/url_help/help_en.html"));
} 
*/
/*
void HvHelpMs::SetText_bg()
{
	text_browser->setSource(QUrl::fromLocalFile(":url/help_bg.html"));
}
*/