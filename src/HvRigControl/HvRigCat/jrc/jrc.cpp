/* MSHV Part from RigControl
 * Copyright 2015 Hrisimir Hristov, LZ2HV
 * May be used under the terms of the GNU General Public License (GPL)
 */
#include "jrc_def.h"
#include "jrc.h"
//#include <QtGui>

#define JRC_CMD_LENGTH 7
#define PTT_STR 202

//#define EOMS "\015"	/* CR */
#define EOM 0x0d /* CR */
#define EOMS "\r"

enum native_cmd_e {
    NATIVE_CAT_PTT_ON = 0,
    NATIVE_CAT_PTT_OFF,
    NATIVE_CAT_SET_FREQ,
    NATIVE_CAT_GET_FREQ,
    NATIVE_CAT_SET_MODE,
    NATIVE_CAT_GET_MODE
};
struct jrc_cmd_set
{
    unsigned char ncomp;		        /* 1 = complete, 0 = incomplete, needs extra info */
    unsigned char nseq[JRC_CMD_LENGTH];	/* native cmd sequence */
};

typedef struct jrc_cmd_set jrc_cmd_set_t;

static const jrc_cmd_set_t ncmd[JRC_COUNT][6] =
    {
        //"NRD-525"
        {{ PTT_STR, { "None" } }, /* \r=0x0D ili->. *identif newcat NEWCAT TX1; ptt on */
         { PTT_STR, { "None" } }, /* \r=0x0D ili->. identif newcat NEWCAT TX0;  ptt off */
         { PTT_STR, { "F" } },   // set frq
         { PTT_STR, { "None" } },   // get frq
         { PTT_STR, { "D" } },   //set mod
         { PTT_STR, { "None" } },}, //get mode
        //"NRD-535D"
        {{ PTT_STR, { "None" } }, /* \r=0x0D ili->. *identif newcat NEWCAT TX1; ptt on */
         { PTT_STR, { "None" } }, /* \r=0x0D ili->. identif newcat NEWCAT TX0;  ptt off */
         { PTT_STR, { "F" } },   // set frq
         { PTT_STR, { "I1\rI0\r" } },   // get frq
         { PTT_STR, { "D" } },   //set mod
         { PTT_STR, { "I1\rI0\r" } },}, //get mode
        //"NRD-545DSP"
        {{ PTT_STR, { "None" } }, /* \r=0x0D ili->. *identif newcat NEWCAT TX1; ptt on */
         { PTT_STR, { "None" } }, /* \r=0x0D ili->. identif newcat NEWCAT TX0;  ptt off */
         { PTT_STR, { "F" } },   // set frq
         { PTT_STR, { "I\r" } },   // get frq
         { PTT_STR, { "D" } },   //set mod
         { PTT_STR, { "I\r" } },}, //get mode
        //"JST-245"
        {{ PTT_STR, { "None" } }, /* \r=0x0D ili->. *identif newcat NEWCAT TX1; ptt on */
         { PTT_STR, { "None" } }, /* \r=0x0D ili->. identif newcat NEWCAT TX0;  ptt off */
         { PTT_STR, { "H1\rF" } },   // set frq
         { PTT_STR, { "I\r" } },   // get frq
         { PTT_STR, { "H1\rD" } },   //set mod
         { PTT_STR, { "I\r" } },}, //get mode
    };

