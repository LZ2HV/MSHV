/* The algorithms, source code, look-and-feel of WSJT-X and related
 * programs, and protocol specifications for the modes FSK441, FT8, JT4,
 * JT6M, JT9, JT65, JTMS, QRA64, ISCAT, MSK144, are Copyright © 2001-2017
 * by one or more of the following authors: Joseph Taylor, K1JT; Bill
 * Somerville, G4WJS; Steven Franke, K9AN; Nico Palermo, IV3NWV; Greg Beam,
 * KI7MT; Michael Black, W9MDB; Edson Pereira, PY2SDR; Philip Karn, KA9Q;
 * and other members of the WSJT Development Group.
 *
 * MSHV JT65 Generator
 * Rewritten into C++ and modified by Hrisimir Hristov, LZ2HV 2015-2017
 * May be used under the terms of the GNU General Public License (GPL)
 */

#include "gen65.h"

//#include <QtGui>



/* Reed-Solomon encoder
 * Copyright 2002, Phil Karn, KA9Q
 * May be used under the terms of the GNU General Public License (GPL)
 */
///////////////////////// init_rs_int, encode_rs_int //////////
/* Initialize a Reed-Solomon codec
 * symsize = symbol size, bits (1-8)
 * gfpoly = Field generator polynomial coefficients
 * fcr = first root of RS code generator polynomial, index form
 * prim = primitive element to generate polynomial roots
 * nroots = RS code generator polynomial degree (number of roots)
 * pad = padding bytes at front of shortened block
 */
//void *INIT_RS(int symsize,int gfpoly,int fcr,int prim,
//	int nroots,int pad){
#include <stdlib.h>
//#include "int.h"
//#include "char.h"
#define DTYPE int
/* Reed-Solomon codec control block */
struct rs
{
    int mm;              /* Bits per symbol */
    int nn;              /* Symbols per block (= (1<<mm)-1) */
    DTYPE *alpha_to;     /* log lookup table */
    DTYPE *index_of;     /* Antilog lookup table */
    DTYPE *genpoly;      /* Generator polynomial */
    int nroots;     /* Number of generator roots = number of parity symbols */
    int fcr;        /* First consecutive root, index form */
    int prim;       /* Primitive element, index form */
    int iprim;      /* prim-th root of 1, index form */
    int pad;        /* Padding bytes in shortened block */
};

static int modnn(struct rs *rs,int x)
{
    while (x >= rs->nn)
    {
        x -= rs->nn;
        x = (x >> rs->mm) + (x & rs->nn);
    }
    return x;
}
#define MODNN(x) modnn(rs,x)

#define MM (rs->mm)
#define NN (rs->nn)
#define ALPHA_TO (rs->alpha_to)
#define INDEX_OF (rs->index_of)
#define GENPOLY (rs->genpoly)
//#define NROOTS (rs->nroots)
#define NROOTS (51)
#define FCR (rs->fcr)
#define PRIM (rs->prim)
#define IPRIM (rs->iprim)
#define PAD (rs->pad)
#define A0 (NN)

#define ENCODE_RS encode_rs_int
#define DECODE_RS decode_rs_int
#define INIT_RS init_rs_int
#define FREE_RS free_rs_int

