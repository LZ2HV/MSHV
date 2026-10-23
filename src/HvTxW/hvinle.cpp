/* MSHV Hv LeEdit Widgets
 * Copyright 2015 Hrisimir Hristov, LZ2HV
 * May be used under the terms of the GNU General Public License (GPL)
 */

#include "hvinle.h"
#include "config_rpt_all.h"
#include "../HvMsPlayer/libsound/HvGenMsk/config_rpt_msk40.h"
#include "../config_str_color.h"

//#include <QtGui>

HvLeWithSpace::HvLeWithSpace(QWidget * parent )
        : QLineEdit(parent)
{
    setAcceptDrops(false); //2.28
    connect(this, SIGNAL(textChanged(QString)), this, SLOT(TextChanged(QString)));
}
HvLeWithSpace::~HvLeWithSpace()
{}
void HvLeWithSpace::keyPressEvent(QKeyEvent* event)
{

    if ((event->key() == Qt::Key_C || event->key() == Qt::Key_V) && event->modifiers() == Qt::ControlModifier)
    {
        QLineEdit::keyPressEvent(event);
        return;
    }

    //    Qt::Key_NumberSign-># Qt::Key_Bar->| Qt::Key_Semicolon->;
    if (event->key() == Qt::Key_NumberSign
            || event->key() == Qt::Key_Bar || event->key() == Qt::Key_Semicolon
            || event->modifiers() == Qt::ControlModifier || event->modifiers() == Qt::AltModifier)
    {
        QWidget::keyPressEvent(event);
        return;
    }

    QLineEdit::keyPressEvent(event);
}
void HvLeWithSpace::TextChanged(QString str)
{
    int c_pos = cursorPosition();
    setText(str.toUpper());
    setCursorPosition(c_pos);
}

///////////////////////////////////////////////////////////

HvLeNoSpace::HvLeNoSpace(QWidget * parent )
        : QLineEdit(parent)
{
    setAcceptDrops(false); //2.28
    connect(this, SIGNAL(textChanged(QString)), this, SLOT(TextChanged(QString)));
    //setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Fixed);
}

HvLeNoSpace::~HvLeNoSpace()
{}

void HvLeNoSpace::mousePressEvent(QMouseEvent *event)
{
    // kogato e s LeHisLoc->SetMask() kursura da otiva nakraia na texta
    QLineEdit::mousePressEvent(event);
    //QChar s = QChar::fromAscii(text().at(cursorPosition()).toAscii());
    //int i = text().at(cursorPosition()).toLatin1();
    if (!text().at(cursorPosition()).isPrint())//stana HV
        setCursorPosition(text().count());
    // qDebug()<<"t="<<s<<i;
}

void HvLeNoSpace::keyPressEvent(QKeyEvent* event)
{

    if ((event->key() == Qt::Key_C || event->key() == Qt::Key_V) && event->modifiers() == Qt::ControlModifier)
    {
        QLineEdit::keyPressEvent(event);
        return;
    }

    //Qt::Key_NumberSign-># Qt::Key_Bar->| Qt::Key_Semicolon->;
    if (event->key() == Qt::Key_Space || event->key() == Qt::Key_NumberSign
            || event->key() == Qt::Key_Bar || event->key() == Qt::Key_Semicolon
            || event->modifiers() == Qt::ControlModifier || event->modifiers() == Qt::AltModifier)
    {
        QWidget::keyPressEvent(event);
        return;
    }

    if (event->key() == Qt::Key_Return)//Qt::Key_Enter
        emit EmitEnter();

    QLineEdit::keyPressEvent(event);
}
void HvLeNoSpace::TextChanged(QString str)
{
    int c_pos = cursorPosition();
    setText(str.toUpper());
    setCursorPosition(c_pos);
}
////////////////////////////////////////////
HvInLe::HvInLe(QString type, QString name, bool f, QWidget * parent )
        : QWidget(parent)
{
	dsty = f;
    f_error = true;
    s_type = type;
    s_name = name;
    l_n = new QLabel();
    l_n->setText(name);
    
    if (dsty) l_n->setStyleSheet("QLabel{color:rgb(255,255,255)}");
    else l_n->setStyleSheet("QLabel{color:"+ColorStr_[0]+"}");
    
    le_in = new HvLeNoSpace();
    le_in->setContentsMargins(2,0,0,0);
    //le_in->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
    //le_in->setFixedHeight(20);
    /*l_error = new QLabel();
    l_error->setMinimumWidth(0);
    l_error->setText("");*/

    H_in = new QHBoxLayout(this);
    H_in->setContentsMargins( 0, 0, 0, 0);
    H_in->setSpacing(0);
    H_in->addWidget(l_n);
    //H_in->setAlignment(l_n,Qt::AlignLeft);
    //H_in->addWidget(l_error);
    H_in->addWidget(le_in);
    //H_in->setAlignment(Qt::AlignHCenter);
    this->setLayout(H_in);


    QFontMetrics fm(this->font());
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)//2.56
    this->setMinimumWidth(fm.horizontalAdvance(name+"   "));       
