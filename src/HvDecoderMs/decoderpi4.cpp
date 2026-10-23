/* The algorithms, source code, look-and-feel of WSJT-X and related
 * programs, and protocol specifications for the modes FSK441, FT8, JT4,
 * JT6M, JT9, JT65, JTMS, QRA64, ISCAT, MSK144, are Copyright © 2001-2017
 * by one or more of the following authors: Joseph Taylor, K1JT; Bill
 * Somerville, G4WJS; Steven Franke, K9AN; Nico Palermo, IV3NWV; Greg Beam,
 * KI7MT; Michael Black, W9MDB; Edson Pereira, PY2SDR; Philip Karn, KA9Q;
 * and other members of the WSJT Development Group.
 *
 * MSHV PI4 Decoder Part from JT4 
 * Rewritten into C++ and modified by Hrisimir Hristov, LZ2HV 2015-2017
 * May be used under the terms of the GNU General Public License (GPL)
 */

#include "decoderms.h"
//#include <QtGui>


static const double tx_nfqsopi4 = 682.8125;//1270.46;//682.8125;//1270.46;
static const int N_SYMMAX = 2000;//2520; 2000  2743 12000->2176,8
static const int G_NSYM = 146;//207 146
//static const int N_FFTMAX = 2048;

/*static const int ch_count = 3;//7
static const int nch_pi4[ch_count]=
    {
        //36,38,40,42,44,46,48 //7
        //42,42,42,42,42,42,42 //7
        //36,38,40,42,44//,46,48 //5
        //40,40,40,40,40//,46,48 //5
        38,40,42//,46,48//5
    };*/
static const int npr2_pi4[146] =
{
    0,0,1,0,0,1,1,1,1,0,1,0,1,0,1,0,0,1,0,0,0,1,0,0,0,1,1,0,0,1,
    1,1,1,0,0,1,1,1,1,1,0,0,1,1,0,1,1,1,1,0,1,0,1,1,0,1,1,0,1,0,
    0,0,0,0,1,1,1,1,1,0,1,0,1,0,0,0,0,0,1,1,1,1,1,0,1,0,0,1,0,0,
    1,0,1,0,0,0,0,1,0,0,1,1,0,0,0,0,0,1,1,0,0,0,0,1,1,0,0,1,1,1,
    0,1,1,1,0,1,1,0,1,0,1,0,1,0,0,0,0,1,1,1,0,0,0,0,1,1
};

static const char PI4_valid_chars[38] =
    {
        '0', '1', '2', '3', '4', '5', '6', '7',
        '8', '9', 'A', 'B', 'C', 'D', 'E', 'F',
        'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N',
        'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V',
        'W', 'X', 'Y', 'Z', ' ', '/'
    };

//static const int npoly1=-221228207;
//static const int npoly2=-463389625;

#define	npoly1	0xf2d05351
#define	npoly2	0xe4613c47
static const int partab_pi4[256]=
    {
        0, 1, 1, 0, 1, 0, 0, 1,
        1, 0, 0, 1, 0, 1, 1, 0,
        1, 0, 0, 1, 0, 1, 1, 0,
        0, 1, 1, 0, 1, 0, 0, 1,
        1, 0, 0, 1, 0, 1, 1, 0,
        0, 1, 1, 0, 1, 0, 0, 1,
        0, 1, 1, 0, 1, 0, 0, 1,
        1, 0, 0, 1, 0, 1, 1, 0,
        1, 0, 0, 1, 0, 1, 1, 0,
        0, 1, 1, 0, 1, 0, 0, 1,
        0, 1, 1, 0, 1, 0, 0, 1,
        1, 0, 0, 1, 0, 1, 1, 0,
        0, 1, 1, 0, 1, 0, 0, 1,
        1, 0, 0, 1, 0, 1, 1, 0,
        1, 0, 0, 1, 0, 1, 1, 0,
        0, 1, 1, 0, 1, 0, 0, 1,
        1, 0, 0, 1, 0, 1, 1, 0,
        0, 1, 1, 0, 1, 0, 0, 1,
        0, 1, 1, 0, 1, 0, 0, 1,
        1, 0, 0, 1, 0, 1, 1, 0,
        0, 1, 1, 0, 1, 0, 0, 1,
        1, 0, 0, 1, 0, 1, 1, 0,
        1, 0, 0, 1, 0, 1, 1, 0,
        0, 1, 1, 0, 1, 0, 0, 1,
        0, 1, 1, 0, 1, 0, 0, 1,
        1, 0, 0, 1, 0, 1, 1, 0,
        1, 0, 0, 1, 0, 1, 1, 0,
        0, 1, 1, 0, 1, 0, 0, 1,
        1, 0, 0, 1, 0, 1, 1, 0,
        0, 1, 1, 0, 1, 0, 0, 1,
        0, 1, 1, 0, 1, 0, 0, 1,
        1, 0, 0, 1, 0, 1, 1, 0
    };
