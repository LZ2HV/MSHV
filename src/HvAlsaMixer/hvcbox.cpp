#include "hvcbox.h"

HvCBox::HvCBox( int index_ident, QComboBox *parent )
        : QComboBox( parent )
{
    s_index_ident = index_ident;
    setFixedHeight(18);
  /*  QFont font_t(font());
    font_t.setPixelSize(10);
    setFont(font_t);*/
    connect(this, SIGNAL(currentIndexChanged(QString)), this, SLOT(DeviceChanged(QString)));
}

HvCBox::~HvCBox()
{}

void HvCBox::SetValue(int idx)
{
    setCurrentIndex(idx);
}

void HvCBox::DeviceChanged(QString)
{
    SendVals(s_index_ident, currentIndex());
}