#else
    this->setMinimumWidth(fm.width(name+"   "));      
#endif    
    //setMaximumWidth(460);

    /*QVBoxLayout *V_in = new QVBoxLayout(this);
    V_in->setContentsMargins( 1, 1, 1, 1);
    V_in->setSpacing(1);
    V_in->addLayout(H_in);
    V_in->addWidget(le_in);
    this->setLayout(V_in);*/

    connect(le_in, SIGNAL(textChanged(QString)), this, SLOT(SndCheck_s(QString)));
    connect(le_in, SIGNAL(EmitEnter()), this, SIGNAL(EmitEntered()));
}
HvInLe::~HvInLe()
{}
void HvInLe::SetFont(QFont f)
{
    l_n->setFont(f);
    le_in->setFont(f);
}
void HvInLe::SetValidatorHv(QValidator *v)
{
    le_in->setValidator(v);
}
void HvInLe::SetTitle(QString s)
{
    s_name = s;
    l_n->setText(s_name);
}
void HvInLe::SetFocus()
{
    le_in->setFocus();
    //if (le_in->text().at(le_in->cursorPosition()).toLatin1() == 0 )
    if (le_in->text().at(le_in->cursorPosition()).isPrint())
        le_in->setCursorPosition(le_in->text().count());
    //focusNextChild();
}
void HvInLe::setFixedWidthLine(int w)
{
    le_in->setFixedWidth(w);
}
void HvInLe::setMaxLength(int l)
{
    le_in->setMaxLength(l);
}
void HvInLe::SndCheck_s(QString s)
{
    emit EmitSndCheck(s_type);
    emit EmitTextChanged(s.toUpper());//for DetectTextInMsg
}
void HvInLe::SetMask(QString mask)
{
    le_in->setInputMask(mask);
}
void HvInLe::SetText(QString str)
{

    le_in->setText(str);
    //SndCheck(s_type);
}
void HvInLe::setReadOnly(bool flag)
{
    le_in->setDisabled(flag);
}
void HvInLe::setError(bool flag)
{
    if (flag)
    {
        l_n->setText(s_name);
        if (dsty) l_n->setStyleSheet("QLabel{color:rgb(255,200,0)}");
        else l_n->setStyleSheet("QLabel{color:"+ColorStr_[1]+"}");
    }
    else
    {
        l_n->setText(s_name);
        if (dsty) l_n->setStyleSheet("QLabel{color:rgb(255,255,255)}");
        else l_n->setStyleSheet("QLabel{color:"+ColorStr_[0]+"}");
    }
}
void HvInLe::setErrorColorLe(bool f)
{
    if (!f)
    {
		//if (dsty)
        le_in->setStyleSheet("QLineEdit{background-color:palette(Base);}");
        f_error = false;
    }
    else
    {
    	if (dsty) le_in->setStyleSheet("QLineEdit{background-color:rgb(55,55,55);color:rgb(255,200,0)}");
        else le_in->setStyleSheet("QLineEdit{background-color:"+ColorStr_[3]+";color:"+ColorStr_[1]+"}");
        f_error = true;
    }
}
////////////////////////////////////////////////////////////////////////
HvRptLe::HvRptLe(QString type, QString name, bool f,QWidget * parent )
        : QWidget(parent)
{
    //rpt EU VHF Contest -> 52 to 59
    //rpt ARRL RTTY Roundup -> 529 to 599
    dsty = f;
    is_real_rpt = false;
    s_cont_type = 0;
    s_rpt_rsq = false;
    s_sh_rpt = false;
    allq65 = false;
    s_mode = 2;////HV important set to default mode fsk441
    s_rep_pos = 0;
    f_error = true;
    s_type = type;
    s_name = name;
    l_n = new QLabel(name);
    //l_n->setText(name);
    
    if (dsty) l_n->setStyleSheet("QLabel{color:rgb(255,255,255)}");
    else l_n->setStyleSheet("QLabel{color:"+ColorStr_[0]+"}");
    
    //l_n->setContentsMargins(0,0,0,2);
    //l_n->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
    //l_n->setFixedWidth(55);
    le_in = new HvLeNoSpace();
    le_in->setContentsMargins(2,0,0,0);
    //le_in->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Fixed);
    //le_in->setFixedWidth(36);//1.41 be6e->32 36forOOO
    //le_in->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
    le_in->setMinimumWidth(37);//34
    le_in->setMaximumWidth(54);//54
    //l_error = new QLabel();
    //l_error->setMinimumWidth(0);
    //l_error->setText("");
    //l_error->setFixedSize(1,1);

    //bt_up = new HvButton_Left2();
    QPixmap pixmap23,pixmap24;
    if (dsty)
    {
    	pixmap23 = QPixmap(":pic/up_press_b.png");
    	pixmap24 = QPixmap(":pic/up_rep_b.png");    	
   	}
    else
    {
    	pixmap23 = QPixmap(":pic/up_press.png");
    	pixmap24 = QPixmap(":pic/up_rep.png");    	
   	}   
    bt_up = new HvButton_Left2();
    bt_up->SetupButton_hv(pixmap24, pixmap23, 0, 0,true);
    connect(bt_up, SIGNAL(Press_Lift_Button_hv()), this, SLOT(up_rpt()));
    connect(bt_up, SIGNAL(Release_Lift_Button_hv()), this, SLOT(stop_inc()));
    //bt_down = new HvButton_Left2();

    QPixmap pixmap25,pixmap26;
    if (dsty)
    {
    	pixmap25 = QPixmap(":pic/down_press_b.png");
    	pixmap26 = QPixmap(":pic/down_rep_b.png");     	
   	}
    else
    {
    	pixmap25 = QPixmap(":pic/down_press.png");
    	pixmap26 = QPixmap(":pic/down_rep.png");    	
   	}        
    bt_down = new HvButton_Left2();
    bt_down->SetupButton_hv(pixmap26, pixmap25, 0, 0,true);
    connect(bt_down, SIGNAL(Press_Lift_Button_hv()), this, SLOT(down_rpt()));
    connect(bt_down, SIGNAL(Release_Lift_Button_hv()), this, SLOT(stop_inc()));
    mid_line = new QWidget();
    mid_line->setFixedSize(15,1);
    if (dsty) mid_line->setStyleSheet("QWidget{background-color:rgb(70,70,70)}");
    else mid_line->setStyleSheet("QWidget{background-color:rgb(170,170,170)}");

    QVBoxLayout *vl_but = new QVBoxLayout();
    vl_but->setContentsMargins(0,0,0,0);
    vl_but->setSpacing(0);
    vl_but->addWidget(bt_up);
    vl_but->addWidget(mid_line);
    vl_but->addWidget(bt_down);
    vl_but->setAlignment(Qt::AlignCenter);//2.54
    bt_up->setVisible(false);
    mid_line->setVisible(false);
    bt_down->setVisible(false);


    H_in = new QHBoxLayout(this);
    H_in->setContentsMargins(0,0,0,0);
    H_in->setSpacing(0);
    H_in->addWidget(l_n);
    //H_in->setAlignment(l_n,Qt::AlignLeft);
    //H_in->addWidget(l_error);
    H_in->addWidget(le_in);
    H_in->addLayout(vl_but);
    //H_in->setAlignment(vl_but,Qt::AlignVCenter);
    this->setLayout(H_in);


    //QFontMetrics fm(this->font());
    //this->setMinimumWidth(fm.width(name)+le_in->width()+pixmap25.width());

    /*QVBoxLayout *V_in = new QVBoxLayout(this);
    V_in->setContentsMargins( 1, 1, 1, 1);
    V_in->setSpacing(1);
    V_in->addLayout(H_in);
    V_in->addWidget(le_in);
    this->setLayout(V_in);*/

    connect(le_in, SIGNAL(textChanged(QString)), this, SLOT(SndCheck_s(QString)));
    connect(le_in, SIGNAL(EmitEnter()), this, SIGNAL(EmitEntered()));

    timer_inc = new QTimer();
    connect(timer_inc, SIGNAL(timeout()), this, SLOT(timer_inc_rpt()));
    inc_up_down = 0;
    wait_tacts = 0;

    setMaxLength(3);
    SetMask("#99");
    le_in->setText("26");//SetText("26");

    //vl_but->removeWidget(bt_up);
    //vl_but->removeWidget(bt_down);
    //bt_up->setVisible(false);
    //bt_down->setVisible(false);
}

