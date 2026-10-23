/* MSHV Part from RigControl
 * Copyright 2015 Hrisimir Hristov, LZ2HV
 * May be used under the terms of the GNU General Public License (GPL)
 */
#include "racal_def.h"
#include "racal.h"
#include <stdio.h>//sprintf or //#include <cstdio>
//#include <QtGui>

//#define EOM "\x0d"
//6790
//#define SOM "$"
//#define EOM "\x0d"	/* CR */

//3702
//#define SOM "\x0a"  /* LF */
//#define EOM "\x0d"	/* CR */

#define RACAL_CMD_LENGTH 7
#define PTT_STR 202

#define EOM 0x0d

enum native_cmd_e {
    NATIVE_CAT_PTT_ON = 0,
    NATIVE_CAT_PTT_OFF,
    NATIVE_CAT_SET_FREQ,
    NATIVE_CAT_GET_FREQ,
    NATIVE_CAT_SET_MODE,
    NATIVE_CAT_GET_MODE
};
struct racal_cmd_set
{
    unsigned char ncomp;		        /* 1 = complete, 0 = incomplete, needs extra info */
    unsigned char nseq[RACAL_CMD_LENGTH];	/* native cmd sequence */
};

typedef struct racal_cmd_set racal_cmd_set_t;

static const racal_cmd_set_t ncmd[RACAL_COUNT][6] =
    {
        //RA6790/GM
        {{ PTT_STR, { "None" } }, /* \r=0x0D ili->. *identif newcat NEWCAT TX1; ptt on */
         { PTT_STR, { "None" } }, /* \r=0x0D ili->. identif newcat NEWCAT TX0;  ptt off */
         { PTT_STR, { "F" } },   // set frq
         { PTT_STR, { "TF" } },   // get frq
         { PTT_STR, { "D" } },   //set mod
         { PTT_STR, { "TDI" } },}, //get mode
        //RA3702
        {{ PTT_STR, { "None" } }, /* \r=0x0D ili->. *identif newcat NEWCAT TX1; ptt on */
         { PTT_STR, { "None" } }, /* \r=0x0D ili->. identif newcat NEWCAT TX0;  ptt off */
         { PTT_STR, { "F" } },   // set frq
         { PTT_STR, { "QF" } },   // get frq
         { PTT_STR, { "M" } },   //set mod
         { PTT_STR, { "QM" } },}, //get mode
    };

Racal::Racal(int ModelID,QWidget *parent)
        : QWidget(parent)
{
    s_ModelID = ModelID;
    ////////////////////////////////////////////////////////new read com
    //s_rig_name = rigs_kenwood[s_ModelID].name;
    s_CmdID = -1;
    s_read_array.clear();

    if (s_ModelID==0)//"RA6790/GM"
    {
        s_receiver_id=0;
        s_SOM = "$";
    }
    if (s_ModelID==1)//"RA3702"
    {
        s_receiver_id=-1;
        s_SOM = "\x0a";
    }
    ////////////////////////////////////////////////////////end new read com
}

Racal::~Racal()
{
    //qDebug()<<"Delete"<<rigs_Elecraft[s_ModelID].name;
}

