/* The algorithms, source code, look-and-feel of WSJT-X and related
 * programs, and protocol specifications for the modes FSK441, FT8, JT4,
 * JT6M, JT9, JT65, JTMS, QRA64, ISCAT, MSK144, are Copyright © 2001-2017
 * by one or more of the following authors: Joseph Taylor, K1JT; Bill
 * Somerville, G4WJS; Steven Franke, K9AN; Nico Palermo, IV3NWV; Greg Beam,
 * KI7MT; Michael Black, W9MDB; Edson Pereira, PY2SDR; Philip Karn, KA9Q;
 * and other members of the WSJT Development Group.
 *
 * MSHV ISCAT Decoder 
 * Rewritten into C++ and modified by Hrisimir Hristov, LZ2HV 2015-2017
 * May be used under the terms of the GNU General Public License (GPL)
 */

#include "decoderms.h"

#define _ISCAT_DH_
#include "../config_msg_all.h"
//#include <QtGui>
 
int DecoderMs::ana932(double *dat,int npts0,double complex *cdat)
{
    int npts = 0;
    //double complex cfft[147456];
    //common/down932/cfft(147456),npts2,df932

    //n=log(float(npts0))/log(2.0)
    if(npts0==0) npts0=1;
    int n=log(double(npts0))/log(2.0);
    //nfft1=2**(n+1)
    int nfft1=pow(2,(n+1));
    int nfft2=(double)9.0*nfft1/32.0;//qDebug()<<nfft1<<nfft2;
    double fac=2.0/nfft1;

    for (int i = 0; i<npts0/2; i++)
        //cdat[i]=fac*cmplx(dat(2*i-1),dat(2*i))
        cdat[i]=fac*(dat[2*i] + dat[2*i+1]*I);

    //for (int i = 0; i<nfft1/2; i++)
    //cdat(npts0/2+1:nfft1/2)=0.0;
    //cdat[i+npts0/2]=0.0+0.0*I;
    for (int i = npts0/2; i<nfft1/2; i++)// 1.32
        //cdat(npts0/2+1:nfft1/2)=0.0;
        cdat[i]=0.0+0.0*I;

    //qDebug()<<npts0/2<<nfft1/2;
    //call four2a(cdat,nfft1,1,-1,0)
    f2a.four2a_d2c(cdat,dat,nfft1,-1,0);//!Forward r2c FFT
    //call four2a(cdat,nfft2,1,1,1)
    f2a.four2a_c2c(cdat,nfft2,1,1);                //!Inverse c2c FFT
    npts=(double)npts0*9.0/32.0;                         //!Downsampled data length
    //int npts2=npts;
    return npts;
}

void DecoderMs::synciscat(double complex*cdat,int npts,double s0_[5601][289],int &jsym,int DFTolerance,
                          int mode4,double &xsync,double &sig,int &ndf0,int &msglen,int &ipk,int &jpk,
                          int &idf)
