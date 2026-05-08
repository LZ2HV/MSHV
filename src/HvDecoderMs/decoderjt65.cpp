/* The algorithms, source code, look-and-feel of WSJT-X and related
 * programs, and protocol specifications for the modes FSK441, FT8, JT4,
 * JT6M, JT9, JT65, JTMS, QRA64, ISCAT, MSK144, are Copyright © 2001-2017
 * by one or more of the following authors: Joseph Taylor, K1JT; Bill
 * Somerville, G4WJS; Steven Franke, K9AN; Nico Palermo, IV3NWV; Greg Beam,
 * KI7MT; Michael Black, W9MDB; Edson Pereira, PY2SDR; Philip Karn, KA9Q;
 * and other members of the WSJT Development Group.
 *
 * MSHV JT65 Decoder
 * Rewritten into C++ and modified by Hrisimir Hristov, LZ2HV 2015-2017
 * May be used under the terms of the GNU General Public License (GPL)
 */

#include "decoderms.h"
//#include <QtGui>

/* Reed-Solomon decoder
 * Copyright 2002 Phil Karn, KA9Q
 * May be used under the terms of the GNU General Public License (GPL)
 * Modified by Steve Franke, K9AN, for use in a soft-symbol RS decoder
 */
static void *rs;

/////////////////////////end init_rs_int_rx, decode_rs_int  //////////
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
#define DEBUG 0
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

//#define ENCODE_RS encode_rs_int
//#define DECODE_RS decode_rs_int
//#define INIT_RS init_rs_int
//#define FREE_RS free_rs_int

void *init_rs_int_rx(int symsize,int gfpoly,int fcr,int prim,
                     int nroots,int pad)
{
    struct rs *rs;
    int i, j, sr,root,iprim;

    /* Check parameter ranges */
    if (symsize < 0 || symsize > (int)(8*sizeof(DTYPE)))
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

#define	min(a,b)	((a) < (b) ? (a) : (b))

// tova da se proveri v lib e edno a v ftrsd drugo ?????
// ot ftrsd ne ot lib
int decode_rs_int(
#ifndef FIXED
    void *p,
#endif
    DTYPE *data, int *eras_pos, int no_eras, int calc_syn)
{

#ifndef FIXED
    struct rs *rs = (struct rs *)p;
#endif
    int deg_lambda, el, deg_omega;
    int i, j, r,k;
    DTYPE u,q,tmp,num1,num2,den,discr_r;
    DTYPE lambda[NROOTS+1];	// Err+Eras Locator poly
    static DTYPE s[51];					 // and syndrome poly
    DTYPE b[NROOTS+1], t[NROOTS+1], omega[NROOTS+1];
    DTYPE root[NROOTS], reg[NROOTS+1], loc[NROOTS];
    int syn_error, count;

    if ( calc_syn )
    {
        /* form the syndromes; i.e., evaluate data(x) at roots of g(x) */
        for (i=0;i<NROOTS;i++)
            s[i] = data[0];

        for (j=1;j<NN;j++)
        {
            for (i=0;i<NROOTS;i++)
            {
                if (s[i] == 0)
                {
                    s[i] = data[j];
                }
                else
                {
                    s[i] = data[j] ^ ALPHA_TO[MODNN(INDEX_OF[s[i]] + (FCR+i)*PRIM)];
                }
            }
        }

        /* Convert syndromes to index form, checking for nonzero condition */
        syn_error = 0;
        for (i=0;i<NROOTS;i++)
        {
            syn_error |= s[i];
            s[i] = INDEX_OF[s[i]];
        }


        if (!syn_error)
        {
            /* if syndrome is zero, data[] is a codeword and there are no
             * errors to correct. So return data[] unmodified
             */
            count = 0;
            goto finish;
        }

    }

    memset(&lambda[1],0,NROOTS*sizeof(lambda[0]));
    lambda[0] = 1;

    if (no_eras > 0)
    {
        /* Init lambda to be the erasure locator polynomial */
        lambda[1] = ALPHA_TO[MODNN(PRIM*(NN-1-eras_pos[0]))];
        for (i = 1; i < no_eras; i++)
        {
            u = MODNN(PRIM*(NN-1-eras_pos[i]));
            for (j = i+1; j > 0; j--)
            {
                tmp = INDEX_OF[lambda[j - 1]];
                if (tmp != A0)
                    lambda[j] ^= ALPHA_TO[MODNN(u + tmp)];
            }
        }

#if DEBUG >= 1
        /* Test code that verifies the erasure locator polynomial just constructed
         Needed only for decoder debugging. */

        /* find roots of the erasure location polynomial */
        for (i=1;i<=no_eras;i++)
            reg[i] = INDEX_OF[lambda[i]];

        count = 0;
        for (i = 1,k=IPRIM-1; i <= NN; i++,k = MODNN(k+IPRIM))
        {
            q = 1;
            for (j = 1; j <= no_eras; j++)
                if (reg[j] != A0)
                {
                    reg[j] = MODNN(reg[j] + j);
                    q ^= ALPHA_TO[reg[j]];
                }
            if (q != 0)
                continue;
            /* store root and error location number indices */
            root[count] = i;
            loc[count] = k;
            count++;
        }
        if (count != no_eras)
        {
            printf("count = %d no_eras = %d\n lambda(x) is WRONG\n",count,no_eras);
            count = -1;
            goto finish;
        }
#if DEBUG >= 2
        printf("\n Erasure positions as determined by roots of Eras Loc Poly:\n");
        for (i = 0; i < count; i++)
            printf("%d ", loc[i]);
        printf("\n");
#endif
#endif
    }
    for (i=0;i<NROOTS+1;i++)
        b[i] = INDEX_OF[lambda[i]];

    /*
     * Begin Berlekamp-Massey algorithm to determine error+erasure
     * locator polynomial
     */
    r = no_eras;
    el = no_eras;
    while (++r <= NROOTS)
    {	/* r is the step number */
        /* Compute discrepancy at the r-th step in poly-form */
        discr_r = 0;
        for (i = 0; i < r; i++)
        {
            if ((lambda[i] != 0) && (s[r-i-1] != A0))
            {
                discr_r ^= ALPHA_TO[MODNN(INDEX_OF[lambda[i]] + s[r-i-1])];
            }
        }
        discr_r = INDEX_OF[discr_r];	/* Index form */
        if (discr_r == A0)
        {
            /* 2 lines below: B(x) <-- x*B(x) */
            memmove(&b[1],b,NROOTS*sizeof(b[0]));
            b[0] = A0;
        }
        else
        {
            /* 7 lines below: T(x) <-- lambda(x) - discr_r*x*b(x) */
            t[0] = lambda[0];
            for (i = 0 ; i < NROOTS; i++)
            {
                if (b[i] != A0)
                    t[i+1] = lambda[i+1] ^ ALPHA_TO[MODNN(discr_r + b[i])];
                else
                    t[i+1] = lambda[i+1];
            }
            if (2 * el <= r + no_eras - 1)
            {
                el = r + no_eras - el;
                /*
                 * 2 lines below: B(x) <-- inv(discr_r) *
                 * lambda(x)
                 */
                for (i = 0; i <= NROOTS; i++)
                    b[i] = (lambda[i] == 0) ? A0 : MODNN(INDEX_OF[lambda[i]] - discr_r + NN);
            }
            else
            {
                /* 2 lines below: B(x) <-- x*B(x) */
                memmove(&b[1],b,NROOTS*sizeof(b[0]));
                b[0] = A0;
            }
            memcpy(lambda,t,(NROOTS+1)*sizeof(t[0]));
        }
    }

    /* Convert lambda to index form and compute deg(lambda(x)) */
    deg_lambda = 0;
    for (i=0;i<NROOTS+1;i++)
    {
        lambda[i] = INDEX_OF[lambda[i]];
        if (lambda[i] != A0)
            deg_lambda = i;
    }
    /* Find roots of the error+erasure locator polynomial by Chien search */
    memcpy(&reg[1],&lambda[1],NROOTS*sizeof(reg[0]));
    count = 0;		/* Number of roots of lambda(x) */
    for (i = 1,k=IPRIM-1; i <= NN; i++,k = MODNN(k+IPRIM))
    {
        q = 1; /* lambda[0] is always 0 */
        for (j = deg_lambda; j > 0; j--)
        {
            if (reg[j] != A0)
            {
                reg[j] = MODNN(reg[j] + j);
                q ^= ALPHA_TO[reg[j]];
            }
        }
        if (q != 0)
            continue; /* Not a root */
        /* store root (index-form) and error location number */
#if DEBUG>=2
        printf("count %d root %d loc %d\n",count,i,k);
#endif
        root[count] = i;
        loc[count] = k;
        /* If we've already found max possible roots,
         * abort the search to save time
         */
        if (++count == deg_lambda)
            break;
    }
    if (deg_lambda != count)
    {
        /*
         * deg(lambda) unequal to number of roots => uncorrectable
         * error detected
         */
        count = -1;
        goto finish;
    }
    /*
     * Compute err+eras evaluator poly omega(x) = s(x)*lambda(x) (modulo
     * x**NROOTS). in index form. Also find deg(omega).
     */
    deg_omega = 0;
    for (i = 0; i < NROOTS;i++)
    {
        tmp = 0;
        j = (deg_lambda < i) ? deg_lambda : i;
        for (;j >= 0; j--)
        {
            if ((s[i - j] != A0) && (lambda[j] != A0))
                tmp ^= ALPHA_TO[MODNN(s[i - j] + lambda[j])];
        }
        if (tmp != 0)
            deg_omega = i;
        omega[i] = INDEX_OF[tmp];
    }
    omega[NROOTS] = A0;

    /*
     * Compute error values in poly-form. num1 = omega(inv(X(l))), num2 =
     * inv(X(l))**(FCR-1) and den = lambda_pr(inv(X(l))) all in poly-form
     */
    for (j = count-1; j >=0; j--)
    {
        num1 = 0;
        for (i = deg_omega; i >= 0; i--)
        {
            if (omega[i] != A0)
                num1  ^= ALPHA_TO[MODNN(omega[i] + i * root[j])];
        }
        num2 = ALPHA_TO[MODNN(root[j] * (FCR - 1) + NN)];
        den = 0;

        /* lambda[i+1] for i even is the formal derivative lambda_pr of lambda[i] */
        for (i = min(deg_lambda,NROOTS-1) & ~1; i >= 0; i -=2)
        {
            if (lambda[i+1] != A0)
                den ^= ALPHA_TO[MODNN(lambda[i+1] + i * root[j])];
        }
        if (den == 0)
        {
#if DEBUG >= 1
            printf("\n ERROR: denominator = 0\n");
#endif
            count = -1;
            goto finish;
        }
        /* Apply error to data */
        if (num1 != 0)
        {
            data[loc[j]] ^= ALPHA_TO[MODNN(INDEX_OF[num1] + INDEX_OF[num2] + NN - INDEX_OF[den])];
        }
    }
finish:
    if (eras_pos != NULL)
    {
        for (i=0;i<count;i++)
            eras_pos[i] = loc[i];
    }
    return count;
}
/////////////////////////end init_rs_int_rx, decode_rs_int Reed-Solomon decoder //////////
#include "../Hv_Lib_fftw/fftw3.h"

void DecoderMs::Set65DeepSearchDb(QStringList lst)
{
    db_call_loc4_list = lst;
    //qDebug()<<"db_call_loc4_list="<<db_call_loc4_list.count();//<<db_call_loc_list.at(0);
}
void DecoderMs::hint65(double s3_[63][64],int nadd,int nflip,QString mycall,QString hiscall,
                       QString hisgrid,double &qual,QString &decoded)
{
    QString msg = "";
    int MAXRPT = 63;
    int MAXCALLS = db_call_loc4_list.count();
    int MAXMSG = (2*MAXCALLS + 2 + MAXRPT);
    //wsjt-x max=20065 maxcalls=10000  mshv max=30065 maxcalls=15000
    const int hv_limit = 30065;
    if (MAXMSG>hv_limit)
    {
        MAXMSG = hv_limit;
        MAXCALLS = (MAXMSG - (2 + MAXRPT))/2;
    }
    QString msg0[hv_limit+40];
    //qDebug()<<"MAXMSG ="<<MAXMSG<<MAXCALLS<<db_call_loc4_list.count();

    QString rpt[63]
    {
        "-01","-02","-03","-04","-05",
        "-06","-07","-08","-09","-10",
        "-11","-12","-13","-14","-15",
        "-16","-17","-18","-19","-20",
        "-21","-22","-23","-24","-25",
        "-26","-27","-28","-29","-30",
        "R-01","R-02","R-03","R-04","R-05",
        "R-06","R-07","R-08","R-09","R-10",
        "R-11","R-12","R-13","R-14","R-15",
        "R-16","R-17","R-18","R-19","R-20",
        "R-21","R-22","R-23","R-24","R-25",
        "R-26","R-27","R-28","R-29","R-30",
        "RO","RRR","73"
    };
    int dgen[13]; //12
    for (int z = 0; z < 13; z++)
        dgen[z] = 0;
    int sym_rev[63];
    //int sym[63];//](0:62)

    int (*sym2_)[63]=new int[MAXMSG+10][63];
    /*for (int i = 0; i < MAXMSG; i++)
    {
        for (int j = 0; j < 63; j++)
            sym2_[i][j] = 0;
    }*/

    QString msg00;

    //! NB: generation of test messages is not yet complete!
    int jj=0;
    for (int i = -2; i < MAXCALLS; i++)
    {//do i=-1,ncalls

        if (i==-1 && hiscall.isEmpty() && hisgrid.isEmpty()) continue;//if(i.eq.0 .and. hiscall.eq.'      ' .and. hisgrid(1:4).eq.'    ') cycle
        int mz=2;
        if (i==-2) mz=1;//?? if(i.eq.-1) mz=1
        if (i==-1) mz=65;//?? if(i.eq.0) mz=65
        for (int m = 1; m <= mz; m++)
        {//do m=1,mz
            //j=j+1
            if (i==-2) //if(i.eq.-1) then
                msg="0123456789ABC";
            else if (i==-1) //else if(i.eq.0) then //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
            {
                if (m==1) msg=mycall+" "+hiscall+" "+hisgrid.mid(0,4); //if(m.eq.1) msg=mycall//' '//hiscall//' '//hisgrid(1:4)
                if (m==2) msg="CQ "+hiscall+" "+hisgrid.mid(0,4);     //if(m.eq.2) msg='CQ '//hiscall//' '//hisgrid(1:4)
                if (m>=3) msg=mycall+" "+hiscall+" "+rpt[m-3];        //if(m.ge.3) msg=mycall//' '//hiscall//' '//rpt(m-2)
            }
            else
            {
                QString t_cal_loc4 = db_call_loc4_list.at(i);
                QStringList call_loc4 = t_cal_loc4.split(",");
                //QString loc_4 = call_loc.at(1).mid(0,4);
                if (m==1)  msg=mycall+" "+call_loc4.at(0)+" "+call_loc4.at(1); //if(m.eq.1)  msg=mycall//' '//call2(i)//' '//grid2(i)
                if (m==2)  msg="CQ "+call_loc4.at(0)+" "+call_loc4.at(1);     //if(m.eq.2)  msg='CQ '//call2(i)//' '//grid2(i)
            }
            //call fmtmsg(msg,iz)
            int itype = -1;//??
            char cmsg[50];
            for (int x = 0; x < 50; x++)
            {
                if (x < msg.count())
                    cmsg[x]=msg.at(x).toLatin1();
                else
                    cmsg[x]=' ';
            }

            TGen65->packmsg(cmsg,dgen,itype);            //!Pack message into 72 bits
            TGen65->rs_encode(dgen,sym_rev);            //!RS encode
            /*for (int x = 0; x < 63; x++)
            {
                sym[x]=sym_rev[62-x]; //sym(0:62)=sym_rev(62:0:-1)
            }*/
            //sym1(0:62,j)=sym

            TGen65->interleave63(sym_rev,1);            //!Interleave channel symbols
            TGen65->graycode(sym_rev,63,1,sym_rev);     //!Apply Gray code
            for (int x = 0; x < 63; x++)
                sym2_[jj][x]=sym_rev[x];//sym2(0:62,j)=sym_rev(0:62)
            msg0[jj]=msg;
            //qDebug()<<"msg0 =============="<<j<<msg0[j];
            jj++;
        }
    }
    int nused=jj;
    //qDebug()<<"nused ="<<nused;

    double ref0=0.0;
    for (int j=0; j<63; j++)
    {//do j=1,63
        ref0+=s3_[j][mrs_jt65[j]];//+1 ??   ref0=ref0 + s3(mrs(j)+1,j)
    }

    double u1=-99.0;//u1=0.
    //u1=-99.0;     //u1=-99.0
    double u2=u1;

    //! Find u1 and u2 (best and second-best) codeword from a list, using
    //! a bank of matched filters on the symbol spectra s3(i,j).
    int ipk=0; //hv 0 ipk=1;
    //int ipk2=0;//ipk2=0;
    msg00="";//msg00=' '
    qual = -99.0;

    int parts_max = 4000;//max calls 3000 - 4000 5000->no tested HV min is inportent min ->66
    int real_nused = nused + (nused/(parts_max-66))*66;//+nused/parts_max;
    int array_max_qual_ipk = (real_nused/parts_max)+21;
    int cou_qual_ipk = 0;
    double *qual_part = new double[array_max_qual_ipk+5];
    int *ipk_part = new int[array_max_qual_ipk+5];
    //qDebug()<<"array_max_qual_ipk"<<array_max_qual_ipk;
    int parts_cou = 0;
    int s_last_k = 66;


    //int ttt=0;
    for (int k=0; k<nused; k++)//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    {
        if (k==66)
            k=s_last_k;
        //qDebug()<<"hint65 msgs="<<msg0[k]<<k;

        //if(k.ge.2 .and. k.le.64 .and. nflip.lt.0) cycle
        if (k>=2 && k<=65 && nflip<0) continue;//mshv v1.49 tested 2-65 exclude ->cq hiscall loc, RPTs RO RRR 73
        //! Test all messages if nflip=+1; skip the CQ messages if nflip=-1.
        if (nflip>0 || msg0[k].mid(0,3)!="CQ ") //if(nflip.gt.0 .or. msg0(k)(1:3).ne.'CQ ') then
        {
            //qDebug()<<"hint65=TestMSGs"<<msg0[k];
            double psum=0.0;
            double ref=ref0;
            for (int j=0; j<63; j++)
            {
                int i=sym2_[k][j]; //i=sym2(j-1,k)+1;
                psum += s3_[j][i]; //psum=psum + s3(i,j)
                //if(i.eq.mrs(j)+1) ref=ref - s3(i,j) + s3(mrs2(j)+1,j)
                if (i==mrs_jt65[j]) ref -= (s3_[j][i] + s3_[j][mrs2_jt65[j]]);
            }

            double p=psum/ref;
            if (p>u1)
            {
                if (msg0[k]!=msg00)
                {
                    //ipk2=ipk;
                    u2=u1;
                }
                u1=p;
                ipk=k;
                msg00=msg0[k];
            }
            if (msg0[k]!=msg00 && p>u2)
            {
                u2=p;
                //ipk2=k;
            }

            if (parts_cou >= parts_max)
            {
                //qDebug()<<"hint65 msgs="<<msg0[k]<<k<<cou_qual_ipk<<array_max_qual_ipk;
                double p_bias=fmax(1.12*u2,0.35);
                if (nadd>=4) p_bias=fmax(1.08*u2,0.45);
                if (nadd>=8) p_bias=fmax(1.04*u2,0.60);
                double p_qual=100.0*(u1-p_bias);
                ipk_part[cou_qual_ipk]=ipk;
                qual_part[cou_qual_ipk]=p_qual;
                cou_qual_ipk++;

                parts_cou = 0;
                u1=-99.0;
                u2=u1;
                s_last_k = k+1;
                k = -1;
            }
            else
                parts_cou++;

            //ttt++;
        }
    }
    //qDebug()<<"hint65=ttttttttttttttt"<<ttt;
    double fin_bias=fmax(1.12*u2,0.35);
    if (nadd>=4) fin_bias=fmax(1.08*u2,0.45);
    if (nadd>=8) fin_bias=fmax(1.04*u2,0.60);
    double fin_qual=100.0*(u1-fin_bias);
    ipk_part[cou_qual_ipk]=ipk;
    qual_part[cou_qual_ipk]=fin_qual;
    cou_qual_ipk++;
    //qDebug()<<"hint65=LAST parts_cou"<<parts_cou;

    for (int x=0; x<cou_qual_ipk; x++)// find max qual and his ipk
    {
        if (qual_part[x]>qual)
        {
            qual = qual_part[x];
            ipk = ipk_part[x];
            //qDebug()<<"XXX="<<qual_part[x]<<msg0[ipk_part[x]]<<x;
        }
    }

    decoded="";//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    /*double bias=fmax(1.12*u2,0.35);
    if (nadd>=4) bias=fmax(1.08*u2,0.45);
    if (nadd>=8) bias=fmax(1.04*u2,0.60);
    qual=100.0*(u1-bias);*/

    if (qual>=1.0) decoded=msg0[ipk];
    //qDebug()<<"hint65 decoded="<<qual<<"=="<<nadd<<decoded<<nused<<real_nused<<cou_qual_ipk<<array_max_qual_ipk<<nflip;

    delete [] qual_part;
    delete [] ipk_part;
    delete [] sym2_;
}

/* po bavno e taka hv 1.46
void DecoderMs::first_subtract65()
{
    int NMAX=60*DEC_SAMPLE_RATE;
    int NFILT=1600;
    int nfft=564480;

    double window_p[1600+40];  //double window(-NFILT/2:NFILT/2)
    double *window = &window_p[800+20];
    //! Create and normalize the filter
    double sum=0.0;
    for (int j = -NFILT/2; j<NFILT/2; j++)
    {//do j=-NFILT/2,NFILT/2
        window[j]=cos(pi*(double)j/(double)NFILT)*cos(pi*(double)j/(double)NFILT);    //cos(pi*(double)j/(double)NFILT)**2
        sum+=window[j];
    }
    zero_double_comp_beg_end(cw_jt65,0,NMAX);// my e drugo kato na ft8 ina4e * po nula
    for (int i = -NFILT/2; i<NFILT/2; i++)
    {//do i=-NFILT/2,NFILT/2
        int j=i+1; //int j=i+1;
        if (j<0) j=j+nfft; //if(j<1) j=j+nfft;
        cw_jt65[j]=window[i]/sum;
    }
    four2a_compex_to_cmplex(cw_jt65,nfft,1,-1,1);
}
*/
/*static const int nprc[126]=
    {
        1,0,0,1,1,0,0,0,1,1,1,1,1,1,0,1,0,1,0,0,
        0,1,0,1,1,0,0,1,0,0,0,1,1,1,0,0,1,1,1,1,
        0,1,1,0,1,1,1,1,0,0,0,1,1,0,1,0,1,0,1,1,
        0,0,1,1,0,1,0,1,0,1,0,0,1,0,0,0,0,0,0,1,
        1,0,0,0,0,0,0,0,1,1,0,1,0,0,1,0,1,1,0,1,
        0,1,0,1,0,0,1,1,0,0,1,0,0,1,0,0,0,0,1,1,
        1,1,1,1,1,1
    };*/
