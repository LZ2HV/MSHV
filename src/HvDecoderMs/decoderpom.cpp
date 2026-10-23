/* MSHV decoderpom
 * Copyright 2020 Hrisimir Hristov, LZ2HV
 * May be used under the terms of the GNU General Public License (GPL)
 */
//#include "decoderms.h"
#include "decoderpom.h"
#include <QRegExp>
#include <unistd.h>

//#include <QtGui>
//// F2A ///
//nflags=FFTW_ESTIMATE;
//if (npatience==1) nflags=FFTW_ESTIMATE_PATIENT;
//if (npatience==2) nflags=FFTW_MEASURE;
//if (npatience==3) nflags=FFTW_PATIENT;
//if (npatience==4) nflags=FFTW_EXHAUSTIVE;
// MSK144 four2a_c2c nfft=32768  7  plans        four2a_d2c none
// MSKMS  four2a_c2c nfft=32768  7  plans        four2a_d2c none
// JTMS   four2a_c2c nfft=20552  20 plans        four2a_d2c nfft=524288  4 plans + ZAP only
// FSK441 four2a_c2c nfft=32768  8  plans        four2a_d2c nfft=524288  4 plans + ZAP only
// ISCAT  four2a_c2c nfft=73728  2  plans        four2a_d2c nfft=524288  4 plans + ZAP only
// JT6M   four2a_c2c nfft=512	 1  plans		 four2a_d2c nfft=524288  4 plans + ZAP only
// FT8    four2a_c2c nfft=180000 4  plans        four2a_d2c nfft=192000  3 plans
// FT4    four2a_c2c nfft=72576  4  plans        four2a_d2c nfft=72576   2 plans
// JT65   four2a_c2c nfft=2048   2  plans        four2a_d2c nfft=8192    1 plans
// PI4    four2a_c2c none                        four2a_d2c nfft=768000  3 plans
// Q65    four2a_c2c 1440000 for x120sec

//static int retr = 0;
//static int cplu = 0;
#define SLPAMIN  2000 //2000 importent SLPAMIN > SLPASTEP
#define SLPASTEP 1000 //1000 importent SLPAMIN > SLPASTEP
static bool _block_th_all_ = false;        //need to be static for all
static int _wait_t_ = SLPAMIN - SLPASTEP;  //need to be static for all
static int setup_c2c_d2c_(bool &wait,fftw_plan &p,double complex *a,int nfft,int isign,int iform,double *d = 0)
{
    if (_block_th_all_ || !wait)
    {
        _wait_t_ += SLPASTEP;
        wait = true; //retr++; qDebug()<<"retry---->"<<retr<<_wait_t_;
        return _wait_t_;
    }
    _block_th_all_ = true; //if (cplu == 0) qDebug()<<"----------------------"; cplu++; qDebug()<<"PLANS="<<cplu<<nfft;

    if 		(isign==-1 && iform==1) p=fftw_plan_dft_1d(nfft,a,a,FFTW_FORWARD,FFTW_ESTIMATE_PATIENT);
    else if (isign==1 &&  iform==1) p=fftw_plan_dft_1d(nfft,a,a,FFTW_BACKWARD,FFTW_ESTIMATE_PATIENT);
    else if (isign==-1 && iform==0) p=fftw_plan_dft_r2c_1d(nfft,d,a,FFTW_ESTIMATE_PATIENT);
    else if (isign==1 && iform==-1) p=fftw_plan_dft_c2r_1d(nfft,a,d,FFTW_ESTIMATE_PATIENT);

    _block_th_all_ = false;
    return 0;
}
#define NPAMA_ 2048
#define NPAMAX 1441000  //q65 max=1440000 PI4 max 768000
#define NSMALL 16384
void HvThr::four2a_c2c(double complex *a,double complex *a1,fftw_plan *pc,int &cpc,int nfft,int isign,int iform)
{
    double complex aa[NSMALL+10];

    if (cpc>NPMAX || nfft>NPAMAX) return;

    bool found_plan = false;
    int z = 0;
    for (z = 0; z < cpc; ++z)
    {
        if (nfft==nn_c2c[z] && isign==ns_c2c[z] && iform==nf_c2c[z])
        {
            found_plan = true;
            break;
        }
    }

    for (int i = 0; i < nfft; ++i) a1[i]=a[i];

    if (nfft<=NSMALL)
    {
        int jz=nfft;
        if (iform==0) jz=nfft/2;//iform==0 no in c2c
        for (int i = 0; i<jz; ++i) aa[i]=a1[i];
    }

    if (!found_plan)
    {
        z = cpc;
        nn_c2c[z]=nfft;
        ns_c2c[z]=isign;
        nf_c2c[z]=iform;
        int slpp = 1000;
        bool wait = false; //if (nthreads==1) wait = false; ??? hv
        while (slpp!=0)
        {
            usleep(slpp);
            slpp = setup_c2c_d2c_(wait,pc[z],a1,nfft,isign,iform);
        }
        cpc++;  //qDebug()<<"c2c====="<<cpc<<nfft;
    }

    if (nfft<=NSMALL)
    {
        int jz=nfft;
        if (iform==0) jz=nfft/2;//iform==0 no in c2c
        for (int i = 0; i<jz; ++i) a1[i]=aa[i];
    }

    fftw_execute(pc[z]);
    for (int i = 0; i < nfft; ++i) a[i]=a1[i];
}
void HvThr::four2a_d2c(double complex *a,double complex *a1,double *d,double *d1,fftw_plan *pd,int &cpd,
                       int nfft,int isign,int iform)
{
    double complex aa[NSMALL+10];

    if (cpd>NPMAX || nfft>NPAMAX) return;

    bool found_plan = false;
    int z = 0;
    for (z = 0; z < cpd; ++z)
    {
        if (nfft==nn_d2c[z] && isign==ns_d2c[z] && iform==nf_d2c[z])
        {
            found_plan = true;
            break;
        }
    }

    int cfft = nfft;
    if (iform==0) cfft = nfft/2;
    for (int i = 0; i < nfft; ++i)
    {
        if (i<cfft) a1[i]=a[i];
        d1[i]=d[i];
    }

    if (nfft<=NSMALL)
    {
        for (int i = 0; i<cfft; ++i) aa[i]=a1[i];
    }

    if (!found_plan)
    {
        z = cpd;
        nn_d2c[z]=nfft;
        ns_d2c[z]=isign;
        nf_d2c[z]=iform;
        int slpp = 1000;
        bool wait = false; //if (nthreads==1) wait = false; ??? hv
        while (slpp!=0)
        {
            usleep(slpp);
            slpp = setup_c2c_d2c_(wait,pd[z],a1,nfft,isign,iform,d1);
        }
        cpd++;  //qDebug()<<"d2c="<<cpd<<nfft;
    }

    if (nfft<=NSMALL)
    {
        for (int i = 0; i<cfft; ++i) a1[i]=aa[i];
    }

    fftw_execute(pd[z]);
    for (int i = 0; i < nfft; ++i)
    {
        if (i<cfft) a[i]=a1[i];
        d[i]=d1[i];
    }
}
#define NPLIM 20 // reset if > 20 JTMS
void HvThr::DestroyPlans(fftw_plan *pc,int &cpc,fftw_plan *pd,int &cpd,bool imid)
{
    if (cpc > NPLIM || imid)
    {
        //qDebug()<<"Plans C2C="<<cpc;
        for (int z = 0; z < cpc; ++z) fftw_destroy_plan(pc[z]);
        cpc = 0;
        _wait_t_ = SLPAMIN - SLPASTEP;
    }
    if (cpd > NPLIM || imid)
    {
        //qDebug()<<"Plans D2C="<<cpd;
        for (int z = 0; z < cpd; ++z) fftw_destroy_plan(pd[z]);
        cpd = 0;
        _wait_t_ = SLPAMIN - SLPASTEP;
    }
}
//// END class HvThr ///

//// class F2A ///
static int nplan_c2c0 = 0;
static fftw_plan plan_c2c0[NPMAX+NAMA_];
static double complex ca_c2c0[NPAMAX+NPAMA_];
static int nplan_c2c1 = 0;
static fftw_plan plan_c2c1[NPMAX+NAMA_];
static double complex ca_c2c1[NPAMAX+NPAMA_];
static int nplan_c2c2 = 0;
static fftw_plan plan_c2c2[NPMAX+NAMA_];
static double complex ca_c2c2[NPAMAX+NPAMA_];
static int nplan_c2c3 = 0;
static fftw_plan plan_c2c3[NPMAX+NAMA_];
static double complex ca_c2c3[NPAMAX+NPAMA_];
static int nplan_c2c4 = 0;
static fftw_plan plan_c2c4[NPMAX+NAMA_];
static double complex ca_c2c4[NPAMAX+NPAMA_];
static int nplan_c2c5 = 0;
static fftw_plan plan_c2c5[NPMAX+NAMA_];
static double complex ca_c2c5[NPAMAX+NPAMA_];
void F2a::four2a_c2c(double complex *a,int nfft,int isign,int iform,int thr)
{
    if 		(thr==0) HvThr0.four2a_c2c(a,ca_c2c0,plan_c2c0,nplan_c2c0,nfft,isign,iform);
    else if (thr==1) HvThr1.four2a_c2c(a,ca_c2c1,plan_c2c1,nplan_c2c1,nfft,isign,iform);
    else if (thr==2) HvThr2.four2a_c2c(a,ca_c2c2,plan_c2c2,nplan_c2c2,nfft,isign,iform);
    else if (thr==3) HvThr3.four2a_c2c(a,ca_c2c3,plan_c2c3,nplan_c2c3,nfft,isign,iform);
    else if (thr==4) HvThr4.four2a_c2c(a,ca_c2c4,plan_c2c4,nplan_c2c4,nfft,isign,iform);
    else if (thr==5) HvThr5.four2a_c2c(a,ca_c2c5,plan_c2c5,nplan_c2c5,nfft,isign,iform);
}
static int nplan_d2c0 = 0;
static fftw_plan plan_d2c0[NPMAX+NAMA_];
static double da_d2c0[NPAMAX+NPAMA_];
static double complex ca_d2c0[NPAMAX+NPAMA_];
static int nplan_d2c1 = 0;
static fftw_plan plan_d2c1[NPMAX+NAMA_];
static double da_d2c1[NPAMAX+NPAMA_];
static double complex ca_d2c1[NPAMAX+NPAMA_];
static int nplan_d2c2 = 0;
static fftw_plan plan_d2c2[NPMAX+NAMA_];
static double da_d2c2[NPAMAX+NPAMA_];
static double complex ca_d2c2[NPAMAX+NPAMA_];
static int nplan_d2c3 = 0;
static fftw_plan plan_d2c3[NPMAX+NAMA_];
static double da_d2c3[NPAMAX+NPAMA_];
static double complex ca_d2c3[NPAMAX+NPAMA_];
static int nplan_d2c4 = 0;
static fftw_plan plan_d2c4[NPMAX+NAMA_];
static double da_d2c4[NPAMAX+NPAMA_];
static double complex ca_d2c4[NPAMAX+NPAMA_];
static int nplan_d2c5 = 0;
static fftw_plan plan_d2c5[NPMAX+NAMA_];
static double da_d2c5[NPAMAX+NPAMA_];
static double complex ca_d2c5[NPAMAX+NPAMA_];
void F2a::four2a_d2c(double complex *a,double *d,int nfft,int isign,int iform,int thr)
{
    if 		(thr==0) HvThr0.four2a_d2c(a,ca_d2c0,d,da_d2c0,plan_d2c0,nplan_d2c0,nfft,isign,iform);
    else if (thr==1) HvThr1.four2a_d2c(a,ca_d2c1,d,da_d2c1,plan_d2c1,nplan_d2c1,nfft,isign,iform);
    else if (thr==2) HvThr2.four2a_d2c(a,ca_d2c2,d,da_d2c2,plan_d2c2,nplan_d2c2,nfft,isign,iform);
    else if (thr==3) HvThr3.four2a_d2c(a,ca_d2c3,d,da_d2c3,plan_d2c3,nplan_d2c3,nfft,isign,iform);
    else if (thr==4) HvThr4.four2a_d2c(a,ca_d2c4,d,da_d2c4,plan_d2c4,nplan_d2c4,nfft,isign,iform);
    else if (thr==5) HvThr5.four2a_d2c(a,ca_d2c5,d,da_d2c5,plan_d2c5,nplan_d2c5,nfft,isign,iform);
}
void F2a::DestroyPlansAll(bool imid)
{
    //imid = true; // test only
    //qDebug()<<"Plans C2C="<<nplan_c2c0<<nplan_c2c1<<nplan_c2c2<<nplan_c2c3<<nplan_c2c4<<nplan_c2c5;
    //qDebug()<<"Plans D2C="<<nplan_d2c0<<nplan_d2c1<<nplan_d2c2<<nplan_d2c3<<nplan_d2c4<<nplan_d2c5;
    HvThr0.DestroyPlans(plan_c2c0,nplan_c2c0,plan_d2c0,nplan_d2c0,imid);
    HvThr1.DestroyPlans(plan_c2c1,nplan_c2c1,plan_d2c1,nplan_d2c1,imid);
    HvThr2.DestroyPlans(plan_c2c2,nplan_c2c2,plan_d2c2,nplan_d2c2,imid);
    HvThr3.DestroyPlans(plan_c2c3,nplan_c2c3,plan_d2c3,nplan_d2c3,imid);
    HvThr4.DestroyPlans(plan_c2c4,nplan_c2c4,plan_d2c4,nplan_d2c4,imid);
    HvThr5.DestroyPlans(plan_c2c5,nplan_c2c5,plan_d2c5,nplan_d2c5,imid);
    //qDebug()<<"Destroy2-------------------";
    //cplu = 0;
    //retr = 0;
}
/*void F2a::InitPlansAllStatic()//2.76.2
{
	for (int i = 0; i < (NPAMAX/10); ++i)//NPAMAX=1441000
	{
		//NPAMAX=421Mb, NPAMAX/2=256Mb, NPAMAX/4=170Mb, NPAMAX/10=124Mb <- from RAM memory
		ca_c2c0[i] = 0.0+0.0*I;
		da_d2c0[i] = 0.0;
		ca_d2c0[i] = 0.0+0.0*I;
		ca_c2c1[i] = 0.0+0.0*I;
		da_d2c1[i] = 0.0;
		ca_d2c1[i] = 0.0+0.0*I;
		ca_c2c2[i] = 0.0+0.0*I;
		da_d2c2[i] = 0.0;
		ca_d2c2[i] = 0.0+0.0*I;
		ca_c2c3[i] = 0.0+0.0*I;
		da_d2c3[i] = 0.0;
		ca_d2c3[i] = 0.0+0.0*I;
		ca_c2c4[i] = 0.0+0.0*I;
		da_d2c4[i] = 0.0;
		ca_d2c4[i] = 0.0+0.0*I;
		ca_c2c5[i] = 0.0+0.0*I;
		da_d2c5[i] = 0.0;
		ca_d2c5[i] = 0.0+0.0*I;
	}
    //for (int i = 0; i < NPMAX; ++i)
    //{
    	//fftw_destroy_plan(plan_c2c0[i]);
    	//fftw_destroy_plan(plan_c2c1[i]);
    	//fftw_destroy_plan(plan_c2c2[i]);
    	//fftw_destroy_plan(plan_c2c3[i]);
    	//fftw_destroy_plan(plan_c2c4[i]);
    	//fftw_destroy_plan(plan_c2c5[i]);

    	//fftw_destroy_plan(plan_d2c0[i]);
    	//fftw_destroy_plan(plan_d2c1[i]);
    	//fftw_destroy_plan(plan_d2c2[i]);
    	//fftw_destroy_plan(plan_d2c3[i]);
    	//fftw_destroy_plan(plan_d2c4[i]);
    	//fftw_destroy_plan(plan_d2c5[i]);
   	//}
}*/
//// END F2A ///

