// (c) 2020 - Nico Palermo, IV3NWV - Microtelecom Srl, Italy
// ------------------------------------------------------------------------------
// This file is part of the qracodes project, a Forward Error Control
// encoding/decoding package based on Q-ary RA (Repeat and Accumulate) LDPC codes.
//
//    qracodes is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//    qracodes is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.

//    You should have received a copy of the GNU General Public License
//    along with qracodes source distribution.
//    If not, see <http://www.gnu.org/licenses/>.

/* The algorithms, source code, look-and-feel of WSJT-X and related
 * programs, and protocol specifications for the modes FSK441, FT8, JT4,
 * JT6M, JT9, JT65, JTMS, QRA64, ISCAT, MSK144, are Copyright © 2001-2017
 * by one or more of the following authors: Joseph Taylor, K1JT; Bill
 * Somerville, G4WJS; Steven Franke, K9AN; Nico Palermo, IV3NWV; Greg Beam,
 * KI7MT; Michael Black, W9MDB; Edson Pereira, PY2SDR; Philip Karn, KA9Q;
 * and other members of the WSJT Development Group.
 *
 * MSHV Q65 Subs
 * Rewritten into C++ and modified by Hrisimir Hristov, LZ2HV 2015-2021
 * May be used under the terms of the GNU General Public License (GPL)
 */
//#include <QtGui>
#include "q65_subs.h"
#include <stdio.h>
#include <math.h>
#include <memory.h>
 
//#define PD_NDIM(nlogdim)              ((1<<(nlogdim))
#define PD_SIZE(ndim)                 ((ndim)*sizeof(float))
#define PD_ROWADDR(fp,ndim,idx)       (fp+((ndim)*(idx)))
#define pd_init(dst,src,ndim) memcpy(dst,src,PD_SIZE(ndim))

// type of codes
#define QRATYPE_NORMAL			0x00 // normal code
#define QRATYPE_CRC 			0x01 // code with crc - last information symbol is a CRC-6
#define QRATYPE_CRCPUNCTURED 	0x02 // the CRC-6 symbol is punctured (not sent along the channel)
#define QRATYPE_CRCPUNCTURED2 	0x03 // code with CRC-12. The two crc symbols are punctured

typedef const float *ppd_uniform;
// define uniform distributions of given size
static const float pd_uniform1[1] =
    {
        1.
    };
static const float pd_uniform2[2] =
    {
        1./2., 1./2.
    };
static const float pd_uniform4[4] =
    {
        1./4., 1./4.,1./4., 1./4.
    };
static const float pd_uniform8[8] =
    {
        1./8., 1./8.,1./8., 1./8.,1./8., 1./8.,1./8., 1./8.
    };
static const float pd_uniform16[16] =
    {
        1./16., 1./16., 1./16., 1./16.,1./16., 1./16.,1./16., 1./16.,
        1./16., 1./16., 1./16., 1./16.,1./16., 1./16.,1./16., 1./16.
    };
static const float pd_uniform32[32] =
    {
        1./32., 1./32., 1./32., 1./32.,1./32., 1./32.,1./32., 1./32.,
        1./32., 1./32., 1./32., 1./32.,1./32., 1./32.,1./32., 1./32.,
        1./32., 1./32., 1./32., 1./32.,1./32., 1./32.,1./32., 1./32.,
        1./32., 1./32., 1./32., 1./32.,1./32., 1./32.,1./32., 1./32.
    };
static const float pd_uniform64[64] =
    {
        1./64., 1./64., 1./64., 1./64.,1./64., 1./64.,1./64., 1./64.,
        1./64., 1./64., 1./64., 1./64.,1./64., 1./64.,1./64., 1./64.,
        1./64., 1./64., 1./64., 1./64.,1./64., 1./64.,1./64., 1./64.,
        1./64., 1./64., 1./64., 1./64.,1./64., 1./64.,1./64., 1./64.,
        1./64., 1./64., 1./64., 1./64.,1./64., 1./64.,1./64., 1./64.,
        1./64., 1./64., 1./64., 1./64.,1./64., 1./64.,1./64., 1./64.,
        1./64., 1./64., 1./64., 1./64.,1./64., 1./64.,1./64., 1./64.,
        1./64., 1./64., 1./64., 1./64.,1./64., 1./64.,1./64., 1./64.

    };
static const ppd_uniform pd_uniform_tab[7] =
    {
        pd_uniform1,
        pd_uniform2,
        pd_uniform4,
        pd_uniform8,
        pd_uniform16,
        pd_uniform32,
        pd_uniform64
    };
const float *pd_uniform(int nlogdim)
{
    return pd_uniform_tab[nlogdim];
}
static const int pd_log2dim[7] =
    {
        1,2,4,8,16,32,64
    };
/*typedef float (*ppd_norm)(float*);        
static float pd_norm1(float *ppd)
{
    float t = ppd[0];
    ppd[0] = 1.f;
    return t;
}
static float pd_norm2(float *ppd)
{
    float t,to;

    t =ppd[0];
    t +=ppd[1];

    if (t<=0)
    {
        pd_init(ppd,pd_uniform(1),pd_log2dim[1]);
        return t;
    }

    to = t;
    t = 1.f/t;
    ppd[0] *=t;
    ppd[1] *=t;
    return to;

}
static float pd_norm4(float *ppd)
{
    float t,to;

    t =ppd[0];
    t +=ppd[1];
    t +=ppd[2];
    t +=ppd[3];

    if (t<=0)
    {
        pd_init(ppd,pd_uniform(2),pd_log2dim[2]);
        return t;
    }

    to = t;
    t = 1.f/t;
    ppd[0] *=t;
    ppd[1] *=t;
    ppd[2] *=t;
    ppd[3] *=t;
    return to;
}
static float pd_norm8(float *ppd)
{
    float t,to;

    t  =ppd[0];
    t +=ppd[1];
    t +=ppd[2];
    t +=ppd[3];
    t +=ppd[4];
    t +=ppd[5];
    t +=ppd[6];
    t +=ppd[7];

    if (t<=0)
    {
        pd_init(ppd,pd_uniform(3),pd_log2dim[3]);
        return t;
    }

    to = t;
    t = 1.f/t;
    ppd[0] *=t;
    ppd[1] *=t;
    ppd[2] *=t;
    ppd[3] *=t;
    ppd[4] *=t;
    ppd[5] *=t;
    ppd[6] *=t;
    ppd[7] *=t;
    return to;
}
static float pd_norm16(float *ppd)
{
    float t,to;

    t  =ppd[0];
    t +=ppd[1];
    t +=ppd[2];
    t +=ppd[3];
    t +=ppd[4];
    t +=ppd[5];
    t +=ppd[6];
    t +=ppd[7];
    t +=ppd[8];
    t +=ppd[9];
    t +=ppd[10];
    t +=ppd[11];
    t +=ppd[12];
    t +=ppd[13];
    t +=ppd[14];
    t +=ppd[15];

    if (t<=0)
    {
        pd_init(ppd,pd_uniform(4),pd_log2dim[4]);
        return t;
    }

    to = t;
    t = 1.f/t;
    ppd[0]  *=t;
    ppd[1]  *=t;
    ppd[2]  *=t;
    ppd[3]  *=t;
    ppd[4]  *=t;
    ppd[5]  *=t;
    ppd[6]  *=t;
    ppd[7]  *=t;
    ppd[8]  *=t;
    ppd[9]  *=t;
    ppd[10] *=t;
    ppd[11] *=t;
    ppd[12] *=t;
    ppd[13] *=t;
    ppd[14] *=t;
    ppd[15] *=t;

    return to;
}
static float pd_norm32(float *ppd)
{
    float t,to;

    t  =ppd[0];
    t +=ppd[1];
    t +=ppd[2];
    t +=ppd[3];
    t +=ppd[4];
    t +=ppd[5];
    t +=ppd[6];
    t +=ppd[7];
    t +=ppd[8];
    t +=ppd[9];
    t +=ppd[10];
    t +=ppd[11];
    t +=ppd[12];
    t +=ppd[13];
    t +=ppd[14];
    t +=ppd[15];
    t +=ppd[16];
    t +=ppd[17];
    t +=ppd[18];
    t +=ppd[19];
    t +=ppd[20];
    t +=ppd[21];
    t +=ppd[22];
    t +=ppd[23];
    t +=ppd[24];
    t +=ppd[25];
    t +=ppd[26];
    t +=ppd[27];
    t +=ppd[28];
    t +=ppd[29];
    t +=ppd[30];
    t +=ppd[31];

    if (t<=0)
    {
        pd_init(ppd,pd_uniform(5),pd_log2dim[5]);
        return t;
    }

    to = t;
    t = 1.f/t;
    ppd[0]  *=t;
    ppd[1]  *=t;
    ppd[2]  *=t;
    ppd[3]  *=t;
    ppd[4]  *=t;
    ppd[5]  *=t;
    ppd[6]  *=t;
    ppd[7]  *=t;
    ppd[8]  *=t;
    ppd[9]  *=t;
    ppd[10] *=t;
    ppd[11] *=t;
    ppd[12] *=t;
    ppd[13] *=t;
    ppd[14] *=t;
    ppd[15] *=t;
    ppd[16] *=t;
    ppd[17] *=t;
    ppd[18] *=t;
    ppd[19] *=t;
    ppd[20] *=t;
    ppd[21] *=t;
    ppd[22] *=t;
    ppd[23] *=t;
    ppd[24] *=t;
    ppd[25] *=t;
    ppd[26] *=t;
    ppd[27] *=t;
    ppd[28] *=t;
    ppd[29] *=t;
    ppd[30] *=t;
    ppd[31] *=t;

    return to;
}
static float pd_norm64(float *ppd)
{
    float t,to;

    t  =ppd[0];
    t +=ppd[1];
    t +=ppd[2];
    t +=ppd[3];
    t +=ppd[4];
    t +=ppd[5];
    t +=ppd[6];
    t +=ppd[7];
    t +=ppd[8];
    t +=ppd[9];
    t +=ppd[10];
    t +=ppd[11];
    t +=ppd[12];
    t +=ppd[13];
    t +=ppd[14];
    t +=ppd[15];
    t +=ppd[16];
    t +=ppd[17];
    t +=ppd[18];
    t +=ppd[19];
    t +=ppd[20];
    t +=ppd[21];
    t +=ppd[22];
    t +=ppd[23];
    t +=ppd[24];
    t +=ppd[25];
    t +=ppd[26];
    t +=ppd[27];
    t +=ppd[28];
    t +=ppd[29];
    t +=ppd[30];
    t +=ppd[31];

    t +=ppd[32];
    t +=ppd[33];
    t +=ppd[34];
    t +=ppd[35];
    t +=ppd[36];
    t +=ppd[37];
    t +=ppd[38];
    t +=ppd[39];
    t +=ppd[40];
    t +=ppd[41];
    t +=ppd[42];
    t +=ppd[43];
    t +=ppd[44];
    t +=ppd[45];
    t +=ppd[46];
    t +=ppd[47];
    t +=ppd[48];
    t +=ppd[49];
    t +=ppd[50];
    t +=ppd[51];
    t +=ppd[52];
    t +=ppd[53];
    t +=ppd[54];
    t +=ppd[55];
    t +=ppd[56];
    t +=ppd[57];
    t +=ppd[58];
    t +=ppd[59];
    t +=ppd[60];
    t +=ppd[61];
    t +=ppd[62];
    t +=ppd[63];

    if (t<=0)
    {
        pd_init(ppd,pd_uniform(6),pd_log2dim[6]);
        return t;
    }

    to = t;
    t = 1.0f/t;
    ppd[0]  *=t;
    ppd[1]  *=t;
    ppd[2]  *=t;
    ppd[3]  *=t;
    ppd[4]  *=t;
    ppd[5]  *=t;
    ppd[6]  *=t;
    ppd[7]  *=t;
    ppd[8]  *=t;
    ppd[9]  *=t;
    ppd[10] *=t;
    ppd[11] *=t;
    ppd[12] *=t;
    ppd[13] *=t;
    ppd[14] *=t;
    ppd[15] *=t;
    ppd[16] *=t;
    ppd[17] *=t;
    ppd[18] *=t;
    ppd[19] *=t;
    ppd[20] *=t;
    ppd[21] *=t;
    ppd[22] *=t;
    ppd[23] *=t;
    ppd[24] *=t;
    ppd[25] *=t;
    ppd[26] *=t;
    ppd[27] *=t;
    ppd[28] *=t;
    ppd[29] *=t;
    ppd[30] *=t;
    ppd[31] *=t;

    ppd[32] *=t;
    ppd[33] *=t;
    ppd[34] *=t;
    ppd[35] *=t;
    ppd[36] *=t;
    ppd[37] *=t;
    ppd[38] *=t;
    ppd[39] *=t;
    ppd[40] *=t;
    ppd[41] *=t;
    ppd[42] *=t;
    ppd[43] *=t;
    ppd[44] *=t;
    ppd[45] *=t;
    ppd[46] *=t;
    ppd[47] *=t;
    ppd[48] *=t;
    ppd[49] *=t;
    ppd[50] *=t;
    ppd[51] *=t;
    ppd[52] *=t;
    ppd[53] *=t;
    ppd[54] *=t;
    ppd[55] *=t;
    ppd[56] *=t;
    ppd[57] *=t;
    ppd[58] *=t;
    ppd[59] *=t;
    ppd[60] *=t;
    ppd[61] *=t;
    ppd[62] *=t;
    ppd[63] *=t;

    return to;
}
static const ppd_norm pd_norm_tab[7] =
    {
        pd_norm1,
        pd_norm2,
        pd_norm4,
        pd_norm8,
        pd_norm16,
        pd_norm32,
        pd_norm64
    };
float q65subs::pd_norm(float *pd, int nlogdim)
{
    return pd_norm_tab[nlogdim](pd);
}
*/
/*const float *pd_uniform(int c0)
{
	static float res[64];
	float un = 1.0;
	if	  	(c0==1) un=1.0/2.0;
	else if (c0==2) un=1.0/4.0;
	else if (c0==3) un=1.0/8.0;
	else if (c0==4) un=1.0/16.0;
	else if (c0==5) un=1.0/32.0;
	else if (c0==6) un=1.0/64.0;
	int c1 = pow(2,c0);
	for (int i = 0; i<c1; ++i) res[i] = un;
	return res;	
}*/
static float pd_norm_tab(float *ppd,int c0)
{
    float t,to;
    int c1 = 1; 
    if (c0>6) c0=6;  
	if (c0==0)
	{
    	t = ppd[0];
    	ppd[0] = 1.f;
    	return t;		
	}
	c1 = pow(2,c0);
	//if	  (c0==1) c1=2;
	//else if (c0==2) c1=4;
	//else if (c0==3) c1=8;
	//else if (c0==4) c1=16;
	//else if (c0==5) c1=32;
	//else if (c0==6) c1=64;
    t  =ppd[0];
    for (int i = 1; i<c1; ++i) t +=ppd[i];
    if (t<=0)
    {
        pd_init(ppd,pd_uniform(c0),pd_log2dim[c0]);
        return t;
    }
    to = t;
    t = 1.f/t;
    for (int i = 0; i<c1; ++i) ppd[i] *=t;    
    return to;
}  
float q65subs::pd_norm(float *pd, int nlogdim)
{
    return pd_norm_tab(pd,nlogdim);
}
// helper functions -------------------------------------------------------------
int q65subs::_q65_get_message_length(const qracode *pCode)
{
    // return the actual information message length (in symbols)
    // excluding any punctured symbol

    int nMsgLength;

    switch (pCode->type)
    {
    case QRATYPE_NORMAL:
        nMsgLength = pCode->K;
        break;
    case QRATYPE_CRC:
    case QRATYPE_CRCPUNCTURED:
        // one information symbol of the underlying qra code is reserved for CRC
        nMsgLength = pCode->K-1;
        break;
    case QRATYPE_CRCPUNCTURED2:
        // two code information symbols are reserved for CRC
        nMsgLength = pCode->K-2;
        break;
    default:
        nMsgLength = -1;
    }

    return nMsgLength;
}
int q65subs::_q65_get_codeword_length(const qracode *pCode)
{
    // return the actual codeword length (in symbols)
    // excluding any punctured symbol

    int nCwLength;

    switch (pCode->type)
    {
    case QRATYPE_NORMAL:
    case QRATYPE_CRC:
        // no puncturing
        nCwLength = pCode->N;
        break;
    case QRATYPE_CRCPUNCTURED:
        // the CRC symbol is punctured
        nCwLength = pCode->N-1;
        break;
    case QRATYPE_CRCPUNCTURED2:
        // the two CRC symbols are punctured
        nCwLength = pCode->N-2;
        break;
    default:
        nCwLength = -1;
    }

    return nCwLength;
}
float q65subs::_q65_get_code_rate(const qracode *pCode)
{
    return 1.0f*_q65_get_message_length(pCode)/_q65_get_codeword_length(pCode);
}

int q65subs::_q65_get_alphabet_size(const qracode *pCode)
{
    return pCode->M;
}
int q65subs::_q65_get_bits_per_symbol(const qracode *pCode)
{
    return pCode->m;
}
void q65subs::_q65_mask(const qracode *pcode, float *ix, const int *mask, const int *x)
{
    // mask intrinsic information ix with available a priori knowledge

    int k,kk, smask;
    const int nM=pcode->M;
    const int nm=pcode->m;
    int nK;

    // Exclude from masking the symbols which have been punctured.
    // nK is the length of the mask and x arrays, which do
    // not include any punctured symbol
    nK = _q65_get_message_length(pcode);

    // for each symbol set to zero the probability
    // of the values which are not allowed by
    // the a priori information

    for (k=0;k<nK;k++)
    {
        smask = mask[k];
        if (smask)
        {
            for (kk=0;kk<nM;kk++)
                if (((kk^x[k])&smask)!=0)
                    // This symbol value is not allowed
                    // by the AP information
                    // Set its probability to zero
                    *(PD_ROWADDR(ix,nM,k)+kk) = 0.f;

            // normalize to a probability distribution
            pd_norm(PD_ROWADDR(ix,nM,k),nm);
        }
    }
}
int q65subs::qra_encode(const qracode *pcode, int *y, const int *x)
{
    int k,j,kk,jj;
    int t, chk = 0;

    const int K = pcode->K;
    const int M = pcode->M;
    const int NC= pcode->NC;
    const int a = pcode->a;
    const int  *acc_input_idx  = pcode->acc_input_idx;
    const int *acc_input_wlog = pcode->acc_input_wlog;
    const int  *gflog		   = pcode->gflog;
    const int *gfexp          = pcode->gfexp;

    // copy the systematic symbols to destination
    memcpy(y,x,K*sizeof(int));

    y = y+K;	// point to check symbols

    // compute the code check symbols as a weighted accumulation of a permutated
    // sequence of the (repeated) systematic input symbols:
    // chk(k+1) = x(idx(k))*alfa^(logw(k)) + chk(k)
    // (all operations performed over GF(M))

    if (a==1)
    { // grouping factor = 1
        for (k=0;k<NC;k++)
        {
            t = x[acc_input_idx[k]];
            if (t)
            {
                // multiply input by weight[k] and xor it with previous check
                t = (gflog[t] + acc_input_wlog[k])%(M-1);
                t = gfexp[t];
                chk ^=t;
            }
            y[k] = chk;
        }

#ifdef QRA_DEBUG
        // verify that the encoder accumulator is terminated to 0
        // (we designed the code this way so that Iex = 1 when Ia = 1)
        t = x[acc_input_idx[k]];
        if (t)
        {
            t = (gflog[t] + acc_input_wlog[k])%(M-1);
            t = gfexp[t];
            // accumulation
            chk ^=t;
        }
        return (chk==0);
#else
        return 1;
#endif // QRA_DEBUG
    }
    else
    { // grouping factor > 1
        for (k=0;k<NC;k++)
        {
            kk = a*k;
            for (j=0;j<a;j++)
            {
                jj = kk+j;
                // irregular grouping support
                if (acc_input_idx[jj]<0)
                    continue;
                t = x[acc_input_idx[jj]];
                if (t)
                {
                    // multiply input by weight[k] and xor it with previous check
                    t = (gflog[t] + acc_input_wlog[jj])%(M-1);
                    t = gfexp[t];
                    chk ^=t;
                }
            }
            y[k] = chk;
        }
#ifdef QRA_DEBUG
        // verify that the encoder accumulator is terminated to 0
        // (we designed the code this way so that Iex = 1 when Ia = 1)
        kk = a*k;
        for (j=0;j<a;j++)
        {
            jj = kk+j;
            if (acc_input_idx[jj]<0)
                continue;
            t = x[acc_input_idx[jj]];
            if (t)
            {
                // multiply input by weight[k] and xor it with previous check
                t = (gflog[t] + acc_input_wlog[jj])%(M-1);
                t = gfexp[t];
                chk ^=t;
            }
        }
        return (chk==0);
#else
        return 1;
#endif // QRA_DEBUG
    }
}

