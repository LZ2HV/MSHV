/* MSHV
 *
 * By Hrisimir Hristov - LZ2HV
 * May be used under the terms of the GNU General Public License (GPL)
 */

#ifndef HVALLTXT_H
#define HVALLTXT_H

#include <QObject>
#include <QFile>
#include <QStringList>
#include <QTextStream>
#include <QTimer>

class AllTxt : public QObject
{
    Q_OBJECT
public:
    AllTxt(QString path,QObject * parent = 0);
    virtual ~AllTxt();  
    
    void SaveAllTxt(QString yyyy_mm);
    void ReadAllTxt(QString yyyy_mm);
    void SetTxt(QString yyyy_mm_dd,QString str);
       
//public slots:

//signals:

private slots:
    void RefreshSaveTimer();

private:
    int beg_append;
    QList<QString>alltxt_data_buf;
    QList<QString>alltxt_data;
    QString App_Path;
    QString s_yyyy_mm;
    QString s_yyyy_mm_dd;
    QTimer *alltxt_save_timer;
    void SetTxt_p(QString yyyy_mm_dd);
	
//protected:

};
#endif