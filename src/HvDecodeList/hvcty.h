/* MSHV HVCTY
 * Copyright 2020 Hrisimir Hristov, LZ2HV
 * May be used under the terms of the GNU General Public License (GPL)
 */
#ifndef HVCTY_H
#define HVCTY_H

//#include <QObject>
#include <QString>
#include <QApplication>
#include <QFile>
#include <QStringList>
//#include <QList>

//#include "../config.h"
//#include "../HvTxW/hvqthloc.h"

class HvCty //: public QObject
{
    //Q_OBJECT
public:
    HvCty(int);
    ~HvCty();
	QString FindCountry(QString,bool);    
	bool FilndDbPfx(QString,QString &,QString &,QString &);
    bool HideContinent(QString,bool fh[8],bool,QString);
    bool HideCountry(QString,QStringList,QString);//bool,
    bool ShowCNYDecode(QString,QStringList,bool,QString);
    bool ShowPFXDecode(QString,QStringList,bool,QString);
    QStringList GetCountries();

private:
    void ReadCtyDat();

protected:

};
#endif



