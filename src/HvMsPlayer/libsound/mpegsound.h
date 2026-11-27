/* MPEG Sound library

   (C) 1997 by Woo-jae Jung */
// Mpegsound.h
//   This is typeset for functions in MPEG Sound library.
// Now, it's for only linux-pc-?86
// Updated For MSHV by Hrisimir Hristov, LZ2HV 2015
/************************************/
/* Inlcude default library packages */
/************************************/

#include "../../config.h"
#include "config.h"

#include <stdio.h>
#include <sys/types.h>
#include <string.h>

#if defined _LINUX_
#ifdef PTHREADEDMPEG
#ifdef HAVE_PTHREAD_H
#include <pthread.h>
#else
#ifdef HAVE_PTHREAD_MIT_PTHREAD_H
#include <pthread/mit/pthread.h>
#endif
#endif
#endif
#endif

#ifndef _L__SOUND__
#define _L__SOUND__

/****************/
/* Sound Errors */
/****************/
// General error
#define SOUND_ERROR_OK                0
#define SOUND_ERROR_FINISH           -1

// Device error (for player)
#define SOUND_ERROR_DEVOPENFAIL       1
#define SOUND_ERROR_DEVBUSY           2
#define SOUND_ERROR_DEVBADBUFFERSIZE  3
#define SOUND_ERROR_DEVCTRLERROR      4

// Sound file (for reader)
#define SOUND_ERROR_FILEOPENFAIL      5
#define SOUND_ERROR_FILEREADFAIL      6

// Network
#define SOUND_ERROR_UNKNOWNPROXY      7
#define SOUND_ERROR_UNKNOWNHOST       8
#define SOUND_ERROR_SOCKET            9
#define SOUND_ERROR_CONNECT          10
#define SOUND_ERROR_FDOPEN           11
#define SOUND_ERROR_HTTPFAIL         12
#define SOUND_ERROR_HTTPWRITEFAIL    13
#define SOUND_ERROR_TOOMANYRELOC     14

// Miscellneous (for translater)
#define SOUND_ERROR_MEMORYNOTENOUGH  15
#define SOUND_ERROR_EOF              16
#define SOUND_ERROR_BAD              17

#define SOUND_ERROR_THREADFAIL       18

#define SOUND_ERROR_UNKNOWN          19
//#define SOUND_ERROR_BADHEADER        20 //ogg


/**************************/
/* Define values for MPEG */
/**************************/
#define SCALEBLOCK     12
#define CALCBUFFERSIZE 512
#define MAXSUBBAND     32
#define MAXCHANNEL     2
#define MAXTABLE       2
#define SCALE          32768
#define MAXSCALE       (SCALE-1)
#define MINSCALE       (-SCALE)
#define RAWDATASIZE    (2*2*32*SSLIMIT)

#define LS 0
#define RS 1

#define SSLIMIT      18
#define SBLIMIT      32

#define WINDOWSIZE   4096

// Huffmancode
#define HTN 34 

/*******************************************/
/* Define values for Microsoft WAVE format */
/*******************************************/
/*#define RIFF		0x46464952
#define WAVE		0x45564157
#define FMT		    0x20746D66
#define DATA		0x61746164
#define PCM_CODE	1
#define WAVE_MONO	1
#define WAVE_STEREO	2
#define FRAMESIZE (2048) //hv zaradi wav formata hv be6e 4096 no zaradi skalata v msec e 2048

#define MODE_MONO   0
#define MODE_STEREO 1
*/

#ifdef WORDS_BIGENDIAN

#define RIFF            0x52494646
#define WAVE            0x57415645
#define FMT             0x666D7420
#define DATA            0x64617461
#define PCM_CODE        (1 << 8)
#define WAVE_MONO       (1 << 8)
#define WAVE_STEREO     (2 << 8)
#define IEEE_FLOAT		(3 << 8)

#define MODE_MONO   0
#define MODE_STEREO 1

#else

#define RIFF		0x46464952
#define WAVE		0x45564157
#define FMT		    0x20746D66
#define DATA		0x61746164
#define PCM_CODE	1
#define WAVE_MONO	1
#define WAVE_STEREO	2
#define IEEE_FLOAT	3

#define MODE_MONO   0
#define MODE_STEREO 1

#endif

//#define FRAMESIZE (2048) //hv zaradi wav formata hv be6e 4096 no zaradi skalata v msec e 2048

#include <stdint.h>
typedef uint32_t u_int32_t;
typedef uint16_t u_int16_t;

enum soundtype { NONE, RAW, WAV };