////  POMALL ///
void PomAll::initPomAll()
{
    for (int i = 0; i < NMAX_PT+50; ++i) pctile_shell_tmp[i] = 0.0;//int NMAX=141072;
}
double PomAll::peakup(double ym,double y0,double yp)
{
    double dx = 0.0;
    double b=(yp-ym)/2.0;
    double c=(yp+ym-(2*y0))/2.0;
    if (c==0.0) c=0.0000001; //2.76 qDebug()<<c;
    dx=-b/(2.0*c);
    return dx;
}
double PomAll::maxval_da_beg_to_end(double*a,int a_beg,int a_end)
{
    double max = a[a_beg];
    for (int i = a_beg; i < a_end; ++i)
    {
        if (a[i]>max)
        {
            max = a[i];
        }
    }
    return max;
}
double PomAll::minval_da_beg_to_end(double*a,int a_beg,int a_end)
{
    double min = a[a_beg];
    for (int i = a_beg; i < a_end; ++i)
    {
        if (a[i]<min)
        {
            min = a[i];
        }
    }
    return min;
}
int PomAll::maxval_ia_beg_to_end(int*a,int a_beg,int a_end)//2.76sf
{
    int max = a[a_beg];
    for (int i = a_beg; i < a_end; i++)
    {
        if (a[i]>max)
        {
            max = a[i];
        }
    }
    return max;
}
int PomAll::maxloc_da_end_to_beg(double*a,int a_beg,int a_end)
{
    double max = a[a_end-1];
    int loc = a_end-1;
    for (int i = a_end-1; i >= a_beg; --i)
    {
        if (a[i]>max)
        {
            loc = i;
            max = a[i];
        }
    }
    return loc;
}
int PomAll::maxloc_da_beg_to_end(double*a,int a_beg,int a_end)
{
    double max = a[a_beg];
    int loc = a_beg;
    for (int i = a_beg; i < a_end; ++i)
    {
        if (a[i]>max)
        {
            loc = i;
            max = a[i];
        }
    }
    return loc;
}
int PomAll::minloc_da_beg_to_end(double*a,int a_beg,int a_end)
{
    double min = a[a_beg];
    int loc = a_beg;
    for (int i = a_beg; i < a_end; ++i)
    {
        if (a[i]<min)
        {
            loc = i;
            min = a[i];
        }
    }
    return loc;
}
int PomAll::maxloc_ia_beg_to_end(int*a,int a_beg,int a_end)
{
    int max = a[a_beg];
    int loc = a_beg;
    for (int i = a_beg; i < a_end; ++i)
    {
        if (a[i]>max)
        {
            loc = i;
            max = a[i];
        }
    }
    return loc;
}
double PomAll::db(double x)
{
    double db=-99.0;
    if (x>1.259e-10)
    {
        if (x<0.000001) x=0.000001;
        db=10.0*log10(x);
    }
    return db;
}
double PomAll::determ(double array_[10][10],int norder)
{
    //real*8 function determ(array,norder)
    //implicit real*8 (a-h,o-z)
    //real*8 array(10,10)
    double determ=1.0;
    for (int k = 0; k<norder; k++)
    {//do k=1,norder
        if (array_[k][k]!=0) goto c41;
        int j;
        for (j = k; j<norder; j++)
        {//do j=k,norder
            if (array_[j][k]!=0) goto c31;
        }
        determ=0.0;
        goto c60;
c31:
        for (int i = k; i<norder; i++)
        {//do i=k,norder
            double s8tt=array_[j][i];
            array_[j][i]=array_[k][i];
            array_[k][i]=s8tt;
        }
        determ=-1.0*determ;
c41:
        determ=determ*array_[k][k];
        if (k<norder-1)   //hv -1 if(k.lt.norder) then
        {
            int k1=k+1;

            for (int i = k1; i<norder; i++)
            {//do i=k1,norder
                for (j = k1; j<norder; j++)
                {//do j=k1,norder
                    //array(i,j)=array(i,j)-array(i,k)*array(k,j)/array(k,k)
                    double tarr = array_[k][k];//2.76
                    if (tarr==0.0) tarr = 0.000001;//qDebug()<<"zero="<<array_[k][k];
                    array_[j][i]=array_[j][i]-array_[k][i]*array_[j][k]/tarr;//array_[j][i]=array_[j][i]-array_[k][i]*array_[j][k]/array_[k][k];
                }
            }
        }
    }
c60:
    //qDebug()<<determ;
    return determ;
}
void PomAll::polyfit(double*x,double*y,double*sigmay,int npts,int nterms,int mode,double*a,double &chisqr)
{
    //subroutine polyfit(x,y,sigmay,npts,nterms,mode,a,chisqr)
    //implicit real*8 (a-h,o-z)
    //real*8 x(npts), y(npts), sigmay(npts), a(nterms)
    //real*8 sumx(10), sumy(10), array(10,10)
    double sumx[10];
    double sumy[10];
    double array_[10][10];
    //double sigmay[npts];

    //! Accumulate weighted sums
    int nmax = 2*nterms-1;//hv -1=0
    zero_double_beg_end(sumx,0,10);
    zero_double_beg_end(sumy,0,10);
    double chisq=0.0;
    for (int i = 0; i<npts; i++)
    {//do i=1,npts //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
        double xi=x[i];
        double yi=y[i];
        double weight=0.0;
        if (mode<0)
        {
            double yi0 = yi;
            if (yi0==0.0) yi0=0.000001;//2.76
            weight=1.0/fabs(yi0);
        }
        else if (mode==0) weight=1.0;
        else
        {
            double si2 = sigmay[i]*sigmay[i];
            if (si2==0.0) si2=0.000001;//2.76
            weight=1.0/(si2);//weight=1./sigmay(i)**2
        }
        //qDebug()<<"weight"<<weight<<sigmay[i];

        double xterm=weight;
        for (int n = 0; n<nmax; n++)
        {//do n=1,nmax
            sumx[n]=sumx[n]+xterm;
            xterm=xterm*xi;
            //qDebug()<<"chisq"<<sumx[n]<<xterm<<xi<<n;
        }
        double yterm=weight*yi;
        for (int n = 0; n<nterms; n++)
        {//do n=1,nterms
            sumy[n]=sumy[n]+yterm;
            yterm=yterm*xi;
        }
        chisq+=weight*(yi*yi); //chisq=chisq+weight*yi**2
    }

    //! Construct matrices and calculate coefficients
    for (int j = 0; j<nterms; j++)
    {//do j=1,nterms
        for (int k = 0; k<nterms; k++)
        {//do k=1,nterms
            int n=j+k-0;//-1
            array_[k][j]=sumx[n];
            //qDebug()<<n;
        }
    }

    double delta=determ(array_,nterms); //qDebug()<<"delta="<<delta;
    if (delta==0.0)
    {
        chisqr=0.0;
        zero_double_beg_end(a,0,5);
    }
    else
    {
        for (int l = 0; l<nterms; l++)
        {//do l=1,nterms
            for (int j = 0; j<nterms; j++)
            {//do j=1,nterms
                for (int k = 0; k<nterms; k++)
                {//do k=1,nterms
                    int n=j+k-0; // -1
                    array_[k][j]=sumx[n];
                }
                array_[l][j]=sumy[j];
            }
            a[l]=determ(array_,nterms)/delta;
        }

        //! Calculate chi square
        for (int j = 0; j<nterms; j++)
        {//do j=1,nterms
            chisq-=2.0*a[j]*sumy[j];  //chisq=chisq-2*a(j)*sumy(j)
            for (int k = 0; k<nterms; k++)
            {//do k=1,nterms
                int n=j+k-0;   // -1
                chisq+=a[j]*a[k]*sumx[n]; //chisq=chisq+a(j)*a(k)*sumx(n)
            }
        }
        double free=npts-nterms; //qDebug()<<"free"<<free;
        if (free==0.0) free=1.0;// no devide by zero
        chisqr=chisq/free;
        //qDebug()<<chisq<<free;
    } //qDebug()<<"chisq"<<a[0]<<a[1]<<a[2]<<a[3]<<a[4];
}
void PomAll::zero_double_beg_end(double*d,int begin,int end)
{
    for (int i = begin; i<end; ++i) d[i]=0.0;
}
void PomAll::shell(int n,double*a)
{
    int i,j,inc;
    double v;
    //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    inc=1;//inc=1;
c1:
    inc=3*inc+1;
    if (inc<=n) goto c1;
c2:
    inc=inc/3;
    //qDebug()<<"INC="<<inc<<n;
    for (i = inc+0; i < n+0; i++)
    {//do i=inc+1,n
        //qDebug()<<"MINA"<<i;
        v=a[i];
        j=i;

c3:
        if (a[j-inc]>v)
        {
            a[j]=a[j-inc];
            j=j-inc;
            if (j<=inc) goto c4;
            goto c3;
        }
c4:
        a[j]=v;
    }

    if (inc>1) goto c2;
}
double PomAll::pctile_shell(double *x,int npts,int npct)
{
    double xpct = 1.0;
    //const int NMAX=141072;// 100000;//100000  32768
    //real*4 x(npts)
    //double tmp[NMAX];// ={0.0};//real*4 tmp(NMAX)
    //double *tmp = new double[NMAX];// ={0.0};//real*4 tmp(NMAX)
    /*static int sss = 0;
    if (npts>sss)
    {
    	sss=npts; 
    	qDebug()<<sss;	
    }*/
    int j =0; //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    if (npts<=0)//if(npts<0 || npct<0 || npct>100)
    {
        xpct=1.0;
        goto c900;
    }
    if (npts>NMAX_PT) return xpct; // if(npts.gt.NMAX) stop

    for (j = 0; j < npts; j++) pctile_shell_tmp[j]=x[j];  // tmp(1:npts)=x +beg

    shell(npts,pctile_shell_tmp);

    j=(int)((double)npts*0.01*npct);
    //sort(npts,tmp);
    if (j<0) j=0;//0 if(j.lt.1) j=1
    if (j>npts-1) j=npts-1; // if(j.gt.npts) j=npts
    xpct=pctile_shell_tmp[j];

c900:
    //if (xpct==0.0) xpct=0.0000001;//for the future ???
    return xpct;
}
void PomAll::zero_double_comp_beg_end(double complex*d,int begin,int end)
{
    for (int i = begin; i<end; ++i) d[i]=0.0+0.0*I;
}
double PomAll::ps_hv(double complex z)
{
    double d;//(real(c(i))**2 + aimag(c(i))**2)
    d = creal(z)*creal(z) + cimag(z)*cimag(z);//d = pow(creal(z),2) + pow(cimag(z),2);
    return d;
}
/*double PomAll::ps_hv001(double complex z)
{
    double d;
    d = (creal(z)*creal(z)*0.01) + (cimag(z)*cimag(z)*0.01);
    return d;
}*/
void PomAll::cshift1(double complex *a,int cou_a,int ish)
{
    //HV for single shift vareable
    //Left Shift 	ISHFT 	ISHFT(N,M) (M > 0) 	<< 	n<<m 	n shifted left by m bits
    //Right Shift 	ISHFT 	ISHFT(N,M) (M < 0) 	>> 	n>>m 	n shifted right by m bits
    //double complex t[cou_a];  //garmi hv v1.42
    //double complex t[cou_a*2+ish+50];  //garmi hv v1.43 ok
    double complex *t = new double complex[cou_a+100]; //garmi pri goliam count hv v1.43 correct ok
    for (int i=0; i< cou_a; i++) t[i]=a[i];
    if (ish>0)
    {
        for (int i = 0; i <  cou_a; i++)
        {
            if (i+ish<cou_a) a[i]=t[i+ish];
            else a[i]=t[i+ish-cou_a];
        }
    }
    if (ish<0)
    {
        for (int i = 0; i <  cou_a; i++)
        {
            if (i+ish<0) a[i]=t[i+ish+cou_a];
            else a[i]=t[i+ish];
        }
    }
    delete [] t;
}
void PomAll::dshift1(double *a,int cou_a,int ish)
{
    //HV for single shift vareable
    //Left Shift 	ISHFT 	ISHFT(N,M) (M > 0) 	<< 	n<<m 	n shifted left by m bits
    //Right Shift 	ISHFT 	ISHFT(N,M) (M < 0) 	>> 	n>>m 	n shifted right by m bits
    //double complex t[cou_a];  //garmi hv v1.42
    //double complex t[cou_a*2+ish+50];  //garmi hv v1.43 ok
    double *t = new double[cou_a+100]; //garmi pri goliam count hv v1.43 correct ok
    for (int i=0; i< cou_a; i++) t[i]=a[i];
    if (ish>0)
    {
        for (int i = 0; i <  cou_a; i++)
        {
            if (i+ish<cou_a) a[i]=t[i+ish];
            else a[i]=t[i+ish-cou_a];
        }
    }
    if (ish<0)
    {
        for (int i = 0; i <  cou_a; i++)
        {
            if (i+ish<0) a[i]=t[i+ish+cou_a];
            else a[i]=t[i+ish];
        }
    }
    delete [] t;
}
double complex PomAll::sum_dca_mplay_conj_dca(double complex *a,int a_beg,int a_end,double complex *b)
{
    double complex sum = 0.0+0.0*I;
    int b_c = 0;
    for (int i = a_beg; i < a_end; i++)//1.68 i++ ok
    {
        sum+=a[i]*conj(b[b_c]);
        b_c++;
    }
    return sum;
}
void PomAll::indexx_msk(double *arr,int n,int *indx)
{
    int M=7;
    const int NSTACK=50;
    //integer n,indx(n)
    //real arr(n)
    int i,indxt,ir,itemp,j,jstack,k,l;
    int istack[NSTACK+50];//v1.46 +5
    double a;

    for (j = 0; j <= n; j++)
    {//do j=1,n
        indx[j]=j;
    }

    jstack=-1;//-1-hv 0;
    l=0;//l=1;
    ir=n; //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
c1:
    if (ir-l<M)
    {
        for (j = l+1; j <= ir; j++)
        {//do j=l+1,ir
            indxt=indx[j];
            a=arr[indxt];
            for (i = j-1; i >= 0; i--)
            {//do i=j-1,1,-1
                if (arr[indx[i]]<=a) goto c2;
                indx[i+1]=indx[i];
            }
            i=-1;//-1-hv 0;
c2:
            indx[i+1]=indxt;
            //if(indxt>350 || indxt<5)
            //qDebug()<<"indxt="<<indxt<<"max="<<n;
        }
        if (jstack<1) return;//2.76 if (jstack==-1) return;//-1-hv 0;

        ir=istack[jstack];
        l=istack[jstack-1];
        jstack=jstack-2;
    }
    else
    {
        k=(l+ir)/2;
        itemp=indx[k];
        indx[k]=indx[l+1];
        indx[l+1]=itemp;

        if (arr[indx[l+1]]>arr[indx[ir]])
        {
            itemp=indx[l+1];
            indx[l+1]=indx[ir];
            indx[ir]=itemp;
        }
        //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
        if (arr[indx[l]]>arr[indx[ir]])
        {
            itemp=indx[l];
            indx[l]=indx[ir];
            indx[ir]=itemp;
        }

        if (arr[indx[l+1]]>arr[indx[l]])
        {
            itemp=indx[l+1];
            indx[l+1]=indx[l];
            indx[l]=itemp;
        }

        i=l+1;
        j=ir;
        indxt=indx[l];
        a=arr[indxt];
c3:    //continue
        i=i+1;
        if (arr[indx[i]]<a) goto c3;

c4:    //continue
        j=j-1;
        if (j<0) j=0;//2.76 qDebug()<<j;
        if (arr[indx[j]]>a) goto c4;
        if (j<i) goto c5;
        itemp=indx[i];
        indx[i]=indx[j];
        indx[j]=itemp;
        goto c3;

c5:
        indx[l]=indx[j];
        indx[j]=indxt;
        jstack=jstack+2;
        if (jstack>NSTACK) return;//stop //'NSTACK too small in indexx'
        if (ir-i+1>=j-l)
        {
            istack[jstack]=ir;
            istack[jstack-1]=i;
            ir=j-1;
        }
        else
        {
            istack[jstack]=j-1;
            istack[jstack-1]=l;
            l=i;
        }
    }
    goto c1;
}
bool PomAll::isStandardCall(QString w)//2.61 same as  MultiAnswerModW
{
    //static QRegularExpression standard_call_re {
    //  R"(
    //      ^\s*				# optional leading spaces
    //      ( [A-Z]{0,2} | [A-Z][0-9] | [0-9][A-Z] )  # part 1
    //      ( [0-9][A-Z]{0,3} )                       # part 2
    //      (/R | /P)?			# optional suffix
    //      \s*$				# optional trailing spaces
    //  )", QRegularExpression::CaseInsensitiveOption | QRegularExpression::ExtendedPatternSyntaxOption};
    //return standard_call_re.match (w).hasMatch ();
    QRegExp rx("^\\s*([A-Z]{0,2}|[A-Z][0-9]|[0-9][A-Z])([0-9][A-Z]{0,3})(/R|/P)?\\s*$");
    rx.setCaseSensitivity(Qt::CaseInsensitive);
    bool res0 = rx.exactMatch(w); //qDebug()<<w<<res0;
    return res0;
}
void PomAll::smo121(double *x,int beg,int nz)
{
    double x0=x[beg];
    for (int i =1; i<nz-1; i++)
    {//do i=2,nz-1
        double x1=x[i+beg];
        x[i+beg]=0.5*x[i+beg] + 0.25*(x0+x[i+1+beg]);// x(i)=0.5*x(i) + 0.25*(x0+x(i+1))
        x0=x1;
    }
}
/*bool PomAll::isStandardCall(QString w)
{
    bool res = true;
    bool spr = true;
    short n = w.count();
    if (n<2) return false;
    if (w[n-2]=='/')
    {
    	if (w.at(n-1)=='P' || w.at(n-1)=='R') n = n - 2;
    	else spr = false;
   	}
    //! Check for standard callsign
    short iarea=-1;
    //int n = strlen(callsign);  //n=len(trim(callsign))
    int i;
    for (i = n-1; i >=1; --i)
    {//do i=n,2,-1
        if (w.at(i).isDigit()) break;//exit
    }
    iarea=i;                                   //!Right-most digit (call area)
    short npdig=0;                                   //!Digits before call area
    short nplet=0;                                   //!Letters before call area
    for (i = 0; i < iarea; ++i)
    {//do i=1,iarea-1
        if (w.at(i).isDigit())  npdig++;
        if (w.at(i).isLetter()) nplet++;
    }
    short nslet=0;                                   //!Letters in suffix
    for (i = iarea+1; i < n; ++i)
    {//do i=iarea+1,n
        if (w.at(i).isLetter()) nslet++;
    }
    if (iarea<1 || iarea>2 || nplet==0 || npdig>=iarea || nslet>3) res = false;
    if (!res || !spr) res = false;
    return res;
}*/
/*bool PomAll::is_digit(char c)
{
    bool res = false;
    if (c>='0' && c<='9') res = true;
    return res;
}
bool PomAll::is_letter(char c)
{
    bool res = false;
    if (c>='A' && c<='Z') res = true;
    return res;
}
bool PomAll::isStandardCall(QString callsign0)//not correct XX2XX/P or R is standard
{
	char callsign[100];
    strncpy(callsign,callsign0.toUtf8(),32);
	int n = callsign0.count();

    bool res = true;
    bool spr = true;
    if (n<2) return false;
    if (callsign[n-2]=='/')
    {
    	if (callsign[n-1]=='P' || callsign[n-1]=='R') n = n - 2;
    	else spr = false;
   	}
    //! Check for standard callsign
    int iarea=-1;
    //int n = strlen(callsign);  //n=len(trim(callsign))
    int i;
    for (i = n-1; i >=1; --i)
    {//do i=n,2,-1
        if (is_digit(callsign[i])) break;//exit
    }
    iarea=i;                                   //!Right-most digit (call area)
    int npdig=0;                                   //!Digits before call area
    int nplet=0;                                   //!Letters before call area
    for (i = 0; i < iarea; ++i)
    {//do i=1,iarea-1
        if (is_digit(callsign[i]))  npdig++;
        if (is_letter(callsign[i])) nplet++;
    }
    int nslet=0;                                   //!Letters in suffix
    for (i = iarea+1; i < n; ++i)
    {//do i=iarea+1,n
        if (is_letter(callsign[i])) nslet++;
    }
    //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    //if(iarea<2 || iarea>3 || nplet==0 || npdig>=iarea-1 || nslet>3)
    if (iarea<1 || iarea>2 || nplet==0 || npdig>=iarea || nslet>3) res = false;
    if (!res || !spr) res = false;
    return res;
}*/
//// END  POMALL ///

