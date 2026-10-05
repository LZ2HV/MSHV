#ifndef HVSTYLEPLASTIQUE_H
#define HVSTYLEPLASTIQUE_H


//#undef __has_builtin stop 2.56

#include <QtCore/qglobal.h>
#include <QtCore/qpoint.h>
#include <QtCore/qstring.h>
#include <QtGui/qpolygon.h>

#include <QtCore/qstringbuilder.h>
#include <QtCore/qmath.h>
#include <QCommonStyle>
#include <QFontDatabase>
#include <QListView>
#include <QWizard>
#include <qapplication.h>
#include <qbitmap.h>
#include <qabstractitemview.h>
#include <qcheckbox.h>
#include <qcombobox.h>
//#include <qdebug.h>
#include <qdialogbuttonbox.h>
#include <qformlayout.h>
#include <qgroupbox.h>
#include <qimage.h>
#include <qlineedit.h>
#include <qmainwindow.h>
#include <qmenu.h>
#include <qmenubar.h>
#include <qpainter.h>
#include <qpaintengine.h>
#include <qpainterpath.h>
#include <qpalette.h>
#include <qpen.h>
#include <qpixmap.h>
#include <qpixmapcache.h>
#include <qprogressbar.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qscrollbar.h>
#include <qspinbox.h>
#include <qsplitter.h>
#include <qstyleoption.h>
#include <qtextedit.h>
#include <qelapsedtimer.h>
#include <qtoolbar.h>
#include <qtoolbox.h>
#include <qtoolbutton.h>
//#include <qworkspace.h>
#include <qprocess.h>
#include <qvarlengtharray.h>
#include <limits.h>


#define HvPublicStyle QCommonStyle//QProxyStyle//QCommonStyle

//class HvStylePlastiquePrivate;

class HvStylePlastique : public HvPublicStyle //QFusionStyle   //QCommonStyle //QProxyStyle  //QPlastiqueStyle //QStyleFactory
{
    Q_OBJECT
    //Q_DECLARE_PRIVATE(HvStylePlastique)
public:
    HvStylePlastique(bool);
    ~HvStylePlastique();

    //void SetStyleHV(bool);
    void drawPrimitive(PrimitiveElement element, const QStyleOption *option,
                       QPainter *painter, const QWidget *widget) const;

    void drawControl( ControlElement element, const QStyleOption * option, QPainter * painter, const QWidget * widget = 0 ) const;
    void drawComplexControl(ComplexControl control, const QStyleOptionComplex *option,
                            QPainter *painter, const QWidget *widget) const;
    QSize sizeFromContents ( ContentsType type, const QStyleOption * option, const QSize & contentsSize, const QWidget * widget = 0 ) const;

    QRect subElementRect ( SubElement element, const QStyleOption * option, const QWidget * widget = 0 ) const;
    QRect subControlRect(ComplexControl cc, const QStyleOptionComplex *opt, SubControl sc, const QWidget *widget) const;

    int styleHint(StyleHint hint, const QStyleOption * option = 0, const QWidget * widget = 0, QStyleHintReturn * returnData = 0 ) const;
    SubControl hitTestComplexControl(ComplexControl control, const QStyleOptionComplex *option,
                                     const QPoint &pos, const QWidget *widget = 0) const;
    int pixelMetric(PixelMetric metric,const QStyleOption *option,const QWidget *widget) const;

    QPixmap standardPixmap(StandardPixmap standardPixmap, const QStyleOption *opt,
                           const QWidget *widget = 0) const;

    void polish(QWidget *widget);
    void polish(QApplication *app);
    void polish(QPalette &pal);
    void unpolish(QWidget *widget);
    void unpolish(QApplication *app);
    //new qt5 old is qt4=layoutSpacingImplementation
    int layoutSpacing(QSizePolicy::ControlType control1, //2.66
                      QSizePolicy::ControlType control2,
                      Qt::Orientation orientation,
                      const QStyleOption *option = 0,
                      const QWidget *widget = 0) const;

    //QPalette standardPalette() const;

//protected Q_SLOTS: //old qt4 definitions qt5=layoutSpacing
    //QIcon standardIconImplementation(StandardPixmap standardIcon, const QStyleOption *opt = 0,
    // const QWidget *widget = 0) const;
    /*int layoutSpacingImplementation(QSizePolicy::ControlType control1,
                                    QSizePolicy::ControlType control2, 
                                    Qt::Orientation orientation,
                                    const QStyleOption *option = 0, 
                                    const QWidget *widget = 0) const;*/

//protected:
    //bool eventFilter(QObject *watched, QEvent *event);// hv niama smisal
    //void timerEvent(QTimerEvent *event);  //hv niama smisal

private:
    bool f_dark_style;
    Q_DISABLE_COPY(HvStylePlastique)
    void *reserved;
    //static void setTexture(QPalette &palette, QPalette::ColorRole role,
    // const QPixmap &pixmap);
    //static QPainterPath roundRectPath(const QRect &rect);
    //static QColor mergedColors(const QColor &colorA, const QColor &colorB, int factor = 50);

};
#endif