HvRptLe::~HvRptLe()
{}
void HvRptLe::SetFont(QFont f)
{
    l_n->setFont(f);
    le_in->setFont(f);
}
void HvRptLe::SetSpinCtrl(bool f)
{
    bt_up->setVisible(f);
    bt_down->setVisible(f);
    mid_line->setVisible(f);
}
QString HvRptLe::format_rpt_tx(QString s)
{
    QString ss;
    if ((s_mode == 0 || s_mode == 11 || s_mode == 13 || s_mode == 18 || allq65) && !s_sh_rpt)//msk144 ft8 ft4
    {
        if (s_cont_type==3)//52-59 for Contest v2
            ss=s;
        else if (s_cont_type==5)//529-599
            ss=s;
        else
        {
            if (s[0].toLatin1()!='-' && s[0].toLatin1()!='+')
            {
                s="+"+s;
            }
            if (s.toInt()==0)
                s.replace('-','+');
            ss=s[0];
            ss.append(QString("%1").arg(abs(s.toInt()),2,10,QChar('0')));
        }
    }
    else
    {
        if (s_mode == 0 || s_mode == 11 || s_mode == 13 || s_mode == 18 || allq65)//msk144 ft8 ft4
        {
            if (s_cont_type==3)//52-59 for Contest v2
                ss=s;
            else if (s_cont_type==5)//529-599
                ss=s;
            else
            {
                if (s_mode == 0 && s_sh_rpt)//only for msk40 other case '*** bad message ***'
                {//"-03 ","+00 ","+03 ","+06 ","+10 ","+13 ","+16 "
                    int ir = s.toInt();
                    if (ir <=-2)
                        ss = rpt_msk40[0];//.mid(0,3);
                    else if (ir>-2 && ir<=1)
                        ss = rpt_msk40[1];//.mid(0,3);
                    else if (ir>1 && ir<=4)
                        ss = rpt_msk40[2];//.mid(0,3);
                    else if (ir>4 && ir<=8)
                        ss = rpt_msk40[3];//.mid(0,3);
                    else if (ir>8 && ir<=11)
                        ss = rpt_msk40[4];//.mid(0,3);
                    else if (ir>11 && ir<=14)
                        ss = rpt_msk40[5];//.mid(0,3);
                    else if (ir>14)
                        ss = rpt_msk40[6];//.mid(0,3);
                }
                else
                {
                    if (s[0].toLatin1()!='-' && s[0].toLatin1()!='+')
                    {
                        s="+"+s;
                    }
                    if (s.toInt()==0)
                        s.replace('-','+');
                    ss=s[0];
                    ss.append(QString("%1").arg(abs(s.toInt()),2,10,QChar('0')));
                }
            }
        }
        else
            ss=s;
    }
    return ss;
}
QString HvRptLe::format_rpt(QString s)
{
    QString ss;
    if ((s_mode == 0 || s_mode == 11 || s_mode == 13 || s_mode == 18 || allq65) && !s_sh_rpt)//msk144 ft8 ft4
    {
        if (s_cont_type==3)////52-59 for Contest v2
            ss=s;
        else if (s_cont_type==5)//529-599
            ss=s;
        else
        {
            if (s[0].toLatin1()!='-' && s[0].toLatin1()!='+')
            {
                s="+"+s;
            }
            if (s.toInt()==0)
                s.replace('-','+');
            ss=s[0];
            ss.append(QString("%1").arg(abs(s.toInt()),2,10,QChar('0')));
        }
    }
    else
    {
        if (s_mode == 0 || s_mode == 11 || s_mode == 13 || s_mode == 18 || allq65)//msk144 ft8 ft4
        {
            if (s_cont_type==3)////52-59 for Contest v2
                ss=s;
            else if (s_cont_type==5)//529-599
                ss=s;
            else
            {
                if (s[0].toLatin1()!='-' && s[0].toLatin1()!='+')
                {
                    s="+"+s;
                }
                if (s.toInt()==0)
                    s.replace('-','+');
                ss=s[0];
                ss.append(QString("%1").arg(abs(s.toInt()),2,10,QChar('0')));
            }
        }
        else
            ss=s;
    }
    return ss;
}
void HvRptLe::SetRptRsqSettings(QString title, int mode, bool rpt_rsq, bool sh_rpt,int i_cont_type)
{
    // id=2 rpt EU VHF Contest -> 52 to 59
    // id=4 rpt ARRL RTTY Roundup -> 529 to 599
    is_real_rpt = false;
    s_cont_type = i_cont_type;
    s_rpt_rsq = rpt_rsq;
    if (mode == 14 || mode == 15 || mode == 16 || mode == 17) allq65 = true;
    else allq65 = false;
    s_mode = mode;
    s_sh_rpt = sh_rpt;
    le_in->setReadOnly(false);
    SetMask("#99");

    if (!rpt_rsq)
    {
        if (s_mode == 0)//MSK144 MSK40
        {
            if (s_sh_rpt)//MSK40
            {
                if (s_cont_type==3)//for Contest v2
                {
                    s_rep_pos = 0;
                    le_in->setText("59");
                }
                else if (s_cont_type==5)
                {
                    s_rep_pos = 0;
                    le_in->setText("599");
                }
                else
                {
                    s_rep_pos = 1;
                    le_in->setText(rpt_msk40[s_rep_pos]);
                    le_in->setReadOnly(true);
                }

                SetTitle(title+" RPT :");
                //s_rep_pos = 1;
                //le_in->setText(rpt_msk40[s_rep_pos]);
                SetSpinCtrl(true);
                //le_in->setReadOnly(true);
            }
            else //MSK144
            {
                if (s_cont_type==3)//for Contest v2
                    le_in->setText("59");
                else if (s_cont_type==5)
                    le_in->setText("599");
                else
                    le_in->setText("+00");

                SetTitle(title+" RPT :");
                s_rep_pos = 0;
                //le_in->setText("+00");
                SetSpinCtrl(true);
            }
        }
        if (s_mode == 11 || s_mode == 13 || s_mode == 18 || allq65)//ft8 ft4 q65
        {
            if (s_cont_type==3)//for Contest v2
                le_in->setText("59");
            else if (s_cont_type==5)
                le_in->setText("599");
            else
                le_in->setText("+00");

            SetTitle(title+" RPT :");
            s_rep_pos = 0;
            //le_in->setText("+00");
            SetSpinCtrl(true);
        }
        if (s_mode == 12)//msk144ms
        {
            SetTitle(title+" RPT :");
            s_rep_pos = 0;
            le_in->setText(rpt_ms_p[s_rep_pos]);
            //le_in->setText("26");//SetText("26");
            SetSpinCtrl(true);
        }
        if (s_mode == 1 || s_mode == 2 || s_mode == 3 || s_mode == 6)//JTMS FSK441 FSK315 JT6M
        {
            SetTitle(title+" RPT :");
            s_rep_pos = 0;
            le_in->setText(rpt_ms_p[s_rep_pos]);
            //le_in->setText("26");//SetText("26");
            SetSpinCtrl(true);
        }
        if (s_mode == 4 || s_mode == 5 || s_mode == 7 || s_mode == 8 || s_mode == 9 || s_mode == 10)//ISCAT-A ISCAT-B jt65abc
        {
            SetTitle(title+" RPT :");
            if ((s_mode == 7 || s_mode == 8 || s_mode == 9) && s_sh_rpt)
            {
                SetMask("XXX");
                le_in->setText("O");//SetText("O");
                SetSpinCtrl(false);

            }
            else
            {
                s_rep_pos = -15;
                le_in->setText("-15");//SetText("-15");
                SetSpinCtrl(true);
            }
        }
    }// not MSK144 MSK40 jt65abc pi4 ft8 ft4  if (s_mode>0 && s_mode<7)
    //else if (s_mode != 0 && s_mode != 7 && s_mode != 8 && s_mode != 9 && s_mode != 10 && s_mode != 11 && s_mode != 12 && s_mode != 13 && !allq65)
    else if (s_mode>0 && s_mode<7)//2.65 old modes only
    {
        SetTitle(title+" RSQ :");
        le_in->setText("599");//SetText("599");
        SetSpinCtrl(false);
    }
    //qDebug()<<rpt_rsq<<s_mode;
}
void HvRptLe::IncreaseRpt(int i)
{
    if (s_mode == 0)//MSK144 MSK40
    {
        if (s_cont_type==3)//for Contest v2
        {
            s_rep_pos = le_in->text().toInt() + i; //52-59
            le_in->setText(format_rpt(QString("%1").arg(s_rep_pos)));
        }
        else if (s_cont_type==5)//529-599
        {
            s_rep_pos = le_in->text().toInt() + i*10;
            le_in->setText(format_rpt(QString("%1").arg(s_rep_pos)));
            //qDebug()<<s_rep_pos;
        }
        else
        {
            if (s_sh_rpt)//MSK40
            {
                s_rep_pos = s_rep_pos + i;
                if (s_rep_pos < 0)
                    s_rep_pos = 0;
                if (s_rep_pos > 6)
                    s_rep_pos = 6;

                le_in->setText(rpt_msk40[s_rep_pos]);
            }
            else  //MSK144
            {
                s_rep_pos = le_in->text().toInt() + i;
                le_in->setText(format_rpt(QString("%1").arg(s_rep_pos)));
            }
        }
    }
    if (s_mode == 1 || s_mode == 2 || s_mode == 3 || s_mode == 6 || s_mode == 12)//JTMS FSK441 FSK315 JT6M
    {
        s_rep_pos = s_rep_pos + i;
        if (s_rep_pos < 0)
            s_rep_pos = 0;
        if (s_rep_pos > 15)
            s_rep_pos = 15;
        le_in->setText(rpt_ms_p[s_rep_pos]);
    }
    //ISCAT-A ISCAT-B jt65abc pi4 ft8 ft4
    if (s_mode == 4 || s_mode == 5 || s_mode == 7 || s_mode == 8 || s_mode == 9 || s_mode == 10 || s_mode == 11 || s_mode == 13 || s_mode == 18 || allq65)
    {
        if ((s_mode == 11 || s_mode == 13 || s_mode == 18 || allq65) && s_cont_type==3)//for Contest v2 ft8 ft4
        {
            s_rep_pos = le_in->text().toInt() + i; //52-59
            le_in->setText(format_rpt(QString("%1").arg(s_rep_pos)));
        }
        else if ((s_mode == 11 || s_mode == 13 || s_mode == 18 || allq65) && s_cont_type==5)//529-599  ft8 ft4
        {
            s_rep_pos = le_in->text().toInt() + i*10;
            le_in->setText(format_rpt(QString("%1").arg(s_rep_pos)));
            //qDebug()<<s_rep_pos;
        }
        else
        {
            s_rep_pos = le_in->text().toInt() + i;
            le_in->setText(QString("%1").arg(s_rep_pos));
        }
    }

    emit EmitEntered();
}
void HvRptLe::stop_inc()
{
    timer_inc->stop();
}
void HvRptLe::timer_inc_rpt()
{
    if (wait_tacts>2)
        IncreaseRpt(inc_up_down);
    else
        wait_tacts++;
}
void HvRptLe::up_rpt()
{
    inc_up_down = 1;
    IncreaseRpt(inc_up_down);
    wait_tacts = 0;
    timer_inc->start(150);
}
void HvRptLe::down_rpt()
{
    inc_up_down = -1;
    IncreaseRpt(inc_up_down);
    wait_tacts = 0;
    timer_inc->start(150);
}
void HvRptLe::SetTitle(QString s)
{
    s_name = s;
    l_n->setText(s_name);
}

