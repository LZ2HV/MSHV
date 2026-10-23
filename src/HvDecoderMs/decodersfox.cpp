/* The algorithms, source code, look-and-feel of WSJT-X and related
 * programs, and protocol specifications for the modes FSK441, FT8, JT4,
 * JT6M, JT9, JT65, JTMS, QRA64, ISCAT, MSK144, are Copyright © 2001-2017
 * by one or more of the following authors: Joseph Taylor, K1JT; Bill
 * Somerville, G4WJS; Steven Franke, K9AN; Nico Palermo, IV3NWV; Greg Beam,
 * KI7MT; Michael Black, W9MDB; Edson Pereira, PY2SDR; Philip Karn, KA9Q;
 * and other members of the WSJT Development Group.
 *
 * MSHV SFox Decoder
 * Rewritten into C++ and modified by Hrisimir Hristov, LZ2HV 2015-2024
 * May be used under the terms of the GNU General Public License (GPL)
 */

#include "decoderms.h"
//#include "../HvMsPlayer/libsound/genpom.h"
//#include "ft_all_ap_def.h"
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
qpccode_ds qpccoderx = {
                           128,        //n
                           127,        //np
                           50,         //k
                           128,        //q
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
static void   _qpc_sumdiff8_16(float* y, float* t)
{
    y[0] = t[0] + t[16];
    y[16] = t[0] - t[16];

    y[1] = t[1] + t[17];
    y[17] = t[1] - t[17];

    y[2] = t[2] + t[18];
    y[18] = t[2] - t[18];

    y[3] = t[3] + t[19];
    y[19] = t[3] - t[19];

    y[4] = t[4] + t[20];
    y[20] = t[4] - t[20];

    y[5] = t[5] + t[21];
    y[21] = t[5] - t[21];

    y[6] = t[6] + t[22];
    y[22] = t[6] - t[22];

    y[7] = t[7] + t[23];
    y[23] = t[7] - t[23];
}
static void   _qpc_sumdiff8_32(float* y, float* t)
{
    y[0] = t[0] + t[32];
    y[32] = t[0] - t[32];

    y[1] = t[1] + t[33];
    y[33] = t[1] - t[33];

    y[2] = t[2] + t[34];
    y[34] = t[2] - t[34];

    y[3] = t[3] + t[35];
    y[35] = t[3] - t[35];

    y[4] = t[4] + t[36];
    y[36] = t[4] - t[36];

    y[5] = t[5] + t[37];
    y[37] = t[5] - t[37];

    y[6] = t[6] + t[38];
    y[38] = t[6] - t[38];

    y[7] = t[7] + t[39];
    y[39] = t[7] - t[39];
}
static void   _qpc_sumdiff8_64(float* y, float* t)
{
    y[0] = t[0] + t[64];
    y[64] = t[0] - t[64];

    y[1] = t[1] + t[65];
    y[65] = t[1] - t[65];

    y[2] = t[2] + t[66];
    y[66] = t[2] - t[66];

    y[3] = t[3] + t[67];
    y[67] = t[3] - t[67];

    y[4] = t[4] + t[68];
    y[68] = t[4] - t[68];

    y[5] = t[5] + t[69];
    y[69] = t[5] - t[69];

    y[6] = t[6] + t[70];
    y[70] = t[6] - t[70];

    y[7] = t[7] + t[71];
    y[71] = t[7] - t[71];
}
float* qpc_fwht8(float* y, float* x)
{
    float t[8];
    // first stage
    y[0] = x[0] + x[1];
    y[1] = x[0] - x[1];

    y[2] = x[2] + x[3];
    y[3] = x[2] - x[3];

    y[4] = x[4] + x[5];
    y[5] = x[4] - x[5];

    y[6] = x[6] + x[7];
    y[7] = x[6] - x[7];

    // second stage
    t[0] = y[0] + y[2];
    t[2] = y[0] - y[2];

    t[1] = y[1] + y[3];
    t[3] = y[1] - y[3];

    t[4] = y[4] + y[6];
    t[6] = y[4] - y[6];

    t[5] = y[5] + y[7];
    t[7] = y[5] - y[7];

    // third stage
    y[0] = t[0] + t[4];
    y[4] = t[0] - t[4];

    y[1] = t[1] + t[5];
    y[5] = t[1] - t[5];

    y[2] = t[2] + t[6];
    y[6] = t[2] - t[6];

    y[3] = t[3] + t[7];
    y[7] = t[3] - t[7];

    return y;
}
float* qpc_fwht16(float* y, float* x)
{
    float t[16];
    qpc_fwht8(t, x);
    qpc_fwht8(t + 8, x + 8);

    y[0] = t[0] + t[8];
    y[8] = t[0] - t[8];

    y[1] = t[1] + t[9];
    y[9] = t[1] - t[9];

    y[2] = t[2] + t[10];
    y[10] = t[2] - t[10];

    y[3] = t[3] + t[11];
    y[11] = t[3] - t[11];

    y[4] = t[4] + t[12];
    y[12] = t[4] - t[12];

    y[5] = t[5] + t[13];
    y[13] = t[5] - t[13];

    y[6] = t[6] + t[14];
    y[14] = t[6] - t[14];

    y[7] = t[7] + t[15];
    y[15] = t[7] - t[15];

    return y;
}
float* qpc_fwht32(float* y, float* x)
{
    float t[32];
    qpc_fwht16(t, x);
    qpc_fwht16(t + 16, x + 16);

    _qpc_sumdiff8_16(y, t);
    _qpc_sumdiff8_16(y + 8, t + 8);

    return y;
}
float* qpc_fwht64(float* y, float* x)
{
    float t[64];
    qpc_fwht32(t, x);
    qpc_fwht32(t + 32, x + 32);

    _qpc_sumdiff8_32(y, t);
    _qpc_sumdiff8_32(y + 8, t + 8);
    _qpc_sumdiff8_32(y + 16, t + 16);
    _qpc_sumdiff8_32(y + 24, t + 24);

    return y;
}
float* qpc_fwht128(float* y, float* x)
{
    float t[128];
    qpc_fwht64(t, x);
    qpc_fwht64(t + 64, x + 64);

    _qpc_sumdiff8_64(y, t);
    _qpc_sumdiff8_64(y + 8, t + 8);
    _qpc_sumdiff8_64(y + 16, t + 16);
    _qpc_sumdiff8_64(y + 24, t + 24);
    _qpc_sumdiff8_64(y + 32, t + 32);
    _qpc_sumdiff8_64(y + 40, t + 40);
    _qpc_sumdiff8_64(y + 48, t + 48);
    _qpc_sumdiff8_64(y + 56, t + 56);

    return y;
}

#if QPC_LOG2Q==7
#define qpc_fwht(a,b) qpc_fwht128(a,b)
#endif
#if QPC_LOG2Q==6
#define qpc_fwht(a,b) qpc_fwht64(a,b)
#endif
#if QPC_LOG2Q==5
#define qpc_fwht(a,b) qpc_fwht32(a,b)
#endif
#if QPC_LOG2Q==4
#define qpc_fwht(a,b) qpc_fwht16(a,b)
#endif
// Note that it makes no sense to use fast convolutions
// for transforms that are less than 16 symbols in size.
// For such cases direct convolutions are faster.
#if QPC_LOG2Q==3
#define qpc_fwht(a,b) qpc_fwht8(a,b)
#endif

// static constant / functions
static const float knorm = 1.0f / QPC_Q;
static float* pdf_conv(float *dst, float* pdf1, float* pdf2)
{
    // convolution between two pdf

    float fwht_pdf1[QPC_Q];
    float fwht_pdf2[QPC_Q];
    int k;

    qpc_fwht(fwht_pdf1, pdf1);
    qpc_fwht(fwht_pdf2, pdf2);

    for (k = 0; k < QPC_Q; k++)
        fwht_pdf1[k] *= fwht_pdf2[k];

    qpc_fwht(dst, fwht_pdf1);

    for (k = 0; k < QPC_Q; k++)
        dst[k] *= knorm;

    return dst;
}
static void pdfarray_conv(float* dstarray, float* pdf1array, float* pdf2array, int numrows)
{
    int k;
    // convolutions between rows of pdfs
    for (k = 0; k < numrows; k++)
    {
        pdf_conv(dstarray, pdf1array, pdf2array);
        dstarray  += QPC_Q;
        pdf1array += QPC_Q;
        pdf2array += QPC_Q;
    }

}
static float _pdfuniform[QPC_Q];
static const float* _pdf_uniform1();
static const float* _pdf_uniform0();
typedef const float*(*ptr_pdfuniform)(void);
static ptr_pdfuniform _ptr_pdf_uniform = _pdf_uniform0;
static const float* _pdf_uniform1()
{
    return _pdfuniform;
}
static const float* _pdf_uniform0()
{
    // compute uniform pdf once for all
    int k;
    for (k = 0; k < QPC_Q; k++)
        _pdfuniform[k] = knorm;

    // next call to _qpc_pdfuniform
    // will be handled directly by _pqc_pdfuniform1
    _ptr_pdf_uniform = _pdf_uniform1;

    return _pdfuniform;
}
static const float*  pdf_uniform()
{
    return _ptr_pdf_uniform();
}
static float * pdf_mul(float *dst, float* pdf1, float* pdf2)
{
    int k;
    float v;
    float norm = 0;
    for (k = 0; k < QPC_Q; k++)
    {
        v = pdf1[k] * pdf2[k];
        dst[k] = v;
        norm += v;
    }
    // if norm of the result is not positive
    // return in dst a uniform distribution
    if (norm <= 0)
        memcpy(dst, pdf_uniform(), QPC_Q * sizeof(float));
    else
    {
        norm = 1.0f / norm;
        for (k = 0; k < QPC_Q; k++)
            dst[k] = dst[k] * norm;
    }


    return dst;
}
static void pdfarray_mul(float* dstarray, float* pdf1array, float* pdf2array, int numrows)
{
    int k;

    // products between rows of pdfs

    for (k = 0; k < numrows; k++)
    {
        pdf_mul(dstarray, pdf1array, pdf2array);
        dstarray += QPC_Q;
        pdf1array += QPC_Q;
        pdf2array += QPC_Q;
    }
}
static float* pdf_convhard(float* dst, const float* pdf, unsigned char hd)
{
    // convolution between a pdf and a hard-decision feedback

    int k;
    for (k=0;k<QPC_Q;k++)
        dst[k] = pdf[k^hd];

    return dst;
}
static void pdfarray_convhard(float* dstarray, const float* pdfarray, const unsigned char *hdarray, int numrows)
{
    int k;

    // hard convolutions between rows

    for (k = 0; k < numrows; k++)
    {
        pdf_convhard(dstarray, pdfarray, hdarray[k]);
        dstarray += QPC_Q;
        pdfarray += QPC_Q;
    }
}
static unsigned char pdf_max(const float* pdf)
{
    int k;

    unsigned char imax = 0;
    float pdfmax = pdf[0];

    for (k=1;k<QPC_Q;k++)
        if (pdf[k] > pdfmax)
        {
            pdfmax = pdf[k];
            imax = k;
        }

    return imax;
}
// local stack functions ---------------------------------------
static float _qpc_stack[QPC_N * QPC_Q * 2];
static float* _qpc_stack_base = _qpc_stack;

static float* _qpc_stack_push(int numfloats)
{
    float* addr = _qpc_stack_base;
    _qpc_stack_base += numfloats;
    return addr;
}
static void _qpc_stack_pop(int numfloats)
{
    _qpc_stack_base -= numfloats;
}
// qpc polar decoder (internal use )--------------------------------------------------
void _qpc_decode(unsigned char* xdec, unsigned char* ydec, const float* py, const unsigned char* f, const unsigned char* fsize, const int numrows)
{

    if (numrows == 1)
    {
        if (fsize[0] == 0)
        {
            // dbgprintf_vector_float("py", py, QPC_Q);
            // frozen symbol
            xdec[0] = pdf_max(py);
            ydec[0] = f[0];
        }
        else
        {
            // fsize = 1 => information symbol
            xdec[0] = pdf_max(py);
            ydec[0] = xdec[0];
        }
    }
    else
    {
        int k;
        int nextrows = numrows >> 1;
        int size = nextrows << QPC_LOG2Q;

        // upper block variables
        unsigned char* xdech = xdec + nextrows;
        unsigned char* ydech = ydec + nextrows;
        const unsigned char* fh = f + nextrows;
        const unsigned char* fsizeh = fsize + nextrows;

        // Step 1.
        // stack and init variables used in the recursion

        float* pyl = _qpc_stack_push(size);
        memcpy(pyl, py, size * sizeof(float));

        float* pyh = _qpc_stack_push(size);
        memcpy(pyh, py + size, size * sizeof(float));

        // Step 2. Recursion on upper block
        // Forward pdf convolutions for the upper block
        // (place in the lower part of py the convolution of lower and higher blocks)

        //float* pyh = py + size;
        //pdfarray_conv(py, pyl, pyh, nextrows); // convolution overwriting the lower block of py which is not needed

        pdfarray_conv(pyh, pyl, pyh, nextrows);
        _qpc_decode(xdech, ydech, pyh, fh, fsizeh, nextrows);

        // Step 3. compute pdfs in the lower block
        pdfarray_convhard(pyh, py+size, ydech,nextrows); // dst ptr must be different form src ptr
        pdfarray_mul(pyl, pyl, pyh, nextrows);
        // we don't need pyh anymore
        _qpc_stack_pop(size);

        // Step 4. Recursion on the lower block
        _qpc_decode(xdec, ydec, pyl, f, fsize, nextrows);
        // we don't need pyl anymore
        _qpc_stack_pop(size);

        // Step 5. Update backward results
        // xdec is already ok, we need just to update ydech
        for (k = 0; k < nextrows; k++)
            ydech[k] = ydech[k] ^ ydec[k];
    }
}
void qpc_decode(unsigned char* xdec, unsigned char* ydec, float* py)
{
    int k;
    unsigned char x[QPC_N];
    // set the first py row with know frozen (punctured) symbol
    if (qpccoderx.np < qpccoderx.n)
    {
        // assume that we punctured only the first output symbol
        memset(py, 0, QPC_Q * sizeof(float));
        py[qpccoderx.f[0]] = 1.0f;
    }
    // decode
    _qpc_decode(x, ydec, py, qpccoderx.f, qpccoderx.fsize, QPC_N);
    // demap information symbols
    for (k = 0; k < QPC_K; k++)
    {
        xdec[k] = x[qpccoderx.xpos[k]];
        //qDebug()<<xdec[k];
    }
}
///////////////////////////////////// qpc_decode
/*#include <QApplication>
#include "../HvMsPlayer/msplayerhv.h"
MsPlayerHV *TMsPlayerHV;*/ 
DecoderSFox::DecoderSFox()
{	
	/*TMsPlayerHV = new MsPlayerHV(QCoreApplication::applicationDirPath());
	TMsPlayerHV->SetModeForWavSaves(11);*/		
    TGenSFox = new GenSFox(true);//f_dec_gen = dec=true gen=false
    TGenFt8 = new GenFt8(true);//f_dec_gen = dec=true gen=false
    //gen_pulse_gfsk_(pulse_ft4_rx,864.0,1.0,576);  //576
    pomAll.initPomAll();
    nftx = 1200.0;
    s_time = "000000";
    pi=4.0*atan(1.0);
    twopi=8.0*atan(1.0);
    gen_pulse_gfsk_(pulse_ft8_rx,2880.0,2.0,1920);
    for (int i= 0; i < 65536; ++i)
    {//do i=0,NTAB-1
        double phi0=(double)i*twopi/65536.0; //16777216.0  65536.0
        ctab8_[i]=(cos(phi0)+sin(phi0)*I);
    }
    ctab8_[65536]=0.0+0.0*I;
    ctab8_[65537]=0.0+0.0*I;
    //f_otp_sfox_msg = false;   
}
DecoderSFox::~DecoderSFox()
{}
/*void DecoderSFox::SetSFoxRxMsg(bool f)
{
	f_otp_sfox_msg = f; //qDebug()<<"RX f_otp_sfox_msg="<<f_otp_sfox_msg;
}*/
void DecoderSFox::SetTxFreq(double fr)
{
    nftx = fr;
}
void DecoderSFox::SetStDecode(QString time,int /*mousebutton*/,bool /*ffopen*/)
{
    s_time = time;
    /*s_mousebutton = mousebutton;//mousebutton Left=1, Right=3 fullfile=0 rtd=2
    s_fopen65 = ffopen; //qDebug()<<s_fopen65;//2.72 for ap pileup*/
}
void DecoderSFox::sfox_ana(double *iwave,int npts,double complex *c0)
{
    int nfft1=npts;//=180000
    int nfft2=nfft1;// nfft2=nfft1  c2c=1440000max
    //double fac=(2.0/(32767.0*(double)nfft1))*0.01;// hv correction duble   32767.0  8388607.0
    double fac=(1.0/300.0)*0.01;
    //double x[190000];
    for (int i=0; i<npts ; ++i) c0[i]=fac*iwave[i];//c0(0:npts-1)=fac*iwave(1:npts)
    //f2a.four2a_d2c(c0,x,nfft1,-1,0);
    /*double xxx = 0.0;
    for (int i=0; i<npts ; ++i)
    {
    	if (fac*iwave[i]>xxx) xxx=fac*iwave[i];   	
    }   qDebug()<<"max="<<xxx;*/
    f2a.four2a_c2c(c0,nfft1,-1,1);//four2a(c0,nfft1,1,-1,1) //!Forward c2c FFT
    for (int i=nfft2/2; i<nfft2; ++i) c0[i]=0.0+0.0*I;//c0(nfft2/2+1:nfft2-1)=0.
    c0[0]=0.5*c0[0];// //c0(0)=0.5*c0(0) (0.5+0.5*I)
    f2a.four2a_c2c(c0,nfft2,1,1); //call four2a(c0,nfft2,1,1,1)              //!Inverse c2c FFT; c0 is analytic sig
}
static const int icos7_2[7] =
    {
        3,1,4,0,6,5,2
    };
void DecoderSFox::baseline(double *s,int nfa,int nfb,double *sbase)
{
    /*! Fit baseline to spectrum (for FT8)
    ! Input:  s(npts)         Linear scale in power
    ! Output: sbase(npts)    Baseline
    */
    double df=12000.0/3840.0;                    //!3.125 Hz
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
    for (int i = 0; i<1920; ++i) sbase[i]=0.0;
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
void DecoderSFox::get_spectrum_baseline(double *dd,int nfa,int nfb,double *sbase)
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
    static bool first_ft8sbl = true;

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
        f2a.four2a_d2c(cx,x,NFFT1,-1,0);  //call four2a(x,NFFT1,1,-1,0)              //!r2c FFT
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
void DecoderSFox::sync8(double *dd,double nfa,double nfb,double syncmin,double nfqso,
                        double s_[402][1970],double candidate[2][620],int &ncand,double *sbase)
{
    const int NSPS=1920;
    int NSTEP=NSPS/4;//=480
    const int NFFT1=2*NSPS;
    int NMAX=15*12000.0;
    int NHSYM=NMAX/NSTEP-3;//372
    const int NH1=NFFT1/2;       //NH1=1920
    const int max_c0 = 800;//2.69 old=400;//2.2.0=500 260r5=1000
    const int max_c_ = 600;//2.69 old=249 max 254 260r5=600

    //! Search over +/- 1.5s relative to 0.5s TX start time.
    //parameter (JZ=38)
    //old 72/2 int JZ=38;
    //! Search over +/- 2.5s relative to 0.5s TX start time.
    //parameter (JZ=62) new
    const int JZ=62;//62;

    //! Compute symbol spectra, stepping by NSTEP steps.
    //double savg=0.0;
    double tstep=(double)NSTEP/12000.0;
    double df=12000.0/(double)NFFT1;                            //!3.125 Hz
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
        f2a.four2a_d2c(cx,x,NFFT1,-1,0);  //call four2a(x,NFFT1,1,-1,0)              //!r2c FFT
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
    //if (decid>0) corrt = 17;       //2.72 5=16Hz 17=53Hz 18=56Hz
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
                int m=j+jstrt+nssy*n; //k=j+jstrt+nssy*n //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
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
        int j0=(pomAll.maxloc_da_beg_to_end(sync2d[i],10,32+10)-offset_sync2d);//hv=Search over +/-0.6s = +-16 //260r5= +-10
        jpeak[i]=j0;
        red[i]=sync2d[i][j0+offset_sync2d];
        j0=(pomAll.maxloc_da_beg_to_end(sync2d[i],10,124+10)-offset_sync2d);//Search over +/-2.5s = +-62
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
            candidate0[1][k]=(double)jpeak[n]*tstep;//(jpeak(n)-0.5)  2.34 tested jpeak[n]-1 to jpeak[n] ///candidate0(2,k)=(jpeak(n)-1)*tstep
            candidate0[2][k]=red[n];  //if (n>1500) qDebug()<<"n="<<n;
            k++;
        }
        if (k>=max_c0) break;
        if (abs(jpeak2[n]-jpeak[n])==0) continue;//if(abs(jpeak2(n)-jpeak(n)).eq.0) cycle
        //n=ia + indx2[(iz-1)-i];
        if (red2[n]>=syncmin)  //exit if(red(n).lt.syncmin.or.isnan(red(n)).or.k.eq.MAXPRECAND) exit
        {
            candidate0[0][k]=(double)n*df;
            candidate0[1][k]=(double)jpeak2[n]*tstep;//(jpeak(n)-0.5)  2.34 tested jpeak[n]-1 to jpeak[n] ///candidate0(2,k)=(jpeak(n)-1)*tstep
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
#define F0_DB 8.49*6.25  //2.70
#define F1_DB 0.91*6.25  //2.70
void DecoderSFox::ft8_downsample(double *dd,bool &newdat,double f0,double complex *c1)
{
    const int NFFT2=3200;
    const int c_c1=NFFT2;//1.44 tuk be6e problema be6e->(NFFT2-1) triabva da e->(NFFT2-0)
    const int NMAX=15*12000.0;  //=180000    !192000/60 = 3200
    const int NFFT1=192000;
    static bool first_ft8_downsample = true;

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
        f2a.four2a_d2c(cx_ft8,x,NFFT1,-1,0);//call four2a(cx,NFFT1,1,-1,0)             //!r2c FFT to freq domain
        newdat=false;
        delete [] x;
    }

    double df=12000.0/(double)NFFT1;
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
    f2a.four2a_c2c(c1,NFFT2,1,1); //call four2a(c1,NFFT2,1,1,1)            //!c2c FFT back to time domain

    double fac=1.0/sqrt((double)NFFT1*(double)NFFT2);
    for (int i = 0; i < c_c1; ++i) c1[i]*=fac;
}
void DecoderSFox::sync8d(double complex *cd0,int i0,double complex *ctwk,int itwk,double &sync)
{
    //p(z1)=real(z1)**2 + aimag(z1)**2          !Statement function for power*/
    int NP2=2812;
    //int NDOWN=60;
    //double complex z1,z2,z3;
    double complex csync2[36];
    static bool first_sync8d = true;
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
    } //qDebug()<<"sync=========================="<<sync;
}
void DecoderSFox::gen_ft8cwaveRx(int *i4tone,double f_tx,double complex *cwave)
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
#define K_SUB 1.9962 //2.70
double DecoderSFox::BestIdtft8(double *dd,double f0,double dt,double idt,double complex *cref,
                               double complex *cfilt,double complex *cw_subs,double *endcorr,
                               double *xdd,double complex *cx)
{
    double sqq=0.0;
    const int NFRAME=1920*79;//=151680
    const int NMAX=180000;
    const int NFFT=180000;
    const int NFILT=4000;
    int nstart=dt*12000.0+1.0+idt;//0 +1.0  -1920

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
    f2a.four2a_c2c(cfilt,NFFT,-1,1);
    for (int i = 0; i < NFFT; ++i) cfilt[i]*=cw_subs[i];
    f2a.four2a_c2c(cfilt,NFFT,1,1);
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

    f2a.four2a_d2c(cx,xdd,NFFT,-1,0);   //!Forward FFT, r2c
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
    } //qDebug()<<sqq;
    return sqq;
}
void DecoderSFox::subtractft8(double *dd,int *itone,double f0,double dt,bool lrefinedt)
{
    static bool first_subsft8 = true;
    const int NFRAME=1920*79;//new=151680
    const int NMAX=180000;//15*DEC_SAMPLE_RATE;//=180000
    const int NFFT=180000;//NMAX; need to be number if no crash  180000
    const int NFILT=4000;//3700;//old NFILT=1400; 4000
    int offset_w = NFILT/2+25;
    //int nstart=dt*DEC_SAMPLE_RATE+1.0;//0 +1.0  -1920

    double complex *cref = new double complex[153681];//151681+ramp      double complex cref[NFRAME+100];
    double complex *cfilt= new double complex[180300];//NMAX+100
    //double dd66[180192]= {0.0};// __attribute__((aligned(16))) = {0.0};  //32 =190,192,182,206,174
    //pomAll.zero_double_beg_end(dd66,0,180005);
    //double *dd66 = new double[180100];  // <- slow w10

    pomAll.zero_double_comp_beg_end(cfilt,0,(NMAX+25));
    //pomAll.zero_double_comp_beg_end(cref,0,NFRAME+25);
    gen_ft8cwaveRx(itone,f0,cref);

    if (first_subsft8) //then //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    {
        //! Create and normalize the filter
        double window[NFILT+250] __attribute__((aligned(16)));//double *window= new double[NFILT+250];//
        double fac=1.0/double(NFFT);
        double sumw=0.0;
        for (int j = -NFILT/2; j < NFILT/2; ++j)
        {//do j=-NFILT/2,NFILT/2
            window[j+offset_w]=cos(pi*(double)j/(double)NFILT)*cos(pi*(double)j/(double)NFILT);
            sumw+=window[j+offset_w];
        }
        pomAll.zero_double_comp_beg_end(cw_subsft8,0,NMAX+25);
        if (sumw<=0.0) sumw=0.01;// no devide by zero
        for (int i = 0; i < NFILT+1; ++i) cw_subsft8[i]=window[i+offset_w-NFILT/2]/sumw;//cw(1:NFILT+1)=window/sum
        pomAll.cshift1(cw_subsft8,NMAX,(NFILT/2+1));    //cw=cshift(cw,NFILT/2+1);
        f2a.four2a_c2c(cw_subsft8,NFFT,-1,1);//four2a(cw,nfft,1,-1,1)
        for (int i = 0; i < NMAX; ++i) cw_subsft8[i]*=fac;
        for (int j = 0; j < NFILT/2+1; ++j)
        {// do j=1,NFILT/2+1
            // endcorrection(j)=1.0/(1.0-sum(window(j-1:NFILT/2))/sumw)
            double dell;
            double summ = 0.0;
            for (int z = j; z < NFILT/2; ++z)
                summ+=window[z+offset_w];//hv error  offset_w
            dell = 1.0-summ/sumw;
            if (dell<=0.0) dell = 0.0001;
            endcorrectionft8[j]=1.0/dell;
        }
        first_subsft8=false; //qDebug()<<"oooooooooo";
        //delete [] window;
    }

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
        int nstart=dt*12000.0+1.0+idt;//0 +1.0  -1920
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
        f2a.four2a_c2c(cfilt,NFFT,-1,1);
        for (int i = 0; i < NFFT; ++i) cfilt[i]*=cw_subsft8[i];
        f2a.four2a_c2c(cfilt,NFFT,1,1);
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
void DecoderSFox::sfox_remove_ft8(double *dd)//,int npts
{
    static bool first = true;
    static int NDOWN = 60;
    double candidate[2][620];//2.69 old=255 start from here;
    double (*s_)[1970] = new double[402][1970];//2.39 start from here;  2.66
    double sbase[1970];//2.39 start from here;
    int ncand = 0;
    double complex cd0[3350];
    pomAll.zero_double_comp_beg_end(cd0,0,3300);
    double complex ctwk[32+5];
    double delfbest=0.0;
    const int NS=21;//
    const int NN=NS+58;//                  !Total channel symbols (79)
    double complex csymb[40];//32+8
    const int NP2=2812;
    double complex cs_[NN][8]; 				//complex cs(0:7,NN)
    double s8_[NN][8];
    double s2[512];
    const int graymap[8] =
        {
            0,1,3,2,5,6,4,7
        };
    double bmeta[174];
    //double bmetb[174];
    //double bmetc[174];//stop tbt...
    //double bmetd[174];//stop tbt...
    double llra[174];
    //double llrb[174];
    //double llrz[174];
    bool apmask[174];
    bool cw[194];//174
    bool message91[140];

    if (first)
    {
        for (int i = 0; i < 3; ++i)
        {
            for (int j = 0; j < 8; ++j)
            {
                if ((j & (int)pow(2,i))!=0) one[i][j]=true;
                else one[i][j]=false;
            }
        }
        first=false;
    }

    double fs2=12000.0/(double)NDOWN;
    double dt2=1.0/fs2;
    int nfa=500;
    int nfb=2500;
    double syncmin=1.5;
    double nfqso=750;
    sync8(dd,nfa,nfb,syncmin,nfqso,s_,candidate,ncand,sbase);//sync8(dd,nfa,nfb,syncmin,nfqso,candidate,ncand,sbase)
    bool newdat=true;
    for (int icand = 0; icand < ncand; ++icand)
    {
        double f1 =candidate[0][icand];
        double xdt=candidate[1][icand];
        //double xbase=pow(10.0,(0.1*(sbase[int(f1/3.125)]-40.0)));
        ft8_downsample(dd,newdat,f1,cd0);
        newdat=false;
        int i0=int((xdt+0.5)*fs2);//i0=nint((xdt+0.5)*fs2)
        double smax=0.0;
        double sync=0.0;
        int ibest=0;
        for (int idt = i0-10; idt <= i0+10; ++idt)//2.39 ????
        {//2.39 do idt=i0-10,i0+10  do idt=i0-8,i0+8                         //!Search over +/- one quarter symbol
            sync8d(cd0,idt,ctwk,0,sync); //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
            if (sync>smax)
            {
                smax=sync; //qDebug()<<idt<<sync;
                ibest=idt;
            }
        }

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

        /*for (int i = 0; i < 5; ++i) a[i]=0.0;//-delfbest;//0.0; ???
        a[0]=-delfbest;
        pomFt.twkfreq1(cd0,NP2,fs2,a,cd0);*/

        f1=f1+delfbest;                           //!Improved estimate of DF

        ft8_downsample(dd,newdat,f1,cd0);   //!Mix f1 to baseband and downsample
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
        //if (ap7) xdt=(double)ibest*dt2 - 0.5;
        xdt=(double)ibest*dt2;//hv tested no-1 (double)(ibest-1)*dt2; xdt=(ibest-1)*dt2
        sync=smax;

        for (int k = 0; k < NN; ++k)
        {
            int i1=ibest+k*32;//i1=ibest+(k-1)*32
            pomAll.zero_double_comp_beg_end(csymb,0,34);
            if ( i1>=0 && (i1+32) < NP2 )
            {
                for (int z = 0; z < 32; ++z)
                    csymb[z]=cd0[i1+z];
            }
            f2a.four2a_c2c(csymb,32,-1,1);
            for (int z = 0; z < 8; ++z)
            {
                cs_[k][z]=csymb[z]*0.001;//1000.0;//cs(0:7,k)=csymb(1:8)/1e3
                s8_[k][z]=cabs(csymb[z]);//s8(0:7,k)=abs(csymb(1:8))
                //sss.append(QString("%1 ").arg(cabs(csymb[z]),0,'f',1));
            }//sss.append("\n");
        }

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
        int nsync=0;
        //int nbadcrc = 0;
        nsync=is1+is2+is3;
        if (nsync <= 6) //! bail out if(nsync .le. 6) then ! bail out
        {
            //nbadcrc=1;
            //qDebug()<<"ft8b NONONO"<<ncand;
            continue;//return false;
        }

        int nsym = 1;
        //for (int nsym = 1; nsym <= 3; ++nsym)
        //{//do nsym=1,3
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
                    //int i1=i/64;
                    //int i2=(i & 63)/8;
                    int i3=(i & 7);
                    //if (nsym==1)   //complex cs_[NN][8]; //complex cs(0:7,NN)
                    s2[i]=cabs(cs_[ks-1][graymap[i3]]);//s2(i)=abs(cs(graymap(i3),ks))
                    /*else if (nsym==2)
                        s2[i]=cabs(cs_[ks-1][graymap[i2]]+cs_[ks][graymap[i3]]); //s2(i)=abs(cs(graymap(i2),ks)+cs(graymap(i3),ks+1))
                    else if (nsym==3)
                        s2[i]=cabs(cs_[ks-1][graymap[i1]]+cs_[ks][graymap[i2]]+cs_[ks+1][graymap[i3]]);//s2(i)=abs(cs(graymap(i1),ks)+cs(graymap(i2),ks+1)+cs(graymap(i3),ks+2))
                        */
                    //else
                    //qDebug()<<"Error - nsym must be 1, 2, or 3.";
                }
                //for (int i = 0; i < nt; ++i)
                //s2l[i]=log(s2[i]+1e-32);//??? s2l(0:nt-1)=log(s2(0:nt-1)+1e-32)
                int i32=(k-1)*3+(ihalf-1)*87;  //??? i32=1+(k-1)*3+(ihalf-1)*87
                int ibmax=2;
                //if (nsym==1) ibmax=2;
                //if (nsym==2) ibmax=5;
                //if (nsym==3) ibmax=8;
                for (int ib = 0; ib <= ibmax; ++ib)
                {//do ib=0,ibmax
                    //bm=maxval(s2(0:nt-1),one(0:nt-1,ibmax-ib)) - maxval(s2(0:nt-1),.not.one(0:nt-1,ibmax-ib))
                    double max1v=0.0;
                    for (int zz = 0; zz < nt; ++zz)
                    {
                        if (one[ibmax-ib][zz]==true)
                        {
                            double tmax1v=s2[zz];
                            if (tmax1v>max1v) max1v=tmax1v;
                        }
                    }
                    double max2v=0.0;
                    for (int zz = 0; zz < nt; ++zz)
                    {
                        if (one[ibmax-ib][zz]==false)
                        {
                            double tmax2v=s2[zz];
                            if (tmax2v>max2v) max2v=tmax2v;
                        }
                    }
                    double bm=max1v-max2v; //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
                    //if (max1v==0.0 || max2v==0.0) qDebug()<<max1v<<max2v;
                    if (i32+ib>173) continue;//cycle //if(i32+ib .gt.174) cycle

                    //if (nsym==1)
                    {
                        bmeta[i32+ib]=bm; //qDebug()<<"0-173"<<i32+ib;
                        //den=max(maxval(s2(0:nt-1),one(0:nt-1,ibmax-ib)), maxval(s2(0:nt-1),.not.one(0:nt-1,ibmax-ib)))
                        /*double den = max1v;
                        if (max2v > max1v) den = max2v;*/

                        /*double cm = 0.0;
                        if (den>0.0) //if(den.gt.0.0) then
                            cm=bm/den;
                        else
                            cm=0.0;*/ //! erase it
                        //bmetd[i32+ib]=cm;
                    }
                    //else if (nsym==2) bmetb[i32+ib]=bm;//bmetb(i32+ib)=bm
                    //else if (nsym==3) bmetc[i32+ib]=bm;//bmetc(i32+ib)=bm
                }
            }
        }
        //}
        pomFt.normalizebmet(bmeta,174);

        double scalefac = 2.83;//scalefac=2.83 double ss=0.85;//hv tested->85-86    0.84;//0.84

        for (int z = 0; z < 174; ++z)
        {
            llra[z]=scalefac*bmeta[z];
        }

        for (int z = 0; z < 174; ++z)
        {
            apmask[z]=0;
            cw[z]=0;
            if (z<120) message91[z]=0;
        }

        double dmin=0.0;
        int norder=2;
        int maxosd=-1;
        int nharderrors=-1;
        pomFt.decode174_91(llra,maxosd,norder,apmask,message91,cw,nharderrors,dmin);

        if (nharderrors<0/* || nharderrors>36*/) continue;

        int c_cw = 0;
        for (int z = 0; z < 174; z++)//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
        {
            if (cw[z]==0)// 3*ND=174
                c_cw++;
        }
        if (c_cw==174) continue;

        int i3,n3;
        n3=4*message91[71] + 2*message91[72] + message91[73];//??? check need
        i3=4*message91[74] + 2*message91[75] + message91[76];//??? check need
        if (i3>5 || (i3==0 && n3>6)) continue; //2.39 EU VHF Contest
        if (i3==0 && n3==2) continue; //2.42 222 ignore old EU VHF Contest

        bool unpk77_success = false;
        TGenFt8->unpack77(message91,unpk77_success);  //qDebug()<<message<<unpk77_success;
        //qDebug()<<"100 Unpack=77--->"<<message<<i3<<n3<<unpk77_success<<f1;
        if (!unpk77_success) continue;

        int i4tone[120];
        TGenFt8->make_c77_i4tone(message91,i4tone);
        subtractft8(dd,i4tone,f1,xdt,true);
    }
    delete [] s_;
}
#define NQ 128
#define NN 127
#define NS 24
#define NSPS 1024//???
#define NDS 151
#define NMAX (15*12000)
const int isync[151] = //151=24
    {
        1,2,4,7,11,16,22,29,37,39,42,43,45,48,52,57,63,70,78,80,83,84,86,89
    };