/*no used 1.33 int mousebutton,int nafc,double &df1*/
{

    //! Synchronize an ISCAT signal
    //! cdat() is the downsampled analytic signal.
    //! Sample rate = fsample = BW = DEC_SAMPLE_RATE * (9/32) = 3100.78125 Hz
    //! npts, nsps, etc., are all reduced by 9/32

    //int NMAX=30*3101;
    //int NSZ=4*1400;
    //qDebug()<<"WWWWWWWW";
    double complex c[289*4+10];
    double fs0_[96][289];                        //!108 = 96 + 3*4

    int icos[4] = {0,1,3,2};

    //qDebug()<<"WWWWWWWW"<<nlen;
    //! Silence compiler warnings:
    double sigbest=-20.0;
    int ndf0best=0;
    int msglenbest=0;
    int ipkbest=0;
    int jpkbest=0;
    int ipk2=0;
    //int idfbest=mousebutton;// no use 1.33

    double fsample=3100.78125;                   //!New sample rate
    int nsps=144/mode4;
    //nsym=npts/nsps - 1
    int nsym=npts/nsps - 0;//qDebug()<<nsym<<npts<<nsps;
    int nblk=nsync_rx+nlen_rx+ndat_rx;
    int nfft=2*nsps;                          //!FFTs at twice the symbol length,

    int kstep=nsps/4;                         //!  stepped by 1/4 symbol
    double df=(double)fsample/nfft;
    double fac=1.0/1000.0;                       //!Somewhat arbitrary

    double sref[289] = {0.0};
    double tmp[289];
    int nh = 3;
    int npct = 40;
    double savg[289];
    //zero_double_beg_end(sref,0,289);
    //zero_double_beg_end(tmp,0,289);

    //savg=0.
    //for (int i = 0; i<289; i++)
    pomAll.zero_double_beg_end(savg,0,289);
    /*for (int i = 0; i<5601; i++)
    {
        for (int j = 1; j<289; j++)
            s0_[i][j]=0.0;
    }*/

    //qDebug()<<nsps;
    int ia=0-kstep;//1-kstep po4va ot 1 a pri men ot 0
    // double hh[289];
    //double savg[289];

    //qDebug()<<npts<<4*nsym;
    for (int j = 0; j<4*nsym; j++)
    {//do j=1,4*nsym
        //j=0;
        //double h;                          //!Compute symbol spectra

        ia+=kstep;
        //ib=ia+nsps-1
        int ib=ia+nsps;

        //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
        if (ib>npts) break;
        //c(1:nsps)=fac*cdat(ia:ib)

        for (int x = 0; x<nsps; x++)
            c[x]=fac*cdat[x+ia];

        //for (int h = 0; h<nfft; h++)
        //c(nsps+1:nfft)=0.
        //c[h+nsps]=0.0+0.0*I;//hv pri 0 po dobre stana
        for (int h = nsps; h<nfft; h++)// 1.32
            //c(nsps+1:nfft)=0.
            c[h]=0.0+0.0*I;

        f2a.four2a_c2c(c,nfft,-1,1);

        /*fftw_plan plan;
        plan=fftw_plan_dft_1d(nfft,c,c,FFTW_FORWARD,FFTW_ESTIMATE_PATIENT);
        fftw_execute(plan);
        fftw_destroy_plan(plan);*/

        //qDebug()<<"pppppp"<<j<<nfft;
        for (int i = 0; i<nfft; i++)
        {//do i=1,nfft
            s0_[j][i]=pomAll.ps_hv(c[i]);//*0.0000000000001;
            savg[i]+=s0_[j][i];//!Accumulate avg spectrum
        }
    }

    jsym=4*nsym; //qDebug()<<savg[0]<<savg[1]<<savg[2]<<savg[3];
    //qDebug()<<"Sum20savg"<<savg[20];
    //qDebug()<<"Sum20h"<<hh[20];
    for (int i = 0; i<289; i++)
        savg[i]=(double)savg[i]/jsym;

    //qDebug()<<savg[20]<<jsym;

    for (int i = nh; i<nfft-nh; i++) //nfft=288/144  0,1,2
    {//do i=nh+1,nfft-nh
        sref[i] = pctile(savg,i-nh,tmp,2*nh+1,npct); //call pctile(savg(i-nh),tmp,2*nh+1,npct,sref(i))
    }
    for (int i = 0; i<nh; i++)
    {
        sref[i]=sref[nh+10]; //sref(1:nh)=sref(nh+11)
        sref[(nfft-nh+0)+i]=sref[(nfft-nh-1)]; //sref(nfft-nh+1:nfft)=sref(nfft-nh)
    }

    for (int i = 0; i<nfft; i++) //nfft=288/144
    {//do i=1,nfft                                 //!Normalize the symbol spectra
        //fac=(1.0/savg[i]);//qDebug()<<fac; 1.32
        fac=(1.0/sref[i]);  // 1.32
        //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
        //if(i.lt.11) fac=1.0/savg(11)
        if (i<10) fac=1.0/savg[10];
        for (int j = 0; j<jsym; j++)
        {//do j=1,jsym
            s0_[j][i]=fac*s0_[j][i];//*0.001;
        }
    }

/*    for (int j = 0; j<jsym; j++) //nfft=288/144
    {//do i=1,nfft                                 //!Normalize the symbol spectra
        for (int i = 0; i<nfft; i++)
        {//do j=1,jsym
        //fac=(1.0/savg[i]);//qDebug()<<fac; 1.32
        fac=(1.0/sref[i]);  // 1.32
        //if(i.lt.11) fac=1.0/savg(11)
        if (i<10) fac=1.0/savg[10];
            s0_[j][i]=fac*s0_[j][i];
        }
    }
*/

    double nfold=(double)jsym/96.0;//qDebug()<<nfold<<jsym;
    int jb=96.0*nfold;

    //double ttot=npts/fsample; // no used 1.33                         //!Length of record (s)
    //df1=df/ttot;                                      //!Step size for f1=fdot
    // no use 1.33
    //int idf1=-25.0/df1;
    //int idf2=5.0/df1;
    //qDebug()<<"IDF"<<idf1<<idf2;
    //if (nafc==0)
    //{
    //    idf1=0;
    //    idf2=0;
    //}
    //else if (fmod(-idf1,2)==1)
    //    idf1=idf1-1;

    double xsyncbest=0.0;
    //qDebug()<<"KKKK"<<idf1<<idf2<<df1;
    /// VAZNO <<<=== HV //////
    //for (idf = idf1; idf<=idf2; idf++)
    //{//do idf=idf1,idf2                         //!Loop over fdot
    for (int z = 0; z<96; z++)//288 -> 289 hv podobri se zna4itelno
    {//fs0[289][96]; fs0_[96][289];
        for (int x = 0; x<289; x++)
            fs0_[z][x]=0.0;
    }
    //qDebug()<<jb<<4*nblk;
    for (int j = 0; j<jb; j++)
    {//do j=1,jb                             //!Fold s0 into fs0, modulo 4*nblk
        //k=mod(j-1,4*nblk)+1
        int k=fmod(j,4*nblk);
        int ii=int(idf*double(j-jb/2.0)/double(jb));
        //ia=max(1-ii,1)
        ia=fmax(0-ii,0);//hv 1-143 posle 1-144 ???????
        int ib=fmin(nfft-ii,nfft);//qDebug()<<ia<<ib;
        //qDebug()<<"ia-ib"<<ia<<ib;
        for (int i = ia; i<=ib; i++)// po dobre <=
            //do i=ia,ib
            fs0_[k][i]+=s0_[j][i+ii];
        //enddo
    }
    //qDebug()<<fs0[20][20];
    double ref=nfold*4.0;

    //i0=27
    int i0=26;
    if (mode4==1) i0=94; //if(mode4.eq.1) i0=95
    ia=i0-(int)((double)DFTolerance/df);
    int ib=i0+(int)((double)DFTolerance/df);
    //qDebug()<<ia<<ib<<DFTolerance<<df;

    //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    //if(ia.lt.1) ia=1
    if (ia<0) ia=0;//hv
    //if(ib.gt.nfft-3) ib=nfft-3
    if (ib>nfft-3) ib=nfft-3;

    double smax=0.0;
    //ipk=1
    //jpk=1
    ipk=0;
    jpk=0;
    //qDebug()<<"Boo="<<4*nblk<<ia<<ib;
    for (int j = 0; j<4*nblk; j++) //4*nblk=96
    {//do j=0,4*nblk-1
        //qDebug()<<"ia-ib"<<ia<<ib;                           //!Find sync pattern: lags 0-95
        for (int i = ia; i<=ib; i++)
        {//do i=ia,ib                              //!Search specified freq range
            double ss=0.0;
            //int k;
            for (int n = 0; n<4; n++)
            {//do n=1,4
                //k=j+4*n-3                         //!Sum over 4 sync tones
                int k=j+4*n;
                //if(k.gt.96) k=k-96
                if (k>95)     k=k-96;
                //qDebug()<<j<<k;
                ss=ss + fs0_[k][i+2*icos[n]];
            }
            if (ss>smax)
            {   //qDebug()<<ss<<smax;
                smax=ss;
                ipk=i;
                //jpk=j+1                              //!Frequency offset, DF
                jpk=j;//hv 0 ima efekt               //!Time offset, DT
                //qDebug()<<"fs0_"<<k*i+2;
            }
        }
    }

    xsync=smax/ref - 1.0;//qDebug()<<smax<<ref<<nfold;
    //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    //qDebug()<<xsync<<sqrt(nfold/26.0);
    if (nfold<26) xsync=xsync * sqrt(nfold/26.0);
    xsync=xsync-0.5;                           //!Empirical

    sig=(pomAll.db(smax/ref - 1.0) -15.0);
    if (mode4==1) sig=sig-5.0;
    //if (nsig<-20 || xsync<1.0) nsig=-20; // 1.32
    //qDebug()<<"ndf0"<<ipk<<i0;
    ndf0=int(df*(ipk-i0));
    //if(ndf0>200)
    //qDebug()<<"ndf0"<<ndf0<<df<<ipk<<i0;

    smax=0.0;
    int ja=jpk+16-1;//tested hv 1.32 -1
    if (ja>4*nblk) ja=ja-4*nblk;
    int jj=jpk+20-1;//tested hv 1.32 -1
    if (jj>4*nblk) jj=jj-4*nblk;
    for (int i = ipk; i<=ipk+80; i+=2)//hv v1.01 ipk+60 to ipk+120 -> hv v1.32 ipk+120 to ipk+80 max 40 symbols
    {//do i=ipk,ipk+60,2                         //!Find User's message length
        double ss=fs0_[ja][i] + fs0_[jj][i+10];
        if (ss>smax)
        {
            smax=ss;
            ipk2=i;
        }
    }

    msglen=(ipk2-ipk)/2;
    //if (msglen<2 || msglen>40) msglen=3;//hv v1.01 29 to 40 kolkoto e v listata za 1 red max za iskat
    //if (msglen<2 || msglen>39) continue; //1.32 max 39 symbols
    if (msglen<2 || msglen>39) goto c22; // 1.33 no need loop

    if (xsync>=xsyncbest)
    {
        xsyncbest=xsync;
        sigbest=sig;
        ndf0best=ndf0;
        msglenbest=msglen;
        ipkbest=ipk;
        jpkbest=jpk;
        //idfbest=idf;
    }
    //}
c22:

    xsync=xsyncbest;
    sig=sigbest;
    ndf0=ndf0best;
    msglen=msglenbest;
    ipk=ipkbest;
    jpk=jpkbest;
    //idf=idfbest;
    //if (nafc==0) idf=0;

//qDebug()<<"IDF DF1"<<idf<<df1;
}