////  POMFT ///
#define MN_NM_NRW_FT_174_91
#include "../HvMsPlayer/libsound/HvGenFt8/bpdecode_ft8_174_91.h"
/*PomFt::PomFt()
{
    lastpat_ft8_2 = 0;
    inext_ft8_2 = 0;
    first_osd174_91 = true;
    twopi=8.0*atan(1.0);
}
PomFt::~PomFt()
{}*/
void PomFt::initPomFt()
{
    //pomAll.initPomAll();//2.66 no pctile_shell no need init
    lastpat_ft8_2 = -1;//0;
    inext_ft8_2 = -1;//0;
    first_osd174_91 = true;
    first_enc174_91_nocrc = true;
    twopi=8.0*atan(1.0);
    pi=4.0*atan(1.0);
}
bool PomFt::isgrid4(QString s)
{
    bool res = false;
    if (s.count()>3)
    {
        int c1 = (int)s.at(0).toLatin1();
        int c2 = (int)s.at(1).toLatin1();
        int c3 = (int)s.at(2).toLatin1();
        int c4 = (int)s.at(3).toLatin1();
        if (c1>=(int)'A' && c1<=(int)'R' && c2>=(int)'A' && c2<=(int)'R' &&
                c3>=(int)'0' && c3<=(int)'9' && c4>=(int)'0' && c4<=(int)'9')// && s.mid(0,4)!="RR73"
            res = true;
    }
    return res;
}
#include "../HvMsPlayer/libsound/boost/boost_14.hpp"
short crc14_pomft(unsigned char const * data, int length)
{
    return boost::augmented_crc<14, TRUNCATED_POLYNOMIAL14>(data, length);
}
short PomFt::crc14(unsigned char const * data, int length)
{
    return crc14_pomft(data,length);
}

