/* MSHV
 * HvButtonLeftRightClick
 * By Hrisimir Hristov - LZ2HV
 * May be used under the terms of the GNU General Public License (GPL)
 */
#ifndef HVBUTTON_LEFTRIGHTC
#define HVBUTTON_LEFTRIGHTC

#include <QWidget>
#include <QPushButton>
#include <QMouseEvent>

class HvButtonLeftRightClick : public QPushButton
{
    Q_OBJECT
public:
    HvButtonLeftRightClick(const QString &text, QWidget *parent = 0);
    virtual ~HvButtonLeftRightClick();

signals:
    void lclicked();
    void rclicked();

private:

protected:
    void mousePressEvent ( QMouseEvent * event);

};
#endif
