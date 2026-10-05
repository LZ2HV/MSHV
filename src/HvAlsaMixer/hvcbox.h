#ifndef HVCBOX_H
#define HVCBOX_H

#include <QComboBox>

class HvCBox : public QComboBox
{
    Q_OBJECT
public:
    HvCBox( int index_ident, QComboBox *parent = 0 );
    virtual ~HvCBox();

    int get_value()
    {
        return currentIndex();
    }
    ;
    void SetValue(int);

signals:
    void SendVals(int, int);

private slots:
    void DeviceChanged(QString);

private:
    int s_index_ident;
};
#endif



