/* The algorithms, source code, look-and-feel of WSJT-X and related
 * programs, and protocol specifications for the modes FSK441, FT8, JT4,
 * JT6M, JT9, JT65, JTMS, QRA64, ISCAT, MSK144, are Copyright © 2001-2017
 * by one or more of the following authors: Joseph Taylor, K1JT; Bill
 * Somerville, G4WJS; Steven Franke, K9AN; Nico Palermo, IV3NWV; Greg Beam,
 * KI7MT; Michael Black, W9MDB; Edson Pereira, PY2SDR; Philip Karn, KA9Q;
 * and other members of the WSJT Development Group.
 *
 * MSHV FSK Decoder 
 * Rewritten into C++ and modified by Hrisimir Hristov, LZ2HV 2015-2017
 * May be used under the terms of the GNU General Public License (GPL)
 */

#include "decoderms.h"

#define _FSK441_DH_
#include "../config_msg_all.h"
//#include <QtGui>

double DecoderMs::max_3double(double a,double b,double c)
{
    double res = a;
    if (b>res)
        res=b;
    if (c>res)
        res=c;
    return res;
}
int DecoderMs::min_3int(int a,int b,int c)
{
    int res = a;
    if (b<res)
        res=b;
    if (c<res)
        res=c;
    return res;
}
double DecoderMs::max_4double(double a,double b,double c,double d)
{
    double res = a;
    if (b>res)
        res=b;
    if (c>res)
        res=c;
    if (d>res)
        res=d;
    return res;
}