static const int MAXAVE_PI4 = 20; //64 max
/*
int mettab_pi4_1[2][256] =
    {
        {
            3,   3,   3,   3,   3,   3,   3,   3, // mettab[0][  0..  7]
            3,   3,   3,   3,   3,   3,   3,   3, // mettab[0][  8.. 15]
            3,   3,   3,   3,   3,   3,   3,   3, // mettab[0][ 16.. 23]
            3,   3,   3,   3,   3,   3,   3,   3, // mettab[0][ 24.. 31]
            3,   3,   3,   3,   3,   3,   3,   3, // mettab[0][ 32.. 39]
            3,   3,   3,   3,   3,   3,   3,   3, // mettab[0][ 40.. 47]
            3,   3,   3,   3,   3,   3,   3,   3, // mettab[0][ 48.. 55]
            3,   3,   3,   3,   3,   3,   3,   3, // mettab[0][ 56.. 63]
            3,   3,   3,   3,   3,   3,   3,   3, // mettab[0][ 64.. 71]
            3,   2,   2,   2,   2,   2,   2,   2, // mettab[0][ 72.. 79]
            2,   2,   2,   2,   2,   2,   2,   2, // mettab[0][ 80.. 87]
            2,   2,   2,   2,   2,   2,   2,   2, // mettab[0][ 88.. 95]
            1,   1,   1,   1,   1,   1,   1,   1, // mettab[0][ 96..103]
            1,   1,   1,   0,   0,   0,   0,   0, // mettab[0][104..111]
            0,   0,   0,  -1,  -1,  -1,  -1,  -1, // mettab[0][112..119]
            -1,  -2,  -2,  -2,  -2,  -2,  -3,  -3, // mettab[0][120..127]
            -3,  -3,  -3,  -4,  -4,  -4,  -4,  -5, // mettab[0][128..135]
            -5,  -5,  -5,  -6,  -6,  -6,  -7,  -7, // mettab[0][136..143]
            -7,  -8,  -8,  -8,  -8,  -9,  -9,  -9, // mettab[0][144..151]
            -10, -10, -11, -11, -11, -12, -12, -12, // mettab[0][152..159]
            -13, -13, -13, -14, -14, -15, -15, -15, // mettab[0][160..167]
            -16, -16, -16, -17, -17, -18, -18, -18, // mettab[0][168..175]
            -19, -19, -20, -20, -21, -21, -21, -22, // mettab[0][176..183]
            -22, -23, -23, -23, -24, -24, -25, -25, // mettab[0][184..191]
            -26, -26, -26, -27, -27, -28, -28, -29, // mettab[0][192..199]
            -29, -29, -30, -30, -31, -31, -32, -32, // mettab[0][200..207]
            -32, -33, -33, -34, -34, -35, -35, -35, // mettab[0][208..215]
            -36, -36, -37, -37, -38, -38, -39, -39, // mettab[0][216..223]
            -39, -40, -40, -41, -41, -42, -42, -42, // mettab[0][224..231]
            -43, -43, -44, -44, -45, -45, -46, -46, // mettab[0][232..239]
            -46, -47, -47, -48, -48, -49, -49, -49, // mettab[0][240..247]
            -50, -50, -51, -51, -52, -52, -53, -57  // mettab[0][248..255]
        },
        {
            -57, -53, -53, -52, -52, -51, -51, -50, // mettab[1][  0..  7]
            -50, -49, -49, -49, -48, -48, -47, -47, // mettab[1][  8.. 15]
            -46, -46, -46, -45, -45, -44, -44, -43, // mettab[1][ 16.. 23]
            -43, -42, -42, -42, -41, -41, -40, -40, // mettab[1][ 24.. 31]
            -39, -39, -39, -38, -38, -37, -37, -36, // mettab[1][ 32.. 39]
            -36, -35, -35, -35, -34, -34, -33, -33, // mettab[1][ 40.. 47]
            -32, -32, -32, -31, -31, -30, -30, -29, // mettab[1][ 48.. 55]
            -29, -29, -28, -28, -27, -27, -26, -26, // mettab[1][ 56.. 63]
            -26, -25, -25, -24, -24, -23, -23, -23, // mettab[1][ 64.. 71]
            -22, -22, -21, -21, -21, -20, -20, -19, // mettab[1][ 72.. 79]
            -19, -18, -18, -18, -17, -17, -16, -16, // mettab[1][ 80.. 87]
            -16, -15, -15, -15, -14, -14, -13, -13, // mettab[1][ 88.. 95]
            -13, -12, -12, -12, -11, -11, -11, -10, // mettab[1][ 96..103]
            -10,  -9,  -9,  -9,  -8,  -8,  -8,  -8, // mettab[1][104..111]
            -7,  -7,  -7,  -6,  -6,  -6,  -5,  -5, // mettab[1][112..119]
            -5,  -5,  -4,  -4,  -4,  -4,  -3,  -3, // mettab[1][120..127]
            -3,  -3,  -3,  -2,  -2,  -2,  -2,  -2, // mettab[1][128..135]
            -1,  -1,  -1,  -1,  -1,  -1,   0,   0, // mettab[1][136..143]
            0,   0,   0,   0,   0,   0,   1,   1, // mettab[1][144..151]
            1,   1,   1,   1,   1,   1,   1,   1, // mettab[1][152..159]
            1,   2,   2,   2,   2,   2,   2,   2, // mettab[1][160..167]
            2,   2,   2,   2,   2,   2,   2,   2, // mettab[1][168..175]
            2,   2,   2,   2,   2,   2,   2,   2, // mettab[1][176..183]
            3,   3,   3,   3,   3,   3,   3,   3, // mettab[1][184..191]
            3,   3,   3,   3,   3,   3,   3,   3, // mettab[1][192..199]
            3,   3,   3,   3,   3,   3,   3,   3, // mettab[1][200..207]
            3,   3,   3,   3,   3,   3,   3,   3, // mettab[1][208..215]
            3,   3,   3,   3,   3,   3,   3,   3, // mettab[1][216..223]
            3,   3,   3,   3,   3,   3,   3,   3, // mettab[1][224..231]
            3,   3,   3,   3,   3,   3,   3,   3, // mettab[1][232..239]
            3,   3,   3,   3,   3,   3,   3,   3, // mettab[1][240..247]
            3,   3,   3,   3,   3,   3,   3,   3  // mettab[1][248..255]
        }
    };
*/
/*
#define	ENCODE(sym,encstate){\
unsigned long _tmp;\
\
_tmp = (encstate) & npoly1;\
_tmp ^= _tmp >> 16;\
(sym) = partab_pi4[(_tmp ^ (_tmp >> 8)) & 0xff] << 1;\
_tmp = (encstate) & npoly2;\
_tmp ^= _tmp >> 16;\
(sym) |= partab_pi4[(_tmp ^ (_tmp >> 8)) & 0xff];\
}
struct node {
  unsigned long encstate;	// Encoder state of next node
  long gamma;		        // Cumulative metric to this node
  int metrics[4];		// Metrics indexed by all possible tx syms
  int tm[2];		        // Sorted metrics for current hypotheses
  int i;			// Current branch being tested
};
int fano(
	 unsigned int  *metric,	   // Final path metric (returned value)
	 unsigned int  *cycles,	   // Cycle count (returned value)
	 unsigned int  *maxnp,     // Progress before timeout (returned value)
	 unsigned char *data,	   // Decoded output data
	 unsigned char *symbols,   // Raw deinterleaved input symbols
	 unsigned int nbits,	   // Number of output bits
	 int mettab[2][256],	   // Metric table, [sent sym][rx symbol]
	 int delta,		   // Threshold adjust parameter
	 unsigned int maxcycles)   // Decoding timeout in cycles per bit
{
  struct node *nodes;		   // First node
  struct node *np;	           // Current node
  struct node *lastnode;	   // Last node
  struct node *tail;		   // First node of tail
  int t;			   // Threshold
  int  m0,m1;
  int ngamma;
  unsigned int lsym;
  unsigned int i;

  if((nodes = (struct node *)malloc((nbits+1)*sizeof(struct node))) == NULL) {
    printf("malloc failed\n");
    return 0;
  }
  lastnode = &nodes[nbits-1];
  tail = &nodes[nbits-31];
  *maxnp = 0;


  for(np=nodes;np <= lastnode;np++) {
    np->metrics[0] = mettab[0][symbols[0]] + mettab[0][symbols[1]];
    np->metrics[1] = mettab[0][symbols[0]] + mettab[1][symbols[1]];
    np->metrics[2] = mettab[1][symbols[0]] + mettab[0][symbols[1]];
    np->metrics[3] = mettab[1][symbols[0]] + mettab[1][symbols[1]];
    symbols += 2;
  }
  np = nodes;
  np->encstate = 0;

// Compute and sort branch metrics from root node
  ENCODE(lsym,np->encstate);	// 0-branch (LSB is 0)
  m0 = np->metrics[lsym];



  m1 = np->metrics[3^lsym];
  if(m0 > m1) {
    np->tm[0] = m0;                             // 0-branch has better metric
    np->tm[1] = m1;
  } else {
    np->tm[0] = m1;                             // 1-branch is better
    np->tm[1] = m0;
    np->encstate++;	                        // Set low bit
  }
  np->i = 0;	                                // Start with best branch
  maxcycles *= nbits;
  np->gamma = t = 0;

  // Start the Fano decoder
  for(i=1;i <= maxcycles;i++) {
    if((int)(np-nodes) > (int)*maxnp) *maxnp=(int)(np-nodes);
#ifdef	debug
    printf("k=%ld, g=%ld, t=%d, m[%d]=%d, maxnp=%d, encstate=%lx\n",
	   np-nodes,np->gamma,t,np->i,np->tm[np->i],*maxnp,np->encstate);
#endif

    ngamma = np->gamma + np->tm[np->i];
    if(ngamma >= t) {
      if(np->gamma < t + delta) {               // Node is acceptable

	while(ngamma >= t + delta) t += delta;
      }
      np[1].gamma = ngamma;                     // Move forward
      np[1].encstate = np->encstate << 1;
      if( ++np == (lastnode+1) ) {
	break;	                                // Done!
      }


      ENCODE(lsym,np->encstate);
      if(np >= tail) {

	np->tm[0] = np->metrics[lsym];
      } else {
	m0 = np->metrics[lsym];
	m1 = np->metrics[3^lsym];
	if(m0 > m1) {
	  np->tm[0] = m0;                       // 0-branch is better
	  np->tm[1] = m1;
	} else {
	  np->tm[0] = m1;                       // 1-branch is better
	  np->tm[1] = m0;
	  np->encstate++;	                // Set low bit
	}
      }
      np->i = 0;	                        // Start with best branch
      continue;
    }
    // Threshold violated, can't go forward
    for(;;) {                                   // Look backward
      if(np == nodes || np[-1].gamma < t) {

	t -= delta;
	if(np->i != 0) {
	  np->i = 0;
	  np->encstate ^= 1;
	}
	break;
      }
      // Back up
      if(--np < tail && np->i != 1) {
	np->i++;                          // Search next best branch
	np->encstate ^= 1;
	break;
      }                                   // else keep looking back
    }
  }
  *metric =  np->gamma;	                  // Return the final path metric

  // Copy decoded data to user's buffer
  nbits >>= 3;
  np = &nodes[7];
  while(nbits-- != 0) {
    *data++ = np->encstate;
    np += 8;
  }
  *cycles = i+1;

  free(nodes);
  if(i >= maxcycles) return -1;	          // Decoder timed out
  return 0;		                  // Successful completion
}

*/
/*
#define	ENCMAC(sym,encstate){\
	unsigned long _tmp;\
\
	_tmp = (encstate) & npoly1;\
	_tmp ^= _tmp >> 16;\
	(sym) = partab_pi4[(_tmp ^ (_tmp >> 8)) & 0xff] << 1;\
	_tmp = (encstate) & npoly2;\
	_tmp ^= _tmp >> 16;\
	(sym) |= partab_pi4[(_tmp ^ (_tmp >> 8)) & 0xff];\
}
*/
void DecoderMs::fano232(char *symbol,int beg,int nbits,int maxcycles,unsigned char *dat,int &ncycles,int &ierr)
{

    const int MAXBITS=104;//103;
    int metrics[4][MAXBITS];//(0:3,0:MAXBITS)
    int nstate[MAXBITS];// = {0};
    int tm[2][MAXBITS]; //(0:1,0:MAXBITS)
    int ii[MAXBITS];
    int gamma[MAXBITS];
    bool noback=false;
    //int metric = 0;
    int i = 0;

    int ntail=nbits-31;
    int i4a=0;
    int i4b=0;
    int np;
    zero_int_beg_end(nstate,0,MAXBITS);
    //ndelta_pi4 = 70.0;  //170 e sega

    //qDebug()<<"111111symbol[j]"<<beg<<nbits-1;

    for (np = 0; np <= (nbits-1); np++)
    {//do np=0,nbits-1
        int j=2*np;

        i4a=symbol[j+beg]+off_mettab_pi4;
        i4b=symbol[j+1+beg]+off_mettab_pi4;
        metrics[0][np] = mettab_pi4_[0][i4a] + mettab_pi4_[0][i4b];//(i4b,0)
        metrics[1][np] = mettab_pi4_[0][i4a] + mettab_pi4_[1][i4b];//(i4b,1)
        metrics[2][np] = mettab_pi4_[1][i4a] + mettab_pi4_[0][i4b];//(i4b,0)
        metrics[3][np] = mettab_pi4_[1][i4a] + mettab_pi4_[1][i4b];//(i4b,1)
        //qDebug()<<"i4a"<<mettab_pi4_[0][i4b];


        /*i4a=symbol[j+beg]+128;
        i4b=symbol[j+1+beg]+128;
        if(i4a<0) i4a=0;
        if(i4b<0) i4b=0;
        if(i4a>255) i4a=255;
        if(i4b>255) i4b=255;

        metrics[0][np] = mettab_pi4_1[0][i4a] + mettab_pi4_1[0][i4b];//(i4b,0)
        metrics[1][np] = mettab_pi4_1[0][i4a] + mettab_pi4_1[1][i4b];//(i4b,1)
        metrics[2][np] = mettab_pi4_1[1][i4a] + mettab_pi4_1[0][i4b];//(i4b,0)
        metrics[3][np] = mettab_pi4_1[1][i4a] + mettab_pi4_1[1][i4b];//(i4b,1)*/
        //qDebug()<<"iii================"<<i4a<<i4b<<np;
    }

    np=0;
    nstate[np]=0;

    //Left Shift 	ISHFT 	ISHFT(N,M) (M > 0) 	<< 	n<<m 	n shifted left by m bits
    //Right Shift 	ISHFT 	ISHFT(N,M) (M < 0) 	>> 	n>>m 	n shifted right by m bits
    // IEOR(N,M)   n^m

    int n=(nstate[np] & npoly1);
    n=(n ^ (n >> 16));
    int lsym=partab_pi4[((n ^ (n >> 8)) & 255)];
    //int lsym=partab_pi4[((n ^ (n >> 8)) & 0xff)] << 1;
    n=(nstate[np] & npoly2);
    n=(n ^ (n >> 16));
    lsym=lsym+lsym+partab_pi4[((n ^ (n >> 8)) & 255)];
    //lsym |= partab_pi4[((n ^ (n >> 8)) & 0xff)];
    //int lsym = 0;
    //int n = 0;
    //ENCMAC(nstate[np],lsym);

    int m0=metrics[lsym][np];
    int m1=metrics[(3 ^ lsym)][np];
    if (m0>m1) //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    {
        tm[0][np]=m0;
        tm[1][np]=m1;
    }
    else
    {
        tm[0][np]=m1;
        tm[1][np]=m0;
        nstate[np]=nstate[np] + 1;
    }

    ii[np]=0;
    gamma[np]=0;
    int nt=0;

    //int max = 0;

    for (i = 1; i < (nbits*maxcycles); i++)  //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    {//do i=1,nbits*maxcycles
        int ngamma=gamma[np] + tm[ii[np]][np];
        if (ngamma>=nt)
        {
            if (gamma[np]<(nt+ndelta_pi4)) nt=nt + ndelta_pi4 * ((ngamma-nt)/ndelta_pi4);
            gamma[np+1]=ngamma;
            nstate[np+1]=(nstate[np] << 1);
            np=np+1;

            //if (np>max)
            //max=np;

            if (np==nbits) goto c100;
            //if (np>=68) goto c100;

            //if(np>87 && np<89)
            //qDebug()<<"iii================"<<np<<nbits;

            n=(nstate[np] & npoly1);
            n=(n ^ (n >> 16));
            lsym=partab_pi4[((n ^ (n >> 8)) & 255)];
            //lsym=partab_pi4[((n ^ (n >> 8)) & 255)] << 1;
            n=(nstate[np] & npoly2);
            n=(n ^ (n >> 16));
            lsym=lsym+lsym+partab_pi4[((n ^ (n >> 8)) & 255)];
            //lsym |= partab_pi4[((n ^ (n >> 8)) & 255)];
            //ENCMAC(nstate[np],lsym);

            if (np>=ntail)
            {
                tm[0][np]=metrics[lsym][np];
            }
            else
            {
                m0=metrics[lsym][np];
                m1=metrics[(3 ^ lsym)][np];
                if (m0>m1)
                {
                    tm[0][np]=m0;
                    tm[1][np]=m1;
                }
                else
                {
                    tm[0][np]=m1;
                    tm[1][np]=m0;
                    nstate[np]=nstate[np] + 1;
                }
            }
            ii[np]=0;
        }
        else
        {
            while (true)
            {
                noback=false;
                if (np==0) noback=true;
                if (np>0)
                {
                    if (gamma[np-1]<nt) noback=true;
                }
                if (noback)
                {
                    nt=nt-ndelta_pi4;
                    if (ii[np]!=0)
                    {
                        ii[np]=0;
                        nstate[np]=(nstate[np] ^ 1);
                    }
                    break;
                }
                np=np-1;
                if (np<ntail && ii[np]!=1)
                {
                    ii[np]=ii[np]+1;
                    nstate[np]=(nstate[np] ^ 1);
                    break;
                }
            }
        }
    }
    i=nbits*maxcycles;

