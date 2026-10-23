/* MSHV
 *
 * By Hrisimir Hristov - LZ2HV
 * May be used under the terms of the GNU General Public License (GPL)
 */

#ifndef HVBUTTON_LEFT2_H
#define HVBUTTON_LEFT2_H
//
//#include <QWidget>
//#include <QtGui> // slaga se zaradi qdebug
#include <QWidget>
#include <QBitmap>
#include <QPainter>
#include <QMouseEvent>
//#include <QtGui/qbitmap.h>
//#include <QtGui/qpainter.h>
//#include <QtGui/qevent.h>
//
class HvButton_Left2 : public QWidget
{
	Q_OBJECT
public:
	HvButton_Left2(QWidget *parent = 0);
	virtual ~HvButton_Left2();

    void SetupButton_hv(QPixmap release, QPixmap press, int x_pos, int y_pos,bool); 
    
signals:
    void Press_Lift_Button_hv();
    void Release_Lift_Button_hv();     
    
//public slots:  
    //void SetupButton_hv(QPixmap release, QPixmap press, int x_pos, int y_pos);    

private: 	    
	QPixmap release_butt_stop;
    QPixmap press_butt_stop;
    QPixmap pupdate_hv;    
    bool Button_Release_Left;
    bool Mouse_Over_Button;
    bool f_outin;
 	    
protected:
	void mousePressEvent ( QMouseEvent * event);
	void mouseReleaseEvent(QMouseEvent * event);
	void mouseMoveEvent(QMouseEvent *event);
    void paintEvent( QPaintEvent * event);	
    void setPixmap_hv(QPixmap pixmap_hv);

//private: //private slots:
    //void slot_Press_Button_2();
    //void slot_Release_Button_2();    
  
    
};
#endif