void DecoderMs::subtract65(double *dd,int npts,double f0,double dt)
{
    //qDebug()<<"1subtract65-->";
    int NMAX=60*DEC_SAMPLE_RATE;//=720000
    int nprc[126]={1,0,0,1,1,0,0,0,1,1,1,1,1,1,0,1,0,1,0,0,
                   0,1,0,1,1,0,0,1,0,0,0,1,1,1,0,0,1,1,1,1,
                   0,1,1,0,1,1,1,1,0,0,0,1,1,0,1,0,1,0,1,1,
                   0,0,1,1,0,1,0,1,0,1,0,0,1,0,0,0,0,0,0,1,
                   1,0,0,0,0,0,0,0,1,1,0,1,0,0,1,0,1,1,0,1,
                   0,1,0,1,0,0,1,1,0,0,1,0,0,1,0,0,0,0,1,1,
                   1,1,1,1,1,1};
    //int correct[63];
    //double complex cref[NMAX];
    double complex *cref = new double complex[721000];
    //double complex camp[NMAX];
    double complex *camp = new double complex[721000];
    //double complex cfilt[60*12000];
    double complex *cfilt = new double complex[721000];

    //qDebug()<<"2subtract65-->";
    const int NFILT=1600;
    double window_p[1600+40];  //double window(-NFILT/2:NFILT/2)
    double *window = &window_p[800+20];

    int nstart=dt*DEC_SAMPLE_RATE+0;//nstart=dt*12000+1;
    int nsym=126;
    int ns=4458;
    int nref=nsym*ns;//=561708
    //int nend=nstart+nref-0;//nend=nstart+nref-1;
    double phi=0.0;
    //int iref=1;
    int ind=0;//1
    int isym=1;


    for (int k = 0; k<nsym; k++)//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    {//do k=1,nsym
        double omega = 0.0;
        if ( nprc[k] == 1 )
            omega=2.0*pi*f0;
        else
        {
            omega=2.0*pi*(f0+2.6917*(correct_jt65[isym]+2));
            isym++;
        }
        double dphi=omega/DEC_SAMPLE_RATE;
        for (int i = 0; i<ns; i++)
        {//do i=1,ns
            cref[ind]=cexp(0.0+phi*I);//cref(ind)=cexp(cmplx(0.0,phi))
            phi=fmod(phi+dphi,2.0*pi);//(phi+dphi % 2.0*pi);  //modulo(phi+dphi,2*pi);
            int id=nstart-0+ind; //id=nstart-1+ind
            if (id>=1) camp[ind]=dd[id]*conj(cref[ind]);
            ind++;
        }
    }
    //qDebug()<<cimag(cref[nstart])<<cimag(cref[nstart+1])<<cimag(cref[nstart+2]);
    //! Smoothing filter: do the convolution by means of FFTs. Ignore end-around
    //! cyclic effects for now.
    int nfft=564480;

    if (first_subtract65)
    {
        //! Create and normalize the filter
        double sum=0.0;
        for (int j = -NFILT/2; j<NFILT/2; j++)
        {//do j=-NFILT/2,NFILT/2
            window[j]=cos(pi*(double)j/(double)NFILT)*cos(pi*(double)j/(double)NFILT);    //cos(pi*(double)j/(double)NFILT)**2
            sum+=window[j];
        }
        pomAll.zero_double_comp_beg_end(cw_jt65,0,NMAX);
        for (int i = -NFILT/2; i<NFILT/2; i++)
        {//do i=-NFILT/2,NFILT/2
            int j=i+1; //int j=i+1;
            if (j<0) j=j+nfft; //if(j<1) j=j+nfft;

            if (sum==0.0) // no devide by zero
                sum=1.0;
            cw_jt65[j]=window[i]/sum;
        }
        f2a.four2a_c2c(cw_jt65,nfft,-1,1);
        first_subtract65=false;
    }

    int nz=561708;
    for (int i = 0; i<nz; i++)
        cfilt[i]=camp[i];//cfilt(1:nz)=camp(1:nz)
    for (int i = nz; i<nfft; i++)
        cfilt[i]=0.0+0.0*I;//cfilt(nz+1:nfft)=0.
    f2a.four2a_c2c(cfilt,nfft,-1,1);
    double fac=1.0/double(nfft);
    for (int i = 0; i<nfft; i++)
        cfilt[i]=fac*cfilt[i]*cw_jt65[i];//cfilt(1:nfft)=fac*cfilt(1:nfft)*cw(1:nfft)
    f2a.four2a_c2c(cfilt,nfft,1,1);

    //! Subtract the reconstructed signal
    for (int i = 0; i<nref; i++)//nref=561708  c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    {//do i=1,nref
        int j=nstart+i-1; //j=nstart+i-1
        if (j>=0 && j<npts) //if(j>=1 && j<=npts)   npts=720000
            dd[j]=dd[j]-2.0*creal(cfilt[i]*cref[i]);
    }

    //double dd66[721000];
    /*double *dd66 = new double[721000];
    int kstop = 0; 	
    for (int i = 0; i < nref; ++i)
    {
       int j=nstart+i-1;
       if (j>=0 && j<npts) 
       {
       	 dd66[j]=2.0*creal(cfilt[i]*cref[i]);
       	 kstop=j;
       }	
    }    
    int kstart = nstart-1; 
    if (kstart<0) kstart=0;
    for (int i = kstart; i < kstop+1; ++i) 
    dd[i]-=dd66[i]; 
    delete dd66;	
    qDebug()<<kstart<<kstop;*/

    delete [] cref;
    delete [] camp;
    delete [] cfilt;
}

void DecoderMs::getpp_(int *workdat, float &p)
{
    /*use jt65_mod
    integer workdat(63)
    integer a(63)*/
    int aaz[63];
    int k = 0;

    for (int i=62; i>=0; i--)
    {
        aaz[k]=workdat[i];//a(1:63)=workdat(63:1:-1)
        k++;
    }

    TGen65->interleave63(aaz,1);
    TGen65->graycode(aaz,63,1,aaz);

    double psum=0.0;
    for (int j=0; j<63; j++)
    {  //do j=1,63
        int i=aaz[j]+0;            //i=a[j]+1;
        double x=s3a_jt65_[j][i];   //x=s3a(i,j);   double s3a_jt65_[63][64];
        //s3a_jt65_[j][i]=0.0; //s3a[i,j]=0.0;
        psum=psum + x;       //psum=psum + x;
        s3a_jt65_[j][i]=x;   //??? s3a(i,j)=x;
    }
    p=psum/63.0;
}
/*
void DecoderMs::ftrsd2(int *mrsym, int *mrprob, int *mr2sym, int *mr2prob,
                       int ntrials0, int *correct, int *param, int *ntry)
{
    int rxdat[63], rxprob[63], rxdat2[63], rxprob2[63];
    int workdat[63];
    int indexes[63];
    int era_pos[51];
    int i, j, numera, nerr, nn=63;
    int ntrials = ntrials0;
    int nhard=0,nhard_min=32768,nsoft=0,nsoft_min=32768;
    int ntotal=0,ntotal_min=32768,ncandidates;
    int nera_best=0;
    float pp,pp1,pp2;
    static unsigned int nseed;

    // Power-percentage symbol metrics - composite gnnf/hf
    int perr[8][8] =
        {
            {
                4,      9,     11,     13,     14,     14,     15,     15
            },
            { 2,     20,     20,     30,     40,     50,     50,     50},
            { 7,     24,     27,     40,     50,     50,     50,     50},
            {13,     25,     35,     46,     52,     70,     50,     50},
            {17,     30,     42,     54,     55,     64,     71,     70},
            {25,     39,     48,     57,     64,     66,     77,     77},
            {32,     45,     54,     63,     66,     75,     78,     83},
            {51,     58,     57,     66,     72,     77,     82,     86}
        };

    // Initialize the KA9Q Reed-Solomon encoder/decoder
    unsigned int symsize=6, gfpoly=0x43, fcr=3, prim=1, nroots=51;
    rs=init_rs_int_rx(symsize, gfpoly, fcr, prim, nroots, 0);
    //rs=init_rs_int_rx(symsize, gfpoly, fcr, prim, nroots);

    // Reverse the received symbol vectors for BM decoder
    for (i=0; i<63; i++)
    {
        rxdat[i]=mrsym[62-i];
        rxprob[i]=mrprob[62-i];
        rxdat2[i]=mr2sym[62-i];
        rxprob2[i]=mr2prob[62-i];
    }

    // Sort rxprob to find indexes of the least reliable symbols
    int k, pass, tmp, nsym=63;
    int probs[63];
    for (i=0; i<63; i++)
    {
        indexes[i]=i;
        probs[i]=rxprob[i];
    }
    for (pass = 1; pass <= nsym-1; pass++)
    {
        for (k = 0; k < nsym - pass; k++)
        {
            if ( probs[k] < probs[k+1] )
            {
                tmp = probs[k];
                probs[k] = probs[k+1];
                probs[k+1] = tmp;
                tmp = indexes[k];
                indexes[k] = indexes[k+1];
                indexes[k+1] = tmp;
            }
        }
    }

    // See if we can decode using BM HDD, and calculate the syndrome vector.
    memset(era_pos,0,51*sizeof(int));
    numera=0;
    memcpy(workdat,rxdat,sizeof(rxdat));
    nerr=decode_rs_int(rs,workdat,era_pos,numera,1);
    //nerr=decode_rs_int(rs,workdat,era_pos,numera);
    //qDebug()<<"nerr==============="<<nerr;
    if ( nerr >= 0 )
    {
        // Hard-decision decoding succeeded.  Save codeword and some parameters.
        nhard=0;
        for (i=0; i<63; i++)
        {
            if ( workdat[i] != rxdat[i] ) nhard=nhard+1;
        }
        memcpy(correct,workdat,63*sizeof(int));
        param[0]=0;
        param[1]=nhard;
        param[2]=0;
        param[3]=0;
        param[4]=0;
        param[5]=0;
        param[7]=1000*1000;
        ntry[0]=0;
        return;
    }


    nseed=1;                                 //Seed for random numbers
    float ratio;
    int thresh, nsum;
    int thresh0[63];
    ncandidates=0;
    nsum=0;
    int ii,jj;

    for (i=0; i<nn; i++)
    {
        nsum=nsum+rxprob[i];
        j = indexes[62-i];
        ratio = (float)rxprob2[j]/((float)rxprob[j]+0.01);
        ii = 7.999*ratio;
        jj = (62-i)/8;
        thresh0[i] = 1.3*perr[ii][jj];
    }
    if (nsum<=0) return;

    pp1=0.0;
    pp2=0.0;
    //qDebug()<<"pp================"<<ntrials;
    for (k=1; k<=ntrials; k++)
    {
        memset(era_pos,0,51*sizeof(int));
        memcpy(workdat,rxdat,sizeof(rxdat));


        numera=0;
        for (i=0; i<nn; i++)
        {
            j = indexes[62-i];
            thresh=thresh0[i];
            long int ir;

            // Generate a random number ir, 0 <= ir < 100 (see POSIX.1-2001 example).
            nseed = nseed * 1103515245 + 12345;
            ir = (unsigned)(nseed/65536) % 32768;
            ir = (100*ir)/32768;

            if ((ir < thresh ) && numera < 51)
            {
                era_pos[numera]=j;
                numera=numera+1;
            }
        }

        nerr=decode_rs_int(rs,workdat,era_pos,numera,0);
        //nerr=decode_rs_int(rs,workdat,era_pos,numera);
        //qDebug()<<"pp==================================nerr===nerr>=0->"<<nerr;
        if ( nerr >= 0 )
        {
            // We have a candidate codeword.  Find its hard and soft distance from
            // the received word.  Also find pp1 and pp2 from the full array
            // s3(64,63) of synchronized symbol spectra.
            ncandidates=ncandidates+1;
            nhard=0;
            nsoft=0;
            for (i=0; i<63; i++)
            {
                if (workdat[i] != rxdat[i])
                {
                    nhard=nhard+1;
                    if (workdat[i] != rxdat2[i])
                    {
                        nsoft=nsoft+rxprob[i];
                    }
                }
            }
            nsoft=63*nsoft/nsum;
            ntotal=nsoft+nhard;

            getpp_(workdat,pp); //getpp_(workdat,&pp);
            //qDebug()<<"pp================"<<pp;
            if (pp>pp1)
            {
                pp2=pp1;
                pp1=pp;
                nsoft_min=nsoft;
                nhard_min=nhard;
                ntotal_min=ntotal;
                memcpy(correct,workdat,63*sizeof(int));
                nera_best=numera;
                ntry[0]=k;
            }
            else
            {
                if (pp>pp2 && pp!=pp1) pp2=pp;
            }
            if (nhard_min <= 41 && ntotal_min <= 71) break;
        }
        if (k == ntrials) ntry[0]=k;
    }

    param[0]=ncandidates;
    param[1]=nhard_min;
    param[2]=nsoft_min;
    param[3]=nera_best;
    param[4]=1000.0*pp2/pp1;
    param[5]=ntotal_min;
    param[6]=ntry[0];
    param[7]=1000.0*pp2;
    param[8]=1000.0*pp1;
    if (param[0]==0) param[2]=-1;
    return;
}
*/
void DecoderMs::ftrsd2_ap(int *mrsym, int *mrprob, int *mr2sym, int *mr2prob,
                          bool f_ap_d,int *ap,int ntrials0, int *correct, int *param, int *ntry)
{
    int rxdat[63], rxprob[63], rxdat2[63], rxprob2[63];
    int workdat[63];
    int indexes[63];
    int era_pos[51];
    int i, j, numera, nerr, nn=63;
    int ntrials = ntrials0;
    //int nhard=0;
    //int nsoft=0;
    //int ntotal=0;
    int nhard_min=32768;
    int nsoft_min=32768;
    int ntotal_min=32768;
    int ncandidates;
    int nera_best=0;
    float pp,pp1,pp2;
    static unsigned int nseed;

    // Power-percentage symbol metrics - composite gnnf/hf
    int perr[8][8] =
        {
            {
                4,      9,     11,     13,     14,     14,     15,     15
            },
            { 2,     20,     20,     30,     40,     50,     50,     50},
            { 7,     24,     27,     40,     50,     50,     50,     50},
            {13,     25,     35,     46,     52,     70,     50,     50},
            {17,     30,     42,     54,     55,     64,     71,     70},
            {25,     39,     48,     57,     64,     66,     77,     77},
            {32,     45,     54,     63,     66,     75,     78,     83},
            {51,     58,     57,     66,     72,     77,     82,     86}
        };

    // Initialize the KA9Q Reed-Solomon encoder/decoder
    unsigned int symsize=6, gfpoly=0x43, fcr=3, prim=1, nroots=51;
    rs=init_rs_int_rx(symsize, gfpoly, fcr, prim, nroots, 0);
    //rs=init_rs_int_rx(symsize, gfpoly, fcr, prim, nroots);

    // Reverse the received symbol vectors for BM decoder
    for (i=0; i<63; i++)
    {
        rxdat[i]=mrsym[62-i];
        rxprob[i]=mrprob[62-i];
        rxdat2[i]=mr2sym[62-i];
        rxprob2[i]=mr2prob[62-i];
    }

    //AP Set ap symbols and ap mask
    if (f_ap_d)
    {
        for (i=0; i<12; i++)
        {
            if (ap[i]>=0)
            {
                rxdat[11-i]=ap[i];
                rxprob2[11-i]=-1;
            }
        }
    }
    //END AP

    // Sort rxprob to find indexes of the least reliable symbols
    int k, pass, tmp, nsym=63;
    int probs[63];
    for (i=0; i<63; i++)
    {
        indexes[i]=i;
        probs[i]=rxprob[i];
    }
    for (pass = 1; pass <= nsym-1; pass++)
    {
        for (k = 0; k < nsym - pass; k++)
        {
            if ( probs[k] < probs[k+1] )
            {
                tmp = probs[k];
                probs[k] = probs[k+1];
                probs[k+1] = tmp;
                tmp = indexes[k];
                indexes[k] = indexes[k+1];
                indexes[k+1] = tmp;
            }
        }
    }

    // See if we can decode using BM HDD, and calculate the syndrome vector.
    memset(era_pos,0,51*sizeof(int));
    numera=0;
    memcpy(workdat,rxdat,sizeof(rxdat));
    nerr=decode_rs_int(rs,workdat,era_pos,numera,1);
    //nerr=decode_rs_int(rs,workdat,era_pos,numera);
    //qDebug()<<"nerr==============="<<nerr;
    if ( nerr >= 0 )
    {
        // Hard-decision decoding succeeded.  Save codeword and some parameters.
        int nhard=0;
        for (i=0; i<63; i++)
        {
            if ( workdat[i] != rxdat[i] ) nhard=nhard+1;
        }
        memcpy(correct,workdat,63*sizeof(int));
        param[0]=0;
        param[1]=nhard;
        param[2]=0;
        param[3]=0;
        param[4]=0;
        param[5]=0;
        param[7]=1000*1000;
        ntry[0]=0;
        return;
    }

    /*
    Hard-decision decoding failed.  Try the FT soft-decision method.
    Generate random erasure-locator vectors and see if any of them
    decode. This will generate a list of "candidate" codewords.  The
    soft distance between each candidate codeword and the received
    word is estimated by finding the largest (pp1) and second-largest
    (pp2) outputs from a synchronized filter-bank operating on the
    symbol spectra, and using these to decide which candidate
    codeword is "best".
    */

    nseed=1;                                 //Seed for random numbers
    float ratio;
    int thresh, nsum;
    int thresh0[63];
    ncandidates=0;
    nsum=0;
    int ii,jj;

    if (f_ap_d)
    {
        for (i=0; i<nn; i++)
        {
            nsum=nsum+rxprob[i];
            j = indexes[62-i];
            if ( rxprob2[j]>=0 )
            {
                ratio = (float)rxprob2[j]/((float)rxprob[j]+0.01);
                ii = 7.999*ratio;
                jj = (62-i)/8;
                thresh0[i] = 1.3*perr[ii][jj];
            }
            else
            {
                thresh0[i] = 0.0;
            }
            //printf("%d %d %d\n",i,j,rxdat[i]);
        }
    }
    else
    {
        for (i=0; i<nn; i++)
        {
            nsum=nsum+rxprob[i];
            j = indexes[62-i];
            ratio = (float)rxprob2[j]/((float)rxprob[j]+0.01);
            ii = 7.999*ratio;
            jj = (62-i)/8;
            thresh0[i] = 1.3*perr[ii][jj];
        }
    }
    if (nsum<=0) return;

    pp1=0.0;
    pp2=0.0;
    //qDebug()<<"pp================"<<ntrials;
    for (k=1; k<=ntrials; k++)
    {
        memset(era_pos,0,51*sizeof(int));
        memcpy(workdat,rxdat,sizeof(rxdat));

        /*
        Mark a subset of the symbols as erasures.
        Run through the ranked symbols, starting with the worst, i=0.
        NB: j is the symbol-vector index of the symbol with rank i.
        */
        numera=0;
        for (i=0; i<nn; i++)
        {
            j = indexes[62-i];
            thresh=thresh0[i];
            long int ir;

            // Generate a random number ir, 0 <= ir < 100 (see POSIX.1-2001 example).
            nseed = nseed * 1103515245 + 12345;
            ir = (unsigned)(nseed/65536) % 32768;
            ir = (100*ir)/32768;

            if ((ir < thresh ) && numera < 51)
            {
                era_pos[numera]=j;
                numera=numera+1;
            }
        }

        nerr=decode_rs_int(rs,workdat,era_pos,numera,0);
        //nerr=decode_rs_int(rs,workdat,era_pos,numera);
        //qDebug()<<"pp==================================nerr===nerr>=0->"<<nerr;
        if ( nerr >= 0 )
        {
            // We have a candidate codeword.  Find its hard and soft distance from
            // the received word.  Also find pp1 and pp2 from the full array
            // s3(64,63) of synchronized symbol spectra.
            ncandidates=ncandidates+1;
            int nhard=0;
            int nsoft=0;
            for (i=0; i<63; i++)
            {
                if (workdat[i] != rxdat[i])
                {
                    nhard=nhard+1;
                    if (workdat[i] != rxdat2[i])
                    {
                        nsoft=nsoft+rxprob[i];
                    }
                }
            }
            nsoft=63*nsoft/nsum;
            int ntotal=nsoft+nhard;

            getpp_(workdat,pp); //getpp_(workdat,&pp);
            //qDebug()<<"pp================"<<pp;
            if (pp>pp1)
            {
                pp2=pp1;
                pp1=pp;
                nsoft_min=nsoft;
                nhard_min=nhard;
                ntotal_min=ntotal;
                memcpy(correct,workdat,63*sizeof(int));
                nera_best=numera;
                ntry[0]=k;
            }
            else
            {
                if (pp>pp2 && pp!=pp1) pp2=pp;
            }
            if (nhard_min <= 41 && ntotal_min <= 71) break;
        }
        if (k == ntrials) ntry[0]=k;
    }

    param[0]=ncandidates;
    param[1]=nhard_min;
    param[2]=nsoft_min;
    param[3]=nera_best;
    param[4]=1000.0*pp2/pp1;
    param[5]=ntotal_min;
    param[6]=ntry[0];
    param[7]=1000.0*pp2;
    param[8]=1000.0*pp1;
    if (param[0]==0) param[2]=-1;
    return;
}

void DecoderMs::chkhist(int *mrsym,int &nmax,int &ipk)
{
    int hist[65+5];//1.54= crach linux i686 old -> int hist[63];
    //hist=0
    zero_int_beg_end(hist,0,65); //1.54= crach linux i686   hist=0
    for (int j =0; j<63; j++)
    {//do j=1,63
        int i=mrsym[j];//mrsym from 0-63
        hist[i]=hist[i]+1; //hist[i]=hist[i]+1;
    }
    nmax=0;
    for (int i =0; i<64; i++)////1.54=64 no 63 crach linux i686
    {//do i=0,63
        if (hist[i]>nmax)
        {
            nmax=hist[i];
            ipk=i; //ipk=i+1; ipk e colona v array s 62 mesta 63 count
        }
    }
}

void DecoderMs::demod64a(double s3_[63][64],int nadd,int *mrsym,int *mrprob,
                         int *mr2sym,int *mr2prob,int &ntest,int &nlow)
{
    /*! Demodulate the 64-bin spectra for each of 63 symbols in a frame.

    ! Parameters
    !    nadd     number of spectra already summed
    !    mrsym    most reliable symbol value
    !    mr2sym   second most likely symbol value
    !    mrprob   probability that mrsym was the transmitted value
    !    mr2prob  probability that mr2sym was the transmitted value
    */
    //double fs[64];no used 1.52
    //zero_double_beg_end(fs,0,64);

    if (nadd==-999) return;
    ///nfft=fmin(pow(2,n),(1024.0*1024.0));//    nfft=min(2**n,1024*1024)
    //double afac=afac1 * (double)pow(nadd,0.64);  //afac=afac1 * float(nadd)**0.64 no used 1.52
    double scale=254.999; //1.53=254.999; other=254.999;

    //! Compute average spectral value
    /*double ave = 0.0; //no used 1.52

    for (int j =0; j<63; j++)
    {
        for (int i =0; i<64; i++)
            ave+=s3_[j][i];  //hv ???? ave=sum(s3)/(64.*63.)
    }
    ave = ave/(64.0*63.0);*/

    int i1=0;   //i1=1;                                    //!Silence warning
    int i2=0;   //i2=1;

    //! Compute probabilities for most reliable symbol values
    for (int j =0; j<63; j++)
    {//do j=1,63
        double s1=-1.e30;
        double psum=0.0;
        for (int i =0; i<64; i++)
        {//do i=1,64
            //double x=fmin(afac*s3_[j][i]/ave,50.0);no used 1.52
            //fs[i]=exp(x);no used 1.52
            psum=psum+s3_[j][i];//(i,j) //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
            if (s3_[j][i]>s1) // then if(s3(i,j).gt.s1) then
            {
                s1=s3_[j][i];//(i,j)
                i1=i;                              //!Most reliable
            }
        }

        if (psum==0.0) psum=1.e-6;

        double s2=-1.e30;
        for (int i =0; i<64; i++)
        {//do i=1,64
            if (i!=i1 && s3_[j][i]>s2) //then
            {
                s2=s3_[j][i];//(i,j)
                i2=i;                              //!Second most reliable
            }
        }
        double p1=s1/psum;                              //!Symbol metrics for ftrsd
        double p2=s2/psum;

        //if(i1>62)//1.54 crash linux i686
        //mrsym[j]=62;
        //else
        mrsym[j]=i1; //mrsym[j]=i1-1

        //if(i2>62)//1.54 crash linux i686
        //mr2sym[j]=62; //mr2sym(j)=i2-1
        //else
        mr2sym[j]=i2;

        mrprob[j]=scale*p1; //mrprob(j)=scale*p1;
        mr2prob[j]=scale*p2; //mr2prob(j)=scale*p2;
    }

    nlow=0;
    for (int j =0; j<63; j++)
    {//do j=1,63
        if (mrprob[j]<=5) nlow=nlow+1; //if(mrprob(j).le.5) nlow=nlow+1
    }
    for (int j =0; j<63; j++)
        ntest+=mrprob[j];
}

static const int nappasses_jt65_[6]=
    {
        3,4,2,3,3,4 //1.70
        //3,4,2,3,3,4)

        //2,2,2,3,3,3//to 1.69
        //2,2,2,4,4,3,3//MSHV +1  CQ LZ2HV 237 for +AP
    };
static const int naptypes_jt65_[6][4]=
    {
        //tx6-7		 tx1	  tx2		tx3       tx4       tx5
        {1,2,6,0},{2,3,6,7},{2,3,0,0},{3,4,5,0},{3,4,5,0},{2,3,4,5}//1.70
        //{1,2,0,0},{2,3,0,0},{2,3,0,0},{3,4,5,0},{3,4,5,0},{3,1,2,0}//1.69
        //{1,2,0,0},{2,3,0,0},{2,3,0,0},{3,4,5,6},{3,4,5,6},{3,1,2,0},{3,1,2,0}//MSHV +1  CQ LZ2HV 237 for +AP
    };
