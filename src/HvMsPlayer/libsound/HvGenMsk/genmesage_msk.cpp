/* The algorithms, source code, look-and-feel of WSJT-X and related
 * programs, and protocol specifications for the modes FSK441, FT8, JT4,
 * JT6M, JT9, JT65, JTMS, QRA64, ISCAT, MSK144, are Copyright © 2001-2017
 * by one or more of the following authors: Joseph Taylor, K1JT; Bill
 * Somerville, G4WJS; Steven Franke, K9AN; Nico Palermo, IV3NWV; Greg Beam,
 * KI7MT; Michael Black, W9MDB; Edson Pereira, PY2SDR; Philip Karn, KA9Q;
 * and other members of the WSJT Development Group.
 *
 * MSHV MSK144/40 Generator
 * Rewritten into C++ and modified by Hrisimir Hristov, LZ2HV 2015-2017
 * May be used under the terms of the GNU General Public License (GPL)
 */

#include "genmesage_msk.h"
#include "../../../nhash.h"

#include "bpdecode_msk.h"
#include "bpdecode_msk_128_90.h"
#include "config_rpt_msk40.h"

//#include <QtGui>

GenMsk::GenMsk(bool f_dec_gen)//f_dec_gen = dec=true gen=false
{
    twopi=8.0*atan(1.0);
    //first_short_msk = true;

    ///MSK144
    first_msk144_enc = true;
    first_msk144 = true;
    //App_Path = path;
    //MSK40
    first_msk40_enc = true;
    //first_msk40 = true;

    TPackUnpackMsg77.initPackUnpack77(f_dec_gen);//f_dec_gen = dec=true gen=false
    first_msk144_enc_v2 = true;
}
GenMsk::~GenMsk()
{}
// vremenno mai postoianno 1.46
QString GenMsk::unpackmsg144(int *dat,char &ident,QString &c1,QString &c2,bool f)
{
    return TPackUnpackMsg.unpackmsg144(dat,ident,c1,c2,f);
}
void GenMsk::save_hash_call_my_his_r1_r2(QString call,int pos)
{
    TPackUnpackMsg77.save_hash_call_my_his_r1_r2(call,pos);
}
QString GenMsk::unpack77(bool *c77,bool &unpk77_success)
{
    return TPackUnpackMsg77.unpack77(c77,unpk77_success);
}
void GenMsk::Get_C1_C2_RX_Calls(QString &c1,QString &c2)
{
    c1=TPackUnpackMsg77.c1_rx_calls;
    c2=TPackUnpackMsg77.c2_rx_calls;
}
QString GenMsk::RemWSpacesInside(QString s)
{
    for (int i = 0; i<s.count(); i++)
        s.replace("  "," ");
    return s;
}
int GenMsk::hash(QString string,int len,int ihash)
{
    //use iso_c_binding, only: c_loc,c_size_t
    //use hashing
    int MASK15=32767;//parameter (MASK15=32767)
    //!  character*(*), target :: string
    //character*1, target :: string
    //unsigned char word[50] = {' '};
    unsigned char word[50] = {' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',
                              ' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',
                              ' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',
                              ' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',
                              ' ',' ',' ',' ',' ',' ',' ',' ',' ',' '};
    for (int i = 0; i < string.count(); i++)
    {
        word[i] = (unsigned char)string[i].toLatin1();
        //qDebug()<<"word="<<(char)word[i]<<i;
    }
    //for (int i = 0; i < 50; i++)
    //qDebug()<<"word="<<(char)word[i];

    uint32_t i=nhash(word,len,146);//hash(msg(2:i1-1),i1-2,ihash)
    i=(32767 & i);//2.76 
    ihash=(i & MASK15);
    return ihash;
}

