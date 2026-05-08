/* MSHV FilterDialog
 * Copyright 2020 Hrisimir Hristov, LZ2HV
 * May be used under the terms of the GNU General Public License (GPL)
 */
#include "hvfilterdialog.h"

#include <QVBoxLayout>
#include <QPushButton>
#include <QGroupBox>
//#include <QtGui>

HvFilterDialog::HvFilterDialog(bool f,QWidget *parent)
        : QDialog(parent)
{
	dsty = f;
    setWindowTitle (tr("Decode List Filters")+" FT");//All For Me And +/-10 kHz From RX
    setWindowFlags(windowFlags() ^ Qt::WindowContextHelpButtonHint);// maha help butona
    QVBoxLayout *LV = new QVBoxLayout(this);
    LV->setContentsMargins(10, 10, 10, 8);
    LV->setSpacing(4);

    //QLabel *ltext = new QLabel("No Hideable messages are: All for me and +/-10 kHz around RX frequency");
    ltext = new QLabel(tr("Filter does not work for: All for me and +/-10 kHz around RX frequency"));
    if(dsty) ltext->setStyleSheet("QLabel{font-weight:bold;color:rgb(255,150,150);}");
    else ltext->setStyleSheet("QLabel{font-weight:bold;color:rgb(180,0,0);}");
    //ltext->setStyleSheet("QLabel {font-weight: bold; color : red; }");

    cb_hacontt[0] = new QCheckBox(tr("Africa"));
    cb_hacontt[1] = new QCheckBox(tr("Antarctica"));
    cb_hacontt[2] = new QCheckBox(tr("Asia"));
    cb_hacontt[3] = new QCheckBox(tr("Europe"));
    cb_hacontt[4] = new QCheckBox(tr("Oceania"));
    cb_hacontt[5] = new QCheckBox(tr("North America"));
    cb_hacontt[6] = new QCheckBox(tr("South America"));
    QGroupBox *GB_HIDE = new QGroupBox(tr("Hide Messages From Continents")+":");
    //GB_HIDE->setContentsMargins(0, 15, 0, 0);

	cb_hide_b4qso = new QCheckBox(tr("Hide QSO B4 Messages"));
	//cb_hide_b4qso->setToolTip(tr("Filtered By: Call, Band, Mode")+"\n"
							 //+tr("Activated in Menu Options, Text Highlight, Section 2"));

    cb_cq = new QCheckBox(tr("Show CQ Messages Only"));
    cb_cq73 = new QCheckBox(tr("Show CQ,RR73,73 Messages Only"));
    QGroupBox *GB_CCQ73 = new QGroupBox(tr("Show Custom CQs,RRR,RR73,73 Messages Only:    (Maximum 150 Characters)"));
    cb_ccq = new QCheckBox(tr("Enable"));
    le_ccq = new HvLeWithSpace();
    le_ccq->setMaxLength(150);

    //Show And Unhide Only Messages At End That Contain
    int limit3 = 500;
    QGroupBox *GB_CTXT3 = new QGroupBox(tr("Show Only Messages With Grid Locator, That Contain:    (Maximum 500 Characters)"));
    cb_contm3 = new QCheckBox(tr("Enable"));
    le_contm3 = new HvLeWithSpace();
    le_contm3->setMaxLength(limit3);

    //"And Show Only All Messages Ho Contains With:"
    //"And Show Only All Messages That Contain With:"
    //"Show Only And Unhide All Messages That Contain With:"
    //Show And Unhide Only Messages That Contain DX Call
    //Show And Unhide Only Messages DX Call That Contain With Key:"
    int limit0 = 1000;
    QGroupBox *GB_CTXT0 = new QGroupBox(tr("Show Only Messages With DX Call, That Contain:    (Maximum 1000 Characters)"));
    cb_contm0 = new QCheckBox(tr("Enable"));
    le_contm0 = new HvLeWithSpace();
    le_contm0->setMaxLength(limit0);

    int limit1 = 3000;                    //Show And Unhide Only Messages With Specific His Call Content
    QGroupBox *GB_CTXT1 = new QGroupBox(tr("Show Only Messages With DX Call:    (Maximum 3000 Characters)"));
    cb_contm1 = new QCheckBox(tr("Enable"));
    le_contm1 = new HvLeWithSpace();
    le_contm1->setMaxLength(limit1);
    cb_contm2 = new QCheckBox(tr("Enable"));
    le_contm2 = new HvLeWithSpace();
    le_contm2->setMaxLength(limit1);

    // Hide 2.62
 	QGroupBox *GB_HIDCCNT = new QGroupBox(tr("Hide Messages From Country:    (Please Use Maximum 10 Countrys)"));
    QLabel *l_hidcnt = new QLabel(tr("Add Country")+":");
    CbHidCountrys = new QComboBox();
    QLabel *l_hidremcnt = new QLabel(tr("REM Country")+":");
    CbHidRemCountrys = new QComboBox();
    QHBoxLayout *hidhbccnt = new QHBoxLayout();
    b_hidclr = new QPushButton(tr("CLEAR"));
    hidhbccnt->setContentsMargins(5,5,5,5);
    hidhbccnt->setSpacing(5);
    hidhbccnt->addWidget(l_hidcnt);
    //hidhbccnt->setAlignment(l_cnt,Qt::AlignRight);
    hidhbccnt->addWidget(CbHidCountrys);
    hidhbccnt->addWidget(l_hidremcnt);
    hidhbccnt->addWidget(CbHidRemCountrys);
    hidhbccnt->addWidget(b_hidclr);
    CbHidCountrys->setMinimumWidth(200);
    //CbHidRemCountrys->setMinimumWidth(200);
    hidhbccnt->setAlignment(Qt::AlignLeft);
    cb_contm6 = new QCheckBox(tr("Enable"));
    le_contm6 = new QLineEdit();
    le_contm6->setReadOnly(true);
    QHBoxLayout *hidhbcontm4 = new QHBoxLayout();
    hidhbcontm4->setContentsMargins(5,5,5,5);
    hidhbcontm4->setSpacing(5);
    hidhbcontm4->addWidget(cb_contm6);
    hidhbcontm4->addWidget(le_contm6);
    QVBoxLayout *hidvbccnt = new QVBoxLayout();
    hidvbccnt->setContentsMargins(0,0,0,0);
    hidvbccnt->setSpacing(0);
    hidvbccnt->addLayout(hidhbccnt);
    hidvbccnt->addLayout(hidhbcontm4);
    GB_HIDCCNT->setLayout(hidvbccnt);

	// Unhide 
    QGroupBox *GB_CCNT = new QGroupBox(tr("Show Only Messages From Country:    (Please Use Maximum 10 Countrys)"));
    QLabel *l_cnt = new QLabel(tr("Add Country")+":");
    CbCountrys = new QComboBox();
    QLabel *l_remcnt = new QLabel(tr("REM Country")+":");
    CbRemCountrys = new QComboBox();
    QHBoxLayout *hbccnt = new QHBoxLayout();
    b_clr = new QPushButton(tr("CLEAR"));
    hbccnt->setContentsMargins(5,5,5,5);
    hbccnt->setSpacing(5);
    hbccnt->addWidget(l_cnt);
    //hbccnt->setAlignment(l_cnt,Qt::AlignRight);
    hbccnt->addWidget(CbCountrys);
    hbccnt->addWidget(l_remcnt);
    hbccnt->addWidget(CbRemCountrys);
    hbccnt->addWidget(b_clr);
    CbCountrys->setMinimumWidth(200);
    //CbRemCountrys->setMinimumWidth(200);
    hbccnt->setAlignment(Qt::AlignLeft);
    cb_contm4 = new QCheckBox(tr("Enable"));
    le_contm4 = new QLineEdit();
    le_contm4->setReadOnly(true);
    QHBoxLayout *hbcontm4 = new QHBoxLayout();
    hbcontm4->setContentsMargins(5,5,5,5);
    hbcontm4->setSpacing(5);
    hbcontm4->addWidget(cb_contm4);
    hbcontm4->addWidget(le_contm4);
    QVBoxLayout *vbccnt = new QVBoxLayout();
    vbccnt->setContentsMargins(0,0,0,0);
    vbccnt->setSpacing(0);
    vbccnt->addLayout(hbccnt);
    vbccnt->addLayout(hbcontm4);
    GB_CCNT->setLayout(vbccnt);   
    
    QGroupBox *GB_CPFX = new QGroupBox(tr("Show Only Messages With DX Prefix:    (Standard Prefix Only!, Maximum 500 Characters)"));
    cb_pfx5 = new QCheckBox(tr("Enable"));
    le_pfx5 = new HvLeWithSpace();
    le_pfx5->setMaxLength(500);
    QHBoxLayout *hbpfx5 = new QHBoxLayout();
    hbpfx5->setContentsMargins(5, 5, 5, 5);
    hbpfx5->setSpacing(5);
    hbpfx5->addWidget(cb_pfx5);
    hbpfx5->addWidget(le_pfx5);
    GB_CPFX->setLayout(hbpfx5);

    connect(b_clr, SIGNAL(clicked(bool)), this, SLOT(ClrCountrys()));
    connect(b_hidclr, SIGNAL(clicked(bool)), this, SLOT(ClrHidCountrys()));

    cb_usefudpdectxt = new QCheckBox(tr("Apply Filters to UDP Decoded Text Messages Broadcast"));
    QHBoxLayout *hb10 = new QHBoxLayout();
    hb10->setContentsMargins(7,1,5,1);
    hb10->setSpacing(5);
    hb10->addWidget(cb_usefudpdectxt);
    
    //Use Filters For Automatic Sequencing Answer (Not Recommended)
    //Use Filtered ASeq Answer (Not Recommended)
    //Use The Filters For An Automatic Sequencing Answer (Not Recommended)
    //Use Filters Restrictions For Automatic Sequencing Answer (Not Recommended)
    //Use the filters restrictions to reply with an automatic sequence (Not Recommended)
    //Use Filters for Answering on Automatic Sequencing (Not Recommended)
    //Auto CQ and auto responding
    cb_filtered_answer = new QCheckBox(tr("Use Filters For Automatic Sequencing Answer (Not Recommended)"));//2.73
    if(dsty) cb_filtered_answer->setStyleSheet("QCheckBox{font-weight:bold;color:rgb(255,150,150);}");
    else cb_filtered_answer->setStyleSheet("QCheckBox{font-weight:bold;color:rgb(180,0,0);}");
    QHBoxLayout *hb11 = new QHBoxLayout();
    hb11->setContentsMargins(7,1,5,1);
    hb11->setSpacing(5);
    hb11->addWidget(cb_filtered_answer);    
    //cb_filtered_answer->setHidden(true);

    cb_gonoff = new QCheckBox(tr("USE FILTERS"));//Global Stop/Start Filters   "On/Off All Filters"
    QPushButton *b_set_default_filter = new QPushButton(tr("SET DEFAULT FILTER (NO FILTER)"));
    QPushButton *b_apply_filter = new QPushButton(tr("Apply Changes"));
    
    pb_fltrOnOff = new QPushButton(tr("FILTERS OFF"));
    pb_fltrOnOff->setStyleSheet("QPushButton{background-color:palette(Button);}");
    pb_fltrOnOff->setHidden(true);
    cb_usebtflonoff  = new QCheckBox(tr("USE Button FILTERS ON/OFF"));
    //SetDefaultFilter();

    connect(b_set_default_filter, SIGNAL(clicked(bool)), this, SLOT(SetDefaultFilter()));
    connect(b_apply_filter, SIGNAL(clicked(bool)), this, SLOT(ApplyChFilter()));
    connect(cb_cq, SIGNAL(clicked(bool)), this, SLOT(CBCQChanged(bool)));
    connect(cb_cq73, SIGNAL(clicked(bool)), this, SLOT(CBCQ73Changed(bool)));
    connect(cb_ccq, SIGNAL(clicked(bool)), this, SLOT(CBCCQChanged(bool)));

    //connect(cb_contm0, SIGNAL(clicked(bool)), this, SLOT(CBContmChanged0(bool)));
    connect(cb_contm1, SIGNAL(clicked(bool)), this, SLOT(CBContmChanged1(bool)));
    connect(cb_contm2, SIGNAL(clicked(bool)), this, SLOT(CBContmChanged2(bool)));
    connect(cb_gonoff, SIGNAL(clicked(bool)), this, SLOT(CBGOnOffChanged(bool)));
    connect(pb_fltrOnOff, SIGNAL(clicked(bool)), this, SLOT(PbSetOnOff()));
    connect(cb_usebtflonoff, SIGNAL(toggled(bool)), this, SLOT(CbHidFLBtOnOff(bool)));
    //qDebug()<<"Connecttttttttttt";

    SetDefaultFilter();

    QHBoxLayout *hbcq = new QHBoxLayout();
    hbcq->setContentsMargins(7, 5, 5, 5);
    hbcq->setSpacing(5);
    hbcq->addWidget(cb_hide_b4qso);
    hbcq->addWidget(cb_cq);
    hbcq->addWidget(cb_cq73);

    QHBoxLayout *hb00 = new QHBoxLayout();
    hb00->setContentsMargins(5, 5, 5, 5);
    hb00->setSpacing(5);
    for (int i = 0; i < 7; ++i) hb00->addWidget(cb_hacontt[i]);
    GB_HIDE->setLayout(hb00);

    QHBoxLayout *hb01 = new QHBoxLayout();
    hb01->setContentsMargins(5, 5, 5, 5);
    hb01->setSpacing(5);
    hb01->addWidget(cb_ccq);
    hb01->addWidget(le_ccq);
    GB_CCQ73->setLayout(hb01);

    QHBoxLayout *hbcontm3 = new QHBoxLayout();
    hbcontm3->setContentsMargins(5, 5, 5, 5);
    hbcontm3->setSpacing(5);
    hbcontm3->addWidget(cb_contm3);
    hbcontm3->addWidget(le_contm3);
    GB_CTXT3->setLayout(hbcontm3);

    QHBoxLayout *hbcontm0 = new QHBoxLayout();
    hbcontm0->setContentsMargins(5, 5, 5, 5);
    hbcontm0->setSpacing(5);
    hbcontm0->addWidget(cb_contm0);
    hbcontm0->addWidget(le_contm0);
    GB_CTXT0->setLayout(hbcontm0);

    QHBoxLayout *hbcontm1 = new QHBoxLayout();
    hbcontm1->setContentsMargins(5, 5, 5, 5);
    hbcontm1->setSpacing(5);
    hbcontm1->addWidget(cb_contm1);
    hbcontm1->addWidget(le_contm1);
    QHBoxLayout *hbcontm2 = new QHBoxLayout();
    hbcontm2->setContentsMargins(5, 5, 5, 5);
    hbcontm2->setSpacing(5);
    hbcontm2->addWidget(cb_contm2);
    hbcontm2->addWidget(le_contm2);
    QVBoxLayout *vbcontm = new QVBoxLayout();
    vbcontm->setContentsMargins(0, 0, 0, 0);
    vbcontm->setSpacing(0);
    vbcontm->addLayout(hbcontm1);
    vbcontm->addLayout(hbcontm2);
    GB_CTXT1->setLayout(vbcontm);

    QHBoxLayout *hb02 = new QHBoxLayout();
    hb02->setContentsMargins(7, 0, 0, 0);
    hb02->setSpacing(5);
    hb02->addWidget(cb_gonoff);
    hb02->addWidget(cb_usebtflonoff);
    //hb02->setAlignment(cb_gonoff,Qt::AlignLeft);
    hb02->addWidget(b_apply_filter);
    //hb02->setAlignment(b_apply_filter,Qt::AlignLeft);
    hb02->addWidget(b_set_default_filter);
    //hb02->setAlignment(Qt::AlignHCenter);

    ltext->setContentsMargins(0,0,0,2);
    LV->addWidget(ltext);
    LV->setAlignment(ltext,Qt::AlignCenter);
    LV->addWidget(GB_HIDE);
    LV->addWidget(GB_HIDCCNT);
    LV->addLayout(hbcq);
    LV->addWidget(GB_CCQ73);
    LV->addWidget(GB_CCNT);
    LV->addWidget(GB_CPFX);
    LV->addWidget(GB_CTXT3);
    LV->addWidget(GB_CTXT0);
    LV->addWidget(GB_CTXT1);
    LV->addLayout(hb10);
    LV->addLayout(hb11);

    LV->addLayout(hb02);
    //LV->addWidget(b_set_default_filter);
    this->setLayout(LV);
    //this->setMinimumWidth(620);
    //this->setMinimumSize(200,100);
}
HvFilterDialog::~HvFilterDialog()
{}
void HvFilterDialog::SetTextMark(bool *f,QString)
{
    QString s = tr("Filtered By")+": "+tr("Call");
    if (f[20]) s.append(", "+tr("Band"));// band
    if (f[21]) s.append(", "+tr("Mode"));// mode
    cb_hide_b4qso->setToolTip(s+"\n"+tr("Enabled in Menu Options, Text Highlight, Section 2"));
}
void HvFilterDialog::RefrCountrys()
{
    QStringList l;
    if (le_contm4->text()!="") l = le_contm4->text().split(",");    
	l.prepend(" - Remove Country - ");
    CbRemCountrys->clear();
    CbRemCountrys->addItems(l);
    CbCountrys->setCurrentIndex(0);
    CbRemCountrys->setCurrentIndex(0);
}
void HvFilterDialog::ClrCountrys()
{
    le_contm4->setText("");
    RefrCountrys();
}
void HvFilterDialog::CbRemCountrysChanged(QString s)
{
    if (s==" - Remove Country - " || s.isEmpty()) return;
    QString str;
    str = le_contm4->text();
    str.remove(s);
    le_contm4->setText(CorrectSyntax(str,false));  
    RefrCountrys();
}
void HvFilterDialog::CbCountrysChanged(QString s)
{
    if (s==" - Choice Country - ") return;
    int l_limit = 18;//for 20
    QString str = le_contm4->text();
    if (str.count(",")>l_limit) return;
    if (str!="") str.append(",");
    str.append(s);
    le_contm4->setText(str);
    RefrCountrys();
}
void HvFilterDialog::RefrHidCountrys()
{
    QStringList l;
    if (le_contm6->text()!="") l = le_contm6->text().split(",");    
	l.prepend(" - Remove Country - ");
    CbHidRemCountrys->clear();
    CbHidRemCountrys->addItems(l);
    CbHidCountrys->setCurrentIndex(0);
    CbHidRemCountrys->setCurrentIndex(0);
}
void HvFilterDialog::ClrHidCountrys()
{
    le_contm6->setText("");
    RefrHidCountrys();
}
void HvFilterDialog::CbHidRemCountrysChanged(QString s)
{
    if (s==" - Remove Country - " || s.isEmpty()) return;
    QString str;
    str = le_contm6->text();
    str.remove(s);
    le_contm6->setText(CorrectSyntax(str,false));  
    RefrHidCountrys();
}
void HvFilterDialog::CbHidCountrysChanged(QString s)
{
    if (s==" - Choice Country - ") return;
    int l_limit = 18;//for 20
    QString str = le_contm6->text();
    if (str.count(",")>l_limit) return;
    if (str!="") str.append(",");
    str.append(s);
    le_contm6->setText(str);
    RefrHidCountrys();
}
void HvFilterDialog::SetCountries(QStringList l)
{
    l.prepend(" - Choice Country - ");
    CbCountrys->addItems(l);
    connect(CbCountrys, SIGNAL(currentIndexChanged(QString)), this, SLOT(CbCountrysChanged(QString)));
    connect(CbRemCountrys, SIGNAL(currentIndexChanged(QString)), this, SLOT(CbRemCountrysChanged(QString)));      
    CbHidCountrys->addItems(l);
    CbHidCountrys->insertItem(1,"Unknown");//2.66
    connect(CbHidCountrys, SIGNAL(currentIndexChanged(QString)), this, SLOT(CbHidCountrysChanged(QString)));
    connect(CbHidRemCountrys, SIGNAL(currentIndexChanged(QString)), this, SLOT(CbHidRemCountrysChanged(QString)));
}
void HvFilterDialog::SetFont(QFont f)
{
    le_ccq->setFont(f);
    if(dsty) 
    {
    	ltext->setStyleSheet("QLabel{font-weight:bold;color:rgb(255,150,150);}"); 
    	cb_filtered_answer->setStyleSheet("QCheckBox{font-weight:bold;color:rgb(255,150,150);}");   	
   	}
    else 
    {
    	ltext->setStyleSheet("QLabel{font-weight:bold;color:rgb(180,0,0);}"); 
    	cb_filtered_answer->setStyleSheet("QCheckBox{font-weight:bold;color:rgb(180,0,0);}");   	
   	}
}
void HvFilterDialog::CBCQChanged(bool)
{
    if (cb_cq->isChecked())
    {
        cb_ccq->setChecked(false);
        cb_cq73->setChecked(false);
    }
}
void HvFilterDialog::CBCQ73Changed(bool)
{
    if (cb_cq73->isChecked())
    {
        cb_ccq->setChecked(false);
        cb_cq->setChecked(false);
    }
}
void HvFilterDialog::CBCCQChanged(bool)
{
    if (cb_ccq->isChecked())
    {
        cb_cq->setChecked(false);
        cb_cq73->setChecked(false);
    }
}
void HvFilterDialog::CBContmChanged1(bool)
{
    if (cb_contm1->isChecked()) cb_contm2->setChecked(false);
}
void HvFilterDialog::CBContmChanged2(bool)
{
    if (cb_contm2->isChecked()) cb_contm1->setChecked(false);
}
QString HvFilterDialog::CorrectSyntax(QString txt ,bool cordot)
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
                if (!stxt.isEmpty())
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
void HvFilterDialog::ApplyChFilter()
{
    le_ccq->setText(CorrectSyntax(le_ccq->text(),true));
    le_contm0->setText(CorrectSyntax(le_contm0->text(),true));
    le_contm1->setText(CorrectSyntax(le_contm1->text(),true));
    le_contm2->setText(CorrectSyntax(le_contm2->text(),true));
    le_contm3->setText(CorrectSyntax(le_contm3->text(),true));
    le_contm4->setText(CorrectSyntax(le_contm4->text(),false));    
    le_pfx5->setText(CorrectSyntax(le_pfx5->text(),true));
    le_contm6->setText(CorrectSyntax(le_contm6->text(),false));   
    SetFilter();
}
void HvFilterDialog::PbSetOnOff()
{
	if (cb_gonoff->isChecked()) cb_gonoff->setChecked(false);		
	else cb_gonoff->setChecked(true);
	SetFilter(); //ApplyChFilter();
}
static bool g_ModeIsHiden = true;
void HvFilterDialog::CbHidFLBtOnOff(bool f)
{
	if (!f) pb_fltrOnOff->setHidden(true);
	else if (!g_ModeIsHiden) pb_fltrOnOff->setHidden(false);
}
void HvFilterDialog::SetHidFLBtOnOff(bool f)
{
	g_ModeIsHiden = f; //qDebug()<<f;
	if (!cb_usebtflonoff->isChecked()) pb_fltrOnOff->setHidden(true);
	else pb_fltrOnOff->setHidden(f);
}
void HvFilterDialog::CBGOnOffChanged(bool)
{	
    ApplyChFilter();     
}
void HvFilterDialog::RefreshPbSetOnOff(bool f)
{
	if (cb_gonoff->isChecked())
	{
		//pb_fltrOnOff->setText(tr("FL IS ON"));
		pb_fltrOnOff->setText(tr("FILTERS ON"));
		if (f)
		{
        	if (dsty) pb_fltrOnOff->setStyleSheet("QPushButton{background-color:rgb(140,60,60);}"); //170, 28, 28
        	else pb_fltrOnOff->setStyleSheet("QPushButton{background-color:rgb(255,158,158);}"); //255, 128, 128 
        	//if (dsty) pb_fltrOnOff->setStyleSheet("QPushButton{background-color:rgb(155,90,90);}");
        	//else pb_fltrOnOff->setStyleSheet("QPushButton{background-color:rgb(255,190,190);}");			
		}
		else pb_fltrOnOff->setStyleSheet("QPushButton{background-color:palette(Button);}"); 
	}
	else 
	{
		//pb_fltrOnOff->setText(tr("FL IS OFF"));
		pb_fltrOnOff->setText(tr("FILTERS OFF"));
		pb_fltrOnOff->setStyleSheet("QPushButton{background-color:palette(Button);}");		
	}	
}
void HvFilterDialog::SetDefaultFilter()
{    
    for (int i = 0; i < 7; ++i) cb_hacontt[i]->setChecked(false);
    cb_cq->setChecked(false);
    cb_cq73->setChecked(false);
    cb_ccq->setChecked(false);
    //le_ccq->setText("CQ RU,CQ WW,CQ TEST,CQ FD,RRR,RR73,73");
    le_ccq->setText("CQ RU,CQ WW,CQ BU,CQ FT,CQ TEST,CQ PDC,CQ FD,RRR,RR73,73");
    cb_contm0->setChecked(false);
    le_contm0->setText("IMAGE,/QRP,/P,/R");
    cb_contm1->setChecked(false);
    le_contm1->setText("LZ2HV,SP9HWY");
    cb_contm2->setChecked(false);
    le_contm2->setText("OZ2M,G8JVM");
    cb_contm3->setChecked(false);
    le_contm3->setText("KN,FN,JN,JO");
    cb_contm4->setChecked(false);
    le_contm4->setText("Bulgaria,Poland,Denmark,England");
    cb_pfx5->setChecked(false);
    le_pfx5->setText("LZ,SP,OZ,G");
    cb_contm6->setChecked(false);
    le_contm6->setText("");	
    cb_usefudpdectxt->setChecked(false);
    cb_gonoff->setChecked(false); 
    cb_hide_b4qso->setChecked(false);
    cb_usebtflonoff->setChecked(false);
    cb_filtered_answer->setChecked(false);

    SetFilter();
    RefrCountrys();
    RefrHidCountrys();
}
QStringList HvFilterDialog::GetLineParms(HvLeWithSpace *le)
{
    QStringList sl;
    sl.clear();
    QString txt = le->text();
    QStringList ltxt = txt.split(",");
    for (int i = 0; i<ltxt.count(); ++i)
    {
        QString stxt = ltxt.at(i);
        if (!stxt.isEmpty()) sl.append(stxt);
    }
    return sl;
}
void HvFilterDialog::SetFilter()
{
    QStringList lc;
    QStringList lc0;
    QStringList lc1;
    QStringList lc2;
    QStringList lc3;
    QStringList lc4;
    QStringList lc5;
    bool fh[10];
    fh[0]=false;
    fh[1]=false;
    fh[2]=false;
    fh[3]=false;
    fh[4]=false;
    fh[5]=false;
    fh[6]=false;
    fh[7]=false;
    fh[8]=false;
    fh[9]=false;
    lc.clear();
    lc0.clear();
    lc1.clear();
    lc2.clear();
    lc3.clear();
    lc4.clear();
    lc5.clear();

    if (cb_gonoff->isChecked())
    {        
        for (int i = 0; i < 7; ++i) fh[i]=cb_hacontt[i]->isChecked();        
        fh[7]=cb_usefudpdectxt->isChecked(); //usefudpdectxt
        fh[8]=cb_hide_b4qso->isChecked();
        fh[9]=cb_filtered_answer->isChecked();
        if (cb_cq->isChecked())   lc.append("CQ ");
        if (cb_cq73->isChecked()) lc << "CQ " << " RR73" << " 73";
        if (cb_ccq->isChecked())
        {
            QString txt = le_ccq->text();
            QStringList ltxt = txt.split(","); //qDebug()<<ltxt<<ltxt.count();
            for (int i = 0; i<ltxt.count(); ++i)
            {
                QString stxt = ltxt.at(i);
                if (!stxt.isEmpty())
                {
                    if (stxt=="73" || stxt=="RR73" || stxt=="RRR") lc.append(" "+stxt);
                    else lc.append(stxt+" ");
                }
            }
        }
        if      (cb_contm0->isChecked()) lc0 = GetLineParms(le_contm0);
        if      (cb_contm1->isChecked()) lc1 = GetLineParms(le_contm1);
        else if (cb_contm2->isChecked()) lc1 = GetLineParms(le_contm2);
        if 		(cb_contm3->isChecked()) lc2 = GetLineParms(le_contm3);
        if 		(cb_contm4->isChecked()) lc3 = GetLineParms((HvLeWithSpace*)le_contm4);
        if 		(cb_pfx5->isChecked()) 	 lc4 = GetLineParms(le_pfx5);
        if 		(cb_contm6->isChecked()) lc5 = GetLineParms((HvLeWithSpace*)le_contm6);
    }
    emit EmitSetFilter(lc,fh,lc0,lc1,lc2,lc3,lc4,lc5);
    
    bool f = false;
    for (int i = 0; i < 7; ++i) 
    {
    	if (fh[i])
    	{
    		f = true;
    		break;
   		}
   	}
 	if (fh[8] || !lc.isEmpty() || !lc0.isEmpty() || !lc1.isEmpty()|| !lc2.isEmpty()|| !lc3.isEmpty() ||
 		 !lc4.isEmpty() || !lc5.isEmpty()) f = true;
    RefreshPbSetOnOff(f);
}
void HvFilterDialog::closeEvent(QCloseEvent*)
{
    ApplyChFilter();
}
void HvFilterDialog::SetSettings_p(QString s, QCheckBox *cb, HvLeWithSpace *le,bool d)
{
    QStringList lts = s.split(",");
    if (lts.count()>0)
    {
        if (lts.at(0)=="1") cb->setChecked(true);
        int indx = s.indexOf(",");
        QString str = "";
        if (indx>-1) str = s.mid(indx, s.count()-indx);
        le->setText(CorrectSyntax(str,d));
    }
}
void HvFilterDialog::SetSettings(QString s)//,bool f
{
    QStringList lts = s.split("#");
    if (lts.count()>14)
    {      
    	//lts<<"0"<<"0"; //<-2.73 stop if old version 
        for (int i = 0; i < 7; ++i)
        {
        	if (lts.at(i)=="1") cb_hacontt[i]->setChecked(true);
       	}
        if (lts.at(7)=="1") cb_cq->setChecked(true);
        if (lts.at(8)=="1") cb_cq73->setChecked(true);
        QString str1 = lts.at(9);
        SetSettings_p(str1,cb_ccq,le_ccq,true);
        if (lts.at(10)=="1") cb_usefudpdectxt->setChecked(true);
        if (lts.at(11)=="1") cb_gonoff->setChecked(true);
        if (lts.at(12)=="1") cb_hide_b4qso->setChecked(true);
        if (lts.at(13)=="1") cb_usebtflonoff->setChecked(true);
        if (lts.at(14)=="1") cb_filtered_answer->setChecked(true);
		/*if (f) cb_filtered_answer->setHidden(false);
		else 
		{
			cb_filtered_answer->setChecked(false);
			cb_filtered_answer->setHidden(true);		
		}*/        
    }
    SetFilter();
    RefrCountrys();
    RefrHidCountrys();
}
void HvFilterDialog::SetSettings0(QString s)
{
    SetSettings_p(s,cb_contm0,le_contm0,true);
}
void HvFilterDialog::SetSettings1(QString s)
{
    SetSettings_p(s,cb_contm1,le_contm1,true);
}
void HvFilterDialog::SetSettings2(QString s)
{
    SetSettings_p(s,cb_contm2,le_contm2,true);
}
void HvFilterDialog::SetSettings3(QString s)
{
    SetSettings_p(s,cb_contm3,le_contm3,true);
}
void HvFilterDialog::SetSettings4(QString s)
{
    SetSettings_p(s,cb_contm4,(HvLeWithSpace*)le_contm4,false);
}
void HvFilterDialog::SetSettings5(QString s)
{
    SetSettings_p(s,cb_pfx5,le_pfx5,true);
}
void HvFilterDialog::SetSettings6(QString s)
{
    SetSettings_p(s,cb_contm6,(HvLeWithSpace*)le_contm6,false);
}
QString HvFilterDialog::GetSettings_p(QCheckBox *cb, HvLeWithSpace *le,bool d)
{
    QString res;
    res.append(QString("%1").arg(cb->isChecked())+",");
    res.append(CorrectSyntax(le->text(),d));
    if (res.endsWith(",")) res = res.mid(0,res.count()-1);
    return res.trimmed();
}
QString HvFilterDialog::GetSettings()
{
    QString res;    
    for (int i = 0; i < 7; ++i) res.append(QString("%1").arg(cb_hacontt[i]->isChecked())+"#");
    res.append(QString("%1").arg(cb_cq->isChecked())+"#");
    res.append(QString("%1").arg(cb_cq73->isChecked())+"#");
    res.append(GetSettings_p(cb_ccq,le_ccq,true));
    res.append("#");
    res.append(QString("%1").arg(cb_usefudpdectxt->isChecked()));
    res.append("#");
    res.append(QString("%1").arg(cb_gonoff->isChecked()));
    res.append("#");
    res.append(QString("%1").arg(cb_hide_b4qso->isChecked()));
    res.append("#");
    res.append(QString("%1").arg(cb_usebtflonoff->isChecked()));
    res.append("#");
    res.append(QString("%1").arg(cb_filtered_answer->isChecked()));
    return res.trimmed();
}
QString HvFilterDialog::GetSettings0()
{
    return GetSettings_p(cb_contm0,le_contm0,true);
}
QString HvFilterDialog::GetSettings1()
{
    return GetSettings_p(cb_contm1,le_contm1,true);
}
QString HvFilterDialog::GetSettings2()
{
    return GetSettings_p(cb_contm2,le_contm2,true);
}
QString HvFilterDialog::GetSettings3()
{
    return GetSettings_p(cb_contm3,le_contm3,true);
}
QString HvFilterDialog::GetSettings4()
{
    return GetSettings_p(cb_contm4,(HvLeWithSpace*)le_contm4,false);
}
QString HvFilterDialog::GetSettings5()
{
    return GetSettings_p(cb_pfx5,le_pfx5,true);
}
QString HvFilterDialog::GetSettings6()
{
    return GetSettings_p(cb_contm6,(HvLeWithSpace*)le_contm6,false);
}