void *init_rs_int(int symsize,int gfpoly,int fcr,int prim,
                  int nroots,int pad)
{
    struct rs *rs;
    int i, j, sr,root,iprim;

    /* Check parameter ranges */
    //if(symsize < 0 || symsize > 8*sizeof(DTYPE))
    if (symsize < 0 || (unsigned int)symsize > 8*sizeof(DTYPE))
        return NULL; /* Need version with ints rather than chars */

    if (fcr < 0 || fcr >= (1<<symsize))
        return NULL;
    if (prim <= 0 || prim >= (1<<symsize))
        return NULL;
    if (nroots < 0 || nroots >= (1<<symsize))
        return NULL; /* Can't have more roots than symbol values! */
    if (pad < 0 || pad >= ((1<<symsize) -1 - nroots))
        return NULL; /* Too much padding */

    rs = (struct rs *)calloc(1,sizeof(struct rs));
    rs->mm = symsize;
    rs->nn = (1<<symsize)-1;
    rs->pad = pad;

    rs->alpha_to = (DTYPE *)malloc(sizeof(DTYPE)*(rs->nn+1));
    if (rs->alpha_to == NULL)
    {
        free(rs);
        return NULL;
    }
    rs->index_of = (DTYPE *)malloc(sizeof(DTYPE)*(rs->nn+1));
    if (rs->index_of == NULL)
    {
        free(rs->alpha_to);
        free(rs);
        return NULL;
    }

    /* Generate Galois field lookup tables */
    rs->index_of[0] = A0; /* log(zero) = -inf */
    rs->alpha_to[A0] = 0; /* alpha**-inf = 0 */
    sr = 1;
    for (i=0;i<rs->nn;i++)
    {
        rs->index_of[sr] = i;
        rs->alpha_to[i] = sr;
        sr <<= 1;
        if (sr & (1<<symsize))
            sr ^= gfpoly;
        sr &= rs->nn;
    }
    if (sr != 1)
    {
        /* field generator polynomial is not primitive! */
        free(rs->alpha_to);
        free(rs->index_of);
        free(rs);
        return NULL;
    }

    /* Form RS code generator polynomial from its roots */
    rs->genpoly = (DTYPE *)malloc(sizeof(DTYPE)*(nroots+1));
    if (rs->genpoly == NULL)
    {
        free(rs->alpha_to);
        free(rs->index_of);
        free(rs);
        return NULL;
    }
    rs->fcr = fcr;
    rs->prim = prim;
    rs->nroots = nroots;

    /* Find prim-th root of 1, used in decoding */
    for (iprim=1;(iprim % prim) != 0;iprim += rs->nn)
        ;
    rs->iprim = iprim / prim;

    rs->genpoly[0] = 1;
    for (i = 0,root=fcr*prim; i < nroots; i++,root += prim)
    {
        rs->genpoly[i+1] = 1;

        /* Multiply rs->genpoly[] by  @**(root + x) */
        for (j = i; j > 0; j--)
        {
            if (rs->genpoly[j] != 0)
                rs->genpoly[j] = rs->genpoly[j-1] ^ rs->alpha_to[modnn(rs,rs->index_of[rs->genpoly[j]] + root)];
            else
                rs->genpoly[j] = rs->genpoly[j-1];
        }
        /* rs->genpoly[0] can never be zero */
        rs->genpoly[0] = rs->alpha_to[modnn(rs,rs->index_of[rs->genpoly[0]] + root)];
    }
    /* convert rs->genpoly[] to index form for quicker encoding */
    for (i = 0; i <= nroots; i++)
        rs->genpoly[i] = rs->index_of[rs->genpoly[i]];

    return rs;
}

void encode_rs_int(
#ifdef FIXED
    DTYPE *data, DTYPE *bb,int pad)
{
#else
    void *p,DTYPE *data, DTYPE *bb)
{
    struct rs *rs = (struct rs *)p;
#endif
    int i, j;
    DTYPE feedback;

#ifdef FIXED
    /* Check pad parameter for validity */
    if (pad < 0 || pad >= NN)
        return;
#endif

    memset(bb,0,NROOTS*sizeof(DTYPE));

    for (i=0;i<NN-NROOTS-PAD;i++)
    {
        feedback = INDEX_OF[data[i] ^ bb[0]];
        if (feedback != A0)
        {      /* feedback term is non-zero */
#ifdef UNNORMALIZED
            /* This line is unnecessary when GENPOLY[NROOTS] is unity, as it must
             * always be for the polynomials constructed by init_rs()
             */
            feedback = MODNN(NN - GENPOLY[NROOTS] + feedback);
#endif
            for (j=1;j<NROOTS;j++)
                bb[j] ^= ALPHA_TO[MODNN(feedback + GENPOLY[NROOTS-j])];
        }
        /* Shift */
        memmove(&bb[0],&bb[1],sizeof(DTYPE)*(NROOTS-1));
        if (feedback != A0)
            bb[NROOTS-1] = ALPHA_TO[MODNN(feedback + GENPOLY[0])];
        else
            bb[NROOTS-1] = 0;
    }
}

