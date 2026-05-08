/* MSHV MsDb
 * Copyright 2015 Hrisimir Hristov, LZ2HV
 * May be used under the terms of the GNU General Public License (GPL)
 */
#include "hvmsdb.h"
//#include <QtGui>

MsDb::MsDb(QString path, QObject * parent )
        : QObject(parent)
{
    App_Path = path;

    sr_path = App_Path+"/settings/database/msloc_db.dbmh";

    ReadDb(sr_path,false);

    //q_Sort(db_data.begin().at(0), db_data.end().at(0));
    //qDebug()<<db_data.count();
    //qDebug()<<db_data.at(db_data.count()-1).at(0);
}

MsDb::~MsDb()
{
    //SaveDb();  2.35 stop close error no needed
    //qDebug()<<"save";
}
void MsDb::Refr65DeepSearchDb()//1.49 deep search 65
{
    QStringList list;
    QString dupe_call = "NO_7224_DupeC_identif";
    for (int j=0;  j < db_data.count(); ++j)
    {
    	if(db_data.at(j).count()<2) continue;// no locator
    	QString str_call = db_data.at(j).at(0);    	
        if (str_call.contains("/")) continue;// remove all / for jt65 deep decode
        if (str_call == dupe_call) continue;// no dule calls
            dupe_call = str_call;
            	
        QString str_loc4 = db_data.at(j).at(1).mid(0,4);// 4char loc for jt65 deep decode 
        QString str = str_call+","+str_loc4;
        list.append(str);
    }
    emit Emit65DeepSearchDb(list);
}
void MsDb::SetToDbNew(QStringList list)
{
    for (int j=0;  j < db_data.count(); ++j)
    {
        if (db_data.at(j).at(0)==list.at(0))
        {
            db_data.replace(j,list);
            goto c100;
        }
    }
    db_data.append(list);

c100:
    SaveDb();
    Refr65DeepSearchDb();
    // qDebug()<<list_new;
}
QString MsDb::CheckBD(QString str)
{
    QString res = "";
    for (int j=0;  j < db_data.count(); ++j)
    {
        if (db_data.at(j).at(0)==str)
        {
            res = db_data.at(j).at(1);
            //qDebug()<<res<<str<<db_data.count();
            return res;
        }
    }
    return res;
}
void MsDb::SetNewDb(QString file)
{
    if (!file.isEmpty())
    {
        db_data.clear();
        ReadDb(file, true);
        Refr65DeepSearchDb();
    }
}
void MsDb::ReadDb(QString path_and_file, bool f_save)
{
    QFile file(path_and_file);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        file.setFileName(App_Path+"/settings/database/msloc_db");
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
            return;
        else
            f_save = true;
    }

    QStringList list;

    QTextStream in(&file);
    while (!in.atEnd())
    {
        QString str = in.readLine();
        if (!str.isEmpty())
        {
        	if (str.mid(0,2)=="//")	  continue;//2.85  //
        	if (str.mid(0,4)=="ZZZZ") continue;//ZZZZ
        	QStringList list_tt = str.split(",");
        	if (list_tt.count() > 1)
            {
           		str = list_tt.at(0)+","+list_tt.at(1);
            	list.append(str);           	
           	}	
       	}            
    }
    //q_Sort(list.begin(), list.end());
    std::sort(list.begin(),list.end()); //qDebug()<<list;
    Insert_full_model(list);

    file.close();

    if (f_save)
        SaveDb();
}
void MsDb::Insert_full_model(QStringList list)
{
    if (!list.isEmpty())
    {
        QStringList list_t;
        QString line;
        for (QStringList::iterator it =  list.begin(); it != list.end(); ++it)
        {
            line = QString(*it);
            list_t = line.split(",");
 
            /*if (list_t.count() > 1)
            {
            	QStringList list_tt;
           		list_tt << list_t.at(0) << list_t.at(1);
            	db_data.append(list_tt);            	
           	}
            list_t.clear();*/
                        
            // org procedure to add BD            	
            db_data.append(list_t);
            list_t.clear();
            //end org procedure to add BD                 
        }
    }
}
void MsDb::SaveDb()
{
    QFile file(sr_path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    QTextStream out(&file);
    QString dupe_call = "NO_7224_DupeC_identif";
    
    for (int j=0;  j < db_data.count(); ++j)
    {
    	 if (db_data.at(j).at(0) == dupe_call) continue;// no save dule calls
             dupe_call = db_data.at(j).at(0);
             
        for (int i=0; i < db_data.at(j).count(); ++i)
        {
            if ( i < (db_data.at(j).count()-1))
                out << db_data.at(j).at(i) <<",";
            else
                out << db_data.at(j).at(i);
        }
        out<<"\n";
    }
    file.close();
    //qDebug()<<"save";
}