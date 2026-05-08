/* MSHV Part from RigControl
 * Copyright 2015 Hrisimir Hristov, LZ2HV
 * May be used under the terms of the GNU General Public License (GPL)
 */
#include "tentec_def.h"
#include "tentec.h"
//#include <QtGui>

#define TENTEC_CMD_LENGTH 7
//#define CAT_7B 7 
//#define CAT_5B 5
#define PTT_STR 202

//#define EOM "\015"	/* CR */
//#define FI 0x3b
#define FI 0x0d /* CR */
#define FD 0xfd
#include "../hvutils.h"

#define S_LSB	    0x00     /* Set to LSB */
#define S_USB	    0x01		/* Set to USB */

#define PR		    0xfe
#define CTRLID		0xe0
#define C_RD_MODE	0x04
#define C_RD_FREQ	0x03

//unsigned long long freq_t;
//#define MHz(f)	((unsigned long long)((f)*(unsigned long long)1000000))

enum native_cmd_e {
    NATIVE_CAT_PTT_ON = 0,
    NATIVE_CAT_PTT_OFF,
    NATIVE_CAT_SET_FREQ,
    NATIVE_CAT_GET_FREQ,
    NATIVE_CAT_SET_MODE,
    NATIVE_CAT_GET_MODE
};

struct tentec_cmd_set
{
    unsigned char ncomp;		        /* 1 = complete, 0 = incomplete, needs extra info */
    unsigned char nseq[TENTEC_CMD_LENGTH];	/* native cmd sequence */
};

typedef struct tentec_cmd_set tentec_cmd_set_t;

