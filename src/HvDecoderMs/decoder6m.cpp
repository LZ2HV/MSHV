/* The algorithms, source code, look-and-feel of WSJT-X and related
 * programs, and protocol specifications for the modes FSK441, FT8, JT4,
 * JT6M, JT9, JT65, JTMS, QRA64, ISCAT, MSK144, are Copyright © 2001-2017
 * by one or more of the following authors: Joseph Taylor, K1JT; Bill
 * Somerville, G4WJS; Steven Franke, K9AN; Nico Palermo, IV3NWV; Greg Beam,
 * KI7MT; Michael Black, W9MDB; Edson Pereira, PY2SDR; Philip Karn, KA9Q;
 * and other members of the WSJT Development Group.
 *
 * MSHV JT6M Decoder 
 * Rewritten into C++ and modified by Hrisimir Hristov, LZ2HV 2015-2017
 * May be used under the terms of the GNU General Public License (GPL)
 */

#include "decoderms.h"

#define _JT6M_DH_
#include "../config_msg_all.h"

//#include <QtGui>
void DecoderMs::add_da2_da_to_da2(double a_[6][128],double *b,double c_[6][128],int begin,int row,int n)
{
    //add(s2(ia,k),x(ia),s2(ia,k),ib-ia+1)
    //add(s2,x,s2,ia,k,ib-ia+1);
    //subroutine add(a,b,c,n)
    //real a(n),b(n),c(n)
    //do i=1,n
    //c(i)=a(i)+b(i)

    // gre6no ///////////////
    //for (int i = begin; i<n; i++)
    //c[i][row]=a[i][row]+b[i];
    // gre6no  //////////////

    for (int i = 0; i<n; i++)
        c_[row][i+begin]=a_[row][i+begin]+b[i+begin];
}
void DecoderMs::syncf0(double *data,int jz,int NFreeze,int NTol,int &jstart,double &f0)
{
    //! Does 512-pt FFTs of data with 256-pt step size.
    //! Finds sync tone and determines aproximate values for jstart and f0.
    double smax = 0.0;
    //real s2(128,6)
    double s2_[6][128];              //!Average spectra at half-symbol spacings
    double x[512+10];
    //complex cx(0:511)
    double complex cx[512+10];
    //double complex z;
    //equivalence (x,cx)
    //double ps[z];
    //ps[z]=creal(z)*creal(z) + cimag(z)*cimag(z);    //???      //!Power spectrum function
    //zero(s2,6*128)                                //!Clear average
    for (int i = 0; i<6; i++)
    {
        for (int j = 0; j<128; j++)
            s2_[i][j]=0.0;
    }

    double df=DEC_SAMPLE_RATE/512.0;

    int ia=(f0-400)/df;
    int ib=(f0+400)/df + 0.999;
    if (NFreeze==1)
    {
        ia=(f0-NTol)/df;
        ib=(f0+NTol)/df + 0.999;
    }
    //qDebug()<<f0;
    //! Most of the time in this routine is in this loop.
    int nblk=jz/256 - 6;
    for (int n = 0; n<nblk; n++)                            //!Accumulate avg spectrum for
    {//do n=1,nblk
        //int j=256*(n-1)+1;
        int j=256*(n);
        //call move(data(j),x,512)                      //!512-pt blocks, stepping by 256
        move_da_to_da(data,j,x,0,512); //qDebug()<<j<<512;

        xfft(cx,x,512);
        for (int i = ia; i<ib; i++)
            //x(i)=ps(cx(i))
            //x[i]=creal(cx[i]);
        {
            x[i]=pomAll.ps_hv(cx[i]);//creal(cx[i])*creal(cx[i]) + cimag(cx[i])*cimag(cx[i]);
            //qDebug()<<x[i]<<i;
        }
        //k=mod(n-1,6)+1
        int k=(int)fmod(n,6);
        //add(s2(ia,k),x(ia),s2(ia,k),ib-ia+1)  !Average at each step
        add_da2_da_to_da2(s2_,x,s2_,ia,k,ib-ia+1);  //!Average at each step
        //qDebug()<<n<<"ib-ia+1";
    }

    //! Look for best spectral peak, using the "sync off" phases as reference.
    smax=0.0;
    for (int i = ia; i<ib; i++)
    {//  do i=ia,ib
        for (int k = 0; k<6; k++)
        {//do k=1,6
            //k1=mod(k+1,6)+1
            //k2=mod(k+3,6)+1
            int k1=fmod(k+2,6);//ok provereno
            int k2=fmod(k+4,6);//ok provereno
            //qDebug()<<k1<<k2;
            double r=0.5*(s2_[k1][i]+s2_[k2][i]);
            if(r==0.0)//no devide by zero
            	r=1.0;
            double s=s2_[k][i]/r;
            //qDebug()<<i<<s<<r;
            //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
            if (s>smax)
            {
                smax=s;
                //jstart=(k-1)*256 + 1
                jstart=(k)*256;      //!Best starting place for sync
                f0=(double)i*df; // 1076.77                 //!Best sync frequency
                //qDebug()<<i<<df;
            }
            //qDebug()<<df<<ib;
        }
    }
    //f0 = 1076.77;
    //qDebug()<<ib<<df;
}
void DecoderMs::synct(double *data,int jz,int &jstart,double f0)
{
    //! Gets a refined value of jstart.
    const int NMAX=(1024*1024);//2.12
    int NB3=3*512;

    double smax = 0.0;

    double dpha;
    double complex z,dz;
    double complex c1,zz;
    double complex *c = new double complex[NMAX];

    //qDebug()<<"RRRRRRRRRRRRRRRRRRR=";
    //common/hcom/c(NMAX)
    //ps(zz)=real(zz)**2 + aimag(zz)**2          !Power spectrum function
    // ! Convert data to baseband (complex result) using quadrature LO.
    //twopi=8*atan(1.d0)
    dpha=twopi*f0/DEC_SAMPLE_RATE;
    dz=cos(dpha)+(-sin(dpha))*I;
    //z=(1.0/dz)+(1.0/dz)*I;
    z=1.0/dz;

    //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    if (jz >= NMAX)
    {
        //print*,"synct jz >= NMAX ", jz
        //stop
        delete [] c;//delete [] c;
        return;
    }
    for (int i = 0; i<jz; i++)
    {//  do i=1,jz
        z=z*dz;
        c[i]=data[i]*z;
    }
    //! Detect zero-beat sync tone in 512-sample chunks, stepped by 1.
    //! Sums replace original values in c(i).
    zz=0.0+0.0*I;
    for (int i = 0; i<512; i++)                    //!Compute first sum
        zz=zz+c[i];

    //c1=c(1)
    //c(1)=zz
    c1=c[0];
    c[0]=zz;

    for (int i = 1; i<jz-512; i++)
    {//do i=2,jz-512                   //!Compute the rest recursively
        //zz=c(i-1)+c(i+511)-c1
        zz=c[i-1]+c[i+510]-c1;
        c1=c[i];                      //!Save original value
        c[i]=zz;                      //!Save the sum
    }

    //! Iterate to find the best jstart.
    jstart=jstart+NB3;
    int nz=(jz-jstart)/NB3 -0; //qDebug()<<nz;
    smax=0.0;
    int jstep=256;
    int jbest=jstart;

c10:
    jstep=jstep/2;
    jstart=jbest;
    //do j=jstart-jstep,jstart+jstep,jstep
    for (int j = jstart-jstep; j<jstart+jstep; j+=jstep)
    {
        double s=0.0;
        double r=0.0;
        for (int n = 0; n<nz; n++)
        {//do n=1,nz
            //k=(n-1)*NB3 + j
            int k=(n)*NB3 + j;
            //s=s + ps(c(k))
            //r=r + ps(c(k+512)) + ps(c(k+1024))
            s=s + pomAll.ps_hv(c[k]);
            r=r + pomAll.ps_hv(c[k+512])+pomAll.ps_hv(c[k+1024]);
            //qDebug()<<j<<n<<k;
        }
        if(r==0.0)//no devide by zero
        	r=1.0;
        s=2.0*s/r;                               //!Better to use s/r or s-r?
        if (s>smax)
        {
            smax=s;
            jbest=j;
        }
        //qDebug()<<j;
    }
    //if(jstep.gt.1) go to 10
    if (jstep>0) goto c10;

    jstart=jbest;
    if (jstart>NB3) jstart=jstart-NB3;

    //qDebug()<<jstart;
    delete [] c;
}
void DecoderMs::add_da_da_to_da(double *a,double *b,double *c,int n)
{
    //add(s,x,s,NQ);
    for (int i = 0; i<n; i++)
    {
        c[i]=a[i]+b[i];
    }
}