////MSK144/////////////
void GenMsk::copy_char_ar(char*a,int a_beg,int a_end,char*b,int b_beg,int b_odd)
{
    int c = 0;
    for (int i = a_beg; i < a_end; i++)
    {
        a[i]=b[c+b_beg];
        c+=b_odd;
    }
}
void GenMsk::copy_double_ar(double*a,int a_beg,int a_end,double*b,int b_beg)
{
    int c = 0;
    for (int i = a_beg; i < a_end; i++)
    {
        a[i]=b[c+b_beg];
        c++;
    }
}
void GenMsk::platanh(double x, double &y)
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
void GenMsk::bpdecode144(double *llr,int maxiterations,char *decoded,int &niterations)
{
    const int N=128;
    const int K=80;
    const int M=N-K;//48

    double tov_[N][3];     //real*8 tov(3,N)
    double toc_[M][8];     //real*8 toc(8,M)
    double tanhtoc_[M][8]; //real*8 tanhtoc(8,M)
    double zn[N];          //real*8 zn(N)
    char cw[N];
    int synd[M];
    char codeword[N];

    int nrw=8;
    int ncw=3;

    /*for (int j = 0; j < M; j++) //toc=0
    {
        for (int i = 0; i < nrw; i++)
            toc_[j][i]=0.0;
    }*/
    for (int j = 0; j < N; j++) //tov=0
    {
        for (int i = 0; i < ncw; i++)
            tov_[j][i]=0.0;
    }
    /*for (int j = 0; j < M; j++) //tanhtoc=0
    {
        for (int i = 0; i < nrw; i++)
            tanhtoc_[j][i]=0.0;
    }*/

    //! initial messages to checks
    for (int j = 0; j < M; j++)
    {//do j=1,M
        for (int i = 0; i < nrw; i++)
        {//do i=1,nrw
            toc_[j][i]=llr[Nm_msk144_[j][i]-1]; //hv-1 toc(i,j)=llr((Nm(i,j)))
        }
    }
    //qDebug()<<"toc_[j][i]="<<toc_[5][6];

    int nclast = 0;
    int ncnt = 0;
    for (int iter = 0; iter <= maxiterations; iter++)// v1.29 <=
    {//do iter=0,maxiterations

        //! Update bit log likelihood ratios
        for (int i = 0; i < N; i++)
        {//do i=1,N
            //zn(i)=llr(i)+sum(tov(1:ncw,i))
            double sum = 0.0;
            for (int x = 0; x < ncw; x++)
                sum += tov_[i][x];
            zn[i]=llr[i]+sum;

            if (zn[i]>0.0)//cw=0 //where( zn .gt. 0. ) cw=1
                cw[i]=1;
            else
                cw[i]=0;
        }

        //! Check to see if we have a codeword //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
        /*for (int i = 0; i < N; i++)
        {
            if (zn[i]>0.9999)//cw=0 //where( zn .gt. 0. ) cw=1
                cw[i]=1;
            else
                cw[i]=0;
        }*/

        int ncheck=0;
        for (int i = 0; i < M; i++)
        {//do i=1,M
            //synd(i)=sum(cw(Nm(:,i)))
            int sum = 0;
            for (int x = 0; x < nrw; x++)
                sum += (int)cw[Nm_msk144_[i][x]-1];//hv-1
            synd[i]= sum;
            //if( mod(synd(i),2) .ne. 0 ) ncheck=ncheck+1
            if ( fmod(synd[i],2) != 0 ) ncheck++;
        }

        if ( ncheck == 0 ) //then //! we have a codeword - reorder the columns and return it.
        {
            //qDebug()<<"iter="<<iter;
            niterations=iter;
            for (int i = 0; i < N; i++)//codeword=cw(colorder+1)
                codeword[i]=cw[(int)colorder_msk144[i]];

            int cc = 0;
            for (int i = M+0; i < N; i++) //decoded=codeword(M+1:N) ?????
            {
                decoded[cc]=codeword[i];
                cc++;
            }
            //qDebug()<<"niterations="<<niterations;
            return;
        }

        //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
        if ( iter > 0 )  //! this code block implements an early stopping criterion
        {
            int nd=ncheck-nclast;
            if ( nd < 0 ) //! # of unsatisfied parity checks decreased
                ncnt=0;  //! reset counter
            else
                ncnt++;  //ncnt=ncnt+1;

            //!    write(*,*) iter,ncheck,nd,ncnt
            if ( ncnt >= 3 && iter >= 5 && ncheck > 10)
            {
                niterations=-1;
                return;
            }
        }
        nclast=ncheck;


        //! Send messages from bits to check nodes
        for (int j = 0; j < M; j++)
        {//do j=1,M
            for (int i = 0; i < nrw; i++)
            {//do i=1,nrw
                int ibj=Nm_msk144_[j][i]-1;//ibj=Nm(i,j)
                toc_[j][i]=zn[ibj];//toc(i,j)=zn(ibj)
                for (int kk = 0; kk < ncw; kk++)
                {//do kk=1,ncw //! subtract off what the bit had received from the check
                    if ( Mn_msk144_[ibj][kk]-1 == j )//hv-1 if( Mn(kk,ibj) .eq. j ) then   //! Mn(3,128)
                        toc_[j][i]=toc_[j][i]-tov_[ibj][kk]; //hv-1 toc(i,j)=toc(i,j)-tov(kk,ibj)
                }
                tanhtoc_[j][i]=tanh(-toc_[j][i]/2.0);//tanhtoc(1:nrw,i)=tanh(-toc(1:nrw,i)/2)
            }
        }

        //! send messages from check nodes to variable nodes
        /*/for (int i = 0; i < M; i++)
        {//do i=1,M
            for (int x = 0; x < nrw; x++)////tanhtoc(1:nrw,i)=tanh(-toc(1:nrw,i)/2)
                tanhtoc_[i][x]=tanh(-toc_[i][x]/2.0);
        }*/

        for (int j = 0; j < N; j++)
        {//do j=1,N
            for (int i = 0; i < ncw; i++)
            {//do i=1,ncw
                int ichk=Mn_msk144_[j][i]-1; //ichk=Mn(i,j)  //! Mn(:,j) are the checks that include bit j
                //Tmn=product(tanhtoc(:,ichk),mask=Nm(:,ichk).ne.j)
                double Tmn = 1.0;//mulilay
                for (int z = 0; z < 8; z++)
                {
                    if (Nm_msk144_[ichk][z]-1 != j)
                        Tmn = Tmn*tanhtoc_[ichk][z];
                }
                double y;
                platanh(-Tmn,y);//call platanh(-Tmn,y)
                tov_[j][i]=2.0*y;
            }
        }
    }
    niterations=-1;
}
// for mskms v1
void GenMsk::encode_msk144(char *message, char *codeword)
{
    //! Encode an 80-bit message and return a 128-bit codeword.
    //! The generator matrix has dimensions (48,80).
    //! The code is a (128,80) regular ldpc code with column weight 3.
    //! The code was generated using the PEG algorithm.
    //! After creating the codeword, the columns are re-ordered according to
    //! "colorder" to make the codeword compatible with the parity-check
    //! matrix stored in Radford Neal's "pchk" format.
    char itmp[128];//integer*1 itmp(128)
    char pchecks[48];//integer*1 pchecks(48)

    //unsigned long long k = 0x24084000800020008000;

    QString g[48] =
        {"24084000800020008000",
         "b39678f7ccdb1baf5f4c",
         "10001000400408012000",
         "08104000100002010800",
         "dc9c18f61ea0e4b7f05c",
         "42c040160909ca002c00",
         "cc50b52b9a80db0d7f9e",
         "dde5ace80780bae74740",
         "00800080020000890080",
         "01020040010400400040",
         "20008010020000100030",
         "80400008004000040050",
         "a4b397810915126f5604",
         "04040100001040200008",
         "00800006000888000800",
         "00010c00000104040001",
         "cc7cd7d953cdc204eba0",
         "0094abe7dd146beb16ce",
         "5af2aec8c7b051c7544a",
         "14040508801840200088",
         "7392f5e720f8f5a62c1e",
         "503cc2a06bff4e684ec9",
         "5a2efd46f1efbb513b80",
         "ac06e9513fd411f1de03",
         "16a31be3dd3082ca2bd6",
         "28542e0daf62fe1d9332",
         "00210c002001540c0401",
         "0ed90d56f84298706a98",
         "939670f7ecdf9baf4f4c",
         "cfe41dec47a433e66240",
         "16d2179c2d5888222630",
         "408000160108ca002800",
         "808000830a00018900a0",
         "9ae2ed8ef3afbf8c3a52",
         "5aaafd86f3efbfc83b02",
         "f39658f68cdb0baf1f4c",
         "9414bb6495106261366a",
         "71ba18670c08411bf682",
         "7298f1a7217cf5c62e5e",
         "86d7a4864396a981369b",
         "a8042c01ae22fe191362",
         "9235ae108b2d60d0e306",
         "dfe5ade807a03be74640",
         "d2451588e6e27ccd9bc4",
         "12b51ae39d20e2ea3bde",
         "a49387810d95136fd604",
         "467e7578e51d5b3b8a0e",
         "f6ad1ac7cc3aaa3fe580"};

    if ( first_msk144_enc ) //then ! fill the generator matrix
    {
        for (int i = 0; i < 80; i++)
        {
            for (int j = 0; j < 48; j++)
                gen144_[i][j]=0;
        }
        for (int i = 0; i < 48; i++)
        {//do i=1,48
            for (int j = 0; j < 5; j++)
            {//do j=1,5
                bool ok;
                //int hex = str.toInt(&ok, 16);       // hex == 255, ok == true
                int istr = g[i].mid(j*4,4).toInt(&ok, 16);   //read(g(i)( (j-1)*4+1:(j-1)*4+4 ),"(Z4)") istr
                //qDebug()<<"istr="<<g[i].mid(j*4,4)<<g[i].mid(j*4,4).toInt(&ok, 16);
                for (int jj = 0; jj < 16; jj++)
                {//do jj=1,16
                    int icol=(j-0)*16+jj;  //icol=(j-1)*16+jj
                    //if( btest(istr,16-jj) ) gen144(i,icol)=1
                    gen144_[icol][i]=(1 & (istr >> (15-jj)));
                    //qDebug()<<"bits="<<int(1 & (istr << (15-jj)));
                }
            }
        }
        first_msk144_enc=false;
    }

    for (int i = 0; i < 48; i++)
    {//do i=1,48
        int nsum=0;
        for (int j = 0; j < 80; j++)
        {//do j=1,80
            nsum=nsum+message[j]*gen144_[j][i];//nsum=nsum+message(j)*gen144(i,j)
        }
        pchecks[i]=fmod(nsum,2);
        //qDebug()<<"pchecks="<<(int)nsum;
    }
    for (int i = 0; i < 48; i++)    //itmp(1:48)=pchecks
        itmp[i]=pchecks[i];

    for (int i = 0; i < 80; i++) //itmp(49:128)=message(1:80)
        itmp[48+i]=message[i];

    for (int i = 0; i < 128; i++)
        codeword[(int)colorder_msk144[i]]=itmp[i];//codeword(colorder+1)=itmp(1:128)
}
// for msk144 v2
#include "../boost/crc.hpp"
#include "../boost/config.hpp"
#define POLY13 0x15D7
#ifdef BOOST_NO_CXX11_CONSTEXPR
#define TRUNCATED_POLYNOMIAL13 POLY13
#else
namespace
{
unsigned long constexpr TRUNCATED_POLYNOMIAL13 = POLY13;
}
#endif
short crc13_(unsigned char const * data, int length)
{
    return boost::augmented_crc<13, TRUNCATED_POLYNOMIAL13>(data, length);
}
short GenMsk::crc13(unsigned char const * data, int length)
{
    return crc13_(data,length);
}
// for msk144 v2
void GenMsk::chkcrc13a(bool *decoded,int &nbadcrc)
{
    /*  use crc
    integer*1 decoded(90)
    integer*1, target:: i1Dec8BitBytes(12)
    character*90 cbits

    ! Write decoded bits into cbits: 77-bit message plus 13-bit CRC
    write(cbits,1000) decoded
    1000 format(90i1)
    read(cbits,1001) i1Dec8BitBytes
    1001 format(12b8)
    read(cbits,1002) ncrc13                         !Received CRC13
    1002 format(77x,b13)

    i1Dec8BitBytes(10)=iand(i1Dec8BitBytes(10),128+64+32+16+8)
    i1Dec8BitBytes(11:12)=0
    icrc13=crc13(c_loc(i1Dec8BitBytes),12)          !CRC13 computed from 77 msg bits

    nbadcrc=1
    if(ncrc13.eq.icrc13) nbadcrc=0*/

    unsigned char i1Dec8BitBytes[12+5];
    for (int ibyte = 0; ibyte<12; ibyte++)
    {//do ibyte=1,10
        int itmp=0;
        for (int ibit = 0; ibit<8; ibit++)
        {//do ibit=1,8
            itmp=(itmp << 1)+(1 & decoded[(ibyte-0)*8+ibit]);//itmp=ishft(itmp,1)+iand(1,decoded((ibyte-1)*8+ibit))
            //qDebug()<<(int)decoded[(ibyte-0)*7+ibit];
        }
        i1Dec8BitBytes[ibyte]=itmp;
    }
    int ncrc13=0;
    for (int ibit = 0; ibit<13; ibit++)
        ncrc13=(ncrc13 << 1)+(1 & decoded[77+ibit]);

    i1Dec8BitBytes[9]=(i1Dec8BitBytes[9] & 248); //i1Dec8BitBytes(10)=iand(i1Dec8BitBytes(10),128+64+32+16+8)
    i1Dec8BitBytes[10]=0;
    i1Dec8BitBytes[11]=0;
    int icrc13=crc13(i1Dec8BitBytes,12);

    nbadcrc=1;  //qDebug()<<"ncrc14==icrc14"<<ncrc14<<icrc14;

    /*int sum_5687 = 0;
    for (int i = 77; i<91; i++)
        sum_5687+=(int)decoded[i];*/

    if (ncrc13==icrc13)// && ncrc14!=0  sum_5687   || sum_5687==0
        nbadcrc=0;
}
void GenMsk::bpdecode128_90(double *llr,int maxiterations,bool *message77,int &nharderror)//bool *cw,bool *apmask,int &iter
{
    /*! A log-domain belief propagation decoder for the (128,90) code.
     include "ldpc_128_90_reordered_parity.f90"*/
    const int ncw=3; 
    const int N=128;
    const int K=90;
    const int M=N-K;//38
    double toc_[M][11];//toc(11,M)
    double tov_[N][ncw];//tov(4,N)//hv no 4
    double tanhtoc_[M][11];//tanhtoc(11,M)
    double zn[N];
    bool cw[N];
    int synd[M];
    bool decoded[K+20];//91 need 96 for check14a integer*1 decoded(K)

    for (int i = 0; i<N; i++)
    {
        for (int j = 0; j<ncw; j++)
            tov_[i][j]=0.0;//tov=0
    }
    for (int i = 80; i<100; i++)
        decoded[i]=0;//for check13a

    /*for (int j = 0; j<M; j++)
    {
    	for (int i = 0; i<11; i++)
    		toc_[j][i]=0.0;  
    }*/
    //decoded=0
    //toc=0
    //tanhtoc=0
    //! initialize messages to checks
    for (int j = 0; j<M; j++)
    {//do j=1,M
        for (int i = 0; i<(nrw_msk144_128_90[j]); i++)
        {//do i=1,nrw(j)
            toc_[j][i]=llr[Nm_msk144_128_90_[j][i]-1];//toc(i,j)=llr((Nm(i,j)))
        }
    }
    int ncnt=0;
    int nclast = 0;

    for (int iter = 0; iter<=maxiterations; iter++)
    {
        //! Update bit log likelihood ratios (tov=0 in iteration 0).
        for (int i = 0; i< N; i++)
        {//do i=1,N
            //if ( apmask[i] != 1 )
            //{
            double sumt = 0.0;
            for (int x = 0; x < ncw; x++)
                sumt+=tov_[i][x];//zn[i]=llr[i]+sum(tov(1:ncw,i))
            zn[i]=llr[i]+sumt;
            //}
            //else
            //zn[i]=llr[i];

            if (zn[i]>0.0)//cw=0 //where( zn .gt. 0. ) cw=1
                cw[i]=1;
            else
                cw[i]=0;
        }

        int ncheck=0;
        for (int i = 0; i < M; i++)
        {//do i=1,M
            // synd(i)=sum(cw(Nm(1:nrw(i),i)))
            int sum = 0;
            for (int x = 0; x < nrw_msk144_128_90[i]; x++)
                sum += (int)cw[Nm_msk144_128_90_[i][x]-1];//hv-1
            synd[i]= sum;
            //if( mod(synd(i),2) .ne. 0 ) ncheck=ncheck+1
            if ( fmod(synd[i],2) != 0 ) ncheck++;
            //if ( (synd[i] % 2) != 0 ) ncheck++;
            //!   if( mod(synd(i),2) .ne. 0 ) write(*,*) 'check ',i,' unsatisfied'
        }

        if ( ncheck == 0 ) //then ! we have a codeword - if crc is good, return it
        {
            //QString sss;
            for (int i = 0; i < K; i++)
            {
                decoded[i]=cw[i];//decoded=cw(1:K)
                //sss.append(QString("%1").arg((int)decoded[i]));
            }
            //qDebug()<<"chkcrc14a"<<sss;
            int nbadcrc; //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
            chkcrc13a(decoded,nbadcrc);//qDebug()<<"chkcrc14a"<<nbadcrc<<iter;

            if (nbadcrc==0)
            {
                for (int i = 0; i < 77; i++)
                    message77[i]=decoded[i];//message77[]=decoded(1:77)
                int count = 0;
                for (int i = 0; i < N; i++)
                {
                    //if (llr[i]*(double)(cw[i]*2-1)<0.0)
                    if ((double)(2*cw[i]-1)*llr[i] < 0.0 )
                        count++;
                }
                nharderror=count;
                return;
            }
        }

        if ( iter > 0 )  //! this code block implements an early stopping criterion
        {
            int nd=ncheck-nclast;
            if ( nd < 0 ) //! # of unsatisfied parity checks decreased
                ncnt=0;  //! reset counter
            else
                ncnt++;  //ncnt=ncnt+1;

            //!    write(*,*) iter,ncheck,nd,ncnt
            if ( ncnt >= 3 && iter >= 5 && ncheck > 10)//if( ncnt .ge. 5 .and. iter .ge. 10 .and. ncheck .gt. 15) then
            {
                nharderror=-1;
                //qDebug()<<"iter FFFFF="<<ncnt<<iter<<ncheck;
                return;
            }
        }
        nclast=ncheck;

        for (int j = 0; j < M; j++)
        {//do j=1,M
            for (int i = 0; i < nrw_msk144_128_90[j]; i++)
            {//do i=1,nrw
                int ibj=Nm_msk144_128_90_[j][i]-1;//ibj=Nm(i,j)
                toc_[j][i]=zn[ibj];//toc(i,j)=zn(ibj)
                for (int kk = 0; kk < ncw; kk++)
                {//do kk=1,ncw //! subtract off what the bit had received from the check
                    if ( Mn_msk144_128_90_[ibj][kk]-1 == j )//hv-1 if( Mn(kk,ibj) .eq. j ) then   //! Mn(3,128)
                        toc_[j][i]=toc_[j][i]-tov_[ibj][kk]; //hv-1 toc(i,j)=toc(i,j)-tov(kk,ibj)
                }
                //tanhtoc_[j][i]=tanh(-toc_[j][i]/2.0);//tanhtoc(1:nrw,i)=tanh(-toc(1:nrw,i)/2)
            }
        }

        for (int i = 0; i < M; i++)
        {//do i=1,M
            for (int x = 0; x < 11; x++)////tanhtoc(1:nrw,i)=tanh(-toc(1:nrw,i)/2)
                tanhtoc_[i][x]=tanh(-toc_[i][x]/2.0);
        }

        for (int j = 0; j < N; j++)
        {//do j=1,N
            for (int i = 0; i < ncw; i++)
            {//do i=1,ncw
                int ichk=Mn_msk144_128_90_[j][i]-1; //ichk=Mn(i,j)  //! Mn(:,j) are the checks that include bit j
                //Tmn=product(tanhtoc(1:nrw(ichk),ichk),mask=Nm(1:nrw(ichk),ichk).ne.j)
                //Tmn=product(tanhtoc(1:nrw(ichk),ichk),mask=Nm(1:nrw(ichk),ichk).ne.j)
                double Tmn = 1.0;//mulilay
                for (int z = 0; z < nrw_msk144_128_90[ichk]; z++)
                {
                    if (Nm_msk144_128_90_[ichk][z]-1 != j)
                        Tmn = Tmn*tanhtoc_[ichk][z];
                }
                double y;
                platanh(-Tmn,y);//call platanh(-Tmn,y)
                //!      y=atanh(-Tmn)
                tov_[j][i]=2.0*y;
            }
        }
    }
    nharderror=-1;
}
void GenMsk::encode_128_90(bool *message77,bool* codeword)
{
    const int N=128;
    const int K=90;
    const int M=N-K;//38
    unsigned char i1MsgBytes[15];
    bool pchecks[M];

    QString g[M] =
        {
            "a08ea80879050a5e94da994",
            "59f3b48040ca089c81ee880",
            "e4070262802e31b7b17d3dc",
            "95cbcbaf032dc3d960bacc8",
            "c4d79b5dcc21161a254ffbc",
            "93fde9cdbf2622a70868424",
            "e73b888bb1b01167379ba28",
            "45a0d0a0f39a7ad2439949c",
            "759acef19444bcad79c4964",
            "71eb4dddf4f5ed9e2ea17e0",
            "80f0ad76fb247d6b4ca8d38",
            "184fff3aa1b82dc66640104",
            "ca4e320bb382ed14cbb1094",
            "52514447b90e25b9e459e28",
            "dd10c1666e071956bd0df38",
            "99c332a0b792a2da8ef1ba8",
            "7bd9f688e7ed402e231aaac",
            "00fcad76eb647d6a0ca8c38",
            "6ac8d0499c43b02eed78d70",
            "2c2c764baf795b4788db010",
            "0e907bf9e280d2624823dd0",
            "b857a6e315afd8c1c925e64",
            "8deb58e22d73a141cae3778",
            "22d3cb80d92d6ac132dfe08",
            "754763877b28c187746855c",
            "1d1bb7cf6953732e04ebca4",
            "2c65e0ea4466ab9f5e1deec",
            "6dc530ca37fc916d1f84870",
            "49bccbbee152355be7ac984",
            "e8387f3f4367cf45a150448",
            "8ce25e03d67d51091c81884",
            "b798012ffa40a93852752c8",
            "2e43307933adfca37adc3c8",
            "ca06e0a42ca1ec782d6c06c",
            "c02b762927556a7039e638c",
            "4a3e9b7d08b6807f8619fac",
            "45e8030f68997bb68544424",
            "7e79362c16773efc6482e30"
        };

    if ( first_msk144_enc_v2 )//gen144_v2_[95][43];//integer*1 gen(M,K)  N=128, K=90, M=N-K=38
    {
        for (int i = 0; i < K; ++i)
        {
            for (int j = 0; j < M; ++j)
                gen144_v2_[i][j]=0;
        }
        for (int i = 0; i < M; ++i)
        {//do i=1,M
            for (int j = 0; j < 23; ++j)//23 bb a8 30 e2 3b 6b 6f 50 98 2e
            {//do j=1,11
                bool ok;
                //int hex = str.toInt(&ok, 16);       // hex == 255, ok == true
                int istr = g[i].mid(j*1,1).toInt(&ok, 16); //read(g(i)(j:j),"(Z1)") istr
                
                for (int jj = 0; jj < 4; jj++)
                {//jj=1, 8
                    int icol=(j-0)*4+jj;
                    //if( icol .le. 87 ) then //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
                    if ( icol <= K-1 )
                    {
                        //if( btest(istr,4-jj) ) gen(i,icol)=1
                        gen144_v2_[icol][i]=(1 & (istr >> (3-jj)));
                        //qDebug()<<"bits="<<icol<<i;
                    }
                }
            }
        }
        first_msk144_enc_v2=false;
    }

    /*! Add 13 bit CRC to form 90-bit message+CRC13
    write(tmpchar,'(77i1)') message77
    tmpchar(78:80)='000'
    i1MsgBytes=0
    read(tmpchar,'(10b8)') i1MsgBytes(1:10)
    ncrc13 = crc13 (c_loc (i1MsgBytes), 12)
    write(tmpchar(78:90),'(b13)') ncrc13
    read(tmpchar,'(90i1)') message*/

    for (int i = 0; i < 15; ++i)
        i1MsgBytes[i]=0;
    int c_77 = 0;
    for (int i = 0; i < 10; ++i)
    {
        int k = 0;
        for (int j = 0; j < 8; ++j)
        {
            k <<= 1;
            k |= message77[c_77];//- 0
            c_77++;
        }
        i1MsgBytes[i] = k;
    }

    int ncrc13=crc13(i1MsgBytes,12);

    int izz = 13-1;
    int pos = 77;
    for (int i = 0; i < 13; ++i)
    {
        message77[pos]=(1 & (ncrc13 >> -(i-izz)));
        pos++;
    }
    for (int i = 0; i < M; ++i)
    {//do i=1,M
        int nsum=0;
        for (int j = 0; j < K; ++j)//k=90
        {//do j=1,K
            nsum+=message77[j]*gen144_v2_[j][i];//nsum=nsum+message(j)*gen(i,j);
        }
        pchecks[i]=fmod(nsum,2);
    }
    for (int i = 0; i < K; ++i)//90
        codeword[i]=message77[i];
    for (int i = 0; i < M; ++i)//128-90=38
        codeword[i+K]=pchecks[i];
}
void GenMsk::genmsk144(int *i4Msg6BitWords, unsigned char *i1Msg8BitBytes, int *i4tone,bool *c77,bool msk144ms)
{
    int i=0;
    unsigned char i1hash[4] = {0};
    //char msgbits[81];//integer*1 msgbits(80)
    char codeword[130];//integer*1 codeword(128)                 !Encoded bits before re-ordering
    //char reorderedcodeword[129];  //integer*1 reorderedcodeword(128)        !Odd bits first, then even
    char bitseq[145]={0};//  integer*1 bitseq(144)                   !Tone #s, data and sync (values 0-1)
    double xi[865],xq[865];//real*8 xi(864),xq(864)

    if ( first_msk144 )
    {
        first_msk144=false;
        double pi=4.0*atan(1.0);
        //twopi=8.*atan(1.0)
        for (i = 0; i < 12; i++)
        {//do i=1,12
            pp[i]=sin((i-0)*pi/12.0);//pp(i)=sin( (i-1)*pi/12 )
        }
        //qDebug()<<"INIT144";
    }



    if (msk144ms)//mskms
    {
        int i4=0;
        int ik=-1;//0
        int im=-1;//0
        for (i = 0; i < 12; i++)
        {//do i=1,12
            int nn=i4Msg6BitWords[i];
            for (int j = 0; j < 6; j++)
            {//do j=1, 6
                ik=ik+1;
                i4=i4+i4+(1 & (nn >> -(j-5)));//i4=i4+i4+iand(1,ishft(nn,j-6))
                i4=(i4 & 255);
                if (ik==7)//if(ik.eq.8) then
                {
                    im=im+1;
                    //!           if(i4.gt.127) i4=i4-256
                    i1Msg8BitBytes[im]=i4;
                    ik=-1;
                    //qDebug()<<i4<<im;
                }
            }
        }

        uint32_t ihash=nhash(i1Msg8BitBytes,9,146); 
        ihash=(32767 & ihash);//2.76
        ihash=2*(ihash & 32767);   //ihash=2*iand(ihash,32767)                   //!Generate the 8-bit hash
        memcpy(i1hash, (unsigned char*)&ihash, 4);
        i1Msg8BitBytes[9]=i1hash[0];//i1Msg8BitBytes(10)=i1hash(1)                !CRC to byte 10

        int mbit=-1; //0
        char msgbits[81];
        for (i = 0; i < 10; i++)
        {//do i=1, 10
            unsigned char i1=i1Msg8BitBytes[i];
            for (int ibit = 0; ibit < 8; ibit++)
            {//do ibit=1,8
                mbit=mbit+1;
                msgbits[mbit]=(1 & (i1 >> -(ibit-7)));//msgbits(mbit)=iand(1,ishft(i1,ibit-8))
            }
        }
        encode_msk144(msgbits,codeword);//ldpc_encode(msgbits,codeword);
        copy_char_ar(bitseq,0,8,(char*)s8ms,0,1);
        copy_char_ar(bitseq,8,56,codeword,0,1);
        copy_char_ar(bitseq,56,64,(char*)s8ms,0,1);
        copy_char_ar(bitseq,64,144,codeword,48,1);
    }
    else //msk144 v2
    {
        encode_128_90(c77,(bool *)codeword);
        copy_char_ar(bitseq,0,8,(char*)s8,0,1);//bitseq(1:8)=s8
        copy_char_ar(bitseq,8,56,codeword,0,1);//bitseq(9:56)=reorderedcodeword(1:48)
        copy_char_ar(bitseq,56,64,(char*)s8,0,1);//bitseq(57:64)=s8
        copy_char_ar(bitseq,64,144,codeword,48,1);//bitseq(65:144)=reorderedcodeword(49:128)
    }

    //char s8ms[8] = {0,1,0,1,1,0,1,0};
    //char s8[8] = {0,1,1,1,0,0,1,0};
    //char s8r[8]= {1,0,1,1,0,0,0,1};
    /*QString sss;
    for (i = 0; i < 144; i++)
    	sss.append(QString("%1").arg((int)c77[i]));
    qDebug()<<"1mm="<<sss;*/

    for (i = 0; i < 144; i++)
        bitseq[i]=2*bitseq[i]-1;  //bitseq=2*bitseq-1

    copy_double_ar(xq,0,6,pp,6);//xq(1:6)=bitseq(1)*pp(7:12)   //!first bit is mapped to 1st half-symbol on q
    for (i = 0; i < 6; i++)
        xq[i]=xq[i]*bitseq[0];

    for (i = 0; i < 71; i++)
    {//do i=1,71
        int is=(i-0)*12+7;//int is=(i-1)*12+7;
        copy_double_ar(xq,is,is+12,pp,0);//xq(is:is+11)=bitseq(2*i+1)*pp
        for (int j = is; j < is+12; j++)
            xq[j]=xq[j]*bitseq[2*i+1];
    }
    copy_double_ar(xq,863-5,864,pp,0);//xq(864-5:864)=bitseq(1)*pp(1:6)   !last half symbol
    for (int j = 865-5; j < 864; j++)
        xq[j]=xq[j]*bitseq[0];

    for (i = 0; i < 72; i++)
    {//do i=1,72
        int is=(i-0)*12+1; //is=(i-1)*12+1
        copy_double_ar(xi,is,is+12,pp,0);//xi(is:is+11)=bitseq(2*i)*pp
        for (int j = is; j < is+12; j++)
            xi[j]=xi[j]*bitseq[2*i];
    }

    //! Map I and Q  to tones.
    //i4tone=0
    for (i = 0; i < 72; i++)
    {//do i=1,72
        //i4tone(2*i-1)=(bitseq(2*i)*bitseq(2*i-1)+1)/2; 2,4*1,3
        //i4tone(2*i)=-(bitseq(2*i)*bitseq(mod(2*i,144)+1)-1)/2; 2,4*2,4
        i4tone[2*i-0]=(bitseq[2*i+1]*bitseq[2*i-0]+1)/2; //1,3*0,2
        i4tone[2*i+1]=-(bitseq[2*i+1]*bitseq[(int)fmod(2*i+1,144)+1]-1)/2;//1,3*1,3
    }
    //! Flip polarity
    //i4tone=-i4tone+1
    for (i = 0; i < 144; i++)
    {
        i4tone[i]=-i4tone[i]+1;
        //qDebug()<< i4tone[i];
        //qDebug()<< bitseq[i];
    }
}
////MSK144/////////////

