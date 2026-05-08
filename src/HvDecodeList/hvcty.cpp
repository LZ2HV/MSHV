/* MSHV HVCTY
 * Copyright 2020 Hrisimir Hristov, LZ2HV
 * May be used under the terms of the GNU General Public License (GPL)
 */
#include "hvcty.h"
#include <QTextStream>

//#include <QtGui>

//// statis for 2 lists /////
typedef struct
{
    QString pfx;
    QString country;
    QString continent;
    //int id_cname;
}
recordd;
static QList<recordd>db_cty_p;
static QList<recordd>db_cty_c;
static QStringList _lcountries_;
bool hv_sort_disorder_(const recordd &d1,const recordd &d2)
{
    if (d1.pfx.count() == d2.pfx.count()) return d1.country.count() < d2.country.count(); //2.69 new equal calls, not /
    else return d1.pfx.count() > d2.pfx.count(); //2.69 old
}
/*bool hv_sort_00_(const recordd &d1,const recordd &d2)
{
    if (d1.pfx == d2.pfx) return d1.country.count() < d2.country.count();
    else return d1.pfx < d2.pfx; //by names
}*/
//// end statis for 2 lists /////
/*
void debuglst()
{
    //for (int i = 0; i < db_cty.count(); ++i)
        //qDebug()<<"e="<<db_cty.at(i).pfx<<db_cty.at(i).name;
    qDebug()<<"count_lines"<<db_cty.count();
}
*/
HvCty::HvCty(int id_n)//QObject *parent
//: QObject(parent)
{
    if (id_n==1) ReadCtyDat();//only 1 list refresh cty.dat
}
HvCty::~HvCty()
{/*qDebug()<<"DELETE";*/
}
QStringList HvCty::GetCountries()
{
    return _lcountries_;
}
void HvCty::ReadCtyDat()
{
    QString path = (QCoreApplication::applicationDirPath());
    path.append("/settings/database/cty.dat");
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) return;
    QTextStream in(&file);
    QString line = "";
    //int c_lines = 0;
    while (!in.atEnd())
    {
        line.append(in.readLine());
        if (!line.endsWith(';')) continue;
        line = line.mid(0,line.count()-1);//remove ;
        QStringList lst = line.split(":");
        line.clear();
        int iclst = lst.count();
        if (iclst<8) continue; //{c_lines++; continue;}
        recordd trec;
        //QList<recordd>tall_c;//2.69
        QString sppfx = lst.at(7).trimmed().toUpper();
        sppfx.remove("*");//*TA1 *4U1V ... ???
        QString scoun = lst.at(0).trimmed();
        QString scont = lst.at(3).trimmed().toUpper();
        if (!sppfx.contains('/'))//2.69 save scoun, scont and continue may-be have individual calls, exception "FO/AC4LN/A"
            //if (!sppfx.contains('/') && sppfx.at(0)!='*')
        {
            trec.pfx = sppfx;
            trec.country = scoun;
            trec.continent = scont;
            db_cty_p.append(trec);
        }
        _lcountries_.append(scoun);
        //c_lines++;
        if (iclst<9) continue;//{c_lines++; continue;}//3Y[73],=7S8AAA(38)[67],
        //c_lines = 1;
        QString stt = lst.at(8);
        QStringList ltt = stt.split(",");
        for (int j = 0; j < ltt.count(); ++j)
        {
            QString sttj = ltt.at(j).trimmed().toUpper();
            int ib1 = sttj.indexOf('(');
            if (ib1>-1)
            {
                int ib2 = sttj.indexOf(')',ib1+1);
                if (ib2>-1)
                {
                    QString rmv = sttj.mid(ib1,ib2-ib1+1);
                    sttj.remove(rmv);//cq zone
                }
            }
            ib1 = sttj.indexOf('[');
            if (ib1>-1)
            {
                int ib2 = sttj.indexOf(']',ib1+1);
                if (ib2>-1)
                {
                    QString rmv = sttj.mid(ib1,ib2-ib1+1);
                    sttj.remove(rmv);//itu zone
                }
            }
            if (sttj == sppfx) continue;//{if (sttj.contains('/')) c_lines++; continue;}// exsist in primary pfx
            if (sttj.at(0)=='=')
            {
                //if (sttj.contains('/')) continue; //2.69 stop individual cals with '/'
                sttj.remove('=');
                /*bool found = false;
                for (int x = 0; x < tall_c.count(); ++x)
                {   //if (sttj=="RL1O") qDebug()<<tall_c.at(x).pfx<<sttj<<tall_c.at(x).country<<scoun;
                    if (tall_c.at(x).pfx==sttj)
                    {
                        if (tall_c.at(x).country==scoun)
                        {
                            found = true; //qDebug()<<tall_c.at(x).pfx<<tall_c.at(x).country<<tall_c.at(x).continent<<scont;
                            break;
                        }
                    }
                }
                if (found) continue;//{c_lines++; continue;}*/
                trec.pfx = sttj;
                trec.country = scoun;
                trec.continent = scont;
                db_cty_c.append(trec);
                //tall_c.append(trec);//2.69
                continue;
            }
            /*bool found = false;
            for (int x = 0; x < tallpfx.count(); ++x)
            {   //if (sttj=="RL1O") qDebug()<<tallpfx.at(x).pfx<<sttj<<tallpfx.at(x).country<<scoun;
                if (tallpfx.at(x).pfx==sttj)
                {
                    if (tallpfx.at(x).country==scoun)
                    {
                        found = true; //qDebug()<<tallpfx.at(x).pfx<<tallpfx.at(x).country<<tallpfx.at(x).continent<<scont;
                        break;
                    }
                }
            }
            if (found) continue;//{c_lines++; continue;}*/
            trec.pfx = sttj;
            trec.country = scoun;
            trec.continent = scont;
            db_cty_p.append(trec);
            //c_lines++;
        }
        //if (c_lines>1) qDebug()<<c_lines<<scoun;
    }
    file.close();

    std::sort(db_cty_c.begin(),db_cty_c.end(),hv_sort_disorder_);
    std::sort(db_cty_p.begin(),db_cty_p.end(),hv_sort_disorder_);
    std::sort(_lcountries_.begin(),_lcountries_.end());

    /*QStringList l;
       for (int i = 0; i < db_cty_c.count(); ++i)
       {
           QString res = db_cty_c.at(i).country;
           res.replace ("Islands", "Is.");
           res.replace ("Island", "Is.");
           res.replace ("North ", "N. ");
           res.replace ("Northern ", "N. ");
           res.replace ("South ", "S. ");
           res.replace ("East ", "E. ");
           res.replace ("Eastern ", "E. ");
           res.replace ("West ", "W. ");
           res.replace ("Western ", "W. ");
           res.replace ("Central ", "C. ");
           res.replace (" and ", " & ");
           res.replace ("Republic", "Rep.");
           res.replace ("United States of America", "U.S.A.");
           res.replace ("United States", "U.S.A.");
           res.replace ("Fed. Rep. of ", "");
           res.replace ("French ", "Fr.");
           res.replace ("Asiatic", "AS");
           res.replace ("European", "EU");
           res.replace ("African", "AF");
           res.replace ("Rep. of ", "Rep. ");//2.73
           if (res.count()>11) l.append(res);
           //if (i<10) qDebug()<<i<<db_cty_c.at(i).pfx<<db_cty_c.at(i).country<<db_cty_c.at(i).continent;
       }
       l.removeDuplicates();
       for (int i = 0; i < l.count(); ++i) qDebug()<<l.at(i);*/
    /*for (int i = 0; i < db_cty_p.count(); ++i)
    {
        if (i<50 || i>7690 ) qDebug()<<i<<db_cty_p.at(i).pfx<<db_cty_p.at(i).country<<db_cty_p.at(i).continent;
        //if (db_cty_p.at(i).pfx.isEmpty() || db_cty_p.at(i).country.isEmpty() || db_cty_p.at(i).continent.isEmpty())
        //qDebug()<<"Empty="<<i<<db_cty_p.at(i).pfx<<db_cty_p.at(i).country<<db_cty_p.at(i).continent;
        //if (db_cty_p.at(i).pfx.contains("RL1O")) qDebug()<<i<<db_cty_p.at(i).pfx<<db_cty_p.at(i).country<<db_cty_p.at(i).continent;
        //if (db_cty_p.at(i).pfx.startsWith("WP4OZK")) qDebug()<<i<<db_cty_p.at(i).pfx<<db_cty_p.at(i).country<<db_cty_p.at(i).continent;
        //if (db_cty_p.at(i).pfx.startsWith("4U1A")) qDebug()<<i<<db_cty_p.at(i).pfx<<db_cty_p.at(i).country<<db_cty_p.at(i).continent;
    }*/
    /*QList<recordd>db_00;
    for (int i = 0; i < db_cty_p.count(); ++i)
    {
        QString c = db_cty_p.at(i).pfx;
        for (int j = 0; j < db_cty_p.count(); ++j)
        {
            if (j==i) j++;
            if (j>=db_cty_p.count()) break;
            QString d = db_cty_p.at(j).pfx;
            if (c==d)
            {
                db_00.append(db_cty_p.at(j));
            }
        }
    }
    std::sort(db_00.begin(),db_00.end(),hv_sort_00_);
    QString ddd=db_00.at(0).pfx;
    int ccc = 1;
    for (int i = 0; i < db_00.count(); ++i)
    {
        if (ddd!=db_00.at(i).pfx)
        {
            ccc++;
            qDebug()<<"---------------------------------------";
        }
        qDebug()<<ccc<<db_00.at(i).pfx<<db_00.at(i).country<<db_00.at(i).continent;
        ddd=db_00.at(i).pfx;
    }
    qDebug()<<"---PFX---";
    db_00.clear();
    for (int i = 0; i < db_cty_c.count(); ++i)
    {
        QString c = db_cty_c.at(i).pfx;
        for (int j = 0; j < db_cty_c.count(); ++j)
        {
            if (j==i) j++;
            if (j>=db_cty_c.count()) break;
            QString d = db_cty_c.at(j).pfx;
            if (c==d)
            {
                db_00.append(db_cty_c.at(j));
            }
        }
    }
    std::sort(db_00.begin(),db_00.end(),hv_sort_00_);
    ddd=db_00.at(0).pfx;
    ccc = 1;
    for (int i = 0; i < db_00.count(); ++i)
    {
        if (ddd!=db_00.at(i).pfx)
        {
            ccc++;
            qDebug()<<"---------------------------------------";
        }
        qDebug()<<ccc<<db_00.at(i).pfx<<db_00.at(i).country<<db_00.at(i).continent;
        ddd=db_00.at(i).pfx;
    }
    qDebug()<<"--CALL--";*/
    //printf("DbCall= %d DbPfx= %d DbCountries= %d\n",db_cty_c.count(),db_cty_p.count(),_lcountries_.count());
    //2.69-2.74 calls=  15867, pfx=  7708,   countries=346, equal calls=55  //1.9.2022
    //2.76.1    DbCall= 21362 DbPfx= 7552 DbCountries= 346  //28.12.2024
    //2.76.2    DbCall= 21679 DbPfx= 7552 DbCountries= 346  //27.03.2025
    //2.76.5    DbCall= 21568 DbPfx= 7553 DbCountries= 346  //23.02.2026
}
#include <QRegularExpression>
QRegularExpression non_prefix_suffix {R"(\A([0-9AMPQR]|QRP|F[DF]|[AM]M|L[HT]|LGT)\z)"};
bool HvCty::FilndDbPfx(QString call0, QString &pfx, QString &coy, QString &cot)
{
    bool res = false;
    pfx="";
    coy="Unknown";
    cot="";
    if (call0.isEmpty()) return false; //{qDebug()<<"CallEmpty="<<pfx<<coy<<cot<<call; return false;}
	QString call = call0;//2.76.3
    int slash_pos = call.indexOf ('/');
    if (slash_pos >= 0)
    {
        int right_size = call0.size() - slash_pos - 1;
        if (right_size >= slash_pos) call = call0.left(slash_pos);
        else
        {
            call = call0.mid (slash_pos + 1);
            if (call.contains(non_prefix_suffix) || call=="B") call = call0.left(slash_pos);
        }
    }//end 2.76.3   	
    for (int i = 0; i < db_cty_c.count(); ++i)
    {
        QString pfxxdb = db_cty_c.at(i).pfx;
        if (call.count()<pfxxdb.count()) continue;
        if (call.count()>pfxxdb.count()) break; //{qDebug()<<"break"; break;}
        if (call==pfxxdb)
        {
            pfx=pfxxdb;
            coy=db_cty_c.at(i).country;
            cot=db_cty_c.at(i).continent;
            res = true; //qDebug()<<"CALL="<<i<<pfx<<call<<db_cty_p.at(i).country;
            break;
        }
    }
    if (res) return true;
    for (int i = 0; i < db_cty_p.count(); ++i)
    {
        QString pfxxdb = db_cty_p.at(i).pfx;
        if (call.count()<pfxxdb.count()) continue;//exception call small then prefix {qDebug()<<pfxcall<<pfxxdb<<call.count()<<pfxxdb.count();
        QString pfxcall = call.mid(0,pfxxdb.count());
        if (pfxcall==pfxxdb)// && pfxxdb.count()>cpfx
        {
            pfx=pfxxdb;
            coy=db_cty_p.at(i).country;
            cot=db_cty_p.at(i).continent;
            res = true; //qDebug()<<"PFX="<<i<<pfx<<call<<db_cty_p.at(i).country;
            break;
        }
    }
    return res;
}
//QElapsedTimer ttt;
QString HvCty::FindCountry(QString call,bool f)
{
    QString res = "Unknown"; //ttt.start();
    QString pfx,coy,cot;
    if (f)
    {
        if (FilndDbPfx(call,pfx,coy,cot)) res = coy + "," + cot;
    }
    else
    {
        if (FilndDbPfx(call,pfx,coy,cot))
        {
            res = coy;
            res.replace ("Islands", "Is.");
            res.replace ("Island", "Is.");
            res.replace ("North ", "N. ");
            res.replace ("Northern ", "N. ");
            res.replace ("South ", "S. ");
            res.replace ("East ", "E. ");
            res.replace ("Eastern ", "E. ");
            res.replace ("West ", "W. ");
            res.replace ("Western ", "W. ");
            res.replace ("Central ", "C. ");
            res.replace (" and ", " & ");
            res.replace ("Republic", "Rep.");
            res.replace ("United States of America", "U.S.A.");
            res.replace ("United States", "U.S.A.");
            res.replace ("Fed. Rep. of ", "");
            res.replace ("French ", "Fr.");
            res.replace ("Asiatic", "AS");
            res.replace ("European", "EU");
            res.replace ("African", "AF");
            res.replace ("Rep. of ", "Rep. ");//2.73
        }
    }
    return res;
}
bool HvCty::HideContinent(QString call,bool fh[8],bool ispfx, QString cot)
{
    if (call.isEmpty()) return true;//exception no std message
    bool res = true;
    if (ispfx)
    {
        if (fh[3] && 		cot=="EU") res = false;
        if (fh[0] && res && cot=="AF") res = false;
        if (fh[2] && res && cot=="AS") res = false;
        if (fh[5] && res && cot=="NA") res = false;
        if (fh[6] && res && cot=="SA") res = false;
        if (fh[4] && res && cot=="OC") res = false;
        if (fh[1] && res && cot=="AN") res = false;
    }
    return res;
}
bool HvCty::HideCountry(QString call,QStringList ls,QString coy) //2.66 bool ispfx,
{
    if (call.isEmpty()) return true;
    bool res = true;
    //if (ispfx)
    //{
    for (int i = 0; i<ls.count(); ++i)
    {
        if (coy==ls.at(i))
        {
            res = false;
            break;
        }
    }
    //}
    return res;
}
bool HvCty::ShowCNYDecode(QString call,QStringList ls,bool ispfx, QString coy)
{
    if (call.isEmpty()) return false;
    bool res = false;
    if (ispfx)
    {
        for (int i = 0; i<ls.count(); ++i)
        {
            if (coy==ls.at(i))
            {
                res = true;
                break;
            }
        }
    }
    return res;
}
bool HvCty::ShowPFXDecode(QString call,QStringList ls,bool ispfx, QString pfx)
{
    if (call.isEmpty()) return false;
    bool res = false;
    if (ispfx)
    {
        for (int i = 0; i<ls.count(); ++i)
        {
            if (pfx==ls.at(i))
            {
                res = true;
                break;
            }
        }
    }
    return res;
}

