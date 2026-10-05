/* MSHV
 *
 * By Hrisimir Hristov - LZ2HV
 * May be used under the terms of the GNU General Public License (GPL)
 */

#ifndef HVHELPSKMS_H
#define HVHELPSKMS_H

#include <QWidget>
//#include <QDialog>
//#include <QVBoxLayout>
//#include <QLabel>
//#include <QDesktopWidget>
//#include <QApplication>
//#include <QIcon>
//#include <QByteArray>
//#include <QPixmap>
//#include <QtGui>
 
class HvHelpSkMs : public QWidget
{
    //Q_OBJECT //2.65 <- for tr() Q_OBJECT 
public:
    HvHelpSkMs(QString title,QString app_name, int lid,int,int,QWidget *parent = 0);
    virtual ~HvHelpSkMs();

//public slots:
    //void 


//private slots:
    //void SetText_en();
    //void SetText_bg();

//private:
    //QTextBrowser *text_browser;
    //QUrl *url_hv;
    //HvButton_Left2 *Bt_En;
    //HvButton_Left2 *Bt_Bg;


};
#endif
