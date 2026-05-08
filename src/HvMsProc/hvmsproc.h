/* MSHV
 *
 * By Hrisimir Hristov - LZ2HV
 * May be used under the terms of the GNU General Public License (GPL)
 */

#ifndef HVMSPROC_H
#define HVMSPROC_H

//#include "../config.h"

#include <QWidget>

class HvMsProc : public QWidget
{
    //Q_OBJECT //2.65 <- for tr() Q_OBJECT 
public:
    HvMsProc(QString title,QString app_name,QString path,int lid,int,int,QWidget * parent = 0);
    virtual ~HvMsProc();
 
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
