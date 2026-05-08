/* The algorithms, source code, look-and-feel of WSJT-X and related
 * programs, and protocol specifications for the modes FSK441, FT8, JT4,
 * JT6M, JT9, JT65, JTMS, QRA64, ISCAT, MSK144, are Copyright © 2001-2017
 * by one or more of the following authors: Joseph Taylor, K1JT; Bill
 * Somerville, G4WJS; Steven Franke, K9AN; Nico Palermo, IV3NWV; Greg Beam,
 * KI7MT; Michael Black, W9MDB; Edson Pereira, PY2SDR; Philip Karn, KA9Q;
 * and other members of the WSJT Development Group.
 *
 * MSHV FT8 Generator
 * Rewritten into C++ and modified by Hrisimir Hristov, LZ2HV 2015-2017
 * May be used under the terms of the GNU General Public License (GPL)
 */

#include "gen_ft8.h"
//#include <QtGui>

static bool inicialize_pulse_ft8_trx = false;
static double pulse_ft8_tx[23060];          //    !1920*4*3=23040

GenFt8::GenFt8(bool f_dec_gen)//f_dec_gen = dec=true gen=false
{   
    TPackUnpackMsg77.initPackUnpack77(f_dec_gen);//f_dec_gen = dec=true gen=false
    genPomFt.initGenPomFt();//first_ft8_enc_174_91 = true;
    twopi=8.0*atan(1.0);   

    if (inicialize_pulse_ft8_trx) return;
    /*int nsps=4*1920;//48000hz=7680
    //! Compute the frequency-smoothing pulse
    for (int i= 0; i < 3*nsps; ++i)//=23040
    {//do i=1,3*nsps
        double tt=(i-1.5*nsps)/(double)nsps;
        pulse_ft8_tx[i]=gfsk_pulse(2.0,tt);//tx=2.0
    }*/
    gen_pulse_gfsk_(pulse_ft8_tx,11520.0,2.0,7680);
    inicialize_pulse_ft8_trx = true;
}
GenFt8::~GenFt8()
{}
void GenFt8::save_hash_call_from_dec(QString c13,int n10,int n12,int n22)
{
    TPackUnpackMsg77.save_hash_call(c13,n10,n12,n22);
}
void GenFt8::save_hash_call_my_his_r1_r2(QString call,int pos)
{
    TPackUnpackMsg77.save_hash_call_my_his_r1_r2(call,pos);
}
/*
void GenFt8::save_hash_call_mam(QStringList ls)
{
    TPackUnpackMsg77.save_hash_call_mam(ls);
}
*/
QString GenFt8::unpack77(bool *c77,bool &unpk77_success)
{
    return TPackUnpackMsg77.unpack77(c77,unpk77_success);
}
void GenFt8::pack77(QString msgs,int &i3,int n3,bool *c77)// for apset v2
{
    TPackUnpackMsg77.pack77(msgs,i3,n3,c77);
}
void GenFt8::split77(QString &msg,int &nwords,/*int *nw,*/QString *w)// for apset v2
{
    TPackUnpackMsg77.split77(msg,nwords,/*int *nw,*/w);
}
/*
short crc10_(unsigned char const * data, int length)
{
    return boost::augmented_crc<10, TRUNCATED_POLYNOMIAL10>(data, length);
}
short crc12_(unsigned char const * data, int length)
{
    return boost::augmented_crc<12, TRUNCATED_POLYNOMIAL12>(data, length);
}
bool crc12_check_(unsigned char const * data, int length)
{
    return !boost::augmented_crc<12, TRUNCATED_POLYNOMIAL12>(data, length);
}
short GenFt8::crc10(unsigned char const * data, int length)
{
    return crc10_(data,length);
}
short GenFt8::crc12(unsigned char const * data, int length)
{
    return crc12_(data,length);
}
*/
void GenFt8::make_c77_i4tone_codeword(bool *c77,int *i4tone,bool *codeword)//,bool f_gen,bool f_addc
{
    const int icos7_77 [7]={3,1,4,0,6,5,2}; 
    const int graymap77[8]={0,1,3,2,5,6,4,7}; 
    
    //bool codeword[180];//3*58+5   linux subtract error
	//bool *codeword = new bool[180];//3*58+5   full=174
    bool cc77[180]; // w10 32bit error
	//bool *cc77 = new bool[180]; // w10 32bit error
    for (int i= 0; i < 176; ++i)
    {
    	if (i<77) cc77[i]=c77[i];
    	else cc77[i] = 0;
    	codeword[i] = 0;
   	} 

    genPomFt.encode174_91(cc77,codeword); 

    for (int i= 0; i < 100; ++i) i4tone[i] = 0; //! Message structure: S7 D29 S7 D29 S7         
    for (int i= 0; i < 7; ++i) i4tone[i]=icos7_77[i]; //itone(1:7)=icos7 i4tone[79]; realno
    for (int i= 0; i < 7; ++i) i4tone[(35+1)+i]=icos7_77[i]; //i4tone(36+1:36+7)=icos7
    for (int i= 0; i < 7; ++i) i4tone[(79-7)+i]=icos7_77[i];  //itone(NN-6:NN)=icos7  NN=79
    int k=7-1;
    
    for (int j= 0; j < 58; ++j)
    {
    	int i=3*j;
    	int idx = codeword[i]*4 + codeword[i+1]*2 + codeword[i+2];
    	int idxx = 0;
    	if      (idx==1) idxx=1;
    	else if (idx==2) idxx=2;
    	else if (idx==3) idxx=3;
    	else if (idx==4) idxx=4;
    	else if (idx==5) idxx=5;
    	else if (idx==6) idxx=6;
    	else if (idx==7) idxx=7;
    	else 			 idxx=0;
    	k++;
    	if (j==29) k=k+7;
    	i4tone[k]=graymap77[idxx];
   	}
}
void GenFt8::make_c77_i4tone(bool *c77,int *i4tone)//,bool f_gen,bool f_addc
{
    const int icos7_77 [7]={3,1,4,0,6,5,2}; 
    const int graymap77[8]={0,1,3,2,5,6,4,7}; 
    
    bool codeword[180];//3*58+5   linux subtract error
	//bool *codeword = new bool[180];//3*58+5   full=174
    bool cc77[180]; // w10 32bit error
	//bool *cc77 = new bool[180]; // w10 32bit error
    for (int i= 0; i < 176; ++i)
    {
    	if (i<77) cc77[i]=c77[i];
    	else cc77[i] = 0;
    	codeword[i] = 0;
   	} 

    genPomFt.encode174_91(cc77,codeword); 

    for (int i= 0; i < 100; ++i) i4tone[i] = 0; //! Message structure: S7 D29 S7 D29 S7         
    for (int i= 0; i < 7; ++i) i4tone[i]=icos7_77[i]; //i4tone[79]; realno
    for (int i= 0; i < 7; ++i) i4tone[(35+1)+i]=icos7_77[i]; //i4tone(36+1:36+7)=icos7
    for (int i= 0; i < 7; ++i) i4tone[(79-7)+i]=icos7_77[i];  //itone(NN-6:NN)=icos7  NN=79
    int k=7-1;
    
    for (int j= 0; j < 58; ++j)
    {
    	int i=3*j;
    	int idx = codeword[i]*4 + codeword[i+1]*2 + codeword[i+2];
    	int idxx = 0;
    	if      (idx==1) idxx=1;
    	else if (idx==2) idxx=2;
    	else if (idx==3) idxx=3;
    	else if (idx==4) idxx=4;
    	else if (idx==5) idxx=5;
    	else if (idx==6) idxx=6;
    	else if (idx==7) idxx=7;
    	else 			 idxx=0;
    	k++;
    	if (j==29) k=k+7;
    	i4tone[k]=graymap77[idxx];
   	}     
    //delete cc77;
    //delete codeword;
}
void GenFt8::pack77_make_c77_i4tone(QString msg,int *itone)
{
	int i3=-1;
    int n3=-1;
    bool c77[100];
    for (int z = 0; z < 90; ++z) c77[z]=false;
	TPackUnpackMsg77.pack77(msg,i3,n3,c77);
	make_c77_i4tone(c77,itone);
}
/*void GenFt8::pack77_make_c77_i4tone(QString msg,int &i3,int n3,bool *c77,int *itone)
{
	TPackUnpackMsg77.pack77(msg,i3,n3,c77);
	make_c77_i4tone(c77,itone);
}*/                
/*void GenFt8::genft8rx(char *message_in,int cmsg,int *i4tone,int i3bit)
{
    bool c77[120];// fictive
    //bool *c77 = new bool[120];
    QString msg_short = format_msg(message_in,cmsg);
    make_c77_i4tone(msg_short,c77,i4tone,i3bit);//,false,false
    //delete c77;
}*/
int GenFt8::genft8(QString str_mam,int *t_iwave,double samp_rate,double f_tx,QString s0,QString sotp)//,int i3bit
{ 
    int nwave = 0;
    int k1 = 0;
    int i3bit = 0;
    int n3 = 0;
    int i4tone[120];
    bool c77[140];//2.68 130 to 140
    //bool *c77 = new bool[130];
    double *d_iwave = new double[864000];//15s= 720000 18s= 864000   alltx=614400
    double *dphi = new double[623080];//need tobe here hv  real dphi=79*7680=606720 + 7680*2=622080 
    s_unpack_msg = "";//reset

    //QString str_mam = format_msg(message_in,cmsg);
    
    QStringList SlMsgs; //qDebug()<<str_mam<<str_mam.count(); qDebug()<<"----------------";   
    if (str_mam.contains("#")) SlMsgs = str_mam.split("#");
    else SlMsgs << str_mam;
    
    int nslots = SlMsgs.count();

    QStringList l0 = s0.split("#"); //qDebug()<<"L0="<<l0;
    QStringList lotpk = sotp.split("#"); //qDebug()<<nslots<<l0.at(6).toInt();
    if (l0.at(5)=="1" && l0.at(4)=="1")
    {
    	int limits = l0.at(6).toInt();
    	if (limits==1 || nslots<limits)//2.76sf for mamdx no TX OTP if slots is full
    	{
    		QString ckey = genPomFt.foxOTPcode(lotpk.at(1));
    		if (ckey!="000000")
    		{
    			QString str0 = l0.at(3)+".";     	   
        		str0.append(ckey); 
        		SlMsgs.prepend(str0);
        		nslots++;    		
   			}    		
   		}  	
   	}
	TPackUnpackMsg77.reset_save_hash_calls_gen();//2.76.2
    int nsym=79;
    int nsps=4*1920;// for tx=48000->4*1920=7680 ; for rx=12000->1920
    nwave=nsym*nsps;//=606720
    for (int islot = 0; islot < nslots; ++islot)
    {
        ///////////Multi Answer protocol fox /////////////
        QString msg_short = SlMsgs.at(islot);
        
        //make_c77_i4tone(msg_short,c77,i4tone,i3bit);//,true,f_addc
        //int n3 = 0;
        for (int z =0; z<100; ++z) c77[z]=0;
    	TPackUnpackMsg77.pack77(msg_short,i3bit,n3,c77);// call pack77(msg,i3,n3,c77)
        
        /*QString sss; // CQ type test to 29
        for (int ii = 0; ii < 29; ++ii) sss.append(QString("%1").arg((int)c77[ii])+",");
        qDebug()<<sss;*/
        
        // no problem to be here 2.20 becouse -> encode174_91 add +14bit affter 76bit
        bool unpk77_success;
        QString tms = TPackUnpackMsg77.unpack77(c77,unpk77_success);
        s_unpack_msg.append(tms);
        if (islot < nslots - 1) s_unpack_msg.append("#"); 
            
        make_c77_i4tone(c77,i4tone);    

        double f0=f_tx + (60.0*(double)islot);//60.0 Hz slots diff, move freq up down  f0=nfreq + fstep*(n-1)

        if (f0<100.0 ) f0=100.0;// min 100hz
        if (f0>5000.0) f0=5000.0;//max 5000.0
        //////////////////////// GFSK MODULATOR ////////////////////////////////////////////
        double hmod=1.0;
        double dt=1.0/samp_rate;
        double dphi_peak=twopi*hmod/(double)nsps;
        for (int i= 0; i < 622100; ++i) dphi[i]=0.0;//max tx=622080            
        for (int j= 0; j < nsym; ++j)//nsym=79   nsps=7680
        {
            int ib=j*nsps;
            for (int i= 0; i < 3*nsps; ++i) dphi[i+ib] += dphi_peak*pulse_ft8_tx[i]*(double)i4tone[j];//23040                
        }

        //! Add dummy symbols at beginning and end with tone values equal to 1st and last symbol, respectively
        //dphi(0:2*nsps-1)=dphi(0:2*nsps-1)+dphi_peak*itone(1)*pulse(nsps+1:3*nsps)
        //dphi(nsym*nsps:(nsym+2)*nsps-1)=dphi(nsym*nsps:(nsym+2)*nsps-1)+dphi_peak*itone(nsym)*pulse(1:2*nsps)
        int bgn =nsym*nsps;
        for (int i= 0; i < 2*nsps; ++i)//15360
        {
            dphi[i]+=dphi_peak*i4tone[0]*pulse_ft8_tx[i+nsps];
            dphi[i+bgn]+=dphi_peak*i4tone[nsym-1]*pulse_ft8_tx[i];
        }
		//qDebug()<<"dphi="<<bgn+2*nsps-1;
        double ofs = twopi*f0*dt;
        double phi=0.0;
        k1=0;
        for (int j= nsps; j < nsps+nwave; ++j)//nsps=7680 nwave=606720   alltx=614400
        {
            if (islot==0) d_iwave[k1]=0.0;//linux problem               
            d_iwave[k1]+=sin(phi);
            phi=fmod(phi+dphi[j]+ofs,twopi);
            k1++;
        } //qDebug()<<"2dphi="<<nsps+nsps+nwave;
    }

    int nramp=(int)((double)nsps/8.0);
    for (int i = 0; i < nramp; ++i) d_iwave[i]*=(1.0-cos(twopi*(double)i/(2.0*nramp)))/2.0;        
    int k2=nsym*nsps-nramp+1; //k1=nsym*nsps-nramp+1   k2=(nsym+1)*nsps+1; int k2=k1-nramp;
    for (int i = 0; i < nramp; ++i) d_iwave[i+k2]*=(1.0+cos(twopi*(double)i/(2.0*nramp)))/2.0;//i+k1-nsps        
    //qDebug()<<"nsamp="<<k2+nramp-1<<k1;
    if (nslots<1) nslots=1;// no div by zero
 
    for (int z = 0; z < k1; ++z) t_iwave[z]=(int)(8380000.0*(d_iwave[z]/(double)nslots));//2.70 24-bit=8380000.0 full=8388607 //28-bit=134080000
        
    delete [] d_iwave;
    delete [] dphi;
    //delete c77;

    for (int z = 0; z < 192000 ; ++z)  //+4s duration=12.64s
    {
        t_iwave[k1] = 0;
        k1++;
    }
    //delete d_iwave;//inportent to delete here or in the up return
    //nwave = k1;//k;//k;//omly one msg 100050;
    return k1;
}