Jrc::Jrc(int ModelID,QWidget *parent)
        : QWidget(parent)
{
    s_ModelID = ModelID;
    ////////////////////////////////////////////////////////new read com
    //s_rig_name = rigs_kenwood[s_ModelID].name;
    s_CmdID = -1;
    s_read_array.clear();
    ////////////////////////////////////////////////////////end new read com
}
Jrc::~Jrc()
{
    //qDebug()<<"Delete"<<rigs_Elecraft[s_ModelID].name;
}
void Jrc::SetCmd(CmdID i,ptt_t/* ptt*/,QString str)
{
    switch (i)
    {
    case GET_SETT:
        emit EmitRigSet(rigs_jrc[s_ModelID]);
        break;
    case SET_PTT:
        //set_ptt(ptt);// all no ptt via cat
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
/*void Jrc::set_ptt(ptt_t)
{
}*/
////////////////////////////////////////////////////////new read com
//#define PRIll "lld"
void Jrc::set_freq(unsigned long long freq)
{
    QString ide = (char*)ncmd[s_ModelID][NATIVE_CAT_SET_FREQ].nseq;
    if (ide=="None") return;
    //char freqbuf[50];
    //sprintf(freqbuf, "F%08u", (unsigned)(freq/10));
    //int freq_len = sprintf(freqbuf, "F%0*"PRIll EOM1, 10, (unsigned long long)freq);
    //emit EmitWriteCmd(freqbuf,freq_len);
    char cmdnc[60];
    QString frq = ide;

    if (s_ModelID==0)//NRD-525
    {
        freq = freq/10;
        int max_freq_len = 8;
        frq.append(QString("%1").arg(freq,max_freq_len,10,QChar('0')));
    }
    else if (s_ModelID==1)//NRD-535D
    {
        int max_freq_len = 8;
        frq.append(QString("%1").arg(freq,max_freq_len,10,QChar('0')));
        frq.append(EOMS);
    }
    else if (s_ModelID==2)//NRD-545DSP
    {
        int max_freq_len = 10;
        frq.append(QString("%1").arg(freq,max_freq_len,10,QChar('0')));
        frq.append(EOMS);
    }
    else if (s_ModelID==3)//JST-245  //H1<cr>F........A<cr>H0<cr>
    {
        int max_freq_len = 8;// set to VFO A ->  H1<cr>FA<cr>H0<cr>
        frq.append(QString("%1").arg(freq,max_freq_len,10,QChar('0')));
        frq.append("A\rH0\r");      
    }    
    
    for (int i = 0; i < frq.count(); i++) cmdnc[i]=frq.at(i).toLatin1();
    emit EmitWriteCmd(cmdnc,frq.count());
}
void Jrc::set_mode(QString str)
{
    QString ide = (char*)ncmd[s_ModelID][NATIVE_CAT_SET_MODE].nseq;
    if (ide=="None") return;

    char cmdnc[50];
    QString mod = ide;
    //if (str=="LSB") mod.append("3");
    //else mod.append("2");
    if 		(str=="LSB" ) mod.append("3");//LSB
    else if (str=="USB" ) mod.append("2");//USB
    else if (str=="DIGU") mod.append("0");//DIGU
    else return;

    if (s_ModelID==1 || s_ModelID==2) mod.append(EOMS);
    if (s_ModelID==3) mod.append("\rH0\r");  //JST-245  H1<cr>D2<cr>H0<cr>   
        
    for (int i = 0; i < mod.count(); i++) cmdnc[i]=mod.at(i).toLatin1();
    emit EmitWriteCmd(cmdnc,mod.count());          
    //qDebug()<<"JRC="<<mod<<mod.count();
}
void Jrc::get_freq()
{
    QString all = (char*)ncmd[s_ModelID][NATIVE_CAT_GET_FREQ].nseq;
    if (all=="None") return;
    char cmdnc[50];
    for (int i = 0; i < all.count(); i++) cmdnc[i]=all.at(i).toLatin1();
    //s_read_array.clear(); //for error corection no word end
    emit EmitWriteCmd(cmdnc,all.count());
}
void Jrc::get_mode()
{
    QString all = (char*)ncmd[s_ModelID][NATIVE_CAT_GET_MODE].nseq;
    if (all=="None") return;
    char cmdnc[50];
    for (int i = 0; i < all.count(); i++) cmdnc[i]=all.at(i).toLatin1();
    //s_read_array.clear(); //for error corection no word end
    emit EmitWriteCmd(cmdnc,all.count());
}
void Jrc::SetReadyRead(QByteArray ar,int size0)
{
    for (int i = 0; i < size0; i++)
    {
        if (ar[i]==(char)EOM)
        {
            //qDebug()<<"JRC READ ALL COMMAND="<<(QString(s_read_array.toHex()))<<s_read_array.size();
            int size = s_read_array.size();
            //rejekt small array
            if (size < 13)//eom bez \r
            {                
                s_read_array.clear(); //qDebug()<<"NO INAF SIZE";
                return;
            }
            // I=0x49
            if ((s_CmdID==GET_FREQ || s_CmdID==GET_MODE) && s_read_array[0]==(char)0x49)//my qestion for freq and freq filter vfoA
            { 
                if ((s_ModelID==1 || s_ModelID==3) && size==13 )//size eom bez \r NRD-535D and JST-254
                {
                    QByteArray tfreq;
                    tfreq.append(s_read_array.mid(4,8));
                    unsigned long long f = tfreq.toLongLong();
                    emit EmitReadedInfo(GET_FREQ,QString("%1").arg(f));

                    QString smode = "WRONG_MODE";
                    if (s_read_array[3]==(char)0x32)//0x32=USB=2
                        smode = "USB";
                    else if (s_read_array[3]==(char)0x30)//0x30=digu USB=0
                        smode = "DIGU";    
                        
                    emit EmitReadedInfo(GET_MODE,smode);
                    s_CmdID = -1;//I Find my answer no need more
                }
                if (s_ModelID==2 && size>=15)//size eom bez \r NRD-545DSP
                {
                    QByteArray tfreq;
                    tfreq.append(s_read_array.mid(4,10));
                    unsigned long long f = tfreq.toLongLong();
                    emit EmitReadedInfo(GET_FREQ,QString("%1").arg(f));

                    QString smode = "WRONG_MODE";
                    if (s_read_array[3]==(char)0x32)//0x32=USB=2
                        smode = "USB";
                    else if (s_read_array[3]==(char)0x30)//0x30=digu USB=0
                        smode = "DIGU";
                        
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