/* The algorithms, source code, look-and-feel of WSJT-X and related
 * programs, and protocol specifications for the modes FSK441, FT8, JT4,
 * JT6M, JT9, JT65, JTMS, QRA64, ISCAT, MSK144, are Copyright © 2001-2017
 * by one or more of the following authors: Joseph Taylor, K1JT; Bill
 * Somerville, G4WJS; Steven Franke, K9AN; Nico Palermo, IV3NWV; Greg Beam,
 * KI7MT; Michael Black, W9MDB; Edson Pereira, PY2SDR; Philip Karn, KA9Q;
 * and other members of the WSJT Development Group.
 *
 * MSHV FT4 Generator
 * Rewritten into C++ and modified by Hrisimir Hristov, LZ2HV 2015-2019
 * May be used under the terms of the GNU General Public License (GPL)
 */

#include "gen_ft4.h"
//#include <QtGui>

static bool inicialize_pulse_ft4_trx = false;
static double pulse_ft4_tx[7000];          //    !576*4*3=6912

GenFt4::GenFt4(bool f_dec_gen)//f_dec_gen = dec=true gen=false
{
    TPackUnpackMsg77.initPackUnpack77(f_dec_gen);//f_dec_gen = dec=true gen=false
    genPomFt.initGenPomFt();//first_ft4_enc_174_91 = true;
    twopi=8.0*atan(1.0);   

    if (inicialize_pulse_ft4_trx) return;
    /*int nsps=4*512;//=2048 48000hz
    //! Compute the frequency-smoothing pulse
    for (int i= 0; i < 3*nsps; ++i)//6144
    {//do i=1,3*nsps
        double tt=(i-1.5*nsps)/(double)nsps;
        pulse_ft4_tx[i]=gfsk_pulse(1.0,tt);
    }*/
    //nsps=4*576;//=2304 48000hz
    //nsps*1.5=3456.0
    gen_pulse_gfsk_(pulse_ft4_tx,3456.0,1.0,2304);
    inicialize_pulse_ft4_trx = true;
}
GenFt4::~GenFt4()
{}
/*
void GenFt4::save_hash_call_from_dec(QString c13,int n10,int n12,int n22)
{
    TPackUnpackMsg77.save_hash_call(c13,n10,n12,n22);
}
void GenFt4::save_hash_call_my_his_r1_r2(QString call,int pos)
{
    TPackUnpackMsg77.save_hash_call_my_his_r1_r2(call,pos);
}
*/
void GenFt4::save_hash_call_mam(QStringList ls)
{
    TPackUnpackMsg77.save_hash_call_mam(ls);
}
QString GenFt4::unpack77(bool *c77,bool &unpk77_success)
{
    return TPackUnpackMsg77.unpack77(c77,unpk77_success);
}
void GenFt4::pack77(QString msgs,int &i3,int n3,bool *c77)// for apset v2
{
    TPackUnpackMsg77.pack77(msgs,i3,n3,c77);
}
void GenFt4::split77(QString &msg,int &nwords,/*int *nw,*/QString *w)// for apset v2
{
    TPackUnpackMsg77.split77(msg,nwords,/*int *nw,*/w);
}
void GenFt4::encode174_91(bool *message77,bool *codeword)
{
	genPomFt.encode174_91(message77,codeword);
}
void GenFt4::make_c77_i4tone_codeword(bool *c77,int *i4tone,bool *codeword)//,bool f_gen,bool f_addc
{	
    const int icos4a[4]={0,1,3,2};
    const int icos4b[4]={1,0,2,3};
    const int icos4c[4]={2,3,1,0};
    const int icos4d[4]={3,2,0,1};
    const bool rvec[77]={0,1,0,0,1,0,1,0,0,1,0,1,1,1,1,0,1,0,0,0,1,0,0,1,1,0,1,1,0,
                   1,0,0,1,0,1,1,0,0,0,0,1,0,0,0,1,0,1,0,0,1,1,1,1,0,0,1,0,1,
                   0,1,0,1,0,1,1,0,1,1,1,1,1,0,0,0,1,0,1};
    int itmp[92];//(ND=87)                                  
    //bool codeword[180];//3*58+5 linux subtract error
    //bool *codeword = new bool[180];//3*58+5  full=174             
    bool cc77[180]; // w10 32bit error
    //bool *cc77 = new bool[100]; // w10 32bit error
    for (int i= 0; i < 176; ++i)
    {
    	if (i<77) cc77[i]=c77[i];
    	else cc77[i] = 0;
    	codeword[i] = 0;
   	}               
    for (int i= 0; i < 77; ++i) cc77[i]=fmod(cc77[i]+rvec[i],2); //msgbits=mod(msgbits+rvec,2)    	
    genPomFt.encode174_91(cc77,codeword);
    for (int i= 0; i < 87; ++i)//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    {   // do i=1,ND=87
        int is=codeword[2*i+1]+2*codeword[2*i];//is=codeword(2*i=2,4)+2*codeword(2*i-1)=1,3
        if      (is==1) itmp[i]=1;
        else if (is==2) itmp[i]=3;
        else if (is==3) itmp[i]=2;
        else            itmp[i]=0;      	
    }
    for (int i= 0; i < 4; ++i ) i4tone[i] = icos4a[i];//i4tone(1:4)=icos4a        
    for (int i= 0; i < 29; ++i) i4tone[i+4]=itmp[i];//i4tone(5:33)=itmp(1:29)        
    for (int i= 0; i < 4; ++i ) i4tone[i+33]=icos4b[i];//i4tone(34:37)=icos4b
    for (int i= 0; i < 29; ++i) i4tone[i+37]=itmp[i+29];//i4tone(38:66)=itmp(30:58)
    for (int i= 0; i < 4; ++i ) i4tone[i+66]=icos4c[i];//i4tone(67:70)=icos4c
    for (int i= 0; i < 29; ++i) i4tone[i+70]=itmp[i+58];//i4tone(71:99)=itmp(59:87)
    for (int i= 0; i < 4; ++i ) i4tone[i+99]=icos4d[i];//i4tone(100:103)=icos4d        
}
void GenFt4::make_c77_i4tone(bool *c77,int *i4tone)//,bool f_gen,bool f_addc
{	
    const int icos4a[4]={0,1,3,2};
    const int icos4b[4]={1,0,2,3};
    const int icos4c[4]={2,3,1,0};
    const int icos4d[4]={3,2,0,1};
    const bool rvec[77]={0,1,0,0,1,0,1,0,0,1,0,1,1,1,1,0,1,0,0,0,1,0,0,1,1,0,1,1,0,
                   1,0,0,1,0,1,1,0,0,0,0,1,0,0,0,1,0,1,0,0,1,1,1,1,0,0,1,0,1,
                   0,1,0,1,0,1,1,0,1,1,1,1,1,0,0,0,1,0,1};
    int itmp[92];//(ND=87)               
                   
    bool codeword[180];//3*58+5 linux subtract error
    //bool *codeword = new bool[180];//3*58+5  full=174             
    bool cc77[180]; // w10 32bit error
    //bool *cc77 = new bool[100]; // w10 32bit error
    for (int i= 0; i < 176; ++i)
    {
    	if (i<77) cc77[i]=c77[i];
    	else cc77[i] = 0;
    	codeword[i] = 0;
   	} 
                  
    for (int i= 0; i < 77; ++i) cc77[i]=fmod(cc77[i]+rvec[i],2);  //msgbits=mod(msgbits+rvec,2)
    genPomFt.encode174_91(cc77,codeword);

    for (int i= 0; i < 87; ++i)//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    {   // do i=1,ND=87
        int is=codeword[2*i+1]+2*codeword[2*i];//is=codeword(2*i=2,4)+2*codeword(2*i-1)=1,3
        /*if (is<=1) itmp[i]=is;
        if (is==2) itmp[i]=3;
        if (is==3) itmp[i]=2;*/
        if      (is==1) itmp[i]=1;
        else if (is==2) itmp[i]=3;
        else if (is==3) itmp[i]=2;
        else            itmp[i]=0;      	
    }

    for (int i= 0; i < 4; ++i ) i4tone[i] = icos4a[i];//i4tone(1:4)=icos4a        
    for (int i= 0; i < 29; ++i) i4tone[i+4]=itmp[i];//i4tone(5:33)=itmp(1:29)        
    for (int i= 0; i < 4; ++i ) i4tone[i+33]=icos4b[i];//i4tone(34:37)=icos4b
    for (int i= 0; i < 29; ++i) i4tone[i+37]=itmp[i+29];//i4tone(38:66)=itmp(30:58)
    for (int i= 0; i < 4; ++i ) i4tone[i+66]=icos4c[i];//i4tone(67:70)=icos4c
    for (int i= 0; i < 29; ++i) i4tone[i+70]=itmp[i+58];//i4tone(71:99)=itmp(59:87)
    for (int i= 0; i < 4; ++i ) i4tone[i+99]=icos4d[i];//i4tone(100:103)=icos4d        
    //delete cc77;
    //delete codeword;
}
/*void GenFt4::genft4rx(char *message_in,int cmsg,int *i4tone,int i3bit)
{    
    bool *c77 = new bool[120];  //bawi -> bool c77[120];// fictive
    QString msg_short = format_msg(message_in,cmsg);
    int n3 = 0;
    
    for (int z =0; z<100; ++z)
        c77[z]=0;
    TPackUnpackMsg77.pack77(msg_short,i3bit,n3,c77);// call pack77(msg,i3,n3,c77)
    make_c77_i4tone(c77,i4tone);//,false,false
        
    delete c77;
}*/
int GenFt4::genft4(QString str_mam,int *t_iwave,double GEN_SAMPLE_RATE,double f_tx)//int i3bit
{
    int nwave = 0;
    int k1 =0;
    int i3bit = 0;
    int n3 = 0;
    int i4tone[120];
    bool c77[140];//2.68 130 to 140
    //bool *c77 = new bool[130]; 
    double *wave = new double[242200];//241920
    double *dphi = new double[242200];//241920    //real dphi(0:240000-1)
    s_unpack_msg = "";//reset

    //QString str_mam = format_msg(message_in,cmsg);
    //str_mam = "CQ LZ2HV KN23#SP9HWY LZ2HV KN23#R5WM LZ2HV R+06";
    QStringList SlMsgs;
    if (str_mam.contains("#"))
    {
        SlMsgs = str_mam.split("#");
    }
    else SlMsgs << str_mam;
    int nslots = SlMsgs.count();

    TPackUnpackMsg77.reset_save_hash_calls_gen();//2.76.2
    int nsym=103;
    int nsps=4*576;//=2304
    nwave=(nsym+2)*nsps;//241920
    for (int islot = 0; islot < nslots; ++islot)
    {
        ///////////Multi Answer protocol fox /////////////
        QString msg_short = SlMsgs.at(islot); //qDebug()<<msg_short; 
  
      	for (int z =0; z<100; ++z) c77[z]=0;
    	TPackUnpackMsg77.pack77(msg_short,i3bit,n3,c77);// call pack77(msg,i3,n3,c77)
        
        bool unpk77_success;
        QString tms = TPackUnpackMsg77.unpack77(c77,unpk77_success);
        s_unpack_msg.append(tms);
        if (islot < nslots - 1)
            s_unpack_msg.append("#");
        
        make_c77_i4tone(c77,i4tone);

        double f0=f_tx + (100.0*(double)islot);//100.0 Hz slots diff

        if (f0<100.0 ) f0=100.0;// min 100hz
        if (f0>5000.0) f0=5000.0;//max 5000.0
        //////////////////////// GFSK MODULATOR ////////////////////////////////////////////

        double hmod=1.0;
        double dphi_peak=twopi*hmod/(double)nsps;
        double dt=1.0/GEN_SAMPLE_RATE;
        //! Compute the smoothed frequency waveform.
        //! Length = (nsym+2)*nsps samples, zero-padded (nsym+2)*nsps TX=215040 RX=53760
        for (int i= 0; i < 242000; ++i)//max tx=241920
            dphi[i]=0.0;
        for (int j= 0; j < nsym; ++j)
        {
            int ib=j*nsps;
            for (int i= 0; i < 3*nsps; ++i) dphi[i+ib] += dphi_peak*pulse_ft4_tx[i]*(double)i4tone[j];//6912                
        }
		//qDebug()<<"dphi="<<(nsym-1)*nsps+3*nsps-1;
        double ofs = twopi*f0*dt;
        double phi=0.0;
        k1=0;
        for (int j= 0; j < nwave; ++j)//tx=241920
        {
        	if (islot==0) wave[k1]=0.0;//linux problem                
            wave[k1]+=sin(phi);
            phi=fmod(phi+dphi[j]+ofs,twopi);
            k1++;
        }
    }
    //qDebug()<<"0 k1="<<nwave<<s_unpack_msg;
    
    for (int i = 0; i < nsps; ++i) wave[i]*=(1.0-cos(twopi*(double)i/(2.0*nsps)))/2.0;        
    int k2=(nsym+1)*nsps+1; //=2047 before stop
    for (int i = 0; i < nsps; ++i) wave[i+k2]*=(1.0+cos(twopi*(double)i/(2.0*nsps)))/2.0;//i+k1-nsps        
	//qDebug()<<"nsamp="<<k2+nsps-1<<k1;
    if (nslots<1) nslots=1;// no div by zero      

    for (int z = 0; z < k1; ++z) t_iwave[z]=(int)(8380000.0*(wave[z]/(double)nslots));//2.70 8380000.0 full=8388607
        
    delete [] wave;
    delete [] dphi;
    //delete c77;
    //////////////////////// END GFSK MODULATOR ////////////////////////////////////////////

    for (int z = 0; z < 144000 ; ++z)  //+3s duration=5.04s   duration=4.48s
    {
        t_iwave[k1] = 0;
        k1++;
    }
    //nwave = k1;//k;//k;//omly one msg 100050;*/
    return k1;
}