void DecoderSFox::sfox_remove_tone(double complex *c0,double fsync)
{
    static bool first = true;
    const int NFILT = 8000;
    int offset_w = NFILT/2+25;
    double complex *cfilt= new double complex[190000];//double complex cfilt[NMAX+2048];
    double s[(NMAX/4)+1024];//=45000
    double complex *cref = new double complex[190000];//double complex cref[NMAX+2048];
    if (first)
    {
        double window[NFILT+250] __attribute__((aligned(16)));
        double fac=1.0/double(NMAX);
        double sumw=0.0;
        for (int j = -NFILT/2; j < NFILT/2; ++j)
        {//do j=-NFILT/2,NFILT/2
            window[j+offset_w]=cos(pi*(double)j/(double)NFILT)*cos(pi*(double)j/(double)NFILT);
            sumw+=window[j+offset_w];
        }
        pomAll.zero_double_comp_beg_end(cwindow,0,NMAX+25);
        if (sumw<=0.0) sumw=0.01;
        for (int i = 0; i < NFILT+1; ++i) cwindow[i]=window[i+offset_w-NFILT/2]/sumw;//cwindow(1:NFILT+1)=window/sumw
        pomAll.cshift1(cwindow,NMAX,(NFILT/2+1));//cwindow=cshift(cwindow,NFILT/2+1)
        f2a.four2a_c2c(cwindow,NMAX,-1,1);//call four2a(cwindow,NMAX,1,-1,1)
        for (int i = 0; i < NMAX; ++i) cwindow[i]*=fac;//cwindow=cwindow*fac        ! frequency domain smoothing filter
        first=false;
    }
    double fsample=12000.0;
    double baud=fsample/1024.0;
    double df=fsample/(double)NMAX;
    double fac=1.0/(double)NMAX;
    double nbaud=(baud/df);
    for (int it = 0; it < 1; ++it)//! Remove 1 tone, if present
    {//do it=1,1
        for (int i = 0; i < NMAX; ++i) cfilt[i]=fac*c0[i];//cfilt=fac*c0
        f2a.four2a_c2c(cfilt,NMAX,-1,1);//call four2a(cfilt,NMAX,1,-1,1)   ! fourier transform of input data
        int iz=NMAX/4;
        for (int i = 0; i < iz; ++i) s[i]=pomAll.ps_hv(cfilt[i]);  //do i=1,iz  s(i)=real(cfilt(i))**2 + aimag(cfilt(i))**2
        int ia=(int)((fsync-50.0)/df);
        int ib=(int)((fsync+1500.0+50.0)/df); //qDebug()<<fsync<<((3200)/df)<<ia<<ib;
        if (ia<0) ia=0;
        if (ib>45000) ib=45000;//=3000Hz
        int i0 = pomAll.maxloc_da_beg_to_end(s,ia,ib);//ipk=maxloc(s(ia:ib))
        //i0=ipk(1) + ia - 1
        ia=(int)((double)i0-nbaud);
        ib=(int)((double)i0+nbaud);
        if (ia<0) ia=0;
        if (ib>45000) ib=45000;//=3000Hz
        double s0=0.0;
        double s1=0.0;
        double s2=0.0;
        for (int i=ia; i <=ib; ++i)
        {//do i=ia,ib
            s0+=s[i];//s0=s0+s(i)
            s1+=(double)(i-i0)*s[i];//s1=s1+(i-i0)*s(i)
        }
        if (s0==0.0) s0=0.000001;
        double delta=s1/s0;
        i0=(int)((double)i0+delta);
        double f2=(double)i0*df;
        ia=(int)((double)i0-nbaud);
        ib=(int)((double)i0+nbaud);
        if (ia<0) ia=0;
        if (ib>45000) ib=45000;//=3000Hz
        for (int i = ia; i < ib; ++i) s2+=(s[i]*((i-i0)*(i-i0)));//do i=ia,ib  s2=s2 + s(i)*(i-i0)**2
        double sigma=sqrt(s2/s0)*df;//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
        //!      write(*,*) 'frequency, spectral width ',f2,sigma
        if (sigma > 2.5) break; //qDebug()<<"Remove="<<sigma<<f2;//if (sigma > 2.5) break; if(sigma .gt. 2.5) exit
        //!      write(*,*) 'remove_tone - frequency: ',f2
        double dt=1.0/fsample;
        for (int i = 0; i < NMAX; ++i)
        {//do i=1, NMAX
            double arg=2.0*pi*f2*(double)i*dt;
            cref[i]=cos(arg)+sin(arg)*I;//cref(i)=cmplx(cos(arg),sin(arg))
        }
        for (int i = 0; i < NMAX; ++i) cfilt[i]=c0[i]*conj(cref[i]);//cfilt=c0*conjg(cref)   ! baseband to be filtered
        f2a.four2a_c2c(cfilt,NMAX,-1,1);//call four2a(cfilt,NMAX,1,-1,1)
        for (int i = 0; i < NMAX; ++i) cfilt[i]=cfilt[i]*cwindow[i];//cfilt=cfilt*cwindow
        f2a.four2a_c2c(cfilt,NMAX,1,1);//call four2a(cfilt,NMAX,1,1,1)
        int nframe=50*3456;
        for (int i = 0; i < nframe; ++i)
        {//do i=1,nframe
            cref[i]=cfilt[i]*cref[i];//cref(i)=cfilt(i)*cref(i)
            c0[i]=c0[i]-cref[i];//c0(i)=c0(i)-cref(i)
        }
    }
    delete [] cfilt;
    delete [] cref;
}
void DecoderSFox::qpc_sync(double complex *crcvd0,double fsample,double fsync,double fa0,double fb0,
                           double *f2,double *t2,double *snrsync)
{
    const int NDOWN=16; //qDebug()<<"ssss";
    //const int N9SEC=(9.9*12000.0);
    const int N9SEC=(9.2*12000.0);//=108000
    //const int N9SEC=(9.9*12000.0);//=108000
    const int NZ=N9SEC/NDOWN;//NZ=6750
    //const int SNZ=N9SEC/4;//=27000
    double complex *c00 = new double complex[N9SEC+2048];//double complex c00[N9SEC+2048];//c0(0:N9SEC-1)
    double *s = new double[N9SEC/4+1024];//double s[28000];//N9SEC/4=27000
    double baud=(12000.0/1024.0);//= 11.71875
    //baud=13.8;//(12000.0/1024.0);//*1.2;//23;//*2.0;
    double df2=fsample/(double)N9SEC;
    double fac=1.0/(double)N9SEC;
    double complex *c1 = new double complex[NZ+1024];//double complex c1[NZ+1024];
    double complex *c1sum = new double complex[NZ+1024];//double complex c1sum[NZ+1024];
    const int offsetp = 1125+50;
    double p[2250+100];//p(-1125:1125)=2250
    double nbaud = baud/df2;

    for (int i=0; i<N9SEC; ++i) c00[i]=fac*crcvd0[i];//c0=fac*crcvd0(1:N9SEC)
    /*int smpl = 24000;
    double st_smpl = 1.0/(double)smpl;
    double kv_smpl = 0.0;  
    for (int i=0; i<smpl; ++i)
    {
    	c00[i]*=kv_smpl;
     	kv_smpl += st_smpl;
    }*/
    //for (int i=12000.0*8.9; i<N9SEC; ++i) c00[i]=0.0+0.0*I;
    f2a.four2a_c2c(c00,N9SEC,-1,1);//call four2a(c0,N9SEC,1,-1,1)                !Forward c2c FFT
    int iz=N9SEC/4;//=27000
    for (int i=0; i<iz; ++i) s[i] = pomAll.ps_hv(c00[i]); //do i=1,iz   s(i)=real(c0(i))**2 + aimag(c0(i))**2
    for (int i=0; i<4; ++i) pomAll.smo121(s,0,iz);//do i=1,4 call smo121(s,iz) !Smooth the spectrum a bit
    double fa = fsync-60.0;
    double fb = fsync+60.0;
    double knb = 1.0;

    for (int ccand=0; ccand<3; ++ccand)
    {
        if (ccand==1)
        {
            fa = 700.0;
            fb = 800.0; 
            knb = 2.0;
        }
        else if (ccand==2)
        {
            fa = fa0;
            fb = fb0;   
            knb = 1.5;//2.0
        }
        int ia=(int)(fa/df2);//ia=nint((fsync-ftol)/df2)
        int ib=(int)(fb/df2);//ib=nint((fsync+ftol)/df2)
        if (ia<0) ia=0;
        if (ib>27000) ib=27000; //qDebug()<<"0="<<ia<<ib;//tbd... 27000=3000Hz
        int ipk = pomAll.maxloc_da_beg_to_end(s,ia,ib);//ipk=maxloc(s(ia:ib))
        //int ipk = pomAll.maxloc_da_end_to_beg(s,ia,ib);
        int i0=ipk;//i0=6750; ipk-0 //i0=ipk(1) + ia - 1
        f2[ccand]=df2*(double)i0-750.0;//f2=df2*i0-750.0 ! f2 is the offset from nominal 750 Hz.
        ia=(int)((double)i0-nbaud);
        ib=(int)((double)i0+nbaud); //qDebug()<<"1========"<<ia<<ib<<i0<<f2<<nbaud;
        if (ia<0) ia=0;
        if (ib>27000) ib=27000;
        double s1=0.0;
        double s0=0.0;
        for (int i=ia; i<=ib; ++i)
        {//do i=ia,ib
            s0+=s[i];//s0=s0+s(i)
            s1+=(double)(i-i0)*s[i];//s1=s1+(i-i0)*s(i)
        }
        if (s0==0.0) s0=0.0000001;
        double delta=s1/s0; //delta=0;
        i0=(int)((double)i0+delta);
        f2[ccand]=((double)i0*df2)-750.0;

        for (int i=0; i<NZ+10; ++i) c1[i]=0.0+0.0*I;//NZ=6750
        ia=(int)((double)i0-(nbaud*knb));//HV*2.0
        ib=(int)((double)i0+(nbaud*knb));//HV*2.0
        if (ia<0) ia=0;
        if (ib>N9SEC) ib=N9SEC;//if (ib>27000) ib=27000;//ib=27000;//108000
        for (int i=ia; i<=ib; ++i)//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
        {//do i=ia,ib
            int j=i-i0; //j=i-i0
            if (j>=0) c1[j]=c00[i];//if(j.ge.0) c1(j)=c0(i)
            if (j<0 ) c1[j+NZ]=c00[i];//(j.lt.0) c1(j+NZ)=c0(i)
        }
        f2a.four2a_c2c(c1,NZ,1,1);//call four2a(c1,NZ,1,1,1) !Reverse c2c FFT: back to time domain        
        c1sum[0]=c1[0];
        for (int i=NZ; i<NZ+10; ++i) c1[i]=0.0+0.0*I;
        for (int i=1; i<NZ; ++i) c1sum[i]=(c1sum[i-1] + c1[i]); //do i=1,NZ-1 c1sum(i)=c1sum(i-1) + c1(i)
        int nspsd=1024/NDOWN;//=64
        double dt=(double)NDOWN/12000.0;//=0,0013333333333333
        int lagmax=(int)(1.5/dt);//=1125,000000000028
        i0=(int)(0.5*fsample/(double)NDOWN);//!Nominal start time is 0.5 s
        double pmax=0.0;
        int lagpk=0;
        double sp=0.0;
        for (int lag=-lagmax; lag<=lagmax; ++lag)//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
        {//do lag=-lagmax,lagmax
            sp=0.0;
            for (int j=0; j<24; ++j)
            {//do j=1,24
                int i1=i0 + (isync[j]-1)*nspsd + lag;//i1=i0 + (isync(j)-1)*nspsd + lag
                int i2=i1 + nspsd;//i2=i1 + nspsd
                if (i1<0 || i1>NZ-1) continue;//if(i1.lt.0 .or. i1.gt.NZ-1) cycle
                if (i2<0 || i2>NZ-1) continue;//if(i2.lt.0 .or. i2.gt.NZ-1) cycle
                double complex z = c1sum[i2]-c1sum[i1];//z=c1sum(i2)-c1sum(i1)
                sp += pomAll.ps_hv(z);//sp=sp + real(z)**2 + aimag(z)**2
            }
            if (sp>pmax)//if(sp.gt.pmax) then
            {
                pmax=sp;
                lagpk=lag;//+5
            }
            p[lag+offsetp]=sp;//p(lag)=sp //if (lag>60 && lag<80) qDebug()<<lag<<sp*0.001;
        }
        t2[ccand]=(double)lagpk*dt;
        snrsync[ccand]=0.0;
        sp=0.0;
        double sq=0.0;
        int nsum=0;
        double  tsym=1024.0/12000.0;
        for (int lag=-lagmax; lag<=lagmax; ++lag)//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
        {//do lag=-lagmax,lagmax
            double t=(double)(lag-lagpk)*dt;
            if (fabs(t)<tsym) continue;
            nsum++;//nsum=nsum+1
            sp+=p[lag+offsetp];//sp=sp + p(lag)
            sq+=p[lag+offsetp]*p[lag+offsetp];//sq=sq + p(lag)*p(lag)
        }
        double ave=sp/(double)nsum;
        double rms=sqrt((sq/(double)nsum)-ave*ave);//rms=sqrt(sq/nsum-ave*ave)
        if (rms==0.0) rms=0.000001;
        snrsync[ccand]=(pmax-ave)/rms;
        if (ccand==0) snrsync[ccand]+=0.55;//knb=1.0 vs. knb=2.0
        if (ccand==2) snrsync[ccand]+=0.25;
        //printf("F2=%f Delta=%f FBest=%f DT=%f Sync=%f\n",f2[ccand],delta,((1500.0+f2[ccand])-750.0),t2[ccand],snrsync[ccand]);
    }
    delete [] c00;
    delete [] s;
    delete [] c1;
    delete [] c1sum;
}
int DecoderSFox::maxloc_ia_beg_to_end(int*a,int a_beg,int a_end)
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
bool DecoderSFox::any_eq(int *a,int b,int count)
{
    bool res = false;
    for (int i = 0; i < count; ++i)
    {
        if (a[i]==b)
        {
            res = true;
            break;
        }
    }
    return res;
}
void DecoderSFox::sfox_demod(double complex *crcvd,double f,double t,double s2_[161][137],double s3_[137][137])
{
    double complex c[NSPS+100];//double complex *c = new double complex[NSPS+100];//double complex c[NSPS+100];//complex c(0:NSPS-1)//!Work array, one symbol long
    int hist1[NQ+10],hist2[NQ+10];//hist1(0:NQ-1),hist2(0:NQ-1)

    int j0=(int)(12000.0*(t+0.5));
    double df=12000.0/(double)NSPS;
    int i0=(int)((f/df)-(double)NQ/2.0);//i0=nint(f/df)-NQ/2
    int k2=0;
    for (int i =0; i<130; ++i)
    {
        s2_[0][i]=0.0;//s2(:,0)=0. //!The punctured symbol
        s3_[0][i]=0.0;//s3(:,0)=0. //!The punctured symbol
    }

    for (int n =0; n<NDS; ++n)
    {//do n=1,NDS                             !Loop over all symbols
        //int jb=(n+1)*(NSPS-0) + j0;//jb=n*NSPS + j0
        //int ja=jb-NSPS+0;//ja=jb-NSPS+1
        int ja = n*NSPS + j0;
        int jb = ja + NSPS-1;
        k2++;//??? k2=k2+1  //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
        if (ja<0 || jb>NMAX-1) continue;//if(ja.lt.1 .or. jb.gt.NMAX) cycle
        for (int i = 0; i<NSPS; ++i) c[i]=crcvd[i+ja]; //c=crcvd(ja:jb)
        f2a.four2a_c2c(c,NSPS,-1,1);//call four2a(c,NSPS,1,-1,1)//!Compute symbol spectrum
        for (int i = 0; i<NQ; ++i) s2_[k2][i]=pomAll.ps_hv(c[i0+i]);//do i=0,NQ-1  s2(i,k2)=real(c(i0+i))**2 + aimag(c(i0+i))**2
        //k2++; //??? if (k2>150) qDebug()<<k2;
    }
    double pom[NQ*152+20];
    int cp = 0;
    for (int x = 0; x < 152; ++x)
    {
        for (int y = 0; y < NQ; ++y)
        {
            pom[cp] = s2_[x][y];
            cp++;
        }
    }
    double base2 = pomAll.pctile_shell(pom,NQ*152,50);//call pctile(s2,NQ*151,50,base2)
    if (base2==0.0) base2=0.000001;
    for (int x = 0; x < 152; ++x) //s2=s2/base2
    {
        for (int y = 0; y < NQ; ++y) s2_[x][y]/=base2;
    }// qDebug()<<base2;
    for (int i = 0; i < NQ; ++i) hist1[i]=0; //hist1=0
    for (int i = 0; i < NQ; ++i) hist2[i]=0; //hist2=0
    int ipk = 0;
    for (int j = 0; j < 152; ++j)//	int ipk = pomAll.maxloc_da_beg_to_end(s,ia,ib);
    {//do j=0,151
        ipk=pomAll.maxloc_da_beg_to_end(s2_[j],1,NQ);//ipk=maxloc(s2(1:NQ-1,j))//!Find the spectral peak
        //i=ipk(1)-1; //i=ipk(1)-1
        hist1[ipk]++;//hist1(i)=hist1(i)+1
    }
    hist1[0]=0; //hist1(0)=0                        //!Don't treat sync tone as a birdie
    for (int i = 0; i < 124; ++i)
    {//do i=0,123
        int sum = 0;
        for (int z = 0; z < 4; ++z) sum+=hist1[i+z];
        hist2[i]=sum;//hist2(i)=sum(hist1(i:i+3))
    }

    //ipk = maxloc_ia_beg_to_end(hist1,0,NQ); //ipk=maxloc(hist1)
    //i1=ipk(1)-1
    int m1=pomAll.maxval_ia_beg_to_end(hist1,0,NQ); //maxval(hist1)
    int i2 = maxloc_ia_beg_to_end(hist2,0,NQ); //ipk=maxloc(hist2)
    //i2=ipk(1)-1
    int m2 = pomAll.maxval_ia_beg_to_end(hist2,0,NQ);  //m2=maxval(hist2)
    if (m1>12) //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    {
        for (int i = 0; i < NQ; ++i)
        {//do i=0,127
            if (hist1[i]>12)
            {
                for (int j = 0; j < 152; ++j) s2_[j][i]=1.0; //s2(i,:)=1.0
            }
        }
    }
    if (m2>20)
    {
        if (i2>=1) i2=i2-1;//if(i2.ge.1) i2=i2-1
        if (i2>120) i2=120;//if(i2.gt.120) i2=120
        for (int i = 0; i < 152; ++i)
        {
            for (int j = 0; j < 8; ++j) s2_[i][i2+j]=1.0;//s2(i2:i2+7,:)=1.0
        }
    }

    int k3=0;
    int isync0[30];
    for (int i = 0; i < NS; ++i) isync0[i]=isync[i]-1;
    for (int n = 0; n < NDS; ++n)
    {//do n=1,NDS                             !Copy symbol spectra from s2 into s3
        //if(any(isync(1:NS).eq.n)) continue;//if(any(isync(1:NS).eq.n)) cycle     !Skip the sync symbols
        if (any_eq(isync0,n,NS)) continue;
        k3++;//k3=k3+1
        for (int z = 0; z < NQ; ++z) s3_[k3][z]=s2_[n+1][z];//s3(:,k3)=s2(:,n)
        //k3++;
    } //qDebug()<<k3;
    cp = 0;
    for (int x = 0; x < NN+1; ++x)
    {
        for (int y = 0; y < NQ; ++y)
        {
            pom[cp] = s3_[x][y];
            cp++;
        }
    }
    double base3 = pomAll.pctile_shell(pom,NQ*(NN+1),50);//call pctile(s3,NQ*NN,50,base3)
    if (base3==0.0) base3=0.000001;
    for (int x = 0; x < NN+1; ++x) //s3=s3/base3
    {
        for (int y = 0; y < NQ; ++y) s3_[x][y]/=base3;
    }//qDebug()<<s3_[0][0]<<s3_[1][0]<<s3_[2][0]<<"---"<<s3_[0][1]<<s3_[1][1]<<s3_[2][1];
}
void DecoderSFox::twkfreq2(double complex *c3,double complex *c4,int npts,double fsample,double fshift)
{
    double complex w=1.0+1.0*I;//double twopi=6.283185307;//! Adjust frequency of complex waveform
    double dphi=fshift*twopi/fsample;
    double complex wstep=cos(dphi)+sin(dphi)*I;
    for (int i =0; i<npts; ++i)
    {//do i=1,npts
        w=w*wstep;
        c4[i]=w*c3[i];
    }
}
void DecoderSFox::smo121a(double *x,int nz,double a,double b)
{
    double fac=(1.0/(a+2.0*b));//*1.08;
    double x0=x[0];//x0=x(1)
    for (int i =1; i<nz-1; ++i)
    {//do i=2,nz-1
        double x1=x[i];//x1=x(i)
        x[i]=fac*(a*x[i] + b*(x0+x[i+1]));//x(i)=fac*(a*x(i) + b*(x0+x(i+1)))
        x0=x1;
    }
}
#define ESNODEC 3.16 //EsNoDec=3.16; 1.95
#define CESNODE 3.2 //??? 1.85
//#define ESNODEC 3.16 //EsNoDec=3.16; 1.95
//#define CESNODE 4.0 //??? 1.85
void DecoderSFox::qpc_likelihoods2(double py_[137][137],double s3_[137][137])//,double EsNo,double No)
{
    const int QQ=128;
    const int QN=128;//original =double EsNoDec=3.16;
    double norm=(ESNODEC/(ESNODEC+1.0))/1.0;//norm=(EsNo/(EsNo+1.0))/No;
    double normpwrmax;
    double normpwr;
    for (int k =0; k<QN; ++k) //! Compute likelihoods for symbol values, from the symbol power spectra
    {//do k=0,QN-1
        normpwrmax=0.0;
        for (int j =0; j<QQ; ++j)
        {//do j=0,QQ-1
            normpwr=norm*s3_[k][j];//s3(j,k)
            py_[k][j]=normpwr;//py(j,k)=normpwr
            normpwrmax=fmax(normpwr,normpwrmax);
        }
        double pynorm=0.0;
        for (int j =0; j<QQ; ++j)
        {//do j=0,QQ-1
            py_[k][j]=exp(py_[k][j]-normpwrmax);//py(j,k)=exp(py(j,k)-normpwrmax)
            pynorm += py_[k][j];//py(j,k)
        }
        if (pynorm==0.0) pynorm=0.000001;
        pynorm*=CESNODE;//HV 2.76.2
        for (int x =0; x<QQ; ++x) py_[k][x]=py_[k][x]/pynorm;//py(0:QQ-1,k)=py(0:QQ-1,k)/pynorm//!Normalize to probabilities
    }//qDebug()<<"PY="<<py_[0][0]<<py_[1][0]<<py_[2][0]<<"---"<<py_[0][1]<<py_[1][1]<<py_[2][1]<<exp(0.0);
}
void DecoderSFox::qpc_snr(double s3_[137][137],unsigned char *y,double &snr)
{
    double p = 0.0;
    for (int j =0; j<128; ++j)
    {//do j=1,127
        int i=y[j];
        p+=s3_[j][i];//p=p + s3(i,j)
    }
    snr = pomAll.db(p/127.0)-pomAll.db(127.0)-4.0;//snr = db(p/127.0) - db(127.0) - 4.0
}
//#define K_RAND_MAX (unsigned int)(525332000)//(unsigned int)((double)RAND_MAX0*0.2441480759)//to=524304000
//#define CORMEAN   -0.158//-0.14//TYPE3 step=1
#define K_RAND_MAX (unsigned int)(515396075)//((double)0x7fffffff*0.24)=515396075
#define CORMEAN   -0.15//TYPE3 step=1
int DecoderSFox::random_r0(int32_t &result)
{    
    int32_t *state;
    random_data0 *buf = &unsaf_buf;
    if (buf == NULL/* || result == NULL*/) return -1;//goto fail;
    state = unsaf_buf.state;
    if (unsaf_buf.rand_type == TYPE_00)
    {
        int32_t val = ((state[0] * 1103515245U) + 12345U) & 0x7fffffff;
        state[0] = val;
        result = val;
    }
    else
    {
        int32_t *fptr = unsaf_buf.fptr;
        int32_t *rptr = unsaf_buf.rptr;
        int32_t *end_ptr = unsaf_buf.end_ptr;
        uint32_t val;
        val = *fptr += (uint32_t) *rptr;// Chucking least random bit.
        result = val >> 1;
        ++fptr;
        if (fptr >= end_ptr)
        {
            fptr = state;
            ++rptr;
        }
        else
        {
            ++rptr;
            if (rptr >= end_ptr) rptr = state;
        }
        unsaf_buf.fptr = fptr;
        unsaf_buf.rptr = rptr;
    }
    return 0;
}
int DecoderSFox::srand0(unsigned int seed)
{
    int type;
    int32_t *state;
    long int i;
    int32_t word;
    int32_t *dst;
    int kc; 
    random_data0 *buf = &unsaf_buf;
    if (buf == NULL) return -1;//goto fail;
    type = unsaf_buf.rand_type; //if ((unsigned int) type >= MAX_TYPES) goto fail;
    state = unsaf_buf.state; // We must make sure the seed is not 0.  Take arbitrarily 1 in this case.
    if (seed == 0) seed = 1;
    state[0] = seed;            
    if (type == TYPE_00) return 0;//goto done;
    dst = state;
    word = seed;
    kc = unsaf_buf.rand_deg;
    for (i = 1; i < kc; ++i)
    {
        //This does:
        //state[i] = (16807 * state[i - 1]) % 2147483647;
        //but avoids overflowing 31 bits.
        long int hi = word / 127773;
        long int lo = word % 127773;
        word = 16807 * lo - 2836 * hi;
        if (word < 0) word += 2147483647;
        *++dst = word;
    }
    unsaf_buf.fptr = &state[unsaf_buf.rand_sep];
    unsaf_buf.rptr = &state[0];
    kc *= 10;
    while (--kc >= 0)
    {
        int32_t discard;
        random_r0(discard);
    }
    return 0;
}
long int DecoderSFox::rand0()
{	
    int32_t retval = 0; 
    random_r0(retval);
    return retval;
}
double DecoderSFox::rand_noice()
{
    const double stdev = 0.5 / sqrt(2.0);//max=2147483647
    const double scalephi = (6.283185307 / (1.0 + (double)K_RAND_MAX));
    const double scaleu = (1.0 / (1.0 + (double)K_RAND_MAX)); //const double CORMEAN = 0.0;//-3.95;
    static double phi=0.0;
    static double u=0.0;
    static bool set = false;
    unsigned int r;
    double out = 0.0;
    if (set)
    {
        out = sin(phi) * u * stdev + CORMEAN;
        set = false;
    }
    else
    {
        r=(rand0()%K_RAND_MAX);// generate a uniform distributed phase phi in the range [0,2*PI)
        phi = scalephi * (double)r;
        r=(rand0()%K_RAND_MAX);// generate a uniform distributed random number u in the range (0,1]
        u = scaleu * (0.5 + (double)r);
        u = sqrt(-2.0 * log(u));// generate normally distributed number
        out = cos(phi) * u * stdev + CORMEAN;
        set = true;
    }
    return out;
}
#include "../nhash.h"