void DecoderMs::syncf1(double*data,int jz,int jstart,double &f0,int NFreeze,int DFTol,double*red)
{
    //! Does 16k FFTs of data with stepsize 15360, using only "sync on" intervals.
    //! Returns a refined value of f0, the sync-tone frequency.
    double smax = 0.0;
    const int NFFT=16384;
    const int NH=NFFT/2;
    const int NQ=NFFT/4;
    int NB3=3*512;
    
    double x[NFFT+10];
    double complex c[NH+10];    
    double s[NQ+50];     //!Ref spectrum for flattening and birdie-zapping

    //! Accumulate a high-resolution average spectrum
    double df=(double)DEC_SAMPLE_RATE/(double)NFFT;
    int jstep=10*NB3;
    int nz=(jz-jstart)/jstep - 0;
    pomAll.zero_double_beg_end(s,0,NQ);

    for (int n = 0; n<nz; n++)
    {//do n=1,nz
        pomAll.zero_double_beg_end(x,0,NFFT);

        //k=(n-1)*jstep
        int k=(n)*jstep;
        for (int i = 0; i<10; i++)
        {//do i=1,10
            //j=(i-1)*NB3 + 1
            int j=(i)*NB3;
            //move(data(jstart+k+j),x(j),512)
            move_da_to_da(data,(jstart+k+j),x,(j),512);
            //qDebug()<<jstart+k+j<<j<<jz;
        }
        //call xfft(x,NFFT)
        xfft(c,x,NFFT);

        for (int i = 0; i<NQ; i++)
            //x(i)=ps(c(i))
            x[i]=pomAll.ps_hv(c[i]);

        //add(s,x,s,NQ);
        add_da_da_to_da(s,x,s,NQ);
        //qDebug()<<n<<nz<<jz;
    }

    double fac=((double)1.0/NFFT)*((double)1.0/NFFT);
    for (int i = 0; i<NQ; i++)
    {//do i=1,NQ                               //!Normalize
        s[i]=fac*s[i];
    }
    smooth(s,NQ);

    //! NB: could also compute a "blue" spectrum, using the sync-off intervals.
    int n8=NQ/8;

    for (int i = 1; i<=n8; i++)
    {
        red[i-1]=0.0;
        for (int k = (8*i)-7; k<=8*i; k++)
        {
            red[i-1]=red[i-1]+s[k-1];
            //qDebug()<<i-1<<k-1;
        }
        red[i-1]=10.0*red[i-1]/(8.0*nz);
    }

    double dftol=fmin(DFTol,25.0);//25
    //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    if (NFreeze==1) dftol=DFTol;   //qDebug()<<"NFreeze="<<NFreeze;
    //! Find improved value for f0
    int ipk=0; //!Shut up compiler warning -db
    smax=0.0;

    int ia=(double)(f0-dftol)/df;
    int ib=(double)(f0+dftol)/df + 0.999;
    //!      if(NFreeze.eq.1) then
    //!         ia=(f0-5.)/df
    //!         ib=(f0+5.)/df
    //!     endif
    for (int i = ia; i<ib; i++)
    {
        if (s[i]>smax)
        {
            smax=s[i];
            ipk=i;
            //qDebug()<<"syncf10="<<ipk<<i<<ia<<ib;
        }
    }
    //qDebug()<<"syncf101="<<f0;
    f0=(double)ipk*df;
    //f0=1078.68;
    //qDebug()<<"syncf102="<<f0<<ipk<<df;

    //! Remove line at f0 from spectrum -- if it's strong enough.
    ia=(double)(f0-150.0)/df;
    ib=(double)(f0+150.0)/df;
    double a1=0.0;
    double a2=0.0;
    int nsum=50;
    for (int i = 0; i<nsum; i++)
    {
        a1=a1+s[ia-i];
        a2=a2+s[ib+i];
    }
    a1=a1/(double)nsum;
    a2=a2/(double)nsum;
    smax=2.0*smax/(a1+a2);

    if (smax>3.0)
    {
        double b=(a2-a1)/(double)(ib-ia);
        for (int i = ia; i<ib; i++)
        {
            s[i]=a1 + (double)(i-ia)*b;
        }
    }

    //! Make a smoothed version of the spectrum.
    nsum=50;
    fac=1.0/(double)(2*nsum+1);
    pomAll.zero_double_beg_end(x,0,nsum);
    pomAll.zero_double_beg_end(s,0,50);
    //zero_double_beg_count(s,(NQ-nsum),nsum); call zero(s(NQ-nsum),nsum)
    pomAll.zero_double_beg_end(s,(NQ-nsum),NQ);// v1.17
    double sum=0.0;
    for (int i = nsum+1; i<NQ-nsum; i++)
    {
        sum=sum+s[i+nsum]-s[i-nsum];
        x[i]=fac*sum;
    }
    //call zero(x(NQ-nsum),nsum+1)
    pomAll.zero_double_beg_end(x,(NQ-nsum),(NQ+1));// v1.17

    //! To zap birdies, compare s(i) and x(i).  If s(i) is larger by more
    //! than some limit, replace x(i) by s(i).  That will put narrow birdies
    //! on top of the smoothed spectrum.

    //move(x,s,NQ)                 //!Copy smoothed spectrum into s
    move_da_to_da(x,0,s,0,NQ);
}
void DecoderMs::add_da_da2_to_da(double *a,double b_[646][44],int b_row,double *c,int n)
{
    //call add(ref,s2db(0,j),ref,44)
    //add_d_to_dd(ref,s2db,j,ref,44);
    for (int i = 0; i<n; i++)
        c[i]=a[i]+b_[b_row][i];
}

