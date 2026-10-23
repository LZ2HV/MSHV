/* MSHV Part from RigControl
 * Copyright 2015 Hrisimir Hristov, LZ2HV
 * May be used under the terms of the GNU General Public License (GPL)
 */
#include "icmarine_def.h"
#include "icmarine.h"
//#include <QtGui>

//#define PR		    0xfe
//#define C_CTL_PTT	0x1c		/* Control Transmit On/Off, Sc */
//#define FI		    0xfd		/* End of message code */
//#define CTRLID		0xe0		/* Controllers's default address */

//#define S_PTT		0x00
//#define S_ANT_TUN	0x01	/* Auto tuner 0=OFF, 1 = ON, 2=Start Tuning */
#include <stdio.h>
#define CONTROLLER_ID 90
#define BUFSZ 96
#define EOM "\x0d\x0a"
#define CMD_PTT "TRX"	/* PTT */
//#define M_ID "$PICOA"
//#define CMD_PTT "TRX"	/* PTT */
#define CMD_TXFREQ	"TXF"	/* Transmit frequency */
#define CMD_RXFREQ	"RXF"	/* Receive frequency */
#define CMD_MODE	"MODE"
#define MD_USB	"USB"
#define MD_LSB	"LSB"

#define OFFSET_CMD 13
#define EOMI 0x0a //???

static const unsigned char icmarine_addr_list[ICMARINE_COUNT] =
    {
        /*IC-M802*/0x08,
        /*IC-M710*/0x01,
        /*IC-M700PRO*/0x02,
    };

Icmarine::Icmarine(int ModelID,QWidget *parent)
        : QWidget(parent)
{
    s_ModelID = ModelID;
    ////////////////////////////////////////////////////////new read com
    //s_rig_name = rigs_kenwood[s_ModelID].name;
    s_CmdID = -1;
    s_read_array.clear();
    ////////////////////////////////////////////////////////end new read com
}

Icmarine::~Icmarine()
{
    //qDebug()<<"Delete"<<rigs_icom[s_ModelID].name;
}

void Icmarine::SetCmd(CmdID i,ptt_t ptt,QString str)
{
    switch (i)
    {
    case GET_SETT:
        emit EmitRigSet(rigs_icmarine[s_ModelID]);
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

void Icmarine::make_cmd_all(const char *cmd, const char *param, char */*response*/)
{
    char cmd_all[BUFSZ+1];
    int cmd_len = 0;
    int i;
    unsigned csum = 0;

    cmd_all[BUFSZ]='\0';
    cmd_len = (int)snprintf(cmd_all, BUFSZ, "$PICOA,%02u,%02u,%s",
                       CONTROLLER_ID,
                       icmarine_addr_list[s_ModelID],
                       cmd);
    if (param)
        cmd_len += snprintf(cmd_all+cmd_len, BUFSZ-cmd_len, ",%s", param);
    //cmd_len += snprintf(cmd_all+cmd_len, BUFSZ-cmd_len, "%02u", 0x01);
    // check sum HV
    for (i=1; i<cmd_len; i++)
        csum = csum ^ (unsigned)cmd_all[i];
    cmd_len += snprintf(cmd_all+cmd_len, BUFSZ-cmd_len, "*%02X" EOM, csum);

    emit EmitWriteCmd((char *)cmd_all,cmd_len);

}
void Icmarine::set_ptt(ptt_t ptt)
{
    const char *cmd = "";

    if (ptt==RIG_PTT_ON)
        cmd = "TX";
    else
        cmd = "RX";

    make_cmd_all(CMD_PTT,cmd,NULL);
}
/*void Icom::get_ptt()
{
	emit EmitWriteCmd("Icom GET_PTT");
}*/
////////////////////////////////////////////////////////new read com
//#define PRIll "lld"
void Icmarine::set_freq(unsigned long long freq)
{
    char freqbuf[BUFSZ];
    float ff = (float)freq/1000000.0;//vinagi mi e w Hz
    sprintf(freqbuf, "%.6f", ff);
    //double g = ((double)freq/1000000.0);
    //qDebug()<<"f="<<freqbuf<<QString("%1").arg(g,0,'f',6);
    make_cmd_all(CMD_TXFREQ,freqbuf,NULL);
    make_cmd_all(CMD_RXFREQ,freqbuf,NULL);
}
/*
#if 0
#define MD_LSB	"J3E"
#define MD_USB	"J3E"
#define MD_CW 	"A1A"
#define MD_AM	"A3E"
#else
#define MD_LSB	"LSB"
#define MD_USB	"USB"
#define MD_CW 	"CW"
#define MD_AM	"AM"
#endif
#define MD_FSK	"J2B"
*/
void Icmarine::set_mode(QString str)
{
    const char *pmode;
    if (str=="LSB")
        pmode = MD_LSB; //"LSB";
    else
        pmode = MD_USB; //"USB";
    make_cmd_all(CMD_MODE,pmode,NULL);
}
void Icmarine::get_freq()
{
    char freqbuf[BUFSZ] = "";
    make_cmd_all(CMD_RXFREQ,NULL,freqbuf);
}
void Icmarine::get_mode()
{
    char freqbuf[BUFSZ] = "";
    make_cmd_all(CMD_MODE,NULL,freqbuf);
}
/* CR LF */
//#define EOM "\x0d\x0a"
//#define LF "\x0a"
void Icmarine::SetReadyRead(QByteArray ar,int size0)
{
    for (int i = 0; i < size0; i++)
    {
        if (ar[i]==(char)EOMI) //EOMI=0x0a #define OFFSET_CMD 13   // Minimal length OFFSET_CMD+5
        {
            char respbuf[BUFSZ+20];
            char *p;
            char response[BUFSZ+20];

            s_read_array.append(ar[i]);//add EOMI=0x0a
            //qDebug()<<"ICMARINE READ ALL COMMAND="<<(QString(s_read_array.toHex()))<<s_read_array.size();
            int size = s_read_array.size();

            for (int j = 0; j<size; j++)//2.12
                respbuf[j]=(char)s_read_array.at(j);

            //rejekt small array
            if (size < OFFSET_CMD+5)//eom bez 0x0a // Minimal length OFFSET_CMD+5
            {
                //qDebug()<<"NO INAF SIZE";
                s_read_array.clear();
                return;
            }
            respbuf[size] = 0;
            /* check response */
            if (memcmp(respbuf, "$PICOA,", strlen("$PICOA,")))
            {
                s_read_array.clear();
                return;
            }
            /* strip *checksum and CR/LF from string */
            respbuf[size-5] = 0;
            p = strchr(respbuf+OFFSET_CMD, ',');
            if (p)
                strncpy(response, p+1, BUFSZ);
            else
            {
                s_read_array.clear();
                return;
            }

            if (s_CmdID==GET_FREQ)
            {
                double d;
                unsigned long long freq;
                if (response[0] == '\0')
                {
                    freq = 0;
                }
                else
                {
                    if (sscanf(response, "%lf", &d) != 1)
                    {
                        s_read_array.clear();
                        return;
                    }
                    freq = (unsigned long long)(d*1000000.0);
                }
                emit EmitReadedInfo(GET_FREQ,QString("%1").arg(freq));
                s_CmdID = -1;//I Find my answer no need more
            }
            if (s_CmdID==GET_MODE)
            {
            	QString smode = "WRONG_MODE";
                //if (!memcmp(modebuf, MD_LSB, strlen(MD_LSB)))
                //*mode = RIG_MODE_LSB;
                if (!memcmp(response, MD_USB, strlen(MD_USB)))
                    smode = "USB";

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