////MSK40/////////////
void GenMsk::bpdecode40(double *llr,int maxiterations,char *decoded,int &niterations)
{
    const int N=32;
    const int K=16;
    const int M=N-K; //HV v1,25 ne stava na staro gcc iska po dolnia na4in
    //int M=16;//HV v1,25 ne stava na staro gcc iska po dolnia na4in

    //integer nrw(M)
    //int nrw[M] = {7,6,6,6,6,6,6,6,6,5,6,6,6,6,6,6};//HV v1,25 ne stava na staro gcc iska po dolnia na4in
    int nrw[16] =
        {
            7,6,6,6,6,6,6,6,6,5,6,6,6,6,6,6
        };

    double tov_[N][3];     //real*8 tov(3,N)
    double toc_[M][7];     //real*8 toc(7,M)
    double tanhtoc_[M][7]; //real*8 tanhtoc(7,M)
    double zn[N];          //real*8 zn(N)
    char cw[N];
    int synd[M];
    char codeword[N];
    //double xth=35.0;

    int ncw=3;

    /*for (int j = 0; j < M; j++) //toc=0
    {
        for (int i = 0; i < 7; i++)
            toc_[j][i]=0.0;
    }*/
    for (int j = 0; j < N; j++) //tov=0
    {
        for (int i = 0; i < ncw; i++)
            tov_[j][i]=0.0;
    }
    /*for (int j = 0; j < M; j++) //tanhtoc=0
    {
        for (int i = 0; i < 7; i++)
            tanhtoc_[j][i]=0.0;
    }*/

    //! initialize messages to checks
    for (int j = 0; j < M; j++) //toc=0
    {//do j=1,M
        for (int i = 0; i < nrw[j]; i++) //toc=0
        {//do i=1,nrw(j)
            toc_[j][i]=llr[Nm_msk40_[j][i]-1];//toc(i,j)=llr((Nm(i,j)))
        }
    }

    for (int iter = 0; iter <= maxiterations; iter++)// v1.29 <=
    {//do iter=0,maxiterations

        //! Update bit log likelihood ratios
        for (int i = 0; i < N; i++)
        {//do i=1,N
            //zn(i)=llr(i)+sum(tov(1:ncw,i))
            double sum = 0.0;
            for (int x = 0; x < ncw; x++)
                sum += tov_[i][x];
            zn[i]=llr[i]+sum;

            if (zn[i]>0.0)//cw=0 //where( zn .gt. 0. ) cw=1
                cw[i]=1;
            else
                cw[i]=0;
        }

        //! Check to see if we have a codeword //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
        /*for (int i = 0; i < N; i++)
        {
            if (zn[i]>0.9999)//cw=0 //where( zn .gt. 0. ) cw=1
                cw[i]=1;
            else
                cw[i]=0;
        }*/

        int ncheck=0;
        for (int i = 0; i < M; i++)
        {//do i=1,M
            //synd(i)=sum(cw(Nm(1:nrw(i),i)))
            int sum = 0;
            for (int x = 0; x < nrw[i]; x++)
                sum += (int)cw[Nm_msk40_[i][x]-1];//hv-1
            synd[i]= sum;
            //if( mod(synd(i),2) .ne. 0 ) ncheck=ncheck+1
            if ( fmod(synd[i],2) != 0 ) ncheck++;
        }

        if ( ncheck == 0 ) //then //! we have a codeword - reorder the columns and return it.
        {
            //qDebug()<<"ncheck="<<ncheck;
            niterations=iter;
            for (int i = 0; i < N; i++)//codeword=cw(colorder+1)
                codeword[i]=cw[(int)colorder_msk40[i+0]];

            int cc = 0;
            for (int i = M+0; i < N; i++) //decoded=codeword(M+1:N) ?????
            {
                decoded[cc]=codeword[i];
                cc++;
            }
            //qDebug()<<"niterations="<<niterations;
            return;
        }

        //! Send messages from bits to check nodes
        for (int j = 0; j < M; j++)
        {//do j=1,M
            for (int i = 0; i < nrw[j]; i++)
            {//do i=1,nrw
                int ibj=Nm_msk40_[j][i]-1;//hv-1 ibj=Nm(i,j)
                toc_[j][i]=zn[ibj];// toc(i,j)=zn(ibj)
                for (int kk = 0; kk < ncw; kk++)
                {//do kk=1,ncw //! subtract off what the bit had received from the check
                    if ( Mn_msk40_[ibj][kk]-1 == j )//hv-1 if( Mn(kk,ibj) .eq. j ) then
                        toc_[j][i]=toc_[j][i]-tov_[ibj][kk]; //toc(i,j)=toc(i,j)-tov(kk,ibj)
                }
                //tanhtoc_[j][i]=tanh(-toc_[j][i]/2.0);
            }
        }

        //! send messages from check nodes to variable nodes
        for (int i = 0; i < M; i++)
        {//do i=1,M
            for (int x = 0; x < 7; x++)
                tanhtoc_[i][x]=tanh(-toc_[i][x]/2.0);//tanhtoc(1:7,i)=tanh(-toc(1:7,i)/2)
        }

        for (int j = 0; j < N; j++)
        {//do j=1,N
            for (int i = 0; i < ncw; i++)
            {//do i=1,ncw
                int ichk=Mn_msk40_[j][i]-1; //ichk=Mn(i,j)  //! Mn(:,j) are the checks that include bit j
                //Tmn=product(tanhtoc(1:nrw(ichk),ichk),mask=Nm(1:nrw(ichk),ichk).ne.j)
                double Tmn = 1.0;//mulilay
                for (int z = 0; z < nrw[ichk]; z++)//nrw[i] i=????? 0 to 3 but M=16 mybe is nrw[ichk]
                {
                    if (Nm_msk40_[ichk][z]-1 != j)
                        Tmn = Tmn*tanhtoc_[ichk][z];
                }
                double y;
                platanh(-Tmn,y);//call platanh(-Tmn,y)
                tov_[j][i]=2.0*y;//tov(i,j)=2*y
            }
        }
    }
    niterations=-1;
}