void HvRptLe::SetFocus()
{
    le_in->setFocus();
    //if (le_in->text().at(le_in->cursorPosition()).toLatin1() == 0 )
    if (le_in->text().at(le_in->cursorPosition()).isPrint())
        le_in->setCursorPosition(le_in->text().count());
    //focusNextChild();
}
void HvRptLe::setFixedWidthLine(int w)
{
    le_in->setFixedWidth(w);
}
void HvRptLe::setMaxLength(int l)
{
    le_in->setMaxLength(l);
}
void HvRptLe::SndCheck_s(QString s)
{
    if (s_rpt_rsq)
    {
        //s_f_rpt_rsq
        bool f = true;
        QString s0 = getText().at(0);
        QString s1 = getText().at(1);
        QString s2 = getText().at(2);

        if (s0.toInt()<1 || s0.toInt()>5)
            f=false;
        if (s1.toInt()<1)
            f=false;
        if (s2.toInt()<1)
            f=false;

        if (!f)
        {
            setErrorColorLe(true);
            setError(true);
        }
        else
        {
            setErrorColorLe(false);
            setError(false);
        }
    }
    else
    {
        if (s_mode == 0)//MSK144 MSK40
        {
            if (s_cont_type==3)//for Contest v2
            {
                if ( s.toInt() < 52 )
                    le_in->setText("52");//52-59
                if ( s.toInt() > 59 )
                    le_in->setText("59");
            }
            else if (s_cont_type==5)//529-599
            {
                QString corr = le_in->text(); //qDebug()<<"corr"<<s_mode<<s_id_contest_mode<<corr;
                if (s.midRef(0,1).toInt() < 5)
                    corr="529";
                if (s.midRef(0,1).toInt() > 5)
                    corr="599";
                if (s.midRef(1,1).toInt() < 2)
                    corr="529";
                if (s.midRef(2,1).toInt() < 9)
                    corr="599";
                if (s.toInt() > 599)
                    corr="599";
                le_in->setText(corr);
            }
            else
            {
                if ( s.toInt() < -8 ) //-8   min msk144=-4, msk40=-5, Rtd msk144 msk40=-8
                    le_in->setText("-8");
                if ( s.toInt() > 25 )  //25  max msk144=24, msk40=25, Rtd msk144 msk40=24
                    le_in->setText("25");
            }
        }
        if (s_mode == 1 || s_mode == 2 || s_mode == 3 || s_mode == 6 || s_mode == 12)//JTMS FSK441 FSK315 JT6M MSK144MS //lz2hv Hanbook 7.00
        {
            if (s.count() == 1)//1 digit
            {
                if (s.toInt()<2 || s.toInt()>5)//2345
                {
                    le_in->setText("2");
                    le_in->setCursorPosition(1);
                }
            }
            else if (s.count() == 2)
            {
                //if (s[1]<'6' || s[1]>'9')//6789
                if (s.midRef(1,1).toInt()<6 || s.midRef(1,1).toInt()>9)//6789
                {
                    le_in->setText("26");
                    le_in->setCursorPosition(2);
                }
            }
            else if (s.count() >= 3)
            {
                le_in->setText("26");
                le_in->setCursorPosition(2);
            }
        }
        //ISCAT-A ISCAT-B pi4
        if (s_mode == 4 || s_mode == 5 || s_mode == 10)
        {
            // limit from -50 to 49
            if ( s.toInt() < -50 )
                le_in->setText("-50");
            if ( s.toInt() > 49 )
                le_in->setText("49");
        }
        if (s_mode == 11 || s_mode == 13 || s_mode == 18 || allq65)//ft8 ft4
        {
            if (s_cont_type==3)//for Contest v2
            {
                if ( s.toInt() < 52 )
                    le_in->setText("52");//52-59
                if ( s.toInt() > 59 )
                    le_in->setText("59");
            }
            else if (s_cont_type==5)//529-599
            {
                QString corr = le_in->text(); //qDebug()<<"corr"<<s_mode<<s_cont_type<<corr;
                if (s.midRef(0,1).toInt() < 5)
                    corr="529";
                if (s.midRef(0,1).toInt() > 5)
                    corr="599";
                if (s.midRef(1,1).toInt() < 2)
                    corr="529";
                if (s.midRef(2,1).toInt() < 9)
                    corr="599";
                if (s.toInt() > 599)
                    corr="599";
                le_in->setText(corr);
            }
            else
            {
                if (allq65)
                {
                    if ( s.toInt() < -35 )
                        le_in->setText("-35");
                }
                else
                {
                    if ( s.toInt() < -26 )
                        le_in->setText("-26");
                }
                if ( s.toInt() > 49 )
                    le_in->setText("49");
            }
        }
        if (s_mode == 7 || s_mode == 8 || s_mode == 9)//jt65abc 1.50 max -30 for jt65 deep search
        {
            if ( s.toInt() < -30 )
                le_in->setText("-30");
            if ( s.toInt() > 49 )
                le_in->setText("49");
        }

        setErrorColorLe(false);
        setError(false);
    }
    emit EmitSndCheck(s_type);//2.66
}

