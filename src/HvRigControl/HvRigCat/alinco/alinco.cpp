/* MSHV Part from RigControl
 * Copyright 2015 Hrisimir Hristov, LZ2HV
 * May be used under the terms of the GNU General Public License (GPL)
 */
#include "alinco_def.h"
#include "alinco.h"
//#include <QtGui>


#define EOM "\x0d"
//#define EOM 0xfd
//#define BUFSZ 32
//#define AL "AL"
//#define CMD_RXFREQ	"0B"	/* Receive frequency */
//#define PRIll "lld"
#define EOMR 0x0d


Alinco::Alinco(int ModelID,QWidget *parent)
        : QWidget(parent)
{
    s_ModelID = ModelID;
    ////////////////////////////////////////////////////////new read com
    //s_rig_name = rigs_kenwood[s_ModelID].name;
    s_CmdID = -1;
    s_read_array.clear();
    ////////////////////////////////////////////////////////end new read com
}

Alinco::~Alinco()
{
    //qDebug()<<"Delete"<<rigs_Elecraft[s_ModelID].name;
}

void Alinco::SetCmd(CmdID i,ptt_t ptt,QString str)
{
    switch (i)
    {
    case GET_SETT:
        emit EmitRigSet(rigs_alinco[s_ModelID]);
        break;
    case SET_PTT:
        set_ptt(ptt);
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
void Alinco::set_ptt(ptt_t)
{}
////////////////////////////////////////////////////////new read com
void Alinco::set_freq(unsigned long long freq)
{
    /*char freqbuf[BUFSZ];
    int freq_len;
    / at least 6 digits 
    freq_len = sprintf(freqbuf, AL CMD_RXFREQ "%06"PRIll EOM, (unsigned long long)freq);
    emit EmitWriteCmd(freqbuf,freq_len);*/
    char cmdnc[50];
    QString frq = "AL0B";
    frq.append(QString("%1").arg(freq,6,10,QChar('0')));
    frq.append(EOM);

    for (int i = 0; i < frq.count(); i++)
        cmdnc[i]=frq.at(i).toLatin1();
    emit EmitWriteCmd(cmdnc,frq.count());
}
void Alinco::set_mode(QString str)
{
    char cmdnc[50];
    QString mod = "AL2G";
    if (str=="LSB")
        mod.append("0");
    else
        mod.append("1");
    mod.append(EOM);
    for (int i = 0; i < mod.count(); i++)
        cmdnc[i]=mod.at(i).toLatin1();
    emit EmitWriteCmd(cmdnc,mod.count());
}
void Alinco::get_freq()
{
    char cmdnc[50];
    QString frq = "AL3H";
    frq.append(EOM);

    for (int i = 0; i < frq.count(); i++)
        cmdnc[i]=frq.at(i).toLatin1();
    s_read_array.clear(); //for error corection no word end
    emit EmitWriteCmd(cmdnc,frq.count());
}
void Alinco::get_mode()
{
    char cmdnc[50];
    QString mod = "AL3H";
    mod.append(EOM); 

    for (int i = 0; i < mod.count(); i++)
        cmdnc[i]=mod.at(i).toLatin1();
    s_read_array.clear(); //for error corection no word end
    emit EmitWriteCmd(cmdnc,mod.count());


    /*char cmdnc1[100];
    QString mod1 = "05010300142394400007197040";
    for (int i = 0; i < mod1.count(); i++)
        cmdnc1[i]=mod1.at(i).toLatin1();
    emit EmitWriteCmd(cmdnc1,mod1.count());*/
}
void Alinco::SetReadyRead(QByteArray ar,int size)
{
    for (int i = 0; i < size; i++)
    {

        s_read_array.append(ar[i]);
        if (s_read_array.size()==26)
        {
            //qDebug()<<"ALINCO READ ALL COMMAND="<<(QString(s_read_array.toHex()))<<s_read_array.size();

            QByteArray tfreq; //rx frq
            tfreq.append(s_read_array.mid(6,10));
            unsigned long long f = tfreq.toLongLong();
            emit EmitReadedInfo(GET_FREQ,QString("%1").arg(f));

            QString smode = "WRONG_MODE";
            if (s_read_array[2]==(char)0x31)//0x32=USB=2
                smode = "USB";
            emit EmitReadedInfo(GET_MODE,smode);
            s_CmdID = -1;//I Find my answer no need more
            s_read_array.clear();
        }
    }
}
////////////////////////////////////////////////////////end new read com