void DecoderMs::extract(double s3_[63][64],int nadd,int mode65,int ntrials,int naggressive,
                        bool f_deep_search,int nflip,QString mycall_12,QString hiscall_12,QString hisgrid,
                        int &ncount,int &nhist,QString &decoded,bool &ltext,int &nft,double &qual,
                        int nQSOProgress,bool ljt65apon)
{
    /*! Input:
    !   s3       64-point spectra for each of 63 data symbols
    !   nadd     number of spectra summed into s3
    !   nqd      0/1 to indicate decode attempt at QSO frequency
    ! Output:
    !   ncount   number of symbols requiring correction (-1 for no KV decode)
    !   nhist    maximum number of identical symbol values
    !   decoded  decoded message (if ncount >=0)
    !   ltext    true if decoded message is free text
    !   nft      0=no decode; 1=FT decode; 2=hinted decode
    */
    QString mycall;
    QString hiscall;
    int mrsym[63];
    int mrprob[63];
    int mr2sym[63];
    int mr2prob[63];
    int ntest=0; //hv ?????????
    int nlow=0;  //hv ?????????
    int ntry[1];
    //int correct[63];
    /////nt ncandidates;
    int nhard=0;
    /////int nsoft;
    /////int nerased;
    int rtt;
    int ntotal=0;
    int nd0;
    double r0;
    double qmin;
    int dat4[12];
    int tmp[63];

    if (mode65==-99) return;                  //!Silence compiler warning
    mycall=mycall_12.mid(0,6);//mycall=mycall_12(1:6)
    hiscall=hiscall_12.mid(0,6);//hiscall=hiscall_12(1:6)
    qual=0.0;
    int nbirdie=20;
    int npct=50;
    //double afac1=1.1;//no use 1.52
    nft=0;
    int nfail=0;// nfail=0
    decoded="";

    //AP JT65 1.55=
    int end_ap_ipass;
    int ap[14];//={-1};//[12]
    /*int apsymbols[6][14];//(5,12)
    for (int i =0; i<6; i++)
    {
        for (int j =0; j<14; j++)
            apsymbols[i][j] = -1;
    }*/
    int itype=1;
    char apmessage[50];
    for (int i =0; i<50;i++)
        apmessage[i]=' ';

    if (ljt65apon && (mycall!=mycall0_jt65ap || hiscall!=hiscall0_jt65ap || hisgrid!=hisgrid0_jt65ap)) //???? ->only if saved->mycall0 if(ljt65apon .and. (mycall.ne.mycall0 .or. hiscall.ne.hiscall0)) then //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    {
        //qDebug()<<"Exstract Calls Changes="<<mycall<<hiscall;
        //apsymbols=-1//no need if is no saved
        for (int i =0; i<7; i++)
        {
            for (int j =0; j<12; j++)
                apsymbols_jt65[i][j] = -1;
        }
        mycall0_jt65ap=mycall;
        hiscall0_jt65ap=hiscall;
        hisgrid0_jt65ap=hisgrid;

        for (int i =0; i<12; i++)
            ap[i]=-1;
        apsymbols_jt65[0][0]=62;
        apsymbols_jt65[0][1]=32;
        apsymbols_jt65[0][2]=32;
        apsymbols_jt65[0][3]=49; //! CQ
        if (!mycall.isEmpty())
        {
            QString msgs = mycall+" "+mycall+" RRR";
            for (int i =0; i<msgs.count(); i++)
                apmessage[i]=msgs.at(i).toLatin1();
            itype=1;
            TGen65->packmsg(apmessage,ap,itype);
            if (itype!=1)
            {
                for (int i =0; i<12; i++)
                    ap[i]=-1;
            }
            for (int i =0; i<4; i++)
                apsymbols_jt65[1][i]=ap[i];

            if (!hiscall.isEmpty())
            {
                msgs = mycall+" "+hiscall+" RRR";
                for (int i =0; i<50;i++)
                    apmessage[i]=' ';
                for (int i =0; i<msgs.count(); i++)
                    apmessage[i]=msgs.at(i).toLatin1();
                itype=1;
                TGen65->packmsg(apmessage,ap,itype);
                if (itype!=1)
                {
                    for (int i =0; i<12; i++)
                        ap[i]=-1;
                }
                for (int i =0; i<9; i++)
                    apsymbols_jt65[2][i]=ap[i];
                for (int i =0; i<12; i++)
                    apsymbols_jt65[3][i]=ap[i];

                msgs = mycall+" "+hiscall+" 73";
                for (int i =0; i<50;i++)
                    apmessage[i]=' ';
                for (int i =0; i<msgs.count(); i++)
                    apmessage[i]=msgs.at(i).toLatin1();
                itype=1;
                TGen65->packmsg(apmessage,ap,itype);
                if (itype!=1)
                {
                    for (int i =0; i<12; i++)
                        ap[i]=-1;
                }
                for (int i =0; i<12; i++)
                    apsymbols_jt65[4][i]=ap[i];
                ////
                if (!hisgrid.isEmpty())//if(len_trim(hisgrid(1:4)).gt.0) then
                {
                    msgs = mycall+" "+hiscall+" "+hisgrid.mid(0,4);//apmessage=mycall//' '//hiscall//' '//hisgrid(1:4)
                    for (int i =0; i<50;i++)//call packmsg(apmessage,ap,itype,.false.)
                        apmessage[i]=' ';
                    for (int i =0; i<msgs.count(); i++)
                        apmessage[i]=msgs.at(i).toLatin1();
                    itype=1;
                    TGen65->packmsg(apmessage,ap,itype);
                    if (itype!=1) //ap=-1
                    {
                        for (int i =0; i<12; i++)
                            ap[i]=-1;
                    }
                    for (int i =0; i<12; i++)//apsymbols(6,:)=ap
                        apsymbols_jt65[5][i]=ap[i];

                    msgs = "CQ "+hiscall+" "+hisgrid.mid(0,4);//apmessage='CQ'//' '//hiscall//' '//hisgrid(1:4)
                    for (int i =0; i<50;i++)//call packmsg(apmessage,ap,itype,.false.)
                        apmessage[i]=' ';
                    for (int i =0; i<msgs.count(); i++)
                        apmessage[i]=msgs.at(i).toLatin1();
                    itype=1;
                    TGen65->packmsg(apmessage,ap,itype);
                    if (itype!=1) //ap=-1
                    {
                        for (int i =0; i<12; i++)
                            ap[i]=-1;
                    }
                    for (int i =0; i<12; i++)//apsymbols(7,:)=ap
                        apsymbols_jt65[6][i]=ap[i];
                }
            }
        }
    }
    //qDebug()<<"ffff"<<mycall<<hiscall;
    //End AP JT65

    double t_s3[4032];
    int zz = 0;
    for (int j =0; j<63; j++)
    {
        //1.54 no need
        /*mrsym[j]=0;//added 1.54= crash linux i686
        mrprob[j]=0;//added 1.54= crash linux i686
        mr2sym[j]=0;//added 1.54= crash linux i686
        mr2prob[j]=0;//added 1.54= crash linux i686*/
        for (int i =0; i<64; i++)
        {
            t_s3[zz]=s3_[j][i];
            zz++;
        }
    }

    //double pctile(double *x,int begin_x,double *tmp,int nmax,double npct);
    double base = pomAll.pctile_shell(t_s3,4032,npct);
    //double tmp1[4032];
    //double base = pctile(t_s3,0,tmp1,4032,npct);
    //qDebug()<<"extract===base111=="<<base<<t_s3[6]<<t_s3[8]<<t_s3[20]<<t_s3[31];
    if (base==0.0)// no devide by zero
        base=1.0;
    for (int j =0; j<63; j++)	     //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    {
        for (int i =0; i<64; i++)
            s3_[j][i]=s3_[j][i]/base; //double s3_[63][64]
    }

    //qDebug()<<"extract===base333000=="<<base<<s3_[0][6]<<s3_[0][8]<<s3_[0][20]<<s3_[0][31];

    for (int j =0; j<63; j++)
    {
        for (int i =0; i<64; i++)
            s3a_jt65_[j][i]=s3_[j][i];       //real s3a(64,63)   s3a_jt65_[63][64]; !###
    }

    //! Get most reliable and second-most-reliable symbol values, and their
    //! probabilities
    //1 call demod64a(s3,nadd,afac1,mrsym,mrprob,mr2sym,mr2prob,ntest,nlow) 1<-???????
    //nadd=2;//qDebug()<<"extract===nadd==="<<nadd;
c1:
    //demod64a(s3_,nadd,afac1,mrsym,mrprob,mr2sym,mr2prob,ntest,nlow);//afac1 no used 1.52
    demod64a(s3_,nadd,mrsym,mrprob,mr2sym,mr2prob,ntest,nlow);

    int ipk = 0;
    chkhist(mrsym,nhist,ipk);       //!Test for birdies and QRM
    //qDebug()<<"END extract===chkhist==="<<nhist<<ipk;

    if (nhist>=nbirdie) //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    {
        //qDebug()<<"extract===chkhist===";
        nfail++; //nfail=nfail+1;
        zz=0;
        for (int j =0; j<63; j++)
        {
            for (int i =0; i<64; i++)
            {
                t_s3[zz]=s3_[j][i];
                zz++;
            }
        }

        base = pomAll.pctile_shell(t_s3,4032,npct);

        for (int j =0; j<63; j++)
            s3_[j][ipk]=base;//s3(ipk,1:63)=base

        if (nfail>30)
        {
            //qDebug()<<"extract===nfail==="<<nfail;
            decoded="";
            ncount=-1;
            goto c900;
        }
        goto c1;
    }
    //qDebug()<<"extract===chkhist==="<<nhist<<ipk;

    for (int i =0; i<63; i++)
    {
        mrs_jt65[i]=mrsym[i];
        mrs2_jt65[i]=mr2sym[i];
    }

    TGen65->graycode65(mrsym,63,-1);        //!Remove gray code
    TGen65->interleave63(mrsym,-1);         //!Remove interleaving
    TGen65->interleave63(mrprob,-1);

    TGen65->graycode65(mr2sym,63,-1);      //!Remove gray code and interleaving
    TGen65->interleave63(mr2sym,-1);       //!from second-most-reliable symbols
    TGen65->interleave63(mr2prob,-1);

    //AP JT65 1.55= //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    end_ap_ipass = 1;
    if (ljt65apon && !mycall.isEmpty())
        end_ap_ipass=1+nappasses_jt65_[nQSOProgress];//3,4,2,3,3,4 <-1.70   //1.69-> 2,2,2,3,3,3

    //qDebug()<<"ffff"<<mycall<<hiscall<<end_ap_ipass;
    for (int ipass =1; ipass<=end_ap_ipass; ipass++)
    {
        for (int i =0; i<12; i++)
            ap[i]=-1;

        int ntype=0; //   					 tx6-7		 tx1	  tx2		tx3       tx4       tx5
        if (ipass>1)//naptypes_jt65_[8][4]={1,2,6,0},{2,3,6,7},{2,3,0,0},{3,4,5,0},{3,4,5,0},{2,3,4,5}//1.70
        {           //naptypes_jt65_[6][4]={1,2,0,0},{2,3,0,0},{2,3,0,0},{3,4,5,0},{3,4,5,0},{3,1,2,0}//1.69
            ntype=naptypes_jt65_[nQSOProgress][ipass-(1+1)];//(nQSOProgress,ipass-1)
            //qDebug()<<"nQSOProgress="<<nQSOProgress<<"ntype_ap="<<ntype-1;
            for (int i =0; i<12; i++)
                ap[i]=apsymbols_jt65[ntype-1][i];
            int count_ap =0;
            for (int i =0; i<12; i++)
            {
                if (ap[i]>=0)
                    count_ap++;
            }
            //qDebug()<<"count_ap"<<count_ap<<ipass;
            if (count_ap==0) continue;
            //if(count(ap.ge.0).eq.0) cycle
        }
        ntry[0]=0;
        zero_int_beg_end(param_jt65,0,10);
        ftrsd2_ap(mrsym,mrprob,mr2sym,mr2prob,ljt65apon,ap,ntrials,correct_jt65,param_jt65,ntry);

        //ncandidates=param(0)
        nhard=param_jt65[1];
        //nsoft=param(2)
        //nerased=param(3)
        rtt=0.001*param_jt65[4];
        ntotal=param_jt65[5];
        qual=0.001*param_jt65[7];
        nd0=81;//81;  hv 77
        r0=0.87;//87;
        if (naggressive==10)//1.59=my be need to remove ->naggressive=0 alwais   c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
        {
            nd0=83;
            r0=0.90;
        }
        //qDebug()<<"1end EXSTRACT=========="<<ntotal<<rtt;
        //Left Shift 	ISHFT 	ISHFT(N,M) (M > 0) 	<< 	n<<m 	n shifted left by m bits
        //Right Shift 	ISHFT 	ISHFT(N,M) (M < 0) 	>> 	n>>m 	n shifted right by m bits
        if (ntotal<=nd0 && rtt<=r0)
        {
            nft=1+(ntype << 2);
            //qDebug()<<"EXSTRACT==nft=nap"<<nft<<nap<<ipass;
        }
        //qDebug()<<"EXSTRACT=========="<<nft<<ipass;
        if (nft>0) break;
    }
    //qDebug()<<"EXSTRACT=========="<<nft<<end_ap_ipass;
    //End AP JT65

    /*
        ntry[0]=0;
        zero_int_beg_end(param_jt65,0,10);  //param=0
        //ntrials = 30000; //only for test HV
        ftrsd2(mrsym,mrprob,mr2sym,mr2prob,ntrials,correct,param_jt65,ntry);
        //ncandidates=param_jt65[0];
        nhard=param_jt65[1];
        //nsoft=param_jt65[2];
        //nerased=param_jt65[3];
        rtt=0.001*param_jt65[4];
        ntotal=param_jt65[5];
        qual=0.001*param_jt65[7];

        nd0=81;//81;  hv 77
        r0=0.87;//87;
        if (naggressive==10) //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
        {
            nd0=83;
            r0=0.90;
        }
        //qDebug()<<"1end EXSTRACT=========="<<ntotal<<rtt;
        if (ntotal<=nd0 && rtt<=r0) nft=1; // if(ntotal.le.nd0 .and. rtt.le.r0) nft=1
        //qDebug()<<"extract================nft==="<<nft<<ntotal<<nd0<<rtt<<r0;
    */

    if (nft==0 && f_deep_search)//1.49 ndepth 0=no, 1=avg, 2=avg+deep search    old-> avg=16  hint65=32
    {
        //qmin=2.0 - 0.1*(double)naggressive; //mshv static -> naggressive=0  hv naggressive vareable 0 to 10
        //qmin=3.0 - 0.1*(double)naggressive;  //1.54=1.5  1.52=1.3 hv naggressive 0...5...10 move qmin 2.0...1.5...1.0  posl hary->1.25
        qmin=11.0 - 0.1*(double)naggressive;
        //qDebug()<<"DEEPS=="<<qmin;

        hint65(s3_,nadd,nflip,mycall,hiscall,hisgrid,qual,decoded);

        //qual = 2.2;
        //if(qual<3.0)
        //qDebug()<<"AVERAGE+DEEP==========="<<qual<<qmin;
        if (qual>=qmin)
        {
            nft=2;
            ncount=0;
        }
        else
        {
            decoded="";//decoded='                      '
            ntry[0]=0;
        }
        goto c900;
    }

    ncount=-1;
    decoded="";
    ltext=false;

    if (nft>0) //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    {
        //! Turn the corrected symbol array into channel symbols for subtraction;
        //! pass it back to jt65a via common block "chansyms65".
        for (int i =0; i<12; i++)
        {//do i=1,12
            dat4[i]=correct_jt65[11-i]; //hv dat4[i]=correct(13-i)
        }
        for (int i =0; i<63; i++)
        {//do i=1,63
            tmp[i]=correct_jt65[62-i];  //hv tmp[i]=correct[64-i];
        }

        for (int i =0; i<63; i++)
            correct_jt65[i]=tmp[i];//correct(1:63)=tmp(1:63)

        TGen65->interleave63(correct_jt65,1); //???? -> interleave63(correct,63,1);
        TGen65->graycode65(correct_jt65,63,1);
        char ident;//fictivno call unpackmsg(dat4,decoded)     !Unpack the user message
        decoded = TGen65->unpackmsg(dat4,ident);     //!Unpack the user message
        ncount=0;
        //qDebug()<<"5555555end AVERAGE=========="<<nft<<decoded;
        if ((dat4[9] & 8)!=0) ltext=true; //if(iand(dat4(10),8).ne.0) ltext=.true.
    }

    //qDebug()<<"extract====================decoded="<<decoded;
    //goto c1;

c900: //continue

    /*if(!decoded.isEmpty())
       qDebug()<<"1 extract=decoded="<<decoded<<nft<<nhard<<ntotal<<0.001*param_jt65[4];*/
    //Filter HV
    //decoded = QString(mycall+" "+mycall)+" RRR"; //test AP exception
    int nap=(nft >> 2);//1.55=hv added AP exception
    if (nap!=0 && decoded.contains(QString(mycall+" "+mycall))) ncount=-1;//1.55=hv added AP exception

    if (decoded.mid(0,7)=="000AAA ") ncount=-1; //(decoded(1:7)=="000AAA ")//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    if (decoded.mid(0,7)=="0L6MWK ") ncount=-1; //(decoded(1:7)=="0L6MWK ")
    //LZ2HV	find bridge
    if (decoded.count()==13)
    {
        //if(decoded.count(" ")==1 || !decoded.contains(" "))
        //if (decoded.count(" ")<2)//no pouse or only 1 //1.59=stop
        //1.59=  problem -> "CQ SV7CUD/QRP" count=13 one pause
        //1.59=  problem -> "SP9HWY LZ2HVV" count=13 one pause
        //"0+3+M62K M9-D"
        if (!decoded.contains(" "))
            ncount=-1;
    }
    /*
    if (decoded.count()==12)
    {
    	//1.59=  problem -> RV6AGB/IMAGE ->SP9HWY_LZ2HV_JT65A_171106_110100.WAV
        if (!decoded.contains(" "))
            //if(decoded.count(" ")<1)//no pouse
            ncount=-1;
    }
    */
    if (nft==1 && nhard<0) ncount=-1;//no possible -> nhard<0
    if (nflip<0 && ltext) ncount=-1;
    //END LZ2HV find bridge

    if (ncount<0)
    {
        //if(nft>0)
        /*if(!decoded.isEmpty())
          qDebug()<<"2 BBBBBBBridge="<<decoded<<decoded.count()<<decoded.count(" ")<<nft<<nhard<<nflip<<ltext;*/
        nft=0;
        decoded=""; //decoded='                      '
    }
    //qDebug()<<"3 extract=decoded="<<decoded<<nft<<nhard;
    //end 1.53 HV
}

void DecoderMs::decode65b(double s2_[126][66],int nflip,int nadd,int mode65,int ntrials,int naggressive,bool f_deep_search,
                          QString mycall,QString hiscall,QString hisgrid,int nqd,int &nft,double &qual,int &nhist,
                          QString &decoded,int nQSOProgress,bool ljt65apon)
{
    //y s2_[126][66];
    double s3_[63][64];//real s3(64,63)
    int ncount=0;
    bool ltext = false;

    if (nqd==-99) return;               //!Silence compiler warning
    for (int j =0; j<63; j++)	     //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    {//do j=1,63
        int k=TGen65->mdat[j];                       //!Points to data symbol
        if (nflip<0) k=TGen65->mdat2[j];
        for (int i =0; i<64; i++)
        {//do i=1,64
            s3_[j][i]=s2_[k][i+2]; //s3(i,j)=s2(i+2,k)
        }
    }

    extract(s3_,nadd,mode65,ntrials,naggressive,f_deep_search,nflip,mycall,hiscall,hisgrid,ncount,nhist,
            decoded,ltext,nft,qual,nQSOProgress,ljt65apon);
    //qDebug()<<"1sss="<<decoded;

    //! Suppress "birdie messages" and other garbage decodes:
    //1.53 HV ALL this moved to extract for normal and average decoding
    /*if (decoded.mid(0,7)=="000AAA ") ncount=-1; //(decoded(1:7)=="000AAA ")//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    if (decoded.mid(0,7)=="0L6MWK ") ncount=-1; //(decoded(1:7)=="0L6MWK ")
    if (decoded.mid(0,13)=="SVUW5SVUW5682") ncount=-1;
    if (decoded.mid(0,13)=="0123456789ABC") ncount=-1;
    if (decoded.mid(0,13)=="RGS3L0HK90TRC") ncount=-1;
    if (nflip<0 && ltext) ncount=-1;
    if (ncount<0)
    {
        nft=0;
        decoded=""; //decoded='                      '
    }   */
    //qDebug()<<"1sss="<<decoded<<nflip<<ltext<<ncount;
}

void DecoderMs::smo121(double *x,int beg,int nz)
{
    // real x(nz)
    double x0=x[beg];
    for (int i =1; i<nz-1; i++)
    {//do i=2,nz-1
        double x1=x[i+beg];
        x[i+beg]=0.5*x[i+beg] + 0.25*(x0+x[i+1+beg]);// x(i)=0.5*x(i) + 0.25*(x0+x(i+1))
        x0=x1;
    }
}

void DecoderMs::twkfreq65(double complex *c4aa,int n5,double *a)
{
    //qDebug()<<"aaaaaaaaaaaaaaaaa="<<a[0]<<a[1]<<a[2];//<<a[3]<<a[4]
    //! Apply AFC corrections to the c4aa data
    if (n5==0)// no devide by zero
        return;

    double complex w=1.0+1.0*I;
    double complex wstep=1.0+1.0*I;
    int x0=0.5*(n5+0); //x0=0.5*(n5+1);
    double s=2.0/(double)n5;

    for (int i =0; i<n5; i++)
    {//do i=1,n5
        double x=s*(double)(i-x0);
        if (fmod(i,99)==1) //(fmod(i,100).eq.1) then
        {
            double p2=1.5*x*x - 0.5;
            double dphi=(a[0] + x*a[1] + p2*a[2]) * (twopi/1378.125); //*a[2]
            wstep=cos(dphi)+sin(dphi)*I;
        }
        w=w*wstep;
        c4aa[i]=w*c4aa[i];
    }
}

void DecoderMs::ccf2(double *ss,int nz,int nflip,double &ccfbest,double &xlagpk)
{

    //int LAGMIN=-86;
    int LAGMIN=-112;//1.59=
    int LAGMAX=258;
    int nprc[126]={1,0,0,1,1,0,0,0,1,1,1,1,1,1,0,1,0,1,0,0,
                  0,1,0,1,1,0,0,1,0,0,0,1,1,1,0,0,1,1,1,1,
                  0,1,1,0,1,1,1,1,0,0,0,1,1,0,1,0,1,0,1,1,
                  0,0,1,1,0,1,0,1,0,1,0,0,1,0,0,0,0,0,0,1,
                  1,0,0,0,0,0,0,0,1,1,0,1,0,0,1,0,1,1,0,1,
                  0,1,0,1,0,0,1,1,0,0,1,0,0,1,0,0,0,0,1,1,
                  1,1,1,1,1,1};
    //double ccf_p[516+40];
    //double *ccf = &ccf_p[258+20];
    //double ccf_p[344+60];//1.54=
    //double *ccf = &ccf_p[86+30];//from -86 to 257  ccf=-116 to 288
    double ccf_p[370+60];//1.59=
    double *ccf = &ccf_p[112+30];//1.59= from -112 to 257  ccf=-116 to 288

    ccfbest=0.0;
    int lag1=LAGMIN;
    int lag2=LAGMAX;
    int lagpk = 0;

    for (int lag =lag1; lag<lag2; lag++)
    {//do lag=lag1,lag2
        double s0=0.0;
        double s1=0.0;
        for (int i =0; i<126; i++)//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
        {//do i=1,126
            int j=16*(i) + lag; //   j=16*(i-1)+1 + lag
            if (j>=0 && j<nz-8) //if(j>=1 && j<=nz-8) then
            {
                double x=ss[j];
                if (nprc[i]==0)
                    s0=s0 + x;
                else
                    s1=s1 + x;
            }
        }
        ccf[lag]=nflip*(s1-s0);
        if (ccf[lag]>ccfbest)
        {
            ccfbest=ccf[lag];
            lagpk=lag;
            xlagpk=lagpk;
        }
    }
    if ( lagpk>-LAGMAX && lagpk<LAGMAX-0)//if( lagpk>-LAGMAX .and. lagpk<LAGMAX) then
    {
        //call peakup(ccf(lagpk-1),ccf(lagpk),ccf(lagpk+1),dx)
        double dx = pomAll.peakup(ccf[lagpk-1],ccf[lagpk],ccf[lagpk+1]);
        xlagpk=lagpk+dx;
    }
}

