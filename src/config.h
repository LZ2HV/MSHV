#ifndef __CONFIG_H__
#define __CONFIG_H__

//-------------------------Semantic Versioning 2.0.0---------------------------------------//
#define VERSION_MAJOR 2  //version when you make incompatible API changes
#define VERSION_MINOR 76 //version when you add functionality in a backward compatible manner
#define VERSION_PATCH 7  //version when you make backward compatible bug fixes
//#define VERSION_REVISION 1
//--------------------END--Semantic Versioning 2.0.0--END----------------------------------//

#define STR0(x) #x
#define TOSTR0(x) STR0(x)
#define VER_MS TOSTR0(VERSION_MAJOR) "." TOSTR0(VERSION_MINOR) "." TOSTR0(VERSION_PATCH)
//#define VER_MS "2.65"

#if defined _WIN64_
#define _WIN32_
#endif
#if defined(_WIN32_) && !defined(_WIN64_)  // r008 Only For Testers SP9HWY
#define APP_NAME "MSHV version " VER_MS " 32-bit"// r006 Only For Testers  r008
#define UNICODE //HV FOR WCHAR* LPCSTR
#endif
#if defined _WIN64_
#define APP_NAME "MSHV version " VER_MS " 64-bit"// r002 For Test
#define UNICODE //HV FOR WCHAR* LPCSTR
#endif

#if defined _LINUX_
#define APP_NAME "MSHV version " VER_MS    //c11++ intervaly -> "MSHV version " VER_MS
#endif

#define DATA_HEIGHT         150
#define DATA_WIDTH          780   //1.39 max 30sHdisplays=780 1.35 15-60s=780*2
#define FFT_CUT_LOW_FRQ     11//12 up down graphic
#define FFT_CUT_HI_FRQ      46//
#define DATA_DSP_HEIGHT     DATA_HEIGHT - FFT_CUT_HI_FRQ
#define FFT_END_FRQ         DATA_HEIGHT - (FFT_CUT_HI_FRQ - FFT_CUT_LOW_FRQ)
//#define DATA_VDSP_HEIGHT    256

static const double STAT_FFTW_H30_TIME = 30.0;
static const double STAT_FFTW_V60_TIME = 60.0; //waterfall data for 1 periond and full spead for full waterfall
static const double MAX_OPEN_WAW_TIME = 120.0;

//#define TXT_CODEC_WIN "Windows-1251"

#endif // __CONFIG_H__
