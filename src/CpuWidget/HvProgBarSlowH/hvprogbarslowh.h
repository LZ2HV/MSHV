/* MSHV
 *
 * By Hrisimir Hristov - LZ2HV
 * May be used under the terms of the GNU General Public License (GPL)
 */

#ifndef HVPROGBARSLOWH_H
#define HVPROGBARSLOWH_H

#include <QWidget>
//#include <QTimer>
#include <qpainter.h>
//#include <QDebug>

class HvProgBarSlowV : public QWidget
{
    //Q_OBJECT
public:
    HvProgBarSlowV(int pos_x, int pos_y, QPixmap on_pix, QPixmap back_pix, QWidget * parent = 0);
    virtual ~HvProgBarSlowV();

	void setValue(int value, int maxValue);
	
private:
    QPixmap on, back;
    int x_upd_pos_l, on_width, on_height;
    //int factor;


//public slots:
    //void setValue(int value, int maxValue);

protected:
    void paintEvent(QPaintEvent *);


};


class HvProgBarSlowH : public QWidget
{
    //Q_OBJECT
public:
    HvProgBarSlowH(int pos_x, int pos_y, QPixmap on_pix, QPixmap back_pix, QWidget * parent = 0);
    virtual ~HvProgBarSlowH();
    
	void setValue(int value, int maxValue);
	
private:
    QPixmap on, back;
    int x_upd_pos_l, on_height;
    int pix_width_koef;
    //int factor


//public slots:
    //void setValue(int value, int maxValue);

protected:
    void paintEvent(QPaintEvent *);


};
#endif



