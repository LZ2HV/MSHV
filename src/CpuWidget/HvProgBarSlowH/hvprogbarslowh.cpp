/* MSHV HvProgBarSlowV
 * Copyright 2015 Hrisimir Hristov, LZ2HV
 * May be used under the terms of the GNU General Public License (GPL)
 */
#include "hvprogbarslowh.h"

/////////////////////////V PROGRES BAR///////////////////////////////////////
HvProgBarSlowV::HvProgBarSlowV(int pos_x, int pos_y, QPixmap on_pix,QPixmap back_pix, QWidget * parent)
        : QWidget(parent)
{
    move(pos_x, pos_y);
    on = on_pix;
    back = back_pix;
    setFixedSize(back.width(), back.height());
    on_height = on.height();//on.width();
    x_upd_pos_l = 0;
    //factor = 1;// zavisi ot prez kolko piksela se dwizi indicatora hv // vazno ne factor*factor gore HV!!!!
    on_width = on.width();
    //left_pos_y = back.height() - on.height();
}
HvProgBarSlowV::~HvProgBarSlowV()
{}
void HvProgBarSlowV::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.drawPixmap(0, 0, back);
    //painter.drawPixmap(0, 0, x_upd_pos_l, on_height, on, 0, 0, x_upd_pos_l, on_height);  
    //painter.drawPixmap(0, 0, on_height, x_upd_pos_l, on ,0, 0, on_height, x_upd_pos_l); 
    painter.drawPixmap(0, x_upd_pos_l, on_width, on_height, on,0, x_upd_pos_l, on_width, on_height);
    //0, pix_width_koef - x_upd_pos_l, on_width, pix_width_koef - x_upd_pos_l);   
     //( int x, int y, int width, int height, const QPixmap & pixmap )
}
void HvProgBarSlowV::setValue(int val_l, int max)
{   // qDebug()<<val_l;
    if (val_l < 0) val_l = 0;
    //if (val_l > max) val_l = max;
    double coef = (double)max/on_height;
    val_l = (int)((double)val_l/coef);
    if (val_l>on_height) val_l=on_height;
    //int scale_l = (val_l/factor)*factor; // vazno ne factor*factor gore
    //x_upd_pos_l = on_height-scale_l;    
    x_upd_pos_l = on_height-val_l;
    update();
}
/////////////////////////END V PROGRES BAR///////////////////////////////////////

/////////////////////////H PROGRES BAR///////////////////////////////////////////
HvProgBarSlowH::HvProgBarSlowH(int pos_x, int pos_y, QPixmap on_pix,QPixmap back_pix, QWidget * parent)
        : QWidget(parent)
{
    move(pos_x, pos_y);
    on = on_pix;
    back = back_pix;
    setFixedSize(back.width(), back.height());
    pix_width_koef = on.width();
    x_upd_pos_l = 0;
    //factor = 1;// zavisi ot prez kolko piksela se dwizi indicatora hv // vazno ne factor*factor gore HV!!!!
    on_height = on.height();
    //left_pos_y = back.height() - on.height();
}
HvProgBarSlowH::~HvProgBarSlowH()
{}
void HvProgBarSlowH::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.drawPixmap(0, 0, back);
    painter.drawPixmap(0, 0, x_upd_pos_l, on_height, on, 0, 0, x_upd_pos_l, on_height);  
}
void HvProgBarSlowH::setValue(int val_l, int max)
{   // qDebug()<<val_l;
    if (val_l < 0) val_l = 0;
    //if (val_l > max) val_l = max;
    double coef = (double)max/pix_width_koef;
    val_l = (int)((double)val_l/coef);
    if (val_l>pix_width_koef) val_l=pix_width_koef;
    //int scale_l = (val_l/factor)*factor; // vazno ne factor*factor gore
    //x_upd_pos_l = scale_l;    
    x_upd_pos_l = val_l;
    update();
}
/////////////////////////END H PROGRES BAR///////////////////////////////////////



