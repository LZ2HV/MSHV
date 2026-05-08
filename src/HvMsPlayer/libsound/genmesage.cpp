/* MSHV GenMessage
 * Copyright 2015 Hrisimir Hristov, LZ2HV
 * May be used under the terms of the GNU General Public License (GPL)
 */
#include "mpegsound.h"
#include <math.h>

//static const double OUT_SAMPLE_RATE =11025.0;//no inposible hv rawfilter need to be off
//static const double OUT_SAMPLE_RATE =22050.0;
static const double OUT_SAMPLE_RATE_44100 =44100.0;
//static const double OUT_SAMPLE_RATE =12000.0;
static const double OUT_SAMPLE_RATE_48000 =48000.0;//no inposible hv
//static const double OUT_SAMPLE_RATE_48000 =12000.0;//no inposible hv
//static const double OUT_SAMPLE_RATE =96000.0;//no inposible hv

//#include <QtGui>

static const double ORG_SAMPLE_RATE_11025 = 11025.0;
static const double ORG_SAMPLE_RATE_12000 = 12000.0;
static double vol_win = 1.0;

//#define DEB_LIMI0
#if defined DEB_LIMI0
static int max_0 = 0;
static int max_1 = 0;
#endif 
GenMessage::GenMessage(Soundplayer *player)
	: buffer_gmsgc {0},	//8192
	buffer_gmsgs {0},	//8192
	itone_s {0},		//4092
	iwave {0}			//2976000
{
#if defined DEB_LIMI0	
	max_0 = 0;
	max_1 = 0;
#endif	
	ftci = false;
    f_rawf = false;
    f_gens = false;
    TGenMsk=NULL;
    TGen65=NULL;
    TGenFt8=NULL;
    TGenSFox=NULL;
    TGenFt4=NULL;
    TGenFt2=NULL;
    TGenQ65=NULL;
    THvRawFilter=NULL;
    mute_raw = true;
    forcetomonoflag = false;
    samplesize=0;
    speed=0;
    stereo=0;
    size=0;
    pcmsize=0;
    __errorcode=SOUND_ERROR_OK;
    initialized=false;
    //buffer_gmsg=NULL;
    buffersize_gmsg=0;
    //for (int i= 0; i < 2976000; ++i)
    //iwave[i]=0;
    iwave_size=0;
    iwave_count=0;
    //itone_s=NULL;
    twopi=8.0*atan(1.0);
    //////////////////////////////////////////////////////
    s_mode = 2;//HV important set to default mode fsk441
    s_msf = false;
    GEN_SAMPLE_RATE = OUT_SAMPLE_RATE_44100;//HV important set to default mode fsk441 sample rate
    koef_srate = GEN_SAMPLE_RATE/ORG_SAMPLE_RATE_11025;//HV important set to default mode fsk441 sample rate
    //////////////////////////////////////////////////////
    BITS_PER_SAMPLE =16;
    //App_path = path;
    this->player=player;
}
GenMessage::~GenMessage()
{
    if ((s_mode == 0 || s_mode == 12) && f_gens) delete TGenMsk;//msk144        
    else if ((s_mode == 7 || s_mode == 8 || s_mode == 9) && f_gens) delete TGen65;//jt65abc        
    else if (s_mode == 11 && f_gens)
    {
    	if (s_msf) delete TGenSFox;
    	else delete TGenFt8;//ft8    	
   	}        
    else if (s_mode == 13 && f_gens) delete TGenFt4;//ft4  
    else if (s_mode == 18 && f_gens) delete TGenFt2;//ft4      
    else if ((s_mode == 14 || s_mode == 15 || s_mode == 16|| s_mode == 17) && f_gens) delete TGenQ65;//q65
        
    if (f_gens) f_gens = false;

    if (f_rawf)
    {
        delete THvRawFilter;
        f_rawf = false;
    }
    //qDebug()<<"Delete GenMessage"<<s_mode<<f_gens;
}
int GenMessage::setvolume_all(int volume)
{
    vol_win = (4.615583-log(101-volume))/4.615583;  // to4nost 0.0001002;
    return 0;
}
bool GenMessage::initialize(char *msg,int mod_ident,double tx_freq,int period_t,int bpsampl,bool msf,QString sfmta,QString otptk)//,int &ntxslot QString mygridl
{
    mute_raw = true;//hv za da ne proswirva v na4aloto
    s_mode = mod_ident;
    s_msf = msf;
    BITS_PER_SAMPLE = bpsampl;

    if (s_mode>0 && s_mode<7) //2.65
    {
        GEN_SAMPLE_RATE = OUT_SAMPLE_RATE_44100;
        koef_srate = GEN_SAMPLE_RATE/ORG_SAMPLE_RATE_11025;
    }
    else
    {
        if (s_mode == 0 || s_mode == 12) TGenMsk = new GenMsk(false);
        else if (s_mode == 7 || s_mode == 8 || s_mode == 9) TGen65 = new Gen65();//jt65abc
        else if (s_mode == 11) 
        {
        	if (s_msf) TGenSFox = new GenSFox(false);
        	else TGenFt8 = new GenFt8(false);//ft8        	
       	}
        else if (s_mode == 13) TGenFt4 = new GenFt4(false);//ft4
        else if (s_mode == 18) TGenFt2 = new GenFt2(false);//ft2
        else if (s_mode == 14 || s_mode == 15 || s_mode == 16|| s_mode == 17) TGenQ65 = new GenQ65(false);//q65
            
        if (s_mode != 10) f_gens = true; //pi4
        GEN_SAMPLE_RATE = OUT_SAMPLE_RATE_48000;
        koef_srate = GEN_SAMPLE_RATE/ORG_SAMPLE_RATE_12000;
    }    	

	ftci=player->gettci();
	QString tmp = (QString)msg+"   ";
#if defined DEB_LIMI0
		printf("MsgSend= %s Count=%d\n",qPrintable(tmp),tmp.count());
#endif
	bool tune = false;
	int all_gain = 46;//2.76.4 0ld=46 forFT8=48 true -> LSH+HSH /* Low shelf filter and High shelf filter*/ for TX gine_all 45 zaradi windows	
	if (tmp[0]=='@') tune = true;	
	if (s_mode==11 || s_mode==13 || s_mode==18 || tune) all_gain = 47;//if (((s_mode>-1 && s_mode<7) || s_mode==12) && !tune) all_gain = 46;
	THvRawFilter = new HvRawFilter(all_gain,100.0,8000.0,true);
#if defined DEB_LIMI0
		printf("Filt====================== AllGain=%d Tune=%d \n",all_gain,tune);
#endif
    f_rawf = true;

    buffersize_gmsg=player->getblocksize();
    //iwave_size=hvmsgen(s_msg,s_modexxx,tx_freq,s_period_t);
    iwave_size=hvmsgen(msg,mod_ident,tx_freq,period_t,sfmta,otptk);
    //qDebug()<<"iwave_size="<<sizeof(short)<<((int)GEN_SAMPLE_RATE * 60)*sizeof(short);
    //qDebug()<<"GenMessage initialize="<<iwave_size<<buffersize_gmsg<<ftci;
    return true;
}
void GenMessage::mute_hv(bool mute)//hv
{
    mute_raw = mute;
}
bool GenMessage::getblock_raws(int *buf,int size_p)//stereo out for Windows and Linux
{
    for (int i = 0; i < size_p; i+=2)
    {        
        int raw_dsp = iwave[iwave_count];
#if defined DEB_LIMI0        
        if (!mute_raw && max_0<raw_dsp)
        {
        	max_0=raw_dsp; printf("   Tci MaxIn=====%d\n",max_0);
       	}        
#endif        
        if (mute_raw)
        {
            raw_dsp = raw_dsp*0.00001;
        }
        else
        {
            double data_l = THvRawFilter->band_l(raw_dsp);
            raw_dsp = (int)data_l;
            raw_dsp = raw_dsp*vol_win;
        }
#if defined DEB_LIMI0        
        if (!mute_raw && max_1<raw_dsp)
        {
        	max_1=raw_dsp; printf("Tci MaxOut=%.1f\n",((float)max_1*100.0)/8388607.0);
       	}         
#endif        
        buf[i]   = raw_dsp; //to int for tci HV
        buf[i+1] = raw_dsp;
        iwave_count++;
        if (iwave_count>=iwave_size) iwave_count = 0;
    }
    return true;
} //QElapsedTimer ttt;
bool GenMessage::getblock_rawc(unsigned char *buf,int size_p)//stereo out for Windows and Linux
{
	int bits = 2; //qDebug()<<size_p<<ttt.elapsed(); ttt.start();
	if (samplesize == 24) bits = 3;
	if (samplesize == 32) bits = 4; 
    for (int i = 0; i < size_p; i+=2*bits)
    {        
        int raw_dsp = iwave[iwave_count];
#if defined DEB_LIMI0         
        if (!mute_raw && max_0<raw_dsp)
        {
        	max_0=raw_dsp; printf("   Sb MaxIn=====%d\n",max_0);
       	}
#endif        
        if (mute_raw)
        {
            raw_dsp = raw_dsp*0.00001;
        }
        else
        {
            double data_l = THvRawFilter->band_l(raw_dsp);
            raw_dsp = (int)data_l;
            raw_dsp = raw_dsp*vol_win;
        }  
#if defined DEB_LIMI0         
        if (!mute_raw && max_1<raw_dsp)
        {
        	max_1=raw_dsp; printf("Sb MaxOut=%.1f\n",((float)max_1*100.0)/8388607.0);
       	}      
#endif        
        if (bits == 4)//little-endian-form, to char HV  
        {      
        	raw_dsp *= 256;
        	buf[i]   = (raw_dsp & 0xff);
        	buf[i+1] = ((raw_dsp >>  8) & 0xff);
        	buf[i+2] = ((raw_dsp >> 16) & 0xff);
        	buf[i+3] = ((raw_dsp >> 24) & 0xff);
        	buf[i+4] = (raw_dsp & 0xff);
        	buf[i+5] = ((raw_dsp >>  8) & 0xff); 
        	buf[i+6] = ((raw_dsp >> 16) & 0xff);  
        	buf[i+7] = ((raw_dsp >> 24) & 0xff);    	
       	}    
        else if (bits == 3)
        {      
        	buf[i]   = (raw_dsp & 0xff);
        	buf[i+1] = ((raw_dsp >>  8) & 0xff);
        	buf[i+2] = ((raw_dsp >> 16) & 0xff);
        	buf[i+3] = (raw_dsp & 0xff);
        	buf[i+4] = ((raw_dsp >>  8) & 0xff); 
        	buf[i+5] = ((raw_dsp >> 16) & 0xff);      	
       	}
       	else
       	{        	 
        	raw_dsp /= 256; //raw_dsp = raw_dsp >> 8;      
        	buf[i]   = (raw_dsp & 0xff);
        	buf[i+1] = ((raw_dsp >> 8) & 0xff);
        	buf[i+2] = (raw_dsp & 0xff);
        	buf[i+3] = ((raw_dsp >> 8) & 0xff);       		
      	}
        iwave_count++;
        if (iwave_count>=iwave_size) iwave_count = 0;
    }
    return true;
}
bool GenMessage::run(void)
{
    if (initialized)
    {
        if (ftci)//tci
        {
        	bool f=getblock_raws(buffer_gmsgs,buffersize_gmsg);
        	if (!f) return false;
        	if (player->putblock(buffer_gmsgs,buffersize_gmsg)==false) return false;        	        	        	
       	}
		else
		{
        	bool f=getblock_rawc(buffer_gmsgc,buffersize_gmsg);
        	if (!f) return false;
        	if (player->putblock(buffer_gmsgc,buffersize_gmsg)==false) return false;			
		}
    }
    else
    {
        if (!testwave())return false;
        THvRawFilter->set_rate((double)speed); //printf("TX Rate=%d",speed);
        if (player->setsoundtype(stereo,samplesize,speed)==false) return false;
        buffersize_gmsg=player->getblocksize();//2.70 get actual size
        initialized=true;
    }
    return true;
}
bool GenMessage::testwave()
{
    stereo=1;//0 mono 1 stere
    samplesize=(int)BITS_PER_SAMPLE;
    speed=(int)GEN_SAMPLE_RATE;
    size =(int)10000;// triabwa da e goliamo
    pcmsize=1;
    if (stereo==1)pcmsize*=2;
    if (samplesize==16)pcmsize*=2;
    if (samplesize==24)pcmsize*=3;
    if (samplesize==32)pcmsize*=4;
    return true;
}
/*
void GenMessage::setcurrentpoint(int p)
{
    if (p*pcmsize>size)currentpoint=size;
    else currentpoint=p*pcmsize;
}
*/

