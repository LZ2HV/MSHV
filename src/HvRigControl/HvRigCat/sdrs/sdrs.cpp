/* MSHV Part from RigControl
 * Copyright 2015 Hrisimir Hristov, LZ2HV
 * May be used under the terms of the GNU General Public License (GPL)
 */
#include "sdrs_def.h"
#include "sdrs.h"
//#include <QtGui>

#define FI 0x3b  //end of fraze NEWCAT -> ;


typedef struct
{
    QString on;
    QString off;
    QString ptton;
    QString pttoff;
    QString setfreq;
    QString setmodeu;
	//QString setmodel; //"ZZMD00" and "MD1" = LSB    
    QString setmodedu; 
    QString getfreq;
    QString getmode;
}
sdr_cmd_set_t;

static sdr_cmd_set_t ncmd[SDRS_COUNT] =
    {
        //// fgsdfg  ////        
        {"AI0"        ,"","TX"   ,"RX"   ,"FA"  ,"MD2"   ,"MD9"   ,"IF"  ,"IF"  }, //FlexRadio PowerSDR
        {"AI0"        ,"","TX"   ,"RX"   ,"FA"  ,"MD2"   ,"MD9"   ,"IF"  ,"IF"  }, //Thetis
        {"ZZAI0;ZZDX1","","ZZTX1","ZZTX0","ZZFA","ZZMD01","ZZMD07","ZZIF","ZZIF"}, //FlexRadio SmartSDR
        {"ZZAI0;ZZDX1","","ZZTX1","ZZTX0","ZZFA","ZZMD01","ZZMD07","ZZFA","ZZMD"}, //FlexRadio SmartSDR Slice A 
        {"ZZAI0;ZZDX1","","ZZTX1","ZZTX0","ZZFA","ZZMD01","ZZMD07","ZZFA","ZZMD"}, //FlexRadio SmartSDR Slice B
        {"ZZAI0;ZZDX1","","ZZTX1","ZZTX0","ZZFA","ZZMD01","ZZMD07","ZZFA","ZZMD"}, //FlexRadio SmartSDR Slice C
        {"ZZAI0;ZZDX1","","ZZTX1","ZZTX0","ZZFA","ZZMD01","ZZMD07","ZZFA","ZZMD"}, //FlexRadio SmartSDR Slice D
        {"ZZAI0;ZZDX1","","ZZTX1","ZZTX0","ZZFA","ZZMD01","ZZMD07","ZZFA","ZZMD"}, //FlexRadio SmartSDR Slice E
        {"ZZAI0;ZZDX1","","ZZTX1","ZZTX0","ZZFA","ZZMD01","ZZMD07","ZZFA","ZZMD"}, //FlexRadio SmartSDR Slice F
        {"ZZAI0;ZZDX1","","ZZTX1","ZZTX0","ZZFA","ZZMD01","ZZMD07","ZZFA","ZZMD"}, //FlexRadio SmartSDR Slice G 
        {"ZZAI0;ZZDX1","","ZZTX1","ZZTX0","ZZFA","ZZMD01","ZZMD07","ZZFA","ZZMD"}, //FlexRadio SmartSDR Slice H 
             	
    };