//#define DEB_SEEDS
//#define GET_SEEDS

#if defined GET_SEEDS
static QList<int> ggg;
bool hv_sort_int_(const int &d1,const int &d2)
{
     return d1 > d2;
}
#endif
void DecoderSFox::qpc_decode2(double complex *c0,double fsync,double fa,double fb,unsigned char *xdec,int ndepth,
                              double dth,double damp,bool &crc_ok,double &fbest,double &tbest,double &snr,uint8_t &type)
{			
    const int NZ=100;
    double fsample=12000.0;
    double f2[3];
    double t2[3];
    double snrsync[3] = {0.0,0.0,0.0};
    double complex *c = new double complex[NMAX+2048];//double complex c[NMAX+1024];    
    int cseed = 0;//double No=1.0;        
    const int maxseed = 111;//292,111
#if defined DEB_SEEDS    
    int inter = 0; int new0 = 0; unsigned int startf = 1000; unsigned int cseef = startf; int cseefu = -1; //int idith0 = 0; int kk0 = 0;
#endif    
#if defined GET_SEEDS    
    const int maxdither[8] = {20,50,maxseed+10000,200,500,1000,2000,5000};
#else
    const int maxdither[8] = {20,50,maxseed+20,200,500,1000,2000,5000};    
#endif   
	const unsigned int seed[maxseed] =
    {    	
		60568,33762,33481,32742,30621,30412,23117,22521,20534,20457,	//12
		20180,19959,18552,18174,17727,17450,16703,16661,16483,16120,	//12
		16045,15684,15455,15326,15146,15093,14945,14624,14505,14434,	//12
		14225,14214,13821,13554,13513,13052,12961,12857,12786,12633,	//12
		12424,12045,12004,11986,11965,11807,11764,11568,11450,11149,	//12
		10845,10322,10249,10037,9985,9661,9644,9270,9248,9095,8981,		//12
		7731,7564,7559,6707,6543,6438,6098,6063,5650,5433,5397,5330,	//12
		5246,5229,5091,4968,4852,4455,4143,3523,3326,3166,3126,3052,	//12
		2806,2556,2385,2036,2034,1544,1542,1475,1355,796,666,520,464,	//12
		431,424,398,356,345,334,324,284,193,172,111,62,57				//12
		/*,
		14607,13229,13029,11238,10816,10737,							//11
		10360,9495,9145,8552,7504,7103,6366,							//11
		5700,4208,3498,3404,3276,										//11
		1436,1035,954,686,590,550,519,476,433,342,						//11
		247,195,135,122,95,72											//11
		,
		32572,20961,20838,19475,14958,13589,9996,						//10
		6148,2801,														//10
		673,469,128														//10
		,
		35848,17577,15498,15350,15110,12661,							//9
		10243,9618,8608,7190,5505,4930,2505,							//9
		1009,61,														//9
		
		24314,24228,21977,20029,16833,14804,14782,14472,13573,			//8
		13131,11104,10230,7203,6828,2517,2,								//8
		
		63144,42979,31968,26109,20993,20056,							//7
		16002,12197,9252,5062,4388,2175,2019,1885,1628,					//7
		1356,688,137,													//7
		
		55956,40506,22827,19905,18918,									//6
		14494,12583,11975,11214,11027,									//6	
		9662,7787,7495,5210,4821,4451,									//6	
		2952,2330,253,													//6
		
		24775,23045,22234,22065,20202,									//5
		14201,11539,													//5
		10803,9052,8077,7010,6094,5525,4651,							//5
		3064,1111,79,													//5
		
		58216,49685,46196,45462,39768,28675,24691,						//4
		18236,17187,10143,7650,											//4
		6796,6571,5366,3320,											//4
		2405,1748,897,470,215,											//4
		
		61730,37256,34878,26993,										//3
		23935,23293,22914,21158,18446,									//3
		16609,14090,12089,10471,										//3
		5337,3770,2648,1097,											//3	
		311,															//3	
				
		64246,48446,34591,												//2
		15310,12189,7436,6641,											//2
		3783,1538,1513,274,164,											//2	
		
		//1000 //61,2,1,30094,1000 <-for end*/
	};                                     
    const int idf[NZ] =
        {
            0,  0, -1,  0, -1,  1,  0, -1,  1, -2,  0, -1,  1, -2,  2,
            0, -1,  1, -2,  2, -3,  0, -1,  1, -2,  2, -3,  3,  0, -1,
            1, -2,  2, -3,  3, -4,  0, -1,  1, -2,  2, -3,  3, -4,  4,
            0, -1,  1, -2,  2, -3,  3, -4,  4, -5, -1,  1, -2,  2, -3,
            3, -4,  4, -5,  1, -2,  2, -3,  3, -4,  4, -5, -2,  2, -3,
            3, -4,  4, -5,  2, -3,  3, -4,  4, -5, -3,  3, -4,  4, -5,
            3, -4,  4, -5, -4,  4, -5,  4, -5, -5
        };
    const int idt[NZ] =
        {
            0 , -1,  0,  1, -1,  0, -2,  1, -1,  0,  2, -2,  1, -1,  0,
            -3,  2, -2,  1, -1,  0,  3, -3,  2, -2,  1, -1,  0, -4,  3,
            -3,  2, -2,  1, -1,  0,  4, -4,  3, -3,  2, -2,  1, -1,  0,
            -5,  4, -4,  3, -3,  2, -2,  1, -1,  0, -5,  4, -4,  3, -3,
            2, -2,  1, -1, -5,  4, -4,  3, -3,  2, -2,  1, -5,  4, -4,
            3, -3,  2, -2, -5,  4, -4,  3, -3,  2, -5,  4, -4,  3, -3,
            -5,  4, -4,  3, -5,  4, -4, -5,  4, -5
        };
    double baud=12000.0/1024.0; //double (*s1w_)[7000] = new double[800][7000];
    double (*s2_)[137]  = new double[161][137];//double s2_[161][137];//real s2(0:127,0:151)
    double (*s3_)[137]  = new double[137][137];//double s3_[137][137];//real s3(0:127,0:127)
    double (*py_)[137]  = new double[137][137];//double py_[137][137];//!Probabilities for received synbol values
    double (*py0_)[137] = new double[137][137];//double py0_[137][137];//!Probabilities for strong signal
    double (*pyd_)[137] = new double[137][137];//double pyd_[137][137];//real pyd(0:127,0:127) !Dithered values for py
    float *tpyd = new float[128*128+128];
    double *pom = new double[128*128+130];
    unsigned char ydec[128+10];//unsigned char *ydec = new unsigned char[128+10];//unsigned char ydec[128+10];
    uint32_t mask21=(pow(2,21) - 1);
    uint64_t n47=47;
    qpc_sync(c0,fsample,fsync,fa,fb,f2,t2,snrsync);//qpc_sync(c0,fsample,isync,fsync,ftol,f2,t2,snrsync)

    int index[5]; //f2[0]=0.0; f2[1]=0.0; f2[2]=0.0; //t2[0]=1.0; t2[1]=1.0; t2[2]=1.0;
    for (int i=0; i<4; ++i) index[i]=i;
    if (snrsync[1]>snrsync[0])
    {
        index[0]=1;
        index[1]=0;
        index[2]=2;
    }
    if (snrsync[2]>snrsync[index[0]]+0.05)
    {
        index[2]=index[1];
        index[1]=index[0];
        index[0]=2;
    }
    else if (snrsync[2]>snrsync[index[1]]+0.05)
    {
        index[2]=index[1];
        index[1]=2;
    }
#if defined DEB_SEEDS     
    QString sss; printf("  0-MyRX  =%.3f\n",snrsync[0]); printf("  1-STATIC=%.3f\n",snrsync[1]); printf("  2-FULL  =%.3f\n",snrsync[2]);
    for (int z=0; z<3; ++z)
    {
		if (index[z]==0) sss.append("0-MyRX  "); 
		if (index[z]==1) sss.append("1-STATIC  "); 
		if (index[z]==2) sss.append("2-FULL  ");	
    } printf("   IndexOrder->  %s\n",qPrintable(sss));
#endif     
#if defined GET_SEEDS     
c40:
#endif    
    double f20[3];
    double t20[3];
    for (int z=0; z<3; ++z)
    {
    	f20[z]=-1.0;
    	t20[z]=-1.0;
   	}
    bool f_out_loops = false;
    for (int ccand=0; ccand<3; ++ccand)
    {
        int ncand = index[ccand];
        bool f_continue = false; 
        for (int cft20=0; cft20<3; ++cft20)
        {
        	if (f20[cft20]==f2[ncand] && t20[cft20]==t2[ncand])
        	{       		
        		f_continue = true; //printf(" ->EQ cft20=%d ncand=%d F20=%.3f T20=%.3f F=%.3f T=%.3f\n",cft20,ncand,f20[cft20],t20[cft20],f2[ncand],t2[ncand]);       		        		
        		break;        		
       		}
       	}
       	if (f_continue) continue;
       	f20[ncand]=f2[ncand];
       	t20[ncand]=t2[ncand];        
#if defined DEB_SEEDS        
        printf("F2=%.3f FBest=%.3f DT=%.3f Sync=%.3f\n",f20[ncand],((1500.0+f20[ncand])-750.0),t20[ncand],snrsync[ncand]);
#endif        
        double f00=1500.0 + f2[ncand];
        double t00=t2[ncand];
        fbest=f00;
        tbest=t00;
        int maxd=1;//maxd=1//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
        if (ndepth>0) maxd=maxdither[ndepth-1];
        int maxft=NZ;
        //if (snrsync<4.0 || ndepth<=0) maxft=1;//if(snrsync.lt.4.0 .or. ndepth.le.0) maxft=1
        if (snrsync[ncand]<4.0 || ndepth<=0) maxft=1; //int cp = 0; //for (int z=0; z<NMAX; ++z) c[z]=0.0+0.0*I;
        for (int idith=0; idith<maxft; ++idith)
        {//do idith=1,maxft
        	//if (snrsync[ncand]>9.65)//=*1.44             
			//if (snrsync[ncand]>9.50)//=*1.2 
			//if (snrsync[ncand]>9.40)//=*1.0  
			if (snrsync[ncand]>9.98)//=*2.0  //if (idith>=1) maxd=2;//if(idith.ge.2) maxd=1			
			{
				if (idith>=4) maxd=2; 	
			}
			else if (idith>=1) maxd=2;          	
            double deltaf=(double)idf[idith]*0.5;
            double deltat=(double)idt[idith]*8.0/1024.0;
            double f=f00+deltaf;
            double t=t00+deltat;//0.0971458;
            double fshift=1500.0 - (f+baud);//!Shift frequencies down by f + 1 bin
            twkfreq2(c0,c,NMAX,fsample,fshift); //qDebug()<<"idith="<<idith<<f<<t; idith0 = idith;
            double a=1.0;
            double b=0.0; //idith0 = idith;            
            for (int kk=1; kk<=4; ++kk)
            {//do kk=1,4
                if (kk==2) b=0.4;//if(kk==2) b=0.4
                if (kk==3) b=0.5;//if(kk==3) b=0.5
                if (kk==4) b=0.6;//if(kk==4) b=0.6 //kk0 = kk;
                sfox_demod(c,1500.0,t,s2_,s3_);//call sfox_demod(c,1500.0,t,isync,s2,s3)!Compute s2 and s3
                if (b>0.0) //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
                {
                    for (int j=0; j<128; ++j)
                    {//do j=0,127 call smo121a(s3(:,j),128,a,b)
                        smo121a(s3_[j],128,a,b);//smo121a(s3(:,j),128,a,b)
                    }
                }
                int cp = 0;
                for (int x = 0; x < 128; ++x)
                {
                    for (int y = 0; y < 128; ++y)
                    {
                        pom[cp] = s3_[x][y];
                        cp++;
                    }
                }
                double base3 = pomAll.pctile_shell(pom,128*128,50);//call pctile(s3,128*128,50,base3)
                if (base3==0.0) base3=0.000001;  //qDebug()<<"Base"<<base3;
                for (int x = 0; x < 128; ++x) //s3=s3/base3
                {
                    for (int y = 0; y < 128; ++y)
                    {
                        s3_[x][y]/=base3;
                        py0_[x][y]=s3_[x][y];
                    }
                }
                //double EsNoDec=1.95;//3.16;//3.16;//3.16;  //0.9-1.0
                //No=1.0;  //qDebug()<<"-->"<<((EsNoDec/(EsNoDec+1.0))/No);//norm=(EsNo/(EsNo+1.0))/No;
                qpc_likelihoods2(py_,s3_);//,EsNoDec,No);//qpc_likelihoods2(py,s3,EsNoDec,No)//!For weak signals          
                cseed = 0;
                type = 0;
#if defined DEB_SEEDS                
                cseef = cseef; new0 = 0; cseefu = -1;            
#endif                
                for (int kkk = 1; kkk <= maxd; ++kkk)
                {//do kkk=1,maxd
                    if (kkk==1)//if(kkk.eq.1) then //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
                    {
                        for (int x = 0; x < 128; ++x)//pyd=py0
                        {
                            for (int y = 0; y < 128; ++y) pyd_[x][y]=py0_[x][y];
                        }
                        type = 0;
                    }
                    else
                    {
                        type = 2;
                        for (int x = 0; x < 128; ++x)//pyd=0.
                        {
                            for (int y = 0; y < 128; ++y) pyd_[x][y]=0.0;
                        }
                        if (kkk>2)//if(kkk.gt.2) then
                        {
                       		if (cseed < maxseed)
                        	{ 
                        		srand0(seed[cseed]); //if (cseed % 20==0) printf("Tcseed=%d\n",cseed);
                        		cseed++;   
                       		}
                       		else 
                       		{
                       			cseed = 1001;
#if defined DEB_SEEDS                        			
                       			new0 = 2000000002; 	//find seed start this	
                       			cseefu = cseef;
                       			srand0(cseef);   	//if (cseef % 100==0) printf("---Fcseed=%d\n",cseef); 
                       			cseef++;		    //end find seed start this
c4:                       			
                       			for (int z = 0; z < maxseed; ++z)
                       			{
                       				if (cseef==seed[z])
                       				{
                       					cseef++;
                       					goto c4;
                      				}                       					  
                      			}
#endif                      			
                      		}                       			
                            for (int x = 0; x < 128; ++x)//call random_number(pyd)
                            {
                                for (int y = 0; y < 128; ++y) pyd_[x][y]=rand_noice();
                            }                           
                            /*for (int x = 0; x < 128; ++x)
                            {
                                for (int y = 0; y < 128; ++y) 
                                {
                                	double read=rand_noice();
                                	double imgd=rand_noice();
                                	pyd_[x][y]=(read*read)+(imgd*imgd);	                                 	
                               	}
                            }*/
                            /*for (int x = 0; x < 128; ++x)//pyd=2.0*(pyd-0.5)
                            {
                                for (int y = 0; y < 128; ++y) pyd_[x][y]=2.0*(pyd_[x][y]-0.5);
                            }*/
                            /*cxxx = 0;
                            for (int x = 0; x < 128; ++x)//call random_number(pyd)
                            {
                                for (int y = 0; y < 128; ++y) 
                                {
                                	xxx[cxxx]=pyd_[x][y]*4388607.0; 
                                	cxxx++;                               	
                               	}
                            }*/                             
                            type = 3;
                        }
                        for (int x = 0; x < 128; ++x)//where(py.gt.dth) pyd=0.//!Don't perturb large likelihoods
                        {
                            for (int y = 0; y < 128; ++y)//dth=0.5
                            {
                                if (py_[x][y]>dth) pyd_[x][y]=0.0;
                            }
                        }
                        for (int x = 0; x < 128; ++x)//pyd=py*(1.0 + damp*pyd)//!Compute dithered likelihood
                        {
                            for (int y = 0; y < 128; ++y)//damp=1.0
                            {
                                pyd_[x][y]=py_[x][y]*(1.0 + damp*pyd_[x][y]);
                            }
                        }
                    }
                    for (int j = 0; j < 128; ++j) //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
                    {//do j=0,127
                        double ss = 0;//ss=sum(pyd(:,j))
                        for (int x = 0; x < 128; ++x) ss+=pyd_[j][x];
                        if (ss>0.0)
                        {
                            for (int x = 0; x < 128; ++x) pyd_[j][x]=pyd_[j][x]/ss; //pyd(:,j)=pyd(:,j)/ss
                        }
                        else
                        {
                            for (int x = 0; x < 128; ++x) pyd_[j][x]=0.0;////pyd(:,j)=0.0
                        }
                    }
                    int ctpyd = 0;
                    for (int x = 0; x < 128; ++x)//float tpyd[128*128+128];//=16348
                    {
                        for (int y = 0; y < 128; ++y)
                        {
                            tpyd[ctpyd] = (float)pyd_[x][y];//static_cast<float>(x);
                            ctpyd++;//=16348
                        }

                    }
                    unsigned char xdec0[50+10];//={0};
                    qpc_decode(xdec0,ydec,tpyd);
                    int cxdec0 = 0;
                    for (int x = 49; x >= 0; --x)
                    {
                        xdec[cxdec0]=xdec0[x];//xdec=xdec(49:0:-1)
                        cxdec0++;
                    }
#if defined DEB_SEEDS                    
                    inter++;
#endif                    
                    /*QString sss = TGenSFox->sfox_unpack(xdec,false);                   
                    QStringList lss = sss.split("#");
                    for (int r = 0; r < lss.count(); ++r)
                    {
                        QString sss0 = lss.at(r);
                        if (sss0.contains("CQ J")) printf("Ntype=%d %s\n",type,qPrintable(sss0));
                    }*/
                    uint32_t crc_chk=(nhash2(xdec,n47,571) & mask21);//iand(nhash2(xdec,n47,571),mask21)//!Compute crc_chk
                    uint32_t crc_sent=128*128*xdec[47] + 128*xdec[48] + xdec[49];//crc_sent=128*128*xdec(47) + 128*xdec(48) + xdec(49)
         			if (crc_chk==crc_sent) crc_ok=true;//crc_ok=crc_chk.eq.crc_sent
                    if (crc_ok)
                    {
                        qpc_snr(s3_,ydec,snr);//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
                        if (snr<-16.95) crc_ok=false;//if (snr<-16.5) crc_ok=false;
#if defined DEB_SEEDS
#if defined GET_SEEDS   
						if (crc_ok)
#endif 							 
						{
                        	QString sss1 = TGenSFox->sfox_unpack(xdec,false);
                        	printf("  *** Decode-> %s <- NTYPE=%d %d %.3f ***\n",qPrintable(sss1),type,crc_ok,snr);	
                            /*if(type==3)
                            {                            
                            	TMsPlayerHV->SaveFile(xxx,cxxx,QString("%1").arg(cseed));                            	
                           	}*/                         						
						}                                              
#endif                        
                        f_out_loops = true;
                        break;
                    }
                }//!kkk: dither of probabilities
                if (f_out_loops) break;
            }//!kk: dither of smoothing weights
            if (f_out_loops) break;
        }//!idith: dither of frequency and time
        if (f_out_loops) break;
    }//hv ccand loop 
#if defined DEB_SEEDS    
    if (!crc_ok || type!=3) new0 = 0;
    int tabb = -1;
    if (crc_ok && type==3 && cseed<=maxseed && new0==0) tabb = seed[cseed-1];
    if (new0>0 || tabb>-1) printf("--MAX INTER-- %d--> New=%d FSeed=%d <--TSeed=%d Table=%d\n",inter,new0,cseefu,cseed-1,tabb);
    printf("------------------------------------------------------\n");
#endif
#if defined GET_SEEDS 
    if (new0 > 100)// if (tabb != -1)
    {
    	bool exist = false;
    	//if (ggg.count()<1) ggg.append(1);
    	for (int x = 0; x < ggg.count(); ++x)
    	{
            if (cseefu==ggg.at(x))//if (tabb==ggg.at(x)) 
            {
            	exist = true;
            	break;            	
           	}    		
   		}    	    		
    	if (!exist) ggg.append(cseefu);//ggg.append(tabb);
    	std::sort(ggg.begin(),ggg.end(),hv_sort_int_);
    	for (int x = 0; x < ggg.count(); x+=10)
    	{
    		for (int y = 0; y < 10; ++y)
    		{
    			if (y+x<ggg.count()) printf("%d,",ggg.at(y+x));
    			else printf("%d,",0);
   			}
    		printf("\n"); 
   		} printf("Count=%d\n",ggg.count()); 
   	} if (cseef<maxseed+startf+10000) goto c40;
#endif    	
    delete [] c;
    delete [] s2_;
    delete [] s3_;
    delete [] py_;
    delete [] py0_;
    delete [] pyd_;
    delete [] tpyd;
    delete [] pom;
}
void DecoderSFox::sfox_decode(double *dd,double nfa0,double nfb0,double fqso,bool &have_dec)
{
    have_dec = false; //const int NMAX=15*12000;//=180000
    const int npts=15*12000;
    unsigned char xdec[50+10];//unsigned char *xdec = new unsigned char[50+10];//unsigned char xdec[50+10];
    bool crc_ok=false; //double snrsync;
    double fbest;
    double tbest;
    double snr;
    double complex *c0 = new double complex[NMAX+8192];//NMAX=180000
    double fsync=fqso; 

    //no needed tested for (int j = 174000; j<npts+10; ++j) dd[j]=0.00000001;//2.76.4 mute 14.500

    sfox_remove_ft8(dd);
    sfox_ana(dd,npts,c0); //qDebug()<<fsync;
    if (fsync<2401.0) sfox_remove_tone(c0,fsync); //2.76.1 -> ib=(int)((fsync+1500.0+50.0)/df); max=45000;//=3000Hz

    int ndepth=3;
    double dth=0.5;
    double damp=1.0;//0.680; //damp=1.0 0.651
    uint8_t type;
    qpc_decode2(c0,fsync,nfa0,nfb0,xdec,ndepth,dth,damp,crc_ok,fbest,tbest,snr,type);

    if (crc_ok)
    {
        //bool f_only_one_color = true;
        emit EmitBackColor();
        QString message = TGenSFox->sfox_unpack(xdec,true);//bool (f_otp)
        QStringList SlMsgs;
        if (message.contains("#")) SlMsgs = message.split("#");
        else SlMsgs << message;
        int nslots = SlMsgs.count();
        fbest = fbest - 750.0;
        for (int i = 0; i < nslots; ++i)
        {
            message = SlMsgs.at(i);
            //float qual=1.0-(nhr+dmi)/60.0; //scale qual to 0.0-1.0
            //if (qual<0.001) qual=0.0;//no show -0.0
            QString str_iaptype = QString("%1").arg(type);
            //if (qual<0.17) str_iaptype = "? ";
            /*if (type==1) str_iaptype.append("1");
            elseif (type==1) str_iaptype.append("1");
            if (type==1) str_iaptype.append("1");*/
            //istr_iaptype.append(QString("%1").arg(type));
            int df_hv = fbest-nftx;
            QString sdtx = QString("%1").arg(tbest,0,'f',1);
            if (sdtx=="-0.0") sdtx="0.0";//remove -0.0 exception
            QStringList list;
            list <<s_time<<QString("%1").arg((int)snr)
            <<sdtx<<QString("%1").arg((int)df_hv)
            <<message<<str_iaptype
            <<"1.0"/*QString("%1").arg(qual,0,'f',1)*/
            <<QString("%1").arg((int)fbest);
            emit EmitDecodetText(list);//1.27 psk rep fopen bool true, false no file open
        }
        //nsignature = 1
        //call sfox_unpack(nutc,xdec,nsnr,fbest-750.0,tbest,foxcall,nsignature)
        have_dec = true;
    }
    delete [] c0;
}

