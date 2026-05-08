/* MSHV HvCustomW
 * Copyright 2017 Hrisimir Hristov, LZ2HV
 * May be used under the terms of the GNU General Public License (GPL)
 */
#include "hvcustomw.h"
#include <QHBoxLayout>
//#include <QtGui>

HvTxRxEqual::HvTxRxEqual(QString t0,QString t1,bool t,QWidget * parent)
        : QPushButton(parent)
{
	trx = t;
	setText(t0);
    setToolTip(t1);		
    setFixedHeight(21);//21  
    setMinimumWidth(24);//46
    setMaximumWidth(36);
}
void HvTxRxEqual::paintEvent(QPaintEvent *event)
{
	QPushButton::paintEvent(event);
    QPainter painter(this);
    painter.save();
    QPalette pl = palette();    
    int ch = height()/2;
    int cw = width(); 
    painter.setPen(QPen(pl.buttonText().color(),0));      
    if (trx)
    {    	
    	/*painter.fillRect(cw-4,ch-3,cw,8,pl.button());
    	painter.drawLine(cw-4,ch-1,cw,ch-1);*/
    	painter.fillRect(cw-4,ch-4,cw,9,pl.button());
    	painter.drawLine(cw-4,ch-2,cw,ch-2);    	
    	painter.drawLine(cw-4,ch+2,cw,ch+2);     	
    }
    else
    { 
    	/*painter.fillRect(0,ch-3,4,8,pl.button());
    	painter.drawLine(0,ch-1,3,ch-1);*/
    	painter.fillRect(0,ch-4,4,9,pl.button()); 
    	painter.drawLine(0,ch-2,3,ch-2);    	
    	painter.drawLine(0,ch+2,3,ch+2);     	
   	}
    painter.restore();
}

HvQueuedCallW::HvQueuedCallW(bool f,QWidget * parent )
        : QWidget(parent)
{
	dsty = f;
    f_haveq = false;
    //QSpacerItem *space = new QSpacerItem(1,1,QSizePolicy::Expanding);
    QPixmap pixmap23(":pic/clr_press.png");
    QPixmap pixmap24(":pic/clr_rep.png");
    bt_clr = new HvButton_Left2();
    bt_clr->SetupButton_hv(pixmap24, pixmap23, 0, 0,false);
    connect(bt_clr, SIGNAL(Release_Lift_Button_hv()), this, SLOT(ClrQueuedCallB()));
    bt_clr->setToolTip(tr("CLR Queue"));

    le_call = new QLineEdit();
    le_call->setAcceptDrops(false);//2.28
    le_call->setContextMenuPolicy(Qt::NoContextMenu);// no rithclick menu 
    //connect(le_call, SIGNAL(textChanged(QString)), this, SLOT(ResizeLineEdit()));

    f_last_from_queued = false;

	f_use_queue_cont = false;    
    le_call->setText(tr("Queue - N/A")); //le_call->setText(tr("No Queue"));
    le_call->setReadOnly(true);

    QHBoxLayout *H_B = new QHBoxLayout();
    H_B->setContentsMargins (1, 1, 3, 1);
    H_B->setSpacing(5);
    H_B->addWidget(le_call);
    H_B->addWidget(bt_clr);

    QFrame *Box_c = new QFrame();
    Box_c->setLayout(H_B);

    QHBoxLayout *H_l = new QHBoxLayout(this);
    H_l->setContentsMargins ( 0, 0, 0, 0);
    H_l->setSpacing(0);
    H_l->addWidget(Box_c);
    //H_l->addLayout(H_B);
    this->setLayout(H_l);

    SetBackColor(false);
    SetLineColor(false);    

    setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Fixed);
}
HvQueuedCallW::~HvQueuedCallW()
{}
void HvQueuedCallW::SetUseQueueCont(bool f)
{
	if (f)
	{
		le_call->setText(tr("No Queue"));
		SetLineColor(true);		
	}
	else 
	{
		ClrQueuedCallB();
		le_call->setText(tr("Queue - N/A"));
		SetLineColor(false);
	}
	f_use_queue_cont = f;
}
void HvQueuedCallW::SetFont(QFont f)
{
    le_call->setFont(f);
}
void HvQueuedCallW::SetQueuedCall(QString call,QString my_rpt,QString his_rpt,QString exc,QString sn,QString loc,QString freq)
{
    le_call->setText(call);
    s_my_rpt = my_rpt;
    s_his_rpt = his_rpt;
    s_exc = exc;
    s_sn = sn;
    s_loc = loc;
    s_freq = freq;
    SetBackColor(true);
    f_haveq = true;
}
QStringList HvQueuedCallW::GetQueuedCallData()
{
    QStringList l;
    l<<le_call->text()<<s_my_rpt<<s_his_rpt<<s_exc<<s_sn<<s_loc<<s_freq<<"";
    return l;
}
void HvQueuedCallW::ClrQueuedCallB()
{
    ClrQueuedCall(false);
    emit EmidButClrQueuedCall();
}
void HvQueuedCallW::ClrQueuedCall(bool f)
{
    f_last_from_queued = f;
    if (f_use_queue_cont) le_call->setText(tr("No Queue"));
    s_my_rpt = "";
    s_his_rpt = "";
    s_exc = "";
    s_sn  = "";
    s_loc = "";
    s_freq= "";
    SetBackColor(false);
    f_haveq = false;
}
void HvQueuedCallW::SetLineColor(bool f)
{
	if (f) 
    {
		if (dsty) le_call->setStyleSheet("QLineEdit{border:0px;selection-color:white;selection-background-color:black;}");
    	else le_call->setStyleSheet("QLineEdit{border:0px;selection-color:black;selection-background-color:white;}");	
   	}        
    else
    {
		if (dsty) le_call->setStyleSheet("QLineEdit{border:0px;color:rgb(110,110,110);selection-color:rgb(110,110,110);selection-background-color:black;}");
		else le_call->setStyleSheet("QLineEdit{border:0px;color:rgb(110,110,110);selection-color:rgb(110,110,110);selection-background-color:white;}");   	
   	}
}
void HvQueuedCallW::SetBackColor(bool f)
{
    if (f)
    {
    	if (dsty) setStyleSheet("QWidget{background-color:rgb(10,55,10);border:1px solid rgb(150,150,150);border-radius:3px;}");	
    	else setStyleSheet("QWidget{background-color:rgb(210,255,210);border:1px solid rgb(150,150,150);border-radius:3px;}");	
   	}        
    else
    {
    	if (dsty) setStyleSheet("QWidget{background-color:rgb(30,30,30);border:1px solid rgb(150,150,150);border-radius:3px;}");
        else setStyleSheet("QWidget{background-color:rgb(255,255,255);border:1px solid rgb(150,150,150);border-radius:3px;}");    	
   	}
}

