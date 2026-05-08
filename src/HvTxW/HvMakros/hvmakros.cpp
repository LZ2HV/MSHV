/* MSHV HvMakrIn
 * Copyright 2015 Hrisimir Hristov, LZ2HV
 * May be used under the terms of the GNU General Public License (GPL)
 */
#include "hvmakros.h"
#include "../../config_str_exc.h"

//#include <QtGui>

HvMakrIn::HvMakrIn(int ident, QWidget * parent )
        : QWidget(parent)
{
    //setFrameStyle(QFrame::Panel | QFrame::Raised);
    //setLineWidth(2);
    s_ident = ident;
    line_txt = new HvLeWithSpace;
    //line_txt->setFixedWidth(220);
    line_txt->setMaxLength(28);
    QLabel *l_his_call = new QLabel("Tx"+QString("%1").arg(s_ident+1));
    QHBoxLayout *H_h = new QHBoxLayout(this);
    H_h->setContentsMargins (1,1,1,1);
    H_h->setSpacing(4);
    H_h->addWidget(l_his_call);
    //H_h->setAlignment(l_his_call,Qt::AlignRight);
    H_h->addWidget(line_txt);
    setLayout(H_h);
}
HvMakrIn::~HvMakrIn()
{}
#define _POS_CONT_
#define _CONT_NAME_
#include "../../config_str_con.h"
HvMakros::HvMakros(QString AppP, bool f,QWidget *parent )
        : QWidget(parent)
{
    w_parent = parent;
    dsty = f;
    //this->setWindowTitle(APP_NAME + (QString)" Rig Control");
    allq65 = false;
    s_mode = 2;//HV important set to default mode fsk441
    f_block_gen_msg=true;

    //this->setMinimumWidth(480);
    this->setWindowTitle(tr("Macros"));
    //setWindowFlags(windowFlags() ^ Qt::WindowContextHelpButtonHint);
    //AppPath = AppP;
    //QGridLayout *l = new QGridLayout( this, 1, 1, 0, 0, "boardLayout");
    //setStyleSheet("border: 1px solid red");

    V_la= new QVBoxLayout();
    V_la->setContentsMargins(4,4,4,4);
    V_la->setSpacing(1);
    V_lb= new QVBoxLayout();
    V_lb->setContentsMargins(4,4,4,4);
    V_lb->setSpacing(1);

    rb_eu = new QRadioButton(tr("Region")+" 1");
    rb_eu->setChecked(true);
    rb_na = new QRadioButton(tr("Region")+" 2");
    rb_au = new QRadioButton(tr("Region")+" 3");
    connect(rb_eu, SIGNAL(toggled(bool)), this, SLOT(MacrChanged(bool)));
    connect(rb_na, SIGNAL(toggled(bool)), this, SLOT(MacrChanged(bool)));
    connect(rb_au, SIGNAL(toggled(bool)), this, SLOT(MacrChanged(bool)));
    QHBoxLayout *H_region = new QHBoxLayout();
    H_region->setContentsMargins(1,1,1,1);
    H_region->setSpacing(6);
    H_region->addWidget(rb_eu);
    H_region->addWidget(rb_na);
    H_region->addWidget(rb_au);
    H_region->setAlignment(Qt::AlignHCenter);
    QGroupBox *GB_region = new QGroupBox(tr("Macros By Region")+":");
    GB_region->setLayout(H_region);
    //GB_region->setContentsMargins(3,2,3,3);

    rb_rep = new QRadioButton(tr("Report"));
    rb_rep->setChecked(true);
    rb_drid = new QRadioButton(tr("Grid"));
    rb_contest = new QRadioButton(tr("RSQ And Serial Number"));
    connect(rb_rep, SIGNAL(toggled(bool)), this, SLOT(MacrChanged(bool)));
    connect(rb_drid, SIGNAL(toggled(bool)), this, SLOT(MacrChanged(bool)));
    connect(rb_contest, SIGNAL(toggled(bool)), this, SLOT(MacrChanged(bool)));

    //cb_msk144_contest = new QCheckBox("MSK FT8 NA VHF Contest");
    //connect(cb_msk144_contest, SIGNAL(toggled(bool)), this, SLOT(MacrChangedMsk144(bool)));

    QHBoxLayout *H_rgm = new QHBoxLayout();
    H_rgm->setContentsMargins(1,1,1,1);
    H_rgm->setSpacing(6);
    H_rgm->addWidget(rb_rep);
    H_rgm->addWidget(rb_drid);
    H_rgm->addWidget(rb_contest);
    //H_rgm->addWidget(cb_msk144_contest);
    H_rgm->setAlignment(Qt::AlignHCenter);
    //QGroupBox *GB_rgm = new QGroupBox("Macros By Report, Grid, RSQ And Serial Number");
    QGroupBox *GB_rgm = new QGroupBox(tr("Macros option for")+" JTMS,FSK,ISCAT,JT6M");
    GB_rgm->setLayout(H_rgm);
    //GB_rgm->setContentsMargins(3,2,3,3);

    rb_km = new QRadioButton(tr("Kilometers"));
    rb_km->setChecked(true);
    rb_mi = new QRadioButton(tr("Miles"));
    connect(rb_km, SIGNAL(toggled(bool)), this, SLOT(DistanceChanged(bool)));
    connect(rb_mi, SIGNAL(toggled(bool)), this, SLOT(DistanceChanged(bool)));
    QHBoxLayout *H_distance = new QHBoxLayout();
    H_distance->setContentsMargins ( 1, 1, 1, 1);
    H_distance->setSpacing(6);
    H_distance->addWidget(rb_km);
    H_distance->addWidget(rb_mi);
    H_distance->setAlignment(Qt::AlignHCenter);
    QGroupBox *GB_distance = new QGroupBox(tr("Distance unit")+":");
    GB_distance->setLayout(H_distance);

    QLabel *l_cn = new QLabel();
    l_cn->setText(tr("Activity Type")+":");
    cb_contests = new QComboBox();
    QStringList lst_cont;
    for (int i = 0; i<COUNT_CONTEST; ++i)
    {
        if (i==0) lst_cont <<"Standard";
        else if (i!=1) lst_cont << s_cont_name[pos_cont[i]];
    }
    cb_contests->addItems(lst_cont);
    cb_contests->setMaxVisibleItems(COUNT_CONTEST-1);
    cb_contests->setMinimumWidth(180);

    QLabel *l_exchfd = new QLabel("ARRL Field Day Exch:");
    le_exchfd = new HvLeWithSpace();
    le_exchfd->setAlignment(Qt::AlignCenter);
    //le_exchfd->setFixedWidth(80);
    le_exchfd->setContentsMargins(0,0,10,0);
    l_exchru = new QLabel("Roundup Exch:"); //RTTY Roundup Exch:
    le_exchru = new HvLeWithSpace();
    le_exchru->setAlignment(Qt::AlignCenter);
    //le_exchru->setFixedWidth(80);
    //rb_bstd->setChecked(true);
    cb_contests->setCurrentIndex(0);
    id_boption = 0;

    QLabel *l_trmn = new QLabel();
    //l_trmn->setText("For Multi-Two Category (Need Two Installed Copys) Transmitter Number:");
    l_trmn->setText(tr("Multi-Two Transmitter: (Requires Two Different Installed Copies Of Software)"));
    cb_cabrillo_trmN = new QComboBox();
    QStringList lst_trmn;
    lst_trmn <<"None"<<"Run 1"<<"Run 2";
    cb_cabrillo_trmN->addItems(lst_trmn);
    cb_cabrillo_trmN->setCurrentIndex(0);
    cb_cabrillo_trmN->setMinimumWidth(65);
    QHBoxLayout *H_cmod3 = new QHBoxLayout();
    H_cmod3->setContentsMargins(0,4,0,4);
    H_cmod3->setSpacing(4);
    H_cmod3->addWidget(l_trmn);
    H_cmod3->addWidget(cb_cabrillo_trmN);
    H_cmod3->setAlignment(Qt::AlignHCenter);

    QHBoxLayout *H_cmod1 = new QHBoxLayout();
    H_cmod1->setContentsMargins(0,0,0,0);
    H_cmod1->setSpacing(4);
    H_cmod1->addWidget(l_cn);
    H_cmod1->addWidget(cb_contests);
    /*H_cmod1->addWidget(rb_bstd);
    H_cmod1->addWidget(rb_bnac);
    H_cmod1->addWidget(rb_beuc);
    H_cmod1->addWidget(rb_bwdx);*/
    H_cmod1->setAlignment(Qt::AlignHCenter);
    QHBoxLayout *H_cmod2 = new QHBoxLayout();
    H_cmod2->setContentsMargins(2,6,2,0);
    H_cmod2->setSpacing(2);
    //H_cmod2->addWidget(rb_bafd);
    H_cmod2->addWidget(l_exchfd);
    H_cmod2->addWidget(le_exchfd);
    //H_cmod2->addWidget(rb_baru);
    H_cmod2->addWidget(l_exchru);
    H_cmod2->addWidget(le_exchru);
    H_cmod2->setAlignment(Qt::AlignHCenter);
    QVBoxLayout *V_cmod = new QVBoxLayout();
    V_cmod->setContentsMargins(2,2,2,2);
    V_cmod->setSpacing(1);
    V_cmod->addLayout(H_cmod1);
    V_cmod->addLayout(H_cmod2);
    V_cmod->addLayout(H_cmod3);

    QGroupBox *GB_cmod = new QGroupBox(tr("Macros option for")+" MSK,FT,Q65");
    GB_cmod->setLayout(V_cmod);
    /*connect(rb_bstd, SIGNAL(toggled(bool)), this, SLOT(BOptionChanged(bool)));
    connect(rb_bnac, SIGNAL(toggled(bool)), this, SLOT(BOptionChanged(bool)));
    connect(rb_beuc, SIGNAL(toggled(bool)), this, SLOT(BOptionChanged(bool)));
    connect(rb_bafd, SIGNAL(toggled(bool)), this, SLOT(BOptionChanged(bool)));
    connect(rb_baru, SIGNAL(toggled(bool)), this, SLOT(BOptionChanged(bool)));*/
    connect(cb_contests, SIGNAL(currentIndexChanged(int)), this, SLOT(CbContNameChanged(int)));
    connect(le_exchfd, SIGNAL(textChanged(QString)), this, SLOT(BExchangeChanged(QString)));
    connect(le_exchru, SIGNAL(textChanged(QString)), this, SLOT(BExchangeChanged(QString)));
    connect(cb_cabrillo_trmN, SIGNAL(currentIndexChanged(int)), this, SLOT(CbContTrmNChanged(int)));
    le_exchfd->setText("1D DX");//default
    le_exchru->setText("DX");//default

    QLabel *l_valid = new QLabel(tr("My call =%M  His call =%T  RST or RSQ =%R  "
                                    "4 characters locator =%G4\n6 characters locator =%G6  "
                                    "Random QRG =%QRG  Serial number =%N\n"
                                    "My suffix =%O  His suffix =%H  "
                                    "Separating numeral + my suffix =%SO\nSeparating numeral + his suffix =%SH"));
    l_valid->setAlignment(Qt::AlignCenter);

    QGroupBox *GB_vmac = new QGroupBox(tr("Macros")+":");
    QHBoxLayout *H_vmac = new QHBoxLayout();
    H_vmac->setContentsMargins(1,5,1,10);
    H_vmac->setSpacing(4);
    H_vmac->addWidget(l_valid);
    H_vmac->setAlignment(l_valid,Qt::AlignHCenter);
    GB_vmac->setLayout(H_vmac);

    QPushButton*b_gen_mess= new QPushButton(tr("GEN MESSAGE"));
    //b_gen_mess->setFixedHeight(20);
    connect(b_gen_mess, SIGNAL(clicked(bool)), this, SLOT(gen_mess()));

    //QLabel *lm_call = new QLabel("MY CALL:");
    le_mycall = new HvInLe("call", tr("MY CALL")+":",dsty);

    connect(le_mycall, SIGNAL(EmitSndCheck(QString)), this, SLOT(Check(QString)));
    le_mycall->setMaxLength(25);

    //QLabel *lm_loc = new QLabel("GRID LOCATOR:");
    le_loc = new HvInLe("locator", tr("GRID LOCATOR")+":",dsty);
    connect(le_loc, SIGNAL(EmitSndCheck(QString)), this, SLOT(Check(QString)));
    le_loc->setMaxLength(6);

    QPushButton*b_set_default_macr_a= new QPushButton(tr("SET DEFAULT MACROS"));
    b_set_default_macr_a->setFixedHeight(20);
    connect(b_set_default_macr_a, SIGNAL(clicked(bool)), this, SLOT(SetDefaultMacros_a()));
    QPushButton*b_set_default_macr_b= new QPushButton(tr("SET DEFAULT MACROS"));
    b_set_default_macr_b->setFixedHeight(20);
    connect(b_set_default_macr_b, SIGNAL(clicked(bool)), this, SLOT(SetDefaultMacros_b()));
    /*QPushButton*b_def_macros_g= new QPushButton("SET DEFAULT GRID MACROS");
    connect(b_def_macros_g, SIGNAL(released()), this, SLOT(SetDefaultMacrosGrid()));*/

    /*QHBoxLayout *H_h = new QHBoxLayout();
    H_h->setContentsMargins ( 1, 1, 1, 1);
    H_h->setSpacing(4);
    H_h->addWidget(le_mycall);
    QHBoxLayout *H_hloc = new QHBoxLayout();
    H_hloc->setContentsMargins ( 1, 1, 1, 1);
    H_hloc->setSpacing(4);
    H_hloc->addWidget(le_loc);*/
    QHBoxLayout *H_call_loc = new QHBoxLayout();
    H_call_loc->setContentsMargins(1,1,1,1);
    H_call_loc->setSpacing(16);
    H_call_loc->addWidget(le_mycall);
    H_call_loc->addWidget(le_loc);

    QVBoxLayout *V_l= new QVBoxLayout(this);
    V_l->setContentsMargins (4, 6, 4, 2);
    V_l->setSpacing(4);

    QHBoxLayout *H_dstreg = new QHBoxLayout();
    H_dstreg->setContentsMargins(0,0,0,0);
    H_dstreg->setSpacing(4);
    H_dstreg->addWidget(GB_distance);
    H_dstreg->addWidget(GB_region);

    //V_l->addWidget(GB_distance);
    //V_l->addWidget(GB_vmac);
    //V_l->addWidget(GB_region);
    V_l->addLayout(H_dstreg);
    V_l->addWidget(GB_rgm); 
	//V_l->addItem(new QSpacerItem(0,10, QSizePolicy::Expanding, QSizePolicy::Expanding));
    //V_l->setAlignment(GB_rgm,Qt::AlignBottom);
    V_l->addWidget(GB_cmod); //V_l->setAlignment(GB_cmod,Qt::AlignTop);
	//V_l->addItem(new QSpacerItem(0,10, QSizePolicy::Expanding, QSizePolicy::Expanding));
    V_l->addWidget(GB_vmac); //V_l->setAlignment(GB_vmac,Qt::AlignBottom);
	//V_l->addItem(new QSpacerItem(0,10, QSizePolicy::Expanding, QSizePolicy::Expanding));    
    //V_l->setAlignment(GB_vmac,Qt::AlignTop);
    V_l->addWidget(b_gen_mess); //V_l->setAlignment(b_gen_mess,Qt::AlignBottom);
    //V_l->addLayout(H_h); //V_l->setAlignment(H_h,Qt::AlignBottom);
    //V_l->addLayout(H_hloc); //V_l->setAlignment(H_hloc,Qt::AlignBottom);
    V_l->addLayout(H_call_loc);
    //V_l->setAlignment(Qt::AlignRight);

    //count_widg_plus = 0;//dopalnite widgeti broq ina4e garmi
    count_tx_widget = 7;

    for (int i = 0; i<count_tx_widget; i++)
    {
        HvMakrIn *TempLe = new HvMakrIn(i);

        //TempLe->b_tx->setText("Tx"+QString("%1").arg(i+1));
        V_la->addWidget(TempLe);
        //V_l->setAlignment(TempLe,Qt::AlignRight);

        //connect(TempHvTxIn, SIGNAL(EmitRbPress(int)), this, SLOT(RbPress(int)));
        //connect(TempHvTxIn, SIGNAL(EmitBReleased(int,QString)), this, SLOT(BReleased(int,QString)));
        //pressed ()toggled ( bool checked )
    }
    V_la->addWidget(b_set_default_macr_a);
    QGroupBox *GB_a = new QGroupBox(tr("Macros for")+" JTMS,FSK,ISCAT,JT6M");
    GB_a->setLayout(V_la);
    for (int i = 0; i<count_tx_widget; i++)
    {
        HvMakrIn *TempLe = new HvMakrIn(i);

        //TempLe->b_tx->setText("Tx"+QString("%1").arg(i+1));
        V_lb->addWidget(TempLe);
        //V_l->setAlignment(TempLe,Qt::AlignRight);

        //connect(TempHvTxIn, SIGNAL(EmitRbPress(int)), this, SLOT(RbPress(int)));
        //connect(TempHvTxIn, SIGNAL(EmitBReleased(int,QString)), this, SLOT(BReleased(int,QString)));
        //pressed ()toggled ( bool checked )
    }
    V_lb->addWidget(b_set_default_macr_b);
    QGroupBox *GB_b = new QGroupBox(tr("Macros for")+" MSK,FT,Q65,JT65");
    GB_b->setLayout(V_lb);

    QHBoxLayout *H_ab = new QHBoxLayout();
    H_ab->setContentsMargins(1,1,1,1);
    H_ab->setSpacing(5);
    H_ab->addWidget(GB_a);
    H_ab->addWidget(GB_b);
    V_l->addLayout(H_ab); 
    V_l->setAlignment(Qt::AlignTop);

    setLayout(V_l);
    //V_l->setAlignment(Qt::AlignTop);

    //sr_path = AppPath+"/settings/ms_macros";
    sr_path = AppP+"/settings/ms_macros";
    ReadSettings();

    f_block_gen_msg = false;

}
HvMakros::~HvMakros()
{
    // SaveSettings(); //2.35 error close and save
}
void HvMakros::TwTabClicked(int i)//2.68
{
    static uint8_t prev = 0;
    if (prev==0 && (i==1 || i==2)) gen_mess(); //qDebug()<<i;} // 1=NetW  2=RadFreq
    prev = i;
}
void HvMakros::SetClose()
{
	QString txtl = le_exchru->text();//2.74
    if (!isErrorRUExchDig(txtl))
    {
    	txtl = QString("%1").arg(txtl.toInt(),4,10,QChar('0')); 
    	le_exchru->setText(txtl);
   	}	
    SaveSettings();
    SetMacros();
}
void HvMakros::CheckAllowedModesActivity()//2.51
{
    if (s_mode==0 && (id_boption>3 && id_boption<13))
    {
        QMessageBox::warning(w_parent, "MSHV",
                             (tr("Activity Type will be changed to Standard\n"
                                 "If you are in mode MSK144 the allowed Activity Types are only:")+"\n"
                              "Standard, NA VHF Contest, EU VHF Contest, CQ WW VHF Contest"),
                             QMessageBox::Close);
        cb_contests->setCurrentIndex(0);
    }
    /*if (allq65 && id_boption>1)
    {
    	QMessageBox::warning(this, "MSHV",
                       (tr("Activity Type will be changed to Standard\n"
    				 "If you are in mode Q65 the allowed Activity Types is only:\n"
    				 "Standard")),
                       QMessageBox::Close);
    	cb_contests->setCurrentIndex(0);
    }*/
}
void HvMakros::CbContNameChanged(int i)
{
    if (i==0) id_boption = 0;
    else id_boption = pos_cont[i+1];//id_boption = i+1;

    //For MSK144 Allowed Modes Is Only "Standard, NA VHF Contest and EU VHF Contest and CQ WW VHF Contest"
    //QFont font = l_exchru->font();
    if (id_boption==5 || id_boption==6 || id_boption==7 || id_boption==8 || id_boption==9 ||
            id_boption==10 || id_boption==11 || id_boption==12)
    {
        l_exchru->setText(s_cont_name[id_boption]+" Exch:");
        //font.setWeight(QFont::Bold);
        //l_exchru->setFont(font);
    }
    else
    {
        l_exchru->setText("Roundup Exch:"); //RTTY Roundup Exch:
        //font.setWeight(QFont::Normal);
        //l_exchru->setFont(font);
    }

    if (!f_block_gen_msg) SetDefaultMacros_b();

    CheckAllowedModesActivity();//2.51
}
bool HvMakros::isErrorRUExchDig(QString s)
{
	bool f_ruerr = true;
    bool all_dig = true;
    for (int j = 0; j < s.count(); ++j)
    {
        if (s.at(j).isLetter())
        {
            all_dig = false;
            break;
        }
    }
    if (all_dig && s.toInt()>0 && s.toInt()<8000) f_ruerr = false;
    return f_ruerr;
}
void HvMakros::BExchangeChanged(QString)
{
    //FD 2B EMA
    //RU MA
    if (!f_block_gen_msg) SetDefaultMacros_b();

    bool f_fderr = true;
    QString fdt = le_exchfd->text();
    QStringList lfd = fdt.split(" ");
    if (lfd.count()==2)//le_in->setStyleSheet("QLineEdit {background-color :palette(Base);}");
    {
        QString cat = lfd.at(0);//1A-32F
        QString sec = lfd.at(1);//sec
        int icatl = (int)cat.at(cat.count()-1).toLatin1();
        if (icatl>=(int)'A' && icatl<=(int)'F')
        {
            int icat = cat.midRef(0,cat.count()-1).toInt(); //qDebug()<<cat<<sec<<icat<<sec<<cat.mid(0,cat.count()-1);
            //if (icat>0 && icat<33)//need to be 1-32  pack77 no problem ???
            if (icat>0 && icat<33)
            {
                for (int i = 0; i < NSEC; ++i)
                {
                    if (sec==csec_77[i].trimmed())
                    {
                        f_fderr = false;
                        break;
                    }
                }
            }
        }
    }
    if (f_fderr)
    {
        if (dsty) le_exchfd->setStyleSheet("QLineEdit{background-color:rgb(55,55,55);color:rgb(255,200,0)}");
        else le_exchfd->setStyleSheet("QLineEdit{background-color:rgb(255,255,200);color:rgb(200,0,0)}");
    }
    else le_exchfd->setStyleSheet("QLineEdit{background-color:palette(Base);}");

    bool f_ruerr = true;
    for (int i = 0; i < NUSCAN; ++i)
    {
        if (le_exchru->text()==cmult_77[i].trimmed() || le_exchru->text()=="DX")
        {
            f_ruerr = false;
            break;
        }
    }
    if (f_ruerr && le_exchru->text().count()==4)//2.74 270rc3 // && le_exchru->text().count()==4
    {
        f_ruerr = isErrorRUExchDig(le_exchru->text());        
    }
    if (f_ruerr)
    {
        if (dsty) le_exchru->setStyleSheet("QLineEdit{background-color:rgb(55,55,55);color:rgb(255,200,0)}");
        else le_exchru->setStyleSheet("QLineEdit{background-color:rgb(255,255,200);color:rgb(200,0,0)}");
    }
    else le_exchru->setStyleSheet("QLineEdit{background-color:palette(Base);}");
}
void HvMakros::SetFont(QFont f)
{
    le_mycall->SetFont(f);
    le_loc->SetFont(f);
    le_exchfd->setFont(f);
    le_exchru->setFont(f);
}
void HvMakros::ModeChanget(int mode)
{
    if (mode == 14 || mode == 15 || mode == 16 || mode == 17) allq65 = true;
    else allq65 = false;
    s_mode = mode;
    gen_mess();
    CheckAllowedModesActivity();//2.51
}
void HvMakros::SetDefaultMacros_a()
{
    MacrChanged(true);
}
void HvMakros::SetDefaultMacros_b()
{
    QStringList list;
    //if (id_boption == 0)       //old Standard
    //list <<"%T %M %G4"<<"%T %M %R"<<"%T %M R%R"<<"%T %M RR73"<<"%T %M 73"<<"CQ %M %G4"<<"CQ %QRG %M %G4";
    if (id_boption == 2 || id_boption == 5 || id_boption == 13)  //NA VHF Contest 0=noused CQ WW VHF Contest ARRL Inter. Digital Contest
        list <<"%T %M %G4"<<"%T %M %G4"<<"%T %M R %G4"<<"%T %M RR73"<<"%T %M 73"<<"CQ TEST %M %G4"<<"%M QSOB4"; //"CQ %QRG %M"
    else if (id_boption == 3) //2.39 new EU VHF Contest
        list <<"%T %M %G4"<<"%T %M %R%N %G6"<<"%T %M R %R%N %G6"<<"%T %M RR73"<<"%T %M 73"<<"CQ TEST %M %G4"<<"%M QSOB4";//"CQ %QRG %M"
    else if (id_boption == 4) //ARRL Field Day 0=noused
        list <<"%T %M "+le_exchfd->text()<<"%T %M "+le_exchfd->text()<<"%T %M R "+le_exchfd->text()<<"%T %M RR73"<<"%T %M 73"<<"CQ FD %M %G4"<<"%M QSOB4";//"CQ %QRG %M"
    else if (id_boption == 9 || id_boption == 10 || id_boption == 11 || id_boption == 12)  //ARRL RTTY Roundup  0=noused
    {
        QString cqq = "RU";
        if (id_boption == 10) cqq = "BU";
        if (id_boption == 11) cqq = "FT";
        if (id_boption == 12) cqq = "PDC";
        if (le_exchru->text()=="DX") list <<"%T %M %R %N"<<"%T %M %R %N"<<"%T %M R %R %N"<<"%T %M RR73"<<"%T %M 73"<<"CQ "+cqq+" %M %G4"<<"%M QSOB4";//"CQ %QRG %M"
        else
        {        	
        	QString txtl = le_exchru->text();
        	if (!isErrorRUExchDig(txtl)) txtl = QString("%1").arg(txtl.toInt(),4,10,QChar('0'));
        	list <<"%T %M %R "+txtl<<"%T %M %R "+txtl<<"%T %M R %R "+txtl<<"%T %M RR73"<<"%T %M 73"<<"CQ "+cqq+" %M %G4"<<"%M QSOB4";//"CQ %QRG %M"       	        	
            //list <<"%T %M %R "+le_exchru->text()<<"%T %M %R "+le_exchru->text()<<"%T %M R %R "+le_exchru->text()<<"%T %M RR73"<<"%T %M 73"<<"CQ "+cqq+" %M %G4"<<"%M QSOB4";//"CQ %QRG %M"        	
       	}
    }
    else if (id_boption == 6 || id_boption == 7 || id_boption == 8)   //WW Digi DX Contest FT8/4 DX Contest
        list <<"%T %M %G4"<<"%T %M %G4"<<"%T %M R %G4"<<"%T %M RR73"<<"%T %M 73"<<"CQ WW %M %G4"<<"%M QSOB4"; //"CQ %QRG %M"
    else if (id_boption == 14) //Pileup 2.72
        list <<"%T %M %G4"<<"%T %M %G4"<<"%T %M R %G4"<<"%T %M RR73"<<"%T %M 73"<<"CQ %M %G4"<<"%M QSOB4";
    else if (id_boption == 15) //NCCC Sprint 2.72
        list <<"%T %M %G4"<<"%T %M %G4"<<"%T %M R %G4"<<"%T %M RR73"<<"%T %M 73"<<"CQ NCCC %M %G4"<<"%M QSOB4";//or NS
    else if (id_boption == 16 || id_boption == 17) //ARRL Inter. EME Contest, FT Challenge
    {
    	QString cqq = "";
        if (id_boption == 17) cqq = "FTC";
		list <<"%T %M %G4"<<"%T %M %R"<<"%T %M R%R"<<"%T %M RR73"<<"%T %M 73"<<"CQ "+cqq+" %M %G4"<<"%M QSOB4";    	
   	}		  	       
    else list<<"%T %M %G4"<<"%T %M %R"<<"%T %M R%R"<<"%T %M RR73"<<"%T %M 73"<<"CQ %M %G4"<<"CQ %QRG %M %G4";//Standard
        
    for (int i = 0; i<count_tx_widget; i++)
    {
        HvMakrIn *TempHvTxIn = (HvMakrIn*)V_lb->itemAt(i)->widget();
        TempHvTxIn->line_txt->setText(list.at(i));
    }
    if (s_mode==0 || s_mode>6) gen_mess();//msk144 jt65 pi4 ft q65       
}
void HvMakros::DistanceChanged(bool)
{
    if (rb_km->isChecked()) emit EmitDistUnit(false);
    if (rb_mi->isChecked()) emit EmitDistUnit(true);
}
void HvMakros::MacrChanged(bool)
{
    if (!f_block_gen_msg)
    {
        if (rb_rep->isChecked()) SetDefaultMacrosRep();
        if (rb_drid->isChecked()) SetDefaultMacrosGrid();
        if (rb_contest->isChecked()) SetDefaultMacrosContest();
    }
}
void HvMakros::SetDefaultMacrosGrid()
{
    QStringList list;

    if (rb_eu->isChecked())
        list <<"%T %M"<<"%T %M %G4"<<"%T %M RR %G4"<<"RRRR RRRR %M"<<"73 %M %G4"<<"CQ %M"<<"CQ %QRG %M";//R1
    if (rb_na->isChecked())
        list <<"%T %M"<<"%T %M %G4"<<"RR %G4"<<"RRR"<<"73"<<"CQ %M"<<"CQ %QRG %M";//R2
    if (rb_au->isChecked())
        list <<"%T %M"<<"%T %M %G4"<<"RR %G4"<<"RRR"<<"73"<<"CQ %M"<<"CQ %QRG %M";//R3

    for (int i = 0; i<count_tx_widget; i++)
    {
        HvMakrIn *TempHvTxIn = (HvMakrIn*)V_la->itemAt(i)->widget();
        TempHvTxIn->line_txt->setText(list.at(i));
    }

    if (s_mode>0 && s_mode<7) gen_mess();//all other jtms.......jt6m       
}
void HvMakros::SetDefaultMacrosRep()
{
    QStringList list;

    if (rb_eu->isChecked())
        list <<"%T %M"<<"%T %M %R %R"<<"%T %M R%R R%R"<<"RRRR RRRR %M"<< "73 %M"<<"CQ %M"<<"CQ %QRG %M";//R1
    if (rb_na->isChecked())
        list <<"%T %M"<<"%T %M %R"<<"R%R"<<"RRR"<< "73"<<"CQ %M"<<"CQ %QRG %M";//R2
    if (rb_au->isChecked())
        list <<"%T %M"<<"%T %M %R"<<"R%R"<<"RRR"<< "73"<<"CQ %M"<<"CQ %QRG %M";//R3

    for (int i = 0; i<count_tx_widget; i++)
    {
        HvMakrIn *TempHvTxIn = (HvMakrIn*)V_la->itemAt(i)->widget();
        TempHvTxIn->line_txt->setText(list.at(i));
    }

    if (s_mode>0 && s_mode<7) gen_mess();//all other jtms.......jt6m
}
void HvMakros::SetDefaultMacrosContest()
{
    QStringList list;

    if (rb_eu->isChecked())
        list <<"%T %M"<<"%T %M %R %N %G6"<<"%T %M R%R %N %G6"<<"RRRR RRRR %M"<< "73 %M"<<"CQ TEST %M"<<"CQ TEST %QRG %M";//R1
    if (rb_na->isChecked())
        list <<"%T %M"<<"%T %M %R %N %G6"<<"%T %M R%R %N %G6"<<"RRRR RRRR %M"<< "73 %M"<<"CQ TEST %M"<<"CQ TEST %QRG %M";//R2
    if (rb_au->isChecked())
        list <<"%T %M"<<"%T %M %R %N %G6"<<"%T %M R%R %N %G6"<<"RRRR RRRR %M"<< "73 %M"<<"CQ TEST %M"<<"CQ TEST %QRG %M";//R3

    for (int i = 0; i<count_tx_widget; i++)
    {
        HvMakrIn *TempHvTxIn = (HvMakrIn*)V_la->itemAt(i)->widget();
        TempHvTxIn->line_txt->setText(list.at(i));
    }

    if (s_mode>0 && s_mode<7) gen_mess();//all other jtms.......jt6m        
}
void HvMakros::SendMacros_p() //2.65
{
    /*static QStringList _l_;
    static int _ib_ = -1;
    static QString _exch_ = "";
    static int _trmn_ = -1;*/
    QStringList list;
    list << le_mycall->getText();
    list << le_loc->getText();

    if (s_mode>0 && s_mode<7) //all other jtms.......jt6m
    {
        for (int i = 0; i<count_tx_widget; i++)
        {
            HvMakrIn *TempHvTxIn = (HvMakrIn*)V_la->itemAt(i)->widget();
            list << TempHvTxIn->line_txt->text();
        }
        if (rb_contest->isChecked()) emit EmitRptRsq(true);
        else emit EmitRptRsq(false);
    }
    else //msk jt65 pi4 ft q65
    {
        for (int i = 0; i<count_tx_widget; i++)
        {
            HvMakrIn *TempHvTxIn = (HvMakrIn*)V_lb->itemAt(i)->widget();
            list << TempHvTxIn->line_txt->text();
        }
        emit EmitRptRsq(false);
    }

    QString exch = le_exchfd->text()+"#"+le_exchru->text();
    /*if (_l_==list && _ib_==id_boption && _exch_==exch && _trmn_==cb_cabrillo_trmN->currentIndex()) return;
    _l_ = list;
    _ib_ = id_boption;
    _exch_ = exch;
    _trmn_ = cb_cabrillo_trmN->currentIndex();*/
    emit EmitMacros(list,id_boption,exch,QString("%1").arg(cb_cabrillo_trmN->currentIndex()));
}
void HvMakros::gen_mess()
{
    SendMacros_p();
    SaveSettings();//2.35 error close and save
}
void HvMakros::SetMacros()// first start and close event
{
    SendMacros_p();
    DistanceChanged(true);
}
void HvMakros::CbContTrmNChanged(int)
{
    gen_mess();
}
bool HvMakros::isFindId(QString id,QString line,QString &res)
{
    bool fres = false;
    QRegExp rx;
    rx.setPattern(id+"=\"?([^\"]*)\"?");
    if (rx.indexIn(line) != -1)
    {
        res = rx.cap(1);
        fres = true;
    }
    return fres;
}
void HvMakros::ReadSettings()
{
    //QString macr_tx,macrc_tx;
    const int c_st_id = 20;
    //dopalva se tuk v kraia
    const QString st_id[c_st_id]=
        {
            "def_dist_unit","macr_reg","macr_rep_grd","macr_rep_contest","macr_my_call","macr_my_loc",
            "macr_tx1","macr_tx2","macr_tx3","macr_tx4","macr_tx5","macr_tx6","macr_tx7",
            "mac_ctx1","mac_ctx2","mac_ctx3","mac_ctx4","mac_ctx5","mac_ctx6","mac_ctx7"
        };
    QString st_res[c_st_id];
    for (int i = 0; i < c_st_id; ++i) st_res[i]="";

    QFile file(sr_path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        le_mycall->SetText("CALL"); //LZ2HV ako ne pro4ete ms_makros
        le_loc->SetText("GRID");      //KN23SF
        SetDefaultMacrosRep();
        SetDefaultMacros_b();
        return;
    }
    //QTime ttt;
    //ttt.start();
    QTextStream in(&file);

    while (!in.atEnd())
    {
        QString line = in.readLine();
        //QRegExp rx;

        if (line=="[ms_macros]") //continue;
            line = in.readLine();//next line

        for (int i = 0; i < c_st_id; ++i)
        {
            if (isFindId(st_id[i],line,st_res[i]))
            {
                line = in.readLine();
                //qDebug()<<i<<"idfind="<<st_id[i]<<"id="<<st_res[i];
            }
            //else
            //qDebug()<<i<<"FALSE READ --------------------------------  idfind="<<st_id[i];
        }
    }
    file.close();
    //qDebug()<<"Time="<<ttt.elapsed();

    for (int i = 0; i<count_tx_widget; ++i)
    {
        if (!st_res[i+6].isEmpty())
        {
            HvMakrIn *TempHvTxIn = (HvMakrIn*)V_la->itemAt(i)->widget();
            TempHvTxIn->line_txt->setText(st_res[i+6]);
            //qDebug()<<i<<st_res[i+6];
        }
    }
    for (int i = 0; i<count_tx_widget; ++i)
    {
        if (!st_res[i+13].isEmpty())
        {
            HvMakrIn *TempHvTxIn = (HvMakrIn*)V_lb->itemAt(i)->widget();
            TempHvTxIn->line_txt->setText(st_res[i+13]);
            //qDebug()<<i<<st_res[i+13];
        }
    }

    if (!st_res[4].isEmpty())
        le_mycall->SetText(st_res[4]);
    else
        Check("call");
    if (!st_res[5].isEmpty())
        le_loc->SetText(st_res[5]);
    else
        Check("locator");

    if (!st_res[1].isEmpty())
    {
        if (st_res[1].toInt()==0)
            rb_eu->setChecked(true);
        if (st_res[1].toInt()==1)
            rb_na->setChecked(true);
        if (st_res[1].toInt()==2)
            rb_au->setChecked(true);
    }
    if (!st_res[2].isEmpty())
    {
        if (st_res[2].toInt()==0)
            rb_rep->setChecked(true);
        if (st_res[2].toInt()==1)
            rb_drid->setChecked(true);
        if (st_res[2].toInt()==2)
            rb_contest->setChecked(true);
    }
    if (!st_res[0].isEmpty())
    {
        if (st_res[0].toInt()==0)
            rb_km->setChecked(true);
        if (st_res[0].toInt()==1)
            rb_mi->setChecked(true);
    }

    if (!st_res[3].isEmpty())
    {
        QStringList ls = st_res[3].split("#");
        if (ls.count()>3)
        {
            int idc = ls.at(0).toInt(); //2.65
            if (idc==0) cb_contests->setCurrentIndex(0);
            else
            {
                for (int i = 1; i<COUNT_CONTEST-1; ++i)
                {
                    if (idc==pos_cont[i+1])
                    {
                        cb_contests->setCurrentIndex(i);
                        break;
                    }
                }
            }

            le_exchfd->setText(ls.at(1));
            le_exchru->setText(ls.at(2));

            if (ls.at(3)=="1") cb_cabrillo_trmN->setCurrentIndex(1);
            else if (ls.at(3)=="2") cb_cabrillo_trmN->setCurrentIndex(2);
            else cb_cabrillo_trmN->setCurrentIndex(0);
        }
    }
}
void HvMakros::SaveSettings()
{
    //qDebug()<<"Save-------------->";
    QFile file(sr_path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;
    QTextStream out(&file);

    out << "[ms_macros]" << "\n";

    if (rb_km->isChecked())
        out << "def_dist_unit=" << "0" << "\n";
    if (rb_mi->isChecked())
        out << "def_dist_unit=" << "1" << "\n";

    if (rb_eu->isChecked())
        out << "macr_reg=" << "0" << "\n";
    if (rb_na->isChecked())
        out << "macr_reg=" << "1" << "\n";
    if (rb_au->isChecked())
        out << "macr_reg=" << "2" << "\n";

    if (rb_rep->isChecked())
        out << "macr_rep_grd=" << "0" << "\n";
    if (rb_drid->isChecked())
        out << "macr_rep_grd=" << "1" << "\n";
    if (rb_contest->isChecked())
        out << "macr_rep_grd=" << "2" << "\n";

    QString macr_rep_c = QString("%1").arg(id_boption)+"#";
    macr_rep_c.append(le_exchfd->text()+"#");
    macr_rep_c.append(le_exchru->text()+"#");
    macr_rep_c.append(QString("%1").arg(cb_cabrillo_trmN->currentIndex()));
    out << "macr_rep_contest="+macr_rep_c<< "\n";

    out << "macr_my_call=" << le_mycall->getText() << "\n";
    out << "macr_my_loc=" << le_loc->getText() << "\n";

    for (int i = 0; i<count_tx_widget; i++)
    {
        HvMakrIn *TempHvTxIn = (HvMakrIn*)V_la->itemAt(i)->widget();
        out << "macr_tx"+QString("%1").arg(i+1)+"=" << TempHvTxIn->line_txt->text() << "\n";
    }
    for (int i = 0; i<count_tx_widget; i++)
    {
        HvMakrIn *TempHvTxIn = (HvMakrIn*)V_lb->itemAt(i)->widget();
        out << "mac_ctx"+QString("%1").arg(i+1)+"=" << TempHvTxIn->line_txt->text() << "\n";
    }

    file.close();
}
void HvMakros::Check(QString s_type)
{
    if (s_type == "call")
    {
        //if (le_mycall->getText().count()>1)
        if (THvQthLoc.isValidCallsign(le_mycall->getText()))
        {
            le_mycall->setError(false);
            le_mycall->setErrorColorLe(false);
        }
        else
        {
            le_mycall->setError(true);
            le_mycall->setErrorColorLe(true);
        }

    }
    if (s_type == "locator")
    {
        if (!le_loc->getText().isEmpty() && THvQthLoc.isValidLocator(le_loc->getText()))
        {
            le_loc->setError(false);
            le_loc->setErrorColorLe(false);
        }
        else
        {
            le_loc->setError(true);
            le_loc->setErrorColorLe(true);
        }
    }
}



