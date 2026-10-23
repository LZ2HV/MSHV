/* MSHV Spin Box Widgets
 * Copyright 2015 Hrisimir Hristov, LZ2HV
 * May be used under the terms of the GNU General Public License (GPL)
 */
#include "hvspinbox.h"
//#include <QtGui>

HvSpinBoxDt::HvSpinBoxDt(bool indsty,QWidget * parent)
        : QWidget(parent)
{
    sb_dt = new QDoubleSpinBox();//sb_sn->SetMask("9999");
    s_indsty=indsty;
    s_val=0;
    val0=0.0;
    sb_dt->setRange(-2.0,2.5);
    sb_dt->setSingleStep(0.1);
    sb_dt->setAlignment(Qt::AlignRight);
    sb_dt->setDecimals(1);
    QPixmap pixmap23(":pic/clr_press.png");
    QPixmap pixmap24(":pic/clr_rep.png");
    bt_clr = new HvButton_Left2();
    bt_clr->SetupButton_hv(pixmap24, pixmap23, 0, 0,false);
    connect(bt_clr,SIGNAL(Release_Lift_Button_hv()),this,SLOT(ClrDT()));
    bt_clr->setToolTip(tr("CLR DT"));
    QHBoxLayout *H_s = new QHBoxLayout(this);
    H_s->setContentsMargins(0,0,0,0);
    H_s->setSpacing(2);//
    H_s->addWidget(sb_dt);
    H_s->addWidget(bt_clr);
    this->setLayout(H_s);//yelow 255,255,200    //236,233,216   //215,252,255
    sb_dt->setPrefix("DT ");
    sb_dt->setSuffix(" s");
    sb_dt->findChild<QLineEdit*>()->setReadOnly(true);
    sb_dt->setContextMenuPolicy(Qt::NoContextMenu);
    if (indsty) setStyleSheet("QDoubleSpinBox{selection-color:white;selection-background-color:rgb(30,30,30);}");
    else setStyleSheet("QDoubleSpinBox{selection-color:black;selection-background-color:white;}");  
    connect(sb_dt,SIGNAL(valueChanged(double)),this,SLOT(SetValueChanged(double)));
    tim = new QTimer();
    connect(tim,SIGNAL(timeout()),this,SLOT(Refresh()));
}
HvSpinBoxDt::~HvSpinBoxDt()
{}
void HvSpinBoxDt::Refresh()
{
	static bool f_0 = false;	
	{
		if (!f_0)
		{
			f_0=true;
			if (s_indsty) setStyleSheet("QDoubleSpinBox{selection-color:white;selection-background-color:rgb(150,0,0);background-color:rgb(150,0,0);}");
    		else setStyleSheet("QDoubleSpinBox{selection-color:black;selection-background-color:rgb(255,170,170);background-color:rgb(255,170,170);}");						
		}
		else
		{
			f_0=false;
			if (s_indsty) setStyleSheet("QDoubleSpinBox{selection-color:white;selection-background-color:rgb(30,30,30);}");
    		else setStyleSheet("QDoubleSpinBox{selection-color:black;selection-background-color:white;}");			
		}	
	}
}
void HvSpinBoxDt::SetEmitValueChanged_p(int val)	
{
	if (val!=0) 
	{
		tim->start(1000); //Refresh();		
	}
	else
	{
		tim->stop();
		if (s_indsty) setStyleSheet("QDoubleSpinBox{selection-color:white;selection-background-color:rgb(30,30,30);}");
    	else setStyleSheet("QDoubleSpinBox{selection-color:black;selection-background-color:white;}"); 			
	}
	if(val!=0) val*=-1;
	emit EmitValueChanged(val); //qDebug()<<val;		
}
void HvSpinBoxDt::SetValueChanged(double d)
{
	if 		(d > val0+0.05) s_val+=100;
	else if (d < val0-0.05) s_val-=100;
	else return;
	val0=d;	
	if (s_val==0) sb_dt->setValue(0.0);//if (s_val!=0) tim->start(1000);	
	SetEmitValueChanged_p(s_val);
}
void HvSpinBoxDt::ClrDT()
{
	s_val=0;
	val0=0.0;
	sb_dt->setValue(0.0);
	SetEmitValueChanged_p(0);	
}
void HvSpinBoxDt::SetMode(int mode)
{
    if (mode==11 || mode==13 || mode==18) SetEmitValueChanged_p(s_val);
    else SetEmitValueChanged_p(0);
}
void HvSpinBoxDt::SetFont(QFont f)
{
    sb_dt->setFont(f);
}
void HvSpinBoxDt::SetHidden(bool f)
{
    sb_dt->setHidden(f);
    bt_clr->setHidden(f);
}
////////////////////////////////////////////////////////////////////////////////////////////
HvSpinBox::HvSpinBox(QWidget * parent )
        : QSpinBox(parent)
{}
HvSpinBox::~HvSpinBox()
{}
void HvSpinBox::keyPressEvent(QKeyEvent* event)
{
    //Qt::Key_NumberSign-># Qt::Key_Bar->| Qt::Key_Semicolon->;
    if (event->modifiers() == Qt::ControlModifier || event->modifiers() == Qt::AltModifier)
    {
        QWidget::keyPressEvent(event);
        return;
    }
    QSpinBox::keyPressEvent(event);
}
////////////////////////////////////////////////////////////////////////////////////////////
HvSpinBoxSn::HvSpinBoxSn(QString name,bool indsty,QWidget * parent)
        : QWidget(parent)
{
    l_name = new QLabel(name);
    sb_sn = new HvSpinBox();
    //sb_sn->SetMask("9999");
    sb_sn->setRange(1,7999);
    sb_sn->setSingleStep(1);
    sb_sn->setAlignment(Qt::AlignRight);
    //sb_sn->setFixedWidth(70);
    QHBoxLayout *H_s = new QHBoxLayout(this);
    H_s->setContentsMargins (0, 0, 0, 0);
    H_s->setSpacing(2);//
    H_s->addWidget(l_name);
    H_s->addWidget(sb_sn);
    this->setLayout(H_s);
    //yelow 255,255,200    //236,233,216   //215,252,255
    if (indsty) setStyleSheet("QSpinBox{selection-color:white;selection-background-color:rgb(36,33,16);}");
    else setStyleSheet("QSpinBox{selection-color:black;selection-background-color:rgb(236,233,216);}");

    connect(sb_sn, SIGNAL(valueChanged(int)), this, SIGNAL(EmitValueChanged(int)));
}
HvSpinBoxSn::~HvSpinBoxSn()
{}
void HvSpinBoxSn::SetRange(int i)
{
    sb_sn->setRange(1,i);
}
void HvSpinBoxSn::SetFont(QFont f)
{
    l_name->setFont(f);
    sb_sn->setFont(f);
}
int HvSpinBoxSn::Value()
{
    return sb_sn->value();
}
void HvSpinBoxSn::SetValue(int i)
{
    sb_sn->setValue(i);
}
void HvSpinBoxSn::SetHidden(bool f)
{
    l_name->setHidden(f);
    sb_sn->setHidden(f);
}