#include <QValidator>
HvLeFreq::HvLeFreq(QWidget * parent )
        : QLineEdit(parent)
{
    setAcceptDrops(false);//2.28
    QRegExp rx("^[1-9][0-9.]*$");
    QValidator *validator = new QRegExpValidator(rx, this);
    setValidator(validator);
    setMaxLength(14);
    //le_freq->setFixedWidth(100);
    setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Fixed);
    //setFocusPolicy(Qt::NoFocus);
    setFocusPolicy(Qt::ClickFocus);
    //this->setFocus(Qt::MouseFocusReason);
    this->setContextMenuPolicy(Qt::NoContextMenu);
}
HvLeFreq::~HvLeFreq()
{}
void HvLeFreq::mousePressEvent(QMouseEvent *event)
{
    /*if(event->button()== Qt::RightButton)
    {
    	event->ignore();
    	return;
    }*/
    emit EmitMousePress();
    QLineEdit::mousePressEvent(event);
}
void HvLeFreq::keyPressEvent(QKeyEvent* event)
{
    //qDebug()<<event->key()<<Qt::Key_Enter;
    if (event->key()==Qt::Key_Return || event->key()==Qt::Key_Enter) //Key_Enter Nimlock Keyboard
        emit EmitEnterPress();
    QLineEdit::keyPressEvent(event);
}

#include <QTimer>
HvCatDispW::HvCatDispW(bool f,QWidget * parent )
        : QWidget(parent)
{
    //padding: 10px;
    /*"padding: 10px;"
    "border-style: solid;"
    "border-width: 3px;"
    "border-radius: 7px;"
    "min-width: %1px;"
    "max-width: %2px;"*/

    dsty = f;
    f_edit_mod = false;
    g_block_set_frq = false;

    //QSpacerItem *space = new QSpacerItem(1,1,QSizePolicy::Expanding);
    QPixmap pixmap23(":pic/def_press.png");
    QPixmap pixmap24(":pic/def_rep.png");
    bt_def_freq = new HvButton_Left2();
    bt_def_freq->SetupButton_hv(pixmap24, pixmap23, 0, 0,false);
    //connect(bt_up, SIGNAL(Press_Lift_Button_hv()), this, SLOT(up_rpt()));
    connect(bt_def_freq, SIGNAL(Release_Lift_Button_hv()), this, SLOT(SetDefFreqGlobal()));
    bt_def_freq->setToolTip(tr("Default Frequency"));//Return

    //QFont f_t = font();
    //f_t.setPointSize(12);
    //f_t.setBold(true);
    le_freq = new HvLeFreq();
    l_mode = new QLabel();
    //l_mode->setFont(f_t);

    connect(le_freq, SIGNAL(textChanged(QString)), this, SLOT(ResizeLineEdit()));


    l_mode->setText("USB");
    QFontMetrics fm = l_mode->fontMetrics();
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)//2.56
    w_mod_but = fm.horizontalAdvance(l_mode->text());       
