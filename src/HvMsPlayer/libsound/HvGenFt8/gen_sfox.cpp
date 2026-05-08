/* The algorithms, source code, look-and-feel of WSJT-X and related
 * programs, and protocol specifications for the modes FSK441, FT8, JT4,
 * JT6M, JT9, JT65, JTMS, QRA64, ISCAT, MSK144, are Copyright © 2001-2017
 * by one or more of the following authors: Joseph Taylor, K1JT; Bill
 * Somerville, G4WJS; Steven Franke, K9AN; Nico Palermo, IV3NWV; Greg Beam,
 * KI7MT; Michael Black, W9MDB; Edson Pereira, PY2SDR; Philip Karn, KA9Q;
 * and other members of the WSJT Development Group.
 *
 * MSHV FT8 SF Generator
 * Rewritten into C++ and modified by Hrisimir Hristov, LZ2HV 2015-2024
 * May be used under the terms of the GNU General Public License (GPL)
 */

#include "gen_sfox.h"
#include "../../../nhash.h"
//#include <QtGui>


#define QPC_LOG2N 7             // log2(codeword length) (not punctured)
#define QPC_N (1<<QPC_LOG2N)    // codeword length (not punctured)
#define QPC_LOG2Q 7             // bits per symbol
#define QPC_Q (1<<QPC_LOG2Q)    // alphabet size
#define QPC_K 50               // number of information symbols

typedef struct
{
    int n;                      // codeword length (unpunctured)
    int np;                     // codeword length (punctured)
    int k;                      // number of information symbols
    int q;                      // alphabet size
    int xpos[QPC_N];            // info symbols mapping/demapping tables
    unsigned char f[QPC_N];     // frozen symbols values
    unsigned char fsize[QPC_N]; // frozen symbol flag (fsize==0 => frozen)
}
qpccode_ds;
//n //np //k //q
qpccode_ds qpccodetx = {
                           128, 127, 50, 128,
                           {
                               1,   2,   3,   4,   5,   6,   8,   9,  10,  12,  16,  32,  17,  18,  64,  20,
                               33,  34,  24,   7,  11,  36,  13,  19,  14,  65,  40,  21,  66,  22,  35,  68,
                               25,  48,  37,  26,  72,  15,  38,  28,  41,  67,  23,  80,  42,  69,  49,  96,
                               44,  27,  70,  50,  73,  39,  29,  52,  74,  30,  56,  81,  76,  43,  82,  84,
                               97,  45,  71,  88,  98,  46, 100,  51, 104,  53,  75, 112,  54,  57,  99, 119,
                               92,  77,  58, 117,  59,  83, 106,  31,  85, 108, 115, 116, 122, 125, 124,  91,
                               61,  90,  89, 111,  78,  93,  94, 126,  86, 107, 110, 118, 121,  62, 120,  87,
                               105,  55, 114,  60, 127,  63, 103, 101, 123,  95, 102,  47, 109,  79, 113,   0
                           },
                           {
                               0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
                               0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
                               0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
                               0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
                               0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
                               0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
                               0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
                               0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0
                           },
                           {
                               0,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
                               1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   0,   0,   0,
                               1,   1,   1,   1,   1,   1,   1,   0,   1,   1,   1,   0,   1,   0,   0,   0,
                               1,   1,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
                               1,   1,   1,   1,   1,   1,   0,   0,   1,   0,   0,   0,   0,   0,   0,   0,
                               1,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
                               1,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
                               0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0
                           }
                       };

unsigned char* _qpc_encode(unsigned char* y, unsigned char* x)
{
    // Non recursive polar encoder
    // Same architecture of a fast fourier transform
    // in which the fft butteflies are replaced by the polar transform
    // butterflies
    int k, j, m;
    int groups;
    int bfypergroup;
    int stepbfy;
    int stepgroup;
    int basegroup;
    int basebfy;
    memcpy(y, x, QPC_N);
    for (k = 0; k < QPC_LOG2N; k++)
    {
        groups = 1 << (QPC_LOG2N - 1 - k);
        stepbfy = bfypergroup = 1 << k;
        stepgroup = stepbfy << 1;
        basegroup = 0;
        for (j = 0; j < groups; j++)
        {
            basebfy = basegroup;
            for (m = 0; m < bfypergroup; m++)
            {
                // polar transform
                y[basebfy + stepbfy] = y[basebfy + stepbfy] ^ y[basebfy];
                basebfy = basebfy + 1;
            }
            basegroup = basegroup + stepgroup;
        }
    }
    return y;
}
void qpc_encode(unsigned char* y, const unsigned char* x)
{
    // map information symbols
    int kk, pos;
    for (kk = 0; kk < QPC_K; kk++)
    {
        pos = qpccodetx.xpos[kk];
        qpccodetx.f[pos] = x[kk];
    }
    // do polar encoding
    _qpc_encode(y, qpccodetx.f);
}

static bool inicialize_pulse_sfox_trx = false;
static double pulse_sfox_tx[13288];//1024*4*3=12288