void DecoderMs::iscat(double complex*cdat0,int npts0,double t2,bool pick,int MinSigdB,int DFTolerance,int mode4)
/*no used 1.33 int mousebutton,int nafc,int nmore*/
{
    //! Decode an ISCAT signal
    // pick =true;
    bool f_only_one_color = true;
    const int NMAX=30*3101;
    //int NSZ=4*1400;=5600
    //character cfile6*6                      //!File time
    QString msg;
    QString msg1;
    QString msgbig;
    //char csync;//qDebug()<<"AAAAA";
    double complex cdat[NMAX];
    //double complex *cdat=new double complex[NMAX];
    //double s0_[5601][289];
    //auto s0_=new double[5601][289];
    double (*s0_)[289]=new double[5601][289];

    //qDebug()<<"BBBB";
    //int NSZ=4*1400;=5600
    //double s0[288][NSZ];
    //double (*s0)[5601] = new double[289][5601];

    //double fs1(0:41,30)
    double fs1_[60][42]; //hv v1.01 fs1[42][30] to fs1[42][60]
    //int icos[4] = {0,1,3,2};
    bool last = false;

    double fsample=3100.78125;                   //!Sample rate after 9/32 downsampling
    int nsps=144/mode4;
    double bigworst=-1.e30;                      //!Silence compiler warnings ...
    double bigxsync=0.0;
    double bsigbig=-1.e30;
    int msglenbig=0;
    int ndf0big=0;
    int nfdotbig=0;
    double bigt2=0.0;
    double bigavg=0.0;
    double bigtana=0.0;
    // 1.33 no use
    //if (nmore==-999) bsigbig=-1.0;        //!... to here
    double worst = 0.0;
    double avg=1.0;
    int ndf0=0;
    int ipk3=0;
    double tana = 0.0;
    double isync;
    int nfdot=0;

    double xsync = 0.0;
    int msglen = 0;
    int idf = 0;
    //double df1 = 0.0; no used 1.33
    int jsym = 0;
    int jpk = 0;
    int ipk = 0;
    int nsig = 0;
    int nworst = 0;
    double sig = 0.0;
    int navg = 0;
    QString csync;

    QStringList list_1;
    bool f_msg_1 = false;
    QStringList list_2;

    int nsum[60];//integer nsum(30)
    double dsp_tb = 0.0;
    double dsp_te = 0.0;

    for (int i = 0; i<5601; i++)
    {
        for (int j = 0; j<289; j++)
            s0_[i][j]=0.0;
    }

    for (int inf = 1; inf<=6; inf++)//do inf=1,6
    {                         //!Loop over data-segment sizes
        int nframes=pow(2,inf);//qDebug()<<nframes;
        //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
        //qDebug()<<nframes*24*nsps<<npts0;
        if (nframes*24*nsps>npts0)
        {
            nframes=npts0/(24*nsps);
            last=true; //qDebug()<<"LLLLLLLLLLL";
        }
        int npts=nframes*24*nsps;
        //int move_ib = 0;
        for (int ia = 0; ia<(npts0-npts); ia+=npts)//npts   (nsps*24)
        {    //do ia=1,npts0-npts,nsps*24        //!Loop over start times stepped by 1 frame
            //qDebug()<<ia<<npts0-npts;
            //if(ia==0)
            //qDebug()<<npts<<nframes<<ia<<last;
            double sum;
            int mpk = -1;
            int k;
            int n;
            double nfold;
            double jb;
            int nblk;
            //qDebug()<<ia;
            //int ib=ia+npts-1;
            //int ib=ia+npts-0;// da ne propuska stojnost

            //cdat(1:npts)=cdat0(ia:ib)
            //qDebug()<<"endA="<<ia+npts<<npts0;

            for (int i = 0; i<npts; i++)
                cdat[i]=cdat0[i+ia];

            //t4 = t2+((double)ia + (double)0.5*npts)/fsample + 0.9;
            //double t3=((double)ia + (double)0.5*npts)/fsample + 0.9; //old t3=t2+(ia + 0.5*npts)/fsample + 0.9
            //if (pick) t3=t4;//old t3=t2;   //t3=t2+t3

            //double t3=((double)ia + (double)0.5*npts)/fsample + 0.9;

            double t3=((double)ia + (double)0.5*npts)/fsample + 0.9;
            if (pick) t3=t2+t3;

            //move_ib += npts;
            //qDebug()<<"t3=============="<<t3;

            //! Compute symbol spectra and establish sync:
            synciscat(cdat,npts,s0_,jsym,DFTolerance,/**/
                      mode4,xsync,sig,ndf0,msglen,ipk,jpk,idf);/*mousebutton,nafc,idf,df1,*/
            //qDebug()<<"SyncOut="<<xsync<<nsig<<ndf0<<msglen<<ipk<<jpk<<idf<<df1;
            nfdot=(int)(idf*0.0);//int(idf*df1);
            //qDebug()<<"IDF DF1"<<idf<<df1<<msglen<<xsync;

            isync=xsync; //continue;
            if (msglen==0 || isync<fmax((double)MinSigdB,0))
            {
                msglen=0;
                worst=1.0;
                avg=1.0;
                ndf0=0;
                //qDebug()<<"msglen="<<msglen<<isync<<fmax(MinSigdB,0);
                //goto c100; //1.32
                continue;  //cycle 1.32
            }

            ipk3=0;                                  //!Silence compiler warning
            nblk=nsync_rx+nlen_rx+ndat_rx;

            //if(jpk==0)jpk=1;
            //qDebug()<<jpk<<nblk;
            //double t3=((((npts)-ia)/jpk)*nblk + 0 )/fsample + 0.9;//(double)0.5*npts
            //double t3=((double)ia )/fsample + 0.9;
            //if (pick) t3=t2+t3;
            //qDebug()<<t3;

            //qDebug()<<"s="<<nblk<<nsync_rx<<nlen_rx<<ndat_rx;
            for (int i = 0; i<42; i++)//42 ne pre4i
            {
                for (int z = 0; z<60; z++)//hv v1.01 z<30; to z<60;
                    fs1_[z][i] = 0.0;
            }

            zero_int_beg_end(nsum,0,60);
            nfold=jsym/96.0;
            jb=96.0*nfold;
            k=0;
            n=0;
            //qDebug()<<"Booming="<<jpk<<jsym<<ipk;
            for (int j = jpk; j<=jsym; j+=4)
            {//do j=jpk,jsym,4                //!Fold information symbols into fs1
                //k=k+1
                //km=mod(k-1,nblk)+1
                double km=fmod(k,nblk);
                //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
                //if(km.gt.6) then
                if (km>5)
                {
                    //n=n+1;
                    //m=mod(n-1,msglen)+1
                    int m=fmod(n,msglen);//qDebug()<<m;
                    int ii=int(idf*double(j-jb/2.0)/double(jb));
                    for (int i = 0; i<42; i++)
                    {//do i=0,41
                        int iii=ii+ipk+2*i;
                        //if(iii.ge.1 .and. iii.le.288)
                        if (iii>=0 && iii<=288)
                            fs1_[m][i]+=s0_[j][iii];
                    }
                    n++;
                    nsum[m]=nsum[m]+1;
                }
                k++;
            }

            for (int m = 0; m<msglen; m++)//fs1_[60][42];  real fs1(0:41,30)
            {//do m=1,msglen
                for (int x = 0; x<42; x++)
                {
                	double div = (double)nsum[m];
                	if(div==0.0)
                		div=1.0;
                    fs1_[m][x]=fs1_[m][x]/div;//fs1(0:41,m)=fs1(0:41,m)/nsum(m)
                    //if(fs1_[m][x]>3)
                    //qDebug()<<"m="<<fs1_[m][x]<<nsum[m];
                }

            }

            //! Read out the message contents:
            msg.clear();
            msg1.clear();

            //mpk=-1;
            worst=9999.0;
            sum=0.0;

            //if(msglen>29)msglen=29;

            for (int m = 0; m<msglen; m++)
            {//do m=1,msglen
                double smax=0.0;
                double smax2=0.0;
                for (int i = 0; i<42; i++)
                {//do i=0,41
                    if (fs1_[m][i]>smax)
                    {
                        smax=fs1_[m][i];
                        ipk3=i;
                    }
                }
                for (int i = 0; i<42; i++)
                {//do i=0,41
                    //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
                    if (fs1_[m][i]>smax2 && i!=ipk3)
                        smax2=fs1_[m][i];
                }
                double rr=0.0;
                if (smax2>0.0)
                    rr=smax/smax2;

                //qDebug()<<"rr="<<rr<<smax<<smax2<<m<<msglen;

                sum += rr;
                if (rr<worst) worst=rr;
                //if(ipk3.eq.40) mpk=m
                if (ipk3==40)
                    mpk=m;
                msg1.append(c_ISCAT_RX[ipk3]);
                //qDebug()<<"RR Worst="<<m<<rr<<worst;
            }
            //qDebug()<<"Booming="<<mpk;
            //qDebug()<<"MSG1="<<msg1;
            avg=sum/msglen;
            //qDebug()<<"avg================"<<avg<<sum<<msglen;
            //if(mpk.eq.1) then
            //if (mpk==0)
            //msg=msg1(2:)
            //copy_c2c(msg,0,msg1,1,29);
            // msg.append(msg1.mid(1,msg1.count()));
            if (mpk!=-1 && mpk<msglen)
                //msg=msg1(mpk+1:msglen)//msg1(1:mpk-1)
                //copy_c2c(msg,0,msg1,mpk+1,msglen);// hv +1 reze @ otpred
            {
                //qDebug()<<"1="<<msg1;
                msg.append(msg1.midRef(mpk+1,(msg1.count()-(mpk+1))));
                msg.append(msg1.midRef(0,mpk-0));
                //qDebug()<<"2="<<msg;
            }
            else
                //msg=msg1(1:msglen-1)
                //copy_c2c(msg,0,msg1,0,msglen-0);
                msg.append(msg1);

            //qDebug()<<"MSG="<<msg;

            if (worst>bigworst)
            {   //qDebug()<<"WorstOut="<<worst;
                bigworst=worst;
                bigavg=avg;
                bigxsync=xsync;
                bsigbig=sig;
                ndf0big=ndf0;
                nfdotbig=nfdot;
                msgbig=msg;
                //copy_c2c(msgbig,0,msg,0,29);
                msglenbig=msglen;
                //qDebug()<<"IDF0"<<ndf0<<msg<<t3;
                //qDebug()<<jpk<<ipk<<ia<<npts<<t3<<jpk;
                //if(ia==0)
                //bigt2 = 0.0;
                //else
                //qDebug()<<"IA="<<ia<<inf<<t3;
                //if(ia!=0 || inf<5)
                bigt2 = t3;

                bigtana=nframes*24.0*nsps/fsample;
                dsp_tb = (double)ia/fsample;
                dsp_te = (double)(ia+npts)/fsample;
                //qDebug()<<"T="<<t3<<dsp_tb<<dsp_te;
                //          tana=nframes*24*nsps/fsample;
                //if (bigworst>2.0) goto c110;
                //if (bigworst>2.0) break;
            }

            //!        if(minsigdb.le.0 .and. worst.gt.1.1) then
            //qDebug()<<avg<< xsync<<msg;
            isync = xsync; //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
            //if (navg>0 && isync>=fmax(MinSigdB,0))
            //if(avg.gt.2.5 .and. xsync.ge.max(float(minsync),1.5) .and. maxlines.ge.2)
            if (avg>2.5 && xsync>=fmax(double(MinSigdB),1.5) && bigworst>2.0) //v1.33 && bigworst>2.0
            {
                //qDebug()<<"avg="<<avg<<xsync<<worst;
                nsig=(int)sig;
                nworst=10.0*(worst-1.0);
                navg=10.0*(avg-1.0);
                if (nworst>10) nworst=10;
                if (navg>10) navg=10;
                tana=nframes*24.0*nsps/fsample;
                //qDebug()<<"tana="<<tana<<nframes<<nsps<<fsample;
                csync=" ";
                if (isync>=1) csync="* ";
                //if(nlines<=maxlines-1) nlines = nlines + 1 //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
                if (!msg.isEmpty() && !f_msg_1)    //1.33 && !f_msg_1
                {
                    list_1.clear();//samo poslednoto
                    list_1 <<s_time<<QString("%1").arg((int)isync)<<QString("%1").arg(nsig)<<
                    QString("%1").arg(t2,0,'f',1)<<QString("%1").arg(ndf0)<<QString("%1").arg(nfdot)
                    <<msg<<csync+QString("%1").arg(msglen)<<QString("%1").arg(nworst)
                    <<QString("%1").arg(navg)<<QString("%1").arg(tana,0,'f',1);
                    //t4 = t2;
                    f_msg_1=true;
                    //qDebug()<<avg<<xsync<<bigworst<<list_accumulate;
                }
                //if(isync>=10) goto c110;
            }
            //c100: continue; //1.32
            //continue;  //1.32
        }
        if (last) break;
        //if (last) goto c110;
        //c100:   continue;
    }
    //c110: //continue;

    worst=bigworst;
    avg=bigavg;
    xsync=bigxsync;
    sig=bsigbig;
    ndf0=ndf0big;
    nfdot=nfdotbig;
    msg=msgbig;
    msglen=msglenbig;  //qDebug()<<"t2="<<t2<<bigt2;
    t2=bigt2;
    tana=bigtana;

    //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    isync=xsync;
    nworst=10.0*(worst-1.0);
    navg=10.0*(avg-1.0);
    if (nworst>10) nworst=10;
    if (navg>10) navg=10;

    if (navg<=0 || isync<fmax(MinSigdB,0))
    {
        //for (int i = 0; i<29; i++)
        //msg[i]=' ';
        msg.clear();
        nworst=0;
        navg=0;
        ndf0=0;
        nfdot=0;
        sig=-20.0;
        msglen=0;
        tana=0.0;
        t2=0.0;
    }

    //if(isync.ge.1)
    if (isync>=1)
        csync="* ";

    nsig=(int)sig;


    bool f_msg_2=true;
    int nf1=-DFTolerance;
    int nf2=DFTolerance;
    if ((ndf0<nf1 || ndf0>nf2 || msg.isEmpty()))//hv pazi ot  msg-> 000 i prazno
        f_msg_2 = false;
    else
    {
        list_2.clear();
        list_2 <<s_time<<QString("%1").arg((int)isync)<<QString("%1").arg(nsig)<<
        QString("%1").arg(t2,0,'f',1)<<QString("%1").arg(ndf0)<<QString("%1").arg(nfdot)
        <<msg<<csync+QString("%1").arg(msglen)<<QString("%1").arg(nworst)
        <<QString("%1").arg(navg)<<QString("%1").arg(tana,0,'f',1);
    }


    if (f_msg_1 || f_msg_2)
    {
        if (f_only_one_color)
        {
            f_only_one_color = false;
            SetBackColor();
        }

        if (f_msg_1)
            emit EmitDecodetText(list_1,s_fopen,true);
        if (f_msg_2)
        {
            emit EmitDecodetText(list_2,s_fopen,true);//1.27 psk rep   fopen bool true    false no file open

            if (s_mousebutton == 0) // && t2!=0.0 1.32 ia is no real 0.0   mousebutton Left=1, Right=3 fullfile=0 rtd=2
            {
                emit EmitDecLinesPosToDisplay(1,dsp_tb,dsp_tb,s_time);
                emit EmitDecLinesPosToDisplay(2,dsp_te,dsp_te,s_time);
            }
        }

        /*if (t2>t4)
        {
            //int c_ = 1;
            if (f_msg4)
            {
                emit EmitDecodetText(list_4,s_fopen);
                //if (s_mousebutton == 0) //mousebutton Left=1, Right=3 fullfile=0 rtd=2
                    //emit EmitDecLinesPosToDisplay(c_,t4,t4,s_time);
                //c_++;
            }
            if (f_msg2)
            {
                emit EmitDecodetText(list_2,s_fopen);//1.27 psk rep   fopen bool true    false no file open

                if (s_mousebutton == 0) //mousebutton Left=1, Right=3 fullfile=0 rtd=2
                    emit EmitDecLinesPosToDisplay(1,t2,t2,s_time);
            }
        }
        else
        {
            //int c_ = 1;
            if (f_msg2)
            {
                emit EmitDecodetText(list_2,s_fopen);//1.27 psk rep   fopen bool true    false no file open

                if (s_mousebutton == 0) //mousebutton Left=1, Right=3 fullfile=0 rtd=2
                    emit EmitDecLinesPosToDisplay(1,t2,t2,s_time);
                //c_++;
            }
            if (f_msg4)
            {
                emit EmitDecodetText(list_4,s_fopen);
                //if (s_mousebutton == 0) //mousebutton Left=1, Right=3 fullfile=0 rtd=2
                    //emit EmitDecLinesPosToDisplay(c_,t4,t4,s_time);
            }
        }*/
    }

    /* niama smisal decodira vinagi samo vednaz 1.32
    if (s_mousebutton == 0 && disp_lines_dec_cou < MAX_DISP_DEC_COU) //mousebutton Left=1, Right=3 fullfile=0 rtd=2
       {
           disp_lines_dec_cou++;
           emit EmitDecLinesPosToDisplay(disp_lines_dec_cou,t2,t2,s_time);
       }
    */
    //qDebug()<<"DeLETEs0";
    delete [] s0_;
}