// CRC generation functions
// crc-6 generator polynomial
// g(x) = x^6 + x + 1
#define CRC6_GEN_POL 0x30		// MSB=a0 LSB=a5
// crc-12 generator polynomial
// g(x) = x^12 + x^11 + x^3 + x^2 + x + 1
#define CRC12_GEN_POL 0xF01		// MSB=a0 LSB=a11

// g(x) = x^6 + x^2 + x + 1 (as suggested by Joe. See i.e.:  https://users.ece.cmu.edu/~koopman/crc/)
// #define CRC6_GEN_POL 0x38  // MSB=a0 LSB=a5. Simulation results are similar
int q65subs::_q65_crc6(int *x, int sz)
{
    int k,j,t,sr = 0;
    for (k=0;k<sz;k++)
    {
        t = x[k];
        for (j=0;j<6;j++)
        {
            if ((t^sr)&0x01)
                sr = (sr>>1) ^ CRC6_GEN_POL;
            else
                sr = (sr>>1);
            t>>=1;
        }
    }

    return sr;
}
void q65subs::_q65_crc12(int *y, int *x, int sz)
{
    int k,j,t,sr = 0;
    for (k=0;k<sz;k++)
    {
        t = x[k];
        for (j=0;j<6;j++)
        {
            if ((t^sr)&0x01)
                sr = (sr>>1) ^ CRC12_GEN_POL;
            else
                sr = (sr>>1);
            t>>=1;
        }
    }

    y[0] = sr&0x3F;
    y[1] = (sr>>6);
}
int q65subs::q65_encode(const q65_codec_ds *pCodec, int *pOutputCodeword, const int *pInputMsg)
{
    const qracode *pQraCode;
    int *px;
    int *py;
    int nK;
    int nN;

    if (!pCodec)
        return -1;	// which codec?

    pQraCode = pCodec->pQraCode;
    px = pCodec->x;
    py = pCodec->y;
    nK = _q65_get_message_length(pQraCode);
    nN = _q65_get_codeword_length(pQraCode);

    // copy the information symbols into the internal buffer
    memcpy(px,pInputMsg,nK*sizeof(int));

    // compute and append the appropriate CRC if required
    switch (pQraCode->type)
    {
    case QRATYPE_NORMAL:
        break;
    case QRATYPE_CRC:
    case QRATYPE_CRCPUNCTURED:
        px[nK] = _q65_crc6(px,nK);
        break;
    case QRATYPE_CRCPUNCTURED2:
        _q65_crc12(px+nK,px,nK);
        break;
    default:
        return -2;	// code type not supported
    }

    // encode with the given qra code
    qra_encode(pQraCode,py,px);

    // puncture the CRC symbols as required
    // and copy the result to the destination buffer
    switch (pQraCode->type)
    {
    case QRATYPE_NORMAL:
    case QRATYPE_CRC:
        // no puncturing
        memcpy(pOutputCodeword,py,nN*sizeof(int));
        break;
    case QRATYPE_CRCPUNCTURED:
        // strip the single CRC symbol from the encoded codeword
        memcpy(pOutputCodeword,py,nK*sizeof(int));				// copy the systematic symbols
        memcpy(pOutputCodeword+nK,py+nK+1,(nN-nK)*sizeof(int));	// copy the check symbols skipping the CRC symbol
        break;
    case QRATYPE_CRCPUNCTURED2:
        // strip the 2 CRC symbols from the encoded codeword
        memcpy(pOutputCodeword,py,nK*sizeof(int));				// copy the systematic symbols
        memcpy(pOutputCodeword+nK,py+nK+2,(nN-nK)*sizeof(int)); // copy the check symbols skipping the two CRC symbols
        break;
    default:
        return -2;	// code type unsupported
    }

    return 1; // ok
}
void q65subs::q65_free(q65_codec_ds *pCodec)
{
    if (!pCodec)
        return;

    // free internal buffers
    if (pCodec->x!=NULL)
        free(pCodec->x);

    if (pCodec->y!=NULL)
        free(pCodec->y);

    if (pCodec->qra_v2cmsg!=NULL)
        free(pCodec->qra_v2cmsg);

    if (pCodec->qra_c2vmsg!=NULL)
        free(pCodec->qra_c2vmsg);

    if (pCodec->ix!=NULL)
        free(pCodec->ix);

    if (pCodec->ex!=NULL)
        free(pCodec->ex);

    pCodec->pQraCode	= NULL;
    pCodec->x			= NULL;
    pCodec->y			= NULL;
    pCodec->qra_v2cmsg	= NULL;
    pCodec->qra_c2vmsg	= NULL;
    pCodec->qra_v2cmsg	= NULL;
    pCodec->ix			= NULL;
    pCodec->ex			= NULL;

    return;
}
int q65subs::q65_init(q65_codec_ds *pCodec, 	const qracode *pqracode)
{
    // Eb/No value for which we optimize the decoder metric (AWGN/Rayleigh cases)
    const float EbNodBMetric = 2.8f;
    const float EbNoMetric   = (float)pow(10,EbNodBMetric/10);

    float	R;		// code effective rate (after puncturing)
    int		nm;		// bits per symbol

    if (!pCodec)
        return -1;		// why do you called me?

    if (!pqracode)
        return -2;		// invalid qra code

    if (pqracode->M!=64)
        return -3;		// q65 supports only codes over GF(64)

    pCodec->pQraCode = pqracode;

    // allocate buffers used by encoding/decoding functions
    pCodec->x			= (int*)malloc(pqracode->K*sizeof(int));
    pCodec->y			= (int*)malloc(pqracode->N*sizeof(int));
    pCodec->qra_v2cmsg	= (float*)malloc(pqracode->NMSG*pqracode->M*sizeof(float));
    pCodec->qra_c2vmsg	= (float*)malloc(pqracode->NMSG*pqracode->M*sizeof(float));
    pCodec->ix			= (float*)malloc(pqracode->N*pqracode->M*sizeof(float));
    pCodec->ex			= (float*)malloc(pqracode->N*pqracode->M*sizeof(float));

    if (pCodec->x== NULL			||
            pCodec->y== NULL			||
            pCodec->qra_v2cmsg== NULL	||
            pCodec->qra_c2vmsg== NULL	||
            pCodec->ix== NULL			||
            pCodec->ex== NULL)
    {
        q65_free(pCodec);
        return -4; // out of memory
    }

    // compute and store the AWGN/Rayleigh Es/No ratio for which we optimize
    // the decoder metric
    nm = _q65_get_bits_per_symbol(pqracode);
    R  = _q65_get_code_rate(pqracode);
    pCodec->decoderEsNoMetric   = 1.0f*nm*R*EbNoMetric;

    return 1;
}

#define qra_K       15 // number of information symbols
#define qra_N       65 // codeword length in symbols
#define qra_m        6 // bits/symbol
#define qra_M       64 // Symbol alphabet cardinality
#define qra_a        1 // grouping factor
#define qra_NC      50 // number of check symbols (N-K)

// Defines used by the message passing decoder --------

#define qra_V       65 // number of variables in the code graph (N)
#define qra_C      116 // number of factors in the code graph (N +(N-K)+1)
#define qra_NMSG   216 // number of msgs in the code graph
#define qra_MAXVDEG    5 // maximum variable degree
#define qra_MAXCDEG    3 // maximum factor degree
#define qra_R     0.23077f // code rate (K/N)
#define CODE_NAME "qra15_65_64_irr_e23"  // code name

// table of the systematic symbols indexes in the accumulator chain
static const int qra_acc_input_idx[qra_NC+1] =
    {
        13,   1,   3,   4,   8,  12,   9,  14,  10,   5,
        0,   7,   1,  11,   8,   9,  12,   6,   3,  10,
        7,   5,   2,  13,  12,   4,   8,   0,   1,  11,
        2,   9,  14,   5,   6,  13,   7,  12,  11,   2,
        9,   0,  10,   4,   7,  14,   8,  11,   3,   6,
        10
    };

// table of the systematic symbols weight logarithms over GF(M)
static const int qra_acc_input_wlog[qra_NC+1] =
    {
        0,  14,   0,   0,  13,  37,   0,  27,  56,  62,
        29,   0,  52,  34,  62,   4,   3,  22,  25,   0,
        22,   0,  20,  10,   0,  43,  53,  60,   0,   0,
        0,  62,   0,   5,   0,  61,  36,  31,  61,  59,
        10,   0,  29,  39,  25,  18,   0,  14,  11,  50,
        17
    };

// table of the logarithms of the elements of GF(M) (log(0) never used)
static const int qra_log[qra_M] =
    {
        -1,   0,   1,   6,   2,  12,   7,  26,   3,  32,
        13,  35,   8,  48,  27,  18,   4,  24,  33,  16,
        14,  52,  36,  54,   9,  45,  49,  38,  28,  41,
        19,  56,   5,  62,  25,  11,  34,  31,  17,  47,
        15,  23,  53,  51,  37,  44,  55,  40,  10,  61,
        46,  30,  50,  22,  39,  43,  29,  60,  42,  21,
        20,  59,  57,  58
    };

// table of GF(M) elements given their logarithm
static const int qra_exp[qra_M-1] =
    {
        1,   2,   4,   8,  16,  32,   3,   6,  12,  24,
        48,  35,   5,  10,  20,  40,  19,  38,  15,  30,
        60,  59,  53,  41,  17,  34,   7,  14,  28,  56,
        51,  37,   9,  18,  36,  11,  22,  44,  27,  54,
        47,  29,  58,  55,  45,  25,  50,  39,  13,  26,
        52,  43,  21,  42,  23,  46,  31,  62,  63,  61,
        57,  49,  33
    };

// table of the messages weight logarithms over GF(M)
static const int qra_msgw[qra_NMSG] =
    {
        0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
        0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
        0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
        0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
        0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
        0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
        0,   0,   0,   0,   0,   0,  14,   0,   0,  13,
        37,   0,  27,  56,  62,  29,   0,  52,  34,  62,
        4,   3,  22,  25,   0,  22,   0,  20,  10,   0,
        43,  53,  60,   0,   0,   0,  62,   0,   5,   0,
        61,  36,  31,  61,  59,  10,   0,  29,  39,  25,
        18,   0,  14,  11,  50,  17,   0,   0,   0,   0,
        0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
        0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
        0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
        0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
        0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
        0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
        0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
        0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
        0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
        0,   0,   0,   0,   0,   0
    };

// table of the degrees of the variable nodes
static const int qra_vdeg[qra_V] =
    {
        4,   4,   4,   4,   4,   4,   4,   5,   5,   5,
        5,   5,   5,   4,   4,   3,   3,   3,   3,   3,
        3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
        3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
        3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
        3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
        3,   3,   3,   3,   3
    };

// table of the degrees of the factor nodes
static const int qra_cdeg[qra_C] =
    {
        1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
        1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
        1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
        1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
        1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
        1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
        1,   1,   1,   1,   1,   2,   3,   3,   3,   3,
        3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
        3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
        3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
        3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
        3,   3,   3,   3,   3,   2
    };

// table (uncompressed) of the v->c message indexes (-1=unused entry)
static const int qra_v2cmidx[qra_V*qra_MAXVDEG] =
    {
        0,  75,  92, 106,  -1,
        1,  66,  77,  93,  -1,
        2,  87,  95, 104,  -1,
        3,  67,  83, 113,  -1,
        4,  68,  90, 108,  -1,
        5,  74,  86,  98,  -1,
        6,  82,  99, 114,  -1,
        7,  76,  85, 101, 109,
        8,  69,  79,  91, 111,
        9,  71,  80,  96, 105,
        10,  73,  84, 107, 115,
        11,  78,  94, 103, 112,
        12,  70,  81,  89, 102,
        13,  65,  88, 100,  -1,
        14,  72,  97, 110,  -1,
        15, 116, 117,  -1,  -1,
        16, 118, 119,  -1,  -1,
        17, 120, 121,  -1,  -1,
        18, 122, 123,  -1,  -1,
        19, 124, 125,  -1,  -1,
        20, 126, 127,  -1,  -1,
        21, 128, 129,  -1,  -1,
        22, 130, 131,  -1,  -1,
        23, 132, 133,  -1,  -1,
        24, 134, 135,  -1,  -1,
        25, 136, 137,  -1,  -1,
        26, 138, 139,  -1,  -1,
        27, 140, 141,  -1,  -1,
        28, 142, 143,  -1,  -1,
        29, 144, 145,  -1,  -1,
        30, 146, 147,  -1,  -1,
        31, 148, 149,  -1,  -1,
        32, 150, 151,  -1,  -1,
        33, 152, 153,  -1,  -1,
        34, 154, 155,  -1,  -1,
        35, 156, 157,  -1,  -1,
        36, 158, 159,  -1,  -1,
        37, 160, 161,  -1,  -1,
        38, 162, 163,  -1,  -1,
        39, 164, 165,  -1,  -1,
        40, 166, 167,  -1,  -1,
        41, 168, 169,  -1,  -1,
        42, 170, 171,  -1,  -1,
        43, 172, 173,  -1,  -1,
        44, 174, 175,  -1,  -1,
        45, 176, 177,  -1,  -1,
        46, 178, 179,  -1,  -1,
        47, 180, 181,  -1,  -1,
        48, 182, 183,  -1,  -1,
        49, 184, 185,  -1,  -1,
        50, 186, 187,  -1,  -1,
        51, 188, 189,  -1,  -1,
        52, 190, 191,  -1,  -1,
        53, 192, 193,  -1,  -1,
        54, 194, 195,  -1,  -1,
        55, 196, 197,  -1,  -1,
        56, 198, 199,  -1,  -1,
        57, 200, 201,  -1,  -1,
        58, 202, 203,  -1,  -1,
        59, 204, 205,  -1,  -1,
        60, 206, 207,  -1,  -1,
        61, 208, 209,  -1,  -1,
        62, 210, 211,  -1,  -1,
        63, 212, 213,  -1,  -1,
        64, 214, 215,  -1,  -1
    };

// table (uncompressed) of the c->v message indexes (-1=unused entry)
static const int qra_c2vmidx[qra_C*qra_MAXCDEG] =
    {
        0,  -1,  -1,   1,  -1,  -1,   2,  -1,  -1,   3,  -1,  -1,
        4,  -1,  -1,   5,  -1,  -1,   6,  -1,  -1,   7,  -1,  -1,
        8,  -1,  -1,   9,  -1,  -1,  10,  -1,  -1,  11,  -1,  -1,
        12,  -1,  -1,  13,  -1,  -1,  14,  -1,  -1,  15,  -1,  -1,
        16,  -1,  -1,  17,  -1,  -1,  18,  -1,  -1,  19,  -1,  -1,
        20,  -1,  -1,  21,  -1,  -1,  22,  -1,  -1,  23,  -1,  -1,
        24,  -1,  -1,  25,  -1,  -1,  26,  -1,  -1,  27,  -1,  -1,
        28,  -1,  -1,  29,  -1,  -1,  30,  -1,  -1,  31,  -1,  -1,
        32,  -1,  -1,  33,  -1,  -1,  34,  -1,  -1,  35,  -1,  -1,
        36,  -1,  -1,  37,  -1,  -1,  38,  -1,  -1,  39,  -1,  -1,
        40,  -1,  -1,  41,  -1,  -1,  42,  -1,  -1,  43,  -1,  -1,
        44,  -1,  -1,  45,  -1,  -1,  46,  -1,  -1,  47,  -1,  -1,
        48,  -1,  -1,  49,  -1,  -1,  50,  -1,  -1,  51,  -1,  -1,
        52,  -1,  -1,  53,  -1,  -1,  54,  -1,  -1,  55,  -1,  -1,
        56,  -1,  -1,  57,  -1,  -1,  58,  -1,  -1,  59,  -1,  -1,
        60,  -1,  -1,  61,  -1,  -1,  62,  -1,  -1,  63,  -1,  -1,
        64,  -1,  -1,  65, 116,  -1,  66, 117, 118,  67, 119, 120,
        68, 121, 122,  69, 123, 124,  70, 125, 126,  71, 127, 128,
        72, 129, 130,  73, 131, 132,  74, 133, 134,  75, 135, 136,
        76, 137, 138,  77, 139, 140,  78, 141, 142,  79, 143, 144,
        80, 145, 146,  81, 147, 148,  82, 149, 150,  83, 151, 152,
        84, 153, 154,  85, 155, 156,  86, 157, 158,  87, 159, 160,
        88, 161, 162,  89, 163, 164,  90, 165, 166,  91, 167, 168,
        92, 169, 170,  93, 171, 172,  94, 173, 174,  95, 175, 176,
        96, 177, 178,  97, 179, 180,  98, 181, 182,  99, 183, 184,
        100, 185, 186, 101, 187, 188, 102, 189, 190, 103, 191, 192,
        104, 193, 194, 105, 195, 196, 106, 197, 198, 107, 199, 200,
        108, 201, 202, 109, 203, 204, 110, 205, 206, 111, 207, 208,
        112, 209, 210, 113, 211, 212, 114, 213, 214, 115, 215,  -1
    };