HvSpinBoxDf::HvSpinBoxDf(bool indsty,QWidget * parent )
        : QSpinBox(parent)
{
	//ft_df1500 = true;
    f_no_emit = false;
    setSingleStep(1);
    //s_value = 0;
    s_mode = 2; // fsk441
    s_dftolerance[0] = 150; //msk144
    s_dftolerance[1] = 400;
    s_dftolerance[2] = 400;
    s_dftolerance[3] = 400;
    s_dftolerance[4] = 400;
    s_dftolerance[5] = 400;
    s_dftolerance[6] = 200;//jt6m
    s_dftolerance[7] = 600;//jt65a
    s_dftolerance[8] = 600;//jt65b
    s_dftolerance[9] = 600;//jt65c
    s_dftolerance[10] = 400;//pi4
    s_dftolerance[11] = 1500;//ft8 fictive
    s_dftolerance[12] = 150;//msk144ms
    s_dftolerance[13] = 1500;//ft4 fictive
    s_dftolerance[14] = 1000;//q65a
    s_dftolerance[15] = 1000;//q65b
    s_dftolerance[16] = 1000;//q65c
    s_dftolerance[17] = 1000;//q65d
    s_dftolerance[18] = 1500;//ft2 fictive
    //for (int i = 0; i < COUNT_MODE; ++i)

    setValue(400);

    connect(this, SIGNAL(valueChanged(int)), this, SLOT(Slot_valueChanged(int)));

    setSuffix("  Hz");
    findChild<QLineEdit*>()->setReadOnly(true);
    setContextMenuPolicy(Qt::NoContextMenu);
    if (indsty) setStyleSheet("QSpinBox{selection-color:white;selection-background-color:rgb(30,30,30);}");
    else setStyleSheet("QSpinBox{selection-color:black;selection-background-color:white;}");
}
HvSpinBoxDf::~HvSpinBoxDf()
{}
void HvSpinBoxDf::SetDfAllModes(QString s)
{
    QStringList ls=s.split("#");

    for (int i=0; i<COUNT_MODE; i++)
    {
        QString tstr = ModeStr(i)+"=";
        for (int j=0; j<ls.count(); j++)
        {
            if (ls[j].contains(tstr))
            {
                ls[j].remove(tstr);
                //if (!ls[j].isEmpty() && (ls[j].toInt()>9 && ls[j].toInt()<601))
                if (!ls[j].isEmpty() && (ls[j].toInt()>9 && ls[j].toInt()<1501))//1.78
                {
                    s_dftolerance[i] = ls[j].toInt();
                }
                break;
            }
        }
    }
}
void HvSpinBoxDf::SetValue(int i)
{
    s_dftolerance[s_mode] = i;
    setValue(s_dftolerance[s_mode]); //setValue(i); //1.37 <- moze i taka samo
}
/*void HvSpinBoxDf::SetFtDf1500(bool f)
{
	ft_df1500 = f;	
	if (s_mode==11 || s_mode==13)// ft8 ft4
	{
		if (f) setPrefix("DF Tol def=1500 +/-  ");
		else setPrefix("DF Tol def=1000 +/-  ");		
	}
	//else if (s_mode == 14 || s_mode == 15 || s_mode == 16 || s_mode == 17)
	//setPrefix("DF Tol def=1000 +/-  ");
	//SetMode(s_mode);
	//qDebug()<<"flsg="<<f;
}*/
void HvSpinBoxDf::SetMode(int i)
{
    setRange(1,5000);//importent unset range 1.47
    s_mode = i;
    if (s_mode==7 || s_mode==8 || s_mode==9) // jt65abc
    {
        setPrefix("DF Tol def=600 +/-  ");
        if (s_dftolerance[s_mode]<=50)
        {
            s_dftolerance[s_mode]=600;
            setValue(s_dftolerance[s_mode]);
        }
        else setValue(s_dftolerance[s_mode]);
        setRange(10,600);
    }
    else if (s_mode==10) // pi4
    {
        setPrefix("DF Tol def=400 +/-  ");
        if (s_dftolerance[s_mode]==100)
        {
            s_dftolerance[s_mode]=400;
            setValue(s_dftolerance[s_mode]);
        }
        else setValue(s_dftolerance[s_mode]);
        setRange(100,400);
    }
    else if (s_mode==11) // ft8
    {
    	/*if (ft_df1500) setPrefix("DF Tol def=1500 +/-  ");
        else setPrefix("DF Tol def=1000 +/-  ");*/
        setPrefix("DF Tol def=1500 +/-  ");
        if (s_dftolerance[s_mode]==50)
        {
        	/*if (ft_df1500) s_dftolerance[s_mode]=1500;
            else s_dftolerance[s_mode]=1000;*/
            s_dftolerance[s_mode]=1500;
            setValue(s_dftolerance[s_mode]);
        }
        else setValue(s_dftolerance[s_mode]);
        setRange(50,1500);
    }
    else if (s_mode==13) // ft4
    {
    	/*if (ft_df1500) setPrefix("DF Tol def=1500 +/-  ");
        else setPrefix("DF Tol def=1000 +/-  ");*/
        setPrefix("DF Tol def=1500 +/-  ");
        if (s_dftolerance[s_mode]==100)
        {
        	/*if (ft_df1500) s_dftolerance[s_mode]=1500;
            else s_dftolerance[s_mode]=1000;*/
            s_dftolerance[s_mode]=1500;
            setValue(s_dftolerance[s_mode]);
        }
        else setValue(s_dftolerance[s_mode]);
        setRange(100,1500);
    }
    else if (s_mode==18) // ft2
    {
    	/*if (ft_df1500) setPrefix("DF Tol def=1500 +/-  ");
        else setPrefix("DF Tol def=1000 +/-  ");*/
        setPrefix("DF Tol def=1500 +/-  ");
        if (s_dftolerance[s_mode]==100)
        {
        	/*if (ft_df1500) s_dftolerance[s_mode]=1500;
            else s_dftolerance[s_mode]=1000;*/
            s_dftolerance[s_mode]=1500;
            setValue(s_dftolerance[s_mode]);
        }
        else setValue(s_dftolerance[s_mode]);
        setRange(100,1500);
    }
    else if (s_mode == 14 || s_mode == 15 || s_mode == 16 || s_mode == 17) // q65
    {
    	//if (ft_df1500) setPrefix("DF Tol def=1500 +/-  ");
        //else setPrefix("DF Tol def=1000 +/-  ");
        setPrefix("DF Tol def=1000 +/-  ");
        if (s_dftolerance[s_mode]<=50)//if min to default
        {
        	//if (ft_df1500) s_dftolerance[s_mode]=1500;
            //else s_dftolerance[s_mode]=1000;
            s_dftolerance[s_mode]=1000;
            setValue(s_dftolerance[s_mode]);
        }
        else setValue(s_dftolerance[s_mode]);
        //setRange(50,1500);
        setRange(50,1000);
    }
    else
    {
        if (s_mode==0 || s_mode==12) setPrefix("DF Tol def=150 +/-  ");//msk144 msk144ms
        else if (s_mode==6) setPrefix("DF Tol def=200 +/-  ");//jt6m
        else setPrefix("DF Tol def=400 +/-  ");
        setValue(s_dftolerance[s_mode]);
        setRange(10,400);
    }
}
void HvSpinBoxDf::ExpandShrinkDf(bool f)//2.05
{
    //qDebug()<<s_dftolerance[s_mode]<<minimum()<<maximum();
    if (f)
    {
        if (s_dftolerance[s_mode]>=maximum()) return;
        Slot_valueChanged(1 + s_dftolerance[s_mode]);
    }
    else
    {
        if (s_dftolerance[s_mode]<=minimum()) return;
        Slot_valueChanged(s_dftolerance[s_mode] - 1);
    }
}
void HvSpinBoxDf::Slot_valueChanged(int val)
{
    if (f_no_emit)
    {
        f_no_emit = false;
        return;
    }

    int steps =  val - s_dftolerance[s_mode];

    if (steps == 1)
    {
        f_no_emit = true;
        switch (s_dftolerance[s_mode])
        {
        case 10:
            setValue(20);
            break;
        case 20:
            setValue(50);
            break;
        case 50:
            setValue(100);
            break;
        case 100:
            setValue(150);
            break;
        case 150:
            setValue(200);
            break;
        case 200:
            setValue(250);
            break;
        case 250:
            setValue(300);
            break;
        case 300:
            setValue(350);
            break;
        case 350:
            setValue(400);
            break;
        case 400:
            setValue(450);
            break;
        case 450:
            setValue(500);
            break;
        case 500:
            setValue(550);
            break;
        case 550:
            setValue(600);
            break;

            /// FT8///
        case 600:
            setValue(800);
            break;
        case 800:
            setValue(1000);
            break;
        case 1000:
            setValue(1500);
            break;

            /*case 600:
                 setValue(1000);
                 break;
            case 1000:
                 setValue(1500);
                 break;*/
        }
    }
    else if (steps == -1)
    {
        f_no_emit = true;
        switch (s_dftolerance[s_mode])
        {
        case 20:
            setValue(10);
            break;
        case 50:
            setValue(20);
            break;
        case 100:
            setValue(50);
            break;
        case 150:
            setValue(100);
            break;
        case 200:
            setValue(150);
            break;
        case 250:
            setValue(200);
            break;
        case 300:
            setValue(250);
            break;
        case 350:
            setValue(300);
            break;
        case 400:
            setValue(350);
            break;
        case 450:
            setValue(400);
            break;
        case 500:
            setValue(450);
            break;
        case 550:
            setValue(500);
            break;
        case 600:
            setValue(550);
            break;

            /// FT8
        case 800:
            setValue(600);
            break;
        case 1000:
            setValue(800);
            break;
        case 1500:
            setValue(1000);
            break;

            /*case 1000:
                setValue(600);
                break;
            case 1500:
                setValue(1000);
                break;*/
        }
    }
    s_dftolerance[s_mode] = value();
    emit EmitValueChanged(s_dftolerance[s_mode]);
    //qDebug()<<"EmitValue="<<s_dftolerance[s_mode];
}
void HvSpinBoxDf::keyPressEvent(QKeyEvent* event)
{
    //Qt::Key_NumberSign-># Qt::Key_Bar->| Qt::Key_Semicolon->;
    if (event->modifiers() == Qt::ControlModifier || event->modifiers() == Qt::AltModifier)
    {
        QWidget::keyPressEvent(event);
        return;
    }
    QSpinBox::keyPressEvent(event);
}