double DecoderMs::max_2double(double a,double b)
{
    double res = a;
    if (b>res)
        res=b;
    return res;
}
//static double s2db[44][646];
void DecoderMs::decode6m(double *data,int d_start,int jz,int minSigdb,int NFixLen,double f0)//,int npkept,double*yellow)
{
    //! Decode a JT6M message.  Data must start at the beginning of a
    //! sync symbol; sync frequency is assumed to be f0.
    //!There must be a better way of doing this
    //!NMAX is overruled in lfp1 to be 1024*1024 which is called from here -db
    bool f_only_one_color = true;
    int NMAX=(1024*1024);
    //real s2db(0:43,646)        //!Spectra of symbols
    double s2db_[646][44];
    //!      real s2(128,646)
    double syncsig[646];
    //real ref(0:43)
    double  ref[44];
    //char pua[43];
    //char msg[48];

    QString msg_out;
    QString msg;
    //QString msg1;
    //char cfile6[6];
    double dpha;
    double complex z,dz;
    //double complex zz;
    //complex ct(0:511)
    double complex ct[512+10];
    double complex *c = new double complex[NMAX+10];
    //common/hcom/c(NMAX)

    //data pua/'0123456789., /#?$ABCDEFGHIJKLMNOPQRSTUVWXYZ'/
    //data offset/20.6/
    double offset = 20.6;
    double tping = 0.0;// vremeto ot na4alnia reper
    double width = 0.0;

    //ps(zz)=real(zz)**2 + aimag(zz)**2          !Power spectrum function

    //! Convert data to baseband (complex result) using quadrature LO.
    dpha=twopi*f0/DEC_SAMPLE_RATE;
    //dz=cmplx(cos(dpha),-sin(dpha));
    dz=cos(dpha)+(-sin(dpha))*I;
    z=1.0/dz;
    //qDebug()<<d_start<<jz;
    for (int i = 0; i<jz; i++)
    {
        z=z*dz;
        c[i]=data[i+d_start]*z;
    }
    //qDebug()<<"11111";
    //! Get spectrum for each symbol.
    //! NB: for decoding pings, could do FFTs first for sync intervals only,
    //! and then for data symbols only where the sync amplitude is above
    //! threshold.  However, for the average message we want all FFTs computed.
    pomAll.zero_double_beg_end(ref,0,44);
    //qDebug()<<"22222";

    //nz=jz/512 - 1
    int nz=jz/(512 - 0);//HV 1->0
    double fac=1.0/512.0;
    for (int j = 0; j<nz; j++)
    {//do j=1,nz
        //i0=512*(j-1) + 1
        int i0=512*(j);

        //do i=0,511
        for (int i = 0; i<512; i++)
            ct[i]=fac*c[i0+i];

        //call four2a(ct,512,1,-1,1)
        f2a.four2a_c2c(ct,512,-1,1);

        //! Save PS for each symbol

        for (int i = 0; i<128; i++)
        {// do i=0,127
            //xps=ps(ct(i));
            double xps=pomAll.ps_hv(ct[i]);
            //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
            if (i<=43) s2db_[j][i]=xps;
            //!            s2(i+1,j)=xps
        }
        //if(mod(j,3).eq.1)
        if (fmod(j,3)==0)
            //call add(ref,s2db(0,j),ref,44)
            add_da_da2_to_da(ref,s2db_,j,ref,44); //!Accumulate ref spec
    }

    //! Return sync-tone amplitudes for plotting.
    //iz=nz/3 -1
    /*int iz=nz/3 - 0;//HV 1->0
    //qDebug()<<iz<<nz;
    for (int i = 0; i<iz; i++)
    {//do i=1,iz
        // j=3*i-2
        int j=3*i;
        //yellow(i)=s2db(0,j)-0.5*(s2db(0,j+1)+s2db(0,j+2))
        yellow[i]=s2db[0][j]-0.5*(s2db[0][j+1]+s2db[0][j+2]);
    }
    yellow[216]=iz;//???*/

    fac=(double)3.0/(double)nz;// prag na zadejttwane hv mj 4.0
    for (int i = 0; i<44; i++)
    {                               //!Normalize the ref spectrum
        ref[i]=fac*ref[i];
    }
    ref[0]=ref[2];                           //!Sync bin uses bin 2 as ref

    for (int j = 0; j<nz; j++)
    {//do j=1,nz
        //m=mod(j-1,3)                             //!Compute strength of sync
        double m=fmod(j,3);                         //!signal at each j.
        int ja=j-m-3;
        int jb=ja+3;
        //if(ja.lt.1) ja=ja+3
        if (ja<0) ja=ja+3;
        if (jb>nz) jb=jb-3;
        //qDebug()<<ja<<jb;
        syncsig[j]=0.5*(s2db_[ja][0]+s2db_[jb][0])/ref[0];
        syncsig[j]=pomAll.db(syncsig[j]) - offset;
        for (int i = 0; i<44; i++)
        {                            //!Normalize s2db
        	double div = ref[i];
        	if(div==0.0)//no div by zero
        		div=1.0;
            s2db_[j][i]=s2db_[j][i]/div; 
        }
        //qDebug()<<syncsig[j];
    }

    //! Decode any message of 2 or more consecutive characters bracketed by
    //! sync-tones above a threshold.
    //! Use hard-decoding (i.e., pick max bin).

    int nslim=minSigdb;                       //!Signal limit for decoding
    int ndf0=int(f0-1076.66); //(f0-1076.77); //!Freq offset DF, in Hz
    int n=0;                                  //!Number of decoded characters
    int j0=0;
    double sbest=-1.e9;
    int nsig = (int)sbest;

    double tping_best = 0.0;
    double s_sbest = sbest;
 
    int max_count_msg = 46;//v1.30 46 for 800pix   800pix broia na charakterite v edin red na listata za jt6m
    //int part = max_count_msg;
    int nsig_avg_c = 0;
    int nsig_avg = 0;
    double width_all = 0.0;


    //do j=2,nz-1,3
    for (int j = 1; j<nz-1; j+=3)//ok (nz-1) testvano ina4e iskarva +2000db
    {   //qDebug()<<syncsig[j]<<double(nslim);//qDebug()<<j<<nz;
        if (syncsig[j]>=double(nslim))
        {
            //qDebug()<<syncsig[j]<<double(nslim);
            //! Is it time to write out the results?
            //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
            if ((n==48) || (j!=j0+3 && j0!=0))
            {
                nsig=int(sbest);
                //qDebug()<<"sbest1="<<sbest;
                width=(512.0/DEC_SAMPLE_RATE)*(1.5*n+1.0);
                //qDebug()<<"nsig="<<nsig<<nslim;
                if (nsig>=nslim)
                {
                    //npkept=npkept+1;
                    //call cs_lock('decode6m')
                    //write(lumsg,1010) cfile6,tping,width,nsig,ndf0,(msg(k:k),k=1,n)
                    //if(lcum) write(21,1010) cfile6,tping,width,nsig,ndf0,(msg(k:k),k=1,n);
//c1010:            //format(a6,2f5.1,i4,i5,6x,48a1)       //!### 6x was 7x ###
                    //call cs_unlock

                    width_all += width;
                    nsig_avg += nsig;
                    nsig_avg_c++;

                    msg_out.append(msg); //qDebug()<<"SSS="<<tping<<msg<<syncsig[j]<<nslim;
                    msg.clear();

                    /*if (msg_out.count()>part)
                    {
                        int pouse = msg_out.lastIndexOf(" ",part);

                        if (pouse==-1)// ako niama nikade pauza ina4e bezkraen cikal
                        {
                            pouse=part;
                            //msg_out.insert(pouse,"\n");
                            //qDebug()<<"SSS="<<tping;
                        }
                        //else
                        msg_out.replace(pouse,1,"\n");
                        //qDebug()<<"Kasaa1="<<part<<pouse<<(part - (pouse+2));
                        part+=max_count_msg - (part - (pouse+2));
                        //qDebug()<<"Kasaa2="<<part<<msg_out.count();
                        //msg_out.insert(part,"\n");
                        //part+=max_count_msg;
                    }*/
                    //qDebug()<<"1="<<msg_out<<msg_out.count()<<part;

                }
                n=0;
                sbest=-1.e9;
            }
            j0=j;
            double smax1=-1.e9;
            int ipk=0; //!Shut up compiler warning. -db
            for (int i = 1; i<44; i++)//i=1 ne se polzva 0->s2dc ina4e ipk=-1 provereno 44 ina4e niama Z i 0
            {//do i=1,43                         //!Pick max bin for 1st char
                if (s2db_[j][i]>smax1)
                {
                    smax1=s2db_[j][i];
                    ipk=i;
                }
            }
            //n=n+1;
            if (n==0)
                tping=j*512.0/DEC_SAMPLE_RATE + (s_in_istart)/DEC_SAMPLE_RATE; //-1 ne pri men start at 0
            //msg[n]=pua_JT6M_RX[ipk-1]; //start at 0                      //!Decoded character
            msg.append(pua_JT6M_RX[ipk-1]);
            n++;

            double smax2=-1.e9;
            for (int i = 1; i<44; i++)//i=1 ne se polzva 0->s2dc ina4e ipk=-1 provereno 44 ina4e niama Z i 0
            {//do i=1,43
                if (s2db_[j+1][i]>smax2)
                {
                    smax2=s2db_[j+1][i];//start at 0
                    ipk=i;
                }
            }
            //n=n+1;
            //msg[n]=pua_JT6M_RX[ipk-1];
            msg.append(pua_JT6M_RX[ipk-1]);
            n++;

            double sig0=pow(10.0,(0.1*(syncsig[j]+offset)));
            double sig=pomAll.db(0.5*sig0 + 0.25*(smax1+smax2))-offset;
            sbest=max_2double(sbest,sig);

            if (sbest>s_sbest)
            {
                //qDebug()<<"BEST="<<sbest<<s_sbest<<tping;
                tping_best = tping;
                s_sbest = sbest;
            }
            //qDebug()<<"sbest1="<<smax1<<smax2;
        }
    }

    nsig=int(sbest);
    width=(512.0/DEC_SAMPLE_RATE)*(1.5*n+1.0);

    if (n!=0 && nsig>=nslim)
    {
        if (nsig > 20)
            nsig =20;

        //npkept=npkept+1;

        width_all += width;
        nsig_avg += nsig;
        nsig_avg_c++;

        msg_out.append(msg);
    }
    //qDebug()<<"3="<<msg_out;
    if (!msg_out.isEmpty())
    {
        //qDebug()<<"MSG_IN="<<msg_out;
        msg_out = FormatLongMsg(msg_out,max_count_msg); //max_count_msg
        //qDebug()<<"MSG_OUT="<<msg_out;

        if (f_only_one_color)
        {
            f_only_one_color = false;
            SetBackColor();
        }
        if(nsig_avg_c==0)
        	nsig_avg_c=1;
        nsig_avg = nsig_avg/nsig_avg_c;

        QStringList list;
        list <<s_time<<QString("%1").arg(tping_best,0,'f',1)<<QString("%1").arg(width_all,0,'f',2)<<
        QString("%1").arg(nsig_avg)<<QString("%1").arg(ndf0)
        <<msg_out<<"All"<<QString("%1").arg((int)f0);



        emit EmitDecodetText(list,s_fopen,true);//1.27 psk rep   fopen bool true    false no file open

        if (s_mousebutton == 0) //mousebutton Left=1, Right=3 fullfile=0 rtd=2
            emit EmitDecLinesPosToDisplay(1,tping_best,tping_best,s_time);

        /* niama smisal decodira vinagi samo vednaz 1.32
        if (s_mousebutton == 0 && disp_lines_dec_cou < MAX_DISP_DEC_COU) //mousebutton Left=1, Right=3 fullfile=0 rtd=2
           {
              disp_lines_dec_cou++;
              emit EmitDecLinesPosToDisplay(disp_lines_dec_cou,t2,t2,s_time);
           }
        */
    }


    /*if (n!=0 && nsig>=nslim)
    {
        if (nsig > 20)
            nsig =20;

        npkept=npkept+1;
        //part++;// = 506; //test for tri digits

        msg2 = RemWSpacesInside(msg2);
        msg2 = RemBegEndWSpaces(msg2);

        if (f_only_one_color)
        {
            f_only_one_color = false;
            SetBackColor();
        }

        //part_str++;

        QStringList list;
        list <<s_time<<QString("%1").arg(tping,0,'f',1)<<QString("%1").arg(width,0,'f',2)<<
        QString("%1").arg(nsig)<<QString("%1").arg(ndf0)
        <<msg2<<"Part"+QString("%1").arg(part_str)<<QString("%1").arg((int)f0);
        emit EmitDecodetText(list);

        //write(lumsg,1010) cfile6,tping,width,nsig,ndf0,(msg(k:k),k=1,n)
        //if(lcum) write(21,1010) cfile6,tping,width,nsig,ndf0,(msg(k:k),k=1,n)
    }*/

    //! Decode average message for the whole record.
    avemsg6m(s2db_,nz,nslim,NFixLen,f0,f_only_one_color);

    delete [] c;
}