void DecoderMs::detect(double*data,int dat_count_begin,int npts,double f,double*y,int mode)
{
    //! Compute powers at the tone frequencies using 1-sample steps.
    //int NZ=(int)DEC_SAMPLE_RATE;
    double complex c[12000];//max=[NZ];
    double complex csum;

    double dpha=twopi*f/DEC_SAMPLE_RATE;

    int NSPD = NSPD_FOM_MODE(mode);

    for (int i = 0; i<npts; i++)
    {
        //do i=1,npts
        //c[i]=data[i]*cmplx(cos(dpha*i),-sin(dpha*i))
        c[i]=data[i+dat_count_begin]*(cos(dpha*(double)i)-sin(dpha*(double)i)*I);
    }

    csum=0.0+0.0*I;
    for (int i = 0; i<NSPD; i++)
        csum=csum+c[i];

    //for (int i = 0; i<11025; i++)
	//y[i]=0.0;
    //y(1)=real(csum)**2 + aimag(csum)**2
    y[0]=pomAll.ps_hv(csum);//qDebug()<<"y0"<<y[0];

    //do i=2,npts-(NSPD-1)
    for (int i = 1; i<npts-(NSPD-1); i++)
    {   //csum=csum-c(i-1)+c(i+NSPD-1)
        csum=csum-c[i-1]+c[i+NSPD-1];
        //y(i)=real(csum)**2 + aimag(csum)**2
        y[i]=pomAll.ps_hv(csum);
        //qDebug()<<i<<y[i];
    }
}
int DecoderMs::sync(double*y1,double*y2,double*y3,double*y4,int npts,/*double baud,double &bauderr,*/int mode)
{
    //! Input data are in the y# arrays: detected sigs in four tone-channels,
    //! before decimation by NSPD.
    int jpk = 0;
    //int NSPD=25;

    int NSPD = NSPD_FOM_MODE(mode);// to 105

    double zf[110];
    double tmp1;
    double tmp2;
    double complex csum;
    //int nsum[NSPD+2];
    double z[65538+10];                            //!Ready for FSK110
    //int beg = 0;

//c_agn:
  

    for (int i = 0; i<NSPD; i++)
    {
        zf[i]=0.0;
        //nsum[i]=0;
    }
    for (int i = 0; i<npts; i++)
    {//do i=1,npts
        double a1_a=max_4double(y1[i],y2[i],y3[i],y4[i]);       //!Find the largest one
        double a2_a;

        if (a1_a==y1[i])                 //!Now find 2nd largest
            a2_a=max_3double(y2[i],y3[i],y4[i]);
        else if (a1_a==y2[i])
            a2_a=max_3double(y1[i],y3[i],y4[i]);
        else if (a1_a==y3[i])
            a2_a=max_3double(y1[i],y2[i],y4[i]);
        else
            a2_a=max_3double(y1[i],y2[i],y3[i]);

        z[i]=1.e-6*(a1_a-a2_a);                     //!Subtract 2nd from 1st
        //j=fmod(i-1,NSPD)+1;
        int j=fmod(i,NSPD);
        zf[j]+=z[i];
        //nsum[j]=nsum[j]+1;
        //qDebug()<<"jjjjnpts="<<j;
    }
    
           /* QString sss = "";///gen_osd174_[174][87];
            for (int z= 0; z < NSPD; z++)//decoded=87   cw-174
            {
                sss.append(QString("%1").arg((int)zf[z]));
                sss.append(",");
            }
            qDebug()<<"ZF="<<sss;*/

    /*
    double complex cz[32768+10];
    int n=log(double(npts))/log(2.0);
    //double nfft=2**(n+1)
    int nfft=pow(2,(n+1));
    //call zero(z(npts+1),nfft-npts)??? npts+1
    //zero_double_beg_count(z,npts+0,nfft-npts);// towan ne prawi ni6to
    
    zero_double_beg_end(z,npts,nfft);// v 1.17
    //qDebug()<<npts+0<<nfft-npts;
    xfft(cz,z,nfft); 
    //! Now find the apparent baud rate.
    double df=(double)DEC_SAMPLE_RATE/(double)nfft;
    double zmax=0.0;
    int ia=391.0/df;                                //!Was 341/df   391.0/df;
    int ib=491.0/df;                               //!Was 541/df    491.0/df;
    //double baud=0.0;
    for (int i = ia; i < ib; i++)
    {
        //z(i)=real(cz(i))**2 + aimag(cz(i))**2
        z[i]=ps_hv(cz[i]);
        //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
        if (z[i]>zmax)
        {
            zmax=z[i];
            //baud=df*(double)i;
        }
    }*/
    //bauderr=(baud-11025.0/nspd)/df   //!Baud rate error, in bins
    //double bauderr=(baud-DEC_SAMPLE_RATE/(double)NSPD)/df;// !Baud rate error, in bins  
    //double bauderr=baud-441.0;  
    //qDebug()<<"bauderr="<<baud; 
    //if(bauderr>2.0)
    	//goto c222;
    
    //! Find phase of signal at 441 Hz.
    csum=0.0+0.0*I; //qDebug()<<ia<<NSPD;
    for (int j = 0; j<NSPD; j++)
    {//do j=1,NSPD
        double pha=(double)(j)*(double)twopi/(double)NSPD; //pha=j*twopi/nspd
        //csum=csum+zf[j]*cmplx(cos(pha),-sin(pha))
        csum=csum+zf[j]*(cos(pha)-sin(pha)*I);
        //qDebug()<<cimag(csum)<<creal(csum);
    }

    tmp1=cimag(csum);
    tmp2=creal(csum);
    double pha=-atan2(tmp1,tmp2);
    jpk=(int)((double)NSPD*pha/twopi);
    //jpk=jpk-1;//v1.43 fsk441 change -2 ??? triabva da e -1
    //qDebug()<<"jpk="<<jpk;


    //!The following is nearly equivalent to the above.  I don't know which
    //!(if either) is better.
    //!     zfmax=-1.e30
    //!     do j=1,NSPD
    //!        if(zf(j).gt.zfmax) then
    //!           zfmax=zf(j)
    //!           jpk2=j
    //!        endif
    //!     enddo
    
    /*double zfmax=-1.e30;
    jpk = 0;
    for (int j = 0; j<NSPD; j++)
    {//do j=1,NSPD
        if (zf[j]>zfmax)
        {
            zfmax=zf[j];
            jpk=j;
        }
    }*/
    
    
    /*sss = "";
    for (int z= 0; z < NSPD; z++)
    {
        sss.append(QString("%1").arg((int)zf[z]));
        sss.append(",");
    }
    qDebug()<<"zf2="<<sss;*/
    
    

    //int s_jpk = (jpk2+jpk)/2;
    //qDebug()<<"jpk2==="<<jpk2;
    //qDebug()<<"s_jpk==="<<s_jpk;*/
    
    //if(jpk.lt.1)
    if (jpk<0)
    {
    	//beg+=2100;//NSPD;
    	//qDebug()<<"jpk2="<<jpk<<beg;
    	//goto c_agn;
        jpk=jpk+NSPD-1; 
    }
    //qDebug()<<"jpk final===="<<jpk<<npts;

    return jpk;
}
double DecoderMs::spec441(double*dat,int raw_in_c_begin,int jz,double*s)
{
    //! Computes average spectrum over a range of dat, e.g. for a ping.
    //! Returns spectral array and frequency of peak value.
    double f0 = 0.0;
    const int NFFT=256;//256-1 HV v1.09 df441-315
    const int NR=NFFT+2;
    const int NH=NFFT/2;
    //real*4 x(NR),s(NH)
    double x[NR+10];
    //zero_double_beg_count(x,0,NR);

    double complex c[NH*2+10];
    //zero_double_comp_beg_count(c,0,NH*2);
    //complex double *c = new complex double[NH*2];
    ///complex c(0:NH)
    //equivalence (x,c)

    //zero_double(s,NH);
    pomAll.zero_double_beg_end(s,0,NH);

    int nz=(double)jz/NFFT;
    for (int n = 0; n<nz; n++)
    {//do n=1,nz
        //j=1 + (n-1)*NFFT
        int j=0+((n)*NFFT)+raw_in_c_begin;
        //qDebug()<<jz<<j<<NFFT;
        //call move(dat(j),x,NFFT)
        move_da_to_da(dat,j,x,0,NFFT);

        //xfft(x,NFFT);
        xfft(c,x,NFFT);//(NFFT-1) HV v1.09 df441-315
        //four2a_double_to_complex(c,x,NFFT,1,-1,0);

        for (int i = 0; i<NH; i++)
        {//do i=1,NH
            //s(i)=s(i)+real(c(i))**2 + aimag(c(i))**2
            s[i]+=pomAll.ps_hv(c[i]);
        }
    }

    double smax=0.0;
    double df=(double)DEC_SAMPLE_RATE/(double)(NFFT);
    double fac=1.0/(100.0*(double)NFFT*nz);

    for (int i = 0; i<NH; i++)
    {//do i=1,nh
        s[i]=fac*s[i];
        //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
        if (s[i]>smax)
        {
            smax=s[i];
            f0=(double)i*df;
        }
    }
    //qDebug()<<"spec441"<<f0;// center frq
    return f0;
}
/*double DecoderMs::peakup(double ym,double y0,double yp)
{
    double dx = 0.0;
    double b=(yp-ym)/2.0;
    double c=(yp+ym-(2*y0))/2.0;
    dx=-b/(2.0*c);

    return dx;
}*/
QString DecoderMs::longx(double*dat,int dat_count_begin,int npts0,double*ps,int DFTol,int &msglen,double /*bauderr*/,int mode)
{
    //! Look for 441-baud modulation, synchronize to it, and decode message.
    //! Longest allowed data analysis is 1 second.
    //QString str;
    QString msg;
    int NMAX=(int)DEC_SAMPLE_RATE;
    int NDMAX=NMAX/25;
    double psmo[20+5]={0.0};
    double y1[11026];
    double y2[11026];
    double y3[11026];
    double y4[11026];

    double wgt_plus[5];
    double *wgt = &wgt_plus[2];
    int dit[441+5];//NDMAX = 441
    //integer n4(0:2)
    int n4[3];

    //double dx = 0.0;
    int jpk = 0;
    //double baud = 0.0;
    int jsync = 0;
    //common/acom/a1,a2,a3,a4

    wgt[-2] = 1.0;
    wgt[-1] = 4.0;
    wgt[0] = 6.0;
    wgt[1] = 4.0;
    wgt[2] = 1.0;

    //data c/' 123456789.,?/# $ABCD FGHIJKLMNOPQRSTUVWXY 0EZ*!'/
    //data wgt/1.0,4.0,6.0,4.0,1.0/


    int kpk=0;                                //!Silence compiler warning
    for (int i = 0; i<NDMAX; i++)
        dit[i]=0;

    int NSPD = NSPD_FOM_MODE(mode);
    int LTone = LTONE_FOM_MODE(mode);

    int NBaud=DEC_SAMPLE_RATE/(double)NSPD;
    int npts=fmin(NMAX,npts0);//qDebug()<<npts<<NMAX<<npts0;
    
    double df=(double)DEC_SAMPLE_RATE/256.0;
    double smax=0.0;

    //! Find the frequency offset of this ping.
    //! NB: this might be improved by including a bandpass correction to ps.
    //ntol=(10,25,50,100,200,400,600)              #List of available tolerances
    //DFTolerance=200;
    int ia=(int)((double)(((double)LTone*NBaud)-(double)DFTol)/df);
    int ib=(int)((double)(((double)LTone*NBaud)+(double)DFTol)/df);
    //qDebug()<<DFTol;

    for (int i = ia; i<=ib; i++)
    {                                       //!Search for correct DF
        double sum=0.0;
        //do j=1,4
        for (int j = 0; j<4; j++)
        {                                           //!Sum over the 4 tones
            //m=(int)((i*df+(j-1)*NBaud)/df);
            int m=(int)((double)(((double)(i)*df)+(double)((double)(j)*NBaud))/df);
            //int m=(int)((i*df+(j)*NBaud)/df);
            //QString ttt;
            //do k=-2,2
            for (int k = -2; k<=2; k++)//!Weighted averages over 5 bins
            {
                sum+=wgt[k]*ps[m+k];
            }
        }
        //k=i-ia+1
        int k=i-ia+0;
        psmo[k]=sum;//qDebug()<<k<<sum;
        //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
        //qDebug()<<i*df<<LTone*NBaud<<sum;
        if (sum>smax)
        {
            smax=sum;
            //noffset=nint(i*df-LTone*NBaud)
            //qDebug()<<"00000000="<<i*df<<i;
            noffset_fsk441_dfx=(((double)i*df)-(LTone*NBaud));
            kpk=k;
        }            //qDebug()<<i<<df<<i*df;
    }
    //qDebug()<<"noffset_fsk441_dfx1="<<noffset_fsk441_dfx;
    //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    //if(kpk.gt.1 .and. kpk.lt.20) then
    if (kpk>0 && kpk<19)//ok zaradi psmo[20];
    {
        double dx = pomAll.peakup(psmo[kpk-1],psmo[kpk],psmo[kpk+1]);//call peakup(psmo(kpk-1),psmo(kpk),psmo(kpk+1),dx)
        //qDebug()<<noffset_fsk441_dfx<<dx<<df<<dx*df;
        noffset_fsk441_dfx=(int)((double)noffset_fsk441_dfx+(double)dx*df);
    }
    //qDebug()<<"noffset_fsk441_dfx2="<<noffset_fsk441_dfx;
    //noffset_fsk441_dfx = 14;
    //! Do square-law detection in each of four filters.
    /*int c_pp=-1;
    c666:
    noffset_fsk441_dfx++; 
    c_pp++;*/   
    
    double f1=(double)LTone*NBaud+(double)noffset_fsk441_dfx;
    double f2=(double)(LTone+1)*NBaud+(double)noffset_fsk441_dfx;
    double f3=(double)(LTone+2)*NBaud+(double)noffset_fsk441_dfx;
    double f4=(double)(LTone+3)*NBaud+(double)noffset_fsk441_dfx;

    /*qDebug()<<"npts1"<<f1<<f2<<f3<<f4;
    f1=868;
    f2=1309;
    f3=1750;
    f4=2191;*/
    /*int ttt = npts/NSPD;
    npts = ttt*NSPD-NSPD;
    qDebug()<<"npts2"<<npts;*/   
    
    detect(dat,dat_count_begin,npts,f1,y1,mode);
    detect(dat,dat_count_begin,npts,f2,y2,mode);
    detect(dat,dat_count_begin,npts,f3,y3,mode);
    detect(dat,dat_count_begin,npts,f4,y4,mode);

    //! Bandpass correction:
    //npts=npts-(NSPD-1)
    npts=npts-(NSPD - 0);//???? hv
    //qDebug()<<a1_<<a2_<<a3_<<a4_;
    for (int i = 0; i<npts; i++)
    {
        y1[i]=y1[i]*a1_;
        y2[i]=y2[i]*a2_;
        y3[i]=y3[i]*a3_;
        y4[i]=y4[i]*a4_;
        //qDebug()<<i;
    }      
    /*QString sss = "";///gen_osd174_[174][87];
            for (int z= 0; z < 11025; z++)//decoded=87   cw-174 
            {
                sss.append(QString("%1").arg((int)y1[z]));
                sss.append(",");
            }
            qDebug()<<"222 mi="<<sss;*/
    jpk = sync(y1,y2,y3,y4,npts,/*baud,bauderr,*/mode);
    //jpk=jpk-1;
    //qDebug()<<jpk;
    //qDebug()<<bauderr<<noffset_fsk441_dfx;
    //if(bauderr>1.0 && c_pp<10)
    	//goto c666;
    
    //! Decimate y arrays by NSPD
    //ndits=npts/NSPD - 1

    /*int c_pp=-1;
    jpk = jpk-1;
    c77:    
    c_pp++;   
    jpk = jpk+1;
    qDebug()<<"jpk="<<jpk;
    msg ="";*/
    //ndits=npts/NSPD - 1
    int ndits=npts/NSPD;//??? hv
    for (int i = 0; i<ndits; i++)
    {//do i=1,ndits
        /*y1[i]=y1[jpk+(i-1)*NSPD];
        y2[i]=y2[jpk+(i-1)*NSPD];
        y3[i]=y3[jpk+(i-1)*NSPD];
        y4[i]=y4[jpk+(i-1)*NSPD];*/
        y1[i]=y1[jpk+(i)*NSPD];
        y2[i]=y2[jpk+(i)*NSPD];
        y3[i]=y3[jpk+(i)*NSPD];
        y4[i]=y4[jpk+(i)*NSPD];
    }

    //! Now find the mod3 phase that has no tone 3's
    n4[0]=0;
    n4[1]=0;
    n4[2]=0;
    for (int i = 0; i<ndits; i++)
    {//do i=1,ndits
        double ymax=max_4double(y1[i],y2[i],y3[i],y4[i]);
        if (y1[i]==ymax) dit[i]=0;
        if (y2[i]==ymax) dit[i]=1;
        if (y3[i]==ymax) dit[i]=2;
        if (y4[i]==ymax)
        {
            dit[i]=3;
            int k=fmod(i,3);//qDebug()<<k<<i;
            //n4(k)=n4(k)+1
            n4[k]=n4[k]+1;
            //qDebug()<<n4[k]<<k;
        }
    }

    int n4min=min_3int(n4[0],n4[1],n4[2]);
    if (n4min==n4[0]) jsync=3;
    if (n4min==n4[1]) jsync=1;
    if (n4min==n4[2]) jsync=2;
    //qDebug()<<"n4="<<n4[0]<<n4[1]<<n4[2]<<"jsync="<<jsync;
    //! Might want to notify if n4min>0 or if one of the others is equal
    //! to n4min.  In both cases, could then decode 2 or 3 times, using
    //! other starting phases.

    //! Finally, decode the message.

    msglen=ndits/3.0;//qDebug()<<msglen;
    msglen=fmin(msglen,46);//v1.30 46 max char for fsk to list
    //qDebug()<<msglen;
    for (int i = 0; i<msglen; i++)
    {//do i=1,msglen
        //int j=(i-1)*3+jsync;
        int j=(i)*3+jsync;
        int nc=16*dit[j] + 4*dit[j+1] + dit[j+2];
        //qDebug()<<"j="<<j;
        //str[i]=' ';
        //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
        msg.append(" ");//1.77  
        if (nc<=47 && nc>=0)//1.77 msg(i:i)=c(nc+1:nc+1)            
        {
            msg[i]=c_FSK441_RX[nc];//1.77 
            //msg.append(c_FSK441_RX[nc]);
            //qDebug()<<i<<c_FSK441_RX[nc]<<nc<<msg[i];
        }
    }
    //qDebug()<<"fffffff"<<msg;
    //return str;

    msg = RemWSpacesInside(msg);
    msg = RemBegEndWSpaces(msg);
    //msg = msg.mid(4,msg.count()-6);

    //qDebug()<<"fffffff======================================="<<msg;

    //if(c_pp < 10)// && !msg.contains(" ")
     //goto c666;

    return msg;
}
int DecoderMs::abc441(char*msg,int count_msg,short *itone)
{
    int count;// = 3*count_msg;
    // samo golemi bukwi HV


    for (int i = 0; i < count_msg; i++)
    {
        int j;
        j = (int)msg[i];
        //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
        if (j<0 || j>91)//!Replace illegal char with blank
            j=32;
        int n;
        n = lookup_FSK441_TXRX[j];
        //qDebug()<<"NNN="<<n<<j<<msg[i];
        itone[3*(i+1)-3]=n/16 + 1;
        itone[3*(i+1)-2]=fmod(n/4,4) + 1;
        itone[3*(i+1)-1]=fmod(n,4) + 1;
        //qDebug()<<j<<n;
    }
    count=3*count_msg;
    return count;
}
void DecoderMs::gen441(short *itone,int ndits,double complex *cfrag, int mode)
{
    int nspd = NSPD_FOM_MODE(mode);
    int LTone = LTONE_FOM_MODE(mode);

    double dt=1.0/DEC_SAMPLE_RATE;
    int k=0;
    //int NSPD=25;
    double df=(double)DEC_SAMPLE_RATE/nspd;
    double pha=0.0;
    for (int m = 0; m < ndits; m++)
    {//do m=1,ndits
        double freq=(LTone-1+itone[m])*df;
        double dpha=twopi*freq*dt;
        for (int i = 0; i < nspd; i++)
        {//do i=1,NSPD
            pha=pha+dpha;
            //cfrag(k)=cmplx(sin(pha),-cos(pha));
            cfrag[k]=(sin(pha)+(-cos(pha))*I);
            //cfrag[k]=(sin(pha)*100.0+(-cos(pha)*100.0)*I);
            k++;
        }
    }
}
void DecoderMs::smo(double*x,int x_begin,int npts,double*y,double nadd)
{
    //smo(ccf,-ia,2*ia+1,work,nadd);
    //int NMAX=512*1024;
    int nh=nadd/2;
    for (int i = 1+nh; i < npts-nh; i++)
    {//do i=1+nh,npts-nh
        double sum=0.0;
        for (int j = -nh; j < nh; j++)
        {//do j=-nh,nh
            sum=sum + x[i+(j+x_begin)];
        }
        y[i]=sum;
        //qDebug()<<i;
    }
    //y(:nh)=0.
    pomAll.zero_double_beg_end(y,0,nh);
    //y(npts-nh+1:)=0.
    //qDebug()<<npts-nh+1<<(npts-(npts-nh+1));
    //zero_double_beg_count(y,npts-nh+1,(npts - (npts-nh+1)));
    pomAll.zero_double_beg_end(y,npts-nh+0,npts);// v1.17

    double fac=1.0/nadd;
    for (int i = 0; i < npts; i++)
        //do i=1,npts
        x[i+x_begin]=fac*y[i];
    //enddo
}
int DecoderMs::chk441(double *dat,int jz,double tstart,double width,int nfreeze,int mousedf,
                      int dftolerance,bool pick,int mode,double &dfx_real_hv)
{
    //! Experimental FSK441 decoder
    int nok = 0;
    int NMAX=512*1024;
    //int MAXFFT=8192;
    //qDebug()<<"AAAAA";
    double complex *cdat = new double complex[NMAX+10];                     //!Analytic form of signal

    //double complex *cfrag = new double complex[2100];                     //!Complex waveform of message fragment
    double complex z;
    //!Generated tones for msg fragment
    double *s = new double[NMAX+10];

    //double ccf(-6000:6000)
    double ccf_p[12000];
    double *ccf = &ccf_p[6000];
    int nspd = 0;

    //double dfx = 0.0;
    //common/scratch/work(NMAX)
    //save

    nspd = NSPD_FOM_MODE(mode);

    if (only1_s_mode!=mode)
    {
        //short itone_all[84];
        gen441(itone_s_fsk,ndits_s,cfrag_s,mode);
        only1_s_mode=mode;
        //qDebug()<<"gen441"<<s_mod;
    }

    double ccf0=3.0;
    double sb0=0.75;
    if (pick)
    {
        ccf0=2.1;
        sb0=0.60;        
        //ccf0=2.5;//1.77 my be
        //sb0=0.73;//1.77 my be
    }
    
    //int nsps=25;                                  //!Initialize variables
    int nsam=nspd*ndits_s;
    double dt=1.0/DEC_SAMPLE_RATE;
    //int i0=(tstart-0.02)/dt;
    int i0=(int)((double)(tstart-0.02)/dt);//start -20ms
    //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    //if(i0.lt.1) i0=1
    if (i0<0) i0=0;
    //int npts=int((width+0.02)/dt)+1;//HV +0.02 ne decodira pod -150 DF v0.98
    int npts=(int)(((double)(width+0.06)/dt)+1.0);//HV v0.98 0.06 from 0.02 ina4e ne decodira pod -150 DF
    npts=fmin(npts,jz+1-i0);
    npts=fmin(npts,22050);                     //!Max ping length 2 s

    double xn=log(double(npts))/log(2.0); //qDebug()<<"XN="<<xn;     //+1.0  //      //npts =1988;
    int n=(int)xn;
    if (xn-n >0.001) n++;    //qDebug()<<"NNNN="<<n; //n=12;
    int nfft1=pow(2,n);      //qDebug()<<"nfft11111="<<nfft1;

    //no in wsjt v1.34 pod 4096 ne raboti pravilno
    //qDebug()<<"nfft11111="<<nfft1;
    nfft1=fmax(nfft1,4096);  //hv 2**12=4096
    //qDebug()<<"nfft12222="<<nfft1;

    double df1=(double)DEC_SAMPLE_RATE/(double)nfft1; //qDebug()<<"df1="<<df1; //df1=2.6;
    double sbest=0.0;
    double base = 0.0;
    double *work = new double[NMAX];
    //qDebug()<<npts; //nfft1=4908;

    analytic(dat,i0,npts,nfft1,s,cdat);    //!Convert to analytic signal
    //analytic_msk(dat,i0,npts,nfft1,cdat);

    //!  call len441(cdat,npts,lenacf,nacf)          //!Do ACF to find message length

    int ia=int((double)dftolerance/df1);
    i0=0;
    if (nfreeze!=0) i0=int(mousedf/df1);
    double ccfmax=0.0;
    for (int i = -ia; i < ia; i++)
    {
        //do i=-ia,ia                                 //!Find DF
        //fsk441 882, 1323, 1764, and 2205 Hz
        //fsk315 945, 1260, 1575 and 1890 Hz
        if (mode==2)//fsk441 <-      fsk315
            ccf[i]=s[i0+i+int(882.0/df1)] + s[i0+i+int(1323.0/df1)] + s[i0+i+int(1764.0/df1)] + s[i0+i+int(2205.0/df1)];
        else
            ccf[i]=s[i0+i+int(945.0/df1)] + s[i0+i+int(1260.0/df1)] + s[i0+i+int(1575.0/df1)] + s[i0+i+int(1890.0/df1)];
    }
    //enddo
    //ccf(:-ia-1)=0.
    //qDebug()<<-6000<<(6000+(-ia-1));
    //zero_double_beg_count(ccf,-6000,(6000+(-ia-1)));
    pomAll.zero_double_beg_end(ccf,-6000,(-ia-1));// v1.17

    //ccf(ia+1:)=0.
    //qDebug()<<ia+1<<6000-(ia+1);
    //zero_double_beg_count(ccf,ia+1,6000-(ia+1));
    pomAll.zero_double_beg_end(ccf,ia+1,6000);//v1.17
    //for (int i = -ia-1; i < ia+1+1; i++)
    //ccf[i]=0.0;

    double nadd=2.0*(int)(5.0/df1)+1.0;   //qDebug()<<"nadd="<<nadd; nadd=2;
    //double nadd=2.0*(5.0/df1)+1.0;
    //smo(ccf(-ia),2*ia+1,work,nadd)         //!Smooth CCF by nadd
    smo(ccf,-ia,2*ia+1,work,nadd);

    //double dfx = 0.0;
    int ipk = 0;
    //qDebug()<<"DD"<<-ia<<ia<<width;
    for (int i = -ia; i < ia; i++)
    {//do i=-ia,ia
        //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.                               //!Find max of smoothed CCF
        if (ccf[i]>ccfmax)
        {
            ccfmax=ccf[i];
            ipk=i0+i;
            dfx_real_hv=(double)ipk*df1;
        }
    }

    int ic=fmin((220.0/df1),ia);                    //!Baseline range +/- 220 Hz
    //pctile(ccf(ipk-ic),work,2*ic+1,50,base)
    base = pctile(ccf,ipk-ic,work,2*ic+1,50);
    if(base==0.0)//no divide by zero
    	base=1.0;
    ccfmax=ccfmax/base;
    if (ccfmax<ccf0) goto c800;               //!Reject non-FSK441 signals
    //qDebug()<<"dfx"<<dfx;
    //! We seem to have an FSK441 ping, and we know DF; now find DT.
    tweak1(cdat,npts,-dfx_real_hv,cdat);            //!Mix to standard frequency
    //qDebug()<<dfx;
    //! Look for best match to "frag", find its DT
    for (int i = 0; i < npts-nsam; i++)
    {//do i=1,npts-nsam
        z=0.0;
        double a=0.0;
        for (int j = 0; j < nsam; j++)
        {//do j=1,nsam
            a += cabs(cdat[j+i-0]);
            z += cdat[j+i-0]*conj(cfrag_s[j]);//*10
        }
        if(a==0.0)//no didvide by zero
           a=1.0;
        double ss=cabs(z)/a;
        if (ss>sbest)
        {
            sbest=ss;
            ipk=i;
        }
    }
    //qDebug()<<"sbest"<<sbest<<sb0;

    if (sbest<sb0)
        goto c800;            //!Skip if not decodable FSK441 data
    nok=1;

c800: //continue;
    //qDebug()<<"nok"<<nok<<dfx;
    delete [] cdat;
    delete [] s;
    delete [] work;
    return nok;
}