void Racal::SetCmd(CmdID i,ptt_t /*ptt*/,QString str)
{
    switch (i)
    {
    case GET_SETT:
        emit EmitRigSet(rigs_racal[s_ModelID]);
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

////////////////////////////////////////////////////////new read com
//#define PRIll "lld"
void Racal::make_cmd(QString cmd)
{
    char cmdnc[300];
    QString all = s_SOM;
    if (s_ModelID==0)
        all.append(QString("%1").arg(s_receiver_id));
    if (s_ModelID==1)
    {
        if (s_receiver_id != -1)
            all.append(QString("%1").arg(s_receiver_id));
    }
    all.append(cmd);
    all.append(EOM);
    
    /*all="";
    all = "F1423";
    all.append(EOM);*/

    for (int i = 0; i < all.count(); i++)
        cmdnc[i]=all.at(i).toLatin1();
    emit EmitWriteCmd(cmdnc,all.count());
}
void Racal::set_freq(unsigned long long freq)
{
    QString ide = (char*)ncmd[s_ModelID][NATIVE_CAT_SET_FREQ].nseq;
    if (ide=="None")
        return;

    char freqbuf[50];
    if (s_ModelID==0)
    {
        double ff = (double)freq/1000000.0;
        sprintf(freqbuf, "%0g", (double)(ff));
        ide.append(QString(freqbuf));  //freq_len = sprintf(freqbuf, "F%0g", (double)(freq/MHz(1)));
    }
    if (s_ModelID==1)
    {
        sprintf(freqbuf, "%ld", (long)freq);//2.12
        ide.append(QString(freqbuf));  //freq_len = sprintf(freqbuf, "F%ld", (unsigned long)freq);
    }
    make_cmd(ide);
    // float ff = (float)freq/1000000.0;//vinagi mi e w Hz
    // sprintf(freqbuf, "%0g", ff);
    //char freqbuf[50];
    //freqbuf[1] = 'D';
    //int freq_len = sprintf(freqbuf, "F%ld", (unsigned long)freq);  
    //EmitWriteCmd(freqbuf,1);
    //qDebug()<<"RACAL READ ALL COMMAND="<<ide<<freqbuf;
}
void Racal::set_mode(QString str)
{
    QString ide = (char*)ncmd[s_ModelID][NATIVE_CAT_SET_MODE].nseq;
    if (ide=="None")
        return;

    QString mod;
    if (s_ModelID==0)
    {
        if (str=="LSB")
            mod.append("6");
        else
            mod.append("7");
    }
    if (s_ModelID==1)
    {
        if (str=="LSB")
            mod.append("2");
        else
            mod.append("1");
    }
    ide.append(mod);
    make_cmd(ide);
}
void Racal::get_freq()
{
    QString ide = (char*)ncmd[s_ModelID][NATIVE_CAT_GET_FREQ].nseq;
    if (ide=="None")
        return;
    make_cmd(ide);
}
void Racal::get_mode()
{
    QString ide = (char*)ncmd[s_ModelID][NATIVE_CAT_GET_MODE].nseq;
    if (ide=="None")
        return;
    make_cmd(ide);
}

void Racal::SetReadyRead(QByteArray ar,int size0)
{
    for (int i = 0; i < size0; i++)
    {
        if (ar[i]==(char)EOM)
        {
            //qDebug()<<"RACAL READ ALL COMMAND="<<(QString(s_read_array.toHex()))<<s_read_array.size();
            int size = s_read_array.size();
            //rejekt small array
            /*if (size < 13)//eom bez \r
            {
                //qDebug()<<"NO INAF SIZE";
                s_read_array.clear();
                return;
            }*/
            if (s_ModelID==0)//RA6790/GM
            {
                if (s_CmdID==GET_FREQ && s_read_array[0]==(char)0x46 && size>1)// F=0x46  my qestion for freq and freq filter vfoA
                {
                    QByteArray tfreq;
                    tfreq.append(s_read_array.mid(1,size));
                    double f = tfreq.toDouble()*1000000.0;
                    emit EmitReadedInfo(GET_FREQ,QString("%1").arg((unsigned long long)f));
                    s_CmdID = -1;//I Find my answer no need more
                }
                if (s_CmdID==GET_MODE && s_read_array[0]==(char)0x44 && size>1)// D=0x44
                {
                    QString smode = "WRONG_MODE";
                    if (s_read_array[1]==(char)0x37)//0x32=USB=2
                        smode = "USB";
                    emit EmitReadedInfo(GET_MODE,smode);
                    s_CmdID = -1;//I Find my answer no need more
                }
            }
            if (s_ModelID==1)// size eom bez RA3702
            {
                if (s_CmdID==GET_FREQ && s_read_array[1]==(char)0x46 && size>2)//otpred ima SOM F=0x46
                {
                    QByteArray tfreq;
                    tfreq.append(s_read_array.mid(2,size));
                    unsigned long long f = tfreq.toLongLong();
                    emit EmitReadedInfo(GET_FREQ,QString("%1").arg(f));
                    s_CmdID = -1;//I Find my answer no need more
                }
                if (s_CmdID==GET_MODE && s_read_array[1]==(char)0x4d && size>2)//otpred ima SOM M=0x4d
                {
                    QString smode = "WRONG_MODE";
                    if (s_read_array[2]==(char)0x31)//0x32=USB=2
                        smode = "USB";
                    emit EmitReadedInfo(GET_MODE,smode);
                    s_CmdID = -1;//I Find my answer no need more
                }
            }
            s_read_array.clear();
        }
        else
            s_read_array.append(ar[i]);
    }
    if (s_read_array.size()>1024) s_read_array.clear(); //2.55 protection 
}
////////////////////////////////////////////////////////end new read com