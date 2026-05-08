#include "../../config.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <fcntl.h>
#include <unistd.h>

#include <math.h> //zaradi log hv

#include "mpegsound.h"

/* IOCTL */
/*#ifdef SOUND_VERSION
#define IOCTL(a,b,c)		ioctl(a,b,&c)
#else
#define IOCTL(a,b,c)		(c = ioctl(a,b,c) )
#endif
*/

//#include <QRegExp>//hv
//#include <QtGui>

#if defined _LINUX_
char *Rawplayer::defaultdevice=(char*)"pulse: default";//2.65  "pulse: default"  OSS="/dev/dsp"
int Rawplayer::buffering = 1000;
int Rawplayer::pa_sa_rate = 44100;
int Rawplayer::s_bitpersamplelin = 16;
#endif
#if defined _WIN32_ //Primary Sound Driver or "0"
char *Rawplayer::defaultdevice=(char*)"Primary Sound Driver";
int Rawplayer::buffering = 1000;//1.42 600 to 1000 default
#endif

//bool Rawplayer::defaultdrv=false; //hv
 
//static char *saved_device;
static int buff_hv;

/*******************/
/* Rawplayer class */
/*******************/
// Rawplayer class
#include "../../HvRigControl/HvRigCat/network/network.h"
Rawplayer::~Rawplayer()
{
    //qDebug()<<"Destroy";
    if (ftci)
    {
        _SetTciBuffReset_();
        return;
    }
#if defined _WIN32_
    win_destroy();
#endif
#if defined _LINUX_
    lin_destroy();
#endif 
}
bool Rawplayer::initialize(char *device_name)
{
    audiobuffersize = 8192;//4200;//2.70 audiobuffersize/channel*bits=need to be division without remainder //4096;//1024;
    //saved_device = Rawplayer::defaultdevice;
    buff_hv = Rawplayer::buffering;
    
    QString str_device_name = (QString)device_name;
    if (str_device_name=="TCI Client Output")
    {
        _SetTciBuffReset_();
        ftci = true;
    }
    else ftci = false;

    if (ftci) return true;
#if defined _WIN32_
    return win_initialize(device_name);
#endif
#if defined _LINUX_
    return lin_initialize(device_name,s_bitpersamplelin);
#endif
    return true;
}
/*
void Rawplayer::set_tx_filter_parm(int gain, double f0, double f1, bool type)
{
	THvRawFilter->filter_parm(gain,f0,f1,type);
}
*/
void Rawplayer::abort(void)
{
    //qDebug()<<"abort";
    if (ftci)
    {
        _SetTciBuffReset_();//exeption needed HV
        return;
    }
#if defined _WIN32_
    win_abort();
#endif
}
int Rawplayer::getprocessed(void)
{    
    return 1; //qDebug()<<"gerprocessed";
}
bool Rawplayer::setsoundtype(int stereo,int samplesize,int speed)
{
    rawstereo=stereo;
    rawsamplesize=samplesize;
    rawspeed=speed;
    if (stereo==0) rawchannels = 1;
    if (stereo==1) rawchannels = 2;
#if defined _LINUX_
    rawchannels = 2;
#endif
    //2.70 read all variants
    if (ftci) audiobuffersize = 4096;
    else if (rawsamplesize==16) audiobuffersize = 4096;
    else if (rawsamplesize==24) audiobuffersize = 6000;
    else if (rawsamplesize==32) audiobuffersize = 8192;
    //forcetomono=forceto8=false;
    //qDebug()<<"uuu"<<rawchannels<<rawspeed<<rawsamplesize;
    return resetsoundtype();
}
bool Rawplayer::resetsoundtype(void)
{
    //qDebug()<<"reset_sine_cw";
    if (ftci) return true;
#if defined _WIN32_
    return win_resetsoundtype(rawchannels,rawsamplesize,rawspeed,buff_hv);
#endif
#if defined _LINUX_
    return lin_resetsoundtype();
#endif
}
//void * fast_memcpy(void * to, const void * from, size_t len);
//#define fast_memcpy(a,b,c) memcpy(a,b,c)
//#include <QTime>
//QElapsedTimer ttt;
bool Rawplayer::putblock(void *buffer,int size)
{
    if (ftci)//tci
    {
        return _SetTxAudioTci_((int*)buffer);
    }
    else
    {
#if defined _WIN32_
        return win_putblock(buffer,size);
#endif
#if defined _LINUX_
        return lin_putblock(buffer,size);
#endif
    }
    return 0;
}
int Rawplayer::getblocksize(void)
{
    return audiobuffersize;
}