void DecoderMs::move_da2_to_da2(double x_[646][44],int j,double y_[646][44],int k,int n)
{
    //real x(n),y(n)
    //move(s2db(0,j),s2db(0,k),44)
    for (int i = 0; i<n; i++)
        y_[k][i]=x_[j][i];

}
void DecoderMs::add_da2_da2_to_da2(double a_[646][44],int j,double b_[53][44],int k,double c_[53][44],int kk,int n)// hv v1.01 23 to 53
{
    //real a(n),b(n),c(n)
    //call add(s2db(0,j),s2dc(0,k),s2dc(0,k),44)
    for (int i = 0; i<n; i++)
        c_[kk][i]=a_[j][i]+b_[k][i];
}

void DecoderMs::avemsg6m(double s2db_[646][44],int nz,int nslim,int NFixLen,double f0,bool f_only_one_color)
{
    //! Attempts to find message length and then decodes an average message.

    //real s2db(0:43,nz)
    double s2dc_[53][44]; // hv v1.01 23 to 53
    double wgt[53];//hv v1.01 23 to 53
    double acf[430];
    //char avemsg[23] = {' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',
    //                   ' ',' ',' ',' ',' ',' ',' ',' ',' ',' '};
    QString avemsg;
    //char blanks[23] = {' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',
    //                   ' ',' ',' ',' ',' ',' ',' ',' ',' ',' '};
    double offset=20.6;
    int msglen = 0;

    //! Adjustable sig limit, depending on length of data to average.
    double nslim2=(double)nslim - 9.0 + 4.0*log10((double)624.0/(double)nz);       //!### +10 was here
    //qDebug()<<nslim2<<nslim;

    int k=0;
    double sum=0.0;
    int nsum=0;
    int max_count_msg = 46; //v1.32
    
    
    for (int j = 0; j<nz; j++)
    {
        //if(mod(j,3).eq.1) then
        if (fmod(j,3)==0)
        {
            sum=sum+s2db_[j][0];        //!Measure avg sig strength for sync tone
            nsum=nsum+1;
            //qDebug()<<sum;
        }
        else
        {
            //k=k+1;
            //move(s2db(0,j),s2db(0,k),44)  !Save data spectra
            move_da2_to_da2(s2db_,j,s2db_,k,44);  //!Save data spectra
            k++;
        }
    }
    if(nsum==0)
    	nsum=1;
    double sig=(double)sum/(double)nsum;                                 //!Signal strength estimate
    int nsig=int(pomAll.db(sig)-offset);
    //qDebug()<<nslim2<<nslim<<nsig;

    //! Most of the time in this routine is in this loop.
    int kz=k;
    for (int lag = 0; lag<kz-0; lag++)
    {//do lag=0,kz-1
        sum=0.0;
        for (int j = 0; j<kz-lag; j++)
        {//do j=1,kz-lag
            for (int i = 0; i<44; i++)
            {
                sum=sum+s2db_[j][i]*s2db_[j+lag][i];
            }
        }
        acf[lag]=sum;
    }
    double acf0=acf[0];
    if(acf0==0.0)
    	acf0=1.0;
    for (int lag = 0; lag<kz-0; lag++)
    {//do lag=0,kz-1
        acf[lag]=acf[lag]/acf0;
    }

    int lmsg1=NFixLen/256;
    int lmsg2=NFixLen-(256*lmsg1);
    if (fmod(lmsg1,2)==1) lmsg1=lmsg1+1;
    if (fmod(lmsg2,2)==1) lmsg2=lmsg2+1;
    //qDebug()<<fmod(lmsg1,2)<<fmod(lmsg2,2);
    double smax=-1.e9;
    for (int ip = 4; ip<=52; ip+=2)// hv v1.01 22 to 52
    {//do ip=4,22,2
        double f=1.0/(double)(ip);
        double s=0.0;
        //qDebug()<<ip;
        //!Compute periodogram for allowed msg periods
        if (NFixLen!=0 && ip!=4 && ip!=lmsg1 && ip!=lmsg2) goto c5;


        for (int lag = 0; lag<kz-0; lag++)
        {
            s=s+acf[lag]*cos(twopi*f*lag);
        }
        //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
        if (s>smax)
        {
            smax=s;
            msglen=ip;                            //!Save best message length
        }
c5:
        continue;
    }
    //qDebug()<<msglen<<nz;
    //! Average the symbols from s2db into s2dc.

    //zero(s2dc,44*22);
    for (int i = 0; i<53; i++)
    {
        for (int j = 0; j<44; j++)//hv v1.01 23 to 53
            s2dc_[i][j] = 0.0;
    }
    pomAll.zero_double_beg_end(wgt,0,53);//hv v1.01 23 to 53
    for (int j = 0; j<kz; j++)
    {
        //k=mod(j-1,msglen)+1
        k=fmod(j,msglen);
        //call add(s2db(0,j),s2dc(0,k),s2dc(0,k),44)
        add_da2_da2_to_da2(s2db_,j,s2dc_,k,s2dc_,k,44);
        wgt[k]=wgt[k]+1.0;
    }

    int ipk=0; //!Shut up compiler warnings. -db
    for (int j = 0; j<msglen; j++)
    {                            //!Hard-decode the avg msg,
        smax=-1.e9;                            //!picking max bin for each char
        for (int i = 1; i<44; i++)//i=1 ne se polzva 0->s2dc ina4e ipk=-1 provereno 44 ina4e niama Z i 0
        {//do i=1,43
        	double div = wgt[j];
        	if(div==0.0)
        		div=1.0;
            s2dc_[j][i]=s2dc_[j][i]/div;
            if (s2dc_[j][i]>smax)
            {
                smax=s2dc_[j][i];
                ipk=i;
                //qDebug()<<i<<j;
            }
        }
        k=fmod(ipk,3);
        int i=ipk;
        //avemsg[j]=pua_JT6M_RX[i-1];
        avemsg.append(pua_JT6M_RX[i-1]);
        //qDebug()<<i-1;
    }
    int ndf0=int(f0-1076.66);
    /*tarsi i maha praznite mesta otpred hv pri men s->RemBegEndWSpaces(avemsg)
    for (int i = 0; i<msglen; i++)
    {
        //if(avemsg(i:i).eq.' ') goto 10
        if (avemsg.at(i)==' ') goto c10;
    }
    goto c20;
    c10: //avemsg=avemsg(i+1:msglen)//avemsg(1:i)
    c20:*/
    //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    //qDebug()<<nsig<<nslim2;
    if (nsig>nslim2)
    {
        //npkept=npkept+1;
        //avemsg=avemsg[1:msglen];//blanks <- hv pravi go blanks prazno dalgo za da slozi nakraia *
        //for (int i = 0; i<msglen; i++)
        avemsg = FormatFoldMsg(avemsg);

        bool f_align = false;
        avemsg = AlignMsgSpecWord(avemsg,"CQ",f_align);
        if (!f_align)
            avemsg = AlignMsgSpecWord(avemsg,"QRZ",f_align);
        if (!f_align)
            avemsg = AlignMsgSpecWord(avemsg,s_MyCall,f_align);

        avemsg = RemWSpacesInside(avemsg);
        avemsg = RemBegEndWSpaces(avemsg);//hv add
        //avemsg.prepend("* ");//  removed v095 hv

        double width = 0.0;
        QStringList list;
        double time_sec =1.0/DEC_SAMPLE_RATE*(s_in_istart);//-1 ne pri men start at 0
        //qDebug()<<time_sec<<s_istart;

        avemsg = FormatLongMsg(avemsg,max_count_msg); //v1.32 triabva max_count_msg

        if (f_only_one_color)
        {
            f_only_one_color = false;
            SetBackColor();
        }

        list <<s_time<<QString("%1").arg(time_sec,0,'f',1)<<QString("%1").arg(width,0,'f',2)<<
        QString("%1").arg(nsig)<<QString("%1").arg(ndf0)
        <<avemsg<<"* "+QString("%1").arg(msglen)<<QString("%1").arg((int)f0);// average Avg



        emit EmitDecodetText(list,s_fopen,true);//1.27 psk rep   fopen bool true    false no file open

        //vinagi e 0.0 time_sec niama smisal
        //if (s_mousebutton == 0) //mousebutton Left=1, Right=3 fullfile=0 rtd=2
        //emit EmitRtdDecPosToDisplay(2,time_sec,time_sec,s_time);

        //call cs_lock('avemsg6m')
        //write(lumsg,1020) cfile6,nsig,ndf0,avemsg,msglen
        //if(lcum) write(21,1020) cfile6,nsig,ndf0,avemsg,msglen
        //1020 format(a6,8x,i6,i5,7x,a22,19x,'*',i4)
        //call cs_unlock
    }

}
void DecoderMs::wsjt1_jt6m(double *static_dat, int s_static_dat_count,double /*basevb*/)
{
    //! If we're in JT6M mode, call the 6M decoding routines.

    int jz = s_static_dat_count;
    double red[512];
    double psavg[450];
    int jstart = 0;
    //double yellow0[216];
    //double yellow[216];
    //double ps0[450];           //!Spectrum of best ping
    pomAll.zero_double_beg_end(psavg,0,450);
    // for (int i = 0; i<450; i++)
    //psavg[i] =0.0;

    //int nstep=221;
    //int nchan=64;                   //!Save 64 spectral channels
    //int nz=jz/nstep - 1;            //!# of spectra to compute
    //double s2[nchan][nz];
    double sigma = 0.0;
    int MouseDF =0;
    int DFTol = G_DfTolerance;    //!Defines DF search range
    int nslim = G_MinSigdB;
    int NFreeze=0; //v1.15 -> 0 all other version 1
    //double smax = 0.0;
    //int MinSigdB = 1;      //!Minimum ping strength, dB
    //bool lcum = true;
    //int lumsg = 0;//????          // !Logical unit for decoded.txt
    //int npkept = 1;//???        // !Number of pings kept and decoded

    //double ccf_plus[5200];
    //double *ccf = &ccf_plus[2600];//ot -2600 to 2600
    //double ccf(-5:540)        //!X-cor function in JT65 mode (blue line)
    double ccf_p[545];
    double *ccf = &ccf_p[5];        //!X-cor function in JT65 mode (blue line)
    //int istart = 0;         //!Starting location in original d() array

    for (int i = 0; i<jz; i++)//!### Why is it level-sensitive?
        static_dat[i]=static_dat[i]/25.0;
    //!           write(73) jz,nz,cfile6,(dat(j),j=1,jz)
    //! For waterfall plot
    //spec2d(dat,jz,nstep,s2,nchan,nz,psavg,sigma)
    spec2d(static_dat,jz,sigma);


    int NFixLen;
    double df;
    double f0;
    double f00;

    //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    //if (sigma<0.0) basevb=-99.0;
    //qDebug()<<"SSSS="<<(double)jz/DEC_SAMPLE_RATE<<3.9;
    if ((double)jz/DEC_SAMPLE_RATE<3.9 || sigma<0.0) goto c900;

    f0=1076.66;
    if (NFreeze>=1) f0=1076.66 + MouseDF;
    f00=f0;

    syncf0(static_dat,jz,NFreeze,DFTol,jstart,f0);//jstart,f0,smax se izmeniat
    //qDebug()<<"syncf0="<<f0;
    synct(static_dat,jz,jstart,f0);
    //qDebug()<<"synct="<<f0;
    syncf1(static_dat,jz,jstart,f0,NFreeze,DFTol,red);
    //qDebug()<<"syncf1="<<f0;

    //do i=1,512
    //ccf(i-6)=dB(red(i))
    //enddo
    for (int i = 0; i<512; i++)
        ccf[i-5]=pomAll.db(red[i]);

    df=(double)DEC_SAMPLE_RATE/256.0;
    for (int i = 1; i<=64; i++)
    {
        double sum=0.0;
        for (int k = (8*i)-7; k<=8*i; k++)
            sum=sum+red[k-1];

        psavg[i-1]=5.0*sum;
        double fac=1.0;
        double freq=(double)(i-1)*df;
        if (freq>2500.0)
            //fac=((freq-2500.)/20.0)**(-1.0);
            fac=pow(((freq-2500.0)/20.0),(-1.0));
        psavg[i-1]=fac*psavg[i-1];
        psavg[i+64-1]=0.001;
    }
    //jz=jz-jstart+1
    jz-=jstart;
    NFixLen=0;

    //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    //! Call the decoder if DF is in range or Freeze is off.
    //qDebug()<<"out="<<jz<<jstart;
    //qDebug()<<fabs(f0-f00)<<(double)DFTol;
    if (NFreeze==0 || fabs(f0-f00)<(double)DFTol)
    {
        decode6m(static_dat,jstart,jz,nslim,NFixLen,f0);//,yellow);//f0
        //qDebug()<<"f0="<<f0;
    }

    /*if (pick)
    {
        for (int i = 0; i<216; i++)
            ps0[i]=yellow0[i];
    }
    else
    {
        ps0[216]=yellow[216];//???
        yellow0[216]=yellow[216];//???
        for (int i = 0; i<215; i++)
        {
            ps0[i]=2*yellow[i];
            yellow0[i]=ps0[i];
        }
    }*/
    //goto 800
c900:

    //delete ccf;
    return;
}
