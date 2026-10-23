/* MSHV
 *
 * By Hrisimir Hristov - LZ2HV
 * May be used under the terms of the GNU General Public License (GPL)
 */

#ifndef HVBUTTON_LEFT4_H
#define HVBUTTON_LEFT4_H
//
//#include <QWidget>
//#include <QtGui> // slaga se zaradi qdebug
#include <QWidget>
#include <QBitmap>
#include <QPainter>
#include <QEvent>
#include <QMouseEvent>
//
class HvButton_Left4 : public QWidget
{
    Q_OBJECT
public:
    HvButton_Left4(QWidget * parent = 0);
    virtual ~HvButton_Left4();
    
    bool Button_Stop_b;
    void ExtrnalRelease();
    void SetupButton_hv(QPixmap release_stop, QPixmap press_start, QPixmap release_start, QPixmap press_stop,
                        int x_pos, int y_pos);
    void Release_Button_hv();
    void Press_Button_hv();

signals:
    void Press_Lift_Button_hv();
    void Release_Lift_Button_hv();

//public slots:
    //void SetupButton_hv(QPixmap release_stop, QPixmap press_start, QPixmap release_start, QPixmap press_stop,
    //int x_pos, int y_pos);
    //void Release_Button_hv();
    //void Press_Button_hv();
    
private:
	QPixmap release_butt_start;
    QPixmap press_butt_start;
    QPixmap release_butt_stop;
    QPixmap press_butt_stop;
    QPixmap pupdate_hv;
    bool Button_Release_Left;
    bool Mouse_Over_Button;
    void p_Press_Button_4();
    void p_Release_Button_4();

protected:
    void mousePressEvent ( QMouseEvent * event);
    void mouseReleaseEvent(QMouseEvent * event);
    void mouseMoveEvent(QMouseEvent *event);
    void paintEvent( QPaintEvent * event);
    void setPixmap_hv(QPixmap pixmap_hv);

    /*protected slots:
        void slot_Press_Button_4();
        void slot_Release_Button_4();*/

};
#endif