// ot ftrsd ne ot lib
/*void encode_rs_int(
#ifndef FIXED
void *p,
#endif
DTYPE *data, DTYPE *bb){
#ifndef FIXED
  struct rs *rs = (struct rs *)p;
#endif
  int i, j;
  DTYPE feedback;

  memset(bb,0,NROOTS*sizeof(DTYPE));

  for(i=0;i<NN-NROOTS;i++){
    feedback = INDEX_OF[data[i] ^ bb[0]];
    if(feedback != A0){      // feedback term is non-zero
#ifdef UNNORMALIZED
      // This line is unnecessary when GENPOLY[NROOTS] is unity, as it must
       // always be for the polynomials constructed by init_rs()
       //
      feedback = MODNN(NN - GENPOLY[NROOTS] + feedback);
#endif
      for(j=1;j<NROOTS;j++)
	bb[j] ^= ALPHA_TO[MODNN(feedback + GENPOLY[NROOTS-j])];
    }
    // Shift
    memmove(&bb[0],&bb[1],sizeof(DTYPE)*(NROOTS-1));
    if(feedback != A0)
      bb[NROOTS-1] = ALPHA_TO[MODNN(feedback + GENPOLY[0])];
    else
      bb[NROOTS-1] = 0;
  }
}*/
///////////////////////////////end init_rs_int, encode_rs_int Reed-Solomon encoder //////////