#else
    w_mod_but = fm.width(l_mode->text());      
#endif    
    w_mod_but = w_mod_but + 15+5+3+24+8;//w_mod_but = w_mod_but + pixmap24.width()+5+3+24+8;// +8 neznam ot kade ?
    le_freq->setText("70.270.000");
    s_prev_freq_global = "70.270.000";

    QHBoxLayout *H_B = new QHBoxLayout();
    H_B->setContentsMargins(5, 0, 3, 0);//2.16 (5, 0, 3, 0)
    H_B->setSpacing(10);//2.16 12
    H_B->addWidget(l_mode);
    //H_B->setAlignment(l_mode,Qt::AlignLeft);
    //H_B->addSpacerItem(space);
    H_B->addWidget(le_freq);
    //H_B->setAlignment(le_freq,Qt::AlignRight);
    H_B->addWidget(bt_def_freq);

    //le_user_freq->setHidden(true);
    //H_B->setAlignment(l_freq,Qt::AlignRight);
    this->setMinimumWidth(100);
    //this->setMaximumWidth(150);
    //H_B->setAlignment(Qt::AlignHCenter);
    //this->setFixedWidth(120);
    //this->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum);

    QFrame *Box_c = new QFrame();
    //Box_c->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Fixed);
    //Box_c->setFrameShape(QFrame::WinPanel);
    //Box_c->setFrameShadow(QFrame::Raised);
    //Box_c->setFrameShape(QFrame::StyledPanel);
    //Box_c->setFrameShadow(QFrame::Plain);
    //Box_c->setLineWidth(4);
    //Box_c->setContentsMargins(3,3,3,3);
    //Box_c->setStyleSheet("QFrame{background-color :rgb(255, 0, 0);}");
    Box_c->setLayout(H_B);

    QHBoxLayout *H_l = new QHBoxLayout(this);
    H_l->setContentsMargins ( 0, 0, 0, 0);
    H_l->setSpacing(0);
    H_l->addWidget(Box_c);
    //H_l->addLayout(H_B);
    this->setLayout(H_l);


    //setStyleSheet("QWidget{background-color :rgb(210,255,210); border: 1px solid rgb(150,150,150); border-radius: 3px;}");
    SetEditAndBackColor(false);
    //setStyleSheet("QWidget{background-color :rgb(255,255,255); border: 1px solid rgb(150,150,150); border-radius: 3px;}");

    
    le_freq->setStyleSheet("QLineEdit{border:0px}");

    if (dsty) l_mode->setStyleSheet("QLabel{border:0px;color:rgb(160,160,255)}");
    else l_mode->setStyleSheet("QLabel{border:0px;color:rgb(0,0,100)}");


    setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Fixed);

    connect(le_freq, SIGNAL(EmitMousePress()), this, SLOT(LeFreqMousePressed()));
    connect(le_freq, SIGNAL(selectionChanged()), this, SLOT(LeFreqMousePressed()));
    connect(le_freq, SIGNAL(EmitEnterPress()), this, SLOT(SetDefFreqGlobal()));
    //connect(le_freq, SIGNAL(editingFinished()), this, SLOT(LeFreqMousePressed()));

    c_timer = 0;
    timer_edit = new QTimer();
    connect(timer_edit, SIGNAL(timeout()), this, SLOT(RefreshTimerEdit()));
}
HvCatDispW::~HvCatDispW()
{}
void HvCatDispW::SetFont(QFont f)
{
    QFont f_t = f;//9 in
    f_t.setPointSize(f.pointSize()+3);//f_t.setPointSize(12);
    f_t.setBold(true);
    le_freq->setFont(f_t);
    f_t.setPointSize(f.pointSize()+1);//f_t.setPointSize(10);
    l_mode->setFont(f_t);

    QFontMetrics fm = l_mode->fontMetrics();
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)//2.56
    w_mod_but = fm.horizontalAdvance(l_mode->text());       
#else
    w_mod_but = fm.width(l_mode->text());      
