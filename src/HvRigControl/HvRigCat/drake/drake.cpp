/* MSHV Part from RigControl
 * Copyright 2015 Hrisimir Hristov, LZ2HV
 * May be used under the terms of the GNU General Public License (GPL)
 */
#include "drake_def.h"
#include "drake.h"
//#include <QtGui>

#define EOM 0x0d
#define CR "\x0d"
#define EOMS CR

Drake::Drake(int ModelID,QWidget *parent)
        : QWidget(parent)
{
    s_ModelID = ModelID;
    ////////////////////////////////////////////////////////new read com
    //s_rig_name = rigs_kenwood[s_ModelID].name;
    s_CmdID = -1;
    s_read_array.clear();
    ////////////////////////////////////////////////////////end new read com
}

Drake::~Drake()
{
    //qDebug()<<"Delete"<<rigs_Elecraft[s_ModelID].name;
}

void Drake::SetCmd(CmdID i,ptt_t /*ptt*/,QString str)
{
    switch (i)
    {
    case GET_SETT:
        emit EmitRigSet(rigs_drake[s_ModelID]);
        break;
    case SET_PTT:
        //set_ptt(ptt);
        break;
        ////////////////////////////////////////////////////////new read com
    case SET_FREQ:
        set_freq(str.toLongLong());
        break;
    case GET_FREQ:
        s_CmdID = GET_FREQ;
        get_freq();
        break;
    case SET_MODE:
        set_mode(str);
        break;
    case GET_MODE:
        s_CmdID = GET_MODE;
        get_mode();
        break;
        ////////////////////////////////////////////////////////end new read com
    }
}
/*
void Drake::set_ptt(ptt_t)
{
}
void Elecraft::get_ptt()
{
	emit EmitWriteCmd("Elecraft GET_PTT");
}
*/
////////////////////////////////////////////////////////new read com
//#define PRIll "lld"
void Drake::set_freq(unsigned long long freq)
{
    char cmdnc[50];
    QString frq = "F";
    freq = freq/10;
    frq.append(QString("%1").arg(freq,7,10,QChar('0')));
    frq.append(EOMS);
    for (int i = 0; i < frq.count(); i++)
        cmdnc[i]=frq.at(i).toLatin1();
    emit EmitWriteCmd(cmdnc,frq.count());
}
void Drake::set_mode(QString str)
{
    char cmdnc[50];
    QString mod = "M";
    if (str=="LSB")
        mod.append("2");
    else
        mod.append("1");
    mod.append(EOMS);
    for (int i = 0; i < mod.count(); i++)
        cmdnc[i]=mod.at(i).toLatin1();
    emit EmitWriteCmd(cmdnc,mod.count());
}
void Drake::get_freq()
{
	/////////////*fffff.ff*mHz<CR>
	//QString frq="*14.25312*mHz";
    QString frq = "RF"; //ansver RA command returns *fffff.ff*mHz<CR>  freq_len != 15 no 14>
    frq.append(EOMS);
    char cmdnc[50];
    for (int i = 0; i < frq.count(); i++)
        cmdnc[i]=frq.at(i).toLatin1();
    emit EmitWriteCmd(cmdnc,frq.count());
}
void Drake::get_mode()
{
    QString mod = "RM"; //ansver mdbuf_len != 8  cmode = mdbuf[3];  xxxxxCRLF
    mod.append(EOMS);
    char cmdnc[50];
    for (int i = 0; i < mod.count(); i++)
        cmdnc[i]=mod.at(i).toLatin1();
    emit EmitWriteCmd(cmdnc,mod.count());
}
void Drake::SetReadyRead(QByteArray ar,int size0)
{
    for (int i = 0; i < size0; i++)
    {
        if (ar[i]==(char)EOM)
        {
            //qDebug()<<"DRAKE READ ALL COMMAND="<<(QString(s_read_array.toHex()))<<s_read_array.size();
            int size = s_read_array.size();
            //rejekt small array
            if (size < 5)//eom bez \r
            {
                //qDebug()<<"NO INAF SIZE";
                s_read_array.clear();
                return;
            }
            if (s_CmdID==GET_FREQ && size==13)//ansver RA command returns *fffff.ff*mHz<CR>  freq_len != 15 no 14>
            {
                QByteArray tfreq;
                char fmult = (char)s_read_array[10];
                tfreq.append(s_read_array.mid(1,8));
                double f = tfreq.toDouble(); // qDebug()<<"f="<<QString("%1").arg(f,0,'f',10);
                f *= 1000.0; //qDebug()<<"f1="<<QString("%1").arg(f,0,'f',10);
                if (fmult == 'M' || fmult == 'm')
                    f *= 1000.0;
                //qDebug()<<"f3="<<QString("%1").arg(f,0,'f',10);   

                emit EmitReadedInfo(GET_FREQ,QString("%1").arg((unsigned long long)f));
                s_CmdID = -1;//I Find my answer no need more
            }
            if (s_CmdID==GET_MODE && size==5)//ansver mdbuf_len != 8  cmode = mdbuf[3];  xxxxx CRLF
            {
                QString smode = "WRONG_MODE";
                char cmode = (char)s_read_array[3];
                char cwidth = (char)s_read_array[4];
                if ((cwidth >= '0') && (cwidth <= '4'))
                {
                    switch(cmode & 0x33)
                    {
                    case '0':
                        //*mode = RIG_MODE_LSB;
                        break;
                    }
                }
                else
                {
                    switch(cmode & 0x33)
                    {
                    case '0':
                        smode = "USB";//*mode = RIG_MODE_USB;
                        break;
                    }
                }
                emit EmitReadedInfo(GET_MODE,smode);
                s_CmdID = -1;//I Find my answer no need more
            }
            s_read_array.clear();
        }
        else
            s_read_array.append(ar[i]);
    }
    if (s_read_array.size()>1024) s_read_array.clear(); //2.55 protection 
}
////////////////////////////////////////////////////////end new read com