double DecoderMs::fchisq65(double complex *cx,int npts,double fsample,int nflip,double *a,
                           double &ccfmax,double &dtmax)
{

    //int NMAX=60*DEC_SAMPLE_RATE;
    double fchisq65 = 0.0;
    /*double a1=99.0;
    double a2=99.0;
    double a3=99.0;*/
    double complex csx[720000/8+5]; //complex csx(0:NMAX/8) NMAX=60*DEC_SAMPLE_RATE;
    double complex w;
    double complex wstep = 1.0+1.0*I;
    double complex z;
    double ss[3000];

    double baud=11025.0/4096.0;
    //double baud=12000.0/4096.0;
    int nsps=int(fsample/baud);                 //!Samples per symbol
    //nsph=nsps/2;                             // !Samples per half-symbol
    double ndiv=16.0;                                // !Output ss() steps per symbol
    int nout=int((ndiv*(double)npts)/(double)nsps);
    double dtstep=1.0/(ndiv*baud);                   //!Time per output step

    //if (a[0]!=a1 || a[1]!=a2 || a[2]!=a3)
    //{
    //qDebug()<<a[0]<<a[1]<<a[2];
    //a1=a[0];
    //a2=a[1];
    //a3=a[2];

    //! Mix and integrate the complex signal
    //zero_double_comp_beg_end(csx,0,NMAX/8);
    //csx[0]=1.0+1.0*I;
    csx[0]=0.0+0.0*I;
    w=1.0+1.0*I;
    int x0=0.5*(npts); //1.53 x0=0.5*(npts+1);
    double s=2.0/(double)npts;
    for (int i = 0; i<npts; i++)
    {//do i=1,npts
        double x=s*(double)(i-x0);
        if (fmod(i,99)==1) //1.53=(99)-testedHV  ??? if(mod(i,100).eq.1) then
        {
            double p2=1.5*x*x - 0.5;
            double dphi=(a[0] + x*a[1] + p2*a[2]) * (twopi/fsample);
            wstep=cos(dphi)+sin(dphi)*I;
        }
        w=w*wstep;
        csx[i+1]=csx[i] + w*cx[i];//hv testvano csx[i]=csx[i-1] + w*cx[i]
    }
    //}

    //! Compute whole-symbol powers at 1/16-symbol steps.
    double fac=1.e-4;
    for (int i = 0; i<nout; i++)//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    {//do i=1,nout
        int j=(nsps-0) + i*nsps/16.0;//1.53=(nsps-0)-testedHV  j=nsps+(i-1)*nsps/16 //!steps by 8 samples (1/16 of a symbol)
        int k=j-nsps;
        ss[i]=0.0;
        if (k>=0 && j<npts) //if(k>=0 .and. j<=npts) then
        {
            z=csx[j]-csx[k]; //! difference over span of 128 pts
            ss[i]=fac*pomAll.ps_hv(z);//(real(z)**2 + aimag(z)**2)
        }
    }

    ccfmax=0.0;
    double ccf = 0.0;
    double xlagpk = 0.0;

    ccf2(ss,nout,nflip,ccf,xlagpk);

    if (ccf>ccfmax)
    {
        ccfmax=ccf;
        dtmax=xlagpk*dtstep;
    }
    fchisq65=-ccfmax;

    return fchisq65;
}

void DecoderMs::afc65b(double complex *cx,int npts,double fsample,int nflip,
                       double *a,double &ccfbest,double &dtbest)
{

    double chisqr=0.0;
    double chisqr0=1.e6;
    int nterms=2;
    double deltaa[5];
    double ccfmax = 0.0;
    double dtmax = 0.0;
    a[0]=0.0;
    a[1]=0.0;
    a[2]=0.0;
    a[3]=0.0;
    deltaa[0]=2.0;
    deltaa[1]=2.0;
    deltaa[2]=1.0;
    int end_c = 0;

    for (int iter = 0; iter<3; iter++)
    {//do iter=1,3                               //!One iteration is enough?
        for (int j = 0; j<nterms; j++)//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
        {//do j=1,nterms
            double chisq1=fchisq65(cx,npts,fsample,nflip,a,ccfmax,dtmax);
            double fn=0.0;
            double delta=deltaa[j];
            double chisq2=0.0;
            double chisq3=0.0;

            end_c = 0;
c10:
            a[j]=a[j]+delta;
            chisq2=fchisq65(cx,npts,fsample,nflip,a,ccfmax,dtmax);
            if (chisq2==chisq1)
            {
                if (end_c>500)//HV max 501 interations else out
                {
                    a[0]=0.0;
                    a[1]=0.0;
                    a[2]=0.0;
                    a[3]=0.0;
                    goto c_fall_nan;
                }
                else
                    end_c++;

                goto c10;
            }
            if (chisq2>chisq1)
            {
                delta=-delta;                      //!Reverse direction
                a[j]=a[j]+delta;
                double tmp=chisq1;
                chisq1=chisq2;
                chisq2=tmp;
            }

            end_c = 0;
c20:
            //qDebug()<<"gggg"<<a[0]<<a[1];
            fn=fn+1.0;
            a[j]=a[j]+delta;
            chisq3=fchisq65(cx,npts,fsample,nflip,a,ccfmax,dtmax);
            if (chisq3<chisq2)
            {
                chisq1=chisq2;
                chisq2=chisq3;
                if (end_c>500)//HV max 501 interations else out
                {
                    a[0]=0.0;
                    a[1]=0.0;
                    a[2]=0.0;
                    a[3]=0.0;
                    goto c_fall_nan;
                }
                else
                    end_c++;

                goto c20;
            }
            //! Find minimum of parabola defined by last three points
            //qDebug()<<a[0]<<a[1];
            //delta=delta*(1.0/(1.0+(chisq1-chisq2)/(chisq3-chisq2))+0.5);
            delta=delta*(1.0/(1.0+(chisq1-chisq2)/(chisq3-chisq2))+0.60);//1.53=(+0.60)-testedHV
            a[j]=a[j]-delta;
            deltaa[j]=deltaa[j]*fn/3.0;
        }
        chisqr=fchisq65(cx,npts,fsample,nflip,a,ccfmax,dtmax);
        if (chisqr/chisqr0>0.9999) goto c30;
        chisqr0=chisqr;

c_fall_nan:
        continue;
    }
c30:
    ccfbest=ccfmax * (1378.125/fsample)*(1378.125/fsample);
    dtbest=dtmax;
}

/*
void DecoderMs::afc65b(double complex *cx,int npts,double fsample,int nflip,int mode65,
                       double *a,double &ccfbest,double &dtbest)
{
    double chisqr=0.0;
    double chisqr0=1.e6;
    int nterms=2;
    double deltaa[5];
    double ccfmax = 0.0;
    double dtmax = 0.0;
    a[0]=0.0;
    a[1]=0.0;
    a[2]=0.0;
    a[3]=0.0;
    int end_c = 0;
    double a1 = 0.0;
    double a2 = 0.0;
    //double chisq=0.0;

    int i2=8*mode65;
    int i1=-i2;
    int j2=8*mode65;
    int j1=-j2;
    int istep=2*mode65;
    for (int iter = 0; iter<2; iter++)
    {//do iter=1,2
        for (int i = i1; i<i2; i+=istep)
        {//do i=i1,i2,istep
            a[0]=i;
            for (int j = j1; j<j2; j+=istep)
            {//do j=j1,j2,istep
                a[1]=j;
                double ccf = 0.0;
                //double chisq=fchisq65(cx,npts,fsample,nflip,a,ccf,dtmax);
                fchisq65(cx,npts,fsample,nflip,a,ccf,dtmax);
                if (ccf>ccfmax) //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
                {
                    a1=a[0];
                    a2=a[1];
                    ccfmax=ccf;
                }
            }
        }
        i1=a1-istep;
        i2=a1+istep;
        j1=a2-istep;
        j2=a2+istep;
        istep=1;
    }

    a[0]=a1;
    a[1]=a2;
    a[2]=0.0;
    a[3]=0.0;
    deltaa[0]=2.0*mode65;
    deltaa[1]=2.0*mode65;
    deltaa[2]=1.0;

    for (int iter = 0; iter<50; iter++)
    {//do iter=1,3    org=100 no hv=50
        double fdiff;                             //!One iteration is enough?
        for (int j = 0; j<nterms; j++)//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
        {//do j=1,nterms
            double chisq1=fchisq65(cx,npts,fsample,nflip,a,ccfmax,dtmax);
            double fn=0.0;
            double delta=deltaa[j];
            double chisq2=0.0;
            double chisq3=0.0;

            end_c = 0;
c10:
            a[j]=a[j]+delta;
            chisq2=fchisq65(cx,npts,fsample,nflip,a,ccfmax,dtmax);
            if (chisq2==chisq1)
            {
                if (end_c>500)//HV max 501 interations else out
                {
                    a[0]=0.0;
                    a[1]=0.0;
                    a[2]=0.0;
                    a[3]=0.0;
                    goto c_fall_nan;
                }
                else
                    end_c++;

                goto c10;
            }
            if (chisq2>chisq1)
            {
                delta=-delta;                      //!Reverse direction
                a[j]=a[j]+delta;
                double tmp=chisq1;
                chisq1=chisq2;
                chisq2=tmp;
            }

            end_c = 0;
c20:
            //qDebug()<<"gggg"<<a[0]<<a[1];
            fn=fn+1.0;
            a[j]=a[j]+delta;
            chisq3=fchisq65(cx,npts,fsample,nflip,a,ccfmax,dtmax);
            if (chisq3<chisq2)
            {
                chisq1=chisq2;
                chisq2=chisq3;
                if (end_c>500)//HV max 501 interations else out
                {
                    a[0]=0.0;
                    a[1]=0.0;
                    a[2]=0.0;
                    a[3]=0.0;
                    goto c_fall_nan;
                }
                else
                    end_c++;

                goto c20;
            }
            //! Find minimum of parabola defined by last three points
            //qDebug()<<a[0]<<a[1];
            //delta=delta*(1.0/(1.0+(chisq1-chisq2)/(chisq3-chisq2))+0.5);
            delta=delta*(1.0/(1.0+(chisq1-chisq2)/(chisq3-chisq2))+0.60);//1.53=(+0.60)-testedHV
            a[j]=a[j]-delta;
            deltaa[j]=deltaa[j]*fn/3.0;
        }
        chisqr=fchisq65(cx,npts,fsample,nflip,a,ccfmax,dtmax);
        //if (chisqr/chisqr0>0.9999) goto c30;
        fdiff=chisqr/chisqr0-1.0;
        if (abs(fdiff)<0.0001) break; //exit  goto c30;//
        chisqr0=chisqr;

c_fall_nan:
        continue;
    }
//c30:
    ccfbest=ccfmax * (1378.125/fsample)*(1378.125/fsample);
    dtbest=dtmax;
}
*/
void DecoderMs::fil6521(double complex *c1,int n1,double complex *c2,int &n2)
{
    /*! FIR lowpass filter designed using ScopeFIR
    !                  Pass #1   Pass #2
    ! -----------------------------------------------
    ! fsample    (Hz)  1378.125   Input sample rate
    ! Ntaps            21         Number of filter taps
    ! fc         (Hz)  40         Cutoff frequency
    ! fstop      (Hz)  172.266    Lower limit of stopband
    ! Ripple     (dB)  0.1        Ripple in passband
    ! Stop Atten (dB)  38         Stopband attenuation
    ! fout       (Hz)  344.531    Output sample rate
    */
    int NTAPS=21;
    int NDOWN=4;              //!Downsample ratio = 1/4
    int NH=NTAPS/2;
    double a_[28] = {-0.011958606980,-0.013888627387,-0.015601306443,-0.010602249570,
                     0.003804023436, 0.028320058273, 0.060903935217, 0.096841904411,
                     0.129639871228, 0.152644580853, 0.160917511283, 0.152644580853,
                     0.129639871228, 0.096841904411, 0.060903935217, 0.028320058273,
                     0.003804023436,-0.010602249570,-0.015601306443,-0.013888627387,
                     -0.011958606980,1.43370769e-019,2.64031087e-006,6.25548654e+028,
                     2.44565251e+020,4.74227538e+030,10497312.0e0000,7.74079654e-039};

    n2=(n1-NTAPS+NDOWN)/NDOWN;////c2 max=22500 max n1=77175;

    int k0=NH-NDOWN; //+1
    int ofs = NH;
    //20+NTAPS/3 =27
    //qDebug()<<"fil6521 n2=k0==="<<n2<<k0<<(k0 + NDOWN)<<NH<<20+NTAPS/3;

    //! Loop over all output samples
    for (int i = 0; i<n2; i++)////c2 max=22500
    {//do i=1,n2
        c2[i]=0.0+0.0*I;
        int k=k0 + NDOWN*(i+1); //<<<<<<--------pr0blema
        //qDebug()<<"k======================="<<k;
        for (int j = -NH; j<=NH; j++)
        {//do j=-NH,NH
            c2[i]+=c1[j+k]*a_[j+ofs];
        }
    }
}

void DecoderMs::sh65snr(double *x,int b,int nz,double &snr)
{
    //7 b=436 nz=2546
    int ipk=0; //!Shut up compiler warnings. -db
    double smax=-1.e30;
    double s = 0.0;

    //qDebug()<<"000000000 sh65snrs="<<"max pos="<<nz-1+b<<"NZ smp 0=to=NZ="<<nz;

    for (int i = 0; i<nz; i++)//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    {//do i=1,nz
        if (x[i+b]>smax)
        {
            ipk=i;
            smax=x[i+b];
        }
        s=s+x[i+b];
    }

    s=0.0; // ????
    int ns=0;
    for (int i = 0; i<nz; i++)
    {//do i=1,nz
        if (fabs(i-ipk)>=3)
        {
            s=s+x[i+b];
            ns=ns+1;
        }
    }

    if (ns==0)//no devide by zero
        ns=1;
    double ave=s/(double)ns;

    double sq=0.0;
    for (int i = 0; i<nz; i++)
    {//do i=1,nz
        if (fabs(i-ipk)>=3)
        {
            sq=sq+(x[i+b]-ave)*(x[i+b]-ave);
            ns=ns+1;
        }
    }
    int divid = (nz-2);
    if (divid==0)//no devide by zero
        divid=1;
    double rms=sqrt(sq/(double)divid);
    if (rms==0.0)//no devide by zero
        rms=1.0;
    snr=(smax-ave)/rms;
}

void DecoderMs::sh65(double complex *cx,int n5,int mode65,int ntol,double &xdf,int &nspecial,double &snrdb,
                     double &nstest)
{
    const int NFFT=2048;
    //int MAXSTEPS=150;
    int NH=NFFT/2;

    double complex c[NFFT+10];//complex c(0:NFFT-1)
    //double complex *c = new double complex[NFFT];
    double sigmax[8];
    int ipk[8];

    double ts_sh65[2048+20]; //nh=1024
    double *s_sh65 = &ts_sh65[1024+10];

    double (*s2_sh65_)[2048+20] = new double[150][2048+20];// ima problem NEW ina4e garmi //double (*s0_)[289]=new double[5601][289]
    double (*ss_sh65_)[2048+20] = new double[8][2048+20]; // ima problem NEW ina4e garmi
    int ofs = 1030;

    for (int i = -1030; i < 1030; i++)
        s_sh65[i]=0.0;

    /*for (int i = 0; i < MAXSTEPS; i++)
    {
        for (int j = -1030; j < 1030; j++)
        {
            s2_sh65_[i][j+ofs]=0.0;
        }
    }*/

    for (int i = 0; i < 8; i++)
    {
        ipk[i]=0;
        for (int j = -1030; j < 1030; j++)
        {
            /*if (i==0)
                ss_sh65_[i][j+ofs]=1.2;
            else if (i==1)
                ss_sh65_[i][j+ofs]=2.5;
            else if (i==2)
                ss_sh65_[i][j+ofs]=3.7;
            else
                ss_sh65_[i][j+ofs]=4.8;*/
            ss_sh65_[i][j+ofs]=0.0;
        }
    }

    int jstep=NFFT/4;
    int nblks=n5/jstep - 3;
    int ia=-jstep+0;//+1

    for (int iblk = 0; iblk < nblks; iblk++)
    {//do iblk=1,nblks
        ia=ia+jstep;
        //int ib=ia+NFFT-0;//-1

        for (int i = 0; i < NFFT; i++)
            c[i]=cx[i+ia];//c=cx(ia:ib)

        f2a.four2a_c2c(c,NFFT,1,1);            //!c2c FFT

        for (int i = 0; i < NFFT; i++)//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
        {//do i=0,NFFT-1
            int j=i;
            if (j>NH) j=j-NFFT;
            double p=pomAll.ps_hv(c[i]);//p=real(c(i))**2 + aimag(c(i))**2
            s_sh65[j] += p;
            s2_sh65_[iblk][j+ofs]=p;//s2(j,iblk)=p
        }
        int n=fmod(iblk-0,8) +0; //???   n=mod(iblk-1,8) +1
        //ss(-NH+1:NH,n)=ss(-NH+1:NH,n) + s2(-NH+1:NH,iblk)
        for (int i = -NH+0; i < NH; i++)//+1
            ss_sh65_[n][i+ofs] += s2_sh65_[iblk][i+ofs];
    }

    for (int i = 0; i < 8; i++)
    {
        for (int j = -1030; j < 1030; j++)
        {
            if (i==0)
                s_sh65[j]=1.e-6*s_sh65[j];	     //s=1.e-6*s
            ss_sh65_[i][j+ofs]=1.e-6*ss_sh65_[i][j+ofs]; //ss=1.e-6*ss
        }
    }
    //qDebug()<<"sh65----"<<s_sh65[2+ofs]<<s_sh65[3+ofs]<<s_sh65[4+ofs]<<s_sh65[5+ofs]<<s_sh65[6+ofs];

    double df=1378.1285/(double)NFFT;
    double nfac=40.0*(double)mode65;
    //dtstep=0.25/df;

    //! Define range of frequencies to be searched
    double fa=-ntol;
    double fb=ntol;
    int ia2=fmax(-NH+0,int(fa/df));//-NH+1
    //! Upper tone is above sync tone by 4*nfac*df Hz
    int ib2=fmin(NH,int(fb/df + 4.1*nfac));

    //! Find strongest line in each of the 4 phases, repeating for each drift rate.
    double sbest=0.0;
    //snrbest=0.
    int nbest=1;
    //int ipk=0;

    for (int n = 0; n < 8; n++)
    {//do n=1,8
        sigmax[n]=0.0;
        for (int i = ia2; i < ib2; i++)
        {//do i=ia2,ib2
            double sig=ss_sh65_[n][i+ofs]; //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.

            if (sig>=sigmax[n])
            {
                //if(i>=0)
                //qDebug()<<"iiiiiiiiiiiii"<<i;
                ipk[n]=i;
                sigmax[n]=sig;
                if (sig>=sbest)
                {
                    sbest=sig;
                    nbest=n;
                }
            }
        }
    }
    //qDebug()<<"IPK="<<ipk[0]<<ipk[1]<<ipk[2]<<ipk[3]<<ipk[4]<<ipk[5]<<ipk[6]<<ipk[7];

    int n2best=nbest+4; //n2best=nbest+4
    if (n2best>7) n2best=nbest-4; // if(n2best.gt.8) n2best=nbest-4
    xdf=fmin(ipk[nbest],ipk[n2best])*df;
    //qDebug()<<"sh65= xdf"<<xdf;
    nspecial=0;
    if (fabs(xdf)<=(double)ntol) //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    {
        double idiff=fabs(ipk[nbest]-ipk[n2best]);
        double xk=(idiff)/nfac;
        int k=(int)round(xk);//1.60=hv TA2NC->round()
        /*value   round   floor   ceil    trunc
        -----   -----   -----   ----    -----
        2.3     2.0     2.0     3.0     2.0
        3.8     4.0     3.0     4.0     3.0
        5.5     6.0     5.0     6.0     5.0
        -2.3    -2.0    -3.0    -2.0    -2.0
        -3.8    -4.0    -4.0    -3.0    -3.0
        -5.5    -6.0    -6.0    -5.0    -5.0 */

        //qDebug()<<"sh65==k========="<<k<<xk<<idiff<<ipk[nbest]<<ipk[n2best];
        int iderr=int((xk-(double)k)*nfac);
        //!     maxerr=nint(0.008*abs(idiff) + 0.51)
        int maxerr=int(0.02*fabs(idiff) + 0.51);     //!### Better test ??? ###
        //qDebug()<<"sh65==k==="<<k<<xk<<idiff<<fabs(iderr)<<maxerr;
        if (fabs(iderr)<=maxerr && k>=2 && k<=4) nspecial=k;
        snrdb=-30.0;
        double snr1 = 0.0;
        double snr2 = 0.0;
        double snr = 0.0;
        if (nspecial>0)
        {
            //qDebug()<<"66666666666666666666666666 sh65"<<nbest<<ia2+ofs<<(ib2-ia2+0)<<ib2<<ia2;
            sh65snr(ss_sh65_[nbest],ia2+ofs,(ib2-ia2+0),snr1);//sh65snr(ss(ia2,nbest),ib2-ia2+1,snr1)

            //qDebug()<<"77777777777777777777777777 sh65"<<n2best<<ia2+ofs<<(ib2-ia2+0)<<snr2;
            sh65snr(ss_sh65_[n2best],ia2+ofs,(ib2-ia2+0),snr2);//sh65snr(ss(ia2,n2best),ib2-ia2+1,snr2)

            snr=0.5*(snr1+snr2);

            nstest=snr/2.0 - 2.0;            //!Threshold set here
            if (nstest<0) nstest=0.0;
            if (nstest>10) nstest=10.0;
            //qDebug()<<"1 DELL"<<nstest;

            snrdb=pomAll.db(snr) - pomAll.db(2500.0/df) - pomAll.db(sqrt(nblks/4.0)) + 8.0;
        }
        if (snr1<4.0 || snr2<4.0 || snr<5.0) nspecial=0;
    }


    //qDebug()<<"1 DELL====================================================";
    delete [] s2_sh65_;
    delete [] ss_sh65_;
    //delete c;
}
double complex ca_65[336010]; //[672000/2+10]=336000+10
double rfilt_65[672010];
bool first_filbig_ = true;
void DecoderMs::filbig(double *dd,int npts,double f0,bool &newdat,double complex *c4a,int &n4,double &sq0)
{
    //! Filter and downsample the real data in array dd(npts), sampled at 12000 Hz.
    //! Output is complex, sampled at 1378.125 Hz.
    int NFFT1=672000;
    int NFFT2=77175;
    const int NSZ=3413;
    const int NZ2=1000;

    //double complex cfilt[NFFT2];                       //!Filter (complex; imag = 0)
    //double complex *cfilt =  new double complex[NFFT2+5];
    //!Impulse response of filter (one sided)
    double halfpulse[8]=
        {
            114.97547150,36.57879257,-20.93789101,5.89886379,
            1.59355187,-2.49138308,0.60910773,-0.04248129
        };
    //double rca[NFFT1];
    //double *rca = new double[NFFT1+5];
    //double complex ca[NFFT1/2+1];                      //!FFT of input
    //double complex *ca_65 =  new double complex[NFFT1/2+5]; //complex ca(NFFT1/2+1)
    //double *rfilt_65 = new double[NFFT1+5];
    double s[NZ2+5];

    //int npatience = 1;
    //fftw_plan plan1_fb = NULL;
    fftw_plan plan2_fb = NULL;
    //fftw_plan plan3_fb = NULL;
    double df=DEC_SAMPLE_RATE/(double)NFFT1;
    double fac=0.00625/(double)NFFT1;

    //if(npts<0) goto c900;  //!Clean up at end of program //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    //qDebug()<<"first_filbig"<<first_filbig<<"newdat"<<newdat<<npts;
    plan2_fb=fftw_plan_dft_1d(NFFT2,c4a,c4a,-1,FFTW_ESTIMATE_PATIENT);
    if (first_filbig_)
    {
        fftw_plan plan3_fb = NULL;
        double complex *cfilt =  new double complex[NFFT2+5];
        //! Plan the FFTs just once
        //!$omp critical(fftw) ! serialize non thread-safe FFTW3 calls
        //call fftwf_plan_with_nthreads(nthreads)
        //plan1_fb=fftw_plan_dft_r2c_1d(NFFT1,rca,ca_65,FFTW_ESTIMATE_PATIENT);
        //plan2_fb=fftw_plan_dft_1d(NFFT2,c4a,c4a,-1,FFTW_ESTIMATE_PATIENT);
        plan3_fb=fftw_plan_dft_1d(NFFT2,cfilt,cfilt,+1,FFTW_ESTIMATE_PATIENT);
        //!$omp end critical(fftw)

        //! Convert impulse response to filter function
        for (int i = 0; i < NFFT2; i++)
        {//do i=1,nfft2
            cfilt[i]=0.0+0.0*I;
        }
        //double fac=0.00625/(double)NFFT1;
        cfilt[0]=fac*halfpulse[0]; // cfilt(1)=fac*halfpulse(1)
        for (int i = 1; i < 8; i++)
        {//do i=2,8
            cfilt[i]=fac*halfpulse[i];
            cfilt[NFFT2-i]=fac*halfpulse[i]; //??? cfilt(nfft2+2-i)=fac*halfpulse(i)
        }
        fftw_execute_dft(plan3_fb,cfilt,cfilt);

        double base=creal(cfilt[NFFT2/2+0]); //??? base=creal(cfilt(nfft2/2+1))
        for (int i = 0; i < NFFT2; i++)// hvhv ????
        {//do i=1,nfft2
            rfilt_65[i]=creal(cfilt[i]) - base;
        }

        //double df=DEC_SAMPLE_RATE/(double)NFFT1;
        first_filbig_=false;
        fftw_destroy_plan(plan3_fb);
        delete [] cfilt;
    }
    if (newdat)//(newdat!=0)
    {
        fftw_plan plan1_fb = NULL;
        double *rca = new double[NFFT1+5];
        plan1_fb=fftw_plan_dft_r2c_1d(NFFT1,rca,ca_65,FFTW_ESTIMATE_PATIENT);
        int nz=fmin(npts,NFFT1);
        for (int i = 0; i < nz; i++)
            rca[i]=dd[i];
        for (int i = nz; i < NFFT1; i++)
            rca[i]=0.0;//rca(nz+1:)=0.
        fftw_execute_dft_r2c(plan1_fb,rca,ca_65);

        int ib=0;//0
        for (int j = 0; j < NSZ; j++)
        {//do j=1,NSZ
            int ia=ib;//tuk e problema -16db hv ia=ib+1
            ib=int((double)(j+1)*dfref_jt65/df);//ib=nint(j*dfref/df)

            double divid = ref_jt65[j];
            if (divid==0.0)// no div by zero
                divid=1.0;

            fac=sqrt(fmin(30.0,1.0/divid));//2.12
            for (int z = ia; z < ib; z++)
                ca_65[z]=fac*conj(ca_65[z]);
        }

        newdat=false;//newdat=0;
        fftw_destroy_plan(plan1_fb);
        delete [] rca;
    }

    int i0=int(f0/df) + 0;//i0=nint(f0/df) + 1
    int nh=NFFT2/2;
    for (int i = 0; i < nh; i++)//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    {//do i=1,nh                                //!Copy data into c4a and apply
        int j=i0+i-0;      // j=i0+i-1;                        //!the filter function
        if (j>=0 && j<NFFT1/2+1) //if(j>=1 .and. j<=nfft1/2+1) then if(j.ge.1 .and. j.le.nfft1/2+1) then
            c4a[i]=rfilt_65[i]*ca_65[j];
        else
            c4a[i]=0.0+0.0*I;
    }

    for (int i = nh; i < NFFT2; i++)
    {//do i=nh+1,NFFT2
        int j=i0+i-0-NFFT2;//??? j=i0+i-1-NFFT2  //// j=i0+i-1-NFFT2;
        //!     if(j.lt.1) j=j+NFFT1                  !nfft1 was nfft2
        if (j>=0) //(j>=1) if(j.ge.1) then
            c4a[i]=rfilt_65[i]*ca_65[j];
        else
            c4a[i]=rfilt_65[i]*conj(ca_65[0-j]);//1.53=0-j be6e=0-j  c4a(i)=rfilt(i)*conjg(ca(2-j))
    }
    // qDebug()<<"sh65==ZZZ========="<<i0+nh-0-NFFT2<<i0+NFFT2-1-NFFT2<<i0;

    int nadd=NFFT2/NZ2;
    int i=-1;//0
    for (int j = 0; j < NZ2; j++)
    {//do j=1,NZ2
        s[j]=0.0;
        for (int n = 0; n < nadd; n++)
        {//do n=1,nadd
            i=i+1;
            s[j]=s[j] + pomAll.ps_hv(c4a[i]);//creal(c4a(i))**2 + aimag(c4a(i))**2
        }
    }
    sq0 = pomAll.pctile_shell(s,NZ2,30);

    //for (int j = 0; j < 60*12000/8; j++)
    //c4a[j] = +0.1+(+0.1)*I;

    //qDebug()<<"sh65==ZZZ========="<<creal(c4a[2])<<creal(c4a[3])<<creal(c4a[4])<<rfilt[35]<<rfilt[60]<<rfilt[80];
    //! Do the short reverse transform, to go back to time domain.
    fftw_execute_dft(plan2_fb,c4a,c4a); //call fftwf_execute_dft(plan2,c4a,c4a)
    n4=fmin(npts/8,NFFT2); //n4=min(npts/8,nfft2)
    //qDebug()<<"sh65==FFF========="<<creal(c4a[2])<<creal(c4a[3])<<creal(c4a[4])<<rfilt[35]<<rfilt[60]<<rfilt[80];
    //return;
    //c900: // continue

    //fftw_destroy_plan(plan1_fb);
    fftw_destroy_plan(plan2_fb);
    //fftw_destroy_plan(plan3_fb);
}