// permutation matrix to compute Prob(x*alfa^logw)
static const int qra_pmat[qra_M*qra_M] =
    {
        0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,
        16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
        32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,
        48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,
        0,  33,   1,  32,   2,  35,   3,  34,   4,  37,   5,  36,   6,  39,   7,  38,
        8,  41,   9,  40,  10,  43,  11,  42,  12,  45,  13,  44,  14,  47,  15,  46,
        16,  49,  17,  48,  18,  51,  19,  50,  20,  53,  21,  52,  22,  55,  23,  54,
        24,  57,  25,  56,  26,  59,  27,  58,  28,  61,  29,  60,  30,  63,  31,  62,
        0,  49,  33,  16,   1,  48,  32,  17,   2,  51,  35,  18,   3,  50,  34,  19,
        4,  53,  37,  20,   5,  52,  36,  21,   6,  55,  39,  22,   7,  54,  38,  23,
        8,  57,  41,  24,   9,  56,  40,  25,  10,  59,  43,  26,  11,  58,  42,  27,
        12,  61,  45,  28,  13,  60,  44,  29,  14,  63,  47,  30,  15,  62,  46,  31,
        0,  57,  49,   8,  33,  24,  16,  41,   1,  56,  48,   9,  32,  25,  17,  40,
        2,  59,  51,  10,  35,  26,  18,  43,   3,  58,  50,  11,  34,  27,  19,  42,
        4,  61,  53,  12,  37,  28,  20,  45,   5,  60,  52,  13,  36,  29,  21,  44,
        6,  63,  55,  14,  39,  30,  22,  47,   7,  62,  54,  15,  38,  31,  23,  46,
        0,  61,  57,   4,  49,  12,   8,  53,  33,  28,  24,  37,  16,  45,  41,  20,
        1,  60,  56,   5,  48,  13,   9,  52,  32,  29,  25,  36,  17,  44,  40,  21,
        2,  63,  59,   6,  51,  14,  10,  55,  35,  30,  26,  39,  18,  47,  43,  22,
        3,  62,  58,   7,  50,  15,  11,  54,  34,  31,  27,  38,  19,  46,  42,  23,
        0,  63,  61,   2,  57,   6,   4,  59,  49,  14,  12,  51,   8,  55,  53,  10,
        33,  30,  28,  35,  24,  39,  37,  26,  16,  47,  45,  18,  41,  22,  20,  43,
        1,  62,  60,   3,  56,   7,   5,  58,  48,  15,  13,  50,   9,  54,  52,  11,
        32,  31,  29,  34,  25,  38,  36,  27,  17,  46,  44,  19,  40,  23,  21,  42,
        0,  62,  63,   1,  61,   3,   2,  60,  57,   7,   6,  56,   4,  58,  59,   5,
        49,  15,  14,  48,  12,  50,  51,  13,   8,  54,  55,   9,  53,  11,  10,  52,
        33,  31,  30,  32,  28,  34,  35,  29,  24,  38,  39,  25,  37,  27,  26,  36,
        16,  46,  47,  17,  45,  19,  18,  44,  41,  23,  22,  40,  20,  42,  43,  21,
        0,  31,  62,  33,  63,  32,   1,  30,  61,  34,   3,  28,   2,  29,  60,  35,
        57,  38,   7,  24,   6,  25,  56,  39,   4,  27,  58,  37,  59,  36,   5,  26,
        49,  46,  15,  16,  14,  17,  48,  47,  12,  19,  50,  45,  51,  44,  13,  18,
        8,  23,  54,  41,  55,  40,   9,  22,  53,  42,  11,  20,  10,  21,  52,  43,
        0,  46,  31,  49,  62,  16,  33,  15,  63,  17,  32,  14,   1,  47,  30,  48,
        61,  19,  34,  12,   3,  45,  28,  50,   2,  44,  29,  51,  60,  18,  35,  13,
        57,  23,  38,   8,   7,  41,  24,  54,   6,  40,  25,  55,  56,  22,  39,   9,
        4,  42,  27,  53,  58,  20,  37,  11,  59,  21,  36,  10,   5,  43,  26,  52,
        0,  23,  46,  57,  31,   8,  49,  38,  62,  41,  16,   7,  33,  54,  15,  24,
        63,  40,  17,   6,  32,  55,  14,  25,   1,  22,  47,  56,  30,   9,  48,  39,
        61,  42,  19,   4,  34,  53,  12,  27,   3,  20,  45,  58,  28,  11,  50,  37,
        2,  21,  44,  59,  29,  10,  51,  36,  60,  43,  18,   5,  35,  52,  13,  26,
        0,  42,  23,  61,  46,   4,  57,  19,  31,  53,   8,  34,  49,  27,  38,  12,
        62,  20,  41,   3,  16,  58,   7,  45,  33,  11,  54,  28,  15,  37,  24,  50,
        63,  21,  40,   2,  17,  59,   6,  44,  32,  10,  55,  29,  14,  36,  25,  51,
        1,  43,  22,  60,  47,   5,  56,  18,  30,  52,   9,  35,  48,  26,  39,  13,
        0,  21,  42,  63,  23,   2,  61,  40,  46,  59,   4,  17,  57,  44,  19,   6,
        31,  10,  53,  32,   8,  29,  34,  55,  49,  36,  27,  14,  38,  51,  12,  25,
        62,  43,  20,   1,  41,  60,   3,  22,  16,   5,  58,  47,   7,  18,  45,  56,
        33,  52,  11,  30,  54,  35,  28,   9,  15,  26,  37,  48,  24,  13,  50,  39,
        0,  43,  21,  62,  42,   1,  63,  20,  23,  60,   2,  41,  61,  22,  40,   3,
        46,   5,  59,  16,   4,  47,  17,  58,  57,  18,  44,   7,  19,  56,   6,  45,
        31,  52,  10,  33,  53,  30,  32,  11,   8,  35,  29,  54,  34,   9,  55,  28,
        49,  26,  36,  15,  27,  48,  14,  37,  38,  13,  51,  24,  12,  39,  25,  50,
        0,  52,  43,  31,  21,  33,  62,  10,  42,  30,   1,  53,  63,  11,  20,  32,
        23,  35,  60,   8,   2,  54,  41,  29,  61,   9,  22,  34,  40,  28,   3,  55,
        46,  26,   5,  49,  59,  15,  16,  36,   4,  48,  47,  27,  17,  37,  58,  14,
        57,  13,  18,  38,  44,  24,   7,  51,  19,  39,  56,  12,   6,  50,  45,  25,
        0,  26,  52,  46,  43,  49,  31,   5,  21,  15,  33,  59,  62,  36,  10,  16,
        42,  48,  30,   4,   1,  27,  53,  47,  63,  37,  11,  17,  20,  14,  32,  58,
        23,  13,  35,  57,  60,  38,   8,  18,   2,  24,  54,  44,  41,  51,  29,   7,
        61,  39,   9,  19,  22,  12,  34,  56,  40,  50,  28,   6,   3,  25,  55,  45,
        0,  13,  26,  23,  52,  57,  46,  35,  43,  38,  49,  60,  31,  18,   5,   8,
        21,  24,  15,   2,  33,  44,  59,  54,  62,  51,  36,  41,  10,   7,  16,  29,
        42,  39,  48,  61,  30,  19,   4,   9,   1,  12,  27,  22,  53,  56,  47,  34,
        63,  50,  37,  40,  11,   6,  17,  28,  20,  25,  14,   3,  32,  45,  58,  55,
        0,  39,  13,  42,  26,  61,  23,  48,  52,  19,  57,  30,  46,   9,  35,   4,
        43,  12,  38,   1,  49,  22,  60,  27,  31,  56,  18,  53,   5,  34,   8,  47,
        21,  50,  24,  63,  15,  40,   2,  37,  33,   6,  44,  11,  59,  28,  54,  17,
        62,  25,  51,  20,  36,   3,  41,  14,  10,  45,   7,  32,  16,  55,  29,  58,
        0,  50,  39,  21,  13,  63,  42,  24,  26,  40,  61,  15,  23,  37,  48,   2,
        52,   6,  19,  33,  57,  11,  30,  44,  46,  28,   9,  59,  35,  17,   4,  54,
        43,  25,  12,  62,  38,  20,   1,  51,  49,   3,  22,  36,  60,  14,  27,  41,
        31,  45,  56,  10,  18,  32,  53,   7,   5,  55,  34,  16,   8,  58,  47,  29,
        0,  25,  50,  43,  39,  62,  21,  12,  13,  20,  63,  38,  42,  51,  24,   1,
        26,   3,  40,  49,  61,  36,  15,  22,  23,  14,  37,  60,  48,  41,   2,  27,
        52,  45,   6,  31,  19,  10,  33,  56,  57,  32,  11,  18,  30,   7,  44,  53,
        46,  55,  28,   5,   9,  16,  59,  34,  35,  58,  17,   8,   4,  29,  54,  47,
        0,  45,  25,  52,  50,  31,  43,   6,  39,  10,  62,  19,  21,  56,  12,  33,
        13,  32,  20,  57,  63,  18,  38,  11,  42,   7,  51,  30,  24,  53,   1,  44,
        26,  55,   3,  46,  40,   5,  49,  28,  61,  16,  36,   9,  15,  34,  22,  59,
        23,  58,  14,  35,  37,   8,  60,  17,  48,  29,  41,   4,   2,  47,  27,  54,
        0,  55,  45,  26,  25,  46,  52,   3,  50,   5,  31,  40,  43,  28,   6,  49,
        39,  16,  10,  61,  62,   9,  19,  36,  21,  34,  56,  15,  12,  59,  33,  22,
        13,  58,  32,  23,  20,  35,  57,  14,  63,   8,  18,  37,  38,  17,  11,  60,
        42,  29,   7,  48,  51,   4,  30,  41,  24,  47,  53,   2,   1,  54,  44,  27,
        0,  58,  55,  13,  45,  23,  26,  32,  25,  35,  46,  20,  52,  14,   3,  57,
        50,   8,   5,  63,  31,  37,  40,  18,  43,  17,  28,  38,   6,  60,  49,  11,
        39,  29,  16,  42,  10,  48,  61,   7,  62,   4,   9,  51,  19,  41,  36,  30,
        21,  47,  34,  24,  56,   2,  15,  53,  12,  54,  59,   1,  33,  27,  22,  44,
        0,  29,  58,  39,  55,  42,  13,  16,  45,  48,  23,  10,  26,   7,  32,  61,
        25,   4,  35,  62,  46,  51,  20,   9,  52,  41,  14,  19,   3,  30,  57,  36,
        50,  47,   8,  21,   5,  24,  63,  34,  31,   2,  37,  56,  40,  53,  18,  15,
        43,  54,  17,  12,  28,   1,  38,  59,   6,  27,  60,  33,  49,  44,  11,  22,
        0,  47,  29,  50,  58,  21,  39,   8,  55,  24,  42,   5,  13,  34,  16,  63,
        45,   2,  48,  31,  23,  56,  10,  37,  26,  53,   7,  40,  32,  15,  61,  18,
        25,  54,   4,  43,  35,  12,  62,  17,  46,   1,  51,  28,  20,  59,   9,  38,
        52,  27,  41,   6,  14,  33,  19,  60,   3,  44,  30,  49,  57,  22,  36,  11,
        0,  54,  47,  25,  29,  43,  50,   4,  58,  12,  21,  35,  39,  17,   8,  62,
        55,   1,  24,  46,  42,  28,   5,  51,  13,  59,  34,  20,  16,  38,  63,   9,
        45,  27,   2,  52,  48,   6,  31,  41,  23,  33,  56,  14,  10,  60,  37,  19,
        26,  44,  53,   3,   7,  49,  40,  30,  32,  22,  15,  57,  61,  11,  18,  36,
        0,  27,  54,  45,  47,  52,  25,   2,  29,   6,  43,  48,  50,  41,   4,  31,
        58,  33,  12,  23,  21,  14,  35,  56,  39,  60,  17,  10,   8,  19,  62,  37,
        55,  44,   1,  26,  24,   3,  46,  53,  42,  49,  28,   7,   5,  30,  51,  40,
        13,  22,  59,  32,  34,  57,  20,  15,  16,  11,  38,  61,  63,  36,   9,  18,
        0,  44,  27,  55,  54,  26,  45,   1,  47,   3,  52,  24,  25,  53,   2,  46,
        29,  49,   6,  42,  43,   7,  48,  28,  50,  30,  41,   5,   4,  40,  31,  51,
        58,  22,  33,  13,  12,  32,  23,  59,  21,  57,  14,  34,  35,  15,  56,  20,
        39,  11,  60,  16,  17,  61,  10,  38,   8,  36,  19,  63,  62,  18,  37,   9,
        0,  22,  44,  58,  27,  13,  55,  33,  54,  32,  26,  12,  45,  59,   1,  23,
        47,  57,   3,  21,  52,  34,  24,  14,  25,  15,  53,  35,   2,  20,  46,  56,
        29,  11,  49,  39,   6,  16,  42,  60,  43,  61,   7,  17,  48,  38,  28,  10,
        50,  36,  30,   8,  41,  63,   5,  19,   4,  18,  40,  62,  31,   9,  51,  37,
        0,  11,  22,  29,  44,  39,  58,  49,  27,  16,  13,   6,  55,  60,  33,  42,
        54,  61,  32,  43,  26,  17,  12,   7,  45,  38,  59,  48,   1,  10,  23,  28,
        47,  36,  57,  50,   3,   8,  21,  30,  52,  63,  34,  41,  24,  19,  14,   5,
        25,  18,  15,   4,  53,  62,  35,  40,   2,   9,  20,  31,  46,  37,  56,  51,
        0,  36,  11,  47,  22,  50,  29,  57,  44,   8,  39,   3,  58,  30,  49,  21,
        27,  63,  16,  52,  13,  41,   6,  34,  55,  19,  60,  24,  33,   5,  42,  14,
        54,  18,  61,  25,  32,   4,  43,  15,  26,  62,  17,  53,  12,  40,   7,  35,
        45,   9,  38,   2,  59,  31,  48,  20,   1,  37,  10,  46,  23,  51,  28,  56,
        0,  18,  36,  54,  11,  25,  47,  61,  22,   4,  50,  32,  29,  15,  57,  43,
        44,  62,   8,  26,  39,  53,   3,  17,  58,  40,  30,  12,  49,  35,  21,   7,
        27,   9,  63,  45,  16,   2,  52,  38,  13,  31,  41,  59,   6,  20,  34,  48,
        55,  37,  19,   1,  60,  46,  24,  10,  33,  51,   5,  23,  42,  56,  14,  28,
        0,   9,  18,  27,  36,  45,  54,  63,  11,   2,  25,  16,  47,  38,  61,  52,
        22,  31,   4,  13,  50,  59,  32,  41,  29,  20,  15,   6,  57,  48,  43,  34,
        44,  37,  62,  55,   8,   1,  26,  19,  39,  46,  53,  60,   3,  10,  17,  24,
        58,  51,  40,  33,  30,  23,  12,   5,  49,  56,  35,  42,  21,  28,   7,  14,
        0,  37,   9,  44,  18,  55,  27,  62,  36,   1,  45,   8,  54,  19,  63,  26,
        11,  46,   2,  39,  25,  60,  16,  53,  47,  10,  38,   3,  61,  24,  52,  17,
        22,  51,  31,  58,   4,  33,  13,  40,  50,  23,  59,  30,  32,   5,  41,  12,
        29,  56,  20,  49,  15,  42,   6,  35,  57,  28,  48,  21,  43,  14,  34,   7,
        0,  51,  37,  22,   9,  58,  44,  31,  18,  33,  55,   4,  27,  40,  62,  13,
        36,  23,   1,  50,  45,  30,   8,  59,  54,   5,  19,  32,  63,  12,  26,  41,
        11,  56,  46,  29,   2,  49,  39,  20,  25,  42,  60,  15,  16,  35,  53,   6,
        47,  28,  10,  57,  38,  21,   3,  48,  61,  14,  24,  43,  52,   7,  17,  34,
        0,  56,  51,  11,  37,  29,  22,  46,   9,  49,  58,   2,  44,  20,  31,  39,
        18,  42,  33,  25,  55,  15,   4,  60,  27,  35,  40,  16,  62,   6,  13,  53,
        36,  28,  23,  47,   1,  57,  50,  10,  45,  21,  30,  38,   8,  48,  59,   3,
        54,  14,   5,  61,  19,  43,  32,  24,  63,   7,  12,  52,  26,  34,  41,  17,
        0,  28,  56,  36,  51,  47,  11,  23,  37,  57,  29,   1,  22,  10,  46,  50,
        9,  21,  49,  45,  58,  38,   2,  30,  44,  48,  20,   8,  31,   3,  39,  59,
        18,  14,  42,  54,  33,  61,  25,   5,  55,  43,  15,  19,   4,  24,  60,  32,
        27,   7,  35,  63,  40,  52,  16,  12,  62,  34,   6,  26,  13,  17,  53,  41,
        0,  14,  28,  18,  56,  54,  36,  42,  51,  61,  47,  33,  11,   5,  23,  25,
        37,  43,  57,  55,  29,  19,   1,  15,  22,  24,  10,   4,  46,  32,  50,  60,
        9,   7,  21,  27,  49,  63,  45,  35,  58,  52,  38,  40,   2,  12,  30,  16,
        44,  34,  48,  62,  20,  26,   8,   6,  31,  17,   3,  13,  39,  41,  59,  53,
        0,   7,  14,   9,  28,  27,  18,  21,  56,  63,  54,  49,  36,  35,  42,  45,
        51,  52,  61,  58,  47,  40,  33,  38,  11,  12,   5,   2,  23,  16,  25,  30,
        37,  34,  43,  44,  57,  62,  55,  48,  29,  26,  19,  20,   1,   6,  15,   8,
        22,  17,  24,  31,  10,  13,   4,   3,  46,  41,  32,  39,  50,  53,  60,  59,
        0,  34,   7,  37,  14,  44,   9,  43,  28,  62,  27,  57,  18,  48,  21,  55,
        56,  26,  63,  29,  54,  20,  49,  19,  36,   6,  35,   1,  42,   8,  45,  15,
        51,  17,  52,  22,  61,  31,  58,  24,  47,  13,  40,  10,  33,   3,  38,   4,
        11,  41,  12,  46,   5,  39,   2,  32,  23,  53,  16,  50,  25,  59,  30,  60,
        0,  17,  34,  51,   7,  22,  37,  52,  14,  31,  44,  61,   9,  24,  43,  58,
        28,  13,  62,  47,  27,  10,  57,  40,  18,   3,  48,  33,  21,   4,  55,  38,
        56,  41,  26,  11,  63,  46,  29,  12,  54,  39,  20,   5,  49,  32,  19,   2,
        36,  53,   6,  23,  35,  50,   1,  16,  42,  59,   8,  25,  45,  60,  15,  30,
        0,  41,  17,  56,  34,  11,  51,  26,   7,  46,  22,  63,  37,  12,  52,  29,
        14,  39,  31,  54,  44,   5,  61,  20,   9,  32,  24,  49,  43,   2,  58,  19,
        28,  53,  13,  36,  62,  23,  47,   6,  27,  50,  10,  35,  57,  16,  40,   1,
        18,  59,   3,  42,  48,  25,  33,   8,  21,  60,   4,  45,  55,  30,  38,  15,
        0,  53,  41,  28,  17,  36,  56,  13,  34,  23,  11,  62,  51,   6,  26,  47,
        7,  50,  46,  27,  22,  35,  63,  10,  37,  16,  12,  57,  52,   1,  29,  40,
        14,  59,  39,  18,  31,  42,  54,   3,  44,  25,   5,  48,  61,   8,  20,  33,
        9,  60,  32,  21,  24,  45,  49,   4,  43,  30,   2,  55,  58,  15,  19,  38,
        0,  59,  53,  14,  41,  18,  28,  39,  17,  42,  36,  31,  56,   3,  13,  54,
        34,  25,  23,  44,  11,  48,  62,   5,  51,   8,   6,  61,  26,  33,  47,  20,
        7,  60,  50,   9,  46,  21,  27,  32,  22,  45,  35,  24,  63,   4,  10,  49,
        37,  30,  16,  43,  12,  55,  57,   2,  52,  15,   1,  58,  29,  38,  40,  19,
        0,  60,  59,   7,  53,   9,  14,  50,  41,  21,  18,  46,  28,  32,  39,  27,
        17,  45,  42,  22,  36,  24,  31,  35,  56,   4,   3,  63,  13,  49,  54,  10,
        34,  30,  25,  37,  23,  43,  44,  16,  11,  55,  48,  12,  62,   2,   5,  57,
        51,  15,   8,  52,   6,  58,  61,   1,  26,  38,  33,  29,  47,  19,  20,  40,
        0,  30,  60,  34,  59,  37,   7,  25,  53,  43,   9,  23,  14,  16,  50,  44,
        41,  55,  21,  11,  18,  12,  46,  48,  28,   2,  32,  62,  39,  57,  27,   5,
        17,  15,  45,  51,  42,  52,  22,   8,  36,  58,  24,   6,  31,   1,  35,  61,
        56,  38,   4,  26,   3,  29,  63,  33,  13,  19,  49,  47,  54,  40,  10,  20,
        0,  15,  30,  17,  60,  51,  34,  45,  59,  52,  37,  42,   7,   8,  25,  22,
        53,  58,  43,  36,   9,   6,  23,  24,  14,   1,  16,  31,  50,  61,  44,  35,
        41,  38,  55,  56,  21,  26,  11,   4,  18,  29,  12,   3,  46,  33,  48,  63,
        28,  19,   2,  13,  32,  47,  62,  49,  39,  40,  57,  54,  27,  20,   5,  10,
        0,  38,  15,  41,  30,  56,  17,  55,  60,  26,  51,  21,  34,   4,  45,  11,
        59,  29,  52,  18,  37,   3,  42,  12,   7,  33,   8,  46,  25,  63,  22,  48,
        53,  19,  58,  28,  43,  13,  36,   2,   9,  47,   6,  32,  23,  49,  24,  62,
        14,  40,   1,  39,  16,  54,  31,  57,  50,  20,  61,  27,  44,  10,  35,   5,
        0,  19,  38,  53,  15,  28,  41,  58,  30,  13,  56,  43,  17,   2,  55,  36,
        60,  47,  26,   9,  51,  32,  21,   6,  34,  49,   4,  23,  45,  62,  11,  24,
        59,  40,  29,  14,  52,  39,  18,   1,  37,  54,   3,  16,  42,  57,  12,  31,
        7,  20,  33,  50,   8,  27,  46,  61,  25,  10,  63,  44,  22,   5,  48,  35,
        0,  40,  19,  59,  38,  14,  53,  29,  15,  39,  28,  52,  41,   1,  58,  18,
        30,  54,  13,  37,  56,  16,  43,   3,  17,  57,   2,  42,  55,  31,  36,  12,
        60,  20,  47,   7,  26,  50,   9,  33,  51,  27,  32,   8,  21,  61,   6,  46,
        34,  10,  49,  25,   4,  44,  23,  63,  45,   5,  62,  22,  11,  35,  24,  48,
        0,  20,  40,  60,  19,   7,  59,  47,  38,  50,  14,  26,  53,  33,  29,   9,
        15,  27,  39,  51,  28,   8,  52,  32,  41,  61,   1,  21,  58,  46,  18,   6,
        30,  10,  54,  34,  13,  25,  37,  49,  56,  44,  16,   4,  43,  63,   3,  23,
        17,   5,  57,  45,   2,  22,  42,  62,  55,  35,  31,  11,  36,  48,  12,  24,
        0,  10,  20,  30,  40,  34,  60,  54,  19,  25,   7,  13,  59,  49,  47,  37,
        38,  44,  50,  56,  14,   4,  26,  16,  53,  63,  33,  43,  29,  23,   9,   3,
        15,   5,  27,  17,  39,  45,  51,  57,  28,  22,   8,   2,  52,  62,  32,  42,
        41,  35,  61,  55,   1,  11,  21,  31,  58,  48,  46,  36,  18,  24,   6,  12,
        0,   5,  10,  15,  20,  17,  30,  27,  40,  45,  34,  39,  60,  57,  54,  51,
        19,  22,  25,  28,   7,   2,  13,   8,  59,  62,  49,  52,  47,  42,  37,  32,
        38,  35,  44,  41,  50,  55,  56,  61,  14,  11,   4,   1,  26,  31,  16,  21,
        53,  48,  63,  58,  33,  36,  43,  46,  29,  24,  23,  18,   9,  12,   3,   6,
        0,  35,   5,  38,  10,  41,  15,  44,  20,  55,  17,  50,  30,  61,  27,  56,
        40,  11,  45,  14,  34,   1,  39,   4,  60,  31,  57,  26,  54,  21,  51,  16,
        19,  48,  22,  53,  25,  58,  28,  63,   7,  36,   2,  33,  13,  46,   8,  43,
        59,  24,  62,  29,  49,  18,  52,  23,  47,  12,  42,   9,  37,   6,  32,   3,
        0,  48,  35,  19,   5,  53,  38,  22,  10,  58,  41,  25,  15,  63,  44,  28,
        20,  36,  55,   7,  17,  33,  50,   2,  30,  46,  61,  13,  27,  43,  56,   8,
        40,  24,  11,  59,  45,  29,  14,  62,  34,  18,   1,  49,  39,  23,   4,  52,
        60,  12,  31,  47,  57,   9,  26,  42,  54,   6,  21,  37,  51,   3,  16,  32,
        0,  24,  48,  40,  35,  59,  19,  11,   5,  29,  53,  45,  38,  62,  22,  14,
        10,  18,  58,  34,  41,  49,  25,   1,  15,  23,  63,  39,  44,  52,  28,   4,
        20,  12,  36,  60,  55,  47,   7,  31,  17,   9,  33,  57,  50,  42,   2,  26,
        30,   6,  46,  54,  61,  37,  13,  21,  27,   3,  43,  51,  56,  32,   8,  16,
        0,  12,  24,  20,  48,  60,  40,  36,  35,  47,  59,  55,  19,  31,  11,   7,
        5,   9,  29,  17,  53,  57,  45,  33,  38,  42,  62,  50,  22,  26,  14,   2,
        10,   6,  18,  30,  58,  54,  34,  46,  41,  37,  49,  61,  25,  21,   1,  13,
        15,   3,  23,  27,  63,  51,  39,  43,  44,  32,  52,  56,  28,  16,   4,   8,
        0,   6,  12,  10,  24,  30,  20,  18,  48,  54,  60,  58,  40,  46,  36,  34,
        35,  37,  47,  41,  59,  61,  55,  49,  19,  21,  31,  25,  11,  13,   7,   1,
        5,   3,   9,  15,  29,  27,  17,  23,  53,  51,  57,  63,  45,  43,  33,  39,
        38,  32,  42,  44,  62,  56,  50,  52,  22,  16,  26,  28,  14,   8,   2,   4,
        0,   3,   6,   5,  12,  15,  10,   9,  24,  27,  30,  29,  20,  23,  18,  17,
        48,  51,  54,  53,  60,  63,  58,  57,  40,  43,  46,  45,  36,  39,  34,  33,
        35,  32,  37,  38,  47,  44,  41,  42,  59,  56,  61,  62,  55,  52,  49,  50,
        19,  16,  21,  22,  31,  28,  25,  26,  11,   8,  13,  14,   7,   4,   1,   2,
        0,  32,   3,  35,   6,  38,   5,  37,  12,  44,  15,  47,  10,  42,   9,  41,
        24,  56,  27,  59,  30,  62,  29,  61,  20,  52,  23,  55,  18,  50,  17,  49,
        48,  16,  51,  19,  54,  22,  53,  21,  60,  28,  63,  31,  58,  26,  57,  25,
        40,   8,  43,  11,  46,  14,  45,  13,  36,   4,  39,   7,  34,   2,  33,   1,
        0,  16,  32,  48,   3,  19,  35,  51,   6,  22,  38,  54,   5,  21,  37,  53,
        12,  28,  44,  60,  15,  31,  47,  63,  10,  26,  42,  58,   9,  25,  41,  57,
        24,   8,  56,  40,  27,  11,  59,  43,  30,  14,  62,  46,  29,  13,  61,  45,
        20,   4,  52,  36,  23,   7,  55,  39,  18,   2,  50,  34,  17,   1,  49,  33,
        0,   8,  16,  24,  32,  40,  48,  56,   3,  11,  19,  27,  35,  43,  51,  59,
        6,  14,  22,  30,  38,  46,  54,  62,   5,  13,  21,  29,  37,  45,  53,  61,
        12,   4,  28,  20,  44,  36,  60,  52,  15,   7,  31,  23,  47,  39,  63,  55,
        10,   2,  26,  18,  42,  34,  58,  50,   9,   1,  25,  17,  41,  33,  57,  49,
        0,   4,   8,  12,  16,  20,  24,  28,  32,  36,  40,  44,  48,  52,  56,  60,
        3,   7,  11,  15,  19,  23,  27,  31,  35,  39,  43,  47,  51,  55,  59,  63,
        6,   2,  14,  10,  22,  18,  30,  26,  38,  34,  46,  42,  54,  50,  62,  58,
        5,   1,  13,   9,  21,  17,  29,  25,  37,  33,  45,  41,  53,  49,  61,  57,
        0,   2,   4,   6,   8,  10,  12,  14,  16,  18,  20,  22,  24,  26,  28,  30,
        32,  34,  36,  38,  40,  42,  44,  46,  48,  50,  52,  54,  56,  58,  60,  62,
        3,   1,   7,   5,  11,   9,  15,  13,  19,  17,  23,  21,  27,  25,  31,  29,
        35,  33,  39,  37,  43,  41,  47,  45,  51,  49,  55,  53,  59,  57,  63,  61
    };

