/* MSHV
 *
 * By Hrisimir Hristov - LZ2HV
 * May be used under the terms of the GNU General Public License (GPL)
 */

#ifndef HVHELPMS_H
#define HVHELPMS_H

#include <QWidget>
//#include <QDialog>
//#include <QDesktopWidget>
//#include <QApplication>
//#include <QIcon>
//#include <QTextBrowser>
#include <QVBoxLayout>
#include <QLabel>

//#include "../HvButtons/hvbutton_left2.h"
//#include <QUrl>

class HvHelpMs : public QWidget 
{
	//Q_OBJECT //2.65 <- for tr() Q_OBJECT 
public:  
	HvHelpMs(QString title,QString app_name,QString path,int lid,int,int,QWidget *parent = 0);
	virtual ~HvHelpMs();

public:
	QString GetPosXY()
	{
		return QString("%1").arg(pos().x())+"#"+QString("%1").arg(pos().y());				
	}
	void SetPosXY(QString);
 
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
