/* The algorithms, source code, look-and-feel of WSJT-X and related
 * programs, and protocol specifications for the modes FSK441, FT8, JT4,
 * JT6M, JT9, JT65, JTMS, QRA64, ISCAT, MSK144, are Copyright © 2001-2017
 * by one or more of the following authors: Joseph Taylor, K1JT; Bill
 * Somerville, G4WJS; Steven Franke, K9AN; Nico Palermo, IV3NWV; Greg Beam,
 * KI7MT; Michael Black, W9MDB; Edson Pereira, PY2SDR; Philip Karn, KA9Q;
 * and other members of the WSJT Development Group.
 *
 * MSHV FT8 Decoder
 * Rewritten into C++ and modified by Hrisimir Hristov, LZ2HV 2015-2020
 * May be used under the terms of the GNU General Public License (GPL)
 */

#include "decoderms.h"
#include "../HvMsPlayer/libsound/genpom.h"
#include "ft_all_ap_def.h"
//#include <QtGui>

/*
static const int KK=87;              //!Information bits (75 + CRC12)
static const int NSPS=1920;          //parameter (NSPS=1920)                 !Samples per symbol at 12000 S/s
static const int NMAX=15*12000;      //parameter (NMAX=15*12000)             !Samples in iwave (180,000)
static const int NSTEP=NSPS/4;       //parameter (NSTEP=NSPS/4)              !Rough time-sync step size
static const int NHSYM=NMAX/NSTEP-3; //parameter (NHSYM=NMAX/NSTEP-3)        !Number of symbol spectra (1/4-sym steps)
static const int NFFT1=2*NSPS;       //parameter (NFFT1=2*NSPS, NH1=NFFT1/2) !Length of FFTs for symbol spectra
static const int NH1=NFFT1/2;        //NH1=1920 NHSYM=372
*/

/////////////////// v2 //////////////////////////////////
//data icos7_2/3,1,4,0,6,5,2/
static const int icos7_2[7] =
    {
        3,1,4,0,6,5,2
    };
static const int nappasses_2[6]=
    {
        2,2,2,4,4,3
        //2,2,2,4,4,3,3//MSHV +1  CQ LZ2HV 237 for +AP
    };
static const int naptypes_2[6][4]=
    {//gjhghghj
        {1,2,0,0},{2,3,0,0},{2,3,0,0},{3,4,5,6},{3,4,5,6},{3,1,2,0}
    };
DecoderFt8::DecoderFt8(int id)
{
    pomFt.initPomFt();
    pomAll.initPomAll();//2.66 for pctile_shell
    decid = id;
    TGenFt8 = new GenFt8(true);//f_dec_gen = dec=true gen=false
    first_sync8d = true;
    first_ft8_downsample = true;
    gen_pulse_gfsk_(pulse_ft8_rx,2880.0,2.0,1920); //bt=2.0 fin   old->bt=4.0  <- Temporary compromise?
    gen_pulse_gfsk_(pulse_ft8_rxAp8,48.0,2.0,32);  //48=32*1.5 32=nsps
    //first_subsft8 = true;
    first_ft8b_2 = true;
    cont_id0_ft8_2 = 0;
    DEC_SAMPLE_RATE = 12000.0;
    twopi=8.0*atan(1.0);
    pi=4.0*atan(1.0);
    first_ft8sbl = true;
    //const int NTAB=65536;
    for (int i= 0; i < 65536; ++i)
    {//do i=0,NTAB-1
        double phi0=(double)i*twopi/65536.0; //16777216.0  65536.0
        ctab8_[i]=(cos(phi0)+sin(phi0)*I);
    }
    ctab8_[65536]=0.0+0.0*I;
    ctab8_[65537]=0.0+0.0*I;
    f_new_p = true;

    s_cou_dd1 = 162432;  //not necessary  n=47*3456; //=162432

    nutc0="-1";
    c_zerop = 0;
    jseq=0;
    for (int i= 0; i < 2; ++i)
    {
        for (int j= 0; j < 2; ++j) ndec[i][j] = 0;
    }
    for (int i = 0; i < 9; ++i)
    {
        for (int j = 0; j < 512; ++j)
        {
            if ((j & (int)pow(2,i))!=0) one_ft8_2[i][j]=true;
            else one_ft8_2[i][j]=false;
        }
    }
    s_ltry_a8=false;

    const int NMAX=180000;//15*DEC_SAMPLE_RATE;//=180000
    const int NFFT=180000;//NMAX; need to be number if no crash  180000
    const int NFILT=4000;//3700;//old NFILT=1400; 4000
    int offset_w = NFILT/2+25;
    //! Create and normalize the filter
    //double *window= new double[NFILT+250];
    double window[NFILT+250] __attribute__((aligned(16)));
    double fac=1.0/double(NFFT);
    double sumw=0.0;
    for (int j = -NFILT/2; j < NFILT/2; ++j)
    {//do j=-NFILT/2,NFILT/2
        window[j+offset_w]=cos(pi*(double)j/(double)NFILT)*cos(pi*(double)j/(double)NFILT);
        sumw+=window[j+offset_w];
    }//qDebug()<<window[69]<<cabs(cw_subsft8[400])<<dd[50]<<f0<<dt<<"--33subtractft8";
    pomAll.zero_double_comp_beg_end(cw_subsft8,0,NMAX+25);
    if (sumw<=0.0) sumw=0.01;// no devide by zero
    for (int i = 0; i < NFILT+1; ++i) cw_subsft8[i]=window[i+offset_w-NFILT/2]/sumw;//cw(1:NFILT+1)=window/sum
    pomAll.cshift1(cw_subsft8,NMAX,(NFILT/2+1));    //cw=cshift(cw,NFILT/2+1);
    f2a.four2a_c2c(cw_subsft8,NFFT,-1,1,decid);//four2a(cw,nfft,1,-1,1)
    for (int i = 0; i < NMAX; ++i) cw_subsft8[i]*=fac;
    for (int j = 0; j < NFILT/2+1; ++j)
    {// do j=1,NFILT/2+1
        // endcorrection(j)=1.0/(1.0-sum(window(j-1:NFILT/2))/sumw)
        double dell;
        double summ = 0.0;
        for (int z = j; z < NFILT/2; ++z) summ+=window[z+offset_w];//hv error  offset_w
        dell = 1.0-summ/sumw;
        if (dell<=0.0) dell = 0.0001;
        endcorrectionft8[j]=1.0/dell;
    } //(need to be) ----> SubCW 5.68004e-14 8.00183e-17 -1.57806e-16  <--Criticle (need to be)
    //qDebug()<<" ----> SubCW"<<creal(cw_subsft8[13020])<<creal(cw_subsft8[50800])<<creal(cw_subsft8[101000]);
    ft8var_init();
}
DecoderFt8::~DecoderFt8()
{}
bool DecoderFt8::f_multi_answer_mod8 = false;
void DecoderFt8::SetStMultiAnswerMod(bool f)
{
    f_multi_answer_mod8 = f;
}
QString DecoderFt8::s_MyBaseCall8 = "NA";
int DecoderFt8::s_id_cont_ft8_28 = 0;
int DecoderFt8::s_ty_cont_ft8_28 = 0;
QString DecoderFt8::s_cont_ft8_cq = "CQ";
QString DecoderFt8::s_MyCall8 = "NA";//3>NA count
bool DecoderFt8::s_lmycallstd = false;
bool DecoderFt8::s_lhiscallstd = false;
void DecoderFt8::SetStWords(QString mc,QString s2,int cq3,int ty4,QString cq)
{
    s_MyBaseCall8 = s2;
    s_id_cont_ft8_28 = cq3;
    s_ty_cont_ft8_28 = ty4; //qDebug()<<"CQid="<<cq3<<"ConTyp="<<ty4<<"cq"<<cq;
    s_cont_ft8_cq = cq;
    s_MyCall8 = mc;
    s_lmycallstd = pomAll.isStandardCall(s_MyCall8);
    tone8(s_lmycallstd,s_lhiscallstd);
    tone8myc();
}
QString DecoderFt8::s_HisBaseCall8 = "NA";
QString DecoderFt8::s_HisCall8 = "NA";//3>NA count
QString DecoderFt8::s_HisGrid8 = "NA";
void DecoderFt8::SetStHisCallGrid(QString hc,QString hb,QString g)
{
    s_HisCall8 = hc;
    s_HisGrid8 = g;//ws300rc1
    s_HisBaseCall8 = hb;
    s_lhiscallstd = pomAll.isStandardCall(s_HisCall8);
    tone8(s_lmycallstd,s_lhiscallstd);
}
QString DecoderFt8::s_time8 = "000000";
int DecoderFt8::s_mousebutton8 = 0;
bool DecoderFt8::s_fopen8 = false;
void DecoderFt8::SetStDecode(QString time,int mousebutton,bool ffopen)
{
    s_time8 = time;
    s_mousebutton8 = mousebutton;//mousebutton Left=1, Right=3 fullfile=0 rtd=2
    s_fopen8 = ffopen;//2.66 for ap7 s_fopen8
}
static int s_decoder_deep8 = 1;
void DecoderFt8::SetStDecoderDeep(int d)
{
    s_decoder_deep8 = d; //qDebug()<<"s_decoder_deep8="<<s_decoder_deep8;
}
bool DecoderFt8::s_lapon8 = false;
void DecoderFt8::SetStApDecode(bool f)
{
    s_lapon8 = f;
    lapmyc = f;//???
}
int DecoderFt8::s_nQSOProgress8 = 0;
int DecoderFt8::s_nlasttx = 0;
void DecoderFt8::SetStQSOProgress(int i,int j)
{
    s_nQSOProgress8 = i;
    s_nlasttx = j; //qDebug()<<s_nQSOProgress8<<s_nlasttx;
}
double DecoderFt8::s_nftx8 = 1200.0;
void DecoderFt8::SetStTxFreq(double f)
{
    s_nftx8 = f;
}
void DecoderFt8::sync8d(double complex *cd0,int i0,double complex *ctwk,int itwk,double &sync)
{
    //p(z1)=real(z1)**2 + aimag(z1)**2          !Statement function for power*/
    int NP2=2812;
    //int NDOWN=60;
    //double complex z1,z2,z3;
    double complex csync2[36];

    //! Set some constants and compute the csync array.

    if (first_sync8d)
    {
        /*double fs2=DEC_SAMPLE_RATE/60.0;//int NDOWN=60;//!Sample rate after downsampling
        double dt2=1.0/fs2;                               //!Corresponding sample interval
        double taus=32.0*dt2;                             //!Symbol duration
        double baud=1.0/taus;*/                           //!Keying rate
        for (int i = 0; i < 7; ++i)
        {//do i=0,6
            double phi=0.0;
            double dphi=twopi*(double)icos7_2[i]/32.0;
            for (int j = 0; j < 32; ++j)
            {//do j=1,32
                csync_ft8_2[i][j]=cos(phi)+sin(phi)*I;//csync(i,j)=cmplx(cos(phi),sin(phi))  !Waveform for 7x7 Costas array
                phi=fmod(phi+dphi,twopi);
            }
        }
        first_sync8d=false;
    }

    sync=0.0;
    //qDebug()<<i0
    for (int i = 0; i < 7; ++i)
    {//do i=0,6      	                            //!Sum over 7 Costas frequencies and
        double complex z1=0.0+0.0*I;
        double complex z2=0.0+0.0*I;
        double complex z3=0.0+0.0*I;
        int i1=i0+i*32;                         //!three Costas arrays
        int i2=i1+36*32;
        int i3=i1+72*32;

        for (int j = 0; j < 32; ++j)
            csync2[j]=csync_ft8_2[i][j];//2.00

        if (itwk==1)
        {
            for (int z = 0; z < 32; ++z)
                csync2[z]=ctwk[z]*csync2[z];  //csync2=ctwk*csync2    //!Tweak the frequency
        }
        //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
        if (i1>=0 && i1+32<NP2)  //if(i1>=1 && i1+31<=NP2)
            z1=pomAll.sum_dca_mplay_conj_dca(cd0,i1,i1+32,csync2);//if(i1.ge.1 .and. i1+31.le.NP2) z1=sum(cd0(i1:i1+31)*conjg(csync2))
        if (i2>=0 && i2+32<NP2)  //if(i2>=1 && i2+31<=NP2)
            z2=pomAll.sum_dca_mplay_conj_dca(cd0,i2,i2+32,csync2);//if(i2.ge.1 .and. i2+31.le.NP2) z2=sum(cd0(i2:i2+31)*conjg(csync2))
        if (i3>=0 && i3+32<NP2)  //if(i3>=1 && i3+31<=NP2)
            z3=pomAll.sum_dca_mplay_conj_dca(cd0,i3,i3+32,csync2);//if(i3.ge.1 .and. i3+31.le.NP2) z3=sum(cd0(i3:i3+31)*conjg(csync2))

        z1 *= 0.01;//2.00=0.01 1.76=0.1
        z2 *= 0.01;//2.00=0.01 1.76=0.1
        z3 *= 0.01;//2.00=0.01 1.76=0.1
        sync += (pomAll.ps_hv(z1) + pomAll.ps_hv(z2) + pomAll.ps_hv(z3));//sync = sync + p(z1) + p(z2) + p(z3)
        //sync = sync + ps_hv(z1)*0.01 + ps_hv(z2)*0.01 + ps_hv(z3)*0.01;
    }
    //qDebug()<<"sync=========================="<<sync;
}
#define F0_DB 8.49*6.25  //2.70
#define F1_DB 0.91*6.25  //2.70
//#define F0_DB 9.2*6.25 //2.70
//#define F1_DB 0.8*6.25 //2.70
//#define F0_DB 8.4*6.25 //2.69
//#define F1_DB 1.0*6.25 //2.69
//#define F0_DB 8.5*6.25
//#define F1_DB 1.5*6.25