// SO array
static const int SO[qra_N-qra_K+1] =
    {
        14,   2,   4,   5,   9,  13,  10,  15,  11,   6,   1,   8,   2,  12,   9,  10,
        13,   7,   4,  11,   8,   6,   3,  14,  13,   5,   9,   1,   2,  12,   3,  10,
        15,   6,   7,  14,   8,  13,  12,   3,  10,   1,  11,   5,   8,  15,   9,  12,
        4,   7,  11
    };

// LOGWO array
static const int LOGWO[qra_N-qra_K+1] =
    {
        0,  14,   0,   0,  13,  37,   0,  27,  56,  62,  29,   0,  52,  34,  62,   4,
        3,  22,  25,   0,  22,   0,  20,  10,   0,  43,  53,  60,   0,   0,   0,  62,
        0,   5,   0,  61,  36,  31,  61,  59,  10,   0,  29,  39,  25,  18,   0,  14,
        11,  50,  17
    };

// repfact array
static const int repfact[qra_K] =
    {
        3,   3,   3,   3,   3,   3,   3,   4,   4,   4,   4,   4,   4,   3,   3
    };

const qracode qra15_65_64_irr_e23 =
    {
        qra_K,
        qra_N,
        qra_m,
        qra_M,
        qra_a,
        qra_NC,
        qra_V,
        qra_C,
        qra_NMSG,
        qra_MAXVDEG,
        qra_MAXCDEG,
        QRATYPE_CRCPUNCTURED2,
        qra_R,
        CODE_NAME,
        qra_acc_input_idx,
        qra_acc_input_wlog,
        qra_log,
        qra_exp,
        qra_msgw,
        qra_vdeg,
        qra_cdeg,
        qra_v2cmidx,
        qra_c2vmidx,
        qra_pmat
    };
#undef qra_K
#undef qra_N
#undef qra_m
#undef qra_M
#undef qra_a
#undef qra_NC
#undef qra_V
#undef qra_C
#undef qra_NMSG
#undef qra_MAXVDEG
#undef qra_MAXCDEG
#undef qra_R
#undef CODE_NAME
//extern const qracode qra15_65_64_irr_e23;

#define Q65_DECODE_INVPARAMS	 -1
#define TS_QRA64 0.576
// #define TS_Q65   0.640 // T/R = 60 s
// The tables are computed assuming that the bin spacing is that of QRA64, that's to say
// 1/Ts = 12000/6912 Hz, but in Q65 Ts depends on the T/R interval and the table index
// corresponding to a given B90 must be scaled appropriately.
// See below.
// Gaussian energy fading tables for QRA64
static const int glen_tab_gauss[64] =
    {
        2,   2,   2,   2,   2,   2,   2,   2,
        2,   2,   2,   2,   2,   2,   2,   2,
        3,   3,   3,   3,   3,   3,   3,   3,
        4,   4,   4,   4,   5,   5,   5,   6,
        6,   6,   7,   7,   8,   8,   9,  10,
        10,  11,  12,  13,  14,  15,  17,  18,
        19,  21,  23,  25,  27,  29,  32,  34,
        37,  41,  44,  48,  52,  57,  62,  65
    };
static const float ggauss1[2] =
    {
        0.0296f, 0.9101f
    };
static const float ggauss2[2] =
    {
        0.0350f, 0.8954f
    };
static const float ggauss3[2] =
    {
        0.0411f, 0.8787f
    };
static const float ggauss4[2] =
    {
        0.0483f, 0.8598f
    };
static const float ggauss5[2] =
    {
        0.0566f, 0.8387f
    };
static const float ggauss6[2] =
    {
        0.0660f, 0.8154f
    };
static const float ggauss7[2] =
    {
        0.0767f, 0.7898f
    };
static const float ggauss8[2] =
    {
        0.0886f, 0.7621f
    };
static const float ggauss9[2] =
    {
        0.1017f, 0.7325f
    };
static const float ggauss10[2] =
    {
        0.1159f, 0.7012f
    };
static const float ggauss11[2] =
    {
        0.1310f, 0.6687f
    };
static const float ggauss12[2] =
    {
        0.1465f, 0.6352f
    };
static const float ggauss13[2] =
    {
        0.1621f, 0.6013f
    };
static const float ggauss14[2] =
    {
        0.1771f, 0.5674f
    };
static const float ggauss15[2] =
    {
        0.1911f, 0.5339f
    };
static const float ggauss16[2] =
    {
        0.2034f, 0.5010f
    };
static const float ggauss17[3] =
    {
        0.0299f, 0.2135f, 0.4690f
    };
static const float ggauss18[3] =
    {
        0.0369f, 0.2212f, 0.4383f
    };
static const float ggauss19[3] =
    {
        0.0454f, 0.2263f, 0.4088f
    };
static const float ggauss20[3] =
    {
        0.0552f, 0.2286f, 0.3806f
    };
static const float ggauss21[3] =
    {
        0.0658f, 0.2284f, 0.3539f
    };
static const float ggauss22[3] =
    {
        0.0766f, 0.2258f, 0.3287f
    };
static const float ggauss23[3] =
    {
        0.0869f, 0.2212f, 0.3049f
    };
static const float ggauss24[3] =
    {
        0.0962f, 0.2148f, 0.2826f
    };
static const float ggauss25[4] =
    {
        0.0351f, 0.1041f, 0.2071f, 0.2616f
    };
static const float ggauss26[4] =
    {
        0.0429f, 0.1102f, 0.1984f, 0.2420f
    };
static const float ggauss27[4] =
    {
        0.0508f, 0.1145f, 0.1890f, 0.2237f
    };
static const float ggauss28[4] =
    {
        0.0582f, 0.1169f, 0.1791f, 0.2067f
    };
static const float ggauss29[5] =
    {
        0.0289f, 0.0648f, 0.1176f, 0.1689f, 0.1908f
    };
static const float ggauss30[5] =
    {
        0.0351f, 0.0703f, 0.1168f, 0.1588f, 0.1760f
    };
static const float ggauss31[5] =
    {
        0.0411f, 0.0745f, 0.1146f, 0.1488f, 0.1623f
    };
static const float ggauss32[6] =
    {
        0.0246f, 0.0466f, 0.0773f, 0.1115f, 0.1390f, 0.1497f
    };
static const float ggauss33[6] =
    {
        0.0297f, 0.0512f, 0.0788f, 0.1075f, 0.1295f, 0.1379f
    };
static const float ggauss34[6] =
    {
        0.0345f, 0.0549f, 0.0791f, 0.1029f, 0.1205f, 0.1270f
    };
static const float ggauss35[7] =
    {
        0.0240f, 0.0387f, 0.0575f, 0.0784f, 0.0979f, 0.1118f, 0.1169f
    };
static const float ggauss36[7] =
    {
        0.0281f, 0.0422f, 0.0590f, 0.0767f, 0.0926f, 0.1037f, 0.1076f
    };
static const float ggauss37[8] =
    {
        0.0212f, 0.0318f, 0.0449f, 0.0596f, 0.0744f, 0.0872f, 0.0960f, 0.0991f
    };
static const float ggauss38[8] =
    {
        0.0247f, 0.0348f, 0.0467f, 0.0593f, 0.0716f, 0.0819f, 0.0887f, 0.0911f
    };
static const float ggauss39[9] =
    {
        0.0199f, 0.0278f, 0.0372f, 0.0476f, 0.0584f, 0.0684f, 0.0766f, 0.0819f,
        0.0838f
    };
static const float ggauss40[10] =
    {
        0.0166f, 0.0228f, 0.0303f, 0.0388f, 0.0478f, 0.0568f, 0.0649f, 0.0714f,
        0.0756f, 0.0771f
    };
static const float ggauss41[10] =
    {
        0.0193f, 0.0254f, 0.0322f, 0.0397f, 0.0474f, 0.0548f, 0.0613f, 0.0664f,
        0.0697f, 0.0709f
    };
static const float ggauss42[11] =
    {
        0.0168f, 0.0217f, 0.0273f, 0.0335f, 0.0399f, 0.0464f, 0.0524f, 0.0576f,
        0.0617f, 0.0643f, 0.0651f
    };
static const float ggauss43[12] =
    {
        0.0151f, 0.0191f, 0.0237f, 0.0288f, 0.0342f, 0.0396f, 0.0449f, 0.0498f,
        0.0540f, 0.0572f, 0.0592f, 0.0599f
    };
static const float ggauss44[13] =
    {
        0.0138f, 0.0171f, 0.0210f, 0.0252f, 0.0297f, 0.0343f, 0.0388f, 0.0432f,
        0.0471f, 0.0504f, 0.0529f, 0.0545f, 0.0550f
    };
static const float ggauss45[14] =
    {
        0.0128f, 0.0157f, 0.0189f, 0.0224f, 0.0261f, 0.0300f, 0.0339f, 0.0377f,
        0.0412f, 0.0444f, 0.0470f, 0.0489f, 0.0501f, 0.0505f
    };
static const float ggauss46[15] =
    {
        0.0121f, 0.0146f, 0.0173f, 0.0202f, 0.0234f, 0.0266f, 0.0299f, 0.0332f,
        0.0363f, 0.0391f, 0.0416f, 0.0437f, 0.0452f, 0.0461f, 0.0464f
    };
static const float ggauss47[17] =
    {
        0.0097f, 0.0116f, 0.0138f, 0.0161f, 0.0186f, 0.0212f, 0.0239f, 0.0267f,
        0.0294f, 0.0321f, 0.0346f, 0.0369f, 0.0389f, 0.0405f, 0.0417f, 0.0424f,
        0.0427f
    };
static const float ggauss48[18] =
    {
        0.0096f, 0.0113f, 0.0131f, 0.0151f, 0.0172f, 0.0194f, 0.0217f, 0.0241f,
        0.0264f, 0.0287f, 0.0308f, 0.0329f, 0.0347f, 0.0362f, 0.0375f, 0.0384f,
        0.0390f, 0.0392f
    };
static const float ggauss49[19] =
    {
        0.0095f, 0.0110f, 0.0126f, 0.0143f, 0.0161f, 0.0180f, 0.0199f, 0.0219f,
        0.0239f, 0.0258f, 0.0277f, 0.0294f, 0.0310f, 0.0325f, 0.0337f, 0.0347f,
        0.0354f, 0.0358f, 0.0360f
    };
static const float ggauss50[21] =
    {
        0.0083f, 0.0095f, 0.0108f, 0.0122f, 0.0136f, 0.0152f, 0.0168f, 0.0184f,
        0.0201f, 0.0217f, 0.0234f, 0.0250f, 0.0265f, 0.0279f, 0.0292f, 0.0303f,
        0.0313f, 0.0320f, 0.0326f, 0.0329f, 0.0330f
    };
static const float ggauss51[23] =
    {
        0.0074f, 0.0084f, 0.0095f, 0.0106f, 0.0118f, 0.0131f, 0.0144f, 0.0157f,
        0.0171f, 0.0185f, 0.0199f, 0.0213f, 0.0227f, 0.0240f, 0.0252f, 0.0263f,
        0.0273f, 0.0282f, 0.0290f, 0.0296f, 0.0300f, 0.0303f, 0.0303f
    };
static const float ggauss52[25] =
    {
        0.0068f, 0.0076f, 0.0085f, 0.0094f, 0.0104f, 0.0115f, 0.0126f, 0.0137f,
        0.0149f, 0.0160f, 0.0172f, 0.0184f, 0.0196f, 0.0207f, 0.0218f, 0.0228f,
        0.0238f, 0.0247f, 0.0255f, 0.0262f, 0.0268f, 0.0273f, 0.0276f, 0.0278f,
        0.0279f
    };
static const float ggauss53[27] =
    {
        0.0063f, 0.0070f, 0.0078f, 0.0086f, 0.0094f, 0.0103f, 0.0112f, 0.0121f,
        0.0131f, 0.0141f, 0.0151f, 0.0161f, 0.0170f, 0.0180f, 0.0190f, 0.0199f,
        0.0208f, 0.0216f, 0.0224f, 0.0231f, 0.0237f, 0.0243f, 0.0247f, 0.0251f,
        0.0254f, 0.0255f, 0.0256f
    };
static const float ggauss54[29] =
    {
        0.0060f, 0.0066f, 0.0072f, 0.0079f, 0.0086f, 0.0093f, 0.0101f, 0.0109f,
        0.0117f, 0.0125f, 0.0133f, 0.0142f, 0.0150f, 0.0159f, 0.0167f, 0.0175f,
        0.0183f, 0.0190f, 0.0197f, 0.0204f, 0.0210f, 0.0216f, 0.0221f, 0.0225f,
        0.0228f, 0.0231f, 0.0233f, 0.0234f, 0.0235f
    };
static const float ggauss55[32] =
    {
        0.0053f, 0.0058f, 0.0063f, 0.0068f, 0.0074f, 0.0080f, 0.0086f, 0.0093f,
        0.0099f, 0.0106f, 0.0113f, 0.0120f, 0.0127f, 0.0134f, 0.0141f, 0.0148f,
        0.0155f, 0.0162f, 0.0168f, 0.0174f, 0.0180f, 0.0186f, 0.0191f, 0.0196f,
        0.0201f, 0.0204f, 0.0208f, 0.0211f, 0.0213f, 0.0214f, 0.0215f, 0.0216f
    };
static const float ggauss56[34] =
    {
        0.0052f, 0.0056f, 0.0060f, 0.0065f, 0.0070f, 0.0075f, 0.0080f, 0.0086f,
        0.0091f, 0.0097f, 0.0103f, 0.0109f, 0.0115f, 0.0121f, 0.0127f, 0.0133f,
        0.0138f, 0.0144f, 0.0150f, 0.0155f, 0.0161f, 0.0166f, 0.0170f, 0.0175f,
        0.0179f, 0.0183f, 0.0186f, 0.0189f, 0.0192f, 0.0194f, 0.0196f, 0.0197f,
        0.0198f, 0.0198f
    };
static const float ggauss57[37] =
    {
        0.0047f, 0.0051f, 0.0055f, 0.0058f, 0.0063f, 0.0067f, 0.0071f, 0.0076f,
        0.0080f, 0.0085f, 0.0090f, 0.0095f, 0.0100f, 0.0105f, 0.0110f, 0.0115f,
        0.0120f, 0.0125f, 0.0130f, 0.0134f, 0.0139f, 0.0144f, 0.0148f, 0.0152f,
        0.0156f, 0.0160f, 0.0164f, 0.0167f, 0.0170f, 0.0173f, 0.0175f, 0.0177f,
        0.0179f, 0.0180f, 0.0181f, 0.0181f, 0.0182f
    };
static const float ggauss58[41] =
    {
        0.0041f, 0.0044f, 0.0047f, 0.0050f, 0.0054f, 0.0057f, 0.0060f, 0.0064f,
        0.0068f, 0.0072f, 0.0076f, 0.0080f, 0.0084f, 0.0088f, 0.0092f, 0.0096f,
        0.0101f, 0.0105f, 0.0109f, 0.0113f, 0.0117f, 0.0121f, 0.0125f, 0.0129f,
        0.0133f, 0.0137f, 0.0140f, 0.0144f, 0.0147f, 0.0150f, 0.0153f, 0.0155f,
        0.0158f, 0.0160f, 0.0162f, 0.0163f, 0.0164f, 0.0165f, 0.0166f, 0.0167f,
        0.0167f
    };
