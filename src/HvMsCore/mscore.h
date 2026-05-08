/* MSHV
 *
 * By Hrisimir Hristov - LZ2HV
 * May be used under the terms of the GNU General Public License (GPL)
 */

#ifndef MSCORE_H
#define MSCORE_H

#include "../config.h"

//#include <QtGui>

#include <QObject>
#include <math.h>	//los
//#include <stdlib.h>	//abs
//#include <unistd.h> 
//#include <iostream>
#include <unistd.h>//usleep x86_64
//using namespace std;

#include <QTimer>

//#include "../../../DirectX90c/include/dsound.h"
#if defined _WIN32_
    #include "../Hv_Lib_DirectX90c/dsound.h"
#endif
#if defined _LINUX_
#include <alsa/asoundlib.h>
#include <pthread.h> //	pthread_create, pthread_mutex_unlock
#endif

#define SC_SIZE_L			128
#define CLOSED_TEXT		"The sound device is closed."
//#include <complex.h> // hv ima na dolu deklaracii / Use native C99 complex type for fftw3
#define SAMP_BUFFER_SIZE	80000		//Linux size of arrays used to capture samples
#define CLIP32			2147483647
#define CLIP16			32767

#define MAXDSBUFERTIME_W 15 //2.49 =10sec  old=7 seconds buffer time
#define CHANNELS_W 2 //2 chanels
#define BITPERSAMPLE_MAX 32 //2.70  16bit
//   (192000 * (( CHANNELS_W * BITPERSAMPLE_W ) / 8)) * MAXDSBUFERTIME_W;
#define SAMP_BUFFER_SIZE_WIN	((96000 * (( CHANNELS_W * BITPERSAMPLE_MAX ) / 8)) * MAXDSBUFERTIME_W + 32) // 5376000 za 192000hz i 7 sec MAXDSBUFERTIME ->vista e vinovna
#define SAMP_BUFFER_PRE_BUFF (12000 * 15) //samplerate * seconds

//#define RMS_BUF_SIZE 20 // triabwa da e po goliamo ot max rms_points v TSettingsGeoRad
//otide v config.h #define POINTS_IN1_SCAN 620//384 //i v graphdisprad.cpp saved_points_in_1scan = 384;
//vazno->POINTS_IN1_SCAN = 384 ot geocore; i time_1scan = 26.81 ot graphdisprad->se glasi to4nostta
//vazno->dannite sa ot hardware
// kolko to4ki ima w edin skan s prodalzitelnost 8.7ms na 44100HZ sample rate;

#include <complex.h> // gnu++11 c++11
#define complex		_Complex

#include "../Hv_Lib_fftw/fftw3.h"

#include "../HvMsPlayer/libsound/HvRawFilter/hvrawfilter.h" //hv

//#include "msplayerhv.h"

/*
#include <QThread>
#include <QMutex>
#include <QMutexLocker>
class ThreadRefr : public QThread
{
    Q_OBJECT
signals:
    void refresh();
private:
    //QMutex m_mutex;
    void run()
    {
        while (1)
        {
            //QMutexLocker locker(&m_mutex);
            msleep(5);
            refresh();           
            //usleep(10);
        }
    }
};
*/

extern int _GetRxAudioReadTci_();
extern void _SetRxAudioTci_(int *,int,int);

class MsCore : public QObject
{
    Q_OBJECT
public:
    MsCore();
    ~MsCore();

    //bool is_select_sound_device;
    bool GetSta_Sto()
    {
        return sta_sto_flag;
    };
    void SetStr_Sto(bool);
    void close_sound();
    void SetupSettings(QString,int,int,int,int);//QString,
    void SetMode(int mod_iden);
    void setVDFftwStartStopFreq(int beg,int end);
    void SetDecBusy(bool);
    void SetVDispSpeed(int);
    //void SetAdleDisplay_VD(bool);

public slots:
    void ReciveData(int*,int,bool); //1.27 psk rep fopen bool true false no file open
    void SetInLevel(int);
    void FastResetSoundCardIn();
    //void SetVDispSpeed(int);
    //void SetDecBusy(bool);

signals:
    void Sed_SMeter(int);
    void Set_Graph(double*,int);
    void Set_Raw(int*,int,bool); //1.27 psk rep fopen bool true false no file open
    void Refresh_time();

private slots:
    void Refresh_t();
    void FastResetSoundCardIn_p();

private:
	void ResampleAndFilter(int*,int);
	bool ftci;//tci
	void ReadTci();//tci
    bool dec_busy;
    QTimer *timer_fr_sound;

    int s_data_dsp_height;
    int s_fft_end_frq;
    int s_data_width;
    int s_data_height;
    int s_fft_cut_low_frq;
    
    int count_rfesh_sm;
    int buf_samp_sm[500];
    
    int s_vdisp_speed;
    int s_freq_end; //2150hz
    int s_freq_beg;
    bool f_disp_v_h;
    unsigned int last_reset_sc_in;
    //void FastResetSound();
    //void setPeriodTime(int);
    void SetupSettings_(QString,int,int,int,int,bool);
    int s_mod_iden;
    double y_scale[DATA_WIDTH+80];//2.70 important signal/slot problem
    void Get_Graph(int);
    void record_app();