static const tentec_cmd_set_t ncmd[TENTEC_COUNT][6] =
    {
        //TT-565 Orion
        {{ PTT_STR, { "*TK\r" } }, /* \r=0x0D ili->. *identif newcat NEWCAT TX1; ptt on */
         { PTT_STR, { "*TU\r" } }, /* \r=0x0D ili->. identif newcat NEWCAT TX0;  ptt off */
         { PTT_STR, { "*AF" } },   // set frq
         { PTT_STR, { "?AF\r" } },   // get frq
         { PTT_STR, { "*RMM" } },   //set mod
         { PTT_STR, { "?RMM\r" } },}, //get mode
        //TT-599 Eagle
        {{ PTT_STR, { "*TK\r" } }, /* \r=0x0D ili->. *identif newcat NEWCAT TX1; ptt on */
         { PTT_STR, { "*TU\r" } }, /* \r=0x0D ili->. identif newcat NEWCAT TX0;  ptt off */
         { PTT_STR, { "*AF" } },   // set frq
         { PTT_STR, { "?AF\r" } },   // get frq
         { PTT_STR, { "*RMM" } },   //set mod
         { PTT_STR, { "?RMM\r" } },}, //get mode
        //TT-588 Omni VII
        {{ 5, { 0x2a, 0x54, 0x04, 0x00, 0x0d } }, /* ptt on */
         { 5, { 0x2a, 0x54, 0x00, 0x00, 0x0d } }, /* ptt off */
         { PTT_STR, { "*A" } },   // set frq
         { PTT_STR, { "?A\r" } },   // get frq
         { PTT_STR, { "*M" } },   //set mod
         { PTT_STR, { "?M\r" } },}, //get mode
        //TT-538 Jupiter
        {{ PTT_STR, { "Q1\r" } }, /* \r=0x0D ili->. identif newcat NEWCAT TX1; ptt on */
         { PTT_STR, { "Q0\r" } }, /* \r=0x0D ili->. identif newcat NEWCAT TX0;  ptt off */
         { PTT_STR, { "*A" } },   // set frq
         { PTT_STR, { "?A\r" } },   // get frq
         { PTT_STR, { "*M" } },   //set mod
         { PTT_STR, { "?M\r" } },}, //get mode
        //TT-516 Argonaut V
        {{ PTT_STR, { "#1\r" } }, /* \r=0x0D ili->. identif newcat NEWCAT TX1; ptt on */
         { PTT_STR, { "#0\r" } }, /* \r=0x0D ili->. identif newcat NEWCAT TX0;  ptt off */
         { PTT_STR, { "*A" } },   // set frq
         { PTT_STR, { "?A\r" } },   // get frq
         { PTT_STR, { "*M" } },   //set mod
         { PTT_STR, { "?M\r" } },}, //get mode
        //TT-563 Omni VI Plus as icom
        {{ 7, { 0xfe, 0xfe, 0x04, 0xe0, 0x16, 0x01, 0xfd } }, /* ptt on */
         { 7, { 0xfe, 0xfe, 0x04, 0xe0, 0x16, 0x02, 0xfd } }, /* ptt off */
         { 5, { 0xfe, 0xfe, 0x04, 0xe0, 0x05} },  /* set frq */
         { 5, { 0xfe, 0xfe, 0x04, 0xe0, 0x03} },  /* get freq */
         { 5, { 0xfe, 0xfe, 0x04, 0xe0, 0x06} },  /* set mod */
         { 5, { 0xfe, 0xfe, 0x04, 0xe0, 0x04} },},/* get mod */
        //TT-585 Paragon
        {{ PTT_STR, { "None" } }, /* ptt on */
         { PTT_STR, { "None" } }, /* ptt off */
         { PTT_STR, { "None" } }, /* ptt on */
         { PTT_STR, { "None" } }, /* ptt on */
         { PTT_STR, { "None" } }, /* ptt on */
         { PTT_STR, { "None" } },}, /* ptt on */
        //RX-350
        {{ PTT_STR, { "None" } }, /* ptt on */
         { PTT_STR, { "None" } }, /* ptt off */
         { PTT_STR, { "*A" } },   // set frq
         { PTT_STR, { "?A\r" } },   // get frq
         { PTT_STR, { "*M" } },   //set mod
         { PTT_STR, { "?M\r" } },}, //get mode
        //RX-340
        {{ PTT_STR, { "None" } }, /* ptt on */
         { PTT_STR, { "None" } }, /* ptt off */
         { PTT_STR, { "None" } }, /* ptt on */
         { PTT_STR, { "None" } }, /* ptt on */
         { PTT_STR, { "None" } }, /* ptt on */
         { PTT_STR, { "None" } },}, /* ptt on */
        //RX-331
        {{ PTT_STR, { "None" } }, /* ptt on */
         { PTT_STR, { "None" } }, /* ptt off */
         { PTT_STR, { "None" } }, /* ptt on */
         { PTT_STR, { "None" } }, /* ptt on */
         { PTT_STR, { "None" } }, /* ptt on */
         { PTT_STR, { "None" } },}, /* ptt on */
        //RX-320
        {{ PTT_STR, { "None" } }, /* ptt on */
         { PTT_STR, { "None" } }, /* ptt off */
         { PTT_STR, { "None" } }, /* ptt on */
         { PTT_STR, { "None" } }, /* ptt on */
         { PTT_STR, { "None" } }, /* ptt on */
         { PTT_STR, { "None" } },}, /* ptt on */
        //Delta II
        {{ PTT_STR, { "None" } }, /* ptt on */
         { PTT_STR, { "None" } }, /* ptt off */
         { 5, { 0xfe, 0xfe, 0x01, 0xe0, 0x05} },  /* set frq */
         { 5, { 0xfe, 0xfe, 0x01, 0xe0, 0x03} },  /* get freq */
         { 5, { 0xfe, 0xfe, 0x01, 0xe0, 0x06} },  /* set mod */
         { 5, { 0xfe, 0xfe, 0x01, 0xe0, 0x04} },},/* get mod */
        //TT-550
        //{{ 3, { 0x51, 0x01, 0x0d } }, /* ptt on Q1EOM */
        //{ 3, { 0x51, 0x00, 0x0d } },}, /* ptt off Q0EOM */
        {{ PTT_STR, { "Q1\r" } }, /* ptt on */
         { PTT_STR, { "Q0\r" } }, /* ptt off */
         { PTT_STR, { "None" } }, /* ptt on */
         { PTT_STR, { "None" } }, /* ptt off */
         { PTT_STR, { "None" } }, /* ptt on */
         { PTT_STR, { "None" } },}, /* ptt off */
    };

//#include <QtGui>

Tentec::Tentec(int ModelID,QWidget *parent)
        : QWidget(parent)
{
    s_ModelID = ModelID;
    ////////////////////////////////////////////////////////new read com
    //s_rig_name = rigs_kenwood[s_ModelID].name;
    s_CmdID = -1;
    s_read_array.clear();

    //#define FI 0x0d // CR \r
    //#define FD 0xfd // CR \r
    if (s_ModelID==5 || s_ModelID==11)
        EOM = FD;
    else
        EOM = FI;

    //TT-563 as icom Delta II
    if (s_ModelID==5)
        s_model_addr = 0x04;
    else if (s_ModelID==11)
        s_model_addr = 0x01;
    else
        s_model_addr = 0xff;
    ////////////////////////////////////////////////////////end new read com
}