void DecoderMs::wsjt1_iscat(double *dat,int dat_count,int mode4,bool pick)
{
    int jz  = dat_count;
    double t2=0.0;
    int npts = 0;
    //double complex *cdat = new double complex[262145];
    double complex *cdat = new double complex[262145*2+10];
    int DFTolerance = G_DfTolerance;    //!Defines DF search range
    int MinSigdB = G_MinSigdB;
    //int NFreeze = 0;
    //int MouseDF = 0;
    //int mousebutton = 0;//1.33 no used
    //int nafc = 0;  //1.33 no used integer nafc           !Is AFC checked?
    //int ndebug = 0;  //1.33 no used integer ndebug         !Write debugging info?                   GUI
    //double psavg[450];
    //bool pick = false;
    //int istart =s_istart;

    //if (pick) t2=((double)s_istart+(double)0.5*jz)/DEC_SAMPLE_RATE + 0.5;          //!### +0.5 is empirical
    if (pick) t2=((double)s_in_istart)/DEC_SAMPLE_RATE;

    //for (int i = 0; i < jz; i++)
    //dat[i]=dat[i]*0.01;

    jz=fmin(jz,30.0*DEC_SAMPLE_RATE);//qDebug()<<jz;
    //jz=330750;
    //qDebug()<<"npts";
    //qDebug()<<dat[0]<<dat[4000]<<dat[40002]<<dat[40004]<<jz;
    npts = ana932(dat,jz,cdat);          //!Make downsampled analytic signal
    //for (int m = 0; m<jz; m++)
    //npts = 93030;

    //qDebug()<<npts<<creal(cdat[npts-2])<<creal(cdat[npts/2]);
    //!     write(74) npts,cfile6,(cdat(j),j=1,npts)

    //! Now cdat() is the downsampled analytic signal.
    //! New sample rate = fsample = BW = DEC_SAMPLE_RATE * (9/32) = 3100.78125 Hz
    //! NB: npts, nsps, etc., are all reduced by 9/32
    //qDebug()<<"CCCCC";

    iscat(cdat,npts,t2,pick,MinSigdB,DFTolerance,mode4);/* no used 1.33 mousebutton,nafc,ndebug*/
    //psavg(65:)=0.0;
    //go to 800;

    //qDebug()<<"DeLETE Cdat";
    delete [] cdat;//delete [] cdat;
}

