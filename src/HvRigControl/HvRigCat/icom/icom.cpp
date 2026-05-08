/* MSHV Part from RigControl
 * Copyright 2015 Hrisimir Hristov, LZ2HV
 * May be used under the terms of the GNU General Public License (GPL)
 */
#include "icom.h"
#include "icom_def.h"
#include "../hvutils.h"
//#include <QtGui>

#define PR		    0xfe
#define C_CTL_PTT	0x1c		/* Control Transmit On/Off, Sc */
#define FI		    0xfd		/* End of message code */
#define CTRLID		0xe0		/* Controllers's default address */

////////////////////////////////////////////////////////new read com
#define COL		     0xfc		/* CI-V bus collision detected */
#define C_SET_FREQ	 0x05		/* Set frequency data(1) */
#define C_RD_FREQ	 0x03		/* Read display frequency */
#define C_SET_MODE	 0x06		/* Set mode data, Sc */
#define N_SETRD_MOD  0x26		/* Set mode data USB, Sc */
#define C_RD_MODE	 0x04		/* Read display mode */
#define MAXFRAMELEN 56
//#define CIV_731_MODE 1          /*priv->civ_731_mode ? 4:5*  -> 0=1HZ 1=10Hz   */
////////////////////////////////////////////////////////end new read com

#define S_PTT		0x00
#define S_ANT_TUN	0x01	/* Auto tuner 0=OFF, 1 = ON, 2=Start Tuning */

#define S_LSB	    0x00    /* Set to LSB */
#define S_USB	    0x01	/* Set to USB */
#define S_R7000_SSB	0x05	/* Set to SSB on R-7000 */
#define S_DIGU	    0x08    //DATA-USB=0x08

typedef struct
{
    unsigned char addr;
    bool newm;
}
addr_newm;
static const addr_newm icom_addr_newm[ICOM_COUNT] =
    {
        /*IC-7000*/{0x70,false},
        /*IC-7100*/{0x88,true},
        /*IC-7200*/{0x76,false},
        /*IC-7300*/{0x94,true},
        /*IC-7300MK2*/{0xb6,true},//2.76.6
        /*IC-705*/{0xa4,true},
        /*IC-756*/{0x50,false},
        /*IC-756PRO*/{0x5c,false},
        /*IC-756PROII*/{0x64,false},
        /*IC-756PROIII*/{0x6e,false},
        /*IC-706*/{0x48,false},
        /*IC-706MkII*/{0x4e,false},
        /*IC-706MkIIG*/{0x58,false},
        /*IC-905*/{0xac,false},
        /*IC-9100*/{0x7c,false},
        /*IC-9700*/{0xa2,true},//2.76.4 //false
        /*IC-7800*/{0x6a,false},
        /*IC-785x*/{0x8e,false},
        /*IC-7700*/{0x74,false},
        /*IC-7760*/{0xb2,true},//true = new filter 0x01,0x02,0x03 FIL1(wide), FIL2 (mid) or FIL3 (narrow).
        /*IC-7600*/{0x7a,true},
        /*IC-7610*/{0x98,true},
        /*IC-7410*/{0x80,false},
        /*IC-1275*/{0x18,false},
        /*IC-970*/{0x2e,false},
        /*IC-910*/{0x60,false},
        /*IC-821H*/{0x4c,false},
        /*IC-820H*/{0x42,false},
        /*IC-781*/{0x26,false},
        /*IC-775*/{0x46,false},
        /*IC-765*/{0x2c,false},
        /*IC-761*/{0x1e,false},
        /*IC-751*/{0x1c,false},
        /*IC-746*/{0x56,false},
        /*IC-746PRO*/{0x66,false},
        /*IC-738*/{0x44,false},
        /*IC-737*/{0x3c,false},
        /*IC-736*/{0x40,false},
        /*IC-735*/{0x04,false},
        /*IC-728*/{0x38,false},
        /*IC-726*/{0x30,false},
        /*IC-725*/{0x28,false},
        /*IC-718*/{0x5e,false},
        /*IC-707*/{0x3e,false},
        /*IC-703*/{0x68,false},
        /*IC-475*/{0x14,false},
        /*IC-471*/{0x22,false},
        /*IC-275*/{0x10,false},
        /*IC-275*/{0x20,false},
        /*IC-92D*/{0x20,false},// niama kod
        /*IC-78*/{0x62,false},
        /*IC-R10*/{0x52,false},
        /*IC-R20*/{0x6c,false},
        /*IC-R71*/{0x1a,false},
        /*IC-R72*/{0x32,false},
        /*IC-R75*/{0x5a,false},
        /*IC-R7000*/{0x08,false},
        /*ICR-8500*/{0x4a,false},
        /*IC-R7100*/{0x34,false},
        /*IC-R9000*/{0x2a,false},
        /*IC-R9500*/{0x72,false},
        /*IC-RX7*/{0x78,false},
        /*IC ID-1*/{0x01,false},
        /*ID-5100*/{0x8c,false},
        /*Xiegu G106*/{0x70,true},//0x90 ?
        /*Xiegu G90*/{0x70,true},//0x70 ?        
        /*Xiegu X108G*/{0x70,true},
        /*Xiegu X5105*/{0x70,true},
        /*Xiegu X6100*/{0xa4,true},//0x70 ?
        /*Xiegu X6200*/{0xa4,true} // ?
    };