void DecoderMs::decode65a(double *dd,int npts,bool &newdat,int nqd,double f0,int &nflip,
                          int mode65,int ntrials,int naggressive,bool f_deep_search,int ntol,QString mycall,QString hiscall,
                          QString hisgrid,bool bVHF,double &sync2,double *a,double &dt,int &nft,int &nspecial,
                          double &qual,int nsmo,QString &decoded,double &nstest,int nQSOProgress,bool ljt65apon)
{
    //! Apply AFC corrections to a candidate JT65 signal, then decode it.

    const int NMAX=60*12000;                  //!Samples per 60 s
    double complex c5x[NMAX/32+20];        //!Data at 344.53125 Hz

    //double complex cx[NMAX/8];                 //!Data at 1378.125 sps
    double complex *cx = new double complex[NMAX/8+20];
    //double complex cx1[NMAX/8];                //!Data at 1378.125 sps, offset by 355.3 Hz
    double complex *cx1 = new double complex[NMAX/8+20];

    //zero_double_comp_beg_end(cx,0,NMAX/8);
    //zero_double_comp_beg_end(cx1,0,NMAX/8);
    //qDebug()<<"IN77     sh 65==k========="<<creal(cx[2])<<creal(cx[3])<<creal(cx[4]);

    double sq0 = 0.0;
    int n5 = 0;
    double xdf = 0.0;
    double dtbest;
    double ccfbest;
    double fsample;
    int n6;
    double complex c5a[512+10];
    //double s1[126][512+40];
    //int s1_ofs = 256+20;
    int j;
    double df;
    int nfft;
    int nsym;
    double qualbest;
    double qual0;
    int minsmo;
    int maxsmo;
    int nn;
    double s2_[126][66];
    QString decoded_best;//character decoded*22,decoded_best*22
    //int nnbest =0;
    int nsmobest=0;
    //bool have_best_from_deep_search_nft2 = false; //hv v1.49 idea correction for deep search no good filtering
    //double k1,k0;

    //! Mix sync tone to baseband, low-pass filter, downsample to 1378.125 Hz
    //qDebug()<<"Start filbig newdat==="<<newdat;
    //f0 = 1584.96;
    filbig(dd,npts,f0,newdat,cx,n5,sq0);
    if (mode65==4) filbig(dd,npts,f0+355.297852,newdat,cx1,n5,sq0);

    //qDebug()<<"OUT filbig f0"<<f0<<sq0<<n5;
    //qDebug()<<"OUT66    sh65==k========="<<creal(cx[2])<<creal(cx[3])<<creal(cx[4]);

    //! NB: cx has sample rate 12000*77125/672000 = 1378.125 Hz

    //! Check for a shorthand message
    //qDebug()<<"0 nflip=="<<nflip;
    if (bVHF && mode65!=101) //101=??? //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    {
        sh65(cx,n5,mode65,ntol,xdf,nspecial,sync2,nstest);
        if (nspecial>0)
        {
            for (int i = 0; i < 5; i++)
                a[i]=0.0;  //    double a[5];
            a[0]=xdf;//a(1)
            nflip=0;
            nft=1;//add hv v1.34 !   nft      0=no decode; 1=FT decode; 2=hinted decode
        }
    }
    //nflip=1;
    //qDebug()<<"decode65a 1 nflip=="<<nflip;
    if (nflip==0) goto c900;

    //! Find best DF, drift, curvature, and DT.  Start by downsampling to 344.53125 Hz
    n6 = 0;
    // int NFFT2=77175;
    fil6521(cx,n5,c5x,n6);//c5x max=22500

    fsample=1378.125/4.0;

    //! Best fit for DF, drift, banana-coefficient, and dt. fsample = 344.53125 S/s
    dtbest=dt;

    //qDebug()<<"0 decode65a DTttttttttttttttt="<<dt<<n5<<n6;

    ccfbest = 0.0;
    //afc65b(c5x,n6,fsample,nflip,mode65,a,ccfbest,dtbest);
    afc65b(c5x,n6,fsample,nflip,a,ccfbest,dtbest);

    //dtbest=dtbest+0.003628; //!Remove decimation filter and coh. integrator delay
    dtbest=dtbest+0.003628;
    dt=dtbest;              //!Return new, improved estimate of dt
    //a[0]=-0.051189;

    if (sq0==0.0)// no devide by zero
        sq0=1.0;

    sync2=3.7e-4*ccfbest/sq0;                    //!Constant is empirical
    if (mode65==4)
    {
        for (int i = 0; i < NMAX/8; i++)
            cx[i]=cx1[i];
    }

    //! Apply AFC corrections to the time-domain signal
    //! Now we are back to using the 1378.125 Hz sample rate, enough to
    //! accommodate the full JT65C bandwidth.
    a[2]=0.0;  //a(3)=0
    //a[1]=0.661535;// true -> 0.702005 0.661535
    //a[0]=0.702005;
    /*k1= a[1];
    k0= a[0]; 
    a[1]=k0;
    a[0]=k1;*/

    /*for (int z = 0; z < 5; z++)
    {    
    //dtbest = dtbest + 0.0001;
    if(z>0)
    {
    a[0]+=0.5; 
    a[1]+=0.5; 
    }*/

    twkfreq65(cx,n5,a);

    //! Compute spectrum for each symbol.
    nsym=126;
    nfft=512;
    df=1378.125/(double)nfft;


    //qDebug()<<"1 decode65a DTttttttttttttttt="<<dtbest<<a[0]<<a[1]<<a[2]<<a[3];

    j=int(dtbest*1378.125);

    pomAll.zero_double_comp_beg_end(c5a,0,512);//c5a=cmplx(0.0,0.0)
    for (int k = 0; k < nsym; k++)
    {//do k=1,nsym
        for (int i = 0; i < nfft; i++)//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
        {//do i=1,nfft
            //j=j+1;//j=j+1  j=j+1
            if (j>=0 && j<(NMAX-0)/8) //if(j.ge.1 .and. j.le.NMAX/8) then
                c5a[i]=cx[j];
            else
                c5a[i]=0.0+0.0*I;
            j++;
        }
        f2a.four2a_c2c(c5a,nfft,1,1);//four2a(c5a,nfft,1,1,1)
        for (int i = 0; i < 512; i++)// real s1(-255:256,126)
        {//do i=1,512
            int jj=i;
            if (i>255) jj=i-512;//if(i.gt.256) jj=i-512
            s1_jt65_[k][jj+s1_ofs_jt65]=pomAll.ps_hv(c5a[i]);//s1_jt65_[126][512+40];  real(c5a(i))**2 + aimag(c5a(i))**2
        }
    }

    qualbest=0.0;
    qual0=-1.e30;
    minsmo=0;
    maxsmo=0;
    if (mode65>=2 && mode65!=101)
    {
        //minsmo=int(width_jt65/df);
        //maxsmo=2*minsmo;
        minsmo=int(width_jt65/df)-1;//1.53=-1 tested
        maxsmo=2*int(width_jt65/df);
    }
    nn=-1;//

    //qDebug()<<"101 decode65a maxsmo"<<minsmo<<maxsmo<<(double)width_jt65/df<<width_jt65<<df;
    //for (int ismo = 5; ismo <= 6; ismo++)
    for (int ismo = minsmo; ismo <= maxsmo; ismo++)//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    {//do ismo=minsmo,maxsmo
        if (ismo>0)
        {
            for (int x = 0; x < 126; x++)//2.12
            {//do j=1,126  s1_jt65_[126][512+40];
                smo121(s1_jt65_[x],(-256+s1_ofs_jt65),512);    //call smo121(s1(-255,j),512);
                if (x==0) nn=nn+1;                          //if(j==1) nn=nn+1;
                if (nn>=3)                                  //if(nn>=4)
                {
                    smo121(s1_jt65_[x],(-256+s1_ofs_jt65),512); //call smo121(s1(-255,j),512);
                    if (x==0) nn=nn+1;                       //if(j==1) nn=nn+1;
                }
            }
        }
        for (int i = 0; i < 66; i++)
        {//do i=1,66
            int jj=i;
            if (mode65==2) jj=2*i; //ok hv test            //if(mode65==2) jj=2*i-1;
            if (mode65==4)
            {
                double ff=4.0*(double)(i-0)*df - 355.297852;//ff=4*(i-1)*df - 355.297852;
                jj=int(ff/df)+0;                 //jj=nint(ff/df)+1;
            }
            for (int z = 0; z < 126; z++)//s1_jt65_[126][512+40];
                s2_[z][i]=s1_jt65_[z][jj+s1_ofs_jt65]; //double s2_[126][66];  s2(i,1:126)=s1(jj,1:126)
        }

        int nadd=ismo;
        int nhist = 0;
        decode65b(s2_,nflip,nadd,mode65,ntrials,naggressive,f_deep_search,mycall,hiscall,
                  hisgrid,nqd,nft,qual,nhist,decoded,nQSOProgress,ljt65apon);

        if (nft==1)
        {
            nsmo=ismo;
            param_jt65[9]=nsmo;///???? param(9)=nsmo //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
            nsum_jt65=1;
            goto c900;//exit
        }
        else if (nft==2)
        {
            if (qual>qualbest)
            {
                decoded_best=decoded;
                qualbest=qual;
                //nnbest=nn;
                nsmobest=ismo;
                //have_best_from_deep_search_nft2 = true;//hv v1.49 idea correction for deep search no good filtering
            }
        }
        //qDebug()<<"decode65bsmo1="<<nft<<qual<<qual0<<decoded<<decoded_best;
        if (qual<qual0)
            break;//1.49 correct to break; in 1.48 error -> goto c900;//exit
        qual0=qual;
    }

    //if (have_best_from_deep_search_nft2)//hv v1.49 idea correction for deep search no good filtering
    //idea is make filter min 2 times decode
    if (nft==2)
    {
        decoded=decoded_best;
        //qual=qualbest;//1.59= no qualbest real last decodet qual
        nsmo=nsmobest;
        param_jt65[9]=nsmo;
        //nn=nnbest;//2.12 no need
        //nft = 2;//hv v1.49 idea correction for deep search no good filtering
    }
    //qDebug()<<"decode65bsmo2="<<nft<<qual<<qual0<<decoded<<decoded_best;

    //} //////
c900:
    delete [] cx;
    delete [] cx1;
    //return;
}

void DecoderMs::fqso_first(double nfqso,int ntol,candidate_jt65 *ca,int ncand)
{
    //! If a candidate was found within +/- ntol of nfqso, move it into ca(1).

    //qDebug()<<"0 fqso_first ======="<<ca[0].flip<<ca[1].flip<<ca[2].flip<<ca[3].flip<<ncand;

    candidate_jt65 cb;
    double dmin=1.e30;
    int i0=0;
    //qDebug()<<"1 fqso_first ======="<<QString("%1").arg(dmin,0,'f',5)<<nfqso;
    for (int i = 0; i < ncand; i++)//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    {//do i=1,ncand
        double d=fabs(ca[i].freq-nfqso);
        if (d<dmin)
        {
            i0=i;
            dmin=d;
        }
    }
    //qDebug()<<"2 fqso_first ======="<<dmin<<ntol<<ca[i0].freq;
    if (dmin<double(ntol))
    {
        cb=ca[i0];
        for (int i = i0; i >= 1; i--)//do i=i0,2,-1
        {//do i=i0,2,-1
            ca[i]=ca[i-1];//ca(i)=ca(i-1)
        }
        ca[0]=cb; //ca(1)=cb
    }
    //qDebug()<<"1 fqso_first ======="<<ca[0].flip<<ca[1].flip<<ca[2].flip<<ca[3].flip<<ncand;
}

void DecoderMs::slope(double *y,int beg, int npts,double xpk)
{
    double sumw=0.0;
    double sumx=0.0;
    double sumy=0.0;
    double sumx2=0.0;
    double sumxy=0.0;
    double sumy2=0.0;
    for (int i = 0; i < npts; i++)//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    {//do i=1,npts
        //if (fabs(i-xpk)>2.0)
        if (fabs(i-xpk)>4.0)
        {
            sumw=sumw + 1.0;
            double x=i;
            sumx=sumx + x;
            sumy=sumy + y[i+beg];
            sumx2=sumx2 + x*x;
            sumxy=sumxy + x*y[i+beg];
            sumy2=sumy2 + y[i+beg]*y[i+beg];
        }
    }

    double delta=sumw*sumx2 - sumx*sumx;

    if (delta==0.0)//no devide by zero
        delta=1.0;
    double a=(sumx2*sumy - sumx*sumxy) / delta;
    double b=(sumw*sumxy - sumx*sumy) / delta;

    double sq=0.0;
    for (int i = 0; i < npts; i++)
    {//do i=1,npts
        y[i+beg]=y[i+beg]-(a + b*(double)i);
        if (fabs(i-xpk)>2.0) sq=sq + y[i+beg]*y[i+beg];
        //no good idea tested if (fabs(i-xpk)>4.0) sq=sq + y[i+beg]*y[i+beg];
    }

    double divid = (sumw-2.0);
    if (divid==0.0)//no devide by zero
        divid=1.0;
    double rms=sqrt(sq/divid);//*1.5
    //no good idea tested  double rms=sqrt(sq/(sumw-4.0));//*1.5
    if (rms==0.0)//no devide by zero
        rms=1.0;
    for (int i = 0; i < npts; i++)
        y[i+beg]=(y[i+beg]/rms);//*100.0;
}

