/* MSHV AllTxt
 * Copyright 2015 Hrisimir Hristov, LZ2HV
 * May be used under the terms of the GNU General Public License (GPL)
 */
#include "hvalltxt.h"
//#include <QtGui>

AllTxt::AllTxt(QString path, QObject * parent )
        : QObject(parent)
{
    App_Path = path;
    s_yyyy_mm   = "NONE";
    s_yyyy_mm_dd= "NONE";
    beg_append = 0;
    alltxt_save_timer = new QTimer();
    connect(alltxt_save_timer, SIGNAL(timeout()), this, SLOT(RefreshSaveTimer()));
    //q_Sort(db_data.begin().at(0), db_data.end().at(0));
    //qDebug()<<db_data.count();
    //qDebug()<<db_data.at(db_data.count()-1).at(0);
}
AllTxt::~AllTxt()
{
    //SaveDb();
    //qDebug()<<"save";
}
void AllTxt::ReadAllTxt(QString yyyy_mm)
{
    s_yyyy_mm = yyyy_mm;//vazno pri parvo otvariane

    //AllTxtPerMonth
    QFile file(App_Path+"/AllTxtMonthly/ALL_"+yyyy_mm+".TXT");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    //qDebug()<<file_identif;
    QStringList list;
    //QString line;

    QTextStream in(&file);
    while (!in.atEnd())
    {
        alltxt_data.append(in.readLine());
    }
    beg_append = alltxt_data.count();
    file.close();
}
void AllTxt::SetTxt(QString yyyy_mm_dd,QString str)
{
    if (!yyyy_mm_dd.isEmpty() && !str.isEmpty())
    {
        s_yyyy_mm_dd = yyyy_mm_dd;
        str.remove("\n");//zaradi JT6M
        
        //alltxt_data_buf.append(str); 
        /*QString yyyymmdd = yyyy_mm_dd;
        yyyymmdd.replace("_","");
        alltxt_data_buf.append(yyyymmdd+"|"+str);*/
        alltxt_data_buf.append(yyyy_mm_dd.mid(8,2)+"|"+str);
        
        //qDebug()<<"Start";
        alltxt_save_timer->start(3600);//2.48 from=2000 to 3600
    }
}
void AllTxt::RefreshSaveTimer()
{
    alltxt_save_timer->stop();
    //qDebug()<<"Start save============";
    SetTxt_p(s_yyyy_mm_dd);
    //qDebug()<<"STOP save=============";
}
void AllTxt::SetTxt_p(QString yyyy_mm_dd)
{
    if (!yyyy_mm_dd.isEmpty() && alltxt_data_buf.count()>0)
    {
        QString yyyy_mm = yyyy_mm_dd.mid(0,7);

        if (s_yyyy_mm != yyyy_mm) 
        {
            SaveAllTxt(s_yyyy_mm);
            alltxt_data.clear();
            s_yyyy_mm = yyyy_mm;
        }
        yyyy_mm_dd.replace("_"," ");
        bool f_ymd = false;
        for (int i = 0; i<alltxt_data.count(); i++)
        {
            if (alltxt_data.at(i)=="UTC Date: "+yyyy_mm_dd)
            {
                f_ymd = true;
                break;
            }
        }
        
        beg_append = alltxt_data.count();
        if (!f_ymd)
        {
            alltxt_data.append("--------------------");
            alltxt_data.append("UTC Date: "+yyyy_mm_dd);
            alltxt_data.append("--------------------");
        }
        for (int j = 0; j<alltxt_data_buf.count(); ++j)
            alltxt_data.append(alltxt_data_buf.at(j));

        //qDebug()<<"1save"<<alltxt_data_buf.count()<<alltxt_data.count()-beg_append;
        alltxt_data_buf.clear();
        //qDebug()<<"2save"<<s_yyyy_mm;
        SaveAllTxt(s_yyyy_mm);
    }
}
void AllTxt::SaveAllTxt(QString yyyy_mm)
{
    if (alltxt_data.count()>0)//no save empty file
    {
        QString t_yyyy_mm = yyyy_mm;

        if (s_yyyy_mm != t_yyyy_mm)
            t_yyyy_mm = s_yyyy_mm;

        //AllTxtPerMonth
        QFile file(App_Path+"/AllTxtMonthly/ALL_"+t_yyyy_mm+".TXT");//if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        if (!file.open(QIODevice::Append | QIODevice::Text))
            return;

        QTextStream out(&file);

        for (int j=beg_append;  j < alltxt_data.count(); j++)
        {
            out << alltxt_data.at(j);
            //if (j<alltxt_data.count()-1)//1.71 no need if append no emoty last row
            out<<"\n";
        }
        file.close();
        //beg_append = alltxt_data.count();
    }
}