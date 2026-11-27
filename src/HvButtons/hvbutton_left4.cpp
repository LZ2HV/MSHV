#include "hvbutton_left4.h"
//
HvButton_Left4::HvButton_Left4( QWidget * parent )
        : QWidget(parent)
{
    //setCursor(Qt::PointingHandCursor);
    //connect( this, SIGNAL(Press_Lift_Button_hv()), this, SLOT( slot_Press_Button_4()));
    //connect( this, SIGNAL(Release_Lift_Button_hv()), this, SLOT( slot_Release_Button_4()));
    Button_Release_Left = true;
    Button_Stop_b = true;
    Mouse_Over_Button = true;
    //setToolTip(tr("Change Display Size"));
}
void HvButton_Left4::SetupButton_hv(QPixmap release_stop, QPixmap press_start, QPixmap release_start, QPixmap press_stop, int x_pos, int y_pos)
{
    release_butt_start = release_start;
    press_butt_start = press_start;
    release_butt_stop = release_stop;
    press_butt_stop = press_stop;

    setFixedSize( release_butt_stop.width(), release_butt_stop.height() );
    move(x_pos, y_pos);

    if (release_butt_stop.mask().isNull()) // vazno za mov evanta i release evanta mask().contains ima_niama_maska ???
        setMask(release_butt_stop.rect());
    else
        setMask(release_butt_stop.mask()); // raboti samo ako ima ++alpha chanel (butona ima nepravilna forma)!!!

    setPixmap_hv(release_butt_stop);
}
HvButton_Left4::~HvButton_Left4()
{}
//////////////////////////Update///////////////////////////////////////////
void HvButton_Left4::setPixmap_hv(QPixmap pixmap_hv)
{
    pupdate_hv = pixmap_hv;
    update();
}
void HvButton_Left4::paintEvent(QPaintEvent *)
{
    QPainter painter( this );
    painter.drawPixmap(0, 0, pupdate_hv);
    if (isEnabled() == false)
        painter.fillRect(0, 0, pupdate_hv.width(), pupdate_hv.height(), QColor(150,150,150,100));
}
//////////////////////////Update///////////////////////////////////////////
void HvButton_Left4::mousePressEvent ( QMouseEvent * event)
{
    if (mask().contains(event->pos()))   // i tuk triabva left right izvan left left  da ne se slu4va ni6to !!!!
    {
        if (event->button() == Qt::LeftButton)
        {
        	p_Press_Button_4();
            emit Press_Lift_Button_hv();    
            Button_Release_Left = false;
        }
    }
}
void HvButton_Left4::ExtrnalRelease()
{
	p_Release_Button_4();
    emit Release_Lift_Button_hv();   
    Button_Release_Left = true;
}
void HvButton_Left4::mouseReleaseEvent ( QMouseEvent * event)
{
    if (mask().contains(event->pos()))
    {
        if (event->button() == Qt::LeftButton && Button_Release_Left == false)
        {
        	p_Release_Button_4();
        	emit Release_Lift_Button_hv();
       	}
    }
    if (event->button() == Qt::LeftButton)
        Button_Release_Left = true;
}
void HvButton_Left4::mouseMoveEvent(QMouseEvent *event)
{
    if (Mouse_Over_Button == true)    // za da se slu4va samo vednaz hv
    {
        if (Button_Release_Left == false)
        {
            if (!mask().contains(event->pos()))
            {	 //qDebug()<<  "Izleze1";
                if (Button_Stop_b == false)
                    setPixmap_hv(release_butt_start);
                else
                    setPixmap_hv(release_butt_stop);
                Mouse_Over_Button  = false;
            }
        }
    }
////////////////////////////////////////////////////////////////////////////////////
    if (Mouse_Over_Button == false)     // za da se slu4va samo vednaz hv
    {
        if (Button_Release_Left == false)
        {
            if (mask().contains(event->pos()))
            {	//qDebug()<<  "Vleze1";
                if (Button_Stop_b == true)
                    setPixmap_hv(press_butt_start);
                else
                    setPixmap_hv(press_butt_stop);
                Mouse_Over_Button = true;
            }
        }
    }
}
////////////////////////////////////////////////////////////////////////
void HvButton_Left4::p_Press_Button_4()
{
    if (Button_Stop_b == true)
        setPixmap_hv(press_butt_start);
    else
        setPixmap_hv(press_butt_stop);
}
void HvButton_Left4::p_Release_Button_4()
{
    if (Button_Stop_b == true)
    {
        setPixmap_hv(release_butt_start);
        Button_Stop_b = false;
    }
    else
    {
        setPixmap_hv(release_butt_stop);
        Button_Stop_b = true;
    }
}
/*void HvButton_Left4::slot_Press_Button_4()
{
    if (Button_Stop_b == true)
        setPixmap_hv(press_butt_start);
    else
        setPixmap_hv(press_butt_stop);
}
void HvButton_Left4::slot_Release_Button_4()
{
    if (Button_Stop_b == true)
    {
        setPixmap_hv(release_butt_start);
        Button_Stop_b = false;
    }
    else
    {
        setPixmap_hv(release_butt_stop);
        Button_Stop_b = true;
    }
}*/
/////////////////////////////////////////////////////////////////
void HvButton_Left4::Release_Button_hv()
{
    setPixmap_hv(release_butt_stop);
    Button_Stop_b = true;
}
void HvButton_Left4::Press_Button_hv()
{
    setPixmap_hv(release_butt_start);
    Button_Stop_b = false;
}
//


