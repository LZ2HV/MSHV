#include "hvvtext.h"

HvVText::HvVText( QString text, QWidget * parent )
        : QWidget(parent)
{
    s_text = text;
    setFixedSize(11, 140);
    setContentsMargins(0,0,0,0);
    QFont font_tfi = font();
    font_tfi.setPointSize(9);
    setFont(font_tfi);
}

HvVText::~HvVText()
{}

void HvVText::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.save();
    painter.translate(width() / 2+4, height()-5);
    painter.rotate(270);
    painter.drawText(0, 0, s_text);
    painter.restore();
}


