/* MSHV LabW
 * Copyright 2015 Hrisimir Hristov, LZ2HV
 * May be used under the terms of the GNU General Public License (GPL)
 */
#include "labw.h"

ChangedLab::ChangedLab(QString c_1,QString s_1,QString c_2,QString s_2,QString c_3,QString s_3,int w,int h,QWidget *parent)
        : QLabel(parent)
{
    setFixedSize(w,h);
    c1 = c_1;
    c2 = c_2;
    c3 = c_3;
    s1 = s_1;
    s2 = s_2;
    s3 = s_3;    
    setAlignment(Qt::AlignCenter);
    setFrameStyle(QFrame::Panel | QFrame::Sunken);
    //setFrameStyle(QFrame::Box | QFrame::Raised);
    //setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    //setFrameStyle(QFrame::Box | QFrame::Sunken);
}
ChangedLab::~ChangedLab()
{}
void ChangedLab::SetWidthFromFont(int w)
{
	setFixedWidth(w);
}	
void ChangedLab::SetOnOff(int f)
{
    if (f==0)
    {
        setText(s1);
        setStyleSheet(c1);

    }
    else if (f==1)
    {
        setText(s2);
        setStyleSheet(c2);
    }
    else
    {
        setText(s3);
        setStyleSheet(c3);
    }
}
LabW::LabW(bool indsty,QWidget *parent )
        : QWidget(parent)
{
	//setFixedWidth(142);//2.71 stop
    //TxRxPic = new ChangedLab(QColot,QPixmap(":pic/receive.png"),false);
    //Decode = new ChangedLab(QPixmap(":pic/dec_off.png"),QPixmap(":pic/dec_on.png"),true);
    //this->palette()    border-style: outset;border-width: 2px; border-color: black; font: bold;
   
    QString s1 = "QLabel{background-color :palette(Button);}";
    QString s_dec,s_rec,s_wel;
    if (indsty)
    {
    	s_dec = "QLabel{background-color :rgb(170, 10, 10);}";
    	s_rec = "QLabel{background-color :rgb(64, 150, 0);}";
    	s_wel = "QLabel{background-color :rgb(170, 170, 0);}";     	
   	}
   	else
   	{
    	s_dec = "QLabel{background-color :rgb(250, 0, 0);}";
    	s_rec = "QLabel{background-color :rgb(64, 230, 0);}";
    	s_wel = "QLabel{background-color :rgb(250, 250, 0);}";   		
  	}

    DecodeLab = new ChangedLab(s1,tr("DECODE"),s_dec,tr("DECODE"),s_wel,tr("DECODE"),70,20);//1.30 from 76x20 +125%
    TxRxLab = new ChangedLab(s1,tr("RECEIVE"),s_rec,tr("RECEIVE"),s_wel,tr("RECEIVE"),70,20);//1.30 from 76x20 +125%

    QHBoxLayout *H_l = new QHBoxLayout(this);
    H_l->setContentsMargins (0, 1, 0, 1);//1.30 vazno 0, 1, 0, 1
    H_l->addWidget(DecodeLab);
    H_l->addWidget(TxRxLab);
    H_l->setAlignment(Qt::AlignHCenter);  
}
LabW::~LabW()
{}
void LabW::SetRxMon(int f)
{
    TxRxLab->SetOnOff(f);
}
void LabW::SetDecode(int f)
{
    DecodeLab->SetOnOff(f);
}
void LabW::SetFont(QFont f)
{
	QFontMetrics fm(f);
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)//2.56
    int fwdd = fm.horizontalAdvance(tr("DECODE"))+8;
    int fwdr = fm.horizontalAdvance(tr("RECEIVE"))+8;       
#else
    int fwdd = fm.width(tr("DECODE"))+8;
    int fwdr = fm.width(tr("RECEIVE"))+8;      
#endif    
    setFixedWidth(fwdd+fwdr+8);
 
	DecodeLab->setFont(f);
	DecodeLab->SetWidthFromFont(fwdd);
	TxRxLab->setFont(f);
	TxRxLab->SetWidthFromFont(fwdr);	
}