void PomFt::nuttal_window(double *win,int n)
{
    double a0=0.3635819;
    double a1=-0.4891775;
    double a2=0.1365995;
    double a3=-0.0106411;
    for (int i = 0; i < n; ++i)
    {//do i=1,n
        //win[i]=a0+a1*cos(2*pi*(i-1)/(n))+a2*cos(4*pi*(i-1)/(n))+a3*cos(6*pi*(i-1)/(n));
        win[i]=a0+a1*cos(2.0*pi*(double)i/(double)n)+a2*cos(4.0*pi*(double)i/(double)n)+a3*cos(6.0*pi*(double)i/(double)n);
    }
}
void PomFt::normalizebmet(double *bmet,int n)
{
    double bmetav = 0.0;
    double bmet2av = 0.0;
    for (int z = 0; z < n; ++z)
    {
        bmetav+=bmet[z];////bmetav=sum(bmet)/real(n)
        bmet2av+=bmet[z]*bmet[z];//bmet2av=sum(bmet*bmet)/real(n)
    }
    bmetav=bmetav/(double)n;
    bmet2av=bmet2av/(double)n;
    double var=bmet2av-bmetav*bmetav;//var=bmet2av-bmetav*bmetav
    double bmetsig = 0.0;
    if ( var > 0.0 ) bmetsig=sqrt(var);//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    else bmetsig=sqrt(bmet2av);
    if (bmetsig<=0.00001) bmetsig=0.00001;//2.68 crash no devide by zero
    for (int z = 0; z < n; ++z) bmet[z]=bmet[z]/bmetsig;
}
void PomFt::normalizebmetvar(double *bmet,int n)
{
    double bmet2av = 0.0;
    double bmetsig = 0.0;
    for (int z = 0; z < n; ++z) bmet2av+=bmet[z]*bmet[z];
    bmet2av/=(double)n;
    bmetsig=sqrt(bmet2av);
    if (bmetsig<=0.000001) bmetsig=0.000001;
    for (int z = 0; z < n; ++z) bmet[z]/=bmetsig;
}
void PomFt::twkfreq1(double complex *ca,int npts,double fsample,double *a,double complex *cb)
{
    //! Mix the complex signal
    double complex w=1.0+1.0*I;
    //double complex wstep=1.0+1.0*I;
    int x0=0.5*(npts);//x0=0.5*(npts+1)
    double s=2.0/(double)npts;//s=2.0/npts
    if (fsample==0.0) fsample=0.00000001;//2.76
    for (int i =0; i<npts; ++i)
    {//do i=1,npts
        double x=s*(i-x0);//x=s*(i-x0)
        double p2=1.5*x*x - 0.5;                            //p2=1.5*x*x - 0.5
        double p3=2.5*pow(x,3.0) - 1.5*x;                     //p3=2.5*(x**3) - 1.5*x
        double p4=4.375*pow(x,4.0) - 3.75*pow(x,2.0) + 0.375;    //p4=4.375*(x**4) - 3.75*(x**2) + 0.375
        double dphi=(a[0] + x*a[1] + p2*a[2] + p3*a[3] + p4*a[4]) * (twopi/fsample);
        double complex wstep=cos(dphi)+sin(dphi)*I;//wstep=cmplx(cos(dphi),sin(dphi))
        w=w*wstep;
        cb[i]=w*ca[i];
    }
}
void PomFt::SetLlrFt2_4(double *llra,double *llrb,double *llrc,double *llrd,double *llre,double bitmetrics_[5][220],uint8_t to)
{
    const double scalefac=2.83;//0->to=ALL 3->to=C 4->to=D
    for (int x = 0; x < 58; ++x)
    {
        llra[x]=bitmetrics_[0][x+8]*scalefac;
        llra[x+58]=bitmetrics_[0][x+74]*scalefac;
        llra[x+116]=bitmetrics_[0][x+140]*scalefac;
        llrb[x]=bitmetrics_[1][x+8]*scalefac;
        llrb[x+58]=bitmetrics_[1][x+74]*scalefac;
        llrb[x+116]=bitmetrics_[1][x+140]*scalefac;
        llrc[x]=bitmetrics_[2][x+8]*scalefac;
        llrc[x+58]=bitmetrics_[2][x+74]*scalefac;
        llrc[x+116]=bitmetrics_[2][x+140]*scalefac;
        if (to==3) continue;
        llrd[x]=bitmetrics_[3][x+8]*scalefac;
        llrd[x+58]=bitmetrics_[3][x+74]*scalefac;
        llrd[x+116]=bitmetrics_[3][x+140]*scalefac;
        if (to==4) continue;
        llre[x]=bitmetrics_[4][x+8]*scalefac;
        llre[x+58]=bitmetrics_[4][x+74]*scalefac;
        llre[x+116]=bitmetrics_[4][x+140]*scalefac;
    }
}
int PomFt::count_eq_bits(bool *a,int b_a,const bool *b,int c)
{
    int ns1=0;
    for (int x = 0; x < c; ++x)
    {
        if (a[x+b_a]==b[x]) ns1++;
    }
    return ns1;
}
int PomFt::SyncQualFt2_4(double *bitmetrics)
{
    const int ND=87;
    const int NS=16;
    const int NN=NS+ND;
    const bool ms1[8] =
        {
            0,0,0,1,1,0,1,1
        };
    const bool ms2[8] =
        {
            0,1,0,0,1,1,1,0
        };
    const bool ms3[8] =
        {
            1,1,1,0,0,1,0,0
        };
    const bool ms4[8] =
        {
            1,0,1,1,0,0,0,1
        };
    int nsync_qual=0;
    bool hbits[220];
    for (int x = 0; x < 2*NN; ++x)
    {
        if (bitmetrics[x]>=0.0) hbits[x]=1;
        else hbits[x]=0;
    }
    int ns1=count_eq_bits(hbits,0,  ms1,8);
    int ns2=count_eq_bits(hbits,66, ms2,8);
    int ns3=count_eq_bits(hbits,132,ms3,8);
    int ns4=count_eq_bits(hbits,198,ms4,8);
    nsync_qual=ns1+ns2+ns3+ns4;
    return nsync_qual;
}
void PomFt::SetApFt2_4(double *llr,double *llr_c,bool *apmask,int *apbits,int *mcq,int *apmy_ru,int *aphis_fd,double apmag,int iaptype,int cont_type)
{
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
    const int ND=87;
    double llr_ap[180];
    for (int z = 0; z < 2*ND; ++z) llr_ap[z]=llr_c[z];
    if (iaptype==1) //then  ! CQ or CQ TEST or CQ FD or CQ RU or CQ SCC
    {
        for (int z = 0; z < 2*ND; ++z)//2*ND=174
        {
            apmask[z]=0;
            if (z<29)
            {
                apmask[z]=1;
                llr_ap[z]=apmag*(double)mcq[z];
            }
        } //qDebug()<<iaptype;
    }
    if (iaptype==2) //then ! MyCall,???,???
    {
        for (int z = 0; z < 2*ND; ++z) apmask[z]=0;
        if (cont_type==0 || cont_type==1)// || ncontest==5
        {
            for (int z = 0; z < 29; ++z)
            {
                apmask[z]=1;//apmask(1:29)=1
                llr_ap[z]=apmag*(double)apbits[z];//llrd(1:29)=apmag*apsym(1:29)
            }
        }
        else if (cont_type==2)
        {
            for (int z = 0; z < 28; ++z)
            {
                apmask[z]=1;//apmask(1:28)=1
                llr_ap[z]=apmag*(double)apbits[z];//llrd(1:28)=apmag*apsym(1:28)
            }
        }
        else if (cont_type==3)
        {
            for (int z = 0; z < 28; ++z)
            {
                apmask[z]=1;  //apmask(1:28)=1
                llr_ap[z]=apmag*(double)apbits[z];//llrd(1:28)=apmag*apbits(1:28)
            }
        }
        else if (cont_type==4)//RTTY UU HV-6=BU   || ncontest==6
        {
            for (int z = 1; z < 29; ++z)
            {
                apmask[z]=1;//apmask(2:29)=1
                llr_ap[z]=apmag*(double)apmy_ru[z-1];//llrd(2:29)=apmag*apmy_ru(1:28)
            }
        }
    }
    if (iaptype==3) //then ! MyCall,DxCall,???
    {
        for (int z = 0; z < 2*ND; ++z) apmask[z]=0;
        if (cont_type==0 || cont_type==1 || cont_type==2)// || ncontest==5
        {
            for (int z = 0; z < 58; ++z)
            {
                apmask[z]=1;// apmask(1:58)=1
                llr_ap[z]=apmag*(double)apbits[z];// llrd(1:58)=apmag*apbits(1:58)
            }
        }
        else if (cont_type==3) //then ! Field Day
        {
            for (int z = 0; z < 56; ++z)//2.36 57 or 56 ???
            {
                apmask[z]=1;//apmask(1:56)=1//2.36 if (z<56)//57 or 56 ???
                if (z<28)
                {
                    llr_ap[z]=apmag*(double)apbits[z];//llrd(1:28)=apmag*apbits(1:28)
                    llr_ap[z+28]=apmag*(double)aphis_fd[z];//llrd(29:56)=apmag*aphis_fd(1:28)????
                }
            }
        }
        else if (cont_type==4)//then ! RTTY RU HV 6=BU // || ncontest==6
        {
            for (int z = 0; z < 57; ++z)
            {
                if (z>0) apmask[z]=1;// apmask(2:57)=1
                if (z<28) llr_ap[z+1]=apmag*(double)apmy_ru[z];//llrd(2:29)=apmag*apmy_ru(1:28)
                if (z>28) llr_ap[z]=apmag*(double)apbits[z];// llrd(30:57)=apmag*apbits(30:57)
            }
        }
    }
    if (iaptype==4 || iaptype==5 || iaptype==6)
    {
        for (int z = 0; z < 2*ND; ++z) apmask[z]=0;//if(ncontest.le.5) then
        if (cont_type<=4)//hv new=4
        {
            for (int z = 0; z < 77; ++z) apmask[z]=1; //apmask(1:77)=1   //! mycall, hiscall, RRR|73|RR73
            if (iaptype==6)
            {
                for (int z = 0; z < 77; ++z) llr_ap[z]=apmag*(double)apbits[z];//llrd(1:77)=apmag*apbits(1:77)
            }
        }
    }
    for (int z = 0; z < 2*ND; ++z) llr[z]=llr_ap[z];
}
bool PomFt::SetAp7Msg(QString call_1,bool std_1,QString call_2,bool std_2,QString grid4,int i,QString &msg,int &count_msg)
{
    //const int MAXMSG=206;//= -50 to +49
    const int MAXMSG=158;//= -26 to +49
    msg = call_1+" "+call_2;
    if (call_1=="CQ" && i!=4) msg = "QU1RK "+call_2;
    if (!std_1)
    {
        if (i==0 || i>=5) msg = "<"+call_1+"> "+call_2;
        if (i>=1 && i<=3) msg = call_1+" <"+call_2+">";
    }
    else if (!std_2)
    {
        if (call_1=="CQ" && i!=4) msg = "QU1RK "+call_2;//HV add
        else
        {
            if (i<=3 || i==5) msg = "<"+call_1+"> "+call_2;
            if (i>=6) msg = call_1+" <"+call_2+">";  //if (i==5) msg = "TNX 73 GL";
        }
    }
    //else if (i==0) msg = msg0.trimmed();
    if (i==1) msg.append(" RRR");
    if (i==2) msg.append(" RR73");
    if (i==3) msg.append(" 73");
    if (i==4)
    {
        if (std_2)
        {
            msg = "CQ "+call_2;//+" "+hisgrid.mid(0,4);//KN23SF
            QString call_10 = call_1+"xxxx";//2.70
            if (call_10.mid(2,1)=="_") msg = call_1+" "+call_2;
            if (grid4!="RR73") msg = msg.trimmed()+" "+grid4;
        }
        if (!std_2) msg = "CQ "+call_2;//KN23SF
    }
    if (i==5 && std_2) msg.append(" "+grid4.mid(0,4));
    if (i>=6 && i<MAXMSG)
    {
        if (i>12 && msg.startsWith("QU1RK "))
        {
            //not possyble "CQ <TM22KPW> -26"
            //not possyble "CQ LZ2HV R-26"
            //qDebug()<<"BREAK--->"<<msg<<"msgbest="<<msgbest;
            count_msg = i;
            return true;//break;
        }
        //int isnr = -50 + (i-6)/2;// -50 to +49 MAXMSG=206
        int isnr = -26 + (i-6)/2;  // -26 to +49 MAXMSG=158
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
    return false;
}
void PomFt::TryDecAp7(double *llra,double *llrb,double *llrc,double *llrd,bool *cw,double *dmm,int i,
                      QString msgsent,double pow0,double &dmin,QString &msgbest,double &pbest,int &nharderrors)
{
    bool hdec[178];
    bool nxor[178];
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
}
/*void PomFt::twkfreq2(double complex *c3,double complex *c4,int npts,double fsample,double fshift)
{
    //! Adjust frequency of complex waveform
    double twopi=6.283185307;
    double complex w=1.0+1.0*I;
    double dphi=fshift*twopi/fsample;
    double complex wstep=cos(dphi)+sin(dphi)*I;
    for (int i =0; i<npts; ++i)
    {//do i=1,npts
        w=w*wstep;
        c4[i]=w*c3[i];
    }
}
int PomFt::maxloc_ia_beg_to_end(int*a,int a_beg,int a_end)
{
    int max = a[a_beg];
    int loc = a_beg;
    for (int i = a_beg; i < a_end; ++i)
    {
        if (a[i]>max)
        {
            loc = i;
            max = a[i];
        }
    }
    return loc;
}*/
void PomFt::chkcrc14a(bool *decoded,int &nbadcrc)
{
    unsigned char i1Dec8BitBytes[12+5];
    for (int ibyte = 0; ibyte<12; ++ibyte)
    {//do ibyte=1,10
        int itmp=0;
        for (int ibit = 0; ibit<8; ++ibit)
        {//do ibit=1,8
            itmp=(itmp << 1)+(1 & decoded[(ibyte-0)*8+ibit]);//itmp=ishft(itmp,1)+iand(1,decoded((ibyte-1)*8+ibit))
            //qDebug()<<(int)decoded[(ibyte-0)*7+ibit];
        }
        i1Dec8BitBytes[ibyte]=itmp;
    }
    int ncrc14=0;
    for (int ibit = 0; ibit<14; ++ibit) ncrc14=(ncrc14 << 1)+(1 & decoded[77+ibit]);
    i1Dec8BitBytes[9]=(i1Dec8BitBytes[9] & 248); //i1Dec8BitBytes(10)=iand(i1Dec8BitBytes(10),128+64+32+16+8)
    i1Dec8BitBytes[10]=0;
    i1Dec8BitBytes[11]=0;
    int icrc14=crc14(i1Dec8BitBytes,12);
    nbadcrc=1;  //qDebug()<<"ncrc14==icrc14"<<ncrc14<<icrc14;
    if (ncrc14==icrc14) nbadcrc=0;// && ncrc14!=0  sum_5687   || sum_5687==0
}
void PomFt::platanh(double x, double &y)
{
    double isign=+1.0;
    double z;
    z=x;
    //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    if ( x < 0.0 )
    {
        isign=-1.0;
        z=fabs(x);
    }
    if ( z <= 0.664 )
    {
        y=x/0.83;
        return;
    }
    else if ( z <= 0.9217 )
    {
        y=isign*(z - 0.4064)/0.322;
        return;
    }
    else if ( z <= 0.9951 )
    {
        y=isign*(z - 0.8378)/0.0524;
        return;
    }
    else if ( z <= 0.9998 )
    {
        y=isign*(z - 0.9914)/0.0012;
        return;
    }
    else
    {
        y=isign*7.0;
        return;
    }
}
void PomFt::mrbencode91(bool *me,bool *codeword,bool g2_[91][174],int N,int K)
{
    for (int i = 0; i < N; ++i) codeword[i]=0;
    for (int i = 0; i < K; ++i)
    {
        if ( me[i] == 1 )
        {
            for (int j = 0; j < N; ++j) codeword[j]=(codeword[j] ^ g2_[i][j]);
        }
    }
}
void PomFt::nextpat_step1_91(bool *mi,int k,int iorder,int &iflag)
{
    //iorder = 3;
    if (iflag <= 0)
    {
        iflag=-1;
        return;
    }
    int beg = iflag - 1;
    int end = iflag + iorder;
    if (end > k) end = k; //qDebug()<<"beg end="<<beg<<end;
    for (int i = beg; i < end; ++i)//  add only order and move step=1
    {
        if (i >= beg && i < beg + iorder) mi[i]=true;
        else mi[i]=false;
    }
    iflag--;
    /*bool *ms = new bool[128];//[91+5];
    int ind=-1;
    for (int i = 0; i < k-1; ++i)
    {
        if ( mi[i]==0 && mi[i+1]==1) ind=i;
    }   
    if ( ind < 0 )
    {
        iflag=ind;
        return;
    }
    for (int i = 0; i < 128; ++i) ms[i]=0;
    for (int i = 0; i < ind; ++i) ms[i]=mi[i];
    ms[ind]=1;
    ms[ind+1]=0;  
    if ( ind+0 < k )
    {
        int sum_ms =0;
        for (int i = 0; i < ind; ++i) sum_ms+=ms[i];
        int nz=iorder-sum_ms;
        for (int i = 0; i < k; ++i) ms[k-nz+i+0]=1;
    }
    for (int i = 0; i < 91; ++i) mi[i]=ms[i];
    for (int i = 0; i < k; ++i)
    {
        if (mi[i]==1)
        {
            iflag=i;
            break;
        }
    }
    delete ms;*/
}
bool PomFt::any_ca_iand_ca_eq1_91(bool *a,bool *b,int count)
{
    bool res = false;
    for (int i = 0; i < count; ++i)
    {
        if ((a[i] & b[i])==1)
        {
            res = true;
            break;
        }
    }
    return res;
}
void PomFt::boxit91(bool &reset,bool *e2,int ntau,int npindex,int i1,int i2)
{
    /*integer*1 e2(1:ntau)
    v2 integer   indexes(5000,2),fp(0:525000),np(5000) v1 integer   indexes(4000,2),fp(0:525000),np(4000)
    logical reset
    common/boxes/indexes,fp,np*/
    if (reset)
    {
        //patterns=-1
        //sc=-1
        for (int i = 0; i < 525000; ++i)
            fp_ft8_2[i]=-1;
        for (int i = 0; i < 5000; ++i)
        {
            np_ft8_2[i]=-1;
            indexes_ft8_2_[0][i]=-1;
            indexes_ft8_2_[1][i]=-1;
        }
        reset=false;
    }

    indexes_ft8_2_[0][npindex]=i1;
    indexes_ft8_2_[1][npindex]=i2;
    //Left Shift 	ISHFT 	ISHFT(N,M) (M > 0) 	<< 	n<<m 	n shifted left by m bits
    //Right Shift 	ISHFT 	ISHFT(N,M) (M < 0) 	>> 	n>>m 	n shifted right by m bits
    int ipat=0;
    for (int i = 0; i < ntau; ++i)//ntau=19,14
    {//do i=1,ntau
        if (e2[i]==1)
            ipat+=(1 << ((ntau-1)-i));  //??? hv->(ntau-1) ipat=ipat+ishft(1,ntau-i)
    }
    //qDebug()<<"ipatipat="<<ipat;

    int ip=fp_ft8_2[ipat];   //! see what's currently stored in fp(ipat)
    if (ip==-1)
        fp_ft8_2[ipat]=npindex;
    else
    {
        while (np_ft8_2[ip]!=-1)
        {
            ip=np_ft8_2[ip];
        }
        np_ft8_2[ip]=npindex;
    }
}
void PomFt::fetchit91(bool &reset,bool *e2,int ntau,int &i1,int &i2)
{
    /*v2 integer   indexes(5000,2),fp(0:525000),np(5000)  v1 integer   indexes(4000,2),fp(0:525000),np(4000)
    integer   lastpat
    integer*1 e2(ntau)
    logical reset
    common/boxes/indexes,fp,np
    save lastpat,inext*/
    if (reset)
    {
        lastpat_ft8_2=-1;
        reset=false;
    }
    int ipat=0;
    for (int i = 0; i < ntau; ++i)
    {//do i=1,ntau
        if (e2[i]==1)
            ipat+=(1 << ((ntau-1)-i));//ipat=ipat+ishft(1,ntau-i)
    }
    int index=fp_ft8_2[ipat];
    //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    if (lastpat_ft8_2!=ipat && index>=0) //hv>=0 if(lastpat.ne.ipat .and. index>0) then  then ! return first set of indices
    {
        i1=indexes_ft8_2_[0][index];//i1=indexes(index,1)
        i2=indexes_ft8_2_[1][index];//i2=indexes(index,2)
        inext_ft8_2=np_ft8_2[index];  //inext=np(index)
    }
    else if (lastpat_ft8_2==ipat && inext_ft8_2>=0)//hv>=0 elseif(lastpat.eq.ipat .and. inext>0) then
    {
        i1=indexes_ft8_2_[0][inext_ft8_2];//i1=indexes(inext,1)
        i2=indexes_ft8_2_[1][inext_ft8_2];//i2=indexes(inext,2)
        inext_ft8_2=np_ft8_2[inext_ft8_2];  //inext=np(inext)
    }
    else
    {
        i1=-1;
        i2=-1;
        inext_ft8_2=-1;
    }
    lastpat_ft8_2=ipat;
}
void PomFt::bshift1(bool *a,int cou_a,int ish)
{
    //HV for single shift vareable
    //Left Shift 	ISHFT 	ISHFT(N,M) (M > 0) 	<< 	n<<m 	n shifted left by m bits
    //Right Shift 	ISHFT 	ISHFT(N,M) (M < 0) 	>> 	n>>m 	n shifted right by m bits
    //double complex t[cou_a];  //garmi hv v1.42
    //double complex t[cou_a*2+ish+50];  //garmi hv v1.43 ok
    bool *t = new bool[cou_a+100]; //garmi pri goliam count hv v1.43 correct ok
    for (int i=0; i< cou_a; i++) t[i]=a[i];
    if (ish>0)
    {
        for (int i = 0; i <  cou_a; i++)
        {
            if (i+ish<cou_a) a[i]=t[i+ish];
            else a[i]=t[i+ish-cou_a];
        }
    }
    if (ish<0)
    {
        for (int i = 0; i <  cou_a; i++)
        {
            if (i+ish<0) a[i]=t[i+ish+cou_a];
            else a[i]=t[i+ish];
        }
    }
    delete [] t;
}
void PomFt::get_crc14(bool *mc,int len,int &ncrc)
{
    //! 1. To calculate 14-bit CRC, mc(1:len-14) is the message and mc(len-13:len) are zero.
    //! 2. To check a received CRC, mc(1:len is the received message plus CRC.
    //!    ncrc will be zero if the received message/CRC are consistent
    bool r[15+5];
    const bool p[15]=
        {
            1,1,0,0,1,1,1,0,1,0,1,0,1,1,1
        }
        ;//=26455
    //bool p[15]={1,1,0,0,1,1,1,0,1,0,1,0,1,1,1};

    //! divide by polynomial
    for (int i = 0; i<15; ++i)
        r[i]=mc[i]; //r=mc(1:15)

    for (int i = 0; i<len-14; ++i)	//(len-15)=81   (len-14)=82
    {//do i=0,len-15
        r[14]=mc[i+14];//r(15)=mc(i+15)
        bool r0 = r[0];  //qDebug()<<i+14;
        for (int j = 0; j<15; ++j)
            r[j]=fmod(r[j]+r0*p[j],2);//r=mod(r+r(1)*p,2)
        //for (int j = 0; j<15; ++j)
        bshift1(r,15,1);//r=cshift(r,1)
    }
    /*write(c14,'(14b1)') r(1:14)
    read(c14,'(b14.14)') ncrc*/
    ncrc = 0;
    for (int i = 0; i < 14; ++i)
    {
        ncrc <<= 1;
        ncrc |= r[i];
    }
    //qDebug()<<ncrc;
}
void PomFt::bpdecode174_91var(double *llr,bool *apmask,bool *message77,bool *cw,int &nharderror)
{
    const int N=174;
    const int K=91;
    const int M=(N-K);//83
    double tov_[N][3];//(3,N)
    double toc_[M][7];//(7,M)
    double tanhtoc_[M][7];//(7,M)
    double zn[N+5];
    const int ncw=3;
    bool decoded[96+5];
    int synd[M+5];
    for (int i = 0; i<N; ++i)
    {
        for (int j = 0; j<3; ++j) tov_[i][j]=0.0;//tov=0
    }
    //! initialize messages to checks
    for (int j = 0; j<M; ++j)
    {//do j=1,M
        for (int i = 0; i<(nrw_ft8_174_91[j]); ++i)
        {//do i=1,nrw(j)
            toc_[j][i]=llr[Nm_ft8_174_91_[j][i]-1];//toc(i,j)=llr((Nm(i,j)))
        }
    }
    int ncnt=0;
    int nclast=0;
    for (int iter = 0; iter<=30; ++iter)
    {
        for (int i = 0; i< N; ++i)
        {//do i=1,N
            if ( apmask[i] != 1 )
            {
                double sumt = 0.0;
                for (int x = 0; x < ncw; ++x) sumt+=tov_[i][x];//zn[i]=llr[i]+sum(tov(1:ncw,i))
                zn[i]=llr[i]+sumt;
            }
            else zn[i]=llr[i];
            //! Check to see if we have a codeword (check before we do any iteration).
            /*cw=0
            where( zn .gt. 0. ) cw=1*/
            if (zn[i]>0.0) cw[i]=1;//cw=0 //where( zn .gt. 0. ) cw=1
            else cw[i]=0;
        }
        int ncheck=0;
        for (int i = 0; i < M; ++i)
        {//do i=1,M
            // synd(i)=sum(cw(Nm(1:nrw(i),i)))
            int sum = 0;
            for (int x = 0; x < nrw_ft8_174_91[i]; ++x) sum += (int)cw[Nm_ft8_174_91_[i][x]-1];//hv-1
            synd[i]= sum;
            if ( fmod(synd[i],2) != 0 ) ncheck++;
            //!   if( mod(synd(i),2) .ne. 0 ) write(*,*) 'check ',i,' unsatisfied'
        }
        if ( ncheck == 0 ) //then ! we have a codeword - if crc is good, return it
        {
            int nbadcrc;
            for (int i = 0; i < K; ++i)
            {
                decoded[i]=cw[i];//decoded=cw(1:K)
            }
            chkcrc14a(decoded,nbadcrc);
            int count = 0;
            for (int i = 0; i < N; ++i)
            {
                if ((double)(2*cw[i]-1)*llr[i] < 0.0 ) count++;
            }
            nharderror=count;
            if (nbadcrc==0) //then
            {
                for (int i = 0; i < 91; ++i) message77[i]=decoded[i];// message91=cw(1:91)
                return;
            }
        }
        if ( iter > 0 )  //! this code block implements an early stopping criterion
        {   //if( iter.gt.10000 )
            int nd=ncheck-nclast;
            if ( nd < 0 ) ncnt=0;//! # of unsatisfied parity checks decreased //! reset counter
            else ncnt++;//ncnt=ncnt+1;
            //!    write(*,*) iter,ncheck,nd,ncnt
            if ( ncnt >= 5 && iter >= 10 && ncheck > 15)//if( ncnt .ge. 5 .and. iter .ge. 10 .and. ncheck .gt. 15) then
            {
                nharderror=-1;//if (iter<5) qDebug()<<"iter FFFFF="<<iter;
                return;//?? break;//exit return;
            }
        }
        nclast=ncheck;
        //! Send messages from bits to check nodes
        for (int j = 0; j < M; ++j)
        {//do j=1,M
            for (int i = 0; i < nrw_ft8_174_91[j]; ++i)
            {//do i=1,nrw
                int ibj=Nm_ft8_174_91_[j][i]-1;//ibj=Nm(i,j)
                toc_[j][i]=zn[ibj];//toc(i,j)=zn(ibj)
                for (int kk = 0; kk < ncw; kk++)
                {//do kk=1,ncw //! subtract off what the bit had received from the check
                    if ( Mn_ft8_174_91_[ibj][kk]-1 == j )//hv-1 if( Mn(kk,ibj) .eq. j ) then   //! Mn(3,128)
                        toc_[j][i]=toc_[j][i]-tov_[ibj][kk]; //hv-1 toc(i,j)=toc(i,j)-tov(kk,ibj)
                }//tanhtoc_[j][i]=tanh(-toc_[j][i]/2.0);//tanhtoc(1:nrw,i)=tanh(-toc(1:nrw,i)/2)
            }
        }
        //! send messages from check nodes to variable nodes
        for (int i = 0; i < M; ++i)
        {//do i=1,M
            for (int x = 0; x < 7; ++x) tanhtoc_[i][x]=tanh(-toc_[i][x]/2.0);////tanhtoc(1:nrw,i)=tanh(-toc(1:nrw,i)/2)
        }
        for (int j = 0; j < N; ++j)
        {//do j=1,N
            for (int i = 0; i < ncw; ++i)
            {//do i=1,ncw
                int ichk=Mn_ft8_174_91_[j][i]-1; //ichk=Mn(i,j)  //! Mn(:,j) are the checks that include bit j
                //Tmn=product(tanhtoc(1:nrw(ichk),ichk),mask=Nm(1:nrw(ichk),ichk).ne.j)
                double Tmn = 1.0;//mulilay
                for (int z = 0; z < nrw_ft8_174_91[ichk]; ++z)
                {
                    if (Nm_ft8_174_91_[ichk][z]-1 != j) Tmn = Tmn*tanhtoc_[ichk][z];
                }
                double y;
                platanh(-Tmn,y);//call platanh(-Tmn,y)
                //!      y=atanh(-Tmn)
                tov_[j][i]=2.0*y;
            }
        }
    }   //! bp iterations
    nharderror=-1;
}
void PomFt::decode174_91(double *llr,int maxosd,int norder,bool *apmask,bool *message91,bool *cw,int &nharderror,double &dmin)
{	//int Keff=91,
    const int N=174;
    const int K=91;
    const int M=(N-K);//83
    double zsave_[4][N];//zsave(N,3)
    double tov_[N][3];//(3,N)
    double toc_[M][7];//(7,M)
    double tanhtoc_[M][7];//(7,M)
    double zsum[N+5];
    double zn[N+5];
    const int ncw=3;
    bool m96[96+5];
    bool hdec[N+2];
    bool nxor[N+2];
    int synd[M+5];
    //bool decoded[K+20];//91 need 96 for check14a integer*1 decoded(K)

    //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    //int maxiterations=30;
    //int max_iterations=40;//for FT4

    /*for (int i = 0; i<3; ++i)
    {
    	for (int j = 0; j<N; ++j)
    		zsave_[i][j] = 0.0;
    }*/

    //maxosd +=1;
    int nosd=0;
    if (maxosd>3) maxosd=3;
    if (maxosd==0) //then //! osd with channel llrs
    {
        nosd=1;
        for (int i = 0; i < N; ++i)
            zsave_[0][i]=llr[i];
    }
    else if (maxosd>0)// then !
        nosd=maxosd;
    else if (maxosd<0) //then ! just bp
        nosd=0;

    for (int i = 0; i<N; ++i)
    {
        zsum[i]=0.0;
        for (int j = 0; j<3; ++j)
            tov_[i][j]=0.0;//tov=0
    }

    for (int j = 0; j<M; ++j)
    {//do j=1,M
        for (int i = 0; i<(nrw_ft8_174_91[j]); ++i)
        {//do i=1,nrw(j)
            toc_[j][i]=llr[Nm_ft8_174_91_[j][i]-1];//toc(i,j)=llr((Nm(i,j)))
        }
    }

    int ncnt=0;
    int nclast=0;
    for (int iter = 0; iter<=30; ++iter)
    {
        //! Update bit log likelihood ratios (tov=0 in iteration 0).
        //for (int i = 0; i<N; ++i) zsum[i]=0.0;
        for (int i = 0; i< N; ++i)
        {//do i=1,N
            if ( apmask[i] != 1 )
            {
                double sumt = 0.0;
                for (int x = 0; x < ncw; ++x)
                    sumt+=tov_[i][x];//zn[i]=llr[i]+sum(tov(1:ncw,i))
                zn[i]=llr[i]+sumt;
            }
            else
                zn[i]=llr[i];

            if (zn[i]>0.0)//cw=0 //where( zn .gt. 0. ) cw=1
                cw[i]=1;
            else
                cw[i]=0;

            //if (iter<=maxosd)
            if (iter<maxosd)
                zsum[i]+=zn[i];  //zsum=zsum+zn
        }//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.

        // diff -> if(iter.gt.0 .and. iter.le.maxosd) then
        //if (iter>0 && iter<=maxosd)
        if (iter<maxosd)
        {
            for (int i = 0; i< N; ++i) zsave_[iter][i]=zsum[i]; //zsave_[iter-1][i]=zsum[i];// llr[i];
            //qDebug()<<iter;
        }

        int ncheck=0;
        for (int i = 0; i < M; ++i)
        {//do i=1,M
            // synd(i)=sum(cw(Nm(1:nrw(i),i)))
            int sum = 0;
            for (int x = 0; x < nrw_ft8_174_91[i]; ++x)
                sum += (int)cw[Nm_ft8_174_91_[i][x]-1];//hv-1
            synd[i]= sum;
            //if( mod(synd(i),2) .ne. 0 ) ncheck=ncheck+1
            if ( fmod(synd[i],2) != 0 ) ncheck++;
            //!   if( mod(synd(i),2) .ne. 0 ) write(*,*) 'check ',i,' unsatisfied'
        }

        if ( ncheck == 0 ) //then ! we have a codeword - if crc is good, return it
        {
            int nbadcrc;
            /*for (int i = 0; i < K; ++i)
            {
                decoded[i]=cw[i];//decoded=cw(1:K)
            } 
            chkcrc14a(decoded,nbadcrc);*/

            for (int i = 0; i < 96; ++i)
            {
                m96[i]=0;
                if (i<77) m96[i]=cw[i]; //m96(1:77)=cw(1:77)
                if (i>81) m96[i]=cw[i-5];//m96(83:96)=cw(78:91) {m96[i]=cw[i-5]; qDebug()<<i<<i-5; }
            }
            //int nbadcrc1;
            get_crc14(m96,96,nbadcrc);

            //nharderror=count( (2*cw-1)*llr .lt. 0.0 )
            if (nbadcrc==0) //then
            {
                int count = 0;
                for (int i = 0; i < N; ++i)
                {
                    //if (llr[i]*(double)(cw[i]*2-1)<0.0)
                    /*double xx = cw[i];
                    if ((2.0*xx-1.0)*llr[i] < 0.0 )
                        count++;*/
                    if ((double)(2*cw[i]-1)*llr[i] < 0.0 )
                        count++;
                }
                nharderror=count;

                for (int i = 0; i < 91; ++i)
                    message91[i]=cw[i];// message91=cw(1:91)
                for (int i = 0; i < N; ++i) //where(llr .ge. 0) hdec=1 //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
                {
                    /*if (llr[i]>=0.0)
                        hdec[i]=1;
                    else
                        hdec[i]=0;*/
                    hdec[i]=0;
                    if (llr[i]>=0.0) hdec[i]=1;
                    nxor[i]=hdec[i] ^ cw[i]; //nxor=ieor(hdec,cw)
                    dmin+=(double)nxor[i]*fabs(llr[i]);	//dmin=sum(nxor*abs(llr))
                }
                //for (int i = 0; i < N; ++i)
                //dmin+=nxor[i]*fabs(llr[i]);//dmin=sum(nxor*abs(llr))
                //ntype=1
                return;
            }
        }

        if ( iter > 0 )  //! this code block implements an early stopping criterion
        {   //if( iter.gt.10000 )
            int nd=ncheck-nclast;
            if ( nd < 0 ) //! # of unsatisfied parity checks decreased
                ncnt=0;  //! reset counter
            else
                ncnt++;  //ncnt=ncnt+1;

            //!    write(*,*) iter,ncheck,nd,ncnt
            if ( ncnt >= 5 && iter >= 10 && ncheck > 15)//if( ncnt .ge. 5 .and. iter .ge. 10 .and. ncheck .gt. 15) then
            {
                nharderror=-1;
                //if (iter<5) qDebug()<<"iter FFFFF="<<iter;
                break;//exit return;
            }
        }
        nclast=ncheck;

        //! Send messages from bits to check nodes
        for (int j = 0; j < M; ++j)
        {//do j=1,M
            for (int i = 0; i < nrw_ft8_174_91[j]; ++i)
            {//do i=1,nrw
                int ibj=Nm_ft8_174_91_[j][i]-1;//ibj=Nm(i,j)
                toc_[j][i]=zn[ibj];//toc(i,j)=zn(ibj)
                for (int kk = 0; kk < ncw; kk++)
                {//do kk=1,ncw //! subtract off what the bit had received from the check
                    if ( Mn_ft8_174_91_[ibj][kk]-1 == j )//hv-1 if( Mn(kk,ibj) .eq. j ) then   //! Mn(3,128)
                        toc_[j][i]=toc_[j][i]-tov_[ibj][kk]; //hv-1 toc(i,j)=toc(i,j)-tov(kk,ibj)
                }
                //tanhtoc_[j][i]=tanh(-toc_[j][i]/2.0);//tanhtoc(1:nrw,i)=tanh(-toc(1:nrw,i)/2)
            }
        }

        //! send messages from check nodes to variable nodes
        for (int i = 0; i < M; ++i)
        {//do i=1,M
            for (int x = 0; x < 7; ++x)////tanhtoc(1:nrw,i)=tanh(-toc(1:nrw,i)/2)
                tanhtoc_[i][x]=tanh(-toc_[i][x]/2.0);
        }

        for (int j = 0; j < N; ++j)
        {//do j=1,N
            for (int i = 0; i < ncw; ++i)
            {//do i=1,ncw
                int ichk=Mn_ft8_174_91_[j][i]-1; //ichk=Mn(i,j)  //! Mn(:,j) are the checks that include bit j
                //Tmn=product(tanhtoc(1:nrw(ichk),ichk),mask=Nm(1:nrw(ichk),ichk).ne.j)
                double Tmn = 1.0;//mulilay
                for (int z = 0; z < nrw_ft8_174_91[ichk]; ++z)
                {
                    if (Nm_ft8_174_91_[ichk][z]-1 != j)
                        Tmn = Tmn*tanhtoc_[ichk][z];
                }
                double y;
                platanh(-Tmn,y);//call platanh(-Tmn,y)
                //!      y=atanh(-Tmn)
                tov_[j][i]=2.0*y;
            }
        }
    }   //! bp iterations

    for (int io = 0; io < nosd; ++io)//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    {//do i=1,nosd
        for (int j = 0; j < N; ++j)
            zn[j]=zsave_[io][j];//zn=zsave(:,i)
        double dminosd = 0.0;
        osd174_91_1(zn,apmask,norder,message91,cw,nharderror,dminosd);//Keff=91
        if (nharderror>0)//then
        {
            for (int i = 0; i < N; ++i) //where(llr .ge. 0) hdec=1 //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
            {
                /*if (llr[i]>=0.0)
                    hdec[i]=1;
                else
                    hdec[i]=0;*/
                hdec[i]=0;
                if (llr[i]>=0.0) hdec[i]=1;
                nxor[i]=hdec[i] ^ cw[i]; //nxor=ieor(hdec,cw)
                dmin+=(double)nxor[i]*fabs(llr[i]);	//dmin=sum(nxor*abs(llr))
            }
            //qDebug()<<nharderror<<cw[0]<<cw[1]<<cw[2]<<cw[3];
            //ntype=2;
            return;
        }
    }
    //ntype=0
    nharderror=-1;
    //dminosd=0.0;
}
void PomFt::encode174_91_nocrc(bool *message910,bool *codeword)
{
    const int N=174;
    const int K=91;
    const int M=N-K;//=83
    bool pchecks[95];

    if (first_enc174_91_nocrc)
    {
        for (int i = 0; i < 95; ++i)//91 83   gen_osd174_91_nocrc[100][95];
        {
            for (int j = 0; j < 85; ++j)
                gen_osd174_91_nocrc[i][j]=0;
        }
        for (int i = 0; i < M; ++i)//M=83
        {
            for (int j = 0; j < 23; ++j)
            {
                bool ok;
                QString temp = g_ft8_174_91[i].mid(j,1);
                int istr = temp.toInt(&ok, 16);//read(g(i)(j:j),"(Z1)") istr
                for (int jj = 0; jj < 4; ++jj)
                {
                    int irow=(j)*4+jj;//irow=(j-1)*4+jj
                    if ( irow <= 90 ) gen_osd174_91_nocrc[irow][i]=(1 & (istr >> (3-jj)));//if( btest(istr,4-jj) ) gen(irow,i)=1
                }
            }
        }
        first_enc174_91_nocrc=false;
    }

    for (int i = 0; i < 83; ++i)
    {
        int nsum=0;
        for (int j = 0; j < 91; ++j)
        {
            nsum+=message910[j]*gen_osd174_91_nocrc[j][i];//nsum=nsum+message(j)*gen(i,j);
        }
        pchecks[i]=fmod(nsum,2);
    }
    // codeword(1:K)=message
    // codeword(K+1:N)=pchecks
    for (int i = 0; i < K; ++i) codeword[i]=message910[i];//91
    for (int i = 0; i < M; ++i) codeword[i+91]=pchecks[i];//174-91=83
}
void PomFt::osd174_91_1(double *llr,/*int Keff=91*/bool *apmask,int ndeep,bool *message91,bool *cw,int &nhardmin,double &dmin,bool var)
{
    const int N=174;
    const int K=91;
    const int M=N-K;// M=83
    double rx[N];
    bool apmaskr[N];
    bool apmaskr2[N];
    bool hdec[N+2];
    bool hdec2[N+2];
    double absrx[N];
    double absrx2[N];
    bool genmrb_[N][K];//(K,N)
    bool g2_[K][N]; //(N,K)
    int indices[N];
    int indx[N];
    bool temp[K];
    bool m0[K];
    bool c0[N];
    bool nxor[N+2]; //2.71 old=int
    bool misub[K+8];
    bool me[K],mi[K];
    bool ce[N];
    bool e2sub[M];//e2sub(N-K)
    bool e2[M];//e2(N-K)
    bool ui[M];//ui(N-K)
    bool r2pat[M];
    bool cw_t[N];
    bool m96[96+5];
    //bool decoded[K];//91

    //qDebug()<<var;

    if (first_osd174_91) //then ! fill the generator matrix
    {
        for (int i = 0; i < N; ++i)
        {
            for (int j = 0; j < K; ++j)
                gen_osd174_91_[i][j]=0;
        }

        for (int i = 0; i < K; ++i) //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
        {//do i=1,k
            bool message910[120];
            for (int j = 0; j < 91; ++j)
                message910[j]=0;
            message910[i]=1;
            /*if (i<77)
            {
                //m96=0
                //m96(1:91)=message91
                //call get_crc14(m96,96,ncrc14)
                //write(c14,'(b14.14)') ncrc14
                //read(c14,'(14i1)') message91(78:91)
                for (int j = 77; j < K; ++j)
                    message91[j]=0;//message91(78:k)=0
            }*/
            encode174_91_nocrc(message910,cw);
            for (int j = 0; j < N; ++j)
                gen_osd174_91_[j][i]=cw[j];
        }
        first_osd174_91=false;
    }

    for (int i = 0; i < N; ++i)//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    {
        rx[i]=llr[i];//rx=llr
        apmaskr[i]=apmask[i];//apmaskr=apmask
        hdec[i]=0;//hdec=0
        if (rx[i]>=0.0) hdec[i]=1;              //  where(rx .ge. 0) hdec=1
        absrx[i]=fabs(rx[i]);//absrx=abs(rx)
    }

    //if (N>0)//2.12 no needed
    pomAll.indexx_msk(absrx,N-1,indx);//call indexx(absrx,N,indx)
    //qDebug()<<"in -1- -1-="<<nhardmin;
    //! Re-order the columns of the generator matrix in order of decreasing reliability.
    for (int i = 0; i < N; ++i)
    {//do i=1,N
        for (int j = 0; j < K; ++j)
            genmrb_[i][j]=gen_osd174_91_[indx[(N-1)-i]][j];//genmrb(1:K,i)=gen(1:K,indx(N+1-i))
        indices[i]=indx[(N-1)-i];//indices[i]=indx(N+1-i)
    }

    //! Do gaussian elimination to create a generator matrix with the most reliable
    //! received bits in positions 1:K in order of decreasing reliability (more or less).
    int iflag=0;
    for (int id = 0; id < K; ++id)
    {//do id=1,K //! diagonal element indices
        for (int icol = id; icol < K+20; ++icol)//+20 ???
        {//do icol=id,K+20  ! The 20 is ad hoc - beware
            iflag=0;
            if ( genmrb_[icol][id] == 1 ) //then (id,icol) //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
            {
                iflag=1;
                if ( icol != id ) //then ! reorder column;//if( icol .ne. id ) then ! reorder column
                {
                    for (int z = 0; z < K; ++z)
                    {
                        temp[z]=genmrb_[id][z];//temp(1:K)=genmrb(1:K,id)
                        genmrb_[id][z]=genmrb_[icol][z];//genmrb(1:K,id)=genmrb(1:K,icol)
                        genmrb_[icol][z]=temp[z];//genmrb(1:K,icol)=temp(1:K)
                    }
                    int itmp=indices[id];
                    indices[id]=indices[icol];
                    indices[icol]=itmp;
                }
                for (int ii = 0; ii < K; ++ii)
                {//do ii=1,K
                    if ( ii != id && genmrb_[id][ii] == 1 ) //then if( ii != id && genmrb(ii,id) .eq. 1 ) then
                    {
                        for (int z = 0; z < N; ++z)
                            genmrb_[z][ii]=(genmrb_[z][ii] ^ genmrb_[z][id]);//genmrb(ii,1:N)=ieor(genmrb(ii,1:N),genmrb(id,1:N))
                    }
                }
                break; //exit
            }
        }
    }

    for (int i = 0; i < N; ++i)
    {
        for (int j = 0; j < K; ++j)//char genmrb_[N][K];
            g2_[j][i]=genmrb_[i][j]; //g2=transpose(genmrb)
    }
    //! The hard decisions for the K MRB bits define the order 0 message, m0.
    //! Encode m0 using the modified generator matrix to find the "order 0" codeword.
    //! Flip various combinations of bits in m0 and re-encode to generate a list of
    //! codewords. Return the member of the list that has the smallest Euclidean
    //! distance to the received word.

    for (int i = 0; i < N; ++i)// N
    {
        hdec2[i]=hdec[indices[i]];       //hdec=hdec(indices)
        absrx2[i]=absrx[indices[i]];     //absrx=absrx(indices)
        //rx_t[i]=rx[indices[i]];           //rx=rx(indices)
        apmaskr2[i]=apmaskr[indices[i]]; //apmaskr=apmaskr(indices)
    }
    for (int i = 0; i < K; ++i)
        m0[i]=hdec2[i];      // m0=hdec(1:K)         ! zero'th order message  //! zero'th order message

    mrbencode91(m0,c0,g2_,N,K);//  mrbencode91(m0,c0,g2,N,K);

    nhardmin = 0;
    dmin = 0.0;
    for (int i = 0; i < N; ++i)
    {
        nxor[i]=(c0[i] ^ hdec2[i]);
        nhardmin+=nxor[i];//nhardmin=sum(nxor)
        dmin+=(double)nxor[i]*absrx2[i];
    }

    for (int i = 0; i < N; ++i)
        cw[i]=c0[i];

    int nt=0;
    int nrejected=0;

    int nord = 0;
    int npre1 = 0;
    int npre2 = 0;
    int ntheta = 0;
    int ntau = 0;
    int ntotal=0;

    if (ndeep==0) goto c998; //! norder=0
    if (ndeep>6) ndeep=6;
    if (ndeep==1)
    {
        nord=1;
        npre1=0;
        npre2=0;
        nt=40;
        ntheta=12;
    }
    else if (ndeep==2)
    {
        nord=1;
        npre1=1;
        npre2=0;
        nt=40;
        if (var) ntheta=12;//var
        else  ntheta=11;//2.71=11 old=12 2.2.0org=ntheta=10;   ntheta=12;
    }
    else if (ndeep==3)
    {
        nord=1;
        npre1=1;
        npre2=1;
        nt=40;
        ntheta=12;
        ntau=14;
    }
    else if (ndeep==4)
    {
        nord=2;
        npre1=1;
        nt=40;
        ntheta=12;
        if (var)//var
        {
            npre2=0;
            ntau=19;
        }
        else
        {
            npre2=1; //npre2=0;
            ntau=17;//ntau=19;
        }
    }
    else if (ndeep==5)
    {
        npre1=1;
        npre2=1;
        nt=40;
        ntheta=12;
        if (var)//var
        {
            nord=2;
            ntau=19;
        }
        else
        {
            nord=3;//nord=2
            ntau=15;//ntau=19;
        }
    }
    else //if (ndeep==6)
    {
        nord=4;
        npre1=1;
        npre2=1;
        nt=95;
        ntheta=12;
        ntau=15;
    }

    for (int iorder = 1; iorder <= nord; ++iorder)
    {//do iorder=1,nord

        for (int z = 0; z < K-iorder; ++z) //misub(1:K-iorder)=0
            misub[z]=0;
        for (int z = K-iorder; z < K; ++z)
            misub[z]=1;                    // misub(K-iorder+1:K)=1
        iflag=K-iorder;                    //iflag=K-iorder+1

        while (iflag >= 0 ) //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
        {//do while(iflag .ge.0)
            int iend = 0;
            if (iorder==nord && npre1==0)
                iend=iflag;
            else
                iend=0;//iend=1

            double d1 = 0.0;
            //double dd = 0.0;
            //int nd1Kpt = 0;
            for (int n1 = iflag; n1 >= iend; --n1)
            {//do n1=iflag,iend,-1
                for (int x = 0; x < K; ++x)
                    mi[x]=misub[x];//mi=misub
                mi[n1]=1;//mi(n1)=1;
                //if(any(iand(apmaskr(1:K),mi).eq.1)) continue;//cycle
                if (any_ca_iand_ca_eq1_91(apmaskr2,mi,K)) continue;//cycle
                ntotal++;
                for (int x = 0; x < K; ++x)
                    me[x]=(m0[x] ^ mi[x]);//me=ieor(m0,mi)
                int nd1Kpt = 0;
                //double d1 = 0.0; //hv testsed //error hv ->
                if (n1==iflag)
                {
                    mrbencode91(me,ce,g2_,N,K);
                    for (int x = 0; x < M; ++x)//K=91  M=87
                    {
                        e2sub[x]=(ce[K+x] ^ hdec2[K+x]);//e2sub=ieor(ce(K+1:N),hdec(K+1:N))
                        e2[x]=e2sub[x];
                    }
                    nd1Kpt = 0;
                    for (int x = 0; x < nt; ++x)//nt=40 K=87  M=87
                        nd1Kpt+=e2sub[x];//nd1Kpt=sum(e2sub(1:nt))+1;
                    nd1Kpt = nd1Kpt + 1;
                    d1 = 0.0;
                    for (int x = 0; x < K; ++x)
                        d1+=(double)(me[x] ^ hdec2[x])*absrx2[x];//d1=sum(ieor(me(1:K),hdec(1:K))*absrx(1:K))
                }
                else
                {
                    for (int x = 0; x < M; ++x)
                        e2[x]=(e2sub[x] ^ g2_[n1][K+x]);//e2=ieor(e2sub,g2(K+1:N,n1))
                    nd1Kpt = 0;
                    for (int x = 0; x < nt; ++x)
                        nd1Kpt+=e2[x];//nd1Kpt=sum(e2(1:nt))+2
                    nd1Kpt = nd1Kpt + 2;
                }
                if (nd1Kpt <= ntheta) //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
                {
                    mrbencode91(me,ce,g2_,N,K);
                    for (int x = 0; x < N; ++x)
                        nxor[x]=(ce[x] ^ hdec2[x]);//nxor=ieor(ce,hdec)
                    double dd = 0.0;
                    if (n1==iflag)
                    {
                        dd = 0.0;
                        for (int x = 0; x < M; ++x)
                            dd+=(double)e2sub[x]*absrx2[K+x];//dd=d1+sum(e2sub*absrx(K+1:N))
                        dd=d1 + dd;
                    }
                    else
                    {
                        //dd=d1+ieor(ce(n1),hdec(n1))*absrx(n1)+sum(e2*absrx(K+1:N))
                        dd = 0.0;
                        for (int x = 0; x < M; ++x)
                            dd+=(double)e2[x]*absrx2[K+x];
                        dd = d1+(double)(ce[n1] ^ hdec2[n1])*absrx2[n1] + dd;
                    }
                    if ( dd < dmin )
                    {
                        dmin=dd;
                        for (int x = 0; x < N; ++x)
                            cw[x]=ce[x];//cw=ce
                        nhardmin = 0;
                        for (int x = 0; x < N; ++x)
                            nhardmin+=nxor[x];//nhardmin=sum(nxor)
                        //nd1Kptbest=nd1Kpt;
                    }
                }
                else
                    nrejected++;
            }
            /*QString sss = "";///gen_osd174_[174][87];
            for (int z= 0; z < 87; z++)//decoded=87   cw-174 
            {
                sss.append(QString("%1").arg((int)misub[z]));
                sss.append(",");
            }
            qDebug()<<"222 mi="<<sss<<iorder;*/
            //! Get the next test error pattern, iflag will go negative
            //! when the last pattern with weight iorder has been generated.
            //nextpat(misub,K,iorder,iflag);
            nextpat_step1_91(misub,K,iorder,iflag);

            //qDebug()<<"iflag mi="<<iflag;
        }
    }
    //qDebug()<<"1OSD1 nhardmin================"<<nhardmin;
    if (npre2==1)
    {
        //qDebug()<<"npre2 mi=================";
        bool reset=true;
        ntotal=0;
        for (int i1 = K-1; i1 >= 0; --i1)
        {//do i1=K,1,-1
            for (int i2 = i1-1; i2 >= 0; --i2)//hv ??? i1-1
            {//do i2=i1-1,1,-1
                //ntotal=ntotal+1;
                for (int x = 0; x < ntau; ++x)//char g2_[87][174];//(N=174,K=87); ntau=19,14
                    mi[x]=(g2_[i1][K+x] ^ g2_[i2][K+x]);//mi=ieor(g2(K+1:K+ntau,i1),g2(K+1:K+ntau,i2))

                boxit91(reset,mi,ntau,ntotal,i1,i2);//call boxit(reset,mi(1:ntau),ntau,ntotal,i1,i2)
                ntotal++;
            }
        }

        //int ncount2=0;
        //int ntotal2=0;
        reset=true;
        //! Now run through again and do the second pre-processing rule

        for (int z = 0; z < K-nord; ++z)   //misub(1:K-iorder)=0
            misub[z]=0;
        for (int z = K-nord; z < K; ++z)
            misub[z]=1;                    // misub(K-iorder+1:K)=1
        iflag=K-nord;                      //iflag=K-iorder+1

        while (iflag>=0)//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
        {
            for (int z = 0; z < K; ++z)
                me[z]=(m0[z] ^ misub[z]);//me=ieor(m0,misub)
            mrbencode91(me,ce,g2_,N,K);
            for (int z = 0; z < M; ++z)
                e2sub[z]=(ce[K+z] ^ hdec2[K+z]);//e2sub=ieor(ce(K+1:N),hdec(K+1:N))
            for (int i2 = -1; i2 < ntau; ++i2)//hv ntau+1    do i2=0,ntau
            {//do i2=0,ntau
                //ntotal2++;// no use???
                for (int x = 0; x < M; ++x)
                    ui[x]=0;
                if (i2>-1) ui[i2]=1; ///hv i2>0 if(i2>0) ui(i2)=1
                for (int x = 0; x < M; ++x)
                    r2pat[x]=(e2sub[x] ^ ui[x]);//r2pat=ieor(e2sub,ui)

c778:     // continue;

                int in1=-1;//hv reset -1
                int in2=-1;//hv reset -1
                fetchit91(reset,r2pat,ntau,in1,in2);//fetchit(reset,r2pat(1:ntau),ntau,in1,in2)
                if (in1>=0 && in2>=0)//hv >=0 if(in1>0.and.in2>0) then
                {
                    //ncount2++;
                    for (int z = 0; z < K; ++z)
                        mi[z]=misub[z];
                    mi[in1]=1;
                    mi[in2]=1;
                    int sum_mi = 0;
                    for (int z = 0; z < K; ++z)
                        sum_mi+=mi[z];
                    //if(sum(mi).lt.nord+npre1+npre2.or.any(iand(apmaskr(1:K),mi).eq.1)) cycle
                    if (sum_mi<nord+npre1+npre2 || any_ca_iand_ca_eq1_91(apmaskr2,mi,K)) continue;//cycle
                    for (int z = 0; z < K; ++z)
                        me[z]=(m0[z] ^ mi[z]);
                    mrbencode91(me,ce,g2_,N,K);
                    for (int z = 0; z < N; ++z)
                        nxor[z]=(ce[z] ^ hdec2[z]);
                    double dd = 0.0;
                    for (int z = 0; z < N; ++z)
                        dd+=(double)nxor[z]*absrx2[z];//dd=sum(nxor*absrx)
                    if ( dd < dmin )
                    {
                        dmin=dd;
                        for (int z = 0; z < N; ++z)
                            cw[z]=ce[z];
                        nhardmin = 0;
                        for (int x = 0; x < N; ++x)
                            nhardmin+=nxor[x];//nhardmin=sum(nxor)
                    }
                    goto c778;
                }
            }
            //nextpat(misub,K,nord,iflag);
            nextpat_step1_91(misub,K,nord,iflag);
        }
    }
    //qDebug()<<"2OSD1 nhardmin================"<<nhardmin;
