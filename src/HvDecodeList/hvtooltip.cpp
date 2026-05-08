/* MSHV ToolTipWidget
 * Copyright 2019 Hrisimir Hristov, LZ2HV
 * May be used under the terms of the GNU General Public License (GPL)
 */
#include "hvtooltip.h"

#include <QApplication>
#include <QToolTip>
//#include <QPainter>
#include <QMouseEvent>
#include <QDesktopWidget>
//#include <QtGui>

HvToolTip::HvToolTip(QWidget *w)//
        :QLabel(w,Qt::ToolTip | Qt::BypassGraphicsProxyWidget),widget(0)
        //no have shadow -> Qt::ToolTip | Qt::BypassGraphicsProxyWidget  | Qt::FramelessWindowHint
{
    tim_is_active = false;
    setForegroundRole(QPalette::ToolTipText);
    setBackgroundRole(QPalette::ToolTipBase);
    setPalette(QToolTip::palette());

    //setAttribute(Qt::FramelessWindowHint);
    //l_tooltip->setMargin(1 + style()->pixelMetric(QStyle::PM_ToolTipLabelFrameWidth, 0, this));
    //setFrameStyle(QFrame::Box);// | QFrame::Sunken  StyledPanel
    //setLineWidth(1);
    //setFrameShadow(QFrame::Plain);
    setAlignment(Qt::AlignLeft);
    qApp->installEventFilter(this);//qApp form All app
    //l_tooltip->setWindowOpacity(style()->styleHint(QStyle::SH_ToolTipLabel_Opacity, 0, this) / 255.0);
    //this->setWindowFlags(Qt::FramelessWindowHint);
    setStyleSheet("QFrame, QLabel {border: 2px solid rgb(10,70,10); border-radius: 0px;"
                  "padding-left: 2px; padding-top: 1px; padding-right: 2px; padding-bottom: 1px;"
                  "color: black; background-color: #ffffe1;}"); //color: black; background-color: #ffffe1; <- for linux 

	//setWindowFlags(Qt::ToolTip | Qt::FramelessWindowHint);
    setHidden(true);
    tim_stop = new QTimer(this);
    connect(tim_stop, SIGNAL(timeout()), this, SLOT(Slot_stop()));
}
HvToolTip::~HvToolTip()
{
    tim_stop->stop();
}
/*int HvToolTip::getTipScreen(const QPoint &pos, QWidget *w)
{
	//qt5.6.3
    if (QApplication::desktop()->isVirtualDesktop())
        return QApplication::desktop()->screenNumber(pos);
    else
        return QApplication::desktop()->screenNumber(w);                      
           
    // >= qt5.10.0         
    if (QDesktopWidgetPrivate::isVirtualDesktop())
        return QDesktopWidgetPrivate::screenNumber(pos);
    else
        return QDesktopWidgetPrivate::screenNumber(w);     
}*/
#include <QScreen>
void HvToolTip::placeTip(const QPoint &pos, QWidget */*w*/)
{	
	//qDebug()<<getTipScreen(pos, w);
    //QRect screen = QApplication::desktop()->screenGeometry(getTipScreen(pos, w)); 
    //QRect screen12 = QApplication::desktop()->screenGeometry(10); 
    /*QDesktopWidget dw;   
    QScreen* screen0 = QGuiApplication::screens()[dw.screenNumber(this)];
    if (!screen0) return;
    QRect screen = screen0->availableGeometry();*/
     
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)//2.61 
    QScreen* screen0 = QGuiApplication::screenAt(pos); //or screenAt(this->mapToGlobal({this->width()/2,0})); 
#else
	QDesktopWidget dw;   
    QScreen* screen0 = QGuiApplication::screens()[dw.screenNumber(pos)]; //or screenAt(this->mapToGlobal({this->width()/2,0})); 
#endif    
    if (!screen0) return;    
    QRect screen = screen0->availableGeometry(); //qDebug()<<screen0;
 
    QPoint p = pos;
    p += QPoint(2,16
                /*
                #ifdef Q_DEAD_CODE_FROM_QT4_WIN
                                21
                #else
                                16
                #endif
                */
               );
    if (p.x() + this->width() > screen.x() + screen.width())
        p.rx() -= 4 + this->width();
    if (p.y() + this->height() > screen.y() + screen.height())
        p.ry() -= 24 + this->height();
    if (p.y() < screen.y())
        p.setY(screen.y());
    if (p.x() + this->width() > screen.x() + screen.width())
        p.setX(screen.x() + screen.width() - this->width());
    if (p.x() < screen.x())
        p.setX(screen.x());
    if (p.y() + this->height() > screen.y() + screen.height())
        p.setY(screen.y() + screen.height() - this->height());
    move(p);
}
void HvToolTip::showText(QPoint pos,QString s,int time, QWidget *w)
{
    //tim_is_active = false;
    //setHidden(true); <- not good for linux
    setText(s);
    adjustSize();// for linux
    widget = w;
    rect = QRect();
	placeTip(pos,w);
	//setObjectName(QLatin1String("qtooltip_label"));
    setHidden(false);
    //showNormal();
    tim_stop->start(time);
    tim_is_active = true;
}
void HvToolTip::Slot_stop()
{
    tim_stop->stop();
    setHidden(true);
    tim_is_active = false;
}
void HvToolTip::hideTip()
{
    tim_stop->start(300);
    tim_is_active = false;//qDebug()<<"hideTip"<<rrr;  rrr++;    
}
void HvToolTip::hideTipImmediately()
{
    tim_stop->start(30);
    tim_is_active = false;//qDebug()<<"hideTipImmediately"<<rrr;  rrr++;   
}
bool HvToolTip::eventFilter(QObject *o, QEvent *e)
{
    if (!tim_is_active) return false;

    switch (e->type())
    {
    case QEvent::Leave:
        hideTip();
        break;
#if defined (Q_OS_QNX) // On QNX the window activate and focus events are delayed and will appear
        // after the window is shown.
    case QEvent::WindowActivate:
    case QEvent::FocusIn:
        return false;
    case QEvent::WindowDeactivate:
        if (o != this)
            return false;
        hideTipImmediately();
        break;
    case QEvent::FocusOut:
        if (reinterpret_cast<QWindow*>(o) != windowHandle())
            return false;
        hideTipImmediately();
        break;
#else
    case QEvent::WindowActivate:
    case QEvent::WindowDeactivate:
    case QEvent::FocusIn:
    case QEvent::FocusOut:
#endif
    case QEvent::Close: // For QTBUG-55523 (QQC) specifically: Hide tooltip when windows are closed
    case QEvent::MouseButtonPress:
        //case QEvent::MouseButtonRelease:
    case QEvent::Wheel:
        hideTipImmediately();
        break;
    case QEvent::MouseButtonDblClick:
        hideTip();// qDebug()<<"dsfgsfs"<<e->type();
        break;
    case QEvent::MouseMove:
        if (o == widget && !rect.isNull() && !rect.contains(static_cast<QMouseEvent*>(e)->pos()))
            hideTip();
    default:
        break;
    }
    return false;
}