void HvRptLe::SetMask(QString mask)
{
    le_in->setInputMask(mask);
}

void HvRptLe::SetText(QString str, bool f)//0=nill reaction, 1=real RPT
{
    //qDebug()<<"YES="<<str;
    if ((s_mode == 7 || s_mode == 8 || s_mode == 9) && s_sh_rpt)//1.42 no in jt65+sh mode
    {
        return;
    }
    if (f) is_real_rpt = true;	//2.33
    le_in->setText(str);
    //SndCheck(s_type);
}
void HvRptLe::setReadOnly(bool flag)
{
    le_in->setDisabled(flag);
}
void HvRptLe::setError(bool flag)
{
    if (flag)
    {
        l_n->setText(s_name);
        if (dsty) l_n->setStyleSheet("QLabel{color:rgb(255,150,150)}");
        else l_n->setStyleSheet("QLabel{color:"+ColorStr_[1]+"}");
    }
    else
    {
        l_n->setText(s_name);
        if (dsty) l_n->setStyleSheet("QLabel{color:rgb(255,255,255)}");
        else l_n->setStyleSheet("QLabel{color:"+ColorStr_[0]+"}");
    }
}
/*
void HvRptLe::SetErrotText(QString str)
{
    H_in->setSpacing(10);
    l_error->setText(str);
    l_error->setStyleSheet("QLabel {color: "+ColorStr_[1]+"}");

    //l_error->setText("<h3><font color=red>"+str);//"<font size='+1' color=red>"
}
*/
void HvRptLe::setErrorColorLe(bool f)
{
    if (!f)
    {
        le_in->setStyleSheet("QLineEdit{background-color:palette(Base);}");
        f_error = false;
    }
    else
    {
    	if (dsty) le_in->setStyleSheet("QLineEdit{background-color:rgb(45,45,45);color:rgb(255,150,150)}");
        else le_in->setStyleSheet("QLineEdit{background-color:"+ColorStr_[3]+";color:"+ColorStr_[1]+"}");
        f_error = true;
    }
}