static const float ggauss59[44] =
    {
        0.0039f, 0.0042f, 0.0044f, 0.0047f, 0.0050f, 0.0053f, 0.0056f, 0.0059f,
        0.0062f, 0.0065f, 0.0068f, 0.0072f, 0.0075f, 0.0079f, 0.0082f, 0.0086f,
        0.0089f, 0.0093f, 0.0096f, 0.0100f, 0.0104f, 0.0107f, 0.0110f, 0.0114f,
        0.0117f, 0.0120f, 0.0124f, 0.0127f, 0.0130f, 0.0132f, 0.0135f, 0.0138f,
        0.0140f, 0.0142f, 0.0144f, 0.0146f, 0.0148f, 0.0149f, 0.0150f, 0.0151f,
        0.0152f, 0.0153f, 0.0153f, 0.0153f
    };
static const float ggauss60[48] =
    {
        0.0036f, 0.0038f, 0.0040f, 0.0042f, 0.0044f, 0.0047f, 0.0049f, 0.0052f,
        0.0055f, 0.0057f, 0.0060f, 0.0063f, 0.0066f, 0.0068f, 0.0071f, 0.0074f,
        0.0077f, 0.0080f, 0.0083f, 0.0086f, 0.0089f, 0.0092f, 0.0095f, 0.0098f,
        0.0101f, 0.0104f, 0.0107f, 0.0109f, 0.0112f, 0.0115f, 0.0117f, 0.0120f,
        0.0122f, 0.0124f, 0.0126f, 0.0128f, 0.0130f, 0.0132f, 0.0134f, 0.0135f,
        0.0136f, 0.0137f, 0.0138f, 0.0139f, 0.0140f, 0.0140f, 0.0140f, 0.0140f
    };
static const float ggauss61[52] =
    {
        0.0033f, 0.0035f, 0.0037f, 0.0039f, 0.0041f, 0.0043f, 0.0045f, 0.0047f,
        0.0049f, 0.0051f, 0.0053f, 0.0056f, 0.0058f, 0.0060f, 0.0063f, 0.0065f,
        0.0068f, 0.0070f, 0.0073f, 0.0075f, 0.0078f, 0.0080f, 0.0083f, 0.0085f,
        0.0088f, 0.0090f, 0.0093f, 0.0095f, 0.0098f, 0.0100f, 0.0102f, 0.0105f,
        0.0107f, 0.0109f, 0.0111f, 0.0113f, 0.0115f, 0.0116f, 0.0118f, 0.0120f,
        0.0121f, 0.0122f, 0.0124f, 0.0125f, 0.0126f, 0.0126f, 0.0127f, 0.0128f,
        0.0128f, 0.0129f, 0.0129f, 0.0129f
    };
static const float ggauss62[57] =
    {
        0.0030f, 0.0031f, 0.0033f, 0.0034f, 0.0036f, 0.0038f, 0.0039f, 0.0041f,
        0.0043f, 0.0045f, 0.0047f, 0.0048f, 0.0050f, 0.0052f, 0.0054f, 0.0056f,
        0.0058f, 0.0060f, 0.0063f, 0.0065f, 0.0067f, 0.0069f, 0.0071f, 0.0073f,
        0.0075f, 0.0077f, 0.0080f, 0.0082f, 0.0084f, 0.0086f, 0.0088f, 0.0090f,
        0.0092f, 0.0094f, 0.0096f, 0.0097f, 0.0099f, 0.0101f, 0.0103f, 0.0104f,
        0.0106f, 0.0107f, 0.0108f, 0.0110f, 0.0111f, 0.0112f, 0.0113f, 0.0114f,
        0.0115f, 0.0116f, 0.0116f, 0.0117f, 0.0117f, 0.0118f, 0.0118f, 0.0118f,
        0.0118f
    };
static const float ggauss63[62] =
    {
        0.0027f, 0.0029f, 0.0030f, 0.0031f, 0.0032f, 0.0034f, 0.0035f, 0.0037f,
        0.0038f, 0.0040f, 0.0041f, 0.0043f, 0.0045f, 0.0046f, 0.0048f, 0.0049f,
        0.0051f, 0.0053f, 0.0055f, 0.0056f, 0.0058f, 0.0060f, 0.0062f, 0.0063f,
        0.0065f, 0.0067f, 0.0069f, 0.0071f, 0.0072f, 0.0074f, 0.0076f, 0.0078f,
        0.0079f, 0.0081f, 0.0083f, 0.0084f, 0.0086f, 0.0088f, 0.0089f, 0.0091f,
        0.0092f, 0.0094f, 0.0095f, 0.0096f, 0.0098f, 0.0099f, 0.0100f, 0.0101f,
        0.0102f, 0.0103f, 0.0104f, 0.0105f, 0.0105f, 0.0106f, 0.0107f, 0.0107f,
        0.0108f, 0.0108f, 0.0108f, 0.0108f, 0.0109f, 0.0109f
    };
static const float ggauss64[65] =
    {
        0.0028f, 0.0029f, 0.0030f, 0.0031f, 0.0032f, 0.0034f, 0.0035f, 0.0036f,
        0.0037f, 0.0039f, 0.0040f, 0.0041f, 0.0043f, 0.0044f, 0.0046f, 0.0047f,
        0.0048f, 0.0050f, 0.0051f, 0.0053f, 0.0054f, 0.0056f, 0.0057f, 0.0059f,
        0.0060f, 0.0062f, 0.0063f, 0.0065f, 0.0066f, 0.0068f, 0.0069f, 0.0071f,
        0.0072f, 0.0074f, 0.0075f, 0.0077f, 0.0078f, 0.0079f, 0.0081f, 0.0082f,
        0.0083f, 0.0084f, 0.0086f, 0.0087f, 0.0088f, 0.0089f, 0.0090f, 0.0091f,
        0.0092f, 0.0093f, 0.0094f, 0.0094f, 0.0095f, 0.0096f, 0.0097f, 0.0097f,
        0.0098f, 0.0098f, 0.0099f, 0.0099f, 0.0099f, 0.0099f, 0.0100f, 0.0100f,
        0.0100f
    };
static const float *gptr_tab_gauss[64] =
    {
        ggauss1, ggauss2, ggauss3, ggauss4,
        ggauss5, ggauss6, ggauss7, ggauss8,
        ggauss9, ggauss10, ggauss11, ggauss12,
        ggauss13, ggauss14, ggauss15, ggauss16,
        ggauss17, ggauss18, ggauss19, ggauss20,
        ggauss21, ggauss22, ggauss23, ggauss24,
        ggauss25, ggauss26, ggauss27, ggauss28,
        ggauss29, ggauss30, ggauss31, ggauss32,
        ggauss33, ggauss34, ggauss35, ggauss36,
        ggauss37, ggauss38, ggauss39, ggauss40,
        ggauss41, ggauss42, ggauss43, ggauss44,
        ggauss45, ggauss46, ggauss47, ggauss48,
        ggauss49, ggauss50, ggauss51, ggauss52,
        ggauss53, ggauss54, ggauss55, ggauss56,
        ggauss57, ggauss58, ggauss59, ggauss60,
        ggauss61, ggauss62, ggauss63, ggauss64
    };
// Lorentz energy fading tables for QRA64
static const int glen_tab_lorentz[64] =
    {
        2,   2,   2,   2,   2,   2,   2,   2,
        2,   2,   2,   2,   2,   2,   3,   3,
        3,   3,   3,   3,   3,   4,   4,   4,
        4,   4,   5,   5,   5,   5,   6,   6,
        7,   7,   7,   8,   8,   9,  10,  10,
        11,  12,  13,  14,  15,  16,  17,  19,
        20,  22,  23,  25,  27,  30,  32,  35,
        38,  41,  45,  49,  53,  57,  62,  65
    };
static const float glorentz1[2] =
    {
        0.0214f, 0.9107f
    };
static const float glorentz2[2] =
    {
        0.0244f, 0.9030f
    };
static const float glorentz3[2] =
    {
        0.0280f, 0.8950f
    };
static const float glorentz4[2] =
    {
        0.0314f, 0.8865f
    };
static const float glorentz5[2] =
    {
        0.0349f, 0.8773f
    };
static const float glorentz6[2] =
    {
        0.0388f, 0.8675f
    };
static const float glorentz7[2] =
    {
        0.0426f, 0.8571f
    };
static const float glorentz8[2] =
    {
        0.0463f, 0.8459f
    };
static const float glorentz9[2] =
    {
        0.0500f, 0.8339f
    };
static const float glorentz10[2] =
    {
        0.0538f, 0.8210f
    };
static const float glorentz11[2] =
    {
        0.0579f, 0.8074f
    };
static const float glorentz12[2] =
    {
        0.0622f, 0.7930f
    };
static const float glorentz13[2] =
    {
        0.0668f, 0.7777f
    };
static const float glorentz14[2] =
    {
        0.0715f, 0.7616f
    };
static const float glorentz15[3] =
    {
        0.0196f, 0.0765f, 0.7445f
    };
static const float glorentz16[3] =
    {
        0.0210f, 0.0816f, 0.7267f
    };
static const float glorentz17[3] =
    {
        0.0226f, 0.0870f, 0.7080f
    };
static const float glorentz18[3] =
    {
        0.0242f, 0.0925f, 0.6885f
    };
static const float glorentz19[3] =
    {
        0.0259f, 0.0981f, 0.6682f
    };
static const float glorentz20[3] =
    {
        0.0277f, 0.1039f, 0.6472f
    };
static const float glorentz21[3] =
    {
        0.0296f, 0.1097f, 0.6255f
    };
static const float glorentz22[4] =
    {
        0.0143f, 0.0316f, 0.1155f, 0.6031f
    };
static const float glorentz23[4] =
    {
        0.0153f, 0.0337f, 0.1213f, 0.5803f
    };
static const float glorentz24[4] =
    {
        0.0163f, 0.0358f, 0.1270f, 0.5570f
    };
static const float glorentz25[4] =
    {
        0.0174f, 0.0381f, 0.1325f, 0.5333f
    };
static const float glorentz26[4] =
    {
        0.0186f, 0.0405f, 0.1378f, 0.5095f
    };
static const float glorentz27[5] =
    {
        0.0113f, 0.0198f, 0.0429f, 0.1428f, 0.4855f
    };
static const float glorentz28[5] =
    {
        0.0120f, 0.0211f, 0.0455f, 0.1473f, 0.4615f
    };
static const float glorentz29[5] =
    {
        0.0129f, 0.0225f, 0.0481f, 0.1514f, 0.4376f
    };
static const float glorentz30[5] =
    {
        0.0137f, 0.0239f, 0.0508f, 0.1549f, 0.4140f
    };
static const float glorentz31[6] =
    {
        0.0095f, 0.0147f, 0.0254f, 0.0536f, 0.1578f, 0.3907f
    };
static const float glorentz32[6] =
    {
        0.0101f, 0.0156f, 0.0270f, 0.0564f, 0.1600f, 0.3680f
    };
static const float glorentz33[7] =
    {
        0.0076f, 0.0109f, 0.0167f, 0.0287f, 0.0592f, 0.1614f, 0.3458f
    };
static const float glorentz34[7] =
    {
        0.0081f, 0.0116f, 0.0178f, 0.0305f, 0.0621f, 0.1620f, 0.3243f
    };
static const float glorentz35[7] =
    {
        0.0087f, 0.0124f, 0.0190f, 0.0324f, 0.0649f, 0.1618f, 0.3035f
    };
static const float glorentz36[8] =
    {
        0.0069f, 0.0093f, 0.0133f, 0.0203f, 0.0343f, 0.0676f, 0.1607f, 0.2836f
    };
static const float glorentz37[8] =
    {
        0.0074f, 0.0100f, 0.0142f, 0.0216f, 0.0362f, 0.0702f, 0.1588f, 0.2645f
    };
static const float glorentz38[9] =
    {
        0.0061f, 0.0080f, 0.0107f, 0.0152f, 0.0230f, 0.0382f, 0.0726f, 0.1561f,
        0.2464f
    };
static const float glorentz39[10] =
    {
        0.0052f, 0.0066f, 0.0086f, 0.0115f, 0.0162f, 0.0244f, 0.0402f, 0.0747f,
        0.1526f, 0.2291f
    };
static const float glorentz40[10] =
    {
        0.0056f, 0.0071f, 0.0092f, 0.0123f, 0.0173f, 0.0259f, 0.0422f, 0.0766f,
        0.1484f, 0.2128f
    };
static const float glorentz41[11] =
    {
        0.0049f, 0.0061f, 0.0076f, 0.0098f, 0.0132f, 0.0184f, 0.0274f, 0.0441f,
        0.0780f, 0.1437f, 0.1975f
    };
static const float glorentz42[12] =
    {
        0.0044f, 0.0053f, 0.0065f, 0.0082f, 0.0106f, 0.0141f, 0.0196f, 0.0290f,
        0.0460f, 0.0791f, 0.1384f, 0.1831f
    };
static const float glorentz43[13] =
    {
        0.0040f, 0.0048f, 0.0057f, 0.0070f, 0.0088f, 0.0113f, 0.0150f, 0.0209f,
        0.0305f, 0.0477f, 0.0797f, 0.1327f, 0.1695f
    };
static const float glorentz44[14] =
    {
        0.0037f, 0.0043f, 0.0051f, 0.0062f, 0.0075f, 0.0094f, 0.0121f, 0.0160f,
        0.0221f, 0.0321f, 0.0493f, 0.0799f, 0.1267f, 0.1568f
    };
static const float glorentz45[15] =
    {
        0.0035f, 0.0040f, 0.0047f, 0.0055f, 0.0066f, 0.0081f, 0.0101f, 0.0129f,
        0.0171f, 0.0234f, 0.0335f, 0.0506f, 0.0795f, 0.1204f, 0.1450f
    };
static const float glorentz46[16] =
    {
        0.0033f, 0.0037f, 0.0043f, 0.0050f, 0.0059f, 0.0071f, 0.0087f, 0.0108f,
        0.0138f, 0.0181f, 0.0246f, 0.0349f, 0.0517f, 0.0786f, 0.1141f, 0.1340f
    };
static const float glorentz47[17] =
    {
        0.0031f, 0.0035f, 0.0040f, 0.0046f, 0.0054f, 0.0064f, 0.0077f, 0.0093f,
        0.0116f, 0.0147f, 0.0192f, 0.0259f, 0.0362f, 0.0525f, 0.0773f, 0.1076f,
        0.1237f
    };
static const float glorentz48[19] =
    {
        0.0027f, 0.0030f, 0.0034f, 0.0038f, 0.0043f, 0.0050f, 0.0058f, 0.0069f,
        0.0082f, 0.0100f, 0.0123f, 0.0156f, 0.0203f, 0.0271f, 0.0374f, 0.0530f,
        0.0755f, 0.1013f, 0.1141f
    };
static const float glorentz49[20] =
    {
        0.0026f, 0.0029f, 0.0032f, 0.0036f, 0.0041f, 0.0047f, 0.0054f, 0.0063f,
        0.0074f, 0.0088f, 0.0107f, 0.0131f, 0.0165f, 0.0213f, 0.0282f, 0.0383f,
        0.0531f, 0.0734f, 0.0950f, 0.1053f
    };
static const float glorentz50[22] =
    {
        0.0023f, 0.0025f, 0.0028f, 0.0031f, 0.0035f, 0.0039f, 0.0044f, 0.0050f,
        0.0058f, 0.0067f, 0.0079f, 0.0094f, 0.0114f, 0.0139f, 0.0175f, 0.0223f,
        0.0292f, 0.0391f, 0.0529f, 0.0709f, 0.0889f, 0.0971f
    };
static const float glorentz51[23] =
    {
        0.0023f, 0.0025f, 0.0027f, 0.0030f, 0.0034f, 0.0037f, 0.0042f, 0.0048f,
        0.0054f, 0.0062f, 0.0072f, 0.0085f, 0.0100f, 0.0121f, 0.0148f, 0.0184f,
        0.0233f, 0.0301f, 0.0396f, 0.0524f, 0.0681f, 0.0829f, 0.0894f
    };
static const float glorentz52[25] =
    {
        0.0021f, 0.0023f, 0.0025f, 0.0027f, 0.0030f, 0.0033f, 0.0036f, 0.0040f,
        0.0045f, 0.0051f, 0.0058f, 0.0067f, 0.0077f, 0.0090f, 0.0107f, 0.0128f,
        0.0156f, 0.0192f, 0.0242f, 0.0308f, 0.0398f, 0.0515f, 0.0650f, 0.0772f,
        0.0824f
    };
static const float glorentz53[27] =
    {
        0.0019f, 0.0021f, 0.0022f, 0.0024f, 0.0027f, 0.0029f, 0.0032f, 0.0035f,
        0.0039f, 0.0044f, 0.0049f, 0.0055f, 0.0062f, 0.0072f, 0.0083f, 0.0096f,
        0.0113f, 0.0135f, 0.0164f, 0.0201f, 0.0249f, 0.0314f, 0.0398f, 0.0502f,
        0.0619f, 0.0718f, 0.0759f
    };
static const float glorentz54[30] =
    {
        0.0017f, 0.0018f, 0.0019f, 0.0021f, 0.0022f, 0.0024f, 0.0026f, 0.0029f,
        0.0031f, 0.0034f, 0.0038f, 0.0042f, 0.0047f, 0.0052f, 0.0059f, 0.0067f,
        0.0076f, 0.0088f, 0.0102f, 0.0120f, 0.0143f, 0.0171f, 0.0208f, 0.0256f,
        0.0317f, 0.0395f, 0.0488f, 0.0586f, 0.0666f, 0.0698f
    };
static const float glorentz55[32] =
    {
        0.0016f, 0.0017f, 0.0018f, 0.0019f, 0.0021f, 0.0022f, 0.0024f, 0.0026f,
        0.0028f, 0.0031f, 0.0034f, 0.0037f, 0.0041f, 0.0045f, 0.0050f, 0.0056f,
        0.0063f, 0.0071f, 0.0081f, 0.0094f, 0.0108f, 0.0127f, 0.0149f, 0.0178f,
        0.0214f, 0.0261f, 0.0318f, 0.0389f, 0.0470f, 0.0553f, 0.0618f, 0.0643f
    };
static const float glorentz56[35] =
    {
        0.0014f, 0.0015f, 0.0016f, 0.0017f, 0.0018f, 0.0020f, 0.0021f, 0.0023f,
        0.0024f, 0.0026f, 0.0028f, 0.0031f, 0.0033f, 0.0036f, 0.0040f, 0.0044f,
        0.0049f, 0.0054f, 0.0060f, 0.0067f, 0.0076f, 0.0087f, 0.0099f, 0.0114f,
        0.0133f, 0.0156f, 0.0184f, 0.0220f, 0.0264f, 0.0318f, 0.0381f, 0.0451f,
        0.0520f, 0.0572f, 0.0591f
    };
static const float glorentz57[38] =
    {
        0.0013f, 0.0014f, 0.0015f, 0.0016f, 0.0017f, 0.0018f, 0.0019f, 0.0020f,
        0.0021f, 0.0023f, 0.0024f, 0.0026f, 0.0028f, 0.0031f, 0.0033f, 0.0036f,
        0.0039f, 0.0043f, 0.0047f, 0.0052f, 0.0058f, 0.0064f, 0.0072f, 0.0081f,
        0.0092f, 0.0104f, 0.0120f, 0.0139f, 0.0162f, 0.0190f, 0.0224f, 0.0265f,
        0.0315f, 0.0371f, 0.0431f, 0.0487f, 0.0529f, 0.0544f
    };
static const float glorentz58[41] =
    {
        0.0012f, 0.0013f, 0.0014f, 0.0014f, 0.0015f, 0.0016f, 0.0017f, 0.0018f,
        0.0019f, 0.0020f, 0.0022f, 0.0023f, 0.0025f, 0.0026f, 0.0028f, 0.0030f,
        0.0033f, 0.0036f, 0.0039f, 0.0042f, 0.0046f, 0.0050f, 0.0056f, 0.0061f,
        0.0068f, 0.0076f, 0.0086f, 0.0097f, 0.0110f, 0.0125f, 0.0144f, 0.0167f,
        0.0194f, 0.0226f, 0.0265f, 0.0309f, 0.0359f, 0.0409f, 0.0455f, 0.0488f,
        0.0500f
    };
static const float glorentz59[45] =
    {
        0.0011f, 0.0012f, 0.0012f, 0.0013f, 0.0013f, 0.0014f, 0.0015f, 0.0016f,
        0.0016f, 0.0017f, 0.0018f, 0.0019f, 0.0021f, 0.0022f, 0.0023f, 0.0025f,
        0.0026f, 0.0028f, 0.0030f, 0.0033f, 0.0035f, 0.0038f, 0.0041f, 0.0045f,
        0.0049f, 0.0054f, 0.0059f, 0.0065f, 0.0072f, 0.0081f, 0.0090f, 0.0102f,
        0.0115f, 0.0130f, 0.0149f, 0.0171f, 0.0197f, 0.0227f, 0.0263f, 0.0302f,
        0.0345f, 0.0387f, 0.0425f, 0.0451f, 0.0460f
    };
