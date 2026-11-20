#ifndef HVRBUTTON_H
#define HVRBUTTON_H

#include <QRadioButton>

class HvRbutton : public QRadioButton
{
    Q_OBJECT
public:
    HvRbutton(int index_ident, QRadioButton * parent = 0 );
    virtual ~HvRbutton();

    int get_value()
    {    	
        return isChecked();
    }
    ;
//void SetRbCaptureColor(bool flag);

    
    void SetValue(int);
signals:
    void SendVals(int,int);
private slots:
    void toggled_s(bool);
private:
    //int s_handle_ident;
    int s_index_ident;
    //int s_tipe;

};
#endif
