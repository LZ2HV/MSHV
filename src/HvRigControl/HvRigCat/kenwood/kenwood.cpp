/* MSHV Part from RigControl
 * Copyright 2015 Hrisimir Hristov, LZ2HV
 * May be used under the terms of the GNU General Public License (GPL)
 */
#include "kenwood_def.h" 
#include "kenwood.h"

//#include <QtGui>

#define FI 0x3b  //end of fraze NEWCAT -> ;

Kenwood::Kenwood(int ModelID,QWidget *parent)
        : QWidget(parent)
{
    s_ModelID = ModelID;
    s_rig_name = rigs_kenwood[s_ModelID].name;
    s_CmdID = -1;
    s_read_array.clear();
}
Kenwood::~Kenwood()
{
    //qDebug()<<"Delete"<<rigs_kenwood[s_ModelID].name;
}
void Kenwood::SetOnOffCatCommand(bool f, int model_id, int fact_id)//2.56
{
    if (model_id!=s_ModelID || fact_id!=KENWOOD_ID) return;
    if (f)
    {
        //qDebug()<<"Kenwood CAT INIT SetOnOffCatCommand========================";
        char cmd[10];  //AI0;    "Auto Information (AI) function ON/OFF."
		cmd[0]='A';
		cmd[1]='I';
		cmd[2]='0';
		cmd[3]=';';
        emit EmitWriteCmd(cmd,4); //usleep(100000);
        //RIG_MODEL_THD7A: = AI 0;
        //RIG_MODEL_THD74: = AI 0;
        //cmd[2]=';';
        //emit EmitWriteCmd(cmd,3);
    }
}
void Kenwood::SetCmd(CmdID i,ptt_t ptt,QString str)
{
    switch (i)
    {
    case GET_SETT:
        emit EmitRigSet(rigs_kenwood[s_ModelID]);
        break;
    case SET_PTT:
        set_ptt(ptt);
        break;
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
    }
}
/*samo get ptt  Hamlib Kenwood backend - IC-10 interface for:
TS-940, TS-811, TS-711, TS-440, and R-5000*/
void Kenwood::set_ptt(ptt_t ptt)
{
    const char *ptt_cmd = "";

    switch (ptt)
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
    }
    //return kenwood_simple_cmd(rig, ptt_cmd);*/
    int len = strlen(ptt_cmd);
    char cmd[128];
    memcpy(cmd, ptt_cmd, len);
    cmd[len] = ';';
    len++;

    emit EmitWriteCmd((char*)cmd,len);
    //qDebug()<<(char*)cmd<<ptt_cmd;
}
void Kenwood::set_freq(unsigned long long freq)
{
    ////14.250.000 ->FA014250000;  FA;<-read ansver->FA014250000;
    ///144.195.000 ->FA00144195000;
    char cmdnc[50];
    QString frq = "FA";
    //QString frq = "IF";
    frq.append(QString("%1").arg(freq,11,10,QChar('0')));
    frq.append(";");

    for (int i = 0; i < frq.count(); i++) cmdnc[i]=frq.at(i).toLatin1();
    emit EmitWriteCmd(cmdnc,frq.count()); /* get native sequence */
    //qDebug()<<"freqbuf="<<freq<<cmdnc<<frq;
}
void Kenwood::set_mode(QString str)
{
    //MD2; 2=USB; 1=LSB;  or TS-990s->OM02 OM02; //TS590S TS590SG have DA0/1 data node on of
    //TS590S TS590SG //TS990S
    char cmdnc[50];
    QString md;
    if (s_rig_name=="TS-990s") md="OM0";//0 vfo A main  1 vfoB
    else md="MD";

    if 		(str=="LSB" ) md.append("1;");//LSB
    else if (str=="USB" ) md.append("2;");//USB
    else if (str=="DIGU") 
    {
    	if (s_rig_name=="TS-990s") md.append("D;");//DIGU  
    	else md.append("9;");//DIGU    	
   	}
    else return;

    for (int i = 0; i < md.count(); i++) cmdnc[i]=md.at(i).toLatin1();
    emit EmitWriteCmd(cmdnc,md.count()); /* get native sequence */
}
void Kenwood::get_freq()
{
    // IF;  answer -> 144.195.000 ->IF00144195000........;
    char *cmdnc;//
    cmdnc =  (char *)"IF;";
    int len = strlen(cmdnc);
    emit EmitWriteCmd(cmdnc,len);
}
void Kenwood::get_mode()
{
    // IF;  answer -> 144.195.000 ->IF00144195000........ 29=MD;
    char *cmdnc;
    cmdnc =  (char *)"IF;";
    int len = strlen(cmdnc);
    emit EmitWriteCmd(cmdnc,len);
}
void Kenwood::SetReadyRead(QByteArray ar,int size0)
{
    for (int i = 0; i < size0; i++)
    {	
        if (ar[i]==(char)FI)//;=hex value ??? end of word EOM
        {
            //qDebug()<<"KENWOOD READ ALL COMMAND="<<(QString(s_read_array.toHex()))<<s_read_array.size();
            //s_read_array.append(ar[i]); // no ;=fi added
            int size = s_read_array.size();
            //rejekt small array
            if (size < 37)//37 bez ;
            {
                //qDebug()<<"NO INAF SIZE";
                s_read_array.clear();
                return;
            }
            //I=49 F=46
            if ((s_CmdID==GET_FREQ || s_CmdID==GET_MODE) && s_read_array[0]==(char)0x49 && s_read_array[1]==(char)0x46)//my qestion for freq and freq filter vfoA
            {
                QByteArray tfreq;
                tfreq.append(s_read_array.mid(2,11));
                unsigned long long f = tfreq.toLongLong();
                emit EmitReadedInfo(GET_FREQ,QString("%1").arg(f));

                QString smode = "WRONG_MODE";
                if 		(s_read_array[29]==(char)0x31) smode = "LSB";  //0x31=LSB=1
                else if (s_read_array[29]==(char)0x32) smode = "USB";  //0x32=USB=2  
                else if (s_read_array[29]==(char)0x33) smode = "CWU";  //0x33=CW-USB=3 
                else if (s_read_array[29]==(char)0x34) smode = "FM";   //0x34=FM=4  
                else if (s_read_array[29]==(char)0x35) smode = "AM";   //0x35=AM=5  
                else if (s_read_array[29]==(char)0x36)
                {
                	if (s_rig_name=="TS-990s") smode = "FSK";
                	else smode = "DIGL"; //0x36=DATA-LSB=6                 	
               	}
                else if (s_read_array[29]==(char)0x37) smode = "CWL";  //0x37=CW-LSB=7              
                else if (s_read_array[29]==(char)0x39)
                {
                	if (s_rig_name=="TS-990s") smode = "FSKR";
                	else smode = "DIGU"; //0x39=DATA-USB=9                 	
               	}               
                else if (s_read_array[29]==(char)0x41) smode = "PSK";  //0x41=PSK=A <- TS-990s
                else if (s_read_array[29]==(char)0x42) smode = "PSKR"; //0x42=PSK-R=B <- TS-990s                 
                else if (s_read_array[29]==(char)0x43) smode = "DIGL"; //0x43=DATA-LSB=C <- TS-990s
                else if (s_read_array[29]==(char)0x44) smode = "DIGU"; //0x44=DATA-USB=D <- TS-990s               
    			//printf("->ModeRead=%x  =%c\n",(char)s_read_array[29],(int)s_read_array[29]);               
                emit EmitReadedInfo(GET_MODE,smode);
                s_CmdID = -1;//I Find my answer no need more
            }
            s_read_array.clear();
        }
        else
            s_read_array.append(ar[i]);    
    }
    if (s_read_array.size()>1024) s_read_array.clear(); //2.55 protection  max>37=TS-2000  something is wrong
}