Gen65::Gen65()
{
	//2.12
    for (int i = 0; i < 126; ++i)
    {
        pr[i]=0.0;
    	mdat[i] = 0;
    	mdat2[i] = 0;
    	mref_[0][i] = 0;
    	mref_[1][i] = 0;
    	mref2_[0][i] = 0;
    	mref2_[1][i] = 0;
    }
    rs = NULL;
	//2.12    
    
    twopi=8.0*atan(1.0);
    first=true;
    sendingsh = 0;

    //(GEN_SAMPLE_RATE= 48000sr*250ms)/1000ms = 12000 smples for 250ms  48000-12000=36000
    //count_1s = 48000.0-((48000.0*700.0)/1000.0);//siquencer 250ms <- MsPlayerHV::xplaymessage
    //qDebug()<<count_1s;
}
Gen65::~Gen65()
{}
void Gen65::setup65()
{
    int nprc[126]={1,0,0,1,1,0,0,0,1,1,1,1,1,1,0,1,0,1,0,0,
                   0,1,0,1,1,0,0,1,0,0,0,1,1,1,0,0,1,1,1,1,
                   0,1,1,0,1,1,1,1,0,0,0,1,1,0,1,0,1,0,1,1,
                   0,0,1,1,0,1,0,1,0,1,0,0,1,0,0,0,0,0,0,1,
                   1,0,0,0,0,0,0,0,1,1,0,1,0,0,1,0,1,1,0,1,
                   0,1,0,1,0,0,1,1,0,0,1,0,0,1,0,0,0,0,1,1,
                   1,1,1,1,1,1};
    int mr2 = 0;
    //int mdat[126];
    //int mdat2[126];
    //int mref_[2][126];
    //int mref2_[2][126];
    //mref_[0][0] = 0;
    //mref2_[0][0]= 0;

    //! Put the appropriate pseudo-random sequence into pr
    int nsym=126;
    for (int i = 0; i < nsym; i++)
    {//do i=1,nsym
        pr[i]=(double)2.0*nprc[i]-1.0;
    }

    //! Determine locations of data and reference symbols
    int k=0;
    int mr1=-1;  //0
    for (int i = 0; i < nsym; i++)
    {//do i=1,nsym
        if (pr[i]<0.0)
        {
            //k=k+1;
            mdat[k]=i;
            k++;
        }
        else
        {
            mr2=i;
            if (mr1==-1) mr1=i; //if(mr1.eq.0) mr1=i
        }
    }
    int nsig=k;

    //! Determine the reference symbols for each data symbol.
    for (k = 0; k < nsig; k++)
    {//do k=1,nsig
        int n;
        int m=mdat[k];
        mref_[0][k]=mr1; //mref(k,1)=mr1
        for (n = 0; n < 10; n++)
        {//do n=1,10                     //!Get ref symbol before data
            if ((m-n)>-1) //hv if((m-n)>0) then if((m-n).gt.0) then
            {
                if (pr[m-n]>0.0) goto c10;
            }
        }
        goto c12;
c10:
        mref_[0][k]=m-n;
c12:
        mref_[1][k]=mr2;
        for (n = 0; n < 10; n++)
        {//do n=1,10              //!Get ref symbol after data //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
            if ((m+n)<nsym)   //hv if((m+n)<=nsym) then
            {
                if (pr[m+n]>0.0) goto c20; //if (pr(m+n).gt.0.0) go to 20
            }
        }
        goto c22;
c20:
        mref_[1][k]=m+n;
c22:
        continue;
    }

    //! Now do it all again, using opposite logic on pr(i)
    k=0;
    mr1=-1;//0
    for (int i = 0; i < nsym; i++)
    {//do i=1,nsym
        if (pr[i]>0.0)
        {
            //k=k+1
            mdat2[k]=i;
            k++;
        }
        else
        {
            mr2=i;
            if (mr1==-1) mr1=i;//hv -1 from 0
        }
    }
    nsig=k;
    for (k = 0; k < nsig; k++)
    {//do k=1,nsig
        int n;
        int m=mdat2[k];
        mref2_[0][k]=mr1;
        for (n = 0; n < 10; n++)
        {//do n=1,10
            if ((m-n)>-1)    //hv if((m-n)>0) then //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
            {
                if (pr[m-n]<0.0) goto c110;
            }
        }
        goto c112;
c110:
        mref2_[0][k]=m-n;
c112:
        mref2_[1][k]=mr2;
        for (n = 0; n < 10; n++)
        {//do n=1,10
            if ((m+n)<nsym)  //hv if((m+n)<=nsym) then
            {
                if (pr[m+n]<0.0) goto c120;
            }
        }
        goto c122;
c120:
        mref2_[1][k]=m+n;
c122:
        continue;
    }
}
// vremenno mai postoianno 1.46
QString Gen65::unpackmsg(int *dat,char &ident)
{
    return TPackUnpackMsg.unpackmsg(dat,ident,false);//rpt_db_msg ,"      "
}
void Gen65::packmsg(char *cmsg,int *dgen,int &itype)//1.49 for deep search 65
{
    TPackUnpackMsg.packmsg(cmsg,dgen,itype,false);//false,
}
void Gen65::chkmsg(QString &message,QString &cok,int &nspecial,double &flip)
{
    nspecial=0;
    flip=1.0;
    int i = 0;
    for (i = 21; i >= 0; i--)
    {//do i=22,1,-1
        if (message.at(i)!=' ') goto c10;
    }
    i=21;//22   //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.

c10:
    if (i>=10) //11
    {//if((message(i-3:i).eq.' OOO') .or. (message(20:22).eq.' OO')) then
        if ((message.mid(i-3,4)==" OOO") || (message.mid(19,3)==" OO"))
        {
            //qDebug()<<"message.mid"<<message.mid(19,3);
            cok="OOO";
            flip=-1.0;
            if (message.mid(19,3)==" OO") //if(message(20:22)==" OO")
                message=message.mid(0,19); //message=message(1:19)
            else
                message=message.mid(0,i-3);     //1.35 4=3 message=message(1:i-4)
        }
    }
    message.append("                               ");//30 triabva

    if (message.mid(4,18)=="                  ") //if(message(5:22)=="                  ")
    {
        if (message.mid(0,2)=="RO")  nspecial=2;
        if (message.mid(0,3)=="RRR") nspecial=3;
        if (message.mid(0,2)=="73")  nspecial=4;
    }
    //qDebug()<<"message"<<message<<nspecial;
}

void Gen65::rs_encode(int *dgen, int *sent)
// Encode JT65 data dgen[12], producing sent[63].
{
    int dat1[12];
    int b[51];
    int i;

    if (first)
    {
        // Initialize the JT65 codec
        rs=init_rs_int(6,0x43,3,1,51,0); //rs=init_rs_int(6,0x43,3,1,51,0);
        first=false;
    }

    // Reverse data order for the Karn codec.
    for (i=0; i<12; i++)
    {
        dat1[i]=dgen[11-i];
    }
    // Compute the parity symbols
    encode_rs_int(rs,dat1,b);

    // Move parity symbols and data into sent[] array, in reverse order.
    for (i = 0; i < 51; i++) sent[50-i] = b[i];
    for (i = 0; i < 12; i++) sent[i+51] = dat1[11-i];
}

