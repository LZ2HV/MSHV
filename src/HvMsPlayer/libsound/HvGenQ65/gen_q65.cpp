/* The algorithms, source code, look-and-feel of WSJT-X and related
 * programs, and protocol specifications for the modes FSK441, FT8, JT4,
 * JT6M, JT9, JT65, JTMS, QRA64, ISCAT, MSK144, are Copyright © 2001-2017
 * by one or more of the following authors: Joseph Taylor, K1JT; Bill
 * Somerville, G4WJS; Steven Franke, K9AN; Nico Palermo, IV3NWV; Greg Beam,
 * KI7MT; Michael Black, W9MDB; Edson Pereira, PY2SDR; Philip Karn, KA9Q;
 * and other members of the WSJT Development Group.
 *
 * MSHV Q65 Generator
 * Rewritten into C++ and modified by Hrisimir Hristov, LZ2HV 2015-2021
 * May be used under the terms of the GNU General Public License (GPL)
 */

#include "gen_q65.h"
//#include <QtGui>

GenQ65::GenQ65(bool f_dec_gen)//f_dec_gen = dec=true gen=false
{
    TPackUnpackMsg77.initPackUnpack77(f_dec_gen);//f_dec_gen = dec=true gen=false
    //twopi=8.0*atan(1.0);
}
GenQ65::~GenQ65()
{}
QString GenQ65::unpack77(bool *c77,bool &unpk77_success)
{
	return TPackUnpackMsg77.unpack77(c77,unpk77_success);
}
void GenQ65::pack77(QString msgs,int &i3,int n3,bool *c77)// for apset v2
{
    TPackUnpackMsg77.pack77(msgs,i3,n3,c77);
}
/*QString GenQ65::format_msg(char *message_in, int cmsg)
{
    QString out;
    //skip ' ' in begining
    int c1 = 0;
    for (c1 = 0; c1 < 20; ++c1)
    {
        if (message_in[c1]!=' ')
            break;
    }
    int c2 = 0;
    for (int ii = c1; ii < cmsg; ++ii)
    {
        message_in[c2]=message_in[ii];
        c2++;
    }
    //end skip ' ' in begining
    int end;
    for (end = cmsg-1; end >= 0; --end)
    {
        if (message_in[end]!=' ')
            break;
    }
    for (int z = 0; z < end+1; ++z)
        out.append(message_in[z]);
    return out;
    QString out = "";
    int beg,end;
    for (beg = 0; beg < cmsg; ++beg)
    {
        if (message_in[beg]!=' ') break;
    }
    for (end = cmsg-1; end >= 0; --end)
    {
        if (message_in[end]!=' ') break;
    }
    for (int z = beg; z < end+1; ++z) out.append(message_in[z]); 
    return out; 
}*/
int GenQ65::BinToInt32(bool*a,int b_a,int bits_sz)
{
    int k = 0;
    for (int i = b_a; i < bits_sz; ++i)
    {
        k <<= 1;
        k |= a[i];
    }
    return k;
}
void GenQ65::genq65itone(QString msg0,int *itone,bool unpck)
{
    int i3 = 0;
    int n3 = 0;
    bool c77[130];
    int dgen[13+10];//13
    int sent[63+10];//63
    static const bool m73[15] =
        {
            1,1,1,1,1,1,0,1,0,0,1,0,0,1,1
        }
        ;//=32403 ?
    static const int isync[86] =
        {
            1,9,12,13,15,22,23,26,27,33,35,38,46,50,55,60,62,66,69,74,76,85
        }
        ;//22
    for (int i = 0; i < 100; ++i) c77[i]=false;
    
    TPackUnpackMsg77.pack77(msg0,i3,n3,c77);
    int ng15 = BinToInt32(c77,59,74);
    if (ng15 == 32373) //!Message is RR73 ??
    {
        for (int i = 0; i < 15; ++i) c77[59+i] = m73[i];
    }
    bool unpk77_success;
    if (unpck) s_unpack_msg = TPackUnpackMsg77.unpack77(c77,unpk77_success);
    int z = 0;
    for (int i = 0; i < 12; ++i)
    {
        dgen[i] = BinToInt32(c77,z,z+6);
        z += 6;
    }
    dgen[12] = BinToInt32(c77,72,77);
    dgen[12]=2*dgen[12];//dgen(13)=2*dgen(13) !Convert 77-bit to 78-bit payload
    //if(ichk.eq.1) go to 999  !Return if checking only ??
    q65S.q65_enc(dgen,sent);    //!Encode message, dgen(1:13) ==> sent(1:63)

    z = 0;
    int x = 0;
    for (int i = 0; i < 85; ++i)
    {//do i=1,85
        if (i==isync[z]-1/* && z<22*/)
        {
            itone[i]=0; //!Insert sync symbol at tone 0
            z++;  //!Index for next sync symbol
        }
        else// if (k<63)
        {
            int pp = 0;
            if (unpck) pp = 1;
            itone[i]=sent[x] + pp;    //!Q65 symbol=0 is transmitted at tone 1, etc.
            x++;
        }
    }
    //qDebug()<<x<<z;
    /*if (msg0 == "CQ LZ2HV KN23")
    {
        QString sss;
        for (int i = 0; i < 63; ++i)
        {
            sss.append(QString("%1").arg((int)sent[i]));
        }
        if (unpck) qDebug()<<"TX"<<msg0<<sss;
        else qDebug()<<"RX"<<msg0<<sss;
    }
    qDebug()<<"22="<<z<<"63="<<x;*/
}
int GenQ65::genq65(QString message_in,int *t_iwave,double GEN_SAMPLE_RATE,double f_tx,
                   int mq65,int period_t)
{
	//qDebug()<<f_tx;
    int k1 = 0;
    int itone[90];//85               //!QRA64 uses only 84
    double twopi=8.0*atan(1.0);
    QString msg0 = message_in;//format_msg(message_in,cmsg);

    TPackUnpackMsg77.reset_save_hash_calls_gen();//2.76.2
    genq65itone(msg0,itone,true);

    ///////////////// generator wave //////////////////////////////////c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    int nsps=1800;
    if (period_t== 30) nsps=3600;
    if (period_t== 60) nsps=7200;
    if (period_t==120) nsps=16000;
    if (period_t==300) nsps=41472;
    int nsps4=4*nsps;                           //48000 Hz sampling
    int nsym=85;
    //int nwave=(nsym+2)*nsps4;
    double baud=GEN_SAMPLE_RATE/(double)nsps4;
    double dt=1.0/GEN_SAMPLE_RATE;
    double hb = baud*(double)mq65;
    //double hb = baud*1.0;

    double phi=0.0;
    for (int j = 0; j < nsym; ++j)
    {//do j=1,nsym
        double freq = f_tx + hb*(double)itone[j];
        double dphi = twopi*freq*dt;
        for (int i = 0; i < nsps4; ++i)
        {//do i=1,nsps
            t_iwave[k1]=(int)(8380000.0*sin(phi)); //2.70 8380000.0 full=8388607
            phi+=dphi;
            if (phi>twopi) phi-=twopi;
            k1++;
        }
    }
    int to_end = (period_t*GEN_SAMPLE_RATE) - k1;  //qDebug()<<"Time="<<(double)to_end/GEN_SAMPLE_RATE;
    for (int z = 0; z < to_end; ++z)
    {
        t_iwave[k1] = 0;
        k1++;
    }

    ///////////////// generator /////////////////////////////////////

    //qDebug()<<"mmmm="<<s_unpack_msg<<ng15;
    /*QString sss;
    for (int i = 0; i < 14; ++i)
    {
        //if (i>58 && i<74) sss.append(",");
        sss.append(QString("%1").arg((int)dgen[i]));
    }
    qDebug()<<sss;
    sss.clear();
    for (int i = 0; i < 63; ++i)
    {
        //if (i>58 && i<74) sss.append(",");
        sss.append(QString("%1").arg((int)sent[i]));
    }
    qDebug()<<sss;
    QString sss;
    sss.clear();
    for (int i = 0; i < 85; ++i)
    {
        //if (i>58 && i<74) sss.append(",");
        sss.append(QString("%1").arg((int)itone[i]));
    }
    qDebug()<<k1<<"Time="<<(double)k1/48000.0;*/
    return k1;
}
