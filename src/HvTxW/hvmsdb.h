/* MSHV MsDb
 * Copyright 2015 Hrisimir Hristov, LZ2HV
 * May be used under the terms of the GNU General Public License (GPL)
 */

#ifndef HVMSDB_H
#define HVMSDB_H

#include <QObject>
#include <QStringList>
#include <QFile>
#include <QTextStream>
//#include <QtGui>

class MsDb : public QObject
{
    Q_OBJECT
public:
    MsDb(QString path,QObject * parent = 0);
    virtual ~MsDb();
    
    QString CheckBD(QString str);
    void SetToDbNew(QStringList list);
    void SetNewDb(QString);
    void Refr65DeepSearchDb();//1.49 deep search 65
    
//public slots:

signals:
    void Emit65DeepSearchDb(QStringList);//1.49 deep search 65

//private slots:



private:
    QList<QStringList > db_data;
    QString App_Path;
    void ReadDb(QString path_and_file, bool f_save);
    void Insert_full_model(QStringList list);
    QString sr_path;
    void SaveDb();


protected:


};
#endif