SDRs::SDRs(int ModelID,QWidget *parent)
        : QWidget(parent)
{
    s_ModelID = ModelID;
    ////////////////////////////////////////////////////////new read com
    //s_rig_name = rigs_sdrs[s_ModelID].name;
    s_CmdID = -1;
    s_read_array.clear();
    ////////////////////////////////////////////////////////end new read com   
    /*QByteArray array;
    array.append("99");
    qDebug()<<"WriteCmd"<<(QString(array.toHex()));*/
}
SDRs::~SDRs()
{
    //qDebug()<<"Delete"<<rigs_sdrs[s_ModelID].name;
}
void SDRs::SetOnOffCatCommand(bool f, int model_id, int fact_id)
{
    if (model_id!=s_ModelID || fact_id!=SDRS_ID) return;
    if (f && !ncmd[s_ModelID].on.isEmpty())
    {
        char cmdnc[50];
        QString cmd = ncmd[s_ModelID].on;
        cmd.append(";");
        for (int i = 0; i < cmd.count(); i++) cmdnc[i]=cmd.at(i).toLatin1();
        emit EmitWriteCmd(cmdnc,cmd.count());
    }
    if (!f && !ncmd[s_ModelID].off.isEmpty())
    {
        char cmdnc[50];
        QString cmd = ncmd[s_ModelID].off;
        cmd.append(";");
        for (int i = 0; i < cmd.count(); i++) cmdnc[i]=cmd.at(i).toLatin1();
        emit EmitWriteCmd(cmdnc,cmd.count());
    }
}
void SDRs::SetCmd(CmdID i,ptt_t ptt,QString str)
{
    switch (i)
    {
    case GET_SETT:
        emit EmitRigSet(rigs_sdrs[s_ModelID]);
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
void SDRs::set_ptt(ptt_t ptt)
{
    char cmdnc[50];
    QString cmd;
    if (ptt==RIG_PTT_ON) cmd = ncmd[s_ModelID].ptton;       
    else cmd = ncmd[s_ModelID].pttoff;
    cmd.append(";");
    for (int i = 0; i < cmd.count(); i++) cmdnc[i]=cmd.at(i).toLatin1();
    emit EmitWriteCmd(cmdnc,cmd.count());
}
void SDRs::set_freq(unsigned long long freq)
{
    ////14.250.000 ->FA014250000;  FA;<-read ansver->FA014250000;
    ///144.195.000 ->FA00144195000;
    char cmdnc[50];
    QString frq = ncmd[s_ModelID].setfreq;
    frq.append(QString("%1").arg(freq,11,10,QChar('0')));
    frq.append(";");

    for (int i = 0; i < frq.count(); i++) cmdnc[i]=frq.at(i).toLatin1();
    emit EmitWriteCmd(cmdnc,frq.count()); /* get native sequence */
}
void SDRs::set_mode(QString str)
{
    //MD2; 2=USB; 1=LSB;  or TS-990s->OM02 OM02; //TS590S TS590SG have DA0/1 data node on of
    //TS590S TS590SG //TS990S
    char cmdnc[50];
    QString md;
    if (str=="USB") md.append(ncmd[s_ModelID].setmodeu+";");//USB
    else if (str=="DIGU") md.append(ncmd[s_ModelID].setmodedu+";");//DIGU
    //else if (str=="LSB") md.append(ncmd[s_ModelID].setmodel+";");//LSB
    else return;

    for (int i = 0; i < md.count(); i++) cmdnc[i]=md.at(i).toLatin1();
    emit EmitWriteCmd(cmdnc,md.count());
}
void SDRs::get_freq()
{
    // IF;  answer -> 144.195.000 ->IF00144195000........;
    char cmdnc[50];
    QString cmd = ncmd[s_ModelID].getfreq+";";
    for (int i = 0; i < cmd.count(); i++) cmdnc[i]=cmd.at(i).toLatin1();
    emit EmitWriteCmd(cmdnc,cmd.count());
}
void SDRs::get_mode()
{
    // IF;  answer -> 144.195.000 ->IF00144195000........ 29=MD;
    char cmdnc[50];
    QString cmd = ncmd[s_ModelID].getmode+";";
    for (int i = 0; i < cmd.count(); i++) cmdnc[i]=cmd.at(i).toLatin1();
    emit EmitWriteCmd(cmdnc,cmd.count());
}
void SDRs::SetReadyRead(QByteArray ar,int size0)
{
    for (int i = 0; i < size0; i++)
    {
        if (ar[i]==(char)FI)//;=hex value ??? end of word EOM
        {
        	//qDebug()<<"SDRS READ ALL COMMAND="<<(QString(s_read_array.toHex()))<<s_read_array.size();
            int size = s_read_array.size();
            if (s_ModelID==0 || s_ModelID==1)//PowerSDR, Thetis
            {            	
                if (size < 37)//37 bez ;
                {
                    s_read_array.clear();
                    return;
                }//I=49 F=46
                if ((s_CmdID==GET_FREQ || s_CmdID==GET_MODE) &&
                        s_read_array[0]==(char)0x49 && s_read_array[1]==(char)0x46)
                {
                    QByteArray tfreq;
                    tfreq.append(s_read_array.mid(2,11));
                    unsigned long long f = tfreq.toLongLong();
                    emit EmitReadedInfo(GET_FREQ,QString("%1").arg(f));

                    QString smode = "WRONG_MODE";
                    if (s_read_array[29]==(char)0x32) smode = "USB";//0x32=USB=2                        
                    else if (s_read_array[29]==(char)0x39) smode = "DIGU";// DATA-USB=09                                                  
                    emit EmitReadedInfo(GET_MODE,smode);
                    s_CmdID = -1;//I Find my answer no need more
                }
            }
            else if (s_ModelID==2)//SmartSDR
            {
                if (size < 40)//37 bez ;
                {
                    s_read_array.clear();
                    return;
                }//Z=5A Z=5A I=49 F=46
                QString sid =(QString)s_read_array.mid(0,4);//qDebug()<<"SDRS"<<sid;
                if ((s_CmdID==GET_FREQ || s_CmdID==GET_MODE) && sid=="ZZIF")
                {
                    QByteArray tfreq;
                    tfreq.append(s_read_array.mid(4,11));
                    unsigned long long f = tfreq.toLongLong();
                    emit EmitReadedInfo(GET_FREQ,QString("%1").arg(f));

                    QString smode = "WRONG_MODE";
                    if (s_read_array[31]==(char)0x30 && s_read_array[32]==(char)0x31) smode = "USB";//0x30=0 0x31=1 USB=01                        
                    else if (s_read_array[31]==(char)0x30 && s_read_array[32]==(char)0x37) smode = "DIGU";// DATA-USB=07                                                    
                    emit EmitReadedInfo(GET_MODE,smode);
                    s_CmdID = -1;//I Find my answer no need more
                }
            }
            else if (s_ModelID>2 && s_ModelID<11)//FlexRadio SmartSDR Slice A-H
            {
                if (size < 6)//6 bez ;
                {
                    s_read_array.clear();
                    return;
                }//Z=5A Z=5A I=49 F=46
                QString sid =(QString)s_read_array.mid(0,4); //qDebug()<<"SDRS"<<size<<s_read_array;
                if (s_CmdID==GET_FREQ && sid=="ZZFA" && size>14)
                {
                    QByteArray tfreq;
                    tfreq.append(s_read_array.mid(4,11));
                    unsigned long long f = tfreq.toLongLong();
                    emit EmitReadedInfo(GET_FREQ,QString("%1").arg(f));
                    s_CmdID = -1;//I Find my answer no need more*/
                }
                if (s_CmdID==GET_MODE && sid=="ZZMD" && size>5)
                {
                    QString smode = "WRONG_MODE";
                    if (s_read_array[4]==(char)0x30 && s_read_array[5]==(char)0x31) smode = "USB";//0x30=0 0x31=1 USB=01                        
                    else if (s_read_array[4]==(char)0x30 && s_read_array[5]==(char)0x37) smode = "DIGU";// DATA-USB=07                                                    
                    emit EmitReadedInfo(GET_MODE,smode);
                    s_CmdID = -1;//I Find my answer no need more*/
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