void GenMsk::encode_msk40(char *message,char *codeword)
{
    //! Encode a 16-bit message and return a 32-bit codeword.
    //! The code is a (32,16) regular ldpc code with column weight 3.
    //! The code was generated using the PEG algorithm.
    //! After creating the codeword, the columns are re-ordered according to
    //! "colorder" to make the codeword compatible with the parity-check
    //! matrix stored in Radford Neal's "pchk" format.

    char itmp[32];//integer*1 itmp(128)
    char pchecks[16];//integer*1 pchecks(48)
    /*QString g[16] =
        {"4428", "5a6b", "1b04", "2c12", "60c4", "1071", "be6a", "36dd",
         "c580", "ad9a", "eca2", "7843", "332e", "a685", "5906", "1efe"};*/
    //unsigned short
    int g[16] = {0x4428, 0x5a6b, 0x1b04, 0x2c12, 0x60c4, 0x1071, 0xbe6a, 0x36dd,
                 0xc580, 0xad9a, 0xeca2, 0x7843, 0x332e, 0xa685, 0x5906, 0x1efe};

    if ( first_msk40_enc ) // ! fill the generator matrix
    {
        //gen40=0
        for (int i = 0; i < 16; i++)
        {
            for (int j = 0; j < 16; j++)
                gen40_[i][j]=0;
        }

        for (int i = 0; i < 16; i++)
        {//do i=1,16
            //bool ok;
            //int istr = g[i].toInt(&ok, 16);
            int istr = g[i];
            for (int j = 0; j < 16; j++)
            {//do j=1,16
                //if( btest(g(i),16-j) ) gen40(i,j)=1
                gen40_[j][i]=(1 & (istr >> (15-j)));
            }
        }
        first_msk40_enc=false;
    }

    for (int i = 0; i < 16; i++)
    {//do i=1,16
        int nsum=0;
        for (int j = 0; j < 16; j++)
        {//do j=1,16
            nsum=nsum+message[j]*gen40_[j][i];//nsum=nsum+message(j)*gen40(i,j)
        }
        pchecks[i]=fmod(nsum,2);
    }

    for (int i = 0; i < 16; i++)    //itmp(1:16)=pchecks
        itmp[i]=pchecks[i];

    for (int i = 0; i < 16; i++) //itmp(17:32)=message(1:16)
        itmp[16+i]=message[i];

    for (int i = 0; i < 32; i++)
        codeword[(int)colorder_msk40[i+0]]=itmp[i];//codeword(colorder+1)=itmp(1:32)
}
int GenMsk::hash_msk40(QString s)
{
    int res = 0;
    int len = 37;//22 old
    res = hash(s,len,res);  //qDebug()<<"Calls Hash="<<s<<res;
    res = (res & 4095);     //qDebug()<<"Calls Hash 4095="<<s<<res; //!12-bit hash
    return res;
}
QString GenMsk::genmsk40(QString msg,int ichk,int *itone,int &itype)
{
    QString msgsent;
    QString crpt;
    int i1,i,irpt,ig;//ncodeword;
    char message[16];
    char codeword[32];//integer*1 codeword(24),bitseq(32)
    char bitseq[40];
    int ihash=0;
    QString ttt4;
    //int c = 15;

    /*if ( first_msk40 )
    {
        first_msk40=false;
        //nsym=128;
        //init_ldpc(1);//MSK40 = 1
        //qDebug()<<"INIT40";
    }*/

    itype=-1;
    msgsent="*** bad message ***";
    for (i = 0; i < 234; i++)//Very inportent LZ2HV za6toto dolu obra6ta samo v 1 parity bit !Start with all 0's
        itone[i]=0;

    //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    i1=TPackUnpackMsg.index(msg,'>');//"<C1ALL C2ALL> 73"
    if (i1<8) goto c900; //org->(i1<9)

    msg = RemWSpacesInside(msg);//fmtmsg(msg,iz);
    msg.append(" ");//HV pri men e taka

    crpt=TPackUnpackMsg.qstr_beg_end(msg,i1+2,i1+6);//msg.mid(i1+2,3);  //rpt=msg(i1+2:i1+5)
    //qDebug()<<"MSK32crpt="<<crpt<<msg;
    for (i = 0; i < 16; i++)
    {//do i=0,31
        if (crpt == rpt_msk40[i]) goto c10;
    }
    goto c900;


c10:
    irpt=i;                               //!Report index, 0-15

    //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    if (ichk<10000)
    {
        //QString hashmsg = "                      ";
        //old ihash = hash(qstr_beg_end(msg,1,i1-0),i1-1,ihash);  //ihash = hash(msg(2:i1-1),i1-2,ihash);
        QString hashmsg = TPackUnpackMsg.qstr_beg_end(msg,1,i1-0); //+" "+crpt;//proveri??? hashmsg=msg(2:i1-1)//' '//crpt
        //qDebug()<<"hashmsg="<<hashmsg;
        //ihash = hash(hashmsg,22,ihash);          //call hash(hashmsg,22,ihash)   ihash = hash(msg(2:i1-1),i1-2,ihash);
        //ihash=(ihash & 4095);                       //ihash=iand(ihash,4095)                 !12-bit hash
        ihash = hash_msk40(hashmsg);
        //qDebug()<<"ihashTX="<<ihash;
        ig=16*ihash + irpt;                        //ig=16*ihash + irpt                     !4-bit report
        //qDebug()<<"irpt="<<qstr_beg_end(msg,1,i1-0)<<ihash<<irpt<<i1-1;
    }
    else
        ig=ichk-10000;

    //qDebug()<<"ig="<<ig;
    //ncodeword=ig24_msk32[ig]; //ncodeword=ig24(ig)

    //!write(*,*) 'codeword is: ',ncodeword,'message is: ',ig,'report index: ',irpt,'hash: ',ihash
    //Left Shift 	ISHFT 	ISHFT(N,M) (M > 0) 	<< 	n<<m 	n shifted left by m bits
    //Right Shift 	ISHFT 	ISHFT(N,M) (M < 0) 	>> 	n>>m 	n shifted right by m bits

    for (i = 0; i < 16; i++)
    {//do i=1,16
        //message[i]=(1 & (ig >> -(i-15)));
        message[i]=(1 & (ig >> i));//message(i)=iand(1,ishft(ig,1-i))
        //qDebug()<<"FFF="<<(int)(1 & (ig >> (i)));
        //c--;
    }

    //ldpc_encode(message,codeword);//call ldpc_encode(message,codeword)
    encode_msk40(message,codeword);//call encode_msk40(message,codeword)

    /*for (i = 0; i < 16; i++)
    {
    	//decoded[i]=i;
        ttt4.append(QString("%1").arg((int)message[i]));
        ttt4.append(",");
    }
    qDebug()<<"codeword="<<ttt4;*/


    for (i = 0; i < 8; i++)
        bitseq[i]=s8r[i];//bitseq(1:8)=s8r

    for (i = 0; i < 32; i++)
        bitseq[8+i]=codeword[i]; //bitseq(9:40)=codeword
    for (i = 0; i < 40; i++)
        bitseq[i]=2*bitseq[i]-1;//bitseq=2*bitseq-1

    //! Map I and Q  to tones.
    //itone=0
    for (i = 0; i < 20; i++)
    {//do i=1, 16
        itone[2*i-0]=(bitseq[2*i+1]*bitseq[2*i-0]+1)/2;       //itone(2*i-1)=(bitseq(2*i)*bitseq(2*i-1)+1)/2;
        itone[2*i+1]=-(bitseq[2*i+1]*bitseq[(int)fmod(2*i+1,40)+1]-1)/2;
        //itone(2*i)=-(bitseq(2*i)*bitseq(mod(2*i,32)+1)-1)/2;
        //itone(2*i)=-(bitseq(2*i)*bitseq(mod(2*i,40)+1)-1)/2;
    }

    //! Flip polarity
    //itone=-itone+1
    //i4tone=-i4tone+1
    for (i = 0; i < 40; i++)
    {
        itone[i]=-itone[i]+1;
        //qDebug()<< i4tone[i];
        //qDebug()<< bitseq[i];
    }
    //LZ2HV REAL SEND MSG
    msgsent = TPackUnpackMsg.qstr_beg_end(msg,0,i1+1)+" "+crpt;
    //msgsent=msg;

    itype=7;
c900:
    return msgsent;
}
////MSK40


