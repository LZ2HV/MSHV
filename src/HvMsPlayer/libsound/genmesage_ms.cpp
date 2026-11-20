/* The algorithms, source code, look-and-feel of WSJT-X and related
 * programs, and protocol specifications for the modes FSK441, FT8, JT4,
 * JT6M, JT9, JT65, JTMS, QRA64, ISCAT, MSK144, are Copyright © 2001-2017
 * by one or more of the following authors: Joseph Taylor, K1JT; Bill
 * Somerville, G4WJS; Steven Franke, K9AN; Nico Palermo, IV3NWV; Greg Beam,
 * KI7MT; Michael Black, W9MDB; Edson Pereira, PY2SDR; Philip Karn, KA9Q;
 * and other members of the WSJT Development Group.
 *
 * MSHV Generator 
 * Rewritten into C++ and modified by Hrisimir Hristov, LZ2HV 2015-2017
 * May be used under the terms of the GNU General Public License (GPL)
 */

#include "mpegsound.h"
#include <math.h>
//using namespace std;

#define _FSK441_DH_
#define _JTMS_DH_
#define _ISCAT_DH_
#include "../../config_msg_all.h"

//#include <QtGui>

int GenMessage::abc441(char*msg,int count_msg)
{
    int count;// = 3*count_msg;
    // samo golemi bukwi HV

    for (int i = 0; i < count_msg; i++)
    {
        int j;
        j = (int)msg[i];
        //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
        if (j<0 || j>91)
            j=32;
        int n;
        n = lookup_FSK441_TXRX[j];
        itone_s[3*(i+1)-3]=n/16 + 1;
        itone_s[3*(i+1)-2]=fmod(n/4,4) + 1;
        itone_s[3*(i+1)-1]=fmod(n,4) + 1;
        //qDebug()<<j<<n;
    }
    //qDebug()<<"itone1_441"<<3*(count_msg+1)-1;
    count=3*count_msg;
    return count;
}
int GenMessage::genms(char*msg28,double samfac,int isrch)
{
    int k = 0;
    // hv v1.01 29 to 59
    char msg[59+1] = {' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',
                      ' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',
                      ' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',
                      ' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',
                      ' ',' '};
    /// //  2 2 2  2  4  2  4  6
    //data np/5,7,9,11,13,17,19,23,29/  !Permissible message lengths
    int np[13] ={5,7,9,11,13,17,19,23,29,31,33,35,37};// hv v.1.01 {5,7,9,11,13,17,19,23,29} to {5,7,9,11,13,17,19,23,29,31,33,35,37,39}
    int sent[203*2];

	//!                    1         2         3         4         5         6
	//!          0123456789012345678901234567890123456789012345678901234567890123
	//  data cc/'0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ./?-                 _     @'/

    double dt,phi,f,f0,dfgen,dphi,foffset;
    int NMAX=30*GEN_SAMPLE_RATE;

    //qDebug()<<"strlen(msg)";
    for (int z = 0; z < 58; z++)// hv v1.01 28 to 58
    {
        msg[z] = msg28[z];//Extend to 29 characters
    }

    int i =0;
    for (i = 58; i >= 0; i--)// hv v1.01 28 to 58
    {
        if (msg[i]!=' ')
            break;
    }
    int iz=i+2;  //hv ok +1 interval nakraia !Add one for space at EOM
    int msglen=iz;
    //qDebug()<<msg<<msglen;

    if (isrch!=0) goto c3;//hv ????
//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    for (i = 0; i < 13; i++)//hv v.1.01 9 to 23
    {
        if (np[i]>=iz) goto c2;
    }
    i=7;//f90 = 8 c++ = 7
c2:
    msglen=np[i];//???hv
    //qDebug()<<msglen<<i<<iz;

//! Convert message to a bit sequence, 7 bits per character (6 + even parity)
c3: // sent=0;
    k=0;
    for (int j = 0; j < msglen; j++)
    {
        if (msg[j]==' ')
        {
            i=57;//v negovata 58 no go vra6ta -1 posle
            goto c5;
        }
        else
        {
            for (i = 0; i < 64; i++)
            {
                if (msg[j]==cc_JTMS_TX[i])
                {
                    //qDebug()<<msg[j];
                    goto c5;
                }
            }
        }

c5:
        //qDebug()<<i;
        /*
        AND 	IAND 	IAND(N,M) 	& 	n&m 	.
        OR 	IOR 	IOR(N,M) 	| 	n|m 	.
        Exclusive OR 	IEOR 	IEOR(N,M) 	^ 	n^m 	.
        1's Complement 	INOT 	INOT(N) 	~ 	~n 	.
        Left Shift 	ISHFT 	ISHFT(N,M) (M > 0) 	<< 	n<<m 	n shifted left by m bits
        Right Shift 	ISHFT 	ISHFT(N,M) (M < 0) 	>> 	n>>m 	n shifted right by m bits
        */

        int m=0;
        for (int n = 5; n >= 0; n--)
        {
            sent[k]=1 & (i >> n);//sent(k)=iand(1,ishft(i-1,-n))
            m=m+sent[k];
            //qDebug()<<sent[k]<<i<<n<<k;
            k++;
        }

        sent[k]=m & 1;  //sent(k)=iand(m,1)                   //!Insert parity bit
        //qDebug()<<sent[k]<<"parity"<<k;
        k++;
    }
    int nsym=k;
    //qDebug()<<k;

	//! Set up necessary constants
    int nsps=8*koef_srate; //111 to4no 4islo
    dt=1.0/(samfac*GEN_SAMPLE_RATE);
    f0=(double)GEN_SAMPLE_RATE/nsps;  //qDebug()<<f0;                             // 1575.0 Hz
    dfgen=0.5*f0;       //qDebug()<<dfgen;                          //  787.5 Hz
    foffset=1500.0 - f0;
    k=0;
    phi=0.0;
    int nrpt=(double)NMAX/(nsym*nsps); //qDebug()<<"1"<<nrpt;
    //nrpt=(double)NMAX/((double)nsym*(8.0*koef_11025));//qDebug()<<"2"<<nrpt;
    if (isrch!=0) nrpt=1;

    for (int irpt = 0; irpt < nrpt; irpt++)
    {
        for (int j = 0; j < nsym; j++)
        {
            if (sent[j]==1)
                f=f0 + 0.5*dfgen + foffset;
            else
                f=f0 - 0.5*dfgen + foffset;

            dphi=twopi*f*dt;

            for (i = 0; i < nsps; i++)
            {
                phi=phi+dphi;
                if (isrch==0)
                {
                    iwave[k]=(int)(8380000.0*sin(phi));//2.70 8380000.0 full=8388607
                }
                //else
                //cwave[k]=cmplx(cos(phi),sin(phi))
                //cwave[k]=cos(phi)+sin(phi)*I;
                k++;
            }
        }
    }

    if (isrch==0)
    {        
        for (i = 0; i < 5; ++i) iwave[k+i]=0; //iwave(k+1:)=0
    }

    //qDebug()<<k;
    return k;
}
///////////// JT6M /////////////////////////////////////
int GenMessage::gentone(double *x,int n,int k,double samfac)
{
    //real*4 x(512)
    double dt,f;
    int kk = 0;
    dt=1.0/(samfac*GEN_SAMPLE_RATE);
    f=(double)(n+51)*GEN_SAMPLE_RATE/(512.0*koef_srate);//111
    //qDebug()<<samfac;
    //do i=1,512
    for (int i = 0; i < 512*koef_srate; i++)
        x[i+k]=sin(twopi*i*dt*f);

    kk=k+512*koef_srate;

    return kk;
}
int GenMessage::gen6m(char *msg,double samfac)
{
    //! Encodes a message into a wavefile for transmitting JT6M signals.
    //const int NMAX=44544*koef_srate;  //hv v1.01 //!NMAX=28*512*3/2: hv v1.01 !NMAX=58*512*3/2: number of waveform samples
    double x[178180]; //178176             //!Data for wavefile
    int imsg[58+1];    //hv v1.01
    int nwave = 0;

    int nmsg=0;
    int i;
    for (i = 58; i >= 0; i--)
    {
        if (msg[i]!=' ')
            break;
    }
    nmsg=i+2;
    int odd = (int)fmod(nmsg,2);
    // na ne4eten broi ima dva intervala fakt
    if (odd==1)
    {
        nmsg=nmsg+1;       //!Make it even 4eten
    }
    //qDebug()<<"len 4eten"<<nmsg;
    //qDebug()<<"msg="<<msg<<nmsg;
    nwave=(double)nmsg*512*koef_srate*3/2;
    for (int m = 0; m < nmsg; m++)                            //!Get character code numbers
    {
        int ic=m;
        int n=(int)msg[ic];
        //! Calculate i in range 0-42:
        //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
        if (n>=(int)('0') && n<=(int)('9')) i=n-(int)('0');
        if (msg[ic]=='.') i=10;
        if (msg[ic]==',') i=11;
        if (msg[ic]==' ') i=12;
        if (msg[ic]=='/') i=13;
        if (msg[ic]=='#') i=14;
        if (msg[ic]=='?') i=15;
        if (msg[ic]=='$') i=16;
        if (n>=(int)('a') && n<=(int)('z')) i=n-(int)('a')+17;
        if (n>=(int)('A') && n<=(int)('Z')) i=n-(int)('A')+17;
        imsg[m]=i;
        //qDebug()<<"msg="<<msg[ic]<<"=end";
    }

    int k=0;
    //do i=1,nmsg,2
    for (i = 0; i < nmsg; i+=2)
    {
        k = gentone(x,-1,k,samfac);               //!Generate a sync tone
        k = gentone(x,imsg[i],k,samfac);          //!First character
        k = gentone(x,imsg[i+1],k,samfac);        //!Second character
    }

    //do i=1,nwave
    for (i = 0; i < nwave; i++)
        iwave[i]=(int)(8380000.0*x[i]);//2.70 8380000.0 full=8388607

    return nwave;
}
///////////// JT6M /////////////////////////////////////

