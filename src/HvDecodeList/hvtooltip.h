/* MSHV ToolTipWidget
 *
 * By Hrisimir Hristov - LZ2HV
 * May be used under the terms of the GNU General Public License (GPL)
 */
#ifndef HVTOOLTIP_H
#define HVTOOLTIP_H

#include <QTimer>
#include <QLabel>
#include <QEvent>

class HvToolTip : public QLabel
{
    Q_OBJECT
public:
    HvToolTip(QWidget *w = 0);
    ~HvToolTip();
    void showText(QPoint,QString,int,QWidget *w = Q_NULLPTR);

private slots:
    void Slot_stop();

private:
    QWidget *widget;
    QRect rect;
    //int getTipScreen(const QPoint &pos, QWidget *w);
    void placeTip(const QPoint &pos, QWidget *w);
    void hideTip();
    void hideTipImmediately();

protected:
	bool tim_is_active;
    QTimer *tim_stop;
    bool eventFilter(QObject *o, QEvent *e);
    //void paintEvent(QPaintEvent *);

};
#endif
