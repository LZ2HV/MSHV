/* MSHV Part from RigControl
 * Copyright 2015 Hrisimir Hristov, LZ2HV
 * May be used under the terms of the GNU General Public License (GPL)
 */
#include "elecraft_def.h"
#include "elecraft.h"
//#include <QtGui>

#define FI 0x3b  //end of fraze NEWCAT -> ;

Elecraft::Elecraft(int ModelID,QWidget *parent)
        : QWidget(parent)
{

    s_ModelID = ModelID;

    ////////////////////////////////////////////////////////new read com
    s_rig_name = rigs_elecraft[s_ModelID].name;
    s_CmdID = -1;
    s_read_array.clear();
    ////////////////////////////////////////////////////////end new read com
}

Elecraft::~Elecraft()
{
    //qDebug()<<"Delete"<<rigs_Elecraft[s_ModelID].name;
}

void Elecraft::SetCmd(CmdID i,ptt_t ptt,QString str)
{
    switch (i)
    {
    case GET_SETT:
        emit EmitRigSet(rigs_elecraft[s_ModelID]);
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

void Elecraft::set_ptt(ptt_t ptt)
{
    const char *ptt_cmd = "";

    if (rigs_elecraft[s_ModelID].name == "XG3")
    {
        /*switch (ptt)
        {
        case RIG_PTT_ON:
            ptt_cmd = "O,01";
            break;
        case RIG_PTT_ON_MIC:
            ptt_cmd = "O,01";
            break;
        case RIG_PTT_ON_DATA:
            ptt_cmd = "O,01";
            break;
        case RIG_PTT_OFF:
            ptt_cmd = "O,00";
            break;
        }*/
        if (ptt == RIG_PTT_ON)
            ptt_cmd = "O,01";
        else
            ptt_cmd = "O,00";
    }
    else
    {
        /*switch (ptt)
        {
        case RIG_PTT_ON:
            ptt_cmd = "TX";
            break;
        case RIG_PTT_ON_MIC:
            ptt_cmd = "TX0";
            break;
        case RIG_PTT_ON_DATA:
            ptt_cmd = "TX1";
            break;
        case RIG_PTT_OFF:
            ptt_cmd = "RX";
            break;
            //default:
            //return -RIG_EINVAL;
        }*/
        if (ptt == RIG_PTT_ON)
            ptt_cmd = "TX";
        else
            ptt_cmd = "RX";
    }
    //return Elecraft_simple_cmd(rig, ptt_cmd);*/
    int len = strlen(ptt_cmd);
    char cmd[128];
    memcpy(cmd, ptt_cmd, len);
    cmd[len] = ';';
    len++;

    emit EmitWriteCmd((char*)cmd,len);
    // "O,01" : "O,00", 0);

    /*    if (f)
            EmitWriteCmd((char *)"Elecraft SET_PTT ON",1);
        else
            EmitWriteCmd((char *)"Elecraft SET_PTT OFF",1);*/
}
////////////////////////////////////////////////////////new read com
void Elecraft::set_freq(unsigned long long freq)
{
    ////14.250.000 ->FA014250000;  FA;<-read ansver->FA014250000;
    ///144.195.000 ->FA00144195000;
    // "XG3" F;  -> F,00014025000;
    char cmdnc[50];
    QString frq = "FA";

    if (s_rig_name=="XG3")
        frq = "F,";

    frq.append(QString("%1").arg(freq,11,10,QChar('0')));
    frq.append(";");

    for (int i = 0; i < frq.count(); i++)
        cmdnc[i]=frq.at(i).toLatin1();
    emit EmitWriteCmd(cmdnc,frq.count()); /* get native sequence */

    //qDebug()<<"freqbuf="<<freq<<frq;
}
void Elecraft::set_mode(QString str)
{
    //MD2; 2=USB; 1=LSB;
    //xg3 no mode
    if (s_rig_name=="XG3") return;

    char cmdnc[50];
    QString md;
    md="MD";

    //if (str=="LSB") md.append("1;");//LSB
    //else md.append("2;");//USB
    if 		(str=="LSB" ) md.append("1;");//LSB
    else if (str=="USB" ) md.append("2;");//USB
    else if (str=="DIGU") md.append("6;");//DIGU mybe=6 
    //else if (str=="DIGL") md.append("9;");//DIGL 9    	
    else return;

    for (int i = 0; i < md.count(); i++) cmdnc[i]=md.at(i).toLatin1();
    emit EmitWriteCmd(cmdnc,md.count()); /* get native sequence */
}
void Elecraft::get_freq()
{
    // FA;  answer -> 10.000.000 ->FA00010000000;
    //xg3 F; -> F,00014025000;
    char *cmdnc;//
    cmdnc = (char *)"FA;";
    if (s_rig_name=="XG3")
        cmdnc = (char *)"F;";

    int len = strlen(cmdnc);
    emit EmitWriteCmd(cmdnc,len);
}
void Elecraft::get_mode()
{
    // MD;  answer -> MD1; 1LSB 2USB
    char *cmdnc;
    cmdnc =  (char *)"MD;";

    if (s_rig_name=="XG3") //no mode
    {
        s_CmdID = GET_FREQ;
        cmdnc = (char *)"F;";
    }

    int len = strlen(cmdnc);
    emit EmitWriteCmd(cmdnc,len);
}

void Elecraft::SetReadyRead(QByteArray ar,int size0)
{
    for (int i = 0; i < size0; i++)
    {
        if (ar[i]==(char)FI)//;=hex value ??? end of word EOM
        {
            //qDebug()<<"ELECRAFT READ ALL COMMAND="<<(QString(s_read_array.toHex()))<<s_read_array.size();
            //s_read_array.append(ar[i]); // no ;=fi added
            int size = s_read_array.size();
            //rejekt small array
            if (size < 3)//MD2 bez ;
            {                
                s_read_array.clear();//qDebug()<<"NO INAF SIZE";
                return;
            }
            //F=0x46 A=0x41  ,=0x2c
            char id =(char)0x41;
            if (s_rig_name=="XG3") id = (char)0x2c;
            if (s_CmdID==GET_FREQ && s_read_array[0]==(char)0x46 && s_read_array[1]==id)//my qestion for freq and freq filter vfoA
            {
                if (size==13)
                {
                    QByteArray tfreq;
                    tfreq.append(s_read_array.mid(2,11));
                    unsigned long long f = tfreq.toLongLong();
                    emit EmitReadedInfo(GET_FREQ,QString("%1").arg(f));
                    s_CmdID = -1;//I Find my answer no need more
                    if (s_rig_name=="XG3") emit EmitReadedInfo(GET_MODE,"USB");//no mode read
                }
            }
            //M=0x4d D=0x44   MD2 no ;
            if (s_CmdID==GET_MODE && s_read_array[0]==(char)0x4d && s_read_array[1]==(char)0x44)//my qestion for freq and freq filter vfoA
            {
                if (size==3)
                {
                	// 1 (LSB), 2 (USB), 3 (CW), 4 (FM), 5 (AM), 6 (DATA), 7 (CW-REV), or 9 (DATA-REV)
                    QString smode = "WRONG_MODE";
                    if 		(s_read_array[2]==(char)0x31) smode = "LSB";//0x31
                    else if (s_read_array[2]==(char)0x32) smode = "USB";//0x32=USB=2
                    else if (s_read_array[2]==(char)0x33) smode = "CWL";
                    else if (s_read_array[2]==(char)0x34) smode = "FM";
                    else if (s_read_array[2]==(char)0x35) smode = "AM";
                    else if (s_read_array[2]==(char)0x36) smode = "DIGU";//0x39=DATA-USB=6
                    else if (s_read_array[2]==(char)0x37) smode = "CWU";
                    else if (s_read_array[2]==(char)0x39) smode = "DIGL";

                    emit EmitReadedInfo(GET_MODE,smode);
                    s_CmdID = -1;//I Find my answer no need more
                }
            }
            s_read_array.clear();
        }
        else
            s_read_array.append(ar[i]);
    }
    if (s_read_array.size()>512) s_read_array.clear(); //2.55 protection  max>37=TS-2000???  something is wrong
}
////////////////////////////////////////////////////////end new read com