int GenMsk::genmsk(char *message,double samfac,int *i4tone,bool f_generate,
                   int *t_iwave,double GEN_SAMPLE_RATE,double koef_srate,int ichk,int mode,
                   bool msk144ms)
{
    //bool bcontest = false;
    int nwave = 0;
    int i4Msg6BitWords[13] = {0};     //integer*4 -2,147,483,648 to 2,147,483,647 integer*4 i4Msg6BitWords(13)
    unsigned char i1Msg8BitBytes[13] = {0}; //integer*1 -128 to 127  Character	1 -128	127  //integer*1, target:: i1Msg8BitBytes(13)  !72 bits and zero tail as 8-bit bytes
    //i1Msg8BitBytes[13] =0;
    int x = 0;

    int itype = -1;
    //int nsym=0;                                    //!(72+12+15)*2 = 198
    int nsym_gen=234;
    bool c77[120];
    for (int i = 0; i < 110; ++i)
        c77[i]=0;

    for (x = 0; x < 50; x++)
    {//do i=1,58                               //!Strip leading blanks
        if (message[x]!=' ')
            break;
    }
    //qDebug()<<"org="<<message;
    int c = 0;
    QString msg_short;
    for (int i = x; i < 50; i++)
    {
        message[c]=message[i];
        msg_short.append(message[i]);
        c++;
    }
    //int nmsg = strlen(message);
    //qDebug()<<"new="<<message;
    //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.

    for (int i = 0; i < 234; i++) i4tone[i]=0;//Very inportent LZ2HV za6toto dolu obra6ta samo v 1 parity bit !Start with all 0's
        
    if (message[0]=='<')
    {
        int next_ = msg_short.indexOf(">");//2.00
        if (msg_short.mid(0,next_+1).contains(" "))//2.00
        {
            if (mode == 0)//MSK40 no need HV 2.00
            {
                s_unpack_msg = genmsk40(msg_short,ichk,i4tone,itype);// icheck ne mi trqbva
                if (itype < 0) return 0;
                i4tone[40]=-40; //i4tone(33)=-35
                nsym_gen = 40;//MSK40
            }
            goto c999;
        }
    }

    //qDebug()<<message;
    itype = 0;
    if (msk144ms) TPackUnpackMsg.packmsg(message,i4Msg6BitWords,itype,msk144ms);
    else
    {
    	if (f_generate) TPackUnpackMsg77.reset_save_hash_calls_gen();//2.76.2
        int i3 = -1;
        int n3 = -1;
        TPackUnpackMsg77.pack77(msg_short,i3,n3,c77);
    }
    if (f_generate)
    {
        if (msk144ms)
        {
            char tempp;
            s_unpack_msg = TPackUnpackMsg.unpackmsg(i4Msg6BitWords,tempp,msk144ms);
        }
        else
        {
            bool unpk77_success;
            s_unpack_msg = TPackUnpackMsg77.unpack77(c77,unpk77_success);
        }
    }
    if (mode == 0 || mode == 12) //msk144 msk144ms
    {
        genmsk144(i4Msg6BitWords,i1Msg8BitBytes,i4tone,c77,msk144ms);
        nsym_gen = 144; //MSK144
        goto c999;
    }

c999:
    if (!f_generate) return 0;

    //double dpha=0.0;
    double dt=1.0/(samfac*GEN_SAMPLE_RATE);//samfac
    double nsps=6*koef_srate;//3 5 30simb

    //double f=0.0;
    double df=1000.0;
    double f0=1000.0;// move freq up down
    //double f1=2000.0;

    int k =0;
    double pha=0.0;
    //nsym=234;
    //if (itype_hv==35) nsym=35;  //JTMSK+SH
    //if (itype_hv==144) nsym=144;//MSK144
    //if (itype_hv==40) nsym=144; //MSK40

    for (int m = 0; m < nsym_gen; m++)//+36  234<-real nsym
    {                                   //!Generate iwave
        double f=f0 + i4tone[m]*df;
        double dpha=twopi*f*dt;
        //if (i4tone[m]==0) dpha=twopi*f0*dt;
        //if (i4tone[m]==1) dpha=twopi*f1*dt;
        for (int i = 0; i < nsps; i++)
        {
            pha=pha+dpha;
            if (pha > twopi) pha -= twopi;
            t_iwave[k]=(int)(8380000.0*sin(pha)); //2.70 8380000.0 full=8388607
            k++;
        }
    }

    nwave = k;//k;//omly one msg 100050;
    return nwave;
}