c100:
    //metric=gamma[np];

    int nbytes=(nbits+7)/8; //=13     qDebug()<<"iii================"<<nbytes;
    np=7;
    for (int j = 0; j <= nbytes-1; j++)
    {
        i4a=nstate[np];
        dat[j]=i4a;
        np=np+8;
    }
    dat[nbytes]=0;
    ncycles=i+1;
    ierr=0;
    if (i>=maxcycles*nbits) ierr=-1;


    /*if (max>=64)
    {
        if (max==73)
            qDebug()<<"famo max============================ OK OK OK ="<<max<<nbits;
        else
            qDebug()<<"famo max================ NO ="<<max<<nbits;
        //qDebug()<<dat[0]<<dat[1]<<dat[2]<<dat[3]<<dat[4]<<dat[5]<<dat[6]
        //<<dat[7]<<dat[8]<<dat[9]<<dat[10]<<dat[11]<<dat[12];
    }*/
}
void DecoderMs::interleavepi4(char *id,int beg,int ndir)
{
    int itmp[207];

    if (first_interleavepi4)
    {
        int k=-1;
        for (int i = 0; i < 256; i++)
        {
            int m=i;
            int n=(m & 1);
            n=2*n + (m/2 & 1);
            n=2*n + (m/4 & 1);
            n=2*n + (m/8 & 1);
            n=2*n + (m/16 & 1);
            n=2*n + (m/32 & 1);
            n=2*n + (m/64 & 1);
            n=2*n + (m/128 & 1);
            if (n<=G_NSYM-1) //(n<=205) jt-4->G_NSYM-2
            {
                k=k+1;
                j0_pi4[k]=n;
                //qDebug()<<k;
            }
        }
        //qDebug()<<"6==================="<<k;
        first_interleavepi4=false;
    }

    //qDebug()<<j0_pi4[0]<<j0_pi4[1]<<j0_pi4[2]<<j0_pi4[3]<<j0_pi4[4]<<j0_pi4[5]<<j0_pi4[6]<<j0_pi4[204]<<j0_pi4[205];
    if (ndir==1)
    {
        for (int i = 0; i < G_NSYM-0; i++)
        {
            itmp[j0_pi4[i]]=id[i+beg];
        }
    }
    else
    {
        for (int i = 0; i < G_NSYM-0; i++)
        {
            itmp[i]=id[j0_pi4[i]+beg];
        }
    }
    for (int i = 0; i < G_NSYM-0; i++)
    {
        id[i+beg]=itmp[i];
    }
}

void DecoderMs::getmetpi4()
{
    double xx0[256]=
        {1.000, 1.000, 1.000, 1.000, 1.000, 1.000, 1.000, 1.000,
         1.000, 1.000, 1.000, 1.000, 1.000, 1.000, 1.000, 1.000,
         1.000, 1.000, 1.000, 1.000, 1.000, 1.000, 1.000, 1.000,
         1.000, 1.000, 1.000, 1.000, 1.000, 1.000, 1.000, 1.000,
         1.000, 1.000, 1.000, 1.000, 1.000, 1.000, 1.000, 1.000,
         1.000, 1.000, 1.000, 1.000, 1.000, 1.000, 1.000, 1.000,
         0.988, 1.000, 0.991, 0.993, 1.000, 0.995, 1.000, 0.991,
         1.000, 0.991, 0.992, 0.991, 0.990, 0.990, 0.992, 0.996,
         0.990, 0.994, 0.993, 0.991, 0.992, 0.989, 0.991, 0.987,
         0.985, 0.989, 0.984, 0.983, 0.979, 0.977, 0.971, 0.975,
         0.974, 0.970, 0.970, 0.970, 0.967, 0.962, 0.960, 0.957,
         0.956, 0.953, 0.942, 0.946, 0.937, 0.933, 0.929, 0.920,
         0.917, 0.911, 0.903, 0.895, 0.884, 0.877, 0.869, 0.858,
         0.846, 0.834, 0.821, 0.806, 0.790, 0.775, 0.755, 0.737,
         0.713, 0.691, 0.667, 0.640, 0.612, 0.581, 0.548, 0.510,
         0.472, 0.425, 0.378, 0.328, 0.274, 0.212, 0.146, 0.075,
         0.000,-0.079,-0.163,-0.249,-0.338,-0.425,-0.514,-0.606,
         -0.706,-0.796,-0.895,-0.987,-1.084,-1.181,-1.280,-1.376,
         -1.473,-1.587,-1.678,-1.790,-1.882,-1.992,-2.096,-2.201,
         -2.301,-2.411,-2.531,-2.608,-2.690,-2.829,-2.939,-3.058,
         -3.164,-3.212,-3.377,-3.463,-3.550,-3.768,-3.677,-3.975,
         -4.062,-4.098,-4.186,-4.261,-4.472,-4.621,-4.623,-4.608,
         -4.822,-4.870,-4.652,-4.954,-5.108,-5.377,-5.544,-5.995,
         -5.632,-5.826,-6.304,-6.002,-6.559,-6.369,-6.658,-7.016,
         -6.184,-7.332,-6.534,-6.152,-6.113,-6.288,-6.426,-6.313,
         -9.966,-6.371,-9.966,-7.055,-9.966,-6.629,-6.313,-9.966,
         -5.858,-9.966,-9.966,-9.966,-9.966,-9.966,-9.966,-9.966,
         -9.966,-9.966,-9.966,-9.966,-9.966,-9.966,-9.966,-9.966,
         -9.966,-9.966,-9.966,-9.966,-9.966,-9.966,-9.966,-9.966,
         -9.966,-9.966,-9.966,-9.966,-9.966,-9.966,-9.966,-9.966,
         -9.966,-9.966,-9.966,-9.966,-9.966,-9.966,-9.966,-9.966,
         -9.966,-9.966,-9.966,-9.966,-9.966,-9.966,-9.966,-9.966};

    double bias=0.45;//0.5
    double scale=50.0;//be6e 50  tested 47
    ndelta_pi4=int(3.4*scale);
    for (int i = 0; i < 256; i++)
    {
        double xx=xx0[i];
        if (i>=160) xx=xx0[160] - ((double)i-160.0)*6.822/65.3;//0.1666666;//
        mettab_pi4_[0][(i-128)+off_mettab_pi4]=int(scale*(xx-bias));
        if (i>=1) mettab_pi4_[1][(128-i)+off_mettab_pi4]=mettab_pi4_[0][(i-128)+off_mettab_pi4];
    }
    mettab_pi4_[1][-128+off_mettab_pi4]=mettab_pi4_[1][-127+off_mettab_pi4];

    /*double bias=0.45;//0.5
    double scale=10.0;//be6e 50  tested 47
    ndelta_pi4=50.0;
    for (int i = 0; i < 256; i++)
    {
        double xx=xx0[i];
        mettab_pi4_[0][(i-128)+off_mettab_pi4]=int(scale*(xx-bias));
        if (i>=1) mettab_pi4_[1][(128-i)+off_mettab_pi4]=mettab_pi4_[0][(i-128)+off_mettab_pi4];
    }*/

    //for (int i = 248; i < 256; i++)
    //qDebug()<<mettab_pi4_[0][i];
}

