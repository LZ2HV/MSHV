/* MSHV HvSlider_V_Identif
 * Copyright 2015 Hrisimir Hristov, LZ2HV
 * May be used under the terms of the GNU General Public License (GPL)
 */
#include "hvslider_v_identif.h"

HvSlider_V_Identif::HvSlider_V_Identif(int identif, int MaxValue, int x_pos, int y_pos, QPixmap sld_up,
                                       QPixmap sld_track, QPixmap sld_down, QPixmap tumb, QPixmap tumb_over, QWidget * parent)
        : QWidget(parent)
{
    s_identif = identif;
   // s_hand_identif = hand_identif;
    seve_val = 0;
    if (MaxValue<=0)// pazi da ne garmi
        MaxValuein = 10;
    else
        MaxValuein = MaxValue;
    move(x_pos, y_pos);
    pix_sld_up = sld_up;
    pix_sld_track = sld_track;
    pix_sld_down = sld_down;
    pix_tumb = tumb;
    pix_tumb_over = tumb_over;
    setFixedSize(pix_sld_track.width(),
                 (pix_sld_up.height()+pix_sld_track.height()+pix_sld_down.height()));  ///

    X_tumb_offset =(pix_sld_track.width() - pix_tumb.width())/2; ///
    Sl_track_range = pix_sld_track.height() - pix_tumb.height();  ///
    Sl_track_begin = pix_sld_up.height(); ///
    Sl_track_end = Sl_track_begin + Sl_track_range;
    MidDel = pix_sld_track.height()/MaxValuein/2;   ///
    upd_pos_y = Sl_track_begin;
    Reg_offset = 0;
    wheeldata = 0;
    wheelevent = true;
    rightbutton = true;

    if (pix_tumb.mask().isNull()) // vazno za mov evanta i release evanta mask().contains ima_niama_maska ???
        pix_tumb.setMask(pix_tumb.createHeuristicMask(true));

    if (pix_tumb_over.mask().isNull()) // vazno za mov evanta i release evanta mask().contains ima_niama_maska ???
        pix_tumb_over.setMask(pix_tumb_over.createHeuristicMask(true));

    Tregion = pix_tumb.mask();
    Tregion.translate(X_tumb_offset, upd_pos_y);  ///

    setPix_tumb(pix_tumb);
    update();
    SetValue(0);
}
HvSlider_V_Identif::~HvSlider_V_Identif()
{}
void HvSlider_V_Identif::SetThumbs(QPixmap pic_t, QPixmap pic_t_over)
{
	pix_tumb = pic_t;
    pix_tumb_over = pic_t_over;
}
void HvSlider_V_Identif::setPix_tumb(QPixmap pix)
{
    upd_tumb = pix;
    update(X_tumb_offset, Sl_track_begin, pix_tumb.width(), pix_sld_track.height()); ///
}
void HvSlider_V_Identif::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.drawPixmap(0, 0, pix_sld_up);
    painter.drawPixmap(0, pix_sld_up.height(), pix_sld_track);
    painter.drawPixmap(0, pix_sld_up.height() + pix_sld_track.height(), pix_sld_down);
    painter.drawPixmap(X_tumb_offset, upd_pos_y, upd_tumb);
}
void HvSlider_V_Identif::mousePressEvent ( QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        //MouseDragThumb(true);
        if (!(Tregion.contains(event->pos())))
        {
            Reg_offset = pix_tumb.height()/2 - MidDel;//    -MidDel moze i tuk HV za da zastava po sredata na delenieto!!!
            MoveSlider(event->y());
        }
        else
        {
            Reg_offset =(event->y()) - upd_pos_y - MidDel;//    -MidDel moze i tuk HV za da zastava po sredata na delenieto!!!
            setPix_tumb(pix_tumb_over);
        }
    }

    if (rightbutton == true)
    {
        if (event->button() == Qt::RightButton)
            Move(MaxValuein/2, false);
    }
}
void HvSlider_V_Identif::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        setPix_tumb(pix_tumb);
        //MouseDragThumb(false);
    }

    if (rightbutton == true)
    {
        if (event->button() == Qt::RightButton)
            setPix_tumb(pix_tumb);
    }
}
void HvSlider_V_Identif::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() == Qt::LeftButton)
    {
        MoveSlider(event->y()); ///
    }
}
////////////////////////////////////////////////////////////////////////
void HvSlider_V_Identif::MoveSlider(int pos)
{
    upd_pos_y = pos - Reg_offset ; //+MidDel moze i tuk HV za da zastava po sredata na delenieto!!! qDebug()<< "WWW";
    register unsigned int a;
    a=MaxValuein*((unsigned int)upd_pos_y - Sl_track_begin);
    a/=(unsigned int)Sl_track_range;

    if ((pos - Reg_offset) >= Sl_track_begin && (pos - Reg_offset) <= Sl_track_end)
    {
        Move(a, false);
    }
    if (pos - Reg_offset < Sl_track_begin )
    {
        Move(0, false);
    }
    if (pos - Reg_offset > Sl_track_end )
    {
        Move(MaxValuein, false);
    }
}
void HvSlider_V_Identif::Move(int val, bool mouseMove)
{
    register unsigned int b;
    b=(unsigned int)Sl_track_range*(unsigned int)val;
    b/=(unsigned int)MaxValuein;
    upd_pos_y = b + Sl_track_begin;
    Tregion = pix_tumb.mask();
    Tregion.translate(X_tumb_offset, upd_pos_y);

    if (mouseMove == true)
        setPix_tumb(pix_tumb);
    else
        setPix_tumb(pix_tumb_over);

    val = MaxValuein - val;  ///
    seve_val = val;
    emit SendValue(val, s_identif);
    wheeldata = val;    
}
////////////////////////////////////////////////////////////////////////////////////
void HvSlider_V_Identif::SetValue(int val)
{
    val = MaxValuein - val; ///
    if (val >= 0 && val <= MaxValuein)
    {
        Move(val, true);
    }
    if (val < 0 )
    {
        Move(0, true);
    }
    if (val > MaxValuein)
    {
        Move(MaxValuein, true);
    }
}
void HvSlider_V_Identif::wheelEvent(QWheelEvent *event)
{
    if (wheelevent == true)
    {
        //int Step = event->delta() / 120;
        int Step = event->angleDelta().y() / 120;//2.56  
        wheeldata = wheeldata + Step;
        SetValue(wheeldata);
    }
}
/*void HvSlider_V_Identif::MouseWheelEvent(bool event_b)
{
    wheelevent = event_b;
}
void HvSlider_V_Identif::MouseRightButtonEvent(bool event_b)
{
    rightbutton = event_b;
}*/


