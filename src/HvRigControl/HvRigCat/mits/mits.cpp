/* MSHV Part from RigControl
 * Copyright 2015 Hrisimir Hristov, LZ2HV
 * May be used under the terms of the GNU General Public License (GPL)
 */
#include "mits_def.h"
#include "mits.h"
#include <unistd.h>

//#include <QtGui>

#define CR (0x0d) 

Mits::Mits(int ModelID,QWidget *parent)
        : QWidget(parent)
{
    s_ModelID = ModelID;
    ////////////////////////////////////////////////////////new read com
    //s_rig_name = rigs_kenwood[s_ModelID].name;
    s_CmdID = -1;
    s_read_array.clear();
    ////////////////////////////////////////////////////////end new read com
}
Mits::~Mits()
{
    //qDebug()<<"Delete"<<rigs_kenwood[s_ModelID].name;
}
void Mits::MakeCommandAndSend(QString sdata)
{
    QByteArray data = sdata.toLatin1();
    int c = 0;
    for (int i = 0; i < data.length(); ++i) c = (c ^ (int)data.at(i));
    c = c ^ 0xff;
    QString cs = QString::number(c,16);
    QString all = "$"+sdata+cs+"\r";
    char cmd[256];
    for (int i = 0; i < all.count(); i++) cmd[i]=all.at(i).toLatin1();
    emit EmitWriteCmd(cmd,all.count());
    //qDebug() << all;
}
void Mits::SetOnOffCatCommand(bool f, int model_id, int fact_id)
{
	if (model_id!=s_ModelID || fact_id!=MITS_ID) return;	
    if (f) 
    {
    	emit EmitPttDtr(true);
    	MakeCommandAndSend("PSEAS,43,1,*"); 			//HAM = '$PSEAS,43,1,*9B'             # Ham mode (transmit 1.6-30 MHz, no limits)
    	usleep(60000);
    	MakeCommandAndSend("PSEAS,38,,,,1,,,,1,1,,*"); 	//DB9 = '$PSEAS,38,,,,1,,,,1,1,,*BB'  # DB9 sound input (for soundcard)
    	usleep(60000);
    	MakeCommandAndSend("PSEAS,46,2,3,3,3*"); 		//AG3 = '$PSEAS,46,2,3,3,3*AE'        # AGC Fast, no feed forward
    	//usleep(50000);
    	//MakeCommandAndSend("PSEAS,60*");  			//DIA = '$PSEAS,60*hh'                # Diagnostic  	
   	}
}
void Mits::SetCmd(CmdID i,ptt_t ptt,QString str)
{
    switch (i)
    {
    case GET_SETT:
        emit EmitRigSet(rigs_mits[s_ModelID]);
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
void Mits::set_ptt(ptt_t ptt)
{
    if (ptt==RIG_PTT_ON) 
    {
    	MakeCommandAndSend("PSEAS,16,T*"); //STR = '$PSEAS,16,T*D2'              # Transmit but also needs DTR
    	emit EmitPttDtr(false);    	 	
   	}
    else 
    {
    	emit EmitPttDtr(true); 
    	MakeCommandAndSend("PSEAS,16,R*"); //SRX = '$PSEAS,16,R*D4'              # Receive   	
   	}
}
////////////////////////////////////////////////////////new read com
void Mits::set_freq(unsigned long long freq)
{
    //$PSEAS,15,,07070000,07070000*85     # Set freq to 7.070 MHz
    //$PSEAS,15,,03573000,03573000*85     # Set freq to 3.573 MHz
    QString cmd = "PSEAS,15,,";
    QString f = (QString("%1").arg(freq,8,10,QChar('0')));
    cmd.append(f+","+f+"*");
    MakeCommandAndSend(cmd);  //qDebug()<<cmd;
}
void Mits::set_mode(QString str)
{
    //USB = '$PSEAS,16,U*D3'              # USB (300-2700 Hz)
    if (str=="USB") MakeCommandAndSend("PSEAS,16,U*");
}
void Mits::get_freq()
{
    // $PSEAS,10*AC   statuc
    MakeCommandAndSend("PSEAS,10*");
}
void Mits::get_mode()
{
    // $PSEAS,10*AC   statuc
    MakeCommandAndSend("PSEAS,10*");
    //MakeCommandAndSend("PSEAR,11,0,2182000,2182000,,R,H,U,S*");// test mode usb
}
void Mits::SetReadyRead(QByteArray ar,int size)
{
    for (int i = 0; i < size; i++)
    {
        //# Response 11, status: $PSEAR,11,0,2182000,2182000,,R,H,E,S*hh
        //$PSEAR,11,CHAN,RXFREQ,TXFREQ,TAG,MODE FLAGS*hh
        if (ar[i]==(char)CR)//;=hex value ??? end of word EOM
        {
            if (size < 7)//10 bez end  $PSEAR <- as minimum
            {
                //qDebug()<<"NO INAF SIZE"<<s_read_array<<size;
                s_read_array.clear();
                return;
            }
            //qDebug()<<"SEA READ ALL COMMAND="<<(QString(s_read_array.toHex()))<<s_read_array.size();
            if ((s_CmdID==GET_FREQ || s_CmdID==GET_MODE) && s_read_array[0]==(char)0x24 && s_read_array[1]==(char)0x50)
            {
                QString in = s_read_array.data();
                QStringList inl = in.split(",");
                QString nom = "";
                if (inl.count()>1) nom = inl.at(1);
                //qDebug()<<"SEA READ ALL COMMAND="<<inl;
                if(nom =="11")// comamnd 11
                {
                    if (inl.count()>3)
                    {
                        QString tfreq = inl.at(3);
                        unsigned long long f = tfreq.toLongLong();
                        emit EmitReadedInfo(GET_FREQ,QString("%1").arg(f));
                    }
                    if (inl.count()>8)
                    {
                        QString smode = "WRONG_MODE";
                        QString tfreq = inl.at(8);
                        if (tfreq=="U") smode = "USB";
                        emit EmitReadedInfo(GET_MODE,smode);
                    }
                }
                s_CmdID = -1;//I Find my answer no need more
            }
            s_read_array.clear();
        }
        else
            s_read_array.append(ar[i]);
    }
    if (s_read_array.size()>1024) s_read_array.clear(); //2.55 protection 
}
