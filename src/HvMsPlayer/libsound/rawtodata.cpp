#include "../../config.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <fcntl.h>
#include <unistd.h>

//#include <math.h> //zaradi log hv

#include "mpegsound.h"

/* IOCTL */
#ifdef SOUND_VERSION
#define IOCTL(a,b,c)		ioctl(a,b,&c)
#else
#define IOCTL(a,b,c)		(c = ioctl(a,b,c) )
#endif

//#include <QRegExp>//hv
//#include <QtGui>

//char *Rawtodata::defaultdevice=(char*)"/dev/dsp";
//bool Rawtodata::defaultdrv=false; //hv
//int Rawtodata::buffering = 88200;//=500ms
//static char *saved_device;
//static bool saved_drv = false;
//static int buff_hv;

//static double vol_win = 0.0001;

/*******************/
/* Rawtodata class */
/*******************/
// Rawtodata class
Rawtodata::~Rawtodata()
{}
bool Rawtodata::initialize(char*)
{
    audiobuffersize = 4200;   //2.70 audiobuffersize/channel*bits=need to be division without remainder //4096
    //audiobuffersize = 1080; //2.70 small size no make large file after rewrite
    //saved_device = Rawtodata::defaultdevice;
    //buff_hv = Rawtodata::buffering;
    return true;
}
int Rawtodata::getprocessed(void)
{
    return 1;
}
bool Rawtodata::setsoundtype(int stereo,int samplesize,int speed)
{
    // pauza e prez tazi (procedura + resetsoundtype(void)) a stop e samo resetsoundtype(void)
    rawstereo=stereo;
    rawsamplesize=samplesize;
    rawspeed=speed;
    if (stereo==0) rawchannels = 1;
    if (stereo==1) rawchannels = 2;
    //2.70 read all variants
    //qDebug()<<rawformatt<<rawstereo<<samplesize<<rawspeed;
    return true;
}
/*
bool Rawtodata::resetsoundtype(void)
{
//qDebug()<<"LINressss";


    return win_resetsoundtype(saved_device,rawchannels,rawsamplesize,rawspeed,false,buff_hv);
}
*/
bool Rawtodata::putblock(void *buffer,int size)//2.70
{
    unsigned char *buf = (unsigned char*)buffer;//2.70 importanat -> unsigned char*
    int ca = 0;
    int bits = 2;
    if (rawsamplesize==24) bits = 3;
    if (rawsamplesize==32 || rawsamplesize==33) bits = 4;
    for (int i = 0; i <size; i+=bits*rawchannels)//2.70 for stereo ->*rawchannels
    {
        if (rawsamplesize==33)//33bit fictive secret to recognize float32
        {
            union {
                unsigned char a[4];
                float f;
            } U;
            U.a[0]=buf[i+0];
            U.a[1]=buf[i+1];
            U.a[2]=buf[i+2];
            U.a[3]=buf[i+3];
            intArr[ca]=(int)(U.f*8380000.0);//2.71 8380000.0 full=8388607
        }
        else
        {
            if (bits==2) intArr[ca] = (buf[i] << 16) + (buf[i+1] << 24);
            if (bits==3) intArr[ca] = (buf[i] << 8) + (buf[i+1] << 16) + (buf[i+2] << 24);
            if (bits==4) intArr[ca] =  buf[i] + (buf[i+1] << 8) + (buf[i+2] << 16) + (buf[i+3] << 24);
            intArr[ca] = (intArr[ca] >> 8);//to 24-bit
        }
        ca++;
    } //qDebug()<<rawformatt;
    emit SentData(intArr,ca,true);
    return true;
}
int Rawtodata::getblocksize(void)
{
    return audiobuffersize;
}