    bool sta_sto_flag;
    bool g_read_snd;
    void decode_fft_size_samples(int*,int);
    bool draw_flag;

    //double rms_point[RMS_BUF_SIZE];
    //double sinh_point[RMS_BUF_SIZE];
    int y_count;
    int saved_sinh_val;
    int compare_sinh_val;
    int sinh_sleep;
    int koef_sinh_sleep;

    //int save_rms_points;

    enum fft_status
    {
        EMPTY,			// fft_data is currently unused
        FILLING,			// now writing samples to this fft
        READY
    }
    ;				// ready to perform fft
    typedef struct
    {
        fftw_complex * samples;		// complex data for fft
        fftw_plan plan_dsp;			// fft plan for fftW
        int index;			// position of next fft sample
        enum fft_status status;		// whether the fft is busy
    }
    fft_data;

    fft_data * FFT1, * FFT2;		// data for two fft's
    fft_data * ptWriteFft;		// Write the current samples to this fft
    double * fft_avg;		// Array to average the FFT
    double * fft_window;		// Window for FFT data
    int count_fft;			// how many fft's have occurred (for average)
    int fft_size;			// size of fft, e.g. 1024

    int average_count;		// Number of FFT's to average for graph
    int fft_error;			// fft error count
    double s_scale;

    int count_ftt_window;
    int audiobuffer[SAMP_BUFFER_PRE_BUFF];
    int fftw_sample_rate;
    int koef_resample;
    int in_sample_rate;
    int in_bitpersample;
    int s_tci_k_res0;

    struct sound_conf
    {
        char dev_capt_name[SC_SIZE_L];
        char err_msg[SC_SIZE_L];
        int bad_device;
        unsigned int rate_min;
        unsigned int rate_max;
        unsigned int chan_min;
        unsigned int chan_max;
        //int sample_rate;
        int channel_I;
        int channel_Q;
        int data_poll_usec;
        int latency_millisecs;
        char msg1[128];
        int read_error;
        int write_error;
        int underrun_error;
        int interupts;
    } ;
    struct sound_conf rad_sound_state; 
    
#if defined _WIN32_
    unsigned char all_data[SAMP_BUFFER_SIZE_WIN];//2.70
    int cSamples_mono[SAMP_BUFFER_SIZE_WIN/2];
    bool start_sound();
    bool stop_sound();
    bool select_device(bool immediately);//2.49
    bool ReadData();
    //bool ReadData_1();
    DWORD	CapturePosition;	// capture position in the  capture buffer
    DWORD   ReadPosition;		// read position in the capture buffer
    //DWORD   PreviousReadPosition;
    WAVEFORMATEX				wf;				// .wav format description structure
    DSCBUFFERDESC				dscbd;			// DirectSoundCaptureBuffer description structure
    LPDIRECTSOUNDCAPTURE		lpdCapture;	    // Pointer to IDirectSoundCapture8 object
    LPDIRECTSOUNDCAPTUREBUFFER  lpdCaptureBuff; // Holds DirectSoundCaptureBuffer object
    DSCBCAPS					dscbcaps;		// DirectSoundCapture Buffer capabilities
#endif    
#if defined _LINUX_
	///alsa_sound
    enum
    {						// Method of converting sample bytes to an int
        ThreeLittle2Little,					// Three little endian bytes to little endian int
        ThreeLittle2Big
    }					// Three little endian bytes to big endian int
    convert_sample_bytes;
    int alsa_read_sound();
    void open_alsa_capture();
    snd_pcm_t *hCapture;			// handle for alsa soundcard capture or NULL
	//snd_pcm_t *hPlayback;		    // handle for alsa soundcard playback
    snd_pcm_format_t check_formats(snd_pcm_t *h, snd_pcm_hw_params_t *hware);
    int channel_I;
    int channel_Q;
    int chan_capt, chan_play;
    int latency_frames;			// desired latency in audio play samples
	//int play_buffer_size;		// Size of play buffer in samples
    int sample_bytes;			// Size of one channel sample in bytes, either 2 or 4
    int is_little_endian;
    void rad_close_sound();
    void rad_open_sound();
    int read_alsa();
    short buffer2[SAMP_BUFFER_SIZE+1000];				// Buffer for 2-byte samples from sound
    int quisk_overrange;							// count of ADC overrange (clip)
    unsigned char buffer3[(3 * SAMP_BUFFER_SIZE)+1000];	// Buffer for 3-byte samples from sound
    int buffer4[SAMP_BUFFER_SIZE+1000];					// Buffer for 4-byte samples from sound
    int cSamples_l[SAMP_BUFFER_SIZE+1000];
    int cSamples_r[SAMP_BUFFER_SIZE+1000];   
    bool is_pulse_a_in;
    
#endif
    int s_count_resample;
    double s_in_level;    
    HvRawFilter *THvRawFilter0;
    HvRawFilter *THvRawFilter1;

};
#endif



