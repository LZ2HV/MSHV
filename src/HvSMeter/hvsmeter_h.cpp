#include "hvsmeter_h.h"

HvSMeter_H::HvSMeter_H(int pos_x, int pos_y, QPixmap on_pix, QPixmap clip_pix, QPixmap back_pix, QWidget * parent)
        : QWidget(parent)
{ 
    move(pos_x, pos_y);
    on = on_pix;
    clip = clip_pix;
    back = back_pix;
    setFixedSize(back.width(),back.height());
    x_upd_pos_l = 0;
	//x_upd_pos_r = 0;
    clip_l = 0;
	//clip_r = 0;
    factor = 4;// zavisi ot prez kolko piksela se dwizi indicatora hv
    on_height = on.height();
    clip_height = clip.height();
	//left_pos_y = back.height() - on.height();

    c_delay = 30; // zadrazka na clipa
    c_spead = 3; // ckorost na padane na clipa
    d_limit_l = 0;
	//d_limit_r = 0;
    save_val = -1;//2.61 0;

    avg_val = 0;
    avg_count = 0;
    avg_count_delay = 0;
}
HvSMeter_H::~HvSMeter_H()
{}
void HvSMeter_H::SetLMRefr(int v)
{
    avg_count_delay = v;
    c_delay = 30/(avg_count_delay+1);
}
void HvSMeter_H::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.drawPixmap(0, 0, back);
    painter.drawPixmap(0, 0, x_upd_pos_l, on_height, on, 0, 0, x_upd_pos_l, on_height);
    painter.drawPixmap(clip_l, 0, clip);   
 	if(save_val!=-1 && x_upd_pos_l<width()/3)//2.61  //if(save_val!=-1 && (x_upd_pos_l<width()/3 || x_upd_pos_l>width()-30))//2.62    
    {
    	//QFontMetrics fm = QFontMetrics(font());
    	//int h = (height()/2)+(fm.height()/4);
    	painter.fillRect(0,0,width(),height(),QColor(0,0,0,110));//width()
    	painter.setPen(QPen(QColor(255,255,255),0));
    	painter.drawText(50,height()-5,tr("Low Input Level")); //height()-5   Ниско входно ниво   	
    	//if(x_upd_pos_l<width()/3) painter.drawText(50,height()-5,tr("Low Input Level")); //height()-5   Ниско входно ниво 
    	//else painter.drawText(50,height()-5,tr("High Input Level"));	  	
   	}
}
void HvSMeter_H::setValue(int val_l)
{
    if (val_l < 0)
        val_l = 0;

    avg_val += val_l;
    avg_count++;

    //if (avg_count>0)//0 fast max
    if (avg_count>avg_count_delay)
    {
        val_l = avg_val/avg_count;
        avg_count = 0;
        avg_val = 0;
    }
    else
        return;

	/// plavno strelkata
    if (save_val < val_l)
        save_val = val_l;
    else
        save_val = save_val - 1;
    if (save_val < 0)
        save_val = 0;
	/// plavno strelkata

    int scale_l = save_val/2*factor;

    if (clip_l < scale_l)
    {
        clip_l = scale_l;
        //d_limit_l = - (3);//30
        d_limit_l = - (c_delay);
    }

    //if (d_limit_l > 2)//3
    if (d_limit_l > c_spead)
    {
        clip_l = clip_l - factor;
        d_limit_l = 0;
    }
    ++d_limit_l;

    x_upd_pos_l = scale_l;

    update();
}