void Gen65::move(int *x, int *y, int n)
{
    //real x(n),y(n)
    for (int i = 0; i < n; i++)
    {//do i=1,n
        y[i]=x[i];
    }
}
void Gen65::interleave63(int *d1,int idir)
{
    //! Interleave (idir=1) or de-interleave (idir=-1) the array d1.
    //integer d1(0:6,0:8)
    //integer d2(0:8,0:6)
    int d2[63];
    if (idir >= 0) //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE. then
    {
        int z = 0;
        for (int i = 0; i < 7; i++)
        {//do i=0,6
            int k = 0;
            for (int j = 0; j < 9; j++)
            {//do j=0,8
                //qDebug()<<"z="<<i+k<<z;
                d2[z]=d1[i+k]; //d2(j,i)=d1(i,j)
                k+=7;
                z++;
            }
        }
        move(d2,d1,63); //call move(d2,d1,63)
    }
    else
    {
        move(d1,d2,63); //call move(d1,d2,63)
        int z = 0;
        for (int i = 0; i < 7; i++)
        {//do i=0,6
            int k = 0;
            for (int j = 0; j < 9; j++)
            {//do j=0,8
                d1[i+k]=d2[z];//d1(i,j)=d2(j,i)
                k+=7;
                z++;
            }
        }
    }
}
int Gen65::igray(int n0, int idir)
{
    int n;
    unsigned long sh;
    unsigned long nn;
    n=n0;//n=*n0;

    if (idir>0) return (n ^ (n >> 1));//if(*idir>0) return (n ^ (n >> 1));

    sh = 1;
    nn = (n >> sh);
    while (nn > 0)
    {
        n ^= nn;
        sh <<= 1;
        nn = (n >> sh);
    }
    return (n);
}
void Gen65::graycode(int *ia,int n,int idir,int *ib)
{
    //integer ia(n),ib(n)
    for (int i = 0; i < n; i++)
    {//do i=1,n
        ib[i]=igray(ia[i],idir);
    }
}
void Gen65::graycode65(int *dat,int n,int idir)
{
    for (int i = 0; i < n; i++)
    {//do i=1,n
        dat[i]=igray(dat[i],idir);
    }
}
int Gen65::gen65(char *cmsg,int mode65,int nfast,double samfac,double GEN_SAMPLE_RATE,int *t_iwave)//double ntxdf, double /*koef_srate*/
{
    //qDebug()<<"GEN_SAMPLE_RATE"<<GEN_SAMPLE_RATE;
    //! Encodes a JT65 message into a wavefile.
    s_unpack_msg = "";
    QString cok = "   ";
    int nspecial = 0;
    double flip = 1.0;
    QString message;
    int dgen[13]; //12
    int itype = -1;
    int sent[63];
    double tsymbol = 0.0;
    int nsym = 0;
    double dt,f0,dfgen,t,phi;
    double dphi = 0.0;
    int nwave = 0;
    sendingsh = 0;

    if (fabs(pr[0])!=1.0) setup65();//if(abs(pr(1)).ne.1.0) call setup65

    //for (int z = 0; z < 126; z++)
    // qDebug()<<pr[z]<<z;

    int nmsg1 = strlen(cmsg);
    for (int z = 0; z < nmsg1; z++) message.append(cmsg[z]);
    message.append("                               ");//30 triabva

    for (int z = 0; z < 13; z++)
        dgen[z] = 0;

    //for (int z = 0; z < 63; z++)
    //sent[z] = 0;

    chkmsg(message,cok,nspecial,flip);

    if (nspecial==0)
    {
        TPackUnpackMsg.packmsg(cmsg,dgen,itype,false);          //!Pack message into 72 bits false,
        sendingsh=0;
        if ((dgen[9] & 8)!=0)//if(iand(dgen(10),8)!=0) sendingsh=-1;    //!Plain text flag //sent(k)=iand(1,ishft(i-1,-n)) -> sent[k]=1 & (i >> n)
        {
            //qDebug()<<"Plain text flag";
            sendingsh=-1;
        }
        rs_encode(dgen,sent);

        interleave63(sent,1);              //!Apply interleaving


        //qDebug()<<"nspecial="<<nspecial;

        graycode65(sent,63,1); //graycode(sent,63,1);               //!Apply Gray code
        tsymbol=4096.0/((double)nfast*11025.0);//ne se buta48khz raboti se s11025 *1.088435374149;
        nsym=126;                                                //!Symbols per transmission
    }
    else
    {
        tsymbol=16384.0/11025.0;   //ne se buta48khz raboti se s11025 *1.088435374149;
        nsym=32;
        sendingsh=1;                                   //!Flag for shorthand message
    }
    //qDebug()<<"sendingsh"<<sendingsh;
    //! Set up necessary constants
    dt=1.0/(samfac*GEN_SAMPLE_RATE);// moze bi samo towa48khz
    f0=(118.0*(11025.0)/1024.0);//+ ntxdf; //1270.46  ne se buta48khz raboti se s11025   /1.0884 ;
    dfgen=(mode65*(11025.0)/4096.0);    //ne se buta48khz raboti se s11025   /1.0884
    t=0.0;
    phi=0.0;
    int k=0;
    int j0=-1; //-1 hv  vazno za " ooo"  j0=0
    int ndata=((double)nsym*GEN_SAMPLE_RATE*samfac*tsymbol)/2.0; // moze bi samo towa48khz
    ndata=2*ndata;
    int i = 0;

    for (int z = 0; z < ndata; z++)
    {//do i=1,ndata  //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
        t=t+dt;
        int j=int(t/tsymbol) + 0;  //vazno za " ooo" j=int(t/tsymbol) + 1                // !Symbol number, 1-126
        if (j!=j0)
        {
            double f=f0;
            if (nspecial!=0 && fmod(j,2)==0) f=f0+10.0*(double)nspecial*dfgen;
            if (nspecial==0 && (flip*pr[j])<0.0)
            {
                //k=k+1
                f=f0+(double)(sent[k]+2)*dfgen;  //f=f0+(sent(k)+2)*dfgen
                k++;
                //qDebug()<<"=========================flip="<<pr[j];
            }
            //qDebug()<<"NOflip=";
            dphi=twopi*dt*f;
            j0=j;
        }
        phi=phi+dphi;
        if (phi > twopi) phi -= twopi;
        t_iwave[i]=(int)(8380000.0*sin(phi)); //2.70 8380000.0 full=8388607
        i++;
    }

    for (int z = 0; z < GEN_SAMPLE_RATE*5 ; z++)  ////5s + 1.35 6000=0.5za48khz 11025=5512 moze bi samo towa48khz *1,088435374149
    {//do j=1,5512                                // !Put another 0.5 sec of silence at end
        //i=i+1
        t_iwave[i]=0;
        i++;
    }
    nwave=i;
    //qDebug()<<"nwave"<<nwave/GEN_SAMPLE_RATE;

    char ident;//fiktivno
    //QString msgsent;
    if (sendingsh == 1)
    {
        bool flg = false;//1.69 sh skip blinks
        for (int i1 = 25; i1 >= 0; --i1)
        {
            if (message[i1]!=' ')
                flg = true;
            if (flg)
                s_unpack_msg.prepend(message[i1]);
        }
        //s_unpack_msg = message;// sh ne se codira za towa e taka na jt65
    }
    else
        s_unpack_msg = TPackUnpackMsg.unpackmsg(dgen,ident,false);//last is rpt_db_msk false,"      ",

    if (flip<0.0)
    {
        for (i = 21; i >= 0; i--)
        {//do i=22,1,-1
            if (s_unpack_msg.at(i)!=' ') goto c10;
        }

c10:
        s_unpack_msg.append(" OOO");//' OOO'//msgsent=msgsent(1:i)//' OOO'
    }
    //qDebug()<<"s_unpack_msg"<<s_unpack_msg;

    /*for (i = 21; i >= 0; i--)
    {//do i=22,1,-1
       if(msgsent(i:i).ne.' ') goto c20;
    }

    c20; 
    int nmsg=i;*/

    return nwave;
}