/////////////////////////////////////////////////////////////////////////
HvSpinBox4per30s::HvSpinBox4per30s(bool indsty,QWidget * parent )
        : QSpinBox(parent)
{
    f_no_emit = false;
    setSingleStep(1);
    //s_value = 0;
    s_mode = 2; // fsk441
    s_pt_all_modes[0] = 30;//msk144  2.67 form 15 to 30
    s_pt_all_modes[1] = 30;//jtms
    s_pt_all_modes[2] = 30;//fsk441
    s_pt_all_modes[3] = 30;//fsk315
    s_pt_all_modes[4] = 30;//iskat-a
    s_pt_all_modes[5] = 30;//iskat-b
    s_pt_all_modes[6] = 30;//jt6m
    s_pt_all_modes[7] = 60;//jt65a
    s_pt_all_modes[8] = 60;//jt65b
    s_pt_all_modes[9] = 60;//jt65c
    s_pt_all_modes[10] = 60;//pi4
    s_pt_all_modes[11] = 15;//ft8
    s_pt_all_modes[12] = 30;//msk144ms
    s_pt_all_modes[13] = 7;//ft4 7.5s
    s_pt_all_modes[14] = 30;//q65a
    s_pt_all_modes[15] = 30;//q65b
    s_pt_all_modes[16] = 30;//q65c
    s_pt_all_modes[17] = 30;//q65d
    s_pt_all_modes[18] = 3;//ft4 3.75s
    //for (int i = 0; i < COUNT_MODE; ++i)

    setValue(30);

    connect(this, SIGNAL(valueChanged(int)), this, SLOT(Slot_valueChanged(int)));
    //qDebug()<< value();

    setPrefix("TX/RX ");
    setSuffix("  s");
    findChild<QLineEdit*>()->setReadOnly(true);
    setContextMenuPolicy(Qt::NoContextMenu);
    if (indsty) setStyleSheet("QSpinBox{selection-color:white;selection-background-color:rgb(30,30,30);}");
    else setStyleSheet("QSpinBox{selection-color:black;selection-background-color:white;}");
}
HvSpinBox4per30s::~HvSpinBox4per30s()
{}
void HvSpinBox4per30s::SetPiriodTimeAllModes(QString s)
{
    QStringList ls=s.split("#");

    for (int i=0; i<COUNT_MODE; i++)
    {
        QString tstr = ModeStr(i)+"=";
        for (int j=0; j<ls.count(); j++)
        {
            if (ls[j].contains(tstr))
            {
                ls[j].remove(tstr);
                if (!ls[j].isEmpty() && (ls[j]=="3" || ls[j]=="5" || ls[j]=="7" || ls[j]=="10" || ls[j]=="15" ||                	
                	 ls[j]=="30" || ls[j]=="60" || ls[j]=="120" || ls[j]=="300"))// only valid values hv
                {
                    s_pt_all_modes[i] = ls[j].toInt();
                    //qDebug()<<"44444 SB_PeriodTime"<<s_pt_all_modes[i]<<i;
                }
                break;
            }
        }
    }
}
void HvSpinBox4per30s::SetMode(int i)
{
    s_mode = i;
    if (s_mode==7 || s_mode==8 || s_mode==9  || s_mode==10) // jt65abc pi4
    {
        setRange(5,120);//2.53  inportent open from min to max val
        setEnabled(false);
        setValue(60);
    }
    else if (s_mode==11) // ft8
    {
        setRange(15,15);//1.62= no problem because is 15s static
        setEnabled(false);
        setValue(15);
    }
    else if (s_mode==13) // ft4
    {
        setRange(7,7);//2.20 new ft4  problem because is 15s static
        setEnabled(false);
        setValue(7);
    }
    else if (s_mode==18) // ft2
    {
        setRange(3,3);//new ft2  problem because is 15s static
        setEnabled(false);
        setValue(3);
    }
    else if (s_mode == 14 || s_mode == 15 || s_mode == 16 || s_mode == 17) // q65
    {
        setRange(5,120);//2.53  inportent open from min to max val
        setEnabled(true);
        if (value()==30) emit EmitValueChanged(s_pt_all_modes[s_mode]);//2.53 except for start 30 but def value is 30
        setValue(s_pt_all_modes[s_mode]);
        setRange(15,120);//2.53= vazno da e tuk
    }    
    else
    {
        setRange(5,120);//2.53  inportent open from min to max val
        setEnabled(true);
        setValue(s_pt_all_modes[s_mode]);
        setRange(5,30);//2.53== vazno da e tuk
        //qDebug()<<"2 SB_PeriodTime"<<s_pt_all_modes[s_mode]<<s_mode;
    }
    if (value()==7) setSuffix(".5s");
    else if (value()==3) setSuffix(".8s");
    else setSuffix("  s");
}
void HvSpinBox4per30s::Slot_valueChanged(int val)
{
    if (f_no_emit)
    {
        f_no_emit = false;
        return;
    }

    int steps =  val - s_pt_all_modes[s_mode];

    if (steps == 1)
    {
        f_no_emit = true;
        switch (s_pt_all_modes[s_mode])
        {
        case 3:
            //f_no_emit = false;// if step 1 f_no_emit = false;
            setValue(5); //2.20 new ft4
            break;
        case 5:
            //f_no_emit = false;// if step 1 f_no_emit = false;
            setValue(7); //2.20 new ft4
            break;
        case 7:
            setValue(10);
            break;
        case 10:
            setValue(15);
            break;
        case 15:
            setValue(30);
            break;
        case 30:
            setValue(60);
            break;
        case 60:
            setValue(120);
            break; 
        case 120:
            setValue(300);
            break;   
        }
    }
    else if (steps == -1)
    {
        f_no_emit = true;
        switch (s_pt_all_modes[s_mode])
        {
        case 5:
            //f_no_emit = false;// if step 1 f_no_emit = false;
            setValue(3);
            break;
        case 7:
            //f_no_emit = false;// if step 1 f_no_emit = false;
            setValue(5);
            break;
        case 10:
            setValue(7);//2.20 new ft4
            break;
        case 15:
            setValue(10);
            break;
        case 30:
            setValue(15);
            break;
        case 60:
            setValue(30);
            break;
        case 120:
            setValue(60);
            break;    
        case 300:
            setValue(120);
            break;      
        }
    }
    s_pt_all_modes[s_mode] = value();
    float ptf = s_pt_all_modes[s_mode];//ft4 problem
    if (ptf==7.0) ptf = 7.5;//
    if (ptf==3.0) ptf = 3.75;//3.75=ft2
    emit EmitValueChanged(ptf);    
    if (value()==7) setSuffix(".5s");
    else if (value()==3) setSuffix(".8s");
    else setSuffix("  s");
}
float HvSpinBox4per30s::get_period_time()//1.56=
{
	float ptf = s_pt_all_modes[s_mode];//ft4 problem
    if (ptf==7.0) ptf = 7.5;
    if (ptf==3.0) ptf = 3.75;//3.75=ft2
    return ptf;
}
void HvSpinBox4per30s::keyPressEvent(QKeyEvent* event)
{
    //Qt::Key_NumberSign-># Qt::Key_Bar->| Qt::Key_Semicolon->;
    if (event->modifiers() == Qt::ControlModifier || event->modifiers() == Qt::AltModifier)
    {
        QWidget::keyPressEvent(event);
        return;
    }
    QSpinBox::keyPressEvent(event);
}


