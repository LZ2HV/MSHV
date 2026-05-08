/* MSHV HvTxtColor
 * Copyright 2015 Hrisimir Hristov, LZ2HV
 * May be used under the terms of the GNU General Public License (GPL)
 */
#include "hvtxtcolor.h"
//#include <QtGui>

HvTxtColor::HvTxtColor(bool f,QWidget *parent )
        : QDialog(parent)
{
    dsty = (int)f;
    //this->setWindowTitle(APP_NAME + (QString)" Rig Control");
    //this->setWindowTitle("Rig Control");

    this->setWindowTitle(tr("Text Highlight"));
    setWindowFlags(windowFlags() ^ Qt::WindowContextHelpButtonHint);
    //QGridLayout *l = new QGridLayout( this, 1, 1, 0, 0, "boardLayout");
    //setStyleSheet("border: 1px solid red");
    //setMinimumSize(340,460);

    cb[0] = new QCheckBox(tr("CQ in message"));
    cb[0]->setChecked(true); // inportet defauld HV 1.26 here and in hvtxw.cpp
    cb[1] = new QCheckBox(tr("My call in message"));
    cb[1]->setChecked(true);// inportet defauld HV 1.26 here and in hvtxw.cpp
    cb[2] = new QCheckBox(tr("His call in message"));//Its
    cb[2]->setChecked(true);// inportet defauld HV 1.26 here and in hvtxw.cpp
    cb[3] = new QCheckBox(tr("His 4-character locator in message"));//Its
    cb[3]->setChecked(true);// inportet defauld HV 1.26 here and in hvtxw.cpp
    cb[4] = new QCheckBox(tr("His 6-character locator in message"));//Its
    cb[4]->setChecked(true);// inportet defauld HV 1.26 here and in hvtxw.cpp
    cb[5] = new QCheckBox(tr("Minimum 3 Rs (e.g. RRR) in message"));
    cb[5]->setChecked(true);// inportet defauld HV 1.26 here and in hvtxw.cpp
    cb[6] = new QCheckBox(tr("RR73 in message"));
    cb[6]->setChecked(true);
    cb[7] = new QCheckBox(tr("73 in message"));
    cb[7]->setChecked(true);
    cb[8] = new QCheckBox(tr("QRZ in message"));
    cb[8]->setChecked(true);// inportet defauld HV 1.26 here and in hvtxw.cpp
    cb[9] = new QCheckBox(tr("Monitor radio 1"));
    cb[9]->setChecked(true);// inportet defauld HV 1.26 here and in hvtxw.cpp
    cb[10] = new QCheckBox(tr("Monitor radio 2"));
    cb[10]->setChecked(true);// inportet defauld HV 1.26 here and in hvtxw.cpp
    cb[11] = new QCheckBox(tr("My suffix in message"));
    cb[11]->setChecked(true);// inportet defauld HV 1.26 here and in hvtxw.cpp
    cb[12] = new QCheckBox(tr("My separating numeral + suffix in message"));
    cb[12]->setChecked(true);// inportet defauld HV 1.26 here and in hvtxw.cpp
    cb[13] = new QCheckBox(tr("His suffix in message"));
    cb[13]->setChecked(true);// inportet defauld HV 1.26 here and in hvtxw.cpp
    cb[14] = new QCheckBox(tr("His separating numeral + suffix in message"));
    cb[14]->setChecked(true);// inportet defauld HV 1.26 here and in hvtxw.cpp
    cb[15] = new QCheckBox(tr("Monitor radio 1 suffix in message"));
    cb[15]->setChecked(true);// inportet defauld HV 1.26 here and in hvtxw.cpp
    cb[16] = new QCheckBox(tr("Monitor radio 1 separating numeral + suffix in message"));
    cb[16]->setChecked(true);// inportet defauld HV 1.26 here and in hvtxw.cpp
    cb[17] = new QCheckBox(tr("Monitor radio 2 suffix in message"));
    cb[17]->setChecked(true);// inportet defauld HV 1.26 here and in hvtxw.cpp
    cb[18] = new QCheckBox(tr("Monitor radio 2 separating numeral + suffix in message"));
    cb[18]->setChecked(true);// inportet defauld HV 1.26 here and in hvtxw.cpp        
    cb[19] = new QCheckBox(tr("QSO B4 Call in MSG"));
    cb[19]->setChecked(true);
    cb[20] = new QCheckBox(tr("By Band"));
    cb[20]->setChecked(true);
    cb[21] = new QCheckBox(tr("By Mode"));
    cb[21]->setChecked(true);
    cb[22] = new QCheckBox(tr("B4 Grid in MSG"));
    cb[22]->setChecked(true);
    cb[23] = new QCheckBox(tr("Grid 4&&6/6"));
    cb[23]->setChecked(true);
    cb[24] = new QCheckBox(tr("By Band"));
    cb[24]->setChecked(true);
    cb[25] = new QCheckBox(tr("By Mode"));
    cb[25]->setChecked(true);
    cb[26] = new QCheckBox(tr("Transmitted message will change at next TX"));
    cb[26]->setChecked(true);
    cb[27] = new QCheckBox(tr("QSY message will change at next QSY"));
    cb[27]->setChecked(true);
    cb[28] = new QCheckBox(tr("Calls"));
    cb[28]->setChecked(true);
    le_cals_msg = new HvLeWithSpace();
    le_cals_msg->setMaxLength(150);
    le_cals_msg->setToolTip("LZ2HV,K1ABC  "+tr("(Call: Minimum 3 Characters)"));//Maximum 150 Characters
    
	//#define C_CB 29
	for (int i = 0; i < 26; ++i) connect(cb[i], SIGNAL(toggled(bool)), this, SLOT(TextMarkChanged(bool))); 
	connect(cb[26], SIGNAL(toggled(bool)), this, SLOT(TxTextMarkChanged(bool)));
	connect(cb[27], SIGNAL(toggled(bool)), this, SLOT(TxTextMarkChanged(bool)));
	connect(cb[28], SIGNAL(toggled(bool)), this, SLOT(TextMarkChanged(bool)));

    QVBoxLayout *V_l0m = new QVBoxLayout();
    V_l0m->setContentsMargins (0, 0, 0, 0);
    V_l0m->setSpacing(0);
    V_l0m->addWidget(cb[1]);
    V_l0m->addWidget(cb[11]);
    V_l0m->addWidget(cb[12]);
    
    QVBoxLayout *V_l0h = new QVBoxLayout();
    V_l0h->setContentsMargins (0, 0, 0, 0);
    V_l0h->setSpacing(0);
    V_l0h->addWidget(cb[2]);
    V_l0h->addWidget(cb[13]);
    V_l0h->addWidget(cb[14]);  
    
    QHBoxLayout *H_call = new QHBoxLayout();
    H_call->setContentsMargins(0,0,0,0);
    H_call->setSpacing(4);
    H_call->addWidget(cb[28]);
    H_call->addWidget(le_cals_msg);  

    QVBoxLayout *V_l1 = new QVBoxLayout();
    V_l1->setContentsMargins (0, 0, 0, 0);
    V_l1->setSpacing(0);
    V_l1->addWidget(cb[0]);
    //V_l1->addWidget(cb_my_call);
    //V_l1->addWidget(cb_his_call);
    V_l1->addWidget(cb[3]);
    V_l1->addWidget(cb[4]);
    V_l1->addWidget(cb[5]);
    V_l1->addWidget(cb[6]);
    V_l1->addWidget(cb[7]);
    V_l1->addWidget(cb[8]);
    //V_l1->addWidget(cb_my_call_s);
    //V_l1->addWidget(cb_my_call_ns);
    //V_l1->addWidget(cb_his_call_s);
    //V_l1->addWidget(cb_his_call_ns);
    V_l1->addWidget(cb[9]);
    V_l1->addWidget(cb[15]);
    V_l1->addWidget(cb[16]);
    V_l1->addWidget(cb[10]);
    V_l1->addWidget(cb[17]);
    V_l1->addWidget(cb[18]);
    V_l1->addLayout(H_call);

    b_ident = -1;
    b_[0] = new QPushButton(tr("COLOR"));
    b_[1] = new QPushButton(tr("COLOR"));
    b_[2] = new QPushButton(tr("COLOR"));
    b_[3] = new QPushButton(tr("COLOR"));
    b_[4] = new QPushButton(tr("COLOR"));
    b_[5] = new QPushButton(tr("COLOR"));
    b_[6] = new QPushButton(tr("COLOR"));

    QPushButton *b_set_default = new QPushButton(tr("SET DEFAULT COLORS"));
    
    QHBoxLayout *H_h0m = new QHBoxLayout();
    H_h0m->setContentsMargins (1, 1, 1, 1);
    H_h0m->setSpacing(4);
    H_h0m->addLayout(V_l0m);
    H_h0m->addWidget(b_[5]);
    QGroupBox *GB_C0m = new QGroupBox(tr("SECTION : My Call"));
    GB_C0m->setLayout(H_h0m);
    
    QHBoxLayout *H_h0h = new QHBoxLayout();
    H_h0h->setContentsMargins (1, 1, 1, 1);
    H_h0h->setSpacing(4);
    H_h0h->addLayout(V_l0h);
    H_h0h->addWidget(b_[6]);
    QGroupBox *GB_C0h = new QGroupBox(tr("SECTION : His Call"));
    GB_C0h->setLayout(H_h0h);
    
    QHBoxLayout *H_h1 = new QHBoxLayout();
    H_h1->setContentsMargins (1, 1, 1, 1);
    H_h1->setSpacing(4);
    H_h1->addLayout(V_l1);
    H_h1->addWidget(b_[0]);
    QGroupBox *GB_C1 = new QGroupBox(tr("SECTION 1:"));
    GB_C1->setLayout(H_h1);

    QHBoxLayout *H_h2 = new QHBoxLayout();
    H_h2->setContentsMargins (0, 0, 0, 0);
    H_h2->setSpacing(4);
    H_h2->addWidget(cb[19]);
    H_h2->addWidget(cb[20]);
    H_h2->addWidget(cb[21]);
    H_h2->addWidget(b_[1]);

    QHBoxLayout *H_h3 = new QHBoxLayout();
    H_h3->setContentsMargins (0, 0, 0, 0);
    H_h3->setSpacing(4);
    H_h3->addWidget(cb[22]);
    H_h3->addWidget(cb[23]);
    H_h3->addWidget(cb[24]);
    H_h3->addWidget(cb[25]);
    H_h3->addWidget(b_[2]);

    QHBoxLayout *H_ht = new QHBoxLayout();
    H_ht->setContentsMargins (0, 0, 0, 0);
    H_ht->setSpacing(4);
    H_ht->addWidget(cb[26]);
    H_ht->addWidget(b_[3]);

    QHBoxLayout *H_hq = new QHBoxLayout();
    H_hq->setContentsMargins (0, 0, 0, 0);
    H_hq->setSpacing(4);
    H_hq->addWidget(cb[27]);
    H_hq->addWidget(b_[4]);

    QVBoxLayout *V_l2 = new QVBoxLayout();
    V_l2->setContentsMargins (1, 1, 1, 1);
    V_l2->setSpacing(3);
    V_l2->addLayout(H_h2);
    V_l2->addLayout(H_h3);
    V_l2->addLayout(H_ht);
    V_l2->addLayout(H_hq);
    //QGroupBox *GB_C2 = new QGroupBox("COLOR SECTION 2: QSO B4, Band Mode, will change filtering for Warn Me If QSO B4");//MSK,FT8,JT65
    QGroupBox *GB_C2 = new QGroupBox(tr("SECTION 2: QSO B4, Band Mode, will change filter Warn Me If QSO B4"));
    GB_C2->setLayout(V_l2);

    connect(b_[0], SIGNAL(clicked(bool)), this, SLOT(OpenCDBox1()));
    connect(b_[1], SIGNAL(clicked(bool)), this, SLOT(OpenCDBox2()));
    connect(b_[2], SIGNAL(clicked(bool)), this, SLOT(OpenCDBox3()));
    connect(b_[3], SIGNAL(clicked(bool)), this, SLOT(OpenCDBoxT()));
    connect(b_[4], SIGNAL(clicked(bool)), this, SLOT(OpenCDBoxQ()));
    connect(b_[5], SIGNAL(clicked(bool)), this, SLOT(OpenCDBoxMC()));
    connect(b_[6], SIGNAL(clicked(bool)), this, SLOT(OpenCDBoxHC()));
    connect(b_set_default, SIGNAL(clicked(bool)), this, SLOT(setDefColors()));

    CD_box = new QColorDialog(this);
    CD_box->setOption(QColorDialog::NoButtons);
    connect(CD_box, SIGNAL(currentColorChanged(QColor)), this, SLOT(ColorChanged(QColor)));

    QVBoxLayout *V_l = new QVBoxLayout(this);
    V_l->setContentsMargins (4, 4, 4, 4);
    V_l->setSpacing(4);
    //V_l->addLayout(H_h1);
    //V_l->addLayout(H_h2);
    //V_l->addLayout(H_h3);
    V_l->addWidget(GB_C0m);
    V_l->addWidget(GB_C0h);
    V_l->addWidget(GB_C1);
    V_l->addWidget(GB_C2);
    //V_l->addLayout(H_h2);
    //V_l->addLayout(H_h3);
    /*V_l->addWidget(GB_C2);
    V_l->addWidget(GB_C3);*/
    V_l->addWidget(b_set_default);

    this->setLayout(V_l);

	//default normal sheme
    collors[0][0] = collors0[0][0] = QColor(100,254,100);//CQ RRR 73
    collors[0][1] = collors0[0][1] = QColor(254,110,110);//QSO B4
    collors[0][2] = collors0[0][2] = QColor(130,255,255);//loc
    collors[0][3] = collors0[0][3] = QColor(255,210,210);//tx in list2
    collors[0][4] = collors0[0][4] = QColor(255,255,150);//qsy in list2
    collors[0][5] = collors0[0][5] = QColor(255,180,100);//2.56 mycall
    collors[0][6] = collors0[0][6] = QColor(100,210,255);//2.63 hiscall
	//default dark sheme
    collors[1][0] = collors0[1][0] = QColor(0,125,0);//CQ RRR 73
    collors[1][1] = collors0[1][1] = QColor(130,0,0);//QSO B4
    collors[1][2] = collors0[1][2] = QColor(20,120,120);//loc
    collors[1][3] = collors0[1][3] = QColor(110,44,44);//tx in list2
    collors[1][4] = collors0[1][4] = QColor(100,100,59);//qsy in list2
    collors[1][5] = collors0[1][5] = QColor(100,70,40);//2.56 mycall
    collors[1][6] = collors0[1][6] = QColor(50,100,140);//2.63 hiscall

    RefreshAll();
}
HvTxtColor::~HvTxtColor()
{}
/*void HvTxtColor::LeTextChanged(QString)
{
	//qDebug()<<le_cals_msg->text();
	QString s = le_cals_msg->text();
	le_cals_msg->setText(CorrectSyntax(s,true));
}*/
void HvTxtColor::closeEvent(QCloseEvent*)
{	
	TextMarkChanged(true);//refresh
}
void HvTxtColor::OpenCDBox1()
{
    b_ident = 0;    
    CD_box->setWindowTitle(tr("Choose Section 1 Color")); //CD_box->move(pos().x()+width(),pos().y());//+80);
    CD_box->open();
    CD_box->setCurrentColor(collors[dsty][0].rgb());
}
void HvTxtColor::OpenCDBox2()
{
    b_ident = 1;
    CD_box->setWindowTitle(tr("Choose QSO B4 Call in message Color"));
    CD_box->open();
    CD_box->setCurrentColor(collors[dsty][1].rgb());
}
void HvTxtColor::OpenCDBox3()
{
    b_ident = 2;
    CD_box->setWindowTitle(tr("Choose B4 Grid in message Color"));
    CD_box->open();
    CD_box->setCurrentColor(collors[dsty][2].rgb());
}
void HvTxtColor::OpenCDBoxT()
{
    b_ident = 3;
    CD_box->setWindowTitle(tr("Choose Transmitted message Color"));
    CD_box->open();
    CD_box->setCurrentColor(collors[dsty][3].rgb());
}
void HvTxtColor::OpenCDBoxQ()
{
    b_ident = 4;
    CD_box->setWindowTitle(tr("Choose QSY message Color"));
    CD_box->open();
    CD_box->setCurrentColor(collors[dsty][4].rgb());
}
void HvTxtColor::OpenCDBoxMC()
{
    b_ident = 5;
    CD_box->setWindowTitle(tr("Choose My Call in message Color"));
    CD_box->open();
    CD_box->setCurrentColor(collors[dsty][5].rgb());
}
void HvTxtColor::OpenCDBoxHC()
{
    b_ident = 6;
    CD_box->setWindowTitle(tr("Choose His Call in message Color"));
    CD_box->open();
    CD_box->setCurrentColor(collors[dsty][6].rgb());
}
void HvTxtColor::SetFont(QFont f)
{
    for (int i =0; i<C_BT; ++i) b_[i]->setFont(f);
}
void HvTxtColor::ColorChanged(QColor c)
{
    QString s ="QPushButton{border-color :rgb("+QString("%1").arg(c.red())+","+QString("%1").arg(c.green())+
               ","+QString("%1").arg(c.blue())+");border-width: 4px;border-style: outset;border-radius: 5px;}";
    b_[b_ident]->setStyleSheet(s);
    collors[dsty][b_ident]=c;
    emit EmitTextMarkColors(collors[dsty],C_COLORS,cb[26]->isChecked(),cb[27]->isChecked());
}
void HvTxtColor::RefreshAll()
{
    for (int i=0; i < C_COLORS; ++i)
    {
        b_ident = i;
        ColorChanged(collors[dsty][i]);
    }
}
void HvTxtColor::setDefColors()
{
    collors[dsty][0] = collors0[dsty][0];
    collors[dsty][1] = collors0[dsty][1];
    collors[dsty][2] = collors0[dsty][2];
    collors[dsty][3] = collors0[dsty][3];
    collors[dsty][4] = collors0[dsty][4];
    collors[dsty][5] = collors0[dsty][5];
    collors[dsty][6] = collors0[dsty][6];
    RefreshAll();
}
void HvTxtColor::SetTextColor(QString s)
{
    QStringList lss=s.split("|");
    if (lss.count()>=C_SHEM)
    {
        for (int j =0; j<C_SHEM; j++)
        {
            QString ss = lss.at(j);
            QStringList ls=ss.split("#");
            if (ls.count()>2)// min one color
            {
                int k = 0;
                for (int i =0; i<C_COLORS; i++)
                {
                    if (k+3>ls.count()) break;//2.06 crash
                    collors[j][i].setRed(ls[k].toInt());
                    k++;
                    collors[j][i].setGreen(ls[k].toInt());
                    k++;
                    collors[j][i].setBlue(ls[k].toInt());
                    k++;
                }
            }
        }
    }
    RefreshAll();
}
QString HvTxtColor::GetTextMark()
{
    QString res;    
	for (int i = 0; i < C_CB; ++i)
	{
		if (cb[i]->isChecked()) res.append("1#");
    	else res.append("0#");
	}     
    res.append(le_cals_msg->text());//29      
    return res;
}
QString HvTxtColor::CorrectSyntax(QString txt ,bool cordot)
{
    QString res;
    txt = txt.trimmed();
    if (!txt.isEmpty())
    {
        if (cordot) txt.replace(".",",");
        QStringList ltxt = txt.split(",");
        ltxt.removeDuplicates();
        if (ltxt.count()>0)
        {
            for (int i = 0; i<ltxt.count(); ++i)
            {
                QString stxt = ltxt.at(i);
                stxt = stxt.trimmed();
                //if (!stxt.isEmpty())
                if (stxt.count()>2)//minimum 3 char
                {
                    res.append(stxt);
                    if (i<ltxt.count()-1)res.append(",");
                }
            }
        }
        else
        {
            res.append(txt);
        }
    }
    if (res.endsWith(",")) res = res.mid(0,res.count()-1);
    return res.trimmed();
}
void HvTxtColor::SetTextMark(QString s)
{
    QStringList ls=s.split("#");
    ls<<"1"<<"1"<<"1"<<"1"<<"1"<<"1"<<"1"<<"1"<<"1"<<"1"<<"1"<<"1"<<"1"<<"1"<<"1"<<
        "1"<<"1"<<"1"<<"1"<<"1"<<"1"<<"1"<<"1"<<"1"<<"1"<<"1"<<"1"<<"1"<<"1"<<"1"<<"1";//31	  
	for (int i = 0; i < C_CB; ++i)
	{
		if (!ls.at(i).isEmpty() && ls.at(i)=="1") cb[i]->setChecked(true);
        else cb[i]->setChecked(false);
	}    
    if (!ls.at(29).isEmpty()) 
    {
    	if (ls.at(29)!="1") le_cals_msg->setText(ls.at(29));//le_cals_msg->setText(CorrectSyntax(ls.at(29),true));    	
   	}
  	TextMarkChanged(true);//refresh
}
void HvTxtColor::TxTextMarkChanged(bool)
{
    emit EmitTextMarkColors(collors[dsty],C_COLORS,cb[26]->isChecked(),cb[27]->isChecked());
}
void HvTxtColor::TextMarkChanged(bool)
{
    bool f[32]={false,false,false,false,false,false,false,false,false,false,false
                ,false,false,false,false,false,false,false,false,false,false,false,false,false
                ,false,false,false,false,false,false,false,false};
	for (int i = 0; i < C_CB; ++i)
	{
		if (i==23)
		{
    		if (cb[23]->isChecked())
    		{
        		f[23]=true;
        		cb[23]->setText(tr("Grid 4&&6/6"));//new QCheckBox("By 4,6 Char");
    		}
    		else cb[23]->setText(tr("Grid 6/4&&6"));			
		}
		else if (cb[i]->isChecked()) f[i]=true;
	}
    le_cals_msg->setText(CorrectSyntax(le_cals_msg->text(),true));      
    emit EmitTextMark(f,le_cals_msg->text()); //qDebug()<<f[0]<<f[1]<<f[28]<<le_cals_msg->text();
}
