#ifndef HVVTEXT_H
#define HVVTEXT_H

#include <QWidget>
#include <qpainter.h>

class HvVText : public QWidget
{
    Q_OBJECT
public:
    HvVText( QString text, QWidget * parent = 0 );
    virtual ~HvVText();
    
private:
    QString s_text;
    
protected:
    void paintEvent(QPaintEvent *);
};
#endif