typedef struct _waveheader
{
    u_int32_t     main_chunk;  // 'RIFF'
    u_int32_t     length;      // filelen
    u_int32_t     chunk_type;  // 'WAVE'

    u_int32_t     sub_chunk;   // 'fmt '
    u_int32_t     sc_len;      // length of sub_chunk, =16
    u_int16_t     format;      // should be 1 for PCM-code
    u_int16_t     modus;       // 1 Mono, 2 Stereo
    u_int32_t     sample_fq;   // frequence of sample
    u_int32_t     byte_p_sec;
    u_int16_t     byte_p_spl;  // samplesize; 1 or 2 bytes
    u_int16_t     bit_p_spl;   // 8, 12 or 16 bit

    u_int32_t     data_chunk;  // 'data'
    u_int32_t     data_length; // samplecount
}
WAVEHEADER;

/*********************************/
/* Sound input interface classes */
/*********************************/
// Superclass for Inputbitstream // Yet, Temporary
class Soundinputstream
{
public:
    Soundinputstream();
    virtual ~Soundinputstream();

    static Soundinputstream *hopen(char *filename,int *errcode);

    int geterrorcode(void)
    {
        return __errorcode;
    };

    virtual bool open(char *filename)              =0;
    virtual int  getbytedirect(void)               =0;
    virtual bool _readbuffer(char *buffer,int size)=0;
    virtual bool eof(void)                         =0;
    virtual int  getblock(char *buffer,int size)   =0;

    virtual int  getsize(void)                     =0;
    virtual int  getposition(void)                 =0;
    virtual void setposition(int pos)              =0;

protected:
    void seterrorcode(int errorcode)
    {
        __errorcode=errorcode;
    };

private:
    int __errorcode;
};

// Inputstream from file
class Soundinputstreamfromfile : public Soundinputstream
{
public:
    Soundinputstreamfromfile()
    {
        fp=NULL;
        size=0;
    };
    ~Soundinputstreamfromfile();

    bool open(char *filename);
    bool _readbuffer(char *buffer,int bytes);
    int  getbytedirect(void);
    bool eof(void);
    int  getblock(char *buffer,int size);

    int  getsize(void);
    int  getposition(void);
    void setposition(int pos);

private:
    FILE *fp;
    int  size;
};

class Soundplayer_wr
{
public:
  Soundplayer_wr() {__errorcode=SOUND_ERROR_OK;};
  virtual ~Soundplayer_wr();

  virtual void abort(void);
  virtual bool setsoundtype(int stereo,int samplesize,int speed)=0;
  virtual void set8bitmode()=0;
  virtual bool resetsoundtype(void);
  virtual void releasedevice(void) = 0;
  virtual bool attachdevice(void) = 0;
  virtual bool putblock(void *buffer,int size)                  =0;
  virtual int  putblock_nt(void *buffer, int size)		=0;
  virtual int  getblocksize(void);
  int geterrorcode(void) {return __errorcode;};

protected:
  bool seterrorcode(int errorno) {__errorcode=errorno; return false;};

private:
  int  __errorcode;
};

class Rawtofile_wr : public Soundplayer_wr
{
public:
  ~Rawtofile_wr();

  static Rawtofile_wr *opendevice(char *filename);

  bool setsoundtype(int stereo,int samplesize,int speed);
  void set8bitmode() { want8bit = 1; }
  bool setfiletype(enum soundtype);
  bool putblock(void *buffer,int size);
  int putblock_nt(void *buffer,int size);
  void releasedevice(void) {};
  bool attachdevice(void) { return true; };
private:
  Rawtofile_wr();
  int init_putblock;
  int rawstereo,rawsamplesize,rawspeed,want8bit;
  soundtype filetype;
  WAVEHEADER hdr;
};

/**********************************/
/* Sound player interface classes */
/**********************************/
// Superclass for player
class Soundplayer
{
public:
    Soundplayer()
    {
        __errorcode=SOUND_ERROR_OK;
    };
    virtual ~Soundplayer();

    virtual bool initialize(char *filename)                       =0;
    virtual void abort(void);
    virtual int  getprocessed(void);
    virtual bool setsoundtype(int stereo,int samplesize,int speed)=0;
    virtual bool resetsoundtype(void);
    virtual bool putblock(void *buffer,int size)                  =0;
    virtual int  getblocksize(void);
    int geterrorcode(void)
    {
        return __errorcode;
    };
    virtual bool gettci(void)
    {
        return false;
    };

protected:
    bool seterrorcode(int errorno)
    {
        __errorcode=errorno;
        return false;
    };

private:
    int  __errorcode;
};