HvInLeFreq::HvInLeFreq(QString name, QWidget * parent )
        : QWidget(parent)
{
    QLabel *l_n = new QLabel();
    l_n->setText(name);
    QRegExp rx("^[1-9][0-9.]*$");
    le_frq = new QLineEdit();
    QValidator *validator = new QRegExpValidator(rx, this);
    le_frq->setValidator(validator);
    le_frq->setMaxLength(14);
    //le_in->setFixedHeight(20);
    QHBoxLayout *H_in = new QHBoxLayout(this);
    H_in->setContentsMargins( 0, 0, 0, 0);
    H_in->setSpacing(5);
    H_in->addWidget(l_n);
    //H_in->setAlignment(l_n,Qt::AlignLeft);
    H_in->addWidget(le_frq);
    //H_in->setAlignment(Qt::AlignHCenter);
    this->setLayout(H_in);
}
HvInLeFreq::~HvInLeFreq()
{}
QString HvInLeFreq::Text()
{
    return le_frq->text();
}
void HvInLeFreq::SetText(QString s)
{
    le_frq->setText(s);
}

#include <QToolTip>
HvLabQrg::HvLabQrg(bool f,QWidget * parent )
        : QLabel(parent)
{
	dsty = f;
    setText("QRG :");
    setFrameStyle(QFrame::Panel | QFrame::Sunken);
    setFixedHeight(19);//19
    active_id = 0;// 0=noCAT, 1=Cat, 2=QRG Active
    //tooltip_ltext = "";
    RefreshAll();
    //QToolTip::setAlignment(Qt::AlignLeft);
    //setReadOnly(true);
}
HvLabQrg::~HvLabQrg()
{}
void HvLabQrg::SetActiveId(int i)
{
    active_id = i;
    RefreshAll();
}
void HvLabQrg::RefreshAll()
{
    if (active_id==0)
        setStyleSheet("QLabel{background-color:palette(Button);}");
    else if (active_id==1)
    {
    	if (dsty) setStyleSheet("QLabel{background-color:rgb(0,170,0);}"); 
        else setStyleSheet("QLabel{background-color:rgb(0,255,0);}");    	
   	}
    else if (active_id==2)
    {
    	if (dsty) setStyleSheet("QLabel{background-color:rgb(160,0,0);}");   
        else setStyleSheet("QLabel{background-color :rgb(255,0,0);}");    	
   	}
    emit EmitRefresh();              
}
void HvLabQrg::mousePressEvent(QMouseEvent *)
{
    if (active_id==1) active_id=2;
    else if (active_id==2) active_id=1;
    if (active_id==1 || active_id==2) RefreshAll();
}
static QString tooltip_ltext_ = "Random QRG: For MSK And FSK\n If CAT Is Active";
void HvLabQrg::mouseReleaseEvent(QMouseEvent * event)
{
    QPoint post = event->globalPos();
    post += QPoint(0,+5);
    QToolTip::showText(post,tooltip_ltext_,this,rect(),3000);
}
HvQrg::HvQrg(bool indsty,QWidget * parent )
        : QWidget(parent)
{
    l_qrg = new HvLabQrg(indsty);
    le_qrg = new HvLeNoSpace();
    le_qrg->setInputMask("NNN");
    connect(le_qrg, SIGNAL(textChanged(QString)), this, SIGNAL(EmitTextChanged(QString)));//for MAM
    connect(le_qrg, SIGNAL(EmitEnter()),this, SLOT(SetEnter()));
    connect(l_qrg, SIGNAL(EmitRefresh()), this, SLOT(SetEnter()));

    QHBoxLayout *H_in = new QHBoxLayout(this);
    H_in->setContentsMargins( 0, 0, 0, 0);
    H_in->setSpacing(0);
    H_in->addWidget(l_qrg);
    H_in->addWidget(le_qrg);
    rx_sqrg = "";
    tx_sqrg = "";
    f_one_no_refr_from_rig = false;
    this->setLayout(H_in);
}
HvQrg::~HvQrg()
{}
void HvQrg::SetFont(QFont f)
{
    l_qrg->setFont(f);
}
void HvQrg::SetReadOnly(bool f)
{
    if (f && l_qrg->active_id!=0)
    {
        //le_qrg->setReadOnly(f);
        //l_qrg->setEnabled(false);
        setEnabled(false);
    }
    else if (!isEnabled())
    {
        //le_qrg->setReadOnly(f);
        //l_qrg->setEnabled(true);
        setEnabled(true);
    }
}
void HvQrg::SetQrgActive(int i)
{
    l_qrg->SetActiveId(i); //qDebug()<<"EmitQgrParms="<<i;
}
void HvQrg::SetQrgFromRig(QString s)
{
    if (f_one_no_refr_from_rig)
    {
        //qDebug()<<"One Tact="<<s;
        f_one_no_refr_from_rig = false;
        return;
    }

    int beg = s.count() - 6;
    QString skhz = s.mid(beg,3);
    if (l_qrg->active_id!=2 || rx_sqrg==skhz) return;
    rx_sqrg=skhz;

    le_qrg->setText(skhz);
    //SetEnter();
    emit EmitEnter();
    //qDebug()<<"Ref Tact="<<s;
}
void HvQrg::SetQrgInfoFromCat(QString frq)
{
    int k = frq.count();
    k -= 3;
    if (k>0)
        frq.insert(k,".");
    k -= 3;
    if (k>0)
        frq.insert(k,".");
    tx_sqrg = frq;
}
void HvQrg::SetEnter()
{
	static int p_id = 0; //2.60
    f_one_no_refr_from_rig = true;
    QString s = le_qrg->text();
	bool islet = false;
    if (s.isEmpty() || s.at(0).isLetter() || s.at(1).isLetter() || s.at(2).isLetter())
    {
        if (l_qrg->active_id==2) l_qrg->SetActiveId(1);
        islet = true;
    }
    bool res = false;
    if (l_qrg->active_id==2) res = true;
    rx_sqrg=s;
    emit EmitQgrParms(s,res); //qDebug()<<"EmitQgrParms="<<s<<res<<l_qrg->active_id;
    emit EmitEnter();
    //else if (l_qrg->active_id==2) tooltip_ltext_ = "<p align=center>Random QRG: Is ON<br>For TX7<br>TX = "+tx_sqrg+"</p>";
    if (l_qrg->active_id==1) tooltip_ltext_ = tr("Random QRG: Is OFF");
    else if (l_qrg->active_id==2) tooltip_ltext_ = tr("Random QRG: Is ON For TX7\n TX = ")+tx_sqrg;
    else tooltip_ltext_ = tr("Random QRG: For MSK And FSK\n If CAT Is Active"); //"<p style=text-align:center>hjghghghgggg\nColumn2</p>";
    
    if (p_id == l_qrg->active_id) return;//2.60
    p_id = l_qrg->active_id;
    if (!islet) emit EmitActiveId(l_qrg->active_id);      
}
QString HvQrg::text()
{
    return le_qrg->text();
}
void HvQrg::setText(QString s)
{
    le_qrg->setText(s); 
}