void DecoderMs::extractpi4(double *sym0,int &ncount,QString &decoded)
{
    double sym[207];
    char symbol[207];
    unsigned char data1[13]={0}; //unsigned char unsigned char can be 0,..255 and char can be -128 , .. +127
    pomAll.zero_double_beg_end(sym,0,207);
    //zero_int_beg_end(symbol,0,207);
    for (int i = 0; i < G_NSYM; i++)
        symbol[i]=0;

    if (first_extractpi4)
    {
        getmetpi4();
        first_extractpi4=false;
    }

    double amp=30.0;// be6e 30.0
    int limit=20000;//be6e 10000

    double ave0=0.0;
    for (int i = 0; i < G_NSYM; i++)
        ave0 += sym0[i];
    ave0 = ave0 / (double)G_NSYM;
    for (int i = 0; i < G_NSYM; i++)
        sym[i]=sym0[i]-ave0;
    double sq=dot_product_da_da(sym,sym,G_NSYM,0);
    double rms0=sqrt(sq/(double)((double)G_NSYM-0.0));
    if(rms0==0.0)// no div by zero
    	rms0=1.0;
    for (int i = 0; i < G_NSYM; i++)
        sym[i]=sym[i]/rms0;

    //int mins = 0.0;
    //int maxs = 0.0;
    for (int j = 0; j < G_NSYM; j++)
    {//do j=1,207
        int n=int(amp*sym[j]);
        if (n<-128) n=-128;
        if (n>127) n=127;
        symbol[j]=n;
        //qDebug()<<"symbol[j]"<<symbol[j];

        /*if(mins>symbol[j])
           mins=symbol[j];
        if(maxs<symbol[j])
           maxs=symbol[j];*/
    }
    //qDebug()<<"minsminsmins="<<mins<<maxs;
    /*QString sss;
          for (int i = 0; i <146; i++)
          {
              //ss1_[i][j]=0.0;
              //sss.append(QString("%1").arg(sym[i],0,'f',2));
              sss.append(QString("%1").arg((int)symbol[i]));
              sss.append(",");
          }
          qDebug()<<"1sss="<<sss;*/


    int nbits=42; //72  42
    int ncycles=0;
    ncount=-1;
    decoded="";

    interleavepi4(symbol,0,-1);

    fano232(symbol,0,nbits+31,limit,data1,ncycles,ncount);

    /*unsigned int  *metric = new unsigned int[1000];
    unsigned int  *cycles = new unsigned int[1000];
    unsigned int  *maxnp = new unsigned int[1000];
    unsigned int maxcycles = 10000;
    unsigned char symbolss[146];
    for (int j = 0; j < G_NSYM; j++)
    	{
    	int i4a = symbol[j]+128;
    	if(i4a<0) i4a=0;
        if(i4a>255) i4a=255;
        symbolss[j]=i4a;
        }
      ncount = fano(metric,cycles,maxnp,data1,symbolss,nbits+31,mettab_pi4_1,ndelta_pi4,maxcycles); */

    //qDebug()<<"ncount"<<ncount<<"data1"<<data1[0]<<data1[1]<<data1[2]<<data1[3]<<data1[4]<<data1[5]<<data1[6]
    //<<data1[7]<<data1[8]<<data1[9]<<data1[10]<<data1[11]<<data1[12];
    //PI4 120300 6 -17 dB 1.1 s -36 Hz 32 W Freq 646 Hz > 00000000
    if (ncount>=0)//0=ok  "-1=no ok go to avg"  "-2no real coll no go in avg"
    {
        long long int data_val = (long long int)data1[0] << 34;
        data_val |= (long long int)data1[1] << 26;
        data_val |= (long long int)data1[2] << 18;
        data_val |= (long long int)data1[3] << 10;
        data_val |= (long long int)data1[4] << 2;
        data_val |= (long long int)data1[5] >> 6;

        for (int i = 7; i >= 0; i--)
        {
            //currentResult_.dataGram[i] = PI4_valid_chars[data_val % 38];
            decoded.prepend(PI4_valid_chars[data_val % 38]);
            data_val /= 38;
        }
        //qDebug()<<"DDDDDDDDDD"<<decoded;
        if (decoded.mid(0,8)=="00000000" || decoded.mid(0,8)=="AWDA5SEY"/* || decoded.mid(0,8)=="PTGIVNNM"*/)//00000000
        {
            decoded="";
            ncount=-2;
        }
    }
}
/*void sq_filter(double *d,int d_c)
{


}*/
void DecoderMs::decodepi4(double *dat,int npts,double dtx,double nfreq/*,double flip*/,int mode4,
                          QString &decoded,int &nfano)
{
    double dt=2.0/DEC_SAMPLE_RATE; //2.0/DEC_SAMPLE_RATE;
    //double df=DEC_SAMPLE_RATE/N_SYMMAX;
    double df=DEC_SAMPLE_RATE/2048.0;
    //double df=0.5*DEC_SAMPLE_RATE/1024.0;
    //  double DEC_SAMPLE_RATE/2048.0;//ok tested hv
    int nsym=G_NSYM-1;//206;
    double amp=15.0;//15
    int istart=int((dtx+0.0)/dt);
    //qDebug()<<"1istart"<<istart;
    //                                       if (istart<0) istart=0;
    //istart=13969;
    //qDebug()<<"2istart"<<istart;
    int nchips=0;

    //ichbest=-1;
    double complex c0,cz0,c1,cz1;
    int k=istart;
    double phi0=0.0;
    double phi1=0.0;
    double sym[207];
    //double sqall[40];

    //short *ddd = new short[180000];
    //int c_ddd =0;
    /*double complex *cx = new double complex[npts];
    int NMAX=512*1024;
    double *s = new double[NMAX];
    analytic(dat,0,npts,25,s,cx); 
    for (int i = 0; i < npts; i++)
    	cx[i]=cx[i]*1000.0;*/

    //short *ddd = new short[180000];
    //int c_ddd =0;


    //ddd[c_ddd]=(creal(c0))*500.0;
    //c_ddd++;
    //emit EmitDataToSaveInFile(ddd,c_ddd,"LZ2HV_LZ2HV_PI4_170508_000000");


    //for (int ich = ich1_pi4; ich <= ich2_pi4; ich++)
    //for (int ich = 1; ich <= 1; ich++)
    //{
    //qDebug()<<"ichichichichichichich"<<ich<<nch_pi4[ich];
    nchips=mode4;
    int nspchip=((double)(N_SYMMAX/2)/(double)nchips);

    k=istart; //qDebug()<<"istart"<<istart;
    phi0=0.0;
    phi1=0.0;
    //double fac2=1.e-8 * sqrt((double)mode4);
    double fac2=0.0001 * sqrt((double)mode4);
    //double f0,f1;//,f2,f3;
    //double dphi0,dphi1;//,dphi2,dphi3;
    //double sq0,sq1;//,sq2,sq3;
    //double sq01,sq11;


    for (int j = 0; j < nsym+1; j++) //dtx+0.0)/dt
    {
        double f0=(nfreq) + (double)(npr2_pi4[j]*mode4)*df;
        double f1=(nfreq) + (double)((2+npr2_pi4[j])*mode4)*df;

        double dphi0=twopi*dt*f0;
        double dphi1=twopi*dt*f1;
        double sq0=0.0;
        double sq1=0.0;

        for (int nc = 0; nc < nchips; nc++)
        {
            c0=0.0+0.0*I;
            c1=0.0+0.0*I;
            for (int i = 0; i < nspchip; i++)
            {
                //if (phi > twopi) phi -= twopi;
                phi0=phi0+dphi0;
                phi1=phi1+dphi1;
                cz0=(cos(phi0)-sin(phi0)*I);
                cz1=(cos(phi1)-sin(phi1)*I);

                if (k<npts)
                {
                    if (k>=0)// hv i -dt
                    {
                        c0 += cz0*dat[k];
                        c1 += cz1*dat[k];

                        //ddd[c_ddd]=(short)1000.0*(dat[k]*sin(phi));
                        //ddd[c_ddd]=(short)10.0*(cimag(cz1)*200.0);
                        //ddd[c_ddd]=(short)1000.0*(sin(phi1));
                        //ddd[c_ddd]=(short)1000.0*dat[k];
                        //ddd[c_ddd]=0.1*(cabs(c1)-cabs(c0));
                        //ddd[c_ddd]=(creal(c0))*500.0;
                        //c_ddd++;
                    }
                }
                k++;
            }
            sq0 += pomAll.ps_hv(c0);//0.01;0.0039
            sq1 += pomAll.ps_hv(c1);//0.01;
        }
        //sq0=fac2*sq0;
        //sq1=fac2*sq1;
        //sq0=sq0;
        //sq1=sq1;
        sq0=sqrt(fac2*sq0);//*0.01;
        sq1=sqrt(fac2*sq1);//*0.01;

        double rsym=amp*(sq1-sq0);//a[j]=(fmax(ss1,ss3) - fmax(ss0,ss2))/sqrt(wsum);

        if (j>=0) // if(j>=1)
        {
            rsymbol_pi4_[j]=rsym;
            sym[j]=rsym;
        }
    }
    int ncount=-1;

    //emit EmitDataToSaveInFile(ddd,c_ddd,"LZ2HV_LZ2HV_PI4_170508_000000");
    //qDebug()<<"kkkk"<<c_ddd;

    /*QString sss;
    for (int i = 0; i <146; i++)
    {
        //ss1_[i][j]=0.0;
        sss.append(QString("%1").arg(sym[i],0,'f',0));
        //sss.append(QString("%1").arg(s_sh65[j]));
        sss.append(",");
    }
    qDebug()<<"1sss="<<sss;*/

    extractpi4(sym,ncount,decoded);

    /*double sss[146]={2,0,1,0,0,3,3,3,3,2,3,2,1,2,1,2,0,3,2,2,0,3,2,2,0,1,1,0,0,1,3,1,3,0,2,1,1,3,3,1,
                     2,0,1,3,2,1,3,3,3,2,1,2,3,1,2,1,1,0,3,2,0,2,0,0,1,3,3,1,3,2,3,2,3,0,2,0,0,2,1,3,
                     3,3,1,2,3,0,0,3,0,2,3,2,1,0,2,0,2,1,0,0,1,1,0,2,0,2,2,3,3,2,2,2,2,3,1,0,0,1,3,3,
                     0,1,3,1,2,1,3,0,3,0,3,0,1,2,2,0,2,3,1,3,2,0,0,2,1,1};
    extractpi4(sss,ncount,decoded);*/

    nfano=0; //ncount ->0=ok  "-1=no ok go to avg"  "-2no real coll no go in avg"
    if (ncount>=0)
    {
        nfano=1; //nfano -> 1=ok  "0=no ok go to avg"  "-1no real coll no go in avg"
    }
    else if (ncount==-2)
    {
        nfano=-1;
    }

    //nfano = ncount;

    //qual=0.0;
    //!     if(ndepth.ge.1) then
    /*if(iand(ndepth,32).eq.32) then
       call deep4(sym(2),neme,flip,mycall,hiscall,hisgrid,deepmsg,qual)
       if(qual.gt.qbest) then
          qbest=qual
          deepbest=deepmsg
          ichbest=ich
       endif
    endif*/
    //}
    //delete cx;
    //delete s;
    /*if (qbest>qtop)
    {
        qtop=qbest;
    }*/
    //qual=qbest;
//c3:
    //return;
}
void DecoderMs::xcorpi4(double s2_[770][1260],int ipk,int nsteps,int nsym,int lag1,int lag2,int mode4,
                        double *ccf,double &ccf0,int &lagpk,double &flip)
{
    const int NSMAX=770;   //525;
    double a[NSMAX];
    int lagmin = 0;
    //zero_double_beg_end(a,0,770);

    if (first_xcorpi4)
    {
        for (int i = 0; i < nsym; i++)
        {
            pr2_pi4[i]=2.0*npr2_pi4[i]-1;
        }
        first_xcorpi4=false;
    }
    double ccfmax=0.0;
    double ccfmin=0.0;
    double nw=mode4;
    //qDebug()<<"1111ipkkkkkkkkkkkkkkkkkkkkkkk"<<ipk;
    for (int j = 0; j < nsteps; j++)
    {
        int n=2*mode4;
        if (mode4==1)
        {
            //a[j]=fmax(s2(ipk+n,j),s2(ipk+3*n,j)) - fmax(s2(ipk  ,j),s2(ipk+2*n,j))
            a[j]=fmax(s2_[j][ipk+n],s2_[j][ipk+3*n]) - fmax(s2_[j][ipk],s2_[j][ipk+2*n]);
        }
        else
        {

            int kz=fmax(1,nw/2);
            double ss0=0.0;
            double ss1=0.0;
            double ss2=0.0;
            double ss3=0.0;
            double wsum=0.0001;// hv vazno
            int beg = -kz+1;
            if (beg<0) beg = 0;//hv inportent 1.40
            for (int k = beg; k < kz-0; k++)// problem is -> kz-1
            {
                //qDebug()<<"222ipk"<<ipk+k<<k;
                double w=double(kz-abs(k))/(double)nw;
                wsum=wsum+w;
                ss0=ss0 + w*s2_[j][ipk    +k];
                ss1=ss1 + w*s2_[j][ipk+  n+k];
                ss2=ss2 + w*s2_[j][ipk+2*n+k];
                ss3=ss3 + w*s2_[j][ipk+3*n+k];
            }
            double div = sqrt(wsum);
            if(div==0.0)
            	div=1.0;
            a[j]=(fmax(ss1,ss3) - fmax(ss0,ss2))/div;
        }

    }
    //qDebug()<<"222ipkkkkkkkkkkkkkkkkkkkkkkk"<<lag1<<lag2<<nsteps;
    for (int lag = lag1; lag < lag2; lag++)
    {
        double x=0.0;
        for (int i = 0; i < nsym; i++)
        {
            int j=2*i-0+lag;
            if (j>=0 && j<nsteps) x=x+a[j]*pr2_pi4[i]; // *0.01  if(j>=1 && j<=nsteps)
        }

        ccf[lag]=2.0*x;
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
        {
            ccf[lag]=-ccf[lag];
        }
        lagpk=lagmin;
        ccf0=-ccfmin;
        flip=-1.0;
    }
}
void DecoderMs::smo_pi4(double*x,int x_begin,int npts,double*y,double nadd)
{
    int nh=nadd/2;
    for (int i = 1+nh; i < npts-nh; i++)
    {
        double sum=0.0;
        for (int j = -nh; j < nh; j++)
        {
            sum=sum + x[i+(j+x_begin)];
        }
        y[i]=sum;
    }
    for (int i = 0; i < npts; i++)
        x[i+x_begin]=y[i];

    pomAll.zero_double_beg_end(x,0,nh);
    pomAll.zero_double_beg_end(x,npts-nh+0,npts);
}
void DecoderMs::flat1b(double *psavg,int nsmo,double s2_[770][1260],int nh,int nsteps)
{
    double x[8192];
    double *temp = new double[nsmo];//double temp[nsmo];

    int ia=nsmo/2 + 1;
    int ib=nh - nsmo/2 - 1;
    for (int i = ia; i < ib; i++)
    {

        for (int z = 0; z < nsmo; z++)
            temp[z]=psavg[(i-nsmo/2)+z];
        x[i] = pomAll.pctile_shell(temp,nsmo,50);
    }
    delete [] temp;
    for (int i = 0; i < ia; i++)
    {
        x[i]=x[ia];
    }
    for (int i = ib; i < nh; i++)
    {
        x[i]=x[ib-1];
    }
    for (int i = 0; i < nh; i++)
    {
    	double div = x[i];
    	if(div==0.0)
    		div=1.0;
        psavg[i]=psavg[i]/div;
        for (int j = 0; j < nsteps; j++)
        {
            s2_[j][i]=s2_[j][i]/div;
        }
    }
}
void DecoderMs::pspi4(double *dat,int beg,int nfft,double *s)
{
    const int NMAX=N_SYMMAX+50;
    double dat2[NMAX+10];
    double complex c[NMAX+10];

    int nh=nfft/2;   //qDebug()<<"jjjjjjjjjjj"<<beg<<nh+beg;
    for (int i = 0; i < nh; i++)
    {
        dat2[i]=dat[i+beg]/128.0;
    }
    for (int i = nh; i < nfft; i++)
    {
        dat2[i]=0.0;
    }

    f2a.four2a_d2c(c,dat2,nfft,-1,0);
    //qDebug()<<"sss="<<beg<<nh+beg;
    double fac=1.0/(double)nfft;
    for (int i = 0; i < nh; i++)
    {
        s[i]=fac*pomAll.ps_hv(c[i]);
    }
}
void DecoderMs::syncpi4(double *dat,int jz,int ntol,int NFreeze,int MouseDF,int mode,int mode4,
                        double &dtx,double &dfx,double &snrx,double &snrsync,double *ccfblue,double *ccfred1,
                        double &flip,double &width/*,double *ps0*/)
{
    //jz = fmin(180000 ,jz);
    const int NFFTMAX=N_SYMMAX;//2520;
    int nsym = G_NSYM;//207; 146

    const int NHMAX=NFFTMAX/2; // 1260
    int NSMAX=770; //for 12000hz=751 no-525;

    double ccfred_p[900+40];
    double *ccfred = &ccfred_p[450+20];
    //double red_p[900+40];
    //double *red = &red_p[450+20];

    double psavg[NHMAX];
    //double tmp[NHMAX];
    //double s2_[NSMAX][NHMAX];
    double (*s2_)[1260]=new double[NSMAX][1260];

    int nfft=NFFTMAX;//NFFTMAX;
    int nh=nfft/2;
    int nq=nfft/4;
    int nsteps=jz/nq - 0; //jz/nq - 1;
    double df=0.5*DEC_SAMPLE_RATE/2048.0;//0.5
    int lagpk0 = 0;
    int lagpk = 0;
    double ccf0 = 0.0;

    /*for (int i = 0; i < NSMAX; i++)
        {
            for (int j = 0; j < 1200; j++)   // row major = i
                s2_[i][j]=0.0;
    }*/



    pomAll.zero_double_beg_end(psavg,0,nh);
    if (mode==-999) width=0.0;
    //qDebug()<<"dat c="<<nsteps<<nh;
    for (int j = 0; j < nsteps; j++)
    {
        int k=(j-0)*nq + 0;//(j-1)*nq + 1
        //qDebug()<<"j="<<j;
        pspi4(dat,k,nfft,s2_[j]);
        for (int i = 0; i < nh; i++)
            psavg[i]=psavg[i] + s2_[j][i];
    }

    int nsmo=fmin(10*mode4,150);//40
    flat1b(psavg,nsmo,s2_,nh,nsteps);

    /*QString sss;
    for (int i = 0; i < nh; i++)
    {
        //for (int j = 0; j < 1260; j++)
        	//{
        //sss.append(QString("%1").arg(s2_[i][j],0,'f',2));
        //sss.append(",");
        //}
        //sss.append("#");
        sss.append(QString("%1").arg(psavg[i],0,'f',2));
        sss.append(",");
        
    }
    qDebug()<<"1sss="<<sss;*/

    if (mode4>=9)
    {
    	double tmp[NHMAX];
        smo_pi4(psavg,0,nh,tmp,(mode4/4)); /*qDebug()<<"mode4="<<mode4;*/
    }

    int i0=132;

    double famin=200.0 + 3.0*(double)mode4*df;
    double fbmax=2700.0 - 3.0*(double)mode4*df;

    double fa=famin;
    double fb=fbmax;
    if (NFreeze==1)
    {
        fa=fmax(famin,tx_nfqsopi4+MouseDF-ntol);
        fb=fmin(fbmax,tx_nfqsopi4+MouseDF+ntol);
    }
    else
    {
        fa=fmax(famin,tx_nfqsopi4+MouseDF-600);
        fb=fmin(fbmax,tx_nfqsopi4+MouseDF+600);
    }

    int ia=fa/df - 3.0*(double)mode4;
    int ib=fb/df - 3.0*(double)mode4;

    i0=int(tx_nfqsopi4/df);
    int irange=450;
    if (ia-i0<-irange) ia=i0-irange;
    if (ib-i0>irange)  ib=i0+irange;

    //qDebug()<<"ffffttttttttttttttttttttttttttt="<<fa<<fb;

    int lag1=-5;//-5;
    int lag2=79;//2.16=79 -3s to +3.5sec    old 59; 
    double syncbest=-1.e30;
    pomAll.zero_double_beg_end(ccfred,-460,460);
    int jmax=-1000;
    int jmin=1000;
    int ipk=0;

    int kz=mode4/2;
    //bool savered=false;
    //qDebug()<<"ffff="<<ia+kz<<ib-kz<<ia<<ib<<kz;

    for (int i = ia+kz; i < ib-kz; i++)// min 100hz df
    {
        xcorpi4(s2_,i,nsteps,nsym,lag1,lag2,mode4,ccfblue,ccf0,lagpk0,flip);
        //qDebug()<<"iiiiiiii="<<ccf0<<lagpk0<<flip;
        int j=i-i0 + 3*mode4;
        if (j>=-372 && j<=372)
        {
            ccfred[j]=ccf0;
            jmax=fmax(j,jmax);
            jmin=fmin(j,jmin);
        }
        //            qDebug()<<"ffff="<<ccfblue[lagpk0];
        slope(ccfblue,lag1,lag2-lag1+0,lagpk0-lag1+0.0);
        double sync=fabs(ccfblue[lagpk0]);

        //qDebug()<<"ffff="<<sync<<lagpk0;
        if (flip<0) continue;
        if (sync>syncbest)
        {
            ipk=i;
            lagpk=lagpk0;
            //ichpk=ich;
            syncbest=sync;
            //savered=true;
            //double pppp=((double)(ipk-i0) + 3.0*(double)mode4)*df;
            //qDebug()<<"iiiiiiiiiiiiiiiiiiiiiiiiii="<<(pppp + tx_nfqsopi4 - 1.5*4.375*mode4)<<sync<<ich;
        }
    }
    /*if (savered)
    {
        for (int x = -460; x < 460; x++)
            red[x]=ccfred[x];
    }
    //qDebug()<<"iiiiiiiiiiiiiiiiiiiiiiiiii="<<ichpk;
    for (int x = -460; x < 460; x++)
        ccfred[x]=red[x];    //ccfred=red*/

    //!  width=df*nch(ichpk)
    dfx=((double)(ipk-i0) + 3.0*(double)mode4)*df; /// mahane 5

    //qDebug()<<"111iiiiiiiiiiiiiiiiiiiiiiiiii="<<dfx<<ipk<<lag1<<lag2<<nsteps<<nsym;
    double ccfmax = 0.0;

    //zero_double_beg_end(ccfblue,-10,550);
    //qDebug()<<"222iiiiiiiccf10="<<ipk<<ichpk;
    //ichpk = 2;
    xcorpi4(s2_,ipk,nsteps,nsym,lag1,lag2,mode4,ccfblue,ccfmax,lagpk,flip);
    //ccfmax = 0.75347;
    //qDebug()<<"222iiiiiiiiiiiiiiiiiiiiiiiiii="<<flip;
    double xlag=lagpk;

    if (lagpk>lag1 && lagpk<lag2)
    {
        double dx2 = pomAll.peakup(ccfblue[lagpk-1],ccfmax,ccfblue[lagpk+1]);
        xlag=lagpk+dx2;
    }
    //qDebug()<<"222iiiiiiiccf10="<<xlag<<lag1<<lag2<<lagpk<<ccfmax;
    slope(ccfblue,lag1,lag2-lag1+0,xlag-lag1+0.0);
    double sq=0.0;
    double nsq=0.0;
    for (int lag = lag1; lag < lag2; lag++)
    {
        if (fabs(lag-xlag)>2.0)//2.0 pri men 20.0
        {
            sq += ccfblue[lag]*ccfblue[lag];
            nsq=nsq+1.0;
            //qDebug()<<"222iiiiiiiccf10="<<lag<<fabs(lag-xlag);
        }
    }
    if(nsq==0.0)
    	nsq=1.0;
    double rms=sqrt(sq/nsq);
    if(rms==0.0)
    	rms=1.0;
    snrsync=fmax(0.0,pomAll.db(fabs(ccfblue[lagpk]/rms - 1.0)) - 4.5);

    double snrx1=fabs(ccfblue[lagpk]/rms - 1.0) - 30.0;// + snrsync;
    //double snrx1=fabs(ccfblue[lagpk]/rms - 1.0)*1.5-35.0;//*2.0-50.0
    //double snrx1=fabs(ccfblue[lagpk]/rms - 1.0)-20.0;
    //double snrx1=db(fabs(ccfred[lagpk])) ;// + snrsync;
    //snrx=db(fabs(ccfblue[lagpk]/rms)) - 36.0 + snrsync;
    double snrx2=pomAll.db(snrsync) - 26.0;
    //qDebug()<<"1ssssssssssss="<<snrx1<<snrx2;
    if (snrx1<snrx2)
        snrx=snrx2;
    else
        snrx=snrx1;

    ////old method ///////////
    /*snrx=-21.0;
    //36,38,40,42,44,46,72
    if (mode4==36)  snrx=-21.0;
    if (mode4==38)  snrx=-21.0;
    if (mode4==40)  snrx=-21.0;
    if (mode4==42) snrx=-20.0;
    if (mode4==44) snrx=-20.0;
    if (mode4==46) snrx=-20.0;
    //if(mode4==2)  snrx=-25.
    //if(mode4==4)  snrx=-24.
    //if(mode4==9)  snrx=-23.
    //if(mode4==18) snrx=-22.
    //if(mode4==36) snrx=-21.
    //if(mode4==72) snrx=-20.
    //snrx=snrx + snrsync;
    ////end old method ///////////*/

    double dt=2.0/DEC_SAMPLE_RATE; //???? 2.0/DEC_SAMPLE_RATE
    double istart=xlag*(double)nq;//xlag*2048.0/11025.0;
    dtx=istart*dt;//3.3

    //dtx=xlag*2048.0/12000.0;

    pomAll.zero_double_beg_end(ccfred1,-230,230);//ccfred1=0.  ve4e e napraveno
    jmin=fmax(jmin,-224);
    jmax=fmin(jmax,224);
    for (int i = jmin; i < jmax; i++)
    {
        ccfred1[i]=ccfred[i];
    }

    int ipk1 = pomAll.maxloc_da_beg_to_end(ccfred1,-224,224);//-225;//
    //qDebug()<<"222iiiiiiiccf10="<<ipk1;
    double ns=0.0;
    double s=0.0;
    double iw=fmin(mode4,(ib-ia)/4);
    for (int i = jmin; i < jmax; i++)
    {
        if (fabs(i-ipk1)>iw)
        {
            s=s+ccfred1[i];
            ns=ns+1.0;
        }
    }
    double base=s/ns;
    for (int i = -230; i < 230; i++)
        ccfred1[i]=ccfred1[i]-base;

    double ccf10=0.5*pomAll.maxval_da_beg_to_end(ccfred1,-224,224);//
    //qDebug()<<"222iiiiiiiccf10="<<ccfred1[ipk1]<<ccf10;
    int i = 0;
    for (i = ipk1; i >= jmin; i--)
    {
        if (ccfred1[i]<=ccf10) break;
    }
    int i1=i;
    for (i = ipk1; i < jmax; i++)
    {
        if (ccfred1[i]<=ccf10) break;
    }
    //width=(i-i1)*df/5.52;
    width=((i-i1)/4.0)*df-2.197265;

    //ia=fa/df - 3.0*(double)mode4;
    //qDebug()<<"222iiiiiiiiiiiiiiiiiiiiiiiiii="<<i<<i1<<df<<jmin<<jmax;

    delete [] s2_;
}


