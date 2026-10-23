#include "hvrbutton.h"

HvRbutton::HvRbutton(int index_ident, QRadioButton * parent  )
        : QRadioButton(parent)
{
    setFixedSize(14,14);
    setContentsMargins(0,0,0,0);
    setAutoExclusive(false);
    //s_handle_ident = handle_ident;
    s_index_ident = index_ident;
    //s_tipe = tipe;

    connect(this, SIGNAL(toggled(bool)), this, SLOT(toggled_s(bool)));
}

HvRbutton::~HvRbutton()
{}
/*
void HvRbutton::SetRbCaptureColor(bool flag)
{
    QPalette palette1 = palette();
    if (flag)
        palette1.setColor(QPalette::Text, QColor(255,0,0));
    else
        palette1.setColor(QPalette::Text, QColor(0,255,0));
    setPalette(palette1);
}
*/
void HvRbutton::toggled_s(bool flag)
{
    if (flag)
        SendVals(1,s_index_ident);
    else
        SendVals(0,s_index_ident);
}

void HvRbutton::SetValue(int val)
{
    if (val == 0)
        setChecked(false);
    else
        setChecked(true);
}