//#define K_SUB 1.9842 //2.68
//#define K_SUB 1.9840 //2.66
#define K_SUB 1.9962 //2.70
//#define K_SUB 1.9998
void DecoderFt8::ft8_downsample(double *dd,bool &newdat,double f0,double complex *c1)
{
    const int NFFT2=3200;
    const int c_c1=NFFT2;//1.44 tuk be6e problema be6e->(NFFT2-1) triabva da e->(NFFT2-0)
    const int NMAX=15*DEC_SAMPLE_RATE;  //=180000    !192000/60 = 3200
    const int NFFT1=192000;

    if (first_ft8_downsample)
    {
        for (int i = 0; i < 101; ++i) taper_ft8_ds[i]=0.5*(1.0+cos((double)i*pi/100.0));//0.25 taper(i)=0.5*(1.0+cos(i*pi/100))
        first_ft8_downsample=false;
    }
    if (newdat)
    {
        double *x = new double[NFFT1+150];//double x[NFFT1+150]; //! Data in dd have changed, recompute the long FFT
        for (int i = 0; i < NFFT1; ++i)
        {
            if (i < NMAX) x[i]=dd[i]*0.01;
            else x[i]=0.0;
        }
        f2a.four2a_d2c(cx_ft8,x,NFFT1,-1,0,decid);//call four2a(cx,NFFT1,1,-1,0)             //!r2c FFT to freq domain
        newdat=false;
        delete [] x;
    }
    //qDebug()<<"ft8_downsample"; return;
    double df=DEC_SAMPLE_RATE/(double)NFFT1;
    int i0=int(f0/df); //qDebug()<<"baud="<<baud;
    double ft=f0+F0_DB;
    double fb=f0-F1_DB;
    int it=fmin(int(ft/df),NFFT1/2);
    int ib=fmax(0,int(fb/df));
    int k=0; //if (f0==1200.0) qDebug()<<"fdown="<<fb<<ft;

    for (int i = ib; i < it; ++i)
    {
        c1[k]=cx_ft8[i];//c1(k)=cx(i)
        k++;
    }

    int to_tap = 101;
    int ctrp = to_tap - 1; //qDebug()<<k; k=900
    for (int i = 0; i < to_tap; ++i)
    {
        c1[i]*=taper_ft8_ds[ctrp];//c1(0:100)=c1(0:100)*taper(100:0:-1) //c1[i]=taper_ft8_ds[ctrp]*conj(c1[i]);
        ctrp--;
    }
    for (int i = 0; i < to_tap; ++i)// k=900
        c1[i+k-to_tap]*=taper_ft8_ds[i];//c1(k-1-100:k-1)=c1(k-1-100:k-1)*taper //c1[i+k-to_tap]=taper_ft8_ds[i]*conj(c1[i+k-to_tap]);

    pomAll.cshift1(c1,c_c1,i0-ib);//c1=cshift(c1,i0-ib)
    f2a.four2a_c2c(c1,NFFT2,1,1,decid); //call four2a(c1,NFFT2,1,1,1)            //!c2c FFT back to time domain

    double fac=1.0/sqrt((double)NFFT1*(double)NFFT2);
    for (int i = 0; i < c_c1; ++i) c1[i]*=fac;
}
void DecoderFt8::gen_ft8cwaveRx(int *i4tone,double f_tx,double complex *cwave)
{
    //////////////////////// GFSK MODULATOR ///////////////////////////////////////////
    //const int NTAB=65536;
    int nsym=79;
    int nsps=1920; //for 12000=1920 for=48000->4*1920;
    int nwave=nsym*nsps;//max rx=151680
    //static double dphi[155570];
    double hmod=1.0;
    double dt=1.0/12000.0;// for RX=12000 for tx=48000

    //! Compute the smoothed frequency waveform.
    //! Length = (nsym+2)*nsps samples, zero-padded (nsym+2)*nsps TX=215040 RX=53760
    double dphi_peak=twopi*hmod/(double)nsps;
    double *dphi = new double[155620];  //max rx=155520    dphi=79*1920=151680 + 1920*2=155520

    for (int i= 0; i < 155540; ++i) dphi[i]=0.0;

    for (int j= 0; j < nsym; ++j)
    {
        int ib=j*nsps;
        for (int i= 0; i < 3*nsps; ++i)//5760
            dphi[i+ib] += dphi_peak*pulse_ft8_rx[i]*(double)i4tone[j];
    }

    int bgn =nsym*nsps;//=151680
    for (int i= 0; i < 2*nsps; ++i)//=3840
    {
        dphi[i]+=dphi_peak*i4tone[0]*pulse_ft8_rx[i+nsps];
        dphi[i+bgn]+=dphi_peak*i4tone[nsym-1]*pulse_ft8_rx[i];
    }

    double ofs = twopi*f_tx*dt;
    double phi=0.0;
    for (int j=0; j < nwave; ++j)//=151680
    {
        //2.39 old cwave[j]=(cos(phi)+sin(phi)*I);
        int i=(int)(phi*65536.0/twopi); //16777216.0  65536.0
        //if (i<0 || i>65535) qDebug()<<i;
        if (i<0) 	 i=0;
        if (i>65535) i=65535;
        cwave[j]=ctab8_[i];
        phi=fmod(phi+dphi[j+nsps]+ofs,twopi);
    }

    int nramp=(int)((double)nsps/8.0);
    for (int i = 0; i < nramp; ++i) cwave[i]*=(1.0-cos(twopi*(double)i/(2.0*nramp)))/2.0;
    int k2=nsym*nsps-nramp+1; //k1=nsym*nsps-nramp+1   //k2=(nsym+1)*nsps+1;
    for (int i = 0; i < nramp; ++i) cwave[i+k2]*=(1.0+cos(twopi*(double)i/(2.0*nramp)))/2.0;//i+k1-nsps
    //qDebug()<<"nsamp="<<k2+nramp-1;
    delete [] dphi;
}
double DecoderFt8::BestIdtft8(double *dd,double f0,double dt,double idt,double complex *cref,
                              double complex *cfilt,double complex *cw_subs,double *endcorr,
                              double *xdd,double complex *cx)
{
    double sqq=0.0;
    const int NFRAME=1920*79;//=151680
    const int NMAX=180000;
    const int NFFT=180000;
    const int NFILT=4000;
    int nstart=dt*DEC_SAMPLE_RATE+1.0+idt;//0 +1.0  -1920

    //for (int i = 0; i < NMAX; ++i) xdd[i]=0.0;
    for (int i = 0; i < NFRAME; ++i)
    {
        int id=nstart+i-1;//0 -1
        if (id>=0 && id<NMAX)
        {
            xdd[id]=dd[id];//no problem to be here
            cfilt[i]=xdd[id]*conj(cref[i]);//camp[i];//cfilt(1:nframe)=camp(1:nframe)
        }
    }

    for (int i = NFRAME; i < 180010; ++i) cfilt[i]=0.0;
    f2a.four2a_c2c(cfilt,NFFT,-1,1,decid);
    for (int i = 0; i < NFFT; ++i) cfilt[i]*=cw_subs[i];
    f2a.four2a_c2c(cfilt,NFFT,1,1,decid);
    for (int i = 0; i < NFILT/4; ++i) cfilt[i]*=endcorr[i];//hv NFILT/2+1
    int revv = NFRAME-1;
    for (int i = 0; i < NFILT/4; ++i)//hv NFILT/2+1  //NFRAME=151680 NFILT=4000;
    {
        cfilt[revv]*=endcorr[i];
        revv--;
    }

    double t_dd[180300];
    //for (int i = 0; i < NMAX; ++i) t_dd[i]=0.0;

    int c_beg = nstart-1;
    if (c_beg<0) c_beg = abs(c_beg);
    else c_beg = 0;
    if (c_beg>NMAX) c_beg=NMAX;
    for (int i = 0; i < c_beg; ++i) t_dd[i]=0.0;
    for (int i = NFRAME-2; i < NMAX; ++i) t_dd[i]=0.0;
    //qDebug()<<c_beg<<NFRAME-2;

    for (int i = 0; i < NFRAME; ++i)
    {
        int j=nstart+i-1;//0 -1
        if (j>=0 && j<NMAX)
        {
            double complex cfr = cfilt[i]*cref[i];
            //xdd[j]-=1.96*creal(cfr);//2.41=1.96 2.39=1.97  2.35=1.94   2.26 1.93 no->2.0  //2.07 1.92<-tested    1.5,1.6  ,1.7ok,
            xdd[j]-=K_SUB*creal(cfr);
            t_dd[i]=xdd[j];
        }
    }
    for (int i = 0; i < NMAX; ++i) xdd[i]=t_dd[i];
    //qDebug()<<"real="<<start_i<<stop_i;

    /*int c_end = nstart-1; //qDebug()<<c_end<<j;
    if (c_end>NMAX) c_end=NMAX;//2.40 protection
    for (int i = 0; i < c_end; ++i) xdd[i]=0.0;
    if (j<0) j=0;//2.40 protection
    for (int i = j; i < NMAX; ++i) 	xdd[i]=0.0;*/

    f2a.four2a_d2c(cx,xdd,NFFT,-1,0,decid);   //!Forward FFT, r2c
    double df=12000.0/(double)NFFT;

    //int ia=((f0-1.05*6.25)/df);//ia=(f0-1.5*6.25)/df
    //int ib=((f0+8.3*6.25)/df);//ib=(f0+8.5*6.25)/df
    int ia=((f0-F1_DB)/df);//ia=(f0-1.5*6.25)/df
    int ib=((f0+F0_DB)/df);//ib=(f0+8.5*6.25)/df
    //if (ia<5000 || ib>NFFT/2-100) qDebug()<<f0<<ia<<ib;
    if (ia<0)        ia = 0;//2.40 protection
    if (ib>(NFFT/2)) ib = (NFFT/2);//2.40 protection

    for (int i = ia; i < ib; ++i)
    {
        sqq+=(creal(cx[i])*creal(cx[i]) + cimag(cx[i])*cimag(cx[i]));//sqq=sqq + real(cx(i))*real(cx(i)) + aimag(cx(i))*aimag(cx(i))
    }
    //qDebug()<<sqq;
    return sqq;
}
void DecoderFt8::subtractft8(double *dd,int *itone,double f0,double dt,bool lrefinedt)
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
    bool f_du_sub = true;
    if (lrefinedt)//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    {
        double *xdd = new double[180300];//NFFT=180200
        double complex *cx = new double complex[90400];//NFFT/2=90000
        double sqa = BestIdtft8(dd,f0,dt,-90,cref,cfilt,cw_subsft8,endcorrectionft8,xdd,cx);
        double sqb = BestIdtft8(dd,f0,dt,+90,cref,cfilt,cw_subsft8,endcorrectionft8,xdd,cx);
        double sq0 = BestIdtft8(dd,f0,dt,  0,cref,cfilt,cw_subsft8,endcorrectionft8,xdd,cx);
        double dx  = pomAll.peakup(sqa,sq0,sqb);
        if (fabs(dx)>1.0) f_du_sub = false; //no subtract
        else idt=(int)(90.0*dx);//subtract
        delete [] xdd;
        delete [] cx;
        //if (!f_du_sub) qDebug()<<"idt="<<fabs(dx);
    }

    if (f_du_sub)
    {
        int nstart=dt*DEC_SAMPLE_RATE+1.0+idt;//0 +1.0  -1920
        for (int i = 0; i < NFRAME; ++i)
        {
            int id=nstart+i-1;//0 -1
            if (id>=0 && id<NMAX)
            {
                cfilt[i]=dd[id]*conj(cref[i]);//camp[i];//cfilt(1:nframe)=camp(1:nframe)
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
                dd[j]-=K_SUB*creal(cfr);
            }
        }
    }
    delete [] cref;
    delete [] cfilt;
}
bool DecoderFt8::ft8_downs_sync_bmet(double *dd,bool ap7,bool &newdat,double &f1,double &xdt,int &nbadcrc,
                                     int &nsync,double s8_[79][8],double *bmeta,double *bmetb,
                                     double *bmetc,double *bmetd,double *bmete,int imetric,int ndepth)
{
    double complex cd0[3350];			   //3200+100  __attribute__((aligned(32)))
    double complex ctwk[32+5];
    double a[5];
    const int NP2=2812;
    //const int ND=58;//                   !Data symbols
    const int NS=21;//                     !Sync symbols (3 @ Costas 7x7)
    const int NN=NS+58;//                  !Total channel symbols (79)
    double complex csymb[40];//32+8
    //double s8_[NN][8]; 					//real s1(0:7,ND),s2(0:7,NN)
    double complex cs_[NN][8]; 				//complex cs(0:7,NN)
    double s2[512];
    const int graymap[8] =
        {
            0,1,3,2,5,6,4,7
        };

    pomAll.zero_double_comp_beg_end(cd0,0,3300);//3200+100
    //int NDOWN=60;
    double fs2=DEC_SAMPLE_RATE/60.0;
    double dt2=1.0/fs2;
    double delfbest=0.0;
    int ibest=0;
    nbadcrc=1;
    double temp[5];//ws300rc1

    ft8_downsample(dd,newdat,f1,cd0);   //!Mix f1 to baseband and downsample

    //qDebug()<<"ft8b="<<creal(cd0[100])<<cimag(cd0[100]);
    int i0=int((xdt+0.5)*fs2);   //0.5                 //!Initial guess for start of signal
    //int i0=int((xdt+0.54)*fs2); 2.34
    double smax=0.0;
    double sync=0.0;
    //pomAll.zero_double_comp_beg_end(ctwk,0,33);//no needed 2.37
    for (int idt = i0-10; idt <= i0+10; ++idt)//2.39 ????
    {//2.39 do idt=i0-10,i0+10  do idt=i0-8,i0+8                         //!Search over +/- one quarter symbol
        sync8d(cd0,idt,ctwk,0,sync); //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
        if (sync>smax)
        {
            smax=sync; //qDebug()<<idt<<sync;
            ibest=idt;
        }
    }
    //2.39 double xdt2=(double)ibest*dt2;                           //!Improved estimate for DT

    //! Now peak up in frequency
    //i0=int(xdt2*fs2);
    smax=0.0;//0.0;
    for (int ifr = -10; ifr <= 10; ++ifr)
        //for (int ifr = -20; ifr <= 20; ++ifr)
        //for (int ifr = -5; ifr <= 5; ifr++)
    {//do ifr=-5,5                              !Search over +/- 2.5 Hz
        double delf=(double)ifr*0.25;
        //double delf=(double)ifr*0.125;
        //double delf=(double)ifr*0.5;
        double dphi=twopi*delf*dt2;
        double phi=0.0; //qDebug()<<"2-ft8b delfbest="<<delf;
        for (int i = 0; i < 32; ++i)
        {//do i=1,32
            ctwk[i]=cos(phi)+sin(phi)*I;
            phi=fmod(phi+dphi,twopi);
        }
        sync8d(cd0,ibest,ctwk,1,sync);//sync8d(cd0,i0,ctwk,1,sync);
        if (sync>smax)
        {
            smax=sync;  //qDebug()<<"2-ft8b delfbest="<<sync;
            delfbest=delf;
        }
    }
    //qDebug()<<"2-ft8b delfbest="<<"delfbest"<<delfbest<<"sync"<<sync<<xdt2;
    for (int i = 0; i < 5; ++i) a[i]=0.0;//-delfbest;//0.0;
    a[0]=-delfbest;
    pomFt.twkfreq1(cd0,NP2,fs2,a,cd0);
    //2.39 old xdt=xdt2;
    f1=f1+delfbest;                           //!Improved estimate of DF

    bool fnewdat = false;
    ft8_downsample(dd,fnewdat,f1,cd0);   //!Mix f1 to baseband and downsample
    smax=0.0;
    double ss[12];// ss(9)
    for (int idt = -4; idt < 5; ++idt)
    { //do idt=-4,4                         !Search over +/- one quarter symbol
        sync8d(cd0,ibest+idt,ctwk,0,sync);
        ss[idt+4]=sync; 	//ss(idt+5)=sync
    }
    smax=pomAll.maxval_da_beg_to_end(ss,0,9);//9
    int iloc=pomAll.maxloc_da_end_to_beg(ss,0,9);//9
    ibest=iloc-4+ibest;//ibest=iloc(1)-5+ibest

    if (ap7) xdt=(double)ibest*dt2 - 0.5;
    else xdt=(double)ibest*dt2;//hv tested no-1 (double)(ibest-1)*dt2; xdt=(ibest-1)*dt2

    sync=smax;

    //QString sss;
    for (int k = 0; k < NN; ++k)
    {
        int i1=ibest+k*32;//i1=ibest+(k-1)*32
        pomAll.zero_double_comp_beg_end(csymb,0,34);
        if ( i1>=0 && (i1+32) < NP2 )
        {
            for (int z = 0; z < 32; ++z)
                csymb[z]=cd0[i1+z];
        }
        f2a.four2a_c2c(csymb,32,-1,1,decid);
        for (int z = 0; z < 8; ++z)
        {
            cs_[k][z]=csymb[z]*0.001;//1000.0;//cs(0:7,k)=csymb(1:8)/1e3
            s8_[k][z]=cabs(csymb[z]);//s8(0:7,k)=abs(csymb(1:8))
            //sss.append(QString("%1 ").arg(cabs(csymb[z]),0,'f',1));
        }//sss.append("\n");
    }

    if (!ap7)
    {
        //! sync quality check
        int is1=0;
        int is2=0;
        int is3=0;
        for (int k = 0; k < 7; ++k)
        {
            int ip=pomAll.maxloc_da_beg_to_end(s8_[k],0,8);   //ip=maxloc(s2(:,k))
            if (icos7_2[k]==(ip)) is1++; //if(icos7(k-1).eq.(ip(1)-1)) is1=is1+1
            ip=pomAll.maxloc_da_beg_to_end(s8_[k+36],0,8);//ip=maxloc(s2(:,k+36))
            if (icos7_2[k]==(ip)) is2++;//if(icos7(k-1).eq.(ip(1)-1)) is2=is2+1;
            ip=pomAll.maxloc_da_beg_to_end(s8_[k+72],0,8);//ip=maxloc(s2(:,k+72))
            if (icos7_2[k]==(ip)) is3++;//if(icos7(k-1).eq.(ip(1)-1)) is3=is3+1;
        }
        //! hard sync sum - max is 21
        int syncmin=6;//ws300rc1
        if (imetric==2) syncmin=6;//if(imetric.eq.2) syncmin=7
        if (ndepth<=2)  syncmin=8;//if(ndepth.le.2) syncmin=8
        nsync=is1+is2+is3;
        if (nsync <= syncmin) //! bail out if(nsync .le. 6) then ! bail out
        {
            nbadcrc=1; //qDebug()<<"ft8b NONONO";
            return false;
        }
    }
    /*for (int i = 0; i < 174; ++i)//no needed
    {
    	bmeta[i] = 0.001;
    	bmetb[i] = 0.001;
    	bmetc[i] = 0.001;
    	bmetd[i] = 0.001;
    }*/
    for (int nsym = 1; nsym <= 3; ++nsym)
    {//do nsym=1,3
        int nt=pow(2,(3*nsym));//nt=2**(3*nsym);
        //qDebug()<<"ft8b nsync<=6<-"<<nt; 8,64,512
        for (int ihalf = 1; ihalf <= 2; ++ihalf)
        {//do ihalf=1,2
            for (int k = 1; k <= 29; k+=nsym)
            {//do k=1,29,nsym
                int ks=0;
                if (ihalf==1) ks=k+7;
                if (ihalf==2) ks=k+43;
                //if(ks<=2 || ks>=76)
                //qDebug()<<"ft8b ks"<<ks;
                for (int i = 0; i < nt; ++i)
                {//do i=0,nt-1
                    int i1=i/64;
                    int i2=(i & 63)/8;
                    int i3=(i & 7);
                    if (nsym==1)   //complex cs_[NN][8]; //complex cs(0:7,NN)
                        s2[i]=cabs(cs_[ks-1][graymap[i3]]);//s2(i)=abs(cs(graymap(i3),ks))
                    else if (nsym==2)
                        s2[i]=cabs(cs_[ks-1][graymap[i2]]+cs_[ks][graymap[i3]]); //s2(i)=abs(cs(graymap(i2),ks)+cs(graymap(i3),ks+1))
                    else if (nsym==3)
                        s2[i]=cabs(cs_[ks-1][graymap[i1]]+cs_[ks][graymap[i2]]+cs_[ks+1][graymap[i3]]);//s2(i)=abs(cs(graymap(i1),ks)+cs(graymap(i2),ks+1)+cs(graymap(i3),ks+2))
                    //else
                    //qDebug()<<"Error - nsym must be 1, 2, or 3.";
                }
                //for (int i = 0; i < nt; ++i)
                //s2l[i]=log(s2[i]+1e-32);//??? s2l(0:nt-1)=log(s2(0:nt-1)+1e-32)
                /*if(imetric==2) //ws300rc1 if(imetric.eq.2) s2=s2**2
                {
                	for (int i = 0; i < 512; ++i)
                	{
                		double s22 = s2[i]*s2[i];
                		s2[i] = s22;                
                	}
                }*/
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
                            if (tmax1v>max1v)
                                max1v=tmax1v;
                        }
                    }
                    double max2v=0.0;
                    for (int zz = 0; zz < nt; ++zz)
                    {
                        if (one_ft8_2[ibmax-ib][zz]==false)
                        {
                            double tmax2v=s2[zz];
                            if (tmax2v>max2v)
                                max2v=tmax2v;
                        }
                    }
                    double bm=max1v-max2v; //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
                    //if (max1v==0.0 || max2v==0.0) qDebug()<<max1v<<max2v;
                    if (i32+ib>173) continue;//cycle //if(i32+ib .gt.174) cycle

                    if (nsym==1)
                    {
                        bmeta[i32+ib]=bm; //qDebug()<<"0-173"<<i32+ib;
                        //den=max(maxval(s2(0:nt-1),one(0:nt-1,ibmax-ib)), maxval(s2(0:nt-1),.not.one(0:nt-1,ibmax-ib)))
                        double den = max1v;
                        if (max2v > max1v) den = max2v;

                        double cm = 0.0;
                        if (den>0.0) //if(den.gt.0.0) then
                            cm=bm/den;
                        else
                            cm=0.0; //! erase it
                        bmetd[i32+ib]=cm;
                    }
                    else if (nsym==2) bmetb[i32+ib]=bm;//bmetb(i32+ib)=bm
                    else if (nsym==3) bmetc[i32+ib]=bm;//bmetc(i32+ib)=bm
                }
            }
        }
    }
    /*for (int i = 0; i < 174; ++i)
    {
    	if (bmeta[i] == 0.001) qDebug()<<"a";
    	if (bmetb[i] == 0.001) qDebug()<<"b";
    	if (bmetc[i] == 0.001) qDebug()<<"c";
    	if (bmetd[i] == 0.001) qDebug()<<"d";
    }*/
    /*do i=1,174
    temp(1)=bmeta(i)
    temp(2)=bmetb(i)
    temp(3)=bmetc(i)
    ip=maxloc(abs(temp))
    bmete(i)=temp(ip(1))
    enddo*/
    if (!ap7)//ws300rc1
    {
        for (int i = 0; i < 174; ++i)
        {
            temp[0]=bmeta[i];
            temp[1]=bmetb[i];
            temp[2]=bmetc[i];
            int ip = 0;
            double max = fabs(temp[0]);
            for (int i0 = 1; i0 < 3; ++i0)//int ip = pomAll.maxloc_da_beg_to_end(temp,0,3);
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
        pomFt.normalizebmet(bmete,174);//ws300rc1
    }
    pomFt.normalizebmet(bmeta,174);
    pomFt.normalizebmet(bmetb,174);
    pomFt.normalizebmet(bmetc,174);
    pomFt.normalizebmet(bmetd,174);
    //if (!ap7) pomFt.normalizebmet(bmete,174);//ws300rc1
    return true;
}
void DecoderFt8::first_ft8b_and_ft8var(QString hiscall12,int cont_id)
{   //if (!first_ft8b_2 && cont_id==cont_id0_ft8_2 && hiscall12==hiscall12_0_ft8_2) return;
    if (first_ft8b_2 || (cont_id!=cont_id0_ft8_2))
    {
        bool c77[100];
        if (cont_id!=0)
        {
            int i3=0;
            int n3=0;
            for (int i = 0; i < 78; ++i) c77[i]=0;
            TGenFt8->pack77(s_cont_ft8_cq+" LZ2HV KN23",i3,n3,c77);
        }
        for (int i = 0; i < 29; ++i)
        {
            if (cont_id==0) mcq_ft8_2[i]=2*mcq_ft[i]-1;
            else mcq_ft8_2[i]=2*c77[i]-1;
            if (i<19)
            {
                mrrr_ft8_2[i]=2*mrrr_ft[i]-1;
                m73_ft8_2[i]=2*m73_ft[i]-1;
                mrr73_ft8_2[i]=2*mrr73_ft[i]-1;
            }
        }
        hiscall12_0_ft8_2="";
        cont_id0_ft8_2 = cont_id;
        first_ft8b_2=false;
    }
    if (hiscall12!=hiscall12_0_ft8_2)
    {
        QString c13=hiscall12+"            ";
        int n10 = 0;
        int n12 = 0;
        int n22 = 0;
        TGenFt8->save_hash_call_from_dec(c13,n10,n12,n22);
        hiscall12_0_ft8_2=hiscall12;
    }
}
void DecoderFt8::ft8b(double *dd,bool &newdat,int nQSOProgress,double nfqso,double nftx,int ndepth,bool n4pas3int,bool lapon,
                      double napwid,bool lsubtract,bool nagain,int cont_id,int cont_type,int imetric,int &iaptype,double &f1,double &xdt,
                      double xbase,int *apsym,int &nharderrors,double &dmin,int &nbadcrc,QString &message,
                      double &xsnr,QString hiscall12,int *i4tone,int &i3,int &n3)//QString mygrid6,bool bcontest,
{
    //int NDOWN=60;//parameter (NDOWN=60)                  !Downsample factor
    //int NP2=2812;
    //const int ND=58;//                     !Data symbols
    const int NS=21;//                     !Sync symbols (3 @ Costas 7x7)
    const int NN=NS+58;//                  !Total channel symbols (79)
    //const int KK=87;//                     !Information bits (75 + CRC12)

    //double complex ctwk[32+5];
    //double a[5];
    //double complex csymb[40];//32+8
    double s8_[NN][8]; //real s1(0:7,ND),s2(0:7,NN)
    //double complex cs_[NN][8]; //complex cs(0:7,NN)
    //double s2[512];

    double bmeta[174];
    double bmetb[174];
    double bmetc[174];
    double bmetd[174];
    double bmete[174];//ws300rc1

    double llra[174];
    double llrb[174];
    double llrc[174];//llr1(3*ND)
    double llrd[174];
    double llre[174];//ws300rc1
    double llrz[174];

    bool apmask[174];
    bool cw[194];//174
    bool message91[140];

    first_ft8b_and_ft8var(hiscall12,cont_id);

    //int max_iterations=30;//2.39 30
    nbadcrc=1;  //! this is used upstream to flag good decodes.
    nharderrors=-1;
    int nsync=0;
    if (!ft8_downs_sync_bmet(dd,false,newdat,f1,xdt,nbadcrc,nsync,s8_,bmeta,bmetb,bmetc,bmetd,bmete,imetric,ndepth)) return;

    double scalefac = 2.83;//scalefac=2.83 double ss=0.85;//hv tested->85-86    0.84;//0.84
    //double maxval_llra_abs = 0.0;//ws300rc1 stop
    //double maxval_llrb_abs = 0.0;
    //double mina=0.0; double maxa=0.0;
    for (int z = 0; z < 174; ++z)
    {
        llra[z]=scalefac*bmeta[z];
        llrb[z]=scalefac*bmetb[z];
        llrc[z]=scalefac*bmetc[z];//  ! llr's for use with ap
        llrd[z]=scalefac*bmetd[z];
        llre[z]=scalefac*bmete[z];//ws300rc1
        /*double llra_abs = fabs(llra[z]);//ws300rc1 stop
        if (llra_abs>maxval_llra_abs)
            maxval_llra_abs=llra_abs;*/
        /*double llrb_abs = fabs(llrb[z]);
        if (llrb_abs>maxval_llrb_abs)
            maxval_llrb_abs=llrb_abs;*/
        //if (llra[z]<mina) mina=llra[z];
        //if (llra[z]>maxa) maxa=llra[z];
    } //if (maxa>1.0 || mina<-1.0) qDebug()<<"A="<<mina<<maxa;
    //double apmag = maxval_llra_abs*1.01; //ws300rc1 stop qDebug()<<"apmag="<<apmag;//double apmag = scalefac*(maxval_llra_abs*1.01);//old 2.39

    /*! pass #
    !------------------------------
    !   1        regular decoding, nsym=1 
    !   2        regular decoding, nsym=2 
    !   3        regular decoding, nsym=3 
    !   4        regular decoding, nsym=1, bit-by-bit normalized 
    !   5        regular decoding, choose best (largest) metric from 1-3
    !   6        ap pass 1, nsym=1
    !   7        ap pass 1, nsym=2
    !   8        ap pass 2, nsym=1
    !   9        ap pass 2, nsym=2
    !   10       ap pass 3, nsym=1
    !   11       ap pass 3, nsym=2
    !   12       ap pass 4, nsym=1
    !   13       ap pass 4, nsym=2*/
    int npasses = 0;

    bool lapcqonly = false;//lapcqonly<-no used fom me. if no TX more then 10min use only AP1
    //if(lapon.or.ncontest.eq.7) then !Hounds always use AP
    if (lapon)//no MSHV Hounds
    {
        if (!lapcqonly) npasses=5+2*nappasses_2[nQSOProgress];//ws300rc1 if (!lapcqonly) npasses=4+nappasses_2[nQSOProgress];//?? old v1 hv 4 // 2,2,2,4,4,3
        else npasses=7;//ws300rc1 else npasses=5;//2.29=5 from=4 ?? old v1 hv 5
    }
    else npasses=5;//ws300rc1 else npasses=4;//2.29=4 from=3 ?? old v1 hv 4

    //2.39 nzhsym=???    if(nzhsym<50) npasses=4;
    if (n4pas3int) npasses=5;//ws300rc1 if (n4pas3int) npasses=4;//2.40
    //qDebug()<<"npasses="<<npasses;

    for (int ipass = 1; ipass <= npasses; ++ipass)
    {//do ipass=1,npasses    //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
        //qDebug()<<"ft8b ipass======== Start"<<ipass;
        //qDebug()<<"1 Unpack==================77="<<nbadcrc<<ipass;
        for (int z = 0; z < 174; ++z)
        {
            if 		(ipass==1) llrz[z]=llra[z];
            else if (ipass==2) llrz[z]=llrb[z];
            else if (ipass==3) llrz[z]=llrc[z];
            else if (ipass==4) llrz[z]=llrd[z];
            else if (ipass==5) llrz[z]=llre[z];//ws300rc1
            // if (ipass > 4) dolu go pokriva
        }
        if (ipass<=5) //ws300rc1 if (ipass<=4) //4new if(ipass.le.4)
        {
            for (int z = 0; z < 174; ++z) apmask[z]=0;
            iaptype=0;
        }
        if (ipass > 5)//ws300rc1 if (ipass > 4) //4new if(ipass .gt. 4)  old if(ipass .gt. 3)  //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
        {
            //naptypes_{1,2,0,0},{2,3,0,0},{2,3,0,0},{3,4,5,6},{3,4,5,6},{3,1,2,0}
            for (int z = 0; z < 174; ++z) llrz[z]=llra[z];
            if (((ipass-5) % 2)==1)//if(mod(ipass-5,2).eq.1) llrz=llra
            {
                for (int z = 0; z < 174; ++z) llrz[z]=llra[z]; //qDebug()<<"1=="<<ipass;
            }
            if (((ipass-5) % 2)==0)////if(mod(ipass-5,2).eq.0) llrz=llrc
            {
                for (int z = 0; z < 174; ++z) llrz[z]=llrc[z]; //qDebug()<<"0=="<<ipass;
            }
            double maxval_llra_abs = fabs(llrz[0]);
            for (int z = 0; z < 174; ++z)
            {
                double llra_abs = fabs(llrz[z]);//ws300rc1 stop
                if (llra_abs>maxval_llra_abs) maxval_llra_abs=llra_abs;
            }
            double apmag = maxval_llra_abs*1.1;
            //iaptype=naptypes_2[nQSOProgress][ipass-(4+1)];//4new v1 4+1
            if (!lapcqonly) iaptype=naptypes_2[nQSOProgress][(ipass-(4+2))/2];//ws300rc1
            else iaptype=1;

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

            //qDebug()<<"type1"<<iaptype<<ncontest;
            //if(f1>2098 && f1<2110) qDebug()<<"f1-nfqso="<<f1<<fabs(f1-nfqso)<<"f1-nftx="<<fabs(f1-nftx);
            //if(ncontest.le.5 .and. iaptype.ge.3 .and. (abs(f1-nfqso).gt.napwid .and. abs(f1-nftx).gt.napwid) ) cycle
            //HV new=4
            if (cont_type<=4 && iaptype>=3 && (fabs(f1-nfqso)>napwid && fabs(f1-nftx)>napwid) ) continue;
            //if(ncontest.eq.6) cycle  ! No AP for Foxes     MSHV MAM have AP
            //if(ncontest.eq.7.and.f1.gt.950.0) cycle  ! Hounds use AP only for signals below 950 Hz  MSHV no Hounds
            if (iaptype>=2 && apsym[0]>1) continue;  //! No, or nonstandard, mycall if(iaptype>=2 && apsym(1).gt.1) cycle
            //if(ncontest.eq.7 .and. iaptype.ge.2 .and. aph10(1).gt.1) cycle
            if (iaptype>=3 && apsym[29]>1) continue; //! No, or nonstandard, dxcall if(iaptype>=3 && apsym(30).gt.1) cycle
            //qDebug()<<"type2"<<iaptype<<ncontest;
            for (int z = 0; z < 58; ++z) apsym[z]=2*apsym[z]-1;// apsym=2*apsym-1  //! Change from [0,1] to antipodal//apsym[58+5]               
            if (iaptype==1) //! CQ or CQ RU or CQ TEST or CQ FD
            {
                for (int z = 0; z < 174; ++z)//3*ND 174
                {
                    apmask[z]=0;
                    if (z<29)
                    {
                        apmask[z]=1;
                        llrz[z]=apmag*(double)mcq_ft8_2[z];
                        //no in MSHV if(ncontest.eq.7) llrz(1:29)=apmag*mcq(1:29)
                    }
                }
                apmask[74]=1;  //apmask(75:77)=1
                apmask[75]=1;
                apmask[76]=1;
                llrz[74]=apmag*(-1.0);//llrd(75:76)=apmag*(-1)
                llrz[75]=apmag*(-1.0);//llrd(75:76)=apmag*(-1)
                llrz[76]=apmag*(+1.0);//llrd(77)=apmag*(+1)
            }
            if (iaptype==2) //then //! MyCall,???,???
            {
                for (int z = 0; z < 174; ++z)
                    apmask[z]=0;
                if (cont_type==0 || cont_type==1)//hv new || cont_type==5
                {
                    for (int z = 0; z < 29; ++z)
                    {
                        apmask[z]=1;//apmask(1:29)=1
                        llrz[z]=apmag*(double)apsym[z];//llrd(1:29)=apmag*apsym(1:29)
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
                        llrz[z]=apmag*(double)apsym[z];//llrd(1:28)=apmag*apsym(1:28)
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
                        llrz[z]=apmag*(double)apsym[z];//llrd(1:28)=apmag*apsym(1:28)
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
                        llrz[z]=apmag*(double)apsym[z-1];//llrd(2:29)=apmag*apsym(1:28)
                    }
                    apmask[74]=1;//apmask(75:77)=1
                    apmask[75]=1;
                    apmask[76]=1;
                    llrz[74]=apmag*(-1.0);//llrd(75)=apmag*(-1)
                    llrz[75]=apmag*(+1.0);//llrd(76:77)=apmag*(+1)
                    llrz[76]=apmag*(+1.0);
                }
                // HOUND = MSHV no
                /*else if(ncontest.eq.7) then ! ??? RR73; MyCall <Fox Call hash10> ???
                apmask(29:56)=1  
                llrz(29:56)=apmag*apsym(1:28)
                apmask(57:66)=1
                llrz(57:66)=apmag*aph10(1:10)
                apmask(72:77)=1 
                llrz(72:73)=apmag*(-1)
                llrz(74)=apmag*(+1)
                llrz(75:77)=apmag*(-1)
                endif*/
            }
            if (iaptype==3) // ! MyCall,DxCall,???
            {
                for (int z = 0; z < 174; ++z) apmask[z]=0;//apmask=0
                //if(ncontest.eq.0.or.ncontest.eq.1.or.ncontest.eq.2.or.ncontest.eq.5.or.ncontest.eq.7) then
                if (cont_type==0 || cont_type==1 || cont_type==2)//||ncontest==7   || ncontest==5
                {
                    for (int z = 0; z < 58; ++z)
                    {
                        apmask[z]=1;//apmask(1:58)=1
                        llrz[z]=apmag*(double)apsym[z];//llrd(1:58)=apmag*apsym
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
                        if (z<56)
                            apmask[z]=1;//apmask(1:56)=1
                        if (z<28)
                            llrz[z]=apmag*(double)apsym[z];//llrd(1:28)=apmag*apsym(1:28)
                        if (z>28)
                            llrz[z-1]=apmag*(double)apsym[z];//llrd(29:56)=apmag*apsym(30:57)
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
                        if (z>0)
                            apmask[z]=1;//apmask(2:57)=1
                        if (z<28)
                            llrz[z+1]=apmag*(double)apsym[z];//llrd(2:29)=apmag*apsym(1:28)
                        if (z>28)
                            llrz[z]=apmag*(double)apsym[z];//llrd(30:57)=apmag*apsym(30:57)
                    }
                    apmask[74]=1;//apmask(75:77)=1
                    apmask[75]=1;
                    apmask[76]=1;
                    llrz[74]=apmag*(-1.0);//llrd(75)=apmag*(-1)
                    llrz[75]=apmag*(+1.0);//llrd(76:77)=apmag*(+1)
                    llrz[76]=apmag*(+1.0);
                }
            }
            //if(iaptype==5 && ncontest==7) continue;//cycle !Hound
            if (iaptype==4 || iaptype==5 || iaptype==6)
            {
                for (int z = 0; z < 174; ++z)
                    apmask[z]=0;//apmask=0
                //if(ncontest.le.5 || (ncontest.eq.7.and.iaptype.eq.6)) then
                if (cont_type<=4)//HV new=4
                {
                    for (int z = 0; z < 77; ++z)
                    {
                        apmask[z]=1;//apmask(1:77)=1   //! mycall, hiscall, RRR|73|RR73
                        if (z<58)
                            llrz[z]=apmag*(double)apsym[z];//llrd(1:58)=apmag*apsym
                    }
                    for (int z = 0; z < 19; ++z)
                    {
                        if (iaptype==4)
                            llrz[z+58]=apmag*(double)mrrr_ft8_2[z]; //llrd(59:77)=apmag*mrrr
                        if (iaptype==5)
                            llrz[z+58]=apmag*(double)m73_ft8_2[z]; //llrd(59:77)=apmag*m73
                        if (iaptype==6)
                            llrz[z+58]=apmag*(double)mrr73_ft8_2[z];//llrd(59:77)=apmag*mrr73
                    }
                }
                //HOUND = MSHV no
                /*else if(ncontest.eq.7.and.iaptype.eq.4) then ! Hound listens for MyCall RR73;...
                apmask(1:28)=1
                llrz(1:28)=apmag*apsym(1:28)
                apmask(57:66)=1
                llrz(57:66)=apmag*aph10(1:10)
                apmask(72:77)=1
                llrz(72:73)=apmag*(-1)
                llrz(74)=apmag*(1)
                llrz(75:77)=apmag*(-1)
                endif*/
            }
        }

        for (int z = 0; z < 174; ++z)
        {
            cw[z]=0;
            if (z<120) message91[z]=0;
        }

        dmin=0.0;
        int norder=2;
        int maxosd=2;   //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
        //if (ndepth<3) maxosd=1;//old if(ndepth.lt.3) maxosd=1
        if (ndepth==1) maxosd=-1; //2.42 ! BP only
        if (ndepth==2) maxosd=0;//ws300rc1  //2.42 ! uncoupled BP+OSD
        //if(ndepth.eq.3 .and. (abs(nfqso-f1).le.napwid .or. abs(nftx-f1).le.napwid .or. ncontest.eq.7))
        if (ndepth==3 && (fabs(nfqso-f1)<=napwid || fabs(nftx-f1)<=napwid /*|| ncontest==7*/))//2.42
            maxosd=2;
        if (nagain)//222 removed
        {
            norder=3;
            maxosd=1;
        }
        //all =int Keff=91;
        pomFt.decode174_91(llrz,maxosd,norder,apmask,message91,cw,nharderrors,dmin);//ntype, Keff,
        //if(f1>2098 && f1<2110) qDebug()<<"f1-nfqso="<<f1<<xdt<<nharderrors;
        /*if (nharderrors>=0)
        {
            for (int z = 0; z < 77; ++z)//message77=message91(1:77)
                message77[z]=message91[z];
        }*/


        message="";
        xsnr=-99.0;
        nbadcrc=1;//2.42   222=?
        if (nharderrors<0 || nharderrors>36) continue;//cycle
        int c_cw = 0;
        for (int z = 0; z < 174; z++)//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
        {
            if (cw[z]==0)// 3*ND=174
                c_cw++;
        }
        if (c_cw==174) continue;

        //int i3,n3;
        n3=4*message91[71] + 2*message91[72] + message91[73];//??? check need
        i3=4*message91[74] + 2*message91[75] + message91[76];//??? check need
        // old if (i3>4 || (i3==0 && n3>5)) continue; //cycle
        if (i3>5 || (i3==0 && n3>6)) continue; //2.39 EU VHF Contest
        if (i3==0 && n3==2) continue; //2.42 222 ignore old EU VHF Contest
        bool unpk77_success = false;
        message = TGenFt8->unpack77(message91,unpk77_success);  //qDebug()<<message<<unpk77_success;
        //qDebug()<<"100 Unpack=77--->"<<message<<i3<<n3<<unpk77_success<<f1;

        /*if(!unpk77_success || message.indexOf("/R")>-1 || message.mid(0,4)=="TU; ")//ws300rc1 if(!unpk77_success || index(msg37,'/R')>0 || msg37(1:4)=='TU; ')
        {
          	if(i3>=1 && i3<=3 && cont_type==0) continue; //cont_type=ncontest   	
        }*/

        if (!unpk77_success) continue; //cycle
        //if (nharderrors==21) qDebug()<<"100 Unpack=77="<<nbadcrc<<message<<unpk77_success;

        nbadcrc=0;  //! If we get this far: valid codeword, valid (i3,n3), nonquirky message.

        //int i4tone[120];
        TGenFt8->make_c77_i4tone(message91,i4tone);

        if (lsubtract) subtractft8(dd,i4tone,f1,xdt,false);//qDebug()<<"subtractft8"<<message<<xdt2<<f1<<ipass;

        double xsig=0.0;
        for (int i = 0; i < NN; ++i)//NN=79  c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
        {//do i=1,79
            //old variant v1 xsig=xsig+s2_[i][i4tone[i]]*s2_[i][i4tone[i]];
            //s8_/1000.0 = s2_  HV from v1.
            double s88 = s8_[i][i4tone[i]]*0.001;//1000.0;
            xsig=xsig+s88*s88; //double s2_[NN][8]; xsig=xsig+s2(itone(i),i)**2
            //int ios=fmod(i4tone[i]+4,7);//ios=mod(itone(i)+4,7)
            //xnoi=xnoi+s2_[i][ios]*s2_[i][ios]; //xnoi=xnoi+s2(ios,i)**2
        }

        if (xbase<=0.0) xbase=0.001;//no devide by zero
        xsnr=pomAll.db(xsig/xbase - 1.0) - 32.0 - 4.0;

        if (nsync<=10 && xsnr<-25.0)//ws300rc1 if (nsync<=10 && xsnr<-24.0)    //220rc2 !bail out, likely false decode
        {
            nbadcrc=1;
            return;
        }

        if (xsnr < -25.0) xsnr=-25.0;
        if (xsnr > 49.0)  xsnr=49.0; //2.31

        /*xsig=0.0 //rc3
        xnoi=0.0
        do i=1,79
           xsig=xsig+s8(itone(i),i)**2
           ios=mod(itone(i)+4,7)
           xnoi=xnoi+s8(ios,i)**2
        enddo
        xsnr=0.001
        xsnr2=0.001
        arg=xsig/xnoi-1.0 
        if(arg.gt.0.1) xsnr=arg
        arg=xsig/xbase/2.6e6-1.0
        if(arg.gt.0.1) xsnr2=arg
        xsnr=10.0*log10(xsnr)-27.0
        xsnr2=10.0*log10(xsnr2)-27.0
        if(.not.nagain) then
          xsnr=xsnr2
        endif
        if(xsnr .lt. -24.0) xsnr=-24.0*/

        return;
    }
}
void DecoderFt8::baseline(double *s,int nfa,int nfb,double *sbase)
{
    /*! Fit baseline to spectrum (for FT8)
    ! Input:  s(npts)         Linear scale in power
    ! Output: sbase(npts)    Baseline
    */
    double df=DEC_SAMPLE_RATE/3840.0;                    //!3.125 Hz
    int ia=fmax(0,int(nfa/df));
    int ib=int(nfb/df);
    int nseg = 10;
    int npct = 10;
    double t_s[1970];
    //double *t_s = new double[1920+20];
    double x[1010];
    double y[1010];
    double a[8];
    //qDebug()<<ia<<ib;
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

        for (int z = 0; z<1920-ja; ++z)
            t_s[z] = s[z+ja];
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
                if (k<999)
                    k++;
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
    for (int i = 0; i<1920; ++i)
        sbase[i]=0.0;
    for (int i = ia; i<ib; ++i)
    {//do i=ia,ib
        double t=i-i0;
        sbase[i]=a[0]+t*(a[1]+t*(a[2]+t*(a[3]+t*(a[4])))) + 0.65;
        //!     write(51,3051) i*df,s(i),sbase(i)
        //!3051 format(3f12.3)
    }
    //int hhh=int(1200.0/3.125);
    //qDebug()<<"xbase=="<<sbase[hhh-2]<<sbase[hhh-1]<<sbase[hhh]<<sbase[hhh+1]<<sbase[hhh+2];
}
void DecoderFt8::get_spectrum_baseline(double *dd,int nfa,int nfb,double *sbase)
{
    const int NSPS=1920;
    const int NFFT1=2*NSPS; //=3840
    const int NH1=NFFT1/2;//1920
    const int NF=93;
    const int NST=NFFT1/2;//=960
    const int NMAX=180000;//NMAX=15*12000
    double savg[NH1+50];
    double x[NFFT1+50];
    double complex cx[NH1+200];
    //double ss_[NF+10][NH1+50];//hv no needed  (NH1,NF)

    if (first_ft8sbl)
    {
        first_ft8sbl=false;
        for (int i = 0; i<NFFT1; ++i) window_ft8sbl[i]=0.0;
        pomFt.nuttal_window(window_ft8sbl,NFFT1);
        //qDebug()<<window_ft8sbl[10]<<window_ft8sbl[1000]<<window_ft8sbl[2000]<<window_ft8sbl[3000];
        double summ = 0.0;
        for (int i = 0; i<NFFT1; ++i) summ+=window_ft8sbl[i];
        summ = summ*(double)NSPS*2.0/300.0;
        //qDebug()<<summ;
        for (int i = 0; i<NFFT1; ++i) window_ft8sbl[i]/=summ;//window=window/sum(window)*NSPS*2/300.0
        //qDebug()<<window_ft8sbl[10]<<window_ft8sbl[100]<<window_ft8sbl[200]<<window_ft8sbl[300];
    }

    //! Compute symbol spectra, stepping by NSTEP steps.
    for (int i = 0; i<NH1+2; ++i) savg[i]=0.0;
    //double df=12000.0/(double)NFFT1;
    for (int j = 0; j<NF; ++j)
    {//do j=1,NF
        int ia=j*NST;//(j-1)*NST + 1
        int ib=ia+NFFT1-1;//ib=ia+NFFT1-1
        //if (ia+NFFT1-1>178000) qDebug()<<ia+NFFT1-1;
        if (ib>NMAX-1) break;// if(ib.gt.NMAX) exit  //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
        for (int z = 0; z < NFFT1; ++z)
            x[z]=dd[ia+z]*window_ft8sbl[z]; //x=dd(ia:ib)*window
        //if (ia+NFFT1-1>178000) qDebug()<<ia+NFFT1-1;
        //qDebug()<<x[10]<<x[1000]<<x[2000]<<x[3000];
        f2a.four2a_d2c(cx,x,NFFT1,-1,0,decid);  //call four2a(x,NFFT1,1,-1,0)              //!r2c FFT
        for (int i = 0; i < NH1; ++i)
        {//do i=1,NH1
            //ss_[j][i]=cabs(cx[i])*cabs(cx[i]);//fin s(1:NH1,j)=abs(cx(1:NH1))**2
            //savg[i]+=ss_[j][i];//savg=savg + s(1:NH1,j)                   //!Average spectrum
            savg[i]+=cabs(cx[i])*cabs(cx[i]);
        }
    }
    /*qDebug()<<"-------------------------------------";
    QString sss;
    for (int i = 0; i<NH1; ++i) sss.append(QString("%1 ").arg(savg[i],0,'f',1));
    qDebug()<<sss;*/
    if (nfa<100)  nfa=100;
    if (nfb>4910) nfb=4910;
    baseline(savg,nfa,nfb,sbase);
}
void DecoderFt8::sync8(double *dd,double nfa,double nfb,double syncmin,double nfqso,
                       double s_[402][1970],double candidate[2][620],int &ncand,double *sbase)
{
    const int NSPS=1920;
    int NSTEP=NSPS/4;//=480
    const int NFFT1=2*NSPS;
    int NMAX=15*DEC_SAMPLE_RATE;
    int NHSYM=NMAX/NSTEP-3;//372
    const int NH1=NFFT1/2;       //NH1=1920
    const int max_c0 = 900;//2.69 old=400;//2.2.0=500 260r5=1000
    const int max_c_ = 600;//2.69 old=249 max 254 260r5=600

    //! Search over +/- 1.5s relative to 0.5s TX start time.
    //parameter (JZ=38)
    //old 72/2 int JZ=38;
    //! Search over +/- 2.5s relative to 0.5s TX start time.
    //parameter (JZ=62) new
    const int JZ=62;//62;

    //! Compute symbol spectra, stepping by NSTEP steps.
    //double savg=0.0;
    double tstep=(double)NSTEP/DEC_SAMPLE_RATE;
    double df=DEC_SAMPLE_RATE/(double)NFFT1;                            //!3.125 Hz
    double fac=1.0/300.0;

    double x[NFFT1+20];//=3840     //real x(NFFT1)
    //double *x = new double[NFFT1+20];
    double complex cx[NFFT1+20]; //2.09 error-> cx[NH1+20];       //complex cx(0:NH1)

    // old 76 double sync2d[1920+50][76+20];  //real sync2d(NH1,-JZ:JZ) -JZ=-38 JZ=+38 = 76
    // old 76 double (*sync2d)[76+20]=new double[1920+50][76+20];
    // old 76 int offset_sync2d = JZ+10;
    //static double sync2d[1920+50][124+20];  //real sync2d(NH1,-JZ:JZ) -JZ=-62 JZ=+62 = 124
    //double (*sync2d)[124+20]=new double[1920+50][124+20]; // largest 2d array need create hv
    double (*sync2d)[144] = new double[1970][144];
    //for(auto q=0; q<1970; q++) memset(&sync2d[q][0], 0, 144*sizeof(double));

    int offset_sync2d = JZ+10;
    //double sync2d_[1920+50][124+20];

    double red[NH1+10];
    double red2[NH1+10];//2.69
    int jpeak[NH1+10];
    int jpeak2[NH1+10];//2.69
    int indx[NH1+10];
    int indx2[NH1+10];//2.69
    double candidate0[3][max_c0+100];//2.00 HV no inaf ->200+5 need 300
    //double savg[NH1+10];
    //pomAll.zero_double_beg_end(savg,0,NH1+2);

    int ia = 0; //qDebug()<<"sync8-  0"<<dd[5000]<<dd[6000]<<dd[8000]<<dd[9000]<<dd[10000]<<dd[20000]<<dd[30000];
    for (int j = 0; j < NHSYM; ++j)
    {//do j=1,NHSYM
        ia=j*NSTEP;    //ia=(j-1)*NSTEP + 1
        //ib=ia+NSPS;      //ib=ia+NSPS-1
        for (int z = 0; z < NSPS; ++z) x[z]=fac*dd[ia+z]*0.01;//hv double coeficient        //x(1:NSPS)=fac*dd(ia:ib)
        //if (((NHSYM-1)*NSTEP+NSPS-1)>180000) qDebug()<<(NHSYM-1)*NSTEP+NSPS-1;
        for (int z = NSPS; z < NFFT1+1; ++z) x[z]=0.0;            //x(NSPS+1:)=0.
        f2a.four2a_d2c(cx,x,NFFT1,-1,0,decid);  //call four2a(x,NFFT1,1,-1,0)              //!r2c FFT
        for (int i = 0; i < NH1; ++i)
        {//do i=1,NH1
            s_[j][i]=pomAll.ps_hv(cx[i]);    //s(i,j)=real(cx(i))**2 + aimag(cx(i))**2
            //if(j==150)
            //qDebug()<<"sync8 t="<<s_[j][i];
            //if(j==0)//1.76 here zero_double
            //savg[i]=0.0;
            //savg[i]+=s_[j][i];//!Average spectrum
        }   //qDebug()<<"sync_abc<<sync_bc"<<s_[j][100];
    }

    int nfa1 = (int)nfa;
    int nfb1 = (int)nfb;
    if ((nfb-nfa)<2000.0) //2000 HV correction
    {
        double ftmp = nfa + ((nfb-nfa)/2.0);
        nfa1 = (int)(ftmp - 1000.0);
        nfb1 = (int)(ftmp + 1000.0);
        if (nfa1<100)
        {
            nfa1 = 100;
            nfb1 = 100 + 2000;
        }
        if (nfb1>5000)
        {
            nfb1 = 5000;
            nfa1 = 5000 - 2000;
        }
    } //qDebug()<<"1="<<nfa<<nfb<<nfb-nfa<<nfa1<<nfb1<<nfb1-nfa1;
    get_spectrum_baseline(dd,nfa1,nfb1,sbase);

    /*int ib=int(nfb/df);
    double xavg = 0.0;
    if (decid>0)
    {
    	int ia0=fmax(0,int(nfa/df));
    	for (int i = ia0; i < ib; ++i) xavg+=sbase[i]; 
    	xavg /= (double)(ib-ia0);  
    	if (xavg<-1.0) qDebug()<<"corr"<<decid<<xavg;
    	else qDebug()<<"NO"<<decid<<xavg;    
    }*/

    int corrt = 0;
    if (decid>0) corrt = 17;       //2.72 5=16Hz 17=53Hz 18=56Hz
    ia=fmax(0,(int(nfa/df)-corrt));//2.72 corrt for threads //ia=fmax(0,int(nfa/df));

    int ib=int(nfb/df); //qDebug()<<"sync8="<<nfa<<ia<<ib;
    int nssy=NSPS/NSTEP;//! # steps per symbol =4
    int nfos=NFFT1/NSPS;//! # frequency bin oversampling factor =2
    int jstrt=0.5/tstep;

    int k = 0;
    for (int i = 0; i < 3; ++i)
    {
        for (int j = 0; j < max_c0; ++j) candidate0[i][j]=0.0;
    }

    for (int i = ia; i < ib; ++i)
    {
        for (int j = -JZ; j < JZ+1; ++j)
        {
            double ta= 0.0;
            double tb= 0.0;
            double tc= 0.0;
            double t0a=0.0;
            double t0b=0.0;
            double t0c=0.0;
            for (int n = 0; n < 7; ++n)//do n=0,6
            {
                int m=(int)((double)(j+nssy*n)+jstrt); //k=j+jstrt+nssy*n //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
                if (m>=0 && m<NHSYM) //if(k.ge.1.and.k.le.NHSYM) then
                {
                    ta+=s_[m][i+nfos*icos7_2[n]]; //ta=ta + s(i+nfos*icos7(n),k)
                    for (int z = i; z < i+nfos*6+1; z+=nfos) t0a+=s_[m][z];  //???? t0a=t0a + sum(s(i:i+nfos*6:nfos,k))
                }
                tb+=s_[m+nssy*36][i+nfos*icos7_2[n]];   //  tb=tb + s(i+nfos*icos7(n),k+nssy*36)
                for (int z = i; z < i+nfos*6+1; z+=nfos) t0b+=s_[m+nssy*36][z]; //??? t0b=t0b + sum(s(i:i+nfos*6:nfos,k+nssy*36))
                if (m+nssy*72<NHSYM)   //if(k+nssy*72.le.NHSYM) then
                {
                    tc+=s_[m+nssy*72][i+nfos*icos7_2[n]]; //tc=tc + s(i+nfos*icos7(n),k+nssy*72)
                    for (int z = i; z < i+nfos*6+1; z+=nfos) t0c+=s_[m+nssy*72][z];//??? t0c=t0c + sum(s(i:i+nfos*6:nfos,k+nssy*72))
                }
            }
            double t=ta+tb+tc;
            double t0=t0a+t0b+t0c;
            t0=(t0-t)/6.0;
            double sync_abc; //stop devide by 0 HV double sync_abc=t/t0; if (t0==0.0) qDebug()<<"abc"<<decid<<t;
            if (t0==0.0) sync_abc=0.0;
            else sync_abc=t/t0;

            t=tb+tc;
            t0=t0b+t0c;
            t0=(t0-t)/6.0;
            double sync_bc; //stop devide by 0 HV double sync_bc=t/t0; if (t0==0.0) qDebug()<<"bc"<<decid<<t;
            if (t0==0.0) sync_bc=0.0;
            else sync_bc=t/t0;

            //if (i>=ia) sync2d[i][j+offset_sync2d]=fmax(sync_abc,sync_bc); //sync2d(i,j)=fmax(sync_abc,sync_bc);
            sync2d[i][j+offset_sync2d]=fmax(sync_abc,sync_bc); //sync2d(i,j)=fmax(sync_abc,sync_bc);
            //qDebug()<<sync_abc<<sync_bc<<fmax(sync_abc,sync_bc);
        }
    }

    for (int z = 0; z < NH1; ++z)
    {
        red[z] =0.0;
        red2[z]=0.0; //2.69
    }  //qDebug()<<"symbol="<<((2.5/0.16)*4)<<((0.65/0.16)*4);
    for (int i = ia; i < ib; ++i)//2.69
    {
        int j0=(pomAll.maxloc_da_beg_to_end(sync2d[i],offset_sync2d-18,offset_sync2d+18)-offset_sync2d);//hv=Search over +/-0.6s = +-16 //260r5= +-10
        jpeak[i]=j0;
        red[i]=sync2d[i][j0+offset_sync2d];
        j0=(pomAll.maxloc_da_beg_to_end(sync2d[i],offset_sync2d-JZ,offset_sync2d+JZ)-offset_sync2d);//Search over +/-2.5s = +-62
        jpeak2[i]=j0;
        red2[i]=sync2d[i][j0+offset_sync2d];
    }

    int iz=ib-ia; //iz=ib-ia+1
    if (iz>1920) iz=1920; //NH1=1920

    double t_red[2048];//2.12 (10Hz to 6000Hz = 1917)
    double t_red2[2048];
    for (int i = 0; i < iz; ++i)
    {
        t_red[i]  = red[i+ia];
        t_red2[i] = red2[i+ia];
        indx[i] =0;
        indx2[i]=0;
    }
    indx[iz]   =0;
    indx[iz+1] =0;
    indx2[iz]  =0;
    indx2[iz+1]=0;
    if (iz>0)
    {
        pomAll.indexx_msk(t_red, iz-1,indx);
        pomAll.indexx_msk(t_red2,iz-1,indx2);
    }
    int npctile=(int)(0.40*(double)(iz));
    if (npctile<0) // something is wrong; bail out
    {
        ncand=0;
        return;
    }
    int ibase =indx[npctile] + ia;  //hv 1.48 0 mybe -1 tested ibase=indx(nint(0.40*iz)) - 1 + ia
    int ibase2=indx2[npctile] + ia;
    if (ibase<0) ibase=0;//2.12
    if (ibase>NH1-1) ibase=NH1-1;//2.12
    if (ibase2<0) ibase2=0;//2.69
    if (ibase2>NH1-1) ibase2=NH1-1;//2.69

    double base=red[ibase];
    double base2=red2[ibase2];
    if (base <=0.001) base =0.001;//no devide by 0
    if (base2<=0.001) base2=0.001;

    for (int i = 0; i < NH1; ++i)
    {
        red[i] =red[i] /base;  //no devide by 0
        red2[i]=red2[i]/base2;
    }

    //int to_iz = iz;
    //if (to_iz > max_c0) to_iz = max_c0;//max_c0;
    //qDebug()<<"FULL==="<<max_c0<<"iz="<<iz<<indx[(iz-1)];
    //for (int i = 0; i < to_iz; ++i)//2.00
    for (int i = 0; i < iz; ++i)//2.69
    {
        int n=ia + indx[(iz-1)-i]; //indx[1920]  tested -1 ->1.69 n=ia + indx(iz+1-i) - 1
        //double ff = (double)n*df;
        //if (ff<nfa-50.0 || ff>nfb) continue;
        if (red[n]>=syncmin)
        {
            candidate0[0][k]=(double)n*df;
            candidate0[1][k]=(double)(jpeak[n])*tstep;//(jpeak(n)-0.5)  2.34 tested jpeak[n]-1 to jpeak[n] ///candidate0(2,k)=(jpeak(n)-1)*tstep
            candidate0[2][k]=red[n];  //if (n>1500) qDebug()<<"n="<<n;
            k++;
        }
        if (k>=max_c0) break;
        if (abs(jpeak2[n]-jpeak[n])==0) continue;//if(abs(jpeak2(n)-jpeak(n)).eq.0) cycle
        //n=ia + indx2[(iz-1)-i];
        if (red2[n]>=syncmin)  //exit if(red(n).lt.syncmin.or.isnan(red(n)).or.k.eq.MAXPRECAND) exit
        {
            candidate0[0][k]=(double)n*df;
            candidate0[1][k]=(double)(jpeak2[n])*tstep;//(jpeak(n)-0.5)  2.34 tested jpeak[n]-1 to jpeak[n] ///candidate0(2,k)=(jpeak(n)-1)*tstep
            candidate0[2][k]=red2[n];  //if (n>1500) qDebug()<<"n="<<n;
            k++;
        }
        if (k>=max_c0) break;
    }
    ncand=k;

    /*for (int i = 0; i < k; ++i)
    {
    	if (candidate0[0][i]>2025.0 && candidate0[0][i]<2030.0) 
    	{
    		qDebug()<<"1FULL="<<candidate0[0][i]<<candidate0[2][i]<<candidate0[1][i]<<k;
    		//candidate0[1][i]=-0.44;    
    	}   	
    }*/

    //! Save only the best of near-dupe freqs.
    for (int i = 0; i < ncand; ++i)//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    {//do i=1,ncand
        if (i>=1) //if(i.ge.2) then
        {
            for (int j = 0; j < i; ++j)
            {//do j=1,i-1
                double fdiff=fabs(candidate0[0][i])-fabs(candidate0[0][j]);//fdiff=abs(candidate0(1,i))-abs(candidate0(1,j))
                double tdiff=fabs(candidate0[1][i]-candidate0[1][j]);
                if (fabs(fdiff)<4.0 && tdiff<0.08)//one step=0.04  if(abs(fdiff).lt.4.0.and.tdiff.lt.0.04) then
                {
                    if (candidate0[2][i]>=candidate0[2][j]) candidate0[2][j]=0.0; //if(candidate0(3,i).ge.candidate0(3,j)) candidate0(3,j)=0.
                    if (candidate0[2][i]<candidate0[2][j]) candidate0[2][i]=0.0; //if(candidate0(3,i).lt.candidate0(3,j)) candidate0(3,i)=0.
                } //if (tdiff>0.04) qDebug()<<tdiff;
            }
        }
    }

    double maxval_s = s_[0][0];
    for (int i = 0; i < 372; ++i)//maxval -> s_[372+20][1920+20]
    {
        for (int j = 0; j < 1920; ++j)//maxval(s)
        {
            if (s_[i][j]>maxval_s) maxval_s = s_[i][j];
        }
    }
    if (maxval_s<=0.001) maxval_s = 0.001;//2.68 0.001, 2.66 0.01 no devide by zero

    fac=20.0/maxval_s;
    for (int i = 0; i < 372; ++i)//maxval -> s_[372+20][1920+20]
    {
        for (int j = 0; j < 1920; ++j) s_[i][j]=fac*s_[i][j];
    }

    //! Sort by sync
    //!  call indexx(candidate0(3,1:ncand),ncand,indx)
    //! Sort by frequency
    for (int i = 0; i < ncand+2; ++i) indx[i] = 0;//2.37 NH1=1920
    if (ncand>0) pomAll.indexx_msk(candidate0[2],ncand-1,indx); //  Sort by sync //call indexx(candidate0(3,1:ncand),ncand,indx)
    //freq pomAll.indexx_msk(candidate0[0],ncand-1,indx);//call indexx(candidate0(1,1:ncand),ncand,indx)

    k=0;
    //! Place candidates within 10 Hz of nfqso at the top of the list
    for (int i = 0; i < ncand; ++i)
    {
        if (fabs(candidate0[0][i]-nfqso)<=10.0 && candidate0[2][i]>=syncmin && candidate0[1][i]>=-2.5)  //if( fabs( candidate0(1,i)-nfqso ).le.10.0 .and. candidate0(3,i).ge.syncmin ) then
        {
            candidate[0][k]=candidate0[0][i];//candidate(1:3,k)=candidate0(1:3,i) && candidate0[1][i]>=-2.5
            candidate[1][k]=candidate0[1][i];
            //candidate[2][k]=candidate0[2][i];
            candidate0[2][i]=0.0; //hv null for next loop candidate0(3,i)=0.0
            //k=k+1
            if (k<max_c_) k++;
            else break;
        }
    }
    for (int i = ncand-1; i>= 0; --i)
    {
        int j=indx[i]; //if (j>max_c0/2-2) qDebug()<<j;
        if (candidate0[2][j] >= syncmin && candidate0[1][i]>=-2.5) //if( candidate0(3,j) .ge. syncmin ) then
        {
            candidate[1][k]=candidate0[1][j];//candidate(2:3,k)=candidate0(2:3,j) && candidate0[1][j]>=-2.5
            //candidate[2][k]=candidate0[2][j];
            candidate[0][k]=fabs(candidate0[0][j]);//candidate(1,k)=abs(candidate0(1,j))
            //k=k+1
            //if(k.gt.maxcand) exit
            if (k<max_c_) k++;
            else break;
        }
    }
    ncand=k; //if (k>300) qDebug()<<"2FULL==="<<max_c_<<">"<<ncand;
    delete [] sync2d;
}
void DecoderFt8::ft8apset(QString mycall12,QString hiscall12,int *apsym2)
{
    int i3=0;
    int n3=0;
    bool c77[100];
    for (int i = 0; i < 78; ++i) c77[i]=0;

    if (mycall12.isEmpty())
    {
        for (int i = 0; i < 58; ++i) //	apsym2[58+5]
            apsym2[i]=0;
        apsym2[0]=99;
        apsym2[29]=99;
        return;
    }
    bool nohiscall=false;
    //hiscall=hiscall12;
    if (hiscall12.isEmpty())//if(len(trim(hiscall)).eq.0) then
    {
        hiscall12="LZ2ABC";//"K9ABC";
        nohiscall=true;
    }
    /*if needed for HOUND   MSHV no HOUND Activity
    call save_hash_call(hc13,n10,n12,n22)
    write(c10,'(b10.10)') iand(n10,Z'3FF') 
    read(c10,'(10i1.1)',err=1) aph10   <-- int n10 = BinToInt32(aph10,0,10);
    aph10=2*aph10-1*/

    //! Encode a dummy standard message: i3=1, 28 1 28 1 1 15
    //!
    QString msgs2;
    if (pomAll.isStandardCall(mycall12.trimmed())) msgs2.append(mycall12.trimmed()); //2.61
    else msgs2.append("<"+mycall12.trimmed()+">");   //2.61

    msgs2.append(" ");
    msgs2.append(hiscall12.trimmed());
    msgs2.append(" ");
    msgs2.append("RRR"); //qDebug()<<"msg"<<msgs2;

    TGenFt8->pack77(msgs2,i3,n3,c77);  //call pack77(msg,i3,n3,c77)

    // ??? no need this
    /*bool unpk77_success;
    QString msgchk = TGenFt8->unpack77(c77,unpk77_success); //call unpack77(c77,1,msgchk,unpk77_success)
    if(i3!=1 || (msgs2!=msgchk) || !unpk77_success) return;*/

    if (i3!=1)
    {
        for (int i = 0; i < 58; ++i) //	apsym2[58+5]
            apsym2[i]=0;
        apsym2[0]=99;
        apsym2[29]=99;
        return;
    }

    //read(c77,'(58i1)',err=1) apsym(1:58)
    for (int i = 0; i < 58; ++i)
        apsym2[i] = c77[i];
    if (nohiscall) apsym2[29]=99;
    return;
    //c1:
    //??????????????????????
    for (int i = 0; i < 58; ++i) //	apsym2[58+5]
        apsym2[i]=0;
    apsym2[0]=99;
    apsym2[29]=99;
    return;
}
void DecoderFt8::SetNewP(bool f)
{
    f_new_p = f;
}

