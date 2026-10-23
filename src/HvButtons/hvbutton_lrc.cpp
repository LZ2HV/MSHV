#include "hvbutton_lrc.h"
HvButtonLeftRightClick::HvButtonLeftRightClick(const QString &text, QWidget *parent)
        : QPushButton(parent)
{
    setText(text); //connect(this,SIGNAL(clicked(bool)),this,SIGNAL(lclicked(bool)));
}
HvButtonLeftRightClick::~HvButtonLeftRightClick()
{}
void HvButtonLeftRightClick::mousePressEvent ( QMouseEvent * event)
{
    QPushButton::mousePressEvent(event);
    if (event->button() == Qt::LeftButton ) emit lclicked();
    if (event->button() == Qt::RightButton) emit rclicked();
}