void DecoderMs::xcor(double ss_[3413][642],int ipk,int nsteps,int nsym,int lag1,int lag2,double *ccf,
                     double &ccf0,int &lagpk,double &flip,double fdot,int nrobust)
{
    /*xcor(ss_,i,nhsym,nsym,lag1,lag2,ccfblue,ccf0,lagpk0,flip,fdot,nrobust);
    ! Computes ccf of a row of ss and the pseudo-random array pr.  Returns
    ! peak of the CCF and the lag at which peak occurs.  For JT65, the
    ! CCF peak may be either positive or negative, with negative implying
    ! the "OOO" message.
    */
    int NHMAX=3413;
    const int NSMAX=642;//  //nqsym=638 nhsym=319.323   old = 322
    double df=DEC_SAMPLE_RATE/8192.0;
    //double dtstep=0.5/df;
    double dtstep=0.25/df;
    double fac=dtstep/(60.0*df);
    double aaa[NSMAX+20];
    pomAll.zero_double_beg_end(aaa,0,NSMAX);
    int j;
    int lagmin = 0;

    //qDebug()<<"1 ssss ncand IN"<<nsteps<<ipk;
    for (j = 0; j < nsteps; j++)//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    {//do j=1,nsteps

        int ii=int((double)(j-nsteps/2)*fdot*fac)+ipk; //ii=nint((j-nsteps/2)*fdot*fac)+ipk
        if ( ii>=0 && ii<NHMAX )
            aaa[j]=ss_[ii][j]*0.01; //1.36 osobennost pri double HV
    }
    //qDebug()<<"1sss="<<int((j-nsteps/2)*fdot*fac)+ipk<<ipk;

    if (nrobust==1)
    {
        //! use robust correlation estimator to mitigate AGC attack spikes at beginning
        //! this reduces the number of spurious candidates overall
        double xmed = pomAll.pctile_shell(aaa,nsteps,50);
        for (j = 0; j < nsteps; j++)
        {//do j=1,nsteps
            if (aaa[j]>=xmed)
                aaa[j]=1;
            else
                aaa[j]=-1;
        }
    }

    double ccfmax=0.0;
    double ccfmin=0.0;
    for (int lag = lag1; lag < lag2; lag++)
    {//do lag=lag1,lag2
        double x=0.0;
        for (int i = 0; i < nsym; i++)
        {//do i=1,nsym
            //j=2*i-0+lag; //j=2*i-1+lag
            j=4*i-0+lag;  //j=4*i-3+lag
            if (j>=0 && j<nsteps) x=x+aaa[j]*(TGen65->pr[i]); //if(j>=1 && j<=nsteps) x=x+a[j]*pr[i];
        }
        ccf[lag]=(2.0*x);                    // !The 2 is for plotting scale
        if (ccf[lag]>ccfmax)
        {
            ccfmax=ccf[lag];
            lagpk=lag;
        }

        if (ccf[lag]<ccfmin)
        {
            ccfmin=ccf[lag];
            lagmin=lag;
        }
    }

    ccf0=ccfmax;
    flip=1.0;
    if (-ccfmin>ccfmax)
    {
        for (int lag = lag1; lag < lag2; lag++)
        {//do lag=lag1,lag2
            ccf[lag]=-ccf[lag];
        }
        lagpk=lagmin;
        ccf0=-ccfmin;
        flip=-1.0;
    }
    //qDebug()<<"LAGPKKKKKKK="<<lagpk;
}
void DecoderMs::sync65_single(double ss_[3413][642],double nfa,double nfb,int nqsym,double &dtx,double &dfx,
                              double &snrx,double &snrsync,double &flip,/*double &width,*/double *psavg,int mode65,int nrobust)
{

    double ccfblue_p[600]; //real ccfblue(-32:82)
    double *ccfblue = &ccfblue_p[50];//
    pomAll.zero_double_beg_end(ccfblue,-45,110); //-50,550

    int nsym=126;
    int nfft=8192;
    int NSZ=3413;
    //double ccfred[NSZ];
    //if(nfast.eq.2) nfft=1024
    //int nsteps=fmin(322,2*jz/nfft - 0);//nsteps=fmin(320,2*jz/nfft - 1);

    double df=(DEC_SAMPLE_RATE/(double)nfft);  //df=0.5*11025.0/(double)nfft;
    int ia=fmax(1,int(nfa/df));//ia=fmax(2,int(nfa/df));
    int ib=fmin(NSZ-1,int(nfb/df));

    //int lag1=-11;
    //int lag2=59;
    int lag1=-32;//1.59=-32 tested   -32;
    int lag2=100;//1.59=100 tested   82;
    double syncbest=-1.e30;
    double syncbest2=-1.e30;
    int ipk2 = 0;
    //double lagpk2 = 0.0;
    int lagpk0 = 0.0;
    int ipk = 0;
    //int lagpk = 0;
    double ppmax = 0.0;
    double ccfmax=0.0;

    //for (int i = -460; i < 460; i++)
    //ccfred[i]=0.0;//call zero(ccfred,745)

    for (int i = ia; i < ib; i++)
    {//do i=ia,ib
        double ccf0 = 0.0;
        xcor(ss_,i,nqsym,nsym,lag1,lag2,ccfblue,ccf0,lagpk0,flip,0.0,nrobust);

        //if(lagpk0<-30 || lagpk0>80)
        //qDebug()<<"lagpk0="<<lagpk0;
        //int j=i-i0;
        //if (j>=-372 && j<=372) ccfred[j]=ccf0;

        //! Find rms of the CCF, without the main peak
        slope(ccfblue,lag1,lag2-lag1+0,lagpk0-lag1+0.0);//1.58=lag2-lag1+0 tested

        double sync=fabs(ccfblue[lagpk0]);

        ppmax=psavg[i]-1.0;//ppmax=psavg(i)-1.0

        //! Find the best sync value
        if (sync>syncbest2)
        {
            ipk2=i;
            //lagpk2=lagpk0;
            syncbest2=sync;
        }
        //if(i*df>1200 && i*df<1300)
        //qDebug()<<"ppmax======"<<ppmax<<sync<<(double)i*df<<i;
        //! We are most interested if snrx will be more than -30 dB.

        //if (ppmax>0.2938)            //!Corresponds to snrx.gt.-30.0
        if (ppmax>0.15) //0.15 0.1038  0.14   0.1938   1.52 moze da se digne na 0.29
        {
            //qDebug()<<"ppmax======"<<ppmax<<sync<<(double)i*df<<i;
            if (sync>syncbest)
            {
                ipk=i;
                //lagpk=lagpk0;
                syncbest=sync;
            }
        }
    }
    //qDebug()<<"MAX-------------===="<<syncbest<<syncbest2<<(double)ipk*df<<ipk;

    //! If we found nothing with snrx > -30 dB, take the best sync that *was* found.
    if (syncbest < -10.0) //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    {
        //qDebug()<<"ipk2="<<ipk2;
        ipk=ipk2;
        //lagpk=lagpk2;
        //syncbest=syncbest2;
    }

    dfx=((double)ipk*df);
    ccfmax = 0.0;
    //qDebug()<<"1flip="<<flip;
    //! Peak up in time, at best whole-channel frequency //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    int lagpk = 0;
    xcor(ss_,ipk,nqsym,nsym,lag1,lag2,ccfblue,ccfmax,lagpk,flip,0.0,nrobust);
    //if(lagpk<-30 || lagpk>80)
    //qDebug()<<"lagpk="<<lagpk;

    double xlag=lagpk;
    if (lagpk>lag1 && lagpk<lag2)
    {
        double dx2 = pomAll.peakup(ccfblue[lagpk-1],ccfmax,ccfblue[lagpk+1]);
        xlag=lagpk+dx2;
    }

    //! Find rms of the CCF, without the main peak
    slope(ccfblue,lag1,lag2-lag1+0,xlag-lag1+0.0);//1.58=lag2-lag1+0 tested
    double sq=0.0;
    double nsq=0.0;
    //qDebug()<<"lagpk="<<lag1<<lag2<<lagpk;
    for (int lag = lag1; lag < lag2; lag++)
    {//do lag=lag1,lag2
        if (fabs(lag-xlag)>2.0)
        {
            sq=sq+ccfblue[lag]*ccfblue[lag];
            nsq=nsq+1.0;
        }
    }
    if (nsq==0.0)//no devide by zero
        nsq=1.0;
    double rms=sqrt(sq/nsq);
    if (rms==0.0)//no devide by zero
        rms=1.0;
    snrsync=fabs(ccfblue[lagpk])/rms - 1.1;//    !Empirical
    dtx = xlag*2408.0/11025.0;

    //int nh=nfft/2.0;
    //double dt=2.0/DEC_SAMPLE_RATE;
    //double istart=xlag*nh;  //qDebug()<<"xlag="<<xlag*2408.0/11025.0;  //xlag*2048.0/11025.0;
    //dtx=istart*dt;

    snrx=-99.0;
    //!      ppmax=psavg(ipk)/base-1.0  //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    ppmax=psavg[ipk]-1.0; //ppmax=psavg[ipk]-1.0;
    //! Plus 3 dB because sync tone is on half the time.  (Don't understand
    //! why an additional +2 dB is needed ...)
    if (ppmax>0.0001) snrx=pomAll.db(ppmax*(0.5*11025.0/1024.0)/2500.0) + 5.0;   //!### df=0.5*11025.0/nfft
    if (mode65==4) snrx=snrx + 2.0;
    if (snrx < -33.0) snrx=-33.0;

    //qDebug()<<"snrx="<<snrx;

    //! Compute width of sync tone to outermost -3 dB points
    /*
    double tmp[450];
    double base = pctile(ccfred,ia-i0,tmp,ib-ia,45); //pctile(ccfred[ia-i0],tmp,ib-ia+1,45,)

    int jpk=ipk-i0;  //qDebug()<<"jpk="<<jpk;
    int i=0;
    double stest=base + 0.5*(ccfred[jpk]-base);                //! -3 dB
    for (i = -11; i < 0; i++)
    {//do i=-10,0
        if (jpk+i>=-371)
        {
            if (ccfred[jpk+i]>stest) goto c30;
        }
    }
    i=0;
    c30:
    double x1=i-1.0+(stest-ccfred[jpk+i-1])/(ccfred[jpk+i]-ccfred[jpk+i-1]);

    for (i = 11; i >= 0; i--) //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    {//do i=10,0,-1
        if (jpk+i<=371)
        {
            if (ccfred[jpk+i]>stest) goto c32;
        }
    }
    i=0;
    c32:
    double x2=i+1.0-(stest-ccfred[jpk+i+1])/(ccfred[jpk+i]-ccfred[jpk+i+1]);
    width=x2-x1;
    if (width>1.2) width=sqrt(width*width - 1.44);
    width=df*width;
    width=fmax(0.0,fmin(99.0,width));
    */
}
void DecoderMs::sync65(double ss_[3413][642],double nfa,double nfb,/*int naggressive,int ntol,*/
                       int nqsym,candidate_jt65 *ca,int &ncand,int nrobust,bool bVHF)
{
    //sync65(ss_,nfa,nfb,naggressive,ntol,nhsym,ca,ncand,0,bVHF);
    //sync65(ss_,nfa,nfb,nfqso,naggressive,nhsym,ca,ncand,0,bVHF);
    //double ccf(-6000:6000)
    //double ccf_p[12000];
    //double *ccf = &ccf_p[6000];
    //naggressive=0;

    //15 za single decode triabvat mnogo kandidati
    int MAXCAND=300;//1.58=300 need for HF Features;//1.54=32+1  1.36=24+1 max from menud decode    hv no more candidats max 4 for slow speed PC 300;
    int NFFT=8192;
    const int NSZ=3413;
    double ccfred[NSZ+5];                  //!Peak of ccfblue, as function of freq

    double ccfblue_p[600]; //real ccfblue(-32:82) //double ccfblue_p[600]; //real ccfblue(-11:540)!CCF with pseudorandom sequence
    double *ccfblue = &ccfblue_p[50];//  double *ccfblue = &ccfblue_p[25];
    pomAll.zero_double_beg_end(ccfblue,-45,110); //-50,550

    //if (ntol==-99) return;                       //!Silence compiler warning

    double df=DEC_SAMPLE_RATE/(double)NFFT;                            //!df = 12000.0/8192 = 1.465 Hz
    int ia=fmax(1,int(nfa/df));//ia=fmax(2,int(nfa/df));
    int ib=fmin(NSZ-1,int(nfb/df));
    //int lag1=-11;
    //int lag2=59;
    int lag1=-32;//1.59=-32 tested   -32;
    int lag2=100;//1.59=100 tested   82;
    int nsym=126;
    ncand=0; //0
    double fdot=0.0;
    pomAll.zero_double_beg_end(ccfred,0,NSZ);
    double ccfmax=0.0;
    //int ipk=0; //1.59= no used
    double ccf0 = 0.0;
    double flip = 0.0;

    //moved to first start decoder TGen65->setup65();

    for (int i = ia; i < ib; i++)//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    {//do i=ia,ib
        int lagpk0 = 0;
        //qDebug()<<"1 ssss ncand IN"<<i;
        xcor(ss_,i,nqsym,nsym,lag1,lag2,ccfblue,ccf0,lagpk0,flip,fdot,nrobust);
        //! Remove best-fit slope from ccfblue and normalize so baseline rms=1.0
        ///////////slope(ccfblue(lag1),lag2-lag1+1,lagpk0-lag1+1.0)
        if (!bVHF) slope(ccfblue,lag1,lag2-lag1+2,lagpk0-lag1+0.0);//1.59=lag2-lag1+2 tested
        ccfred[i]=ccfblue[lagpk0];
        if (ccfred[i]>ccfmax)
        {
            ccfmax=ccfred[i];
            //ipk=i;//1.59= no used
        }
        //if(lagpk0<-30 || lagpk0>80)
        //qDebug()<<"lagpk0="<<lagpk0;
    }

    double t_ccfred[NSZ];
    for (int i = 0; i < ib; i++)
        t_ccfred[i]=ccfred[i+ia];
    double xmed = pomAll.pctile_shell(t_ccfred,ib-ia+1,35);//call pctile(ccfred(ia:ib),ib-ia+1,35,xmed)
    for (int i = ia; i < ib; i++)
        ccfred[i]=ccfred[i]-xmed;//ccfred(ia:ib)=ccfred(ia:ib)-xmed
    ccfred[ia-1]=ccfred[ia];//ccfred(ia-1)=ccfred(ia)
    ccfred[ib]=ccfred[ib-1];//  ccfred(ib+1)=ccfred(ib)

    //qDebug()<<"xmed==="<<xmed<<ccfmax;

    for (int i = ia; i < ib; i++)//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    {//do i=ia,ib
        double freq=(double)i*df;
        //qDebug()<<"sync65 ipk"<<freq<<freq;
        int itry=0;
        //!     if(naggressive.gt.0 .and. ntol.lt.1000 .and. ccfmax.ge.thresh0) then
        //if(naggressive.gt.0 .and. ccfmax.ge.thresh0) then
        //1.59= no used in HF naggressive = 0 for single I have other procedure -> sync65_single(  );
        //if (naggressive>0 && ccfmax>=thresh0_jt65 && ccfred[i]<1000.0)//1.36 ccfred[i]<1000.0 no qrm
        //{
        //    if (i!=ipk) continue;//cycle
        //    itry=1;
        //    //ncand++;
        //}
        //else
        //{
        //if(ccfred(i).ge.thresh0 .and. ccfred(i).gt.ccfred(i-1) .and. ccfred(i).gt.ccfred(i+1)) then
        //if (ccfred[i]>0.4)
        //1.36 ccfred[i]<1000.0 no qrm
        //qDebug()<<"ccfred==="<<ccfred[i]<<(double)i*df<<thresh0_jt65;
        //1.52 moze da se digne na thresh0_jt65=1.0
        //   if(naggressive>0 .and. ntol<1000 .and. ccfmax>=thresh0) then
        //   if(naggressive>0 .and. ccfmax>=thresh0) then
        //   if(ccfred(i)>=thresh0 .and. ccfred(i)>ccfred(i-1) .and. ccfred(i)>ccfred(i+1)) then
        if (ccfred[i]>=thresh0_jt65 && ccfred[i]>ccfred[i-1] && ccfred[i]>ccfred[i+1] && ccfred[i]<1000.0)// && ccfred[i]<1000.0  thresh0_jt65
        {
            //qDebug()<<"ccfred000==="<<ccfred[i]<<thresh0_jt65;//*100.0
            //if(ccfred[i-1]>ccfred[i-2] && ccfred[i+1]>ccfred[i+2])
            itry=2;
            //ncand++;
        }
        //}
        //qDebug()<<"ccfred==="<<itry;
        if (itry!=0)
        {
            //qDebug()<<"sync65 ipk"<<freq<<ia<<i<<itry<<"ccfmax=="<<ccfmax<<thresh0_jt65<<"naggressive="<<naggressive;
            int lagpk = 0;
            xcor(ss_,i,nqsym,nsym,lag1,lag2,ccfblue,ccf0,lagpk,flip,fdot,nrobust);
            //if(lagpk<-20 || lagpk>70)
            //qDebug()<<"lagpk="<<lagpk;
            //////////slope(ccfblue(lag1),lag2-lag1+1,lagpk-lag1+1.0)
            if (!bVHF) slope(ccfblue,lag1,lag2-lag1+0,lagpk-lag1+0.0);//1.58=lag2-lag1+0 tested
            double xlag=lagpk;
            if (lagpk>lag1 && lagpk<lag2)
            {
                //call peakup(ccfblue(lagpk-1),ccfmax,ccfblue(lagpk+1),dx2)
                double dx2 = pomAll.peakup(ccfblue[lagpk-1],ccfmax,ccfblue[lagpk+1]);
                xlag=lagpk+dx2;
            }
            //double dtx=xlag*2048.0/11025.0;
            double dtx=xlag*1024.0/11025.0;  //dtx=xlag*1024.0/11025.0
            if (dtx < -2.5) continue; //1.58= no need -2.5s candidats

            ccfblue[lag1]=0.0;
            ccfblue[lag2]=0.0;
            ca[ncand].freq=freq;
            ca[ncand].dt=dtx;   //qDebug()<<"sync65 ipk"<<freq;
            ca[ncand].flip=flip;
            if (bVHF)
                ca[ncand].sync=pomAll.db(ccfred[i]) + 1;//1.52=+1 1.49=+2 1.48=+1 - 16.0; + 2 pri thresh0_jt65=0.4
            else
                ca[ncand].sync=ccfred[i];
            //qDebug()<<"final ccfred[i]"<<ca[ncand].sync<<ccfred[i];
            ncand++;
        }
        //qDebug()<<"ccfred==="<<ncand;
        if (ncand>=MAXCAND) return;
    }
}

double DecoderMs::fchisq0(double *y,int npts,double *a)
{
    //real y(npts),a(4)
    //!  rewind 51
    double chisq = 0.0;
    for (int i = 0; i < npts; i++)
    {//do i=1,npts
        double x=i;
        double z=(x-a[2])/(0.5*a[3]);//z=(x-a(3))/(0.5*a(4))
        double yfit=a[0]; //yfit=a(1)
        if (fabs(z)<3.0)
        {
            double d=1.0 + z*z;
            yfit=a[0] + a[1] * (1.0/d - 0.1);
        }
        chisq=chisq + (y[i] - yfit)*(y[i] - yfit);// chisq=chisq + (y(i) - yfit)**2
        //!     write(51,3001) i,y(i),yfit,y(i)-yfit
        //!3001 format(i5,3f10.4)
    }
    //fchisq0=chisq
    return chisq;
}

void DecoderMs::lorentzian(double *y,int npts,double *a)
{
    /*
    ! Input:  y(npts); assume x(i)=i, i=1,npts
    ! Output: a(5)
    !         a(1) = baseline
    !         a(2) = amplitude
    !         a(3) = x0
    !         a(4) = width
    !         a(5) = chisqr
    */
    pomAll.zero_double_beg_end(a,0,5);//a=0.0;
    //double df=DEC_SAMPLE_RATE/8192.0;                               //!df = 1.465 Hz
    double width=0.0;
    int ipk=0;
    double ymax=-1.e30;
    double deltaa[4];

    for (int i = 0; i < npts; i++) //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    {//do i=1,npts
        if (y[i]>ymax)
        {
            ymax=y[i];
            ipk=i;
        }
        //!     write(50,3001) i,i*df,y(i)
        //!3001 format(i6,2f12.3)
    }
    //ipk=467;
    //qDebug()<<"Lorentzian ipk"<<npts<<ipk;
    //!  base=(sum(y(ipk-149:ipk-50)) + sum(y(ipk+51:ipk+150)))/200.0
    double base = 0.0; //base=(sum(y(1:20)) + sum(y(npts-19:npts)))/40.0
    for (int i = 0; i < 20; i++)
    {
        base += (y[i] + y[i+(npts-20)]);
    }
    base = base/40.0;
    double stest=ymax - 0.5*(ymax-base);
    double ssum=y[ipk];
    for (int i = 0; i < 50; i++)
    {//do i=1,50
        if (ipk+i>npts) break; //exit   //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
        if (y[ipk+i]<stest) break; //exit
        ssum=ssum + y[ipk+i];
    }
    for (int i = 0; i < 50; i++)
    {//do i=1,50
        if (ipk-i<0) break; //exit  if(ipk-i.lt.1) exit
        if (y[ipk-i]<stest) break; //exit
        ssum=ssum + y[ipk-i];
    }
    double ww=ssum/y[ipk];
    width=2.0;
    double t=ww*ww - 5.67;
    if (t>0.0) width=sqrt(t);
    a[0]=base;
    a[1]=ymax-base;
    a[2]=(double)ipk;
    a[3]=width;

    //qDebug()<<"0 Lorentzian def"<<a[0]<<a[1]<<a[2]<<a[3]<<a[4];

    //! Now find Lorentzian parameters

    deltaa[0]=0.1;
    deltaa[1]=0.1;
    deltaa[2]=1.0;
    deltaa[3]=1.0;
    int nterms=4;

    //!  Start the iteration
    double chisqr=0.0;
    double chisqr0=1.e6;
    int end_c = 0;
    //qDebug()<<"IN Lorentzian";
    for (int iter = 0; iter < 5; iter++)
    {//do iter=1,5
        for (int j = 0; j < nterms; j++)
        {//do j=1,nterms
            double chisq1=fchisq0(y,npts,a);
            double fn=0.0;
            double delta=deltaa[j];
            double chisq2=0.0;
            double chisq3=0.0;

            end_c = 0;
c10:
            //qDebug()<<"0 Lorentzian delta===="<<chisq2<<chisq1<<j;
            a[j]=a[j]+delta;
            chisq2=fchisq0(y,npts,a);
            if (chisq2==chisq1)
            {
                if (end_c>500)//HV max 501 interations else out
                {
                    //qDebug()<<"Lorentzian LOOP=1 NO END ==== j="<<j<<"end_c="<<end_c<<"width="<<a[3];
                    a[0]=1.0;
                    a[1]=1.0;
                    a[2]=1.0;
                    a[3]=3.0;
                    goto c_fall_nan;
                }
                else
                    end_c++;

                goto c10;
            }
            if (chisq2>chisq1)
            {
                delta=-delta;                      //!Reverse direction //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
                a[j]=a[j]+delta;
                double tmp=chisq1;
                chisq1=chisq2;
                chisq2=tmp;
            }

            end_c = 0;
c20:
            fn=fn+1.0;
            a[j]=a[j]+delta;
            chisq3=fchisq0(y,npts,a);
            if (chisq3<chisq2)
            {
                chisq1=chisq2;
                chisq2=chisq3;
                if (end_c>500)//HV max 501 interations else out
                {
                    //qDebug()<<"Lorentzian LOOP=2 NO END ==== j="<<j<<"end_c="<<end_c<<"width="<<a[3];
                    a[0]=1.0;
                    a[1]=1.0;
                    a[2]=1.0;
                    a[3]=3.0;
                    goto c_fall_nan;
                }
                else
                    end_c++;

                goto c20;
            }
            //! Find minimum of parabola defined by last three points
            //delta=delta*(1.0/(1.0+(chisq1-chisq2)/(chisq3-chisq2))+0.5);
            delta=delta*(1.0/(1.0+(chisq1-chisq2)/(chisq3-chisq2))+0.60);//1.53=(+0.60)-testedHV
            a[j]=a[j]-delta;
            deltaa[j]=deltaa[j]*fn/3.0;
        }
        chisqr=fchisq0(y,npts,a);
        //!       write(*,4000) 0,0,a,chisqr
        if (chisqr/chisqr0>0.99) break; // exit
        chisqr0=chisqr;

c_fall_nan:
        continue;
    }
    //qDebug()<<"otu Lorentzian";
    a[4]=chisqr;
}
void DecoderMs::flat65(double ss_[3413][642],int nhsym,int nsz,double *ref)
{
    //real stmp(nsz)
    //real ss(322,3413)
    //real ref(nsz)
    //MAXHSY = 322
    //nsz = 3413
    double stmp[3413+2];//[nsz]
    int npct=28;                                       //!Somewhat arbitrary
    double temp[3413+2];//[nsz]

    //qDebug()<<"nhsym="<<nhsym;

    for (int i = 0; i < nsz; i++)
    {//do i=1,nsz
        //for (int z = 0; z < nhsym; z++)
        //temp[z]=ss_[z][0];
        stmp[i]=pomAll.pctile_shell(ss_[i],nhsym,npct); //pctile(ss(1,i),nhsym,npct,stmp(i))
    }

    int nsmo=33;
    int ia=nsmo/2 + 0; // hv ? ia=nsmo/2 + 1
    int ib=nsz - nsmo/2 - 0; // hv ? ib=nsz - nsmo/2 - 1
    for (int i = ia; i < ib; i++)
    {//do i=ia,ib
        for (int z = 0; z < nsmo; z++)
            temp[z]=stmp[(i-nsmo/2)+z];
        ref[i]=pomAll.pctile_shell(temp,nsmo,npct); //pctile(stmp(i-nsmo/2),nsmo,npct,ref(i))
    }

    for (int i = 0; i < ia; i++)
    {
        ref[i]=ref[ia];   //ref(:ia-1)=ref(ia)
    }
    for (int i = ib; i < nsz; i++)
    {
        ref[i]=ref[ib-1];  //ref(ib+1:)=ref(ib)
    }
    for (int i = 0; i < nsz; i++)
        ref[i]=4.0*ref[i];     //ref=4.0*ref
}

void DecoderMs::symspec65(double *dd,int npts,double ss_[3413][642],int &nqsym,double *savg)
{
    int NSZ=3413;
    const int NFFT=8192;

    double hstep=2048.0*DEC_SAMPLE_RATE/11025.0;              //!half-symbol = 2229.116 samples
    double qstep=hstep/2.0;                                  // !quarter-symbol = 1114.558 samples
    //int nsps=(int)(2.0*hstep);
    //  int nsps=(int)(2.0*qstep);//hv 1.52 tested modification
    double df=(double)DEC_SAMPLE_RATE/NFFT;
    double fac1=1.e-3;//double fac1=1.e-3;
    double complex c[NFFT+100];//2.09 error -> c[NFFT/2+255];   //complex c(0:NFFT/2)
    double x[NFFT+100];             //real*4 x(NFFT)

    //double nhsym=(double)(npts-NFFT)/hstep; // ne >322
    nqsym=(double)(npts-NFFT)/qstep;//nqsym=638 nhsym=319.323
    //qDebug()<<"nqsym nhsym="<<nqsym<<nqsym*qstep+NFFT<<npts; //nqsym*qstep=711088  dd=720000

    pomAll.zero_double_beg_end(savg,0,NSZ);

    //for (int i = 0; i < NSZ; i++)
    // savg[i] = 0.0;

    if (first_symspec65)
    {
        //! Compute the FFT window//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
        //pi=4.0*atan(1.0)
        //   double width=0.25*(double)nsps;//hv 1.52 tested modification
        for (int i = 0; i < NFFT; i++)
        {//do i=1,NFFT
            //       double z=(double)((double)(i-NFFT)/2.0)/width;//hv 1.52 tested modification
            //       w_symspec65[i]=exp(-z*z);//hv 1.52 tested modification
            w_symspec65[i]=1.0;//hv stoped 1.52 tested modification
            if (i>4458) w_symspec65[i]=0.0;//hv stoped 1.52 tested modification
        }
        first_symspec65 = false;
    }

    //for (int j = 0; j < nhsym; j++)
    for (int j = 0; j < nqsym; j++)
    {//do j=1,nhsym
        //int i0=(j-0)*hstep; //i0=(j-1)*hstep
        int i0=(j-0)*qstep; //i0=(j-1)*hstep

        for (int z = 0; z < NFFT; z++)
            x[z]=fac1*w_symspec65[z]*dd[i0+0+z];// hv 0 +1 ????   x=fac1*w*dd(i0+1:i0+NFFT)

        f2a.four2a_d2c(c,x,NFFT,-1,0);//call four2a(c,NFFT,1,-1,0)                //!r2c forward FFT
        for (int i = 0; i < NSZ; i++)
        {//do i=1,NSZ
            double s=pomAll.ps_hv(c[i]);                 //s=real(c(i))**2 + aimag(c(i))**2
            ss_[i][j]=s;
            savg[i]=savg[i]+s;
        }
    }

    for (int z = 0; z < NSZ; z++)
        //savg[z]=savg[z]/(double)nhsym;
        savg[z]=savg[z]/(double)nqsym;

    //flat65(ss_,nhsym,NSZ,ref_jt65);      //!Flatten the 2d spectrum, saving
    flat65(ss_,nqsym,NSZ,ref_jt65);

    dfref_jt65=df;  //HV ?????                   //! the reference spectrum ref()



    for (int z = 0; z < NSZ; z++)
    {
        double divid = ref_jt65[z];
        if (divid==0.0)//no div by zero
            divid=1.0;
        savg[z]=savg[z]/divid;
    }

    //for (int j = 0; j < nhsym; j++)
    for (int j = 0; j < nqsym; j++)
    {//do j=1,nhsym
        for (int z = 0; z < NSZ; z++)
        {
            double divid = ref_jt65[z];
            if (divid==0.0)//no div by zero
                divid=1.0;
            ss_[z][j]=ss_[z][j]/divid;      //ss(j,1:NSZ)=ss(j,1:NSZ)/ref
        }
    }
}