static const float glorentz60[49] =
    {
        0.0010f, 0.0011f, 0.0011f, 0.0012f, 0.0012f, 0.0013f, 0.0013f, 0.0014f,
        0.0014f, 0.0015f, 0.0016f, 0.0017f, 0.0018f, 0.0019f, 0.0020f, 0.0021f,
        0.0022f, 0.0024f, 0.0025f, 0.0027f, 0.0028f, 0.0030f, 0.0033f, 0.0035f,
        0.0038f, 0.0041f, 0.0044f, 0.0048f, 0.0052f, 0.0057f, 0.0063f, 0.0069f,
        0.0077f, 0.0085f, 0.0095f, 0.0106f, 0.0119f, 0.0135f, 0.0153f, 0.0174f,
        0.0199f, 0.0227f, 0.0259f, 0.0293f, 0.0330f, 0.0365f, 0.0395f, 0.0415f,
        0.0423f
    };
static const float glorentz61[53] =
    {
        0.0009f, 0.0010f, 0.0010f, 0.0011f, 0.0011f, 0.0011f, 0.0012f, 0.0012f,
        0.0013f, 0.0014f, 0.0014f, 0.0015f, 0.0016f, 0.0016f, 0.0017f, 0.0018f,
        0.0019f, 0.0020f, 0.0021f, 0.0023f, 0.0024f, 0.0025f, 0.0027f, 0.0029f,
        0.0031f, 0.0033f, 0.0035f, 0.0038f, 0.0041f, 0.0044f, 0.0047f, 0.0051f,
        0.0056f, 0.0061f, 0.0067f, 0.0073f, 0.0081f, 0.0089f, 0.0099f, 0.0110f,
        0.0124f, 0.0139f, 0.0156f, 0.0176f, 0.0199f, 0.0225f, 0.0253f, 0.0283f,
        0.0314f, 0.0343f, 0.0367f, 0.0383f, 0.0389f
    };
static const float glorentz62[57] =
    {
        0.0009f, 0.0009f, 0.0009f, 0.0010f, 0.0010f, 0.0011f, 0.0011f, 0.0011f,
        0.0012f, 0.0012f, 0.0013f, 0.0013f, 0.0014f, 0.0015f, 0.0015f, 0.0016f,
        0.0017f, 0.0018f, 0.0019f, 0.0020f, 0.0021f, 0.0022f, 0.0023f, 0.0024f,
        0.0026f, 0.0027f, 0.0029f, 0.0031f, 0.0033f, 0.0035f, 0.0038f, 0.0040f,
        0.0043f, 0.0047f, 0.0050f, 0.0055f, 0.0059f, 0.0064f, 0.0070f, 0.0077f,
        0.0085f, 0.0093f, 0.0103f, 0.0114f, 0.0127f, 0.0142f, 0.0158f, 0.0177f,
        0.0198f, 0.0221f, 0.0246f, 0.0272f, 0.0297f, 0.0321f, 0.0340f, 0.0353f,
        0.0357f
    };
static const float glorentz63[62] =
    {
        0.0008f, 0.0008f, 0.0009f, 0.0009f, 0.0009f, 0.0010f, 0.0010f, 0.0010f,
        0.0011f, 0.0011f, 0.0011f, 0.0012f, 0.0012f, 0.0013f, 0.0013f, 0.0014f,
        0.0015f, 0.0015f, 0.0016f, 0.0017f, 0.0017f, 0.0018f, 0.0019f, 0.0020f,
        0.0021f, 0.0022f, 0.0023f, 0.0025f, 0.0026f, 0.0028f, 0.0029f, 0.0031f,
        0.0033f, 0.0035f, 0.0038f, 0.0040f, 0.0043f, 0.0046f, 0.0050f, 0.0053f,
        0.0058f, 0.0062f, 0.0068f, 0.0074f, 0.0081f, 0.0088f, 0.0097f, 0.0106f,
        0.0117f, 0.0130f, 0.0144f, 0.0159f, 0.0176f, 0.0195f, 0.0216f, 0.0237f,
        0.0259f, 0.0280f, 0.0299f, 0.0315f, 0.0325f, 0.0328f
    };
static const float glorentz64[65] =
    {
        0.0008f, 0.0008f, 0.0008f, 0.0009f, 0.0009f, 0.0009f, 0.0010f, 0.0010f,
        0.0010f, 0.0011f, 0.0011f, 0.0012f, 0.0012f, 0.0012f, 0.0013f, 0.0013f,
        0.0014f, 0.0014f, 0.0015f, 0.0016f, 0.0016f, 0.0017f, 0.0018f, 0.0019f,
        0.0020f, 0.0021f, 0.0022f, 0.0023f, 0.0024f, 0.0025f, 0.0027f, 0.0028f,
        0.0030f, 0.0031f, 0.0033f, 0.0035f, 0.0038f, 0.0040f, 0.0043f, 0.0046f,
        0.0049f, 0.0052f, 0.0056f, 0.0061f, 0.0066f, 0.0071f, 0.0077f, 0.0084f,
        0.0091f, 0.0100f, 0.0109f, 0.0120f, 0.0132f, 0.0145f, 0.0159f, 0.0175f,
        0.0192f, 0.0209f, 0.0228f, 0.0246f, 0.0264f, 0.0279f, 0.0291f, 0.0299f,
        0.0301f
    };
static const float *gptr_tab_lorentz[64] =
    {
        glorentz1, glorentz2, glorentz3, glorentz4,
        glorentz5, glorentz6, glorentz7, glorentz8,
        glorentz9, glorentz10, glorentz11, glorentz12,
        glorentz13, glorentz14, glorentz15, glorentz16,
        glorentz17, glorentz18, glorentz19, glorentz20,
        glorentz21, glorentz22, glorentz23, glorentz24,
        glorentz25, glorentz26, glorentz27, glorentz28,
        glorentz29, glorentz30, glorentz31, glorentz32,
        glorentz33, glorentz34, glorentz35, glorentz36,
        glorentz37, glorentz38, glorentz39, glorentz40,
        glorentz41, glorentz42, glorentz43, glorentz44,
        glorentz45, glorentz46, glorentz47, glorentz48,
        glorentz49, glorentz50, glorentz51, glorentz52,
        glorentz53, glorentz54, glorentz55, glorentz56,
        glorentz57, glorentz58, glorentz59, glorentz60,
        glorentz61, glorentz62, glorentz63, glorentz64
    };
int q65subs::q65_intrinsics_fastfading(q65_codec_ds *pCodec,
                                       float *pIntrinsics,				// intrinsic symbol probabilities output
                                       const float *pInputEnergies,	// received energies input
                                       const int submode,				// submode idx (0=A ... 4=E)
                                       const float B90Ts,				// spread bandwidth (90% fractional energy)
                                       const int fadingModel)			// 0=Gaussian 1=Lorentzian fade model
{
    int n, k, j;
    int nM, nN, nBinsPerTone, nBinsPerSymbol, nBinsPerCodeword;
    int hidx, hlen, hhsz, hlast;
    const float *hptr;
    float fTemp, fNoiseVar, sumix, maxlogp;
    float EsNoMetric,B90;
    float *weight;
    const float *pCurSym, *pCurBin;
    float *pCurIx;

    //	printf("pcodec=%08x submode=%d fadingmodel=%d B90Ts=%f\n",pCodec, submode,fadingModel, B90Ts);
    if (pCodec==NULL)
        return Q65_DECODE_INVPARAMS;	// invalid pCodec pointer


    if (submode<0 || submode>4)
        return Q65_DECODE_INVPARAMS;	// invalid submode

    // As the symbol duration in q65 is different than in QRA64,
    // the fading tables continue to be valid if the B90Ts parameter
    // is properly scaled to the QRA64 symbol interval
    // Compute index to most appropriate weighting function coefficients
    B90 = B90Ts/TS_QRA64;
    hidx = (int)(logf(B90)/logf(1.09f) - 0.499f);

    // Unlike in QRA64 we accept any B90, anyway limiting it to
    // the extreme cases (0.9 to 210 Hz approx.)
    if (hidx<0)
        hidx = 0;
    else
        if (hidx > 63)   //Changed by K1JT: previously max was 64.
            hidx=63;       //Changed by K1JT: previously max was 64.

    // select the appropriate weighting fading coefficients array
    if (fadingModel==0)
    {	 // gaussian fading model
        // point to gaussian energy weighting taps
        hlen = glen_tab_gauss[hidx];	 // hlen = (L+1)/2 (where L=(odd) number of taps of w fun)
        hptr = gptr_tab_gauss[hidx];     // pointer to the first (L+1)/2 coefficients of w fun
    }
    else if (fadingModel==1)
    {
        // point to lorentzian energy weighting taps
        hlen = glen_tab_lorentz[hidx];	 // hlen = (L+1)/2 (where L=(odd) number of taps of w fun)
        hptr = gptr_tab_lorentz[hidx];   // pointer to the first (L+1)/2 coefficients of w fun
    }
    else
        return Q65_DECODE_INVPARAMS;	 // invalid fading model

    // compute (euristically) the optimal decoder metric accordingly the given spread amount
    // We assume that the decoder 50% decoding threshold is:
    // Es/No(dB) = Es/No(AWGN)(dB) + 8*log(B90)/log(240)(dB)
    // that's to say, at the maximum Doppler spread bandwidth (240 Hz for QRA64)
    // there's a ~8 dB Es/No degradation over the AWGN case
    fTemp = 8.0f*logf(B90)/logf(240.0f); // assumed Es/No degradation for the given fading bandwidth
    EsNoMetric = pCodec->decoderEsNoMetric*powf(10.0f,fTemp/10.0f);

    nM = q65_get_alphabet_size(pCodec);
    nN = q65_get_codeword_length(pCodec);
    nBinsPerTone   = 1<<submode;

    nBinsPerSymbol = nM*(2+nBinsPerTone);
    nBinsPerCodeword = nN*nBinsPerSymbol;

    // In the fast fading case , the intrinsic probabilities can be computed only
    // if both the noise spectral density and the average Es/No ratio are known.

    // Assuming that the energy of a tone is spread, on average, over adjacent bins
    // with the weights given in the precomputed fast-fading tables, it turns out
    // that the probability that the transmitted tone was tone j when we observed
    // the energies En(1)...En(N) is:

    // prob(tone j| en1....enN) proportional to exp(sum(En(k,j)*w(k)/No))
    // where w(k) = (g(k)*Es/No)/(1 + g(k)*Es/No),
    // g(k) are constant coefficients given on the fading tables,
    // and En(k,j) denotes the Energy at offset k from the central bin of tone j

    // Therefore we:
    // 1) compute No - the noise spectral density (or noise variance)
    // 2) compute the coefficients w(k) given the coefficient g(k) for the given decodeer Es/No metric
    // 3) compute the logarithm of prob(tone j| en1....enN) which is simply = sum(En(k,j)*w(k)/No
    // 4) subtract from the logarithm of the probabilities their maximum,
    // 5) exponentiate the logarithms
    // 6) normalize the result to a probability distribution dividing each value
    //    by the sum of all of them


    // Evaluate the average noise spectral density
    //qDebug()<<"------>"<<nN<<nM<<nBinsPerSymbol<<"all="<<nBinsPerCodeword;
    fNoiseVar = 0;
    for (k=0;k<nBinsPerCodeword;k++)
        fNoiseVar += pInputEnergies[k];
    fNoiseVar = fNoiseVar/nBinsPerCodeword;
    // The noise spectral density so computed includes also the signal power.
    // Therefore we scale it accordingly to the Es/No assumed by the decoder
    fNoiseVar = fNoiseVar/(1.0f+EsNoMetric/nBinsPerSymbol);
    // The value so computed is an overestimate of the true noise spectral density
    // by the (unknown) factor (1+Es/No(true)/nBinsPerSymbol)/(1+EsNoMetric/nBinsPerSymbol)
    // We will take this factor in account when computing the true Es/No ratio

    // store in the pCodec structure for later use in the estimation of the Es/No ratio
    pCodec->ffNoiseVar		= fNoiseVar;
    pCodec->ffEsNoMetric	= EsNoMetric;
    pCodec->nBinsPerTone    = nBinsPerTone;
    pCodec->nBinsPerSymbol  = nBinsPerSymbol;
    pCodec->nWeights        = hlen;
    weight					= pCodec->ffWeight;

    // compute the fast fading weights accordingly to the Es/No ratio
    // for which we compute the exact intrinsics probabilities
    for (k=0;k<hlen;k++)
    {
        fTemp = hptr[k]*EsNoMetric;
        //		printf("%d  %d  %f  %f  %f\n",hlen,k,EsNoMetric,hptr[k],fTemp);
        weight[k] = fTemp/(1.0f+fTemp)/fNoiseVar;
    }

    // Compute now the instrinsics as indicated above
    pCurSym = pInputEnergies + nM;	// point to the central bin of the the first symbol tone
    pCurIx  = pIntrinsics;			// point to the first intrinsic

    hhsz  = hlen-1;		// number of symmetric taps
    hlast = 2*hhsz;		// index of the central tap
    //qDebug()<<"row="<<nN<<"col="<<nM;
    for (n=0;n<nN;n++)
    {			// for each symbol in the message

        // compute the logarithm of the tone probability
        // as a weighted sum of the pertaining energies
        pCurBin = pCurSym -hlen+1;	// point to the first bin of the current symbol

        maxlogp = 0.0f;
        for (k=0;k<nM;k++)
        {		// for each tone in the current symbol
            // do a symmetric weighted sum
            fTemp = 0.0f;
            for (j=0;j<hhsz;j++)
                fTemp += weight[j]*(pCurBin[j] + pCurBin[hlast-j]);
            fTemp += weight[hhsz]*pCurBin[hhsz];

            if (fTemp>maxlogp)		// keep track of the max
                maxlogp = fTemp;
            pCurIx[k]=fTemp;

            pCurBin += nBinsPerTone;	// next tone
        }

        // exponentiate and accumulate the normalization constant
        sumix = 0.0f;
        /*for (k=0;k<nM;k++)
        {
            fTemp = expf(pCurIx[k]-maxlogp);
            pCurIx[k]=fTemp;
            sumix  +=fTemp;
        }*/
        for (k=0;k<nM;k++) //2.57
        {
		  float x=pCurIx[k]-maxlogp;
		  if(x < -85.0) x=-85.0;
		  if(x >  85.0) x= 85.0;
		  fTemp = expf(x);
		  pCurIx[k]=fTemp;
		  sumix  +=fTemp;
		}

        // scale to a probability distribution
        sumix = 1.0f/sumix;
        for (k=0;k<nM;k++)
            pCurIx[k] = pCurIx[k]*sumix;

        pCurSym +=nBinsPerSymbol;	// next symbol input energies
        pCurIx  +=nM;				// next symbol intrinsics
    }

    return 1;
}

// Max codeword list size in q65_decode_fullaplist
#define Q65_FULLAPLIST_SIZE	256
// Minimum codeword loglikelihood for decoding
#define Q65_LLH_THRESHOLD -260.0f
// Full AP decoding from a list of codewords
// Compute and verify the loglikelihood of the decoded codeword
int q65subs::q65_check_llh(float *llh, const int* ydec, const int nN, const int nM, const float *pIntrin)
{
    int k;
    float t = 0;

    /*for (k=0;k<nN;k++)
    {
        t+=logf(pIntrin[ydec[k]]);
        pIntrin+=nM;
    }*/
	for (k=0;k<nN;k++)//2.57 
	{
	  float x=pIntrin[ydec[k]];
	  if(x < 1.0e-36) x = 1.0e-36; 
	  t+=logf(x);
	  pIntrin+=nM;
	}    

    if (llh!=NULL)
        *llh = t;

    return (t>=Q65_LLH_THRESHOLD);
}
float q65_llh;
#define Q65_DECODE_FAILED		 -2
int q65subs::q65_decode_fullaplist(q65_codec_ds *codec,
                                   int *ydec,
                                   int *xdec,
                                   const float *pIntrinsics,
                                   const int *pCodewords,
                                   const int nCodewords)
{
    int			k;
    int			nK, nN, nM;

    float llh, maxllh, llh_threshold;
    int   maxcw = -1;					// index of the most likely codeword
    const int  *pCw;

    if (nCodewords<1 || nCodewords>Q65_FULLAPLIST_SIZE)
        return Q65_DECODE_INVPARAMS;	// invalid list length

    nK  = q65_get_message_length(codec);
    nN	= q65_get_codeword_length(codec);
    nM	= q65_get_alphabet_size(codec);
    //qDebug()<<"nN"<<nN;
    // we adjust the llh threshold in order to mantain the
    // same false decode rate independently from the size
    // of the list
    llh_threshold = Q65_LLH_THRESHOLD + logf(1.0f*nCodewords/3);
    maxllh = llh_threshold; // at least one llh should be larger than the threshold

    // compute codewords log likelihoods and find max
    pCw = pCodewords;	// start from the first codeword

    for (k=0;k<nCodewords;k++)
    {
        // compute and check this codeword loglikelihood
        if (q65_check_llh(&llh,pCw, nN, nM, pIntrinsics)==1) // larger than threshold
            // select the codeword with max logll
            if (llh>maxllh)
            {
                maxllh = llh;
                maxcw    = k;
            }
        //		printf("BBB  %d  %f\n",k,llh);
        // point to next codeword
        //qDebug()<<"bbbb===="<<llh<<maxllh;
        pCw+=nN;

        /*QString sss;
              for (int x = 0; x < 63; ++x) sss.append(QString("%1").arg((int)pCw[x]));
              qDebug()<<"msg"<<sss;*/
    }

    q65_llh=maxllh;		// save for Joe's use

    //qDebug()<<"nN===="<<nCodewords<<maxcw<<nN<<nM;

    if (maxcw<0) // no llh larger than threshold found
        return Q65_DECODE_FAILED;

    pCw = pCodewords+nN*maxcw;
    memcpy(ydec,pCw,nN*sizeof(int));
    memcpy(xdec,pCw,nK*sizeof(int));

    return maxcw;	// index to the decoded message (>=0)

}
int q65subs::q65_esnodb_fastfading(
    const q65_codec_ds *pCodec,
    float		*pEsNodB,
    const int   *ydec,
    const float *pInputEnergies)
{
    // Estimate the Es/No ratio of the decoded codeword

    int n,j;
    int nN, nM, nBinsPerSymbol, nBinsPerTone, nWeights, nTotWeights;
    const float *pCurSym, *pCurTone, *pCurBin;
    float EsPlusWNo,u, minu, ffNoiseVar, ffEsNoMetric;

    if (pCodec==NULL)
        return Q65_DECODE_INVPARAMS;

    nN = q65_get_codeword_length(pCodec);
    nM = q65_get_alphabet_size(pCodec);

    nBinsPerTone   = pCodec->nBinsPerTone;
    nBinsPerSymbol = pCodec->nBinsPerSymbol;
    nWeights       = pCodec->nWeights;
    ffNoiseVar	   = pCodec->ffNoiseVar;
    ffEsNoMetric   = pCodec->ffEsNoMetric;
    nTotWeights    = 2*nWeights-1;

    // compute symbols energy (noise included) summing the
    // energies pertaining to the decoded symbols in the codeword

    EsPlusWNo = 0.0f;
    pCurSym = pInputEnergies + nM;	// point to first central bin of first symbol tone
    for (n=0;n<nN;n++)
    {
        pCurTone = pCurSym + ydec[n]*nBinsPerTone;	 // point to the central bin of the current decoded symbol
        pCurBin  = pCurTone - nWeights+1;			 // point to first bin

        // sum over all the pertaining bins
        for (j=0;j<nTotWeights;j++)
            EsPlusWNo += pCurBin[j];

        pCurSym +=nBinsPerSymbol;

    }
    EsPlusWNo =  EsPlusWNo/nN;	// Es + nTotWeigths*No


    // The noise power ffNoiseVar computed in the q65_intrisics_fastading(...) function
    // is not the true noise power as it includes part of the signal energy.
    // The true noise variance is:
    // No = ffNoiseVar*(1+EsNoMetric/nBinsPerSymbol)/(1+EsNo/nBinsPerSymbol)

    // Therefore:
    // Es/No = EsPlusWNo/No - W = EsPlusWNo/ffNoiseVar*(1+Es/No/nBinsPerSymbol)/(1+Es/NoMetric/nBinsPerSymbol) - W
    // and:
    // Es/No*(1-u/nBinsPerSymbol) = u-W or Es/No = (u-W)/(1-u/nBinsPerSymbol)
    // where:
    // u = EsPlusNo/ffNoiseVar/(1+EsNoMetric/nBinsPerSymbol)

    u = EsPlusWNo/(ffNoiseVar*(1+ffEsNoMetric/nBinsPerSymbol));

    minu = nTotWeights+0.316f;
    if (u<minu)
        u = minu;		// Limit the minimum Es/No to -5 dB approx.

    u = (u-nTotWeights)/(1.0f -u/nBinsPerSymbol);  // linear scale Es/No
    *pEsNodB = 10.0f*log10f(u);

    return 1;
}



