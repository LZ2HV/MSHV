/* MSHV
 *
 * By Hrisimir Hristov - LZ2HV
 * May be used under the terms of the GNU General Public License (GPL)
 */

#ifndef HVABOUTMSHV_H
#define HVABOUTMSHV_H

//#include "../config.h"

//#include <QWidget>
//#include <QPainter>
//#include <QMouseEvent>
//#include <QBitmap>
//#include <QVBoxLayout>
//#include <QTextEdit>
#include <QDialog>
//#include <QLabel>
//#include <QTextBrowser>
//#include <QIcon>
//#include <QPixmap>

//#include <QApplication>

class HvAboutMsHv : public QDialog
{
    //Q_OBJECT //2.65 <- for tr() Q_OBJECT 
public:
    HvAboutMsHv(QString title,QString app_name,QString path,int lid,QWidget * parent = 0);
    virtual ~HvAboutMsHv();

    /*QString GetName()
    {
        return NAM_s;
    };
    QString GetVer()
    {
        return VER_s;
    };*/
    
//public slots:
    //void ShowBox();

//private:
    /*QPixmap p_about_over;
    QPixmap p_about;
    QPixmap pupdate_hv;
    void setPixmap_hv(QPixmap);
    QString TextInfo();
    QRegion in_region;
    QString VER_s;
    QString NAM_s;
    QString YEAR_s;*/

//protected:
    //void mousePressEvent ( QMouseEvent * event);
    //void mouseReleaseEvent(QMouseEvent * event);
    //void mouseMoveEvent(QMouseEvent * event);
    //void paintEvent( QPaintEvent * event);
};
#endif