static bool s_3intFt8_d_ = true;//2.51 default
void DecoderFt8::Decode3intFt(bool f)//2.39 remm
{
    s_3intFt8_d_ = f; //qDebug()<<"DecoderFt8 s_3intFt_d_="<<s_3intFt_d_;
}
int DecoderFt8::ft8_even_odd(QString s)//res=0 (even first), or res=1 (odd second)
{
    bool res = 0;
    if (s.count()<6) return 0; //2.70 protection
    int time_ss =  s.midRef(4,2).toInt();//get seconds 120023
    int time_mm =  s.midRef(2,2).toInt();//get min 120023
    int time_p = (time_mm*60)+time_ss;
    //int ntrperiod = 15;
    time_p = time_p % (15*2); //j=mod(nutc/5,2)
    if (time_p<15) res = 0;
    else res = 1;
    return res;
}
void DecoderFt8::ft8_a7_save(QString nutc,double dt,double f,QString msg)
{
    int nwords = 0;
    QString w[19+8];

    if (msg.indexOf("/")>-1 || msg.indexOf("<")>-1) return;

    msg.append(" ");//for any case for label->c5
    for (int z = 0; z<19; ++z) w[z]="             ";//13 blinks char
    TGenFt8->split77(msg,nwords,w);
    if (nwords<1) return;
    if (w[0].mid(0,3)=="CQ_") return;//???

    int j=ft8_even_odd(nutc);
    jseq=j;

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
    TGenFt8->split77(s,nwords,w);
    for (int z = 0; z<ndec[0][j]; ++z)
    {
        //hv protect from dupe decode in same period
        if (f0[0][j][z]<=-98.0) continue;
        int i2=msg0[0][j][z].indexOf(" "+w[1].trimmed());
        if (fabs(f-f0[0][j][z])<=3.0 && i2>=2) f0[0][j][z]=-98.0;//Flag as "do not use" for a potential a7 decode
    }
    if (i>MAXDEC-2) return;
    ndec[1][j]++; //qDebug()<<"Save="<<j<<msg<<ndec[1][j];
}
void DecoderFt8::ft8_a7d(double *dd0,bool &newdat,QString call_1,QString call_2,QString grid4,double &xdt,
                         double &f1,double xbase,int &nharderrors,double &dmin,QString &message,double &xsnr)
{
    //const int MAXMSG=206;//= -50 to +49
    const int MAXMSG=158;//= -26 to +49
    //const int MAXMSG=140;//= -26 to +40
    //const int MAXMSG=120;//= -26 to +30
    //const int MAXMSG=110;//= -26 to +25
    //const int MAXMSG=100;//= -26 to +20

    bool std_1,std_2;
    //int NDOWN=60;
    //double complex cd0[3350];//3200+100  __attribute__((aligned(32)))
    //pomAll.zero_double_comp_beg_end(cd0,0,3300);//3200+100
    const int NS=21;//
    const int NN=NS+58;//
    //double complex ctwk[32+5];
    //double a[5];
    //int NP2=2812;
    //double complex csymb[40];//32+8
    double s8_[NN][8]; //real s1(0:7,ND),s2(0:7,NN)
    //double complex cs_[NN][8]; //complex cs(0:7,NN)
    //double s2[512];
    double bmeta[174];
    double bmetb[174];
    double bmetc[174];
    double bmetd[174];
    double bmete[174];//ws300rc1
    double llra[174];
    double llrb[174];
    double llrc[174];
    double llrd[174];
    //double rcw[174];
    int itone[105];//NN
    bool hdec[178];
    bool nxor[178];
    double dmm[MAXMSG+50];
    QString msgbest = "";
    QString msgsent = "";
    double pbest;

    //call_1="CQ"; call_2="LZ444HV"; grid4="    ";
    std_1 = pomAll.isStandardCall(call_1.trimmed());
    if (call_1=="CQ") std_1=true;
    std_2 = pomAll.isStandardCall(call_2.trimmed());
    //qDebug()<<"     IN================="<<call_1<<call_2<<grid4<<"std"<<std_1<<std_2;

    int nbadcrc = 1;
    nharderrors=-1;
    int nsync = 0;
    ft8_downs_sync_bmet(dd0,true,newdat,f1,xdt,nbadcrc,nsync,s8_,bmeta,bmetb,bmetc,bmetd,bmete,2,3);
    /*if (!ft8_downs_sync_bmet(dd0,true,newdat,f1,xdt,nbadcrc,nsync,s8_,bmeta,bmetb,bmetc,bmetd))
    {
       	if (nsync<2) return;
    }*/

    double scalefac = 2.83;//scalefac=2.83 double ss=0.85;//hv tested->85-86    0.84;//0.84
    for (int z = 0; z < 174; ++z)
    {
        llra[z]=scalefac*bmeta[z];
        llrb[z]=scalefac*bmetb[z];
        llrc[z]=scalefac*bmetc[z];
        llrd[z]=scalefac*bmetd[z];
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
        TGenFt8->pack77(msg,i3,n3,c77);
        TGenFt8->make_c77_i4tone_codeword(c77,itone,cw);
        if (msg.startsWith("QU1RK ")) msgsent = msg;
        else
        {
            bool unpk77_success = false;
            msgsent = "";
            msgsent = TGenFt8->unpack77(c77,unpk77_success);
        }
        //if (msgsent!=msg.trimmed()) qDebug()<<"---------------"<<i<<msg<<msgsent; //no-std-call problem  "CQ <TM22KPW> R-26"
        //if (msgsent.isEmpty()) msgsent = "QU1RK ";

        //for (int z= 0; z < 176; ++z) rcw[]=2*cw-1 //rcw=2*cw-1
        /*double pow0=0.0;
        for (int z= 0; z < 79; ++z)//double s8_[NN][8];
        {
            pow0+=s8_[z][itone[z]]*s8_[z][itone[z]];  //pow=pow+s8_(itone[i],i)**2
        }*/
        double pow0=0.0;
        for (int z = 0; z < 79; ++z)
        {
            double s88 = s8_[z][itone[z]]*0.001;//1000.0; //s8_/1000.0 = s2_  HV from v1.
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
    /*int pos = pomAll.minloc_da(dmm,count_msg);
    dmm[pos] = 1.e30;
    pos = pomAll.minloc_da(dmm,count_msg);*/
    double dmin2 = dmm[pos];

    /*xsnr=-24.0;
    double arg=pbest/xbase/3.0e6 - 1.0;
    if (arg>0.0) xsnr=fmax(-24.0,pomAll.db(arg)-27.0);*/
    if (xbase<=0.0) xbase=0.001;
    xsnr=pomAll.db(pbest/xbase - 1.0) - 36.0;
    if (xsnr < -25.0) xsnr=-25.0;
    if (xsnr > 49.0)  xsnr=49.0;

    message=msgbest; //NN=79  c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    msgbest += "xxxxxx";//2.70 protection
    if (dmin==0.0) dmin=0.0001;//devide by zero
    if (dmin>100.0 || dmin2/dmin<1.27) nharderrors=-1;
    if (msgbest.mid(0,3)=="CQ " && std_2 && grid4=="    ") nharderrors=-1;
    if (msgbest.mid(0,6)=="QU1RK " || message.isEmpty()) nharderrors=-1;
    /*if (dmin2/dmin<1.4 && nharderrors>36)//HV add   //if(nharderrors>=0 && nharderrors<=44 && dmin<=80.0)
    {
        QString s = "";//dmin>100.0 ||
        if (nharderrors>95) s = "<- stop";//dmin2/dmin<1.3 ||
        qDebug()<<message.leftJustified(20,' ')
        <<QString("%1").arg(dmin,0,'f',1)+">100.0"<<"-"
        <<QString("%1").arg(dmin2/dmin,0,'f',2)+"<1.27"<<"-"
        <<QString("%1").arg(nharderrors)+">95"<<s; //<<"Sync="<<nsync
        if (nharderrors>95) nharderrors=-1;//dmin2/dmin<1.3 ||
    }*/
    if (nharderrors>95) nharderrors=-1;
}
void DecoderFt8::gen_ft8cwaveRxAp8(int *i4tone,double f_tx,double complex *cwave)
{
    //////////////////////// GFSK MODULATOR ///////////////////////////////////////////
    //const int NTAB=65536;
    int nsym=79;
    int nsps=32; //for 200.0   for 12000=1920 for=48000->4*1920;
    int nwave=nsym*nsps;//max rx=2528
    //static double dphi[155570];
    double hmod=1.0;
    double dt=1.0/200.0;//for=200.0 for RX=12000 for tx=48000

    //! Compute the smoothed frequency waveform.
    //! Length = (nsym+2)*nsps samples, zero-padded (nsym+2)*nsps TX=215040 RX=53760
    double dphi_peak=twopi*hmod/(double)nsps;
    double *dphi = new double[3000];  //max rx=2592 dphi=79*32=2528 + 32*2=2592 //max rx=155520    dphi=79*1920=151680 + 1920*2=155520

    for (int i= 0; i < 2700; ++i) dphi[i]=0.0;

    for (int j= 0; j < nsym; ++j)
    {
        int ib=j*nsps;
        for (int i= 0; i < 3*nsps; ++i)//5760
            dphi[i+ib] += dphi_peak*pulse_ft8_rxAp8[i]*(double)i4tone[j];
    }

    int bgn =nsym*nsps;//=151680
    for (int i= 0; i < 2*nsps; ++i)//=3840
    {
        dphi[i]+=dphi_peak*i4tone[0]*pulse_ft8_rxAp8[i+nsps];
        dphi[i+bgn]+=dphi_peak*i4tone[nsym-1]*pulse_ft8_rxAp8[i];
    }

    double ofs = twopi*f_tx*dt;
    double phi=0.0;
    for (int j=0; j < nwave; ++j)//for (int j=nsps; j < nsps+nwave-1; ++j)//=151680   do j=nsps,nsps+nwave-1
    {
        //2.39 old cwave[j]=(cos(phi)+sin(phi)*I);
        int i=(int)(phi*65536.0/twopi); //16777216.0  65536.0
        //if (i<0 || i>65535) qDebug()<<i;
        if (i<0) 	 i=0;
        if (i>65535) i=65535;
        cwave[j]=ctab8_[i];
        phi=fmod(phi+dphi[j+nsps]+ofs,twopi);
    }

    int nramp=(int)((double)nsps/8.0);
    for (int i = 0; i < nramp; ++i) cwave[i]*=(1.0-cos(twopi*(double)i/(2.0*nramp)))/2.0;
    int k2=nsym*nsps-nramp+1; //k1=nsym*nsps-nramp+1   //k2=(nsym+1)*nsps+1;
    for (int i = 0; i < nramp; ++i) cwave[i+k2]*=(1.0+cos(twopi*(double)i/(2.0*nramp)))/2.0;//i+k1-nsps
    //qDebug()<<"nsamp="<<k2+nramp-1;
    delete [] dphi;
}
void DecoderFt8::ft8_a8d(double *dd,QString mycall,QString dxcall,QString dxgrid,double f1a,double &xdt,
                         double &fbest,double &xsnr,double &plog,QString &msgbest)
{
    //! List decoding for FT8, activated only at nfqso (Rx Freq) +/- 10 Hz and when
    //! DxCall and DxGrid are populated. Returns xdt, fbest, and msgbest.
    const int NZZ=3200;               //!Length of downsampled arrays
    const int NFFT=NZZ;
    const int NH=NFFT/2;//=1600
    const int NN=79;                  //!Total channel symbols (79)
    const int NSPS=1920;              //!Samples per symbol at 12000 S/s
    const int NDOWN=60;
    const int NWAVE=NN*NSPS/NDOWN;    //!Length of generated cwave = 2528
    const int NMSGS=206;
    const int offset_s = NH+4;

    double complex cd[NZZ+512];//3350 double complex *cd = new double complex[NZZ+512];//
    double complex cd0[NZZ+512];//!All tones in cd nominally shifted to 0 Hz double complex *cd0= new double complex[NZZ+512];//
    double f1=f1a;
    bool newdat=true;
    //! Mix from f1 to baseband and downsample from dd into cd (complex, 200 Hz sampling)
    for (int i= 0; i < NZZ+100; ++i) cd[i]=0.0+0.0*I;
    ft8_downsample(dd,newdat,f1,cd);//ft8_downsample(dd,newdat,f1,cd)
    double fac=1.e-6;
    double fsd=200.0;//!Downsampled rate (Hz)
    //double bt=2.0; //!Gaussian smoothing parameter
    double dt=1.0/fsd;
    double df=200.0/(double)NFFT;
    double s [NH*2+200]; //real s(-NH:NH) !Power spectrum of cd0
    double s0[NH*2+200]; //double *s0 = new double[NH*2+200]; //double s0[NH*2+200]; //!Best-fit re-centered spectrum for this msg
    double s1[NH*2+200]; //!Overall best-fit re-centered spectrum
    int itone[NN+40];
    int itone_best[NN+40];
    double complex *cwave= new double complex[NZZ+512]; //double complex cwave[NZZ+512];//complex cwave(0:NZZ-1)!Complex waveform corresponding to msg
    //bool c77[140];
    double a[10];
    double complex csymb[32+10];//(0:31)
    double s8[8+2];//real s8(0:7)

    for (int i= 0; i < NH*2+190; ++i) s[i]=0.0;//s=0.
    //ss=0.
    //sq=0.
    //! Find the message that fits the received waveform best
    double sbest=0.0;
    double tbest=0.0;
    fbest=0.0;
    msgbest="";
    for (int i= 0; i < NN+10; ++i) itone_best[i]=0;
    bool my_std  = pomAll.isStandardCall(mycall.trimmed());
    bool his_std = pomAll.isStandardCall(dxcall.trimmed());
    QString msg,msgsent;
    for (int i = 0; i < NMSGS; ++i)//do imsg=1,NMSGS
    {
        //call getmsg(imsg,mycall,dxcall,dxgrid,msg)
        msg = mycall+" "+dxcall;
        if (!my_std)
        {
            //if (i==0) msg = "<"+mycall+"> "+hiscall;
            //if (i>=5) msg = "<"+mycall+"> "+hiscall+" ";
            if (i==0 || i>=5) msg = "<"+mycall+"> "+dxcall;
            if (i>=1 && i<=3) msg = mycall+" <"+dxcall+">";
        }
        else if (!his_std)
        {
            //if(i==0) msg = "<"+mycall+"> "+hiscall;
            //if(i>=1 && i<=3) msg = "<"+mycall+"> "+hiscall+" ";
            if (i<=3 || i==5) msg = "<"+mycall+"> "+dxcall;//wsjt252
            //if (i==5) msg = "TNX 73 GL";
            if (i>=6) msg = mycall+" <"+dxcall+">";
        }
        //else if (i==0) msg = msg0.trimmed();

        if (i==1) msg.append(" RRR");
        if (i==2) msg.append(" RR73");
        if (i==3) msg.append(" 73");
        if (i==4)
        {
            if (his_std)  msg="CQ "+dxcall+" "+dxgrid.mid(0,4);//KN23SF
            if (!his_std) msg="CQ "+dxcall;//KN23SF
        }
        if (i==5 && his_std) msg.append(" "+dxgrid.mid(0,4));

        if (i>=6 && i<206)
        {
            int isnr = -50 + (i-6)/2; //isnr = -50 + (i-7)/2 ss.append(QString("%1").arg(abs(s.toInt()),2,10,QChar('0')));
            if (((i+1) & 1)==1)
            {
                if (isnr>-1) msg.append(" +"+QString("%1").arg(abs(isnr),2,10,QChar('0')));
                else msg.append(" -"+QString("%1").arg(abs(isnr),2,10,QChar('0')));
            }
            else
            {
                if (isnr>-1) msg.append(" R+"+QString("%1").arg(abs(isnr),2,10,QChar('0')));
                else msg.append(" R-"+QString("%1").arg(abs(isnr),2,10,QChar('0')));
            }
        }

        //! Source-encode the message, get itone(), and generate complex FT8 waveform.
        //int i3=-1;
        //int n3=-1;//bool cw[180];
        //TGenFt8->pack77(msg,i3,n3,c77); //call pack77(msg,i3,n3,c77)
        //TGenFt8->make_c77_i4tone(c77,itone);//TGenFt8->make_c77_i4tone_codeword(c77,itone,cw);//call genft8(msg,i3,n3,msgsent,msgbits,itone)
        TGenFt8->pack77_make_c77_i4tone(msg,itone);
        gen_ft8cwaveRxAp8(itone,0.0,cwave); //call gen_ft8wave(itone,NN,32,bt,fsd,0.0,cwave,xjunk,1,NWAVE)
        for (int x = NWAVE; x < NZZ+1; ++x) cwave[x]= 0.0+0.0*I;//cwave(NWAVE:)=0.
        //! For this message, find the best lag
        double spk=0.0;
        double fpk=0.0;
        double tpk=0.0;
        int lagpk=0;
        int lag1=-200; //if(i==107) qDebug()<<i<<"cw="<<creal(cd[0])<<creal(cd[10])<<creal(cd[30])<<creal(cd[200]);
        int lag2=200;  //if(i==107) qDebug()<<i<<"cw="<<creal(cwave[0])<<creal(cwave[10])<<creal(cwave[30])<<creal(cwave[200]);
        int lagstep=4; //if(i==107) qDebug()<<i<<"itone="<<itone[0]<<itone[1]<<itone[78]<<itone[79]<<itone[80];
        for (int iter = 1; iter <= 2; ++iter)
        {//do iter=1,2
            if (iter==2)
            {
                lag1=lagpk-8;
                lag2=lagpk+8;
                lagstep=1;
            }
            for (int lag = lag1; lag <= lag2; lag+=lagstep)
            {//do lag=lag1,lag2,lagstep c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
                for (int ii = 0; ii < NWAVE; ++ii)
                {// do i=0,NWAVE-1
                    int j=ii+lag+100;
                    cd0[ii]=0.0+0.0*I;
                    if (j>=0 && j<=NWAVE-1) cd0[ii]=cd[j]*conj(cwave[ii]);//if(j.ge.0 .and. j.le.NWAVE-1) cd0(i)=cd(j)*conjg(cwave(i))
                }
                for (int ii = NWAVE; ii < NZZ; ++ii) cd0[ii]=0.0+0.0*I;//cd0(NWAVE:)=0.
                f2a.four2a_c2c(cd0,NFFT,-1,1,decid);// call four2a(cd0,NFFT,1,-1,1) //!Forward c2c FFT

                for (int ii = 0; ii < NFFT; ++ii)
                {//do i=0,NFFT-1
                    int j=ii;
                    if (ii>NH) j=ii-NFFT;
                    s[j+offset_s]=fac*pomAll.ps_hv(cd0[ii]);//(real(cd0[ii])**2 + aimag(cd0(ii))**2)
                } //qDebug()<<i<<msg;  continue;
                pomAll.smo121(s,0,NFFT+0);//call smo121(s,NFFT+1)
                //double smax=0.0;
                for (int j = -NH; j <=NH ; ++j)
                {//do j=-NH,NH
                    //smax=fmax(s[j+offset_s],smax);
                    if (s[j+offset_s]>spk)
                    {
                        spk=s[j+offset_s];
                        fpk=(j*df) + f1;//fpk=(j+offset_s)*df + f1;
                        lagpk=lag;
                        tpk=lag*dt;
                        for (int x = 0; x < NH*2+190; ++x) s0[x]=s[x]; //s0=s //!Save best-fit spectrum for this msg
                        //if(i==107)qDebug()<<"ITER="<<i<<iter<<msg<<spk;
                    }
                }//! j*/
            }
        }

        if (spk>sbest)
        {
            sbest=spk;
            fbest=fpk;
            tbest=tpk;
            msgbest=msg;
            for (int x = 0; x < NN+10; ++x) itone_best[x]=itone[x];
            for (int x = 0; x < NH*2+190; ++x) s1[x]=s0[x]; //!Save overall best-fit spectrum
            //qDebug()<<i<<msg;
        }
    }
    for (int i = 0; i < 6; ++i) a[i]=0.0;//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    a[0]=f1-fbest;
    //qDebug()<<"MSG="<<msgbest<<f1<<fbest<<f1-fbest;
    //!  if(abs(a(1)+5.0).gt.10.0) then             !Effective Ftol = 10 Hz
    if (fabs(a[0])>5.0) msgbest="";//if(abs(a(1)).gt.5.0) then                   !Effective Ftol = 5 Hz
    if (!msgbest.isEmpty())
    {
        pomFt.twkfreq1(cd,NZZ,200.0,a,cd);//twkfreq1(cd,NZZ,200.0,a,cd)           !Re-center the spectrum
        xdt=tbest;
        double sum0 = 0.0;
        double sum1 = 0.0; //qDebug()<<"S1="<<s1[100]<<s1[120]<<s1[140]<<s1[150];
        for (int i = offset_s; i <= offset_s+100; ++i)//(sum(s1(-200:-100)) + sum(s1(100:200)))/202.0
        {
            sum0+=s1[i-200];
            sum1+=s1[i+100];
        }
        double ave=(sum0+sum1)/202.0;
        for (int i = 0; i < NH*2+10; ++i) s1[i]=s1[i]/ave - 1.0; //s1=s1/ave - 1.0
        double s1pk=pomAll.maxval_da_beg_to_end(s1,offset_s-32,offset_s+32+1);//maxval(s1(-32:32))
        double sig=0.0;
        int nsig=0;
        for (int i = -32; i <= 32; ++i)
        {//do i=-32,32
            if (s1[i+offset_s]<0.5*s1pk) continue;
            sig += s1[i+offset_s];//*0.00001;
            nsig++;
        } //qDebug()<<"MSG3========="<<msgbest<<"SIG="<<sig<<nsig;
        sig=sig/nsig;
        xsnr=pomAll.db(sig)-42.0;//db(sig)-35.0;
        if (xsnr<-30.0) xsnr=-30.0;
        //qDebug()<<"MSG3========="<<msgbest<<"xsnr="<<xsnr;
        int ipk = 0;
        //if (!msgbest.isEmpty())
        //{
        plog=0.0;//! Compute probability of successful decode.
        int nhard=0;
        int nsum=0;
        double sum_sync=0.0;
        double sum_sig=0.0;
        double sum_big=0.0;
        for (int k = 0; k <NN; ++k)//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
        {//do k=1,NN
            int i0=32*k + (int)((tbest+0.5)/0.005);
            for (int z = 0; z < 33; ++z) csymb[z]=0.0+0.0*I;
            for (int i = 0; i < 32; ++i)
            {//do i=0,31
                if (i0+i>=0 && i0+i<=NZZ-1) csymb[i]=cd[i0+i];//if(i0+i.ge.0 .and. i0+i.le.NZZ-1) csymb(i)=cd(i0+i)
            }
            f2a.four2a_c2c(csymb,32,-1,1,decid);//four2a(csymb,32,1,-1,1)           !c2c
            double s8sum = 0.0;
            for (int i = 0; i < 8; ++i)
            {//do i=0,7
                s8[i]=pomAll.ps_hv(csymb[i]);//real(csymb(i))**2 + aimag(csymb(i))**2
                s8sum+=s8[i];//s8sum=sum(s8)
            }
            if (s8sum>0.0)
            {
                double p=s8[itone_best[k]]/s8sum;
                if (p<0.0000001) p=0.0000001;
                plog+=log(p);
                nsum++;
            }
            ipk=pomAll.maxloc_da_beg_to_end(s8,0,8);//ipk=maxloc(s8)-1
            if (ipk!=itone_best[k]) nhard++;//if(ipk(1).ne.itone_best(k)) nhard=nhard+1
            //if(k.le.7 .or. (k.ge.37 .and. k.le.43) .or. k.ge.73) then
            if (k<=7-1 || (k>=37-1 && k<=43-1) || k>=73-1) sum_sync += s8[itone_best[k]];
            else sum_sig += s8[itone_best[k]];
            sum_big += s8[ipk];
        }
        if (nsum<NN) plog += (NN-nsum)*log(0.125);//if(nsum.lt.NN) plog=plog + (NN-nsum)*log(0.125)
        double sigobig=(sum_sync+sum_sig)/sum_big;
        //!write(60,3060) nsum,xdt,fbest,xsnr,plog,nhard,sigosync,sigobig,trim(msgbest)
        //!3060 format(i2,f8.3,3f8.1,i5,2f7.2,1x,a)
        //if(nhard.gt.54 .or. plog.lt.-159.0 .or. sigobig.lt.0.71) msgbest=''
        if (nhard>54 || plog<-159.0 || sigobig<0.71) msgbest="";
        //qDebug()<<" END="<<nhard<<plog<<sigobig<<msgbest<<"FrDtSnr="<<fbest<<xdt<<xsnr;
        //}
    }
    //delete [] s0;
    delete [] cwave;
    //delete [] cd0;
    //delete [] cd;
}
void DecoderFt8::PrintMsg(QString tmm,int nsnr,double xdt,double f1,QString message,
                          int iaptype,float nhr,float dmi,bool &have_dec,bool fshow,bool qual_eq_dmin)
{
    if (fshow)//2.69
    {
        if (f_new_p)
        {
            f_new_p = false;
            emit EmitBackColor();
        }
        float qual=1.0-(nhr+dmi)/60.0; //scale qual to 0.0-1.0
        if (qual_eq_dmin)//222 iaptype==8
        {
            qual=dmi;//iaptype = 8;
        }
        if (qual<0.001) qual=0.0;//no show -0.0
        QString str_iaptype = "";
        if (qual<0.17) str_iaptype = "? ";
        if (iaptype!=0) str_iaptype.append("AP");
        if (iaptype>8) iaptype=9;//2.76.5
        str_iaptype.append(QString("%1").arg(iaptype));
        int df_hv = f1-s_nftx8;
        QString sdtx = QString("%1").arg(xdt,0,'f',1);
        if (sdtx=="-0.0") sdtx="0.0";//remove -0.0 exception
        QStringList list;
        list <<tmm<<QString("%1").arg(nsnr)
        <<sdtx<<QString("%1").arg((int)df_hv)
        <<message<<str_iaptype
        <<QString("%1").arg(qual,0,'f',1)
        <<QString("%1").arg((int)f1);
        emit EmitDecodetTextFt(list);//1.27 psk rep fopen bool true, false no file open
        have_dec = true; //qDebug()<<message.indexOf(s_HisCall8)<<s_HisCall8;
        if (s_HisCall8.count()>2 && message.indexOf(s_HisCall8)>-1) s_ltry_a8=false;//ws300rc1 if(index(msg37,trim(hiscall12)).gt.0) la8=.false.
    }
    if (!s_fopen8 && message.count()>2) ft8_a7_save(tmm,xdt,f1,message);//2.70 for ap7 s_fopen8 and no empty msg
}
void DecoderFt8::TryAp8(double *dd,bool fl_lapon,int id3dec,int cont_type,double nfqso,bool &have_dec)
{
	//if (fl_lapon && id3dec>2 && s_ltry_a8 && s_HisCall8.count()>2 && s_HisGrid8.count()>3 && !s_fopen8 && cont_type==0)
	if (!fl_lapon || s_fopen8 || s_HisCall8.count()<3 || s_HisGrid8.count()<4) return;
	//if (!s_ltry_a8 || id3dec<3 || !fl_lapon || s_fopen8 || cont_type!=0 || s_HisCall8.count()<3 || s_HisGrid8.count()<4) return;
    if (id3dec>2 && s_ltry_a8 && cont_type==0)
    {
        double f1=nfqso;//! Try for an a8 decode at nfqso
        QString message;
        double xsnr = 0.0;
        double plog = 0.0;
        double xdt = 0.0;
        double fbest = 0.0; //qDebug()<<"START AP8 -> "<<"DecID="<<decid<<s_ltry_a8<<s_HisCall8<<s_HisGrid8<<f1;
        ft8_a8d(dd,s_MyBaseCall8,s_HisCall8,s_HisGrid8,f1,xdt,fbest,xsnr,plog,message);
        if (!message.isEmpty())//if(msg37(1:1).ne.' ') then //message.mid(0,1)!=" "
        {
            int nsnr=(int)xsnr;//sync=10.0;//  !### ???
            double qual=1.0;
            if (plog<-147.0) qual=0.16;//call this%callback(sync,nsnr,xdt,fbest,msg37,iaptype,qual)
            PrintMsg(s_time8,nsnr,xdt,f1,message,8,qual,qual,have_dec,true,true);
        }
    }
}
void DecoderFt8::ft8_decode(double *dd,int c_dd,double f0a,double f0b,double fqso,bool &have_dec,
                            int id3dec,double w_f00,double w_f01)//,int /*npts no need*/)
{
    have_dec = false;
    if (f_new_p)
    {
        s_ndecodes=0;
    }

    double nfqso=fqso; //1500.0; // mybe from waterfull scale
    double nfqso2=fqso;//2.72
    double nfa=f0a;//100.0;              // mybe from waterfull scale
    double nfb=f0b;//3000.0;             // mybe from waterfull scale
    bool nagain = false;//s_mousebutton = mousebutton; //mousebutton Left=1, Right=3 fullfile=0 rtd=2
    if (s_mousebutton8==3) nagain = true;
    if (nagain)
    {
        nfa = nfqso-25;//ntol = 25;// +/-10 hv +/-25
        nfb = nfqso+25;
    }
    //double corrt = 0.0;
    //if (decid>0) corrt = 56.0;//2.72 5=16Hz 18=56Hz
    bool lqsothread=true;
    nfa=fmax(100,nfa);//nfa=fmax(100,nfqso-ntol);    //hv nfa down
    nfa=fmin(5000-10,nfa);       //hv nfa up
    nfb=fmax(100+10,nfb);//nfb=fmax(100+10,nfqso+ntol); //hv nfb down
    nfb=fmin(5000,nfb); //qDebug()<<"FT8 D="<<decid<<nfa<<nfb<<nfqso<<id3dec;
    if (id3dec==1 || id3dec==100) s_ltry_a8=true;
    if (nfqso<nfa || nfqso>nfb)
    {
        nfqso = nfa + ((nfb-nfa)/2.0);
        s_ltry_a8=false; //ws300rc1 nfqso out of decoder bandwidth
        lqsothread=false;
    }    //qDebug()<<decid<<"try_a8="<<s_ltry_a8;

    bool use_var_dec = s_use_var_dec; //qDebug()<<"USE_VAR_DEC="<<use_var_dec;
    int jseqr = ft8_even_odd(s_time8);
    bool f_eve0_odd1=true;
    if (jseqr==0) f_eve0_odd1=false;
    if (use_var_dec)
    {
        if (id3dec==1 || id3dec==100) ft8_SetStart_ev_od_var(f_eve0_odd1);
        if (!s_3intFt8_d_)
        {
            ft8_decodevar(dd,c_dd,nfa,nfb,nfqso,have_dec,w_f00,w_f01,nagain,lqsothread,f_eve0_odd1);
            return;
        }
    }

    int cont_type = 0;
    int cont_id   = 0;
    if (!f_multi_answer_mod8)
    {
        cont_type = s_ty_cont_ft8_28;
        cont_id   = s_id_cont_ft8_28;
    } //qDebug()<<"ncontest="<<ncontest;
    QString his_call_f = s_HisCall8;
    //bool f_only_one_color = true;
    int iaptype = 4; //2.00 = RRR
    //const int DD=58;
    int apsym2[63];//58+5
    bool lsubtract;

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
    if (s_time8!=nutc0 && !s_fopen8)//2.66 for ap7 s_fopen8
    {
        if (ndec[1][jseq]==0) c_zerop++; //2.66 HV add
        else c_zerop=0;

        int iz=ndec[1][jseq];
        for (int i = 0; i < iz; ++i)
        {
            dt0[0][jseq][i]  = dt0[1][jseq][i];
            f0[0][jseq][i]   = f0[1][jseq][i];
            msg0[0][jseq][i] = msg0[1][jseq][i];
        }
        int t_ss1 =  s_time8.midRef(4,2).toInt();
        int t_mm1 =  s_time8.midRef(2,2).toInt();
        int t_hh1 =  s_time8.midRef(0,2).toInt();
        int t_p1  = (t_hh1*3600)+(t_mm1*60)+t_ss1;
        t_ss1 =  nutc0.midRef(4,2).toInt();
        t_mm1 =  nutc0.midRef(2,2).toInt();
        t_hh1 =  nutc0.midRef(0,2).toInt();
        int t_p0  = (t_hh1*3600)+(t_mm1*60)+t_ss1;
        if (t_p1-t_p0>45 || c_zerop>2)//2.66 HV add=3p   if (t_p1-t_p0>60 || c_zerop>3)//2.66 HV add=4p
        {
            //ft8 45 and 3 for 3 periods//60 and 4 for 4 periods ...
            //1. We lost 3 periods (no decode),= data is not actual, ap7 not permitted
            //2. We stopped APP for 3 periods, = data is not actual, ap7 not permitted
            ndec[0][0]=0;
            ndec[0][1]=0;
            /*for (int i = 0; i < MAXDEC; ++i)// no needed
            {
               	dt0[0][0][i]=0.0;
               	f0[0][0][i]=0.0;         
               	dt0[0][1][i]=0.0;
               	f0[0][1][i]=0.0; 
            }*/
            c_zerop=0;
        }
        else
        {
            if (iz>0) ndec[0][jseq]=iz;//2.66 HV correction
        }
        /*for (int i = 0; i < ndec[0][jseq]; ++i) qDebug()<<msg0[0][jseq][i]<<f0[0][jseq][i];
        qDebug()<<"----------------------";*/
        ndec[1][jseq]=0;
        for (int i = 0; i < MAXDEC; ++i)// no needed
        {
            dt0[1][jseq][i]=0.0;
            f0[1][jseq][i]=0.0;
        }
        nutc0=s_time8;
        /*int e,r,t,y;
        e=ndec[0][0];
        r=ndec[0][1];
        t=ndec[1][0];
        y=ndec[1][1];
        QString s = "   Move UP --------- "+QString("%1").arg(ndec[0][jseq]).leftJustified(2,' ')+" ";     
        if (jseq==1) s.append("candidates for second period");//res=0 (even first), or res=1 (odd second)
        else s.append("candidates for first  period");
        if (resss) qDebug()<<s<<t_p1-t_p0<<"RESET"<<e<<r<<t<<y<<"cc="<<c_zerop;
        else qDebug()<<s<<t_p1-t_p0<<"-OK--"<<e<<r<<t<<y<<"cc="<<c_zerop;*/
        /*int jseqr = ft8_even_odd(s_time8);
        for (int i = 0; i<ndec[0][jseqr]; ++i) qDebug()<<msg0[0][jseqr][i]<<f0[0][jseqr][i];
             	qDebug()<<"-------------------------------";*/
    }

    /*! iaptype //mybe no use HV
    !------------------------
    !   1        CQ     ???    ???
    !   2        DE     ???    ???
    !   3        MyCall ???    ???
    !   4        MyCall DxCall ???
    !   5        MyCall DxCall RRR
    !   6        MyCall DxCall 73
    !   7        MyCall DxCall RR73
    !   8        ???    DxCall ??? */
    //qDebug()<<s_dxcall<<My_Grid_Loc<<MyCall;

    ft8apset(s_MyBaseCall8,s_HisCall8,apsym2);//JO90NH test iaptype HisGridLoc,msk144_ft8_cont_msg My_Grid_Loc,

    int n2 = 0;
    bool fl_lapon = s_lapon8;
    int nQSOProgress = s_nQSOProgress8;
    int npass=3;
    int imetric=1;//ws300rc1

    int ndepth = s_decoder_deep8;
    bool n4pas3int = false;//2.4 HV added for ft8b()  -> if(nzhsym.lt.50) npasses=4
    if (s_3intFt8_d_)
    {
        n4pas3int = true;
        /*if (id3dec==1 || id3dec==100)
        {
        	qDebug()<<"RESET D="<<decid<<s_ndecodes;
        }*/
        if (id3dec==2 && s_ndecodes==0) //2=162432 samples
        {
            return;
        }
        else if (id3dec==2 && s_ndecodes>0) //2=162432 samples
        {
            for (int i = 0; i < ALL_MSG_SNR; ++i) lsubtracted[i] = false;
            bool lrefinedt = true;
            if (ndepth<=2) lrefinedt = false;
            for (int i = 0; i < s_ndecodes; ++i)
            {
                if (xdt_save[i]-0.5<0.396)//if(xdt_save(i)-0.5.lt.0.396) then
                {
                    subtractft8(dd,itone_save[i],f1_save[i],xdt_save[i],lrefinedt);
                    lsubtracted[i] = true;
                }
            }
            s_cou_dd1 = c_dd;
            for (int i = 0; i < s_cou_dd1; ++i) dd1[i]=dd[i];
            //qDebug()<<"Save count dd1="<<s_cou_dd1;
            return; //goto c900; //return; 2.66 w260rc1
        }
        else if (id3dec==3 && s_ndecodes>0) //3=172800 samples
        {
            //int n=47*3456; //1=141696 2=162432  3=172800 samples
            //if (decid==1) qDebug()<<"2 162432<"<<s_cou_dd1<<" 3 172800<"<<c_dd;
            for (int i = 0; i < s_cou_dd1; ++i) dd[i]=dd1[i];//dd(1:n)=dd1(1:n)
            //for (int i = n; i < 181000; ++i) dd[i]=dd1[i];//dd(n+1:)=iwave(n+1:)
            for (int i = 0; i < s_ndecodes; ++i)
            {
                if (lsubtracted[i]) continue;
                subtractft8(dd,itone_save[i],f1_save[i],xdt_save[i],true);
            }
            n4pas3int = false;
        }
        if (use_var_dec && id3dec==3)
        {
            ft8_decodevar(dd,c_dd,nfa,nfb,nfqso,have_dec,w_f00,w_f01,nagain,lqsothread,f_eve0_odd1);
            return;
        }
    }

    double candidate[2][620];//2.69 old=255 start from here;
    double (*s_)[1970] = new double[402][1970];//2.39 start from here;  2.66
    double sbase[1970];//2.39 start from here;

    //! For now:
    //! ndepth=1: no subtraction, 1 pass, belief propagation only
    //! ndepth=2: subtraction, 3 passes, belief propagation only
    //! ndepth=3: subtraction, 3 passes, bp+osd

    //int npass=3;   2.66
    if (ndepth==1) npass=2;//2.69 old260r5=npass=1;
    //if (ndepth>=2) npass=3;//if(ndepth.ge.2) npass=3

    for (int ipass = 1; ipass <= npass; ++ipass)
    {
        //qDebug()<<"ft8_decode  Start========================================"<<ndepth;
        int ndeep=ndepth;
        bool newdat=true;  //! Is this a problem? I hijacked newdat.
        double syncmin = 1.33;//1.3 old=1.5; syncmin=1.3
        if (s_3intFt8_d_)
        {
            if (ndepth<=2) syncmin = 1.56;//1.55;//if(ndepth.le.2) syncmin=1.6 220rc3
            if (id3dec==1) syncmin = 1.96;//1.95;//2.69 260r5
            if (ipass==1)
            {
                lsubtract=true;
                //if (id3dec==3 && npass==1) lsubtract=false;//HV added 2.66 stop for AP7
                ndeep=ndepth;
                if (ndepth==3) ndeep=2;
                imetric=1;//ws300rc1
            }
            else if (ipass==2)
            {
                n2=s_ndecodes;
                if (s_ndecodes==0) continue; //2.70=(npass<3 &&) old=if (ndecodes==0) continue;
                lsubtract=true;
                ndeep=ndepth;
                imetric=2;//ws300rc1
            }
            else if (ipass==3)
            {
                if ((s_ndecodes-n2)==0) continue;
                lsubtract=true;
                //if (id3dec==3) lsubtract=false;//HV added 2.66 stop for AP7
                ndeep=ndepth;
                imetric=2;//ws300rc1
            }
        }
        else
        {
            syncmin = 1.5;//1.6 equal to old ver. for speed
            if (ipass==1)
            {
                lsubtract=true;
                //if (ndeep==1) lsubtract=false;//2.66 stop for AP7
                imetric=1;//ws300rc1
            }
            else if (ipass==2)
            {
                n2=s_ndecodes;
                if (s_ndecodes==0) continue;
                lsubtract=true;
                imetric=2;//ws300rc1
            }
            else if (ipass==3)
            {
                if ((s_ndecodes-n2)==0) continue;
                lsubtract=true; //2.66 add for AP7
                //lsubtract=false;//2.66 stop for AP7
                imetric=2;//ws300rc1
            }
        }

        int ncand = 0;
        //qDebug()<<"sync8  Start=========xxxxxxxxxx"<<ipass<<nfa<<nfb<<syncmin;

        sync8(dd,nfa,nfb,syncmin,nfqso2,s_,candidate,ncand,sbase);
        //if (sbase[800]==0.0) qDebug()<<"sync8 "<<decid<<ipass<<ncand<<sbase[200];

        if (ipass == 1)
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
                int zero_dd = ((frst_dd+last_dd)/2.0)*12000.0 + 151680 + 600;//151680+1200=frame+100ms  frame+360ms=156000
                for (int ic = zero_dd; ic < 180010; ++ic) dd[ic]=0.00000001;//2.70
                //qDebug()<<decid<<"S="<<frst_dd<<last_dd<<zero_dd<<dd[140000]<<dd[180000];
                /*int smpl = 300;
                double st_smpl = 1.0/(double)smpl;
                double kv_smpl = 1.0;
                for (int ic = zero_dd-smpl; ic < 180010; ++ic)
                {
                	dd[ic] *=kv_smpl;
                	kv_smpl -= st_smpl;
                	if (kv_smpl<0.00000001) kv_smpl = 0.00000001; 
                }*/
            }
        }

        for (int icand = 0; icand < ncand; ++icand)
        {//do icand=1,ncand
            //qDebug()<<"ipass=============="<<ipass<<icand<<ncand;
            //double sync=candidate[2][icand];
            double f1 =candidate[0][icand];
            double xdt=candidate[1][icand];

            double xbase=pow(10.0,(0.1*(sbase[int(f1/3.125)]-40.0))); //xbase=10.0**(0.1*(sbase(nint(f1/3.125))-40.0))
            //qDebug()<<icand<<"xbase "<<xbase;
            /*double sync=1.6;
            double f1=1200.0;
            double xdt=1.05;*/
            //nsnr0=min(99,nint(10.0*log10(sync) - 25.5))    //!### empirical ###
            double xsnr = 0.0;

            //bool lapon = true;//Ap decoding ?
            double napwid=60.0;//hv=+/-60 -> default=+/-50 kolko na starni da puska AP dec_data.params.napwid=50;
            int nharderrors = 0;
            double dmin = 0.0;
            int nbadcrc = 0;
            QString message = "";
            int itone[120];
            int i3=-1; int n3=-1;
            //int iappass = 0;
            //lsubtract = false;
            ft8b(dd,newdat,nQSOProgress,nfqso,s_nftx8,ndeep,n4pas3int,fl_lapon,napwid, //ndepth
                 lsubtract,nagain,cont_id,cont_type,imetric,iaptype,f1,xdt,xbase,apsym2,
                 nharderrors,dmin,nbadcrc,message,xsnr,his_call_f,itone,i3,n3);

            int nsnr=(int)xsnr;
            xdt=xdt-0.5; //int hd=nharderrors+dmin;

            if (nbadcrc==0)
            {
                bool ldupe=false;
                for (int id = 0; id < s_ndecodes; ++id)//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
                {
                    if (message==allmessages[id])//2.48 problem 2xRR73 no end QSO && nsnr<=allsnrs[id]
                    {
                        ldupe=true; //qDebug()<<"DUPE"<<message;
                        break;
                    }
                }
                if (!ldupe) //then  if (message.mid(0,3)=="CQ ")
                {
                    allmessages[s_ndecodes]=message; //printf("Msg= %-20s --- i3=%d n3=%d\n",qPrintable(message),i3,n3);
                    xdt_per_dec[s_ndecodes]=xdt;
                    if (s_3intFt8_d_)//2.39 remm
                    {
                        f1_save[s_ndecodes]=f1;
                        xdt_save[s_ndecodes]=xdt+0.5;
                        for (int is = 0; is < 79; ++is) itone_save[s_ndecodes][is]=itone[is];
                    }
                    if (s_ndecodes < (ALL_MSG_SNR-1)) s_ndecodes++;
                    PrintMsg(s_time8,nsnr,xdt,f1,message,iaptype,(float)nharderrors,(float)dmin,have_dec,true,false);
                    if (use_var_dec)
                    {
                    	bool lFreeText=false;
                        if (i3==0 && n3==0) lFreeText=true;
                    	ft8_Write_evt_odt_var(message,i3,f_eve0_odd1,f1,xdt,lFreeText,false,false);                    	
                   	}
                }             
            }//c4: //if (icand < ncand) continue;
        }
    }
    //c900: //2.66
    //qDebug()<<"id3dec="<<id3dec<<fl_lapon;
    //if (fl_lapon)
    //{
    //int jseqr = ft8_even_odd(s_time8);//int jseqr = jseq;
    /*if (id3dec>2)
    {
    	if (ndec[0][jseqr]>0 && !s_fopen8) qDebug()<<"Read="<<jseqr<<ndec[0][jseqr]<<msg0[0][jseqr][0];
    	else  qDebug()<<"NO Read="<<jseqr<<ndec[0][jseqr];    
    }*/
    if (/*fl_lapon && */id3dec>2 && ndec[0][jseqr]>0 && !s_fopen8)////2.69 for ap7 s_fopen8 260r5=+fl_lapon
    {
        bool newdat = true;
        for (int i = 0; i<ndec[0][jseqr]; ++i)
        {
            //qDebug()<<"======="<<msg0[0][jseqr][i]<<f0[0][jseqr][i]<<newdat;
            //if (f0[0][jseqr][i]==-99.0) break; = s_fopen
            if (f0[0][jseqr][i]==-98.0) continue;
            if (msg0[0][jseqr][i].indexOf("<")>-1) continue;
            QString message=msg0[0][jseqr][i]+"      ";//2.70 =6 pouse crash mid
            int i1=message.indexOf(" ");
            int i2=message.indexOf(" ",i1+1);
            QString call_1=message.mid(0,i1);
            QString call_2=message.mid(i1+1,i2-i1-1);
            QString grid4 =message.mid(i2+1,4); //qDebug()<<i1<<i2+1<<i2+1+4<<message.count()<<message;
            //if(grid4.eq.'RR73' .or. index(grid4,'+').gt.0 .or. index(grid4,'-').gt.0) grid4='    '
            if (grid4=="RR73" || grid4.indexOf("+")>-1 || grid4.indexOf("-")>-1) grid4="    ";
            double xdt=dt0[0][jseqr][i];
            double f1=f0[0][jseqr][i];
            double xbase=pow(10.0,(0.1*(sbase[(int)fmax(1.0,(f1/3.125))]-40.0)));
            message="";
            int nharderrors = 0;
            double xsnr = 0.0;
            double dmin = 0.0;
            //qDebug()<<"======="<<call_1<<call_2<<grid4<<newdat<<s_fopen8;
            ft8_a7d(dd,newdat,call_1,call_2,grid4,xdt,f1,xbase,nharderrors,dmin,message,xsnr);

            if (nharderrors>=0)
            {
                int nsnr=(int)xsnr; //iaptype=7;
                bool fshow = true;
                float qual=1.0-((float)nharderrors+(float)dmin)/60.0;
                if (w_f00>f1 || w_f01<f1 || (!fl_lapon && qual<0.2))//2.69
                {
                    /*float qual=1.0-((float)nharderrors+(float)dmin)/60.0; //!scale qual to [0.0,1.0]
                    if (qual<0.001) qual=0.0;//no show -0.0*/
                    //qDebug()<<"F0="<<w_f00<<"F1="<<w_f01<<message<<"FDecod="<<f1<<qual<<fl_lapon;
                    fshow = false;
                }
                PrintMsg(s_time8,nsnr,xdt,f1,message,7,(float)nharderrors,(float)dmin,have_dec,fshow,false);
            }
        }
    }
    /*! ncontest=0 : NONE   //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    !          6 : FOX
    !          7 : HOUND*/
    //qDebug()<<"DecID="<<decid<<"s_ltry_a8="<<s_ltry_a8<<s_HisCall8<<s_HisGrid8<<id3dec<<nfqso;
    //bool ltry_a8 = true;  /*ncontest!=6 && ncontest!=7 &&*/
    TryAp8(dd,fl_lapon,id3dec,cont_type,nfqso,have_dec);
    
    delete [] s_;
}