#endif    
    w_mod_but = w_mod_but + 15+5+3+24+8;
    ResizeLineEdit();
}
void HvCatDispW::SetTxActive(int i)//0=off TX, 1=normal TX, 2=Static TX
{
    if (i==1 || i==2)
    {
        g_block_set_frq = true;
        le_freq->setEnabled(false);
        if (i==2)
        {
        	if (dsty) le_freq->setStyleSheet("QLineEdit{border:0px;color:rgb(255,180,180)}");
            else le_freq->setStyleSheet("QLineEdit{border:0px;color:rgb(150,0,0)}");        	
       	}
        else
        {
        	if (dsty) le_freq->setStyleSheet("QLineEdit{border:0px;color:rgb(180,180,255)}");
            else le_freq->setStyleSheet("QLineEdit{border:0px;color:rgb(0,0,150)}");        	
       	}
    }
    else
    {
        le_freq->setEnabled(true);
        le_freq->setStyleSheet("QLineEdit{border:0px}");
        g_block_set_frq = false;
    }
}
//QTime ttt;
void HvCatDispW::SetDefFreqGlobal()
{
    if (g_block_set_frq) return;
    QString frq = "default";
    if (f_edit_mod)
    {
        frq = le_freq->text();
        //s_previous_freq = frq;
        frq.remove(".");
        //f_edit_mod = false;
        emit EmitSetDefFreqGlobal(2,frq);//0=mybe frq,no mod, 1=mybe frq,mod, 2=frq,mod, 3=frq,no mod //true only from button F
        //this->setFocus();
        le_freq->clearFocus();
        SetEditAndBackColor(1);//light red
    }
    else emit EmitSetDefFreqGlobal(2,frq);//0=mybe frq,no mod, 1=mybe frq,mod, 2=frq,mod, 3=frq,no mod //true only from button F
    c_timer = 5;
}
void HvCatDispW::SetFreq(QString frq)
{
    //frq.append(lfrq_mod.at(x));
    int k = frq.count();
    k -= 3;
    if (k>0)
        frq.insert(k,".");//Hz 1.44
    k -= 3;
    if (k>0)
        frq.insert(k,".");//KHz 1.44

    s_prev_freq_global = frq;
    if (f_edit_mod)
        return;

    le_freq->setText(frq);
}
void HvCatDispW::SetMode(QString s)
{
    if (s=="USB" || s=="DIGU") // || s=="R-U"
    {
        l_mode->setText(s);
        if (dsty) l_mode->setStyleSheet("QLabel{border:0px;color:rgb(160,160,255)}");
        else l_mode->setStyleSheet("QLabel{border:0px;color:rgb(0,0,100)}");
    }
    else
    {
        if (s=="WRONG_MODE") l_mode->setText("MOD");
        else l_mode->setText(s);
        if (dsty) l_mode->setStyleSheet("QLabel{border:0px;color:rgb(255,100,100)}");
        else l_mode->setStyleSheet("QLabel{border:0px;color:rgb(150,0,0)}");
    }
}
void HvCatDispW::ResizeLineEdit()
{
    QString str = le_freq->text();
    QFontMetrics fm = le_freq->fontMetrics();
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)//2.56
    int w = fm.horizontalAdvance(str);       
#else
    int w = fm.width(str);      
#endif    
    this->setFixedWidth(w_mod_but+w);
    c_timer = 0;
    //qDebug()<<w_mod_but<<w<<w_mod_but+w;
}
void HvCatDispW::RefreshTimerEdit()
{
    if (c_timer<10)//9=5000-2000ms 10=5500-2500ms
    {
        c_timer++;
        //qDebug()<<c_timer;
    }
    else
    {
        //qDebug()<<"Time="<<ttt.elapsed();
        timer_edit->stop();
        c_timer = 0;
        //if (f_edit_mod)
        le_freq->setText(s_prev_freq_global);
        f_edit_mod = false;
        //this->setFocus();
        le_freq->clearFocus();
        SetEditAndBackColor(0);//yelow
    }
}
void HvCatDispW::LeFreqMousePressed()
{
    f_edit_mod = true;
    SetEditAndBackColor(2);//red
    timer_edit->start(500);
    c_timer = 0;
    //ttt.start();
}
void HvCatDispW::SetEditAndBackColor(int id) // 2-red 1-light red 0-yelow
{
    if (dsty)
    {
        if (id==2)
            setStyleSheet("QWidget{background-color:rgb(55,10,10);border:1px solid rgb(150,150,150);border-radius:3px;}");
        else if (id==1)
            setStyleSheet("QWidget{background-color:rgb(55,25,25);border:1px solid rgb(150,150,150);border-radius:3px;}");
        else
            setStyleSheet("QWidget{background-color:rgb(10,55,10);border:1px solid rgb(150,150,150);border-radius:3px;}");
    }
    else
    {
        if (id==2)
            setStyleSheet("QWidget{background-color:rgb(255,210,210);border:1px solid rgb(150,150,150);border-radius:3px;}");
        else if (id==1)
            setStyleSheet("QWidget{background-color :rgb(255,225,225);border:1px solid rgb(150,150,150);border-radius:3px;}");
        else
            setStyleSheet("QWidget{background-color :rgb(210,255,210);border:1px solid rgb(150,150,150);border-radius:3px;}");
    }
}

