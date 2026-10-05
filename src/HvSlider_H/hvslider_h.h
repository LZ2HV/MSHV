/* MSHV AggressiveDialog
 * Copyright 2015 Hrisimir Hristov, LZ2HV
 * May be used under the terms of the GNU General Public License (GPL)
 */

#ifndef HVSLIDER_H_H
#define HVSLIDER_H_H
//
#include <qwidget.h>
#include <qpainter.h>
#include <qbitmap.h>
#include <qevent.h>
//
class HvSlider_H : public QWidget
{
    Q_OBJECT
public:
    HvSlider_H(int MaxValue, int x_pos, int y_pos, QPixmap pic_sld_left, QPixmap pic_sld_track,
               QPixmap pic_sld_right, QPixmap pic_tumb, QPixmap pic_tumb_over, QWidget * parent = 0);
    virtual ~HvSlider_H();

    int get_value(void) const
    {
        return saved_val;
    }; //hv
    void SetValue(int);

private: // 4astni ne se vizdat ot van
    QPixmap upd_tumb;
    QPixmap pix_sld_left;
    QPixmap pix_sld_track;
    QPixmap pix_sld_right;
    QPixmap pix_tumb;
    QPixmap pix_tumb_over;
    QRegion Tregion;
    int MaxValuein;
    int upd_pos_x;
    int Reg_offset, Y_tumb_offset, Sl_track_range, Sl_track_begin, Sl_track_end;
    int MidDel;
    int wheeldata;
    bool wheelevent, rightbutton;
    int saved_val;

signals:
    void SendValue(int);
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