Tentec::~Tentec()
{
    //qDebug()<<"Delete"<<rigs_yeasu[s_ModelID].name;
}

void Tentec::SetCmd(CmdID i,ptt_t ptt,QString str)
{
    switch (i)
    {
    case GET_SETT:
        emit EmitRigSet(rigs_tentec[s_ModelID]);
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
//#include <cstdio>
void Tentec::set_ptt(ptt_t ptt)
{
    unsigned char *cmd;		/* points to sequence to send */

    QString ide = (char*)ncmd[s_ModelID][NATIVE_CAT_PTT_ON].nseq;
    if (ide=="None")
        return;

    if (ptt==RIG_PTT_ON)
        cmd = (unsigned char *) &ncmd[s_ModelID][NATIVE_CAT_PTT_ON].nseq; // get native sequence /
    else
        cmd = (unsigned char *) &ncmd[s_ModelID][NATIVE_CAT_PTT_OFF].nseq;


    if (ncmd[s_ModelID][NATIVE_CAT_PTT_ON].ncomp == PTT_STR)//STRING MAX 7 BITS
    {
        int len = strlen((const char*)cmd);
        emit EmitWriteCmd((char *)cmd,len);
    }
    else
        emit EmitWriteCmd((char *)cmd,ncmd[s_ModelID][NATIVE_CAT_PTT_ON].ncomp);


    /*char cmdbuf[16];
    int cmd_len = sprintf (cmdbuf, "Q%c", ptt == 0 ? '0' : '1');
    //return (write_block (&rs->rigport, cmdbuf, cmd_len));
    emit EmitWriteCmd((char *)cmdbuf,cmd_len);*/

}

////////////////////////////////////////////////////////new read com
void Tentec::set_freq(unsigned long long freq)
{
    /*char buf[100], *p;
       int ret;
       #define FREQBUFSZ 16
       ret = num_snprintf(buf, FREQBUFSZ-1, "%.5f@", (double)freq/MHz(1));
       buf[FREQBUFSZ-1] = '\0';

       / replace decimal point with W 
       p = strchr(buf, '.');
       *p = 'W';
       qDebug()<<"set_freq"<<buf;*/

    QString ide = (char*)ncmd[s_ModelID][NATIVE_CAT_SET_FREQ].nseq;
    if (ide=="None")
        return;

    //TT-563 as icom Delta II
    if (s_ModelID==5 || s_ModelID==11)
    {
        unsigned char cmd[50];
        for (int i = 0; i<5; i++)
            cmd[i] = (unsigned char)ncmd[s_ModelID][NATIVE_CAT_SET_FREQ].nseq[i];
        unsigned char freqbuf[12];
        int freq_len = 5;
        if (s_ModelID==11)
            freq_len = 4;

        to_bcd_(freqbuf, freq, freq_len*2);//10 8 ???
        for (int i = 0; i<freq_len; i++)//5 4 ???
            cmd[i+5]=freqbuf[i];
        cmd[freq_len+5]=FD;
        emit EmitWriteCmd((char*)cmd,11);
    }
    else
    {
        char cmdnc[50];
        //char bytes[4];
        QString frq = (char*)ncmd[s_ModelID][NATIVE_CAT_SET_FREQ].nseq;
        if (s_ModelID==2 || s_ModelID==3 || s_ModelID==4 || s_ModelID==7)//2="TT-588,TT-538,TT-516 RX-350
        {
        	char bytes[4];
            //15000000->"2a4100e4e1c00d"
            bytes[3] = ((int) freq >> 24) & 0xff;
            bytes[2] = ((int) freq >> 16) & 0xff;
            bytes[1] = ((int) freq >>  8) & 0xff;
            bytes[0] =  (int) freq        & 0xff;
            frq.append((char)bytes[3]);
            frq.append((char)bytes[2]);
            frq.append((char)bytes[1]);
            frq.append((char)bytes[0]);
        }
        else //TT-565 TT-599
        {
            //*AF14100000<CR> (14.1MHz) cr=\r
            frq.append(QString("%1").arg(freq));
        }
        frq.append("\r");

        for (int i = 0; i < frq.count(); i++)
            cmdnc[i]=frq.at(i).toLatin1();
        emit EmitWriteCmd(cmdnc,frq.count());
    }
}
void Tentec::set_mode(QString str)
{
    QString ide = (char*)ncmd[s_ModelID][NATIVE_CAT_SET_MODE].nseq;
    if (ide=="None") return;

    //TT-563 as icom Delta II
    if (s_ModelID==5 || s_ModelID==11)
    {
        unsigned char cmd[50];
        for (int i = 0; i<5; i++)
            cmd[i] = (unsigned char)ncmd[s_ModelID][NATIVE_CAT_SET_MODE].nseq[i];

        //unsigned char bpmode = 0x01;  //0x01 <- slaga lentata na nai 6iroka 0x00<-ni6to ne prawi
        unsigned char mode;
        //if (str=="LSB") mode = S_LSB;
        //else mode = S_USB;
    	if 		(str=="LSB" ) mode = S_LSB;//LSB
    	else if (str=="USB" ) mode = S_USB;//USB
    	else if (str=="DIGU") mode = 0x08; //DIGU
    	else return;        
        
        cmd[5] = mode;
        cmd[6] = 0x01;
        cmd[7]=FD;
        emit EmitWriteCmd((char*)cmd,8);
    }
    else
    {
        // *RMM1<CR> (LSB)=1 USB=0 TT-565 TT-599
        // *M11<CR> sets USB for VFO-A, USB for VFO-B  TT-588,TT-538,TT-516
        char cmdnc[50];
        QString md = (char*)ncmd[s_ModelID][NATIVE_CAT_SET_MODE].nseq;

        if (s_ModelID==2 || s_ModelID==3 || s_ModelID==4 || s_ModelID==7)//2="TT-588,TT-538,TT-516 RX-350
        {
            if (str=="LSB") md.append("22");//2 LSB
            else md.append("11");//1 USB
        }
        else //TT-565 TT-599
        {
            //if (str=="LSB") md.append("1");//1 LSB
            //else md.append("0");//0 USB
            if 		(str=="LSB" ) md.append("1");//LSB
    		else if (str=="USB" ) md.append("0");//USB
    		else if (str=="DIGU") md.append("6");//DIGU
    		else return;
        }
        md.append("\r");

        for (int i = 0; i < md.count(); i++) cmdnc[i]=md.at(i).toLatin1();
        emit EmitWriteCmd(cmdnc,md.count()); // get native sequence
    }
}
void Tentec::get_freq()
{
    QString ide = (char*)ncmd[s_ModelID][NATIVE_CAT_GET_FREQ].nseq;
    if (ide=="None")
        return;

    //TT-563 as icom Delta II
    if (s_ModelID==5  || s_ModelID==11)
    {
        unsigned char cmd[50];
        for (int i = 0; i<5; i++)
            cmd[i] = (unsigned char)ncmd[s_ModelID][NATIVE_CAT_GET_FREQ].nseq[i];
        cmd[5]=FD;
        emit EmitWriteCmd((char*)cmd,6);
    }
    else
    {
        //?AF<CR>   answer -> @AFnnnnnnnn<CR> (8 characters) TT-565 TT-599
        //?A<CR>    answer -> A<d3 d2 d1 d0> <CR> or @Afnnnnnnnn <CR> TT-588,TT-538,TT-516
        char *cmdnc;
        cmdnc =  (char *)ncmd[s_ModelID][NATIVE_CAT_GET_FREQ].nseq;
        int len = strlen(cmdnc);
        emit EmitWriteCmd(cmdnc,len);
    }
}
void Tentec::get_mode()
{
    QString ide = (char*)ncmd[s_ModelID][NATIVE_CAT_GET_MODE].nseq;
    if (ide=="None")
        return;

    //TT-563 as icom Delta II
    if (s_ModelID==5  || s_ModelID==11)
    {
        unsigned char cmd[50];
        for (int i = 0; i<5; i++)
            cmd[i] = (unsigned char)ncmd[s_ModelID][NATIVE_CAT_GET_MODE].nseq[i];
        cmd[5]=FD;
        emit EmitWriteCmd((char*)cmd,6);
    }
    else
    {
        //?RMM<CR>   answer -> @RMM0<CR> (USB) TT-565 TT-599
        //?M <CR>    answer -> M VFOA VFOB <CR> M11<CR> VFOA-usb VFOB-usb TT-588,TT-538,TT-516
        char *cmdnc;
        cmdnc =  (char *)ncmd[s_ModelID][NATIVE_CAT_GET_MODE].nseq;
        int len = strlen(cmdnc);
        emit EmitWriteCmd(cmdnc,len);
    }
}

void Tentec::SetReadyRead(QByteArray ar,int size0)
{
    for (int i = 0; i < size0; i++)
    {
        if ((unsigned char)ar[i]==EOM)
        {
            //qDebug()<<"TENTEC READ ALL COMMAND="<<(QString(s_read_array.toHex()))<<s_read_array.size();
            //s_read_array.append(ar[i]); // no EOM added
            int size = s_read_array.size();
            //rejekt small array
            if (size < 3)//3 bez eom
            {
                //qDebug()<<"NO INAF SIZE";
                s_read_array.clear();
                return;
            }

            if (s_ModelID==2 || s_ModelID==3 || s_ModelID==4 || s_ModelID==7)//2="TT-588,TT-538,TT-516 RX-350
            {   //A=0x41 F=46   @=40  R=52  M=4d   Annnn = 11
                if (s_CmdID==GET_FREQ && size==5 && s_read_array[0]==(char)0x41)//my qestion for freq and freq filter vfoA
                {
                    unsigned char frq[6];
                    for (int x = 0; x < 4; x++)
                        frq[x] = s_read_array[x+1];
                    unsigned long long f = (frq[0] << 24)+(frq[1] << 16)+(frq[2] << 8)+frq[3];
                    emit EmitReadedInfo(GET_FREQ,QString("%1").arg(f));
                    s_CmdID = -1;//I Find my answer no need more
                }
                if (s_CmdID==GET_MODE && size==3 && s_read_array[0]==(char)0x4d )//my qestion for mod and freq filter vfoA
                {   //M VFOA VFOB <CR>  M11\r
                    QString smode = "WRONG_MODE";
                    if (s_read_array[1]==(char)0x31)//0x31=USB=0
                        smode = "USB";
                    emit EmitReadedInfo(GET_MODE,smode);
                    s_CmdID = -1;//I Find my answer no need more
                }
            }
            else if ((s_ModelID==0  || s_ModelID==1) && s_read_array[0]==(char)0x40)//@ TT-565 TT-599
            {   //A=0x41 F=46   @=40  R=52  M=4d   @AFnnnnnnnn = 11
                if (s_CmdID==GET_FREQ && size==11 && s_read_array[1]==(char)0x41 && s_read_array[2]==(char)0x46)//my qestion for freq and freq filter vfoA
                {
                    QByteArray tfreq;
                    tfreq.append(s_read_array.mid(3,8));
                    unsigned long long f = tfreq.toLongLong();
                    emit EmitReadedInfo(GET_FREQ,QString("%1").arg(f));
                    s_CmdID = -1;//I Find my answer no need more
                }
                if (s_CmdID==GET_MODE && size==5 && s_read_array[1]==(char)0x52 && s_read_array[2]==(char)0x4d
                        && s_read_array[3]==(char)0x4d)//my qestion for freq and freq filter vfoA
                {   //@RMM0
                    QString smode = "WRONG_MODE";
                    if (s_read_array[4]==(char)0x30)//0x30=USB=0
                        smode = "USB";
                    else if (s_read_array[4]==(char)0x36)//0x36=DATA-USB=6    
                		smode = "DIGU";    
                        
                    emit EmitReadedInfo(GET_MODE,smode);
                    s_CmdID = -1;//I Find my answer no need more
                }
            }
            else if (s_ModelID==5 || s_ModelID==11)//TT-563 as icom Delta II
            {
                //rejekt no icom in network
                if (s_read_array[0]!=(char)PR || s_read_array[1]!=(char)PR)
                {
                    //qDebug()<<"NO ICOM IN NETWORK";
                    s_read_array.clear();
                    return;
                }
                //rejekt no my correspondent in network or my full duplex signal
                if (s_read_array[2]!=(char)CTRLID || s_read_array[3]!=(char)s_model_addr)
                {
                    //qDebug()<<"NO MAY CORRESPONDENT OR FULL DUPLEX SIGNAL";
                    s_read_array.clear();
                    return;
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
                if (s_CmdID==GET_MODE && s_read_array[4]==(char)C_RD_MODE)//my qestion for mode and mode filter
                {
                    if (size==7)//bez fd=fi
                    {
                        unsigned char mod = (unsigned char)s_read_array[5];
                        QString smode = "WRONG_MODE";
                        if (mod==0x01)//USB
                            smode = "USB";
                        else if(mod==0x08)//DATA-USB=0x08
                       		smode = "DIGU";    
                            
                        emit EmitReadedInfo(GET_MODE,smode);
                        s_CmdID = -1;//I Find my answer no need more
                    }
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