int GenMessage::geniscat(char *msg,int nmsg,int mode4,double samfac)
{
    //! Generate an ISCAT waveform.
    int nwave = 0;
    int NMAX=30*GEN_SAMPLE_RATE;
    //int NSZ=1291;
    //char msgsent[29];                    //!Message to be transmitted
    int imsg[61];//hv v1.01 31 to 61
    //char c[43];
    double dt,f0,df,pha;
    int icos[4] = {0,1,3,2};                                //!Costas array
    /*int nsync = 3;
    int nlen = 2;
    int ndat = 18;*/

    //data nsync/4/,nlen/2/,ndat/18/
    //data c/'0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ /.?@-'/

    double nsps=(double)512.0*koef_srate/mode4;//111
    df=(double)GEN_SAMPLE_RATE/nsps;
    dt=1.0/(samfac*GEN_SAMPLE_RATE);
    f0=47.0*df;
    if (mode4==2) f0=13*df;
    int nsym=NMAX/nsps;

    int nblk=nsync_tx+nlen_tx+ndat_tx;
    int msglen=nmsg+1;
    int k=0;
    //int kk=0;
    imsg[0]=40; //!Always start with BOM char: '@'
    
    
    for (int i = 0; i < nmsg; i++)
    {                                //!Define the tone sequence
        //imsg(i+1)=36
        imsg[i+1]=36;                             //!Illegal char set to blank
        for (int j = 0; j< 42; j++)
        {
            if (msg[i]==c_ISCAT_TX[j])
                //imsg(i+1)=j-1;
                imsg[i+1]=j;
        }
    }
    for (int i = 0; i < nsym; i++)
    {//do i=1,nsym                                 //!Total symbols in 30 s
        //j=mod(i-1,nblk)+1
        int j=fmod(i,nblk);//pri nsync =3 -> nblk+1 na dolu gi pazi >= i <=
        //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
        //if(j.le.nsync) then
        if (j+1<=nsync_tx)
            itone_s[i]=icos[j];                      //!Insert 4x4 Costas array
        //else if(j.gt.nsync .and. j.le.nsync+nlen) then
        else if (j+1>nsync_tx && j+1<=nsync_tx+nlen_tx)
        {
            itone_s[i]=msglen;                       //!Insert message-length indicator
            //if(j.ge.nsync+2) then
            if (j+1>=nsync_tx+2)
            {
                //n=msglen + 5*(j-nsync-1)
                int n=msglen + 5*(j+1-nsync_tx-1);// 0 mai e 1
                if (n>41)
                    n=n-42;
                itone_s[i]=n;
            }
        }
        else
        {
            //k=k+1;
            //kk=mod(k-1,msglen)+1
            int kk=fmod(k,msglen);  //qDebug()<<"iskat"<<kk<<i;
            itone_s[i]=imsg[kk];
            k++;
        }
    } 

    k=0;
    pha=0.0;
    for (int m = 0; m < nsym; m++)
    {                                   //!Generate iwave
        double f=f0 + itone_s[m]*df;
        double dpha=twopi*f*dt;
        for (int i = 0; i < nsps; i++)
        {
            //k=k+1
            pha=pha+dpha;
            iwave[k]=(int)(8380000.0*sin(pha));//2.70 8380000.0 full=8388607
            k++;
        }
    }
    nwave=k;
    //qDebug()<<"nwave="<<nwave;
    //qDebug()<<"itone2_iskat"<<nsym;
    return nwave;
}
QString GenMessage::format_msg(char *message_in, int cmsg)
{       
    QString out = "";
    int beg,end;
    for (beg = 0; beg < cmsg; ++beg)
    {
        if (message_in[beg]!=' ') break;
    }
    for (end = cmsg-1; end >= 0; --end)
    {
        if (message_in[end]!=' ') break;
    }
    for (int z = beg; z < end+1; ++z) out.append(message_in[z]); 
    return out;           
}
int GenMessage::hvmsgen(char*msg_org,int mod_ident,double tx_freq,int period_t,QString sfmta,QString otptk)//,int &ntxslot QString mygridloc,
{
    int nwave = 0;
    int k = 0;
    int nspd = 0;
    double df;
    double pha = 0.0;
    int nrpt;
    double freq = 0.0;
    int LTone;
    double dpha = 0.0;
    //tuk e corekciata ako ne ti e dobre tx sempliraneto fivehztx
    double samfacout = 1.0;//1.0024;//samfacout    fsample_out/11025.d0
    double fsample_out;// = 11025.0*samfacout;
    double dt;//=1.0/fsample_out;
    int ndits;
    int NMSGMAX = 36; //v1.01 28 to 58
    //ft8-fox msg  LZ2HVV RR73; SP2HWY <123456789asd> -11#    40*5=200  40*10=400
    const int max_len = 420;
    char msg[max_len+10];
    for (int z = 0; z < max_len; ++z) msg[z]=' ';                 
                      
    fsample_out=GEN_SAMPLE_RATE*samfacout;
    if (fabs(samfacout-1.0)>0.02) fsample_out=1.0;
    // fsample_out=fsample_out/4;
    
    int nmsg = strlen(msg_org); //qDebug()<<"nmsg="<<nmsg;
    if(nmsg > max_len) nmsg = max_len;
    for (int z = 0; z < nmsg; ++z) msg[z] = msg_org[z];

    //Find message length for old modes
    int x;
    nmsg=0;
    for (x = NMSGMAX; x >= 0; x--)
    {
        if (msg[x]!=' ') break;
    }
    nmsg=x+1;// hv +1 za da ima interval nakraia

    if (msg[0]=='@')
    {
        if (msg[1]=='A') freq=882.0;
        if (msg[1]=='B') freq=1323.0;
        if (msg[1]=='C') freq=1764.0;
        if (msg[1]=='D') freq=2205.0;
        //if (msg[1]=='T') freq=1000.0;
        if (freq==0.0)
        {
            QString s;
            for (int i = 1; i < nmsg; i++) s.append(msg[i]);
            if (s=="TUNE")
            {
                freq = 1000.0;
                goto tune;
            }
            if (s.toInt()>=100 && s.toInt()<=3500) freq = s.toInt();//3000                
        }
tune:
        nwave=1*fsample_out;//be6e nwave=60*fsample_out; 60sec
        dpha=twopi*freq/fsample_out;
        for (int i = 0; i < nwave; i++) iwave[i]=(int)(8380000.0*sin(i*dpha));//2.70 8380000.0 full=8388607
        goto end;
    }

    if (mod_ident==11)
    {//FT8
    	QString tstr = format_msg(msg,max_len);//2.65
        if (s_msf) nwave = TGenSFox->gensfox(tstr,iwave,GEN_SAMPLE_RATE,tx_freq,sfmta,otptk);
        else nwave = TGenFt8->genft8(tstr,iwave,GEN_SAMPLE_RATE,tx_freq,sfmta,otptk); //,i3b
        //printf("period_t=%d\n",period_t);	
        goto end;
    }
    else if (mod_ident==13)
    {//FT4
    	QString tstr = format_msg(msg,max_len);//2.65
        nwave = TGenFt4->genft4(tstr,iwave,GEN_SAMPLE_RATE,tx_freq);//,i3b 
        goto end;
    }   
    else if (mod_ident==18)
    {//FT2
    	QString tstr = format_msg(msg,max_len);//2.65
        nwave = TGenFt2->genft2(tstr,iwave,GEN_SAMPLE_RATE,tx_freq);//,i3b 
        goto end;
    } 
    else if (mod_ident==14 || mod_ident==15  || mod_ident==16 || mod_ident==17)
    {//Q65
    	int mq65 = 1;
    	if		(mod_ident==15) mq65 = 2;
    	else if (mod_ident==16) mq65 = 4;
    	else if (mod_ident==17) mq65 = 8;
    	QString tstr = format_msg(msg,max_len);//2.65
        nwave = TGenQ65->genq65(tstr,iwave,GEN_SAMPLE_RATE,tx_freq,mq65,period_t);//,i3b 
        goto end;
    }                
    else if (mod_ident==4)
    {//ISCAT-A
        int mode4 = 1;
        nwave = geniscat(msg,nmsg,mode4,samfacout);
        goto end;
    }
    else if (mod_ident==5)
    {//ISCAT-B
        int mode4 = 2;
        nwave = geniscat(msg,nmsg,mode4,samfacout);
        goto end;
    }
    else if (mod_ident==0)
    {//MSK144
        int i4tone[234];// v procedurata se nulira TGenMsk->genmsk
        nwave = TGenMsk->genmsk(msg,samfacout,i4tone,true,iwave,GEN_SAMPLE_RATE,koef_srate,0,mod_ident,false);
        goto end;
    }
    else if (mod_ident==12)
    {//MSK144ms
        int i4tone[234];// v procedurata se nulira TGenMsk->genmsk
        nwave = TGenMsk->genmsk(msg,samfacout,i4tone,true,iwave,GEN_SAMPLE_RATE,koef_srate,0,mod_ident,true);
        goto end;
    }
    else if (mod_ident==1)
    {//JTMS
        nwave = genms(msg,samfacout,0);
        goto end;
    }
    else if (mod_ident==6)
    {//JT6M
        nwave = gen6m(msg,samfacout);        
        goto end;
    }
    else if (mod_ident==7)
    {//JT65A
	    //int mode65 = 1; //A=1, B=2, C=4  
	    //int nfast=1;    //A=1, B=1, C=1, B2=2, C2=2
	    //if((mode(5:5)=='B' || mode(5:5)=='C') && mode(6:6)=='2') nfast=2
        //if(mode65==2 || mode65==4) nfast=2;
        //double ntxdf = 0.0;
        nwave = TGen65->gen65(msg,1,1,samfacout,GEN_SAMPLE_RATE,iwave);//ntxdf,koef_srate,   
        goto end;
    }
    else if (mod_ident==8)
    {//JT65B
    	//double ntxdf = 0.0;
        nwave = TGen65->gen65(msg,2,1,samfacout,GEN_SAMPLE_RATE,iwave);//ntxdf,koef_srate,   
        goto end;
    }
    else if (mod_ident==9)
    {//JT65C
    	//double ntxdf = 0.0;
        nwave = TGen65->gen65(msg,4,1,samfacout,GEN_SAMPLE_RATE,iwave);//ntxdf,koef_srate,  
        goto end;
    }
	//else if (mod_ident==2 || mod_ident==3) 
	//{
    //FSK
    dt=1.0/fsample_out;
    if (nmsg<=NMSGMAX) nmsg=nmsg+1; //Add trailing blank if nmsg < 28
    ndits = abc441(msg,nmsg);
    nspd = NSPD_FOM_MODE(mod_ident)*koef_srate;
    LTone = LTONE_FOM_MODE(mod_ident);
    k=0;
    df=(double)GEN_SAMPLE_RATE/nspd;
    pha = 0.0;
    nrpt=30.0*GEN_SAMPLE_RATE/(double)(nspd*ndits);
    for (int irpt = 0; irpt < nrpt; irpt++)
    {
        for (int m = 0; m < ndits; m++)
        {
            freq=(LTone-1+itone_s[m])*df;//-100.0
            dpha=twopi*freq*dt;
            for (int i = 0; i < nspd; i++)
            {
                pha=pha+dpha;
                //if (pha > twopi) pha -= twopi;
                iwave[k]=(int)(8380000.0*sin(pha));//2.70 8380000.0 full=8388607                                                
                k++;
            }
        }
    }//qDebug()<<"itone3_441"<<ndits;
    nwave=k;
    //}
end:
    return nwave;
}