#if defined _WIN32_
    #include "../../Hv_Lib_DirectX90c/dsound.h"
#endif
#if defined _LINUX_
// Class for playing raw data
#include <alsa/asoundlib.h>//hv for alsa sound
#endif

//static LPDIRECTSOUNDBUFFER hdsbuf = NULL;
//#include <windows.h>

#include <QObject> // hv
//#include "../../HvEq/eqbiquadhv.h" //hv

//////////////////////////////////////////////////////////////////////////////////////////
class Rawtodata : public QObject, public Soundplayer //hv
{
    Q_OBJECT // hv
public:
    ~Rawtodata();

    bool initialize(char*);

    int  getprocessed(void);
    bool setsoundtype(int stereo,int samplesize,int speed);

    bool putblock(void *buffer,int size);
    int  getblocksize(void);
    void setquota(int q)
    {
        quota=q;
    };
    int  getquota(void)
    {
        return quota;
    };

signals://hv
    void SentData(int*, int, bool);//1.27 psk rep fopen bool true

private:
	int intArr[8192]; //2.70 =audiobuffersize=4096 slot emmit
    int  quota;
    int audiobuffersize;
    //bool forcetomono,forceto8;
    int  rawstereo,rawsamplesize,rawspeed,rawchannels,rawformatt;//for alsa rawchannels hv

};
////////////////////////////////////////////////////////////////////////////////////////////
//#include "HvRawFilter/hvrawfilter.h" //hv

class Rawplayer : public Soundplayer //hv   public QObject, 
{
    //Q_OBJECT // hv
public:
    ~Rawplayer();

    bool initialize(char *drv_name);
    void abort(void);
    int  getprocessed(void);
    bool setsoundtype(int stereo,int samplesize,int speed);
    bool resetsoundtype(void);
    bool putblock(void *buffer,int size);
    int  getblocksize(void);
    void setquota(int q)
    {
        quota=q;
    };
    int getquota(void)
    {
        return quota;
    };
    bool gettci(void)
    {
        return ftci;//2.58 need for GenMessage
    };

    static char *defaultdevice;
    //static bool defaultdrv;//hv
    static int buffering;
    
#if defined _LINUX_ 
    static int pa_sa_rate;  
    static int s_bitpersamplelin; 
    //static int  setvolume_lin(int volume);
#endif    

//signals:
 
private: 
	bool ftci;
    int  rawbuffersize;
    int  audiohandle,audiobuffersize;
    int  rawstereo,rawsamplesize,rawspeed,rawchannels;//for alsa rawchannels hv
    //bool forcetomono,forceto8;
    int  quota;

#if defined _WIN32_
	//WaveOut
    HWAVEOUT m_hOut;
    int s_dwInstance;
    int m_dwAudioOutId;
    int wo_read_wr_offset;
	//DirectSound
    LPDIRECTSOUNDBUFFER lpDirectSoundBuffer;
    DSBUFFERDESC playbackBuff;
    LPDIRECTSOUND lpDirectSound;   
    int ds_read_wr_offset;
	//Windows Sound ALL
    bool win_putblock(void *buffer,int size);
    bool win_initialize(char *device_name);
    bool win_resetsoundtype(int channels,int samplesize,int speed, int buffer);
    void win_abort();
    void win_destroy();
#endif
#if defined _LINUX_
    snd_pcm_t *playback_handle;//hv
    void lin_destroy();
    bool lin_initialize(char *device_name,int);
    bool lin_resetsoundtype();
    bool lin_putblock(void *buffer,int size);
    snd_pcm_format_t check_formats(snd_pcm_t *h, snd_pcm_hw_params_t *hware);
#endif
    int buffer_size;
    int write_position;
    //int save_channels,save_samplesize,save_speed, save_buffer;

};

/*********************************/
/* Data format converter classes */
/*********************************/
// Class for converting wave format to raw format
#include <QWidget>
class Wavetoraw : public QWidget  // for nessage box
{
	//Q_OBJECT
public:
    Wavetoraw(Soundinputstream *loader,Soundplayer *player/*,QWidget * parent = 0*/);
    ~Wavetoraw();

    bool initialize(int srate);
    void setforcetomono(bool flag)
    {
        forcetomonoflag=flag;
    };
    bool run(void);

    int  getfrequency(void)    const
    {
        return speed;
    };
    bool isstereo(void)        const
    {
        return stereo;
    };
    int  getsamplesize(void)   const
    {
        return samplesize;
    };

