/* MSHV Switcher Buttons
 * Copyright 2024 Hrisimir Hristov, LZ2HV
 * May be used under the terms of the GNU General Public License (GPL)
 */
#include "hvmodbtsw.h"
//#include <QtGui>

CbSwID::CbSwID(int id,QString n,QWidget *parent)
        : QCheckBox(parent)
{
    sid = id;
    setText(n);
    connect(this,SIGNAL(clicked(bool)),this,SLOT(sclicked(bool)));
}
CbSwID::~CbSwID()
{}
void CbSwID::sclicked(bool f)
{
    emit cbclicked(sid,f);
}
HvButton_ID::HvButton_ID(int id, QString n, QWidget * parent)
        : QPushButton(parent)
{
    sid = id;
    setText(n);
    connect(this,SIGNAL(clicked(bool)),this,SLOT(sclicked()));
}
HvButton_ID::~HvButton_ID()
{}
void HvButton_ID::sclicked()
{
    emit btclicked(sid);
}
HvDialogBtSw::HvDialogBtSw(int c_bt,QString *str_bt,QString tit,QString gbt,QString buse,QWidget *parent)//int cl2,
        : QDialog(parent)
{
    setWindowTitle(tit);//setWindowTitle(tr("Mode Switcher Buttons"));
    setWindowFlags(windowFlags() ^ Qt::WindowContextHelpButtonHint);
    //setMinimumSize(200,300);
    QVBoxLayout *LV0 = new QVBoxLayout();
    LV0->setContentsMargins(10,2,6,2);
    LV0->setSpacing(0);
    //LV0->setAlignment(Qt::AlignTop);
    QVBoxLayout *LV1 = new QVBoxLayout();
    LV1->setContentsMargins(10,2,6,2);
    LV1->setSpacing(0);
    //LV1->setAlignment(Qt::AlignTop);
    QHBoxLayout *HL = new QHBoxLayout();
    HL->setContentsMargins(0,0,0,0);
    HL->setSpacing(0);
    HL->addLayout(LV0);
    HL->addLayout(LV1);
    //HL->setAlignment(Qt::AlignTop);
    QGroupBox *GB_M = new QGroupBox(gbt);//QGroupBox *GB_M = new QGroupBox(tr("Choose Modes")+":");
    QVBoxLayout *LV_D = new QVBoxLayout(this);
    LV_D->setContentsMargins(10,8,10,8);
    LV_D->setSpacing(8);
    for (int i = 0; i < c_bt; ++i)
    {
        CbSwID *cb = new CbSwID(i,str_bt[i]);
        ListCbID.append(cb);
        if (i<c_bt/2) LV0->addWidget(cb);//if (i<cl2) LV0->addWidget(cb);
        else LV1->addWidget(cb);
        connect(cb,SIGNAL(cbclicked(int,bool)),this,SIGNAL(clicked(int,bool)));
    }
    cb220 = new CbSwID(c_bt,buse);//cb220 = new CbSwID(c_bt,tr("USE MODE SWITCHER"));//"USE MODE SWITCHER BUTTONS"
    connect(cb220,SIGNAL(cbclicked(int,bool)),this,SIGNAL(clicked(int,bool)));
    GB_M->setLayout(HL);
    LV_D->addWidget(GB_M);
    LV_D->addWidget(cb220);
    setLayout(LV_D);
}
HvDialogBtSw::~HvDialogBtSw()
{}
void HvDialogBtSw::SetDsettings(int i,bool f,int c_bt)
{
    if (i<c_bt) ListCbID.at(i)->setChecked(f);
    if (i==c_bt) cb220->setChecked(f);
}
#define MAX_BT 220
HvWBtSw::HvWBtSw(QWidget *dp,int c_bt,int *btid,QString *str_bt,QString tit,QString gbt,QString buse,QString dss,bool dsty0,QWidget *parent)//int cl2,
        : QWidget(parent)
{
	dsty = dsty0;
	btprv = -1;
    sc_bt = c_bt;
    if (sc_bt > MAX_BT) sc_bt = MAX_BT;
    H_bt_ = new QHBoxLayout();
    H_bt_->setContentsMargins(0,0,0,0);
    H_bt_->setSpacing(0);
    for (int i = 0; i < sc_bt; ++i)
    {
        HvButton_ID *bt = new HvButton_ID(btid[i],str_bt[i]);
        bt->setFixedHeight(19);
        bt->setHidden(true);//default
        H_bt_->addWidget(bt);
        connect(bt,SIGNAL(btclicked(int)),this,SIGNAL(clicked(int)));
    }
    setHidden(true);//default
    setLayout(H_bt_);
    D_bt_sw = new HvDialogBtSw(sc_bt,str_bt,tit,gbt,buse,dp);//cl2,
    connect(D_bt_sw,SIGNAL(clicked(int,bool)),this,SLOT(SetHidden(int,bool)));
    SetSettings(dss);//default SetSettings("0#2#13#11#14#15#7#8#222");
}
HvWBtSw::~HvWBtSw()
{}
/*void HvWModBtSw::SetDefault()
{
	SetSettings("0#2#13#11#14#15#7#8#222");//default
}*/
void HvWBtSw::dexec()
{
    D_bt_sw->exec();
}
void HvWBtSw::SetFont(QFont f)
{
    QFontMetrics fm(f);
    for (int i = 0; i < sc_bt; ++i)
    {
        HvButton_ID *bt = (HvButton_ID*)H_bt_->itemAt(i)->widget();
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)//2.56
        int fcor = fm.horizontalAdvance(bt->text())+10;//int fcor = fm.horizontalAdvance(sstr_bt[i])+10;
#else
        int fcor = fm.width(bt->text())+10;//int fcor = fm.width(sstr_bt[i])+10;
#endif
        bt->setMinimumWidth(fcor);
        bt->setFixedHeight(fm.height()+2);
    }
}
void HvWBtSw::SetHidden(int i,bool f)
{
    bool f0 = true;
    if (f) f0 = false;
    if (i<sc_bt)
    {
        if (H_bt_->isEmpty() && !isHidden()) emit clicked(220);
        HvButton_ID *bt = (HvButton_ID*)H_bt_->itemAt(i)->widget();
        bt->setHidden(f0);
    }
    if (i==sc_bt)
    {
        if (!H_bt_->isEmpty() && isHidden()) emit clicked(220);
        setHidden(f0);
    }
}
void HvWBtSw::SetSettings(QString s)
{
    QStringList l = s.split("#");
    bool f = false;
    for (int i = 0; i < sc_bt; ++i)
    {
        f = false;
        HvButton_ID *bt = (HvButton_ID*)H_bt_->itemAt(i)->widget();
        for (int j = 0; j < l.count(); ++j)
        {
            if (bt->sid == l.at(j).toInt())
            {
                bt->setHidden(false);
                D_bt_sw->SetDsettings(i,true,sc_bt);
                f = true;
                break;
            }
        }
        if (!f)
        {
            bt->setHidden(true);
            D_bt_sw->SetDsettings(i,false,sc_bt);
        }
    }
    f = false;
    for (int j = 0; j < l.count(); ++j)
    {
        if (l.at(j) == "220")
        {
            setHidden(false);
            D_bt_sw->SetDsettings(sc_bt,true,sc_bt);
            f = true;
            break;
        }
    }
    if (!f)
    {
        setHidden(true);
        D_bt_sw->SetDsettings(sc_bt,false,sc_bt);
    }
}
QString HvWBtSw::GetSettings()
{
    QString str;
    for (int i = 0; i < sc_bt; ++i)
    {
        HvButton_ID *bt = (HvButton_ID*)H_bt_->itemAt(i)->widget();
        if (!bt->isHidden()) str.append(QString("%1").arg(bt->sid)+"#");
    }
    if (!isHidden()) str.append("220#");
    str.append("222");//id all is off
    return str;
}
void HvWBtSw::SetActiveBt(int i)
{ 
	int fndbt = -2;
	for (int x = 0; x < sc_bt; ++x)
	{
		HvButton_ID *btt = (HvButton_ID*)H_bt_->itemAt(x)->widget();
		if (btt->sid == i)
		{
			fndbt = x;
			break;
		}
	}
	if (fndbt == btprv) return; //qDebug()<<fndbt<<btprv; 
	if (btprv > -1) 
	{
		HvButton_ID *btp = (HvButton_ID*)H_bt_->itemAt(btprv)->widget();
		btp->setStyleSheet("QPushButton{background-color:palette(Button);}");		
	}
	btprv = fndbt;
	if (fndbt < 0) return;
	HvButton_ID *btn = (HvButton_ID*)H_bt_->itemAt(fndbt)->widget();
	if (dsty) btn->setStyleSheet("QPushButton{background-color:rgb(64,130,0);}");//64,150,0
	else btn->setStyleSheet("QPushButton{background-color:rgb(140,240,140);}");//50,240,50
	//if (dsty) btn->setStyleSheet("QPushButton{background-color:rgb(155,10,10);}");
	//else btn->setStyleSheet("QPushButton{background-color:rgb(255,210,210);}");	
}