void DecoderMs::avgpi4(int nutc,double snrsync,double dtxx,double flip,double nfreq,int ntol,
                       QString &avemsg/*,double &qave*/,int &nfanoave)
{
    double sym_[207];//(207,7)
    //double syncave;
    //double dtave;
    //double fave;
    int ncount = 0;

    if (first_avgpi4)
    {
        for (int i = 0; i < MAXAVE_PI4; i++)
        {
            iutc_pi4[i]=-1;
            nfsave_pi4[i]=0.0;
        }
        dtdiff_pi4=0.2;
        first_avgpi4=false;
        nsave_pi4 = 0;
    }

    for (int i = 0; i < MAXAVE_PI4; i++)
    {
        if (iutc_pi4[i]<0) break;
        if (nutc==iutc_pi4[i] && fabs(nfreq-nfsave_pi4[i])<=ntol) goto c10;
    }

    if (iutc_pi4[nsave_pi4] < 0)
        count_saved_avgs_pi4++;

    iutc_pi4[nsave_pi4]=nutc;
    syncsave_pi4[nsave_pi4]=snrsync;
    dtsave_pi4[nsave_pi4]=dtxx;
    nfsave_pi4[nsave_pi4]=nfreq;
    flipsave_pi4[nsave_pi4]=flip;

    for (int j = 0; j < G_NSYM; j++)
        ppsave_pi4_[nsave_pi4][j]=rsymbol_pi4_[j];

    nsave_pi4++;

c10:


    for (int j = 0; j < G_NSYM; j++)
        sym_[j]=0.0;

    double syncsum=0.0;
    double dtsum=0.0;
    double nfsum=0.0;
    int nsum=0;

    for (int i = 0; i < MAXAVE_PI4; i++)
    {
        cused_pi4[i]='.';
        if (iutc_pi4[i]<0) continue;
        //if(mod(iutc_pi4(i),2).ne.mod(nutc_pi4,2)) cycle  !Use only same sequence
        if (fabs(dtxx-dtsave_pi4[i])>dtdiff_pi4) continue;//cycle  !DT must match
        if (fabs(nfreq-nfsave_pi4[i])>ntol) continue;//cycle   !Freq must match
        if (flip!=flipsave_pi4[i]) continue;// cycle            //!Sync (* / #) must match

        for (int x = 0; x < G_NSYM; x++)
            sym_[x]=sym_[x] +  ppsave_pi4_[i][x];


        syncsum=syncsum + syncsave_pi4[i];
        dtsum=dtsum + dtsave_pi4[i];
        nfsum=nfsum + nfsave_pi4[i];
        cused_pi4[i]='$';
        iused_pi4[nsum]=i;
        nsum++;
    }
    if (nsum<MAXAVE_PI4-0) iused_pi4[nsum+0]=0;


    if (nsum<2)//2
    {
        emit EmitAvgSavesPi4(count_use_avgs_pi4,count_saved_avgs_pi4);
        goto c900; //ne pomalko ot 1 da ima if(nsum<2) go to 900
    }

    count_use_avgs_pi4 = nsum;
    emit EmitAvgSavesPi4(count_use_avgs_pi4,count_saved_avgs_pi4);
    //qDebug()<<"AVG TRAY nsum used============"<<nsum;

    //syncave=0.0;
    //dtave=0.0;
    //fave=0.0;
    if (nsum>0)
    {
        for (int x = 0; x < G_NSYM; x++)
            sym_[x]=sym_[x]/(double)nsum;
        //syncave=syncsum/nsum;
        //dtave=dtsum/nsum;
        //fave=(double)nfsum/nsum;
    }

    /*do i=1,nsave
        csync='*'
        if(flipsave(i).lt.0.0) csync='#'
        if (associated (this%average_callback)) then
           call this%average_callback(cused(i) .eq. '$',iutc(i),               &
                syncsave(i),dtsave(i),nfsave(i),flipsave(i).lt.0.)
        end if
     enddo

     sqt=0.
     sqf=0.
     do j=1,64
        i=iused(j)
        if(i.eq.0) exit
        csync='*'
        if(flipsave(i).lt.0.0) csync='#'
        sqt=sqt + (dtsave(i)-dtave)**2
        sqf=sqf + (nfsave(i)-fave)**2
     enddo
     rmst=0.
     rmsf=0.
     if(nsum.ge.2) then
        rmst=sqrt(sqt/(nsum-1))
        rmsf=sqrt(sqf/(nsum-1))
     endif
     kbest=ich1*/

    //qDebug()<<"AVGnsum============"<<nsum;


    extractpi4(sym_,ncount,avemsg);
    nfanoave=0;
    if (ncount>=0)
    {
        nfanoave=nsum;
        goto c900;
    }


c900:
    nsave_pi4=fmod(nsave_pi4,MAXAVE_PI4);//MAXAVE_PI4 hv if MAXAVE place chenged to 0
    //qDebug()<<"Next nsave"<<nsave_pi4;
    return;
}
void DecoderMs::print_msgpi4(int nsync,int nsnr,double dtx,int df,int width,QString decoded,QString csync,
                             QString cflags,int nfreq)
{
    bool f_only_one_color = true;
    if (f_only_one_color)
    {
        f_only_one_color = false;
        SetBackColor();
    }

    decoded = RemBegEndWSpaces(decoded);

    QStringList list;
    list <<s_time<<QString("%1").arg(nsync)<<QString("%1").arg(nsnr)
    <<QString("%1").arg(dtx,0,'f',1)
    <<QString("%1").arg(df)<<QString("%1").arg(width)
    <<decoded<<csync<<cflags<<QString("%1").arg(nfreq);
    //<<QString("%1").arg(8)<<QString("%1").arg(9.0,0,'f',1);
    emit EmitDecodetText(list,s_fopen,true);

}
void DecoderMs::mshvpi4(double *dat,int npts,int nutc,int minsync,int ntol,
                        int mode4,double nfqso,int ndepth/*,int neme*/)
{
    double ccfblue_p[545+40];
    double *ccfblue = &ccfblue_p[5+10];
    double ccfred_p[448+40];
    double *ccfred = &ccfred_p[224+10];
    double dtx,dfx,snrx,snrsync,flip,width;
    QString decoded;
    //QString special="     ";
    //double nsync = 0.0;
    QString csync;
    //double qbest;
    //double qabest;
    bool prtavg;
    //double qave;
    int nfanoave=0;

    pomAll.zero_double_beg_end(ccfblue,-10,550);
    //zero_double_beg_end(ccfred,-230,230);

    if (first_pi4)
    {
        nsave_pi4=0;
        first_pi4=false;
        blank_pi4 = "";
        clearave_pi4 = true;//hv first time clar all avg
        //ccfblue_pi4=0.
        //ccfred_pi4=0.
        //if (dttol==-99.0 && emedelay==-99.0 && nagain) return;
    }
    //!    syncmin=3.0 + minsync
    double syncmin=1.0+minsync;
    //int naggressive=0;
    //if (ndepth>=2) naggressive=1;
    //int nq1=3;
    //nq2=6
    //if (naggressive==1) nq1=1;
    if (clearave_pi4)
    {
        clearave_pi4 = false;
        nsave_pi4=0;

        for (int i = 0; i < MAXAVE_PI4; i++)
        {
            iutc_pi4[i]=-1;
            nfsave_pi4[i]=0.0;
            dtsave_pi4[i]=0.0;
            syncsave_pi4[i]=0.0;

            for (int z = 0; z < G_NSYM; z++)
            {
                if (i==0)
                    rsymbol_pi4_[z]=0.0;
                ppsave_pi4_[i][z]=0.0;
            }
        }
        //listutc[10]=0  no use
        //nfanoave_pi4=0;
        //ndeepave_pi4=0;
        count_use_avgs_pi4 = 0;
        count_saved_avgs_pi4 = 0;
    }

    // call timer('sync4   ',0)   7.142857  1.5*4.375
    int MouseDF=int(nfqso + 8.142857*mode4 - tx_nfqsopi4);   //qDebug()<<"zzzzzz";
    //qDebug()<<"MouseDF"<<nfqso<<1.5*4.375*mode4<<tx_nfqsopi4<<MouseDF;

    //npts = fmin(180000 ,npts);

    syncpi4(dat,npts,ntol,1,MouseDF,4,mode4,dtx,dfx,snrx,snrsync,ccfblue,ccfred,flip,width/*,ps0*/);
    //qDebug()<<"syncpi4"<<dfx<<dfx + tx_nfqsopi4 - mode4;

    double sync=snrsync;
    double dtxz=dtx-0.0;//dtx-0.0; -0.337406
    double nfreqz=dfx + tx_nfqsopi4 - 8.142857*mode4;//1.5*4.375  682.8125 - 9.0;//
    double nfreq = 0.0;
    //double idf = 1.0; //??? HV no exist

    //int ich1 = 0;
    //double qual = 0.0;
    int nfano = 0;
    //double dtx0 = 0.0;
    //double nfreq0 = 0.0;
    //double ich0 = 0.0;
    QString cflags = "";
    csync="*";

    //double dtx1 = 0.0;
    //double nfreq1 = 0.0;

    if (flip<0.0) csync="#";
    decoded=blank_pi4;

    //snrx=db(sync) - 26.0; // old method
    int nsnr=int(snrx);
    if (nsnr < -30) nsnr=-30; //hv v1.41


    //qDebug()<<"sync snrx nsnr "<<sync<<snrx<<"dtxz snr freq flip"<<dtxz<<nfreqz<<flip<<width;
    //sync = 2;
    if (sync<syncmin)
    {
        csync = "$";//2.16 +3s  -3.0
        print_msgpi4((int)sync,nsnr,dtxz-3.0,(nfreqz-tx_nfqsopi4),width,decoded,csync,cflags,(int)nfreqz);
        goto c990;
    }

    //deepmsg=blank_pi4;
    //special="     ";
    //nsync=sync;
    //nsnrlim=-33;

    //qbest=0.0;
    //qabest=0.0;
    prtavg=false;

    /*double dt=2.0/DEC_SAMPLE_RATE;
    if (dtxz<0)
    {
        int istart_min = fabs(dtxz/dt);
        for (int i = 0; i < istart_min; i++)
             dat
    }*/
    //dtxz=1.14118;
    for (int idt = -10; idt <= 10; idt++)//10
        //for (int idt = -0; idt <= 0; idt++)
    {//do idt=-2,2
        dtx=dtxz + 0.001666666666*(double)idt; //prez 10=samples dtx=dtxz + 0.03*idt       1.52830 i 1.52820
        //dtx=dtxz + 0.0001666666666*(double)idt;//prez 1=sample
        //dtx=dtxz + 0.03*(double)idt;
        nfreq=nfreqz;// + 1.0*(double)idt;//idf ????? 2.2
        //qDebug()<<"START NORMAL decoded=="<<dtx;

        decodepi4(dat,npts,dtx,nfreq/*,flip*/,mode4,decoded,nfano);

        //double dt=2.0/DEC_SAMPLE_RATE;
        //qDebug()<<"ssss="<<idt<<dtx<<"istart="<<int(dtx/dt);

        //qDebug()<<"NORMAL decoded=="<<decoded<<nfano;

        //nfano = 0;

        if (nfano>0)//nfano -> 1=ok  "0=no ok go to avg"  "-1no real coll no go in avg"
        {
            cflags = "Nor";//norm  //2.16 +3s  -3.0
            print_msgpi4((int)sync,nsnr,dtx-3.0,(nfreq-tx_nfqsopi4),width,decoded,csync,cflags,(int)nfreq);

            nsave_pi4=0;
            goto c990;
        }
        //qDebug()<<"AVGGG=qDebug()"<<nfano;

        if (idt!=0) continue;

        //qave=0.0;

        //qDebug()<<"AVGGG=="<<(ndepth & 16)<<prtavg<<nutc<<nutc0_pi4<<nfreq-nfreq0_pi4<<ntol;
        //nfano -> 1=ok  "0=no ok go to avg"  "-1no real coll no go in avg"
        if ((ndepth & 16)==16 && !prtavg && nfano!=-1)
        {
            //qDebug()<<"AVGGG=qDebug()Innnnnnnnnnnn"<<nfano<<idt;
            QString avemsg;
            if (nutc!=nutc0_pi4 || fabs(nfreq-nfreq0_pi4)>ntol)//
            {

                nutc0_pi4=nutc;
                nfreq0_pi4=nfreq;

                //nsave=nsave+1
                //nsave=mod(nsave-1,64)+1

                avgpi4(nutc,sync,dtx,flip,nfreq,ntol,avemsg/*,qave*/,nfanoave);
                //qDebug()<<"AVG MSGGGG = "<<avemsg;

            }
            //qDebug()<<"AVGGG==nfanoave_pi4"<<nfanoave<<ich1_pi4<<ich2_pi4<<nutc<<nutc0_pi4;
            if (nfanoave>0)
            {
                cflags = "Avg";
                csync = "**";//2.16 +3s  -3.0
                print_msgpi4((int)sync,nsnr,dtx-3.0,(nfreq-tx_nfqsopi4),width,avemsg,csync,cflags,(int)nfreq);
                prtavg=true;

                //SetClearAvgPi4();

                continue; // cycle
            }
        }
    }
    //nfano -> 1=ok  "0=no ok go to avg"  "-1no real coll no go in avg"
    if (nfano!=-1 && decoded.isEmpty() && !prtavg)//nfano -> 1=ok  "0=no ok go to avg"  "-1no real coll no go in avg"
    {
        cflags = "Nor";//norm
        csync = "#";
        decoded = "?";//2.16 +3s  -3.0
        print_msgpi4((int)sync,nsnr,dtxz-3.0,(nfreq-tx_nfqsopi4),width,decoded,csync,cflags,(int)nfreq);
    }

c990:
    return;
}
void DecoderMs::lpf1(double *dd,int jz,double *dat,int &jz2)
{
    int NFFT1=64*DEC_SAMPLE_RATE;// change hv 
    int NFFT2=32*DEC_SAMPLE_RATE;//2.16 3s

    //int offset_pi4 = 1.0*(DEC_SAMPLE_RATE/2.0);//3.2     1.0,2.0,3.0 ... sec  work from to 0.2s-4.0s

    //double x[NFFT1+50];
    double *x = new double[NFFT1+1024];//+1024 for linux crash
    //double complex cx[NFFT1/2+50];
    double complex *cx = new double complex[NFFT1+1024];//2.09 crash->[NFFT1/2+1024];//+1024 for linux crash

    double fac=1.0/double(NFFT1);
    for (int i = 0; i < jz; i++)
        x[i]=fac*dd[i];
    for (int i = jz; i < NFFT1; i++)
        x[i]=0.0;
    f2a.four2a_d2c(cx,x,NFFT1,-1,0);
    for (int i = NFFT2/2; i < NFFT1/2; i++)
        cx[i]=0.0+0.0*I;

    f2a.four2a_d2c(cx,x,NFFT2,1,-1);
    jz2=jz/2;

    /*for (int i = 0; i < offset_pi4; i++)
              dat[i] = 0.0;
    int count_offset = offset_pi4;
    for (int i = 0; i < jz2-(offset_pi4+1); i++)
    {
        dat[count_offset]=x[i];
        count_offset++;
    }*/

    for (int i = 0; i < jz2; i++)
    {
        dat[i]=x[i]*0.01;//hv koeficient double
    }

    delete [] cx;
    delete [] x;
}
void DecoderMs::SetClearAvgPi4()
{
    clearave_pi4 = true;
    emit EmitAvgSavesPi4(0,0); // exception
}
void DecoderMs::pi4_decode(double *dd,int jz)
{
    int jz2 = 0;
    //double dat[361000]; //60*6000=360000   61*6000=366000
    double *dat = new double[366000];//366000

    int mode4=40;//nmode hv

    //bool NClearAve = clearave_pi4;
    int ntol = G_DfTolerance;
    //int neme = 0;
    int nutc = s_time.toInt()/100;
    int minsync = G_MinSigdB;
    //double emedelay = 0.0; // -99.0 outof cicle
    //double dttol = 3.0;
    //int minw = 0;
    double s_nfqsoPi4 = s_nfqso_all;
    //bool NAgain = false;
    int ndepth = 16;//G_DepthAvg;//0 stop  avg=16 ;

    //qDebug()<<"s_nfqsoPi4 ntol"<<s_nfqsoPi4<<ntol<<jz;

    //qDebug()<<"in raw="<<jz;
    //inportent hv 1.41 max_samp = 1s from prev period + 25s from msg + 4s for sync = 1+25+4=30s
    //int max_samp = 31.0*DEC_SAMPLE_RATE;
    //inportent hv 2.16 max_samp = 3s from prev period + 25s from msg + 4s for sync = 3+25+4=32s
    int max_samp = 33.0*DEC_SAMPLE_RATE;//2.16 +3s 33
    jz = fmin(max_samp,jz);
    //qDebug()<<"out raw="<<jz;

    lpf1(dd,jz,dat,jz2);

    //qDebug()<<"sssshhhhhhhhhhhhhhhhhhhhhhhhhhhhhh"<<jz<<jz2;

    //for (int i = 10; i < 20; i++)
    //{
    //ndelta_pi4 = i;
    //first_extractpi4 = true;
    //ssss = (double)i;
    mshvpi4(dat,jz2,nutc,minsync,ntol,mode4,s_nfqsoPi4,ndepth/*,neme*/);
    //qDebug()<<"sssshhhhhhhhhhhhhhhhhhhhhhhhhhhhhh"<<ssss;
    //}

    //resample_12_6(dd,jz,dat);
    //mshvpi4(dat,jz,nutc,minsync,ntol,emedelay,dttol,mode4,minw,s_nfqsoPi4,NAgain,ndepth/*,neme*/);

    delete [] dat;//delete [] dat;
}
/*
void DecoderMs::resample_12_6(double *d2,int &npts,double *dd)
{
    int NZ12 =60*12000;
    int NFFT2=64*6000;
    int NFFT1=64*12000;
    //double x[NFFT2];
    //double complex cx[NFFT1/2];//(0:NFFT1/2)
    double *x = new double[NFFT1];
    double complex *cx = new double complex[NFFT1];

    int jz=fmin(NZ12,npts);
    for (int i = 0; i < jz; i++)
        x[i]=d2[i];
    for (int i = jz; i < NFFT2; i++)
        x[i]=0.0;
    //qDebug()<<"s_nfqsoPi4 ntol";
    four2a_double_to_complex(cx,x,NFFT1,1,-1,0); //call four2a(x,nfft1,1,-1,0)//!Forward FFT, r2c
    double df=12000.0/(double)NFFT1;
    int ia=5000.0/df;//??? ia=5000.0/df
    for (int i = ia; i < NFFT1/2; i++)
        cx[i]=0.0;// cx(ia:)=0.0
    four2a_double_to_complex(cx,x,NFFT2,1,1,-1);//call four2a(cx,nfft2,1,1,-1)//!Inverse FFT, c2r

    npts=jz*6000.0/12000.0;
    double fac=1.e-6;
    for (int i = 0; i < npts; i++)
        dd[i]=fac*x[i];//dd(1:npts)=fac*x(1:npts)

    delete cx;
    delete x;
}
*/