    int  geterrorcode(void)    const
    {
        return __errorcode;
    };

    int  gettotallength(void)  const
    {
        return size/pcmsize;
    };
    int  getcurrentpoint(void) const
    {
        return currentpoint/pcmsize;
    };
    //void setcurrentpoint(int p);

private:
    int s_max_period;
    int s_srate;
    int  __errorcode;
    void seterrorcode(int errorcode)
    {
        __errorcode=errorcode;
    };
    bool forcetomonoflag;
    Soundinputstream *loader;
    Soundplayer *player;
    bool initialized;
    char *buffer;
    int  buffersize;
    int  samplesize,speed,stereo;
    int  currentpoint,size;
    int  pcmsize;
    bool testwave(char *buffer);
};

//#include <complex.h> // gnu++11
//#define complex		_Complex

#include "HvGenMsk/genmesage_msk.h"
#include "HvGen65/gen65.h"
#include "HvGenFt8/gen_ft8.h"
#include "HvGenFt8/gen_sfox.h"//2.76
#include "HvGenFt4/gen_ft4.h"
#include "HvGenFt2/gen_ft2.h"
#include "HvGenQ65/gen_q65.h" 
#include "HvRawFilter/hvrawfilter.h" //hv
#define IWAVE_BUFFER  (48000*140)  //2.53=2min.20sec  62s*48000=2976000  60s*48000=2880000

class GenMessage //: public QObject
{
	//Q_OBJECT
public:
    GenMessage(Soundplayer *player);
    ~GenMessage();

    //int  getblock2(char *buffer,int size){ return 0; };
    bool initialize(char*,int mod_iden,double tx_freq,int,int,bool,QString,QString);//,int &ntxslot QString mygridl,
    void setforcetomono(bool flag)
    {
        forcetomonoflag=flag;
    };
    bool run(void);
    int  getfrequency(void)    const
    {
        return speed;
    };
    bool isstereo(void)        const
    {
        return stereo;
    };
    int  getsamplesize(void)   const
    {
        return samplesize;
    };
    int  geterrorcode(void)    const
    {
        return __errorcode;
    };

    GenMsk *TGenMsk;      
    Gen65 *TGen65;   
    GenFt8 *TGenFt8;
    GenSFox *TGenSFox;//2.76
    GenFt4 *TGenFt4;
    GenFt2 *TGenFt2;
    GenQ65 *TGenQ65;
      
    static int  setvolume_all(int volume);
    void mute_hv(bool);//hv

//signals:
    //void SendUnpackMsg(QString msg);

private:
	bool ftci;
	bool f_rawf;
	bool f_gens;
    HvRawFilter *THvRawFilter;//hv
    bool mute_raw;//hv
    QString s_MyGridLoc; //for " R " in msg 1.31   
    int s_mode;
    bool s_msf;//2.76
    double twopi;
    int  __errorcode;
    void seterrorcode(int errorcode)
    {
        __errorcode=errorcode;
    };
    bool forcetomonoflag;

    //Soundinputstream *loader;
    Soundplayer *player;
        
    bool initialized;
    unsigned char buffer_gmsgc[16384]; //8192<-max 
    int buffer_gmsgs[16384];		   //tci=4096<-max 
    int  buffersize_gmsg;
    int  samplesize,speed,stereo;
    int  size; //currentpoint,
    int  pcmsize;
    bool testwave();
    double GEN_SAMPLE_RATE;
    double koef_srate;
    int BITS_PER_SAMPLE;
    int abc441(char*,int);
    int genms(char*,double,int);
	///////////// JT6M /////////////////////////////////////    
    int gen6m(char*,double);
    int gentone(double *x,int n,int k,double samfac);
	///////////// JT6M ///////////////////////////////////// 
	///////////// ISCAT /////////////////////////////////////    
    int geniscat(char *msg,int nmsg,int mode4,double samfac);
	///////////// ISCAT /////////////////////////////////////
    //double complex cwave[30*48000];// max rate app is 44100 30seconds
    short itone_s[4096]; //=4096    max = 1291 iscat
    int iwave_count;
    int iwave[IWAVE_BUFFER]; //=5952000    2486948  62s*48000=2976000  60s*48000=2880000
    int iwave_size; 
    bool getblock_raws(int*,int);
    bool getblock_rawc(unsigned char*,int); 
    QString format_msg(char *message_in, int cmsg);  
    int hvmsgen(char*,int mod_ident,double tx_freq,int,QString,QString);//,int &ntxslot ,QString
    //char *s_msg;
    //int s_period_t;
  
};

#endif