void DecoderMs::avg65(int nutc,int &nsave,double snrsync,double dtxx,int nflip,int nfreq,int mode65,int ntol,
                      bool f_deep_search,bool nagain,int ntrials,int naggressive,int neme,QString mycall,
                      QString hiscall,QString hisgrid,int &nftt,QString &avemsg,double &qave,QString &deepave,
                      int &nsum,int ndeepave,int nQSOProgress,bool ljt65apon)
{

    int MAXAVE=24;//64 wsjt-x  hv 24
    double syncsum,dtsum;
    int nfsum;

    double s3b_[63][64];//(64,63)
    double s3c_[63][64];//(64,63)
    double s1b_[126][512+40];//(-255:256,126)
    double s2_[126][66];//(66,126);//real s2(66,126)
    int nn=0;
    double qualbest;
    int minsmo;
    int maxsmo;
    double df;

    //// moze da sa ob6ti ??
    int nsmo = 0;
    QString deepbest;
    int nsmobest=0;
    int nfttbest=0;
    ////end moze da sa ob6ti ??
    nutc = nutc/100;//hv odd even minute 60s period
    //bool f_p1_p2;

    if (first_avg65 || clearave_jt65)
    {
        clearave_jt65 = false;
        for (int i = 0; i < MAXAVE; i++)
        {
            iutc_jt65[i]=-1;
            nfsave_jt65[i]=0;

            for (int j = 0; j < 63; j++)
            {
                for (int k = 0; k < 64; k++)
                    s3save_jt65_[i][j][k]=0.0;  //double s3save_jt65_[64][63][64]; //real s3save(64,63,MAXAVE)
            }
            for (int j = 0; j < 126; j++)
            {
                for (int k = 0; k < 512+20; k++)
                    s1save_jt65_[i][j][k]=0.0;   // double s1save_jt65_[64][126][512+40];  //double s1save(-255:256,126,MAXAVE)
            }
        }
        dtdiff_jt65=0.2;
        nsave=0;   //hv 1        //!### ???

        count_saved_avgs_1jt65 = 0;
        count_saved_avgs_2jt65 = 0;
        count_use_avgs_1jt65 = 0;
        count_use_avgs_2jt65 = 0;

        first_avg65=false;
        //! Silence compiler warnings
        if (nagain && ndeepave==-99 && neme==-99) return; //stop
    }
    for (int i = 0; i < MAXAVE; i++)
    {//do i=1,64    //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
        if (iutc_jt65[i]<0) break; //exit
        if (nutc==iutc_jt65[i] && fabs(nfreq-nfsave_jt65[i])<=ntol) goto c10;
    }

    //qDebug()<<(nutc % 2);

    if (nutc % 2 == 0) //is even=0,2,4....
    {
        if (iutc_jt65[nsave] < 0 /*&& (count_saved_avgs_1jt65+count_saved_avgs_2jt65)<MAXAVE*/)
            count_saved_avgs_1jt65++;
        else if (iutc_jt65[nsave] % 2 == 1)//odd
        {
            count_saved_avgs_2jt65--;
            count_saved_avgs_1jt65++;
        }
    }
    else                //is odd=1,3,5.....
    {
        if (iutc_jt65[nsave] < 0 /*&& (count_saved_avgs_1jt65+count_saved_avgs_2jt65)<MAXAVE*/)
            count_saved_avgs_2jt65++;
        else if (iutc_jt65[nsave] % 2 == 0)//even
        {
            count_saved_avgs_2jt65++;
            count_saved_avgs_1jt65--;
        }
    }

    //Save data for message averaging
    iutc_jt65[nsave]=nutc;
    syncsave_jt65[nsave]=snrsync;
    dtsave_jt65[nsave]=dtxx;
    nfsave_jt65[nsave]=nfreq;
    nflipsave_jt65[nsave]=nflip;
    for (int i = 0; i < 126; i++)
    {
        for (int j = -256; j < 256; j++)
            s1save_jt65_[nsave][i][j+s1_ofs_jt65]=s1_jt65_[i][j+s1_ofs_jt65];//s1save_jt65_[64][126][512+40];  s1save(-255:256,1:126,nsave)=s1;
    }
    for (int i = 0; i < 63; i++)
    {
        for (int j = 0; j < 64; j++)
            s3save_jt65_[nsave][i][j]=s3a_jt65_[i][j];    //s3save(1:64,1:63,nsave)=s3a;
    }
    //qDebug()<<"1nsave="<<nsave;
    nsave++;


    /*if ((count_saved_avgs_1jt65+count_saved_avgs_2jt65)<MAXAVE)
    {
        if (nutc % 2 == 0) //is even=0,2,4....
            count_saved_avgs_1jt65++;
        else                //is odd=1,3,5.....
            count_saved_avgs_2jt65++;
    }*/
    //emit EmitAvgSaves(count_saved_avgs_1jt65,count_saved_avgs_2jt65);

c10:
    syncsum=0.0;
    dtsum=0.0;
    nfsum=0;
    //qDebug()<<"1nsum="<<nsum;
    nsum=0;
    for (int i = 0; i < 126; i++)
    {
        for (int j = 0; j < 512+20; j++)
            s1b_[i][j]=0.0;//s1b=0.0;
    }
    for (int i = 0; i < 63; i++)
    {
        for (int j = 0; j < 64; j++)
        {
            s3b_[i][j]=0.0;//s3b=0.0;
            s3c_[i][j]=0.0;//s3c=0.0;
        }
    }
    //qDebug()<<"1 AVERAGE==========";
    for (int i = 0; i < MAXAVE; i++)//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    {//do i=1,MAXAVE   //!Consider all saved spectra
        //if (i<nsave)
        //qDebug()<<"2 AVERAGE=========="<<iutc_jt65[i]<<fabs(nfreq-nfsave_jt65[i])<<ntol;
        cused_jt65[i]='.';
        if (iutc_jt65[i]<0)
        {
            continue;
        }  //cycle
        if (fmod(iutc_jt65[i],2)!=fmod(nutc,2))
        {/*qDebug()<<"2AUT"<<nutc;*/continue;
        }//cycle  //!Use only same (odd/even) seq
        if (fabs(dtxx-dtsave_jt65[i])>dtdiff_jt65)
        {/*qDebug()<<"3AUT"<<dtxx-dtsave_jt65[i]<<dtdiff_jt65<<i;*/continue;
        }//cycle  //!DT must match
        if (fabs(nfreq-nfsave_jt65[i])>ntol)
        {/*qDebug()<<"4AUT";*/continue;
        }//cycle   //!Freq must match
        if (nflip!=nflipsave_jt65[i])
        {/*qDebug()<<"5AUT";*/continue;
        }//cycle          //!Sync type (* / #) must match

        for (int z = 0; z < 63; z++)
        {
            for (int x = 0; x < 64; x++)
                s3b_[z][x]+=s3save_jt65_[i][z][x];    //s3b=s3b + s3save(1:64,1:63,i)
        }
        for (int z = 0; z < 126; z++)
        {
            for (int x = -256; x < 256; x++)
                s1b_[z][x+s1_ofs_jt65]+=s1save_jt65_[i][z][x+s1_ofs_jt65];    //s1b=s1b + s1save(-255:256,1:126,i)
        }
        syncsum+=syncsave_jt65[i];
        dtsum+=dtsave_jt65[i];
        nfsum+=nfsave_jt65[i];
        cused_jt65[i]='$';
        iused_jt65[nsum]=i;
        //qDebug()<<"SUM FROM="<<iutc_jt65[i];
        nsum++;
    }

    //if (nsum<64-1) iused_jt65[nsum+1]=0; ////c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    if (nsum<MAXAVE-0) iused_jt65[nsum+0]=0;

    /*syncave=0.
    dtave=0.
    fave=0.
    if(nsum.gt.0) then
       syncave=syncsum/nsum
       dtave=dtsum/nsum
       fave=float(nfsum)/nsum
    endif*/

    for (int i = 0; i < nsave; i++)//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    {//do i=1,nsave
        csync_jt65='*';
        if (nflipsave_jt65[i]<0.0) csync_jt65='#';//if(nflipsave(i).lt.0.0) csync='#'
        //write(14,1000) cused(i),iutc(i),syncsave(i),dtsave(i)-1.0,nfsave(i),csync
        //1000   format(a1,i5.4,f6.1,f6.2,i6,1x,a1)
    }

    //qDebug()<<"2 AVERAGE==========uuuuuuuuuuuuuu========nsum----------"<<nsum;
    if (nsum<2)
    {
        emit EmitAvgSaves(count_use_avgs_1jt65,count_saved_avgs_1jt65,count_use_avgs_2jt65,count_saved_avgs_2jt65);
        goto c900; //ne pomalko ot 1 da ima if(nsum<2) go to 900
    }

    if (nutc % 2 == 0)  //is even=0,2,4....
        count_use_avgs_1jt65 = nsum;
    else                //is odd=1,3,5.....
        count_use_avgs_2jt65 = nsum;

    emit EmitAvgSaves(count_use_avgs_1jt65,count_saved_avgs_1jt65,count_use_avgs_2jt65,count_saved_avgs_2jt65);

    nftt=0;
    df=1378.125/512.0;

    //! Do the smoothing loop
    qualbest=0.0;
    minsmo=0;
    maxsmo=0;
    if (mode65>=2)
    {
        minsmo=int(width_jt65/df);
        maxsmo=2*minsmo;
    }
    nn=-1;//0
    //qDebug()<<"2 AVERAGE==========uuuuuuuuuuuuuu========nsumuuuu"<<maxsmo;
    for (int ismo = minsmo; ismo <= maxsmo; ismo++)//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    {//do ismo=minsmo,maxsmo
        if (ismo>0)
        {
            for (int j = 0; j < 126; j++)
            {//do j=1,126
                smo121(s1b_[j],(-256+s1_ofs_jt65),512);//smo121(s1b(-255,j),512)
                if (j==0) nn=nn+1;//if(j==1) nn=nn+1;
                if (nn>=3)//4
                {
                    smo121(s1b_[j],(-256+s1_ofs_jt65),512);//smo121(s1b(-255,j),512)
                    if (j==0) nn=nn+1;//if(j==1) nn=nn+1;
                }
            }
        }
        for (int i = 0; i < 66; i++)
        {//do i=1,66
            int jj=i;
            if (mode65==2) jj=2*i-0;//jj=2*i-1
            if (mode65==4)
            {
                double ff=4.0*(double)(i-0)*df - 355.297852;//ff=4*(i-1)*df - 355.297852
                jj=int(ff/df)+0;//jj=int(ff/df)+1
            }
            for (int z = 0; z < 126; z++)
                s2_[z][i]=s1b_[z][jj+s1_ofs_jt65];//s2(i,1:126)=s1b(jj,1:126)
        }

        for (int j = 0; j < 63; j++)//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
        {//do j=1,63
            int k=TGen65->mdat[j];                       //!Points to data symbol
            if (nflip<0) k=TGen65->mdat2[j];
            for (int i = 0; i < 64; i++)
            {//do i=1,64
                s3c_[j][i]=4.e-5*s2_[k][i+2];//s3c(i,j)=4.e-5*s2(i+2,k)
            }
        }

        int nadd=nsum*ismo;

        //// moze da sa ob6ti ??
        double qual = 0.0;
        int ncount = 0;
        int nhist = 0;
        bool ltext = false;
        //// end moze da sa ob6ti ??
        //int nQSOProgress = 0;//1.55= for the moment
        //bool ljt65apon = false;//1.55= for the moment
        extract(s3c_,nadd,mode65,ntrials,naggressive,f_deep_search,nflip,mycall,hiscall,hisgrid,ncount,nhist,
                avemsg,ltext,nftt,qual,nQSOProgress,ljt65apon);


        //qDebug()<<"2 AVERAGE===================================nsum="<<avemsg<<nftt<<ismo<<nsum<<ismo;
        //nftt=1;
        if (nftt==1)
        {
            nsmo=ismo;
            param_jt65[9]=nsmo;
            goto c900;
        }
        else if (nftt==2)
        {
            if (qual>qualbest)
            {
                deepbest=avemsg;
                qualbest=qual;
                //nnbest=nn;
                nsmobest=ismo;
                nfttbest=nftt;
            }
        }
    }

    if (nfttbest==2)
    {
        avemsg=deepbest;       //!### ???
        deepave=deepbest;
        qave=qualbest;
        nsmo=nsmobest;
        param_jt65[9]=nsmo;
        nftt=nfttbest;
    }

c900:
    nsave=fmod(nsave,MAXAVE);//hv if MAXAVE place chenged to 0
    //qDebug()<<"Next nsave"<<nsave;
    return;
}
void DecoderMs::AvgDecodeChanged(bool f)
{
    s_avg_jt65 = f;
    DecQ65->AvgDecodeChanged(f);
    DecFt2_0->AvgDecodeChanged(f);
    //qDebug()<<"s_avg_jt65"<<s_avg_jt65;
}
void DecoderMs::DeepSearchChanged(bool f)
{
    s_deep_search_jt65 = f;
    //qDebug()<<"s_deep_search_jt65"<<s_deep_search_jt65;
}
void DecoderMs::SetClearAvg65()
{
    clearave_jt65 = true;
    emit EmitAvgSaves(0,0,0,0); // exception
}
void DecoderMs::SetMaxCandidats65(int i)
{
    if (i==0)
        s_max65_cand_for_dec = 1;
    if (i==1)
        s_max65_cand_for_dec = 4;
    if (i==2)
        s_max65_cand_for_dec = 8;
    if (i==3)
        s_max65_cand_for_dec = 16;
    if (i==4)
        s_max65_cand_for_dec = 32;
    //qDebug()<<"s_max65_cand_for_dec="<<s_max65_cand_for_dec;
}

