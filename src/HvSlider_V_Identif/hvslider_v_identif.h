/* MSHV
 *
 * By Hrisimir Hristov - LZ2HV
 * May be used under the terms of the GNU General Public License (GPL)
 */

#ifndef HVSLIDER_V_IDENTIF_H
#define HVSLIDER_V_IDENTIF_H
//
#include <qwidget.h>
#include <qpainter.h>
#include <qbitmap.h>
#include <qevent.h>
//
class HvSlider_V_Identif : public QWidget
{
    Q_OBJECT
public:
    HvSlider_V_Identif(int identif, int MaxValue, int x_pos, int y_pos, QPixmap pic_sld_up, QPixmap pic_sld_track,
               QPixmap pic_sld_down, QPixmap pic_tumb, QPixmap pic_tumb_over, QWidget * parent = 0);
    virtual ~HvSlider_V_Identif();

    int get_value()
    {
        return seve_val;
    }; //hv
    void SetThumbs(QPixmap pic_thumb, QPixmap pic_tumb_over);
    void SetValue(int);

private: // 4astni ne se vizdat ot van
    QPixmap upd_tumb;
    QPixmap pix_sld_up;
    QPixmap pix_sld_track;
    QPixmap pix_sld_down;
    QPixmap pix_tumb;
    QPixmap pix_tumb_over;
    QRegion Tregion;
    int MaxValuein;
    int upd_pos_y;
    int Reg_offset, X_tumb_offset, Sl_track_range, Sl_track_begin, Sl_track_end;
    int MidDel;
    int wheeldata;
    bool wheelevent, rightbutton;
    int seve_val;
    int s_identif;
   // int s_hand_identif;

signals:
    void SendValue(int,int);
    //void MouseDragThumb(bool);

//public slots:
    //void SetValue(int);
    //void MouseWheelEvent(bool);
    //void MouseRightButtonEvent(bool);

protected:
    void paintEvent(QPaintEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void mousePressEvent ( QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void wheelEvent(QWheelEvent *);
    void setPix_tumb(QPixmap);
    void Move(int, bool);
    void MoveSlider(int);

};
#endif
