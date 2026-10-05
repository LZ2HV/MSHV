#ifndef HVSMETER_H_H
#define HVSMETER_H_H

#include <QWidget>
#include <qpainter.h>
//#include <QtGui>

class HvSMeter_H : public QWidget
{
    Q_OBJECT
public:
    HvSMeter_H(int pos_x, int pos_y, QPixmap on_pix, QPixmap clip_pix, QPixmap back_pix, QWidget * parent = 0);
    virtual ~HvSMeter_H();

    void SetLMRefr(int);

private:
    QPixmap on, clip, back;
    int x_upd_pos_l, on_height, clip_height;
    int factor, clip_l;
    int c_spead, c_delay ,d_limit_l; 
    int save_val;    
    int avg_val;
    int avg_count;
    int avg_count_delay;

public slots:
    void setValue(int);

protected:
    void paintEvent(QPaintEvent *);

};
#endif

