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
#ifndef Q65_SUBS_H
#define Q65_SUBS_H

typedef struct
{
    // code parameters
    const int K;			// number of information symbols
    const int N;			// codeword length in symbols
    const int m;			// bits/symbol
    const int M;			// Symbol alphabet cardinality (2^m)
    const int a;			// code grouping factor
    const int NC;			// number of check symbols (N-K)
    const int V;			// number of variables in the code graph (N)
    const int C;			// number of factors in the code graph (N +(N-K)+1)
    const int NMSG;		// number of msgs in the code graph
    const int MAXVDEG;		// maximum variable degree
    const int MAXCDEG;		// maximum factor degree
    const int type;		// see QRATYPE_xx defines
    const float R;			// code rate (K/N)
    const char  name[64];	// code name
    // tables used by the encoder
    const int	 *acc_input_idx;
    const int   *acc_input_wlog;
    const int	 *gflog;
    const int   *gfexp;
    // tables used by the decoder -------------------------
    const int *msgw;
    const int *vdeg;
    const int *cdeg;
    const int  *v2cmidx;
    const int  *c2vmidx;
    const int  *gfpmat;
}
qracode;

#define Q65_FASTFADING_MAXWEIGTHS 65
typedef struct
{
    const qracode *pQraCode; // qra code to be used by the codec
    float decoderEsNoMetric; // value for which we optimize the decoder metric
    int		*x;				 // codec input
    int		*y;				 // codec output
    float	*qra_v2cmsg;	 // decoder v->c messages
    float	*qra_c2vmsg;	 // decoder c->v messages
    float	*ix;			 // decoder intrinsic information
    float	*ex;			 // decoder extrinsic information
    // variables used to compute the intrinsics in the fast-fading case
    int     nBinsPerTone;
    int		nBinsPerSymbol;
    float   ffNoiseVar;
    float   ffEsNoMetric;
    int	    nWeights;
    float   ffWeight[Q65_FASTFADING_MAXWEIGTHS];
}
q65_codec_ds;

class q65subs
{
public:
    void q65_enc(int x[], int y[]);
    void q65_intrinsics_ff(float s3[], int submode, float B90Ts, int fadingModel, float s3prob[]);
    void q65_dec(float s3[], float s3prob[], int APmask[], int APsymbols[],int maxiters,float &esnodb0,
                 int xdec[], int &rc0);
    void q65_dec_fullaplist(float s3[], float s3prob[], int codewords[],
                            int ncw, float &esnodb0, int xdec[], float &plog, int &rc0);

private:
    int _q65_crc6(int *x, int sz);
    void _q65_crc12(int *y, int *x, int sz);
    float pd_norm(float *pd, int nlogdim);
    int qra_encode(const qracode *pcode, int *y, const int *x);
    int	q65_encode(const q65_codec_ds *pCodec, int *pOutputCodeword, const int *pInputMsg);
    void q65_free(q65_codec_ds *pCodec);
    int	q65_init(q65_codec_ds *pCodec, const qracode *pQraCode);
    int	q65_intrinsics_fastfading(q65_codec_ds *pCodec,
                                  float *pIntrinsics,				// intrinsic symbol probabilities output
                                  const float *pInputEnergies,	// received energies input
                                  const int submode,				// submode idx (0=A ... 4=E)
                                  const float B90Ts,				// normalized spread bandwidth (90% fractional energy)
                                  const int fadingModel);			// 0=Gaussian 1=Lorentzian fade model

    int	q65_esnodb_fastfading(
        const q65_codec_ds *pCodec,
        float		*pEsNodB,
        const int   *ydec,
        const float *pInputEnergies);
    int q65_check_llh(float *llh, const int* ydec, const int nN, const int nM, const float *pIntrin);
    int q65_decode(q65_codec_ds *pCodec, int* pDecodedCodeword, int *pDecodedMsg,
                   const float *pIntrinsics, const int *pAPMask, const int *pAPSymbols,int maxiters);
    int q65_decode_fullaplist(q65_codec_ds *codec,
                              int *ydec,
                              int *xdec,
                              const float *pIntrinsics,
                              const int *pCodewords,
                              const int nCodewords);

    int pd_argmax(float *pmax, float *src, int ndim);
    float pd_max(float *src, int ndim);
    void pd_bwdperm(float *dst, float *src, const  int *perm, int ndim);
    void pd_fwdperm(float *dst, float *src, const  int *perm, int ndim);
    void pd_imul(float *dst, const float *src, int nlogdim);
    void np_fwht(int nlogdim, float *dst, float *src);
    int qra_extrinsic(const qracode *pcode, float *pex, const float *pix, int maxiter,float *qra_v2cmsg,float *qra_c2vmsg);

    void qra_mapdecode(const qracode *pcode, int *xdec, float *pex, const float *pix);

    // helper functions
#define q65_get_message_length(pCodec)  _q65_get_message_length((pCodec)->pQraCode)
#define q65_get_codeword_length(pCodec) _q65_get_codeword_length((pCodec)->pQraCode)
#define q65_get_code_rate(pCodec)		_q65_get_code_rate((pCodec)->pQraCode)
#define q65_get_alphabet_size(pCodec)	_q65_get_alphabet_size((pCodec)->pQraCode)
#define q65_get_bits_per_symbol(pCodec) _q65_get_bits_per_symbol((pCodec)->pQraCode)

    // internally used but made public for the above defines
    int		_q65_get_message_length(const qracode *pCode);
    int		_q65_get_codeword_length(const qracode *pCode);
    float	_q65_get_code_rate(const qracode *pCode);
    void	_q65_mask(const qracode *pcode, float *ix, const int *mask, const int *x);
    int		_q65_get_alphabet_size(const qracode *pCode);
    int		_q65_get_bits_per_symbol(const qracode *pCode);




};
#endif