void q65subs::pd_bwdperm(float *dst, float *src, const  int *perm, int ndim)
{
    // TODO: non-loop implementation
    while (ndim--)
        dst[perm[ndim]] = src[ndim];
}
void q65subs::pd_fwdperm(float *dst, float *src, const  int *perm, int ndim)
{
    // TODO: non-loop implementation
    while (ndim--)
        dst[ndim] = src[perm[ndim]];
}
float q65subs::pd_max(float *src, int ndim)
{
    // TODO: faster implementation
    float cmax=0;  // we assume that prob distributions are always positive
    float cval;

    while (ndim--)
    {
        cval = src[ndim];
        if (cval>=cmax)
        {
            cmax = cval;
        }
    }
    return cmax;
}
int q65subs::pd_argmax(float *pmax, float *src, int ndim)
{
    // TODO: faster implementation

    float cmax=0;  // we assume that prob distributions are always positive
    float cval;
    int idxmax=-1; // indicates that all pd elements are <0

    while (ndim--)
    {
        cval = src[ndim];
        if (cval>=cmax)
        {
            cmax = cval;
            idxmax = ndim;
        }
    }

    if (pmax)
        *pmax = cmax;

    return idxmax;
}

typedef void  (*ppd_imul)(float*,const float*);
static void pd_imul1(float *dst, const float *src)
{
    dst[0] *= src[0];
}
static void pd_imul2(float *dst, const float *src)
{
    dst[0] *= src[0];
    dst[1] *= src[1];
}
static void pd_imul4(float *dst, const float *src)
{
    dst[0] *= src[0];
    dst[1] *= src[1];
    dst[2] *= src[2];
    dst[3] *= src[3];
}
static void pd_imul8(float *dst, const float *src)
{
    dst[0] *= src[0];
    dst[1] *= src[1];
    dst[2] *= src[2];
    dst[3] *= src[3];
    dst[4] *= src[4];
    dst[5] *= src[5];
    dst[6] *= src[6];
    dst[7] *= src[7];
}
static void pd_imul16(float *dst, const float *src)
{
    dst[0] *= src[0];
    dst[1] *= src[1];
    dst[2] *= src[2];
    dst[3] *= src[3];
    dst[4] *= src[4];
    dst[5] *= src[5];
    dst[6] *= src[6];
    dst[7] *= src[7];
    dst[8] *= src[8];
    dst[9] *= src[9];
    dst[10]*= src[10];
    dst[11]*= src[11];
    dst[12]*= src[12];
    dst[13]*= src[13];
    dst[14]*= src[14];
    dst[15]*= src[15];
}
static void pd_imul32(float *dst, const float *src)
{
    pd_imul16(dst,src);
    pd_imul16(dst+16,src+16);
}
static void pd_imul64(float *dst, const float *src)
{
    pd_imul16(dst, src);
    pd_imul16(dst+16, src+16);
    pd_imul16(dst+32, src+32);
    pd_imul16(dst+48, src+48);
}
static const ppd_imul pd_imul_tab[7] =
    {
        pd_imul1,
        pd_imul2,
        pd_imul4,
        pd_imul8,
        pd_imul16,
        pd_imul32,
        pd_imul64
    };
void q65subs::pd_imul(float *dst, const float *src, int nlogdim)
{
    pd_imul_tab[nlogdim](dst,src);
}
/*void q65subs::pd_imul(float *dst, const float *src, int nlogdim)
{
    int c1 = pow(2,nlogdim);
    for (int i = 0; i<c1; i++) dst[i]*= src[i];
}*/
#define WHBFY(dst,src,base,offs,dist) { dst[base+offs]=src[base+offs]+src[base+offs+dist]; dst[base+offs+dist]=src[base+offs]-src[base+offs+dist]; }
typedef void (*pnp_fwht)(float*,float*);
static void np_fwht2(float *dst, float *src);
static void np_fwht1(float *dst, float *src);
static void np_fwht2(float *dst, float *src);
static void np_fwht4(float *dst, float *src);
static void np_fwht8(float *dst, float *src);
static void np_fwht16(float *dst, float *src);
static void np_fwht32(float *dst, float *src);
static void np_fwht64(float *dst, float *src);
static pnp_fwht np_fwht_tab[7] =
    {
        np_fwht1,
        np_fwht2,
        np_fwht4,
        np_fwht8,
        np_fwht16,
        np_fwht32,
        np_fwht64
    };
static void np_fwht1(float *dst, float *src)
{
    dst[0] = src[0];
}
static void np_fwht2(float *dst, float *src)
{
    float t[2];

    WHBFY(t,src,0,0,1);
    dst[0]= t[0];
    dst[1]= t[1];
}
static void np_fwht4(float *dst, float *src)
{
    float t[4];

    // group 1
    WHBFY(t,src,0,0,2);
    WHBFY(t,src,0,1,2);
    // group 2
    WHBFY(dst,t,0,0,1);
    WHBFY(dst,t,2,0,1);
}
static void np_fwht8(float *dst, float *src)
{
    float t[16];
    float *t1=t, *t2=t+8;

    // group 1
    WHBFY(t1,src,0,0,4);
    WHBFY(t1,src,0,1,4);
    WHBFY(t1,src,0,2,4);
    WHBFY(t1,src,0,3,4);
    // group 2
    WHBFY(t2,t1,0,0,2);
    WHBFY(t2,t1,0,1,2);
    WHBFY(t2,t1,4,0,2);
    WHBFY(t2,t1,4,1,2);
    // group 3
    WHBFY(dst,t2,0,0,1);
    WHBFY(dst,t2,2,0,1);
    WHBFY(dst,t2,4,0,1);
    WHBFY(dst,t2,6,0,1);
}
static void np_fwht16(float *dst, float *src)
{
    float t[32];
    float *t1=t, *t2=t+16;

    // group 1
    WHBFY(t1,src,0,0,8);
    WHBFY(t1,src,0,1,8);
    WHBFY(t1,src,0,2,8);
    WHBFY(t1,src,0,3,8);
    WHBFY(t1,src,0,4,8);
    WHBFY(t1,src,0,5,8);
    WHBFY(t1,src,0,6,8);
    WHBFY(t1,src,0,7,8);
    // group 2
    WHBFY(t2,t1,0,0,4);
    WHBFY(t2,t1,0,1,4);
    WHBFY(t2,t1,0,2,4);
    WHBFY(t2,t1,0,3,4);
    WHBFY(t2,t1,8,0,4);
    WHBFY(t2,t1,8,1,4);
    WHBFY(t2,t1,8,2,4);
    WHBFY(t2,t1,8,3,4);
    // group 3
    WHBFY(t1,t2,0,0,2);
    WHBFY(t1,t2,0,1,2);
    WHBFY(t1,t2,4,0,2);
    WHBFY(t1,t2,4,1,2);
    WHBFY(t1,t2,8,0,2);
    WHBFY(t1,t2,8,1,2);
    WHBFY(t1,t2,12,0,2);
    WHBFY(t1,t2,12,1,2);
    // group 4
    WHBFY(dst,t1,0,0,1);
    WHBFY(dst,t1,2,0,1);
    WHBFY(dst,t1,4,0,1);
    WHBFY(dst,t1,6,0,1);
    WHBFY(dst,t1,8,0,1);
    WHBFY(dst,t1,10,0,1);
    WHBFY(dst,t1,12,0,1);
    WHBFY(dst,t1,14,0,1);

}
static void np_fwht32(float *dst, float *src)
{
    float t[64];
    float *t1=t, *t2=t+32;

    // group 1
    WHBFY(t1,src,0,0,16);
    WHBFY(t1,src,0,1,16);
    WHBFY(t1,src,0,2,16);
    WHBFY(t1,src,0,3,16);
    WHBFY(t1,src,0,4,16);
    WHBFY(t1,src,0,5,16);
    WHBFY(t1,src,0,6,16);
    WHBFY(t1,src,0,7,16);
    WHBFY(t1,src,0,8,16);
    WHBFY(t1,src,0,9,16);
    WHBFY(t1,src,0,10,16);
    WHBFY(t1,src,0,11,16);
    WHBFY(t1,src,0,12,16);
    WHBFY(t1,src,0,13,16);
    WHBFY(t1,src,0,14,16);
    WHBFY(t1,src,0,15,16);

    // group 2
    WHBFY(t2,t1,0,0,8);
    WHBFY(t2,t1,0,1,8);
    WHBFY(t2,t1,0,2,8);
    WHBFY(t2,t1,0,3,8);
    WHBFY(t2,t1,0,4,8);
    WHBFY(t2,t1,0,5,8);
    WHBFY(t2,t1,0,6,8);
    WHBFY(t2,t1,0,7,8);
    WHBFY(t2,t1,16,0,8);
    WHBFY(t2,t1,16,1,8);
    WHBFY(t2,t1,16,2,8);
    WHBFY(t2,t1,16,3,8);
    WHBFY(t2,t1,16,4,8);
    WHBFY(t2,t1,16,5,8);
    WHBFY(t2,t1,16,6,8);
    WHBFY(t2,t1,16,7,8);

    // group 3
    WHBFY(t1,t2,0,0,4);
    WHBFY(t1,t2,0,1,4);
    WHBFY(t1,t2,0,2,4);
    WHBFY(t1,t2,0,3,4);
    WHBFY(t1,t2,8,0,4);
    WHBFY(t1,t2,8,1,4);
    WHBFY(t1,t2,8,2,4);
    WHBFY(t1,t2,8,3,4);
    WHBFY(t1,t2,16,0,4);
    WHBFY(t1,t2,16,1,4);
    WHBFY(t1,t2,16,2,4);
    WHBFY(t1,t2,16,3,4);
    WHBFY(t1,t2,24,0,4);
    WHBFY(t1,t2,24,1,4);
    WHBFY(t1,t2,24,2,4);
    WHBFY(t1,t2,24,3,4);

    // group 4
    WHBFY(t2,t1,0,0,2);
    WHBFY(t2,t1,0,1,2);
    WHBFY(t2,t1,4,0,2);
    WHBFY(t2,t1,4,1,2);
    WHBFY(t2,t1,8,0,2);
    WHBFY(t2,t1,8,1,2);
    WHBFY(t2,t1,12,0,2);
    WHBFY(t2,t1,12,1,2);
    WHBFY(t2,t1,16,0,2);
    WHBFY(t2,t1,16,1,2);
    WHBFY(t2,t1,20,0,2);
    WHBFY(t2,t1,20,1,2);
    WHBFY(t2,t1,24,0,2);
    WHBFY(t2,t1,24,1,2);
    WHBFY(t2,t1,28,0,2);
    WHBFY(t2,t1,28,1,2);

    // group 5
    WHBFY(dst,t2,0,0,1);
    WHBFY(dst,t2,2,0,1);
    WHBFY(dst,t2,4,0,1);
    WHBFY(dst,t2,6,0,1);
    WHBFY(dst,t2,8,0,1);
    WHBFY(dst,t2,10,0,1);
    WHBFY(dst,t2,12,0,1);
    WHBFY(dst,t2,14,0,1);
    WHBFY(dst,t2,16,0,1);
    WHBFY(dst,t2,18,0,1);
    WHBFY(dst,t2,20,0,1);
    WHBFY(dst,t2,22,0,1);
    WHBFY(dst,t2,24,0,1);
    WHBFY(dst,t2,26,0,1);
    WHBFY(dst,t2,28,0,1);
    WHBFY(dst,t2,30,0,1);

}
static void np_fwht64(float *dst, float *src)
{
    float t[128];
    float *t1=t, *t2=t+64;

    // group 1
    WHBFY(t1,src,0,0,32);
    WHBFY(t1,src,0,1,32);
    WHBFY(t1,src,0,2,32);
    WHBFY(t1,src,0,3,32);
    WHBFY(t1,src,0,4,32);
    WHBFY(t1,src,0,5,32);
    WHBFY(t1,src,0,6,32);
    WHBFY(t1,src,0,7,32);
    WHBFY(t1,src,0,8,32);
    WHBFY(t1,src,0,9,32);
    WHBFY(t1,src,0,10,32);
    WHBFY(t1,src,0,11,32);
    WHBFY(t1,src,0,12,32);
    WHBFY(t1,src,0,13,32);
    WHBFY(t1,src,0,14,32);
    WHBFY(t1,src,0,15,32);
    WHBFY(t1,src,0,16,32);
    WHBFY(t1,src,0,17,32);
    WHBFY(t1,src,0,18,32);
    WHBFY(t1,src,0,19,32);
    WHBFY(t1,src,0,20,32);
    WHBFY(t1,src,0,21,32);
    WHBFY(t1,src,0,22,32);
    WHBFY(t1,src,0,23,32);
    WHBFY(t1,src,0,24,32);
    WHBFY(t1,src,0,25,32);
    WHBFY(t1,src,0,26,32);
    WHBFY(t1,src,0,27,32);
    WHBFY(t1,src,0,28,32);
    WHBFY(t1,src,0,29,32);
    WHBFY(t1,src,0,30,32);
    WHBFY(t1,src,0,31,32);    //for (int i = 0; i<32; ++i) WHBFY(t1,src,0,i,32);

    // group 2
    WHBFY(t2,t1,0,0,16);
    WHBFY(t2,t1,0,1,16);
    WHBFY(t2,t1,0,2,16);
    WHBFY(t2,t1,0,3,16);
    WHBFY(t2,t1,0,4,16);
    WHBFY(t2,t1,0,5,16);
    WHBFY(t2,t1,0,6,16);
    WHBFY(t2,t1,0,7,16);
    WHBFY(t2,t1,0,8,16);
    WHBFY(t2,t1,0,9,16);
    WHBFY(t2,t1,0,10,16);
    WHBFY(t2,t1,0,11,16);
    WHBFY(t2,t1,0,12,16);
    WHBFY(t2,t1,0,13,16);
    WHBFY(t2,t1,0,14,16);
    WHBFY(t2,t1,0,15,16); 

    WHBFY(t2,t1,32,0,16);
    WHBFY(t2,t1,32,1,16);
    WHBFY(t2,t1,32,2,16);
    WHBFY(t2,t1,32,3,16);
    WHBFY(t2,t1,32,4,16);
    WHBFY(t2,t1,32,5,16);
    WHBFY(t2,t1,32,6,16);
    WHBFY(t2,t1,32,7,16);
    WHBFY(t2,t1,32,8,16);
    WHBFY(t2,t1,32,9,16);
    WHBFY(t2,t1,32,10,16);
    WHBFY(t2,t1,32,11,16);
    WHBFY(t2,t1,32,12,16);
    WHBFY(t2,t1,32,13,16);
    WHBFY(t2,t1,32,14,16);
    WHBFY(t2,t1,32,15,16);  

    // group 3
    WHBFY(t1,t2,0,0,8);
    WHBFY(t1,t2,0,1,8);
    WHBFY(t1,t2,0,2,8);
    WHBFY(t1,t2,0,3,8);
    WHBFY(t1,t2,0,4,8);
    WHBFY(t1,t2,0,5,8);
    WHBFY(t1,t2,0,6,8);
    WHBFY(t1,t2,0,7,8);
    WHBFY(t1,t2,16,0,8);
    WHBFY(t1,t2,16,1,8);
    WHBFY(t1,t2,16,2,8);
    WHBFY(t1,t2,16,3,8);
    WHBFY(t1,t2,16,4,8);
    WHBFY(t1,t2,16,5,8);
    WHBFY(t1,t2,16,6,8);
    WHBFY(t1,t2,16,7,8);
    WHBFY(t1,t2,32,0,8);
    WHBFY(t1,t2,32,1,8);
    WHBFY(t1,t2,32,2,8);
    WHBFY(t1,t2,32,3,8);
    WHBFY(t1,t2,32,4,8);
    WHBFY(t1,t2,32,5,8);
    WHBFY(t1,t2,32,6,8);
    WHBFY(t1,t2,32,7,8);
    WHBFY(t1,t2,48,0,8);
    WHBFY(t1,t2,48,1,8);
    WHBFY(t1,t2,48,2,8);
    WHBFY(t1,t2,48,3,8);
    WHBFY(t1,t2,48,4,8);
    WHBFY(t1,t2,48,5,8);
    WHBFY(t1,t2,48,6,8);
    WHBFY(t1,t2,48,7,8);

    // group 4
    WHBFY(t2,t1,0,0,4);
    WHBFY(t2,t1,0,1,4);
    WHBFY(t2,t1,0,2,4);
    WHBFY(t2,t1,0,3,4);
    WHBFY(t2,t1,8,0,4);
    WHBFY(t2,t1,8,1,4);
    WHBFY(t2,t1,8,2,4);
    WHBFY(t2,t1,8,3,4);
    WHBFY(t2,t1,16,0,4);
    WHBFY(t2,t1,16,1,4);
    WHBFY(t2,t1,16,2,4);
    WHBFY(t2,t1,16,3,4);
    WHBFY(t2,t1,24,0,4);
    WHBFY(t2,t1,24,1,4);
    WHBFY(t2,t1,24,2,4);
    WHBFY(t2,t1,24,3,4);
    WHBFY(t2,t1,32,0,4);
    WHBFY(t2,t1,32,1,4);
    WHBFY(t2,t1,32,2,4);
    WHBFY(t2,t1,32,3,4);
    WHBFY(t2,t1,40,0,4);
    WHBFY(t2,t1,40,1,4);
    WHBFY(t2,t1,40,2,4);
    WHBFY(t2,t1,40,3,4);
    WHBFY(t2,t1,48,0,4);
    WHBFY(t2,t1,48,1,4);
    WHBFY(t2,t1,48,2,4);
    WHBFY(t2,t1,48,3,4);
    WHBFY(t2,t1,56,0,4);
    WHBFY(t2,t1,56,1,4);
    WHBFY(t2,t1,56,2,4);
    WHBFY(t2,t1,56,3,4);

    // group 5
    WHBFY(t1,t2,0,0,2);
    WHBFY(t1,t2,0,1,2);
    WHBFY(t1,t2,4,0,2);
    WHBFY(t1,t2,4,1,2);
    WHBFY(t1,t2,8,0,2);
    WHBFY(t1,t2,8,1,2);
    WHBFY(t1,t2,12,0,2);
    WHBFY(t1,t2,12,1,2);
    WHBFY(t1,t2,16,0,2);
    WHBFY(t1,t2,16,1,2);
    WHBFY(t1,t2,20,0,2);
    WHBFY(t1,t2,20,1,2);
    WHBFY(t1,t2,24,0,2);
    WHBFY(t1,t2,24,1,2);
    WHBFY(t1,t2,28,0,2);
    WHBFY(t1,t2,28,1,2);
    WHBFY(t1,t2,32,0,2);
    WHBFY(t1,t2,32,1,2);
    WHBFY(t1,t2,36,0,2);
    WHBFY(t1,t2,36,1,2);
    WHBFY(t1,t2,40,0,2);
    WHBFY(t1,t2,40,1,2);
    WHBFY(t1,t2,44,0,2);
    WHBFY(t1,t2,44,1,2);
    WHBFY(t1,t2,48,0,2);
    WHBFY(t1,t2,48,1,2);
    WHBFY(t1,t2,52,0,2);
    WHBFY(t1,t2,52,1,2);
    WHBFY(t1,t2,56,0,2);
    WHBFY(t1,t2,56,1,2);
    WHBFY(t1,t2,60,0,2);
    WHBFY(t1,t2,60,1,2);

    // group 6
    WHBFY(dst,t1,0,0,1);
    WHBFY(dst,t1,2,0,1);
    WHBFY(dst,t1,4,0,1);
    WHBFY(dst,t1,6,0,1);
    WHBFY(dst,t1,8,0,1);
    WHBFY(dst,t1,10,0,1);
    WHBFY(dst,t1,12,0,1);
    WHBFY(dst,t1,14,0,1);
    WHBFY(dst,t1,16,0,1);
    WHBFY(dst,t1,18,0,1);
    WHBFY(dst,t1,20,0,1);
    WHBFY(dst,t1,22,0,1);
    WHBFY(dst,t1,24,0,1);
    WHBFY(dst,t1,26,0,1);
    WHBFY(dst,t1,28,0,1);
    WHBFY(dst,t1,30,0,1);
    WHBFY(dst,t1,32,0,1);
    WHBFY(dst,t1,34,0,1);
    WHBFY(dst,t1,36,0,1);
    WHBFY(dst,t1,38,0,1);
    WHBFY(dst,t1,40,0,1);
    WHBFY(dst,t1,42,0,1);
    WHBFY(dst,t1,44,0,1);
    WHBFY(dst,t1,46,0,1);
    WHBFY(dst,t1,48,0,1);
    WHBFY(dst,t1,50,0,1);
    WHBFY(dst,t1,52,0,1);
    WHBFY(dst,t1,54,0,1);
    WHBFY(dst,t1,56,0,1);
    WHBFY(dst,t1,58,0,1);
    WHBFY(dst,t1,60,0,1);
    WHBFY(dst,t1,62,0,1);
}