Icom::Icom(int ModelID,QWidget *parent)
        : QWidget(parent)
{
    s_ModelID = ModelID;
    newSRMod = icom_addr_newm[s_ModelID].newm;
    s_model_addr = icom_addr_newm[s_ModelID].addr;
    s_rig_name = rigs_icom[s_ModelID].name;
    s_CmdID = -1;
    s_read_array.clear(); //qDebug()<<s_rig_name<<newSRMod;
}
Icom::~Icom()
{
    //qDebug()<<"Delete"<<rigs_icom[s_ModelID].name;
}
/*#include <unistd.h>
struct icom_cmd_init
{
    int c_init;
    unsigned char init0[3][5];
};
static const icom_cmd_init init_0[ICOM_COUNT] =
    {
        {2,{{0x1a,0x05,0x00,0x92,0x00},{0x1a,0x05,0x00,0x79,0x00},{0}}},//IC-7000
        {2,{{0x1a,0x05,0x00,0x95,0x00},{0x1a,0x05,0x00,0x32,0x00},{0}}},//IC-7100
        {2,{{0x1a,0x03,0x48,0x00,0x00},{0x1a,0x03,0x37,0x00,0x00},{0}}},//IC-7200  FEFE76E0.1A0348.00.FD   FEFE76E0.1A0337.00.FD
        {3,{{0x1a,0x05,0x00,0x75,0x01},{0x1a,0x05,0x00,0x71,0x00},{0x1a,0x05,0x00,0x53,0x00}}},//IC-7300
        {3,{{0x1a,0x05,0x00,0x75,0x01},{0x1a,0x05,0x00,0x71,0x00},{0x1a,0x05,0x00,0x53,0x00}}},//IC-7300MK2
        {3,{{0x1a,0x05,0x01,0x32,0x01},{0x1a,0x05,0x01,0x31,0x00},{0x1a,0x05,0x00,0x70,0x00}}},//IC-705
        {0,{{0},{0},{0}}},//IC-756
        {0,{{0},{0},{0}}},//IC-756PRO
        {2,{{0x1a,0x05,0x42,0x00,0x00},{0x1a,0x05,0x43,0x00,0x00},{0}}},//IC-756PROII FEFE64E0.1A0542.00.FD FEFE64E0.1A0543.00.FD
        {0,{{0},{0},{0}}},//IC-756PROIII
        {0,{{0},{0},{0}}},//IC-706
        {0,{{0},{0},{0}}},//IC-706MkII
        {0,{{0},{0},{0}}},//IC-706MkIIG
        {2,{{0x1a,0x05,0x00,0x58,0x00},{0x1a,0x05,0x00,0x40,0x00},{0}}},//IC-9100
        {3,{{0x1a,0x05,0x01,0x30,0x01},{0x1a,0x05,0x01,0x27,0x00},{0x1a,0x05,0x00,0x67,0x00}}},//IC-9700
        {3,{{0x1a,0x05,0x01,0x01,0x00},{0x1a,0x05,0x01,0x02,0x00},{0x1a,0x05,0x00,0x97,0x00}}},//IC-7800
        {3,{{0x1a,0x05,0x01,0x58,0x01},{0x1a,0x05,0x01,0x55,0x00},{0x1a,0x05,0x01,0x40,0x00}}},//IC-785x
        {3,{{0x1a,0x05,0x00,0x95,0x00},{0x1a,0x05,0x00,0x96,0x00},{0x1a,0x05,0x00,0x91,0x00}}},//IC-7700
        {3,{{0x1a,0x05,0x00,0x97,0x00},{0x1a,0x05,0x00,0x98,0x00},{0x1a,0x05,0x00,0x89,0x00}}},//IC-7600
        {3,{{0x1a,0x05,0x01,0x16,0x01},{0x1a,0x05,0x01,0x12,0x00},{0x1a,0x05,0x00,0x62,0x00}}},//IC-7610
        {2,{{0x1a,0x05,0x00,0x40,0x00},{0x1a,0x05,0x00,0x33,0x00},{0}}},//IC-7410
        {0,{{0},{0},{0}}},//IC-1275
        {0,{{0},{0},{0}}},//IC-970
        {0,{{0},{0},{0}}},//IC-910
        {0,{{0},{0},{0}}},//IC-821H
        {0,{{0},{0},{0}}},//IC-820H
        {0,{{0},{0},{0}}},//IC-781
        {0,{{0},{0},{0}}},//IC-775
        {0,{{0},{0},{0}}},//IC-765
        {0,{{0},{0},{0}}},//IC-761
        {0,{{0},{0},{0}}},//IC-751
        {0,{{0},{0},{0}}},//IC-746
        {2,{{0x1a,0x05,0x36,0x00,0x00},{0x1a,0x05,0x37,0x00,0x00},{0}}},//IC-746PRO FEFE66E0.1A0536.00.FD   FEFE66E0.1A0537.00.FD
        {0,{{0},{0},{0}}},//IC-738
        {0,{{0},{0},{0}}},//IC-737
        {0,{{0},{0},{0}}},//IC-736
        {0,{{0},{0},{0}}},//IC-735
        {0,{{0},{0},{0}}},//IC-728
        {0,{{0},{0},{0}}},//IC-726
        {0,{{0},{0},{0}}},//IC-725
        {0,{{0},{0},{0}}},//IC-718
        {0,{{0},{0},{0}}},//IC-707
        {0,{{0},{0},{0}}},//IC-703
        {0,{{0},{0},{0}}},//IC-475
        {0,{{0},{0},{0}}},//IC-471
        {0,{{0},{0},{0}}},//IC-275
        {0,{{0},{0},{0}}},//IC-275
        {0,{{0},{0},{0}}},//IC-92D
        {0,{{0},{0},{0}}},//IC-78
        {0,{{0},{0},{0}}},//IC-R10
        {0,{{0},{0},{0}}},//IC-R20
        {0,{{0},{0},{0}}},//IC-R71
        {0,{{0},{0},{0}}},//IC-R72
        {0,{{0},{0},{0}}},//IC-R75
        {0,{{0},{0},{0}}},//IC-R7000
        {0,{{0},{0},{0}}},//ICR-8500
        {0,{{0},{0},{0}}},//IC-R7100
        {0,{{0},{0},{0}}},//IC-R9000
        {0,{{0},{0},{0}}},//IC-R9500
        {0,{{0},{0},{0}}},//IC-RX7
        {0,{{0},{0},{0}}},//IC ID-1
        {0,{{0},{0},{0}}} //ID-5100
    };
void Icom::SetOnOffCatCommand(bool f, int model_id, int fact_id)
{
    if (model_id!=s_ModelID || fact_id!=ICOM_ID)
        return;

    if (f && init_0[s_ModelID].c_init>0)
    {
    	int c_cmd = 5;
    	if (s_rig_name=="IC-7200" || s_rig_name=="IC-746PRO" || s_rig_name=="IC-756PROII") c_cmd = 4;
        unsigned char cmd[6];
        for (int i = 0; i<init_0[s_ModelID].c_init; ++i)
        {
            for (int j = 0; j<c_cmd; ++j) cmd[j]=init_0[s_ModelID].init0[i][j];
            make_cmd_all(cmd,c_cmd);
            //qDebug()<<"Icom CAT ON="<<QString("%1").arg(cmd[0], 0, 16)
            //<<QString("%1").arg(cmd[1], 0, 16)
            //<<QString("%1").arg(cmd[2], 0, 16)
            //<<QString("%1").arg(cmd[3], 0, 16)
            //<<QString("%1").arg(cmd[4], 0, 16);
            if (i<init_0[s_ModelID].c_init-1) usleep(150000);
        }
    }
    //else if (!f)
    //{
    //qDebug()<<"Icom CAT OFF SetOnOffCatCommand========================"<<f;
    //}
}*/
void Icom::SetCmd(CmdID i,ptt_t ptt,QString str)
{
    s_CmdID = -1;
    switch (i)
    {
    case GET_SETT:
        emit EmitRigSet(rigs_icom[s_ModelID]);
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
void Icom::make_cmd_all(unsigned char *comand_subcomand,int count_c)
{
    unsigned char cmd_all[128];
    int count = 0;
    cmd_all[0]=PR;
    count++;
    cmd_all[count]=PR;
    count++;
    cmd_all[count]=s_model_addr;
    count++;
    cmd_all[count]=CTRLID;
    count++;

    for (int i = 0; i<count_c; i++)
    {
        cmd_all[count]=comand_subcomand[i];
        count++;
    }

    cmd_all[count]=FI;
    count++;
    emit EmitWriteCmd((char *)cmd_all,count);
}
void Icom::set_ptt(ptt_t ptt)
{
    unsigned char cmd[3];
    if (ptt==RIG_PTT_ON)
    {
        cmd[0] = C_CTL_PTT;
        cmd[1] = S_PTT;
        cmd[2] = 1;
    }
    else
    {
        cmd[0] = C_CTL_PTT;
        cmd[1] = S_PTT;
        cmd[2] = 0;
    }
    make_cmd_all(cmd,3);
}
void Icom::set_freq(unsigned long long freq)
{
    unsigned char freqbuf[MAXFRAMELEN];//, ackbuf[MAXFRAMELEN];
    unsigned char cmd[MAXFRAMELEN+5];
    int freq_len = 5;
    int count_cmd = 0;
    //freq_len = 1 ? 4:5;  qDebug()<<"freq_len="<<freq_len;
    //QString mod731 = rigs_icom[s_ModelID].name;
    if (s_rig_name=="IC-735" || s_rig_name=="IC-820H" || s_rig_name=="IC-821H") freq_len = 4;
    to_bcd_(freqbuf, freq, freq_len*2);
    cmd[count_cmd] = C_SET_FREQ;
    count_cmd++;
    for (int i = 0; i<freq_len; i++)
    {
        cmd[count_cmd]=freqbuf[i];
        count_cmd++; //qDebug()<<freq<<"freqbuf="<<i<<(unsigned char)freqbuf[i];
    }
    make_cmd_all(cmd,count_cmd);
    //old 0x05 FEFE88E0.05.0000000000.FD
    //newSRMod FEFE88E0.25.00.0000000000.FD
}
void Icom::set_mode(QString str)
{
    unsigned char cmd[10];
    if (newSRMod)
    {
        if (str=="USB")
        {
            cmd[0] = N_SETRD_MOD;	//new command 0x26
            cmd[1] = 0x00;			//vfo Selected
            cmd[2] = 0x01;			//usb
            cmd[3] = 0x00;			//data
            cmd[4] = 0x01; 			//new filter 0x01,0x02,0x03 FIL1(wide), FIL2 (mid) or FIL3 (narrow).
            make_cmd_all(cmd,5);	//FEFE94E0.26.00.01.01.03.FD
        }
        else if (str=="LSB")
        {
            cmd[0] = N_SETRD_MOD;	//new command 0x26
            cmd[1] = 0x00;			//vfo Selected
            cmd[2] = 0x00;			//lsb
            cmd[3] = 0x00;			//data
            cmd[4] = 0x01; 			//new filter 0x01,0x02,0x03 FIL1(wide), FIL2 (mid) or FIL3 (narrow).
            make_cmd_all(cmd,5);	//FEFE94E0.26.00.01.01.03.FD
        }
        else if (str=="DIGU")
        {
            cmd[0] = N_SETRD_MOD;	//new command 0x26
            cmd[1] = 0x00;			//vfo Selected
            cmd[2] = 0x01;			//usb
            cmd[3] = 0x01;			//data
            cmd[4] = 0x01; 			//new filter 0x01,0x02,0x03 FIL1(wide), FIL2 (mid) or FIL3 (narrow).
            make_cmd_all(cmd,5);	//FEFE94E0.26.00.01.01.03.FD
        }
    }
    else
    {
        unsigned char mode = S_LSB;
        unsigned char bpmode = 0x01;//old filter wide=0x01 narrow=0x02
        if (str=="USB")
        {
            if (s_rig_name=="IC-R7000")
            {
                mode = S_R7000_SSB;
                bpmode = 0x00;
            }
            else mode = S_USB;
        }
        else if (str=="LSB")  mode = S_LSB;
        else if (str=="DIGU") mode = S_DIGU;//2.76.4 0x08
        else return;
        cmd[0] = C_SET_MODE;
        cmd[1] = mode;
        cmd[2] = bpmode;
        make_cmd_all(cmd,3); //FEFE94E0.06.00.01.FD
    }
}
void Icom::get_freq()
{
    unsigned char cmd[MAXFRAMELEN];
    cmd[0] = C_RD_FREQ;
    make_cmd_all(cmd,1);
}
void Icom::get_mode()
{
    unsigned char cmd[MAXFRAMELEN];
    cmd[0] = C_RD_MODE;
    if (newSRMod)
    {
        cmd[0] = N_SETRD_MOD;
        cmd[1] = 0x00; //vfo Selected
        make_cmd_all(cmd,2);
    }
    else make_cmd_all(cmd,1);
}
QString Icom::GetModeStr(unsigned char mod,unsigned char dat)
{
    QString smode = "WRONG_MODE";
    if 		(mod==0x00 && dat==0x00) smode = "LSB"; //LSB=0x00
    else if (mod==0x00 && dat==0x01) smode = "DIGL";//LSBD
    else if (mod==0x01 && dat==0x00) smode = "USB"; //USB=0x01
    else if (mod==0x01 && dat==0x01) smode = "DIGU";//USBD
    else if (mod==0x02 && dat==0x00) smode = "AM";  //AM =0x02
    else if (mod==0x02 && dat==0x01) smode = "AM-D"; //AM-D
    else if (mod==0x03 && dat==0x00) smode = "CWL";//CW-L=0x03
    else if (mod==0x04 && dat==0x00) smode = "R-L";//RTTY=0x04
    else if (mod==0x05 && dat==0x00) smode = "FM";  //FM =0x05
    else if (mod==0x05 && dat==0x01) smode = "FM-D";//FM-D
    else if (mod==0x07 && dat==0x00) smode = "CWU";//CW-R=0x07
    else if (mod==0x08 && dat==0x00) smode = "R-U";//RTTY-R=0x08
    return smode;
}
void Icom::SetReadyRead(QByteArray ar,int size0)
{
    for (int i = 0; i < size0; i++)
    {
        if (ar[i]==(char)FI)//FI=0xfd end of word EOM
        {
            //qDebug()<<"ICOM READ ALL COMMAND="<<(QString(s_read_array.toHex()))<<s_read_array.size();
            int size = s_read_array.size();
            //rejekt small array
            if (size < 5)
            {
                s_read_array.clear();//qDebug()<<"NO INAF SIZE";
                return;
            }
            //rejekt no icom in network
            if (s_read_array[0]!=(char)PR || s_read_array[1]!=(char)PR)
            {
                s_read_array.clear();//qDebug()<<"NO ICOM IN NETWORK";
                continue;//return; <- stop 2.73 odl icom start -> FFFFFFFFFFFF.FFFFFFFF.FF.0000000000.FF
            }
            //rejekt no my correspondent in network or my full duplex signal
            if (s_read_array[2]!=(char)CTRLID || s_read_array[3]!=(char)s_model_addr)
            {
                s_read_array.clear();//qDebug()<<"NO MAY CORRESPONDENT OR FULL DUPLEX SIGNAL";
                continue;//2.55 may be no end, try to end,  may be echo is ON
            }
            if (s_CmdID==GET_FREQ && s_read_array[4]==(char)C_RD_FREQ)//my qestion for freq and freq filter
            {
                if (size==10 || size==9)//bez fd=fi (size==10 || size==9)
                {
                    unsigned char fbsd[10];
                    int cfbsd = 0;
                    for (int j=5; j < size; j++)//2.12
                    {
                        fbsd[cfbsd]=(unsigned char)s_read_array[j];
                        cfbsd++;
                    }
                    unsigned long long f = from_bcd_(fbsd, cfbsd*2);
                    emit EmitReadedInfo(GET_FREQ,QString("%1").arg(f));
                    s_CmdID = -1;//I Find my answer no need more
                }
            }
            if (newSRMod)
            {
                if (s_CmdID==GET_MODE && s_read_array[4]==(char)N_SETRD_MOD)
                {
                    if (size==9)//bez fd=fi FEFE94E0.26.00.01.01.03.FD  
                    {
                    	if (s_read_array[5]==(char)0x00) //vfo Selected -> s_read_array[5]==(char)0x00
                    	{
                        	unsigned char mod = (unsigned char)s_read_array[6];
                        	unsigned char dat = (unsigned char)s_read_array[7];
                        	QString smode = GetModeStr(mod,dat);
                        	emit EmitReadedInfo(GET_MODE,smode);
                        	s_CmdID = -1;//I Find my answer no need more                    		
                   		}
                    }
                }
            }
            else
            {
                if (s_CmdID==GET_MODE && s_read_array[4]==(char)C_RD_MODE)//my qestion for mode and mode filter
                {
                    if (size==7)//bez fd=fi  FEFE94E0.06.00.01.FD
                    {
                        unsigned char mod = (unsigned char)s_read_array[5];
                        QString smode = GetModeStr(mod,0x00);
                        emit EmitReadedInfo(GET_MODE,smode);
                        s_CmdID = -1;//I Find my answer no need more
                    }
                }
            }
            s_read_array.clear();
        }
        else if (ar[i]==(char)COL)//COL=0xfc collision detected
        {
            s_read_array.clear();//qDebug()<<"COLYSION READ";
            s_CmdID = -1;//see you in nex poll
            return;
        }
        else s_read_array.append(ar[i]);
    }
    if (s_read_array.size()>512) s_read_array.clear(); //2.55 protection  max>256 something is wrong
}