c998:

    //! Re-order the codeword to place message bits at the end.
    for (int i = 0; i < N; ++i)
        cw_t[indices[i]]=cw[i];        //cw(indices)=cw
    for (int i = 0; i < N; ++i)
    {
        cw[i]=cw_t[i];                 //cw(indices)=cw
        if (i<91)
            message91[i]=cw_t[i];          //message77=decoded(1:77)
        if (i<96)
        {
            m96[i]=0;
            if (i<77) m96[i]=cw[i]; //m96(1:77)=cw(1:77)
            if (i>81) m96[i]=cw[i-5];//m96(83:96)=cw(78:91)
        }
    }

    int nbadcrc;
    get_crc14(m96,96,nbadcrc); //qDebug()<<nbadcrc;

    if (nbadcrc!=0) nhardmin=-nhardmin;
    //if(nbadcrc==1) nhardmin=-nhardmin;
    //qDebug()<<"OSD2 nhardmin================"<<nhardmin<<dmin;
}
/*void PomFt::decode174_91_ft2(double *llr,int maxosd,int norder,bool *apmask,bool *message91,bool *cw,int &nharderror,double &dmin)
{	//int Keff=91,
    const int N=174;
    const int K=91;
    const int M=(N-K);//83
    double zsave_[5][N];//zsave(N,3)
    double tov_[N][3];//(3,N)
    double toc_[M][7];//(7,M) //double tanhtoc_[M][7];//(7,M)
    double zsum[N+5];
    double zn[N+5];
    const int ncw=3;
    bool m96[96+5];
    bool hdec[N+2];
    bool nxor[N+2];
    int synd[M+5];
    const double alpha_ms = 0.75;  //! Min-Sum normalization factor
    double sign_prod, min_abs;
    //bool decoded[K+20];//91 need 96 for check14a integer*1 decoded(K)
    //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    //int maxiterations=30;
    //int max_iterations=40;//for FT4
    //maxosd +=1;
    int nosd=0;
    if (maxosd>3) maxosd=3;
    if (maxosd==0) //then //! osd with channel llrs
    {
        nosd=1;
        for (int i = 0; i < N; ++i) zsave_[0][i]=llr[i];
    }
    else if (maxosd>0) nosd=maxosd;
    else if (maxosd<0) nosd=0;

    for (int i = 0; i<N; ++i)
    {
        zsum[i]=0.0;
        for (int j = 0; j<3; ++j) tov_[i][j]=0.0;//tov=0
    }
    for (int j = 0; j<M; ++j)
    {//do j=1,M
        for (int i = 0; i<(nrw_ft8_174_91[j]); ++i)
        {//do i=1,nrw(j)
            toc_[j][i]=llr[Nm_ft8_174_91_[j][i]-1];//toc(i,j)=llr((Nm(i,j)))
        }
    }
    int ncnt=0;
    int nclast=0;
    for (int iter = 0; iter<=30; ++iter)
    {
        //! Update bit log likelihood ratios (tov=0 in iteration 0).
        //for (int i = 0; i<N; ++i) zsum[i]=0.0;
        for (int i = 0; i< N; ++i)
        {//do i=1,N
            if ( apmask[i] != 1 )
            {
                double sumt = 0.0;
                for (int x = 0; x < ncw; ++x) sumt+=tov_[i][x];//zn[i]=llr[i]+sum(tov(1:ncw,i))
                zn[i]=llr[i]+sumt;
            }
            else zn[i]=llr[i];
            if (zn[i]>0.0) cw[i]=1;//cw=0 //where( zn .gt. 0. ) cw=1
            else cw[i]=0;
            if (iter<maxosd) zsum[i]+=zn[i];  //zsum=zsum+zn
        }//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
        // diff -> if(iter.gt.0 .and. iter.le.maxosd) then
        //if (iter>0 && iter<=maxosd)
        if (iter<maxosd)
        {
            for (int i = 0; i< N; ++i) zsave_[iter][i]=zsum[i]; //zsave_[iter-1][i]=zsum[i];// llr[i];
        }
        int ncheck=0;
        for (int i = 0; i < M; ++i)
        {//do i=1,M
            int sum = 0;// synd(i)=sum(cw(Nm(1:nrw(i),i)))
            for (int x = 0; x < nrw_ft8_174_91[i]; ++x) sum += (int)cw[Nm_ft8_174_91_[i][x]-1];//hv-1
            synd[i]= sum;
            if ( fmod(synd[i],2) != 0 ) ncheck++;//if( mod(synd(i),2) .ne. 0 ) ncheck=ncheck+1
            //!   if( mod(synd(i),2) .ne. 0 ) write(*,*) 'check ',i,' unsatisfied'
        }
        if ( ncheck == 0 ) //then ! we have a codeword - if crc is good, return it
        {
            int nbadcrc;
            for (int i = 0; i < 96; ++i)
            {
                m96[i]=0;
                if (i<77) m96[i]=cw[i]; //m96(1:77)=cw(1:77)
                if (i>81) m96[i]=cw[i-5];//m96(83:96)=cw(78:91) {m96[i]=cw[i-5]; qDebug()<<i<<i-5; }
            }
            get_crc14(m96,96,nbadcrc);//nharderror=count( (2*cw-1)*llr .lt. 0.0 )
            if (nbadcrc==0) //then
            {
                int count = 0;
                for (int i = 0; i < N; ++i)
                {
                    if ((double)(2*cw[i]-1)*llr[i] < 0.0 ) count++;
                }
                nharderror=count;
                for (int i = 0; i < 91; ++i) message91[i]=cw[i];// message91=cw(1:91)
                for (int i = 0; i < N; ++i) //where(llr .ge. 0) hdec=1 //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
                {
                    hdec[i]=0;
                    if (llr[i]>=0.0) hdec[i]=1;
                    nxor[i]=hdec[i] ^ cw[i]; //nxor=ieor(hdec,cw)
                    dmin+=(double)nxor[i]*fabs(llr[i]);	//dmin=sum(nxor*abs(llr))
                }
                //for (int i = 0; i < N; ++i)
                //dmin+=nxor[i]*fabs(llr[i]);//dmin=sum(nxor*abs(llr))
                //ntype=1
                return;
            }
        }
        if ( iter > 0 )  //! this code block implements an early stopping criterion
        {   //if( iter.gt.10000 )
            int nd=ncheck-nclast;
            if ( nd < 0 ) ncnt=0; //! # of unsatisfied parity checks decreased //! reset counter
            else ncnt++;  //ncnt=ncnt+1;
            if ( ncnt >= 5 && iter >= 10 && ncheck > 15)//if( ncnt .ge. 5 .and. iter .ge. 10 .and. ncheck .gt. 15) then
            {
                nharderror=-1;
                break;//exit return;
            }
        }
        nclast=ncheck;//! Send messages from bits to check nodes
        for (int j = 0; j < M; ++j)
        {//do j=1,M
            for (int i = 0; i < nrw_ft8_174_91[j]; ++i)
            {//do i=1,nrw
                int ibj=Nm_ft8_174_91_[j][i]-1;//ibj=Nm(i,j)
                toc_[j][i]=zn[ibj];//toc(i,j)=zn(ibj)
                for (int kk = 0; kk < ncw; kk++)
                {//do kk=1,ncw //! subtract off what the bit had received from the check
                    if ( Mn_ft8_174_91_[ibj][kk]-1 == j ) toc_[j][i]=toc_[j][i]-tov_[ibj][kk]; //hv-1 toc(i,j)=toc(i,j)-tov(kk,ibj)//hv-1 if( Mn(kk,ibj) .eq. j ) then   //! Mn(3,128)
                } //tanhtoc_[j][i]=tanh(-toc_[j][i]/2.0);//tanhtoc(1:nrw,i)=tanh(-toc(1:nrw,i)/2)
            }
        }//! send messages from check nodes to variable nodes (Normalized Min-Sum)
        for (int j = 0; j < N; ++j)
        {//do j=1,N
            for (int i = 0; i < ncw; ++i)
            {
                int ichk=Mn_ft8_174_91_[j][i]-1; //ichk=Mn(i,j)  //! Mn(:,j) are the checks that include bit j
                sign_prod=1.0;
                min_abs=1.0e30;
                for (int z = 0; z < nrw_ft8_174_91[ichk]; ++z)
                {
                    if (Nm_ft8_174_91_[ichk][z]-1 != j)//if(Nm(kk,ichk).ne.j) then
                    {
                        sign_prod=sign_prod*copysign(1.0,toc_[ichk][z]);//(kk,ichk)
                        if (fabs(toc_[ichk][z])<min_abs) min_abs=fabs(toc_[ichk][z]);
                    }
                }
                tov_[j][i]=alpha_ms*sign_prod*min_abs;//tov(i,j)=alpha_ms*sign_prod*min_abs
            }
        }
    }   //! bp iterations
    for (int io = 0; io < nosd; ++io)//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    {//do i=1,nosd
        for (int j = 0; j < N; ++j) zn[j]=zsave_[io][j];//zn=zsave(:,i)
        double dminosd = 0.0;
        osd174_91_1(zn,apmask,norder,message91,cw,nharderror,dminosd);//Keff=91
        if (nharderror>0)//then
        {
            for (int i = 0; i < N; ++i) //where(llr .ge. 0) hdec=1 //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
            {
                hdec[i]=0;
                if (llr[i]>=0.0) hdec[i]=1;
                nxor[i]=hdec[i] ^ cw[i]; //nxor=ieor(hdec,cw)
                dmin+=(double)nxor[i]*fabs(llr[i]);	//dmin=sum(nxor*abs(llr))
            } //ntype=2;
            return;
        }
    }
    nharderror=-1;
}*/
/*#define MSHV_METHOD
void PomFt::decode174_91_ft2a(double *llr,int maxosd,int norder,bool *apmask,bool *message91,bool *cw,int &nharderror,double &dmin)
{	//int Keff=91,
    const int N=174;
    const int K=91;
    const int M=(N-K);//83
    double zsave_[8][N];//zsave(N,3)
    double tov_[N][3];//(3,N)
    double toc_[M][7];//(7,M) //double tanhtoc_[M][7];//(7,M)
    double zsum[N+5];
    double zn[N+5];
    const int ncw=3;
    bool m96[96+5];
    bool hdec[N+2];
    bool nxor[N+2];
    int synd[M+5]; //const double alpha_ms = 0.75;  //! Min-Sum normalization factor
    double sign_prod, min_abs, second_min;
    const double LLRMAX = 20.0;//! Saturation limit (prevents runaway)
    double llr_in[N+5];
    double tov_prev_[N+5][3];//(3,N)//! Previous iteration tov (for SCMS + damping)
    int perm[M+5];//! Row permutation for restart
    int restart = 0;
    double alpha = 0.0;
    const double SCMS_BETA = 0.25;//! Self-correction strength
    const double DAMP = 0.20;//! Damping factor (mix of old message)
    int nosd=0; //maxosd=7;
    if (maxosd>7) maxosd=7;//if (maxosd>3) maxosd=3;
    if (maxosd==0) //then //! osd with channel llrs
    {
        nosd=1;
        for (int i = 0; i < N; ++i) zsave_[0][i]=llr[i];
    }
    else if (maxosd>0) nosd=maxosd;
    else if (maxosd<0) nosd=0;
    //!  Clamp input LLRs to prevent numerical issues
    for (int i = 0; i < N; ++i) //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    {//do i=1,N
        if (llr[i]> LLRMAX) llr[i]= LLRMAX;
        if (llr[i]<-LLRMAX) llr[i]=-LLRMAX;
    }
    for (int i = 0; i < N; ++i) llr_in[i] = llr[i];
    int nrestarts = 3;
    if (maxosd<0) nrestarts = 3;//! more restarts if no OSD
    //bool bp_converged = false;
    for (restart=0; restart<=nrestarts; ++restart)
    {//do restart = 0, nrestarts
        for (int i = 0; i<M; ++i)
        {
            for (int j = 0; j<7; ++j) toc_[i][j]=0.0;
        }
        for (int i = 0; i<N; ++i)
        {
            llr[i] = llr_in[i];
            zsum[i]=0.0;
            for (int j = 0; j<3; ++j)
            {
                tov_[i][j]=0.0;//tov=0
                tov_prev_[i][j]=0.0;
            }
        }
        for (int j = 0; j<M; ++j)
        {//do j=1,M
            for (int i = 0; i<(nrw_ft8_174_91[j]); ++i)
            {//do i=1,nrw(j)
                toc_[j][i]=llr[Nm_ft8_174_91_[j][i]-1];//toc(i,j)=llr((Nm(i,j)))
            }
        }//! Build row permutation for this restart
        for (int j = 0; j<M; ++j) perm[j] = j;
        if (restart>0) //!    Deterministic permutation: rotate by restart*prime and reverse blocks
        {
            for (int j = 0; j<M; ++j)
            {
            	int p0 = (j+restart*31);
            	perm[j] = (p0 % M);//fmod(j+restart*31,M);//perm(j) = mod(j-1 + restart*31, M) + 1
           	}
        }
        int ncnt=0;
        int nclast=0;
        for (int iter = 0; iter<=30; ++iter)//for (int iter = 0; iter<=50; ++iter)
        {	//! -- Adaptive alpha schedule -----------------------------
            if (iter<=5) alpha=0.60;//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
            else if (iter<=15) alpha=0.60 + 0.30*(double)(iter-5)/10.0;//! 0.60>0.90 linear
            else alpha=0.90;
#if defined MSHV_METHOD
			for (int i = 0; i< N; ++i)
            {//do i=1,N
                if (apmask[i]!=1)
                {
                    double sumt = 0.0;
                    for (int x = 0; x < ncw; ++x) sumt+=tov_[i][x];//zn[i]=llr[i]+sum(tov(1:ncw,i))
                    zn[i]=llr[i]+sumt;
                }
                else zn[i]=llr[i];
                if (zn[i]>0.0) cw[i]=1;
            	else cw[i]=0;
            	if (iter<maxosd) zsum[i]+=zn[i];
            }
            if (iter>0 && restart==0 && iter<=fmin(maxosd,6))
            {
                for (int i = 0; i< N; ++i) zsave_[iter][i]=zsum[i];// zsave(:,iter)=zsum
            }//! -- Check for valid codeword --
            int ncheck=0;
            for (int i = 0; i < M; ++i)
            {//do i=1,M
                int sum = 0;// synd(i)=sum(cw(Nm(1:nrw(i),i)))
                for (int x = 0; x < nrw_ft8_174_91[i]; ++x) sum += (int)cw[Nm_ft8_174_91_[i][x]-1];//hv-1
                synd[i]= sum;
                if (fmod(synd[i],2) != 0) ncheck++;
            }
            if (ncheck==0) //then ! we have a codeword - if crc is good, return it
            {
                int nbadcrc;
                for (int i = 0; i < 96; ++i)
                {
                    m96[i]=0;
                    if (i<77) m96[i]=cw[i]; //m96(1:77)=cw(1:77)
                    if (i>81) m96[i]=cw[i-5];//m96(83:96)=cw(78:91) {m96[i]=cw[i-5]; qDebug()<<i<<i-5; }
                }
                get_crc14(m96,96,nbadcrc);
                if (nbadcrc==0) //then
                {
                    int count = 0;
                    for (int i = 0; i < N; ++i)
                    {
                        if ((double)(2*cw[i]-1)*llr[i] < 0.0 ) count++;
                    }
                    nharderror=count;
                    for (int i = 0; i < 91; ++i) message91[i]=cw[i];// message91=cw(1:91)
                    for (int i = 0; i < N; ++i) //where(llr .ge. 0) hdec=1 //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
                    {
                        hdec[i]=0;
                        if (llr[i]>=0.0) hdec[i]=1;
                        nxor[i]=hdec[i] ^ cw[i]; //nxor=ieor(hdec,cw)
                        dmin+=(double)nxor[i]*fabs(llr[i]);	//dmin=sum(nxor*abs(llr))
                    }
                    //for (int i = 0; i < N; ++i)
                    //dmin+=nxor[i]*fabs(llr[i]);//dmin=sum(nxor*abs(llr))
                    //ntype=1
                    //bp_converged = true;
                    return;
                }
            }
            if ( iter > 0 )  //! this code block implements an early stopping criterion
            {   //if( iter.gt.10000 )
                int nd=ncheck-nclast;
                if ( nd < 0 ) ncnt=0; //! # of unsatisfied parity checks decreased //! reset counter
                else ncnt++;  //ncnt=ncnt+1;
                if ( ncnt >= 5 && iter >= 10 && ncheck > 15)
                {
                    nharderror=-1;
                    break;//exit return;
                }
            }
            nclast=ncheck;
#else
            for (int i = 0; i< N; ++i)
            {//do i=1,N
                if (apmask[i]!=1)
                {
                    double sumt = 0.0;
                    for (int x = 0; x < ncw; ++x) sumt+=tov_[i][x];//zn[i]=llr[i]+sum(tov(1:ncw,i))
                    zn[i]=llr[i]+sumt;
                }
                else zn[i]=llr[i];
            }
#endif
            if (iter>0)//! Save tov for self-correction + damping comparison
            {
                for (int i = 0; i<N; ++i)
                {
                    for (int j = 0; j<3; ++j) tov_prev_[i][j] = tov_[i][j];
                }
            }//! -- Layered update: process check nodes in permuted order --
            for (int jj = 0; jj<M; ++jj)
            {//do jj=1,M
                int j = perm[jj];//!Step A: compute variable-to-check messages for this check
                for (int i = 0; i<nrw_ft8_174_91[j]; ++i)
                {//do i=1,nrw(j)
                    int ibj=Nm_ft8_174_91_[j][i]-1;//Nm(i,j)
                    toc_[j][i]=zn[ibj];//toc(i,j)=zn(ibj)
                    for (int kk = 0; kk<ncw; ++kk)
                    {//do kk=1,ncw
                        if (Mn_ft8_174_91_[ibj][kk]-1 == j) toc_[j][i]=toc_[j][i]-tov_[ibj][kk];
                    }
                }//!Step B: compute new check-to-variable messages (Min-Sum + SCMS + Damping)
                for (int i = 0; i<nrw_ft8_174_91[j]; ++i)
                {//do i=1,nrw(j)
                    int ibj=Nm_ft8_174_91_[j][i]-1;//ibj=Nm(i,j)
                    for (int kk = 0; kk<ncw; ++kk)
                    {//do kk=1,ncw
                        if (Mn_ft8_174_91_[ibj][kk]-1 == j)//if( Mn(kk,ibj) == j )
                        {	//!Standard Normalized Min-Sum
                            sign_prod=1.0;
                            min_abs=1.0e30;
                            second_min=1.0e30;
                            for (int ll = 0; ll<nrw_ft8_174_91[j]; ++ll)
                            {//do ll=1,nrw(j)
                                if (ll!=i)//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
                                {
                                    sign_prod=sign_prod*copysign(1.0,toc_[j][ll]);
                                    if (fabs(toc_[j][ll])<min_abs)
                                    {
                                        second_min=min_abs;
                                        min_abs=fabs(toc_[j][ll]);
                                    }
                                    else if (fabs(toc_[j][ll])<second_min) second_min=fabs(toc_[j][ll]);
                                }
                            }//!-- Self-Corrected Min-Sum (SCMS) ----------
                            double correction = 0.0;
                            if (iter>1)
                            {
                                if (copysign(1.0,tov_prev_[ibj][kk])!=copysign(1.0,sign_prod*min_abs))
                                {
                                    correction = SCMS_BETA * fmax(0.0,min_abs - 0.5*second_min);
                                }
                            }
                            double tov_new = alpha * sign_prod * fmax(0.0, min_abs - correction);
                            //!-- Damping/Momentum ---------------------
                            //!Mix 80% new + 20% old to prevent oscillation
                            if (iter>1) tov_[ibj][kk] = (1.0-DAMP)*tov_new + DAMP*tov_prev_[ibj][kk];
                            else tov_[ibj][kk] = tov_new;//!Clamp to prevent saturation
                            if (tov_[ibj][kk]> LLRMAX) tov_[ibj][kk]= LLRMAX;
                            if (tov_[ibj][kk]<-LLRMAX) tov_[ibj][kk]=-LLRMAX;
                        }
                    }
                }//!Step C: IMMEDIATELY update variable beliefs
                for (int i = 0; i<nrw_ft8_174_91[j]; ++i)
                {//do i=1,nrw(j)
                    int ibj=Nm_ft8_174_91_[j][i]-1;//ibj=Nm(i,j)
                    double sum0=0.0;
                    if (apmask[ibj] != 1)
                    {
                        for (int z = 0; z<ncw; ++z) sum0+=tov_[ibj][z];//(1:ncw,ibj))
                        zn[ibj]=llr[ibj]+sum0;
                    }
                }
            }  //! j (check nodes) — end of layered sweep
#if !defined MSHV_METHOD
			//! -- Accumulate for OSD snapshots (7 instead of 5) --
            for (int i = 0; i< N; ++i) zsum[i]+=zn[i]; //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
            if (iter>0 && restart==0 && iter<=fmin(maxosd,6))
            {
                for (int i = 0; i< N; ++i) zsave_[iter][i]=zsum[i];// zsave(:,iter)=zsum
            }//! -- Check for valid codeword --
            int ncheck=0;
            for (int i = 0; i< N; ++i)
            {
                if (zn[i]>0.0) cw[i]=1;//cw=0 //where( zn .gt. 0. ) cw=1
                else cw[i]=0;
            }
            for (int i = 0; i < M; ++i)
            {//do i=1,M
                int sum = 0;// synd(i)=sum(cw(Nm(1:nrw(i),i)))
                for (int x = 0; x < nrw_ft8_174_91[i]; ++x) sum += (int)cw[Nm_ft8_174_91_[i][x]-1];//hv-1
                synd[i]= sum;
                if (fmod(synd[i],2) != 0) ncheck++;
            }
            if (ncheck==0) //then ! we have a codeword - if crc is good, return it
            {
                int nbadcrc;
                for (int i = 0; i < 96; ++i)
                {
                    m96[i]=0;
                    if (i<77) m96[i]=cw[i]; //m96(1:77)=cw(1:77)
                    if (i>81) m96[i]=cw[i-5];//m96(83:96)=cw(78:91) {m96[i]=cw[i-5]; qDebug()<<i<<i-5; }
                }
                get_crc14(m96,96,nbadcrc);
                if (nbadcrc==0) //then
                {
                    int count = 0;
                    for (int i = 0; i < N; ++i)
                    {
                        if ((double)(2*cw[i]-1)*llr[i] < 0.0 ) count++;
                    }
                    nharderror=count;
                    for (int i = 0; i < 91; ++i) message91[i]=cw[i];// message91=cw(1:91)
                    for (int i = 0; i < N; ++i) //where(llr .ge. 0) hdec=1 //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
                    {
                        hdec[i]=0;
                        if (llr[i]>=0.0) hdec[i]=1;
                        nxor[i]=hdec[i] ^ cw[i]; //nxor=ieor(hdec,cw)
                        dmin+=(double)nxor[i]*fabs(llr[i]);	//dmin=sum(nxor*abs(llr))
                    }
                    //for (int i = 0; i < N; ++i)
                    //dmin+=nxor[i]*fabs(llr[i]);//dmin=sum(nxor*abs(llr))
                    //ntype=1
                    //bp_converged = true;
                    return;
                }
            }
            if ( iter > 0 )  //! this code block implements an early stopping criterion
            {   //if( iter.gt.10000 )
                int nd=ncheck-nclast;
                if ( nd < 0 ) ncnt=0; //! # of unsatisfied parity checks decreased //! reset counter
                else ncnt++;  //ncnt=ncnt+1;
                if ( ncnt >= 7 && iter >= 15 && ncheck > 20)//if( ncnt .ge. 5 .and. iter .ge. 10 .and. ncheck .gt. 15) then
                {
                    nharderror=-1;
                    break;//exit return;
                }
            }
            nclast=ncheck;
#endif
        }   //! bp iterations
    }
    for (int io = 0; io < nosd; ++io)//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    {//do i=1,nosd
        if (io>6) break;
        for (int j = 0; j < N; ++j) zn[j]=zsave_[io][j];//zn=zsave(:,i)
        double dminosd = 0.0;
        osd174_91_1(zn,apmask,norder,message91,cw,nharderror,dminosd);//Keff=91
        if (nharderror>0)//then
        {
            for (int i = 0; i < N; ++i) //where(llr .ge. 0) hdec=1 //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
            {
                hdec[i]=0;
                if (llr[i]>=0.0) hdec[i]=1;
                nxor[i]=hdec[i] ^ cw[i]; //nxor=ieor(hdec,cw)
                dmin+=(double)nxor[i]*fabs(llr[i]);	//dmin=sum(nxor*abs(llr))
            } //ntype=2;
            return;
        }
    }
    nharderror=-1;
}*/
//// END POMFT ///