void q65subs::np_fwht(int nlogdim, float *dst, float *src)
{
    np_fwht_tab[nlogdim](dst,src);
}

#define ADDRMSG(fp, msgidx)    PD_ROWADDR(fp,qra_M,msgidx)
#define C2VMSG(msgidx)         PD_ROWADDR(qra_c2vmsg,qra_M,msgidx)
#define V2CMSG(msgidx)         PD_ROWADDR(qra_v2cmsg,qra_M,msgidx)
#define MSGPERM(logw)          PD_ROWADDR(qra_pmat,qra_M,logw)

#define QRACODE_MAX_M	256	// Maximum alphabet size handled by qra_extrinsic

int q65subs::qra_extrinsic(const qracode *pcode,
                           float *pex,
                           const float *pix,
                           int maxiter,
                           float *qra_v2cmsg,
                           float *qra_c2vmsg)
{
    const int qra_M		= pcode->M;
    const int qra_m		= pcode->m;
    const int qra_V		= pcode->V;
    const int qra_MAXVDEG  = pcode->MAXVDEG;
    const int  *qra_vdeg    = pcode->vdeg;
    const int qra_C		= pcode->C;
    const int qra_MAXCDEG  = pcode->MAXCDEG;
    const int *qra_cdeg    = pcode->cdeg;
    const int  *qra_v2cmidx = pcode->v2cmidx;
    const int  *qra_c2vmidx = pcode->c2vmidx;
    const int  *qra_pmat    = pcode->gfpmat;
    const int *qra_msgw    = pcode->msgw;

//	float msgout[qra_M];		 // buffer to store temporary results
    float msgout[QRACODE_MAX_M]; // we use a fixed size in order to avoid mallocs

    float totex;	// total extrinsic information
    int nit;		// current iteration
    int nv;		// current variable
    int nc;		// current check
    int k,kk;		// loop indexes

    int ndeg;		// current node degree
    int msgbase;	// current offset in the table of msg indexes
    int imsg;		// current message index
    int wmsg;		// current message weight

    int rc     = -1; // rc>=0  extrinsic converged to 1 at iteration rc (rc=0..maxiter-1)
    // rc=-1  no convergence in the given number of iterations
    // rc=-2  error in the code tables (code checks degrees must be >1)
    // rc=-3  M is larger than QRACODE_MAX_M



    if (qra_M>QRACODE_MAX_M)
        return -3;

    // message initialization -------------------------------------------------------

    // init c->v variable intrinsic msgs
    pd_init(C2VMSG(0),pix,qra_M*qra_V);

    // init the v->c messages directed to code factors (k=1..ndeg) with the intrinsic info
    for (nv=0;nv<qra_V;nv++)
    {

        ndeg = qra_vdeg[nv];		// degree of current node
        msgbase = nv*qra_MAXVDEG;	// base to msg index row for the current node

        // copy intrinsics on v->c
        for (k=1;k<ndeg;k++)
        {
            imsg = qra_v2cmidx[msgbase+k];
            pd_init(V2CMSG(imsg),ADDRMSG(pix,nv),qra_M);
        }
    }

    // message passing algorithm iterations ------------------------------

    for (nit=0;nit<maxiter;nit++)
    {

        // c->v step -----------------------------------------------------
        // Computes messages from code checks to code variables.
        // As the first qra_V checks are associated with intrinsic information
        // (the code tables have been constructed in this way)
        // we need to do this step only for code checks in the range [qra_V..qra_C)

        // The convolutions of probability distributions over the alphabet of a finite field GF(qra_M)
        // are performed with a fast convolution algorithm over the given field.
        //
        // I.e. given the code check x1+x2+x3 = 0 (with x1,x2,x3 in GF(2^m))
        // and given Prob(x2) and Prob(x3), we have that:
        // Prob(x1=X1) = Prob((x2+x3)=X1) = sum((Prob(x2=X2)*Prob(x3=(X1+X2))) for all the X2s in the field
        // This translates to Prob(x1) = IWHT(WHT(Prob(x2))*WHT(Prob(x3)))
        // where WHT and IWHT are the direct and inverse Walsh-Hadamard transforms of the argument.
        // Note that the WHT and the IWHF differs only by a multiplicative coefficent and since in this step
        // we don't need that the output distribution is normalized we use the relationship
        // Prob(x1) =(proportional to) WH(WH(Prob(x2))*WH(Prob(x3)))

        // In general given the check code x1+x2+x3+..+xm = 0
        // the output distribution of a variable given the distributions of the other m-1 variables
        // is the inverse WHT of the product of the WHTs of the distribution of the other m-1 variables
        // The complexity of this algorithm scales with M*log2(M) instead of the M^2 complexity of
        // the brute force approach (M=size of the alphabet)

        for (nc=qra_V;nc<qra_C;nc++)
        {

            ndeg = qra_cdeg[nc];		// degree of current node

            if (ndeg==1) 				// this should never happen (code factors must have deg>1)
                return -2;				// bad code tables

            msgbase = nc*qra_MAXCDEG;	// base to msg index row for the current node

            // transforms inputs in the Walsh-Hadamard "frequency" domain
            // v->c  -> fwht(v->c)
            for (k=0;k<ndeg;k++)
            {
                imsg = qra_c2vmidx[msgbase+k];		// msg index
                np_fwht(qra_m,V2CMSG(imsg),V2CMSG(imsg)); // compute fwht
            }

            // compute products and transform them back in the WH "time" domain
            for (k=0;k<ndeg;k++)
            {

                // init output message to uniform distribution
                pd_init(msgout,pd_uniform(qra_m),qra_M);

                // c->v = prod(fwht(v->c))
                // TODO: we assume that checks degrees are not larger than three but
                // if they are larger the products can be computed more efficiently
                for (kk=0;kk<ndeg;kk++)
                    if (kk!=k)
                    {
                        imsg = qra_c2vmidx[msgbase+kk];
                        pd_imul(msgout,V2CMSG(imsg),qra_m);
                    }

                // transform product back in the WH "time" domain

                // Very important trick:
                // we bias WHT[0] so that the sum of output pd components is always strictly positive
                // this helps avoiding the effects of underflows in the v->c steps when multipling
                // small fp numbers
                msgout[0]+=1E-7f;	// TODO: define the bias accordingly to the field size

                np_fwht(qra_m,msgout,msgout);

                // inverse weight and output
                imsg = qra_c2vmidx[msgbase+k]; // current output msg index
                wmsg = qra_msgw[imsg];		   // current msg weight

                if (wmsg==0)
                    pd_init(C2VMSG(imsg),msgout,qra_M);
                else
                    // output p(alfa^(-w)*x)
                    pd_bwdperm(C2VMSG(imsg),msgout, MSGPERM(wmsg), qra_M);

            } // for (k=0;k<ndeg;k++)

        } // for (nc=qra_V;nc<qra_C;nc++)

        // v->c step -----------------------------------------------------
        for (nv=0;nv<qra_V;nv++)
        {

            ndeg = qra_vdeg[nv];		// degree of current node
            msgbase = nv*qra_MAXVDEG;	// base to msg index row for the current node

            for (k=0;k<ndeg;k++)
            {
                // init output message to uniform distribution
                pd_init(msgout,pd_uniform(qra_m),qra_M);

                // v->c msg = prod(c->v)
                // TODO: factor factors to reduce the number of computations for high degree nodes
                for (kk=0;kk<ndeg;kk++)
                    if (kk!=k)
                    {
                        imsg = qra_v2cmidx[msgbase+kk];
                        pd_imul(msgout,C2VMSG(imsg),qra_m);
                    }

#ifdef QRA_DEBUG
// normalize and check if product of messages v->c are null
                // normalize output to a probability distribution
                if (pd_norm(msgout,qra_m)<=0)
                {
                    // dump msgin;
                    printf("warning: v->c pd with invalid norm. nit=%d nv=%d k=%d\n",nit,nv,k);
                    for (kk=0;kk<ndeg;kk++)
                    {
                        imsg = qra_v2cmidx[msgbase+kk];
                        pd_print(imsg,C2VMSG(imsg),qra_M);
                    }
                    printf("-----------------\n");
                }
#else
                // normalize the result to a probability distribution
                pd_norm(msgout,qra_m);
#endif
                // weight and output
                imsg = qra_v2cmidx[msgbase+k]; // current output msg index
                wmsg = qra_msgw[imsg];		   // current msg weight

                if (wmsg==0)
                {
                    pd_init(V2CMSG(imsg),msgout,qra_M);
                }
                else
                {
                    // output p(alfa^w*x)
                    pd_fwdperm(V2CMSG(imsg),msgout, MSGPERM(wmsg), qra_M);
                }

            } // for (k=0;k<ndeg;k++)
        } // for (nv=0;nv<qra_V;nv++)

        // check extrinsic information ------------------------------
        // We assume that decoding is successful if each of the extrinsic
        // symbol probability is close to ej, where ej = [0 0 0 1(j-th position) 0 0 0 ]
        // Therefore, for each symbol k in the codeword we compute max(prob(Xk))
        // and we stop the iterations if sum(max(prob(xk)) is close to the codeword length
        // Note: this is a more restrictive criterium than that of computing the a
        // posteriori probability of each symbol, making a hard decision and then check
        // if the codeword syndrome is null.
        // WARNING: this is tricky and probably works only for the particular class of RA codes
        // we are coping with (we designed the code weights so that for any input symbol the
        // sum of its weigths is always 0, thus terminating the accumulator trellis to zero
        // for every combination of the systematic symbols).
        // More generally we should instead compute the max a posteriori probabilities
        // (as a product of the intrinsic and extrinsic information), make a symbol by symbol hard
        // decision and then check that the syndrome of the result is indeed null.

        totex = 0;
        for (nv=0;nv<qra_V;nv++)
            totex += pd_max(V2CMSG(nv),qra_M);

        if (totex>(1.*(qra_V)-0.01))
        {
            // the total maximum extrinsic information of each symbol in the codeword
            // is very close to one. This means that we have reached the (1,1) point in the
            // code EXIT chart(s) and we have successfully decoded the input.
            rc = nit;
            break;	// remove the break to evaluate the decoder speed performance as a function of the max iterations number)
        }

    } // for (nit=0;nit<maxiter;nit++)

    // copy extrinsic information to output to do the actual max a posteriori prob decoding
    pd_init(pex,V2CMSG(0),(qra_M*qra_V));
    return rc;
}

void q65subs::qra_mapdecode(const qracode *pcode, int *xdec, float *pex, const float *pix)
{
// Maximum a posteriori probability decoding.
// Given the intrinsic information (pix) and extrinsic information (pex) (computed with qra_extrinsic(...))
// compute pmap = pex*pix and decode each (information) symbol of the received codeword
// as the symbol which maximizes pmap

// Returns:
//	xdec[k] = decoded (information) symbols k=[0..qra_K-1]

//  Note: pex is destroyed and overwritten with mapp

    const int qra_M		= pcode->M;
    const int qra_m		= pcode->m;
    const int qra_K		= pcode->K;

    int k;

    for (k=0;k<qra_K;k++)
    {
        // compute a posteriori prob
        pd_imul(PD_ROWADDR(pex,qra_M,k),PD_ROWADDR(pix,qra_M,k),qra_m);
        xdec[k]=pd_argmax(NULL, PD_ROWADDR(pex,qra_M,k), qra_M);
    }
}

#define Q65_DECODE_CRCMISMATCH   -3
int q65subs::q65_decode(q65_codec_ds *pCodec, int* pDecodedCodeword, int *pDecodedMsg,
                        const float *pIntrinsics, const int *pAPMask, const int *pAPSymbols,int maxiters)
{
    const qracode *pQraCode;
    float	*ix, *ex;
    int		*px;
    int		*py;
    int		nK, nN, nM,nBits;
    int		rc;
    int		crc6;
    int		crc12[2];

    if (!pCodec)
        return Q65_DECODE_INVPARAMS;	// which codec?

    pQraCode	= pCodec->pQraCode;
    ix			= pCodec->ix;
    ex			= pCodec->ex;

    nK			= _q65_get_message_length(pQraCode);
    nN			= _q65_get_codeword_length(pQraCode);
    nM			= pQraCode->M;
    nBits		= pQraCode->m;

    px			= pCodec->x;
    py			= pCodec->y;

    // Depuncture intrinsics observations as required by the code type
    switch (pQraCode->type)
    {
    case QRATYPE_CRCPUNCTURED:
        memcpy(ix,pIntrinsics,nK*nM*sizeof(float));							// information symbols
        pd_init(PD_ROWADDR(ix,nM,nK),pd_uniform(nBits),nM);					// crc
        memcpy(ix+(nK+1)*nM,pIntrinsics+nK*nM,(nN-nK)*nM*sizeof(float));	// parity checks
        break;
    case QRATYPE_CRCPUNCTURED2:
        memcpy(ix,pIntrinsics,nK*nM*sizeof(float));							// information symbols
        pd_init(PD_ROWADDR(ix,nM,nK),pd_uniform(nBits),nM);					// crc
        pd_init(PD_ROWADDR(ix,nM,nK+1),pd_uniform(nBits),nM);				// crc
        memcpy(ix+(nK+2)*nM,pIntrinsics+nK*nM,(nN-nK)*nM*sizeof(float));	// parity checks
        break;
    case QRATYPE_NORMAL:
    case QRATYPE_CRC:
    default:
        // no puncturing
        memcpy(ix,pIntrinsics,nN*nM*sizeof(float));							// as they are
    }

    // mask the intrinsics with the available a priori knowledge
    if (pAPMask!=NULL)
        _q65_mask(pQraCode,ix,pAPMask,pAPSymbols);


    // Compute the extrinsic symbols probabilities with the message-passing algorithm
    // Stop if the extrinsics information does not converges to unity
    // within the given number of iterations
    rc = qra_extrinsic( pQraCode,
                        ex,
                        ix,
                        maxiters,
                        pCodec->qra_v2cmsg,
                        pCodec->qra_c2vmsg);

    if (rc<0)
        // failed to converge to a solution
        return Q65_DECODE_FAILED;

    // decode the information symbols (punctured information symbols included)
    qra_mapdecode(pQraCode,px,ex,ix);

    // verify CRC match

    switch (pQraCode->type)
    {
    case QRATYPE_CRC:
    case QRATYPE_CRCPUNCTURED:
        crc6=_q65_crc6(px,nK);			 // compute crc-6
        if (crc6!=px[nK])
            return Q65_DECODE_CRCMISMATCH; // crc doesn't match
        break;
    case QRATYPE_CRCPUNCTURED2:
        _q65_crc12(crc12, px,nK);			 // compute crc-12
        if (crc12[0]!=px[nK] ||
                crc12[1]!=px[nK+1])
            return Q65_DECODE_CRCMISMATCH; // crc doesn't match
        break;
    case QRATYPE_NORMAL:
    default:
        // nothing to check
        break;
    }

    // copy the decoded msg to the user buffer (excluding punctured symbols)
    if (pDecodedMsg)
        memcpy(pDecodedMsg,px,nK*sizeof(int));

#ifndef Q65_CHECKLLH
    if (pDecodedCodeword==NULL)		// user is not interested in the decoded codeword
        return rc;					// return the number of iterations required to decode
#else
    if (pDecodedCodeword==NULL)			// we must have a buffer
        return Q65_DECODE_INVPARAMS;	// return error
#endif

    // crc matches therefore we can reconstruct the transmitted codeword
    //  reencoding the information available in px...

    qra_encode(pQraCode, py, px);

    // ...and strip the punctured symbols from the codeword
    switch (pQraCode->type)
    {
    case QRATYPE_CRCPUNCTURED:
        memcpy(pDecodedCodeword,py,nK*sizeof(int));
        memcpy(pDecodedCodeword+nK,py+nK+1,(nN-nK)*sizeof(int));	// puncture crc-6 symbol
        break;
    case QRATYPE_CRCPUNCTURED2:
        memcpy(pDecodedCodeword,py,nK*sizeof(int));
        memcpy(pDecodedCodeword+nK,py+nK+2,(nN-nK)*sizeof(int));	// puncture crc-12 symbols
        break;
    case QRATYPE_CRC:
    case QRATYPE_NORMAL:
    default:
        memcpy(pDecodedCodeword,py,nN*sizeof(int));		// no puncturing
    }

#ifdef Q65_CHECKLLH
    if (q65_check_llh(NULL,pDecodedCodeword, nN, nM, pIntrinsics)==0) // llh less than threshold
        return Q65_DECODE_LLHLOW;
#endif

    return rc;	// return the number of iterations required to decode

}



///////////////////////////////// subs //////////////////////////////////////////////
static q65_codec_ds codec;
//#include <QtGui>

void q65subs::q65_enc(int x[], int y[])
{
    static int first=1;
    if (first)
    {
        // Set the QRA code, allocate memory, and initialize
        int rc = q65_init(&codec,&qra15_65_64_irr_e23);
        if (rc<0)
        {
            printf("error in q65_init()\n");
            exit(0);
        }
        first=0;
    }
    // Encode message x[13], producing codeword y[63]
    q65_encode(&codec,y,x);
}
void q65subs::q65_intrinsics_ff(float s3[], int submode, float B90Ts,
                                int fadingModel, float s3prob[])
{

    /* Input:   s3[LL,NN]       Received energies
     *          submode         0=A, 4=E
     *          B90             Spread bandwidth, 90% fractional energy
     *          fadingModel     0=Gaussian, 1=Lorentzian
     * Output:  s3prob[LL,NN]   Symbol-value intrinsic probabilities
     */

    int rc;
    static int first=1;

    if (first)
    {
        // Set the QRA code, allocate memory, and initialize
        int rc = q65_init(&codec,&qra15_65_64_irr_e23);
        if (rc<0)
        {
            printf("error in q65_init()\n");
            exit(0);
        }
        first=0;
    }
    rc = q65_intrinsics_fastfading(&codec,s3prob,s3,submode,B90Ts,fadingModel);
    if (rc<0)
    {
        printf("error in q65_intrinsics()\n");
        ///qDebug()<<"hgjhgdjhgj";
        exit(0);
    }
}
void q65subs::q65_dec(float s3[], float s3prob[], int APmask[], int APsymbols[],
                      int maxiters,float &esnodb0, int xdec[], int &rc0)
{

    /* Input:   s3[LL,NN]       Symbol spectra
     *          s3prob[LL,NN]   Symbol-value intrinsic probabilities
     *          APmask[13]      AP information to be used in decoding
     *          APsymbols[13]   Available AP informtion
     * Output:
     *          esnodb0         Estimated Es/No (dB)
     *          xdec[13]        Decoded 78-bit message as 13 six-bit integers
     *          rc0             Return code from q65_decode()
     */

    int rc;
    int ydec[63];
    float esnodb;

    rc = q65_decode(&codec,ydec,xdec,s3prob,APmask,APsymbols,maxiters);
    rc0=rc;
    // rc = -1:  Invalid params
    // rc = -2:  Decode failed
    // rc = -3:  CRC mismatch
    //qDebug()<<rc;
    esnodb0 = 0.0;             //Default Es/No for a failed decode
    if (rc<0) return;

    rc = q65_esnodb_fastfading(&codec,&esnodb,ydec,s3);
    if (rc<0)
    {
        printf("error in q65_esnodb_fastfading()\n");
        exit(0);
    }
    esnodb0 = esnodb;
}
void q65subs::q65_dec_fullaplist(float s3[], float s3prob[], int codewords[],
                                 int ncw, float &esnodb0, int xdec[], float &plog, int &rc0)
{
    /* Input:   s3[LL,NN]         Symbol spectra
     *          s3prob[LL,NN]     Symbol-value intrinsic probabilities
     *          codewords[63,ncw] Full codewords to search for
     *          ncw               Number of codewords
     * Output:
     *          esnodb0           Estimated Es/No (dB)
     *          xdec[13]          Decoded 78-bit message as 13 six-bit integers
     *          rc0               Return code from q65_decode()
     */

    int rc;
    int ydec[63];
    float esnodb;

    rc = q65_decode_fullaplist(&codec,ydec,xdec,s3prob,codewords,ncw);
    plog=q65_llh;
    rc0=rc;

    // rc = -1:  Invalid params
    // rc = -2:  Decode failed
    // rc = -3:  CRC mismatch
    esnodb0 = 0.0;             //Default Es/No for a failed decode
    if (rc<0) return;

    rc = q65_esnodb_fastfading(&codec,&esnodb,ydec,s3);
    if (rc<0)
    {
        printf("error in q65_esnodb_fastfading()\n");
        exit(0);
    }
    esnodb0 = esnodb;
}

