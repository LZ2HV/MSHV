/* MSHV AggressiveDialog
 * Copyright 2015 Hrisimir Hristov, LZ2HV
 * May be used under the terms of the GNU General Public License (GPL)
 */
#include "hvslider_h.h"
//
HvSlider_H::HvSlider_H(int MaxValue, int x_pos, int y_pos, QPixmap sld_left,
                       QPixmap sld_track, QPixmap sld_right, QPixmap tumb, QPixmap tumb_over, QWidget * parent)
        : QWidget(parent)
{
	saved_val = 0;
    MaxValuein = MaxValue;
    move(x_pos, y_pos);
    pix_sld_left = sld_left;
    pix_sld_track = sld_track;
    pix_sld_right = sld_right;
    pix_tumb = tumb;
    pix_tumb_over = tumb_over;
    setFixedSize((pix_sld_left.width()+pix_sld_track.width()+pix_sld_right.width()),
                 pix_sld_track.height());

    Y_tumb_offset =(pix_sld_track.height() - pix_tumb.height())/2;
    Sl_track_range = pix_sld_track.width() - pix_tumb.width();
    Sl_track_begin = pix_sld_left.width();
    Sl_track_end = Sl_track_begin + Sl_track_range;
    MidDel = pix_sld_track.width()/MaxValuein/2;
    upd_pos_x = Sl_track_begin;
    Reg_offset = 0;
    wheeldata = 0;
    wheelevent = true;
    rightbutton = true;

    if (pix_tumb.mask().isNull()) // vazno za mov evanta i release evanta mask().contains ima_niama_maska ???
        pix_tumb.setMask(pix_tumb.createHeuristicMask(true));

    if (pix_tumb_over.mask().isNull()) // vazno za mov evanta i release evanta mask().contains ima_niama_maska ???
        pix_tumb_over.setMask(pix_tumb_over.createHeuristicMask(true));

    Tregion = pix_tumb.mask();
    Tregion.translate(upd_pos_x, Y_tumb_offset);

    setPix_tumb(pix_tumb);
    update();
    SetValue(0);
}
HvSlider_H::~HvSlider_H()
{}
void HvSlider_H::setPix_tumb(QPixmap pix)
{
    upd_tumb = pix;
    update(Sl_track_begin, Y_tumb_offset, pix_sld_track.width(), pix_tumb.height());
}
void HvSlider_H::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.drawPixmap(0, 0, pix_sld_left);
    painter.drawPixmap(pix_sld_left.width(), 0, pix_sld_track);
    painter.drawPixmap(pix_sld_left.width() + pix_sld_track.width(), 0, pix_sld_right);
    painter.drawPixmap(upd_pos_x, Y_tumb_offset, upd_tumb);
}
void HvSlider_H::mousePressEvent ( QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        //MouseDragThumb(true);
        if (!(Tregion.contains(event->pos())))
        {
            Reg_offset = pix_tumb.width()/2 - MidDel;//    -MidDel moze i tuk HV za da zastava po sredata na delenieto!!!
            MoveSlider(event->x());
        }
        else
        {
            Reg_offset = event->x() - upd_pos_x - MidDel;//    -MidDel moze i tuk HV za da zastava po sredata na delenieto!!!
            setPix_tumb(pix_tumb_over);
        }
    }

    if (rightbutton == true)
    {
        if (event->button() == Qt::RightButton)
            Move(MaxValuein/2, false);
    }
}
void HvSlider_H::mouseReleaseEvent(QMouseEvent *event)
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
void HvSlider_H::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() == Qt::LeftButton)
    {
        MoveSlider(event->x());
    }
}
////////////////////////////////////////////////////////////////////////
void HvSlider_H::MoveSlider(int pos)
{
    upd_pos_x = pos - Reg_offset ; //+MidDel moze i tuk HV za da zastava po sredata na delenieto!!! qDebug()<< "WWW";
    register unsigned int a;
    a=MaxValuein*((unsigned int)upd_pos_x - Sl_track_begin);
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
void HvSlider_H::Move(int val, bool mouseMove)
{
    register unsigned int b;
    b=(unsigned int)Sl_track_range*(unsigned int)val;
    b/=(unsigned int)MaxValuein;
    upd_pos_x = b + Sl_track_begin;
    Tregion = pix_tumb.mask();
    Tregion.translate(upd_pos_x, Y_tumb_offset);

    if (mouseMove == true)
        setPix_tumb(pix_tumb);
    else
        setPix_tumb(pix_tumb_over);

    emit SendValue(val);
    wheeldata = val;
    saved_val = val;
}
////////////////////////////////////////////////////////////////////////////////////
void HvSlider_H::SetValue(int val)
{
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
void HvSlider_H::wheelEvent(QWheelEvent *event)
{
    if (wheelevent == true)
    {
        //int Step = event->delta() / 120;
        int Step = event->angleDelta().y() / 120;//2.56  
        wheeldata = wheeldata + Step;
        SetValue(wheeldata);
    }
}
/*void HvSlider_H::MouseWheelEvent(bool event_b)
{
    wheelevent = event_b;
}
void HvSlider_H::MouseRightButtonEvent(bool event_b)
{
    rightbutton = event_b;
}*/


