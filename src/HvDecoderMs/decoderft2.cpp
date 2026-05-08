/* The algorithms, source code, look-and-feel of WSJT-X and related
 * programs, and protocol specifications for the modes FSK441, FT8, JT4,
 * JT6M, JT9, JT65, JTMS, QRA64, ISCAT, MSK144, are Copyright © 2001-2017
 * by one or more of the following authors: Joseph Taylor, K1JT; Bill
 * Somerville, G4WJS; Steven Franke, K9AN; Nico Palermo, IV3NWV; Greg Beam,
 * KI7MT; Michael Black, W9MDB; Edson Pereira, PY2SDR; Philip Karn, KA9Q;
 * and other members of the WSJT Development Group.
 * FT2 mode created by IU8LMC, Martino — ARI Caserta.
 *
 * MSHV FT2 Decoder
 * Rewritten into C++ and modified by Hrisimir Hristov, LZ2HV 2015-2026
 * May be used under the terms of the GNU General Public License (GPL)
 */

#include "decoderms.h"
#include "../HvMsPlayer/libsound/genpom.h"
#include "ft_all_ap_def.h"
//#include <QtGui>

//! iaptype
//!------------------------
//!   1        CQ     ???    ???           (29 ap bits)
//!   2        MyCall ???    ???           (29 ap bits)
//!   3        MyCall DxCall ???           (58 ap bits)
//!   4        MyCall DxCall RRR           (77 ap bits)
//!   5        MyCall DxCall 73            (77 ap bits)
//!   6        MyCall DxCall RR73          (77 ap bits)
//!********
DecoderFt2::DecoderFt2(int id)
{
    pomFt.initPomFt();
    pomAll.initPomAll();//2.66 for pctile_shell
    decid = id;
    TGenFt2 = new GenFt2(true);//f_dec_gen = dec=true gen=false
    //nsps=288;//=288 12000hz
    //nsps*1.5=432.0
    gen_pulse_gfsk_(pulse_ft2_rx,432.0,1.0,288);
    first_ft2_ds = true;
    first_ft2detcad = true;
    first_ft2_sync4d = true;
    first_subsft2 = true;
    first_ft2bm = true;
    first_ft2d = true;
    DEC_SAMPLE_RATE = 12000.0;
    twopi=8.0*atan(1.0);
    pi=4.0*atan(1.0);
    cont_id0_ft2_2 = 0;
    //f_new_p = true;
    jseq_a=0;
    ft2_clravg(0);
    ft2_clravg(1); //printf(" EXP=%lf",exp(10));
    //////////AP7//////////////////
    jseq_a7=0;
    nutc0="-1";
    c_zerop = 0;
    for (int i= 0; i < 2; ++i)
    {
        for (int j= 0; j < 2; ++j) ndec[i][j] = 0;
    }
    //////////end AP7//////////////////
}
DecoderFt2::~DecoderFt2()
{}
static bool s_use_avg = false;//100% ok, by default in Main -> is ON=true In Dcoder need to be =false
void DecoderFt2::AvgDecodeChanged(bool f)
{
    s_use_avg = f; //qDebug()<<"s_use_avg="<<s_use_avg;
    ft2_clravg(0);
    ft2_clravg(1);
}
static double s_nftx2 = 1200.0;
void DecoderFt2::SetStTxFreq(double f)
{
    s_nftx2 = f;
}
static bool f_multi_answer_mod2 = false;
void DecoderFt2::SetStMultiAnswerMod(bool f)
{
    f_multi_answer_mod2 = f;
}
static int s_decoder_deep2 = 1;
void DecoderFt2::SetStDecoderDeep(int d)
{
    s_decoder_deep2 = d; //qDebug()<<"s_decoder_deep="<<s_decoder_deep2;
}
static bool s_lapon2 = false;// only in mshv
void DecoderFt2::SetStApDecode(bool f)
{
    s_lapon2 = f; //qDebug()<<"s_lapon2="<<s_lapon2;
}
static int s_nQSOProgress2 = 0;
void DecoderFt2::SetStQSOProgress(int i)
{
    s_nQSOProgress2 = i;
}
static QString s_time2 = "000000";
static int s_mousebutton2 = 0;
static bool s_fopen2 = false;
void DecoderFt2::SetStDecode(QString time,int mousebutton,bool ffopen)
{
    s_time2 = time;
    s_mousebutton2 = mousebutton;//mousebutton Left=1, Right=3 fullfile=0 rtd=2
    s_fopen2 = ffopen;
}
static QString s_MyCall2 = "NOT__EXIST";
//static QString s_MyBaseCall4 = "NOT__EXIST";
static int s_id_cont_ft2_28 = 0;
static int s_ty_cont_ft2_28 = 0;
static QString s_cont_ft2_cq = "CQ";
void DecoderFt2::SetStWords(QString s1,QString,int cq3,int ty4,QString cq)
{
    s_MyCall2 = s1;
    s_id_cont_ft2_28 = cq3;
    s_ty_cont_ft2_28 = ty4;
    s_cont_ft2_cq = cq;
}
static QString s_HisCall2 = "NOCALL";
void DecoderFt2::SetStHisCall(QString c)
{
    s_HisCall2 = c;
}
void DecoderFt2::SetMAMCalls(QStringList ls)
{
    TGenFt2->save_hash_call_mam(ls);
}
void DecoderFt2::ft2_downsample(double *dd,bool newdata,double f0,double complex *c)
{
    const int NMAX = 45000;//72576; //(NMAX=21*3456)=72576;
    const int NDOWN = 9;//18;
    const int NFFT2 = NMAX/NDOWN; //=5000
    const int NSPS = 288;//576;
    //double x[NMAX+50];
    double complex c1[6500]; //qDebug()<<"CORR======"<<NFFT2;
    double df=DEC_SAMPLE_RATE/(double)NMAX;
    double baud=DEC_SAMPLE_RATE/(double)NSPS;
    if (first_ft2_ds)
    {
        double bw_transition = 0.5*baud;
        double bw_flat = 4.0*baud;
        int iwt = (bw_transition/df);
        int iwf = (bw_flat/df);
        int z = iwt-1;
        for (int i = 0; i < iwt; ++i)
            window_ft2_ds[i] = 0.5*(1.0+cos(pi*(double)(z-i)/(double)iwt));//window(0:iwt-1) = 0.5*(1+cos(pi*(/(i,i=iwt-1,0,-1)/)/iwt))
        for (int i = iwt; i < iwt+iwf; ++i)
            window_ft2_ds[i]=1.0;//window(iwt:iwt+iwf-1)=1.0
        for (int i = 0; i < iwt; ++i)
            window_ft2_ds[i+iwt+iwf] = 0.5*(1.0+cos(pi*(double)(i)/(double)iwt));//window(iwt+iwf:2*iwt+iwf-1) = 0.5*(1+cos(pi*(/(i,i=0,iwt-1)/)/iwt))
        for (int i = 2*iwt+iwf; i < NFFT2; ++i)
            window_ft2_ds[i]=0.0;//window(2*iwt+iwf:)=0.0

        int iws = (int)(baud/df);
        pomAll.dshift1(window_ft2_ds,NFFT2,iws);//window_ft4_ds=cshift(window_ft4_ds,iws)  window=cshift(window,iws)
        first_ft2_ds=false;
    }
    if (newdata)
    {
        double *x = new double[NMAX+50];//
        for (int i = 0; i < NMAX; ++i) x[i]=dd[i]*0.01;
        f2a.four2a_d2c(cx_ft2_ds,x,NMAX,-1,0,decid);//four2a(x,NMAX,1,-1,0)            // !r2c FFT to freq domain
        delete [] x;
    }
    int i0=(int)(f0/df);
    for (int i = 0; i < NFFT2; ++i)
    {
        c[i]=0.0+0.0*I;
        c1[i]=0.0+0.0*I;
    }
    if (i0>=0 && i0<=NMAX/2) c1[0]=cx_ft2_ds[i0];
    for (int i = 1; i < NFFT2/2; ++i)//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    {//do i=1,NFFT2/2
        if (i0+i<NMAX/2) c1[i]=cx_ft2_ds[i0+i];//if(i0+i.le.NMAX/2) c1(i)=cx(i0+i)
        if (i0-i>=0) c1[NFFT2-i]=cx_ft2_ds[i0-i];//if(i0-i.ge.0) c1(NFFT2-i)=cx(i0-i)
    }
    for (int i = 0; i < NFFT2; ++i) c1[i]*=(window_ft2_ds[i]/(double)NFFT2);//c1=c1*window/NFFT2
    f2a.four2a_c2c(c1,NFFT2,1,1,decid);//call four2a(c1,NFFT2,1,1,1)            //!c2c FFT back to time domain
    for (int i = 0; i < NMAX/NDOWN; ++i) c[i]=c1[i];//c[]=c1(0:NMAX/NDOWN-1)
}
void DecoderFt2::ft2_baseline(double *s,int nfa,int nfb,double *sbase)
{
    const int NFFT1=1152;//2304;
    const int NH1=NFFT1/2;//ft2=576  ft4=1152
    double df=DEC_SAMPLE_RATE/(double)NFFT1;//!3.125 Hz
    int ia=fmax((int)(200.0/df),nfa);
    int ib=fmin(NH1,nfb);
    int nseg = 10;
    int npct = 10;
    double t_s[1252];//NH1/2
    //double *t_s = new double[1920+20];
    double x[1010];
    double y[1010];
    double a[8];

    for (int i = ia; i<ib; ++i)
    {//do i=ia,ib
        if (s[i]<0.000001) s[i]=0.000001;
        s[i]=10.0*log10(s[i]);            //!Convert to dB scale
    }

    int nterms=5;
    int nlen=(ib-ia+0)/nseg;                 //!Length of test segment
    int i0=(ib-ia+0)/2;                      //!Midpoint
    int k=0;//???

    for (int n = 0; n<nseg; ++n)
    {//do n=1,nseg                         //!Loop over all segments
        int ja=ia + (n-0)*nlen;
        int jb=ja+nlen-0;

        for (int z = 0; z<NH1-ja; ++z) t_s[z] = s[z+ja];//NH1=1152
        //qDebug()<<"ja jb"<<ja<<jb<<nlen<<1920-ja;
        double base = pomAll.pctile_shell(t_s,nlen,npct);//pctile(s(ja),nlen,npct,base); //!Find lowest npct of points
        //qDebug()<<"base"<<base;
        for (int i = ja; i<jb; ++i)
        {//do i=ja,jb
            if (s[i]<=base)// then //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
            {
                //if (k<1000) k++;       //!Save all "lower envelope" points
                x[k]=i-i0;
                y[k]=s[i];
                if (k<999) k++;
            }
        }
    }

    int kz=k;
    //if(kz<32) kz=32;

    pomAll.zero_double_beg_end(a,0,6);//a=0.
    //call polyfit(x,y,y,kz,nterms,0,a,chisqr)  //!Fit a low-order polynomial
    double chisqr = 0.0;
    pomAll.polyfit(x,y,y,kz,nterms,0,a,chisqr);
    //qDebug()<<"a"<<a[0]<<a[1]<<a[2]<<a[3]<<a[4]<<i0;
    for (int i = ia; i<ib; ++i)
    {//do i=ia,ib
        double t=i-i0;
        sbase[i]=a[0]+t*(a[1]+t*(a[2]+t*(a[3]+t*(a[4])))) + 0.50;//0.65;
        //!     write(51,3051) i*df,s(i),sbase(i)
        //!3051 format(3f12.3)
        sbase[i]=pow(10,(sbase[i]/10.0)); //sbase[i]=10**(sbase[i]/10.0)  nt=pow(2,2*nsym);//nt=2**(2*nsym)
    }
}
void DecoderFt2::getcandidates2(double *dd,double fa,double fb,double fa1,double fb1,double syncmin,double nfqso,
                                int maxcand,double candidate[2][250],int &ncand,double *sbase)
{
    const int NFFT1=1152;//2304;
    const int NSTEP=288;//576;
    const int NMAX=45000;//!Samples in iwave (3.75*12000)//21*3456;//=72576 !Samples in iwave
    const int NHSYM=((NMAX-NFFT1)/NSTEP); //=122 153hv !Number of symbol spectra (1/4-sym steps)
    const int NH1=NFFT1/2;
    double x[NFFT1+10];
    double complex cx[NH1+50];
    double s_[NHSYM+10][NH1+10];//(NH1,NHSYM)
    double savsm[NH1+50];
    //int indx[NH1+50];
    //double sbase[NH1+50];
    double savg[NH1+50];
    double candidatet[2][250];
    //qDebug()<<NHSYM;

    if (first_ft2detcad)
    {
        first_ft2detcad=false;
        for (int i = 0; i < NFFT1; ++i) window_ft2[i]=0.0;
        pomFt.nuttal_window(window_ft2,NFFT1);
    }

    //! Compute symbol spectra, stepping by NSTEP steps.
    for (int i = 0; i < NH1; ++i)
    {
        //sbase[i]=0.0;
        savg[i]=0.0;
    }

    //tstep=NSTEP/12000.0
    double df=12000.0/(double)NFFT1;
    double fac=1.0/300.0;
    for (int j = 0; j < NHSYM; ++j)//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    {//do j=1,NHSYM
        int ia=j*NSTEP;//ia=(j-1)*NSTEP + 1
        int ib=ia+NFFT1;   //ib=ia+NFFT1-1
        if (ib>NMAX) break;// if(ib.gt.NMAX) exit
        for (int z = 0; z < NFFT1; ++z) x[z]=fac*dd[ia+z]*window_ft2[z]*0.01;
        f2a.four2a_d2c(cx,x,NFFT1,-1,0,decid);              //!r2c FFT
        for (int i = 0; i < NH1; ++i)
        {//do i=1,NH1
            //s_[j][i]=pomAll.ps_hv(cx[i]);//old -> s(i,j)=real(cx(i))**2 + aimag(cx(i))**2
            s_[j][i]=cabs(cx[i])*cabs(cx[i]);//fin s(1:NH1,j)=abs(cx(1:NH1))**2
            savg[i]+=s_[j][i];
        }
        //savg=savg + s(1:NH1,j)                   !Average spectrum
    }
    for (int i = 0; i < NH1; ++i) savsm[i]=0.0;
    for (int i = 7; i < NH1-7; ++i)
    {//do i=8,NH1-7
        double sum = 0.0;
        for (int j = -7; j < 8; ++j) sum+=savg[i-j];
        savsm[i]=sum/15.0;//savsm(i)=sum(savg(i-7:i+7))/15.
    }

    //double corrt=0.0;//corr for threads
    //if (decid>0) corrt=100.0;
    //int nfa=(int)(fa-corrt)/df;
    int nfa=(int)fa/df;
    if (nfa<19) nfa=19;
    int nfb=(int)(fb)/df;
    if (nfb>471) nfb=471;

    int nfa1 = (int)fa1/df;//nfa;//
    if (nfa1<19) nfa1=19;
    int nfb1 = (int)fb1/df;//nfb;//
    if (nfb1>471) nfb1=471;

    ncand=0;
    //qDebug()<<200.0/df<<4910.0/df<<2000.0/df;// 38.3   942.7
    //1000/df=192=96 1200/df=230=115 1500/df=288=144 2000/df=384=192 3000/df=576=288 HV correction
    if ((fb1-fa1)<2000)
    {
        nfb1 = nfa1 + 192;//384;
        if (nfb1>471)//942)
        {
            nfb1 = 471; //5000.0/df=960
            //nfa1 = 960 - 384;
        }
    }
    //qDebug()<<decid<<nfa<<nfb<<"---"<<nfa1<<nfb1;
    ft2_baseline(savg,nfa1,nfb1,sbase);
    //ft4_baseline(savg,nfa,nfb,sbase);
    //ft4_baseline(savg,200.0/df,3500.0/df,sbase);//2.22 important
    bool any_is0 = false;
    for (int i = nfa; i < nfb; ++i)
    {
        if (sbase[i]<=0.0)
        {
            any_is0 = true;
            break;
        }
    }
    if (any_is0) return;
    //qDebug()<<"--------------"<<any_is0;

    /*double tdel = 0.0;
    int ctdel = 0;
    for (int i = nfa1+10; i < nfb1-10; i+=5)
    {
    	tdel += sbase[i];
    	ctdel++;
    }
    tdel /= (double)ctdel;
    if (tdel<=0.0) tdel = 0.001;*/
    for (int i = nfa; i < nfb; ++i)
    {
        savsm[i] /= sbase[i];
        //savsm[i] /= tdel;
    }
    //qDebug()<<decid<<nfa<<nfb<<"---"<<nfa1<<nfb1<<" dots="<<ctdel;
    double f_offset = -1.5*12000.0/(double)NSTEP;//-4.0;//no in 2.29 tested -> +0.3 for 32/64bit tested - 0.5
    for (int i = 0; i < 2; ++i)
    {
        for (int j = 0; j < maxcand; ++j) candidatet[i][j]=0.0;
    }
    int nq=0;
    for (int i = nfa+1; i < nfb-1; ++i) //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    {//do i=nfa+1,nfb-1
        //if (i*df>810 && i*df<880) qDebug()<<"Pickkkk===="<<decid<<(int)i*df<<savsm[i];
        if (savsm[i]>=savsm[i-1] && savsm[i]>=savsm[i+1] && savsm[i]>=syncmin)
        {
            //if (i*df>810 && i*df<880) qDebug()<<"Pickkkk===="<<decid<<(int)i*df<<savsm[i]<<"innnnn";
            double den=savsm[i-1]-2.0*savsm[i]+savsm[i+1];//den=savsm(i-1)-2*savsm(i)+savsm(i+1);
            double del=0.0;
            if (den!=0.0) del=0.5*(savsm[i-1]-savsm[i+1])/den;
            double fpeak=((double)i+del)*df+f_offset;
            if (fpeak<200.0 || fpeak>4910.0) continue;//cycle
            double speak=savsm[i]-0.25*(savsm[i-1]-savsm[i+1])*del;
            candidatet[0][ncand]=fpeak;//candidate(1,ncand)=fpeak
            candidatet[1][ncand]=speak;//candidate(3,ncand)=speak
            if (fabs(fpeak-nfqso)<=20.0) nq++;//int nq=count(fabs(candidatet(1,1:ncand)-nfqso)<=20.0)
            if (ncand<(maxcand-1)) ncand++;
            else break; //if(ncand.eq.maxcand) exit
        }
    }
    //qDebug()<<"test"<<ncand; //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    int n1=0;//1;
    int n2=nq;//nq+1;
    for (int i = 0; i < ncand; ++i)
    {
        if (fabs(candidatet[0][i]-nfqso)<=20.0)
        {
            candidate[0][n1]=candidatet[0][i];
            candidate[1][n1]=candidatet[1][i];
            n1++;//n1=n1+1
        }
        else
        {
            candidate[0][n2]=candidatet[0][i];
            candidate[1][n2]=candidatet[1][i];
            n2++;
        }
    }
    // Sort by sync Freq
    //qDebug()<<ncand<<nq<<nfqso;
    for (int i = 0; i < nq-1; ++i)
    {
        for (int j = i+1; j < nq; ++j)
        {
            if (candidate[1][j]>candidate[1][i])
            {
                double tmp1=candidate[0][i];
                double tmp2=candidate[1][i];
                candidate[0][i]=candidate[0][j];
                candidate[1][i]=candidate[1][j];
                candidate[0][j]=tmp1;
                candidate[1][j]=tmp2;
            }
        }
    }
    // Sort by sync all others
    for (int i = nq; i < ncand-1; ++i)
    {
        for (int j = i+1; j < ncand; ++j)
        {
            if (candidate[1][j]>candidate[1][i])
            {
                double tmp1=candidate[0][i];
                double tmp2=candidate[1][i];
                candidate[0][i]=candidate[0][j];
                candidate[1][i]=candidate[1][j];
                candidate[0][j]=tmp1;
                candidate[1][j]=tmp2;
            }
        }
    } //for (int i = 0; i < ncand; ++i) qDebug()<<candidate[1][i]<<candidate[0][i]; qDebug()<<"-------------------------";
}
void DecoderFt2::sync2d(double complex *cd0,int i0,double complex *ctwk,int itwk,double &sync)
{
    const int NDOWN=9;//18;
    const int NSPS=288;//576;//512;
    const int NSS=NSPS/NDOWN;  //576/18=32
    const int NMAX=45000;//21*3456;//=72576
    const int NP=NMAX/NDOWN;//=4032
    double complex csync2[2*NSS+20];//=64
    const int icos4a[4]=
        {
            0,1,3,2
        };
    const int icos4b[4]=
        {
            1,0,2,3
        };
    const int icos4c[4]=
        {
            2,3,1,0
        };
    const int icos4d[4]=
        {
            3,2,0,1
        };

    if (first_ft2_sync4d)
    {
        int k=0; //k=1;
        double phia=0.0;
        double phib=0.0;
        double phic=0.0;
        double phid=0.0;
        for (int i = 0; i < 4; ++i)
        {//do i=0,3
            double dphia=2.0*twopi*(double)icos4a[i]/(double)NSS;
            double dphib=2.0*twopi*(double)icos4b[i]/(double)NSS;
            double dphic=2.0*twopi*(double)icos4c[i]/(double)NSS;
            double dphid=2.0*twopi*(double)icos4d[i]/(double)NSS;
            for (int j = 0; j < NSS/2; ++j)
            {//do j=1,NSS/2
                csynca_ft2_sync[k]=cos(phia)+sin(phia)*I;
                csyncb_ft2_sync[k]=cos(phib)+sin(phib)*I;
                csyncc_ft2_sync[k]=cos(phic)+sin(phic)*I;
                csyncd_ft2_sync[k]=cos(phid)+sin(phid)*I;
                phia=fmod(phia+dphia,twopi);
                phib=fmod(phib+dphib,twopi);
                phic=fmod(phic+dphic,twopi);
                phid=fmod(phid+dphid,twopi);
                k++;//k=k+1
            }
        }
        first_ft2_sync4d=false;
        fac_ft2_sync=1.0/(double)(2.0*NSS);
    }

    int i1=i0;                            //!four Costas arrays
    int i2=i0+33*NSS;
    int i3=i0+66*NSS;
    int i4=i0+99*NSS;

    double complex z1=0.0+0.0*I;
    double complex z2=0.0+0.0*I;
    double complex z3=0.0+0.0*I;
    double complex z4=0.0+0.0*I;  //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.

    if (itwk==1) //csync2=ctwk*csynca      !Tweak the frequency
    {
        for (int i = 0; i < 2*NSS; ++i)
            csync2[i]=ctwk[i]*csynca_ft2_sync[i];
    }
    if (i1>=0 && i1+4*NSS-1<=NP-1) //z1=sum(cd0(i1:i1+4*NSS-1:2)*conjg(csync2))
    {
        int z =0;
        for (int i = 0; i < 2*NSS; ++i)
        {
            z1+=cd0[i1+z]*conj(csync2[i]);
            z+=2;
        }
    }
    else if (i1<0)
    {
        int npts=(i1+4*NSS-1)/2;//    npts=(i1+4*NSS-1)/2
        if (npts<=16)// if(npts.le.16) then
            z1=0.0+0.0*I;
        else
        {
            //z1=sum(cd0(0:i1+4*NSS-1:2)*conjg(csync2(2*NSS-npts:)))
            int z =0;
            for (int i = 2*NSS-npts; i < 2*NSS; ++i)//NSS=32 cd0[4032];
            {
                z1+=cd0[z]*conj(csync2[i]);
                z+=2;
            }
            //qDebug()<<"cd0="<<z<<"csync2="<<2*NSS-1;
        }
    }

    if (itwk==1) //csync2=ctwk*csyncb      //!Tweak the frequency
    {
        for (int i = 0; i < 2*NSS; ++i) csync2[i]=ctwk[i]*csyncb_ft2_sync[i];
    }
    if (i2>=0 && i2+4*NSS-1<=NP-1) //z2=sum(cd0(i2:i2+4*NSS-1:2)*conjg(csync2))
    {
        int z =0;
        for (int i = 0; i < 2*NSS; ++i)
        {
            z2+=cd0[i2+z]*conj(csync2[i]);
            z+=2;
        }
    }

    if (itwk==1) //csync2=ctwk*csyncc      !Tweak the frequency
    {
        for (int i = 0; i < 2*NSS; ++i) csync2[i]=ctwk[i]*csyncc_ft2_sync[i];
    }
    if (i3>=0 && i3+4*NSS-1<=NP-1) //z3=sum(cd0(i3:i3+4*NSS-1:2)*conjg(csync2))
    {
        int z =0;
        for (int i = 0; i < 2*NSS; ++i)
        {
            z3+=cd0[i3+z]*conj(csync2[i]);
            z+=2;
        }
    }

    if (itwk==1) //csync2=ctwk*csyncd      !Tweak the frequency
    {
        for (int i = 0; i < 2*NSS; ++i) csync2[i]=ctwk[i]*csyncd_ft2_sync[i];
    }
    if (i4>=0 && i4+4*NSS-1<=NP-1) //z4=sum(cd0(i4:i4+4*NSS-1:2)*conjg(csync2))
    {
        int z =0;
        for (int i = 0; i < 2*NSS; ++i)
        {
            z4+=cd0[i4+z]*conj(csync2[i]);
            z+=2;
        }
    }//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    else if (i4+4*NSS-1>NP-1) //else if( i4+4*NSS-1.gt.NP-1 ) then
    {
        int npts=(NP-1-i4+1)/2;//npts=(NP-1-i4+1)/2;
        if (npts<=16) z4=0.0+0.0*I;//if(npts.le.16) then
        else
        {
            //z4=sum(cd0(i4:i4+2*npts-1:2)*conjg(csync2(1:npts)))
            int z =0;
            for (int i = 0; i < npts; ++i)//NSS=32  cd0[4032];
            {
                z4+=cd0[i4+z]*conj(csync2[i]);
                z+=2;
            }//qDebug()<<"cd0="<<i4<<i4+(npts-1)<<"csync2="<<npts-1;
        }
    }
    //old p(z1)=real(z1*fac)**2 + aimag(z1*fac)**2
    //new p(z1)=(real(z1*fac)**2 + aimag(z1*fac)**2)**0.5
    //sync = ps_hv(z1*fac_ft4_sync) + ps_hv(z2*fac_ft4_sync) + ps_hv(z3*fac_ft4_sync) + ps_hv(z4*fac_ft4_sync);
    /*sync = pow(pomAll.ps_hv(z1*fac_ft2_sync),0.5) + pow(pomAll.ps_hv(z2*fac_ft2_sync),0.5) +
           pow(pomAll.ps_hv(z3*fac_ft2_sync),0.5) + pow(pomAll.ps_hv(z4*fac_ft2_sync),0.5);*/
    double p1=pow(pomAll.ps_hv(z1*fac_ft2_sync),0.5);
    double p2=pow(pomAll.ps_hv(z2*fac_ft2_sync),0.5);
    double p3=pow(pomAll.ps_hv(z3*fac_ft2_sync),0.5);
    double p4=pow(pomAll.ps_hv(z4*fac_ft2_sync),0.5);
    double pmin=p1;//fmin(p1,p2,p3,p4)
    if (pmin>p2) pmin=p2;
    if (pmin>p3) pmin=p3;
    if (pmin>p4) pmin=p4;
    sync = p1 + p2 + p3 + p4 - pmin; //static int g=0; if (g<20) printf("p1=%.3f p2=%.3f p3=%.3f p4=%.3f pmin=%.3f\n",p1,p2,p3,p4,pmin); g++;
}
void DecoderFt2::gen_ft2cwaveRx(int *i4tone,double f_tx,double complex *cwave)
{
    int nsym=103;
    int nsps=288; //=576      for 12000=512
    int nwave=(nsym+2)*nsps;//max rx=30240   max rx=53760
    double *dphi=new double[40100];  //max rx=30240
    double hmod=1.0;
    double dt=1.0/12000.0;// for RX=12000 for tx=48000
    //! Compute the smoothed frequency waveform.
    //! Length = (nsym+2)*nsps samples, zero-padded (nsym+2)*nsps TX=215040 RX=53760
    double dphi_peak=twopi*hmod/(double)nsps;
    for (int i= 0; i < 40000; ++i) dphi[i]=0.0;//max rx=30240
    for (int j= 0; j < nsym; ++j)
    {
        int ib=j*nsps;//=864
        for (int i= 0; i < 3*nsps; ++i) dphi[i+ib] += dphi_peak*pulse_ft2_rx[i]*(double)i4tone[j];
    }
    double ofs = twopi*f_tx*dt;
    double phi=0.0;
    for (int j= 0; j < nwave; ++j)//rx=30240
    {
        cwave[j]=cos(phi)+sin(phi)*I;
        phi=fmod(phi+dphi[j]+ofs,twopi);
    } //qDebug()<<"nsamp="<<nsamp<<nwave;
    for (int i = 0; i < nsps; ++i) cwave[i]*=(1.0-cos(twopi*(double)i/(2.0*nsps)))/2.0;
    int k2=(nsym+1)*nsps+1; //=2047 before stop
    for (int i = 0; i < nsps; ++i) cwave[i+k2]*=(1.0+cos(twopi*(double)i/(2.0*nsps)))/2.0;
    //qDebug()<<"nwave="<<nwave<<"nsamp="<<k2+nsps-1;
    delete [] dphi;
}
void DecoderFt2::subtractft2(double *dd,int *itone,double f0,double dt)
{
    //const int NSPS=288;//ft4=576;
    const int NFRAME=(103+2)*288;//ft2=30240    ft4=60480
    const int NMAX=45000;//!Samples in iwave (3.75*12000)//21*3456;//=72576
    const int NFILT=700;//ft4=1400;
    const int NFFT=NMAX;
    int offset_w = NFILT/2+25;
    int nstart=dt*12000.0+1.0-288.0; //qDebug()<<"NFRAME="<<NFRAME<<nstart;
    double complex *cref = new double complex[49000];//max 45000+ramp=47341?  ft4=rx=60481 +ramp
    double complex *cfilt= new double complex[52800];//45000     NMAX+100
    //double dd66[72800];// __attribute__((aligned(16)));
    //pomAll.zero_double_beg_end(dd66,0,72600);
    //double *dd66 = new double[72700]; // <- slow w10
    pomAll.zero_double_comp_beg_end(cfilt,0,(NMAX+25));
    //pomAll.zero_double_comp_beg_end(cref,0,NFRAME+25);
    gen_ft2cwaveRx(itone,f0,cref); //gen_ft4wave(itone,nsym,nsps,fs,f0,cref,xjunk,icmplx,NFRAME);
    if (first_subsft2)
    {
        //! Create and normalize the filter
        double window[NFILT+100] __attribute__((aligned(16)));//double *window= new double[NFILT+100];//
        double fac=1.0/double(NFFT);
        double sum=0.0;
        for (int j = -NFILT/2; j < NFILT/2; ++j)
        {//do j=-NFILT/2,NFILT/2
            window[j+offset_w]=cos(pi*(double)j/(double)NFILT)*cos(pi*(double)j/(double)NFILT);
            sum+=window[j+offset_w];
        }
        pomAll.zero_double_comp_beg_end(cw_subsft2,0,NMAX+25);
        if (sum<=0.0) // no devide by zero
            sum=0.01;
        for (int i = 0; i < NFILT+1; ++i)
            cw_subsft2[i]=window[i+offset_w-NFILT/2]/sum;//cw(1:NFILT+1)=window/sum

        pomAll.cshift1(cw_subsft2,NMAX,(NFILT/2+1));    //cw=cshift(cw,NFILT/2+1);

        f2a.four2a_c2c(cw_subsft2,NFFT,-1,1,decid);//call four2a(cw,nfft,1,-1,1)
        for (int i = 0; i < NMAX; ++i)
            cw_subsft2[i]*=fac;
        first_subsft2=false;
        //delete [] window;
    }
    for (int i = 0; i < NFRAME; ++i)
    {
        int id=nstart+i-1;//0 -1
        if (id>=0 && id<NMAX)
        {
            cfilt[i]=dd[id]*conj(cref[i]);//camp[i];//cfilt(1:nframe)=camp(1:nframe)
        }
    }
    f2a.four2a_c2c(cfilt,NFFT,-1,1,decid);//call four2a(cfilt,nfft,1,-1,1)
    for (int i = 0; i < NFFT; ++i) cfilt[i]*=cw_subsft2[i];//cfilt(1:nfft)=cfilt(1:nfft)*cw(1:nfft)
    f2a.four2a_c2c(cfilt,NFFT,1,1,decid);//call four2a(cfilt,nfft,1,1,1)
    //! Subtract the reconstructed signal
    for (int i = 0; i < NFRAME; ++i)//if(j.ge.1 .and. j.le.NMAX) dd(j)=dd(j)-2*REAL(cfilt(i)*cref(i))
    {//do i=1,nframe
        int j=nstart+i-1;//0 -1
        if (j>=0 && j<NMAX) dd[j]-=1.94*creal(cfilt[i]*cref[i]);//2.35 1.94   2.18 1.93
    }

    delete [] cref;
    delete [] cfilt;
}
void DecoderFt2::ft2_channel_est(double complex *cd,double complex *cd_eq,double *ch_snr)
{
    /*! Adaptive Channel Estimation for FT2
    ! ====================================
    ! Estimates complex channel gain H(k) from known Costas sync symbols,
    ! interpolates across data symbols, and equalizes the signal.
    !
    ! On HF ionospheric channels with selective fading and time variation,
    ! this provides +0.5-1.5 dB improvement over static AWGN assumption.
    !
    ! Method:
    !   1. Extract channel H(k) at 4 Costas array positions (16 symbols)
    !   2. Wiener-interpolate H(k) across all 103 symbols
    !   3. MMSE equalization: y_eq(k) = conj(H(k)) * y(k) / (|H(k)|^2 + Nvar)
    !   4. Per-symbol SNR estimate for LLR weighting
    !
    ! Input:  cd(0:NN*NSS-1) — downsampled complex signal
    ! Output: cd_eq(0:NN*NSS-1) — equalized signal
    !         ch_snr(NN) — per-symbol SNR estimate (linear scale)*/
    const int NN=103;
    const int NSPS=288;//576;
    const int NDOWN=9;//18;
    const int NSS=NSPS/NDOWN;//=32
    int sync_pos[20];
    double complex csymb[NSS+33];
    double complex cs_rx[5];//(0:3)! Received sync tones
    double complex h_sync[20];//(16)! Channel at sync positions
    double noise_var=0.0;
    double complex h_est[NN+10];//(NN)
    double w;
    double h_mag[NN+5];
    const int icos4a[4]=
        {
            0,1,3,2
        };
    const int icos4b[4]=
        {
            1,0,2,3
        };
    const int icos4c[4]=
        {
            2,3,1,0
        };
    const int icos4d[4]=
        {
            3,2,0,1
        };
    //! Fill sync symbol positions (1-based)
    //! Costas A: symbols 1-4, Costas B: 34-37, Costas C: 67-70, Costas D: 100-103
    for (int j = 0; j < 4; ++j)
    {//do j = 0, 3
        sync_pos[j]    = j;			//! Costas A
        sync_pos[j+4]  = j + 33;	//! Costas B
        sync_pos[j+8]  = j + 66;	//! Costas C
        sync_pos[j+12] = j + 99;	//! Costas D
    }
    //! =============================================
    //! Step 1: Estimate H(k) at sync positions
    //! =============================================
    //! For each sync symbol, the transmitted tone index is known (Costas sequence).
    //! H(k) = received_tone / expected_tone_phase
    double sum_noise = 0.0;
    double ncount = 0.0;
    for (int j = 0; j < 16; ++j)
    {//do j = 1, 16
        int k = sync_pos[j];
        int idx = k * NSS;
        //! FFT this symbol
        for (int z = 0; z < NSS; ++z) csymb[z] = cd[z+idx];//csymb = cd(idx:idx+NSS-1)
        f2a.four2a_c2c(csymb,NSS,-1,1,decid);//call four2a(csymb, NSS, 1, -1, 1)
        for (int z = 0; z < 4; ++z) cs_rx[z] = csymb[z];//cs_rx(0:3) = csymb(1:4)
        //! Known tone index for this sync symbol //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
        int itone=0;
        if 		(j<=3)  itone = icos4a[j];//(j-1)
        else if (j<=7)  itone = icos4b[j-4];//(j-5)
        else if (j<=11) itone = icos4c[j-8];//(j-9)
        else itone = icos4d[j-12];//(j-13)
        //! Channel estimate: H = received / expected
        //! Expected tone has unit magnitude, phase from DFT position
        //! Since the DFT of a pure tone at bin 'itone' gives just the complex value,
        //! H(k) = cs_rx(itone) (the expected reference is implicitly amplitude 1)
        h_sync[j] = cs_rx[itone];//(itone)
        //! Noise estimate from non-signal tones
        for (int m = 0; m < 4; ++m)
        {//do m = 0, 3
            if (m != itone)
            {
                sum_noise += pomAll.ps_hv(cs_rx[m]); //real(cs_rx(m))**2 + aimag(cs_rx(m))**2
                ncount += 1.0;
            }
        }
    }
    //! Average noise variance per tone
    //qDebug()<<cabs(h_sync[0])<<cabs(h_sync[1])<<cabs(h_sync[5])<<cabs(h_sync[12])<<cabs(h_sync[14])<<cabs(h_sync[15]);
    if (ncount > 0.0) noise_var = sum_noise / ncount;
    else noise_var = 1.0e-10;
    //! =============================================
    //! Step 2: Interpolate H(k) across all symbols
    //! =============================================
    //! Linear interpolation between nearest sync positions.
    //! This tracks time-varying fading across the 2.47s signal.
    //! First, assign H at sync positions
    for (int z = 0; z < NN+1; ++z) h_est[z] = 0.0+0.0*I;
    for (int j = 0; j < 16; ++j) h_est[sync_pos[j]] = h_sync[j];
    //! Interpolate between sync groups
    //! Group boundaries: 1-4, 34-37, 67-70, 100-103
    //! We interpolate between group centers: 2.5, 35.5, 68.5, 101.5
    //! Before first group center (symbols 1-2): use first group average
    h_est[0] =  h_sync[0];
    h_est[1] = (h_sync[0] + h_sync[1]) / 2.0;
    h_est[2] = (h_sync[1] + h_sync[2]) / 2.0;
    h_est[3] = (h_sync[2] + h_sync[3]) / 2.0;
    //! Between Costas A center (2.5) and Costas B center (35.5)
    for (int k = 4; k < 33; ++k)
    {//do k = 5, 33
        w = (double)(k - 2) / (double)(34 - 2);//w = real(k - 3) / real(35 - 3)     //! 0 at sym 3, 1 at sym 35
        w = fmax(0.0,fmin(1.0,w));
        h_est[k] = (1.0 - w) * (h_sync[2] + h_sync[3])/2.0 + w  * (h_sync[4] + h_sync[5])/2.0;
    }
    //! Costas B region
    h_est[33] = (h_sync[4] + h_sync[5]) / 2.0;
    h_est[34] = (h_sync[5] + h_sync[6]) / 2.0;
    h_est[35] = (h_sync[6] + h_sync[7]) / 2.0;
    h_est[36] =  h_sync[7];
    //! Between Costas B center (35.5) and Costas C center (68.5)
    for (int k = 37; k < 66; ++k)
    {//do k = 38, 66
        w = (double)(k - 35) / (double)(67 - 35);
        w = fmax(0.0,fmin(1.0,w));
        h_est[k] = (1.0 - w) * (h_sync[6] + h_sync[7])/2.0 + w  * (h_sync[8] + h_sync[9])/2.0;
    }
    //! Costas C region
    h_est[66] = (h_sync[8] + h_sync[9])  / 2.0;
    h_est[67] = (h_sync[9] + h_sync[10]) / 2.0;
    h_est[68] = (h_sync[10]+ h_sync[11]) / 2.0;
    h_est[69] =  h_sync[11];
    //! Between Costas C center (68.5) and Costas D center (101.5)
    for (int k = 70; k < 99; ++k)
    {//do k = 38, 66
        w = (double)(k - 68) / (double)(100 - 68);
        w = fmax(0.0,fmin(1.0,w));
        h_est[k] = (1.0 - w) * (h_sync[10] + h_sync[11])/2.0 + w  * (h_sync[12] + h_sync[13])/2.0;
    }
    //! Costas D region
    h_est[99]  = (h_sync[12] + h_sync[13]) / 2.0;
    h_est[100] = (h_sync[13] + h_sync[14]) / 2.0;
    h_est[101] = (h_sync[14] + h_sync[15]) / 2.0;
    h_est[102] =  h_sync[15];
    //! =============================================
    //! Step 3: MMSE Equalization
    //! =============================================
    //! y_eq = conj(H) * y / (|H|^2 + Nvar)
    //! This is the Wiener filter / MMSE equalizer
    for (int k = 0; k < NN; ++k)
    {//do k = 1, NN
        h_mag[k] = pomAll.ps_hv(h_est[k]);//real(h_est(k))**2 + aimag(h_est(k))**2 cabs(h_est[k]);//
    }
    for (int k = 0; k < NN; ++k)
    {//do k = 1, NN
        int idx = k * NSS;
        for (int z = 0; z < NSS; ++z) csymb[z] = cd[z+idx];//csymb = cd(idx:idx+NSS-1)
        //! MMSE equalization: multiply by conj(H)/(|H|^2 + Nvar)
        double den = h_mag[k] + noise_var;
        if (den > 1.0e-20)
        {
            for (int z = 0; z < NSS; ++z) cd_eq[z+idx] = csymb[z] * conj(h_est[k]) / den ;//cd_eq(idx:idx+NSS-1) = csymb * conjg(h_est(k)) / den
        }
        else
        {
            for (int z = 0; z < NSS; ++z) cd_eq[z+idx] = csymb[z];//cd_eq(idx:idx+NSS-1) = csymb
        }
        //! Per-symbol SNR estimate (linear)
        if (noise_var > 1.0e-20)
        {
            ch_snr[k] = h_mag[k] / noise_var;//ch_snr(k) = h_mag(k) / noise_var
        }
        else
        {
            ch_snr[k] = 100.0;  //! Very high SNR
        }
    }
}
void DecoderFt2::get_ft2_bitmetrics(double complex *cd,double bitmetrics_[3][220],bool &badsync,double s4_[120][4])
{
    const int NN=103;
    const int NSPS=288;//576;
    const int NDOWN=9;//18;
    const int NSS=NSPS/NDOWN;//=32
    double complex csymb[NSS+10];//complex csymb(NSS)
    double complex cs_[NN+10][4];//complex cs(0:3,NN)
    //double s4_[NN+10][4];// real s4(0:3,NN)
    //double s2[256+20];
    const int graymap[4]=
        {
            0,1,3,2
        };
    const int icos4a[4]=
        {
            0,1,3,2
        };
    const int icos4b[4]=
        {
            1,0,2,3
        };
    const int icos4c[4]=
        {
            2,3,1,0
        };
    const int icos4d[4]=
        {
            3,2,0,1
        };
    double pwr_[NN][4];//(0:3,NN)! Per-tone power |cs|?
    double noise_var;//! Estimated noise variance
    double beta;//! Soft metric scale = 1/(2*noise_var)
    double complex ctmp;
    double sp[256+10];//(0:255);//! Power-domain metrics for log-sum-exp
    double pwr_eq_[NN][4];
    double sp_eq[256+10];////(0:255)

    if (first_ft2bm)
    {
        for (int i = 0; i < 8; ++i)//one_ft4_2[8][256];
        {//do i=0,255
            for (int j = 0; j < 256; ++j)
            {//do j=0,7 //if(iand(i,2**j).ne.0) one(i,j)=.true.
                if ((j & (int)pow(2,i))!=0) one_ft2_2[i][j]=true;
                else one_ft2_2[i][j]=false;
            }
        }
        first_ft2bm = false;
    }

    for (int k = 0; k < NN; ++k)//NN=103
    {//do k=1,NN
        int i1=k*NSS;//i1=(k-1)*NSS
        for (int z = 0; z < NSS; ++z) csymb[z]=cd[z+i1];//NSS=32 //csymb=cd(i1:i1+NSS-1)
        f2a.four2a_c2c(csymb,NSS,-1,1,decid);//four2a(csymb,NSS,1,-1,1)
        for (int x = 0; x < 4; ++x)
        {
            cs_[k][x]=csymb[x];      //cs(0:3,k)=csymb(1:4)
            s4_[k][x]=cabs(csymb[x]);//s4(0:3,k)=abs(csymb(1:4))
            pwr_[k][x]=pomAll.ps_hv(csymb[x]);//real(csymb(1:4))**2 + aimag(csymb(1:4))**2
        }
    }

    //! -- Noise variance estimation -----------------------------
    //! Use Costas sync symbols (known positions) to estimate noise.
    //! For each sync symbol position, the signal is in ONE known tone;
    //! the other 3 tones contain only noise.
    //! Sync positions: 1-4 (icos4a), 34-37 (icos4b), 67-70 (icos4c), 100-103 (icos4d)
    double noise_sum = 0.0;
    int nnoise = 0;
    for (int k = 0; k < 4; ++k)
    {//do k=1,4
        for (int itone = 0; itone < 4; ++itone)
        {// do itone=0,3
            if (itone != icos4a[k])
            {
                noise_sum += pwr_[k][itone];//(itone,k)
                nnoise++;
            }
            if (itone != icos4b[k])
            {
                noise_sum += pwr_[k+33][itone];//(itone,k)
                nnoise++;
            }
            if (itone != icos4c[k])
            {
                noise_sum += pwr_[k+66][itone];//(itone,k)
                nnoise++;
            }
            if (itone != icos4d[k])
            {
                noise_sum += pwr_[k+99][itone];//(itone,k)
                nnoise++;
            }
        }
    }
    if (nnoise>0) noise_var = noise_sum / (double)nnoise; //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    else noise_var = 1.0;
    if (noise_var < 1.0e-10) noise_var = 1.0e-10;
    //! beta = scaling factor for log-sum-exp
    //! Theoretically beta = 1/(2*sigma?), but we use a tuned factor
    //! that accounts for correlation structure and non-Gaussianity
    beta = 0.5 / noise_var;
    //! Clamp beta to avoid numerical issues at extreme SNR
    beta = fmax(0.01,fmin(50.0,beta));

    //! Sync quality check
    int is1=0;
    int is2=0;
    int is3=0;
    int is4=0;
    badsync=false;
    for (int k = 0; k < 4; ++k)//s4_[103][4] s4(0:3,NN) NN=103
    {//do k=1,4
        int ip=pomAll.maxloc_da_beg_to_end(s4_[k],0,4);	//ip=maxloc(s4(:,k))    ip=maxloc_da_beg_to_end(s8_[k],0,8);
        if (icos4a[k]==ip) is1++;          //if(icos4a(k-1).eq.(ip(1)-1)) is1=is1+1
        ip=pomAll.maxloc_da_beg_to_end(s4_[k+33],0,4);	//ip=maxloc(s4(:,k+33))
        if (icos4b[k]==ip) is2++;			//if(icos4b(k-1).eq.(ip(1)-1)) is2=is2+1
        ip=pomAll.maxloc_da_beg_to_end(s4_[k+66],0,4);	//ip=maxloc(s4(:,k+66))
        if (icos4c[k]==ip) is3++;			//if(icos4c(k-1).eq.(ip(1)-1)) is3=is3+1
        ip=pomAll.maxloc_da_beg_to_end(s4_[k+99],0,4);  //ip=maxloc(s4(:,k+99))
        if (icos4d[k]==ip) is4++;           //if(icos4d(k-1).eq.(ip(1)-1)) is4=is4+1
    }
    int nsync=is1+is2+is3+is4; //printf(" %d= %d= %d= %d= %d= \n",nsync,is1,is2,is3,is4);
    if (nsync < 7)//if (nsync < 5)//if (nsync < 4)//ft4=8)
    {
        badsync=true;
        return;
    }
    //if (smax < 0.7 || nsync < 8) continue;//cycle //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    //qDebug()<<"2nsync="<<nsync<<is1<<is2<<is3<<is4;
    /*! =======================================================
    ! LOG-SUM-EXP EXACT SOFT DEMAPPER
    ! =======================================================
    ! For each bit position, compute:
    !   LLR = log[? exp(?·|s|?) for symbols where bit=1]
    !       - log[? exp(?·|s|?) for symbols where bit=0]
    !
    ! This is the EXACT log-likelihood ratio, vs the max-log
    ! approximation which replaces log(?exp) with max.
    ! Gain: +0.5-1.0 dB at low SNR where multiple symbols
    ! contribute meaningfully to the likelihood.
    !
    ! Numerically stable: subtract max before exp to avoid overflow.*/
    for (int nseq = 1; nseq <= 3; ++nseq)
    {//do nseq=1,3            // !Try coherent sequences of 1, 2, and 4 symbols
        int nsym=1;
        if (nseq==1) nsym=1;
        if (nseq==2) nsym=2;
        if (nseq==3) nsym=4;
        int nt=pow(2,2*nsym);//nt=2**(2*nsym)
        for (int ks = 1; ks <= NN-nsym+1; ks+=nsym)//NN=103  103-4+1
        {//do ks=1,NN-nsym+1,nsym  //!87+16=103 symbols.
            //amax=-1.0
            for (int i = 0; i < nt; ++i)//graymap[4]
            {//do i=0,nt-1
                int i1=i/64;
                int i2=(i & 63)/16;
                int i3=(i & 15)/4;
                int i4=(i & 3);
                if (nsym==1) //s2[256]
                {
                    ctmp=cs_[ks-1][graymap[i4]];
                    //s2[i]=cabs(ctmp);//graymap(i4),ks)
                    sp[i]=pomAll.ps_hv(ctmp);
                }
                else if (nsym==2) //cs_(graymap(i3),ks)+cs(graymap(i4),ks+1)
                {
                    ctmp=cs_[ks-1][graymap[i3]]+cs_[ks+0][graymap[i4]];
                    //s2[i]=cabs(ctmp);
                    sp[i]=pomAll.ps_hv(ctmp);
                }
                else if (nsym==4)//cs_(graymap(i1),ks) + cs(graymap(i2),ks+1) + cs(graymap(i3),ks+2) + cs(graymap(i4),ks+3)
                {
                    ctmp=cs_[ks-1][graymap[i1]] + cs_[ks+0][graymap[i2]] + cs_[ks+1][graymap[i3]] + cs_[ks+2][graymap[i4]];
                    //s2[i]=cabs(ctmp);
                    sp[i]=pomAll.ps_hv(ctmp);
                }
                //else //no possyble
                //print*,"Error - nsym must be 1, 2, or 4."
            }
            int ipt=0+(ks-1)*2;//ipt=1+(ks-1)*2;
            int ibmax = 1;
            if (nsym==1) ibmax=1;
            if (nsym==2) ibmax=3;
            if (nsym==4) ibmax=7; //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
            //! Scale beta for multi-symbol coherent combining
            //! (noise variance scales with nsym for coherent sum)
            double beta_eff = beta / (double)nsym;
            for (int ib = 0; ib <= ibmax; ++ib)//bmeta[206] (2*NN)
            {//do ib=0,ibmax   //s2[256]  bool one_ft4_2[8][256];//(0:255,0:7)
                //bm=maxval(s2(0:nt-1),one(0:nt-1,ibmax-ib))-maxval(s2(0:nt-1),.not.one(0:nt-1,ibmax-ib))
                if (ipt+ib>(2*NN-1)) continue;
                //! -- Log-Sum-Exp for bit=1 --
                double maxp1 = -1.0e30;
                for (int i = 0; i < nt; ++i)
                {//do i=0,nt-1
                    if (one_ft2_2[ibmax-ib][i])//if(one(i,ibmax-ib)) then
                    {
                        double pval = beta_eff * sp[i];
                        if (pval > maxp1) maxp1 = pval;
                    }
                }
                double lse1 = 0.0;
                for (int i = 0; i < nt; ++i)
                {//do i=0,nt-1
                    if (one_ft2_2[ibmax-ib][i])
                    {
                        double exp0 = beta_eff*sp[i] - maxp1;
                        if (exp0>20.0) exp0=20.0;
                        lse1 += exp(exp0);//if(one(i,ibmax-ib)) then
                    }
                } //if(lse1>200) qDebug()<<"0--"<<lse1;
                lse1 = maxp1 + log(fmax(lse1,1.0e-30));
                //! -- Log-Sum-Exp for bit=0 --
                double maxp0 = -1.0e30;
                for (int i = 0; i < nt; ++i)
                {//do i=0,nt-1
                    if (!one_ft2_2[ibmax-ib][i])//if(.not.one(i,ibmax-ib)) then
                    {
                        double pval = beta_eff * sp[i];
                        if (pval > maxp0) maxp0 = pval;
                    }
                }
                double lse0 = 0.0;
                for (int i = 0; i < nt; ++i)
                {//do i=0,nt-1
                    if (!one_ft2_2[ibmax-ib][i])
                    {
                        double exp0 = beta_eff*sp[i] - maxp0;
                        if (exp0>20.0) exp0=20.0;
                        lse0 += exp(exp0);
                    }
                } //if(lse0>200) qDebug()<<"1--"<<lse0;
                lse0 = maxp0 + log(fmax(lse0,1.0e-30));
                bitmetrics_[nseq-1][ipt+ib] = lse1 - lse0;//bitmetrics_(ipt+ib,nseq) = lse1 - lse0
            }
        }
    }
    //////////////////////////////////////////////
    double complex cd_eq[4000]; //for (int i = 0; i < 4000; ++i) cd_eq[i]=0.0+0.0*I;
    double ch_snr[NN+10];//103
    double complex csymb_eq[NSS+32];
    //double complex cs_eq_[NN+5][4];//(0:3,NN)
    //double s2_eq[256+10];//(0:255)
    double bmet_eq[2*NN+20];//! Equalized single-symbol metrics
    ft2_channel_est(cd,cd_eq,ch_snr);//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    //! Detect fading: if SNR varies >6dB across symbols, channel is fading
    double snr_min = ch_snr[0]; //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    double snr_max = ch_snr[0];
    double snr_mean = 0.0;
    double fading_depth=0.0;
    for (int k = 0; k < NN; ++k)
    {//do k = 1, NN
        if (ch_snr[k] < snr_min) snr_min = ch_snr[k];
        if (ch_snr[k] > snr_max) snr_max = ch_snr[k];
        snr_mean += ch_snr[k];
    }
    snr_mean = snr_mean/(double)NN;
    //! Fading depth in dB (ratio of max to min channel power)
    if (snr_min > 1.0e-10)
    {
        double ld = snr_max / snr_min;
        if (ld<0.000001) ld=0.000001;
        fading_depth = 10.0 * log10(ld);
    }
    else fading_depth = 30.0;  //! Deep fade detected
    //! Use channel-equalized metrics if fading >3 dB (otherwise AWGN, no benefit)
    bool use_cheq = false;//3.0
    const double limdB=6.0;//4.0;
    if (fading_depth > limdB) use_cheq = true;
    //if (snr_max-snr_min > 3.0) use_cheq = true;
    //if (use_cheq && fading_depth<30) qDebug()<<fading_depth<<use_cheq;//<<snr_max<<snr_min<<
    //if (!use_cheq) qDebug()<<" ---  "<<fading_depth<<use_cheq;

    /*bitmetrics_[1][204]=bitmetrics_[0][204];
    bitmetrics_[1][205]=bitmetrics_[0][205];
    for (int zz = 200; zz < 204; ++zz) bitmetrics_[2][zz]=bitmetrics_[1][zz];
    bitmetrics_[2][204]=bitmetrics_[0][204];
    bitmetrics_[2][205]=bitmetrics_[0][205];*/

    if (use_cheq)
    {	//! Compute single-symbol metrics on equalized signal
        for (int k = 0; k < NN; ++k)
        {//do k=1,NN
            int i1=k*NSS;
            for (int z = 0; z < NSS; ++z) csymb_eq[z]=cd_eq[z+i1];//csymb_eq=cd_eq(i1:i1+NSS-1)
            f2a.four2a_c2c(csymb_eq,NSS,-1,1,decid);//call four2a(csymb_eq,NSS,1,-1,1)
            //for (int z = 0; z < 4; ++z) cs_eq_[k][z]=csymb_eq[z];//cs_eq(0:3,k)=csymb_eq(1:4)
            //s4_eq(0:3,k)=abs(csymb_eq(1:4))
            for (int x = 0; x < 4; ++x) pwr_eq_[k][x]=pomAll.ps_hv(csymb_eq[x]);//real(csymb_eq(1:4))**2 + aimag(csymb_eq(1:4))**2
        }
        //! Estimate noise from equalized signal sync positions
        double noise_sum_eq = 0.0;
        double nnoise_eq = 0;
        for (int k = 0; k < 4; ++k)
        {//do k=1,4
            for (int itone = 0; itone < 4; ++itone)
            {// do itone=0,3
                if (itone != icos4a[k])
                {
                    noise_sum_eq += pwr_eq_[k][itone];//(itone,k)
                    nnoise_eq++;
                }
                if (itone != icos4b[k])
                {
                    noise_sum_eq += pwr_eq_[k+33][itone];//(itone,k)
                    nnoise_eq++;
                }
                if (itone != icos4c[k])
                {
                    noise_sum_eq += pwr_eq_[k+66][itone];//(itone,k)
                    nnoise_eq++;
                }
                if (itone != icos4d[k])
                {
                    noise_sum_eq += pwr_eq_[k+99][itone];//(itone,k)
                    nnoise_eq++;
                }
            }
        }
        double noise_var_eq = noise_sum_eq / fmax(1.0,(double)nnoise_eq);
        if (noise_var_eq < 1.0e-10) noise_var_eq = 1.0e-10;
        double beta_eq = 0.5 / noise_var_eq;
        beta_eq = fmax(0.01,fmin(50.0,beta_eq));
        //! SNR-weighted single-symbol metrics from equalized signal
        for (int ks = 0; ks < NN; ++ks)
        {//do ks=1,NN
            for (int i = 0; i < 4; ++i)
            {
                //s2_eq[i]=cabs(cs_eq_[ks][graymap[i]]);//(graymap[i],ks))
                sp_eq[i]=pwr_eq_[ks][graymap[i]];//(graymap(i),ks)
            }
            int ipt=ks*2;//ipt=1+(ks-1)*2
            //! Weight by per-symbol SNR: high SNR symbols get more influence
            double snr_weight = 1.0;
            if (snr_mean > 1.0e-10)
            {
                snr_weight = sqrt(ch_snr[ks] / snr_mean);
                snr_weight = fmax(0.1,fmin(3.0,snr_weight));//fmax(0.1,fmin(limdB,snr_weight));
                //if (snr_weight>2.8) qDebug()<<snr_weight;
            }
            int ibmax = 1;
            for (int ib = 0; ib <= ibmax; ++ib)
            {   //bm=maxval(s2_eq(0:3),one(0:3,1-ib)) - maxval(s2_eq(0:3),.not.one(0:3,1-ib))
                //if(ipt+ib.le.2*NN) bmet_eq(ipt+ib) = bm * snr_weight
                if (ipt+ib>(2*NN-1)) continue;
                //! -- Log-Sum-Exp for bit=1 --
                double maxp1 = -1.0e30;
                double maxp0 = -1.0e30;
                for (int i = 0; i < 4; ++i)
                {//do i=0,nt-1
                    double pval = beta_eq * sp_eq[i];
                    if (one_ft2_2[ibmax-ib][i])//if(one(i,ibmax-ib)) then
                    {
                        if (pval > maxp1) maxp1 = pval;
                    }
                    else
                    {
                        if (pval > maxp0) maxp0 = pval;
                    }
                }
                double lse1 = 0.0;
                double lse0 = 0.0;
                for (int i = 0; i < 4; ++i)
                {//do i=0,nt-1
                    if (one_ft2_2[ibmax-ib][i])
                    {
                        double exp0 = beta_eq*sp_eq[i] - maxp1;
                        if (exp0>20.0) exp0=20.0;
                        lse1 += exp(exp0);
                    }
                    else
                    {
                        double exp0 = beta_eq*sp_eq[i] - maxp0;
                        if (exp0>20.0) exp0=20.0;
                        lse0 += exp(exp0);
                    }
                } //if(lse0>10 || lse1>10) qDebug()<<"2--"<<lse0<<lse1;
                lse1 = maxp1 + log(fmax(lse1,1.0e-30));
                lse0 = maxp0 + log(fmax(lse0,1.0e-30));
                bmet_eq[ipt+ib] = (lse1 - lse0) * snr_weight;
            }
        } //qDebug()<<bmet_eq[0]<<bmet_eq[10]<<bmet_eq[30]<<bmet_eq[40]<<bmet_eq[50];
        pomFt.normalizebmet(bmet_eq,2*NN);//call normalizebmet(bmet_eq,2*NN)
        //! Blend: replace metric 1 with weighted average of original and equalized
        //! More fading > more weight to equalized metrics
        //double blend = fmin(1.0,(fading_depth - 3.0) / 12.0);//! 0 at 3dB, 1 at 15dB
        double blend = fmin(1.0,(fading_depth - limdB) / 10.0);//! 0 at limdB, 1 at 15dB
        blend = fmax(0.0,fmin(0.90,blend));//! Cap at 0.8 to keep some original info
        //! Normalize original metric 1 first for proper blending
        pomFt.normalizebmet(bitmetrics_[0],2*NN);//call normalizebmet(bitmetrics(:,1),2*NN)
        for (int i = 0; i < 2*NN; ++i)
        {//do i=1,2*NN
            bitmetrics_[0][i] = (1.0-blend)*bitmetrics_[0][i] + blend*bmet_eq[i];//bitmetrics(i,1) = (1.0-blend)*bitmetrics(i,1) + blend*bmet_eq(i)
        }//! Re-normalize after blending
        //pomFt.normalizebmet(bitmetrics_[0],2*NN);//call normalizebmet(bitmetrics(:,1),2*NN)
    }
    //else pomFt.normalizebmet(bitmetrics_[0],2*NN);//call normalizebmet(bitmetrics(:,1),2*NN)
    /////////////////////////////////////////////////////////////////////////////////////
    bitmetrics_[1][204]=bitmetrics_[0][204];//bmetb(205:206)=bmeta(205:206)
    bitmetrics_[1][205]=bitmetrics_[0][205];
    for (int zz = 200; zz < 204; ++zz) bitmetrics_[2][zz]=bitmetrics_[1][zz];//bmetc(201:204)=bmetb(201:204)
    bitmetrics_[2][204]=bitmetrics_[0][204];//bmetc(205:206)=bmeta(205:206)
    bitmetrics_[2][205]=bitmetrics_[0][205];
    pomFt.normalizebmet(bitmetrics_[0],2*NN);
    pomFt.normalizebmet(bitmetrics_[1],2*NN);
    pomFt.normalizebmet(bitmetrics_[2],2*NN);
}
void DecoderFt2::ft2_clravg(int jseq)
{
    //const int ND=87;
    //const int NS=16;
    //const int NN=NS+ND; //=206
    for (int x = 0; x < 3; ++x)//y=2*NN
    {
        for (int y = 0; y < 207; ++y) ft2avg[jseq].bm_avg_[x][y]=0.0;
    }
    ft2avg[jseq].navg_ft2=0;
    ft2avg[jseq].f_avg=0.0;
    ft2avg[jseq].dt_avg=0.0;
}
int DecoderFt2::ft2_even_odd(QString s)//res=0 (even first), or res=1 (odd second)
{
    bool res = 0;
    if (s.count()<6) return 0; //2.70 protection
    int time_ss =  s.midRef(4,2).toInt();//get seconds 120023
    int time_mm =  s.midRef(2,2).toInt();//get min 120023
    int time_ms = 0;
    if      (time_ss == 3 || time_ss == 18 || time_ss == 33 || time_ss == 48) time_ms = 75;
    else if (time_ss == 7 || time_ss == 22 || time_ss == 37 || time_ss == 52) time_ms = 50;
    else if (time_ss ==11 || time_ss == 26 || time_ss == 41 || time_ss == 56) time_ms = 25;
    int time_p = (time_mm*6000)+(time_ss*100)+time_ms;
    time_p = time_p % (375*2);
    if (time_p<375) res = 0;
    else res = 1;
    /*int mm = s.midRef(2, 2).toInt();
    int ss = s.midRef(4, 2).toInt(); // 2. Map the specific second offsets to 250ms units (0, 1, 2, or 3)
    int quarter_units = 0;
    if      (ss % 15 == 3)  quarter_units = 3; // 0.75s -> 3 units
    else if (ss % 15 == 7)  quarter_units = 2; // 0.50s -> 2 units
    else if (ss % 15 == 11) quarter_units = 1; // 0.25s -> 1 unit
    // 3. Calculate total units (1 second = 4 units)
    // Total period (7.5s) = 30 units. Half period (3.75s) = 15 units.
    long total_units = (mm * 60 * 4) + (ss * 4) + quarter_units;
    // 4. Determine phase (Even/Odd 3.75s blocks)
    res = (total_units / 15) % 2;*/
    /*int ss = s.midRef(4,2).toInt();
    if      (ss % 15 == 3)  res = 1; // 0.75s -> 3 units
    else if (ss % 15 == 7)  res = 0; // 0.50s -> 2 units
    else if (ss % 15 == 11) res = 1; // 0.25s -> 1 unit*/
    return res;
}
void DecoderFt2::sync_dt_df(double complex *cd2,int iseg,int &idfbest,int &ibest,double &smax,double &smax1)
{
    idfbest = 0;
    ibest = -1;
    smax=-99.0;
    smax1=-99.0;
    for (int isync = 1; isync <= 2; ++isync)
    {
        int idfmin,idfmax,idfstp,ibmin,ibmax,ibstp;
        ibmin = -688;//108;
        ibmax = 2024;//565;
        if (isync==1)
        {
            idfmin=-12;//12
            idfmax=12; //12
            idfstp=3;  //3
            //ibmin=-344;
            //ibmax=1012;
            if (iseg==1)
            {
                ibmin=216;//108;
                ibmax=1120;//565;//560
            }
            else if (iseg==2)
            {
                smax1=smax;
                ibmin=1120;//555;//560
                ibmax=2024;//1012;
            }
            else if (iseg==3)
            {
                ibmin=-688;//-344;
                ibmax=216;//118;//108
            }
            ibstp=4;
        }
        else
        {
            idfmin=idfbest-4;//-4
            idfmax=idfbest+4;//4
            idfstp=1;
            ibmin=ibest-5;//ibmin=fmax(0,ibest-5);
            ibmax=ibest+5;//ibmax=fmin(ibest+5,NDMAX/NDOWN-1);
            ibstp=1;
        }
        //qDebug()<<"m/m"<<isync<<idfmin<<idfmax<<idfbest;
        ibest=-1;
        smax=-99.0;
        idfbest=0;
        for (int idf = idfmin; idf <= idfmax; idf+=idfstp)
        {//do idf=idfmin,idfmax,idfstp
            for (int istart = ibmin; istart <= ibmax; istart+=ibstp)
            {//do istart=ibmin,ibmax,ibstp
                double sync=-99.0;
                sync2d(cd2,istart,ctwk2_ft2_[idf+20],1,sync);//20  sync4d(cd2,istart,ctwk2(:,idf),1,sync)  //!Find sync power
                if (sync>smax) //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
                {
                    smax=sync;
                    ibest=istart;//+ibstp;
                    idfbest=idf;//+idfstp;
                }
            }
        } //qDebug()<<"iseg"<<smax<<iseg<<f0<<f0+idfbest<<ibest;
    }
    if (iseg==1) smax1=smax;
}
double DecoderFt2::get_xibest(double complex *cd2,int ibest,int idfbest,double smax)
{
    const int NMAX=45000;
    const int NDOWN=9;//ft4=18;
    const int NDMAX=NMAX/NDOWN;
    double xibest=0.0;
    double sm1_sub, sp1_sub, den_sub;
    if (ibest>0 && ibest<NDMAX-1)
    {
        sync2d(cd2,ibest-1,ctwk2_ft2_[idfbest+20],1,sm1_sub);
        sync2d(cd2,ibest+1,ctwk2_ft2_[idfbest+20],1,sp1_sub);
        den_sub = sm1_sub - 2.0*smax + sp1_sub;
        if (fabs(den_sub) > 1.0e-6)//xibest = (double)ibest + 0.5*(sm1_sub - sp1_sub)/den_sub;
        {
            double delta=0.5*(sm1_sub - sp1_sub)/den_sub;
            delta=fmax(-1.0,fmin(1.0,delta));
            xibest=(double)ibest + delta;
        }
        else xibest = (double)ibest;
    }
    else xibest = (double)ibest;
    return xibest;
}
//////////AP7//////////////////
void DecoderFt2::ft2_a7_save(QString nutc,double dt,double f,QString msg)
{
    int nwords = 0;
    QString w[19+8]; //qDebug()<<"msg="<<msg;
    if (msg.indexOf("/")>-1 || msg.indexOf("<")>-1) return;
    msg.append(" ");//for any case for label->c5
    for (int z = 0; z<19; ++z) w[z]="             ";//13 blinks char
    TGenFt2->split77(msg,nwords,w);
    if (nwords<1) return;
    if (w[0].mid(0,3)=="CQ_") return;//???
    int j=ft2_even_odd(nutc);
    jseq_a7=j;
    //Add this decode to current table for this sequence
    //Number of decodes in this sequence
    int i=ndec[1][j];
    dt0[1][j][i]=dt;
    f0[1][j][i]=f;
    msg0[1][j][i]=w[0].trimmed()+" "+w[1].trimmed();
    QString s = w[1].trimmed();
    if (w[0].mid(0,3)=="CQ " && s.count()<=2)
    {
        msg0[1][j][i]="CQ "+w[1].trimmed()+" "+w[2].trimmed();//Save "CQ DX Call_2" ???
    }
    //QString msg1=msg0[1][j][i];//msg1=msg0(i,j,1)
    //nn=len(trim(msg1))
    //Include grid as part of message
    if (pomFt.isgrid4(w[nwords-1])) msg0[1][j][i]=msg0[1][j][i]+" "+w[nwords-1].trimmed();
    //If a transmission at this frequency with message fragment "call_1 call_2"
    //was decoded in the previous sequence, flag it as "DO NOT USE" because
    //we have already decoded and subtracted that station's next transmission.
    s = msg0[1][j][i]+"   ";//for any case for label->c5
    for (int z = 0; z<19; ++z) w[z]="             ";//13 blinks char
    TGenFt2->split77(s,nwords,w);
    for (int z = 0; z<ndec[0][j]; ++z)
    {	//hv protect from dupe decode in same period
        if (f0[0][j][z]<=-98.0) continue;
        int i2=msg0[0][j][z].indexOf(" "+w[1].trimmed());
        if (fabs(f-f0[0][j][z])<=3.0 && i2>=2) f0[0][j][z]=-98.0;//Flag as "do not use" for a potential a7 decode
    }
    if (i>MAXDEC-2) return;
    ndec[1][j]++; //qDebug()<<"Save="<<j<<msg<<ndec[1][j];
}
void DecoderFt2::ft2_a7d(double *dd0,bool &dobigfft,QString call_1,QString call_2,QString grid4,
                         double &xdt,double &f0,double xbase,int &nharderrors,double &dmin,QString &message,double &xsnr)
{
    const int ndepth = 3;
    const int MAXMSG=158;//= -26 to +49
    const int ND=87;
    const int NS=16;
    const int NMAX=45000;
    const int NDOWN=9;//ft4=18;
    const int NDMAX=NMAX/NDOWN;
    const int NN=NS+ND;
    const int NSPS=288;
    const int NSS=NSPS/NDOWN;//=32
    const int c_cb = NDMAX;
    const int c_cd2 = NDMAX;
    const int c_cd = NN*NSS;
    double complex cd2[NDMAX+100];
    double complex cb[NDMAX+100];
    double complex cd[c_cd+500];
    double sum2=0.0;
    bool badsync = false;
    //double sm1_sub, sp1_sub, den_sub;
    //bool hbits[220];
    double s4_[NN+10][4];
    double bitmetrics_[3][220];//NN*2
    double llra[174];
    double llrb[174];
    double llrc[174];
    double llrd[174];
    int itone[NN+5];//NN
    bool hdec[178];
    bool nxor[178];
    double dmm[MAXMSG+50];
    QString msgbest = "";
    QString msgsent = "";
    double pbest;
    double xibest;
    bool std_1,std_2;

    //call_1="CQ"; call_2="LZ444HV"; grid4="    ";
    std_1 = pomAll.isStandardCall(call_1.trimmed());
    if (call_1=="CQ") std_1=true;
    std_2 = pomAll.isStandardCall(call_2.trimmed());
    //qDebug()<<"  IN================="<<call_1<<call_2<<grid4<<"std"<<std_1<<std_2;
    nharderrors=-1;
    ft2_downsample(dd0,dobigfft,f0,cd2);  //!Downsample to 32 Sam/Sym
    if (dobigfft) dobigfft=false;
    for (int i = 0; i < c_cd2; ++i) sum2+=creal(cd2[i]*conj(cd2[i]));//???
    sum2=sum2/((double)NMAX/(double)NDOWN);//sum2=sum(cd2*conjg(cd2))/(real(NZZ)/real(NDOWN))
    if (sum2>0.0)//if(sum2.gt.0.0) cd2=cd2/sqrt(sum2)
    {
        for (int i = 0; i < c_cd2; ++i) cd2[i]=cd2[i]/sqrt(sum2);
    }
    for (int iseg = 1; iseg <= 3; ++iseg)//! DT search is done over 3 segments
    {//do iseg=1,3
        int idfbest = 0;
        int ibest = -1;
        double smax=-99.0;
        double smax1=-99.0;
        sync_dt_df(cd2,iseg,idfbest,ibest,smax,smax1);
        //if (smax<0.9) continue;//stop 038
        double smaxthresh=0.60;//0.80;//0.9;//038
        if (ndepth>=3) smaxthresh=0.50;//0.65;//0.72;//0.65;//0.75;//038
        //if (isp>=2) smaxthresh=smaxthresh*0.88; //! pass 2: ~15%
        //if (isp>=3) smaxthresh=smaxthresh*0.76; //! pass 3+: ~25%
        if (smax<smaxthresh) continue;//038
        if (iseg>1 && smax<smax1) continue;
        double f1=f0+(double)idfbest; //qDebug()<<idfbest<<f1;
        if ( f1<=10.0 || f1>=4990.0 ) continue;//cycle
        ft2_downsample(dd0,dobigfft,f1,cb); //!Final downsample, corrected f1
        sum2=0.0;
        for (int i = 0; i < c_cb; ++i) sum2+=cabs(cb[i])*cabs(cb[i]);//sum2=sum(abs(cb)**2)/(real(NSS)*NN)
        sum2 = sum2/(double)(NSS*NN);
        if (sum2>0.0)
        {
            for (int i = 0; i < c_cb; ++i) cb[i]=cb[i]/sqrt(sum2);
        }
        //????????????????????????????
        //qDebug()<<"kkk="<<ibest<<f1;
        //const int c_cb = NDMAX;//=4032
        //const int c_cd = NN*NSS;//=3296
        for (int i = 0; i < c_cd+5; ++i) cd[i]=0.0+0.0*I;//cd=0. //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
        if (ibest>=0)
        {
            int it=fmin(NDMAX-1,ibest+NN*NSS-1);
            int np=it-ibest+1;
            for (int i = 0; i < np; ++i) cd[i]=cb[i+ibest];//cd(0:np-1)=cb(ibest:it)
        }
        else
        {
            for (int i = 0; i < (NN*NSS+2*ibest); ++i)//cd(-ibest:ibest+NN*NSS-1)=cb(0:NN*NSS+2*ibest-1)
            {
                if (i-ibest>=0) cd[i-ibest]=cb[i];//for any case array out of bounds
            }
        } //qDebug()<<fmin(NDMAX-1,ibest+NN*NSS-1)-ibest+1<<NN*NSS+2*ibest<<c_cd<<c_cb<<f1;
        get_ft2_bitmetrics(cd,bitmetrics_,badsync,s4_);//get_ft2a7_bitmetrics(cd,bitmetrics_,badsync,s4_);//
        if (badsync) continue;

        int nsync_qual = pomFt.SyncQualFt2_4(bitmetrics_[0]);//if (nsync_qual<15) continue;//stop 038
        int nsync_qual_min=13;//15;//038
        if (ndepth>=3) nsync_qual_min=10;//12;//10;//12;//038
        if (nsync_qual<nsync_qual_min) continue;//038

        //! Sub-sample DT refinement via 3-point parabolic interpolation
        //! Improves DT accuracy from ±0.75ms (1 sample) to ±0.1ms
        xibest = get_xibest(cd2,ibest,idfbest,smax);

        double scalefac=2.83;  //llr[174]; =2*ND
        for (int x = 0; x < 58; ++x)
        {
            llra[x]=bitmetrics_[0][x+8];
            llra[x+58]=bitmetrics_[0][x+74];
            llra[x+116]=bitmetrics_[0][x+140];
            llrb[x]=bitmetrics_[1][x+8];
            llrb[x+58]=bitmetrics_[1][x+74];
            llrb[x+116]=bitmetrics_[1][x+140];
            llrc[x]=bitmetrics_[2][x+8];
            llrc[x+58]=bitmetrics_[2][x+74];
            llrc[x+116]=bitmetrics_[2][x+140];
        }
        double maxval_llra_abs = 0.0;
        for (int x = 0; x < 2*ND; ++x)//llr[174]; =2*ND
        {
            llra[x]=scalefac*llra[x];
            llrb[x]=scalefac*llrb[x];
            llrc[x]=scalefac*llrc[x];
            double llra_abs = fabs(llra[x]);
            if (llra_abs>maxval_llra_abs) maxval_llra_abs=llra_abs;
        }
        //double apmag = maxval_llra_abs*1.1;//apmag=maxval(abs(llra))*1.1
        for (int i = 0; i < 2*ND; ++i)//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
        {//do i=1,2*ND
            if (fabs(llra[i])>=fabs(llrb[i]) && fabs(llra[i])>=fabs(llrc[i])) llrd[i]=llra[i];
            else if (fabs(llrb[i])>=fabs(llrc[i])) llrd[i]=llrb[i];
            else llrd[i]=llrc[i]; //llre[i]=(llra[i]+llrb[i]+llrc[i])/3.0;
        }
        bool c77[140];
        bool cw[180];
        pbest=0.0;
        dmin=1.e30;
        int count_msg = MAXMSG;
        for (int i = 0; i < MAXMSG; ++i)
        {
            QString msg;
            if (pomFt.SetAp7Msg(call_1,std_1,call_2,std_2,grid4,i,msg,count_msg)) break;

            int i3=0;
            int n3=0;
            for (int z= 0; z < 176; ++z)
            {
                if (z<100) c77[z]=0;
                cw[z] = 0;
            }
            TGenFt2->pack77(msg,i3,n3,c77);
            TGenFt2->make_c77_i4tone_codeword(c77,itone,cw);
            if (msg.startsWith("QU1RK ")) msgsent = msg;
            else
            {
                bool unpk77_success = false;
                msgsent = "";
                msgsent = TGenFt2->unpack77(c77,unpk77_success);
            }
            //if (msgsent!=msg.trimmed()) qDebug()<<"---------------"<<i<<msg<<msgsent; //no-std-call problem  "CQ <TM22KPW> R-26"
            //if (msgsent.isEmpty()) msgsent = "QU1RK ";
            double pow0=0.0;
            for (int z = 0; z < NN; ++z)
            {
                double s88 = s4_[z][itone[z]]*0.001;//1000.0; //s8_/1000.0 = s2_  HV from v1.
                pow0+=(s88*s88);
            }

            double da = 0.0;
            double dbb= 0.0;
            double dc = 0.0;
            double dd = 0.0;
            for (int z= 0; z < 174; ++z)
            {
                hdec[z] = 0;
                if (llra[z]>=0.0) hdec[z] = 1;
                nxor[z]=hdec[z] ^ cw[z];
                da+=(double)nxor[z]*fabs(llra[z]);
                hdec[z] = 0;
                if (llrb[z]>=0.0) hdec[z] = 1;
                nxor[z]=hdec[z] ^ cw[z];
                dbb+=(double)nxor[z]*fabs(llrb[z]);
                hdec[z] = 0;
                if (llrc[z]>=0.0) hdec[z] = 1;
                nxor[z]=hdec[z] ^ cw[z];
                dc+=(double)nxor[z]*fabs(llrc[z]);
                hdec[z] = 0;
                if (llrd[z]>=0.0) hdec[z] = 1;
                nxor[z]=hdec[z] ^ cw[z];
                dd+=(double)nxor[z]*fabs(llrd[z]);
            }

            double dm = da;
            if (dbb<dm) dm=dbb;
            if (dc<dm)  dm=dc;
            if (dd<dm)  dm=dd;
            dmm[i]=dm;
            if (dm<dmin)
            {
                dmin=dm;
                msgbest=msgsent;
                pbest=pow0;
                nharderrors = -1;
                if (dm==da)
                {
                    for (int z= 0; z < 174; ++z)
                    {
                        if ((double)(2*cw[z]-1)*llra[z]<0.0) nharderrors++;
                    }
                }
                else if (dm==dbb)
                {
                    for (int z= 0; z < 174; ++z)
                    {
                        if ((double)(2*cw[z]-1)*llrb[z]<0.0) nharderrors++;
                    }
                }
                else if (dm==dc)
                {
                    for (int z= 0; z < 174; ++z)
                    {
                        if ((double)(2*cw[z]-1)*llrc[z]<0.0) nharderrors++;
                    }
                }
                else if (dm==dd)
                {
                    for (int z= 0; z < 174; ++z)
                    {
                        if ((double)(2*cw[z]-1)*llrd[z]<0.0) nharderrors++;
                    }
                }
            }
            //pomFt.TryDecAp7(llra,llrb,llrc,llrd,cw,dmm,i,msgsent,pow0,dmin,msgbest,pbest,nharderrors);
        }
        int pos = 0;
        double min = dmm[0];
        for (int z= 1; z < count_msg; ++z)//MAXMSG
        {
            if (dmm[z]<min)
            {
                min=dmm[z];
                pos = z;
            }
        }
        dmm[pos] = 1.e30;
        pos = 0;
        min = dmm[0];
        for (int z= 1; z < count_msg; ++z)//MAXMSG
        {
            if (dmm[z]<min)
            {
                min=dmm[z];
                pos = z;
            }
        }
        double dmin2 = dmm[pos];
        message=msgbest; //qDebug()<<"         DEC AP7===="<<xdt<<f1<<message<<dmin<<dmin2<<nharderrors;
        msgbest += "xxxxxx";//2.70 protection
        if (dmin==0.0) dmin=0.0001;//devide by zero
        if (dmin>100.0 || dmin2/dmin<1.2) nharderrors=-1;//1.27
        if (msgbest.mid(0,3)=="CQ " && std_2 && grid4=="    ") nharderrors=-1;
        if (msgbest.mid(0,6)=="QU1RK " || message.isEmpty()) nharderrors=-1;
        if (nharderrors>95) nharderrors=-1;
        if (nharderrors>=0)
        {
            if (xbase<=0.0) xbase=0.001;
            //double arg=pbest/xbase/3.0e6 - 1.0;
            //if (arg>0.0) xsnr=fmax(-21.0,pomAll.db(arg)-22.0);
            //if (xbase<=0.0) xbase=0.001;
            xsnr=pomAll.db(pbest/xbase - 1.0) - 42.0;
            if (xsnr < -21.0) xsnr=-21.0;
            if (xsnr > 49.0)  xsnr=49.0; //double dt0=xibest/1333.33;
            xdt=xibest/1333.33 - 0.5;
            f0=f1;
            break;
        }
    }
}
//////////end AP7//////////////////
void DecoderFt2::PrintMsg(QString tmm,int nsnr,double xdt,double f1,QString message,int iaptype,
                          bool do_avg,float qual,bool &have_dec,bool &f_only_one_color,bool fshow)
{
    if (fshow)
    {
        if (f_only_one_color)
        {
            f_only_one_color = false;
            emit EmitBackColor();
        }
        if (qual<0.0001) qual=0.0;//no show -0.0
        QString str_iaptype = "";
        if (!do_avg)
        {
            if (qual<0.17) str_iaptype = "? ";
            if (iaptype!=0) str_iaptype.append("AP");
            str_iaptype.append(QString("%1").arg(iaptype));
        }
        else str_iaptype = "Avg";

        int df_hv = f1-s_nftx2;//2.12
        QString sdtx = QString("%1").arg(xdt,0,'f',1);
        if (sdtx=="-0.0") sdtx="0.0";//2.08 remove -0.0 exception

        QStringList list;
        list <<tmm<<QString("%1").arg(nsnr)
        <<sdtx<<QString("%1").arg((int)df_hv)
        <<message<<str_iaptype
        <<QString("%1").arg(qual,0,'f',1)
        <<QString("%1").arg((int)f1);
        emit EmitDecodetTextFt(list);//1.27 psk rep   fopen bool true    false no file open
        have_dec = true;
        if (do_avg) ft2_clravg(jseq_a);
    }
    if (!s_fopen2 && message.count()>2) ft2_a7_save(s_time2,xdt,f1,message);
}
//#define DEB_FALSEDEC
uint8_t DecoderFt2::isFalseDecode(QString msg,int nsnr,double qual,bool do_avg,int nharderror,int i3,int n3)
{
    if (msg.count()<1) return 1;
    if (msg.indexOf("<> ")>-1 || msg.indexOf(" <>")>-1 || msg.startsWith("CQ_001 ")) return 2;
    int harde = 35;
    if (nsnr<-19 && qual<0.08) harde = 18;//0.06
    if (!do_avg && nharderror>harde)
    {
        if (nsnr<-17 && qual<0.09) return 3;//0.07
        if (nsnr<-19 && qual<0.17) return 4;//34
    }
    harde = 36;
    if (!do_avg && nsnr<-19 && qual<0.16) harde = 19;
    if ((i3==2 || i3==3 || i3==4) && nsnr<-18 && qual<0.17 && nharderror>harde) return 5;
    harde = 34;
    if (!do_avg && nsnr<-18 && qual<0.2) harde = 24;
    if (nsnr<-12 && qual<0.55 && msg.indexOf('/')>-1 && nharderror>harde) return 6;
    harde = 34;//30
    if (!do_avg && nsnr<-18 && qual<0.43) harde = 19;//-18
    if (nsnr<-14 && qual<0.45 && (msg.indexOf("/P")>-1 || msg.indexOf("/R")>-1) && nharderror>harde) return 7;
    if (nsnr<-13 && qual<0.50 && msg.indexOf(' ')<0 && nharderror>19) return 8;
    if (nsnr<-13 && qual<0.55 && msg.startsWith("TU; ") && nharderror>18) return 9;
    harde = 18;
    if (nsnr<-18 && qual<0.4) harde = 15;//i3=5 && (n3==5 || n3==7)//5=R+N+Loc 7=N+Loc
    if (nsnr<-10 && qual<0.55 && msg.startsWith("<...> <...> ") && nharderror>harde) return 10;
    if (nsnr<-13 && qual<0.6)
    {
        if ((msg.indexOf(" <...> ")>-1 || msg.startsWith("<...> ") || msg.endsWith(" <...>")) && nharderror>18) return 11;
        harde = 36;
        if (!do_avg && nsnr<-16 && qual<0.55 && (nsnr<-18 || qual<0.17)) harde = 19;
        //if (i3==0 && (n3==3 || n3==4) && (nsnr<-18 || qual<0.2) && nharderror>harde) return 12;//0.3 0.4 WA9XYZ KA1ABC R 16A EMA //ARRL Field Day
        if ((nsnr<-18 || qual<0.3) && msg.indexOf(" R ")>-1 && nharderror>harde) return 12;
        if ((nsnr<-18 || qual<0.3) && msg.count()>5)
        {
            if (msg.at(msg.count()-5)==' ')
            {
                bool fend4D = true;
                for (int i=msg.count()-4; i<msg.count(); ++i)
                {
                    if (msg.at(i).isLetter())
                    {
                        fend4D = false;
                        break;
                    }
                }
                if (fend4D && nharderror>harde) return 13;
            }
        }
        if (!do_avg && nsnr<-18 && msg.startsWith("CQ "))//CQ RY4UY4T6G
        {
            QString s = msg.mid(3,msg.count()-3);
            if (s.indexOf(' ')<0 && nharderror>harde) return 14;
        }//(n3==3 || n3==4) DEC Snr=-20 Qual=0.2450 Msg=1W5DTH 264HKO 21E KS Avg=0 nhrr=28 i3=0 n3=4 iap=0
        if (i3==0 && n3==4 && nsnr<-19 && nharderror>harde) return 15;
    }
    return 0;
}
void DecoderFt2::make_bm_ap(double *dd,double bitmetrics_[3][220],int nQSOProgress,bool lapcqonly,
                            int ndepth,int cont_type,double f1,double snr,double nfqso,bool doosd,
                            bool dosubtract,double xibest,int &ndecodes,QString *decodes,int maxcand,
                            int &nharderror,bool &f_only_one_color,bool &have_dec,bool do_avg)
{
    const int ND=87;
    //const int NS=16;
    //const int NN=NS+ND;
    double llr [180];//(2*ND)=174
    double llra[180];//(2*ND)=174
    double llrb[180];//(2*ND)=174
    double llrc[180];//(2*ND)=174
    double llrd[180];//(2*ND)=174
    double llre[180];
    bool apmask[174];//(2*ND)=174
    int iaptype=0;
    bool cw[2*ND+20];//174
    bool message91[140];
    const bool rvec[77]=
        {
            0,1,0,0,1,0,1,0,0,1,0,1,1,1,1,0,1,0,0,0,1,0,0,1,1,0,1,1,0,
            1,0,0,1,0,1,1,0,0,0,0,1,0,0,0,1,0,1,0,0,1,1,1,1,0,0,1,0,1,
            0,1,0,1,0,1,1,0,1,1,1,1,1,0,0,0,1,0,1
        };
    const int nappasses_2[6]=
        {
            //3,3,3,3,3,4
            3,3,3,4,4,4//038
        };
    const int naptypes_2[6][4]=
        {
            //{1,2,0,0},{2,3,0,0},{2,3,0,0},{3,6,0,0},{3,6,0,0},{3,1,2,0}
            {1,2,0,0},{2,3,0,0},{2,3,0,0},{3,4,5,6},{3,4,5,6},{3,1,2,0}//038
        };
    double scalefac=2.83;  //llr[174]; =2*ND
    for (int x = 0; x < 58; ++x)
    {
        llra[x]=bitmetrics_[0][x+8];
        llra[x+58]=bitmetrics_[0][x+74];
        llra[x+116]=bitmetrics_[0][x+140];
        llrb[x]=bitmetrics_[1][x+8];
        llrb[x+58]=bitmetrics_[1][x+74];
        llrb[x+116]=bitmetrics_[1][x+140];
        llrc[x]=bitmetrics_[2][x+8];
        llrc[x+58]=bitmetrics_[2][x+74];
        llrc[x+116]=bitmetrics_[2][x+140];
    }
    double maxval_llra_abs = 0.0;
    for (int x = 0; x < 2*ND; ++x)//llr[174]; =2*ND
    {
        llra[x]=scalefac*llra[x];
        llrb[x]=scalefac*llrb[x];
        llrc[x]=scalefac*llrc[x];

        double llra_abs = fabs(llra[x]);
        if (llra_abs>maxval_llra_abs) maxval_llra_abs=llra_abs;
    }
    double apmag = maxval_llra_abs*1.1;//apmag=maxval(abs(llra))*1.1

    for (int i = 0; i < 2*ND; ++i)//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    {//do i=1,2*ND
        if (fabs(llra[i])>=fabs(llrb[i]) && fabs(llra[i])>=fabs(llrc[i])) llrd[i]=llra[i];
        else if (fabs(llrb[i])>=fabs(llrc[i])) llrd[i]=llrb[i];
        else llrd[i]=llrc[i];
        llre[i]=(llra[i]+llrb[i]+llrc[i])/3.0;
    }
    int npasses=5+nappasses_2[nQSOProgress];//npasses=3+nappasses(nQSOProgress)
    if (lapcqonly) npasses=6;//??? new if(lapcqonly) npasses=4
    if (ndepth==1) npasses=5;
    //hv=5 new  if(ncontest.ge.6) npasses=3 ! Don't support Fox and Hound
    if (cont_type>=5) npasses=5;//5;//contest max=4

    //int nharderror = -1;
    for (int ipass = 1; ipass <= npasses; ++ipass)
    {//do ipass=1,npasses
        for (int z = 0; z < 2*ND; ++z)
        {
            if 		(ipass==1) llr[z]=llra[z];//if(ipass.eq.2) llr=llr1
            else if (ipass==2) llr[z]=llrb[z];
            else if (ipass==3) llr[z]=llrc[z];//llr=llr0
            else if (ipass==4) llr[z]=llrd[z];//llr=llr0
            else if (ipass==5) llr[z]=llre[z];//llr=llr0
        }
        if (ipass<=5)
        {
            for (int z = 0; z < 2*ND; ++z) apmask[z]=0;
            iaptype=0;
        }
        double napwid=80.0; //2.39 old double napwid=75.0;
        if (ipass > 5)
        {
            //for (int z = 0; z < 2*ND; ++z) llrd[z]=llrc[z]; //2.39 old llrd[z]=llra[z];
            //iaptype=naptypes(nQSOProgress,ipass-3)
            iaptype=naptypes_2[nQSOProgress][ipass-(5+1)];// v1 4+1
            if (lapcqonly) iaptype=1;

            // Activity Type                id	type	dec-id       dec-type	dec-cq
            //"Standard"					0	0		0 = CQ		 0			0
            //"EU RSQ And Serial Number"	1	NONE	1  NONE		 NONE		NONE
            //"NA VHF Contest"				2	2		2  CQ TEST	 1			3 = CQ TEST
            //"EU VHF Contest"				3 	3		3  CQ TEST	 2			3 = CQ TEST
            //"ARRL Field Day"				4	4		4  CQ FD	 3			2 = CQ FD
            //"ARRL Inter. Digital Contest"	5	2		5  CQ TEST   1 			3 = CQ TEST
            //"WW Digi DX Contest"			6	2		6  CQ WW	 1			4 = CQ WW
            //"FT4 DX Contest"				7	2		7  CQ WW	 1			4 = CQ WW
            //"FT8 DX Contest"				8	2		8  CQ WW	 1			4 = CQ WW
            //"FT Roundup Contest"			9	5		9  CQ RU	 4			1 = CQ RU
            //"Bucuresti Digital Contest"	10 	5		10 CQ BU 	 4			5 = CQ BU
            //"FT4 SPRINT Fast Training"	11 	5		11 CQ FT 	 4			6 = CQ FT
            //"PRO DIGI Contest"			12  5		12 CQ PDC 	 4			7 = CQ PDC
            //"CQ WW VHF Contest"			13	2		13 CQ TEST	 1			3 = CQ TEST
            //"Q65 Pileup" or "Pileup"		14	2		14 CQ 		 1			0 = CQ
            //"NCCC Sprint"					15	2		15 CQ NCCC	 1			8 = CQ NCCC
            //"ARRL Inter. EME Contest"		16	6		16 CQ 		 0			0 = CQ
            //"FT Challenge"				17  6       17 CQ FTC    0          9 = CQ FTC

            //double napwid=60.0; //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
            //hv new =4 qDebug()<<"iaptype==="<<iaptype;
            if (cont_type<=4 && iaptype>=3 && fabs(f1-nfqso)>napwid) continue;
            //if(iaptype.ge.2 .and. apbits(1).gt.1) cycle  ! No, or nonstandard, mycall
            //if(iaptype.ge.3 .and. apbits(30).gt.1) cycle ! No, or nonstandard, dxcall
            if (iaptype>=2 && apbits_ft2[0]>1) continue;
            if (iaptype>=3 && apbits_ft2[29]>1) continue;
            pomFt.SetApFt2_4(llr,llrc,apmask,apbits_ft2,mcq_ft2,apmy_ru_ft2,aphis_fd_ft2,apmag,iaptype,cont_type);
        }

        for (int z = 0; z < 174; ++z)
        {
            cw[z]=0;
            if (z<120) message91[z]=0;
        }
        double dmin=0.0;

        int ndeep=2;//2
        int maxosd=3;//2-3
        //if(ndepth>=3)//???
        if (fabs(nfqso-f1)<=napwid)
        {
            ndeep=2;//2
            maxosd=4;//3-4
        } //printf(" OSD ndeep=%d maxosd=%d\n",ndeep,maxosd);
        /*int ndeep=3;//038
        int maxosd=3;
        if (fabs(nfqso-f1)<=napwid && ndepth>=3) maxosd=4;*/

        if (!doosd) maxosd = -1;
        //not used for the moment -> pomFt.decode174_91_ft2(llr,maxosd,ndeep,apmask,message91,cw,nharderror,dmin);
        pomFt.decode174_91(llr,maxosd,ndeep,apmask,message91,cw,nharderror,dmin);//Keff,ntype,

        int c_m77 = 0;
        for (int z = 0; z < 77; z++) c_m77 +=message91[z];
        if (c_m77==0) continue;

        if ( nharderror>=0 )
        {
            for (int z = 0; z < 77; z++) message91[z]=fmod(message91[z]+rvec[z],2); //! remove rvec scrambling
            bool unpk77_success=false;
            QString message = TGenFt2->unpack77(message91,unpk77_success);
            if (!unpk77_success) break;
            int n3=4*message91[71] + 2*message91[72] + message91[73];
            int i3=4*message91[74] + 2*message91[75] + message91[76];
            if (!do_avg)
            {
                if (unpk77_success && dosubtract)
                {
                    int i4tone[120];
                    TGenFt2->make_c77_i4tone(message91,i4tone);
                    double dt=xibest/1333.33;//double dt=(double)ibest/1333.33;//666.67;//750.0;
                    subtractft2(dd,i4tone,f1,dt); //qDebug()<<"subtractft4"<<message<<dt<<f1<<ipass<<i3;
                }
                bool ldupe=false;
                for (int z = 0; z < ndecodes; z++)
                {
                    if (decodes[z]==message)
                    {
                        ldupe=true;
                        break;
                    }
                }
                if (ldupe) break;// inportent for subtract
            }
            decodes[ndecodes]=message;
            if (ndecodes < (maxcand-1)) ndecodes++;
            int nsnr;
            float xdt;
            if (!do_avg)
            {
                double xsnr = 0.0;
                if (snr>0.0) xsnr=10*log10(snr)-13.8;
                else xsnr=-20.0;
                nsnr=(int)xsnr;//(fmax(-20.0,xsnr));
                if (nsnr > 49) nsnr = 49;
                if (nsnr <-20) nsnr =-20; //2.76.5 //if (do_avg) nsnr=-21;
                xdt=xibest/1333.33 - 0.5;//xdt=(float)ibest/1333.33 - 0.5;//666.67 - 0.5; //if (do_avg) xdt=ft2avg[jseq].dt_avg - 0.5;
            }
            else
            {
                nsnr=-21;
                xdt=ft2avg[jseq_a].dt_avg - 0.5;
            }
            float qual=1.0-((float)nharderror+(float)dmin)/60.0;
            if (qual<0.0005) qual=0.0;//no show -0.0

            uint8_t false_dec = isFalseDecode(message,nsnr,qual,do_avg,nharderror,i3,n3);
            if (false_dec>0)
            {
#if defined DEB_FALSEDEC
                printf("->Falsh=%d Snr=%d Qual=%.4f FalseDec=%s Avg=%d nhrr=%d i3=%d n3=%d iap=%d\n",false_dec,nsnr,qual,qPrintable(message),do_avg,nharderror,i3,n3,iaptype);
#endif
                break;
            }
#if defined DEB_FALSEDEC
            if (nsnr<-13 && qual<0.6) printf("    DEC Snr=%d Qual=%.4f Msg=%s Avg=%d nhrr=%d i3=%d n3=%d iap=%d\n",nsnr,qual,qPrintable(message),do_avg,nharderror,i3,n3,iaptype);
#endif
            PrintMsg(s_time2,nsnr,xdt,f1,message,iaptype,do_avg,qual,have_dec,f_only_one_color,true);
            break;//exit
        }
    }  //!Sequence estimation
}
void DecoderFt2::ft2_decode(double *dd,double f0a,double f0b,double f0a1,double f0b1,double fqso,bool &have_dec)//,int /*npts no need*/)
{
    have_dec = false;
    const int NDOWN=9;//18;                 //!Downsample factor
    const int NSPS=288;
    const int NSS=NSPS/NDOWN;//=32
    const int ND=87;
    const int NMAX=45000;//!Samples in iwave (3.75*12000) 21*1728;//36288  =72576
    const int NDMAX=NMAX/NDOWN;//=
    const int NS=16;//<-?????
    const int NN=NS+ND;    //=103
    const int maxcand=200;//2.70 array max=180 old=100
    const int c_cb = NDMAX;//=
    const int c_cd = NN*NSS;//=
    const int c_cd2 = NDMAX;//=
    double fs=DEC_SAMPLE_RATE/(double)NDOWN;                //!Sample rate after downsampling
    //double dt=1.0/fs;                         //!Sample interval after downsample (s)
    //tt=NSPS*dt                      //!Duration of "itone" symbols (s)
    //txt=NZ*dt                       //!Transmission length (s) without ramp up/down
    //h=1.0
    double a[5];
    double complex ctwk[2*NSS];
    //double complex ctwk2_[33][2*NSS]; //ctwk2(2*NSS,-16:16) 2*32=64
    const bool rvec[77]=
        {
            0,1,0,0,1,0,1,0,0,1,0,1,1,1,1,0,1,0,0,0,1,0,0,1,1,0,1,1,0,
            1,0,0,1,0,1,1,0,0,0,0,1,0,0,0,1,0,1,0,0,1,1,1,1,0,0,1,0,1,
            0,1,0,1,0,1,1,0,1,1,1,1,1,0,0,0,1,0,1
        };
    QString hiscall = s_HisCall2;
    QString mycall  = s_MyCall2;
    QString message;
    QString msgsent;
    bool c77[100];//77+10
    //bool message77[130];
    //bool message91[140];
    bool cw[2*ND+20];//174
    //double savg[NH1+10];
    //double sbase[NH1+10];
    double complex cd2[NDMAX+100];// pomAll.zero_double_comp_beg_end(cd2,0,NDMAX);
    double complex cb[NDMAX+100];
    double complex cd[4000];//(103*32-1)=3295   //!Complex waveform
    //bool hbits[220];//(2*NN)=206
    double s4_[120][4];
    /*double llr [180];//(2*ND)=174
    double llra[180];//(2*ND)=174
    double llrb[180];//(2*ND)=174
    double llrc[180];//(2*ND)=174
    double llrd[180];//(2*ND)=174
    double llre[180];
    bool apmask[174];//(2*ND)=174
    int iaptype=0;*/
    bool f_only_one_color = true;
    double bitmetrics_[3][220];//real bitmetrics(2*NN,3)//2*NN = 206
    bool badsync = false;

    int ndecodes=0;
    QString decodes[maxcand+50];
    for (int i = 0; i < maxcand; ++i) decodes[i]=" ";

    //////////AP7//////////////////
    int jseqr = ft2_even_odd(s_time2);
    const int NFFT1=1152;//2304;
    const int NH1=NFFT1/2;
    const double df= 12000.0/(double)NFFT1;
    double sbase[NH1+10];
    if (nutc0=="-1")
    {
        for (int k = 0; k < 2; ++k)
        {
            for (int j = 0; j < 2; ++j)
            {
                for (int i = 0; i < MAXDEC; ++i)
                {
                    msg0[k][j][i]= "";
                    dt0[k][j][i] = 0.0;
                    f0[k][j][i]  = 0.0;
                }
            }
        }
        nutc0="000001";//2.70 crash if = -1
    }
    if (s_time2!=nutc0 && !s_fopen2)//2.66 for ap7 s_fopen8
    {
        if (ndec[1][jseq_a7]==0) c_zerop++; //2.66 HV add
        else c_zerop=0;

        int iz=ndec[1][jseq_a7];
        for (int i = 0; i < iz; ++i)
        {
            dt0[0][jseq_a7][i]  = dt0[1][jseq_a7][i];
            f0[0][jseq_a7][i]   = f0[1][jseq_a7][i];
            msg0[0][jseq_a7][i] = msg0[1][jseq_a7][i];
        }
        int t_ss1 =  s_time2.midRef(4,2).toInt();
        int t_mm1 =  s_time2.midRef(2,2).toInt();
        int t_hh1 =  s_time2.midRef(0,2).toInt();
        int t_p1  = (t_hh1*3600)+(t_mm1*60)+t_ss1;
        t_ss1 =  nutc0.midRef(4,2).toInt();
        t_mm1 =  nutc0.midRef(2,2).toInt();
        t_hh1 =  nutc0.midRef(0,2).toInt();
        int t_p0  = (t_hh1*3600)+(t_mm1*60)+t_ss1;
        if (t_p1-t_p0>11 || c_zerop>2)//2.66 HV add=3p   if (t_p1-t_p0>60 || c_zerop>3)//2.66 HV add=4p
        {
            //ft2 11 and 3 for 3 periods//15 and 4 for 4 periods ...
            //1. We lost 3 periods (no decode),= data is not actual, ap7 not permitted
            //2. We stopped APP for 3 periods, = data is not actual, ap7 not permitted
            ndec[0][0]=0;
            ndec[0][1]=0;
            //for (int i = 0; i < MAXDEC; ++i)// no needed
            //{
            //dt0[0][0][i]=0.0;
            //f0[0][0][i]=0.0;
            //dt0[0][1][i]=0.0;
            //f0[0][1][i]=0.0;
            //}
            c_zerop=0;
        }
        else
        {
            if (iz>0) ndec[0][jseq_a7]=iz;//2.66 HV correction
        }
        ndec[1][jseq_a7]=0;
        for (int i = 0; i < MAXDEC; ++i)// no needed
        {
            dt0[1][jseq_a7][i]=0.0;
            f0[1][jseq_a7][i]=0.0;
        }
        nutc0=s_time2;
    }
    //////////end AP7//////////////////

    //int ntol = G_DfTolerance;
    double nfqso=fqso;
    double nfqso2=fqso;//2.72
    double nfa=f0a;//100.0;              // mybe from waterfull scale
    double nfb=f0b;//3000.0;
    double nfa1=f0a1;//100.0;              // mybe from waterfull scale
    double nfb1=f0b1;//3000.0;
    bool nagain = false;//s_mousebutton = mousebutton; //mousebutton Left=1, Right=3 fullfile=0 rtd=2
    if (s_mousebutton2==3) nagain = true;

    int nQSOProgress = s_nQSOProgress2;
    int cont_type = 0;
    int cont_id   = 0;
    if (!f_multi_answer_mod2)
    {
        cont_type = s_ty_cont_ft2_28;
        cont_id   = s_id_cont_ft2_28;
    }
    //double nftx = s_nftx;
    //bool lapon = s_lapon;
    bool lapcqonly = false;//lapcqonly<-no used fom me. if no TX more then 10min use only AP1
    if (!s_lapon2) lapcqonly = true;// only in mshv

    if (first_ft2d || (cont_id!=cont_id0_ft2_2))
    {
        for (int idf = 0; idf < 41; ++idf)   //  41    33
        {//do idf=-16,16
            a[0]=(double)(idf-20);           //-+20    +-16
            a[1]=0.0;
            a[2]=0.0;
            a[3]=0.0;
            a[4]=0.0;
            for (int i = 0; i < 2*NSS; ++i) ctwk[i]=1.0;
            pomFt.twkfreq1(ctwk,2*NSS,fs/2.0,a,ctwk2_ft2_[idf]);//  ctwk2(:,idf)
        }
        //bool c77[100];
        if (cont_id!=0)
        {
            int i3=0;
            int n3=0;
            for (int i = 0; i < 78; ++i) c77[i]=0;
            TGenFt2->pack77(s_cont_ft2_cq+" LZ2HV KN23",i3,n3,c77);
        }
        for (int i = 0; i < 29; ++i)
        {
            if (cont_id==0) mcq_ft2[i]=2*fmod(mcq_ft[i]+rvec[i],2)-1;
            else mcq_ft2[i]=2*fmod(c77[i]+rvec[i],2)-1;
            if (i<19)
            {
                mrrr_ft2[i]=2*fmod(mrrr_ft[i]+rvec[i+58],2)-1;
                m73_ft2[i]=2*fmod(m73_ft[i]+rvec[i+58],2)-1;
                mrr73_ft2[i]=2*fmod(mrr73_ft[i]+rvec[i+58],2)-1;
            }
        }//qDebug()<<cont_id;
        mycall0_ft2="";
        hiscall0_ft2="";
        cont_id0_ft2_2 = cont_id;
        first_ft2d=false;
    }

    //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    mycall=mycall.trimmed();
    hiscall=hiscall.trimmed();
    if (mycall!=mycall0_ft2 || hiscall!=hiscall0_ft2)
    {
        //QString sss = "";
        for (int i = 0; i < 2*ND; ++i) apbits_ft2[i]=0;
        apbits_ft2[0]=99;
        apbits_ft2[29]=99;
        for (int i = 0; i < 28; ++i)
        {
            apmy_ru_ft2[i]=0;
            aphis_fd_ft2[i]=0;
        }
        bool nohiscall;
        int i3;
        int n3;
        bool unpk77_success;
        if (mycall.count()<3) goto c10;//if(len(trim(mycall)) .lt. 3) go to 10

        nohiscall=false;
        hiscall0_ft2=hiscall;
        if (hiscall0_ft2.count()<3)//if(len(trim(hiscall0)).lt.3) then
        {
            hiscall0_ft2=mycall;  //! use mycall for dummy hiscall - mycall won't be hashed.
            nohiscall=true;
        }
        message=mycall+" "+hiscall0_ft2+" RR73";//message=trim(mycall)//' '//trim(hiscall0)//' RR73'
        i3=0;//i3=-1
        n3=0;//n3=-1
        unpk77_success=false;
        TGenFt2->pack77(message,i3,n3,c77);
        msgsent=TGenFt2->unpack77(c77,unpk77_success);
        if (i3!=1 || (message!=msgsent) || !unpk77_success) goto c10;
        for (int i = 0; i < 28; ++i)
        {
            apmy_ru_ft2[i]=2*fmod(c77[i]+rvec[i+1],2)-1;
            aphis_fd_ft2[i]=2*fmod(c77[i+29]+rvec[i+28],2)-1;
        }
        for (int i = 0; i < 77; ++i) c77[i]=fmod(c77[i]+rvec[i],2);

        TGenFt2->encode174_91(c77,cw);
        for (int i = 0; i < 2*ND; ++i) apbits_ft2[i]=2*cw[i]-1;
        if (nohiscall) apbits_ft2[29]=99;
c10:
        //continue;
        mycall0_ft2=mycall;
        hiscall0_ft2=hiscall;
    }

    if (nagain)
    {
        nfa = nfqso-100;//50
        nfb = nfqso+100;//50
    }
    nfa=fmax(200,nfa);    	//hv nfa down
    nfa=fmin(5000-10,nfa);  //hv nfa up
    nfb=fmax(200+10,nfb);   //hv nfb down
    nfb=fmin(5000,nfb);     //hv nfb up

    nfa1=fmax(200,nfa1);      //hv nfa down
    nfa1=fmin(5000-10,nfa1);  //hv nfa up
    nfb1=fmax(200+10,nfb1);   //hv nfb down
    nfb1=fmin(5000,nfb1);     //hv nfb up

    //double nfqso_calc = nfqso;
    if (nfqso<nfa || nfqso>nfb) nfqso = nfa + ((nfb-nfa)/2.0);
    //qDebug()<<nfa1<<nfb1<<"--"<<nagain<<nfa<<nfqso<<nfb;

    //! ndepth=3: 3 passes, bp+osd
    //! ndepth=2: 3 passes, bp only
    //! ndepth=1: 1 pass, no subtraction
    int ndepth = s_decoder_deep2;
    //int max_iterations=40;
    double syncmin=0.8;//0.90  1.18;//1.2 //ws300rc1 syncmin=1.18
    if (ndepth>=2) syncmin=0.75;//0.85;//038
    if (ndepth>=3) syncmin=0.70;//0.75;//0.70;//0.80;//038
    bool dosubtract=true;
    bool doosd=true;
    int nsp=4;//3;//2

    if (ndepth==2)
    {
        nsp=3;
        doosd=false;
    }
    if (ndepth==1)
    {
        nsp=1;
        dosubtract=false;
        doosd=false;
    }
    //qDebug()<<decid<<nfa<<nfqso<<nfb;

    bool f_use_avg=s_use_avg;
    double best_bm_[3][220];
    double best_sync_avg=-99.0;
    double best_f1_avg=0.0;
    double best_xibest_avg=0.0;//int best_ibest_avg=0; !Sub-sample DT of best candidate this period
    double xibest=0.0;//                    //!Sub-sample ibest (fractional sample)
    //double sm1_sub, sp1_sub, den_sub;   //!Parabolic interpolation temporaries
    bool got_candidate=false;
    if (f_use_avg) jseq_a = jseqr;//ft2_even_odd(s_time2);//res=0 (even first), or res=1 (odd second)
    /*if (jseq==0) qDebug()<<decid<<"---> s_time2="<<s_time2<<"Even="<<jseq;
    else qDebug()<<decid<<"---> s_time2="<<s_time2<<" Odd="<<jseq;*/

    int nd1 = 0;
    int nd2 = 0; //int badc = 0;
    for (int isp = 1; isp <= nsp; ++isp)
    {//do isp = 1,nsp
        if (isp==2)
        {
            if (ndecodes==0) break;
            nd1=ndecodes;
        }
        else if (isp==3)
        {
            nd2=ndecodes-nd1;
            if (nd2==0) break;
        }
        else if (isp==4)
        {
            int nd3=ndecodes-nd1-nd2;
            if (nd3==0) break;
        }

        double syncmin_pass=syncmin;
        if (isp>=2) syncmin_pass=syncmin*0.88;
        if (isp>=3) syncmin_pass=syncmin*0.76;

        double candidate[2][250];//(3,100)
        for (int i = 0; i < 2; ++i)
        {
            for (int j = 0; j < maxcand; ++j) candidate[i][j]=0.0;
        }
        int ncand=0;
        getcandidates2(dd,nfa,nfb,nfa1,nfb1,syncmin_pass,nfqso2,maxcand,candidate,ncand,sbase);
        //printf(" nsp=%d isp=%d ncand=%d\n",nsp,isp,ncand);
        /*for (int z= 0; z < ncand; z++)
        {
        QString sss = "";
            sss.append(QString("%1").arg((int)candidate[0][z]));
            sss.append(" ");
            double xsnr=10*log10(candidate[1][z])-14.0;
            sss.append(QString("%1").arg(xsnr,0,'f',1));
        qDebug()<<z<<sss;
        }*/
        bool dobigfft=true;
        for (int icand = 0; icand < ncand; ++icand)//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
        {//do icand=1,ncand
            double f00=candidate[0][icand];       //f0=candidate(1,icand)
            double snr=candidate[1][icand]-1.0;  //snr=candidate(3,icand)-1.0
            //if (f0>1200 && f0<1400) qDebug()<<icand<<f0<<snr;
            ft2_downsample(dd,dobigfft,f00,cd2);  //!Downsample to 32 Sam/Sym
            if (dobigfft) dobigfft=false;
            double sum2=0.0;
            for (int i = 0; i < c_cd2; ++i) sum2+=creal(cd2[i]*conj(cd2[i]));//???
            sum2=sum2/((double)NMAX/(double)NDOWN);//sum2=sum(cd2*conjg(cd2))/(real(NZZ)/real(NDOWN))

            if (sum2>0.0)//if(sum2.gt.0.0) cd2=cd2/sqrt(sum2)
            {
                for (int i = 0; i < c_cd2; ++i) cd2[i]=cd2[i]/sqrt(sum2);
            }
            //! Sample rate is now 12000/18 = 666.67 samples/second
            for (int iseg = 1; iseg <= 3; ++iseg)//! DT search is done over 3 segments
            {//do iseg=1,3
                int idfbest = 0;
                int ibest = -1;
                double smax=-99.0;
                double smax1=-99.0;
                sync_dt_df(cd2,iseg,idfbest,ibest,smax,smax1);
                //if (smax<0.9) continue;//stop 038
                double smaxthresh=0.60;//0.80;//0.9;//038
                if (ndepth>=3) smaxthresh=0.50;//0.65;//0.72;//0.65;//0.75;//038
                if (isp>=2) smaxthresh=smaxthresh*0.88; //! pass 2: ~15%
                if (isp>=3) smaxthresh=smaxthresh*0.76; //! pass 3+: ~25%

                if (smax<smaxthresh) continue;//038
                if (iseg>1 && smax<smax1) continue;
                double f1=f00+(double)idfbest; //qDebug()<<idfbest<<f1;
                if ( f1<=10.0 || f1>=4990.0 ) continue;//cycle
                ft2_downsample(dd,dobigfft,f1,cb); //!Final downsample, corrected f1
                sum2=0.0;
                for (int i = 0; i < c_cb; ++i) sum2+=cabs(cb[i])*cabs(cb[i]);//sum2=sum(abs(cb)**2)/(real(NSS)*NN)
                sum2 = sum2/(double)(NSS*NN);
                if (sum2>0.0)
                {
                    for (int i = 0; i < c_cb; ++i) cb[i]=cb[i]/sqrt(sum2);
                }
                //????????????????????????????
                //qDebug()<<"kkk="<<ibest<<f1;
                //const int c_cb = NDMAX;//=4032
                //const int c_cd = NN*NSS;//=3296
                for (int i = 0; i < c_cd+5; ++i) cd[i]=0.0+0.0*I;//cd=0. //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
                if (ibest>=0)
                {
                    int it=fmin(NDMAX-1,ibest+NN*NSS-1);
                    int np=it-ibest+1;
                    for (int i = 0; i < np; ++i) cd[i]=cb[i+ibest];//cd(0:np-1)=cb(ibest:it)
                }
                else
                {
                    for (int i = 0; i < (NN*NSS+2*ibest); ++i)//cd(-ibest:ibest+NN*NSS-1)=cb(0:NN*NSS+2*ibest-1)
                    {
                        if (i-ibest>=0) cd[i-ibest]=cb[i];//for any case array out of bounds
                    }
                } //qDebug()<<fmin(NDMAX-1,ibest+NN*NSS-1)-ibest+1<<NN*NSS+2*ibest<<c_cd<<c_cb<<f1;

                get_ft2_bitmetrics(cd,bitmetrics_,badsync,s4_);
                if (badsync) continue;

                //! Sub-sample DT refinement via 3-point parabolic interpolation
                //! Improves DT accuracy from ±0.75ms (1 sample) to ±0.1ms
                xibest = get_xibest(cd2,ibest,idfbest,smax);

                //! Track best candidate near nfqso for multi-period averaging
                if (f_use_avg && fabs(f1-nfqso)<100.0 && smax>best_sync_avg)
                {
                    for (int x = 0; x < 3; ++x)
                    {
                        for (int y = 0; y < 2*NN; ++y) best_bm_[x][y]=bitmetrics_[x][y];
                    }
                    best_sync_avg=smax;
                    best_f1_avg=f1;
                    best_xibest_avg=xibest;//best_ibest_avg=ibest;
                    got_candidate=true; //qDebug()<<"   got_candidate"<<got_candidate<<nfqso;
                }

                int nsync_qual = pomFt.SyncQualFt2_4(bitmetrics_[0]);//if (nsync_qual<15) continue;//stop 038
                int nsync_qual_min=13;//15;//038
                if (ndepth>=3) nsync_qual_min=10;//12;//10;//12;//038
                if (nsync_qual<nsync_qual_min) continue;//038
                //qDebug()<<"nsync_qual="<<nsync_qual<<ns1<<ns2<<ns3<<ns4<<f1;

                int nharderror = -1;
                make_bm_ap(dd,bitmetrics_,nQSOProgress,lapcqonly,ndepth,cont_type,f1,snr,nfqso,doosd,
                           dosubtract,xibest,ndecodes,decodes,maxcand,nharderror,f_only_one_color,have_dec,false);
                if (nharderror>=0) break;
            }      //!3 DT segments
        }          //!Candidate list
    }              //!Subtraction loop

    if (/*fl_lapon && */ndec[0][jseqr]>0 && !s_fopen2)////2.69 for ap7 s_fopen8 260r5=+fl_lapon
    {
        bool newdat = true;
        for (int i = 0; i<ndec[0][jseqr]; ++i)
        {
            //qDebug()<<"======="<<msg0[0][jseqr][i]<<f0[0][jseqr][i]<<newdat;
            //if (f0[0][jseqr][i]==-99.0) break; = s_fopen
            if (f0[0][jseqr][i]==-98.0) continue;
            if (msg0[0][jseqr][i].indexOf("<")>-1) continue;
            message=msg0[0][jseqr][i]+"      ";//2.70 =6 pouse crash mid
            int i1=message.indexOf(" ");
            int i2=message.indexOf(" ",i1+1);
            QString call_1=message.mid(0,i1);
            QString call_2=message.mid(i1+1,i2-i1-1);
            QString grid4 =message.mid(i2+1,4); //qDebug()<<i1<<i2+1<<i2+1+4<<message.count()<<message;
            //if(grid4.eq.'RR73' .or. index(grid4,'+').gt.0 .or. index(grid4,'-').gt.0) grid4='    '
            if (grid4=="RR73" || grid4.indexOf("+")>-1 || grid4.indexOf("-")>-1) grid4="    ";
            double xdt=dt0[0][jseqr][i];
            double f1=f0[0][jseqr][i];
            double xbase=pow(10.0,(0.1*(sbase[(int)fmax(1.0,(f1/df))]-40.0)));
            //double xbase = 1.0;
            //int ixb=fmax(0,(f1/(12000.0/(double)NFFT1)));
            //if(ixb>-1 && ixb<NH1) xbase=sbase[ixb];
            message="";
            int nharderrors = 0;
            double xsnr = 0.0;
            double dmin = 0.0; //qDebug()<<"======="<<call_1<<call_2<<grid4<<newdat<<s_fopen2;
            ft2_a7d(dd,newdat,call_1,call_2,grid4,xdt,f1,xbase,nharderrors,dmin,message,xsnr);
            //if (nharderrors==-1) qDebug()<<" NO DEC AP7===="<<xdt<<f1<<message<<dmin<<nharderrors;
            if (nharderrors>=0)
            {
                bool ldupe=false;
                for (int z = 0; z < ndecodes; z++)
                {
                    if (decodes[z]==message)
                    {
                        ldupe=true;
                        break;
                    }
                }
                if (!ldupe)
                {
                    decodes[ndecodes]=message;//HV protect AVG from dupes
                    if (ndecodes < (maxcand-1)) ndecodes++;
                    /*int i3=0; int n3=0; int i4tone[120];
                    for (int i = 0; i < 78; ++i) c77[i]=0;     
                    TGenFt2->pack77(message,i3,n3,c77);
                    TGenFt2->make_c77_i4tone(c77,i4tone);
                    subtractft2(dd,i4tone,f1,(xdt+0.5));*/
                    int nsnr=(int)xsnr;
                    bool fshow = true;
                    float qual=1.0-((float)nharderrors+(float)dmin)/140.0;
                    //if (qual<0.1) qDebug()<<"----->AP7="<<message<<qual<<dmin<<nharderrors;
                    if (nfa1>f1 || nfb1<f1 || (!s_lapon2 && qual<0.2) || (s_lapon2 && qual<0.02)) fshow = false;
                    PrintMsg(s_time2,nsnr,xdt,f1,message,7,false,qual,have_dec,f_only_one_color,fshow);
                }   //if (ldupe) qDebug()<<"            DUPE DEC AP7===="<<xdt<<f1<<message<<ldupe;
            }
        }
    }

    if (f_use_avg && got_candidate)
    {
        //jseq = ft2_even_odd(s_time2);
        if (ft2avg[jseq_a].navg_ft2==0 || fabs(best_f1_avg-ft2avg[jseq_a].f_avg)>10.0)
        {	//! First period or frequency changed: reset accumulator
            for (int x = 0; x < 3; ++x)//bm_avg=best_bm
            {
                for (int y = 0; y < 2*NN; ++y) ft2avg[jseq_a].bm_avg_[x][y]=best_bm_[x][y];
            }
            ft2avg[jseq_a].navg_ft2=1;
            ft2avg[jseq_a].f_avg=best_f1_avg;
            ft2avg[jseq_a].dt_avg=best_xibest_avg/1333.33; //qDebug()<<"   1 navg_ft2="<<ft2avg[jseq].navg_ft2;
        }
        else
        {	//! Accumulate using EMA (Exponential Moving Average)
            ft2avg[jseq_a].navg_ft2++;
            int ntc=fmin(ft2avg[jseq_a].navg_ft2,6);//int ntc=fmin(ft2avg[jseq].navg_ft2,4);
            double u=1.0/(double)ntc;
            for (int x = 0; x < 3; ++x)//bm_avg=u*best_bm + (1.0-u)*bm_avg
            {
                for (int y = 0; y < 2*NN; ++y) ft2avg[jseq_a].bm_avg_[x][y]=u*best_bm_[x][y] + (1.0-u)*ft2avg[jseq_a].bm_avg_[x][y];
            }
            ft2avg[jseq_a].f_avg=u*best_f1_avg + (1.0-u)*ft2avg[jseq_a].f_avg;
            ft2avg[jseq_a].dt_avg=u*best_xibest_avg/1333.33 + (1.0-u)*ft2avg[jseq_a].dt_avg; //qDebug()<<"   Icrease navg_ft2="<<ft2avg[jseq].navg_ft2;
        }
        if (ndecodes==0 && ft2avg[jseq_a].navg_ft2>=2)
        {   //! Try averaged decode if single-period failed and navg >= 2
            double snr=-21.0;
            int nharderror=-1; //qDebug()<<"   Try AVG="<<ft2avg[jseq].navg_ft2;
            make_bm_ap(dd,ft2avg[jseq_a].bm_avg_,nQSOProgress,lapcqonly,ndepth,cont_type,ft2avg[jseq_a].f_avg,snr,nfqso,doosd,
                       dosubtract,best_xibest_avg,ndecodes,decodes,maxcand,nharderror,f_only_one_color,have_dec,true);
            //if (ft2avg[jseq].navg_ft2>9) ft2_clravg(jseq);
        }
        if (ft2avg[jseq_a].navg_ft2>20) ft2_clravg(jseq_a);//16=2min
    }   //qDebug()<<decid<<"<--- END s_time2="<<s_time2<<jseq;
}