void DecoderMs::SetVhfUhfFeatures(bool f)
{
    s_bVHF_jt65 = f;
    //qDebug()<<"s_bVHF_jt65"<<s_bVHF_jt65;
}
void DecoderMs::SetAggresLevFtd(int val)
{
    s_aggres_lev_ftd = val;
    //qDebug()<<"s_aggres_lev_ftd"<<s_aggres_lev_ftd;
}
void DecoderMs::SetAggresLevDeepS(int val)
{
    s_aggres_lev_deeps = val;
    //qDebug()<<"s_aggres_lev_deeps"<<11.0 - 0.1*(double)s_aggres_lev_deeps;
}
void DecoderMs::jt65_decode(double *dd,int npts,int mode65)
{
    //QTime ttt;
    //ttt.start();

    int NSZ=3413;
    //double ss_[3413][322];
    double (*ss_)[642]=new double[3413][642];//552  322 nqsym=638 nqsym=319
    double savg[3413];
    //int nf1 = 200; //low frq
    //int nf2 = 4000;//hai frq

    bool bVHF;
    double a[5];

    bool nagain = false;
    bool single_decode;
    //int nexp_decode=64; //2.12 default 64=vhf   32=single_decode oba4e triabva mnogo kandidati v sync

    int ntol = G_DfTolerance;
    double tx_nfqso65 = 1270.46;
    //double nfqso = 1270.46;//1270.0;    //1270.0 //1500.0
    //double baseline = 0.0;
    //double amp = 0.0;
    double f0 = 0.0;

    candidate_jt65 ca[306];
    candidate_jt65 ca1[306];
    //candidate ca2[306];
    int indices[306];

    //bool clearave = false;// iztriva avrg datata parvona4alno go pravi ->first_avg65
    int ndecoded=0;//-1
    int minsync = G_MinSigdB; //-30 0 db za sync
    int ntry65a = 0; //poznave na kakvo e sinc
    int ntry65b = 0; //poznave na kakvo e sinc
    double flip = 0.0;

    /*int ndepth = 0;//1.49 ndepth 0=no, 1=avg, 2=avg+deep search    old-> avg=16  hint65=32
    if (s_avg_jt65)
        ndepth = 1;
    if (s_deep_search_jt65)
        ndepth = 2;*/
    bool f_deep_search_jt65 = s_deep_search_jt65;
    bool f_avg_jt65 = s_avg_jt65;

    QString mycall = s_MyBaseCall;//1.70 base_call "";
    QString hiscall = s_HisCall;//1.49 "";
    QString hisgrid = HisGridLoc;//1.49 "";
    double qual = 0.0;
    int nsmo = 0;
    bool f_only_one_color = true;
    //int nsum = 0;
    int nsubtract =0;
    int nhard_min,nrtt1000,ntotal_min;

    int nutc0 = -999;
    int nutc = s_time.toInt();//100;//odd even minute 60s period
    int nfreq0 = -999;
    //int nsave = 0;
    //double nqave=0.0;
    double qave=0.0;
    int neme = 0; // stop avg-> -99
    int ndeepave = 0; // stop avg-> -99

    int naggressive_ds = s_aggres_lev_deeps;//1.59=use only in extract( ); no used     ot  0-10 0=good decode
    int n_dagrres_l = s_aggres_lev_ftd;//1.59=

    //!                0    1    2    3    4    5    6    7    8    9   10   11
    double r0[12] ={0.70,0.72,0.74,0.76,0.78,0.80,0.82,0.84,0.86,0.88,0.90,0.90};//1.59=
    //double r0[12] ={0.86,0.88,0.90,0.92,0.94,0.96,0.98,1.0,1.02,1.04,1.06,1.06};//0.855
    //           0  1  2  3  4  5  6  7  8  9 10 11
    int h0[12]={41,42,43,43,44,45,46,47,48,48,49,49};//1.59=
    //int h0[12]={44,45,46,46,47,48,49,50,51,51,52,52};//hv coef 53=end

    int d0[12]={71,72,73,74,76,77,78,80,81,82,83,83};//1.59=
    //int d0[12]={76,77,78,79,81,82,83,85,86,87,88,88};//hv coef 71+5

    int ndupe;
    typedef struct
    {
        double freq;
        double dt;
        double sync;
        QString decoded;
    }
    accepted_decode;
    accepted_decode dec[100];//3pas*32and
    int n65a =0;
    int n65b =0;
    int nrob = 0;

    typedef struct//1.54=filter all candidats
    {
        int nsync;
        int nsnr;
        double dtx;
        int nfreq;
        double width_jt65;
        int nflip;
        int sync1;
    }
    fa_ca_par_hv;
    fa_ca_par_hv fal_cand_par_hv[33+6];//32+1=max cand for decode

    /*int n2pass=2;  //s_decoder_deep    1-fast 2-normal 3-deep
    int npass=1;
    if (n2pass > 1) npass=s_decoder_deep+1; // 2, 3, or 4 decoding passes.*/

    int nQSOProgress = s_nQSOProgress;
    bool ljt65apon = s_lapon;

    /*if ((nexp_decode & 32)!=0 || nagain) //2.12 stoped  single_decode=iand(nexp_decode,32).ne.0 .or. nagain
        single_decode=true;
    else
        single_decode=false;*/
    single_decode=false;//2.12

    /*if ((nexp_decode & 64)!=0)
        bVHF=true;
    else
        bVHF=false;*/
    bVHF = s_bVHF_jt65;//1.58=

    int ntrials = 3000;//ntrials max for vhf
    int nvec = ntrials; //= 10^(n/2) n=0-12 def=6=1000 3000;//wsjt - 10000 wsjt-x = 1000-3000 for hf=1000
    int npass = s_decoder_deep;//<-one pass only exist in soft
    //int npass =1;//1.57=!!!

    if (bVHF)
    {
        nvec=ntrials;
        npass=1;
        if (s_decoder_deep>1)//if(n2pass>1) npass=2    //s_decoder_deep = 1-fast 2-normal 3-deep
            npass=2;
    }
    else
    {
        //nvec=1000;
        if (s_decoder_deep==1)
        {
            npass=1;//npass=2
            nvec=250; //org nvec=100; for the moment=1000 one pass only exist in soft
        }
        else if (s_decoder_deep==2)
        {
            npass=2;//npass=2
            nvec=1000;
        }
        else
        {
            npass=3;//npass=4
            nvec=2000;
        }
    }
    //qDebug()<<s_mousebutton; //mousebutton Left=1, Right=3 fullfile=0 rtd=2
    double nfa=200; //1.58=
    double nfb=4000;//1.58=
    nfa=fmax(200,s_nfqso_all-ntol); //1.58=  VHF and HF
    nfb=fmin(4000,s_nfqso_all+ntol);//1.58=  VHF and HF
    if (s_mousebutton==3)//same as ft8
    {
        nfa=fmax(200,s_nfqso_all-25);//1.58=   VHF and HF
        nfb=fmin(4000,s_nfqso_all+25);//1.58=  VHF and HF
    }
    /*if (bVHF)
    {
        nfa=fmax(200,s_nfqso65-ntol); //1.58=  VHF
        nfb=fmin(4000,s_nfqso65+ntol);//1.58=  VHF
    }
    else
    {
        if (s_mousebutton==1)
        {
            nfa=fmax(200,s_nfqso65-ntol);//1.58=   HF
            nfb=fmin(4000,s_nfqso65+ntol);//1.58=  HF
        }
        if (s_mousebutton==3)
        {
            nfa=fmax(200,s_nfqso65-25);//1.58=   HF
            nfb=fmin(4000,s_nfqso65+25);//1.58=  HF
        }
    }*/
    //qDebug()<<s_mousebutton<<"FREQ="<<nfa<<nfb;

    //for (int ipass = 1; ipass <= n2pass; ipass++)
    for (int ipass = 1; ipass <= npass; ipass++)
    {//do ipass=1,n2pass
        //qDebug()<<"start ipass ="<<ipass;
        bool first_time = true;
        int nqsym = 0;

        /*if (ipass==1)
        {
            thresh0_jt65=2.5;//2.5; corect in xcoor()1.36 for double *100.0
            nsubtract=1;
            nrob=0;
        }
        else if (ipass==2)        
        {
            thresh0_jt65=2.0;//thresh0_jt65=2.5;//2.5; corect in xcoor()1.36 for double *100.0
            nsubtract=1;//nsubtract=0;
            nrob=0;
        }
        else if (ipass==3)
        {
            thresh0_jt65=2.0;
            nsubtract=1;
            nrob=0;
        }
        else if (ipass==4)
        {
            thresh0_jt65=2.0;
            nsubtract=0;
            nrob=1;
        }
        if(npass==1) 
        {
        	thresh0_jt65=2.0;
            nsubtract=0;  
            nrob=0;        
        }*/

        //hv modification
        if (ipass==1)
        {
            thresh0_jt65=2.5; //for hf  //2.5; corect in xcoor()1.36 for double *100.0
            nsubtract=1;
            nrob=0;
        }
        else if (ipass==2)
        {
            thresh0_jt65=2.0; //for hf  //thresh0_jt65=2.5;//2.5; corect in xcoor()1.36 for double *100.0
            nsubtract=1;      //nsubtract=0;
            nrob=0;
        }
        else if (ipass==3)
        {
            thresh0_jt65=2.0; //for hf
            nsubtract=0;
            nrob=1;
        }
        //if (ipass==npass) //or if last my be pass no need substract or nrob
        if (npass==1)       //if only one pass no need substract or nrob
        {
            thresh0_jt65=2.0; //+1.0 for hf
            nsubtract=0;
            nrob=0;
        }
        //end hv modification

        //if (n2pass<2) nsubtract=0;           //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
        //!  if(newdat) then

        for (int i = 0; i < NSZ; i++)
        {
            for (int j = 0; j < 642; j++)   // row major = i
                ss_[i][j]=0.0;
        }
        symspec65(dd,npts,ss_,nqsym,savg);    //!Get normalized symbol spectra
        //qDebug()<<"0NFFT==========================="<<ss_[1][1]<<ss_[1][2]<<ss_[1][3];
        //!  endif

        //double nfa=nf1;
        //double nfb=nf2;

        //qDebug()<<"0NFFT==========================="<<(nexp_decode & 32)<<(nexp_decode & 64) ;
        //!### Q: should either of the next two uses of "single_decode" be "bVHF" instead?
        //if (single_decode || (bVHF && ntol<1000))
        if (single_decode || (bVHF && ntol<1000))
        {
            //nfa=fmax(200,s_nfqso65-ntol);//nfa=fmax(200,nfqso-ntol);
            //nfb=fmin(4000,s_nfqso65+ntol);//nfb=fmin(4000,nfqso+ntol);

            //1.52 moze da se digne na thresh0_jt65=1.0
            thresh0_jt65=0.8;//0.76-izlizat 1.53=0.76 no riskovo for vhf  //tested->1.49=0.8  1.48=1.0;  corect in xcoor()1.36 for double *100.0  *63
            //thresh0_jt65=1.0;
            /*if (ipass==1)         //!First-pass parameters
                thresh0_jt65=3.0; //for vhf
            else if (ipass==2)
                thresh0_jt65=2.0; //for vhf
            else if (ipass==3)
                thresh0_jt65=0.8; //for vhf*/
            //if (npass==1)
            //thresh0_jt65=0.8; //for vhf
        }
        //qDebug()<<"ipass="<<ipass<<"npass="<<npass<<"bVHF="<<bVHF<<"nvec="<<nvec<<"thresh0_jt65"<<thresh0_jt65<<"nsubtract="<<nsubtract;
        double df=DEC_SAMPLE_RATE/8192.0 ;                   // !df = 1.465 Hz
        if (bVHF)
        {
            //ntol = 200;
            int ia=fmax(193,int(nfa/df)-ntol);  //193 barka width 0   max(1,nint(nfa/df)-ntol)
            int ib=fmin(NSZ-2,int(nfb/df)+ntol);
            int nz=ib-ia+0; //nz=ib-ia+1;
            //qDebug()<<"ia="<<ia<<ib<<nz;
            //ia=0;
            //ib=NSZ;
            //ia=193;

            double *t_savg = new double[nz];//t_savg[nz];
            for (int i = 0; i < nz; i++)
                t_savg[i]=savg[ia+i];

            //qDebug()<<"IN lorentzian=== ia="<<ia<<"ib="<<ib<<nz<<nfa<<nfa/df<<ntol;

            lorentzian(t_savg,nz,a); //call lorentzian(savg(ia),nz,a)
            delete [] t_savg;
            //qDebug()<<"3All=";
            //baseline=a[0]; //baseline=a(1)
            //amp=a[1];  //a[2]=834.683;
            f0=(a[2]+ia-0)*df;   //f0=(a(3)+ia-1)*df
            width_jt65=a[3]*df;
            //qDebug()<<"OUT lorentzian="<<a[0]<<a[1]<<f0<<a[3]<<a[4]<<a[2]<<"naggressive"<<naggressive<<width_jt65;
        }
        else
            width_jt65=2*df;//HF fictive

        int ncand = 0;
        double snrx_single;
        if (s_max65_cand_for_dec==1)
        {
            double dtx1,dfx1,snrsync1,flip1;//,snrx1,width1;
            //sync65_single(ss_,nfa,nfb,nqsym,dtx1,dfx1,snrx_single,snrsync1,flip1,/*width1,*/savg,mode65);
            sync65_single(ss_,nfa,nfb,nqsym,dtx1,dfx1,snrx_single,snrsync1,flip1,/*width1,*/savg,mode65,nrob);//1.52+nrob
            //qDebug()<<"single sync="<<snrsync1/*<<"snrx="<<snrx1*/<<"dt="<<dtx1<<"freq="<<dfx1<<"flip="<<flip1;//<<"width="<<width1;
            ca[0].freq = dfx1;
            ca[0].dt   = dtx1;
            ca[0].sync = snrsync1;
            ca[0].flip = flip1;
            ncand++;

            /*ca[ncand].sync=5.0000000123985;
            ca[ncand].dt=2.5;
            ca[ncand].freq=s_nfqso65;
            ca[ncand].flip=1;
            ncand++;*/
        }
        else
        {
            //qDebug()<<"1ncand IN";
            sync65(ss_,nfa,nfb,/*naggressive,ntol,*/nqsym,ca,ncand,nrob,bVHF);
            //sync65(ss_,nfa,nfb,naggressive,/*ntol,*/nqsym,ca,ncand,nrob,bVHF);
            //qDebug()<<"ALL ncand==============================="<<ncand;
            //! If a candidate was found within +/- ntol of nfqso, move it into ca(1).
            //fqso_first(s_nfqso65,ntol,ca1,ncand);//1.59= no need this here
            //fqso_first(s_nfqso65,ntol,ca,ncand);

            if (ncand>0)
            {
                //reorder by max sync
                double *t_sync = new double[ncand+4];//t_sync[ncand+4];
                for (int i = 0; i < ncand; i++)
                    t_sync[i]=ca[i].sync;
                pomAll.indexx_msk(t_sync,ncand-1,indices);
                int zz = 0;
                for (int i = ncand-1; i >=0; i--)
                {
                    int reord = indices[i];
                    ca1[zz].freq = ca[reord].freq;
                    ca1[zz].dt   = ca[reord].dt;
                    ca1[zz].sync = ca[reord].sync;
                    ca1[zz].flip = ca[reord].flip;
                    zz++;
                }
                if (ncand > s_max65_cand_for_dec)
                    ncand=s_max65_cand_for_dec;

                //reorder by low freq
                for (int i = 0; i < ncand; i++)
                    t_sync[i]=ca1[i].freq;
                pomAll.indexx_msk(t_sync,ncand-1,indices);
                delete [] t_sync;
                zz = 0;
                //for (int i = ncand-1; i >=0; i--)
                for (int i = 0; i <ncand; i++)
                {
                    int reord = indices[i];
                    ca[zz].freq = ca1[reord].freq;
                    ca[zz].dt   = ca1[reord].dt;
                    ca[zz].sync = ca1[reord].sync;
                    ca[zz].flip = ca1[reord].flip;
                    zz++;
                }
                //lets signal in RX freq be first
                fqso_first(s_nfqso_all,ntol,ca,ncand);//1.59= here from ver1.59
            }
        }


        if (single_decode)
        {
            //qDebug()<<"single_decode="<<ncand;
            if (ncand==0)  ncand=1;//if(ncand.eq.0) ncand=1 if(abs(ca(1)%freq - f0).gt.width) width=2*df
            if (fabs(ca[0].freq - f0)>width_jt65) width_jt65=2*df;  // !### ??? ### //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
        }


        //int nvec=ntrials;
        //if (ncand>75)
        //nvec=100;

        //mode65=2**nsubmode
        int nflip=1;
        int nqd=0;
        QString decoded;
        QString decoded0="*";//1.54=
        double freq0=0.0;
        bool prtavg=false;
        nsum_jt65 = 0;
        if (!nagain) nsum_jt65=0;
        if (clearave_jt65)
        {
            nsum_jt65=0;
            nsave_jt65=0;
        }

        if (bVHF && ncand<300 && s_max65_cand_for_dec!=1) //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
        {
            //! Be sure to search for shorthand message at nfqso +/- ntol
            //qDebug()<<"1 bVHF="<<ncand;
            //ca[ncand].sync=5.0;
            ca[ncand].sync=5.0000000123985;
            ca[ncand].dt=2.5;
            ca[ncand].freq=s_nfqso_all;
            ca[ncand].flip=1;
            ncand++; //if(ncand.lt.300) ncand=ncand+1
            //qDebug()<<"2 bVHF="<<ncand;
        }
        //for (int i = 0; i < ncand; i++)
        //qDebug()<<"All cand="<<i<<ca[i].sync<<ca[i].dt<<ca[i].freq<<ca[i].flip;

        int fal_ca_nsnr_max_hv = -100;//1.54=filter all candidats
        int fal_ca_icand_hv = 0;//1.54=filter all candidats
        bool min_1good_cand_hv = false;//1.54=filter all candidats

        for (int icand = 0; icand < ncand; icand++)
        {//do icand=1,ncand

            //qDebug()<<"Cand="<<icand<<ncand<<ca[icand].sync<<ca[icand].dt<<ca[icand].freq<<ca[icand].flip;
            //hv vremenno za mahane
            bool fal_cand_hv;//1.54=folter all candidats
            int nftt,nft,nspecial;
            QString avemsg;
            QString deepave;
            int nfreq;
            decoded = "";
            //int n;
            double rtt,s2db;

            double sync1=ca[icand].sync;
            double dtx=ca[icand].dt;
            double freq=ca[icand].freq;//ca[icand].freq;1524.0;
            //qDebug()<<"All="<<sync1<<dtx<<freq;//6.35803 7.57726 1589.36
            //freq = 1270.0;

            double sync2 = 0.0;
            if (bVHF)
            {
                flip=ca[icand].flip;
                nflip=flip;
                //qDebug()<<"nflipppppppppppp="<<nflip<<sync1<<minsync;
            }
            if (sync1<double(minsync))
            {
                nflip=0;
            }// goto next_cand;
            //qDebug()<<"nfliSSSSSSSSSSSSSSSS="<<nflip<<sync1<<minsync;
            if (ipass==1) ntry65a++; //if(ipass.eq.1) ntry65a=ntry65a + 1
            if (ipass==2) ntry65b++; //if(ipass.eq.2) ntry65b=ntry65b + 1
            nft=0;
            nspecial=0;
            double sync_sh_single;
            //qDebug()<<"first_time decode65a="<<first_time;
            //ntol = 100;
            decode65a(dd,npts,first_time,nqd,freq,nflip,mode65,nvec,
                      naggressive_ds,f_deep_search_jt65,ntol,mycall,hiscall,hisgrid,
                      bVHF,sync2,a,dtx,nft,nspecial,qual,
                      nsmo,decoded,sync_sh_single,nQSOProgress,ljt65apon);
            //qDebug()<<"END decode65a="<<decoded<<nspecial;
            if (nspecial==2) decoded="RO";
            if (nspecial==3) decoded="RRR";
            if (nspecial==4) decoded="73";
            //if (nspecial>0 ) min_1good_cand_hv = true;//1.54=no need here nspecial take care-> filter all candidats

            //qDebug()<<"end AVERAGE=========="<<nft<<nflip<<freq<<dtx<<sync1<<decoded;
            //nft=0;

            if (sync1<double(minsync) && decoded.isEmpty())
            {
                nflip=0;
            }
            if (nft!=0) nsum_jt65=1;

            nhard_min=param_jt65[1]; //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
            nrtt1000=param_jt65[4];
            ntotal_min=param_jt65[5];
            nsmo=param_jt65[9];

            //qDebug()<<"All Cand dtx freq"<<icand<<dtx<<freq<<ca[icand].flip<<a[0];

            nfreq=int(freq+a[0]);

            //int ndrift=int(2.0*a[1]);
            s2db = 0;
            int nsync,nsnr;
            if (s_max65_cand_for_dec==1)/////1.36 sync hv////////////////////////////
            {
                if (nspecial>0)
                {
                    s2db=sync2;
                    nsync=int(sync_sh_single);
                    nsnr=int(s2db);
                }
                else
                {
                    nsync=int(sync1-3.0);
                    nsnr=(int)snrx_single;
                }
            }
            else
            {
                if (bVHF)
                {
                    //qDebug()<<"s2db="<<sync1<<db(width_jt65/3.3);
                    s2db=sync1 - 30.0 + pomAll.db(width_jt65/3.3);//1.52 =-0      // !### VHF/UHF/microwave
                    if (nspecial>0)
                        s2db=sync2;
                }
                else
                {
                    if (sync2<0.000001) sync2=0.000001;
                    s2db=10.0*log10(sync2) - 35.0 ;           // !### Empirical (HF)
                }
                nsync=int(sync1);
                nsnr=int(s2db);
            }

            if (nsnr < -30 || nsync<0) nsync=0;/////1.36 sync hv////////////////////////////

            if (nsnr < -33) nsnr=-33;
            if (nsnr> -1) nsnr=-1;

            nftt=0;
            //qDebug()<<"END decode65a="<<icand<<ncand<<nft<<decoded<<nfreq<<nsnr<<dtx<<thresh0_jt65<<qual;

            //if (nft!=1 && (ndepth & 16)==16 && !prtavg) //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
            //if (nft!=1 && ndepth>0 && !prtavg)	//1.49 ndepth 0=no, 1=avg, 2=avg+deep search
            if (nft!=1 && f_avg_jt65 && !prtavg)	//1.49 ndepth to f_avg_jt65
            {
                //qDebug()<<"AVERAGE=====FFFFF==";
                //! Single-sequence FT decode failed, so try for an average FT decode.
                if (nutc!=nutc0 || fabs(nfreq-nfreq0)>ntol)
                {
                    //! This is a new minute or a new frequency, so call avg65.
                    nutc0=nutc;
                    nfreq0=nfreq;

                    //qDebug()<<"START AVERAGE==========="<<nft<<(ndepth & 16)<<prtavg;
                    //nsave_jt65++;
                    //nsave_jt65=fmod(nsave_jt65-1,5)+1;
                    //////////////////////////////////////////////////////  f_deep_search_jt65 no use deep+avg
                    avg65(nutc,nsave_jt65,sync1,dtx,nflip,nfreq,mode65,ntol,false,nagain,ntrials,naggressive_ds,
                          neme,mycall,hiscall,hisgrid,nftt,avemsg,qave,deepave,nsum_jt65,ndeepave,nQSOProgress,ljt65apon);

                    //qDebug()<<"end AVERAGE=========="<<nft<<prtavg<<avemsg;
                    //qDebug()<<"end AVERAGE=========="<<avemsg;

                    nsmo=param_jt65[9];
                    //nqave=qave;

                    prtavg=true;
                }
            }

            if (nftt==1)//nft 0=no decode; 1=FT decode; 2=deep ->hinted decode
            {
                //nft=1; //HV 1.49 for average ind a1-9-a* //!             nft=1
                decoded=avemsg;
                min_1good_cand_hv = true;//1.54=
                goto c5;
            }

            //qDebug()<<"start subtract65="<<decoded<<nftt<<nft<<nflip<<minsync<<icand;
            //qDebug()<<"BridgeFilDec="<<decoded<<nftt<<nft<<nflip<<"nshr,sync"<<nsnr<<sync1<<nfreq<<min_1good_cand_hv<<icand;
            rtt=0.001*nrtt1000;//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
            fal_cand_hv = false;
            if (nft<2 && nspecial==0)//1.54=all cand is checked 1.36 /*&& minsync>=(0+1)*/ 0db  minsync>=(0+1)
            {
                //if (nhard_min>50) {/*qDebug()<<"1s nhard_min="<<nhard_min<<50;*/ continue;}//cycle
                if (nhard_min>50)
                {/*qDebug()<<"1s nhard_min="<<nhard_min<<50;*/
                    //continue;
                    fal_cand_hv = true;
                }//cycle
                if (nhard_min>h0[n_dagrres_l])//h0[12]={41,42,43,43,44,45,46,47,48,48,49,49};
                {/*qDebug()<<"2s="<<nhard_min<<"h0="<<h0[n];*/
                    //continue;
                    fal_cand_hv = true;
                }//cycle
                if (ntotal_min>d0[n_dagrres_l])//d0[12]={71,72,73,74,76,77,78,80,81,82,83,83};
                {/*qDebug()<<"3s="<<ntotal_min<<"d0="<<d0[n];*/
                    //continue;
                    fal_cand_hv = true;
                }//cycle
                if (rtt>r0[n_dagrres_l])        //r0[12] ={0.70,0.72,0.74,0.76,0.78,0.80,0.82,0.84,0.86,0.88,0.90,0.90};
                {/*qDebug()<<"4s r0"<<rtt<<"r0="<<r0[n];*/
                    //continue;
                    fal_cand_hv = true;
                }//cycle
            }

            //1.54=
            //If Sh nspecial>0 no->fal_cand_hv = false; no need empty candidat min_1good_cand_hv = true;
            //If DS nft=2 no->fal_cand_hv = false; no need empty candidat min_1good_cand_hv = true;
            //If AVG nftt==1 goto c5; no need empty candidat change min_1good_cand_hv = true;
            //If minsync>0 permanently have good candidat
            if (minsync>0)
                min_1good_cand_hv = true;
            if (fal_cand_hv)
            {
                if (min_1good_cand_hv)
                {
                    //qDebug()<<"Have_CandGoodContinueButThisNoGood"<<icand;
                    continue;
                }

                fal_cand_par_hv[icand].nsync = nsync;
                fal_cand_par_hv[icand].nsnr = nsnr;
                fal_cand_par_hv[icand].dtx = dtx;
                fal_cand_par_hv[icand].nfreq = nfreq;
                fal_cand_par_hv[icand].width_jt65 = width_jt65;
                fal_cand_par_hv[icand].nflip = nflip;
                fal_cand_par_hv[icand].sync1 = sync1;

                if (fal_ca_nsnr_max_hv<nsnr)
                {
                    fal_ca_nsnr_max_hv=nsnr;
                    fal_ca_icand_hv=icand;
                }

                if (icand >= ncand-1)//last candidat
                {
                    nsync = fal_cand_par_hv[fal_ca_icand_hv].nsync;
                    nsnr = fal_cand_par_hv[fal_ca_icand_hv].nsnr;
                    dtx = fal_cand_par_hv[fal_ca_icand_hv].dtx;
                    nfreq = fal_cand_par_hv[fal_ca_icand_hv].nfreq;
                    width_jt65 = fal_cand_par_hv[fal_ca_icand_hv].width_jt65;
                    nflip = fal_cand_par_hv[fal_ca_icand_hv].nflip;
                    sync1 = fal_cand_par_hv[fal_ca_icand_hv].sync1;
                }
                else
                {
                    //qDebug()<<"No_CandGoodContinue"<<icand;
                    continue;
                }
            }
            else
                min_1good_cand_hv = true;
            //qDebug()<<"OUT="<<decoded<<nhard_min<<h0[n]<<ntotal_min<<d0[n]<<rtt<<r0[n];
c5:
            //qDebug()<<"OUT="<<decoded<<nhard_min<<h0[n]<<ntotal_min<<d0[n]<<rtt<<r0[n];
            //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
            if (decoded==decoded0 && fabs(freq-freq0)< 3.0 && minsync>=0)
            {
                //qDebug()<<"OUT=d-d0"<<decoded<<decoded0
                continue;
            }//cycle                  //!Don't display dupes

            //qDebug()<<"10 DDDDDDDDDDDDDDDDD="<<minsync<<decoded<<sync1;
            if (decoded.isEmpty() && sync1==5.0000000123985)//1.36
                goto next_cand;  //continue;//

            if (!decoded.isEmpty() || (minsync<(0+1) && !min_1good_cand_hv))//1.54=min_1good_cand_hv 1.36 0db minsync<(0+1)
            {
                //qDebug()<<"start subtract65="<<nsubtract<<npts<<freq<<dtx;
                if (nsubtract==1)
                    subtract65(dd,npts,freq,dtx);
                //qDebug()<<"end subtract65="<<minsync<<ipass<<nsubtract;

                ndupe=0; //! de-dedupe
                for (int i = 0; i<ndecoded; i++)//gore e ndecoded = -1
                {//do i=1, ndecoded
                    if (decoded==dec[i].decoded)
                    {
                        ndupe=1;
                        break; //exit
                    }
                }

                if (ndupe!=1 && sync1>=double(minsync))
                {
                    //qDebug()<<"4444 end subtract65 ndecoded ="<<ndecoded;
                    if (ipass==1) n65a=n65a + 1;
                    if (ipass==2) n65b=n65b + 1;

                    dec[ndecoded].freq=freq+a[0];
                    dec[ndecoded].dt=dtx;
                    dec[ndecoded].sync=sync2;
                    dec[ndecoded].decoded=decoded;
                    if (ndecoded<99) ndecoded++;
                    //qDebug()<<"5555 end subtract65=";
                    //nqual=fmin(qual,9999.0);

                    //qDebug()<<"nft="<<nft<<"nftt="<<nftt;
                    QString cflags="";             //nft -> 0=no decode; 1=FT decode; 2=deep ->hinted decode
                    //nftt-> 0=no decode; 1=FT AVG decode; 2=deep AVG ->hinted decode

                    //if (bVHF && (nft>0 || nftt==1))//pri sh f pri men ot towa da ne wlizat v avrg
                    if ((nft>0 || nftt==1))//pri sh f pri men ot towa da ne wlizat v avrg
                    {
                        cflags="f";

                        if (nft==2)//is_deep=ft.eq.2//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
                        {
                            cflags = "d1";
                            cflags.append(QString("%1").arg((int)fmin(qual,9)));
                            //qual=1;
                            if (qual>=10.0) cflags.replace(2,1,"*");//   (3:3)='*'
                            if (qual<5.0) cflags.append("?");// be6e <3.0 w wsjt 6.0
                            //cflags(1:2)='d1'
                            //write(cflags(3:3),'(i1)') min(qual,9)
                            //if(qual.ge.10) cflags(3:3)='*'
                            //if(qual.lt.3) decoded(22:22)='?'
                        }
                        if (nftt==1)// average
                            //if (nsum_jt65>=2)// average
                        {
                            cflags = "a";
                            cflags.append(QString("%1").arg((int)fmin(nsum_jt65,9)));     //write(cflags(2:2),'(i1)') min(nsum,9)
                            //if (nsum_jt65>=10) cflags.append("*");//cflags(2:2)='*'
                            if (nsum_jt65>=10)cflags.replace(1,1,"*");
                        }
                        int nap=(nft >> 2);
                        if (nap!=0)
                        {
                            cflags="AP"+QString("%1").arg(nap);
                            //cflags = "f.";//1.57=!!!
                            //write(cflags(1:3),'(a1,i2.2)') 'a',nap
                        }

                        if (bVHF)
                            cflags.prepend("VHF ");
                        else
                            cflags.prepend("HF ");

                    }
                    //qDebug()<<"decoded============================"<<decoded<<nspecial;
                    QString csync="#";
                    //bVHF <- may be need removing becouse OOO no RX in HF
                    if (bVHF && nflip!=0 && sync1>=fmax(0.0,double(minsync)))
                    {
                        csync="#*";
                        if (nflip==-1)
                        {
                            csync="##";
                            if (!decoded.isEmpty())
                            {
                                /*do i=22,1,-1
                                   if(decoded(i:i).ne.' ') exit
                                enddo
                                if(i.gt.18) i=18
                                decoded(i+2:i+4)='OOO'*/
                                decoded.append(" OOO");
                            }
                        }
                    }

                    //if (nspecial==2) decoded="RO";
                    //if (nspecial==3) decoded="RRR";
                    //if (nspecial==4) decoded="73";
                    if (nspecial==2 || nspecial==3 || nspecial==4)//1.76
                    {
                        //2 to 4 sec max
                        if (dtx<2.0)
                            dtx=2.0;
                        if (dtx>4.0)
                            dtx=4.0;
                    }

                    if (f_only_one_color)
                    {
                        f_only_one_color = false;
                        SetBackColor();
                    }

                    QStringList list;
                    list <<s_time<<QString("%1").arg(nsync)<<QString("%1").arg(nsnr)
                    <<QString("%1").arg(dtx-1.0,0,'f',1)
                    <<QString("%1").arg((int)(nfreq-tx_nfqso65))<<QString("%1").arg((int)width_jt65)
                    <<decoded<<csync<<cflags<<QString("%1").arg(nfreq);
                    //<<QString("%1").arg(8)<<QString("%1").arg(9.0,0,'f',1);
                    //emit EmitDecodetText(list,s_fopen);//stop 2.43

                    //if (abs((int)s_nfqso65-nfreq)<=10 || list.at(6).contains(s_MyCall))
                    //"TA2NC RR73; SP9HWY <LZ2HV> +00"
                    //"TU; LZ2HV SP9HWY R 589 NY"
                    //"TU; LZ2HV SP9HWY R 589 0001"
                    QString tstr = list.at(6);
                    tstr.remove("<");//v2 protokol 2.02
                    tstr.remove(">");//v2 protokol 2.02
                    QStringList tlist = tstr.split(" ");
                    bool fmyc = false;
                    for (int x = 0; x<tlist.count(); x++)
                    {
                        if (tlist.at(x)==s_MyBaseCall || tlist.at(x)==s_MyCall)//2.02
                        {
                            fmyc = true;
                            break;
                        }
                    }
                    if (abs((int)s_nfqso_all-nfreq)<=10 || fmyc)
                        emit EmitDecodetTextRxFreq(list,true,true); //1.60= true no emit other infos from decode list2 s_fopen

                    emit EmitDecodetText(list,s_fopen,true);//2.43
                }
                decoded0=decoded;
                freq0=freq;
                if (decoded0.isEmpty()) decoded0="*";
            }
next_cand:
            continue;
        }                                 //!Candidate loop
        if (ipass==2 && ndecoded<1) goto END_65;//if(ipass.eq.2 .and. ndecoded.lt.1) exit      if(ndecoded.lt.1) exit
    }
END_65:
    delete [] ss_;
    //qDebug()<<"Time="<<ttt.elapsed();
}
