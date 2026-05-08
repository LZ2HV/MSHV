/* The algorithms, source code, look-and-feel of WSJT-X and related
 * programs, and protocol specifications for the modes FSK441, FT8, JT4,
 * JT6M, JT9, JT65, JTMS, QRA64, ISCAT, MSK144, are Copyright © 2001-2017
 * by one or more of the following authors: Joseph Taylor, K1JT; Bill
 * Somerville, G4WJS; Steven Franke, K9AN; Nico Palermo, IV3NWV; Greg Beam,
 * KI7MT; Michael Black, W9MDB; Edson Pereira, PY2SDR; Philip Karn, KA9Q;
 * and other members of the WSJT Development Group.
 *
 * MSHV FT8 DecoderVar
 * Rewritten into C++ and modified by Hrisimir Hristov, LZ2HV 2015-2026
 * May be used under the terms of the GNU General Public License (GPL)
 */
#include "decoderms.h"
#include "../HvMsPlayer/libsound/genpom.h"
#include "ft_all_ap_def.h"
//#include <QtGui>

//static double dd8m[190000];
//static double dd8[190000];

static const int icos7_2[7] =
    {
        3,1,4,0,6,5,2
    };
#define MAXINC 35
typedef struct
{
    double xdt;
    QString msg;
}
incall_struct;
static incall_struct incall[MAXINC+10];
#define MAXStatOE 150 //130
//#define MAXStatOEt 130
static int s_nmsg=0;
/*typedef struct
{
    double freq;
    double dt;
    bool lstate;
    QString msg;
}
xcopy_struct;*/
static xcopy_struct evencopy[MAXStatOE+10];
static xcopy_struct oddcopy[MAXStatOE+10];
static xcopy_struct even[MAXStatOE+10];
static xcopy_struct odd[MAXStatOE+10];
static double avexdt;
static int c_xdtt=0;
static double xdtt[MAXStatOE+80];
static bool once_static_init = true;
void DecoderFt8::ft8var_init()
{
    //first_agccft8 = true;
    //lagccbail=false;

    first_apsetvar = true;
    hiscallprev="#";
    mycallprev="#";
    lhoundprev = false;
    //first_ft8b_2var = true;

    lapmyc=true;//??
    lhound=false;//??
    lskiptx1=false;//??
    ltxing=false;//??

    if (once_static_init)//syatic///
    {
        once_static_init=false;
        s_nmsg=0;
        c_xdtt=0;
        avexdt=0.0;
        for (int i = 0; i < MAXINC; ++i) incall[i].msg="  ";
        for (int i = 0; i < MAXStatOE+10; ++i) xdtt[i]=0.0;
        for (int i = 0; i < MAXStatOE; ++i)
        {
            evencopy[i].lstate=false;
            oddcopy[i].lstate =false;
            evencopy[i].msg="";
            oddcopy[i].msg ="";
            even[i].lstate=false;
            odd[i].lstate =false;
            even[i].msg="";
            odd[i].msg ="";
        } //qDebug()<<"once_static_init";
    }

    mycalllen1=0;
    nlasttx=0;
    npos=0;
    ncount=0;
    lrepliedother=false;
    lsubtracted_=false;
    lastrxmsg[0].lstate=false;
    lastrxmsg[0].lastmsg="";
    lqsomsgdcd=false;
    lasthcall="";
    lft8sdec=false;
    ncqsignal=0;
    nmycsignal=0;
    msgroot="";
    // FT8AP decoding bandwidth for 'mycall hiscall ???' and RRR,RR73,73 messages
    //napwid8=50.0;
    for (int i = 0; i < 56; ++i) msg_[i]="";
    for (int i = 0; i < ncalldt; ++i)
    {
        calldteven[i].call2="";
        calldtodd[i].call2 ="";
    }

    for (int i = 0; i < numcqsig; ++i)
    {
        evencq[i].freq=6000.0;
        oddcq[i].freq=6000.0;
        tmpcqsig[i].freq=6000.0;
    }
    for (int i = 0; i < nummycsig; ++i)
    {
        evenmyc[i].freq=6000.0;
        oddmyc[i].freq=6000.0;
        tmpmycsig[i].freq=6000.0;
    }
    tmpqsosig[0].freq=6000.0;
    evenqso[0].freq=6000.0;
    oddqso[0].freq=6000.0;
    for (int i = 0; i < 76; ++i)
    {
        msgsd76[i]="";
        for (int j = 0; j < 79; ++j)
        {
            itone76[i][j]=0;
            if (j<58) idtone76[i][j]=0;
        }
    }
    nmsgloc=0;
    cwfilter_init();
}
#define TAPDS 55//55
#define FS8D 5//5
void DecoderFt8::cwfilter_init()
{
    const QString msgcq25[25]=
        {"","CQ 2E0DLA IO92","CQ BH3NEB ON81","CQ CG3CGT FN04","CQ DX CT1JA IM59","CQ CU20E",
         "CQ NA CX1OB GF14","CQ DF2AJ JN49","CQ DG4XPZ JN58","CQ EA8XR IL19","CQ F1YE IN94",
         "CQ DX G1KLN IO82","CQ HL2KVF PM38","CQ IU1ZSV JN45","CQ JG1TWO PM96","CQ K2ST EM96",
         "CQ N9TUX EL98","CQ SA NO2FA FM18","CQ OH6GKE KP13","CQ PD0ORM JO24","CQ DX PT7DS HI06",
         "CQ RA3XEP KO84","CQ SM2GSH KP05","CQ UA9OHX NO15","CQ JA W0YH EN12"
        };

    //bool c77[100];
    int itone[120];
    double complex *csig0 = new double complex[153681];
    double facx=1.0/300.0;

    for (int i = 0; i < 201; ++i) windowx[i]=(1.0+cos((double)i*pi/200.0))/2.0;
    for (int i = 0; i < 201; ++i) windowx[i]=facx*windowx[i];
    for (int i = 201; i < 211; ++i) windowx[i]=0.0;
    for (int i = 0; i < TAPDS; ++i) windowc1[i]=0.4*(1.0+cos((double)i*pi/((double)TAPDS-1.0)));
    //for (int i = 0; i < TAPDS; ++i) windowc1[i]*=0.1;
    for (int i = TAPDS; i < TAPDS+10; ++i) windowc1[i]=0.0;

    for (int z = 0; z<29; ++z) idtone25[0][z]=0;
    for (int i = 1; i<25; ++i)
    {//do i=2,25
        TGenFt8->pack77_make_c77_i4tone(msgcq25[i],itone);
        if (i==1)
        {
            gen_ft8cwaveRx(itone,0.0,csig0);//gen_ft8wave(itone,79,1920,2.0,12000.0,0.0,csig0,xjunk,1,151680)
            int m=0;//1
            for (int j = 0; j<15; ++j)//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
            {//do j=0,14
                for (int k = 0; k<32; ++k)
                {//do k=1,32
                    if (j<7) csync[j][k]=csig0[m];
                    else csynccq[j-7][k]=csig0[m];
                    m+=60;
                }
            }
        }
        for (int z = 0; z<29; ++z)
        {
            idtone25[i][z]=itone[z+7];//(8:36)   (i,1:29)
            idtone25[i][z+29]=itone[z+43];//(44:72)  (i,30:58)
        }
    }
    double dt2=0.005; //! fs2=200 Hz
    int k=0;//1
    for (int ifr = -FS8D; ifr<=FS8D; ++ifr)
    {//do ifr=-5,5                             // !Search over +/- 2.5 Hz
        double delf=(double)ifr*0.5;
        double dphi=twopi*delf*dt2;
        double phi=0.0;
        for (int i = 0; i<32; ++i)
        {//do i=1,32
            ctwkw[k][i]=cos(phi)+sin(phi)*I;
            phi=fmod(phi+dphi,twopi);
        }
        k++;
    }
    k=0;//1
    for (int ifr = -FS8D; ifr<=FS8D; ++ifr)
    {//do ifr=-5,5                             // !Search over +/- 1.25 Hz
        double delf=(double)ifr*0.25;
        double dphi=twopi*delf*dt2;
        double phi=0.0;
        for (int i = 0; i<32; ++i)
        {//do i=1,32
            ctwkn[k][i]=cos(phi)+sin(phi)*I;
            phi=fmod(phi+dphi,twopi);
        }
        k++;
    }
    //! lcqsignal
    double delf=3.125;
    double dphi=twopi*delf*dt2;
    double phi=0.0;
    for (int i = 0; i<256; ++i)
    {//do i=1,256
        ctwk256[i]=cos(phi)+sin(phi)*I;
        phi=fmod(phi+dphi,twopi);
    }
    delete [] csig0;
}
#define	ERSET_IN 4
static int nintcount = ERSET_IN;
void DecoderFt8::SetModeChangedStaticAll()
{
    avexdt=0.0;
    nintcount = ERSET_IN; //qDebug()<<"SetModeChangedStaticAll";
}
bool DecoderFt8::s_use_var_dec = false;
static int s_nft8cycles = 2;
static int s_nft8sens = 2;
void DecoderFt8::SetVarDecodeFtPar(bool f,int dcyc,int dsens)
{
    s_use_var_dec=f;
    s_nft8cycles=dcyc;
    s_nft8sens=dsens; //qDebug()<<f<<dcyc<<dsens;
}
static double s_napwid8 = 50.0;
void DecoderFt8::SetFreqGlobal(QString s)//2.76.5
{
	long long int mfrq = s.toLongLong();
	if (mfrq < 30000000) s_napwid8=10.0;//5.0;
    else if (mfrq < 100000000) s_napwid8=30.0;//15.0;
    else s_napwid8=50.0; //printf(" s_napwid8 = %0.1f\n",s_napwid8);
}
static bool f_statis_all_new_p = true;
void DecoderFt8::SetEndPStaticAll()
{
    f_statis_all_new_p = true;
    int nFT8decd = c_xdtt;
    double dtmed=0.0;
    double sumxdt=0.0;
    if (nintcount>0) nintcount--;
    if (nFT8decd==0) avexdt=0.0;
    else
    {
        if (nFT8decd>2)
        {
            sumxdt=0.0;
            for (int i = 0; i < nFT8decd; ++i)
            {//do i=1,nFT8decd
                if (i<nFT8decd-2)
                {
                    if      ((xdtt[i]>xdtt[i+1] && xdtt[i]<xdtt[i+2])   || (xdtt[i]<xdtt[i+1] && xdtt[i]>xdtt[i+2]))   dtmed=xdtt[i];
                    else if ((xdtt[i+1]>xdtt[i] && xdtt[i+1]<xdtt[i+2]) || (xdtt[i+1]<xdtt[i] && xdtt[i+1]>xdtt[i+2])) dtmed=xdtt[i+1];
                    else if ((xdtt[i+2]>xdtt[i] && xdtt[i+2]<xdtt[i+1]) || (xdtt[i+2]<xdtt[i] && xdtt[i+2]>xdtt[i+1])) dtmed=xdtt[i+2];
                    else dtmed=xdtt[i];
                    sumxdt+=dtmed;
                }
                else sumxdt+=dtmed;// qDebug()<<"TWO"<<i;}//! use last median value
            }
            if (nFT8decd>5) avexdt=(avexdt+sumxdt/(double)nFT8decd)/2.0;
            else if (nFT8decd==5) avexdt=(1.1*avexdt+0.9*sumxdt/(double)nFT8decd)/2.0;
            else if (nFT8decd==4) avexdt=(1.25*avexdt+0.75*sumxdt/(double)nFT8decd)/2.0;
            else if (nFT8decd==3) avexdt=(1.35*avexdt+0.65*sumxdt/(double)nFT8decd)/2.0;
        }
        else if (nFT8decd>0)
        {
            sumxdt=0.0;
            for (int i = 0; i < nFT8decd; ++i) sumxdt+=xdtt[i];
            if (nFT8decd==2) avexdt=(1.5*avexdt+0.5*sumxdt/(double)nFT8decd)/2.0;
            else if (nFT8decd==1) avexdt=(1.75*avexdt+0.25*sumxdt)/2.0;
        }
    }
    if (nFT8decd>10 && (nintcount>=1 && nintcount<=2))//62+avexdt*25.0  || avexdt>1.1
    {   //qDebug()<<"IN reset="<<nintcount<<sumxdt<<avexdt;
        avexdt=sumxdt/(double)nFT8decd; //qDebug()<<"OUT reset="<<avexdt;
        nintcount = -10;//stop
    }   //qDebug()<<"END<---- decid="<<decid<<"Count="<<nFT8decd<<avexdt<<nintcount;
}
void DecoderFt8::tone8(bool lmycallstd,bool lhiscallstd)//double complex csynce[19][32];
{
    const QString rpt[56]=
        {"-01","-02","-03","-04","-05","-06","-07","-08","-09","-10",
         "-11","-12","-13","-14","-15","-16","-17","-18","-19","-20",
         "-21","-22","-23","-24","-25","-26","R-01","R-02","R-03","R-04",
         "R-05","R-06","R-07","R-08","R-09","R-10","R-11","R-12","R-13","R-14",
         "R-15","R-16","R-17","R-18","R-19","R-20","R-21","R-22","R-23","R-24",
         "R-25","R-26","AA00","RRR","RR73","73"
        };
    //bool c77[100];
    int itone[120];
    int itone1[120];
    double complex *csig0 = new double complex[153681];//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    //bool lhound=false;//??
    //int i3,n3;
    QString msg37;
    QString mycall14="";
    QString hiscall14="";
    QString mycall  = s_MyCall8;
    QString mybcall = s_MyBaseCall8;
    QString hiscall = s_HisCall8;
    QString hisbcall= s_HisBaseCall8;
    for (int i = 0; i < 95; ++i)
    {
        //c77[i]=false;
        itone1[i]=0;
    }
    if (lhound && mybcall.count()>2 && hisbcall.count()>2)
    {
        msg37=mybcall+" "+hisbcall+" RR73";
        TGenFt8->pack77_make_c77_i4tone(msg37,itone);
        for (int z = 0; z < 29; ++z)
        {
            idtonefox73[z]=itone[z+7];
            idtonefox73[z+29]=itone[z+43];
        }
        msg37=mybcall+" RR73; "+mybcall+" <"+hiscall+"> -12";//msg37=trim(mybcall)//' RR73; '//trim(mybcall)//' <'//trim(hiscall)//'> -12'
        TGenFt8->pack77_make_c77_i4tone(msg37,itone);
        for (int z = 0; z < 29; ++z)
        {
            idtonespec[z]=itone[z+7];
            idtonespec[z+29]=itone[z+43];
        }
    }
    if (!lhiscallstd && hiscall.count()>2)
    {
        msg37="CQ "+hiscall;
        TGenFt8->pack77_make_c77_i4tone(msg37,itone);
        for (int z = 0; z < 29; ++z)
        {
            idtonecqdxcns[z]=itone[z+7];//idtonecqdxcns(1:29)=itone(8:36)
            idtonecqdxcns[z+29]=itone[z+43];//idtonecqdxcns(30:58)=itone(44:72)
        }
        msg37=="<AA1AAA> "+hiscall+" 73";//msg37='<AA1AAA> '//trim(hiscall)//' 73'
        TGenFt8->pack77_make_c77_i4tone(msg37,itone);
        for (int z = 0; z < 29; ++z)
        {
            idtonedxcns73[z]=itone[z+7];
            idtonedxcns73[z+29]=itone[z+43];
        }
    }
    if (!lhiscallstd && !lmycallstd) return; //! we do not support such message in FT8v2
    if (!lhiscallstd || !lmycallstd)
    {
        mycall14="<"+mycall+">";//mycall14='<'//trim(mycall)//'>'
        hiscall14="<"+hiscall+">";// hiscall14='<'//trim(hiscall)//'>'*/
    }
    if (lhiscallstd && lmycallstd)
    {
        for (int i = 0; i<56; ++i)
        {//do i=1,56
            msg37=mycall+" "+hiscall+" "+rpt[i];//msg37=''; msg37=trim(mycall)//' '//trim(hiscall)//' '//trim(rpt(i))
            msg_[i]=msg37;
            TGenFt8->pack77_make_c77_i4tone(msg37,itone);
            if (i==0) for (int z = 0; z<85; ++z) itone1[z]=itone[z];
            for (int z = 0; z < 29; ++z)
            {
                idtone56[i][z]=itone[z+7];//idtone56(53,1:29)=itone(8:36)
                idtone56[i][z+29]=itone[z+43];//idtone56(53,30:58)=itone(44:72)
            }
            for (int z = 0; z < 79; ++z) itone56[i][z]=itone[z]; //qDebug()<<"GEN=="<<i<<msg37;
        }
        goto c2;
    }
    if (!lhiscallstd && lmycallstd)
    {
        for (int i = 0; i<52; ++i)
        {//do i=1,52
            msg37=mycall+" "+hiscall14+" "+rpt[i];//msg37=''; msg37=trim(mycall)//' '//trim(hiscall14)//' '//trim(rpt(i))
            TGenFt8->pack77_make_c77_i4tone(msg37,itone);
            if (i==0) for (int z = 0; z<85; ++z) itone1[z]=itone[z];
            for (int z = 0; z < 29; ++z)
            {
                idtone56[i][z]=itone[z+7];//idtone56(53,1:29)=itone(8:36)
                idtone56[i][z+29]=itone[z+43];//idtone56(53,30:58)=itone(44:72)
            }
            for (int z = 0; z < 79; ++z) itone56[i][z]=itone[z];
            msg37=mycall+" "+hiscall+" "+rpt[i];//trim(mycall)//' '//trim(hiscall)//' '//trim(rpt(i))
            msg_[i]=msg37;
        }
        for (int i = 53; i<56; ++i)
        {//do i=54,56
            msg37=mycall14+" "+hiscall+" "+rpt[i];//msg37=trim(mycall14)//' '//trim(hiscall)//' '//trim(rpt(i))
            TGenFt8->pack77_make_c77_i4tone(msg37,itone);
            for (int z = 0; z < 29; ++z)
            {
                idtone56[i][z]=itone[z+7];//idtone56(53,1:29)=itone(8:36)
                idtone56[i][z+29]=itone[z+43];//idtone56(53,30:58)=itone(44:72)
            }
            for (int z = 0; z < 79; ++z) itone56[i][z]=itone[z];
            msg37=mycall+" "+hiscall+" "+rpt[i];//msg37=trim(mycall)//' '//trim(hiscall)//' '//trim(rpt(i))
            msg_[i]=msg37;
        }
        msg37=mycall14+" "+hiscall;//msg37=trim(mycall14)//' '//trim(hiscall)
        msg_[52]=msg37;
        TGenFt8->pack77_make_c77_i4tone(msg37,itone);
        for (int z = 0; z < 29; ++z)
        {
            idtone56[52][z]=itone[z+7];//idtone56(53,1:29)=itone(8:36)
            idtone56[52][z+29]=itone[z+43];//idtone56(53,30:58)=itone(44:72)
        }
        for (int z = 0; z < 79; ++z) itone56[52][z]=itone[z];
        goto c2;
    }
    if (lhiscallstd && !lmycallstd)
    {
        for (int i = 0; i<52; ++i)
        {//do i=1,52
            msg37=mycall14+" "+hiscall+" "+rpt[i];//msg37=''; msg37=trim(mycall14)//' '//trim(hiscall)//' '//trim(rpt(i))
            TGenFt8->pack77_make_c77_i4tone(msg37,itone);
            if (i==0) for (int z = 0; z<85; ++z) itone1[z]=itone[z];
            for (int z = 0; z < 29; ++z)
            {
                idtone56[i][z]=itone[z+7];//idtone56(53,1:29)=itone(8:36)
                idtone56[i][z+29]=itone[z+43];//idtone56(53,30:58)=itone(44:72)
            }
            for (int z = 0; z < 79; ++z) itone56[i][z]=itone[z];
            msg37=mycall+" "+hiscall+" "+rpt[i];// msg37=trim(mycall)//' '//trim(hiscall)//' '//trim(rpt(i))
            msg_[i]=msg37;
        }
        for (int i = 53; i<56; ++i)
        {//do i=54,56
            msg37=mycall+" "+hiscall14+" "+rpt[i];//msg37=trim(mycall)//' '//trim(hiscall14)//' '//trim(rpt(i))
            TGenFt8->pack77_make_c77_i4tone(msg37,itone);
            for (int z = 0; z < 29; ++z)
            {
                idtone56[i][z]=itone[z+7];//idtone56(53,1:29)=itone(8:36)
                idtone56[i][z+29]=itone[z+43];//idtone56(53,30:58)=itone(44:72)
            }
            for (int z = 0; z < 79; ++z) itone56[i][z]=itone[z];
            msg37=mycall+" "+hiscall+" "+rpt[i];//msg37=trim(mycall)//' '//trim(hiscall)//' '//trim(rpt(i))
            msg_[i]=msg37;
        }
        msg37=mycall14+" "+hiscall;//msg37=''; msg37=trim(mycall14)//' '//trim(hiscall)
        msg_[52]=msg37;
        TGenFt8->pack77_make_c77_i4tone(msg37,itone);
        for (int z = 0; z < 29; ++z)
        {
            idtone56[52][z]=itone[z+7];//idtone56(53,1:29)=itone(8:36)
            idtone56[52][z+29]=itone[z+43];//idtone56(53,30:58)=itone(44:72)
        }
        for (int z = 0; z < 79; ++z) itone56[52][z]=itone[z];
        goto c2;
    }
c2:
    int m=13441-1; //! 7*1920+1
    gen_ft8cwaveRx(itone1,0.0,csig0);//call gen_ft8wavevar(itone1,79,1920,2.0,12000.0,0.0,csig0,xjunk,1,151680)
    for (int j = 0; j < 19; ++j)
    {//do j=0,18
        for (int k = 0; k < 32; ++k)
        {//do k=1,32;
            csynce[j][k]=csig0[m];//csynce(j,k)=csig0(m);
            m+=60;
        }
    } //qDebug()<<m;
    delete [] csig0; //qDebug()<<"tone8 My="<<mycall<<mybcall<<lmycallstd<<"His="<<hiscall<<hisbcall<<lhiscallstd;
}
void DecoderFt8::MakeApFlip(int *ap,bool *c77,int count)
{
    for (int i = 0; i < count; ++i)
    {
        int ic77 = c77[i];
        ap[i]=2*ic77-1;
    }
}
void DecoderFt8::ft8apsetvar(bool lmycallstd,bool lhiscallstd)
{
    QString mycall  = s_MyCall8;
    mycall=mycall.trimmed();
    QString hiscall = s_HisCall8;
    hiscall=hiscall.trimmed(); //qDebug()<<decid<<"1 ft8apsetvar";
    if (hiscall!=hiscallprev || mycall!=mycallprev || lhound!=lhoundprev || first_apsetvar) //then ! first for lhound triggered
    {
        first_apsetvar = false; //qDebug()<<decid<<"2 ft8apsetvar";
        mycallprev=mycall;
        lhoundprev=lhound;
        QString mybcall = s_MyBaseCall8;
        mybcall=mybcall.trimmed();
        QString hisbcall= s_HisBaseCall8;
        hisbcall=hisbcall.trimmed();
        QString hisgrid4 = s_HisGrid8;
        if (hisgrid4.count()>3) hisgrid4=hisgrid4.mid(0,4);
        hisgrid4=hisgrid4.trimmed();
        int i3=0;
        int n3=0;
        bool c77[100];
        QString msg;

        for (int i = 0; i < 78; ++i) c77[i]=0;
        bool lnohiscall=false; //hiscall=hiscall12;
        QString hiscallt=hiscall;
        if (hiscallt.isEmpty())//if(len(trim(hiscall)).eq.0) then
        {
            hiscallt="LZ2ABC";//"K9ABC";
            lnohiscall=true;
        } //qDebug()<<"May="<<mycall<<mybcall<<lmycallstd<<"His="<<hiscall<<hisbcall<<lhiscallstd;
        for (int i = 0; i < 77; ++i)
        {
            int an = 0;
            if (i==0 || i==29) an=99;
            apsymmynsrrr_[i]=an;
            apsymmyns73_[i]=an;
            apsymmynsrr73_[i]=an;
            apsymdxnsrrr_[i]=an;
            apsymdxns732_[i]=an;
            apsymdxnsr73_[i]=an;
            if (i<58)
            {
                apsym_[i]=an;
                apsymdxns1_[i]=an;
                apsymmyns2_[i]=an;
            }
            if (i<66) apsymsp_[i]=an;
            if (i<29) apsymmyns1_[i]=an;
        }
        if (hiscall!=hiscallprev)
        {
            hiscallprev=hiscall;//! Encode a dummy standard message: i3=1, 28 1 28 1 1 15
            for (int i = 0; i < 77; ++i)
            {
                int an = 0;
                if (i==0 || i==29) an=99;
                apcqsym_[i]=an;
                apsymdxns73_[i]=an;
                apsymdxnsrr73_[i]=an;
                if (i<58) apsymdxstd_[i]=an;
            } //qDebug()<<"ResetAp="<<hiscall<<hiscallt;
            if (hiscall.count()>2)
            {
                if (lhiscallstd && hisgrid4.count()==4) msg="CQ "+hiscall+" "+hisgrid4;
                else msg="CQ "+hiscall;
                TGenFt8->pack77(msg,i3,n3,c77);//call pack77var(msg,i3,n3,c77,0)call unpack77var(c77,1,msgchk,unpk77_success,25)
                /*if ((lhiscallstd && i3!=1) || (!lhiscallstd && i3!=4))
                {
                    for (int i = 0; i < 77; ++i) apcqsym_[i]=0;
                    apcqsym_[0]=99;
                    apcqsym_[29]=99;
                }*/  //qDebug()<<msg<<lhiscallstd<<i3;
                if ((lhiscallstd && i3==1) || (!lhiscallstd && i3==4)) MakeApFlip(apcqsym_,c77,77);
            }
            if (lhiscallstd && !lmycallstd)
            {
                msg=hiscall+" "+hiscall+" RRR";//??? msg=trim(hiscall)//' '//trim(hiscall)//' RRR'
                TGenFt8->pack77(msg,i3,n3,c77);
                if (i3==1) MakeApFlip(apsymdxstd_,c77,58);
            }
            if (!lhound && !lhiscallstd && hiscall.count()>2)
            {
                msg="<W9XYZ> "+hiscall+" RR73";//! nonstandard DXCall searching
                TGenFt8->pack77(msg,i3,n3,c77);
                if (i3==4 ) MakeApFlip(apsymdxnsrr73_,c77,77);
                msg="<W9XYZ> "+hiscall+" 73";
                TGenFt8->pack77(msg,i3,n3,c77);
                if (i3==4) MakeApFlip(apsymdxns73_,c77,77);
            }
        }
        if (!lhound && !lmycallstd && mycall.count()>2)
        {
            if (lhiscallstd)
            {
                msg="<"+mycall+"> "+hiscall+" -15";//msg='<'//trim(mycall)//'> '//trim(hiscall)//' -15'
                TGenFt8->pack77(msg,i3,n3,c77);//call pack77var(msg,i3,n3,c77,0)
                if (i3==1) MakeApFlip(apsymmyns2_,c77,58);
                msg=mycall+" <"+hiscall+"> RR73";//msg=trim(mycall)//' <'//trim(hiscall)//'> RR73'
                TGenFt8->pack77(msg,i3,n3,c77);//call pack77var(msg,i3,n3,c77,0)
                if (i3==4) MakeApFlip(apsymmynsrr73_,c77,77);
                msg=mycall+" <"+hiscall+"> 73";// msg=trim(mycall)//' <'//trim(hiscall)//'> 73'
                TGenFt8->pack77(msg,i3,n3,c77);//call pack77var(msg,i3,n3,c77,0)
                if (i3==4) MakeApFlip(apsymmyns73_,c77,77);
                msg=mycall+" <"+hiscall+"> RRR";//msg=trim(mycall)//' <'//trim(hiscall)//'> RRR'
                TGenFt8->pack77(msg,i3,n3,c77);//call pack77var(msg,i3,n3,c77,0)
                if (i3==4) MakeApFlip(apsymmynsrrr_,c77,77);
            }
            else if (lnohiscall)
            {
                msg="<"+mycall+"> ZZ1ZZZ -15";// msg='<'//trim(mycall)//'> ZZ1ZZZ -15'
                TGenFt8->pack77(msg,i3,n3,c77);//call pack77var(msg,i3,n3,c77,0)
                if (i3==1) MakeApFlip(apsymmyns1_,c77,29);
            }
        }

        if (mycall.count()<3) return;

        if (!lhound /*&& lmycallstd*/ && (lhiscallstd || lnohiscall))
        {
            msg="";//! Encode a dummy standard message: i3=1, 28 1 28 1 1 15
            if (lmycallstd) msg.append(mycall);
            else msg.append("<"+mycall+">");
            msg.append(" "+hiscallt+" RRR");
            TGenFt8->pack77(msg,i3,n3,c77); //qDebug()<<i3<<msg<<apsym_[0];
            if (i3==1) MakeApFlip(apsym_,c77,58);
            if (lnohiscall)
            {
                for (int i = 0; i < 58; ++i) apsym_[i]=0;
                apsym_[0]=99;//HV important trick if stop
                apsym_[29]=99;
            } //qDebug()<<i3<<msg<<apsym_[0];
        }
        if (lhound)
        {   //! standard messages from Fox, always base callsigns //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
            if (hisbcall.count()>2 && mybcall.count()>2)
            {
                msg=mybcall+" "+hisbcall+" -15";//msg=trim(mybcall)//' '//trim(hisbcall)//' -15'
                TGenFt8->pack77(msg,i3,n3,c77);//call pack77var(msg,i3,n3,c77,0)
                if (i3==1) MakeApFlip(apsym_,c77,58);
            }
            msg=mycall+" RR73; "+mycall+" <"+hiscallt+"> -16";//msg=trim(mycall)//' RR73; '//trim(mycall)//' <'//trim(hiscallt)//'> -16'
            TGenFt8->pack77(msg,i3,n3,c77);//call pack77var(msg,i3,n3,c77,0)
            if (i3==0) MakeApFlip(apsymsp_,c77,66);
            //if (lnohiscall) apsymsp_[29]=99;
        }
        if (!lhound && lmycallstd && !lhiscallstd && hiscall.count()>2)
        {
            msg=mycall+" <"+hiscall+"> -16";//msg=trim(mycall)//' <'//trim(hiscall)//'> -16'// ! report, rreport
            TGenFt8->pack77(msg,i3,n3,c77);//call pack77var(msg,i3,n3,c77,0)
            if (i3==1) MakeApFlip(apsymdxns1_,c77,58);
            msg="<"+mycall+"> "+hiscall+" RRR";// msg='<'//trim(mycall)//'> '//trim(hiscall)//' RRR'
            TGenFt8->pack77(msg,i3,n3,c77);//call pack77var(msg,i3,n3,c77,0)
            if (i3==4) MakeApFlip(apsymdxnsrrr_,c77,77);
            msg="<"+mycall+"> "+hiscall+" RR73";//msg='<'//trim(mycall)//'> '//trim(hiscall)//' RR73'
            TGenFt8->pack77(msg,i3,n3,c77);//call pack77var(msg,i3,n3,c77,0)
            if (i3==4) MakeApFlip(apsymdxnsr73_,c77,77);
            msg="<"+mycall+"> "+hiscall+" 73";//msg='<'//trim(mycall)//'> '//trim(hiscall)//' 73'
            TGenFt8->pack77(msg,i3,n3,c77);//call pack77var(msg,i3,n3,c77,0)
            if (i3==4) MakeApFlip(apsymdxns732_,c77,77);
        }
    }
}
void DecoderFt8::tone8myc()
{
    //bool c77[100];
    int itone[120];
    for (int i = 0; i < 95; ++i)
    {
        //c77[i]=false;
        itone[i]=0;
    }
    TGenFt8->pack77_make_c77_i4tone(s_MyCall8+" AA1AAA FN25",itone);
    for (int i = 0; i < 29; ++i)
    {
        idtonemyc[i]=itone[i+7]; //idtonemyc(1:29)=itone(8:36);
        idtonemyc[i+29]=itone[i+43];//idtonemyc(30:58)=itone(44:72);
    }
}
//#define F0_DB 55.75   //2.76.5 ws300rc1
//#define F1_DB 5.75    //2.76.5 ws300rc1
#define UPDOWN 5.75  // +/- in Hz
#define F0_DB (50.0+UPDOWN)
#define F1_DB (UPDOWN)
//#define F0_DB 8.578*6.25 //2.76.5 =8.528*6.25=53.3  8.569*6.25  8.578*6.25  8.58*6.25
//#define F1_DB 0.92*6.25  //2.76.5 =0.920*6.25=5.75
//#define F0_DB 8.49*6.25  //2.70
//#define F1_DB 0.91*6.25  //2.70
//#define K_SUB 1.9842 //2.68
//#define K_SUB 1.9840 //2.66
#define K_SUB 1.9962 //2.70
//#define K_SUB 1.9998
void DecoderFt8::ft8_downsamplevar(bool &newdat1,double f0,int nqso,double complex *c0,double complex *c2,
                                   double complex *c3,bool lhighsens,int cd_off)
{
    const int NFFT1=192000;// NFFT1/2=96000
    const int NMAX=180000;
    const int NFFT2=3200;
    const int c_c1=NFFT2;//c1(0:3199)
    double complex *c1 = new double complex[3199+512];//double complex c1[3199+128];//
    double facc1=0.01/sqrt(61440.0); //! 1.0/sqrt(192000.*3200.) NFFT1*NFFT2
    //double facc1=1.0/sqrt((double)NFFT1*(double)NFFT2);
    //qDebug()<<1.0/sqrt(192000.0*3200.0)<<facc1<<1.0/sqrt((double)NMAX);
    const double facc2 = 0.001;//HV=ok  1.0/300.0;//0.0006;

    bool ldofft=false;
    if (lsubtracted_)//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    {
        for (int i = 0; i < npos; ++i)
        {//do i=1,npos
            if (fabs(f0-freqsub[i])<50.0)
            {
                ldofft=true;
                lsubtracted_=false;// qDebug()<<ldofft;
                break;
            }
        }
    }
    if (newdat1 || ldofft)
    {
        double *x = new double[NFFT1+150];//double x[NFFT1+150];// ! Data in dd have changed, recompute the long FFT
        for (int i = 0; i < NFFT1; ++i)
        {
            if (i < NMAX) x[i]=dd8[i]*facc2;//*0.01;
            else x[i]=0.0;
        }
        /*double max=-5.0; double min=5.0;
        for (int i = 0; i < NFFT1; ++i)// red2[i]=red2[i]/base2;
        {
        if (max<x[i]) max=x[i];
        if (min>x[i]) min=x[i];
        } if (max>0.9) qDebug()<<"   MAX="<<max<<min;*/
        f2a.four2a_d2c(cxx_ft8,x,NFFT1,-1,0,decid);
        newdat1=false; //ldofft=false;
        npos=0;
        delete [] x;
    }
    double df=0.0625; //! 12000.0/NFFT1
    //!  tonewidth=6.25 Hz ! 12000.0/1920.
    int i0=int(f0/df);//i0=nint(f0/df)
    //double ft=f0+55.75; //it=min(nint(ft/df),96000)
    //double fb=f0-5.75;  //ib=max(1,nint(fb/df))
    //double ft=f0+(F0_DB+0.24);//+0.2 //#define F0_DB 8.49*6.25//=53,06 2.70
    //double fb=f0-(F1_DB+0.07);//+0.03//#define F1_DB 0.91*6.25//=5,68 2.70
    double ft=f0+(F0_DB);
    double fb=f0-(F1_DB); //printf("Diff=%1.2f\n",ft-fb);
    int it=fmin(int(ft/df),NFFT1/2);
    int ib=fmax(0,int(fb/df));
    pomAll.zero_double_comp_beg_end(c1,0,3250);//c1=cmplx(0.0,0.0)
    int k=0;
    for (int i = ib; i < it; ++i)
    {
        c1[k]=cxx_ft8[i];//c1(0:k)=cxx(ib:it)
        k++;
        if (k > (c_c1-1)) break;
    }
    int to_tap = TAPDS;
    int ctrp = to_tap - 1; //qDebug()<<k; k=900
    for (int i = 0; i < to_tap; ++i)
    {
        c1[i]*=windowc1[ctrp];//c1(0:54)=c1(0:54)*windowc1(54:0:-1)
        ctrp--;
    } //qDebug()<<ctrp;
    for (int i = 0; i < to_tap; ++i) c1[i+k-to_tap]*=windowc1[i];//c1(k-54:k)=c1(k-54:k)*windowc1
    pomAll.cshift1(c1,c_c1,i0-ib);//c1=cshift(c1,i0-ib)
    //const double ck=+0.50;
    if (lhighsens)
    {
        c1[0]=c1[0]*1.93;
        c1[799]=c1[799]*1.7;
        c1[800]=c1[800]*1.7;
        c1[3199]=c1[3199]*1.93;
        /*c1[0]=c1[0]*(1.93+ck);
        c1[799]=c1[799]*(1.7+ck);
        c1[800]=c1[800]*(1.7+ck);
        c1[3199]=c1[3199]*(1.93+ck);*/
    }
    else
    {
        c1[45]=c1[45]*1.49;
        c1[54]=c1[54]*1.49;
        c1[3145]=c1[3145]*1.49;
        c1[3154]=c1[3154]*1.49;
        /*c1[45]=c1[45]*(1.49+ck);
        c1[54]=c1[54]*(1.49+ck);
        c1[3145]=c1[3145]*(1.49+ck);
        c1[3154]=c1[3154]*(1.49+ck);*/
    } //! 10346..10347  cd_off
    f2a.four2a_c2c(c1,NFFT2,1,1,decid);//call four2avar(c1,NFFT2,1,1,1)!c2c FFT back to time domain
    for (int i = -800; i < 0; ++i) c0[i+cd_off]=0.0+0.0*I;//c0(-800:-1)=0.0+0.0*I; //(-800:4000) [5000]
    for (int i = 0; i < 3200; ++i) c0[i+cd_off]=c1[i]*facc1; //c0(0:3199)=facc1*c1(0:3199);
    for (int i = 3200; i < 4001; ++i) c0[i+cd_off]=0.0+0.0*I;
    if (nqso>1)//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    {
        for (int i = -800; i < 0; ++i) c2[i+cd_off]=0.0+0.0*I;//c2(-800:-1)=0.;
        for (int i = 3200; i < 4001; ++i) c2[i+cd_off]=0.0+0.0*I;//c2(3200:4000)=0.
        for (int i = 0; i < 3199; ++i) c2[i+cd_off]=c0[i+cd_off]+c0[i+cd_off+1];//do i=0,3198; c2(i)=c0(i)+c0(i+1);
        c2[3199+cd_off]=c0[3199+cd_off];
        for (int i = 0; i < 4950; ++i) c2[i]*=0.5;//c2[i]=c2[i]/2.0;
    }
    if (nqso==3)
    {
        for (int i = -800; i < 0; ++i) c3[i+cd_off]=0.0+0.0*I;//c3(-800:-1)=0.;
        for (int i = 3200; i < 4001; ++i) c3[i+cd_off]=0.0+0.0*I;//c3(3200:4000)=0.
        for (int i = 1; i < 3200; ++i) c3[i+cd_off]=c0[i-1+cd_off]+c0[i+cd_off];//do i=1,3199; c3(i)=c0(i-1)+c0(i); enddo
        c3[cd_off]=c0[cd_off];
        for (int i = 0; i < 4950; ++i) c3[i]*=0.5;//c3[i]=c3[i]/2.0;
    }
    delete [] c1;
}
/*void DecoderFt8::agccft8(double nfa,double nfb)
{
    const int NFFT=1024;
    const int NSZ=426;
    const int NHSYM=178;
    const int nhstep=960;
    double nfblim;
    double x11[NFFT+128];
    double complex c11[NFFT+128];
    double ss33[NSZ+128];
    double sa33[NSZ+128];
    int indx[NSZ+128];
    double s3[NHSYM+128];

    if (first_agccft8)//! FFT window //c++ ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    {
        for (int k = 0; k < NFFT; ++k) w11[k]=sin(twopi*(double)(k+2)/2048.0);//! 16384 = 2*NFFT
        first_agccft8=false;
    }
    double fac1=1.1e-3;//1.1; //! 7548, 9.6e-4 7540, 9.7e-4 7546, 9.8e-4 7543, 1.0e-3 7539, 1.2e-3 7534..7538, 1.3e-3 7547, 2.4e-3 7543

    if (nfb<5000.0) nfblim=nfb;
    else nfblim=4999;//! ss33 top index protection
    int nf1=(double)NFFT*nfa/12000.0; //NFFT*nfa/12000+1
    int nf2=(double)NFFT*nfblim/12000.0;//==max 426
    int nmed=(nf2-nf1)/2;
    int nsb=nf2-nf1;//nf2-nf1+1
    for (int j = 0; j < 10; ++j)
    {//do j=1,10
        int i0=j*nhstep;//(j-1)*nhstep;
        for (int z = 0; z < NFFT; ++z) x11[z]=fac1*w11[z]*dd8[i0+z];//(i0+1:i0+NFFT)
        //double max=-5.0; double min=5.0;
        //for (int i = 0; i < NFFT; ++i)// red2[i]=red2[i]/base2;
        //{
        	//if (max<x11[i]) max=x11[i];
            //if (min>x11[i]) min=x11[i];
        //} if (max>0.6) qDebug()<<"   MAX="<<max<<min;
        f2a.four2a_d2c(c11,x11,NFFT,-1,0,decid); //call four2avar(c11,NFFT,1,-1,0)!r2c forward FFT
        for (int i = nf1; i < nf2; ++i)
        {//do i=nf1,nf2
            double s33=cabs(c11[i]);
            ss33[i]=sqrt(s33);
        }
        for (int i =0; i < nsb; ++i)
        {
            sa33[i]=ss33[i+nf1];//sa33(1:nsb)=ss33(nf1:nf2)
            indx[i]=0;
        }
        pomAll.indexx_msk(sa33,nsb-1,indx);//call indexx(sa33(1:nsb),nsb,indx)
        double smed=sa33[indx[nmed]];
        if (smed>1.e-6) s3[j]=smed;
        else s3[j]=1.0;
    }
    for (int j = 168; j < 178; ++j)
    {//do j=1,10
        int i0=j*nhstep;//(j-1)*nhstep;
        for (int z = 0; z < NFFT; ++z) x11[z]=fac1*w11[z]*dd8[i0+z];//(i0+1:i0+NFFT)
        f2a.four2a_d2c(c11,x11,NFFT,-1,0,decid); //call four2avar(c11,NFFT,1,-1,0)!r2c forward FFT
        for (int i = nf1; i < nf2; ++i)
        {//do i=nf1,nf2
            double s33=cabs(c11[i]);
            ss33[i]=sqrt(s33);
        }
        for (int i =0; i < nsb; ++i)
        {
            sa33[i]=ss33[i+nf1];//sa33(1:nsb)=ss33(nf1:nf2)
            indx[i]=0;
        }
        pomAll.indexx_msk(sa33,nsb-1,indx);//call indexx(sa33(1:nsb),nsb,indx)
        double smed=sa33[indx[nmed]];
        if (smed>1.e-6) s3[j]=smed;
        else s3[j]=1.0;
    }
    double s3min1=pomAll.minval_da_beg_to_end(s3,0,10);//minval(s3(1:10));
    double s3min2=pomAll.minval_da_beg_to_end(s3,168,178);//s3min2=minval(s3(169:178));
    double s3min=fmin(s3min1,s3min2);
    double s3max1=pomAll.maxval_da_beg_to_end(s3,0,10);//maxval(s3(1:10));
    double s3max2=pomAll.maxval_da_beg_to_end(s3,168,178);//double s3max2=maxval(s3(169:178));
    double s3max=fmax(s3max1,s3max2);  //qDebug()<<"s3min="<<s3min;
    if (s3min<0.1)
    {
        s3min=1.0;
        s3max=1.0;
    }
    double s3ratio=s3max/s3min;  //qDebug()<<s3max<<s3min<<"s3ratio="<<s3ratio;
    lagccbail=false; //! 2.0dB delta
    if (s3ratio<1.26)
    {
        lagccbail=true;
        return;
    }
    int k=0;
    for (int j = 0; j < 10; ++j)
    {//do j=1,10
        dd8[k+959]/=s3[j]; //dd8(k:k+959)=dd8(k:k+959)/s3(j);
        k+=960;
    }
    k=168*960;//k=1+168*960
    for (int j = 168; j < 178; ++j)
    {//do j=169,178
        dd8[k+959]/=s3[j]; //dd8(k:k+959)=dd8(k:k+959)/s3(j);
        k+=960;
    }
    for (int j = 170880; j < 171650; ++j) dd8[j]/=s3[177]; //dd8(170881:171650)=dd8(170881:171650)/s3(178);
    for (int j = 171650; j < 181000; ++j) dd8[j]=0.0;//dd8(171651:180000)=0.0
}*/
void DecoderFt8::sync8var(double nfa,double nfb,double syncmin,double nfqso,double candidate[4][512],
                          int &ncand,int jzb,int jzt,int ipass,bool lqsothread,int ncandthin,int ndtcenter,
                          double nfawide,double nfbwide)
{
    const int NFFT1=3840;//, NH1=1920)!Length of FFTs for symbol spectra NFFT1=2*NSPS, NH1=NFFT1/2
    const int NH1=1920;
    const int NSTEP=480;//!Rough time-sync step size NSTEP=NSPS/4
    const int NHSYM=372;//!Number of symbol spectra (1/4-sym steps) NHSYM=NMAX/NSTEP-3
    const int NSPS=1920;//!Samples per symbol at 12000 S/s
    const int max_c0 = 500;
    const int max_c  = 490;

    double x[NFFT1+128];//double *x=new double[NFFT1+128];//
    double complex cx[NFFT1+128];//double complex *cx=new double complex[NFFT1+128];//error -> cx[NH1+128];
    double (*s_)[1990] = new double[420][1990];//(NH1,NHSYM)
    int sync_off = 120;
    double (*sync2d)[256] = new double[1990][256];//double sync2d[NH1+128][255];//sync2d(NH1,jzb:jzt)jzb=-62 jzt=+62=124
    bool (*syncq)[256] = new bool[1990][256];//bool syncq[NH1+128][255];//syncq(NH1,jzb:jzt)
    double df=3.125; //! 12000.0/NFFT1 , Hz
    bool lcq=false;
    //bool lcq2=false;
    double tall[50];//tall(30)
    double red[NH1+128];//red(NH1)
    int jpeak[NH1+128];
    bool redcq[NH1+128];
    int indx[NH1+10];
    /*! Compute symbol spectra, stepping by NSTEP steps.
    tstep=0.04 ! NSTEP/12000.0  
    df=3.125 ! 12000.0/NFFT1 , Hz
    syncq=.false.; redcq=.false.; candidate(4,:)=0.
    rcandthin=ncandthin/100.;
    dtcenter=ndtcenter/100.*/
    const double facx=1.0/300.0;
    const double facdd8=0.03;//HV=ok   facx;//0.01;//0.01;
    //static bool first_sync8var = true;
    //static double windowx[220];
    double candidate0[5][max_c0+12];//candidate0(5,450),
    //double (*candidate0)[500] = new double[5][500];//
    double rcandthin=(double)ncandthin/100.0;
    double dtcenter=(double)ndtcenter/100.0;
    double tstep=0.04; //! NSTEP/12000.0

    for (int z = 0; z < max_c; ++z) candidate[3][z]=0.0;//candidate(4,:)=0.
    //for (int z = 0; z < 45; ++z) tall[z] = 0.0;
    for (int z = 0; z < NH1+10; ++z)
    {
        redcq[z]=false;
        for (int z0 = 0; z0 < 250; ++z0) syncq[z][z0]=false;
    }
    //for (int z = 0; z < NFFT1+10; ++z) cx[z]=0.0+0.0*I;
    /*if (first_sync8var)
    {
        for (int i = 0; i < 201; ++i) windowx[i]=(1.0+cos(i*pi/200.0))/2.0;
        for (int i = 0; i < 201; ++i) windowx[i]=facx*windowx[i];
        first_sync8var = false;
    }*/
    int ia=0;
    int ib=0;
    int iaw=0;
    int ibw=0; //190000
    for (int j = 0; j < NHSYM; ++j)//if (ipass==1 || ipass==4 || ipass==7)
    {//do j=1,NHSYM
        ia=j*NSTEP;//int ia=(j-1)*NSTEP + 1
        ib=ia+NSPS;//ib=ia+NSPS-1
        for (int z = 0; z < 759; ++z) x[z]=0.0;//x(1:759)=0.
        if (j!=0)//if(j.ne.1) then; x(760:960)=dd8(ia-201:ia-1)*windowx(200:0:-1); else; x(760:960)=0.; endif
        {
            for (int z = 0; z < 201; ++z) x[z+759]=dd8[(ia-200)+z]*windowx[200-z]*facdd8;
        }
        else
        {
            for (int z = 0; z < 201; ++z) x[z+759]=0.0;
        }
        for (int z = 0; z < NSPS; ++z) x[z+960]=dd8[z+ia]*facdd8*facx;//x(961:2880)=facx*dd8(ia:ib);
        x[960]=x[960]*1.9;
        x[2879]=x[2879]*1.9;//x(961)=x(961)*1.9; x(2880)=x(2880)*1.9
        if (j!=NHSYM-1)//if(j.ne.NHSYM) then; x(2881:3081)=dd8(ib+1:ib+201)*windowx; else; x(2881:3081)=0.; endif
        {
            for (int z = 0; z < 201; ++z) x[z+2880]=dd8[ib+z]*windowx[z]*facdd8;
        }
        else
        {
            for (int z = 0; z < 201; ++z) x[z+2880]=0.0;
        }
        for (int z = 3081; z < NFFT1+10; ++z) x[z]=0.0;//x(3082:)=0.
        f2a.four2a_d2c(cx,x,NFFT1,-1,0,decid);//four2avar(cx,NFFT1,1,-1,0)!r2c FFT
        if (ipass==1 || ipass==4 || ipass==7)
        {
            for (int i = 0; i < NH1; ++i) s_[j][i]=sqrt(pomAll.ps_hv(cx[i]));//do i=1,NH1 s(i,j)=SQRT(real(cx(i))**2 + aimag(cx(i))**2)
        }
        if (ipass==2 || ipass==5 || ipass==8)
        {
            for (int i = 0; i < NH1; ++i) s_[j][i]=pomAll.ps_hv(cx[i]);//s(i,j)=real(cx(i))**2 + aimag(cx(i))**2
        }
        if (ipass==3 || ipass==6 || ipass==9)
        {
            for (int i = 0; i < NH1; ++i) s_[j][i]=fabs(creal(cx[i]))+fabs(cimag(cx[i]));//s(i,j)=abs(real(cx(i))) + abs(aimag(cx(i)))
        }
        //for (int i = 0; i < NH1; ++i) s_[j][i]*=0.00001;
        /*double max=-5.0; double min=5.0;
        for (int i = 0; i < NH1; ++i)// red2[i]=red2[i]/base2;
        {
        if (max<red[i]) max=s_[j][i];
        if (min>red[i]) min=s_[j][i];
        } if (max>0.9) qDebug()<<"   MAX="<<max<<min;*/
    }
    for (int j = NHSYM; j < NHSYM+4; ++j)//for CQ
    {
        for (int i = 0; i < NH1; ++i) s_[j][i]=0.0;
    }
    //ia=max(1,nint(nfa/df)); ib=max(1,nint(nfb/df)); iaw=max(1,nint(nfawide/df)); ibw=max(1,nint(nfbwide/df))
    int corrt = 0;
    if (decid>0) corrt = 17;       //2.72 5=16Hz 17=53Hz 18=56Hz
    ia=fmax(0,(int(nfa/df)-corrt));
    //ia=fmax(0,(int)(nfa/df));
    ib=fmax(0,(int)(nfb/df));
    //iaw=fmax(0,(int(nfawide/df)-corrt));
    iaw=fmax(0,(int)(nfawide/df));
    ibw=fmax(0,(int)(nfbwide/df));
    //iaw=ia; ibw=ib;
    //ia=iaw; ib=ibw;
    int nssy=4; //! NSPS/NSTEP   ! # steps per symbol
    int nssy36=144; //! nssy*36
    int nssy72=288; //! nssy*72
    int nfos=2; //! NFFT1/NSPS   ! # frequency bin oversampling factor
    double jstrt=12.5; //! 0.5/tstep
    bool lagcc=true;//button on the front panel
    bool lagccbail=true;// <- agccft8(nfa,nfb,forcedt) not used for the moment
    if (lagcc && !lagccbail)
    {
        int nfos6=12; //! nfos*6
        for (int j = jzb; j <= jzt; ++j)
        {//do j=jzb,jzt
            for (int i = iaw; i <= ibw; ++i)
            {//do i=iaw,ibw
                double ta=0.0;
                double tb=0.0;
                double tc=0.0;
                for (int n = 0; n <= 6; ++n)//c++ ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
                {//do n=0,6
                    int k=(int)((double)(j+nssy*n)+jstrt);//k=j+jstrt+nssy*n
                    if (k>=0 && k<NHSYM)
                    {
                        ta=s_[k][i+nfos*icos7_2[n]];//ta=s(i+nfos*icos7(n),k)
                        if (ta>1e-9)//if(ta.gt.1e-9) then; tall(n+1)=ta*6.0/(sum(s(i:i+nfos6:nfos,k))-ta); else; tall(n+1)=0.; endif
                        {
                            double sum0 = 0.0;//(sum(s(i:i+nfos6:nfos,k))-ta);
                            for (int z = i; z <= i+nfos6; z+=nfos) sum0 += s_[k][z];
                            sum0-=ta;
                            if (sum0==0.0) sum0=0.0000001;
                            tall[n]=ta*6.0/sum0;
                        }
                        else tall[n]=0.0;//tall(n+1)=0.;
                    }
                    int k36=k+nssy36;
                    if (k36>=0 && k36<NHSYM)
                    {
                        tb=s_[k36][i+nfos*icos7_2[n]];//tb=s(i+nfos*icos7(n),k36)
                        if (tb>1e-9)//tall(n+17)=tb*6.0/(sum(s(i:i+nfos6:nfos,k36))-tb);
                        {
                            double sum0 = 0.0;
                            for (int z = i; z <= i+nfos6; z+=nfos) sum0 += s_[k36][z];
                            sum0-=tb;
                            if (sum0==0.0) sum0=0.0000001;
                            tall[n+16]=tb*6.0/sum0;
                        }
                        else tall[n+16]=0.0;//tall(n+17)=0.; endif
                    }
                    int k72=k+nssy72;
                    if (k72>=0 && k72<NHSYM)
                    {
                        tc=s_[k72][i+nfos*icos7_2[n]];//tc=s(i+nfos*icos7(n),k72)
                        if (tc>1e-9) //then; tall(n+24)=tc*6.0/(sum(s(i:i+nfos6:nfos,k72))-tc); else; tall(n+24)=0.; endif
                        {
                            double sum0 = 0.0;
                            for (int z = i; z <= i+nfos6; z+=nfos) sum0 += s_[k72][z];
                            sum0-=tc;
                            if (sum0==0.0) sum0=0.0000001;
                            tall[n+23]=tc*6.0/sum0;
                        }
                        else tall[n+23]=0.0;//tall(n+24)=0.; endi
                    }
                }
                lcq=false;
                double sync_abc = 0.0;
                double sync_bc = 0.0;
                if (ipass>1)
                {
                    for (int n = 7; n < 16  ; ++n)
                    {//do n=7,15 ????
                        int k=(int)((double)(j+nssy*n)+jstrt);
                        if (k>=0 && k<NHSYM)
                        {
                            if (n<15)
                            {
                                double sum0 = 0.0;//tall(n+1)=s(i,k)*6.0/(sum(s(i:i+nfos6:nfos,k))-s(i,k))
                                for (int z = i; z <= i+nfos6; z+=nfos) sum0 += s_[k][z];
                                sum0 -= s_[k][i];
                                if (sum0==0.0) sum0=0.0000001;
                                tall[n]=s_[k][i]*6.0/sum0;
                            }
                            else
                            {
                                double sum0 = 0.0;//tall(n+1)=s(i+2,k)*6.0/(sum(s(i:i+nfos6:nfos,k))-s(i+2,k))
                                for (int z = i; z <= i+nfos6; z+=nfos) sum0 += s_[k][z];
                                sum0 -= s_[k][i+2];
                                if (sum0==0.0) sum0=0.0000001;
                                tall[n]=s_[k][i+2]*6.0/sum0;
                            }
                        }
                    }
                    double sya=0.0;//sum(tall(1:7));
                    for (int z = 0; z < 7  ; ++z) sya+=tall[z];
                    double sycq=0.0;//sum(tall(8:16));
                    for (int z = 0; z < 9  ; ++z) sycq+=tall[z+7];//tall[z+7];
                    double sybc=0.0;//sum(tall(17:30));
                    for (int z = 0; z < 14  ; ++z) sybc+=tall[z+16];//tall[z+16];
                    double sy1=(sya+sycq+sybc)/30.0;
                    double sy2=(sya+sybc)/21.0;
                    sync_abc=fmax(sy1,sy2);
                    sy1=(sycq+sybc)/23.0;
                    sy2=(sybc)/14.0;
                    sync_bc=fmax(sy1,sy2);
                    if (sy1>sy2) lcq=true;
                }
                else
                {
                    double sybc = 0.0;
                    for (int z = 0; z < 14  ; ++z) sybc += tall[z+16];//sybc=sum(tall(17:30));
                    for (int z = 0; z < 7  ; ++z) sync_abc += tall[z];//sum(tall(1:7))+sybc;
                    sync_abc += sybc;
                    sync_bc=sybc/14.0;
                    sync_abc=sync_abc/21.0;
                }
                sync2d[i][j+sync_off]=fmax(sync_abc,sync_bc);//sync2d(i,j)=max(sync_abc,sync_bc);
                if (lcq) syncq[i][j+sync_off]=true;//syncq(i,j)=.true.
            }
        }
    }
    else
    {
        int nfos6=16; //jzb=-70; jzt=70; //!nfos6=15 ! 16i spec bw -1
        for (int j = jzb; j <= jzt; ++j)
        {//do j=jzb,jzt
            for (int i = iaw; i <= ibw; ++i)
            {//do i=iaw,ibw
                double ta=0.0;
                double tb=0.0;
                double tc=0.0;
                double tcq=0.0;
                double t0a=0.0;
                double t0b=0.0;
                double t0c=0.0;
                double t0cq=0.0;
                for (int n = 0; n <= 6; ++n)//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
                {//do n=0,6
                    int k=(int)((double)(j+nssy*n)+jstrt);//k=j+jstrt+nssy*n
                    if (k>=0 && k<NHSYM)
                    {
                        ta += s_[k][i+nfos*icos7_2[n]];//s(i+nfos*icos7(n),k);
                        double sum0 = 0.0;//t0a=t0a + sum(s(i:i+nfos6,k)) - s(i+nfos*icos7(n)+1,k);
                        for (int z = i; z <= i+nfos6; ++z) sum0 += s_[k][z];
                        t0a += (sum0 - s_[k][i+nfos*icos7_2[n]]);//+1
                    }
                    int k36=k+nssy36;
                    if (k36>=0 && k36<NHSYM) //if(k36.gt.0 .and. k36.le.NHSYM) then
                    {
                        tb += s_[k36][i+nfos*icos7_2[n]];//tb=tb + s(i+nfos*icos7(n),k36);
                        double sum0 = 0.0;//t0b=t0b + sum(s(i:i+nfos6,k36)) - s(i+nfos*icos7(n)+1,k36)
                        for (int z = i; z <= i+nfos6; ++z) sum0 += s_[k36][z];
                        t0b += (sum0 - s_[k36][i+nfos*icos7_2[n]]);//+1
                    }
                    int k72=k+nssy72;
                    if (k72>=0 && k72<NHSYM)//if(k72.le.NHSYM) then
                    {
                        tc += s_[k72][i+nfos*icos7_2[n]];//tc=tc + s(i+nfos*icos7(n),k72)
                        double sum0 = 0.0;//t0c=t0c + sum(s(i:i+nfos6,k72)) - s(i+nfos*icos7(n)+1,k72)
                        for (int z = i; z <= i+nfos6; ++z) sum0 += s_[k72][z];
                        t0c += (sum0 - s_[k72][i+nfos*icos7_2[n]]);//+1
                    }
                }
                for (int n = 7; n < 16; ++n)//OK Tested HV
                {//do n=7,15
                    int k=(int)((double)(j+nssy*n)+jstrt);//k=j+jstrt+nssy*n
                    if (k>=0 && k<NHSYM)//if(k.ge.1) then
                    {
                        if (n<15)//if(n.lt.15) then; tcq=tcq + s(i,k); t0cq=t0cq + sum(s(i:i+nfos6,k)) - s(i,k+1)
                        {
                            tcq += s_[k][i];
                            double sum0 = 0.0;
                            for (int z = i; z <= i+nfos6; ++z) sum0 += s_[k][z];
                            t0cq += (sum0 - s_[k+1][i]);
                        }
                        else//else; tcq=tcq + s(i+2,k); t0cq=t0cq + sum(s(i:i+nfos6,k)) - s(i,k+3)
                        {
                            tcq += s_[k][i+2];
                            double sum0 = 0.0;
                            for (int z = i; z <= i+nfos6; ++z) sum0 += s_[k][z];
                            t0cq += (sum0 - s_[k+3][i]);
                        }
                    }
                }
                double t1=ta+tb+tc;
                double t01=t0a+t0b+t0c;
                double t2=t1+tcq;
                double t02=t01+t0cq;
                t01=(t01-(t1*2.0))/42.0;
                if (t01<1e-8) t01=1.0;
                if (t01==0.0) t01=0.0001;//
                t02=(t02-(t2*2.0))/60.0;
                if (t02<1e-8) t02=1.0;
                if (t02==0.0) t02=0.0001;//
                double sync01=t1/(7.0*t01);
                double sync02=((t1/7.0) + (tcq/9.0))/t02;
                double syncf=fmax(sync01,sync02);
                //if (syncf>100) qDebug()<<"=1==="<<sync01<<sync02<<"t="<<t1<<t01<<t2<<t02;
                lcq=false;
                if (sync02>sync01) lcq=true;
                t1=tb+tc;
                t01=t0b+t0c;
                t2=t1+tcq;
                t02=t01+t0cq;
                t01=(t01-(t1*2.0))/28.0;
                if (t01<1e-8) t01=1.0;
                if (t01==0.0) t01=0.0001;//
                t02=(t02-(t2*2.0))/46.0;
                if (t02<1e-8) t02=1.0;
                if (t02==0.0) t02=0.0001;//
                sync01=t1/(7.0*t01);
                sync02=((t1/7.0) + (tcq/9.0))/t02;
                double syncs=fmax(sync01,sync02);
                //if (syncs>100) qDebug()<<"=2==="<<sync01<<sync02<<"t="<<t1<<t01<<t2<<t02;
                bool lcq2=false;
                if (sync02>sync01) lcq2=true;
                //sync2d[i][j+sync_off]=fmax(syncf,syncs); //sync2d(i,j)=max(syncf,syncs) sync_off  double sync2d_[255][NH1+128];
                /*double res=syncf;
                if (res<syncs) res=syncs; 
                if (res>50.0 ) res=syncf; 
                if (res>50.0 ) res=50.0;*/
                double res=syncf;
                if (res<syncs) res=syncs;
                if (res>50.0 ) res=50.0;
                sync2d[i][j+sync_off]=res;
                if (syncf>syncs)//if(syncf.gt.syncs) then; if(lcq) syncq(i,j)=.true.; else; if(lcq2) syncq(i,j)=.true.; endif
                {
                    if (lcq)
                    {
                        syncq[i][j+sync_off]=true; /*cqqq1++;*/
                    }
                    else
                    {
                        if (lcq2)
                        {
                            syncq[i][j+sync_off]=true; /*cqqq2++;*/
                        }
                    }
                } //if (i>1910 || (j+sync_off)<2 || (j+sync_off)>230) qDebug()<<i<<j+sync_off;//[1990][256]
            }
        }
    }
    //qDebug()<<sync2d[1000][sync_off]<<sync2d[1000][sync_off-1]<<sync2d[1000+1][sync_off]<<sync2d[1000+2][sync_off];
    //double red2[NH1+128];//red(NH1)
    //int jpeak2[NH1+128];
    //int indx2[NH1+10];
    //jzt=+18; jzb=-18;
    //ibw=ib; iaw=ia;
    int t0a=jzb+sync_off; //t0a-=15;
    int t1a=jzt+sync_off; //t1a+=15;
    //qDebug()<<"---3---sync8var"<<sync2d[100][sync_off]<<sync2d[100][sync_off+1]<<sync2d[100][sync_off+2];
    for (int z = 0; z <= NH1+10; ++z)
    {
        red[z]=0.0;//red=0.
        //red2[z]=0.0;
    }// int ccq = 0;
    for (int i = iaw; i < ibw; ++i)
    {//do i=iaw,ibw
        int j0=pomAll.maxloc_da_beg_to_end(sync2d[i],t0a,t1a) - sync_off;//maxloc(sync2d(i,jzb:jzt)) - 1 + jzb
        jpeak[i]=j0;//j0=ii(1)
        red[i]=sync2d[i][j0+sync_off];//red(i)=sync2d(i,j0);
        if (syncq[i][j0+sync_off]) redcq[i]=true;//if(syncq(i,j0)) redcq(i)=.true.
        //j0=(pomAll.maxloc_da_beg_to_end(sync2d[i],sync_off-18,sync_off+18) - sync_off);//hv=Search over +/-0.6s = +-16 //260r5= +-10
        //jpeak2[i]=j0;
        //red2[i]=sync2d[i][j0+offset_sync2d];
    }   //qDebug()<<"    ccq="<<ccq;
    //ibw=ib; iaw=ia;
    int iz=ibw-iaw;
    if (iz>1920) iz=1920;
    double t_red[2048];//2.12 (10Hz to 6000Hz = 1917)
    //double t_red2[2048];
    for (int i = 0; i < iz; ++i)
    {
        t_red[i]=red[i+iaw];
        indx[i]=0;
        //t_red2[i] = red2[i+iaw];
        //indx2[i]=0;
    }
    indx[iz]   =0;
    indx[iz+1] =0;
    //indx2[iz]  =0;
    //indx2[iz+1]=0;
    //qDebug()<<"---3-1-1-sync8var"<<iz<<t_red[40];
    if (iz>0) pomAll.indexx_msk(t_red, iz-1,indx);
    //if (iz>0) pomAll.indexx_msk(t_red2,iz-1,indx2);
    int npctile=(int)(0.40*(double)(iz));//0.40
    if (npctile<0) npctile=0;// something is wrong; bail out
    int ibase =indx[npctile] + iaw;
    //int ibase2=indx2[npctile] + iaw;
    if (ibase<0) ibase=0;//2.12
    if (ibase>NH1-1) ibase=NH1-1;//2.12
    //if (ibase2<0) ibase2=0;//2.69
    //if (ibase2>NH1-1) ibase2=NH1-1;//2.69
    double base=red[ibase];
    //base = pomAll.pctile_shell(t_red,iz-1,40);
    if (base <=0.000001) base=0.000001;//no devide by 0
    //if (base2<=0.001) base2=0.001;
    //qDebug()<<decid<<"DEC"<<(62.5*tstep)<<(-50.0*tstep)<<(75.0*tstep)<<iaw<<ibw<<ia<<ib<<iz<<base;//0.906681
    for (int i = 0; i < NH1; ++i) red[i]/=base;
    /*double max=0.0; double min=5.0; int im=0;
    for (int i = 0; i < NH1; ++i)// red2[i]=red2[i]/base2;
    {
    	if (max<red[i]) {max=red[i]; im=i;}
    	if (min>red[i]) min=red[i];
    } if (max>25) qDebug()<<decid<<"IM="<<im<<"IM0="<<red[im-1]<<"MAX="<<max<<"IM1="<<red[im+1]<<"Base="<<base;*/
    /*iz=ibw-iaw+1
    call indexx(red(iaw:ibw),iz,indx)
    ibase=indx(max(1,nint(0.40*iz))) - 1 + iaw //! max is workaround to prevent indx getting out of bounds
    base=red(ibase)
    if(base.lt.1e-8) base=1.0 ! safe division
    red=red/base*/
    for (int z = 0; z < 5; ++z)
    {
        for (int z0 = 0; z0 < max_c0; ++z0) candidate0[z][z0]=0.0;//candidate0=0.;
    }

    int k=0; //if (decid==0) ib=ibw-150;
    iz=ib-ia;//iz=ib-ia+1;
    if (iz>1920) iz=1920;
    bool lpass1=false;
    bool lpass2=false;//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    if (rcandthin<0.99)
    {
        if (ipass==1 || ipass==4 || ipass==7) lpass1=true;
        else if (ipass==2 || ipass==5 || ipass==8) lpass2=true;
    }
    for (int i = 0; i < iz; ++i)
    {
        t_red[i]  = red[i+ia];
        indx[i] =0;
    }
    indx[iz]   =0;
    indx[iz+1] =0;
    if (iz>0) pomAll.indexx_msk(t_red, iz-1,indx); //call indexx(red(ia:ib),iz,indx)
    //const double ksyncmin = 0.15;
    const double syncmin1 = 1.1;//+ksyncmin;
    //if (ipass==4 || ipass==5 || ipass==9) syncmin+=1.0;
    for (int i = 0; i < iz; ++i)
    {//do i=1,iz    int n=ia + indx[(iz-1)-i]; //indx[1920]  tested -1 ->1.69 n=ia + indx(iz+1-i) - 1
        int n=ia+indx[(iz-1)-i];//n=ia + indx(iz+1-i) - 1
        double freq=(double)n*df;
        if (fabs(freq-nfqso)>3.0)
        {
            if (red[n]<syncmin) continue;
        }
        else
        {
            if (red[n]<syncmin1) continue;//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
        }
        //if (jpeak[n]<(-49) || jpeak[n]>(76)) continue;//-53 hv  if(jpeak(n).lt.-49 .or. jpeak(n).gt.76) cycle
        //if (jpeak[n]<(-50) || jpeak[n]>(75)) continue;//-2sec to +3sec
        if (jpeak[n]<(-53) || jpeak[n]>(72)) continue;//-2.12sec to +2.88sec
        //! being sorted by sync
        candidate0[0][k]=freq;//candidate0(1,k)=freq;
        candidate0[1][k]=(double)(jpeak[n])*tstep;//candidate0(2,k)=(jpeak(n)-1)*tstep
        candidate0[2][k]=red[n];
        if (rcandthin<0.99)// then //! candidate thinning option
        {
            if (lpass2)
            {
                double del0 = ((fabs(candidate0[1][k]-dtcenter)+1.0)*(fabs(candidate0[1][k]-dtcenter)+1.0));
                if (del0==0.0) del0=0.000001;
                candidate0[4][k]=candidate0[2][k]/del0;
            }
            else
            {
                double del0 = (fabs(candidate0[1][k]-dtcenter)+1.0);
                if (del0==0.0) del0=0.000001;
                candidate0[4][k]=candidate0[2][k]/del0;
            }
        }
        if (redcq[n]) candidate0[3][k]=2.0;
        k++;
        if (k>=max_c0) break;
    }
    ncand=k; //qDebug()<<"---4---sync8var"<<k;
    double fdif0=4.0;
    //double xdtdelta=0.0;//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    for (int i = 0; i < ncand; ++i)//! save sync only to the best of near-dupe freqs
    {//do i=1,ncand
        if (i>=1)
        {
            for (int j = 0; j < i; ++j)
            {//do j=1,i-1
                double fdiff=fabs(candidate0[0][i]-candidate0[0][j]);//fdiff=abs(candidate0(1,i)-candidate0(1,j))
                double xdtdelta=fabs(candidate0[1][i]-candidate0[1][j]);//xdtdelta=abs(candidate0(2,i)-candidate0(2,j))
                //double tdiff=fabs(candidate0[1][i]-candidate0[1][j]);
                if (fdiff<fdif0 && fabs(candidate0[0][i]-nfqso)>3.0/* && tdiff<0.08*/)
                {
                    if (xdtdelta<0.1)
                    {
                        if (candidate0[2][i]>=candidate0[2][j]) candidate0[2][j]=0.0; //if(candidate0(3,i).ge.candidate0(3,j)) candidate0(3,j)=0.
                        if (candidate0[2][i]<candidate0[2][j]) candidate0[2][i]=0.0; //if(candidate0(3,i).lt.candidate0(3,j)) candidate0(3,i)=0.
                    }
                }
            }
        }
    }
    //! Sort by sync
    //!  call indexx(candidate0(3,1:ncand),ncand,indx)
    for (int i = 0; i < ncand+1; ++i) indx[i]=0;
    //qDebug()<<"Sync8"<<ncand<<iz<<rcandthin<<t_red[20]; return;
    if (rcandthin>0.99) pomAll.indexx_msk(candidate0[2],ncand-1,indx);//then; call indexx(candidate0(3,1:ncand),ncand,indx)
    else pomAll.indexx_msk(candidate0[4],ncand-1,indx);//else; call indexx(candidate0(5,1:ncand),ncand,indx)
    //! sort by sync value with DT weight UP
    //! Sort by frequency //!  call indexx(candidate0(1,1:ncand),ncand,indx)
    k=0; //k=1;
    int ncandfqso=0;
    //bool ffull = false;
    //!Put nfqso at top of list and apply lowest sync threshold for nfqso
    double fprev=5004.0;
    for (int i = ncand-1; i >= 0; --i)//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    {//do i=ncand,1,-1
        int j=indx[i];
        if (fabs(candidate0[0][j]-nfqso)<=3.0 && candidate0[2][j]>=1.1 && fabs(candidate0[0][j]-fprev)>3.0/* && candidate0[1][i]>=-2.5*/)
        {
            candidate[0][k]=candidate0[0][j];
            candidate[1][k]=candidate0[1][j];//candidate(1,k)=candidate0(1,j); candidate(2,k)=candidate0(2,j)
            candidate[2][k]=candidate0[2][j];
            candidate[3][k]=candidate0[3][j];//candidate(3,k)=candidate0(3,j); candidate(4,k)=candidate0(4,j)
            fprev=candidate0[0][j];//fprev=candidate0(1,j)
            k++;//ncandfqso++;
            /*if (k>=max_c)
            {
                ffull=true;
                break;
            }*/
        }
    } //qDebug()<<"KK="<<k;
    //!put virtual candidates for FT8Svar decoder
    if (lqsothread)// && !ffull
    {
        candidate[0][k]=(nfqso);
        candidate[1][k]=5.0; //! xdt
        candidate[2][k]=0.0; //! sync
        k++;//ncandfqso++;
        candidate[0][k]=(nfqso);
        candidate[1][k]=-5.0;
        candidate[2][k]=0.0;
        k++;//ncandfqso++;
    }
    ncandfqso = k;
    //if (!ffull)
    //{
    for (int i = ncand-1; i >= 0; --i)//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    {//do i=ncand,1,-1
        int j=indx[i];
        double syncmin10=syncmin1;
        if (fabs(candidate0[0][j]-nfqso)>3.0) syncmin10=syncmin;//then; syncmin1=syncmin; else; syncmin1=1.1; endif
        //else syncmin1=1.1;
        if (candidate0[2][j] >= syncmin10/* && candidate0[1][i]>=-2.5*/)// then
        {
            candidate[0][k]=candidate0[0][j];
            candidate[1][k]=candidate0[1][j];//candidate(1,k)=candidate0(1,j); candidate(2,k)=candidate0(2,j)
            candidate[2][k]=candidate0[2][j];
            candidate[3][k]=candidate0[3][j];//candidate(3,k)=candidate0(3,j); candidate(4,k)=candidate0(4,j)
            k++;
            if (k>=max_c) break;//if(k.gt.460) exit
        }
    }
    //}
    ncand=k;//ncand=k-1 //if(ncand-ncandfqso.gt.1 .and. rcandthin.lt.0.99) then
    //qDebug()<<"IN"<<ncand<<ncandfqso<<ncand-ncandfqso<<rcandthin;
    if (ncand-ncandfqso>1 && rcandthin<0.99)
    {  //! applying decoding pass weight factor, derived from number of candidates in each pass
        if (lpass1) rcandthin=fmin(rcandthin*1.27,1.0);
        else if (lpass2)// then
        {
            if (rcandthin>0.79) rcandthin=rcandthin*rcandthin;
            else rcandthin=rcandthin*0.79;
        }
        else rcandthin=fmin(rcandthin*5.0,1.0); //! ipass 3,6,9
        ncand=ncandfqso+(int)((double)(ncand-ncandfqso)*rcandthin);//qDebug()<<"OUT==="<<ncand<<rcandthin;
    }
    /*int ccq = 0;
    for (int i = 0; i < ncand; ++i)
    {
    if (candidate0[3][i]==2.0) ccq++;  
    } qDebug()<<"OUT="<<ncand<<"CQ="<<ccq;*/
    //ncand=(double)ncand*0.6;
    delete [] syncq;
    delete [] sync2d;
    delete [] s_;
}
void DecoderFt8::tonesdvar(QString msgd,bool lcq)
{
    const QString rpt[75]=
        {"+09","+08","+07","+06","+05","+04","+03","+02","+01","+00",
         "-01","-02","-03","-04","-05","-06","-07","-08","-09","-10",
         "-11","-12","-13","-14","-15","-16","-17","-18","-19","-20",
         "-21","-22","-23","-24","-25","-26","R+09","R+08","R+07","R+06",
         "R+05","R+04","R+03","R+02","R+01","R+00","R-01","R-02","R-03","R-04",
         "R-05","R-06","R-07","R-08","R-09","R-10","R-11","R-12","R-13","R-14",
         "R-15","R-16","R-17","R-18","R-19","R-20","R-21","R-22","R-23","R-24",
         "R-25","R-26","RRR","RR73","73"
        };
    QString msg37="";
    //bool c77[100];
    int itone[120];
    bool lgrid=false;
    bool lr73=false;
    QString c1="";
    QString c2="";
    QString grid="";
    int itone1[120];
    double complex *csig0 = new double complex[153681];//double complex csig0[151680+1024];//(151680)
    //msgd = "";//"LZ2HV SP9HWY"; qDebug()<<"tonesdvar="<<lcq<<msgd;
    if (lcq)
    {
        msg37=msgd;
        TGenFt8->pack77_make_c77_i4tone(msg37,itone);
    }
    else//c++ ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    {
        msgd.append("    ");//c1='            '; c2='            ' LZ2HV SP9HWY KN23
        int ispc1=msgd.indexOf(' ');//ispc1=index(msgd,' ');
        int ispc2=msgd.indexOf(' ',ispc1+1);//ispc2=index(msgd((ispc1+1):),' ')+ispc1;
        int ispc3=msgd.indexOf(' ',ispc2+1);//ispc3=index(msgd((ispc2+1):),' ')+ispc2;
        //if(len(msgd(1:ispc1-1)).le.12 .and. len(msgd(ispc1+1:ispc2-1)).le.12) then
        //c1=msgd(1:ispc1-1); c2=msgd(ispc1+1:ispc2-1)
        //if (msgd.mid(0,ispc1).count()<=12 && msgd.mid(ispc1+1,ispc2-(ispc1+1)).count()<=12)
        if (ispc1<=12 && (ispc2-(ispc1+1))<=12)
        {
            c1=msgd.mid(0,ispc1);
            c2=msgd.mid(ispc1+1,ispc2-(ispc1+1));
            c1=c1.trimmed();
            c2=c2.trimmed();
        }
        if (msgd.indexOf(" RR73")>0 || msgd.indexOf(" 73")>0) lr73=true;
        if (!lr73 && (ispc3-ispc2)==5 && msgd.mid(ispc2+1,2)!="R+" && msgd.mid(ispc2+1,2)!="R-") lgrid=true;
        if (lgrid)
        {
            grid=msgd.mid(ispc2+1,ispc3-(ispc2+1));
            if (grid.count()!=4) grid="AA00";
        }
        else grid="AA00";
        for (int i = 0; i < 75; ++i)
        {//do i=1,75
            msg37=c1+" "+c2+" "+rpt[i];//msg37='                                     '
            msgsd76[i]=msg37; //qDebug()<<"ton="<<msgsd76[i];
            TGenFt8->pack77_make_c77_i4tone(msg37,itone);
            if (i==0) for (int x = 0; x < 82; ++x) itone1[x]=itone[x];
            for (int x = 0; x < 29; ++x) idtone76[i][x]=itone[x+7];//idtone76(i,1:29)=itone(8:36)
            for (int x = 0; x < 29; ++x) idtone76[i][x+29]=itone[x+43];//idtone76(i,30:58)=itone(44:72)
            for (int x = 0; x < 79; ++x) itone76[i][x]=itone[x];//itone76(i,1:79)=itone(1:79)
        }
        msg37=c1+" "+c2+" "+grid;//msg37=trim(c1)//' '//trim(c2)//' '//trim(grid)
        msgsd76[75]=msg37;//msg37='                                     '
        TGenFt8->pack77_make_c77_i4tone(msg37,itone);
        for (int x = 0; x < 29; ++x) idtone76[75][x]=itone[x+7];//idtone76(76,1:29)=itone(8:36)
        for (int x = 0; x < 29; ++x) idtone76[75][x+29]=itone[x+43];//idtone76(76,30:58)=itone(44:72)
        for (int x = 0; x < 79; ++x) itone76[75][x]=itone[x];//itone76(76,1:79)=itone(1:79)
        //qDebug()<<"ton="<<c1<<c2<<grid<<"Counts="<<ispc1<<(ispc2-(ispc1+1))<<(ispc3-ispc2);
        //msgd=msgd.trimmed();
    }
    int m=13441-1; //! 7*1920+1
    if (lcq)
    {
        gen_ft8cwaveRx(itone,0.0,csig0);//call gen_ft8wavevar(itone,79,1920,2.0,12000.0,0.0,csig0,xjunk,1,151680)
        for (int i = 0; i < 58; ++i)
        {//do i=0,57
            if (i==29) m+=13440;
            for (int j = 0; j < 32; ++j)
            {//do j=1,32;
                csyncsdcq[i][j]=csig0[m];//csyncsdcq(i,j)=csig0(m);
                m+=60;
            }
        }
    }
    else
    {
        gen_ft8cwaveRx(itone1,0.0,csig0);//call gen_ft8wavevar(itone1,79,1920,2.0,12000.0,0.0,csig0,xjunk,1,151680)
        for (int i = 0; i < 19; ++i)
        {//do i=0,18
            for (int j = 0; j < 32; ++j)
            {//do j=1,32;
                csyncsd[i][j]=csig0[m];
                m+=60;
            }
        }
    }
    delete [] csig0;
}
void DecoderFt8::sync8dvar(double complex *cd0,int i0,double complex *ctwk,int itwk,double &sync,int ipass,
                           bool lastsync,int iqso,bool lcq,bool lcallsstd,bool lcqcand,int cd_off)
{
    double sync1[21];//(0:20)
    double complex zt1[7];//(0:6)
    double complex zt2[7];//(0:6)
    double complex zt3[7];//(0:6)
    double complex zt4[8];//(0:7)
    //double complex z11[224];//(224)
    //double complex z22[224];//(224)
    //double complex z33[224];//(224)
    double complex csync2[32];//(32)
    double complex z1=0.0+0.0*I;
    double complex z2=0.0+0.0*I;
    double complex z3=0.0+0.0*I;
    double complex z4=0.0+0.0*I;
    double complex csync1[19][32];//(0:18,32)
    double complex zt5[19];//(0:18)
    //const double zk = 0.1;

    sync=0.0;
    for (int i = 0; i < 21; ++i) sync1[i]=0.0;
    for (int i = 0; i < 7; ++i)
    {
        zt1[i]=0.0+0.0*I;
        zt2[i]=0.0+0.0*I;
        zt3[i]=0.0+0.0*I;
    }
    /*for (int i = 0; i < 224; ++i)
    {
        z11[i]=0.0+0.0*I;
        z22[i]=0.0+0.0*I;
        z33[i]=0.0+0.0*I;
    }*/
    const int NP2=2812;
    //int k=0;//1;
    for (int i = 0; i <= 6; ++i)
    {//do i=0,6 //! Sum over 7 Costas frequencies and three Costas arrays
        int i1=i0+i*32;
        int i2=i1+1152;
        int i3=i1+2304; //! +36*32, +72*32
        for (int z = 0; z < 32; ++z) csync2[z]=csync[i][z];//(i,1:32)
        if (itwk==1) for (int z = 0; z < 32; ++z) csync2[z]=ctwk[z]*csync2[z];//!Tweak the frequency
        /*double complex sum1=0.0+0.0*I;
        double complex sum2=0.0+0.0*I;
        double complex sum3=0.0+0.0*I;*/
        /*for (int z = 0; z < 32; ++z)
        {
            //z11[k+z]=cd0[i1+z+cd_off]*conj(csync2[z]);//z11(k:k+31)=cd0(i1:i1+31)*conj(csync2);
            //z22[k+z]=cd0[i2+z+cd_off]*conj(csync2[z]);//z22(k:k+31)=cd0(i2:i2+31)*conj(csync2[z])
            //z33[k+z]=cd0[i3+z+cd_off]*conj(csync2[z]);//z33(k:k+31)=cd0(i3:i3+31)*conj(csync2[z])
            zt1[i]+=cd0[i1+z+cd_off]*conj(csync2[z]);
            zt2[i]+=cd0[i2+z+cd_off]*conj(csync2[z]);
            zt3[i]+=cd0[i3+z+cd_off]*conj(csync2[z]);
            //sum1+=z11[k+z];
            //sum2+=z22[k+z];
            //sum3+=z33[k+z]
        }
        //zt1[i]=sum1;//0.01 hv
        //zt2[i]=sum2;//0.01 hv
        //zt3[i]=sum3;//0.01 hv
        //k+=32;*/
        if (i1>=0 && i1+32<NP2)
        {
            for (int z = 0; z < 32; ++z) zt1[i]+=cd0[i1+z+cd_off]*conj(csync2[z]);
        }
        if (i2>=0 && i2+32<NP2)
        {
            for (int z = 0; z < 32; ++z) zt2[i]+=cd0[i2+z+cd_off]*conj(csync2[z]);
        }
        if (i3>=0 && i3+32<NP2)
        {
            for (int z = 0; z < 32; ++z) zt3[i]+=cd0[i3+z+cd_off]*conj(csync2[z]);
        }
    }
    if (ipass==1 || ipass==5 || ipass==9)
    {
        if (!lastsync)
        {
            for (int i = 0; i <= 6; ++i)
            {//do i=0,6
                sync1[i]   = sqrt(pomAll.ps_hv(zt1[i]));//sync1(i) = SQRT(real(zt1(i))**2 + aimag(zt1(i))**2)
                sync1[i+7] = sqrt(pomAll.ps_hv(zt2[i]));//sync1(i+7) = SQRT(real(zt2(i))**2 + aimag(zt2(i))**2)
                sync1[i+14]= sqrt(pomAll.ps_hv(zt3[i]));//sync1(i+14) = SQRT(real(zt3(i))**2 + aimag(zt3(i))**2)
            }
            for (int z = 0; z < 21; ++z) sync+=sync1[z];
        }
        else
        {
            for (int i = 0; i <= 6; ++i)
            {//do i=0,6 //sync = sync + real(zt1(i))**2 + aimag(zt1(i))**2 + real(zt2(i))**2 + aimag(zt2(i))**2 + real(zt3(i))**2 + aimag(zt3(i))**2
                sync+=(pomAll.ps_hv(zt1[i])+pomAll.ps_hv(zt2[i])+pomAll.ps_hv(zt3[i]));
            }
        }
    }
    else if (ipass==2 || ipass==6 || ipass==7)
    {
        if (!lastsync)
        {
            for (int i = 0; i <= 6; ++i)
            {//do i=0,6
                sync1[i]   = pomAll.ps_hv(zt1[i]);//sync1(i) = (real(zt1(i))**2 + aimag(zt1(i))**2)
                sync1[i+7] = pomAll.ps_hv(zt2[i]);//sync1(i+7) = (real(zt2(i))**2 + aimag(zt2(i))**2)
                sync1[i+14]= pomAll.ps_hv(zt3[i]);//sync1(i+14) = (real(zt3(i))**2 + aimag(zt3(i))**2)
            }
            for (int z = 0; z < 21; ++z) sync+=sync1[z];
        }
        else
        {
            for (int i = 0; i <= 6; ++i)
            {//do i=0,6 //sync = sync + real(zt1(i))**2 + aimag(zt1(i))**2 + real(zt2(i))**2 + aimag(zt2(i))**2 + real(zt3(i))**2 + aimag(zt3(i))**2
                sync+=(pomAll.ps_hv(zt1[i])+pomAll.ps_hv(zt2[i])+pomAll.ps_hv(zt3[i]));
            }
        }
    }
    else //! if(ipass.eq.3 .or. ipass.eq.4 .or. ipass.eq.8)  then//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    {
        for (int i = 0; i <= 6; ++i)
        {//do i=0,6
            if (i<6)
            {
                z1=((zt1[i]+zt1[i+1])/2.0);
                z2=((zt2[i]+zt2[i+1])/2.0);
                z3=((zt3[i]+zt3[i+1])/2.0); //! use i=5 z values for i=6
            }
            //if(.not.lastsync) sync = sync + abs(real(z1)) + abs(aimag(z1)) + abs(real(z2)) + abs(aimag(z2)) + abs(real(z3)) + abs(aimag(z3))
            //else sync = sync + real(z1)**2 + aimag(z1)**2 + real(z2)**2 + aimag(z2)**2 + real(z3)**2 + aimag(z3)**2
            //z1*=zk;
            //z2*=zk;
            //z3*=zk;
            if (!lastsync) sync+=(fabs(creal(z1)) + fabs(cimag(z1)) + fabs(creal(z2)) + fabs(cimag(z2)) + fabs(creal(z3)) + fabs(cimag(z3)));
            else sync+=(pomAll.ps_hv(z1) + pomAll.ps_hv(z2) + pomAll.ps_hv(z3));
        }
    }
    if (itwk==1 && lcqcand && iqso==1)
    {
        for (int i = 0; i <= 7; ++i) zt4[i]=0.0+0.0*I;
        for (int i = 0; i <= 7; ++i)
        {//do i=0,7
            for (int z = 0; z < 32; ++z) csync2[z]=csynccq[i][z];//(i,1:32);
            for (int z = 0; z < 32; ++z) csync2[z]=ctwk[z]*csync2[z];//!Tweak the frequency
            int i4=i0+(i+7)*32;
            double complex sum4=0.0+0.0*I;
            for (int z = 0; z < 32; ++z) sum4+=(cd0[i4+z+cd_off]*conj(csync2[z]));
            zt4[i]=sum4;//*0.01;//0.01 hv
        }
        for (int i = 0; i <= 7; ++i)//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
        {//do i=0,7
            if (ipass==3 || ipass==4 || ipass==8)
            {
                if (i<7) z4=(zt4[i]+zt4[i+1])/2.0; //! use i=6 z4 value for i=7
            }
            else z4=zt4[i];
            //z4*=zk;
            if (ipass==1 || ipass==5 || ipass==9) sync += sqrt(pomAll.ps_hv(z4));
            else if (ipass==2 || ipass==6 || ipass==7) sync += pomAll.ps_hv(z4);
            else if (ipass==3 || ipass==4 || ipass==8) sync += fabs(creal(z4)) + fabs(cimag(z4));
        }
    }
    if (!lastsync)//csync1(0:18,32)
    {
        if (iqso>1 && iqso<4 && lcallsstd)
        {
            for (int i = 0; i < 19; ++i)
            {
                for (int z = 0; z < 32; ++z) csync1[i][z]=csynce[i][z];
            }
        }
        if (iqso==4 && !lcq)
        {
            for (int i = 0; i < 19; ++i)
            {
                for (int z = 0; z < 32; ++z) csync1[i][z]=csyncsd[i][z];
            }
        }
        if ((iqso==4 && !lcq) || ((iqso==2 || iqso==3) && lcallsstd))
        {
            for (int i = 0; i < 19; ++i) zt5[i]=0.0+0.0*I;
            for (int i = 0; i < 19; ++i)
            {//do i=0,18
                for (int z = 0; z < 32; ++z) csync2[z]=csync1[i][z];//(i,1:32)
                if (itwk==1) for (int z = 0; z < 32; ++z) csync2[z]=ctwk[z]*csync2[z]; //!Tweak the frequency
                int i4=i0+(i+7)*32;
                double complex sum5=0.0+0.0*I;
                for (int z = 0; z < 32; ++z) sum5+=(cd0[i4+z+cd_off]*conj(csync2[z]));//sum(cd0(i4:i4+31)*conjg(csync2))
                zt5[i]=sum5;//*0.01;//0.01 hv
            }
            for (int i = 0; i < 19; ++i)//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
            {//do i=0,18
                if (ipass==2 || ipass==6 || ipass==7)
                {
                    if (i<18) z4=(zt5[i]+zt5[i+1])/2.0; //! use i=17 z4 value for i=18
                }
                else z4=zt5[i];
                //z4*=zk;
                if (ipass==1 || ipass==5 || ipass==9) sync += sqrt(pomAll.ps_hv(z4));
                else if (ipass==2 || ipass==6 || ipass==7) sync += pomAll.ps_hv(z4);
                else if (ipass==3 || ipass==4 || ipass==8) sync += fabs(creal(z4)) + fabs(cimag(z4));
            }
        }
        if (iqso==4 && lcq)
        {
            int k;
            for (int i = 0; i < 58; ++i)
            {//do i=0,57
                for (int z = 0; z < 32; ++z) csync2[z]=csyncsdcq[i][z];//csync2=csyncsdcq(i,1:32)
                if (itwk==1) for (int z = 0; z < 32; ++z) csync2[z]=ctwk[z]*csync2[z];//!Tweak the frequency
                z4=0.0+0.0*I;
                if (i<29) k=i+7;
                else k=i+14;
                int i4=i0+k*32;
                for (int z = 0; z < 32; ++z) z4+=(cd0[i4+z+cd_off]*conj(csync2[z]));//z4=sum(cd0(i4:i4+31)*conjg(csync2))
                //z4*=zk;
                if (ipass==1 || ipass==5 || ipass==9) sync += sqrt(pomAll.ps_hv(z4));
                else if (ipass==2 || ipass==6 || ipass==7) sync += pomAll.ps_hv(z4);
                else if (ipass==3 || ipass==4 || ipass==8) sync += fabs(creal(z4)) + fabs(cimag(z4));
            }
        }
    } //double sync10=sync*0.01; if (sync10>100) qDebug()<<ipass<<sync10;
}
void DecoderFt8::twkfreq1var(double complex *ca,int npts,double fsample,double *a,double complex *cb)//,int cd_off
{
    double complex w=1.0+1.0*I;//w=1.0; wstep=1.0
    //!!  x0=0.5*(npts+1)
    //!!  s=2.0/npts
    for (int i =0; i<npts; ++i)
    {//do i=0,npts
        //!!     x=s*(i-x0) ! valid for 'do i=1,npts'
        //!!     p2=1.5*x*x - 0.5
        //!!     p3=2.5*(x**3) - 1.5*x
        //!!     p4=4.375*(x**4) - 3.75*(x**2) + 0.375
        //!!     dphi=(a(1) + x*a(2) + p2*a(3) + p3*a(4) + p4*a(5)) * (twopi/fsample)
        double dphi=a[0] * (twopi/fsample);
        double complex wstep=cos(dphi)+sin(dphi)*I;//wstep=cmplx(cos(dphi),sin(dphi))
        w=w*wstep;
        cb[i]=w*ca[i];//cb(i)=w*ca(i)
    }
}
void DecoderFt8::syncdist(double *s81,int *nsmax,int k)
{
    int ip=pomAll.maxloc_da_beg_to_end(s81,0,8);//maxloc(s81)
    if (icos7_2[k]==ip) nsmax[0]++; //if(icos7(k-1).eq.(ip(1)-1)) then
    else
    {
        s81[ip]=0.0;
        ip=pomAll.maxloc_da_beg_to_end(s81,0,8);//ip=maxloc(s81)
        if (icos7_2[k]==ip) nsmax[1]++;
        else
        {
            s81[ip]=0.0;
            ip=pomAll.maxloc_da_beg_to_end(s81,0,8);//ip=maxloc(s81)
            if (icos7_2[k]==ip) nsmax[2]++;
            else
            {
                s81[ip]=0.0;
                ip=pomAll.maxloc_da_beg_to_end(s81,0,8);//ip=maxloc(s81)
                if (icos7_2[k]==ip) nsmax[3]++;
                else
                {
                    s81[ip]=0.0;
                    ip=pomAll.maxloc_da_beg_to_end(s81,0,8);//ip=maxloc(s81)
                    if (icos7_2[k]==ip) nsmax[4]++;
                    else
                    {
                        s81[ip]=0.0;
                        ip=pomAll.maxloc_da_beg_to_end(s81,0,8);//ip=maxloc(s81)
                        if (icos7_2[k]==ip) nsmax[5]++;
                        else
                        {
                            s81[ip]=0.0;
                            ip=pomAll.maxloc_da_beg_to_end(s81,0,8);//ip=maxloc(s81)
                            if (icos7_2[k]==ip) nsmax[6]++;
                            else
                            {
                                s81[ip]=0.0;
                                ip=pomAll.maxloc_da_beg_to_end(s81,0,8);//ip=maxloc(s81)
                                if (icos7_2[k]==ip) nsmax[7]++;
                            }
                        }
                    }
                }
            }
        }
    }
}
void DecoderFt8::ft8sd1var(double s8_[79][8],int *itone,QString msgd,QString &msg37,bool &lft8sd,bool lcq)
{	//Tested OK HV
    int itonedem[58+10];//(58)
    int idtone[58+10];//(58)
    bool lmatched[58];
    bool lr73=false;
    bool lgrid=false;
    QString c1="";
    QString c2="";
    QString msg4[4];//(4)
    QString msg372="";
    //bool c77[100];
    int itone4[4][79];//(4,79)
    double s8_1_[79][8];//(0:7,79)
    int idtone4[4][58];//(4,58)
    for (int i = 0; i < 58; ++i)
    {
        itonedem[i]=11;
        lmatched[i]=false;
    }
    if (!lcq)
    {
        msgd.append("    ");
        int ispc1=msgd.indexOf(' ');
        int ispc2=msgd.indexOf(' ',ispc1+1);
        int ispc3=msgd.indexOf(' ',ispc2+1);
        if (ispc1<=12 && (ispc2-(ispc1+1))<=12)
        {
            c1=msgd.mid(0,ispc1);
            c2=msgd.mid(ispc1+1,ispc2-(ispc1+1));
            c1=c1.trimmed();
            c2=c2.trimmed();
        }
        else return;
        //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
        if (c1.count()<3 || c2.count()<3 || c2==s_MyCall8) return;
        if (msgd.indexOf(" RR73")>0 || msgd.indexOf(" 73")>0) lr73=true;
        if (!lr73 && (ispc3-ispc2)==5 && msgd.mid(ispc2+1,2)!="R+" && msgd.mid(ispc2+1,2)!="R-") lgrid=true;
        msgd=msgd.trimmed();
        if (!lgrid && !lr73)
        {
            for (int i = 0; i < 4; ++i)
            {//do i=1,4
                msg4[i]="                                     ";
                if (i==0) msg4[i]=msgd;
                if (i==1) msg4[i]=c1+" "+c2+" RRR";//trim(c1)//' '//trim(c2)//' RRR'
                if (i==2) msg4[i]=c1+" "+c2+" RR73";//trim(c1)//' '//trim(c2)//' RR73'
                if (i==3) msg4[i]=c1+" "+c2+" 73";//trim(c1)//' '//trim(c2)//' 73'
            }
        }
    }
    if (lcq || lgrid || lr73)
    {
        msg372=msgd;
        TGenFt8->pack77_make_c77_i4tone(msg372,itone);
        for (int i = 0; i < 29; ++i)
        {
            idtone[i]=itone[i+7];//idtone(1:29)=itone(8:36)
            idtone[i+29]=itone[i+43];//idtone(30:58)=itone(44:72)
        }
    }
    else
    {
        for (int i = 0; i < 4; ++i)
        {//do i=1,4
            msg372=msg4[i];
            TGenFt8->pack77_make_c77_i4tone(msg372,itone);
            for (int z = 0; z < 29; ++z)
            {
                idtone4[i][z]=itone[z+7];//idtone(1:29)=itone(8:36)
                idtone4[i][z+29]=itone[z+43];//idtone(30:58)=itone(44:72)
            }
            for (int z = 0; z < 79; ++z) itone4[i][z]=itone[z];//itone4(i,1:79)=itone(1:79)
        }
    }
    for (int i = 0; i < 79; ++i)//double s8_1[8][79];//(0:7,79)
    {
        for (int z = 0; z < 8; ++z) s8_1_[i][z]=s8_[i][z];
    }  //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    for (int i = 0; i < 58; ++i)
    {//do i=1,58
        if (i<=28)
        {
            int ip1=pomAll.maxloc_da_beg_to_end(s8_1_[i+7],0,8);//maxloc(s8_1(:,i+7))-1;
            itonedem[i]=ip1;
            s8_1_[i+7][ip1]=0.0;
        }
        else
        {
            int ip1=pomAll.maxloc_da_beg_to_end(s8_1_[i+14],0,8);//int ip1=maxloc(s8_1(:,i+14))-1;
            itonedem[i]=ip1;
            s8_1_[i+14][ip1]=0.0;//s8_1(ip1(1),i+14)=0.0;
        }
    }
    int imax=-1;
    int nmatch1=0;
    int ncrcpaty1=0;
    if (lcq || lgrid || lr73)
    {
        nmatch1=0;
        ncrcpaty1=0;
        for (int k = 0; k < 58; ++k)
        {//do k=1,58
            if (idtone[k]==itonedem[k])
            {
                nmatch1++;
                if (k>24) ncrcpaty1++;
            }
        } //qDebug()<<"="<<msgd<<lr73<<nmatch1<<ncrcpaty1;
        if (nmatch1>29 && ncrcpaty1>10)
        {
            lft8sd=true;
            msg37=msgd; //qDebug()<<"1 OUT ft8sd1var -->"<<msg37;
            return;
        }
    }
    else
    {
        int nmatchditer1=0;
        int ncrcpatyiter1=0;
        for (int i = 0; i < 4; ++i)
        {//do i=1,4
            nmatch1=0;
            ncrcpaty1=0;
            for (int k = 0; k < 58; ++k)
            {//do k=1,58
                if (idtone4[i][k]==itonedem[k])
                {
                    nmatch1++;
                    if (k>24) ncrcpaty1++;
                }
            }
            if (nmatch1>nmatchditer1)
            {
                imax=i;
                nmatchditer1=nmatch1;
                ncrcpatyiter1=ncrcpaty1;
            }
        }
        if (imax<=-1) return;
        if (lr73 && imax==1) return; //! RRR is not a valid message
        nmatch1=nmatchditer1;
        ncrcpaty1=ncrcpatyiter1; //qDebug()<<"100="<<nmatch1<<ncrcpaty1;
        for (int z = 0; z < 58; ++z) idtone[z]=idtone4[imax][z];//(imax,1:58)
        if (nmatch1>29 && ncrcpaty1>10)
        {
            lft8sd=true;
            msg37=msg4[imax]; //qDebug()<<"2 OUT ft8sd1var -->"<<msg37;
            for (int z = 0; z < 79; ++z) itone[z]=itone4[imax][z];
            return;
        }
    }
    if (nmatch1>=22)//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    {
        for (int k = 0; k < 58; ++k)
        {//do k=1,58
            if (idtone[k]==itonedem[k]) lmatched[k]=true;
        }
        for (int i = 0; i < 58; ++i)
        {//do i=1,58
            if (lmatched[i]) continue;
            if (i<=28)
            {
                int ip1=pomAll.maxloc_da_beg_to_end(s8_1_[i+7],0,8);//ip1=maxloc(s8_1(:,i+7))-1;
                itonedem[i]=ip1;
            }
            else
            {
                int ip1=pomAll.maxloc_da_beg_to_end(s8_1_[i+14],0,8);//int ip1=maxloc(s8_1(:,i+14))-1;
                itonedem[i]=ip1;
            }
        }
        int nmatch2=nmatch1;
        int ncrcpaty2=ncrcpaty1;
        if (lcq || lgrid || lr73)
        {
            for (int k = 0; k < 58; ++k)
            {//do k=1,58
                if (lmatched[k]) continue;
                if (idtone[k]==itonedem[k])//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
                {
                    nmatch2++;
                    if (k>24) ncrcpaty2++;
                }
            } //qDebug()<<"="<<msgd<<lr73<<nmatch2<<ncrcpaty2;
            if (nmatch2>41 && ncrcpaty2>19)
            {
                lft8sd=true;
                msg37=msgd; //qDebug()<<"3 OUT ft8sd1var -->"<<msg37;
                return;
            }
        }
        else
        {
            for (int k = 0; k < 58; ++k)
            {// do k=1,58
                if (lmatched[k]) continue;
                if (idtone[k]==itonedem[k])
                {
                    nmatch2++;
                    if (k>24) ncrcpaty2++;
                }
            }
            if (nmatch2>41 && ncrcpaty2>19)
            {
                lft8sd=true;
                msg37=msg4[imax]; //qDebug()<<"4 OUT ft8sd1var -->"<<msg37;
                for (int z = 0; z < 79; ++z) itone[z]=itone4[imax][z];//(imax,1:79);
                return;
            }
        }
    }
}
void DecoderFt8::ft8mf1var(double s8_[79][8],int *itone,QString msgd,QString &msg37,bool &lft8sd)
{	// Tested OK HV
    QString msggrid="";
    int mrs[58];
    int mrs2[58];
    double s8d_[58][8];
    bool lr73=false;
    bool lgrid=false;
    bool lreport=false;
    bool lrreport=false;
    bool lrrr=false;
    QString c1="";
    QString c2=""; //msgd = "LZ2HV SP9HWY RRR";
    msgd.append("             ");
    int ispc1=msgd.indexOf(' ');
    int ispc2=msgd.indexOf(' ',ispc1+1);
    int ispc3=msgd.indexOf(' ',ispc2+1);
    if (ispc1<=12 && (ispc2-(ispc1+1))<=12)
    {
        c1=msgd.mid(0,ispc1);
        c2=msgd.mid(ispc1+1,ispc2-(ispc1+1));
        c1=c1.trimmed();
        c2=c2.trimmed();
    }
    else return; //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    if (c1.count()<3 || c2.count()<3 || c2==s_MyCall8) return;
    if (msgd.indexOf(" RR73")>0 || msgd.indexOf(" 73")>0) lr73=true;
    if (!lr73 && (ispc3-ispc2)==5 && msgd.mid(ispc2+1,2)!="R+" && msgd.mid(ispc2+1,2)!="R-") lgrid=true;
    if (msgd.mid(ispc2+1,1)=="+" || msgd.mid(ispc2+1,1)=="-") lreport=true;
    if (msgd.mid(ispc2+1,2)=="R+" || msgd.mid(ispc2+1,2)=="R-") lrreport=true;
    if (msgd.mid(ispc2+1,ispc3-(ispc2+1))=="RRR") lrrr=true;
    //msgd=msgd.trimmed();
    //double thresh=0.0;
    int i1=0;
    int i2=0;
    for (int i = 0; i < 58; ++i)
    {//do i=1,58; //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
        if (i<=28)
        {
            for (int z = 0; z < 8; ++z) s8d_[i][z]=s8_[i+7][z];//(0:7,i+7);
        }
        else
        {
            for (int z = 0; z < 8; ++z) s8d_[i][z]=s8_[i+14][z];//s8d[][](0:7,i)=s8(0:7,i+14);
        }
    }
    for (int j = 0; j < 58; ++j)
    {//do j=1,58
        double s1=-1.0E6;
        double s2=-1.0E6;
        for (int i = 0; i < 8; ++i)
        {//do i=0,7;
            if (s8d_[j][i]>s1)
            {
                s1=s8d_[j][i];//(i,j);
                i1=i;
            }
        }
        for (int i = 0; i < 8; ++i)
        {//do i=0,7;
            if (i!=i1 && s8d_[j][i]>s2)
            {
                s2=s8d_[j][i];
                i2=i;
            }
        }
        mrs[j]=i1;
        mrs2[j]=i2;
    }
    double ref0=0.0;//do i=1,58;
    for (int i = 0; i < 58; ++i) ref0+=s8d_[i][mrs[i]];
    int ipk=-1;
    double u1=0.0;
    double u2=0.0; //qDebug()<<"ft8mf1var"<<ref0;
    for (int k = 0; k < 76; ++k)//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    {//do k=1,76
        double psum=0.0;
        double ref=ref0;
        for (int j = 0; j < 58; ++j)
        {//do j=1,58;
            int i3=idtone76[k][j];//(k,j); idtone76[76][58];//(76,58)
            psum+=s8d_[j][i3];//(i3,j);
            if (i3==mrs[j]) ref+=(s8d_[j][mrs2[j]] - s8d_[j][i3]);
        }
        double p=psum/ref;
        if (p>u1)
        {
            u2=u1;
            u1=p;
            ipk=k;
        }
        else
        {
            if (p>u2) u2=p;
        }
    }
    /*!/'+09 ','+08 ','+07 ','+06 ','+05 ','+04 ','+03 ','+02 ','+01 ','+00 ', &
    ! '-01 ','-02 ','-03 ','-04 ','-05 ','-06 ','-07 ','-08 ','-09 ','-10 ', &
    ! '-11 ','-12 ','-13 ','-14 ','-15 ','-16 ','-17 ','-18 ','-19 ','-20 ', &
    ! '-21 ','-22 ','-23 ','-24 ','-25 ','-26 ','R+09','R+08','R+07','R+06', &
    ! 'R+05','R+04','R+03','R+02','R+01','R+00','R-01','R-02','R-03','R-04', &
    ! 'R-05','R-06','R-07','R-08','R-09','R-10','R-11','R-12','R-13','R-14', &
    ! 'R-15','R-16','R-17','R-18','R-19','R-20','R-21','R-22','R-23','R-24', &
    ! 'R-25','R-26','RRR ','RR73','73  ','AA00'/*/
    if (ipk>=0)
    {
        if (lgrid)
        {
            if (ipk==75)//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
            {
                msggrid=msgsd76[75]+"           ";//LZ2HV SP9HWY KN23
                ispc1=msggrid.indexOf(' ');
                ispc2=msggrid.indexOf(' ',(ispc1+1));
                ispc3=msggrid.indexOf(' ',(ispc2+1));
                QString grid=msggrid.mid(ispc2+1,ispc3-(ispc2+1));
                grid=grid.trimmed();
                if (grid=="AA00") return;
            }
            if (ipk<36 || (ipk>71 && ipk<75)) return;
        }
        if (lreport)
        {
            if ((ipk>35 && ipk<72) || ipk==75) return;
        }
        if (lrreport)
        {
            if (ipk<36 || ipk==72 || ipk==75) return;
        }
        if (lrrr)
        {
            if (ipk<72 || ipk==75) return;
        }
        if (lr73)
        {
            if (ipk<73 || ipk==75) return;
        }

        double qual=100.0*(u1-u2);//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
        double thresh=(qual+10.0)*(u1-0.6); //qDebug()<<"ft8mf1var"<<lft8sd<<lreport<<ipk<<"if="<<thresh<<qual<<u1;
        if (thresh>4.0 && qual>2.6 && u1>0.77)
        {
            //!print *,'matched',thresh
            //!print *,msgsd76(ipk)
            //!      if(trim(msgsd76(ipk)).eq.'CQ DE AA00') return ! triggered by memory corruption?
            lft8sd=true;
            msg37=msgsd76[ipk]; //qDebug()<<"ft8mf1var---------->"<<msg37;
            for (int z = 0; z < 79; ++z) itone[z]=itone76[ipk][z];
            return;
        }
    }
}
void DecoderFt8::ft8mfcqvar(double s8_[79][8],int *itone,QString msgd,QString &msg37,bool &lft8sd)
{
    //bool c77[100];	//OK Tested HV
    int mrs[58];
    int mrs2[58];
    double s8d_[58][8];
    msgd=msgd.trimmed();
    if (msgd.count()<6) return;
    TGenFt8->pack77_make_c77_i4tone(msgd,itone);
    for (int i = 0; i < 29; ++i)
    {
        idtone25[0][i]=itone[i+7];//(8:36)
        idtone25[0][i+29]=itone[i+43];//(44:72)
    }
    //double thresh=0.0; //qDebug()<<"IN ft8mfcqvar <--"<<msgd;
    int i1=0;
    int i2=0;
    for (int i = 0; i < 58; ++i)
    {//do i=1,58; //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
        if (i<=28)
        {
            for (int z = 0; z < 8; ++z) s8d_[i][z]=s8_[i+7][z];//(0:7,i+7);
        }
        else
        {
            for (int z = 0; z < 8; ++z) s8d_[i][z]=s8_[i+14][z];//s8d[][](0:7,i)=s8(0:7,i+14);
        }
    }
    for (int j = 0; j < 58; ++j)
    {//do j=1,58
        double s1=-1.0E6;
        double s2=-1.0E6;
        for (int i = 0; i < 8; ++i)
        {//do i=0,7;
            if (s8d_[j][i]>s1)
            {
                s1=s8d_[j][i];//(i,j);
                i1=i;
            }
        }
        for (int i = 0; i < 8; ++i)
        {//do i=0,7;
            if (i!=i1 && s8d_[j][i]>s2)
            {
                s2=s8d_[j][i];
                i2=i;
            }
        }
        mrs[j]=i1;
        mrs2[j]=i2;
    }
    double ref0=0.0;//do i=1,58;
    for (int i = 0; i < 58; ++i) ref0+=s8d_[i][mrs[i]];
    int ipk=-1;
    double u1=0.0;
    double u2=0.0;
    for (int k = 0; k < 25; ++k)//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    {//do k=1,76
        double psum=0.0;
        double ref=ref0;
        for (int j = 0; j < 58; ++j)
        {//do j=1,58;
            int i3a=idtone25[k][j];//(k,j); idtone76[76][58];//(76,58)
            psum+=s8d_[j][i3a];//(i3,j);
            if (i3a==mrs[j]) ref+=(s8d_[j][mrs2[j]] - s8d_[j][i3a]);
        }
        double p=psum/ref;
        if (p>u1)
        {
            u2=u1;
            u1=p;
            ipk=k;
        }
        else
        {
            if (p>u2) u2=p;
        }
    }
    if (ipk>=0)
    {
        double qual=100.0*(u1-u2);//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
        double thresh=(qual+10.0)*(u1-0.6); //qDebug()<<"ft8mfcqvar"<<thresh<<qual<<u1;
        if (thresh>4.0 && qual>2.6 && u1>0.77)
        {
            lft8sd=true;
            msg37=msgd; //qDebug()<<"OUT ft8mfcqvar -->"<<msg37;
            return;
        }
    }
}
void DecoderFt8::ft8svar(double s8_[79][8],double srr,int *itone,QString &msg37,bool &lft8s,int nft8rxfsens,bool stophint)
{
    //OK Tested HV
    double xmnoise[79];//(79)
    double xmdata[58];//(58)
    double xdata[58];//(58)
    double xmsync[21];//(21)
    double xsync[21];//(21)
    double xnoise[79];//(79)
    double s8_1_[79][8];//(0:7,79)
    double s8d_[58][8];//(0:7,58)
    int itonedem[58];
    int idtone[58];//(58),
    int mrs[58];//(58)
    int mrs2[58];//(58)
    int mrs3[58];//(58)
    int mrs4[58];//(58)
    lft8s=false;
    if (stophint) return;
    if (s_HisCall8.count()<3) return;
    if (nlasttx==6 || nlasttx==0) return; //! CQ or TX halted
    bool lgrid=false;
    int nft8rxfslow=nft8rxfsens; //qDebug()<<"1000 IN ft8svar<--";
    bool lmycall=false;
    QString lastrcvdmsg="";
    bool lmatched[58];//(58);
    for (int i = 0; i < 79; ++i)
    {
        for (int j = 0; j < 8; ++j) s8_1_[i][j]=s8_[i][j];
    }
    bool lhiscall=false;
    bool lrrr=false;
    bool lr73=false;
    bool lcallingrprt=false;
    bool lastrrprt=false;
    bool lastreport=false;
    for (int i = 0; i < 58; ++i)
    {
        itonedem[i]=11;
        lmatched[i]=false;
    }
    int ntresh1=26;
    int ntresh2=38;
    if (srr>7.0)//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    {
        ntresh1=29;
        ntresh2=41;
    }
    if (lastrxmsg[0].lstate) lastrcvdmsg=lastrxmsg[0].lastmsg;
    else lastrcvdmsg="";
    if (!lastrcvdmsg.isEmpty())
    {
        if (lastrcvdmsg.indexOf(s_HisCall8)>-1) lhiscall=true;
        if (lastrcvdmsg.indexOf(s_MyCall8)>-1) lmycall=true;
        if (lmycall && lhiscall)
        {
            if (lastrcvdmsg.indexOf(" RRR")>-1) lrrr=true;
            if (lastrcvdmsg.indexOf(" RR73")>-1 || lastrcvdmsg.indexOf(" 73")>-1) lr73=true;
        }
    }
    else
    {
        if (nlasttx==2) lcallingrprt=true; //! calling with REPORT scenario
    }
    if (lmycall && lhiscall)
    {
        if (lastrcvdmsg.indexOf(" +")>-1 || lastrcvdmsg.indexOf(" -")>-1) lastreport=true;
        if (lastrcvdmsg.indexOf(" R+")>-1 || lastrcvdmsg.indexOf(" R-")>-1) lastrrprt=true;
    }
    if (lastrxmsg[0].lstate && lmycall && lhiscall && !lastreport && !lastrrprt && !lrrr && !lr73) lgrid=true;
    if (lastrxmsg[0].lstate && lmycall && lhiscall && nlasttx==2) lcallingrprt=true; //! calling with REPORT
    if (lgrid && msg_[52].indexOf(s_MyCall8+" "+s_HisCall8+" AA00")==0)
    {//! standard callsigns
        msg_[52]=lastrxmsg[0].lastmsg;
        TGenFt8->pack77_make_c77_i4tone(msg_[52],itone);
        for (int i = 0; i < 29; ++i)// int idtone56[56][58];//(56,58), int itone56[56][79];//(56,79)
        {
            idtone56[52][i]=itone[i+7];//(8:36);//idtone56(53,1:29)=itone(8:36)
            idtone56[52][i+29]=itone[i+43];//idtone56(53,30:58)=itone(44:72)
        }
        for (int i = 0; i < 79; ++i) itone56[52][i]=itone[i];//(1:79)
    }
    for (int i = 0; i < 58; ++i)//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    {//do i=1,58
        if (i<=28)
        {
            int ip1=pomAll.maxloc_da_beg_to_end(s8_1_[i+7],0,8);    	//ip1=maxloc(s8_1(:,i+7))-1;
            itonedem[i]=ip1;
            s8_1_[i+7][ip1]=0.0;
        }
        else
        {
            int ip1=pomAll.maxloc_da_beg_to_end(s8_1_[i+14],0,8);    	//ip1=maxloc(s8_1(:,i+14))-1;
            itonedem[i]=ip1;
            s8_1_[i+14][ip1]=0.0;
        }
    }
    int nmycall1=0;
    int nbase1=0;
    for (int k = 0; k < 19; ++k)
    {//do k=1,19
        if (idtone56[0][k]==itonedem[k])
        {
            if (k<9) nmycall1++;
            nbase1++;
        }
    }
    /*! /'-01 ','-02 ','-03 ','-04 ','-05 ','-06 ','-07 ','-08 ','-09 ','-10 ',
    !  '-11 ','-12 ','-13 ','-14 ','-15 ','-16 ','-17 ','-18 ','-19 ','-20 ',
    !  '-21 ','-22 ','-23 ','-24 ','-25 ','-26 ','R-01','R-02','R-03','R-04',
    !  'R-05','R-06','R-07','R-08','R-09','R-10','R-11','R-12','R-13','R-14',
    !  'R-15','R-16','R-17','R-18','R-19','R-20','R-21','R-22','R-23','R-24',
    !  'R-25','R-26','AA00','RRR ','RR73','73  '/*/
    int ilow=0;
    int ihigh=55;
    if (nlasttx==1)
    {
        ilow=0;
        ihigh=25; //! GRID R+REPORT RRR RR73 73 are not valid msgs
    }
    else if (lgrid)
    {
        ilow=26;
        ihigh=52; //! REPORT RRR RR73 73 are not valid msgs
    }
    else if (lcallingrprt)
    {
        ilow=26;
        ihigh=51; //! GRID REPORT RRR RR73 73 are not valid msgs
    }
    else if (lastrrprt)
    {
        ilow=26;
        ihigh=55; //! REPORT is not a valid msg, GRID shall be excluded later
    }
    else if (lr73 || lrrr)
    {
        ilow=53;
        ihigh=55; //! GRID, REPORT or R+REPORT is not a valid msg,
    }
    //qDebug()<<"1002 IN ft8svar<--"<<ilow<<ihigh;
    //qDebug()<<"X="<<s8_[4][5]<<s8_[50][3]<<s8_[60][2]<<s8_[30][7]<<s8_[33][2];
    //! MF GRID shall be excluded later
    int nmatchditer1=0;
    int ncrcpatyiter1=0;
    int nmatch1=0;
    int ncrcpaty1=0;
    int imax=-1;
    for (int i = ilow; i <= ihigh; ++i)//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    {//do i=ilow,ihigh
        if (lastreport && i>25 && i<53) continue; //! RREPORT is not a valid msg, direction change is not allowed
        nmatch1=0;
        ncrcpaty1=0;
        for (int k = 0; k < 58; ++k)
        {//do k=1,58
            if (idtone56[i][k]==itonedem[k])
            {
                nmatch1++;
                if (k>24) ncrcpaty1++;
            }
        }
        if (nmatch1>nmatchditer1)
        {
            imax=i;
            nmatchditer1=nmatch1;
            ncrcpatyiter1=ncrcpaty1;
        }
    }
    //!write (*,"(i2,1x,i2,1x,i2,1x,i2)") nmatchlast1,nmatchditer1,ncrcpatylast1,ncrcpatyiter1
    //qDebug()<<"1003 IN ft8svar<--"<<imax<<nmatchditer1<<ncrcpatyiter1;
    if (imax<=-1) return;
    if (srr>3.0 && (nmycall1<5 || nbase1<10)) return;// ! prevent false decodes from the strong signals
    if ((nlasttx==1 || lcallingrprt) && nmycall1==0) return;// ! answer to other operator
    if (lr73 && (imax==52 || imax==53)) return;// ! RRR,GRID is not a valid msg
    //qDebug()<<"1004 IN ft8svar<--"<<imax;
    nmatch1=nmatchditer1;
    ncrcpaty1=ncrcpatyiter1;
    for (int k = 0; k < 58; ++k) idtone[k]=idtone56[imax][k];//(imax,1:58)
    for (int k = 0; k < 58; ++k)
    {//do k=1,58
        if (idtone[k]==itonedem[k]) lmatched[k]=true;
    }
    if (ncrcpaty1<8) return;
    double thresh=0.0;
    int i1=0;
    int i2=0;
    int i33=0;
    int i4=0;
    for (int i = 0; i < 58; ++i)
    {//do i=1,58;
        if (i<=28)
        {
            for (int z = 0; z < 8; ++z) s8d_[i][z]=s8_[i+7][z];//(0:7,i+7);
        }
        else
        {
            for (int z = 0; z < 8; ++z) s8d_[i][z]=s8_[i+14][z];//(0:7,i+14);
        }
    }
    for (int j = 0; j < 58; ++j)//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    {//do j=1,58
        double s1=-1.0E6;
        double s2=-1.0E6;
        double s3=-1.0E6;
        double s4=-1.0E6;
        for (int i = 0; i < 8; ++i)
        {//do i=0,7;
            if (s8d_[j][i]>s1)
            {
                s1=s8d_[j][i];
                i1=i;
            }
        }
        for (int i = 0; i < 8; ++i)
        {//do i=0,7;
            if (i!=i1 && s8d_[j][i]>s2)
            {
                s2=s8d_[j][i];
                i2=i;
            }
        }
        for (int i = 0; i < 8; ++i)
        {//do i=0,7;
            if (i!=i1 && i!=i2 && s8d_[j][i]>s3)
            {
                s3=s8d_[j][i];
                i33=i;
            }
        }
        for (int i = 0; i < 8; ++i)
        {//do i=0,7;
            if (i!=i1 && i!=i2 && i!=i33 && s8d_[j][i]>s4)
            {
                s4=s8d_[j][i];
                i4=i;
            }
        }
        mrs[j]=i1;
        mrs2[j]=i2;
        mrs3[j]=i33;
        mrs4[j]=i4;
    }
    double ref0=0.0;
    double ref0paty=0.0;
    double ref0mycl=0.0;
    double ref0oth=0.0;
    for (int i = 0; i < 58; ++i)//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    {//do i=1,58
        ref0+=s8d_[i][mrs[i]];
        if (i>25)ref0paty+=s8d_[i][mrs[i]];
        else if (i<9) ref0mycl+=s8d_[i][mrs[i]];
    }
    ref0oth=ref0mycl+ref0paty;
    int ipk=-1;
    double u1=0.0;
    double u2=0.0;
    double u1paty=0.0;
    double u2paty=0.0;
    double u1oth=0.0;
    double u2oth=0.0;
    for (int k=ilow; k<=ihigh; ++k)
    {//do k=ilow,ihigh
        if (lastreport && k>25 && k<53) continue;// ! GRID, RREPORT is not a valid msg, direction change is not allowed
        double psum=0.0;
        double ref=ref0;
        double psumpaty=0.0;
        double refpaty=ref0paty;
        double psumoth=0.0;
        double refoth=ref0oth;
        for (int j = 0; j < 58; ++j)
        {//do j=1,58;
            int i=idtone56[k][j];
            psum+=s8d_[j][i];
            if (j>25)
            {
                psumpaty+=s8d_[j][i];
                psumoth+=s8d_[j][i];
            }
            else if (j<9) psumoth+=s8d_[j][i];
            if (i==mrs[j])//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
            {
                double stmp=(s8d_[j][mrs2[j]] - s8d_[j][i]);//stmp=s8d(mrs2(j),j)-s8d(i,j); ref=ref + stmp
                ref+=stmp;
                if (j>25)
                {
                    refpaty+=stmp;
                    refoth+=stmp;
                }
                else if (j<9) refoth+=stmp;
            }
            if (i==mrs2[j])
            {
                double stmp=(s8d_[j][mrs3[j]] - s8d_[j][mrs2[j]]); //stmp=s8d(mrs3(j),j) - s8d(mrs2(j),j); ref=ref + stmp
                ref+=stmp;
                if (j>25)
                {
                    refpaty+=stmp;
                    refoth+=stmp;
                }
                else if (j<9) refoth+=stmp;
            }
            if (i==mrs3[j])
            {
                double stmp=(s8d_[j][mrs4[j]] - s8d_[j][mrs3[j]]);//stmp=s8d(mrs4(j),j) - s8d(mrs3(j),j);
                ref+=stmp;
                if (j>25)
                {
                    refpaty+=stmp;
                    refoth+=stmp;
                }
                else if (j<9) refoth+=stmp;
            }
            //!!      if(i.eq.mrs2(j)) then; ref=ref - s8d(mrs(j),j) + s8d(mrs3(j),j); endif
            //!!      if(i.eq.mrs3(j)) then; ref=ref - s8d(mrs(j),j) + s8d(mrs4(j),j); endif*/
        }
        if (ref==0.0) ref=0.000001;
        double p=psum/ref;
        if (refpaty==0.0) refpaty=0.000001;
        double ppaty=psumpaty/refpaty;
        if (refoth==0.0) refoth=0.000001;
        double poth=psumoth/refoth;//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
        if (p>u1)
        {
            u2=u1;
            u1=p;
            u2paty=u1paty;
            u1paty=ppaty;
            u2oth=u1oth;
            u1oth=poth;
            ipk=k;
        }
        else if (p>u2)
        {
            u2=p;
            u2paty=ppaty;
            u2oth=poth;
        }
    }
    if (lastrrprt && ipk==52) return;// ! GRID is not valid
    if (lr73 && ipk<54) return;
    if (lcallingrprt || nlasttx==1) nft8rxfslow=1;
    if (ipk>=0)
    {
        double qual=100.0*(u1-u2);
        double qualp=100.0*(u1paty-u2paty);
        double qualo=100.0*(u1oth-u2oth);
        thresh=(qual+10.0)*(u1-0.6); //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
        if (thresh<1.5) return;
        double threshp=(qualp+10.0)*(u1paty-0.6);
        double thresho=(qualo+10.0)*(u1oth-0.6);
        if ((lcallingrprt || nlasttx==1) && thresho<3.43) return;
        if (thresho<2.63 || threshp<2.45) return;
        if (((nft8rxfslow==1 && thresh>4.0) || (nft8rxfslow==2 && thresh>3.55) ||
                (nft8rxfslow==3 && thresh>3.0)) && qual>2.6 && u1>0.77)
        {
            //!if(ipk.ne.7) print *,'match1',thresh
            lft8s=true;
            msg37=msg_[ipk];
            for (int z = 0; z < 79; ++z) itone[z]=itone56[ipk][z];
            goto c2;
        }
    }
    if (imax==ipk && (nft8rxfslow>1 || (nft8rxfslow==1 && thresh>2.7)) && srr<7.0 &&
            (ncrcpaty1>14 || (nmatch1>22 && ncrcpaty1>13)))
    {
        //!if(imax.ne.7) print *,'11',thresh
        lft8s=true;
        msg37=msg_[imax];
        for (int z = 0; z < 79; ++z) itone[z]=itone56[imax][z];//itone(1:79)=itone56(imax,1:79);
        goto c2;
    }
    if (imax==ipk && nmatch1>ntresh1 && ncrcpaty1>10)
    {
        //!if(imax.ne.7) print *,'12',thresh
        lft8s=true;
        msg37=msg_[imax];
        for (int z = 0; z < 79; ++z) itone[z]=itone56[imax][z];//itone(1:79)=itone56(imax,1:79);
        goto c2;
    }
    if (nmatchditer1>=16)//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    {
        for (int i = 0; i < 58; ++i)
        {//do i=1,58
            if (lmatched[i]) continue;
            if (i<=28)
            {
                int ip1=pomAll.maxloc_da_beg_to_end(s8_1_[i+7],0,8);    	//ip1=maxloc(s8_1(:,i+7))-1;
                itonedem[i]=ip1;
                s8_1_[i+7][ip1]=0.0;
            }
            else
            {
                int ip1=pomAll.maxloc_da_beg_to_end(s8_1_[i+14],0,8);
                itonedem[i]=ip1;
                s8_1_[i+14][ip1]=0.0;
            }
        }
        int nmatch2=nmatch1;
        int ncrcpaty2=ncrcpaty1;
        for (int k = 0; k < 58; ++k)
        {//do k=1,58
            if (lmatched[k]) continue;
            if (idtone[k]==itonedem[k])
            {
                nmatch2++;
                lmatched[k]=true;
                if (k>24) ncrcpaty2++;
            }
        }
        if (nmatch2>ntresh2 && ncrcpaty2>19)
        {
            //!if(imax.ne.7) print *,'2',thresh
            lft8s=true;
            msg37=msg_[imax];
            for (int z = 0; z < 79; ++z) itone[z]=itone56[imax][z];//itone(1:79)=itone56(imax,1:79);
            goto c2;
        }
        if (srr>7.0) return;// ! do not process strong signals anymore
        for (int i = 0; i < 58; ++i)//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
        {//do i=1,58
            if (lmatched[i]) continue;
            if (i<=28)
            {
                int ip1=pomAll.maxloc_da_beg_to_end(s8_1_[i+7],0,8);    	//ip1=maxloc(s8_1(:,i+7))-1;
                itonedem[i]=ip1;
                s8_1_[i+7][ip1]=0.0;
            }
            else
            {
                int ip1=pomAll.maxloc_da_beg_to_end(s8_1_[i+14],0,8);
                itonedem[i]=ip1;
                s8_1_[i+14][ip1]=0.0;
            }
        }
        int nmatch3=nmatch2;
        int ncrcpaty3=ncrcpaty2;
        for (int k = 0; k < 58; ++k)
        {//dodo k=1,58
            if (lmatched[k]) continue;
            if (idtone[k]==itonedem[k])
            {
                nmatch3++;
                lmatched[k]=true;
                if (k>24) ncrcpaty3++;
            }
        }
        if (nmatch3>44 && ncrcpaty3>21)
        {
            //!if(imax.ne.7) print *,'3',thresh
            lft8s=true;
            msg37=msg_[imax];
            for (int z = 0; z < 79; ++z) itone[z]=itone56[imax][z];//itone(1:79)=itone56(imax,1:79);
            goto c2;
        }
        for (int i = 0; i < 58; ++i)//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
        {//do i=1,58
            if (lmatched[i]) continue;
            if (i<=28)
            {
                int ip1=pomAll.maxloc_da_beg_to_end(s8_1_[i+7],0,8);    	//ip1=maxloc(s8_1(:,i+7))-1;
                itonedem[i]=ip1;
                s8_1_[i+7][ip1]=0.0;
            }
            else
            {
                int ip1=pomAll.maxloc_da_beg_to_end(s8_1_[i+14],0,8);
                itonedem[i]=ip1;
                s8_1_[i+14][ip1]=0.0;
            }
        }
        int nmatch4=nmatch3;
        int ncrcpaty4=ncrcpaty3;
        for (int k = 0; k < 58; ++k)
        {//dodo k=1,58
            if (lmatched[k]) continue;
            if (idtone[k]==itonedem[k])
            {
                nmatch4++;
                lmatched[k]=true;
                if (k>24) ncrcpaty4++;
            }
        }
        if ((nft8rxfslow==3 || (nft8rxfslow==2 && thresh>2.2)) && nmatch4>47 && ncrcpaty4>23)
        {
            //!if(imax.ne.7) print *,'3',thresh
            lft8s=true;
            msg37=msg_[imax];
            for (int z = 0; z < 79; ++z) itone[z]=itone56[imax][z];//itone(1:79)=itone56(imax,1:79);
            goto c2;
        }
        for (int i = 0; i < 58; ++i)//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
        {//do i=1,58
            if (lmatched[i]) continue;
            if (i<=28)
            {
                int ip1=pomAll.maxloc_da_beg_to_end(s8_1_[i+7],0,8);    	//ip1=maxloc(s8_1(:,i+7))-1;
                itonedem[i]=ip1;
                s8_1_[i+7][ip1]=0.0;
            }
            else
            {
                int ip1=pomAll.maxloc_da_beg_to_end(s8_1_[i+14],0,8);
                itonedem[i]=ip1;
                s8_1_[i+14][ip1]=0.0;
            }
        }
        int nmatch5=nmatch4;
        int ncrcpaty5=ncrcpaty4;
        for (int k = 0; k < 58; ++k)
        {//dodo k=1,58
            if (lmatched[k]) continue;
            if (idtone[k]==itonedem[k])
            {
                nmatch5++;
                lmatched[k]=true;
                if (k>24) ncrcpaty5++;
            }
        }
        if ((nft8rxfslow==3 || (nft8rxfslow==1 && thresh>3.4) || (nft8rxfslow==2 && thresh>3.25)) &&
                nmatch5>50 && (nmatch1>21 || nmatch2>31 || nmatch3>38 || nmatch4>46) &&
                ncrcpaty5>25)
        {
            //!if(imax.ne.7) print *,'3',thresh
            lft8s=true;
            msg37=msg_[imax];
            for (int z = 0; z < 79; ++z) itone[z]=itone56[imax][z];//itone(1:79)=itone56(imax,1:79);
            goto c2;
        }
        for (int i = 0; i < 58; ++i)//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
        {//do i=1,58
            if (lmatched[i]) continue;
            if (i<=28)
            {
                int ip1=pomAll.maxloc_da_beg_to_end(s8_1_[i+7],0,8);    	//ip1=maxloc(s8_1(:,i+7))-1;
                itonedem[i]=ip1;
                s8_1_[i+7][ip1]=0.0;
            }
            else
            {
                int ip1=pomAll.maxloc_da_beg_to_end(s8_1_[i+14],0,8);
                itonedem[i]=ip1;
                s8_1_[i+14][ip1]=0.0;
            }
        }
        int nmatch6=nmatch5;
        int ncrcpaty6=ncrcpaty5;
        for (int k = 0; k < 58; ++k)//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
        {//dodo k=1,58
            if (lmatched[k]) continue;
            if (idtone[k]==itonedem[k])
            {
                nmatch6++;
                lmatched[k]=true;
                if (k>24) ncrcpaty6++;
            }
        }
        if (((nft8rxfslow==2 && thresh>2.6) || (nft8rxfslow==3 && thresh>2.22)) &&
                imax==ipk && ncrcpaty6>26)
        {
            //!if(imax.ne.7) print *,'3',thresh
            lft8s=true;
            msg37=msg_[imax];
            for (int z = 0; z < 79; ++z) itone[z]=itone56[imax][z];//itone(1:79)=itone56(imax,1:79);
            goto c2;
        }
        if (nft8rxfslow==1)
        {
            if (nmatch6>54 && (nmatch1>22 || nmatch2>27 || nmatch3>35 ||
                               (nmatch2-nmatch1)>9 || (nmatch3-nmatch2)>10) && ncrcpaty6>29 && thresh>3.15)
            {
                //!if(imax.ne.7) print *,'62',thresh
                //!print *,'62',thresh
                lft8s=true;
                msg37=msg_[imax];
                for (int z = 0; z < 79; ++z) itone[z]=itone56[imax][z];//itone(1:79)=itone56(imax,1:79);
                goto c2;
            }
        }
        if (ncrcpaty6>29)
        {
            if ((nft8rxfslow==3 /*|| (nft8rxfslow==3 && thresh>1.94)*/) && imax==ipk && ncrcpaty2-ncrcpaty1>3 &&
                    ncrcpaty3-ncrcpaty2>3 && ncrcpaty4-ncrcpaty3>3 && ncrcpaty6-ncrcpaty5<6)
            {
                //!if(imax.ne.7) print *,'63',thresh
                //!print *,'63',thresh
                //!if(imax.ne.7) write (*,"(i2,1x,f5.2,1x,f5.2,1x,f5.2)") 62,u1,qual,thresh
                lft8s=true;
                msg37=msg_[imax];
                for (int z = 0; z < 79; ++z) itone[z]=itone56[imax][z];//itone(1:79)=itone56(imax,1:79);
                goto c2;
            }
        }
    }
c2:
    if (lft8s)
    {
        double snr=0.0;
        double snrbase=0.0;
        double snrpaty=0.0;
        double snrdata=0.0;
        double snrsync=0.0;
        double snrmycall=0.0;
        double snrother=0.0;
        for (int i = 0; i < 79; ++i)//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
        {//do i=1,79
            double xsig=s8_[i][itone[i]];
            double sums8=0.0;
            for (int z = 0; z < 8; ++z) sums8+=s8_[i][z];
            double xnoi=(sums8 - xsig)/7.0;
            double del0 = (xnoi+1E-6);
            if (del0==0.0) del0=0.000001;
            double snr1=xsig/del0;
            snr+=snr1;
            if (i>6 && i<33) snrbase+=snr1;
            if (i<16) snrmycall+=snr1;
            if ((i>42 && i<72) || (i>32 && i<36)) snrpaty+=snr1;
        }
        snrdata=snrbase+snrpaty;
        snrsync=snr-snrdata;
        snrother=snrmycall+snrpaty;
        snrsync=snrsync/21.0;
        snrother=snrother/48.0;
        snrpaty=snrpaty/32.0;
        if (lcallingrprt || nlasttx==1)
        {
            double del0 = snrother;
            if (del0==0.0) del0=0.000001;
            double soratio=snrsync/del0;
            if (soratio>1.29)
            {
                lft8s=false;
                msg37="";
                return;
            }
        }
        double del0 = snrpaty;
        if (del0==0.0) del0=0.000001;
        double spratio=snrsync/del0;
        if (spratio<0.6 || spratio>1.25)
        {
            lft8s=false;
            msg37="";
            return;
        }
        for (int i = 0; i < 7; ++i) xsync[i]=s8_[i][itone[i]];//do i=1,7;
        for (int i = 7; i < 14; ++i)//double xsync[21];
        {//do i=8,14;
            int k=i+29;
            xsync[i]=s8_[k][itone[k]];
        }
        for (int i = 14; i < 21; ++i)
        {//do i=15,21;
            int k=i+58;
            xsync[i]=s8_[k][itone[k]];
        }
        for (int i = 0; i < 29; ++i)//do i=1,29; k=i+7; xdata(i)=s8(itone(k),k); enddo;
        {//do i=1,29;
            int k=i+7;
            xdata[i]=s8_[k][itone[k]];
        }
        for (int i = 29; i < 58; ++i)// do i=30,58; k=i+14; xdata(i)=s8(itone(k),k); enddo
        {//do i=30,58;
            int k=i+14;
            xdata[i]=s8_[k][itone[k]];
        }
        for (int i = 0; i < 79; ++i)//do i=1,79; k=modulo(itone(i)+4,8); xnoise(i)=s8(k,i); enddo
        {//do i=1,79;
            int k=((itone[i]+4) % 8);
            xnoise[i]=s8_[i][k];//double xnoise[79];//(79)double xmsync[21];//(21)
        }
        for (int i = 0; i < 19; ++i)
        {//do i=1,19
            if ((xsync[i]>xsync[i+1] && xsync[i]<xsync[i+2])
                    || (xsync[i]<xsync[i+1] && xsync[i]>xsync[i+2])) xmsync[i]=xsync[i];
            else if ((xsync[i+1]>xsync[i] && xsync[i+1]<xsync[i+2])
                     || (xsync[i+1]<xsync[i] && xsync[i+1]>xsync[i+2])) xmsync[i]=xsync[i+1];
            else if ((xsync[i+2]>xsync[i] && xsync[i+2]<xsync[i+1])
                     || (xsync[i+2]<xsync[i] && xsync[i+2]>xsync[i+1])) xmsync[i]=xsync[i+2];
            else xmsync[i]=xsync[i];
        }
        xmsync[19]=xmsync[17];
        xmsync[20]=xmsync[18];
        for (int i = 0; i < 56; ++i)//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
        {//do i=1,56
            if ((xdata[i]>xdata[i+1] && xdata[i]<xdata[i+2])
                    || (xdata[i]<xdata[i+1] && xdata[i]>xdata[i+2])) xmdata[i]=xdata[i];
            else if ((xdata[i+1]>xdata[i] && xdata[i+1]<xdata[i+2])
                     || (xdata[i+1]<xdata[i] && xdata[i+1]>xdata[i+2])) xmdata[i]=xdata[i+1];
            else if ((xdata[i+2]>xdata[i] && xdata[i+2]<xdata[i+1])
                     || (xdata[i+2]<xdata[i] && xdata[i+2]>xdata[i+1])) xmdata[i]=xdata[i+2];
            else xmdata[i]=xdata[i];
        }
        xmdata[56]=xmdata[54];
        xmdata[57]=xmdata[55];

        for (int i = 0; i < 77; ++i)//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
        {//do i=1,77
            if ((xnoise[i]>xnoise[i+1] && xnoise[i]<xnoise[i+2])
                    || (xnoise[i]<xnoise[i+1] && xnoise[i]>xnoise[i+2])) xmnoise[i]=xnoise[i];
            else if ((xnoise[i+1]>xnoise[i] && xnoise[i+1]<xnoise[i+2])
                     || (xnoise[i+1]<xnoise[i] && xnoise[i+1]>xnoise[i+2])) xmnoise[i]=xnoise[i+1];
            else if ((xnoise[i+2]>xnoise[i] && xnoise[i+2]<xnoise[i+1])
                     || (xnoise[i+2]<xnoise[i] && xnoise[i+2]>xnoise[i+1])) xmnoise[i]=xnoise[i+2];
            else xmnoise[i]=xnoise[i];
        }
        xmnoise[77]=xmnoise[75];
        xmnoise[78]=xmnoise[76];
        double ssync=0.0;
        for (int z = 0; z < 21; ++z) ssync+=xmsync[z];
        ssync/=21.0;
        double spaty=0.0;
        for (int z = 0; z < 32; ++z) spaty+=xmdata[z+26];
        spaty/=32.0;
        double spt1=0.0;
        double spt2=0.0;
        for (int z = 0; z < 3; ++z) spt1+=xmnoise[z+33];
        for (int z = 0; z < 29; ++z) spt2+=xmnoise[z+43];
        double spnoise=spt1+spt2;
        spnoise/=32.0;     //spnoise=(sum(xmnoise(34:36)) + sum(xmnoise(44:72)))/32.0;
        spt1=0.0;
        spt2=0.0;
        for (int z = 0; z < 9; ++z) spt1+=xmdata[z];
        for (int z = 0; z < 32; ++z) spt2+=xmdata[z+26];
        double spother=spt1+spt2;
        spother/=41.0; //spother=(sum(xmdata(1:9)) + sum(xmdata(27:58)))/41.0;
        del0 = (spaty+1E-6);
        if (del0==0.0) del0 = 0.000001;
        double spratiom=ssync/del0;
        del0 = (spnoise+1E-6);
        if (del0==0.0) del0 = 0.000001;
        double spnratiom=spaty/del0;
        del0 = (spother+1E-6);
        if (del0==0.0) del0 = 0.000001;
        double sporatiom=ssync/del0;
        if (spnratiom>2.3)
        {
            if (lcallingrprt || nlasttx==1)
            {
                if (sporatiom>1.35)
                {
                    lft8s=false;
                    msg37="";
                    return;
                }
            }
            else if (spratiom>1.35)
            {
                lft8s=false;
                msg37="";
                return;
            }
        } //qDebug()<<"1 ft8svar<--"<<msg37<<lft8s<<spnratiom;
    } //qDebug()<<"2 ft8svar<--"<<msg37<<lft8s;
}
void DecoderFt8::subtractft8var(int *itone,double f0,double dt)
{
    const int NFRAME=1920*79;//new=151680
    const int NMAX=180000;//15*DEC_SAMPLE_RATE;//=180000
    const int NFFT=180000;//NMAX; need to be number if no crash  180000
    const int NFILT=4000;//3700;//old NFILT=1400; 4000
    //int offset_w = NFILT/2+25;
    //int nstart=dt*DEC_SAMPLE_RATE+1.0;//0 +1.0  -1920

    double complex *cref = new double complex[153681];//151681+ramp      double complex cref[NFRAME+100];
    double complex *cfilt= new double complex[180300];//NMAX+100
    //double dd66[180192]= {0.0};// __attribute__((aligned(16))) = {0.0};  //32 =190,192,182,206,174
    //pomAll.zero_double_beg_end(dd66,0,180005);
    //double *dd66 = new double[180100];  // <- slow w10

    pomAll.zero_double_comp_beg_end(cfilt,0,(NMAX+25));
    //pomAll.zero_double_comp_beg_end(cref,0,NFRAME+25);
    gen_ft8cwaveRx(itone,f0,cref);

    int idt = 0;
    int nstart=dt*DEC_SAMPLE_RATE+1.0+idt;//0 +1.0  -1920
    for (int i = 0; i < NFRAME; ++i)
    {
        int id=nstart+i-1;//0 -1
        if (id>=0 && id<NMAX)
        {
            cfilt[i]=dd8[id]*conj(cref[i]);//camp[i];//cfilt(1:nframe)=camp(1:nframe)
        }
    }
    //2.39
    for (int i = NFRAME; i < 180010; ++i) cfilt[i]=0.0;//cfilt(nframe+1:)=0.0
    f2a.four2a_c2c(cfilt,NFFT,-1,1,decid);
    for (int i = 0; i < NFFT; ++i) cfilt[i]*=cw_subsft8[i];
    f2a.four2a_c2c(cfilt,NFFT,1,1,decid);
    for (int i = 0; i < NFILT/4; ++i) cfilt[i]*=endcorrectionft8[i];//hv NFILT/2+1
    int revv = NFRAME-1;
    for (int i = 0; i < NFILT/4; ++i)//hv NFILT/2+1  //NFRAME=151680 NFILT=4000;
    {
        cfilt[revv]*=endcorrectionft8[i];
        revv--;
    }

    for (int i = 0; i < NFRAME; ++i)//if(j.ge.1 .and. j.le.NMAX) dd(j)=dd(j)-2*REAL(cfilt(i)*cref(i))
    {//do i=1,nframe
        int j=nstart+i-1;//0 -1
        if (j>=0 && j<NMAX)
        {
            double complex cfr = cfilt[i]*cref[i];
            //dd[j]-=1.96*creal(cfr);//2.41=1.96 2.39=1.97  2.35=1.94   2.26 1.93 no->2.0  //2.07 1.92<-tested    1.5,1.6  ,1.7ok,
            dd8[j]-=K_SUB*creal(cfr);
        }
    }
    delete [] cref;
    delete [] cfilt;
}
void DecoderFt8::ft8sdvar(double s8_[79][8],double srr,int *itone,QString msgd,QString &msg37,bool &lft8sd,bool lcq)
{	//OK Tested HV
    double s8_1_[79][8];//(0:7,79)
    int itonedem[58];
    bool lmatched[58];
    QString c1="";
    QString c2="";
    QString msg372="";
    QString msg4[4];
    int idtone[58];
    //int i3=-1;
    //int n3=-1;
    //bool c77[100];
    int idtone4[4][58];
    int itone4[4][79];
    int nmatch1=0;
    int ncrcpaty1=0;
    msgd=msgd.trimmed();

    QString msgd0=msgd+"      ";
    if (msgd.indexOf(" RR73")>-1 || msgd.indexOf(" 73")>-1) return; //! do not process 73 messages
    if (!(msgd0.mid(0,3)=="CQ " || msgd.indexOf("-")>-1 || msgd.indexOf("+")>-1 || msgd.indexOf(" RRR")>-1)) return;//! do not process messages with grid
    //qDebug()<<"1 ft8sdvar <--"<<msgd;
    for (int z = 0; z < 79; ++z)
    {
        for (int z0 = 0; z0 < 8; ++z0) s8_1_[z][z0]=s8_[z][z0];
    }
    for (int z = 0; z < 58; ++z)
    {
        itonedem[z]=11;
        lmatched[z]=false;
    }
    if (!lcq)
    {
        int ispc1=msgd0.indexOf(' ');
        int ispc2=msgd0.indexOf(' ',ispc1+1);
        if (ispc1<=12 && (ispc2-(ispc1+1))<=12)
        {
            c1=msgd0.mid(0,ispc1);
            c2=msgd0.mid(ispc1+1,ispc2-(ispc1+1));
            c1=c1.trimmed();
            c2=c2.trimmed();
        }
        else return;
        if (c1.count()<3 || c2.count()<3 || c2==s_MyCall8) return;//if(len_trim(c1).lt.3 .or. len_trim(c2).lt.3 .or. c2.eq.trim(mycall)) return
        for (int i = 0; i < 4; ++i)
        {//do i=1,4
            msg4[i]="";
            if (i==0) msg4[i]=msgd;
            if (i==1) msg4[i]=c1+" "+c2+" RRR";//trim(c1)//' '//trim(c2)//' RRR'
            if (i==2) msg4[i]=c1+" "+c2+" RR73";//trim(c1)//' '//trim(c2)//' RR73'
            if (i==3) msg4[i]=c1+" "+c2+" 73";//trim(c1)//' '//trim(c2)//' 73'
        }
    }
    if (lcq)
    {
        msg372=msgd;
        TGenFt8->pack77_make_c77_i4tone(msg372,itone);
        for (int z = 0; z < 29; ++z)
        {
            idtone[z]=itone[z+7];//idtone(1:29)=itone(8:36)
            idtone[z+29]=itone[z+43];//idtone(30:58)=itone(44:72)
        }
    }
    else
    {
        for (int i = 0; i < 4; ++i)
        {//do i=1,4
            msg372=msg4[i];
            TGenFt8->pack77_make_c77_i4tone(msg372,itone);
            for (int z = 0; z < 29; ++z)
            {
                idtone4[i][z]=itone[z+7];
                idtone4[i][z+29]=itone[z+43];
            }
            for (int z = 0; z < 79; ++z) itone4[i][z]=itone[z];
        }
    }//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    //qDebug()<<"2 ft8sdvar <--"<<msgd;
    for (int i = 0; i < 58; ++i)
    {//do i=1,58
        if (i<=28)
        {
            int ip1=pomAll.maxloc_da_beg_to_end(s8_1_[i+7],0,8);//maxloc(s8_1(:,i+7))-1;
            itonedem[i]=ip1;
            s8_1_[i+7][ip1]=0.0;
        }
        else
        {
            int ip1=pomAll.maxloc_da_beg_to_end(s8_1_[i+14],0,8);//maxloc(s8_1(:,i+14))-1;
            itonedem[i]=ip1;
            s8_1_[i+14][ip1]=0.0;
        }
    }
    int imax=0;
    if (lcq)
    {
        nmatch1=0;
        ncrcpaty1=0;
        for (int k = 0; k < 58; ++k)
        {//do k=1,58
            if (idtone[k]==itonedem[k])
            {
                nmatch1++;
                lmatched[k]=true;
                if (k>24) ncrcpaty1++;
            }
        }
        if (nmatch1>26)
        {
            lft8sd=true;
            msg37=msgd;
            return;
        }
    }
    else
    {
        int nmatchditer1=0;
        int nbaseiter1=0;
        int ncrcpatyiter1=0;
        //int nbase1=0;
        for (int i = 0; i < 4; ++i)
        {//do i=1,4 //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
            nmatch1=0;
            int nbase1=0;
            ncrcpaty1=0;
            for (int k = 0; k < 58; ++k)
            {//do k=1,58
                if (idtone4[i][k]==itonedem[k])
                {
                    nmatch1++;
                    if (k<22) nbase1++;
                    if (k>24) ncrcpaty1++; //! CRC+parity
                }
            }
            if (nmatch1>nmatchditer1)
            {
                imax=i;
                nmatchditer1=nmatch1;
                nbaseiter1=nbase1;
                ncrcpatyiter1=ncrcpaty1;
            }
        }
        if (imax==0) return;
        if (srr>3.0 && nbaseiter1<12) return;// ! prevent false decodes from the strong signals
        nmatch1=nmatchditer1;
        ncrcpaty1=ncrcpatyiter1;
        //nbase1=nbaseiter1;
        for (int z = 0; z < 58; ++z) idtone[z]=idtone4[imax][z];
        if (nmatch1>26 && ncrcpaty1>10)
        {
            lft8sd=true;
            msg37=msg4[imax];
            for (int z = 0; z < 79; ++z) itone[z]=itone4[imax][z];
            return;
        }
        for (int k = 0; k < 58; ++k)
        {//do k=1,58
            if (idtone[k]==itonedem[k]) lmatched[k]=true;
        }
    }
    if (nmatch1>=16) //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    {
        for (int i = 0; i < 58; ++i)
        {//do i=1,58
            if (lmatched[i]) continue;
            if (i<=28)
            {
                int ip1=pomAll.maxloc_da_beg_to_end(s8_1_[i+7],0,8);//maxloc(s8_1(:,i+7))-1;
                itonedem[i]=ip1;
                s8_1_[i+7][ip1]=0.0;
            }
            else
            {
                int ip1=pomAll.maxloc_da_beg_to_end(s8_1_[i+14],0,8);//maxloc(s8_1(:,i+14))-1;
                itonedem[i]=ip1;
                s8_1_[i+14][ip1]=0.0;
            }
        }
        int nmatch2=nmatch1;
        int ncrcpaty2=ncrcpaty1;
        for (int k = 0; k < 58; ++k)
        {//do k=1,58
            if (lmatched[k]) continue;
            if (idtone[k]==itonedem[k])
            {
                nmatch2++;
                lmatched[k]=true;
                if (k>24) ncrcpaty2++;
            }
        }
        if (nmatch2>38 && ncrcpaty2>19)
        {
            lft8sd=true;
            if (lcq) msg37=msgd;
            else
            {
                msg37=msg4[imax];
                for (int z = 0; z < 79; ++z) itone[z]=itone4[imax][z];
            } //qDebug()<<"1 ft8sdvar <--"<<msg37;
            return;
        }//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
        for (int i = 0; i < 58; ++i)
        {//do i=1,58
            if (lmatched[i]) continue;
            if (i<=28)
            {
                int ip1=pomAll.maxloc_da_beg_to_end(s8_1_[i+7],0,8);//maxloc(s8_1(:,i+7))-1;
                itonedem[i]=ip1;
                s8_1_[i+7][ip1]=0.0;
            }
            else
            {
                int ip1=pomAll.maxloc_da_beg_to_end(s8_1_[i+14],0,8);//maxloc(s8_1(:,i+14))-1;
                itonedem[i]=ip1;
                s8_1_[i+14][ip1]=0.0;
            }
        }
        int nmatch3=nmatch2;
        int ncrcpaty3=ncrcpaty2;
        for (int k = 0; k < 58; ++k)
        {//do k=1,58
            if (lmatched[k]) continue;
            if (idtone[k]==itonedem[k])
            {
                nmatch3++;
                lmatched[k]=true;
                if (k>24) ncrcpaty3++;
            }
        }
        if (nmatch3>44 && ncrcpaty3>21)
        {
            lft8sd=true;
            if (lcq) msg37=msgd;
            else
            {
                msg37=msg4[imax];
                for (int z = 0; z < 79; ++z) itone[z]=itone4[imax][z];//itone(1:79)=itone4(imax,1:79);
            } //qDebug()<<"2 ft8sdvar <--"<<msg37;
            return;
        }
        for (int i = 0; i < 58; ++i)
        {//do i=1,58
            if (lmatched[i]) continue;
            if (i<=28)
            {
                int ip1=pomAll.maxloc_da_beg_to_end(s8_1_[i+7],0,8);//maxloc(s8_1(:,i+7))-1;
                itonedem[i]=ip1;
                s8_1_[i+7][ip1]=0.0;
            }
            else
            {
                int ip1=pomAll.maxloc_da_beg_to_end(s8_1_[i+14],0,8);//maxloc(s8_1(:,i+14))-1;
                itonedem[i]=ip1;
                s8_1_[i+14][ip1]=0.0;
            }
        }
        int nmatch4=nmatch3;
        int ncrcpaty4=ncrcpaty3;
        for (int k = 0; k < 58; ++k)
        {//do k=1,58
            if (lmatched[k]) continue;
            if (idtone[k]==itonedem[k])
            {
                nmatch4++;
                lmatched[k]=true;
                if (k>24) ncrcpaty4++;
            }
        }
        if (nmatch4>47 && ncrcpaty4>23)
        {
            lft8sd=true;
            if (lcq) msg37=msgd;
            else
            {
                msg37=msg4[imax];
                for (int z = 0; z < 79; ++z) itone[z]=itone4[imax][z];//itone(1:79)=itone4(imax,1:79);
            } //qDebug()<<"3 ft8sdvar <--"<<msg37;
            return;
        }
        for (int i = 0; i < 58; ++i)
        {//do i=1,58
            if (lmatched[i]) continue;
            if (i<=28)
            {
                int ip1=pomAll.maxloc_da_beg_to_end(s8_1_[i+7],0,8);//maxloc(s8_1(:,i+7))-1;
                itonedem[i]=ip1;
                s8_1_[i+7][ip1]=0.0;
            }
            else
            {
                int ip1=pomAll.maxloc_da_beg_to_end(s8_1_[i+14],0,8);//maxloc(s8_1(:,i+14))-1;
                itonedem[i]=ip1;
                s8_1_[i+14][ip1]=0.0;
            }
        }
        int nmatch5=nmatch4;
        int ncrcpaty5=ncrcpaty4;
        for (int k = 0; k < 58; ++k)
        {//do k=1,58
            if (lmatched[k]) continue;
            if (idtone[k]==itonedem[k])
            {
                nmatch5++;
                lmatched[k]=true;
                if (k>24) ncrcpaty5++;
            }
        }
        if (nmatch5>50 && (nmatch1>21 || nmatch2>31 || nmatch3>38 || nmatch4>46) && ncrcpaty5>25)
        {
            lft8sd=true;
            if (lcq) msg37=msgd;
            else
            {
                msg37=msg4[imax];
                for (int z = 0; z < 79; ++z) itone[z]=itone4[imax][z];//itone(1:79)=itone4(imax,1:79);
            } //qDebug()<<"4 ft8sdvar <--"<<msg37;
            return;
        }
        for (int i = 0; i < 58; ++i)
        {//do i=1,58
            if (lmatched[i]) continue;
            if (i<=28)
            {
                int ip1=pomAll.maxloc_da_beg_to_end(s8_1_[i+7],0,8);//maxloc(s8_1(:,i+7))-1;
                itonedem[i]=ip1;
                s8_1_[i+7][ip1]=0.0;
            }
            else
            {
                int ip1=pomAll.maxloc_da_beg_to_end(s8_1_[i+14],0,8);//maxloc(s8_1(:,i+14))-1;
                itonedem[i]=ip1;
                s8_1_[i+14][ip1]=0.0;
            }
        }
        int nmatch6=nmatch5;
        int ncrcpaty6=ncrcpaty5;
        for (int k = 0; k < 58; ++k)
        {//do k=1,58
            if (lmatched[k]) continue;
            if (idtone[k]==itonedem[k])
            {
                nmatch6++;
                lmatched[k]=true;
                if (k>24) ncrcpaty6++;
            }
        }
        if (nmatch6>54 && (nmatch1>22 || nmatch2>27 || nmatch3>35 || (nmatch2-nmatch1)>9 || (nmatch3-nmatch2)>10) && ncrcpaty6>29)
        {
            lft8sd=true;
            if (lcq) msg37=msgd;
            else
            {
                msg37=msg4[imax];
                for (int z = 0; z < 79; ++z) itone[z]=itone4[imax][z];//itone(1:79)=itone4(imax,1:79);
            } //qDebug()<<"5 ft8sdvar <--"<<msg37;
            return;
        }
    }
}
/*static const int maskincallthr[25]=
    {
        0,30,45,55,65,75,85,90,95,100,105,110,115,120,125,130,135,140,145,150,155,160,165,170,175
    };*/
void DecoderFt8::ft8bvar(bool &newdat1,int nQSOProgress,double nfqso,double nftx,bool lsubtract,bool nagainfil,
                         double &f1,double &xdt,int &nbadcrc,QString &msg37,double &xsnr,bool stophint,
                         bool &lFreeText,int ipass,bool lft8subpass,bool &lspecial,bool lcqcand,int npass,
                         bool lmycallstd,bool lhiscallstd,bool f_eve0_odd1,bool &lft8sd,int &i3,
                         int nft8rxfsens,bool &lhashmsg,bool lqsothread,bool lft8lowth,bool lhighsens,bool lnohiscall,
                         bool lnomycall,bool lnohisgrid,double &qual,int &iaptype2,int cont_type,double napwid)
{
    const int cd_c = 4950;
    const int cd_off = 900;
    const int graymap[8] =
        {
            0,1,3,2,5,6,4,7
        };
    const uint8_t naptypes[6][27]=
        {   //HV
            {3,3,3,0,0,0,0,0,0,0,0,0,2,2,2,1,1,1,31,31,31,36,36,36,35,35,35},// ! Tx6 CQ
            {3,3,3,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,31,31,31,36,36,36,35,35,35},// ! Tx1 Grid
            {3,3,3,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,31,31,31,36,36,36,35,35,35},// ! Tx2 Report
            {3,3,3,6,6,6,5,5,5,4,4,4,0,0,0,0,0,0,31,31,31,36,36,36,35,35,35},// ! Tx3 RRreport
            {3,3,3,6,6,6,5,5,5,4,4,4,2,2,2,0,0,0,31,31,31,36,36,36,35,35,35},// ! Tx4 RRR,RR73
            {0,0,0,0,0,0,0,0,0,0,0,0,2,2,2,1,1,1,31,31,31,36,36,36,35,35,35}// ! Tx5 73
        };
    const uint8_t ndxnsaptypes[6][27]=
        {   //HV
            {0,0,0,0,0,0,0,0,0,0,0,0,2,2,2,1,1,1,31,31,31,36,36,36,35,35,35},//! Tx6 CQ
            {11,11,11,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,31,31,31,36,36,36,35,35,35},//! Tx1 Grid
            {11,11,11,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,31,31,31,36,36,36,35,35,35},//! Tx2 Report
            {11,11,11,14,14,14,13,13,13,12,12,12,0,0,0,0,0,0,31,31,31,36,36,36,35,35,35},//! Tx3 RRreport
            {11,11,11,14,14,14,13,13,13,12,12,12,2,2,2,0,0,0,31,31,31,36,36,36,35,35,35},//! Tx4 RRR,RR73
            {0,0,0,0,0,0,0,0,0,0,0,0,2,2,2,1,1,1,31,31,31,36,36,36,35,35,35}//! Tx5 73
        };
    const uint8_t nmycnsaptypes[6][27]=
        {   //HV
            { 0,0,0,0,0,0,0,0,0,0,0,0,40,40,40,1,1,1,31,31,31,36,36,36,35,35,35},//! Tx6 CQ
            {41,41,41,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,31,31,31,36,36,36,35,35,35},//! Tx1 DXcall MyCall
            {41,41,41,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,31,31,31,36,36,36,35,35,35},//! Tx2 Report
            {41,41,41,44,44,44,43,43,43,42,42,42,0,0,0,0,0,0,31,31,31,36,36,36,35,35,35},//! Tx3 RRreport
            {41,41,41,44,44,44,43,43,43,42,42,42,40,40,40,0,0,0,31,31,31,36,36,36,35,35,35},//! Tx4 RRR,RR73
            {0,0,0,0,0,0,0,0,0,0,0,0,40,40,40,1,1,1,31,31,31,36,36,36,35,35,35}//! Tx5 73
        }
        ;
    /*const uint8_t nhaptypes[6][27]=
        {   //HV
            {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,111,111,111},//! Tx6 CQ, possible in idle mode if DXCall is empty
            {21,21,21,22,22,22,0,0,0,0,0,0,0,0,0,31,31,31,0,0,0,36,36,36,0,0,0},//! Tx1 Grid idle mode or transmitting
            {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},//! Tx2 none
            {21,21,21,22,22,22,23,23,23,24,24,24,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},//! Tx3 RRreport QSO in progress or QSO is finished
            {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},//! Tx4 none
            {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}//! Tx5 none
        };*/
    bool lenabledxcsearch=false;//?? ! in QSO or TXing CQ or last logged is DX Call: searching disabled
    bool lwidedxcsearch=true;   //?? ! only RX freq DX Call searching
    if (s_HisCall8.count()>2)
    {
        lenabledxcsearch=true;
        //lwidedxcsearch=true;
    }
    int i3bit=0;
    iaptype2 = 99;  //!ft8md
    //int max_iterations=30;
    int nharderrors=-1;
    nbadcrc=1;
    double delfbest=0.0;
    int ibest=0;
    double dfqso=500.0;
    double rrxdt=0.5;
    double fs2=200.0;
    double dt2=0.005; //! fs2=12000.0/NDOWN; dt2=1.0/fs2
    //bool lcall1hash=false;
    //bool lcall2hash=false;
    bool lskipnotap=false;
    bool ldeepsync=false;
    if (lft8lowth || lft8subpass) ldeepsync=true;
    bool lcallsstd=true;
    if (!lmycallstd || !lhiscallstd) lcallsstd=false;
    double xdt0=xdt;
    double f10=f1;//! apply last freq f1 and last DT criteria here
    int nqso=1; //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    //qDebug()<<lft8subpass<<lft8lowth;
    if (lqsothread && !lft8sdec && !lqsomsgdcd && !stophint && nlasttx>=1 && nlasttx<=4 && fabs(f10-nfqso)<2.51)
    {
        if (lastrxmsg[0].lstate && fabs(lastrxmsg[0].xdt-xdt)<0.18) nqso=2;
        else if (!lastrxmsg[0].lstate) nqso=2;
    } //if (decid==0) qDebug()<<lqsothread<<lft8sdec<<lqsomsgdcd<<stophint<<nlasttx<<lastrxmsg[0].lstate<<nqso;
    bool lvirtual2=false;
    bool lvirtual3=false;
    int maxlasttx=4;
    double complex cd0[5000];//double complex *cd0 = new double complex[5000];//double complex cd0[5000];//(-800:4000)
    double complex cd1[5000];//double complex *cd1 = new double complex[5000];//double complex cd1[5000];//(-800:4000)
    double complex cd2[5000];//(-800:4000)
    double complex cd3[5000];//(-800:4000)
    for (int z = 0; z < 5000; ++z)
    {
        cd0[z]=0.0+0.0*I;
        cd1[z]=0.0+0.0*I;
        cd2[z]=0.0+0.0*I;
        cd3[z]=0.0+0.0*I;
    }
    QString msgd="";
    bool lapcqonly=false;
    bool lastsync=false;
    bool lsdone=false;
    double complex ctwk[32+5];
    double a[6]; //double syncavpart[3];
    double snrsync[21];
    double complex csymb[32+5];
    double complex csymbr[32+5];
    double complex cscs_[79][8];//(0:7,79)
    double complex csr_[79][8];//(0:7,79)
    double complex cs_[79][8];//(0:7,79)
    double complex cstmp2_[79][8];//(0:7,79)
    double s8_[79][8];//(0:7,79)
    double s82_[79][8];//(0:7,79)
    double s81[8+2];
    double sp[8];//(0:7),
    double syncw[7];//(7)
    double sumkw[7];//(7)
    double scoreratiow[7];//(7)
    int nsmax[8];//(8)
    int itone[140];//(79)
    int iaptype=0;
    double complex csymb256[256];//(256)
    double s256[9];//(0:8),
    double s2563[27];
    double s2[512+10];//double *s2=new double[512*10];//double s2[512+10];
    double bmeta[174];//double *bmeta=new double[174];//
    double bmetb[174];
    double bmetc[174];
    double bmetd[174];
    double llra[174];
    double llrb[174];
    double llrc[174];
    double llrd[174];
    double llrz[174];
    bool apmask[174];
    bool cw[194];//174
    bool message77[150];
    double dmin=0.0;
    bool lft8s=false;
    double complex csig[32];//(32)
    bool lfoundcq=false;
    bool lfoundmyc=false;
    int n3;
    //double napwid8 = s_napwid8;  qDebug()<<decid<<napwid8;
    /*for (int z = 0; z < 79; ++z)//[79][8]
    {
    	for (int z0 = 0; z0 < 8; ++z0)
    	{
    		s8_[z][z0]=0.0;
    		cs_[z][z0]=0.0+0.0*I;
    		csr_[z][z0]=0.0+0.0*I;
    		cscs_[z][z0]=0.0+0.0*I;	
    	}
    }*/
    for (int z = 0; z < 82; ++z) itone[z]=0; //if (decid==0) qDebug()<<decid<<lqsothread<<lft8sdec<<lqsomsgdcd;
    if (lqsothread && !lft8sdec && !lqsomsgdcd)
    {
        if (s_HisCall8.count()>2) //then s_MyCall8 s_HisCall8
        {
            if (xdt>4.9 || xdt<-4.9)
            {
                if (lastrxmsg[0].lstate && lastrxmsg[0].lastmsg==msgroot+" RRR") maxlasttx=5;
            }
            if (xdt>4.9)
            {
                if (!stophint && nlasttx>=1 && nlasttx<=maxlasttx && fabs(f10-nfqso)<0.1)
                {
                    if (lastrxmsg[0].lstate)
                    {
                        xdt0=lastrxmsg[0].xdt;
                        nqso=2;
                        lvirtual2=true;
                    }
                    if (!lastrxmsg[0].lstate)
                    {
                        if (!f_eve0_odd1)
                        {
                            for (int i = 0; i < ncalldt; ++i)
                            {//do i=1,150
                                if (calldteven[i].call2==s_HisCall8)
                                {
                                    xdt0=calldteven[i].dt;
                                    nqso=3;
                                    lvirtual2=true;
                                    break;
                                }
                            }
                        }
                        else// if (loddint)
                        {
                            for (int i = 0; i < ncalldt; ++i)
                            {//do i=1,150
                                if (calldtodd[i].call2==s_HisCall8)
                                {
                                    xdt0=calldtodd[i].dt;
                                    nqso=3;
                                    lvirtual2=true;
                                    break;
                                }
                            }
                        }
                    }
                }
            }
            else if (xdt<-4.9)//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
            {
                if (!stophint && nlasttx>=1 && nlasttx<=maxlasttx && fabs(f10-nfqso)<0.1)
                {
                    if (lastrxmsg[0].lstate)
                    {
                        xdt0=lastrxmsg[0].xdt;
                        nqso=2;
                        lvirtual2=true;
                    }
                    if (!lastrxmsg[0].lstate)
                    {
                        if (!f_eve0_odd1)
                        {
                            for (int i = 0; i < ncalldt; ++i)
                            {//do i=1,150
                                if (calldteven[i].call2==s_HisCall8)
                                {
                                    xdt0=calldteven[i].dt;
                                    nqso=3;
                                    lvirtual3=true;
                                    break;
                                }
                            }
                        }
                        else// if (loddint)
                        {
                            for (int i = 0; i < ncalldt; ++i)
                            {//do i=1,150
                                if (calldtodd[i].call2==s_HisCall8)
                                {
                                    xdt0=calldtodd[i].dt;
                                    nqso=3;
                                    lvirtual3=true;
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    //if (decid==0) qDebug()<<"In ft8_downsamplevar";
    ft8_downsamplevar(newdat1,f1,nqso,cd0,cd2,cd3,lhighsens,cd_off);//!Mix f1 to baseband and downsample
    bool lsd=false;
    int isd=0;//1;
    bool lcq=false;
    if (!f_eve0_odd1)//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    {
        for (int i = 0; i < MAXStatOE; ++i)
        {//do i=1,130
            if (!evencopy[i].lstate) continue;
            if (fabs(evencopy[i].freq-f1)<3.0 && fabs(evencopy[i].dt-xdt)<0.19)
            {
                msgd=evencopy[i].msg; //if (decid==1) qDebug()<<i<<ipass<<"msgd"<<msgd;
                QString tmsgd=msgd+"#   ";
                lsd=true;
                isd=i;
                if (tmsgd.midRef(0,3)=="CQ " || tmsgd.midRef(0,3)=="DE " || tmsgd.midRef(0,4)=="QRZ ") lcq=true;
            }
        }
    }
    else// if (loddint)
    {
        for (int i = 0; i < MAXStatOE; ++i)
        {//do i=1,130
            if (!oddcopy[i].lstate) continue;
            if (fabs(oddcopy[i].freq-f1)<3.0 && fabs(oddcopy[i].dt-xdt)<0.19)
            {
                msgd=oddcopy[i].msg;
                QString tmsgd=msgd+"#   ";
                lsd=true;
                isd=i;
                if (tmsgd.midRef(0,3)=="CQ " || tmsgd.midRef(0,3)=="DE " || tmsgd.midRef(0,4)=="QRZ ") lcq=true;
            }
        }
    }
    //if (!msgd.isEmpty()) qDebug()<<"-4-1-tonesdvar"<<lsd<<msgd;
    if (lsd && nqso==1) nqso=4;
    /*!nlasttx  last TX message
    !  0       Tx was halted
    !  1      AA1AA BB1BB PL35
    !  2      AA1AA BB1BB -15
    !  3      AA1AA BB1BB R-15
    !  4      AA1AA BB1BB RRR/RR73
    !  5      AA1AA BB1BB 73
    !  6      CQ BB1BB PL35*/
    if (nqso==4) for (int i = 0; i < cd_c; ++i) cd1[i]=cd0[i];
    /*for (int z = 0; z < cd_c; ++z)
    {
        if (qIsNaN(cabs(cd0[z]))) qDebug()<<z<<"cd0 NAN";
        if (qIsNaN(cabs(cd1[z]))) qDebug()<<z<<"cd1 NAN";
        if (qIsNaN(cabs(cd2[z]))) qDebug()<<z<<"cd2 NAN";
        if (qIsNaN(cabs(cd3[z]))) qDebug()<<z<<"cd3 NAN";
    }*/
    //qDebug()<<"DF="<<nqso<<creal(cd1[200])<<creal(cd1[300])<<creal(cd1[400]);
    int i0=0;
    for (int iqso = 1; iqso <= nqso; ++iqso)
    {//do iqso=1,nqso
        lapcqonly=false;//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
        if (iqso>1 && iqso<4 && nqso==4) continue;
        if (xdt0<-4.9 || xdt0>4.9) continue;
        if (lvirtual2 && iqso!=2) continue;
        if (lvirtual3 && iqso<2) continue;
        if ((lvirtual2 || lvirtual3) && nft8rxfsens<3 && iqso==3) continue;
        lastsync=false;
        lsdone=false;
        if (!lvirtual2 && !lvirtual3 && iqso==2) for (int i = 0; i < cd_c; ++i) cd0[i]=cd2[i];
        else if (lvirtual2 && iqso==2) for (int i = 0; i < cd_c; ++i) cd0[i]=cd2[i];
        else if (lvirtual3 && iqso==2) for (int i = 0; i < cd_c; ++i) cd0[i]=cd3[i];
        i0=0;                   //!Initial guess for start of signal
        double smax=0.0;
        double xdt2=0.0;
        int kstep=0;
        double delf=0.0;
        double syncav;
        double synclev;
        double snoiselev;
        bool lreverse;
        int k0x;
        int nsyncscorew;
        //double scoreratiowa=0.0;
        double syncavemax=0.0;
        int is1,is2,is3,nsyncscore,nsyncscore1,nsyncscore2,nsyncscore3,nsync2;
        double scoreratio,scoreratio1,scoreratio2,scoreratio3;
        int nsync;
        double rscq;
        double srr=0.0;
        double sums8t;
        //double max=-5.0; double min=5.0;
        if (iqso==4)
        {
            tonesdvar(msgd,lcq); //qDebug()<<"-4-1-tonesdvar"; msgd="LZ2HV SP9HWY +00"; //lcq=false;
            if (!ldeepsync) goto c32;
            for (int i = 0; i < cd_c; ++i) cd0[i]=cd1[i];
        }
        if (iqso==3) goto c16;
        i0=(int)((xdt0+0.5)*fs2);                   //!Initial guess for start of signal
        smax=0.0;
        for (int z = 0; z < 32; ++z) ctwk[z]=1.0+0.0*I;//HV
        for (int idt = i0-8; idt <= i0+8; ++idt) //for (int idt = i0-8; idt <= i0+8; ++idt)
        {//do idt=i0-8,i0+8                         !Search over +/- one quarter symbol
            double sync;
            sync8dvar(cd0,idt,ctwk,0,sync,ipass,lastsync,iqso,lcq,lcallsstd,lcqcand,cd_off);
            if (sync>smax)
            {
                smax=sync;
                ibest=idt;
            }
        }
        xdt2=(double)ibest*dt2; //qDebug()<<"all="<<xdt2;//!Improved estimate for DT
        //! Now peak up in frequency
        i0=(int)(xdt2*fs2);
        smax=0.0;
        kstep=0;//1;
        for (int ifr = -FS8D; ifr <= FS8D; ++ifr)
        {//do ifr=-5,5
            if (iqso==1 || iqso==4)
            {
                for (int z = 0; z < 32; ++z) ctwk[z]=ctwkw[kstep][z];//(kstep,:)
                delf=(double)ifr*0.5; //! Search over +/- 2.5 Hz
            }
            else
            {
                for (int z = 0; z < 32; ++z) ctwk[z]=ctwkn[kstep][z];//(kstep,:)
                delf=(double)ifr*0.25; //! Search over +/- 1.25 Hz
            }//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
            double sync;
            sync8dvar(cd0,i0,ctwk,1,sync,ipass,lastsync,iqso,lcq,lcallsstd,lcqcand,cd_off);
            if (sync>smax)
            {
                smax=sync;
                delfbest=delf;
            }
            kstep++;
        } //qDebug()<<"sync1="<<smax;
        for (int z = 0; z < 5; ++z) a[z]=0.0;
        a[0]=-delfbest;
        twkfreq1var(cd0,4002+cd_off,fs2,a,cd0);//,cd_off//twkfreq1var(cd0,-800,3199,4000,fs2,a,cd0,cd_off);
        //pomFt.twkfreq1(cd0,4002+cd_off,fs2,a,cd0);
        /*for (int z = 0; z < cd_c; ++z)
        {
           if (qIsNaN(cabs(cd0[z]))) qDebug()<<z<<"cd0 NAN";
           if (qIsInf(cabs(cd0[z]))) qDebug()<<z<<"cd0 INF";
        }*/
        xdt=xdt2;
        f1=f10+delfbest; //!Improved estimate of DF
        dfqso=fabs(nfqso-f1); //if (dfqso<5.0) qDebug()<<"dfqso="<<dfqso;
        /*lastsync=true; //sync8dvar(...,2,....)<- HV no needed only for degug
        double sync; 
        sync8dvar(cd0,i0,ctwk,2,sync,ipass,lastsync,iqso,lcq,lcallsstd,lcqcand,cd_off);*/
        //if (sync<5.2) continue;
        //qDebug()<<"sync3="<<sync;
c16:
        if (iqso==3) ibest++;
        syncav=0.0;
        for (int z = 0; z < 21; ++z) snrsync[z]=0.0;
        for (int k = 0; k < 79; ++k)
        {//do k=1,79 //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
            int i1=ibest+(k)*32;//ibest+(k-1)*32
            for (int z = 0; z < 32; ++z) csymb[z]=cd0[z+i1+cd_off];//(i1:i1+31)
            if ((k>=0 && k<=6) || (k>=36 && k<=42) || (k>=72 && k<=78))
            {
                f2a.four2a_c2c(csymb,32,-1,1,decid);//four2avar(csymb,32,1,-1,1)
                double sums81=0.0;
                for (int z = 0; z < 8; ++z)
                {
                    s81[z]=cabs(csymb[z]);
                    sums81+=s81[z];
                }
                if (k>=0 && k<=6)
                {
                    synclev=s81[icos7_2[k]];
                    snoiselev=(sums81-synclev)/7.0;
                    if (snoiselev>1.E-16) snrsync[k]=synclev/snoiselev;
                }
                else if (k>=36 && k<=42)
                {
                    synclev=s81[icos7_2[k-36]];
                    snoiselev=(sums81-synclev)/7.0;
                    if (snoiselev>1.E-16) snrsync[k-28]=synclev/snoiselev;
                }
                else if (k>=72 && k<=78)
                {
                    synclev=s81[icos7_2[k-72]];
                    snoiselev=(sums81-synclev)/7.0;
                    if (snoiselev>1.E-16) snrsync[k-57]=synclev/snoiselev;
                }
                else continue;
            }
        }
        for (int z = 0; z < 21; ++z) syncav+=snrsync[z];
        syncav/=21.0;
        /*double syncavpart[3];
        for (int z = 0; z < 3; ++z) syncavpart[z]=0.0;
        for (int z = 0; z < 7; ++z)
        {
            syncavpart[0]+=snrsync[z];
            syncavpart[1]+=snrsync[z+7];
            syncavpart[2]+=snrsync[z+14];
        }
        syncavpart[0]/=7.0;
        syncavpart[1]/=7.0;
        syncavpart[2]/=7.0;
        syncavemax=pomAll.maxval_da_beg_to_end(syncavpart,0,3);*/
        //if (syncavemax<1.8) printf(" syncavemax= %.3f\n",syncavemax);
        //!    plev=0.0
        //!    do k=1000,1009; abscd=abs(cd0(k)); if(abscd.gt.plev) plev=abscd; enddo
        //!    plev=plev/61.0
        //!    do k=0,3199; xx=plev*gran(); yy=plev*gran(); cd0(k)=cd0(k) + complex(xx,yy); enddo
        lreverse=false;
        if (nft8cycles<2)
        {
            if (ipass==2) lreverse=true;
        }
        else 
        {	
            if (ipass==5 || ipass==7) lreverse=true;
        }       
        for (int k = 0; k < 79; ++k)
        {//do k=1,79
            int i1=ibest+(k)*32;
            for (int z = 0; z < 32; ++z) csymb[z]=cd0[z+i1+cd_off];//(i1:i1+31)
            if (syncav<2.5)
            {
                csymb[0]=csymb[0]*1.9;
                csymb[31]=csymb[31]*1.9;
                double del0=sqrt(cabs(csymb[31]));
                if (del0==0.0) del0=0.000001;
                double scr=sqrt(cabs(csymb[0]))/del0;
                if (scr>1.0) csymb[31]=csymb[31]*scr;
                else
                {
                    if (scr>1.E-16) csymb[0]=csymb[0]/scr;
                }
            }
            //csymbr(i)=cmplx(real(csymb(33-i)),-aimag(csymb(33-i))) //csymbr[i]=(creal(csymb[31-i])-cimag(csymb[31-i]));
            for (int i = 0; i < 32; ++i) csymbr[i]=creal(csymb[31-i])+(-cimag(csymb[31-i]))*I;            
            if (lreverse)//double complex cscs[8][79];
            {
                f2a.four2a_c2c(csymb,32,-1,1,decid);//call four2avar(csymb,32,1,-1,1)
                for (int z = 0; z < 8; ++z) cscs_[k][z]=(csymb[z]/(double)1e3);
                for (int z = 0; z < 32; ++z) csymb[z]=csymbr[z];
            }
            f2a.four2a_c2c(csymb,32,-1,1,decid);//call four2avar(csymb,32,1,-1,1)
            for (int z = 0; z < 8; ++z) cs_[k][z]=(csymb[z]/(double)1e3);
            if (lreverse)
            {
                for (int z = 0; z < 79; ++z)
                {
                    for (int z0 = 0; z0 < 8; ++z0) csr_[z][z0]=cs_[z][z0];
                }
            }
            else
            {
                f2a.four2a_c2c(csymbr,32,-1,1,decid);//call four2avar(csymbr,32,1,-1,1)
                for (int z = 0; z < 8; ++z) csr_[k][z]=(csymbr[z]/(double)1e3);
            }
            for (int z = 0; z < 8; ++z) s8_[k][z]=cabs(csymb[z]);
        }
        //if (iqso==2 || iqso==3) qDebug()<<"X="<<iqso<<s8_[4][5]<<s8_[50][3]<<s8_[60][2]<<s8_[30][7]<<s8_[33][2];
        for (int z = 0; z < 8; ++z) sp[z]=0.0;
        for (int k = 0; k < 8; ++k)
        {//do k=0,7
            double sum8x = 0.0;
            double sum8y = 0.0;//sp(k)=sum(s8(k,1:7))+sum(s8(k,18:79))
            for (int z = 0; z < 8; ++z)  sum8x+=s8_[z][k];
            for (int z = 0; z < 62; ++z) sum8y+=s8_[z+17][k];
            sp[k]=(sum8x+sum8y);
        }
        //ka=minloc(sp)-1
        //k0x=-1;
        k0x=pomAll.minloc_da_beg_to_end(sp,0,8);// if (k0x<=0) qDebug()<<"k0x="<<k0x;//0;//ka(1)
        if (k0x<0) goto c128;
        for (int kb = 0; kb < 8; ++kb)
        {//do kb=0,7
            if (kb==k0x) continue;
            double del0 = sp[k0x];
            if (del0==0.0) del0=0.000001;
            double spr=sp[kb]/del0;
            if (spr>1.5)
            {
                double sprsqr=sqrt(spr);
                const double faccs=0.9;//HV
                if (sprsqr==0.0) sprsqr=0.000001;
                for (int z = 0; z < 79; ++z)
                {
                    s8_[z][kb]/=spr;
                    cs_[z][kb]/=sprsqr;
                    csr_[z][kb]/=sprsqr;
                    cscs_[z][kb]/=sprsqr;
                    s8_[z][kb]*=faccs;//HV
                    cs_[z][kb]*=faccs;//HV
                    csr_[z][kb]*=faccs;//HV
                    cscs_[z][kb]*=faccs;//HV
                    //if (s8_[z][kb]>max) max=s8_[z][kb];
                    //if (s8_[z][kb]<min) min=s8_[z][kb];
                }
            }
        }
        /*for (int i = 0; i < 8; ++i)// red2[i]=red2[i]/base2;
        {
        	for (int z = 0; z < 79; ++z)
        	{
        		if (max<s8_[z][i]) max=s8_[z][i];
        		if (min>s8_[z][i]) min=s8_[z][i];    	
        	}
        } if (max>50.0) qDebug()<<"   MAX="<<max<<min;*/
c128:
        //continue;//?? //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
        if (iqso>1 && iqso<4)//double s82[8][79];//(0:7,79)
        {
            for (int z = 0; z < 79; ++z)
            {
                for (int z0 = 0; z0 < 8; ++z0) s82_[z][z0]=sqrt(s8_[z][z0]);
            }
            goto c8;
        }
        if (iqso==4) goto c32;
        nsyncscorew=0;
        //scoreratiowa=0.0;
        rrxdt=xdt-0.5;
        if (rrxdt>=-0.5 && rrxdt<=2.13)
        {
            for (int k = 0; k < 7; ++k)//double s8[8][79];//(0:7,79)
            {//do k=1,7 syncw(icos7_2[k]+1)=s8(icos7(k-1),k)+s8(icos7(k-1),k+36) + s8(icos7(k-1),k+72)
                syncw[icos7_2[k]]=s8_[k][icos7_2[k]]+s8_[k+36][icos7_2[k]] + s8_[k+72][icos7_2[k]];
            }
            for (int k = 0; k < 7; ++k)
            {//do k=1,7 sumkw(k)=(sum(s8(k-1,:))-syncw(k))/25.333
                double sums8=0.0;
                for (int z = 0; z < 79; ++z) sums8+=s8_[z][k];//(k-1,:))
                sumkw[k]=(sums8-syncw[k])/25.333;
            }//! (79-3)/3
        }
        else if (rrxdt<-0.5)
        {
            for (int k = 0; k < 7; ++k)
            {//do k=1,7 syncw(icos7(k-1)+1)=s8(icos7(k-1),k+36)+s8(icos7(k-1),k+72)
                syncw[icos7_2[k]]=s8_[k+36][icos7_2[k]] + s8_[k+72][icos7_2[k]];
            }
            for (int k = 0; k < 7; ++k)
            {//do k=1,7 sumkw(k)=(sum(s8(k-1,26:79))-syncw(k))/26.
                double sums8=0.0;
                for (int z = 0; z < 53; ++z) sums8+=s8_[z+25][k];
                sumkw[k]=(sums8-syncw[k])/26.0;
            } //! (54-2)/2
        }
        else if (rrxdt>2.13)
        {
            for (int k = 0; k < 7; ++k)
            {//do k=1,7 syncw(icos7(k-1)+1)=s8(icos7(k-1),k)+s8(icos7(k-1),k+36)
                syncw[icos7_2[k]]=s8_[k][icos7_2[k]]+s8_[k+36][icos7_2[k]];
            }
            for (int k = 0; k < 7; ++k)
            {//do k=1,7 sumkw(k)=(sum(s8(k-1,1:54))-syncw(k))/26.
                double sums8=0.0;
                for (int z = 0; z < 54; ++z) sums8+=s8_[z][k];
                sumkw[k]=(sums8-syncw[k])/26.0;
            }//! (54-2)/2
        }
        smax=0.0;//hv use free vareable
        for (int k = 0; k < 7; ++k)//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
        {//do k=1,7
            if (syncw[k]>sumkw[k]) nsyncscorew++;
            double del0 = sumkw[k];
            if (del0==0.0) del0=0.000001;
            scoreratiow[k]=syncw[k]/del0;
            smax+=scoreratiow[k];
        }//double scoreratiow[7];//(7)
        //scoreratiowa=smax/7.0;
        //! sync quality check
        is1=0;
        is2=0;
        is3=0;
        nsyncscore=0;
        nsyncscore1=0;
        nsyncscore2=0;
        nsyncscore3=0;
        scoreratio=0.0;
        scoreratio1=0.0;
        scoreratio2=0.0;
        scoreratio3=0.0;
        nsync2=0;
        for (int k = 0; k < 7; ++k)
        {//do k=1,7
            double synck;
            double sumk;
            for (int z = 0; z < 8; ++z) s81[z]=s8_[k][z];//(:,k)
            int ip=pomAll.maxloc_da_beg_to_end(s81,0,8);//maxloc(s81)
            if (icos7_2[k]==ip) is1++;
            else//s81(0:7)
            {
                s81[ip]=0.0;//s81(ip(1)-1)=0.
                ip=pomAll.maxloc_da_beg_to_end(s81,0,8);//ip=maxloc(s81)
                if (icos7_2[k]==ip) nsync2++;
            }
            for (int z = 0; z < 8; ++z) s81[z]=s8_[k+36][z];//s81=s8(:,k+36)
            ip=pomAll.maxloc_da_beg_to_end(s81,0,8);//ip=maxloc(s81)
            if (icos7_2[k]==ip) is2++;
            else
            {
                s81[ip]=0.0;//s81(ip(1)-1)=0.
                ip=pomAll.maxloc_da_beg_to_end(s81,0,8);//ip=maxloc(s81)
                if (icos7_2[k]==ip) nsync2++;
            }
            for (int z = 0; z < 8; ++z) s81[z]=s8_[k+72][z];//s81=s8(:,k+72)
            ip=pomAll.maxloc_da_beg_to_end(s81,0,8);//ip=maxloc(s81)
            if (icos7_2[k]==ip) is3++;
            else
            {
                s81[ip]=0.0;//s81(ip(1)-1)=0.
                ip=pomAll.maxloc_da_beg_to_end(s81,0,8);//ip=maxloc(s81)
                if (icos7_2[k]==ip) nsync2++;
            }
            if (rrxdt>=-0.5)//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
            {
                synck=s8_[k][icos7_2[k]];
                double sums8=0.0;
                for (int z = 0; z < 8; ++z) sums8+=s8_[k][z];
                sumk=(sums8-synck)/7.0;
                if (synck>sumk)
                {
                    nsyncscore1++;
                    double del0 = sumk;
                    if (del0==0.0) del0=0.000001;
                    scoreratio1+=(synck/del0);
                }
            }
            synck=s8_[k+36][icos7_2[k]];//s8(icos7(k-1),k+36)
            double sums88=0.0;
            for (int z = 0; z < 8; ++z) sums88+=s8_[k+36][z];
            sumk=(sums88-synck)/7.0;
            if (synck>sumk)
            {
                nsyncscore2++;
                double del0 = sumk;
                if (del0==0.0) del0=0.000001;
                scoreratio2+=(synck/del0);
            }
            if (rrxdt<=2.13)
            {
                synck=s8_[k+72][icos7_2[k]];//s8(icos7(k-1),k+72)
                sums88=0.0;
                for (int z = 0; z < 8; ++z) sums88+=s8_[k+72][z];
                sumk=(sums88-synck)/7.0;
                if (synck>sumk)
                {
                    nsyncscore3++;
                    double del0 = sumk;
                    if (del0==0.0) del0=0.000001;
                    scoreratio3+=(synck/del0);
                }
            }
        }
        nsyncscore=nsyncscore1+nsyncscore2+nsyncscore3;
        scoreratio=scoreratio1+scoreratio2+scoreratio3;
        //! hard sync sum - max is 21
        nsync=is1+is2+is3; //qDebug()<<"nsync="<<is1<<is2<<is3<<lcqcand;
        //! bail out
        rscq=0.0;
        if (lcqcand)//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
        {
            int ip =0;
            for (int k11 = 7; k11 < 16; ++k11)//s8(0:7,79)
            {//do k11=8,16
                ip=pomAll.maxloc_da_beg_to_end(s8_[k11],0,8);//ip=maxloc(s8(:,k11))
                if (k11<15)//if(k11.lt.16) then
                {
                    if (ip==0) rscq+=1.0;//if(ip(1)==1) rscq=rscq+1.0;
                }
                else
                {
                    if (ip==1) rscq+=1.0;//if(ip(1)==2) rscq=rscq+1.0;
                }
            }
            ip=pomAll.maxloc_da_beg_to_end(s8_[16],0,8);//ip=maxloc(s8(:,17))
            if (ip==0 || ip==1) rscq+=0.5;//if(ip(1).eq.1 .or. ip(1).eq.2) rscq=rscq+0.5
            ip=pomAll.maxloc_da_beg_to_end(s8_[26],0,8);
            if (ip==0 || ip==1) rscq+=0.5;//if(ip(1).eq.1 .or. ip(1).eq.2) rscq=rscq+0.5
            ip=pomAll.maxloc_da_beg_to_end(s8_[32],0,8);
            if (ip==2 || ip==3) rscq+=0.5;//if(ip(1).eq.3 .or. ip(1).eq.4) rscq=rscq+0.5
        } //qDebug()<<"---------nsync="<<is1<<is2<<is3<<lcqcand;
        if (lcqcand && nsync==4)
        {
            if (nsync+nsync2<12)
            {
                if (rscq<6.6)
                {
                    nbadcrc=1;
                    return;//goto end;
                }
            }
            lapcqonly=true;
        }
        else if (lcqcand && nsync==5)
        {
            if (nsync+nsync2<12)
            {
                if (rscq<6.6)
                {
                    nbadcrc=1;
                    return;//goto end;
                }
            }
            lapcqonly=true;
        }//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
        else if (lcqcand && nsync==6)
        {
            if (nsync+nsync2<11)
            {
                if (rscq<5.6)
                {
                    nbadcrc=1;
                    return;//goto end;
                }
            }
            lapcqonly=true;
        }
        else
        {
            if (nsync<7)//if (nsync<=8)
            {
                nbadcrc=1;
                return;//goto end;
            }
        }
        lskipnotap=false;
        if (!lapcqonly && nsync<11)//int nsmax[8];//(8)  double s81(0:7)
        {
            for (int z = 0; z < 8; ++z) nsmax[z]=0;
            for (int k = 0; k < 7; ++k)
            {//do k=1,7
                for (int z = 0; z < 8; ++z) s81[z]=s8_[k][z];//s81=s8(:,k)//include 'syncdist.f90'
                syncdist(s81,nsmax,k);
                for (int z = 0; z < 8; ++z) s81[z]=s8_[k+36][z];//s81=s8(:,k+36)//include 'syncdist.f90'
                syncdist(s81,nsmax,k);
                for (int z = 0; z < 8; ++z) s81[z]=s8_[k+72][z];//s81=s8(:,k+72)//include 'syncdist.f90'
                syncdist(s81,nsmax,k);
            }
            int sum01=nsmax[6]+nsmax[7];
            int sum02=nsmax[1]+nsmax[2];
            int sum03=nsmax[4]+nsmax[5];//int sum04=nsmax[6]+nsmax[7];
            //if(sum(nsmax(7:8))>sum(nsmax(2:3)) || sum(nsmax(5:6))>sum(nsmax(2:3))) lskipnotap=true;
            if (sum01>sum02 || sum03>sum01) lskipnotap=true;
        }//qDebug()<<lskipnotap<<lskipnotap;
        //qDebug()<<"---2---nsync="<<is1<<is2<<is3;
        //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
        /*!    if(lcqcand .and. nsync.lt.7 .and. nsync.gt.1) then
        !      if(nsync+nsync2.lt.9) then
        !        rscq=0.
        !        do k11=8,16
        !          ip=maxloc(s8(:,k11))
        !          if(k11.lt.16) then; if(ip(1).eq.1) rscq=rscq+1.; else; if(ip(1).eq.2) rscq=rscq+1.; endif
        !        enddo
        !        ip=maxloc(s8(:,17)); if(ip(1).eq.1 .or. ip(1).eq.2) rscq=rscq+0.5
        !        ip=maxloc(s8(:,27)); if(ip(1).eq.1 .or. ip(1).eq.2) rscq=rscq+0.5
        !        ip=maxloc(s8(:,33)); if(ip(1).eq.3 .or. ip(1).eq.4) rscq=rscq+0.5
        !        if(rscq.lt.4.9) then; nbadcrc=1; return; endif
        !      endif
        !    else
        !      if(nsync.lt.7) then; nbadcrc=1; return; endif
        !    endif*/
        if (nsyncscore>0) scoreratio/=nsyncscore;
        else scoreratio=0.0;
        if (nsyncscore1>0) scoreratio1/=nsyncscore1;
        else scoreratio1=0.0;
        if (nsyncscore3>0) scoreratio3/=nsyncscore3;
        else scoreratio3=0.0;
        //qDebug()<<scoreratio<<scoreratio1<<scoreratio3<<dfqso<<rrxdt;
        if (dfqso>=2.0 || (dfqso<2.0 && stophint))//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
        {
            if (rrxdt>=-0.5 && rrxdt<=2.13)
            {
                if (nsyncscore<8 || (nsyncscore<10 && scoreratio<5.5) || (nsyncscore<11 && scoreratio<3.63))
                {
                    nbadcrc=1;
                    return;//goto end;//return //! 377 out of 20709
                }
                else if (nsyncscore==11 && scoreratio<5.37)
                {
                    if (nsyncscore1<5 && nsyncscore3<5 && scoreratio1<4.2 && scoreratio3<4.2)
                    {
                        nbadcrc=1;
                        return;//goto end;//return ! 261
                    }
                }
                else if (nsyncscore==12 && scoreratio<4.6)
                {
                    if (nsyncscore1<5 && nsyncscore3<5 && scoreratio1<4.0 && scoreratio3<4.0)
                    {
                        nbadcrc=1;
                        return;//goto end;//return ! 222
                    }
                }
                else if (nsyncscore==13 && scoreratio<4.4)
                {
                    if (nsyncscore1<5 && nsyncscore2<6 && nsyncscore3<5 && scoreratio1<4.4 && scoreratio3<4.4)
                    {
                        nbadcrc=1;
                        return;//goto end;//return ! 98
                    }
                }
                else if (nsyncscorew<3)//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
                {
                    if ((nsyncscore1>5 && scoreratio1>13.8) || (nsyncscore2>5 && scoreratio2>13.8) || (nsyncscore3>5 && scoreratio3>13.8)) goto c32;
                    nbadcrc=1;
                    return;//goto end;//return ! 75
                }
                else if (nsyncscorew==3)
                {
                    if (scoreratio1>15.0 || scoreratio2>15.0 || scoreratio3>15.0) goto c32;
                    nbadcrc=1;
                    return;//goto end;//return ! 125
                }
                else if (nsyncscorew==4)
                {
                    if (nsyncscore1==7 || nsyncscore2==7 || nsyncscore3==7 || scoreratio1>10.0 || scoreratio2>10.0 || scoreratio3>10.0) goto c32;
                    nbadcrc=1;
                    return;//goto end;//return ! 94
                }
                else if (nsyncscorew==5)
                {
                    if (nsyncscore>17 || nsyncscore1==7 || nsyncscore2==7 || nsyncscore3==7 || scoreratio1>10.0 || scoreratio2>10.0 || scoreratio3>10.0) goto c32;
                    nbadcrc=1;
                    return;//goto end;//return ! 131
                }
            }//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
            else if (rrxdt<-0.5)
            {
                if (nsyncscore<6 || (nsyncscore>5 && nsyncscore<8 && nsyncscorew<6 && scoreratio2<5.5 && scoreratio3<5.5))
                {
                    nbadcrc=1;
                    return;//goto end; //46
                }
                else if (nsyncscore==8)
                {
                    if (nsyncscore2<6 && nsyncscore3<6 && scoreratio2<6.6 && scoreratio3<6.6)
                    {
                        nbadcrc=1;
                        return;//goto end; //20
                    }
                }
                else if (nsyncscore==9 && scoreratio<6.0)
                {
                    if (nsyncscore2<6 && nsyncscore3<6 && scoreratio2<6.6 && scoreratio3<6.5)
                    {
                        nbadcrc=1;
                        return;//goto end; //5
                    }
                }
                else if (nsyncscorew<3)
                {
                    if ((nsyncscore2>5 && scoreratio2>13.8) || (nsyncscore3>5 && scoreratio3>13.8)) goto c32;
                    nbadcrc=1;
                    return;//goto end; //22
                }
                else if (nsyncscorew==3)
                {
                    if (scoreratio2>15.0 || nsyncscore3>15) goto c32;
                    nbadcrc=1;
                    return;//goto end; //31
                }
                else if (nsyncscorew==4)
                {
                    if (nsyncscore2==7 || nsyncscore3==7 || scoreratio2>10.0 || nsyncscore3>10) goto c32;
                    nbadcrc=1;
                    return;//goto end; //34
                }
                else if (nsyncscorew==5)
                {
                    if (nsyncscore>11 || nsyncscore2==7 || nsyncscore3==7 || scoreratio2>10.0 || scoreratio3>10.0) goto c32;
                    nbadcrc=1;
                    return;//goto end; //35
                }
            }
            else if (rrxdt>2.13)//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
            {
                if (nsyncscore<6 || (nsyncscore>5 && nsyncscore<8 && nsyncscorew<6 && scoreratio1<5.5 && scoreratio2<5.5))
                {
                    nbadcrc=1;
                    return;//goto end;// 4
                }
                else if (nsyncscore==8)
                {
                    if (nsyncscore1<6 && nsyncscore2<6 && scoreratio1<6.6 && scoreratio2<6.6)
                    {
                        nbadcrc=1;
                        return;//goto end;// 8
                    }
                }
                else if (nsyncscore==9 && scoreratio<6.0)
                {
                    if (nsyncscore1<6 && nsyncscore2<6 && scoreratio2<6.6 && scoreratio1<6.5)
                    {
                        nbadcrc=1;
                        return;//goto end;// 2
                    }
                }
                else if (nsyncscorew<3)
                {
                    if ((nsyncscore1>5 && scoreratio1>13.8) || (nsyncscore2>5 && scoreratio2>13.8)) goto c32;
                    nbadcrc=1;
                    return;//goto end;// 12
                }
                else if (nsyncscorew==3)
                {
                    if (scoreratio1>15.0 || scoreratio2>15.0) goto c32;
                    nbadcrc=1;
                    return;//goto end;// 32
                }
                else if (nsyncscorew==4)
                {
                    if (nsyncscore1==7 || nsyncscore2==7 || scoreratio1>10.0 || nsyncscore2>10) goto c32;
                    nbadcrc=1;
                    return;//goto end;// 103
                }
                else if (nsyncscorew==5)
                {
                    if (nsyncscore>11 || nsyncscore1==7 || nsyncscore2==7 || scoreratio1>10.0 || scoreratio2>10.0) goto c32;
                    nbadcrc=1;
                    return;//goto end;// 0
                }
            }
        }
c32:
        if (lsd)
        {
            if (iqso==4 && !ldeepsync) goto c64;
            //if (msgd.contains("CQ JH1PXH")) qDebug()<<"OUT ft8sd1var -->"<<msgd<<msg37<<lcq;//qDebug()<<"IN ft8sd1var -->"<<msgd;
            ft8sd1var(s8_,itone,msgd,msg37,lft8sd,lcq);//Tested OK HV
            if (lft8sd)
            {   //if (msgd.contains("CQ JH1PXH")) qDebug()<<"OUT ft8sd1var -->"<<msgd<<msg37;
                if (!f_eve0_odd1) evencopy[isd].lstate=false;
                else oddcopy[isd].lstate=false;
                i3=1;
                n3=1;
                iaptype2=iaptype;
                iaptype=0;
                nbadcrc=0;
                lsd=false;
                goto c2;
            }
c64:
            if (iqso==4)
            {
                if (!lcq)
                {
                    ft8mf1var(s8_,itone,msgd,msg37,lft8sd);//Tested OK HV
                    if (lft8sd)
                    {
                        if (!f_eve0_odd1) evencopy[isd].lstate=false;
                        else oddcopy[isd].lstate=false;
                        i3=1;
                        n3=1;
                        iaptype2=iaptype;
                        iaptype=0;
                        nbadcrc=0;
                        lsd=false;
                        goto c2;
                    }
                }
                else
                {
                    ft8mfcqvar(s8_,itone,msgd,msg37,lft8sd);//OK Tested HV
                    if (lft8sd)
                    {
                        if (!f_eve0_odd1) evencopy[isd].lstate=false;
                        else oddcopy[isd].lstate=false;
                        i3=1;
                        n3=1;
                        iaptype2=iaptype;
                        iaptype=0;
                        nbadcrc=0;
                        lsd=false;
                        goto c2;
                    }
                }
            }
        }
        if (iqso==4)
        {
            nbadcrc=1;
            goto c2;
        }
        synclev=0.0;
        snoiselev=1.0;
        for (int k = 0; k < 7; ++k)
        {//do k=1,7
            synclev+=s8_[k+36][icos7_2[k]];//synclev=synclev+s8(icos7(k-1),k+36)
        }//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
        sums8t=0.0;//hv
        for (int k = 0; k < 8; ++k)
        {
            for (int z = 0; z < 7; ++z) sums8t+=s8_[z+36][k];//(0:7,37:43)
        }
        snoiselev=(sums8t-synclev)/7.0;
        if (snoiselev<0.1) snoiselev=1.0; //! safe division
        srr=synclev/snoiselev; //qDebug()<<srr;
        /*!  SNR   srr range  average srr
        	! -18 1.8...4.0  2.9
        	! -19 1.7...3.6  2.65
        	! -20 1.6...3.3  2.43
        	! -21 1.6...3.0  2.22
        	! -22 1.55...2.8 2.19
        	! -23 1.4...2.6  2.03
        	! -24            1.94*/
c8:
        if (iqso>1 && iqso<4)
        {
            if (!lqsomsgdcd && !(!lmycallstd && !lhiscallstd))
            {
                if (!lft8sdec && dfqso<2.0)
                {   //qDebug()<<"iqso"<<iqso<<lft8sdec<<dfqso<<lqsomsgdcd;
                    if (lvirtual2 || lvirtual3) srr=0.0;
                    lft8s=false; //qDebug()<<"0 IN ft8svar<--";
                    ft8svar(s82_,srr,itone,msg37,lft8s,nft8rxfsens,stophint);//OK Tested HV
                    if (lft8s)
                    {
                        if (msg37.indexOf('<')>-1) lhashmsg=true;
                        nbadcrc=0;
                        lft8sdec=true;
                        lsdone=true; //qDebug()<<"0 OUT ft8svar -->"<<msg37<<iaptype2;
                        goto c2; //! i3=16 n3=16, any affect?
                    }
                }
            }
            lsdone=true;
            nbadcrc=1;
            continue;
        }
        int i1a=ibest+224; //! 7*32
        for (int z = 0; z < 256; ++z) csymb256[z]=cd0[z+i1a+cd_off]*ctwk256[z];//csymb256[256] (i1:i1+255)
        f2a.four2a_c2c(csymb256,256,-1,1,decid);//call four2avar(csymb256,256,1,-1,1)
        for (int z = 0; z < 9; ++z) s256[z]=cabs(csymb256[z]);//s256[](0:8)=abs(csymb256(1:9)) double s256[9];
        rscq=0.0;
        int nmic=0;
        for (int k11 = 7; k11 < 16; ++k11)
        {//do k11=8,16 //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
            int ip=pomAll.maxloc_da_beg_to_end(s8_[k11],0,8);//int ip=maxloc(s8(:,k11))
            if (ip==idtonemyc[k11-7]) nmic++;//if(ip(1).eq.idtonemyc(k11-7)+1) nmic++;  idtonemyc(58)
            if (k11<15)//if(k11.lt.16) then
            {
                if (ip==0) rscq+=1.0;//if(ip(1).eq.1) rscq=rscq+1.0;
            }
            else
            {
                if (ip==1) rscq+=1.0;//if(ip(1).eq.2) rscq=rscq+1.0;
            }
        }
        int ip=pomAll.maxloc_da_beg_to_end(s8_[16],0,8);//ip=maxloc(s8(:,17))
        if (ip==0 || ip==1) rscq+=0.5;
        ip=pomAll.maxloc_da_beg_to_end(s8_[26],0,8);//ip=maxloc(s8(:,27))
        if (ip==0 || ip==2) rscq+=0.5;
        ip=pomAll.maxloc_da_beg_to_end(s8_[32],0,8);//ip=maxloc(s8(:,33))
        if (ip==2 || ip==3) rscq+=0.5;
        bool lqsosig=false;
        //bool lqsosigtype3=false;
        bool lqso73=false;
        bool lqsorr73=false;//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
        bool lqsorrr=false; //! 73/rr73/rrr for std/nonstd callsigns
        if (!lqsomsgdcd && (dfqso<napwid || fabs(nftx-f1)<napwid) && lapmyc && s_MyCall8.count()>2)
        {
            int nqsot=0;
            for (int i = 0; i < 19; ++i)
            {//do i=1,19
                ip=pomAll.maxloc_da_beg_to_end(s8_[i+7],0,8);//ip=maxloc(s8(:,i+7))
                if (ip==idtone56[0][i]) nqsot++;//(1,i)+1)
            }
            if (nqsot>6) lqsosig=true; //! decoding depth only
            for (int i = 19; i < 22; ++i)
            {//do i=20,22
                ip=pomAll.maxloc_da_beg_to_end(s8_[i+7],0,8);//ip=maxloc(s8(:,i+7))
                if (ip==idtone56[0][i]) nqsot++;//(1,i)+1)
            }
            //if (nqsot>3) lqsosigtype3=true;
            int nqsoend[3];
            for (int z = 0; z < 3; ++z) nqsoend[z]=0;// ! array 73,rr73,rrr
            if (dfqso<napwid && (nQSOProgress==3 || nQSOProgress==4))
            {
                //! QSO RX freq only
                for (int i = 23; i < 58; ++i)
                {//do i=24,58
                    if (i<29) ip=pomAll.maxloc_da_beg_to_end(s8_[i+7],0,8);//
                    else ip=pomAll.maxloc_da_beg_to_end(s8_[i+14],0,8);//ip=maxloc(s8(:,i+14))
                    if (ip==idtone56[55][i]) nqsoend[0]++;//(56,i)+1)
                    if (ip==idtone56[54][i]) nqsoend[1]++;//(55,i)+1
                    if (ip==idtone56[53][i]) nqsoend[2]++;//(54,i)+1)
                }//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
                ip=pomAll.maxloc_ia_beg_to_end(nqsoend,0,3);//ip=maxloc(nqsoend)
                if (nqsoend[ip]>6)
                {
                    if (ip==0) lqso73=true;
                    else if (ip==1) lqsorr73=true;
                    else if (ip==2) lqsorrr=true;
                }
            }
        }
        bool lcqsignal=false;
        ip=pomAll.maxloc_da_beg_to_end(s256,0,9);//ip(1)=maxloc(s256,1)
        if (ip==4 || rscq>3.1) lcqsignal=true;
        if (!lcqsignal && (ip==3 || ip==5))//if(.not.lcqsignal .and. ip(1).eq.4 .or. ip(1).eq.6) then
        {
            for (int z = 0; z < 9; ++z) s2563[z]=s256[z];//s2563(0:8)=s256(0:8)
            for (int z = 0; z < 18; ++z) s2563[z+8]=cabs(csymb256[z+8]);//s2563(9:26)=fabs(csymb256(9:26))//s2563[27](0:26),
            ip=pomAll.maxloc_da_beg_to_end(s2563,0,27);//ip(1)=maxloc(s2563,1)
            if (ip==3 || ip==5) lcqsignal=true;
        }
        bool lmycsignal=false;
        if (lapmyc && nmic>2) lmycsignal=true;
        bool ldxcsig=false;
        bool lcqdxcsig=false;
        bool lcqdxcnssig=false;
        int ndxt=0;
        if (lhiscallstd)
        {
            for (int k11 = 16; k11 < 26; ++k11)
            {//do k11=17,26
                ip=pomAll.maxloc_da_beg_to_end(s8_[k11],0,8);//ip=maxloc(s8(:,k11))
                if (ip==idtone56[0][k11-7]) ndxt++;//if(ip(1).eq.idtone56(1,k11-7)+1) ndxt=ndxt+1
            }//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
            if (ndxt>3) ldxcsig=true;
            if (lcqsignal && ldxcsig) lcqdxcsig=true;
        }//! nonstd DXCall search in idle mode
        if (!lhiscallstd && s_HisCall8.count()>2)// then ! nonstandard DXCall
        {
            int ncqdxcnst=0;
            for (int i = 0; i < 4; ++i)
            {//do i=1,4
                ip=pomAll.maxloc_da_beg_to_end(s8_[i+7],0,8);//ip=maxloc(s8(:,i+7))
                if (ip==idtonecqdxcns[i]) ncqdxcnst++;//idtonecqdxcns(58)
            }
            ndxt=0;
            for (int i = 4; i < 23; ++i)
            {//do i=5,23
                ip=pomAll.maxloc_da_beg_to_end(s8_[i+7],0,8);//ip=maxloc(s8(:,i+7))
                if (ip==idtonedxcns73[i]) ndxt++;
                if (ip==idtonecqdxcns[i]) ncqdxcnst++;
            }
            ldxcsig=false;
            if (dfqso<napwid)
            {
                if (ndxt>4) ldxcsig=true; //! relaxed threshold for RXF napwid
                if (ncqdxcnst>5) lcqdxcnssig=true;
            }
            else
            {
                if (ndxt>5) ldxcsig=true;
                if (ncqdxcnst>6) lcqdxcnssig=true;
            }
        }
        //bool lfoxstdr73=false;//HV stop hound
        //bool lfoxspecrpt=false;//HV stop hound
        int nfoxspecrpt=0; //! checking fo special message with report to mybcall
        if (lhound)
        {
            //int nfoxstdbase=0; //! checking for report signal and base of RR73 signal
            //int nfoxspecr73=0; //! checking fo special message with RR73 to mybcall
            //int nfoxr73=0; //! checking for RR73 signal with standard message
            double fdelta=fabs(f1-nfqso);
            double fdeltam=fmod(fdelta,60.0);//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
            if (fdelta<245.0 && fdeltam<3.0 && (nQSOProgress==1 || nQSOProgress==3)) //! calculate on possible Fox frequencies only
            {
                int nfoxstdbase=0; //! checking for report signal and base of RR73 signal
                int nfoxspecr73=0; //! checking fo special message with RR73 to mybcall
                //int nfoxr73=0; //! checking for RR73 signal with standard message
                for (int i = 0; i < 18; ++i)
                {//do i=1,18
                    ip=pomAll.maxloc_da_beg_to_end(s8_[i+7],0,8);//ip=maxloc(s8(:,i+7))
                    if (ip==idtonefox73[i]) nfoxstdbase++;//=nfoxstdbase+1
                    if (i>9 && ip==idtonespec[i]) nfoxspecrpt++;//=nfoxspecrpt+1
                }
                for (int i = 19; i < 22; ++i)
                {//do i=20,22 //! hash10
                    ip=pomAll.maxloc_da_beg_to_end(s8_[i+7],0,8);//ip=maxloc(s8(:,i+7))
                    if (ip==idtonespec[i])
                    {
                        nfoxspecrpt++;
                        nfoxspecr73++;
                    }
                }
                //for (int z = 0; z < 8; ++z) tpom[z]=s8[z][z+(24+7)];
                ip=pomAll.maxloc_da_beg_to_end(s8_[(24+7)],0,8);//ip=maxloc(s8(:,25+7)) //! i3,n3
                if (ip==idtonespec[24])//error
                {
                    nfoxspecrpt++;
                    nfoxspecr73++;
                }
                /*double rspecstdrpt;//HV stop hound
                if (nfoxstdbase==0) rspecstdrpt=((double)nfoxspecrpt*18.0)/(1.2);
                else rspecstdrpt=((double)nfoxspecrpt*18.0)/(12.0*(double)nfoxstdbase);
                if (rspecstdrpt>1.0) lfoxspecrpt=true;*/
                if (nQSOProgress==3)
                {
                    int nfoxr73=0;
                    for (int i = 23; i < 58; ++i)
                    {//do i=24,58 //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
                        if (i<29) ip=pomAll.maxloc_da_beg_to_end(s8_[i+7],0,8);//ip=maxloc(s8(:,i+7))
                        else ip=pomAll.maxloc_da_beg_to_end(s8_[i+14],0,8);//ip=maxloc(s8(:,i+14))
                        if (ip==idtonefox73[i]) nfoxr73++;
                    }
                    /*double rstdr73;//HV stop hound
                    if (nfoxspecr73==0) rstdr73=((double)nfoxr73*4.0)/(3.5);
                    else rstdr73=(nfoxr73*4.0)/(35.0*(double)nfoxspecr73);
                    if (rstdr73>1.0) lfoxstdr73=true;*/
                }
            }
        }
        bool lsubptxfreq=false;//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
        if (lapmyc && fabs(f1-nftx)<2.0 && !lhound && !lft8sdec && !lqsomsgdcd && ((!lskiptx1 && nlasttx==1) || (lskiptx1 && nlasttx==2))) lsubptxfreq=true;
        int nweak=1;
        if (lft8subpass || dfqso<2.0 || lsubptxfreq) nweak=2;
        int nsubpasses=nweak;//if (ipass==3)
        if (lcqsignal)
        {
            nsubpasses=3;
            if (!f_eve0_odd1)
            {
                for (int ik = 0; ik < numcqsig; ++ik)
                {//do ik=1,numcqsig
                    if (evencq[ik].freq>5001.0) break;//if(evencq(ik,nthr)%freq>5001.0) break;
                    if (fabs(evencq[ik].freq-f1)<2.0 && fabs(evencq[ik].xdt-xdt)<0.05)
                    {
                        nsubpasses=5;
                        for (int z = 0; z < 79; ++z)
                        {
                            for (int z0 = 0; z0 < 8; ++z0) csold_[z][z0]=evencq[ik].cs_[z][z0];
                        }
                    }
                } //qDebug()<<nsubpasses<<nweak<<numcqsig;
            }
            else// if (loddint)
            {
                for (int ik = 0; ik < numcqsig; ++ik)
                {//do ik=1,numcqsig
                    if (oddcq[ik].freq>5001.0) break;
                    if (fabs(oddcq[ik].freq-f1)<2.0 && fabs(oddcq[ik].xdt-xdt)<0.05)
                    {
                        nsubpasses=5;
                        for (int z = 0; z < 79; ++z)
                        {
                            for (int z0 = 0; z0 < 8; ++z0) csold_[z][z0]=oddcq[ik].cs_[z][z0];
                        }
                    }
                }
            }
        }
        if (lmycsignal && lmycallstd)
        {
            nsubpasses=6;
            if (!f_eve0_odd1)
            {
                for (int ik = 0; ik < nummycsig; ++ik)
                {//do ik=1,numcqsig
                    if (evenmyc[ik].freq>5001.0) break;//if(evencq(ik,nthr)%freq>5001.0) break;
                    if (fabs(evenmyc[ik].freq-f1)<2.0 && fabs(evenmyc[ik].xdt-xdt)<0.05)
                    {
                        nsubpasses=5;
                        for (int z = 0; z < 79; ++z)
                        {
                            for (int z0 = 0; z0 < 8; ++z0) csold_[z][z0]=evenmyc[ik].cs_[z][z0];
                        }
                    }
                }
            }
            else// if (loddint)
            {
                for (int ik = 0; ik < nummycsig; ++ik)
                {//do ik=1,numcqsig
                    if (oddmyc[ik].freq>5001.0) break;//if(evencq(ik,nthr)%freq>5001.0) break;
                    if (fabs(oddmyc[ik].freq-f1)<2.0 && fabs(oddmyc[ik].xdt-xdt)<0.05)
                    {
                        nsubpasses=5;
                        for (int z = 0; z < 79; ++z)
                        {
                            for (int z0 = 0; z0 < 8; ++z0) csold_[z][z0]=oddmyc[ik].cs_[z][z0];
                        }
                    }
                }
            }

        }
        bool lqsocandave=false;//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
        if (lapmyc && ndxt>2 && nmic>2 && !lqsomsgdcd && lmycallstd && lhiscallstd && dfqso<napwid/2.0)
        {
            lqsocandave=true;
            nsubpasses=9;
            if (!f_eve0_odd1)
            {
                if (fabs(evenqso[0].freq-f1)<2.0 && fabs(evenqso[0].xdt-xdt)<0.05)
                {
                    nsubpasses=11;
                    for (int z = 0; z < 79; ++z)
                    {
                        for (int z0 = 0; z0 < 8; ++z0) csold_[z][z0]=evenqso[0].cs_[z][z0];
                    }
                }
            }
            else// if (loddint)
            {
                if (fabs(oddqso[0].freq-f1)<2.0 && fabs(oddqso[0].xdt-xdt)<0.05)
                {
                    nsubpasses=11;
                    for (int z = 0; z < 79; ++z)
                    {
                        for (int z0 = 0; z0 < 8; ++z0) csold_[z][z0]=oddqso[0].cs_[z][z0];
                    }
                }
            }
        }
        for (int isubp1 = 1; isubp1 <= nsubpasses; ++isubp1)//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
        {//do isubp1=1,nsubpasses
            if (nweak==1 && isubp1==2) continue;
            if (isubp1>2 && isubp1<6 && lmycsignal) continue;//! skip if it is lmycsignal, can be both lcq and lmy
            syncavemax=3.0;
            if (isubp1==2)
            {
                for (int z = 0; z < 79; ++z)
                {
                    for (int z0 = 0; z0 < 8; ++z0) cs_[z][z0]=csr_[z][z0];
                }
            }
            //if(ipass==npass-1 && (lcqsignal || lmycsignal) && ((nweak==1 && isubp1==1) || (nweak==2 && isubp1==2)))
            if (ipass==npass-1 && (lcqsignal || lmycsignal) && ((nweak==1 && isubp1==1) || (nweak==2 && isubp1==2)))
            {
                for (int z = 0; z < 79; ++z)
                {
                    for (int z0 = 0; z0 < 8; ++z0) cstmp2_[z][z0]=cs_[z][z0];
                }
            }
            if (ipass==npass && lapmyc && ndxt>2 && nmic>2 && ((nweak==1 && isubp1==1) || (nweak==2 && isubp1==2)))
            {
                for (int z = 0; z < 79; ++z)
                {
                    for (int z0 = 0; z0 < 8; ++z0) cstmp2_[z][z0]=cs_[z][z0];
                }
            }

            for (int nsym = 1; nsym <= 3; ++nsym)
            {//do nsym=1,3
                int nt=pow(2,(3*nsym));//nt=2**(3*nsym)-1
                for (int ihalf = 1; ihalf <= 2; ++ihalf)
                {//do ihalf=1,2
                    for (int k = 1; k <= 29; k+=nsym)
                    {//do k=1,29,nsym
                        int ks=0;
                        if (ihalf==1) ks=k+7;
                        else ks=k+43;
                        ks=ks-1;//hv
                        int ks1=ks+1;
                        int ks2=ks+2;
                        for (int i = 0; i < nt; ++i)
                        {//do i=0,nt
                            int i1=i/64;
                            int i2=(i & 63)/8;
                            int i33=(i & 7);//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
                            if (isubp1<3)
                            {
                                //if(nsym.eq.1) s2(i)=abs(cs(graymap(i33),ks))
                                //else if(nsym.eq.2) s2(i)=abs(cs(graymap(i2),ks)+cs(graymap(i33),ks1))
                                //else s2(i)=abs(cs(graymap(i1),ks)+cs(graymap(i2),ks1) + cs(graymap(i33),ks2))
                                if      (nsym==1) s2[i]=cabs(cs_[ks][graymap[i33]]);
                                else if (nsym==2) s2[i]=cabs(cs_[ks][graymap[i2]]+cs_[ks1][graymap[i33]]);
                                else              s2[i]=cabs(cs_[ks][graymap[i1]]+cs_[ks1][graymap[i2]]+cs_[ks2][graymap[i33]]);
                            }
                            else if (isubp1==3 || isubp1==6 || isubp1==9)
                            {
                                //if(nsym==1) s2(i)=abs(cscs(graymap(i33),ks))**2 + abs(csr(graymap(i33),ks))**2
                                //else if(nsym==2) s2(i)=abs(cscs(graymap(i2),ks) + cscs(graymap(i33),ks1))**2 + abs(csr(graymap(i2),ks)+csr(graymap(i33),ks1))**2
                                //else s2(i)=abs(cscs(graymap(i1),ks) + cscs(graymap(i2),ks1)+cscs(graymap(i33),ks2))**2 + abs(csr(graymap(i1),ks)+csr(graymap(i2),ks1) + csr(graymap(i33),ks2))**2
                                if (nsym==1)
                                {
                                    double d0=cabs(cscs_[ks][graymap[i33]]);
                                    double d1=cabs( csr_[ks][graymap[i33]]);
                                    s2[i]=(d0*d0)+(d1*d1);
                                }
                                else if (nsym==2)
                                {
                                    double d0=cabs(cscs_[ks][graymap[i2]]+cscs_[ks1][graymap[i33]]);
                                    double d1=cabs( csr_[ks][graymap[i2]]+ csr_[ks1][graymap[i33]]);
                                    s2[i]=(d0*d0)+(d1*d1);
                                }
                                else
                                {
                                    double d0=cabs(cscs_[ks][graymap[i1]]+cscs_[ks1][graymap[i2]]+cscs_[ks2][graymap[i33]]);
                                    double d1=cabs( csr_[ks][graymap[i1]]+ csr_[ks1][graymap[i2]]+ csr_[ks2][graymap[i33]]);
                                    s2[i]=(d0*d0)+(d1*d1);
                                }
                            }
                            else if (isubp1==4 || isubp1==7 || isubp1==10)
                            {
                                //if(nsym==1) s2(i)=abs(cs(graymap(i33),ks))**2 + abs(csold(graymap(i33),ks))**2
                                //else if(nsym==2) s2(i)=abs(cs(graymap(i2),ks)+cs(graymap(i33),ks1))**2 + abs(csold(graymap(i2),ks)+csold(graymap(i33),ks1))**2
                                //else s2(i)=abs(cs(graymap(i1),ks)+cs(graymap(i2),ks1) + cs(graymap(i33),ks2))**2 + abs(csold(graymap(i1),ks)+csold(graymap(i2),ks1) + csold(graymap(i33),ks2))**2
                                if (nsym==1)
                                {
                                    double d0=cabs(   cs_[ks][graymap[i33]]);
                                    double d1=cabs(csold_[ks][graymap[i33]]);
                                    s2[i]=(d0*d0)+(d1*d1);
                                }
                                else if (nsym==2)
                                {
                                    double d0=cabs(   cs_[ks][graymap[i2]]+   cs_[ks1][graymap[i33]]);
                                    double d1=cabs(csold_[ks][graymap[i2]]+csold_[ks1][graymap[i33]]);
                                    s2[i]=(d0*d0)+(d1*d1);
                                }

                                else
                                {
                                    double d0=cabs(   cs_[ks][graymap[i1]]+   cs_[ks1][graymap[i2]]+   cs_[ks2][graymap[i33]]);
                                    double d1=cabs(csold_[ks][graymap[i1]]+csold_[ks1][graymap[i2]]+csold_[ks2][graymap[i33]]);
                                    s2[i]=(d0*d0)+(d1*d1);
                                }
                            }
                            else if (isubp1==5 || isubp1==8 || isubp1==11)
                            {
                                //if(nsym.eq.1) s2(i)=abs(cs(graymap(i33),ks))+abs(csold(graymap(i33),ks))
                                //else if(nsym.eq.2) s2(i)=abs(cs(graymap(i2),ks)+cs(graymap(i33),ks1)) + abs(csold(graymap(i2),ks)+csold(graymap(i33),ks1))
                                //else s2(i)=abs(cs(graymap(i1),ks)+cs(graymap(i2),ks1) + cs(graymap(i33),ks2)) + abs(csold(graymap(i1),ks)+csold(graymap(i2),ks1) + csold(graymap(i33),ks2))
                                if      (nsym==1) s2[i]=cabs(cs_[ks][graymap[i33]])+cabs(csold_[ks][graymap[i33]]);
                                else if (nsym==2) s2[i]=cabs(   cs_[ks][graymap[i2]]+   cs_[ks1][graymap[i33]])
                                                            +cabs(csold_[ks][graymap[i2]]+csold_[ks1][graymap[i33]]);
                                else              s2[i]=cabs(   cs_[ks][graymap[i1]]+   cs_[ks1][graymap[i2]]+   cs_[ks2][graymap[i33]])
                                                            +cabs(csold_[ks][graymap[i1]]+csold_[ks1][graymap[i2]]+csold_[ks2][graymap[i33]]);
                            }
                            if (isubp1==1 && srr<2.5) //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
                            {
                                if (srr>2.3) s2[i]=s2[i]*s2[i];//!  srr.lt.2.5 -19dB SNR threshold
                                else
                                {
                                    double ss1=s2[i];//s2(i)=1 + 8.*ss1**2 - 0.12*ss1**4
                                    if (ss1<5.77) s2[i]=1.0 + (8.0*ss1)*(8.0*ss1) - pow((0.12*ss1),4);
                                    else s2[i]=(ss1+5.82)*(ss1+5.82);
                                }
                            }
                            if (isubp1>1 && srr<2.5) s2[i]=pow((0.5*s2[i]),3); //! -19dB SNR threshold
                            //s2[i]*=0.01;
                            //if (qIsInf(s2[i])) qDebug()<<"1 INF"<<isubp1<<srr;
                            //if (qIsNaN(s2[i])) qDebug()<<"1 NAN"<<isubp1<<srr;
                        }
                        int i32=(k-1)*3+(ihalf-1)*87;  //??? i32=1+(k-1)*3+(ihalf-1)*87
                        int ibmax=0;
                        if (nsym==1) ibmax=2;
                        if (nsym==2) ibmax=5;
                        if (nsym==3) ibmax=8;
                        for (int ib = 0; ib <= ibmax; ++ib)
                        {//do ib=0,ibmax
                            //bm=maxval(s2(0:nt-1),one(0:nt-1,ibmax-ib)) - maxval(s2(0:nt-1),.not.one(0:nt-1,ibmax-ib))
                            double max1v=0.0;
                            for (int zz = 0; zz < nt; ++zz)
                            {
                                if (one_ft8_2[ibmax-ib][zz]==true)
                                {
                                    double tmax1v=s2[zz];
                                    if (tmax1v>max1v) max1v=tmax1v;
                                }
                            }
                            double max2v=0.0;
                            for (int zz = 0; zz < nt; ++zz)
                            {
                                if (one_ft8_2[ibmax-ib][zz]==false)
                                {
                                    double tmax2v=s2[zz];
                                    if (tmax2v>max2v) max2v=tmax2v;
                                }
                            }
                            double bm=max1v-max2v; //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
                            //if (max1v==0.0 || max2v==0.0) qDebug()<<max1v<<max2v;
                            if (i32+ib>173) continue;//cycle //if(i32+ib .gt.174) cycle
                            //if (qIsNaN(bm)) qDebug()<<"bm NAN"<<isubp1;
                            if (nsym==1)
                            {
                                bmeta[i32+ib]=bm; //qDebug()<<"0-173"<<i32+ib;
                                //den=max(maxval(s2(0:nt-1),one(0:nt-1,ibmax-ib)), maxval(s2(0:nt-1),.not.one(0:nt-1,ibmax-ib)))
                                double den = max1v;
                                if (max2v > max1v) den = max2v;
                                double cm = 0.0;
                                if (den>0.0) cm=bm/den; //if(den.gt.0.0) then
                                else cm=0.0; //! erase it
                                bmetd[i32+ib]=cm;
                            }
                            else if (nsym==2) bmetb[i32+ib]=bm;//bmetb(i32+ib)=bm
                            else if (nsym==3) bmetc[i32+ib]=bm;//bmetc(i32+ib)=bm
                        }//qDebug()<<"------"<<bmeta[10]<<bmeta[30]<<bmeta[50]; return;
                    } //! k
                } //! ihalf
            }// ! nsym
            //!tests
            //!call indexx(bmetc(1:174),174,indx)
            //!src=abs(bmetc(indx(1))/bmetc(indx(174)))
            //!print *,src
            /*double temp[5];
            double bmete[174];
        	for (int i = 0; i < 174; ++i)
        	{
            	temp[0]=bmeta[i];
            	temp[1]=bmetb[i];
            	temp[2]=bmetc[i];
            	int ip = 0;
            	double max = fabs(temp[0]);
            	for (int i0 = 1; i0 < 3; ++i0)
            	{
                	double max1 = fabs(temp[i0]);
                	if (max1>max)
                	{
                    	ip = i0;
                    	max = max1;
                	}
            	}
            	bmete[i]=temp[ip];
        	}
        	pomFt.normalizebmetvar(bmete,174);*/                      
            pomFt.normalizebmetvar(bmeta,174);
            pomFt.normalizebmetvar(bmetb,174);
            pomFt.normalizebmetvar(bmetc,174);
            pomFt.normalizebmetvar(bmetd,174);

            double scalefac=2.83;//83;//1.53 1.83 hv  2.83;
            double maxval_llra_abs = 0.0;
            double maxval_llrb_abs = 0.0;
            double maxval_llrc_abs = 0.0;
            double maxval_llrd_abs = 0.0;
            double maxv_abs;
            //double mina=0.0; double maxa=0.0; double minb=0.0; double maxb=0.0;
            //double minc=0.0; double maxc=0.0; double mind=0.0; double maxd=0.0;
            for (int z = 0; z < 174; ++z)
            {
                llra[z]=scalefac*bmeta[z];
                llrb[z]=scalefac*bmetb[z];
                llrc[z]=scalefac*bmetc[z];
                llrd[z]=scalefac*bmetd[z];
                maxv_abs = fabs(llra[z]);
                if (maxv_abs>maxval_llra_abs) maxval_llra_abs=maxv_abs;
                maxv_abs = fabs(llrb[z]);
                if (maxv_abs>maxval_llrb_abs) maxval_llrb_abs=maxv_abs;
                maxv_abs = fabs(llrc[z]);
                if (maxv_abs>maxval_llrc_abs) maxval_llrc_abs=maxv_abs;
                maxv_abs = fabs(llrd[z]);
                if (maxv_abs>maxval_llrd_abs) maxval_llrd_abs=maxv_abs;
                /*if (llra[z]<mina) mina=llra[z];
                if (llra[z]>maxa) maxa=llra[z]; 
                if (llrb[z]<minb) minb=llrb[z];  
                if (llrb[z]>maxb) maxb=llrb[z]; 
                if (llrc[z]<minc) minc=llrc[z];  
                if (llrc[z]>maxc) maxc=llrc[z]; 
                if (llrd[z]<mind) mind=llrd[z];  
                if (llrd[z]>maxd) maxd=llrd[z];*/
            }
            //double apmag = ((maxval_llra_abs+maxval_llrb_abs+maxval_llrc_abs+maxval_llrd_abs)/4.0);
            maxv_abs = maxval_llra_abs;
            if (maxv_abs<maxval_llrb_abs) maxv_abs=maxval_llrb_abs;
            if (maxv_abs<maxval_llrc_abs) maxv_abs=maxval_llrc_abs;
            if (maxv_abs<maxval_llrd_abs) maxv_abs=maxval_llrd_abs;
            double apmag = maxv_abs*1.01;
            //if (apmag<0.01) break;
            //static int hhh=1; 
            //if (apmag<3.0) {printf(" ipass= %d iqso= %d isubp1= %d val= %.7f\n",ipass,iqso,isubp1,apmag); break;}
            //double apmag = maxval_llra_abs*1.01;
            //printf(" isubp1= %d a= %.3f b= %.3f c= %.3f d= %.3f max= %.3f\n",isubp1,maxval_llra_abs,maxval_llrb_abs,maxval_llrc_abs,maxval_llrd_abs,maxv_abs);
            //double apmagb = maxval_llrb_abs*1.01;
            //double apmagc = maxval_llrc_abs*1.01;
            //double apmagd = maxval_llrd_abs*1.01;
            //if (maxa>1.5 || maxb>1.5 || maxc>1.5 || maxd>1.5 ) printf("MG=%1.2f   A=%1.2f  B=%1.2f  C=%1.2f  D=%1.2f\n",apmag,maxa,maxb,maxc,maxd);

            /*! isubp2 #
            !------------------------------
            !   1        regular decoding, nsym=1
            !   2        regular decoding, nsym=2
            !   3        regular decoding, nsym=3
            !   4        regular decoding, llrd
            !   5..18    ap passes

            ! iaptype Hound OFF, MyCall is standard, DXCall is standard or empty
            !------------------------
            !   0        cycle
            !   1        CQ     ???    ???           (29+3=32 ap bits)
            !   2        MyCall ???    ???           (29+3=32 ap bits)
            !   3        MyCall DxCall ???           (58+3=61 ap bits)
            !   4        MyCall DxCall RRR           (77 ap bits)
            !   5        MyCall DxCall 73            (77 ap bits)
            !   6        MyCall DxCall RR73          (77 ap bits)

            ! naptypes(nQSOProgress, extra decoding pass)
            !                                  1 1 1 1 1 1 1 1 1 1 2 2 2 2  2  2  2  2  2  2  3  3
            !                        5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3  4  5  6  7  8  9  0  1
            !  data naptypes(0,1:27)/3,3,3,0,0,0,0,0,0,0,0,0,2,2,2,1,1,1,31,31,31,36,36,36,35,35,35/ ! Tx6 CQ
            !  data naptypes(1,1:27)/3,3,3,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,31,31,31,36,36,36,35,35,35/ ! Tx1 Grid
            !  data naptypes(2,1:27)/3,3,3,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,31,31,31,36,36,36,35,35,35/ ! Tx2 Report
            !  data naptypes(3,1:27)/3,3,3,6,6,6,5,5,5,4,4,4,0,0,0,0,0,0,31,31,31,36,36,36,35,35,35/ ! Tx3 RRreport
            !  data naptypes(4,1:27)/3,3,3,6,6,6,5,5,5,4,4,4,2,2,2,0,0,0,31,31,31,36,36,36,35,35,35/ ! Tx4 RRR,RR73
            !  data naptypes(5,1:27)/0,0,0,0,0,0,0,0,0,0,0,0,2,2,2,1,1,1,31,31,31,36,36,36,35,35,35/ ! Tx5 73

            ! iaptype standard DxCall tracking, also valid in Hound mode
            !------------------------
            !   31        CQ  DxCall Grid(???)     (77 ap bits)
            !   35        ??? DxCall 73            (29+19 ap bits)
            !   36        ??? DxCall RR73          (29+19 ap bits)

            ! iaptype Hound off, MyCall is nonstandard, DXCall is standard or empty
            !------------------------
            !   0         cycle
            !   1         CQ     ???    ???        (29+3=32 ap bits)
            !   40       <MyCall> ???  ???         (29+3=32 ap bits) incoming call
            !   41       <MyCall> DxCall ???       (58 ap bits) REPORT/RREPORT
            !   42        MyCall <DxCall> RRR      (77 ap bits)
            !   43        MyCall <DxCall> 73       (77 ap bits)
            !   44        MyCall <DxCall> RR73     (77 ap bits)
            !   31        CQ  DxCall Grid(???)     (77 ap bits) standard DxCall tracking
            !   35        ??? DxCall 73            (29+19 ap bits) standard DxCall tracking
            !   36        ??? DxCall RR73          (29+19 ap bits) standard DxCall tracking

            ! nmycnsaptypes(nQSOProgress, extra decoding pass)
            !  data nmycnsaptypes(0,1:27)/0,0,0,0,0,0,0,0,0,0,0,0,40,40,40,1,1,1,31,31,31,36,36,36,35,35,35/             ! Tx6 CQ
            !  data nmycnsaptypes(1,1:27)/41,41,41,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,31,31,31,36,36,36,35,35,35/             ! Tx1 DXcall MyCall
            !  data nmycnsaptypes(2,1:27)/41,41,41,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,31,31,31,36,36,36,35,35,35/             ! Tx2 Report
            !  data nmycnsaptypes(3,1:27)/41,41,41,44,44,44,43,43,43,42,42,42,0,0,0,0,0,0,31,31,31,36,36,36,35,35,35/    ! Tx3 RRreport
            !  data nmycnsaptypes(4,1:27)/41,41,41,44,44,44,43,43,43,42,42,42,40,40,40,0,0,0,31,31,31,36,36,36,35,35,35/ ! Tx4 RRR,RR73
            !  data nmycnsaptypes(5,1:27)/0,0,0,0,0,0,0,0,0,0,0,0,40,40,40,1,1,1,31,31,31,36,36,36,35,35,35/             ! Tx5 73

            ! iaptype Hound off, MyCall is standard, DXCall is not empty and is nonstandard
            !------------------------
            !   0         cycle
            !   1         CQ     ???    ???            (29+3=32 ap bits)
            !   2         MyCall ???    ???            (29+3=32 ap bits)
            !   11        MyCall <DxCall> ???          (58 ap bits) REPORT/RREPORT
            !   12       <MyCall> DxCall RRR           (77 ap bits)
            !   13       <MyCall> DxCall 73            (77 ap bits)
            !   14       <MyCall> DxCall RR73          (77 ap bits)
            !   31        CQ  DxCall                   (77 ap bits) ! full compound or just nonstandard callsign
            !   35        ??? DxCall 73                (64 ap bits) ! full compound or just nonstandard callsign
            !   36        ??? DxCall RR73              (64 ap bits) ! full compound or just nonstandard callsign

            ! ndxnsaptypes(nQSOProgress, extra decoding pass)
            !  data ndxnsaptypes(0,1:27)/0,0,0,0,0,0,0,0,0,0,0,0,2,2,2,1,1,1,31,31,31,36,36,36,35,35,35/             ! Tx6 CQ
            !  data ndxnsaptypes(1,1:27)/11,11,11,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,31,31,31,36,36,36,35,35,35/          ! Tx1 Grid
            !  data ndxnsaptypes(2,1:27)/11,11,11,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,31,31,31,36,36,36,35,35,35/          ! Tx2 Report
            !  data ndxnsaptypes(3,1:27)/11,11,11,14,14,14,13,13,13,12,12,12,0,0,0,0,0,0,31,31,31,36,36,36,35,35,35/ ! Tx3 RRreport
            !  data ndxnsaptypes(4,1:27)/11,11,11,14,14,14,13,13,13,12,12,12,2,2,2,0,0,0,31,31,31,36,36,36,35,35,35/ ! Tx4 RRR,RR73
            !  data ndxnsaptypes(5,1:27)/0,0,0,0,0,0,0,0,0,0,0,0,2,2,2,1,1,1,31,31,31,36,36,36,35,35,35/             ! Tx5 73

            ! iaptype Hound mode
            !------------------------
            !    0        cycle
            !    1        CQ     ???    ???            (29+3=32 ap bits) standard callsign
            !   21        MyBaseCall DxBaseCall ???    (58+3=61 ap bits) Report
            !   22        ??? RR73; MyCall <???> ???   (28+6=34 ap bits)
            !   23        MyBaseCall DxBaseCall RR73   (77 ap bits)
            !   24        MyCall RR73; ??? <???> ???   (28+6=34 ap bits)
            !   31        CQ  DxCall (DXGrid)          (77 ap bits) ! standard/full compound or just nonstandard callsign
            !   36        ??? DxCall RR73              (29+19/64 ap bits) ! standard/ full compound or just nonstandard callsign
            !  111        CQ     ???                   (3 last i3 bits) type 4 message with nonstandard callsign

            ! nhaptypes(nQSOProgress, extra decoding pass)
            !  data nhaptypes(0,1:27)/0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,111,111,111/       ! Tx6 CQ, possible in idle mode if DXCall is empty
            !  data nhaptypes(1,1:27)/21,21,21,22,22,22,0,0,0,0,0,0,0,0,0,31,31,31,0,0,0,36,36,36,0,0,0/ ! Tx1 Grid idle mode or transmitting
            !  data nhaptypes(2,1:27)/0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0/             ! Tx2 none
            !  data nhaptypes(3,1:27)/21,21,21,22,22,22,23,23,23,24,24,24,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0/ ! Tx3 RRreport QSO in progress or QSO is finished
            !  data nhaptypes(4,1:27)/0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0/             ! Tx4 none
            !  data nhaptypes(5,1:27)/0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0/             ! Tx5 none*/
            bool loutapwid=false;
            if (fabs(f1-nfqso)>napwid && fabs(f1-nftx)>napwid) loutapwid=true;
            double scqnr=2.0;
            double smycnr=2.0;
            /*!  print*,'stophint is ',stophint,' lft8sdec is ',lft8sdec
            !  print*,'lnohiscall is ',lnohiscall,' loutapwid is ',loutapwid
            !  print*,'lenabledxcsearch ',lenabledxcsearch, ' lwidedxcsearch is ',lwidedxcsearch
            !  print*,'lcqdxcsig is', lcqdxcsig
            !  print*,'lcqsignal is',lcqsignal
            !  print*,'ldxcsig',ldxcsig
            !  print*,'lhiscallstd',lhiscallstd
            !  print*,'mycall ',mycall,mybcall
            !  print*,'hiscall ',hiscall,hisbcall
            !  print*,'hisgrid is ',hisgrid*/
            for (int isubp2 = 1; isubp2 <= 31; ++isubp2)
            {//do isubp2=1,31
            	//if (cont_type>=5) continue;//HV max cont_type=4
                if (isubp2<5)
                {
                    if (lapcqonly || lskipnotap) continue;
                    if (ltxing)
                    {//! enabled Tx and transmitted message including CQ at last interval
                        if (fabs(f1-nfqso)<3.0) //! +- 3Hz sync8var QSOfreq candidate list
                        {
                            if (syncavemax<1.8) continue;
                        }
                        else
                        {
                            if (syncavemax<1.9) continue;
                        }
                    }
                    else
                    {
                        if (syncavemax<1.8) continue;
                    }
                }
                if (isubp2==4) continue;//HV stop c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
                if (isubp1>2 && isubp2<5) continue;//! skip regular decoding for extra subpasses
                if (lqsocandave)
                {
                    if (isubp1>2 && isubp1<9) continue;//! skip other extra subpasses if QSO signal, highest priority
                    if (lqsomsgdcd) continue;
                }
                else if (lmycsignal && lmycallstd)
                {
                    if (isubp1>2 && isubp1<6) continue;//! skip CQ signal extra subpasses if MyCall signal
                }
                if (isubp2<5)
                {
                    for (int z = 0; z < 174; ++z) apmask[z]=0;
                    iaptype2=iaptype;
                    iaptype=0;
                    if (isubp2==1)
                    {
                        if (ipass==1)
                        {
                            for (int z = 0; z < 174; ++z) llrz[z]=llrd[z];//llrz=llrd
                        }
                        else
                        {
                            for (int z = 0; z < 174; ++z) llrz[z]=llra[z];//llrz=llra
                        }
                        if (isubp1>1 && ipass>1)
                        {
                            for (int z = 0; z < 174; ++z) llrz[z]=llrd[z];//llrz=llrd
                        }
                    }
                    else if (isubp2==2)
                    {
                        for (int z = 0; z < 174; ++z) llrz[z]=llrb[z];//llrz=llrb
                        if (isubp1>1) for (int z = 0; z < 174; ++z) llrz[z]=llra[z];//llrz=llra
                    }
                    else if (isubp2==3)
                    {
                        for (int z = 0; z < 174; ++z) llrz[z]=llrc[z];//llrz=llrc
                    }
                    else if (isubp2==4)
                    {
                        for (int z = 0; z < 174; ++z) llrz[z]=llrd[z];//llrz=llrd
                    }
                    /*double maxval_llra_abs = fabs(llrz[0]);
                    for (int z = 1; z < 174; ++z)
                    {
                        double llra_abs = fabs(llrz[z]);//ws300rc1 stop
                        if (llra_abs>maxval_llra_abs) maxval_llra_abs=llra_abs;
                    }
                    apmag = maxval_llra_abs*1.01;*/
                }
                else
                {
                	//if (apmag<5.0) {printf(" isubp2= %d val= %.7f\n",isubp2,apmag); break;}
                    if (!lhound)
                    {
                        //if (!s_lapon8) continue;//for the moment hv
                        if (lmycallstd && (lhiscallstd || lnohiscall))//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
                        {
                            iaptype=naptypes[nQSOProgress][isubp2-5];
                            iaptype2=iaptype;
                            //!  print*,'line 1357 iaptype,isubp1,isubp2 = ',iaptype,isubp1,isubp2
                            if (iaptype==0) continue;
                            if (lqsomsgdcd && iaptype>2 && iaptype<31) continue;//! QSO message already decoded
                            if (iaptype==2)
                            {
                                if (!lapmyc || lapcqonly) continue;//! skip AP for 'mycall ???? ????' in 2..3 minutes after last TX
                                if (nQSOProgress!=0 && nmic<2) continue;// ! reduce CPU load at QSO
                            }
                            //if(!stophint && iaptype>30) continue;//! no DXCall searching at QSO, reduce Lag
                            //if(stophint && iaptype>2 && iaptype<31) continue;//
                            if (lft8sdec && iaptype>2) continue;//! QSO message already decoded
                            if (iaptype>2 && lnohiscall) continue;//! no DXCall
                            if (iaptype>2 && iaptype<31 && loutapwid) continue;//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
                            //if(iaptype==3 && !lqsosigtype3) continue; //! not QSO signal
                            //if(iaptype==4 && !lqsorrr) continue; //! not RRR signal
                            //if(iaptype==5 && !lqso73) continue; //! not 73 signal
                            //if(iaptype==6 && !lqsorr73) continue; //! not RR73 signal
                            if (iaptype>30 && !lenabledxcsearch) continue;//! in QSO or TXing CQ or last logged is DX Call: searching disabled
                            if (iaptype>30 && !lwidedxcsearch && loutapwid) continue;//! only RX freq DX Call searching
                            if (iaptype==31 && !lcqdxcsig) continue;//! not CQ signal from std DXCall
                            if (iaptype==31 && !lhiscallstd && lapcqonly) continue;//! skip weak CQ signals with std call
                            if (iaptype>31 && lapcqonly) continue;//! skip weak CQ signals
                            if (iaptype==35 && !lqso73) continue;//! DXCall searching, not 73 signal
                            if (iaptype==36 && !lqsorr73) continue;//! DXCall searching, not RR73 signal
                            if (lqsocandave && isubp1>8 && (iaptype<3 || iaptype>6)) continue;//! QSO signal
                            //if(!lqsocandave && lmycsignal && isubp1>5 && isubp1<9 && iaptype!=2) continue; //! skip other AP if lmycsignal extra subpasses)
                            //if (iaptype>31) printf("iaptype=%d nQSOProgress=%d\n",iaptype,nQSOProgress);
                            if (iaptype==2 && apsym_[0]>1) continue;
                            if (iaptype==3 && apsym_[29]>1) continue;
                            if (iaptype==31 && apcqsym_[0]>1) continue;
                            //if (iaptype==35 && apsym_[0]>1) continue;//2
                            //if (iaptype==36 && apsym_[0]>1) continue;//2
                            if (iaptype==1)
                            {
                                if (isubp2==20)
                                {
                                    double scqlev=0.0;//do i4=1,9//idtone25[25][58];//(25,58)
                                    for (int i4 = 0; i4 < 9; ++i4) scqlev+=s8_[i4+7][idtone25[1][i4]];//scqlev=scqlev+s8(idtone25(2,i4),i4+7)
                                    sums8t=0.0;
                                    for (int k = 0; k < 8; ++k)//double s8_[79][8];
                                    {
                                        for (int z = 0; z < 9; ++z) sums8t+=s8_[z+7][k];//(0:7,8:16))
                                    }
                                    snoiselev=(sums8t-scqlev)/7.0;
                                    double del0=snoiselev;
                                    if (del0==0.0) del0=0.000001;
                                    scqnr=scqlev/del0;
                                    if (scqnr<1.0 && !lcqsignal) continue;//
                                }
                                if (isubp2==21)// || isubp2==22
                                {
                                    if (lft8lowth || lft8subpass)
                                    {
                                        if (scqnr<1.2 && !lcqsignal) continue;//
                                    }
                                    if (!lft8subpass && !lft8lowth)
                                    {
                                        if (scqnr<1.3 && !lcqsignal) continue;//
                                    }
                                }
                            }
                            if (iaptype==2)
                            {
                                if (isubp2==17)
                                {
                                    double smyclev=0.0;//do i4=1,9
                                    for (int i4 = 0; i4 < 9; ++i4) smyclev+=s8_[i4+7][idtonemyc[i4]];
                                    sums8t=0.0;
                                    for (int k = 0; k < 8; ++k)//double s8_[79][8];
                                    {
                                        for (int z = 0; z < 9; ++z) sums8t+=s8_[z+7][k];//(0:7,8:16))
                                    }
                                    snoiselev=(sums8t-smyclev)/7.0;
                                    double del0=snoiselev;
                                    if (del0==0.0) del0=0.000001;
                                    smycnr=smyclev/del0;
                                    if (smycnr<1.0 && !lmycsignal) continue;//
                                }
                                if (isubp2==18)
                                {
                                    if (lft8lowth || lft8subpass)
                                    {
                                        if (smycnr<1.2 && !lmycsignal) continue;//
                                    }
                                }
                            }
                            if (iaptype==3)
                            {
                                if (isubp2==5)
                                {
                                    double smyclev=0.0;//do i4=1,9
                                    for (int i4 = 0; i4 < 9; ++i4) smyclev+=s8_[i4+7][idtonemyc[i4]];
                                    sums8t=0.0;
                                    for (int k = 0; k < 8; ++k)//double s8_[79][8];
                                    {
                                        for (int z = 0; z < 9; ++z) sums8t+=s8_[z+7][k];//(0:7,8:16))
                                    }
                                    snoiselev=(sums8t-smyclev)/7.0;
                                    double del0=snoiselev;
                                    if (del0==0.0) del0=0.000001;
                                    smycnr=smyclev/del0;
                                    if (smycnr<1.0) continue;//
                                }
                                else if (isubp2==6)
                                {
                                    if (smycnr<1.2) continue;//
                                }
                            }
                            for (int z = 0; z < 174; ++z) apmask[z]=0; //for (int z = 0; z < 174; ++z) llrz[z]=llra[z];;
                            if (iaptype==1) //! CQ or CQ RU or CQ TEST or CQ FD
                            {
                                for (int z = 0; z < 174; ++z)
                                {
                                    if (isubp2==20) llrz[z]=llrc[z];
                                    else if (isubp2==21) llrz[z]=llrb[z];
                                    else if (isubp2==22) llrz[z]=llra[z];
                                    if (z<29)
                                    {
                                        apmask[z]=1;
                                        llrz[z]=apmag*(double)mcq_ft8_2[z];
                                    }
                                }
                                apmask[74]=1;  //apmask(75:77)=1
                                apmask[75]=1;
                                apmask[76]=1;
                                llrz[74]=apmag*(-1.0);//llrd(75:76)=apmag*(-1)
                                llrz[75]=apmag*(-1.0);//llrd(75:76)=apmag*(-1)
                                llrz[76]=apmag*(+1.0);//llrd(77)=apmag*(+1)
                            }
                            else if (iaptype==2) //! MyCall,???,???
                            {
                                for (int z = 0; z < 174; ++z)//3*ND 174
                                {
                                    if (isubp2==17) llrz[z]=llrc[z];
                                    else if (isubp2==18) llrz[z]=llrb[z];
                                    else if (isubp2==19) llrz[z]=llra[z];
                                }
                                if (cont_type==0 || cont_type==1)//hv new || cont_type==5
                                {
                                    for (int z = 0; z < 29; ++z)
                                    {
                                        apmask[z]=1;//apmask(1:29)=1
                                        llrz[z]=apmag*(double)apsym_[z];//llrd(1:29)=apmag*apsym(1:29)
                                    }
                                    apmask[74]=1;  //apmask(75:77)=1
                                    apmask[75]=1;
                                    apmask[76]=1;
                                    llrz[74]=apmag*(-1.0);//llrd(75:76)=apmag*(-1)
                                    llrz[75]=apmag*(-1.0);//llrd(75:76)=apmag*(-1)
                                    llrz[76]=apmag*(+1.0);//llrd(77)=apmag*(+1)
                                }
                                else if (cont_type==2) //then
                                {
                                    for (int z = 0; z < 28; ++z)
                                    {
                                        apmask[z]=1;//apmask(1:28)=1
                                        llrz[z]=apmag*(double)apsym_[z];//llrd(1:28)=apmag*apsym(1:28)
                                    }
                                    apmask[71]=1;//apmask(72:74)=1
                                    apmask[72]=1;
                                    apmask[73]=1;
                                    llrz[71]=apmag*(-1.0);//llrd(72)=apmag*(-1)
                                    llrz[72]=apmag*(+1.0);//llrd(73)=apmag*(+1)
                                    llrz[73]=apmag*(-1.0);//llrd(74)=apmag*(-1)
                                    apmask[74]=1;//apmask(75:77)=1
                                    apmask[75]=1;
                                    apmask[76]=1;
                                    llrz[74]=apmag*(-1.0);//llrd(75:77)=apmag*(-1)
                                    llrz[75]=apmag*(-1.0);
                                    llrz[76]=apmag*(-1.0);
                                }
                                else if (cont_type==3)
                                {
                                    for (int z = 0; z < 28; ++z)
                                    {
                                        apmask[z]=1;  //apmask(1:28)=1
                                        llrz[z]=apmag*(double)apsym_[z];//llrd(1:28)=apmag*apsym(1:28)
                                    }
                                    apmask[74]=1; //apmask(75:77)=1
                                    apmask[75]=1;
                                    apmask[76]=1;
                                    llrz[74]=apmag*(-1.0);//llrd(75:77)=apmag*(-1)
                                    llrz[75]=apmag*(-1.0);
                                    llrz[76]=apmag*(-1.0);
                                }
                                else if (cont_type==4)// || ncontest==6  RTTY RU  HV new
                                {
                                    for (int z = 1; z < 29; ++z)
                                    {
                                        apmask[z]=1;//apmask(2:29)=1
                                        llrz[z]=apmag*(double)apsym_[z-1];//llrd(2:29)=apmag*apsym(1:28)
                                    }
                                    apmask[74]=1;//apmask(75:77)=1
                                    apmask[75]=1;
                                    apmask[76]=1;
                                    llrz[74]=apmag*(-1.0);//llrd(75)=apmag*(-1)
                                    llrz[75]=apmag*(+1.0);//llrd(76:77)=apmag*(+1)
                                    llrz[76]=apmag*(+1.0);
                                }
                            }
                            else if (iaptype==3) //then ! MyCall DxCall ???
                            {
                                for (int z = 0; z < 174; ++z)//3*ND 174
                                {
                                    if (isubp2==5) llrz[z]=llrc[z];
                                    else if (isubp2==6) llrz[z]=llrb[z];
                                    else if (isubp2==7) llrz[z]=llra[z];
                                }
                                if (cont_type==0 || cont_type==1 || cont_type==2)//||ncontest==7   || ncontest==5
                                {
                                    for (int z = 0; z < 58; ++z)
                                    {
                                        apmask[z]=1;//apmask(1:58)=1
                                        llrz[z]=apmag*(double)apsym_[z];//llrd(1:58)=apmag*apsym
                                    }
                                    apmask[74]=1;//apmask(75:77)=1
                                    apmask[75]=1;
                                    apmask[76]=1;
                                    llrz[74]=apmag*(-1.0);//llrd(75:76)=apmag*(-1)
                                    llrz[75]=apmag*(-1.0);
                                    llrz[76]=apmag*(+1.0);//llrd(77)=apmag*(+1)
                                }
                                else if (cont_type==3) //then ! Field Day
                                {
                                    for (int z = 0; z < 57; ++z)
                                    {
                                        if (z<56) apmask[z]=1;//apmask(1:56)=1
                                        if (z<28) llrz[z]=apmag*(double)apsym_[z];//llrd(1:28)=apmag*apsym(1:28)
                                        if (z>28) llrz[z-1]=apmag*(double)apsym_[z];//llrd(29:56)=apmag*apsym(30:57)
                                    }
                                    apmask[71]=1;//apmask(72:74)=1
                                    apmask[72]=1;
                                    apmask[73]=1;
                                    apmask[74]=1;//apmask(75:77)=1
                                    apmask[75]=1;
                                    apmask[76]=1;
                                    llrz[74]=apmag*(-1.0);//llrd(75:77)=apmag*(-1)
                                    llrz[75]=apmag*(-1.0);
                                    llrz[76]=apmag*(-1.0);
                                }
                                else if (cont_type==4)//RTTY RU  HV new   || ncontest==6
                                {
                                    for (int z = 0; z < 57; ++z)
                                    {
                                        if (z>0) apmask[z]=1;//apmask(2:57)=1
                                        if (z<28) llrz[z+1]=apmag*(double)apsym_[z];//llrd(2:29)=apmag*apsym(1:28)
                                        if (z>28) llrz[z]=apmag*(double)apsym_[z];//llrd(30:57)=apmag*apsym(30:57)
                                    }
                                    apmask[74]=1;//apmask(75:77)=1
                                    apmask[75]=1;
                                    apmask[76]=1;
                                    llrz[74]=apmag*(-1.0);//llrd(75)=apmag*(-1)
                                    llrz[75]=apmag*(+1.0);//llrd(76:77)=apmag*(+1)
                                    llrz[76]=apmag*(+1.0);
                                }
                            }
                            else if (iaptype>3 && iaptype<7)//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
                            {
                                for (int z = 0; z < 174; ++z)//3*ND 174
                                {
                                    if (isubp2==8 || isubp2==11 || isubp2==14) llrz[z]=llrc[z];
                                    else if (isubp2==9 || isubp2==12 || isubp2==15) llrz[z]=llrb[z];
                                    else if (isubp2==10 || isubp2==13 || isubp2==16) llrz[z]=llra[z];//llrb[z];
                                }
                                if (cont_type<=4)//HV new=4
                                {
                                    for (int z = 0; z < 77; ++z)
                                    {
                                        apmask[z]=1;//apmask(1:77)=1   //! mycall, hiscall, RRR|73|RR73
                                        if (z<58) llrz[z]=apmag*(double)apsym_[z];//llrd(1:58)=apmag*apsym
                                    }
                                    for (int z = 0; z < 19; ++z)
                                    {
                                        if (iaptype==4) llrz[z+58]=apmag*(double)mrrr_ft8_2[z]; //llrd(59:77)=apmag*mrrr
                                        if (iaptype==5) llrz[z+58]=apmag*(double)m73_ft8_2[z]; //llrd(59:77)=apmag*m73
                                        if (iaptype==6) llrz[z+58]=apmag*(double)mrr73_ft8_2[z];//llrd(59:77)=apmag*mrr73
                                    }
                                }
                            }
                            else if (iaptype==31) //then ! CQ DxCall Grid(???)
                            {
                                for (int z = 0; z < 174; ++z)
                                {
                                    if (isubp2==23) llrz[z]=llrc[z];
                                    else if (isubp2==24) llrz[z]=llrb[z];
                                    else if (isubp2==25) llrz[z]=llra[z];
                                }
                                if (lnohisgrid)
                                {
                                    for (int z = 0; z < 58; ++z)
                                    {
                                        apmask[z]=1;
                                        llrz[z]=apmag*(double)apcqsym_[z];
                                    }
                                    apmask[74]=1;
                                    apmask[75]=1;
                                    apmask[76]=1;
                                    llrz[74]=apmag*(-1.0);
                                    llrz[75]=apmag*(-1.0);
                                    llrz[76]=apmag*(+1.0);
                                }
                                else
                                {
                                    for (int z = 0; z < 77; ++z)
                                    {
                                        apmask[z]=1;
                                        llrz[z]=apmag*(double)apcqsym_[z];
                                    }
                                }
                            }
                            else if (iaptype==35) //then ! ??? DxCall 73, type 1 std DXCall
                            {   //qDebug()<<"AP35";
                                for (int z = 0; z < 174; ++z)
                                {
                                    if (isubp2==29) llrz[z]=llrc[z];
                                    else if (isubp2==30) llrz[z]=llrb[z];
                                    else if (isubp2==31) llrz[z]=llra[z];
                                }
                                for (int z = 29; z < 77; ++z)
                                {
                                    apmask[z]=1;
                                    if (z<58) llrz[z]=apmag*(double)apsym_[z];//llrz(30:58)=apmag*apsym(30:58)
                                }
                                for (int z = 0; z < 19; ++z) llrz[z+58]=apmag*(double)m73_ft8_2[z];//llrz(59:77)=apmag*m73
                            }
                            else if (iaptype==36) //then ! ??? DxCall RR73 type 1 std DXCall
                            {
                                for (int z = 0; z < 174; ++z)
                                {
                                    if (isubp2==26) llrz[z]=llrc[z];
                                    else if (isubp2==27) llrz[z]=llrb[z];
                                    else if (isubp2==28) llrz[z]=llra[z];
                                }
                                for (int z = 29; z < 77; ++z)
                                {
                                    apmask[z]=1;
                                    if (z<58) llrz[z]=apmag*(double)apsym_[z];//llrz(30:58)=apmag*apsym(30:58)
                                }
                                for (int z = 0; z < 19; ++z) llrz[z+58]=apmag*(double)mrr73_ft8_2[z];//llrz(59:77)=apmag*mrr73
                            }//!!  print*,'line 1533 iaptype,isubp1,isubp2 = ',iaptype,isubp1,isubp2
                            else continue;//<-remove hv
                        }
                        else if (lmycallstd && !lhiscallstd && s_HisCall8.count()>2) //then
                        {
                            iaptype=ndxnsaptypes[nQSOProgress][isubp2-5];//iaptype=ndxnsaptypes(nQSOProgress,isubp2-4)
                            iaptype2=iaptype;
                            if (iaptype==0) continue;
                            if (iaptype==2 && lapcqonly) continue;//! skip weak CQ signals
                            if (!stophint && iaptype>30) continue;//! no DXCall searching at QSO, reduce Lag
                            if ((lqsomsgdcd || !lapmyc) && iaptype>1 && iaptype<15)continue;//! skip AP for mycall in 2..3 minutes after last TX
                            if (iaptype==12 && !lqsorrr) continue;//! not RRR signal
                            if (iaptype==13 && !lqso73) continue;//! not 73 signal
                            if (iaptype==14 && !lqsorr73) continue;//! not RR73 signal
                            if (iaptype>30 && !lenabledxcsearch) continue;//! in QSO or TXing CQ or last logged is DX Call: searching disabled
                            if (iaptype>30 && !lwidedxcsearch && loutapwid) continue;//! only RX freq DX Call searching
                            if (iaptype>30 && lapcqonly) continue;//! skip weak CQ signals
                            if (iaptype==31 && !lcqdxcnssig) continue;//! it is not CQ signal of non-standard DXCall
                            if (iaptype==35 && !lqso73) continue;//! DXCall searching, not 73 signal
                            if (iaptype==36 && !lqsorr73) continue;//! DXCall searching, not RR73 signal
                            if (lqsocandave && isubp1>8 && (iaptype<11 || iaptype>14)) continue;//! QSO signal
                            if (iaptype>2 && iaptype<15 && loutapwid) continue;//
                            if (iaptype==2 && apsym_[0]>1) continue;
                            if (iaptype==3 && apsym_[29]>1) continue;
                            if (iaptype==11 && apsymdxns1_[0]>1) continue;
                            if (iaptype==12 && apsymdxnsrrr_[0]>1) continue;
                            if (iaptype==13 && apsymdxns732_[0]>1) continue;
                            if (iaptype==14 && apsymdxnsr73_[0]>1) continue;
                            if (iaptype==31 && apcqsym_[0]>1) continue;
                            if (iaptype==35 && apsymdxns73_[0]>1) continue;
                            if (iaptype==36 && apsymdxnsrr73_[0]>1) continue;
                            //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
                            if (iaptype==1)
                            {
                                if (isubp2==20)
                                {
                                    double scqlev=0.0;//do i4=1,9//idtone25[25][58];//(25,58)
                                    for (int i4 = 0; i4 < 9; ++i4) scqlev+=s8_[i4+7][idtone25[1][i4]];//scqlev=scqlev+s8(idtone25(2,i4),i4+7)
                                    sums8t=0.0;
                                    for (int k = 0; k < 8; ++k)//double s8_[79][8];
                                    {
                                        for (int z = 0; z < 9; ++z) sums8t+=s8_[z+7][k];//(0:7,8:16))
                                    }
                                    snoiselev=(sums8t-scqlev)/7.0;
                                    double del0=snoiselev;
                                    if (del0==0.0) del0=0.000001;
                                    scqnr=scqlev/del0;
                                    if (scqnr<1.0 && !lcqsignal) continue;//
                                }
                                if (isubp2==21)
                                {
                                    if (lft8lowth || lft8subpass)
                                    {
                                        if (scqnr<1.2 && !lcqsignal) continue;//
                                    }
                                    if (!lft8subpass && !lft8lowth)
                                    {
                                        if (scqnr<1.3 && !lcqsignal) continue;//
                                    }
                                }
                            }
                            if (iaptype==2)
                            {
                                if (isubp2==17)
                                {
                                    double smyclev=0.0;//do i4=1,9
                                    for (int i4 = 0; i4 < 9; ++i4) smyclev+=s8_[i4+7][idtonemyc[i4]];
                                    sums8t=0.0;
                                    for (int k = 0; k < 8; ++k)//double s8_[79][8];
                                    {
                                        for (int z = 0; z < 9; ++z) sums8t+=s8_[z+7][k];//(0:7,8:16))
                                    }
                                    snoiselev=(sums8t-smyclev)/7.0;
                                    double del0=snoiselev;
                                    if (del0==0.0) del0=0.000001;
                                    smycnr=smyclev/del0;
                                    if (smycnr<1.0 && !lmycsignal) continue;//
                                }
                                if (isubp2==18)
                                {
                                    if (lft8lowth || lft8subpass)
                                    {
                                        if (smycnr<1.2 && !lmycsignal) continue;//
                                    }
                                }
                            }
                            for (int z = 0; z < 174; ++z) apmask[z]=0;
                            if (iaptype==1) //! CQ or CQ RU or CQ TEST or CQ FD
                            {
                                for (int z = 0; z < 174; ++z)//3*ND 174
                                {
                                    if (isubp2==20) llrz[z]=llrc[z];
                                    else if (isubp2==21) llrz[z]=llrb[z];
                                    else if (isubp2==22) llrz[z]=llra[z];
                                    if (z<29)
                                    {
                                        apmask[z]=1;
                                        llrz[z]=apmag*(double)mcq_ft8_2[z];
                                    }
                                }
                                apmask[74]=1;  //apmask(75:77)=1
                                apmask[75]=1;
                                apmask[76]=1;
                                llrz[74]=apmag*(-1.0);//llrd(75:76)=apmag*(-1)
                                llrz[75]=apmag*(-1.0);//llrd(75:76)=apmag*(-1)
                                llrz[76]=apmag*(+1.0);//llrd(77)=apmag*(+1)
                                //qDebug()<<"AP-1 CQ";
                            }
                            else if (iaptype==2) //then //! MyCall,???,???
                            {
                                for (int z = 0; z < 174; ++z)//3*ND 174
                                {
                                    if (isubp2==17) llrz[z]=llrc[z];
                                    else if (isubp2==18) llrz[z]=llrb[z];
                                    else if (isubp2==19) llrz[z]=llra[z];
                                }
                                if (cont_type==0 || cont_type==1)//hv new || cont_type==5
                                {
                                    for (int z = 0; z < 29; ++z)
                                    {
                                        apmask[z]=1;//apmask(1:29)=1
                                        llrz[z]=apmag*(double)apsym_[z];//llrd(1:29)=apmag*apsym(1:29)
                                    }
                                    apmask[74]=1;  //apmask(75:77)=1
                                    apmask[75]=1;
                                    apmask[76]=1;
                                    llrz[74]=apmag*(-1.0);//llrd(75:76)=apmag*(-1)
                                    llrz[75]=apmag*(-1.0);//llrd(75:76)=apmag*(-1)
                                    llrz[76]=apmag*(+1.0);//llrd(77)=apmag*(+1)
                                }
                                else if (cont_type==2) //then
                                {
                                    for (int z = 0; z < 28; ++z)
                                    {
                                        apmask[z]=1;//apmask(1:28)=1
                                        llrz[z]=apmag*(double)apsym_[z];//llrd(1:28)=apmag*apsym(1:28)
                                    }
                                    apmask[71]=1;//apmask(72:74)=1
                                    apmask[72]=1;
                                    apmask[73]=1;
                                    llrz[71]=apmag*(-1.0);//llrd(72)=apmag*(-1)
                                    llrz[72]=apmag*(+1.0);//llrd(73)=apmag*(+1)
                                    llrz[73]=apmag*(-1.0);//llrd(74)=apmag*(-1)
                                    apmask[74]=1;//apmask(75:77)=1
                                    apmask[75]=1;
                                    apmask[76]=1;
                                    llrz[74]=apmag*(-1.0);//llrd(75:77)=apmag*(-1)
                                    llrz[75]=apmag*(-1.0);
                                    llrz[76]=apmag*(-1.0);
                                }
                                else if (cont_type==3)//then ! Field Day
                                {
                                    for (int z = 0; z < 28; ++z)
                                    {
                                        apmask[z]=1;  //apmask(1:28)=1
                                        llrz[z]=apmag*(double)apsym_[z];//llrd(1:28)=apmag*apsym(1:28)
                                    }
                                    apmask[74]=1; //apmask(75:77)=1
                                    apmask[75]=1;
                                    apmask[76]=1;
                                    llrz[74]=apmag*(-1.0);//llrd(75:77)=apmag*(-1)
                                    llrz[75]=apmag*(-1.0);
                                    llrz[76]=apmag*(-1.0);
                                }
                                else if (cont_type==4)// || ncontest==6  RTTY RU  HV new
                                {
                                    for (int z = 1; z < 29; ++z)
                                    {
                                        apmask[z]=1;//apmask(2:29)=1
                                        llrz[z]=apmag*(double)apsym_[z-1];//llrd(2:29)=apmag*apsym(1:28)
                                    }
                                    apmask[74]=1;//apmask(75:77)=1
                                    apmask[75]=1;
                                    apmask[76]=1;
                                    llrz[74]=apmag*(-1.0);//llrd(75)=apmag*(-1)
                                    llrz[75]=apmag*(+1.0);//llrd(76:77)=apmag*(+1)
                                    llrz[76]=apmag*(+1.0);
                                }
                            }
                            else if (iaptype==11)//then ! MyCall <DxCall> ???  ! report rreport type 1 msg
                            {
                                for (int z = 0; z < 174; ++z)//3*ND 174
                                {
                                    if (isubp2==5) llrz[z]=llrc[z];
                                    else if (isubp2==6) llrz[z]=llrb[z];
                                    else if (isubp2==7) llrz[z]=llra[z];
                                }
                                for (int z = 0; z < 58; ++z)//apmask(1:58)=1
                                {
                                    apmask[z]=1;
                                    llrz[z]=apmag*(double)apsymdxns1_[z];
                                }
                                apmask[74]=1;//apmask(75:77)=1;
                                apmask[75]=1;
                                apmask[76]=1;
                                llrz[74]=apmag*(-1.0);//llrz(75:76)=apmag*(-1.0)
                                llrz[75]=apmag*(-1.0);
                                llrz[76]=apmag*(+1.0);
                            }
                            else if (iaptype==12) //then  ! type 4 <MyCall> DxCall RRR
                            {
                                for (int z = 0; z < 174; ++z)//3*ND 174
                                {
                                    if (isubp2==14) llrz[z]=llrc[z];
                                    else if (isubp2==15) llrz[z]=llrb[z];
                                    else if (isubp2==16) llrz[z]=llra[z];
                                }
                                for (int z = 0; z < 77; ++z)
                                {
                                    apmask[z]=1;
                                    llrz[z]=apmag*(double)apsymdxnsrrr_[z];
                                }
                            }
                            else if (iaptype==13) //then  ! type 4 <MyCall> DxCall 73
                            {
                                for (int z = 0; z < 174; ++z)//3*ND 174
                                {
                                    if (isubp2==11) llrz[z]=llrc[z];
                                    else if (isubp2==12) llrz[z]=llrb[z];
                                    else if (isubp2==13) llrz[z]=llra[z];
                                }
                                for (int z = 0; z < 77; ++z)
                                {
                                    apmask[z]=1;
                                    llrz[z]=apmag*(double)apsymdxns732_[z];
                                }
                            }
                            else if (iaptype==14) //then  ! type 4 <MyCall> DxCall RR73
                            {
                                for (int z = 0; z < 174; ++z)//3*ND 174
                                {
                                    if (isubp2==8) llrz[z]=llrc[z];
                                    else if (isubp2==9) llrz[z]=llrb[z];
                                    else if (isubp2==10) llrz[z]=llra[z];
                                }
                                for (int z = 0; z < 77; ++z)
                                {
                                    apmask[z]=1;
                                    llrz[z]=apmag*(double)apsymdxnsr73_[z];
                                }
                            }
                            else if (iaptype==31) //then ! CQ  DxCall ! full compound or nonstandard
                            {
                                for (int z = 0; z < 174; ++z)
                                {
                                    if (isubp2==23) llrz[z]=llrc[z];
                                    else if (isubp2==24) llrz[z]=llrb[z];
                                    else if (isubp2==25) llrz[z]=llra[z];
                                }
                                for (int z = 0; z < 77; ++z)
                                {
                                    apmask[z]=1;
                                    llrz[z]=apmag*(double)apcqsym_[z];
                                }
                            }
                            else if (iaptype==35) //then ! ??? DxCall 73 ! full compound or nonstandard
                            {
                                for (int z = 0; z < 174; ++z)
                                {
                                    if (isubp2==29) llrz[z]=llrc[z];
                                    else if (isubp2==30) llrz[z]=llrb[z];
                                    else if (isubp2==31) llrz[z]=llra[z];
                                }
                                for (int z = 13; z < 77; ++z)
                                {
                                    apmask[z]=1;
                                    llrz[z]=apmag*(double)apsymdxns73_[z];
                                }
                            }
                            else if (iaptype==36) //then ! ??? DxCall RR73 ! full compound or nonstandard
                            {
                                for (int z = 0; z < 174; ++z)
                                {
                                    if (isubp2==26) llrz[z]=llrc[z];
                                    else if (isubp2==27) llrz[z]=llrb[z];
                                    else if (isubp2==28) llrz[z]=llra[z];
                                }
                                for (int z = 13; z < 77; ++z)
                                {
                                    apmask[z]=1;
                                    llrz[z]=apmag*(double)apsymdxnsrr73_[z];
                                }
                            }
                            else continue;
                        }
                        else if (!lmycallstd && !lhiscallstd && s_HisCall8.count()>2) //then ! empty mycall or compound/nonstandard both calls
                        {
                            iaptype=ndxnsaptypes[nQSOProgress][isubp2-5];//iaptype=ndxnsaptypes(nQSOProgress,isubp2-4)
                            iaptype2=iaptype;
                            if (iaptype==0) continue;//
                            if (iaptype>1 && iaptype<31) continue;//
                            if (!stophint && iaptype>1) continue;//! on air, QSO is not possible
                            if (iaptype>30 && lapcqonly) continue;//! skip weak CQ signals with std call
                            if (iaptype==31 && !lcqdxcnssig) continue;//! it is not CQ signal of non-standard DXCall
                            if (iaptype>34 && !ldxcsig) continue;//! not DXCall signal
                            if (iaptype==31 && apcqsym_[0]>1) continue;
                            if (iaptype==35 && apsymdxns73_[0]>1) continue;
                            if (iaptype==36 && apsymdxnsrr73_[0]>1) continue;
                            //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
                            if (iaptype==1)
                            {
                                if (isubp2==20)
                                {
                                    double scqlev=0.0;//do i4=1,9//idtone25[25][58];//(25,58)
                                    for (int i4 = 0; i4 < 9; ++i4) scqlev+=s8_[i4+7][idtone25[1][i4]];//scqlev=scqlev+s8(idtone25(2,i4),i4+7)
                                    sums8t=0.0;
                                    for (int k = 0; k < 8; ++k)//double s8_[79][8];
                                    {
                                        for (int z = 0; z < 9; ++z) sums8t+=s8_[z+7][k];//(0:7,8:16))
                                    }
                                    snoiselev=(sums8t-scqlev)/7.0;
                                    double del0=snoiselev;
                                    if (del0==0.0) del0=0.000001;
                                    scqnr=scqlev/del0;
                                    if (scqnr<1.0 && !lcqsignal) continue;//
                                }
                                if (isubp2==21)
                                {
                                    if (lft8lowth || lft8subpass)
                                    {
                                        if (scqnr<1.2 && !lcqsignal) continue;//
                                    }
                                    if (!lft8subpass && !lft8lowth)
                                    {
                                        if (scqnr<1.3 && !lcqsignal) continue;//
                                    }
                                }
                            }
                            for (int z = 0; z < 174; ++z) apmask[z]=0;
                            if (iaptype==1) //then ! CQ ??? ???
                            {
                                for (int z = 0; z < 174; ++z)//3*ND 174
                                {
                                    if (isubp2==20) llrz[z]=llrc[z];
                                    else if (isubp2==21) llrz[z]=llrb[z];
                                    else if (isubp2==22) llrz[z]=llra[z];
                                    if (z<29)
                                    {
                                        apmask[z]=1;
                                        llrz[z]=apmag*(double)mcq_ft8_2[z];
                                    }
                                }
                                apmask[74]=1;  //apmask(75:77)=1
                                apmask[75]=1;
                                apmask[76]=1;
                                llrz[74]=apmag*(-1.0);//llrd(75:76)=apmag*(-1)
                                llrz[75]=apmag*(-1.0);//llrd(75:76)=apmag*(-1)
                                llrz[76]=apmag*(+1.0);//llrd(77)=apmag*(+1)
                                //qDebug()<<"AP-1 CQ";
                                //! Nonstandard DXCall search masks below are for monitoring purpose in idle mode,
                                //! QSO is not possible
                                //! wideband searching being used by default
                            }
                            else if (iaptype==31) //then ! CQ  DxCall ! full compound or nonstandard
                            {
                                for (int z = 0; z < 174; ++z)
                                {
                                    if (isubp2==23) llrz[z]=llrc[z];
                                    else if (isubp2==24) llrz[z]=llrb[z];
                                    else if (isubp2==25) llrz[z]=llra[z];
                                }
                                for (int z = 0; z < 77; ++z)
                                {
                                    apmask[z]=1;
                                    llrz[z]=apmag*(double)apcqsym_[z];
                                }

                            }
                            else if (iaptype==35) //then ! ??? DxCall 73 ! full compound or nonstandard
                            {
                                for (int z = 0; z < 174; ++z)
                                {
                                    if (isubp2==29) llrz[z]=llrc[z];
                                    else if (isubp2==30) llrz[z]=llrb[z];
                                    else if (isubp2==31) llrz[z]=llra[z];
                                }
                                for (int z = 13; z < 77; ++z)
                                {
                                    apmask[z]=1;
                                    llrz[z]=apmag*(double)apsymdxns73_[z];
                                }
                            }
                            else if (iaptype==36) //then ! ??? DxCall RR73 ! full compound or nonstandard
                            {
                                for (int z = 0; z < 174; ++z)
                                {
                                    if (isubp2==26) llrz[z]=llrc[z];
                                    else if (isubp2==27) llrz[z]=llrb[z];
                                    else if (isubp2==28) llrz[z]=llra[z];
                                }
                                for (int z = 13; z < 77; ++z)
                                {
                                    apmask[z]=1;
                                    llrz[z]=apmag*(double)apsymdxnsrr73_[z];
                                }
                            }
                            else continue;
                        }
                        else if (!lmycallstd && (lhiscallstd || lnohiscall))
                        {
                            iaptype=nmycnsaptypes[nQSOProgress][isubp2-5];
                            iaptype2=iaptype;
                            if (iaptype==0) continue;//
                            if (isubp1==2 && nweak==1) continue;//
                            if (isubp1>5) continue;//! so far CQ averaging only
                            if (iaptype==40 && lapcqonly) continue;//! skip weak CQ signals
                            if (iaptype>40 && iaptype<45 && lqsomsgdcd) continue;//! already decoded
                            if (iaptype==42 && !lqsorrr) continue;//! not RRR signal
                            if (iaptype==43 && !lqso73) continue;//! not 73 signal
                            if (iaptype==44 && !lqsorr73) continue;//! not RR73 signal
                            if (iaptype>39 && !lapmyc) continue;//
                            if (lnomycall && iaptype>39 && iaptype<45) continue;//! skip QSO signals if mycall is empty
                            if (lnohiscall && iaptype!=1 && iaptype!=40) continue;//! skip DXCall masks if empty
                            if (iaptype>30 && iaptype<40 && !stophint) continue;//! in QSO, reduce number of CPU cycles
                            if (iaptype==31 && !lcqdxcsig) continue;//! not CQ signal from std DXCall
                            if (iaptype>34 && iaptype<37 && (!ldxcsig || lapcqonly)) continue;//! not DXCall signal, skip weak CQ signals
                            if (iaptype>30 && iaptype<40 && !lwidedxcsearch && loutapwid) continue;//! if wideband DX search disabled
                            if (iaptype==31 && apcqsym_[0]>1) continue;
                            //if (iaptype>=35 && apsymdxstd_[0]>1) continue;//2
                            //if (iaptype>=36 && apsymdxstd_[0]>1) continue;//2
                            if (iaptype==40 && apsymmyns1_[0]>1) continue;
                            if (iaptype==41 && apsymmyns2_[0]>1) continue;
                            if (iaptype==42 && apsymmynsrrr_[0]>1) continue;
                            if (iaptype==43 && apsymmyns73_[0]>1) continue;
                            if (iaptype==44 && apsymmynsrr73_[0]>1) continue;
                            for (int z = 0; z < 174; ++z) apmask[z]=0;
                            if (iaptype==1) //then ! CQ ??? ???
                            {
                                for (int z = 0; z < 174; ++z)//3*ND 174
                                {
                                    if (isubp2==20) llrz[z]=llrc[z];
                                    else if (isubp2==21) llrz[z]=llrb[z];
                                    else if (isubp2==22) llrz[z]=llra[z];
                                    if (z<29)
                                    {
                                        apmask[z]=1;
                                        llrz[z]=apmag*(double)mcq_ft8_2[z];
                                    }
                                }
                                apmask[74]=1;  //apmask(75:77)=1
                                apmask[75]=1;
                                apmask[76]=1;
                                llrz[74]=apmag*(-1.0);//llrd(75:76)=apmag*(-1)
                                llrz[75]=apmag*(-1.0);//llrd(75:76)=apmag*(-1)
                                llrz[76]=apmag*(+1.0);//llrd(77)=apmag*(+1)
                                //qDebug()<<"AP-1 CQ";
                            }
                            else if (iaptype==31) //then ! CQ DxCall Grid(???)
                            {
                                for (int z = 0; z < 174; ++z)
                                {
                                    if (isubp2==23) llrz[z]=llrc[z];
                                    else if (isubp2==24) llrz[z]=llrb[z];
                                    else if (isubp2==25) llrz[z]=llra[z];
                                }
                                if (lnohisgrid)
                                {
                                    for (int z = 0; z < 58; ++z)
                                    {
                                        apmask[z]=1;
                                        llrz[z]=apmag*(double)apcqsym_[z];
                                    }
                                    apmask[74]=1;
                                    apmask[75]=1;
                                    apmask[76]=1;
                                    llrz[74]=apmag*(-1.0);
                                    llrz[75]=apmag*(-1.0);
                                    llrz[76]=apmag*(+1.0);
                                }
                                else
                                {
                                    for (int z = 0; z < 77; ++z)
                                    {
                                        apmask[z]=1;
                                        llrz[z]=apmag*(double)apcqsym_[z];
                                    }
                                }
                            }
                            else if (iaptype==35) //then ! ??? DxCall 73 ! full compound or nonstandard
                            {
                                for (int z = 0; z < 174; ++z)
                                {
                                    if (isubp2==29) llrz[z]=llrc[z];
                                    else if (isubp2==30) llrz[z]=llrb[z];
                                    else if (isubp2==31) llrz[z]=llra[z];
                                }
                                for (int z = 29; z < 77; ++z)
                                {
                                    apmask[z]=1;
                                    if (z<58) llrz[z]=apmag*(double)apsymdxstd_[z];//llrz(30:58)=apmag*apsym(30:58)
                                }
                                for (int z = 0; z < 19; ++z) llrz[z+58]=apmag*(double)m73_ft8_2[z];
                            }
                            else if (iaptype==36) //then ! ??? DxCall RR73 //std DX call
                            {
                                for (int z = 0; z < 174; ++z)
                                {
                                    if (isubp2==26) llrz[z]=llrc[z];
                                    else if (isubp2==27) llrz[z]=llrb[z];
                                    else if (isubp2==28) llrz[z]=llra[z];
                                }
                                for (int z = 29; z < 77; ++z)
                                {
                                    apmask[z]=1;
                                    if (z<58) llrz[z]=apmag*(double)apsymdxstd_[z];//llrz(30:58)=apmag*apsym(30:58)
                                }
                                for (int z = 0; z < 19; ++z) llrz[z+58]=apmag*(double)mrr73_ft8_2[z];
                            }
                            else if (iaptype==40)// then ! <MyCall>,???,??? type 1
                            {
                                for (int z = 0; z < 174; ++z)
                                {
                                    if (isubp2==17) llrz[z]=llrc[z];
                                    else if (isubp2==18) llrz[z]=llrb[z];
                                    else if (isubp2==19) llrz[z]=llra[z];
                                }
                                for (int z = 0; z < 29; ++z)
                                {
                                    apmask[z]=1;
                                    llrz[z]=apmag*(double)apsymmyns1_[z];
                                }
                                apmask[74]=1;//apmask(75:77)=1;
                                apmask[75]=1;
                                apmask[76]=1;
                                llrz[74]=apmag*(-1.0);//llrz(75:76)=apmag*(-1.0);
                                llrz[75]=apmag*(-1.0);
                                llrz[76]=apmag*(+1.0);
                            }
                            else if (iaptype==41) //then ! <MyCall>,DXCall,??? type 1
                            {
                                for (int z = 0; z < 174; ++z)
                                {
                                    if (isubp2==5) llrz[z]=llrc[z];
                                    else if (isubp2==6) llrz[z]=llrb[z];
                                    else if (isubp2==7) llrz[z]=llra[z];
                                }
                                for (int z = 0; z < 58; ++z)
                                {
                                    apmask[z]=1;
                                    llrz[z]=apmag*(double)apsymmyns2_[z];
                                }
                                apmask[74]=1;//apmask(75:77)=1;
                                apmask[75]=1;
                                apmask[76]=1;
                                llrz[74]=apmag*(-1.0);//llrz(75:76)=apmag*(-1.0);
                                llrz[75]=apmag*(-1.0);
                                llrz[76]=apmag*(+1.0);
                            }
                            else if (iaptype==42) //then ! MyCall,<DXCall>,RRR type 4
                            {
                                for (int z = 0; z < 174; ++z)
                                {
                                    if (isubp2==14) llrz[z]=llrc[z];
                                    else if (isubp2==15) llrz[z]=llrb[z];
                                    else if (isubp2==16) llrz[z]=llra[z];
                                }
                                for (int z = 0; z < 77; ++z)
                                {
                                    apmask[z]=1;
                                    llrz[z]=apmag*(double)apsymmynsrrr_[z];
                                }
                            }
                            else if (iaptype==43) //then ! MyCall,<DXCall>,73 type 4
                            {
                                for (int z = 0; z < 174; ++z)
                                {
                                    if (isubp2==11) llrz[z]=llrc[z];
                                    else if (isubp2==12) llrz[z]=llrb[z];
                                    else if (isubp2==13) llrz[z]=llra[z];
                                }
                                for (int z = 0; z < 77; ++z)
                                {
                                    apmask[z]=1;
                                    llrz[z]=apmag*(double)apsymmyns73_[z];
                                }
                            }
                            else if (iaptype==44) //then ! MyCall,<DXCall>,RR73 type 4
                            {
                                for (int z = 0; z < 174; ++z)
                                {
                                    if (isubp2==8) llrz[z]=llrc[z];
                                    else if (isubp2==9) llrz[z]=llrb[z];
                                    else if (isubp2==10) llrz[z]=llra[z];
                                }
                                for (int z = 0; z < 77; ++z)
                                {
                                    apmask[z]=1;
                                    llrz[z]=apmag*(double)apsymmynsrr73_[z];
                                }
                            }
                        }
                        else continue;
                    }
                    /*else if (lhound)//HV stop hound
                    {
                        //if (!s_lapon8) continue;//for the moment hv
                        iaptype=nhaptypes[nQSOProgress][isubp2-5];
                        iaptype2=iaptype;
                        if (iaptype==0) continue;//
                        if (lnomycall && iaptype>1 && iaptype<31) continue;//! skip AP if mycall is missed in config
                        if (lhiscallstd && iaptype==31 && !lcqsignal) continue;//! not CQ signal, skip CQ DXCall DXgrid mask
                        if (lqsomsgdcd && iaptype>0 && iaptype<25) continue;//! QSO message already decoded
                        if (!stophint && (iaptype==31 || iaptype==36)) continue;//! CQ and RR73 decoding not needed if TX is on
                        if (nQSOProgress==1)
                        {
                            if (lfoxspecrpt)
                            {
                                if (iaptype==21) continue;//! it is not standard message report signal
                                if ((iaptype==31 || iaptype==36) && nfoxspecrpt>3) continue;//! probably special message signal
                            }
                            else
                            {
                                if (iaptype==22) continue;//! it is not special message report signal
                                if ((iaptype==31 || iaptype==36) && nmic>3) continue;//! probably standard message signal with mybcall
                            }
                        }
                        if (nQSOProgress==3)
                        {
                            if (lfoxspecrpt)
                            {
                                if (iaptype==21) continue;//! it is not standard message report signal
                            }
                            else
                            {
                                if (iaptype==22) continue;//! it is not special message report signal
                            }
                            if (lfoxstdr73)
                            {
                                if (iaptype==24) continue;//! it is not special message RR73 signal
                            }
                            else
                            {
                                if (iaptype==23) continue;//! it is not standard message RR73 signal
                            }
                        }
                        //! can be staying in queue, need to process possible incoming report iaptype 21/22 even if TX is switched off
                        if (!lapmyc && (iaptype==23 || iaptype==24)) continue;//! skip AP for mycall RR73 in 2..3 minutes after last TX
                        double fdelta=fabs(f1-nfqso);
                        double fdeltam=fmod(fdelta,60.0); //! dupe string to prevent compiler warning
                        if (nQSOProgress>0 && iaptype<31 && (fdelta>245.0 || fdeltam>3.0)) continue;//! AP shall be applied to Fox frequencies only
                        if ((iaptype==31 || iaptype==36) && !lwidedxcsearch && (fdelta>245.0 || fdeltam>3.0)) continue;//! only Fox frequencies DX Call searching
                        if (iaptype==31 && !lhiscallstd && lapcqonly) continue;//! skip weak CQ signals with std call
                        if (iaptype==36 && lwidedxcsearch && lapcqonly) continue;//! skip weak CQ signals
                        if (iaptype==111 && lapcqonly) continue;//! skip weak CQ signals with std call
                        if (iaptype==21 && apsym_[0]>1) continue;
                        //if (iaptype>=22 && apsymsp_[0]>1) continue;//2
                        //if (iaptype>=23 && apsym_[0]>1) continue;//2
                        if (iaptype==22 && apsymsp_[0]>1) continue;
                        if (iaptype==24 && apsymsp_[0]>1) continue;
                        if (iaptype==31 && apcqsym_[0]>1) continue;
                        //if (iaptype>=36 && apsym_[0]>1) continue;//2
                        for (int z = 0; z < 174; ++z) apmask[z]=0;
                        if (iaptype==1) //then ! CQ ??? ???
                        {
                            for (int z = 0; z < 174; ++z)//3*ND 174
                            {
                                if (isubp2==20) llrz[z]=llrc[z];
                                else if (isubp2==21) llrz[z]=llrb[z];
                                else if (isubp2==22) llrz[z]=llra[z];
                                if (z<29)
                                {
                                    apmask[z]=1;
                                    llrz[z]=apmag*(double)mcq_ft8_2[z];
                                }
                            }
                            apmask[74]=1;  //apmask(75:77)=1
                            apmask[75]=1;
                            apmask[76]=1;
                            llrz[74]=apmag*(-1.0);//llrd(75:76)=apmag*(-1)
                            llrz[75]=apmag*(-1.0);//llrd(75:76)=apmag*(-1)
                            llrz[76]=apmag*(+1.0);//llrd(77)=apmag*(+1)
                        }
                        else if (iaptype==21) //then ! MyBaseCall DxBaseCall ???  ! report
                        {
                            for (int z = 0; z < 174; ++z)
                            {
                                if (isubp2==5) llrz[z]=llrc[z];
                                else if (isubp2==6) llrz[z]=llrb[z];
                                else if (isubp2==7) llrz[z]=llra[z];
                            }
                            for (int z = 0; z < 58; ++z)
                            {
                                apmask[z]=1;
                                llrz[z]=apmag*(double)apsym_[z];
                            }
                            apmask[74]=1;//apmask(75:77)=1;
                            apmask[75]=1;
                            apmask[76]=1;
                            llrz[74]=apmag*(-1.0);//llrz(75:76)=apmag*(-1.0);
                            llrz[75]=apmag*(-1.0);
                            llrz[76]=apmag*(+1.0);
                        }
                        else if (iaptype==22) //then ! ??? RR73; MyCall <???> ??? ! report
                        {
                            for (int z = 0; z < 174; ++z)
                            {
                                if (isubp2==8) llrz[z]=llrc[z];
                                else if (isubp2==9) llrz[z]=llrb[z];
                                else if (isubp2==10) llrz[z]=llra[z];
                            }
                            for (int z = 28; z < 66; ++z)
                            {
                                apmask[z]=1;
                                llrz[z]=apmag*(double)apsymsp_[z];
                            }
                            for (int z = 71; z < 77; ++z) apmask[z]=1;//(72:77)
                            llrz[71]=apmag*(-1.0);//llrz(72:73)=apmag*(-1.0);
                            llrz[72]=apmag*(-1.0);
                            llrz[73]=apmag*(+1.0);
                            llrz[74]=apmag*(-1.0);//llrz(75:77)=apmag*(-1.0);
                            llrz[75]=apmag*(-1.0);
                            llrz[76]=apmag*(-1.0);
                        }
                        else if (iaptype==23) //then ! MyBaseCall DxBaseCall RR73
                        {
                            for (int z = 0; z < 174; ++z)
                            {
                                if (isubp2==11) llrz[z]=llrc[z];
                                else if (isubp2==12) llrz[z]=llrb[z];
                                else if (isubp2==13) llrz[z]=llra[z];
                            }
                            for (int z = 0; z < 77; ++z)
                            {
                                apmask[z]=1;
                                if (z<58) llrz[z]=apmag*(double)apsym_[z];
                            }
                            for (int z = 0; z < 19; ++z) llrz[z+58]=apmag*(double)mrr73_ft8_2[z];
                        }
                        else if (iaptype==24) //then ! MyCall RR73; ??? <???> ???
                        {
                            for (int z = 0; z < 174; ++z)
                            {
                                if (isubp2==14) llrz[z]=llrc[z];
                                else if (isubp2==15) llrz[z]=llrb[z];
                                else if (isubp2==16) llrz[z]=llra[z];
                            }
                            for (int z = 0; z < 28; ++z) apmask[z]=1;
                            for (int z = 56; z < 66; ++z) apmask[z]=1;
                            for (int z = 0; z < 28; ++z) llrz[z]=apmag*(double)apsymsp_[z];
                            for (int z = 56; z < 66; ++z) llrz[z]=apmag*(double)apsymsp_[z];
                            for (int z = 71; z < 77; ++z) apmask[z]=1;
                            llrz[71]=apmag*(-1.0);//llrz(72:73)=apmag*(-1.0);
                            llrz[72]=apmag*(-1.0);
                            llrz[73]=apmag*(+1.0);
                            llrz[74]=apmag*(-1.0);//llrz(75:77)=apmag*(-1.0);
                            llrz[75]=apmag*(-1.0);
                            llrz[76]=apmag*(-1.0);
                        }
                        else if (iaptype==31) //then ! CQ  DxCall Grid(???)
                        {
                            for (int z = 0; z < 174; ++z)
                            {
                                if (isubp2==23) llrz[z]=llrc[z];
                                else if (isubp2==24) llrz[z]=llrb[z];
                                else if (isubp2==25) llrz[z]=llra[z];
                            }
                            if (lhiscallstd)
                            {
                                if (lnohisgrid)
                                {
                                    for (int z = 0; z < 58; ++z)
                                    {
                                        apmask[z]=1;
                                        llrz[z]=apmag*(double)apcqsym_[z];
                                    }
                                    apmask[74]=1;//apmask(75:77)=1;
                                    apmask[75]=1;
                                    apmask[76]=1;
                                    llrz[74]=apmag*(-1.0);//llrz(75:76)=apmag*(-1.0);
                                    llrz[75]=apmag*(-1.0);
                                    llrz[76]=apmag*(+1.0);
                                }
                                else
                                {
                                    for (int z = 0; z < 77; ++z)
                                    {
                                        apmask[z]=1;
                                        llrz[z]=apmag*(double)apcqsym_[z];
                                    }
                                }
                            }
                            else
                            {
                                for (int z = 0; z < 77; ++z)
                                {
                                    apmask[z]=1;
                                    llrz[z]=apmag*(double)apcqsym_[z];
                                }
                            }
                        }
                        else if (iaptype==36)
                        {
                            for (int z = 0; z < 174; ++z)
                            {
                                if (isubp2==26) llrz[z]=llrc[z];
                                else if (isubp2==27) llrz[z]=llrb[z];
                                else if (isubp2==28) llrz[z]=llra[z];
                            }
                            if (lhiscallstd || (!lhiscallstd && s_HisCall8.count()>2 && s_HisCall8.indexOf("/")>-1))
                            {
                                for (int z = 29; z < 77; ++z)
                                {
                                    apmask[z]=1;
                                    if (z<58) llrz[z]=apmag*(double)apsym_[z];
                                }
                                for (int z = 0; z < 19; ++z) llrz[z+58]=apmag*(double)mrr73_ft8_2[z]; //! ??? DxBaseCall RR73
                            }
                            else continue; //! (noncompound .and. nonstandard) Fox callsign being not supported by Fox WSJT-X
                        }
                        else if (iaptype==111) //then ! CQ ??? type 4 msg (iflip,nrpt,icq,i3 001100)
                        {
                            for (int z = 0; z < 174; ++z)
                            {
                                if (isubp2==29) llrz[z]=llrc[z];
                                else if (isubp2==30) llrz[z]=llrb[z];
                                else if (isubp2==31) llrz[z]=llra[z];
                            }
                            for (int z = 71; z < 77; ++z) apmask[z]=1;
                            llrz[71]=apmag*(-1.0);//llrz(72:73)=apmag*(-1.0)
                            llrz[72]=apmag*(-1.0);
                            llrz[73]=apmag*(1.0);//llrz(74:75)=apmag*(1.0)
                            llrz[74]=apmag*(1.0);
                            llrz[75]=apmag*(-1.0);//llrz(76:77)=apmag*(-1.0)
                            llrz[76]=apmag*(-1.0);
                        }
                    }
                    else continue;*/
                }
                for (int z = 0; z < 174; ++z)
                {
                    cw[z]=0;
                    if (z<120) message77[z]=0;
                }//nharderrors=100;//hv
                pomFt.bpdecode174_91var(llrz,apmask,message77,cw,nharderrors);
                dmin=0.0; //if (f1>2724.5 && f1<2726) qDebug()<<"-0-icand"<<msg37<<sync<<f1<<xdt<<nharderrors<<apmag;
                if (nharderrors<0) //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
                {
                    int ndeep=3; //qDebug()<<"DEC";
                    if (lqsosig || lmycsignal)
                    {
                        if ((dfqso<napwid && (fabs(nftx-f1)<napwid && lapmyc)) && !nagainfil) ndeep=4;
                        if ((lapmyc && lqsomsgdcd) || iaptype==0) ndeep=3;// ! deep is not needed, reduce number of CPU cycles
                        if (!stophint && s_HisCall8.count()>2) ndeep=3;// ! unload CPU, let ft8svar pick up QSO message
                    }
                    if (ldxcsig && stophint && dfqso<napwid) ndeep=4;// ! DXCall search inside RX napwid
                    if (lhound) ndeep=3; //! we have no enough CPU resources to decode all Fox slots with ndeep=4
                    //!if(nagainfil .or. swl) ndeep=5 ! 30 against 26 -23dB, more than 15sec to decode and many false decodes
                    //!if(swl) ndeep=4 ! 29 decodes -23dB, 7..12sec to decode
                    if (nagainfil) ndeep=5;
                    //ndeep-=1;
                    //!print *,omp_get_nested(),OMP_get_num_threads()//nharderrors=100;//hv
                    //if (sync>20) ndeep=3; qDebug()<<sync<<ndeep;
                    pomFt.osd174_91_1(llrz,apmask,ndeep,message77,cw,nharderrors,dmin,true);//var=true
                } //else qDebug()<<"NO DEC"<<sync;
                //pomFt.decode174_91(llrz,3,2,apmask,message77,cw,nharderrors,dmin);
                nbadcrc=1;
                msg37="";                
                int c_cw = 0; 
                for (int z = 0; z < 174; z++)//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
                {
                    if (cw[z]==0) c_cw++;// 3*ND=174
                }
                if (c_cw==174) continue; //!Reject the all-zero codeword
                if (nharderrors<0 || nharderrors+dmin>=60.0 || (isubp2>2 && nharderrors>39)) //then ! chk isubp2 value
                {   //if (f1>2724.5 && f1<2726) qDebug()<<"-1-icand"<<msg37<<sync<<f1<<xdt<<nharderrors;
                    if (nweak==2 && isubp1==2)
                    {
                        if (ipass==npass-1)
                        {
                            if (lcqsignal)
                            {
                                if (isubp2==22) //then ! last pass, std and nonstd mycall
                                {
                                    lfoundcq=false;
                                    for (int ik = 0; ik < numdeccq; ++ik)
                                    {//do ik=1,numdeccq
                                        if (tmpcqdec[ik].freq>5001.0) break;
                                        if (fabs(tmpcqdec[ik].freq-f1)<5.0 && fabs(tmpcqdec[ik].xdt-xdt)<0.05) //then ! max signal delay
                                        {
                                            lfoundcq=true;
                                            break;
                                        }
                                    }
                                    if (!lfoundcq && ncqsignal<numcqsig)// then
                                    {
                                        tmpcqsig[ncqsignal].freq=f1;//ncqsignal=ncqsignal+1
                                        tmpcqsig[ncqsignal].xdt=xdt;
                                        for (int z = 0; z < 79; z++)
                                        {
                                            for (int z0 = 0; z0 < 8; z0++) tmpcqsig[ncqsignal].cs_[z][z0]=cstmp2_[z][z0];
                                        } //qDebug()<<"-->"<<decid<<ncqsignal<<tmpcqsig[ncqsignal].freq<<tmpcqsig[ncqsignal].xdt;
                                        ncqsignal++;
                                    }
                                }
                            }
                            if (lmycsignal && isubp2==19) //then ! last pass
                            {
                                lfoundmyc=false;
                                for (int ik = 0; ik < numdecmyc; ++ik)
                                {//do ik=1,numdecmyc
                                    if (tmpmyc[ik].freq>5001.0) break;
                                    if (fabs(tmpmyc[ik].freq-f1)<5.0 && fabs(tmpmyc[ik].xdt-xdt)<0.05) //then ! max signal delay
                                    {
                                        lfoundmyc=true;
                                        break;
                                    }
                                }
                                if (!lfoundmyc && nmycsignal<nummycsig)
                                {
                                    tmpmycsig[nmycsignal].freq=f1;
                                    tmpmycsig[nmycsignal].xdt=xdt;
                                    for (int z = 0; z < 79; z++)
                                    {
                                        for (int z0 = 0; z0 < 8; z0++) tmpmycsig[nmycsignal].cs_[z][z0]=cstmp2_[z][z0];
                                    }
                                    nmycsignal++;
                                }
                            }
                        }
                        if (ipass==npass)
                        {
                            if (lqsocandave && (iaptype==3 || iaptype==6))
                            {
                                tmpqsosig[0].freq=f1;
                                tmpqsosig[0].xdt=xdt;
                                for (int z = 0; z < 79; z++)
                                {
                                    for (int z0 = 0; z0 < 8; z0++) tmpqsosig[0].cs_[z][z0]=cstmp2_[z][z0];
                                }
                            }
                        }
                    }
                    //!print*,'line 2149 iaptype,isubp1,isubp2,ndeep = ',iaptype,isubp1,isubp2,ndeep
                    if (isubp1>1) continue;
                    //! print*,'line 2150 did not cycle'//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
                    //if(lqsothread .and. (.not.lhound .and. iaptype.ge.3 .or. lhound .and. (iaptype.eq.21 .or. iaptype.eq.23)) .and. .not.lsdone) then
                    if (lqsothread && ((!lhound && iaptype>=3) || (lhound && (iaptype==21 || iaptype==23))) && !lsdone)// then
                    {
                        if (!lqsomsgdcd && !(!lmycallstd && !lhiscallstd)) //then
                        {
                            if (!lft8sdec && !stophint && dfqso<2.0)
                            {   //qDebug()<<"1 IN ft8svar<--"<<dfqso;
                                ft8svar(s8_,srr,itone,msg37,lft8s,nft8rxfsens,stophint);//OK Tested HV
                                if (lft8s)
                                {
                                    if (msg37.indexOf("<")>-1) lhashmsg=true;
                                    nbadcrc=0;
                                    lft8sdec=true; //qDebug()<<"1 Out ft8svar -->"<<msg37;
                                }
                            }
                        }
                        lsdone=true;
                    }
                    if (nbadcrc==0)
                    {
                        i3=1;
                        n3=1;
                        break;
                    }
                    if (lsd && isubp2==3 && nbadcrc==1 && srr<7.0)//! low DR setups shall not try FT8SDvar for strong signals
                    {   //qDebug()<<"IN ft8sdvar <--"<<msgd;
                        ft8sdvar(s8_,srr,itone,msgd,msg37,lft8sd,lcq);//OK Tested HV
                        if (lft8sd)
                        {   //qDebug()<<"OUT ft8sdvar -->"<<msg37;
                            if (!f_eve0_odd1) evencopy[isd].lstate=false;
                            else oddcopy[isd].lstate=false;
                            i3=1;
                            n3=1;
                            iaptype2=iaptype;
                            iaptype=0;
                            nbadcrc=0;
                            lsd=false; //qDebug()<<"lft8sd"<<msg37;
                            break;
                        }
                    }
                    if (nweak==1 && isubp1==1)
                    {
                        if (ipass==npass-1)
                        {
                            if (lcqsignal)
                            {
                                if (isubp2==22) //then ! last pass, std and nonstd mycall
                                {
                                    lfoundcq=false;
                                    for (int ik = 0; ik < numdeccq; ++ik)
                                    {//do ik=1,numdeccq
                                        if (tmpcqdec[ik].freq>5001.0) break;
                                        if (fabs(tmpcqdec[ik].freq-f1)<5.0 && fabs(tmpcqdec[ik].xdt-xdt)<0.05) //then ! max signal delay
                                        {
                                            lfoundcq=true;
                                            break;
                                        }
                                    }
                                    if (!lfoundcq && ncqsignal<numcqsig)
                                    {
                                        tmpcqsig[ncqsignal].freq=f1;
                                        tmpcqsig[ncqsignal].xdt=xdt;
                                        for (int z = 0; z < 79; z++)
                                        {
                                            for (int z0 = 0; z0 < 8; z0++) tmpcqsig[ncqsignal].cs_[z][z0]=cstmp2_[z][z0];
                                        }
                                        ncqsignal++;
                                    }
                                }
                            }
                            if (lmycsignal && isubp2==19) //then ! last pass
                            {
                                lfoundmyc=false;
                                for (int ik = 0; ik < numdecmyc; ++ik)
                                {//do ik=1,numdecmyc
                                    if (tmpmyc[ik].freq>5001.0) break;
                                    if (fabs(tmpmyc[ik].freq-f1)<5.0 && fabs(tmpmyc[ik].xdt-xdt)<0.05)// then ! max signal delay
                                    {
                                        lfoundmyc=true;
                                        break;
                                    }
                                }
                                if (!lfoundmyc && nmycsignal<nummycsig)
                                {
                                    tmpmycsig[nmycsignal].freq=f1;
                                    tmpmycsig[nmycsignal].xdt=xdt;
                                    for (int z = 0; z < 79; z++)
                                    {
                                        for (int z0 = 0; z0 < 8; z0++) tmpmycsig[nmycsignal].cs_[z][z0]=cstmp2_[z][z0];
                                    }
                                    nmycsignal++;
                                }
                            }
                        }
                        if (ipass==npass)
                        {
                            if (lqsocandave && (iaptype==3 || iaptype==6))
                            {
                                tmpqsosig[0].freq=f1;
                                tmpqsosig[0].xdt=xdt;
                                for (int z = 0; z < 79; z++)
                                {
                                    for (int z0 = 0; z0 < 8; z0++) tmpqsosig[0].cs_[z][z0]=cstmp2_[z][z0];
                                }
                            }
                        }
                    }
                    if (nbadcrc==1) continue;
                }
                n3=4*message77[71] + 2*message77[72] + message77[73];
                i3=4*message77[74] + 2*message77[75] + message77[76];
                /*if((i3.gt.4 .or. (i3.eq.0 .and. n3.gt.5)) .and. (.not.(i3.eq.5 .and.   &
                     (n3.eq.1 .or. n3.eq.7 .or.n3.eq.6 .or.n3.eq.4 .or.n3.eq.3 .or.n3.eq.0 .or.n3.eq.5)))) cycle*/  //! ft8md added n3.eq.6 and n3.eq.4 and n3.eq.3 and n3.eq.0 and n3.eq.5 for USA calls with EU VHF
                //!print*,'did not cycle at line 2248' //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
                //if((i3>4 || (i3==0 && n3>5)) && (!(i3==5 && (n3==0 || n3==1 || n3==3 || n3==4 || n3==5 || n3==6 || n3==7)))) continue;
                // old if (i3>4 || (i3==0 && n3>5)) continue; //cycle
                if (i3>5 || (i3==0 && n3>6)) continue; //2.39 EU VHF Contest
                if (i3==0 && n3==2) continue; //2.42 222 ignore old EU VHF Contest
                bool unpk77_success = false;
                msg37 = TGenFt8->unpack77(message77,unpk77_success); //qDebug()<<"-2-icand"<<msg37<<f1<<xdt<<iaptype2<<sync;
                if (!unpk77_success)
                {   //if(lqsothread .and. (.not.lhound .and. iaptype.ge.3 .or. lhound .and. (iaptype.eq.21 .or. iaptype.eq.23)) .and. .not.lsdone)
                    if (lqsothread && ((!lhound && iaptype>=3) || (lhound && (iaptype==21 || iaptype==23))) && !lsdone)
                    {
                        if (!lqsomsgdcd && !(!lmycallstd && !lhiscallstd))
                        {
                            if (!lft8sdec && !stophint && dfqso<2.0)
                            {   //qDebug()<<"2 IN ft8svar<--";
                                ft8svar(s8_,srr,itone,msg37,lft8s,nft8rxfsens,stophint);//OK Tested HV
                                if (lft8s)
                                {
                                    if (msg37.indexOf('<')>-1) lhashmsg=true;
                                    nbadcrc=0;
                                    lft8sdec=true; //qDebug()<<"2 ft8svar -->"<<msg37;
                                }
                            }
                        }
                        lsdone=true;
                        if (nbadcrc==0)
                        {
                            i3=1;
                            n3=1;
                            break;
                        }
                    }
                    continue;//??
                }//qDebug()<<"message="<<unpk77_success<<msg37<<i3;
                QString tms0 = msg37+"            ";
                //if (iaptype==1 && tms0.midRef(0,10)=="CQ DE AA00")
                if (iaptype==1 && (tms0.midRef(0,10)=="CQ DE AA00" || tms0.midRef(0,13)=="CQ CF DE AA00"))
                {
                    nbadcrc=1;
                    continue;
                }
                //lcall1hash=false;
                //if (tms0.at(0)=='<') lcall1hash=true;
                nbadcrc=0;//! If we get this far: valid codeword, valid (i3,n3), nonquirky message.
                TGenFt8->make_c77_i4tone(message77,itone);//call get_tones_from_77bits(message77,itone)
                //! 0.1  K1ABC RR73; W9XYZ <KH1/KH7Z> -11   28 28 10 5       71   DXpedition Mode
                i3bit=0;
                if (i3==0 && n3==1) i3bit=1;
                //!        iFreeText=message77(57)
                //! 0.0  Free text
                if (i3==0 && n3==0) lFreeText=true;
                else lFreeText=false;
                //! delete braces
                //lcall2hash=false;
                if (!lFreeText && i3bit!=1 && tms0.indexOf("<")>-1)//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
                {
                    //if (tms0.indexOf("<")>3) lcall2hash=true;//E1J RR73; TA1BM <SX60RAAG> -22
                    if (tms0.indexOf("<")>-1) lhashmsg=true;
                }
                if (nbadcrc==0) break;
            } //! ap passes
            if (nbadcrc==0) break;
        } //! weak sigs
        if (nbadcrc==0) break;
    }//!nqso sync
c2:
    if (nbadcrc==0)
    {
        //check for the false FT8Svar decode
        if (lft8s && lrepliedother)
        {
            msg37="";
            nbadcrc=1;
            return;//goto end; //return;
        }
        //msg37_2="";//??
        if (i3bit==1 && !lft8s && !lft8sd)
        {
            //call msgparservar(msg37,msg37_2)//??
            lspecial=true;  //!only in ft8var
            //!protection against a false FT8Svar decode in Hound mode
            if (lhound && dfqso<2.0) lqsomsgdcd=true; //!$OMP FLUSH (lqsomsgdcd)
        }
        qual=1.0-(nharderrors+dmin)/60.0;//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
        if (qual>1.0) qual=1.0;//HV =1.6 SD
        xsnr=0.001;
        double xnoi=1.E-8;
        double xsnrtmp=0.001;
        double tpom[10];
        for (int i = 0; i < 79; i++)
        {//do i=1,79
            double xsig=s8_[i][itone[i]]*s8_[i][itone[i]];
            double sums8=0.0;
            for (int z = 0; z < 8; ++z) tpom[z]=s8_[i][z]*s8_[i][z];
            for (int z = 0; z < 8; ++z) sums8+=tpom[z];
            xnoi=(sums8 - xsig)/7.0;//xnoi=(sum(s8(0:7,i)**2) - xsig)/7.0
            if (xnoi<0.01) xnoi=0.01; //! safe division and better accuracy
            if (xnoi<xsig) xsnr=xsig/xnoi;
            else xsnr=1.01;
            xsnrtmp+=xsnr;
        }
        xsnr=xsnrtmp/79.0-1.0;
        if (xsnr<0.000001) xsnr=0.000001;
        xsnr=10.0*log10(xsnr) -26.5;// - 1.0;// -4.0;
        if (xsnr>7.0) xsnr=xsnr+(xsnr-7.0)/2.0;
        if (xsnr>30.0)
        {
            xsnr=xsnr-1.0;
            if (xsnr>40.0) xsnr=xsnr-1.0;
            if (xsnr>49.0) xsnr=49.0;
        }
        double xsnrs=xsnr;
        if (xsnr < -17.0)
        {
            if (xsnr<-22.5 && xsnr>-23.5) xsnr=-22.5;// ! safe division and better accuracy
            xsnr-=pow((1.0+1.4/(23.0+xsnr)),2)+1.2;
        }
        if (iaptype==0)
        {
            if (xsnr<-23.0) xsnr=-23.0;
        }
        else
        {
            if (xsnr<-24.0) xsnr=-24.0;
        }
        if (lft8s || lft8sd)
        {
            if (xsnr<-22.0) xsnr=xsnrs-1.0; //! correction offset
            if (xsnr<-26.0) xsnr=-26.0;
            ///! -26  0.1 1477 ~ AC1MX AC1MX R-17          ^
            //if(len_trim(mycall).gt.3 .and. index(msg37,' '//trim(mycall)//' ').gt.1) msg37=''
            if (s_MyCall8.count()>3 && msg37.indexOf(" "+s_MyCall8+" ")>-1)
            {
                msg37="";
                return;//goto end;//return;
            }
            goto c4;// ! bypass checking to false decode
        }
        //!if(len_trim(msg37)>0) print*,'line 2371 ',msg37

        /*rxdt=xdt-0.5
        if(iaptype.gt.34 .and. iaptype.lt.40) then ! DX Call searching false iaptype 35,36: 'CS7CYU/R FO5QB 73', 'T57KWP/R FO5QB RR73'
           ispc1=index(msg37,' ')
           if(ispc1.gt.5) then
              if(msg37((ispc1-2):(ispc1-1)).eq.'/R') then ! have to cascade it to prevent getting out of index range
                 call_a=''
                 call_a=msg37(1:(ispc1-3))
                 lfound=.false.
                 call searchcallsvar(call_a,"            ",lfound)
                 if(.not.lfound) then
                    nbadcrc=1
                    msg37=''
                    return
                 endif
              endif
           endif
        else if(qual.lt.0.39 .or. xsnr.lt.-20.5 .or. rxdt.lt.-0.5 .or. rxdt.gt.1.9  &
             .or. (iaptype.gt.0 .and. iaptype.lt.4) .or. iaptype.eq.11 .or.         &
             iaptype.eq.21 .or. iaptype.eq.40 .or. iaptype.eq.41) then
           if(iaptype.gt.3 .and. iaptype.lt.7) go to 4 ! skip, nothing to check
           if(iaptype.ne.2 .and. iaptype.ne.3 .and. iaptype.ne.11 .and. iaptype.ne.21 &
                .and. iaptype.ne.40 .and. iaptype.ne.41) then
              if((mybcall.ne."            " .and. index(msg37,mybcall).gt.0) .or. &
                 (hisbcall.ne."            " .and. index(msg37,hisbcall).gt.0)) go to 256
           endif

           if(i3bit.eq.1) then
           !  print*,'nbadcrc line 2400',nbadcrc
              call chkspecial8var(msg37,msg37_2,nbadcrc)
           !  print*,'nbadcrc line 2402',nbadcrc
           else
           !  print*,'nbadcrc line 2404 nbadcrc,i3,n3',nbadcrc,i3,n3
              call chkfalse8var(msg37,i3,n3,nbadcrc,iaptype,lcall1hash)
           !  print*,'nbadcrc line 2406',nbadcrc
           endif
           if(nbadcrc.eq.1) then
              msg37=''
              return
           endif
        endif
        if(iaptype.eq.2 .or. iaptype.eq.3 .or. iaptype.eq.11 .or. iaptype.eq.21 .or. &
             iaptype.eq.40 .or. iaptype.eq.41) go to 4 ! already checked

        ! decoded without AP masks:
        ! CQ PLIB U40OKH FE61
        ! CQ 000 RO8JSV GJ38
        256  if(iaptype.eq.0 .and. i3.eq.1 .and. msg37(1:3).eq.'CQ ') then
           ispc1=index(msg37,' ')
           ispc2=index(msg37((ispc1+1):),' ')+ispc1
           if((ispc2-ispc1).eq.4 .or. (ispc2-ispc1).eq.5) then
              ispc3=index(msg37((ispc2+1):),' ')+ispc2
              ispc4=index(msg37((ispc3+1):),' ')+ispc3
              if((ispc3-ispc2).gt.3 .and. (ispc4-ispc3).eq.5) then
                 grid=''
                 if(msg37(ispc3+1:ispc3+1).gt.'@' .and. msg37(ispc3+1:ispc3+1).lt.'S' &
                      .and. msg37(ispc3+2:ispc3+2).gt.'@' .and.                       &
                      msg37(ispc3+2:ispc3+2).lt.'S' .and.                             &
                      msg37(ispc3+3:ispc3+3).lt.':' .and.                             &
                      msg37(ispc3+4:ispc3+4).lt.':') grid=msg37(ispc3+1:ispc4-1)
                 if(grid.ne.'') then
                    callsign=''
                    callsign=msg37(ispc2+1:ispc3-1)
                    call chkgridvar(callsign,grid,lchkcall,lgvalid,lwrongcall)
                    if(lwrongcall .or. .not.lgvalid) then
                       nbadcrc=1
                       msg37=''
                       return
                    endif
                 endif
              endif
           endif
        endif

        ! i3=1,2 false decodes with ' R '
        ! 3B4NDC/R C40AUZ/R R IR83  i3=1
        ! MS8QQS UX3QBS/P R NG63  i3=2
        ! <...> P32WRF R LR56
        ! P32WRF <...> R LR56

        if((i3.eq.1 .or. i3.eq.2) .and. index(msg37,' R ').gt.0) then
           ispc1=index(msg37,' ')
           ispc2=index(msg37((ispc1+1):),' ')+ispc1
           ispc3=index(msg37((ispc2+1):),' ')+ispc2
           if(ispc3-ispc2.eq.2) then
              if(msg37(ispc2:ispc3).eq.' R ') then
                 call_b=''
                 if((i3.eq.1 .and. msg37(ispc2-2:ispc2-1).eq.'/R') .or.             &
                      (i3.eq.2 .and. msg37(ispc2-2:ispc2-1).eq.'/P')) then
                    call_b=msg37(ispc1+1:ispc2-3)
                 else
                    call_b=msg37(ispc1+1:ispc2-1)
                 endif
                 if(lcall2hash) then ! valid call_b can be found in hash table, check call_a then
                    call_a=''
                    if((i3.eq.1 .and. msg37(ispc1-2:ispc1-1).eq.'/R') .or.          &
                         (i3.eq.2 .and. msg37(ispc1-2:ispc1-1).eq.'/R')) then
                       call_a=msg37(1:ispc1-3)
                    else
                       call_a=msg37(1:ispc1-1)
                    endif
                    falsedec=.false.
                    call chkflscallvar('CQ          ',call_a,falsedec)
                    if(falsedec) then
                       nbadcrc=1
                       msg37=''
                       return
                    endif
                 else if(len_trim(call_b).gt.2) then
                    ispc4=index(msg37((ispc3+1):),' ')+ispc3
                    grid=''
                    if(ispc4-ispc3.eq.5 .and. msg37(ispc3+1:ispc3+1).gt.'@' .and.   &
                         msg37(ispc3+1:ispc3+1).lt.'S' .and.                        &
                         msg37(ispc3+2:ispc3+2).gt.'@' .and.                        &
                         msg37(ispc3+2:ispc3+2).lt.'S' .and.                        &
                         msg37(ispc3+3:ispc3+3).lt.':' .and.                        &
                         msg37(ispc3+4:ispc3+4).lt.':') grid=msg37(ispc3+1:ispc4-1)
                    if(grid.ne.'') then
                       call chkgridvar(call_b,grid,lchkcall,lgvalid,lwrongcall)
                       if(lwrongcall .or. .not.lgvalid) then
                          nbadcrc=1
                          msg37=''
                          return
                       endif
                    endif
                 endif
              endif
           endif
        endif

        ! 0.3   WA9XYZ KA1ABC R 16A EMA            28 28 1 4 3 7    71   ARRL Field Day
        ! 0.4   WA9XYZ KA1ABC R 32A EMA            28 28 1 4 3 7    71   ARRL Field Day
        ! -23  3.1  197 ~ Z67BGE H67HJI 22G EMA i3=0 n3=4
        !  -3  2.2 FY4IML UV7BEA R 24F NNJ   i3=0 n3=4

        if(i3.eq.0 .and. n3.gt.2 .and. n3.lt.5) then
           ispc1=index(msg37,' ')
           ispc2=index(msg37((ispc1+1):),' ')+ispc1
           if(ispc1.gt.3 .and. ispc2.gt.7) then
              call_a=''
              call_b=''
              if(msg37(1:ispc1-1).eq.'/R' .or. msg37(1:ispc1-1).eq.'/P') then
                 call_a=msg37(1:ispc1-3)
              else
                 call_a=msg37(1:ispc1-1)
              endif
              if(msg37(ispc1+1:ispc2-1).eq.'/R' .or. msg37(ispc1+1:ispc2-1).eq.'/P') then
                 call_b=msg37(ispc1+1:ispc2-3)
              else
                 call_b=msg37(ispc1+1:ispc2-1)
              endif

        ! operators outside USA and Canada shall transmit ' DX '
              if(call_b(1:1).ne.'A' .and. call_b(1:1).ne.'K' .and. call_b(1:1).ne.'N' &
                   .and. call_b(1:1).ne.'W' .and. call_b(1:1).ne.'V' .and.           &
                   call_b(1:1).ne.'C' .and. call_b(1:1).ne.'X') then
                 if(index(msg37,' DX ').lt.1) then
                    nbadcrc=1
                    msg37=''
                    return
                 endif
              endif

        ! USA AA..AL
              if(call_b(1:1).eq.'A' .and. (call_b(2:2).lt.'A' .or. call_b(2:2).gt.'L')) then
                 if(index(msg37,' DX ').lt.1) then
                    nbadcrc=1
                    msg37=''
                    return
                 endif
              endif

        ! Canada CF..CK,CY,CZ,VA..VG,VO,VX,VY,XJ..XO
              if(call_b(1:1).eq.'C' .and. (call_b(2:2).lt.'F' .or. call_b(2:2).gt.'K')) then
                 if(call_b(2:2).ne.'Y' .and. call_b(2:2).ne.'Z') then
                    if(index(msg37,' DX ').lt.1) then
                       nbadcrc=1
                       msg37=''
                       return
                    endif
                 endif
              endif

              if(call_b(1:1).eq.'V' .and. (call_b(2:2).lt.'A' .or. call_b(2:2).gt.'G')) then
                 if(call_b(2:2).ne.'O' .and. call_b(2:2).ne.'X' .and. call_b(2:2).ne.'Y') then
                    if(index(msg37,' DX ').lt.1) then
                       nbadcrc=1
                       msg37=''
                       return
                    endif
                 endif
              endif
              if(call_b(1:1).eq.'X' .and. (call_b(2:2).lt.'J' .or. call_b(2:2).gt.'O')) then
                 if(index(msg37,' DX ').lt.1) then
                    nbadcrc=1
                    msg37=''
                    return
                 endif
              endif
              if(xsnr.lt.-19.0 .or. rxdt.lt.-0.5 .or. rxdt.gt.1.0) then
                 falsedec=.false.
                 call chkflscallvar(call_a,call_b,falsedec)
                 if(falsedec) then
                    nbadcrc=1
                    msg37=''
                    return
                 endif
              endif
           endif
        endif

        ! EU VHF contest exchange i3=5
        ! <PA3XYZ> <G4ABC/P> R 590003 IO91NP      h12 h22 r1 s3 S11 g25
        ! packing is not implemented in WSJT-X 2.5.4?
        ! old implementation with i3=0 n3=2
        ! JL6GSC/P R 571553 CJ76MV i3=5
        ! G59XTB R 521562 RA82SJ

        if(i3.eq.5 .and. n3.eq.1 .and. (xsnr.lt.-19.0 .or. rxdt.lt.-0.5 .or.        &
             rxdt.gt.1.0)) then ! was if i3-0 and n3=2
           ispc1=index(msg37,' ')
           if(ispc1.gt.3) then
              call_b=''
              if(msg37(ispc1-2:ispc1-1).eq.'/P') then
                 call_b=msg37(1:ispc1-3)
              else
                 call_b=msg37(1:ispc1-1)
              endif
              falsedec=.false.
              call chkflscallvar('CQ          ',call_b,falsedec)
              if(falsedec) then
                 nbadcrc=1
                 msg37=''
                 return
              endif
           endif
        endif

        ! FT8OPG/R Z27HRN/R OH12 check all standard messages with contest callsigns
        ! /P has i3.eq.2 .and. n3.eq.0
        if(iaptype.eq.0 .and. i3.eq.1 .and. n3.eq.0 .and. index(msg37,'/R ').gt.3) then
           ispc1=index(msg37,' ')
           ispc2=index(msg37((ispc1+1):),' ')+ispc1
           if(ispc1.gt.3 .and. ispc2.gt.6) then
              call_a=''
              call_b=''
              if(msg37(ispc1-2:ispc1-1).eq.'/R') then
                 call_a=msg37(1:ispc1-3)
              else
                 call_a=msg37(1:ispc1-1)
              endif
              if(msg37(ispc2-2:ispc2-1).eq.'/R') then
                 call_b=msg37(ispc1+1:ispc2-3)
              else
                 call_b=msg37(ispc1+1:ispc2-1)
              endif
              falsedec=.false.
              call chkflscallvar(call_a,call_b,falsedec)
              if(falsedec) then
                 nbadcrc=1
                 msg37=''
                 return
              endif
           endif
        endif

        ! -23 -0.5 2533 ~ <...> W LKNQZG2K4 RR73  invalid message, iaptype=0 this type of message is not allowed for transmission with RR73
        ! -23 -1.2 1335 ~ <...> Z7VENB8R G9 RRR   non AP decode, iaptype=0 invalid message, this type of message is not allowed
        ! for transmission with RRR

        if(msg37(1:2).eq.'<.') then
           ispc1=index(msg37,' ')
           ispc2=index(msg37((ispc1+1):),' ')+ispc1
           ispc3=index(msg37((ispc2+1):),' ')+ispc2
           ispc4=index(msg37((ispc3+1):),' ')+ispc3
           if((ispc4-ispc3.eq.4 .and. msg37(ispc3+1:ispc4-1).eq.'RRR') .or.          &
                (ispc4-ispc3.eq.5 .and. msg37(ispc3+1:ispc4-1).eq.'RR73') .or.       &
                (ispc4-ispc3.eq.3 .and. msg37(ispc3+1:ispc4-1).eq.'73')) then
              nbadcrc=1
              msg37=''
              return
           endif

        ! -19 0.0 2256 ~ <...> 9T4DQZ RP53  non AP decode, iaptype=0 i3=1 n3=1  SAME AS CQ MSG
        ! <...> 9T4DQZ -15(R-15) message has i3=1 n3=4
           if(i3.eq.1 .and. n3.eq.1 .and. (xsnr.lt.-18.0 .or. rxdt.lt.-0.5 .or.     &
                rxdt.gt.1.0)) then
              callsign='            '
              callsign=msg37(ispc1+1:ispc2-1)
              grid=msg37(ispc2+1:ispc3-1)
              include 'callsign_q.f90'
              call chkgridvar(callsign,grid,lchkcall,lgvalid,lwrongcall)
              if(lwrongcall) then
                 nbadcrc=1
                 msg37=''
                 return
              endif
              if(lchkcall .or. .not.lgvalid) then
                 falsedec=.false.
                 call chkflscallvar('CQ          ',callsign,falsedec)
                 if(falsedec) then
                    nbadcrc=1
                    msg37=''
                    return
                 endif
              endif
           endif
        endif

        ! -22  0.3 1000 ~ 9Y4DWY <...> BF70  iaptype=0 i3=1 n3=2  invalid message in FT8
        ! protocol, can be transmitted manually

        if(i3.eq.1) then
           ispc1=index(msg37,' ')
           if(msg37(ispc1+1:ispc1+2).eq.'<.') then
              ispc2=index(msg37((ispc1+1):),' ')+ispc1
              ispc3=index(msg37((ispc2+1):),' ')+ispc2
              if(ispc3-ispc2.eq.5 .and. msg37(ispc2+1:ispc2+1).gt.'@' .and.        &
                   msg37(ispc2+1:ispc2+1).lt.'S' .and.                             &
                   msg37(ispc2+2:ispc2+2).gt.'@' .and.                             &
                   msg37(ispc2+2:ispc2+2).lt.'S' .and.                             &
                   msg37(ispc2+3:ispc2+3).lt.':' .and.                             &
                   msg37(ispc2+4:ispc2+4).lt.':') then ! there is a valid grid in message
                 call_b=''
                 call_b=msg37(1:ispc1-1)
                 falsedec=.false.
                 call chkflscallvar('CQ          ',call_b,falsedec)
                 if(falsedec) then
                    nbadcrc=1
                    msg37=''
                    return
                 endif
              endif
           endif
        endif

        ! i3=3 parse ARRL RTTY contest message
        ! i3 n3                                      Bits               Total  Message type
        ! 3     TU; W9XYZ K1ABC R 579 MA             1 28 28 1 3 13       74   ARRL RTTY contest
        ! 3     TU; W9XYZ G8ABC R 559 0013           1 28 28 1 3 13       74   ARRL RTTY (DX)
        ! TU; D47IAQ <...> 559 032' does protocol support the message?

        if(i3.eq.3 .and. msg37(1:3).eq.'TU;') then
           ispc1=index(msg37,' ')
           ispc2=index(msg37((ispc1+1):),' ')+ispc1 
           ispc3=index(msg37((ispc2+1):),' ')+ispc2
           call_a=''
           call_b=''
           call_a=msg37(ispc1+1:ispc2-1)
           call_b=msg37(ispc2+1:ispc3-1)
        ! check for false, too tough, need to rework if contest is supported
           falsedec=.false.
           call chkflscallvar(call_a,call_b,falsedec)
           if(falsedec) then
              nbadcrc=1
              msg37=''
              return
           endif
        ! parse
           lspecial=.true.
           msg37_2=msg37(5:37)//'    '
           msg37=''
           msg37='DE '//trim(call_b)//' TU'
        endif

        ! DX Call searching false decodes, search for 1st callsign in ALLCALL7
        ! 6W6VIV EY8MM 73
        ! 6Y9KOZ EY8MM RR73
        if(iaptype.gt.34 .and. iaptype.lt.40 .and. (xsnr.lt.-21.0 .or. rxdt.lt.-0.5 &
             .or. rxdt.gt.1.0)) then
           ispc1=index(msg37,' ')
           if(ispc1.gt.1) then
              call_b=''
              call_b=msg37(1:ispc1-1)
              falsedec=.false.
              call chkflscallvar('CQ          ',call_b,falsedec)
              if(falsedec) then
                 nbadcrc=1
                 msg37=''
                 return
              endif
           endif
        endif*/
c4:    
        bool ldupemsg=false;
        if (s_ndecodes>0)
        {
            for (int i = 0; i<s_ndecodes; i++)
            {//do i=1,ndecodes
                if (allmessages[i]==msg37/* && !fabs(allfreq[i]-f1)<45.0*/)
                {
                    ldupemsg=true; //qDebug()<<"DUPE"<<msg37;
                    break;
                }
            }
        }
        if (!ldupemsg && dfqso<2.0 && ((i3==1 && !lft8s) || lft8s))
        {
            if (msg37.mid(0,msgroot.count()+1)==msgroot+" ")
            {
                lasthcall=s_HisCall8;
                lastrxmsg[0].lastmsg=msg37;
                lastrxmsg[0].xdt=xdt-0.5;
                lastrxmsg[0].lstate=true;
                lqsomsgdcd=true;
            }
            else if (!lft8s && mycalllen1>2)//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
            {
                if (msg37.mid(0,mycalllen1+1)!=s_MyCall8+" " && msg37.indexOf(" "+s_HisCall8+" ")>-1) lrepliedother=true;
            }
        }
        if (s_HisCall8.count()>3 && !lqsomsgdcd)//if(msg37(1:msgrootlen+1).eq.trim(msgroot)//' ') then
        {
            if (msg37.mid(0,msgroot.count()+1)==msgroot+" ") lqsomsgdcd=true;
        }

        if (!stophint && !ldupemsg && dfqso<2.0 && nlasttx>0 && nlasttx<6 && msg37.mid(0,3)=="CQ ")
        {
            int hcc = s_HisCall8.count();//nlength=len_trim(hiscall) CQ LZ2HV KN23
            if (hcc>2)
            {
                if (msg37.mid(3,hcc+1)==s_HisCall8+" " || msg37.mid(6,hcc+1)==s_HisCall8+" ") lrepliedother=true;
            }
        }

        if (mycalllen1>2)
        {
            if (!ldupemsg && msg37.mid(0,mycalllen1+1)==s_MyCall8+" ")
            {   //qDebug()<<nindex<<ldupemsg<<mycalllen1<<decid<<msg37;
                if (nindex<MAXINC_PerDec)
                {
                    msgincall[nindex]=msg37;
                    xdtincall[nindex]=xdt-0.5;
                    nindex++;
                }
            }
        }

        /*ldupeft8sd=.false.
        if(ncount.gt.0 .and. lft8sd) then
           ispc1=index(msg37,' ')
           ispc2=index(msg37((ispc1+1):),' ')+ispc1
           msgbase37=''
           msgbase37=msg37(1:ispc2-1)
           do i=1,ncount
              if(trim(msgbase37).eq.trim(msgsrcvd(i))) then
                 ldupeft8sd=.true.
                 exit
              endif
           enddo
        endif
        if(ldupeft8sd) then
           msg37=''
           nbadcrc=1
           return
        endif
        if(.not.ldupemsg .and. i3.eq.1 .and. .not.lft8sd .and. .not.lft8s .and.    &
             msg37(1:3).ne.'CQ ') then
           ispc1=index(msg37,' ')
           ispc2=index(msg37((ispc1+1):),' ')+ispc1
           ncount=ncount+1
           msgsrcvd(ncount)=msg37(1:ispc2-1)
        endif

        ! -23  0.0 1606 ~ <...> 3U1TBM/R CC65 i3=4 n3=0
        ! -18  0.3 1609 ~ CQ 6U6MBL/R IJ90 i3=1  in some noisy setups false decode with any SNR value is possible

        if((i3.eq.4 .and. n3.eq.0) .or. (i3.eq.1 .and. msg37(1:3).eq."CQ ")) then
           if(index(msg37,'/R ').gt.9) then
              ispc1=index(msg37,' ')
              ispc2=index(msg37((ispc1+1):),' ')+ispc1
              if(ispc2.gt.11) then
                 ispc3=index(msg37((ispc2+1):),' ')+ispc2
                 if(msg37((ispc2-2):(ispc2-1)).eq.'/R') then
                    if((ispc3-ispc2).eq.5) then
                       grid=msg37(ispc2+1:ispc3-1)
        ! grid can not be txed, invalid message:
                       if(i3.eq.4 .and. len_trim(grid).eq.4) then
                          nbadcrc=1
                          msg37=''
                          return
                       endif
                       call_b=''
                       call_b=msg37((ispc1+1):(ispc2-3))
                       call chkgridvar(call_b,grid,lchkcall,lgvalid,lwrongcall)
                       if(lwrongcall .or. .not.lgvalid) then
                          nbadcrc=1
                          msg37=''
                          return
                       endif
                    endif
                 endif
              endif
           endif
        endif

        ! protocol violations
        ! 713STG 869TK NO05  i3=2 n3=5, false decode, as per protocol type2 shall be /P message
        if(i3.eq.2 .and. index(msg37,'/P ').lt.1) then
           msg37=''
           nbadcrc=1
           return
        endif

        ! -18  0.5  584 ~ UA3ALE <...> PR07         *  AP decode with grid
        if(iaptype.eq.2) then
           nhash=index(msg37,"<...>")
           if(nhash.gt.4 .and. nhash.lt.13 .and. msg37(nhash+6:nhash+6).gt.'@' .and. &
                msg37(nhash+7:nhash+7).gt.'@') then
              msg37=''
              nbadcrc=1
              return
           endif
        endif*/

        if (lsubtract)
        {
            int noff=10;//10
            double sync0=0.0;
            double syncp=0.0;
            double syncm=0.0;
            int k=0;//1
            double complex *csig0 = new double complex[154681];//151680
            //for (int i = 0; i < 151682; ++i) csig0[i]=0.0+0.0*I;
            gen_ft8cwaveRx(itone,0.0,csig0);//call gen_ft8wavevar(itone,79,1920,2.0,12000.0,0.0,csig0,xjunk,1,151680)
            //gen_ft8cwaveRxAp8(itone,0.0,csig0);
            for (int i = 0; i < 79; ++i)
            {//do i=0,78
                for (int j = 0; j < 32; ++j)
                {//do j=1,32
                    csig[j]=csig0[k];
                    k+=60; //if (qIsNaN(cabs(csig[j]))) qDebug()<<j<<"csig NAN";
                } //i0+=1;
                int i21=i0+i*32;
                double complex z1=0.0+0.0*I;
                for (int z = 0; z < 32; ++z) z1+=cd0[z+i21+cd_off]*conj(csig[z]);//z1=sum(cd0(i21:i21+31)*conjg(csig))
                sync0 += pomAll.ps_hv(z1);//real(z1)**2 + aimag(z1)**2
                i21=i0+i*32+noff;
                z1=0.0+0.0*I;
                for (int z = 0; z < 32; ++z) z1+=cd0[z+i21+cd_off]*conj(csig[z]);//z1=sum(cd0(i21:i21+31)*conjg(csig))
                syncp += pomAll.ps_hv(z1);//syncp + real(z1)**2 + aimag(z1)**2
                i21-=noff*2;
                z1=0.0+0.0*I;
                for (int z = 0; z < 32; ++z) z1+=cd0[z+i21+cd_off]*conj(csig[z]);//z1=sum(cd0(i21:i21+31)*conjg(csig))
                syncm += pomAll.ps_hv(z1);//syncm + real(z1)**2 + aimag(z1)**2;
            }
            double dx=pomAll.peakup(syncm,sync0,syncp);
            /*if (qIsNaN(syncm)) qDebug()<<"syncm"<<syncm;
            if (qIsNaN(sync0)) qDebug()<<"sync0"<<sync0;
            if (qIsNaN(syncp)) qDebug()<<"syncp"<<syncp;*/
            double scorr=0.0;
            if (fabs(dx)>1.0) scorr=0.0;
            else scorr=(double)noff*dx; //! was * dx ft8md
            double xdt3=xdt+scorr*dt2; //qDebug()<<dx<<syncm<<sync0<<syncp;
            subtractft8var(itone,f1,xdt3); //subtractft8var(itone,f1,xdt3) subtractft8(dd8,itone,f1,xdt,true);
            lsubtracted_=true; //! inside current thread //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
            if (npos<200)
            {
                freqsub[npos]=f1;
                npos++;
            }
            delete [] csig0;
        }
    }
//end:
    //return;
    //delete [] cd1;
    //delete [] cd0;
    //delete [] cd2;
    //delete [] cd3; //qDebug()<<"3 END";
}
void DecoderFt8::extract_callvar(QString msg37,QString &call2)
{
    call2="";
    QString part2="";
    QString part3="";
    QString msgd = msg37+"      "; //msgd = "LZ2HV RR73; SP9HWY <A3BB> -16"; msgd = "CQ LZ26666HV";
    int ispc1=msgd.indexOf(' ');
    int ispc2=msgd.indexOf(' ',ispc1+1);
    int ispc3=msgd.indexOf(' ',ispc2+1);
    part2=msgd.mid(ispc1+1,ispc2-(ispc1+1));//msgd(ispc1+1:ispc2-1);
    part3=msgd.mid(ispc2+1,ispc3-(ispc2+1));//msgd(ispc2+1:ispc3-1);
    part2=part2.trimmed();
    part3=part3.trimmed();
    int npart2len=part2.count();//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    if (msgd.mid(0,3)=="CQ " || msgd.mid(0,3)=="DE " || msgd.mid(0,4)=="QRZ ")
    {
        if (npart2len>4) call2=part2;
        else if (npart2len==4)
        {
            QChar c2 = part2.at(1);
            QChar c3 = part2.at(2);
            if ((c2>='0' && c2<':') || (c3>='0' && c3<':')) call2=part2;
            else call2=part3;
        }
        else if (npart2len==3)
        {
            QChar c1 = part2.at(0);
            QChar c2 = part2.at(1);
            if (c1>='A' && c1<'[' && c2>='0' && c2<':') call2=part2;
            else call2=part3;
        }
        else if (npart2len==2) call2=part3;
    }
    else if (ispc1>1 && ispc1<12) call2=part2;
    if (msgd.indexOf(" RR73; ")>-1 && msgd.indexOf("<")>-1)
    {
        if (msgd.count(QLatin1Char(' '))>=4)
        {
            int ispc4=msgd.indexOf(' ',ispc3+1);
            QString part4=msgd.mid(ispc3+1,ispc4-(ispc3+1));
            part4.remove("<");
            part4.remove(">");
            call2=part4;
        }
    } //qDebug()<<msg37<<"call2="<<call2<<"part2="<<part2<<"part3="<<part3;
}
void DecoderFt8::ft8_SetStart_ev_od_var(bool f_eve0_odd1)
{
    nmsgloc=0;
    for (int i = 0; i < MAXStatOEt; ++i)
    {
        eventmp[i].lstate=false;
        oddtmp[i].lstate=false;
    }
    if (f_statis_all_new_p)
    {
        f_statis_all_new_p = false; //qDebug()<<decid<<"START ALL PREV s_nmsg="<<s_nmsg;
        if (s_fopen8)
        {
            for (int i = 0; i < MAXStatOE; ++i)
            {
                evencopy[i].lstate=false;
                oddcopy[i].lstate =false;
                even[i].lstate=false;
                odd[i].lstate =false;
            }
            for (int i = 0; i < MAXINC; ++i) incall[i].msg="  ";
        } //int cc = 0;
        if (!f_eve0_odd1)//levenint
        {
            for (int i = 0; i < MAXStatOE; ++i)
            {
                evencopy[i].msg=even[i].msg;
                evencopy[i].freq=even[i].freq;
                evencopy[i].dt=even[i].dt;
                evencopy[i].lstate=even[i].lstate;
                even[i].lstate=false; //if (evencopy[i].lstate) cc++;
            }   //printf("    Start DEC= %d Eve Copy--> %d\n",decid,cc);
        }
        else//loddint
        {
            for (int i = 0; i < MAXStatOE; ++i)
            {
                oddcopy[i].msg=odd[i].msg;
                oddcopy[i].freq=odd[i].freq;
                oddcopy[i].dt=odd[i].dt;
                oddcopy[i].lstate=odd[i].lstate;
                odd[i].lstate=false; //if (oddcopy[i].lstate) cc++;
            }   //printf("    Start DEC= %d Odd Copy--> %d\n",decid,cc);
        }
        s_nmsg=0;
        c_xdtt=0;
    }
}
void DecoderFt8::ft8_Write_evt_odt_var(QString msg37,int i3,bool f_eve0_odd1,double f1,double xdt,
                                       bool lFreeText,bool lhashmsg,bool lft8sd)
{
    if (i3==4 && msg37.mid(0,3)=="CQ " && nmsgloc<MAXStatOEt)// && (nsec % 15)==0
    {
        if (!f_eve0_odd1)//levenint
        {
            eventmp[nmsgloc].msg=msg37;
            eventmp[nmsgloc].freq=f1;
            eventmp[nmsgloc].dt=xdt;
            eventmp[nmsgloc].lstate=true;
        }
        else//if (loddint)
        {
            oddtmp[nmsgloc].msg=msg37;
            oddtmp[nmsgloc].freq=f1;
            oddtmp[nmsgloc].dt=xdt;
            oddtmp[nmsgloc].lstate=true;
        }
        nmsgloc++;
        return;
    }
    if (!lFreeText)//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    {   //! protection against any possible free txtmsg bit corruption
        int ispc1=msg37.indexOf(" ");//ispc1=index(msg37,' ') // && (nsec % 15)==0
        if (!lhashmsg && ((i3==1 && !lft8sd) || lft8sd) && msg37.mid(0,ispc1)!=s_MyCall8 && nmsgloc<MAXStatOEt && msg37.indexOf("<")<=-1)
        {   //! compound callsigns not supported
            if (msg37.indexOf("/")>-1 && msg37.mid(0,3)!="CQ ") return;
            if (!f_eve0_odd1)//levenint
            {
                eventmp[nmsgloc].msg=msg37;
                eventmp[nmsgloc].freq=f1;
                eventmp[nmsgloc].dt=xdt;
                eventmp[nmsgloc].lstate=true;
            }
            else//if (loddint)
            {
                oddtmp[nmsgloc].msg=msg37;
                oddtmp[nmsgloc].freq=f1;
                oddtmp[nmsgloc].dt=xdt;
                oddtmp[nmsgloc].lstate=true;
            }
            nmsgloc++;
        }
    }
}
void DecoderFt8::ft8_decodevar(double *dd,int c_dd,double nfa,double nfb,double nfqso,bool &have_dec,
                               double nfawide,double nfbwide,bool nagainfil,bool lqsothread,bool f_eve0_odd1)
{
    int cont_type = 0; //qDebug()<<"IN="<<decid<<"nagain="<<nagainfil<<"qsothread="<<lqsothread<<"try_a8="<<s_ltry_a8;
    int cont_id   = 0; //printf("DEC=%d F0=%04d FQSO=%04d F1=%04d\n",decid,(int)nfa,(int)nfqso,(int)nfb);
    if (!f_multi_answer_mod8)
    {
        cont_type = s_ty_cont_ft8_28;
        cont_id   = s_id_cont_ft8_28;
    }
    first_ft8b_and_ft8var(s_HisCall8,cont_id);
    nlasttx=s_nlasttx;
    double napwid = s_napwid8;  //qDebug()<<decid<<napwid8;
    /*!nlasttx  last TX message
    !  0       Tx was halted
    !  1      AA1AA BB1BB PL35
    !  2      AA1AA BB1BB -15
    !  3      AA1AA BB1BB R-15
    !  4      AA1AA BB1BB RRR/RR73
    !  5      AA1AA BB1BB 73
    !  6      CQ BB1BB PL35*/
    for (int x = 0; x < c_dd; ++x)
    {
        dd8[x]=dd[x];
        dd8m[x]=0.0;
    }
    for (int x = c_dd; x < 185000; ++x)
    {
        dd8[x]=0.0;
        dd8m[x]=0.0;
    } //if (decid==0) printf("Tme=%06d RawCount=%d\n",s_time8.toInt(),c_dd);//50*3456=172800samples=14.4sec

    nft8cycles = s_nft8cycles;
    bool lft8lowth = true;
    bool lft8subpass = true;
    if (s_nft8sens==1)
    {
        lft8lowth = false;
        lft8subpass = false;
    }
    if (s_nft8sens==2)
    {
        lft8lowth = true;
        lft8subpass = false;
    }
    if (s_nft8sens==3)
    {
        lft8lowth = true;//lft8lowth = false;//in jt9
        lft8subpass = true;
    } //qDebug()<<"s_Cycles="<<s_nft8cycles<<"Lowth="<<lft8lowth<<"Subpass="<<lft8subpass;

    int nthr = 2;//threads count ??
    int nallocthr=0;
    int nft8rxfsens=3;//1,2,3 QSO RX freq Sensitivity
    lqsomsgdcd=false;//HV if no early decode period, make only if full period
    int ncandthin = 100;//100;//98;//??
    int ndtcenter = 0;//??
    bool lhiscallstd=s_lhiscallstd;
    bool lmycallstd=s_lmycallstd;
    msgroot=s_MyCall8+" "+s_HisCall8;

    double candidate[4][512];//double (*candidate)[500] = new double[4][500];//
    bool stophint = false;

    ncqsignal=0;
    nmycsignal=0;
    int nmsgcq=0;
    int nmsgmyc=0;
    lft8sdec=false;
    lrepliedother=false;
    lsubtracted_=false;
    ncount=0;
    double qual=0.0;

    if (s_fopen8)// && f_new_p
    {
        for (int i = 0; i < numcqsig; ++i)
        {
            evencq[i].freq=6000.0;
            oddcq[i].freq=6000.0;
            tmpcqsig[i].freq=6000.0;
        }
        for (int i = 0; i < nummycsig; ++i)
        {
            evenmyc[i].freq=6000.0;
            oddmyc[i].freq=6000.0;
            tmpmycsig[i].freq=6000.0;
        }
        tmpqsosig[0].freq=6000.0;
        evenqso[0].freq=6000.0;
        oddqso[0].freq=6000.0;
        avexdt=0.0;
        nintcount = ERSET_IN;
        /*for (int i = 0; i < ncalldt; ++i)
        {
           	calldteven[i].call2="";
           	calldtodd[i].call2 ="";
        }*/
    }    

    for (int i = 0; i < numdeccq; ++i) tmpcqdec[i].freq=6000.0;
    for (int i = 0; i < numdecmyc; ++i) tmpmyc[i].freq=6000.0;
    if (s_HisCall8=="") lastrxmsg[0].lstate=false;
    else if (lastrxmsg[0].lstate && lasthcall!=s_HisCall8 && lastrxmsg[0].lastmsg.indexOf(s_HisCall8)<=-1) lastrxmsg[0].lstate=false;

    mycalllen1=s_MyCall8.count();//len_trim(mycall)+1
    nindex=0;

    bool lnohiscall=false;
    if (s_HisCall8.count()<3) lnohiscall=true;
    bool lnomycall=false;
    if (s_MyCall8.count()<3) lnomycall=true;
    bool lnohisgrid=false;
    if (s_HisGrid8.count()<4) lnohisgrid=true;

    if (lqsothread && !lastrxmsg[0].lstate && !stophint && s_HisCall8!="")
    {
        for (int i = 0; i < MAXINC; ++i)//! got incoming call
        {//do i=1,30
            QString tms = incall[i].msg+"    ";
            if (tms.mid(0,1)==" ") break;
            if (tms.indexOf(s_MyCall8+" "+s_HisCall8)==0)
            {
                lastrxmsg[0].lastmsg=incall[i].msg;
                lastrxmsg[0].xdt=incall[i].xdt;
                lastrxmsg[0].lstate=true;
                break;
            }
        }
        if (!lastrxmsg[0].lstate)
        {	//! calling someone, lastrxmsg still not valid
            if (!f_eve0_odd1)
            {
                for (int i = 0; i < MAXStatOE; ++i)
                {//do i=1,130
                    if (!evencopy[i].lstate) break;
                    if (evencopy[i].msg.indexOf(" "+s_HisCall8+" ")>0)
                    {
                        lastrxmsg[0].lastmsg=evencopy[i].msg;
                        lastrxmsg[0].xdt=evencopy[i].dt;
                        lastrxmsg[0].lstate=true;
                        break;
                    }
                }
            }
            else// if (loddint)
            {
                for (int i = 0; i < MAXStatOE; ++i)
                {//do i=1,130
                    if (!oddcopy[i].lstate) break;
                    if (oddcopy[i].msg.indexOf(" "+s_HisCall8+" ")>0)
                    {
                        lastrxmsg[0].lastmsg=oddcopy[i].msg;
                        lastrxmsg[0].xdt=oddcopy[i].dt;
                        lastrxmsg[0].lstate=true;
                        break;
                    }
                }
            }
        }
    }
    const int jz_max = 90;//max=120;
    const int jz_min = 40;//min=??;
    int JZ = (62+avexdt*25.0);
    if (JZ>jz_max) JZ=jz_max;
    if (JZ<jz_min) JZ=jz_min;
    int jzb=-JZ;//jzb=-62 + avexdt*25.
    int jzt=JZ; //qDebug()<<"START->"<<decid<<avexdt<<jzb<<jzt;//jzt=62 + avexdt*25.
    int npass=3; //! fallback
    if (nft8cycles==1) npass=3;
    else if (nft8cycles==2) npass=6;
    else if (nft8cycles==3) npass=9;
    else npass=3;//=100	one dec
    int nQSOProgress = s_nQSOProgress8;

	//agccft8(nfa,nfb); qDebug()<<"-----------------------"<<lagccbail;
    ft8apsetvar(s_lmycallstd,s_lhiscallstd);

    double syncmin=1.33;//1.3 ws300rc1 //qDebug()<<"-->"<<s_MyCall8<<s_lmycallstd<<s_HisCall8<<s_lhiscallstd<<nQSOProgress;
    for (int ipass = 1; ipass <= npass; ++ipass)//do ipass=1,npass
    {
        bool newdat1=true;
        bool lsubtract=true;
        npos=0; //qDebug()<<"ipass-->"<<ipass;
        if (ipass==1 || ipass==4 || ipass==7)
        {
            if (lft8lowth) syncmin=1.225;
        }
        else if (ipass==2 || ipass==5 || ipass==8)
        {
            if (lft8lowth) syncmin=1.33;//1.3 ws300rc1
        }
        else if (ipass==3 || ipass==6 || ipass==9)
        {
            if (lft8lowth) syncmin=1.1;//1.1;
        }
        if (ipass>5 || (ipass==3 && npass==3)) lsubtract=false;
        if (ipass==4)
        {
            if (npass==9) //! 3 decoding cycles
            {
                nallocthr=nthr;
                for (int x = 0; x < 180000; ++x) dd8m[x]=dd8[x];
            }
            for (int i = 0; i < 179999; ++i)
            {//do i=1,179999
                dd8[i]=(dd8[i]+dd8[i+1])*0.5;///2.0;
            } //qDebug()<<"1 dd8="<<ipass<<npass;
        }
        else if (ipass==7)
        {
            if (nthr==nallocthr)
            {
                dd8[0]=dd8m[0];
                for (int i = 1; i < 180000; ++i) dd8[i]=(dd8m[i-1]+dd8m[i])*0.5;///2.0; //do i=2,180000
            } //qDebug()<<"2 dd8="<<ipass<<npass;
        }
        int ncand = 0; //qDebug()<<"-001-ncand="<<ipass<<npass;
        /*if (ipass == 1)
        {
        	for (int ic = 166000; ic < 180600; ++ic) dd8[ic]=0.00000001;//2.70	
        }*/
        //agccft8(nfa,nfb); qDebug()<<"-----------------------"<<lagccbail;
        sync8var(nfa,nfb,syncmin,nfqso,candidate,ncand,jzb,jzt,ipass,lqsothread,ncandthin,ndtcenter,nfawide,nfbwide);
        //qDebug()<<decid<<"-----------------------";
        /*if (ipass == 1)
        {
        	for (int ic = 166000; ic < 180600; ++ic) dd8[ic]=0.00000001;//2.70	
        }*/
        if (ipass == 1 && !s_fopen8)
        {
            float frst_dd = 5.0;    //error val = 5s
            float last_dd = -1.0;   //error val = -1
            for (int ic = 0; ic < ncand; ++ic)
            {
                float cand_start = (float)candidate[1][ic]; //if (cand_start > 2.0) qDebug()<<"u="<<cand_start;
                if (cand_start < 2.5 && cand_start>last_dd) last_dd = cand_start; //30000   +2.5s
                if (cand_start > 0.0 && cand_start<frst_dd) frst_dd = cand_start;
            }
            if (frst_dd != 5.0 && last_dd != -1.0)//no error
            {
                int zero_dd = ((frst_dd+last_dd)/2.0)*12000.0 + 151680 + 10000;//151680+1200=frame+100ms  frame+360ms=156000
                for (int ic = zero_dd; ic < 180700; ++ic) dd8[ic]=0.00000001;//qDebug()<<decid<<"S="<<frst_dd<<last_dd<<zero_dd<<c_dd;
            }
        }
        for (int icand = 0; icand < ncand; ++icand)
        {//do icand=1,ncand
            double sync=candidate[2][icand];
            double f1=candidate[0][icand];
            double xdt=candidate[1][icand]; //if (f1>2722 && f1<2728) qDebug()<<"-2-icand"<<ipass<<icand<<ncand<<sync<<f1<<xdt;
            bool lcqcand=false; //xdt=0.96;
            if (candidate[3][icand]>1.0) lcqcand=true;
            bool lhighsens=false;//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
            if (sync<1.9 || ((ipass==2 || ipass==4 || ipass==6) && sync<3.15)) lhighsens=true;
            bool lspecial=false;   //if (ipass==2 && sync>10) continue; if (ipass==5 && sync>10) continue;
            bool lFreeText=false;  //if (ipass==6 && sync>10) continue;
            //int i3bit=0;
            //bool lft8s=false;
            bool lft8sd=false;
            bool lhashmsg=false;
            //int iaptype=0;
            int iaptype2=0;
            QString msg37="";
            int i3=16;
            //int n3=16;
            double xsnr=0.0;
            int nbadcrc=0;  //qDebug()<<"In="<<decid<<ipass<<icand<<ncand<<"---"<<sync<<xdt<<f1;
            ft8bvar(newdat1,nQSOProgress,nfqso,s_nftx8,lsubtract,nagainfil,f1,xdt,nbadcrc,msg37,xsnr,
                    stophint,lFreeText,ipass,lft8subpass,lspecial,lcqcand,npass,lmycallstd,lhiscallstd,
                    f_eve0_odd1,lft8sd,i3,nft8rxfsens,lhashmsg,lqsothread,lft8lowth,lhighsens,lnohiscall,
                    lnomycall,lnohisgrid,qual,iaptype2,cont_type,napwid);
            int nsnr=(int)xsnr;
            xdt=xdt-0.5; //if (sync<1.10) qDebug()<<"Out="<<decid<<icand<<ncand<<msg37<<sync<<f1;
            if (nbadcrc==0)
            {
                //lhidemsg=false;
                int nspecial=1;
                if (lspecial) nspecial=2;
                //else nspecial=1; //msg37="LZ2HV RR73; SP9HWY <A3BB> -16";
                //nspecial=2;
                for (int k = 1; k <= nspecial; ++k)
                {//do k=1,nspecial
                    //!ft8md  if(k.eq.2) msg37=msg37_2  ! this splits DXpedition mode msg into 2 lines
                    bool ldupe=false;
                    if (msg37.mid(0,6)=="      ") ldupe=true;
                    for (int idec = 0; idec < s_ndecodes; ++idec)//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
                    {
                        if (msg37==allmessages[idec])//2.48 problem 2xRR73 no end QSO && nsnr<=allsnrs[id]
                        {
                            ldupe=true; //qDebug()<<"DUPE"<<msg37;
                            break;
                        }
                    } //qDebug()<<ldupe<<lFreeText<<k<<nspecial<<msg37<<"call2="<<call2;
                    if (msg37.count()<1) ldupe=true;//2.76.6 SD error 
                    if (!ldupe)
                    {
                    	QString call2;
                        if (!lFreeText && k==1) extract_callvar(msg37,call2);//no in MSHV splits DXpedition mode msg into 2 lines
                        allmessages[s_ndecodes]=msg37;
                        xdt_per_dec[s_ndecodes]=xdt;
                        if (s_ndecodes < (ALL_MSG_SNR-1)) s_ndecodes++;
                        //if(.not.lhidemsg) then
                        //! simulated wav tests affected, structure contains data for at least previous and current even|odd intervals
                        if (!f_eve0_odd1)
                        {
                            for (int z = ncalldt-1; z >=1 ; --z) calldteven[z]=calldteven[z-1];//calldteven(150:2:-1)=calldteven(150-1:1:-1)
                            calldteven[0].call2=call2;
                            calldteven[0].dt=xdt; //qDebug()<<"ft8b Stop"<<call2;
                        }
                        else// if (loddint)
                        {
                            for (int z = ncalldt-1; z >=1 ; --z) calldtodd[z]=calldtodd[z-1];
                            calldtodd[0].call2=call2;
                            calldtodd[0].dt=xdt;
                        }
                        QString msg26=msg37; //qDebug()<<"OUT="<<msg26<<sync;
                        PrintMsg(s_time8,nsnr,xdt,f1,msg26,iaptype2,(float)qual,(float)qual,have_dec,true,true);
                        if (msg37.mid(0,3)=="CQ " && nmsgcq<numdeccq)
                        {
                            double xdtr=xdt+0.5;
                            tmpcqdec[nmsgcq].freq=f1;
                            tmpcqdec[nmsgcq].xdt=xdtr;
                            nmsgcq++;
                        }
                        if (lapmyc && lmycallstd)
                        {
                            int ispc1=msg37.indexOf(" ");
                            if (msg37.mid(0,ispc1)==s_MyCall8 && nmsgmyc<numdecmyc)
                            {
                                double xdtr=xdt+0.5;
                                tmpmyc[nmsgmyc].freq=f1;
                                tmpmyc[nmsgmyc].xdt=xdtr;
                                nmsgmyc++; //qDebug()<<nmsgmyc<<f1;
                            }
                        }
                        ft8_Write_evt_odt_var(msg37,i3,f_eve0_odd1,f1,xdt,lFreeText,lhashmsg,lft8sd);
                    }//c4: //if (k < nspecial) continue;
                }//!do k
            }//else { if (sync>10) {qDebug()<<"no decode="<<ipass<<sync;}}
        }//!icand
    }
    if (!f_eve0_odd1)
    {
        for (int i = 0; i < ncqsignal; ++i)
        {
            evencq[i].freq=tmpcqsig[i].freq;
            evencq[i].xdt=tmpcqsig[i].xdt;
            for (int z = 0; z < 79; ++z)
            {
                for (int z0 = 0; z0 < 8; ++z0) evencq[i].cs_[z][z0]=tmpcqsig[i].cs_[z][z0];
            } //qDebug()<<"---"<<decid<<i<<tmpcqsig[i].freq;
        }
        if (lapmyc)
        {
            for (int i = 0; i < nmycsignal; ++i)
            {
                evenmyc[i].freq=tmpmycsig[i].freq;
                evenmyc[i].xdt=tmpmycsig[i].xdt;
                for (int z = 0; z < 79; ++z)
                {
                    for (int z0 = 0; z0 < 8; ++z0)  evenmyc[i].cs_[z][z0]=tmpmycsig[i].cs_[z][z0];
                } //qDebug()<<"---"<<decid<<i<<tmpmycsig[i].freq;
            }
            if (!lqsomsgdcd && tmpqsosig[0].freq<5001.0)
            {
                evenqso[0].freq=tmpqsosig[0].freq;
                evenqso[0].xdt=tmpqsosig[0].xdt;
                for (int z = 0; z < 79; ++z)
                {
                    for (int z0 = 0; z0 < 8; ++z0) evenqso[0].cs_[z][z0]=tmpqsosig[0].cs_[z][z0];
                }
            }
        }
    }
    else// if (loddint)
    {
        for (int i = 0; i < ncqsignal; ++i)
        {
            oddcq[i].freq=tmpcqsig[i].freq;
            oddcq[i].xdt=tmpcqsig[i].xdt;
            for (int z = 0; z < 79; ++z)
            {
                for (int z0 = 0; z0 < 8; ++z0) oddcq[i].cs_[z][z0]=tmpcqsig[i].cs_[z][z0];
            }
        }
        if (lapmyc)
        {
            for (int i = 0; i < nmycsignal; ++i)
            {
                oddmyc[i].freq=tmpmycsig[i].freq;
                oddmyc[i].xdt=tmpmycsig[i].xdt;
                for (int z = 0; z < 79; ++z)
                {
                    for (int z0 = 0; z0 < 8; ++z0)  oddmyc[i].cs_[z][z0]=tmpmycsig[i].cs_[z][z0];
                }
            } //qDebug()<<"-----------------------"<<nmycsignal;
            if (!lqsomsgdcd && tmpqsosig[0].freq<5001.0)
            {
                oddqso[0].freq=tmpqsosig[0].freq;
                oddqso[0].xdt=tmpqsosig[0].xdt;
                for (int z = 0; z < 79; ++z)
                {
                    for (int z0 = 0; z0 < 8; ++z0) oddqso[0].cs_[z][z0]=tmpqsosig[0].cs_[z][z0];
                }
            }
        }
    }
    if (nmsgloc>0)
    {
        if (!f_eve0_odd1)
        {
            int r_nmsgloc = 0; //qDebug()<<decid<<"IN"<<nmsgloc;
            if ((s_nmsg+nmsgloc)>MAXStatOE) nmsgloc=MAXStatOE-s_nmsg; //qDebug()<<"2 leven"<<MAXStatOE<<s_nmsg+nmsgloc<<"RES="<<nmsgloc;
            for (int i = 0; i < nmsgloc; ++i)
            {
                bool f_dup=false;
                for (int z = 0; z < s_nmsg; ++z)
                {
                    if (even[z].msg==eventmp[i].msg)
                    {
                        f_dup=true; //qDebug()<<decid<<"dupe";
                        break;
                    }
                }
                if (f_dup) continue;
                even[r_nmsgloc+s_nmsg].msg=eventmp[i].msg;
                even[r_nmsgloc+s_nmsg].freq=eventmp[i].freq;
                even[r_nmsgloc+s_nmsg].dt=eventmp[i].dt;
                even[r_nmsgloc+s_nmsg].lstate=eventmp[i].lstate; //qDebug()<<"Save Eev all <--"<<eventmp[i].msg;
                //xdtt[r_nmsgloc+s_nmsg]=eventmp[i].dt;
                r_nmsgloc++;
            }
            s_nmsg+=r_nmsgloc;
        }
        else// if (loddint)
        {
            int r_nmsgloc = 0;
            if ((s_nmsg+nmsgloc)>MAXStatOE) nmsgloc=MAXStatOE-s_nmsg;
            for (int i = 0; i < nmsgloc; ++i)
            {
                bool f_dup=false;
                for (int z = 0; z < s_nmsg; ++z)
                {
                    if (odd[z].msg==oddtmp[i].msg)
                    {
                        f_dup=true; //qDebug()<<decid<<"dupe";
                        break;
                    }
                }
                if (f_dup) continue;
                odd[r_nmsgloc+s_nmsg].msg=oddtmp[i].msg;
                odd[r_nmsgloc+s_nmsg].freq=oddtmp[i].freq;
                odd[r_nmsgloc+s_nmsg].dt=oddtmp[i].dt;
                odd[r_nmsgloc+s_nmsg].lstate=oddtmp[i].lstate; //qDebug()<<"Save Odd all <--"<<oddtmp[i].msg;
                //xdtt[r_nmsgloc+s_nmsg]=oddtmp[i].dt;
                r_nmsgloc++;
            }
            s_nmsg+=r_nmsgloc;
        }
    }
    if (!f_eve0_odd1)
    {
        //printf(" Save DEC= %d Eve s_nmsg= %d\n",decid,s_nmsg);
        for (int i = s_nmsg; i < MAXStatOE; ++i)// even[i].lstate=false;
        {
            if (even[i].lstate) even[i].lstate=false;
        }
    }
    else//if (loddint)
    {
        //printf(" Save DEC= %d Odd s_nmsg= %d\n",decid,s_nmsg);
        for (int i = s_nmsg; i < MAXStatOE; ++i)// odd[i].lstate=false;
        {
            if (odd[i].lstate) odd[i].lstate=false;
        }
    }

    for (int z = 0; z < nindex; ++z)
    {
        for (int z0 = MAXINC-1; z0 >= 1; --z0) incall[z0]=incall[z0-1];//incall(30:2:-1)=incall(30-1:1:-1)
        incall[0].msg=msgincall[z];
        incall[0].xdt=xdtincall[z];
    } //for (int z = 0; z < 29; ++z) qDebug()<<decid<<incall[z].msg; qDebug()<<"------------------";

    int t_ndecodes = s_ndecodes;
    if ((c_xdtt+t_ndecodes)>MAXStatOE) t_ndecodes=MAXStatOE-c_xdtt;
    if (t_ndecodes>0)
    {
        for (int i = 0; i < t_ndecodes; ++i)
        {
            /*bool f_dup=false;
            for (int z = 0; z < c_xdtt; ++z)
            {
                if (xdtt[z]==xdt_per_dec[i])
                {
                    f_dup=true;//qDebug()<<decid<<"dupe";
                    break;
                }
            }
            if (f_dup) continue;*/
            xdtt[c_xdtt]=xdt_per_dec[i];
            c_xdtt++;
        }
    }
    TryAp8(dd8,s_lapon8,3,cont_type,nfqso,have_dec);
}