GenSFox::GenSFox(bool f_dec_gen)//f_dec_gen = dec=true gen=false   bool f_dec_gen
{
	//gen_dec = f_dec_gen;
    TPackUnpackMsg77.initPackUnpack77(f_dec_gen);//f_dec_gen = dec=true gen=false
    //genPomFt.initGenPomFt();//no need for sfox  first_ft8_enc_174_91 = true;*/
    twopi=8.0*atan(1.0);
    //mycall = "";
    msg1 = "";
    nb_mycall = 0;
    nbits = 0;
    for (int i = 0; i < 10; ++i)
    {
        if (i<4) nmsg[i]=0;
        hiscall[i]="";
        if (i<5) rpt2[i]="";
    }

    if (inicialize_pulse_sfox_trx) return;
    /*int nsps=4*1920;//48000hz=7680
    //! Compute the frequency-smoothing pulse
    for (int i= 0; i < 3*nsps; ++i)//=23040
    {//do i=1,3*nsps
        double tt=(i-1.5*nsps)/(double)nsps;
        pulse_ft8_tx[i]=gfsk_pulse(2.0,tt);//tx=2.0
    }*/
    gen_pulse_gfsk_(pulse_sfox_tx,6144.0,8.0,4096);
    inicialize_pulse_sfox_trx = true;
}
GenSFox::~GenSFox()
{}
/*void GenFt8::save_hash_call_from_dec(QString c13,int n10,int n12,int n22)
{
    TPackUnpackMsg77.save_hash_call(c13,n10,n12,n22);
}
void GenFt8::save_hash_call_my_his_r1_r2(QString call,int pos)
{
    TPackUnpackMsg77.save_hash_call_my_his_r1_r2(call,pos);
}*/
/*
void GenFt8::save_hash_call_mam(QStringList ls)
{
    TPackUnpackMsg77.save_hash_call_mam(ls);
}
*/
/*QString GenFt8::unpack77(bool *c77,bool &unpk77_success)
{
    return TPackUnpackMsg77.unpack77(c77,unpk77_success);
}
void GenFt8::pack77(QString msgs,int &i3,int n3,bool *c77)// for apset v2
{
    TPackUnpackMsg77.pack77(msgs,i3,n3,c77);
}
void GenFt8::split77(QString &msg,int &nwords,QString *w)// for apset v2 //int *nw,
{
    TPackUnpackMsg77.split77(msg,nwords,w);//int *nw,
}*/
/*
short crc10_(unsigned char const * data, int length)
{
    return boost::augmented_crc<10, TRUNCATED_POLYNOMIAL10>(data, length);
}
short crc12_(unsigned char const * data, int length)
{
    return boost::augmented_crc<12, TRUNCATED_POLYNOMIAL12>(data, length);
}
bool crc12_check_(unsigned char const * data, int length)
{
    return !boost::augmented_crc<12, TRUNCATED_POLYNOMIAL12>(data, length);
}
short GenFt8::crc10(unsigned char const * data, int length)
{
    return crc10_(data,length);
}
short GenFt8::crc12(unsigned char const * data, int length)
{
    return crc12_(data,length);
}
*/
QString GenSFox::sfox_assemble(int ntype,int k,QString msg,QString mycall0/*,QString mygrid0*/)
{
    QString line = "";
    // In subsequent calls, assemble all necessary information for a SuperFox
    // transmission.
    //Message type: 0 Free Text
    //              1 CQ MyCall MyGrid
    //              2 Call_1 MyCall RR73
    //              3 Call_1 MyCall rpt
    //Number of messages of type ntype
    QString mycall = ""; 
    //QString mygrid = "";
    //QString msg2[5];
    //QString msg3[5];
    mycall=mycall0;//if (mycall0.at(0)!=' ') mycall=mycall0;
    //if (mygrid0.at(0)!=' ') mygrid=mygrid0;
    if (ntype>=1) nb_mycall=28;          // Allow for nonstandard MyCall ###
    int sum = nmsg[0]+nmsg[1]+nmsg[2]+nmsg[3]; //qDebug()<<"ssssss"<<nmsg[0]<<nmsg[1]<<nmsg[2]<<nmsg[3];
    if (sum==0)
    {
        for (int i = 0; i < 10; ++i)
        {
            hiscall[i]="";
            if (i<5) rpt2[i]="";
        }
    }
    if (k<=10)
    {
        if (ntype==0)
        {
            if (nbits+nb_mycall<=191)//!Enough room for a free text message?
            {
                nbits=nbits+142;
                //msg0=msg;
                nmsg[ntype]=nmsg[ntype]+1;
            }
        }
        else if (ntype==1)
        {
            if (nbits+nb_mycall<=318)//!Enough room for a CQ ?
            {
                nbits=nbits+15;
                msg1=msg;
                nmsg[ntype]=nmsg[ntype]+1;
            }
        }
        else if (ntype==2)
        {
            if (nbits+nb_mycall<=305)//!Enough room for a RR73 message?
            {
                nbits=nbits+28;
                int j=nmsg[ntype];
                //msg2[j]=msg;
                int i1=msg.indexOf(' ');
                hiscall[j+5]=msg.mid(0,i1);
                nmsg[ntype]=nmsg[ntype]+1;
            }
        }
        else if (ntype==3)
        {
            if (nbits+nb_mycall<=300)//!Enough room for a message with report?
            {
                nbits=nbits+33;
                int j=nmsg[ntype];
                //msg3[j]=msg;
                int i1=msg.indexOf(' ');
                hiscall[j]=msg.mid(0,i1);
                i1=msg.indexOf('-');
                if (i1<0) i1=msg.indexOf('+');
                rpt2[j]=msg.mid(i1,3);
                nmsg[ntype]=nmsg[ntype]+1;
            }
        } //qDebug()<<ntype<<msg<<nmsg[0]<<nmsg[1]<<nmsg[2]<<nmsg[3];
        return line;
    }
    if (k>=11)
    {
        //qDebug()<<"enddddd"<<nmsg[0]<<nmsg[1]<<nmsg[2]<<nmsg[3];
        // All pieces are now available. Put them into a command line for external
        //ntx=ntx+1//Transmission number
        nbits=nbits+nb_mycall;//Add bits for MyCall
        if (nmsg[1]>=1) line = msg1;
        else
        {
            line = mycall;
            for (int i = 0; i < nmsg[3]; ++i)
            {
                line = line +"  "+hiscall[i]+" "+rpt2[i];
            }
            for (int i = 0; i < nmsg[2]; ++i)
            {
                line = line +"  "+hiscall[i+5];
            }
        }
        nbits=0;
        nb_mycall=0;
        for (int i = 0; i < 10; ++i)
        {
            if (i<4) nmsg[i]=0;
            hiscall[i]="";
            if (i<5) rpt2[i]="";
        }
    } //qDebug()<<"2="<<line;
    return line;
}
QString GenSFox::foxgen2(int nslots,QStringList SlMsgs)
{
    int ntype = 0;
    int k = 0;
    QString sfmsg;
    QString mycall = "";
    //QString mygrid = "";
    QString line = "";
    QString msg = "";
    //Message type: 0 Free Text
    //              1 CQ MyCall MyGrid
    //              2 Call_1 MyCall RR73
    //              3 Call_1 MyCall rpt1
    //              4 Call_1 RR73; Call_2 <MyCall> rpt2
    if (nslots>5) nslots = 5; //qDebug()<<nslots<<SlMsgs;
    for (int i = 0; i < nslots; ++i)
    {
    	mycall = "";
    	//mygrid = "";
        msg = SlMsgs.at(i);
        QStringList l = msg.split(' ');
        if (msg.midRef(0,3) == "CQ ")
        {
            ntype = 1;
            mycall=l.at(1);
            //mygrid=l.at(2).mid(0,4);
        }
        else if (msg.indexOf(";") > -1)
        {
            ntype = 4;
            mycall=l.at(3).mid(1,l.at(3).count()-2);
        }
        else if (msg.indexOf(" RR73") >-1 && l.count()==3)
        {
            ntype = 2;
            mycall=l.at(1);
        }
        else if (l.count()==3 && l.at(2).count()==3)
        {
            QString s = l.at(2);
            if (s.at(0)=='-' || s.at(0)=='+')
            {
                ntype = 3;
                mycall=l.at(1);
            }
        }
        k++; //qDebug()<<ntype<<mycall<<mygrid<<msg;
        if (ntype<=3) line = sfox_assemble(ntype,k,msg,mycall/*,mygrid*/);
        if (ntype==4)
        {
            sfmsg = l.at(0)+" "+mycall+" RR73";
            line = sfox_assemble(2,k,sfmsg,mycall/*,mygrid*/);
            sfmsg = l.at(2)+" "+mycall+" "+l.at(4);
            k++;
            line = sfox_assemble(3,k,sfmsg,mycall/*,mygrid*/);
        }
    }
    line = sfox_assemble(ntype,11,msg,mycall/*,mygrid*/);//k=11 to finish up
    return line;
}
void GenSFox::SetArrayBits(int in,int in_bits,bool *ar,int &co)
{
    int izz = in_bits-1;
    for (int i = 0; i < in_bits; ++i)
    {
        ar[co]=(1 & (in >> -(i-izz)));
        co++;
    } //if (!gen_dec) qDebug()<<co;
}
int GenSFox::BinToInt32(bool*a,int b_a,int bits_sz)
{
    int k = 0;
    for (int i = b_a; i < bits_sz; ++i)
    {
        k <<= 1;
        k |= a[i];
    }
    return k;
}
//static const QString c_0=" 0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ/";
QString GenSFox::sfox_unpack(unsigned char *x,bool notp)
{
    QString msg0 = "";
    bool use_otp = notp;//int notp = 0;    
    int ncq = 0;
    bool msgbits[400];//350 unpack=350
    int i3 = 0;
    int n28 = 0;
    bool success;
    QString foxcall;
    QString freeTextMsg;
    QString freeTextMsg1;
    bool c77[80];
    QString grid4;
    const QString c=" 0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ/";
    const int NQU1RKS=203514677;
    QString crpt[5+2];
    QString msg[10+2];
    int j,iz;
    //if (!notp) use_otp = false;
    //else use_otp = true;
    
    for (int i= 0; i < 355; ++i) msgbits[i]=false;
    int co_t0 = 0; //write(msgbits,1000) x(0:46) 1000 format(47b7.7)
    for (int i = 0; i < 50; ++i)
    {
        int in = x[i]; //if (in<0) in=0;
        SetArrayBits(in,7,msgbits,co_t0);
    }

    i3 = BinToInt32(msgbits,326,329);//read(msgbits(327:329),'(b6)') i3            !Message type
    n28 = BinToInt32(msgbits,0,28);//read(msgbits(1:28),'(b28)') n28           !Standard Fox call
    TPackUnpackMsg77.unpack28(n28,foxcall,success);
    /*QString cha = foxcall+"            ";//+12 pouses
    int n10,n12,n22;
    TPackUnpackMsg77.save_hash_call(cha,n10,n12,n22);*/
	//qDebug()<<"foxcall"<<foxcall<<i3<<success<<n28;
    if (i3==1) //!Compound Fox callsign
    {
        //!read(msgbits(87:101),'(b15)') n15
        //!call unpackgrid(n15,grid4)
        //!msg(1)='CQ '//trim(foxcall)//' '//grid4
        //!write(*,1100) nutc,nsnr,dt0,nint(f0),trim(msg(1))
        //!go to 100
    }
    else if (i3==2) //!Up to 4 Hound calls and free text
    {
        freeTextMsg = "";
        freeTextMsg1 = "";
        for (int i = 0; i < 71; ++i) c77[i]=msgbits[i+160];
        TPackUnpackMsg77.unpacktext77(c77,freeTextMsg);//call unpacktext77(msgbits(161:231),freeTextMsg(1:13))
        for (int i = 0; i < 71; ++i) c77[i]=msgbits[i+231];
        TPackUnpackMsg77.unpacktext77(c77,freeTextMsg1);//call unpacktext77(msgbits(232:302),freeTextMsg(14:26))
        freeTextMsg.append(freeTextMsg1);
        for (int i = freeTextMsg.count()-1; i >= 0; --i)
        {//do i=26,1,-1
            if (freeTextMsg.at(i)!='.') break;
            freeTextMsg[i]=' ';
        }
        freeTextMsg=freeTextMsg.trimmed();
        if (!freeTextMsg.isEmpty()) msg0.append(freeTextMsg+"#"); //write(*,1100) nutc,nsnr,dt0,nint(f0),freeTextMsg 1100 format(i6.6,i4,f5.1,i5,1x,"~",2x,a)
    }
    else if (i3==3) //!CQ FoxCall Grid
    {
        long long int n58 = 0;//unsignedn//read(msgbits(1:58),'(b58)') n58 //FoxCall
        for (int i = 0; i < 58; ++i)
        {
            n58 <<= 1;
            n58 |= msgbits[i];//-0
        }
        QString c11 = "                "; //qDebug()<<"1"<<n58<<fmod(n58,38);
        for (int i = 10; i>=0; --i)
        {//do i=11,1,-1  //int j=fmod(n58,38);
            int x0=(n58 % 38);  // c++ mod = % to lost presision in 64-bit
            c11[i]=c[x0];
            n58=n58/38; //qDebug()<<"2"<<c11<<j;
        }
        foxcall = c11.trimmed(); //foxcall(12:13)='  '
        //2.76.2 add HV///////
		QString cha = foxcall+"            ";//+12 pouses
    	int n10,n12,n22;
    	TPackUnpackMsg77.save_hash_call(cha,n10,n12,n22);  
    	//2.76.2 end add HV//      
        int n15 = BinToInt32(msgbits,58,73);//read(msgbits(59:73),'(b15)') n15
        TPackUnpackMsg.unpackgrid(n15,grid4,false); //call unpackgrid(n15,grid4)
        grid4=grid4.trimmed();
        msg0.append("CQ "+foxcall+" "+grid4+"#"); //msg(1)='CQ '//trim(foxcall)//' '//grid4
        //write(*,1100) nutc,nsnr,dt0,nint(f0),trim(msg(1))
        int n32 = BinToInt32(msgbits,73,105);//read(msgbits(74:105),'(b32)') n32
        if (n32==NQU1RKS) goto c100;
        freeTextMsg = "";
        freeTextMsg1 = "";
        for (int i = 0; i < 71; ++i) c77[i]=msgbits[i+73];
        TPackUnpackMsg77.unpacktext77(c77,freeTextMsg);//call unpacktext77(msgbits(74:144),freeTextMsg(1:13))
        for (int i = 0; i < 71; ++i) c77[i]=msgbits[i+144];
        TPackUnpackMsg77.unpacktext77(c77,freeTextMsg1);//call unpacktext77(msgbits(145:215),freeTextMsg(14:26))
        freeTextMsg.append(freeTextMsg1);
        for (int i = freeTextMsg.count()-1; i >= 0; --i)
        {//do i=26,1,-1
            if (freeTextMsg.at(i)!='.') break;
            freeTextMsg[i]=' ';
        }
        freeTextMsg=freeTextMsg.trimmed();
        if (!freeTextMsg.isEmpty()) msg0.append(freeTextMsg+"#"); //if(len(trim(freeTextMsg)).gt.0) write(*,1100) nutc,nsnr,dt0,nint(f0),freeTextMsg
        goto c100;
    }
    j=280;//281
    iz=4;//4                                         !Max number of reports
    if (i3==2) j=140;//141
    for (int i = 0; i < iz; ++i)
    {//do i=1,iz                                    !Extract the reports
        int n = BinToInt32(msgbits,j,j+5);//read(msgbits(j:j+4),'(b5)') n
        if (n==31) crpt[i]="RR73";
        else
        {
            int irpt = n-18;
            if (irpt>-1) crpt[i]="+"+QString("%1").arg(irpt,2,10,QChar('0'));
            else crpt[i]="-"+QString("%1").arg((int)fabs(irpt),2,10,QChar('0'));
            //write(crpt(i),1006) n-18 1006    format(i3.2)
            //if (crpt[i].at(0)==' ') crpt[i][0]='+';//if(crpt(i)(1:1).eq.' ') crpt(i)(1:1)='+'
        }
        j+=5;
    }
    //! Unpack Hound callsigns and format user-level messages:
    iz=9; // !Max number of hound calls
    if (i3==2 || i3==3) iz=4;
    for (int i = 0; i < iz; ++i)
    {//do i=1,iz
        QString c13;
        j=(i+1)*28;//28*i + 1
        n28 = BinToInt32(msgbits,j,j+28);//read(msgbits(j:j+27),'(b28)') n28
        TPackUnpackMsg77.unpack28(n28,c13,success);//call unpack28(n28,c13,success)
    	/*QString cha = c13+"            ";//+12 pouses
    	int n10,n12,n22;
    	TPackUnpackMsg77.save_hash_call(cha,n10,n12,n22);*/        
        if (n28==0 || n28==NQU1RKS) continue;
        msg[i]=(c13+" "+foxcall);//msg(i)=trim(c13)//' '//trim(foxcall)
        if (msg[i].midRef(0,3)=="CQ ") ncq++;
        else
        {
            if (i3==2) msg[i]=msg[i]+" "+crpt[i];//then msg(i)=trim(msg(i))//' '//crpt(i)
            else
            {
                if (i<=4) msg[i]=msg[i]+" RR73";//if(i<=5) msg(i)=trim(msg(i))//' RR73'
                if (i>4 ) msg[i]=msg[i]+" "+crpt[i-5];//msg(i)=trim(msg(i))//' '//crpt(i-5)
            }
        }
        if (ncq<=1 || msg[i].midRef(0,3)!="CQ ")//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
        {
            msg0.append(msg[i]+"#");//write(*,1100) nutc,nsnr,dt0,nint(f0),trim(msg(i))
        }
    }
    if (msgbits[305]==true && ncq<1)
    {
    	if (foxcall.indexOf('<')>-1)//2.76.2 add hv
    	{
    		foxcall.remove('<');
    		foxcall.remove('>');
   		}   	
    	msg0.append("CQ "+foxcall+"#");//then write(*,1100) nutc,nsnr,dt0,nint(f0),'CQ '//foxcall endif    	
   	}
c100:
    if (use_otp)// && foxcall!="<...>"
    {
        int key = BinToInt32(msgbits,306,326);//read(msgbits(307:326),'(b20)') notp
        if (foxcall.indexOf('<')>-1)//2.76.2 add hv
    	{
    		foxcall.remove('<');
    		foxcall.remove('>');
   		} 
        msg0.append("$VERIFY$ "+foxcall+" "+QString("%1").arg(key,6,10,QChar('0'))+"#");//write(ssignature,'(I6.6)') notp
    } //qDebug()<<notp;
    if (msg0.endsWith('#')) msg0 = msg0.mid(0,msg0.count()-1);
    return msg0;
}
void GenSFox::sfox_pack(QString line,QString ckey,bool bMoreCQs,bool bSendMsg,QString freeTextMsg,unsigned char *xin)
{
    unsigned char xin1[50+10];
    //unsigned char *xin1 = new unsigned char[50+10];
    QString w[16+10];
    int nwords;
    bool msgbits[400];//max pack=329
    int i3=0;
    int nh1=0; //!Number of Hound calls with RR73
    int nh2=0;
    const QString c=" 0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ/";
    int i0;
    int n28;
    int n32;
    const int NQU1RKS=203514677;
    bool allz;

    //str_mam = "LZ0AA RR73; LZ0BB <LZ2HVV> +01#K7YY LZ2HVV +02#Z7XX LZ2HVV RR73";
    //str_mam = "CQ LZ2HVV KN23";
    //! Split the command line into words
    //line.append(" ");
    //for (int i = 0; i < 16; ++i) w[i]=" "; //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    int i1=0;//1;
    int j=0;
    int j2=0;
    int lcou = line.count();
    if (lcou>120) lcou = 120;
    line.append("      ");
    for (j= 0; j < 16; ++j)
    {//do j=1,16
        int i;
        if (i1>=lcou) break;
        int end = i1+12;
        if (end>lcou) end = lcou;
        for (i = i1; i < end; ++i)
        {//do i=i1,min(i1+12,120)
            if (line.at(i)==' ') break;//if(line(i:i).eq.' ') exit
        }
        int i2=i+1;//i-1;
        w[j]=line.mid(i1,i2-i1);//w(j)=line(i1:i2)
        for (i = i2; i < lcou; ++i)
        {//do i=i2+1,120
            if (line.at(i)!=' ') break;//if(line(i:i).ne.' ') exit
        }
        i1=i;
    }
    nwords=j; //for (int i = 0; i < 16; ++i) qDebug()<<nwords<<i<<w[i];

    for (int i= 0; i < 350; ++i) msgbits[i]=false;
    //now=itime8()/30
    //now=30*now
    int notp = ckey.toInt();//read(ckey(5:10),*) notp
    //qDebug()<<ckey<<notp;
    int co_t0 = 306;
    SetArrayBits(notp,20,msgbits,co_t0);//write(msgbits(307:326),'(b20.20)') notp  !Insert the digital signature

    QString c11;
    if (w[0].midRef(0,3)=="CQ ")// if(w(1)(1:3).eq.'CQ ') then
    {
        i3=3;
        //QString c11=w[1].trimmed();//w(2)(1:11)  c11 = call_1.mid(0,11).rightJustified(11,' ');
        //c11 = c11.rightJustified(11,' ');
        c11=w[1]+"            "; // <- ???
        long long int n58=0;//unsigned
        for (int i = 0; i<11; ++i)
        {//do i=1,11
            int nn = c.indexOf(c11[i]);//index(c,c11(i:i)) - 1
            if (nn<0) nn = 0;  //HV Clean up any illegal chars
            n58=n58*38 + nn;
        } //qDebug()<<c11;
        co_t0 = 0;
        int izz = 58-1;
        for (int i = 0; i < 58; ++i)//write(msgbits(1:58),'(b58.58)') n58
        {
            msgbits[co_t0]=(1 & (n58 >> -(i-izz)));
            co_t0++;
        }
        bool text=true;
        int n15 =0;
        QString  grid0 = w[2]+"     ";
        grid0 = grid0.mid(0,4);
        TPackUnpackMsg.packgrid(grid0,n15,text,false); /*call packgrid(w(3)(1:4),n15,text)*/
        SetArrayBits(n15,15,msgbits,co_t0);//write(msgbits(59:73),'(b15.15)') n15
        co_t0 = 326;
        SetArrayBits(i3,3,msgbits,co_t0);//write(msgbits(327:329),'(b3.3)') i3     //!Message type i3=3
        goto c800;
    }

    c11 = w[0]+"             "; 
    TPackUnpackMsg77.pack28(c11,n28);                      //!Fox call
    co_t0 = 0; //qDebug()<<c11<<"Fox call"<<n28;
    SetArrayBits(n28,28,msgbits,co_t0);//write(msgbits(1:28),'(b28.28)') n28

    //! Default report is RR73 if we're also sending a free text message.
    if (bSendMsg)
    {
        for (int i= 0; i < 20; ++i) msgbits[i+140]=true;//msgbits(141:160)='11111111111111111111'
    }
    //qDebug()<<fmin(8,7); =7
    j=28;//j=29; //! Process callsigns with RR73
    for (int i= 1; i < nwords; ++i)
    {//do i=2,nwords
        if (w[i].at(0)=='+' || w[i].at(0)=='-') continue;//if(w(i)(1:1).eq.'+' .or. w(i)(1:1).eq.'-') cycle     !Skip report words
        i1=i+1;//fmin(i+1,nwords-1);//i1=min(i+1,nwords)
        if (i1>nwords-1) i1=nwords-1;
        if (w[i1].at(0)=='+' || w[i1].at(0)=='-') continue;//if(w[i1)(1:1) .eq.'+' .or. w(i1)(1:1).eq.'-') cycle  !Skip if i+1 is report
        c11 = w[i]+"             ";
        TPackUnpackMsg77.pack28(c11,n28);//call pack28(w(i),n28)
        co_t0 = j;
        SetArrayBits(n28,28,msgbits,co_t0);//write(msgbits(j:j+27),1002) n28 1002 format(b28.28) !Insert this call for RR73 message
        j+=28;//j=j+28
        nh1++;//nh1=nh1+1   //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
        if (nh1>=5) break;//if(nh1.ge.5) exit //!At most 5 RR73 callsigns
    }

    //! Process callsigns with a report
    j=168;//169;
    j2=280;//281;
    if (bSendMsg)
    {
        i3=2;
        j=28 + 28*nh1;//29 + 28*nh1;
        j2=140 + 5*nh1;//141 + 5*nh1;
    }
    for (int i= 1; i < nwords; ++i)
    {//do do i=2,nwords
        i1=i+1;//fmin(i+1,nwords-1);//min(i+1,nwords)
        if (i1>nwords-1) i1=nwords-1;
        if (w[i1].at(0)=='+' || w[i1].at(0)=='-')//if(w(i1)(1:1).eq.'+' .or. w(i1)(1:1).eq.'-') then
        {
            c11 = w[i]+"             ";
            TPackUnpackMsg77.pack28(c11,n28);//call pack28(w(i),n28)
            co_t0 = j;
            SetArrayBits(n28,28,msgbits,co_t0);//write(msgbits(j:j+27),1002) n28       !Insert this call
            QString rpt0 = w[i1].trimmed();
            int n = rpt0.midRef(0,rpt0.count()).toInt();//read(w(i1),*) n              !Valid reports are -18 to +12, plus RR73
            if (n<-18) n=-18;//if(n.lt.-18) n=-18           !... Even numbers only ...
            if (n>12) n=12;
            co_t0 = j2;
            SetArrayBits((n+18),5,msgbits,co_t0);//write(msgbits(j2:j2+4),1000) n+18 1000    format(b5.5)
            w[i1]="";//w(i1)=""
            nh2++;//nh2=nh2+1 //!        print*,'C',i3,i,j,n,w(i)
            if ( nh2>=4 || (nh1+nh2)>=9 ) break;//if( nh2.ge.4 .or. (nh1+nh2).ge.9 ) exit  ! At most 4 callsigns w/reports
            j+=28;//j=j+28
            j2+=5;//j2=j2+5
        }
    }
c800:
    /*QString sss;
    for (int i= 0; i < 329; ++i) sss.append(QString("%1").arg((int)msgbits[i]));
    qDebug()<<sss;*/
    if (bSendMsg)
    {
        //qDebug()<<"1"<<freeTextMsg;
        i1=25;//26
        for (int i= 0; i < freeTextMsg.count(); ++i)
        {//dodo i=1,26
            if (freeTextMsg.at(i)!=' ') i1=i;
        }
        for (int i= i1+1; i < freeTextMsg.count(); ++i) freeTextMsg[i]='.';//freeTextMsg(i:i)='.'
        //do i=i1+1,26
        //qDebug()<<"1"<<i1<<freeTextMsg;
        if (i3==3)
        {
            int i4,n4;
            bool c77[80];
            QString tstr = freeTextMsg.mid(0,13);
            TPackUnpackMsg77.packtext77(tstr,i4,n4,c77);
            for (int i= 0; i < 71; ++i) msgbits[i+73]=c77[i];//call packtext77(freeTextMsg(1:13),msgbits(74:144))
            tstr = freeTextMsg.mid(13,26);
            TPackUnpackMsg77.packtext77(tstr,i4,n4,c77);
            for (int i= 0; i < 71; ++i) msgbits[i+144]=c77[i];//call packtext77(freeTextMsg(14:26),msgbits(145:215))
        }
        else if (i3==2)
        {
            int i4,n4;
            bool c77[80];
            QString tstr = freeTextMsg.mid(0,13);
            TPackUnpackMsg77.packtext77(tstr,i4,n4,c77);
            for (int i= 0; i < 71; ++i) msgbits[i+160]=c77[i];//call packtext77(freeTextMsg(1:13),msgbits(161:231))
            tstr = freeTextMsg.mid(13,26);
            TPackUnpackMsg77.packtext77(tstr,i4,n4,c77);
            for (int i= 0; i < 71; ++i) msgbits[i+231]=c77[i];//call packtext77(freeTextMsg(14:26),msgbits(232:302))
        }
        co_t0 = 326;
        SetArrayBits(i3,3,msgbits,co_t0); //write(msgbits(327:329),'(b3.3)') i3     //!Message type i3=2
    }
    if (bMoreCQs) msgbits[305]=true;//msgbits(306:306)='1'

    i3 = BinToInt32(msgbits,326,329); //qDebug()<<i3; //??? i3 is ready no need read // read(msgbits(327:329),'(b3)') i3

    if (i3==0)
    {
        for (int i= 0; i < 9; ++i)
        {//do i=1,9
            i0=(i+1)*28;//i0=i*28 + 1
            n28 = BinToInt32(msgbits,i0,i0+28);//read(msgbits(i0:i0+27),'(b28)') n28
            if (n28==0) //if(n28.eq.0) write(msgbits(i0:i0+27),'(b28.28)') NQU1RKS
            {
                co_t0 = i0;
                SetArrayBits(NQU1RKS,28,msgbits,co_t0);
            }
        }
    }
    else if (i3==3)
    {
        allz=true;
        for (int i= 0; i < 7; ++i)
        {//do i=0,6
            i0=i*32 + 73;//i0=i*32 + 74
            n32 = BinToInt32(msgbits,i0,i0+32);//read(msgbits(i0:i0+31),'(b32)') n32
            if (n32!=0) allz=false;
        }
        if (allz)
        {
            for (int i= 0; i < 7; ++i)
            {//do i=0,6
                i0=i*32 + 73;//i0=i*32 + 74
                co_t0 = i0;//write(msgbits(i0:i0+31),'(b32.32)') NQU1RKS
                SetArrayBits(NQU1RKS,32,msgbits,co_t0);
            }
        }
    }

    int x = 0;
    for (int i = 0; i < 47; ++i)//read(msgbits,1004) xin(0:46)  1004 format(47b7)
    {
        xin1[i] = BinToInt32(msgbits,x,x+7);
        x += 7;
    } //qDebug()<<x;

    uint32_t mask21=(pow(2,21) - 1);//2**21 - 1
    uint64_t n47=47;
    uint32_t ncrc21=(nhash2(xin1,n47,571) & mask21);//ncrc21=iand(nhash2(xin,n47,571),mask21)!Compute 21-bit CRC
    xin1[47]=(ncrc21/16384);//ncrc21/16384;                       !First 7 of 21 bits
    xin1[48]=((ncrc21/128) & 127);//iand(ncrc21/128,127)          !Next 7 bits
    xin1[49]=(ncrc21 & 127);//iand(ncrc21,127)              !Last 7 bits

    /*QString sss;
    for (int i= 0; i < 50; ++i) sss.append(QString("%1").arg(xin1[i])+",");
    qDebug()<<sss;*/

    /*xin=xin(49:0:-1)                           !Reverse the symbol order
    ! NB: CRC is now in first three symbols, fox call in the last four.*/
    s_unpack_msg = sfox_unpack(xin1,false);
    x = 0;
    for (int i= 49; i >= 0; --i)
    {
        xin[x] = xin1[i];
        x++;
    }
}
void GenSFox::u8shift1(unsigned char *a,int cou_a,int ish)
{
    unsigned char *t = new unsigned char[cou_a+100];
    for (int i=0; i< cou_a; i++) t[i]=a[i];
    if (ish>0)
    {
        for (int i = 0; i <  cou_a; i++)
        {
            if (i+ish<cou_a)
                a[i]=t[i+ish];
            else
                a[i]=t[i+ish-cou_a];
        }
    }
    if (ish<0)
    {
        for (int i = 0; i <  cou_a; i++)
        {
            if (i+ish<0)
                a[i]=t[i+ish+cou_a];
            else
                a[i]=t[i+ish];
        }
    }
    delete [] t;
}
int GenSFox::gensfox(QString str_mam,int *t_iwave,double samp_rate,double f_tx,QString s0,QString sotp)//,int i3bit
{
    int k1 = 0;
    QStringList SlMsgs;
    QStringList l0;
    QString line = "";
    int nslots = 1;
    QString ckey = "000000";
    unsigned char xin[50+10];
    unsigned char y[128+10];//unsigned char *y = new unsigned char[128+2];
    int chansym0[127+10];//integer chansym0(127)
    const int nsym = 151;
    const int NS = 24;
    const int isync[nsym] =
        {
            1,2,4,7,11,16,22,29,37,39,42,43,45,48,52,57,63,70,78,80,83,84,86,89
        };
    int i4tone[180];

    //str_mam = "K5TT RR73; LZ0BB <LZ2HVV> +01#K4TT RR73; LZ1BB <LZ2HVV> +01#K6TT RR73; LZ7BB <LZ2HVV> +01#K3TT RR73; LZ6BB <LZ2HVV> +01#K2TT RR73; LZ2BB <LZ2HVV> +01";
    //str_mam = "KF5TTF RR73; LZ0FBB <LZ2HVV> +01#KF4TFT RR73; LZ1BBF <LZ2HVV> +01#KF6TTF RR73; LZ7BBF <LZ2HVV> +01#KF3TTF RR73; LZ6BBF <LZ2HVV> +01#KF7YYF LZ2HVV RR73";
    //str_mam = "K5TT RR73; LZ0BB <LZ2HVV> +01#LZ1BB LZ2HVV RR73#K6TT RR73; LZ7BB <LZ2HVV> +01#K3TT RR73; LZ6BB <LZ2HVV> +01#K7YY LZ2HVV +02";
    //str_mam = "KE4RR RR73; LZ0BB <LZ2HVV> +01#K7YY LZ2HVV +02#Z7XX LZ2HVV RR73#Z8XX LZ2HVV RR73";
    //str_mam = "CQ US LZ2HVV KN23";
    //str_mam = "CQ LZ2HVV KN23#5135273576#Z7XX LZ2HVV RR73";
    //str_mam = "";
    //str_mam ="JQ1CIV YJ0VV RR73#OP0P YJ0VV RR73#F4JMS YJ0VV -14#IK0XKP YJ0VV -09#ES1JA YJ0VV -06#OZ1DPP YJ0VV +10";
    //str_mam ="JQ1CIV YJ0VV RR73#OP0P YJ0VV RR73#F4JMS YJ0VV -14#IK0XKP YJ0VV -09#ES1JA YJ0VV -06";
    l0 = s0.split("#");
    if (str_mam.contains("#")) SlMsgs = str_mam.split("#");
    else SlMsgs << str_mam;
    nslots = SlMsgs.count();

    //qDebug()<<"TX="<<SlMsgs<<"OPT="<<l0<<l0.count();
    QString freeTextMsg=l0.at(0);//"123456789 123456789 123456789";//26 char
    bool bSendMsg=false;//bSendMsg=cmsg(nslots)(39:39).eq.'1'
    if (l0.at(1)=="1") bSendMsg=true;
    bool bMoreCQs=false;//bMoreCQs=cmsg(1)(40:40).eq.'1'
    if (l0.at(2)=="1") bMoreCQs=true;
    if (bSendMsg) //QString cmsg = "123456789 123456789 123456789";
    {
        freeTextMsg = freeTextMsg.leftJustified(26,' '); //freeTextMsg=cmsg;//cmsg(nslots)(1:26)
        if (nslots>4) nslots=4; //if(nslots.gt.4) nslots=4//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    }
    line = foxgen2(nslots,SlMsgs); //qDebug()<<"SM="<<SlMsgs; qDebug()<<"line->"<<line;

    l0.clear();
    l0 = sotp.split("#");
    if (l0.at(0)=="1") ckey = genPomFt.foxOTPcode(l0.at(1)); //qDebug()<< ckey;
 
    TPackUnpackMsg77.reset_save_hash_calls_gen();//2.76.2
    // Pack message information and CRC into xin(0:49)
    sfox_pack(line,ckey,bMoreCQs,bSendMsg,freeTextMsg,xin);

    /*QString sss;
    for (int i= 0; i < 50; ++i) sss.append(QString("%1").arg(xin[i])+",");
    qDebug()<<sss;*/

    qpc_encode(y,xin);                    //!Encode the message to 128 symbols

    u8shift1(y,128,1); //y=cshift(y,1)    //!Puncture the code by removing y(0)

    y[127]=0;//y(127)=0
    for (int i = 0; i < 127; ++i) chansym0[i]=y[i];//chansym0=y(0:126)

    int j0 = 0;
    int k = 0;
    for (int i = 0; i < nsym; ++i)
    {
        if (j0<NS && i==isync[j0]-1)
        {
            i4tone[i]=0;// !Insert sync symbol at tone 0
            j0++;//!Index for next sync symbol
        }
        else
        {
            i4tone[i]=chansym0[k]+1;//!Symbol value 0 is transmitted at tone 1, etc.
            k++;
        }
    }

    int nsps=4*1024;
    int nwave=(nsym+2)*nsps;
    double hmod=1.0;
    double *d_iwave = new double[646688];//15s= 626688
    double *dphi = new double[646688];//626688
    double dt=1.0/samp_rate;
    double dphi_peak=twopi*hmod/(double)nsps;
    for (int i= 0; i < 636690; ++i) dphi[i]=0.0;//max tx=626688
    for (int j= 0; j < nsym; ++j)//nsym=151   nsps=4096
    {
        int ib=j*nsps;
        for (int i= 0; i < 3*nsps; ++i) dphi[i+ib] += dphi_peak*pulse_sfox_tx[i]*(double)i4tone[j];
    }
    int bgn =nsym*nsps;
    for (int i= 0; i < 2*nsps; ++i)//8192
    {
        dphi[i]+=dphi_peak*i4tone[0]*pulse_sfox_tx[i+nsps];
        dphi[i+bgn]+=dphi_peak*i4tone[nsym-1]*pulse_sfox_tx[i];
    }
    double ofs = twopi*f_tx*dt;//twopi*f_tx*dt;
    double phi=0.0;
    k1=0;
    for (int j= 0; j < nwave; ++j)//nsps=4096 nwave=626688
    {
        d_iwave[k1]=0.0;//linux problem
        d_iwave[k1]=sin(phi);
        phi=fmod(phi+dphi[j]+ofs,twopi);
        k1++;
    }

    int nramp=(int)((double)nsps/8.0);
    for (int i = 0; i < nramp; ++i) d_iwave[i]*=(1.0-cos(twopi*(double)i/(2.0*nramp)))/2.0;
    int k2=nwave-nramp+1;//int k2=nsym*nsps-nramp+1; //k1=nsym*nsps-nramp+1   k2=(nsym+1)*nsps+1; int k2=k1-nramp;
    for (int i = 0; i < nramp; ++i) d_iwave[i+k2]*=(1.0+cos(twopi*(double)i/(2.0*nramp)))/2.0;//i+k1-nsps

    for (int z = 0; z < k1; ++z) t_iwave[z]=(int)(8380000.0*(d_iwave[z]));//2.70 8380000.0 full=8388607

    delete [] d_iwave;
    delete [] dphi;

    int to_end = (15*samp_rate) - k1;
    for (int z = 0; z < to_end ; ++z)//FT8_SF~=13s
    {
        t_iwave[k1] = 0;
        k1++;
    }
    return k1;
}
