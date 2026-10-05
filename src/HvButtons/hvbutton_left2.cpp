#include "hvbutton_left2.h"
//
HvButton_Left2::HvButton_Left2(QWidget *parent)
        : QWidget(parent)
{
    //setCursor(Qt::PointingHandCursor);
	//connect( this, SIGNAL(Press_Lift_Button_hv()), this, SLOT( slot_Press_Button_2()));
	//connect( this, SIGNAL(Release_Lift_Button_hv()), this, SLOT( slot_Release_Button_2()));
	f_outin = false;//2.68
    Button_Release_Left = true;
    Mouse_Over_Button = true;   
}
void HvButton_Left2::SetupButton_hv(QPixmap release_stop,QPixmap press_stop,int x_pos,int y_pos,bool sb)
{
	f_outin = sb;//2.66
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
HvButton_Left2::~HvButton_Left2()
{}
//////////////////////////Update///////////////////////////////////////////
void HvButton_Left2::setPixmap_hv(QPixmap pixmap_hv)
{
    pupdate_hv = pixmap_hv;
    update();
}
void HvButton_Left2::paintEvent(QPaintEvent *)
{
    QPainter painter( this );
    painter.drawPixmap(0, 0, pupdate_hv);
    if (isEnabled() == false)
        painter.fillRect(0, 0, pupdate_hv.width(), pupdate_hv.height(), QColor(150,150,150,100));
}
//////////////////////////Update///////////////////////////////////////////
void HvButton_Left2::mousePressEvent ( QMouseEvent * event)
{
    if (mask().contains(event->pos()))   // i tuk triabva left right izvan left left  da ne se slu4va ni6to !!!!
    {
        if (event->button() == Qt::LeftButton)
        {
        	setPixmap_hv(press_butt_stop);
            emit Press_Lift_Button_hv(); 
            Button_Release_Left = false;
        }
    }
}
void HvButton_Left2::mouseReleaseEvent ( QMouseEvent * event)
{
    if (mask().contains(event->pos()))
    {
        if (event->button() == Qt::LeftButton && Button_Release_Left == false)
        {
            setPixmap_hv(release_butt_stop);        	
            emit Release_Lift_Button_hv();         	
       	}
    }
    if (event->button() == Qt::LeftButton)
        Button_Release_Left = true;
}
void HvButton_Left2::mouseMoveEvent(QMouseEvent *event)
{
    if (Mouse_Over_Button == true)    // za da se slu4va samo vednaz hv
    {
        if (Button_Release_Left == false)
        {
            if (!mask().contains(event->pos()))
            {	//qDebug()<<  "Izleze1";
                //setPixmap_hv(release_butt_stop);
                Mouse_Over_Button  = false;
                setPixmap_hv(release_butt_stop);
                if (f_outin) emit Release_Lift_Button_hv();//HV special if button use timer to stop, example spinbox                
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
                //setPixmap_hv(press_butt_stop);
                Mouse_Over_Button = true;
                setPixmap_hv(press_butt_stop);
                if (f_outin) emit Press_Lift_Button_hv();//HV special if button use timer to stop, example spinbox                
            }
        }
    }
}
////////////////////////////////////////////////////////////////////////
/*void HvButton_Left2::slot_Press_Button_2()
{
    setPixmap_hv(press_butt_stop);
}

void HvButton_Left2::slot_Release_Button_2()
{
    setPixmap_hv(release_butt_stop);  
}*/
//


