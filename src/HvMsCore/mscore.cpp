/* MSHV MsCore
 * Copyright 2015 Hrisimir Hristov, LZ2HV
 * May be used under the terms of the GNU General Public License (GPL)
 */

#include "mscore.h"
#include <algorithm> // zaradi max(
//#include <unistd.h>

using namespace std; // zaradi max(

//static const double IN_SAMPLE_RATE =11025.0;//no inposible hv rawfilter need to be off
//static const double IN_SAMPLE_RATE =22050.0;
static const double IN_SAMPLE_RATE_44100 =44100.0;
static const double IN_SAMPLE_RATE_48000 =48000.0;

//#define FFT_SAMPLE_RATE 11025
static const double ORG_SAMPLE_RATE_11025 = 11025.0;
static const double ORG_SAMPLE_RATE_12000 = 12000.0;

#include <QDateTime>

//#include <QtGui>

MsCore::MsCore()
{
    //facalcvd = 1.0;
    //f_adle_vd = false;
    ftci = false;//tci
    s_tci_k_res0 = -1;
    //is_select_sound_device = true;
    //s_fft_cut_hi_frq = FFT_CUT_HI_FRQ;
    dec_busy = false;
    s_data_dsp_height = DATA_DSP_HEIGHT;

    s_fft_end_frq = FFT_END_FRQ;
    s_fft_cut_low_frq = FFT_CUT_LOW_FRQ; //1.38 forget in soorry 1.37

    s_data_width = DATA_WIDTH;
    s_data_height = DATA_HEIGHT;

    count_rfesh_sm = 0;// zaradi bavnoto na jt65abc
    s_vdisp_speed = 2;
    s_freq_beg = 100; //2.76.1 3000hz bw
    s_freq_end = 3300;//2.76.1 3000hz bw
    f_disp_v_h = false;
    last_reset_sc_in = QDateTime::currentDateTimeUtc().toTime_t();
    s_in_level = 1.0;
    //period_time_sec = (int)STATIC_FFTW_PERIOD_TIME;
    //save_rms_points = 2;//vnimanie triabva da e pomalko ot ->RMS_BUF_SIZE w MsCore.h

    g_read_snd = false;//true;
    sta_sto_flag = false;

    draw_flag = false;

    y_count = 0;
    saved_sinh_val = 0;
    compare_sinh_val = 0;
    sinh_sleep = 0;
	//////////////////////////////////////////////////////////////////////
    s_mod_iden = 2;//HV important set to default mode fsk441
    fftw_sample_rate = (int)ORG_SAMPLE_RATE_11025;//HV important set to default mode fsk441 sample rate
    in_sample_rate = IN_SAMPLE_RATE_44100; //HV important set to default mode fsk441 sample rate
    koef_resample = (int)((in_sample_rate/ORG_SAMPLE_RATE_11025)-1);//HV important set to default mode fsk441 sample rate
    in_bitpersample = 16;
	//////////////////////////////////////////////////////////////////////

    THvRawFilter0 = new HvRawFilter(53,65.0,4750.0,false);//false -> LPF+HPF /* low pass filter and High pass filter */ for RX
    THvRawFilter0->set_rate(in_sample_rate);//2.76.2
    THvRawFilter0->set_lpf_hpf_parm(0.7071,0.7071);//2.76.3
    THvRawFilter1 = new HvRawFilter(53,65.0,4750.0,false);//4600.0
    THvRawFilter1->set_rate(fftw_sample_rate);//2.76.2
    THvRawFilter1->set_lpf_hpf_parm(0.7071,0.7071);//2.76.3

#if defined _LINUX_
    is_pulse_a_in = true;//2.65  default pulse
    hCapture = NULL;
    is_little_endian = 0;
#endif

#if defined _LINUX_
    //strncpy(rad_sound_state.dev_capt_name, "hw:0,0", SC_SIZE_L); //2.65  default pulse
    strncpy(rad_sound_state.dev_capt_name, "pulse: default", SC_SIZE_L);
#endif
#if defined _WIN32_  //Primary Sound Capture Driver or "0"
    strncpy(rad_sound_state.dev_capt_name, "Primary Sound Capture Driver", SC_SIZE_L);//
#endif
    s_count_resample = 0;

    rad_sound_state.latency_millisecs = 100;
    rad_sound_state.data_poll_usec = 5000;
    rad_sound_state.channel_I = 0;//0  //standart->1 X-Fi->0
    rad_sound_state.channel_Q = 1;//1  //standart->0 X-Fi->1
    rad_sound_state.read_error = 0;
    rad_sound_state.write_error = 0;
    rad_sound_state.underrun_error = 0;
    rad_sound_state.interupts = 0;

    FFT1 = FFT2 = ptWriteFft = NULL;
    count_ftt_window = 0;

    bool p_read_snd = true;
#if defined _WIN32_
    //is_select_sound_device = select_device(true);//v1.46
    //p_read_snd = is_select_sound_device;
    p_read_snd = select_device(true);//2.74
    if (p_read_snd)
    {    	
    	p_read_snd = start_sound();  
    	usleep(1000);//2.74     	
   	}
#endif
#if defined _LINUX_
    rad_open_sound();
#endif

    QTimer *timer_ref_ = new QTimer();
    connect(timer_ref_,SIGNAL(timeout()),this,SLOT(Refresh_t()));//,Qt::DirectConnection
    //timer_ref_->setTimerType(Qt::VeryCoarseTimer);
    timer_ref_->start(5);//5ms refresh time

    timer_fr_sound = new QTimer();
    connect(timer_fr_sound, SIGNAL(timeout()), this, SLOT(FastResetSoundCardIn_p()));

    // SETUP APP FOR ALL MODES   ///////////////////////////
    fftw_sample_rate=(int)ORG_SAMPLE_RATE_12000;
    record_app();//msk144

    f_disp_v_h = true;
    fftw_sample_rate=(int)ORG_SAMPLE_RATE_12000;
    for (int i=1; i<10; i++)//1.51 10 9-speads //da se smeni na -> 6
    {
        s_vdisp_speed = i;
        //SetupSettings_((QString)rad_sound_state.dev_capt_name,QString("%1").arg(in_sample_rate),rad_sound_state.latency_millisecs,
        //rad_sound_state.data_poll_usec,rad_sound_state.channel_I,true);
        record_app(); //jt65abc
    }
    f_disp_v_h = false;
    s_vdisp_speed = 2;

    fftw_sample_rate=(int)ORG_SAMPLE_RATE_11025;
    record_app();//jtms to jt6m
    // END SETUP APP FOR ALL MODES  ///////////////////////////

    usleep(2000);

    //qDebug()<<((192000 * (( 1 * 16 ) / 8)) * 120 + 32);
    //qDebug()<<sizeof(double);
    g_read_snd = p_read_snd;//false;
    //qDebug()<<"CREATED";
}
MsCore::~MsCore()
{
    delete THvRawFilter0;
    delete THvRawFilter1;
}
void MsCore::SetInLevel(int val)
{
    //s_in_level = pow(10, ((double)(val - 50)/4)/20);	// +- 12.5db
    s_in_level = pow(10, ((double)(val - 50)/2.5)/20.0);// +- 20db 1.78
    //qDebug()<<s_in_level;
}
void MsCore::close_sound()
{
    if (ftci) return;//tci
#if defined _WIN32_
    //qDebug()<<"TRAY  StopSound";
    if (lpdCaptureBuff)//from v1.46
    {
        stop_sound();
        lpdCaptureBuff->Release();
        lpdCapture->Release();
        lpdCaptureBuff = NULL;//2.49 protection
        //lpdCapture = NULL;
        //if (lpdCaptureBuff) qDebug()<<"close lpdCaptureBuff true";
        //else qDebug()<<"close lpdCaptureBuff false";
    }
#endif
#if defined _LINUX_
    rad_close_sound();
#endif
}
void MsCore::SetStr_Sto(bool flag)
{
    g_read_snd = false; 
    sta_sto_flag = flag;
    bool p_read_snd = true;
#if defined _WIN32_
    if (!ftci)//tci
    {
        if (sta_sto_flag)
        {
            p_read_snd = select_device(false); //qDebug()<< p_read_snd;
            if (p_read_snd) p_read_snd = start_sound(); 
        }
        else
        {
            if (lpdCaptureBuff) stop_sound();
            //if (lpdCaptureBuff) qDebug()<<"stop lpdCaptureBuff true";
            //else qDebug()<<"stop lpdCaptureBuff false";
        }
    }
#endif
    g_read_snd = p_read_snd;  
    if (sta_sto_flag)  // if app long time in stop fast reset sound card
    {
        FastResetSoundCardIn();
    }
}
void MsCore::SetMode(int mod_iden)
{
    s_mod_iden = mod_iden;    
    /*QString rate;      
    if (s_mod_iden>0 && s_mod_iden<7) //2.65
        rate = QString("%1").arg((int)IN_SAMPLE_RATE_44100);
    else
    	rate = QString("%1").arg((int)IN_SAMPLE_RATE_48000);*/
    if (s_mod_iden == 7 || s_mod_iden == 8 || s_mod_iden == 9 || s_mod_iden == 10 || s_mod_iden == 11 || s_mod_iden == 13 || s_mod_iden == 18 ||
            s_mod_iden == 14 || s_mod_iden == 15 || s_mod_iden == 16 || s_mod_iden == 17) f_disp_v_h = true;////jt65abc pi4 ft8 ft4 q65        
    else f_disp_v_h = false;
    SetupSettings_((QString)rad_sound_state.dev_capt_name,in_bitpersample,rad_sound_state.latency_millisecs,
                   rad_sound_state.data_poll_usec,rad_sound_state.channel_I,true);
}
void MsCore::SetupSettings(QString dev_in_number,int bpsampl,int latency,int card_buffer_polls,int channel)// QString, 
{
    if ((QString)rad_sound_state.dev_capt_name == dev_in_number && bpsampl==in_bitpersample &&/*rad_sound_state.sample_rate == rate.toInt() &&*/
            rad_sound_state.latency_millisecs == latency && rad_sound_state.data_poll_usec == card_buffer_polls &&
            rad_sound_state.channel_I == channel)
        return;//za da ne garmi pod windows
    SetupSettings_(dev_in_number,bpsampl,latency,card_buffer_polls,channel,false);
}
void MsCore::record_app()
{
    //qDebug()<<"Start record_app";
    int i, j;
    fftw_complex * pt;
    fft_data * pt1, * pt2;
    double graph_refresh;

    //if (f_disp_v_h)
    //graph_refresh = 100.0;
    //else
    graph_refresh = 100.0; //7 refresha na fft hv zavisi ot samplerate

    /*(about 800 to 1200 pixels) times the fft_size_multiplier.  You can make fft_size_multiplier
    about 2 to 4, to make the FFT bins smaller if you have the processor power.*/
    ////HV spriano //////////////////////////////////////
    //int fft_size_multiplier = 2;
    //fft_size = GRAPHIC_HEIGHT * fft_size_multiplier;
    //while (fft_size < (rad_sound_state.sample_rate * rad_sound_state.data_poll_usec / 1000000))
    //fft_size += GRAPHIC_HEIGHT;
    ////HV spriano ////////////////////////////////////////

    //fftw_sample_rate=ORG_SAMPLE_RATE_11025;
    double scale_x_inc_pix;
    if (f_disp_v_h)            /// DATA_HEIGHT*2 -25 -19 -25<freq bara (- 10 malko ot dolnia disp)
        scale_x_inc_pix = (double)(((double)(s_data_height*2 -25 - 10)/(double)s_vdisp_speed) /(double)STAT_FFTW_V60_TIME);
    else
        scale_x_inc_pix = (double)((double)s_data_width /(double)STAT_FFTW_H30_TIME);// prez kolko piksela za sekunta

    fft_size = (int)((double)fftw_sample_rate /(double)scale_x_inc_pix);  // fft_size za da upalni celia ekran

    //qDebug()<<"fft_size="<<fft_size<<"fft_size_end="<<fft_size/4;

    average_count = max(1, int((double)fftw_sample_rate / (double)fft_size / graph_refresh));

    /*s_scale = 2.0 / (double)average_count / (double)fft_size;	//add 1.37 Divide by sample count
    s_scale /= pow(2.0, 31);//2.0, 31			//add 1.37 Normalize to max == 1*/

    s_scale = 2.0 / (double)average_count / (double)fft_size;	//add 1.37 Divide by sample count
    if (f_disp_v_h)//v1.51 corr for 9-v speeds
    {
        /*double k_correct = 0.25*(double)(s_vdisp_speed*s_vdisp_speed);
        s_scale = s_scale*((double)s_vdisp_speed*k_correct+(1.0-k_correct));
        s_scale /= pow(2.0, 14)*1.2;//2.46   5)*1.6=half*/

        double k_correct = 0.25; //old
        s_scale = s_scale*((double)s_vdisp_speed*k_correct+(1.0-k_correct));
        s_scale /= pow(2.0, 23); //*1.0
    }
    else
        s_scale /= pow(2.0, 24)*1.2;//2.46 old pow(2.0, 31);			//add 1.37 Normalize to max == 1
    //qDebug()<<"fft_size="<<fft_size;//<<average_count<<s_scale;//<<average_countd;

    strncpy (rad_sound_state.err_msg, CLOSED_TEXT, SC_SIZE_L);
    count_fft = 0;
    pt1 = FFT1;
    pt2 = FFT2;
    FFT1 = FFT2 = ptWriteFft = NULL;	// The callback may be active!
    // Create space for the fft
    if (pt1)
    {
        fftw_destroy_plan(pt1->plan_dsp);
        fftw_free(pt1->samples);
        free(pt1);
    }
    pt1 = (fft_data *)malloc(sizeof(fft_data)); //qDebug()<<"sizeof(fft_data)"<<sizeof(fft_data);
    pt1->status = EMPTY;
    pt1->index = 0;
    pt = pt1->samples = (fftw_complex *) fftw_malloc(sizeof(fftw_complex) * (fft_size+52));//2.45 +50
    //qDebug()<<"pt1 1= creaate";
    //FFTW_ESTIMATE           ni barzo
    //FFTW_ESTIMATE_PATIENT   sredno
    //FFTW_MEASURE            bavi no result
    //FFTW_PATIENT            bavi mnogo
    //FFTW_EXHAUSTIVE         bavi mnogo
    //org samo bavi
    //pt1->plan_dsp = fftw_plan_dft_1d(fft_size, pt, pt, FFTW_FORWARD, FFTW_MEASURE);
    pt1->plan_dsp = fftw_plan_dft_1d(fft_size, pt, pt, FFTW_FORWARD, FFTW_ESTIMATE_PATIENT);//1.35
    //qDebug()<<"pt1 1=end";
    // Create space to write samples while the first fft is in use
    if (pt2)
    {
        fftw_destroy_plan(pt2->plan_dsp);
        fftw_free(pt2->samples);
        free(pt2);
    }
    pt2 = (fft_data *)malloc(sizeof(fft_data));
    pt2->status = EMPTY;
    pt2->index = 0;
    pt = pt2->samples = (fftw_complex *) fftw_malloc(sizeof(fftw_complex) * (fft_size+52));//2.45 +50
    //qDebug()<<"pt2 2= create";
    //FFTW_ESTIMATE           ni barzo
    //FFTW_ESTIMATE_PATIENT   sredno
    //FFTW_MEASURE            bavi no result
    //FFTW_PATIENT            bavi mnogo
    //FFTW_EXHAUSTIVE         bavi mnogo
    //org samo bavi
    //pt2->plan_dsp = fftw_plan_dft_1d(fft_size, pt, pt, FFTW_FORWARD, FFTW_MEASURE);
    pt2->plan_dsp = fftw_plan_dft_1d(fft_size, pt, pt, FFTW_FORWARD, FFTW_ESTIMATE_PATIENT);//1.35
    //qDebug()<<"pt2 2="<<s_vdisp_speed;
    FFT1 = pt1;
    FFT2 = pt2;
    // Create space for the fft average and window
    if (fft_avg)
        free(fft_avg);
    if (fft_window)
        free(fft_window);

    if (f_disp_v_h)
    {
        fft_avg = (double *) malloc(sizeof(double) * ((s_data_width)+50));////1.39 rem s_data_width/2    1.36 +20
        for (i = 0; i < s_data_width; i++)//1.39 rem s_data_width/2
            fft_avg[i] = 0;
        //qDebug()<<"fft_avg-------------------"<<((s_data_width/2)+20);
    }
    else
    {
        fft_avg = (double *) malloc(sizeof(double) * (s_data_height+20));//1.36 +20
        for (i = 0; i < s_data_height; i++)
            fft_avg[i] = 0;
        //qDebug()<<"fft_avg-------------------"<<s_data_height+20;
    }
    // same as nuttal_window but better
    fft_window = (double *) malloc(sizeof(double) * (fft_size+50));//1.36 +50
    for (i = 0, j = -fft_size / 2; i < fft_size; i++, j++)
    {
        if (0)	// Hamming
            fft_window[i] = 0.54 + 0.46 * cos(2.0 * M_PI * j / fft_size);
        else	// Hanning
            fft_window[i] = 0.5 + 0.5 * cos(2.0 * M_PI * j / fft_size);
    }
    //qDebug()<<"Stop record_app";
}
void MsCore::SetDecBusy(bool f)
{
    dec_busy = f;
}
void MsCore::FastResetSoundCardIn()
{
    if (!timer_fr_sound->isActive()) timer_fr_sound->start(220);//2.47
}
void MsCore::FastResetSoundCardIn_p()// for pure sound cards
{
    //1min=60, 2min=120, 15min=900 ,30min=1800, 1h=3600, 1:30h=5400, 2h=7200
    unsigned int hh = QDateTime::currentDateTimeUtc().toTime_t();

    if (hh < last_reset_sc_in + 1802)//1802=30:02  1800+2  2=seconds
    {
        //qDebug()<<"T Stop 1";
        timer_fr_sound->stop();
        return;
    }

    if (dec_busy)
    {
        //qDebug()<<"Decoder Busy";
        return;
    }
    //qDebug()<<"T Stop 2";
    timer_fr_sound->stop();

    //qDebug()<<"Start FastReset====================================";

    g_read_snd = false;//true; // stop reading

    usleep(1000);       //2.37 be6e 1000   1.33=no  20ms in sdr  1.34  // triabva da e mnogo pove4e otkolkoto e na loop

    close_sound();

    usleep(2000);       //1.33=1000  1.34=2000  wait hardware to clear buffers

    THvRawFilter0->set_rate(in_sample_rate);//2.76.2
    THvRawFilter1->set_rate(fftw_sample_rate);//2.76.2		

    bool p_read_snd = true;
#if defined _WIN32_
    if (!ftci) //tci
    {
        p_read_snd = select_device(true);
        if (p_read_snd) p_read_snd = start_sound();
    }
#endif
#if defined _LINUX_
    if (!ftci) //tci
    {
        rad_open_sound();
    }
#endif

    record_app();            //fftw

    usleep(2000);            //1.33=no     1.34 wait fftw

    last_reset_sc_in = hh;
    g_read_snd = p_read_snd;     // start reading
    //qDebug()<<"End FastReset=================================="<<QDateTime::currentDateTimeUtc().toString("hh:mm:ss");
}
static int tci_read_ = 0; //2.58 0=stop 1=buff 2=read
void MsCore::SetupSettings_(QString dev_in_number,int bpsampl,int latency,int card_buffer_polls,
                            int channel,bool imidiatly)
{
    if (!imidiatly)
    {
        if ((QString)rad_sound_state.dev_capt_name == dev_in_number && bpsampl==in_bitpersample &&// && in_sample_rate == sample_rate.toInt()
                rad_sound_state.latency_millisecs == latency && rad_sound_state.data_poll_usec == card_buffer_polls &&
                rad_sound_state.channel_I == channel)
            return;//za da ne garmi pod windows
    }

    g_read_snd = false;//true;  // stop reading

    usleep(1000);        //2.37 be6e=1000 1.33=no in sdr 20ms   //     1.34

    close_sound();

    usleep(2000);        //1.33=yes  1.34  wait hardware to clear buffers

    in_bitpersample = bpsampl; //qDebug()<<bpsampl;
    if (s_mod_iden>0 && s_mod_iden<7) //2.65
    {
        fftw_sample_rate = (int)ORG_SAMPLE_RATE_11025;
        in_sample_rate = (int)IN_SAMPLE_RATE_44100;//sample_rate.toInt();
        koef_resample = (int)((in_sample_rate/(int)ORG_SAMPLE_RATE_11025)-1);
    }  
    else  
    {
        fftw_sample_rate = (int)ORG_SAMPLE_RATE_12000;
        in_sample_rate = (int)IN_SAMPLE_RATE_48000;//sample_rate.toInt();
        koef_resample = (int)((in_sample_rate/(int)ORG_SAMPLE_RATE_12000)-1);
    } 
          
    //for pulse audio max is 512 ???
    strncpy(rad_sound_state.dev_capt_name,dev_in_number.toUtf8(),127); //127 2.37 be6e SC_SIZE_L
    //if ((QString)rad_sound_state.dev_capt_name==dev_in_number)
    //qDebug()<<"mscore="<<(QString)rad_sound_state.dev_capt_name<<dev_in_number;
    QString str_device_name = (QString)rad_sound_state.dev_capt_name; //qDebug()<<"str_device_name="<<str_device_name;
    if (str_device_name=="TCI Client Input")
    {     	
        fftw_sample_rate = (int)ORG_SAMPLE_RATE_12000;
        in_sample_rate = (int)IN_SAMPLE_RATE_48000;//sample_rate.toInt();
        koef_resample = (int)((in_sample_rate/(int)ORG_SAMPLE_RATE_12000)-1);
        ftci = true;
        s_tci_k_res0 = -1;//reset 	
   	}	
    else ftci = false;  	
    tci_read_ = 0; //printf("SoundSetup Mode=%2d in_sample_rate=%d fftw_sample_rate=%d k_res=%d\n",s_mod_iden,in_sample_rate,fftw_sample_rate,koef_resample);
   	
    rad_sound_state.latency_millisecs = latency;//50 be6e hv 50-300
    rad_sound_state.data_poll_usec = card_buffer_polls;
    if (channel == 0) rad_sound_state.channel_I = 0;
    else rad_sound_state.channel_I = 1;

	THvRawFilter0->set_rate(in_sample_rate);//2.76.2
    THvRawFilter1->set_rate(fftw_sample_rate);//2.76.2 //printf(" Mode=%d in_sample_rate=%d\n",s_mod_iden,in_sample_rate);

    bool p_read_snd = true;
#if defined _WIN32_
    if (!ftci) //tci
    {
        p_read_snd = select_device(true);
        if (p_read_snd) p_read_snd = start_sound();
    }
#endif
#if defined _LINUX_
    QString str_device_name0 = str_device_name+"xxxxxxx";//2.70
    if (str_device_name0.mid(0,7)=="pulse: ") is_pulse_a_in = true;
    else is_pulse_a_in = false;
    if (!ftci) //tci
    {
        rad_open_sound();
        int c_retry = 0;
        if (!is_pulse_a_in && !hCapture)
        {
            while (!hCapture)
            {
                usleep(100000);
                rad_open_sound();
                c_retry++;
                if (c_retry>120) break;
            }
        }
    }
#endif

    record_app();       // fftw

    usleep(2000);       //1.33=yes 1.34 wait fftw

    last_reset_sc_in = QDateTime::currentDateTimeUtc().toTime_t();// reset time for fast reset
    g_read_snd = p_read_snd;//false;  // start reading
    //qDebug()<<"GrandReset";
}
#define TCIBUFRX_LIM 480000 //=10sec //=8sec =385024
#define TCIBUFRX TCIBUFRX_LIM + 16450 //sample is 4096 max from network +=16384
static int tci_raw2_0[TCIBUFRX]; //mono
static int tci_cr2_0 = 0;
static int tci_k_res_0 = 3;//default=48000
int _GetRxAudioReadTci_()
{
    return tci_read_;
}
void _SetRxAudioTci_(int *raw2,int cr2,int k_res) //sample is 4096
{
	tci_k_res_0 = k_res; //qDebug()<<"tci_cr2_0"<<tci_cr2_0;
    if (tci_cr2_0 < TCIBUFRX_LIM)
    {
        for (int i = 0; i < cr2; ++i) tci_raw2_0[i+tci_cr2_0] = raw2[i];
        tci_cr2_0 += cr2;
    }
    tci_read_ = 2; 
}
void MsCore::ReadTci()
{
    tci_read_ = 1;
    static int tci_raw2_1[TCIBUFRX];
    int count = 0;
    for (int i = 0; i < tci_cr2_0; ++i)
    {
        tci_raw2_1[count] = tci_raw2_0[i];
        count++;
    }
    tci_cr2_0 = 0; // frre input buffer
    ResampleAndFilter(tci_raw2_1,count); //2.57
}
void MsCore::ResampleAndFilter(int *rawm,int crawm) //2.57
{
    static int dat_tt[130000]; //12000*10=120000 max=10sec  120000+12000=132000    
    int k_res = koef_resample;//2.76.2 
    if (ftci)
    {
    	k_res = tci_k_res_0;
    	if (s_tci_k_res0!=k_res)
    	{
    		s_tci_k_res0 = k_res;
			in_sample_rate = 48000;
			if 		(k_res==1) in_sample_rate = 24000;
			else if (k_res==0) in_sample_rate = 12000;
			THvRawFilter0->set_rate(in_sample_rate);
			s_count_resample = 0;//importante crash
    		//printf("TCI        Mode=%2d in_sample_rate=%d fftw_sample_rate=%d k_res=%d\n",s_mod_iden,in_sample_rate,fftw_sample_rate,k_res);     		
   		} 	
   	}    	
	/*static int sin_sample_rate=0; static int sfftw_sample_rate=0; static int sk_res=-1; static int ss_mod_iden=-1;
	if (sk_res!=k_res || ss_mod_iden!=s_mod_iden || sin_sample_rate!=in_sample_rate || sfftw_sample_rate!=fftw_sample_rate)   
	{
   		printf(" Mode=%2d in_sample_rate=%d fftw_sample_rate=%d k_res=%d\n",s_mod_iden,in_sample_rate,fftw_sample_rate,k_res);
   		ss_mod_iden=s_mod_iden; sin_sample_rate=in_sample_rate; sfftw_sample_rate=fftw_sample_rate; sk_res=k_res;	
	}*/	 
    int limit = 114000*(k_res+1);//9.5sec 48000Hz 12000*9.5=114000
    if (crawm > limit) crawm = limit;
    int k = 0; 
    for (int i = 0; i < crawm; ++i)
    { 
    	double data_l0 = THvRawFilter0->band_l((double)rawm[i]);//if (k_res>0) {if(i==0) printf(" UseF0=%d\n",(in_sample_rate/3));}
        if (s_count_resample>=k_res)//2.76.2 >=  (s_count_resample==k_res)=crash
        {
            double data_l1 = THvRawFilter1->band_l(data_l0);
            dat_tt[k]=(int)(data_l1*s_in_level);
            s_count_resample = 0;
            k++;
        }
        else s_count_resample++;
    } 
    //fil4(rawm,crawm,dat_tt,k);
    
    /*for (int i = 0; i < crawm; ++i)
    {
    	bfr4[s_count_resample] = (double)rawm[i];
        if (s_count_resample==k_res)
        {
        	double tbfr = (bfr4[0]+bfr4[1]+bfr4[2]+bfr4[3])/4.0;
			double data_l = THvRawFilter->band_l(tbfr);
            dat_tt[k]=(int)(data_l*s_in_level);
            s_count_resample = 0;
            k++; 
        }
        else s_count_resample++;
    }*/
    ReciveData(dat_tt,k,false); //if (k>1024) printf("12KHz=%d, 48KHz=%d, limit48KHz=%d,\n",k,crawm,limit);
}
void MsCore::ReciveData(int *dat,int count,bool ffopen)
{	
    emit Set_Raw(dat,count,ffopen); //1.27 psk rep fopen bool true false no file open

    for (int i = 0; i < count; ++i)
    {
        int in_dat = dat[i];
        audiobuffer[count_ftt_window] = in_dat;//*fac; count_ftt_window max = 24452
        if (count_ftt_window >= fft_size-1)// i < fft_size 1.29 for this -1
        {
            decode_fft_size_samples(audiobuffer, fft_size); // i < fft_size 1.29            
            count_ftt_window = 0;//qDebug()<<"count="<<fft_size;
        }
        else count_ftt_window++;

        buf_samp_sm[count_rfesh_sm] = in_dat/256; //2.70 or *0.00390625 to 
        if (count_rfesh_sm >= 450-1)// max 500 v1.36
        {
            count_rfesh_sm = 0;
            double sum(0); //double val;
            int smiter = 0;
            for (int x = 0; x < 450; ++x)
            {
                double val = fabs(buf_samp_sm[x]);
                sum += val;
            }
            sum /= 450.0;
            double log0 = sqrt(sum);
            if (log0<1.0) log0=1.0; //2.58
            smiter = abs(int(log(log0)*20.0)); //qDebug()<<"count="<<sum<<log0<<log(log0);
            emit Sed_SMeter(smiter);// zaradi bavnoto na jt65abc
        }
        else count_rfesh_sm++;
    }
}
//max fft_size= 24452 to 1.54
void MsCore::decode_fft_size_samples(int *data_mono, int count)
{
    //inportant hv -> count==fft_size
    //double sum(0);
    //double val;
    //int smiter = 0;
    //qDebug()<<count;
    //qDebug()<<SAMP_BUFFER_PRE_BUFF;
    //HV VNIMANIE ako iskam s razli4ni samplerate triabva da se saobrazia s whodnite
    //GRAPHIC_WIDTH, period_time_sec, sample_rate i sled towa revord_app()

    static int playSilence = 0;
    fft_data * ptFft;

    /* check for space, then put samples into the fft input array */
    if (! ptWriteFft)
    {	// wait for an empty fft to become available
        if (FFT1 && FFT1->status == EMPTY)
        {
            FFT1->status = FILLING;
            FFT1->index = 0;
            ptWriteFft = FFT1;
        }
        else if (FFT2 && FFT2->status == EMPTY)
        {
            FFT2->status = FILLING;
            FFT2->index = 0;
            ptWriteFft = FFT2;
        }
    }

    if (!playSilence && ptWriteFft)
    {	// write samples to fft data array
        double sum(0);
        int smiter = 0;
        for (int i = 0; i < count; ++i)
        {
            //part of smiter
            if (!f_disp_v_h)
            {
                double val = fabs(data_mono[i]/256);//2.70 =/256 to 24-bit
                sum += val;
            }

            ptWriteFft->samples[ptWriteFft->index] = ((double)data_mono[i]*0.0000390625);//2.70 =0.0000390625 old=0.01
            //if (ptWriteFft->index >= fft_size-1)
            //qDebug()<<"ptWriteFft->index"<<ptWriteFft->index;
            if (++(ptWriteFft->index) >= fft_size)
            {	// check sample count
                ptWriteFft->status = READY;	// ready to run fft
                if (ptWriteFft == FFT1)
                {	// switch to other fft
                    ptFft = FFT2;
                }
                else
                {
                    ptFft = FFT1;
                }
                if (ptFft->status == EMPTY)
                {	// continue writing samples
                    ptFft->status = FILLING;
                    ptFft->index = 0;
                    ptWriteFft = ptFft;
                }
                else
                {				// no place to write samples
                    ptWriteFft = NULL;
                    fft_error++;
                    break;
                }
            }
        }

        //part of smiter
        if (count==0) count=1;
        sum /= count;
        double log0 = sqrt(sum);
        if (log0<1.0) log0=1.0;//2.58
        smiter = abs(int(log(log0)*20.0));
        Get_Graph(smiter);
        // qDebug()<<"period_time_sec";
    }
}
void MsCore::SetVDispSpeed(int speed)
{
    s_vdisp_speed = (10-speed);//1.51 10 9-speeds, 6
    //s_vdisp_speed = speed;
    SetupSettings_((QString)rad_sound_state.dev_capt_name,in_bitpersample,rad_sound_state.latency_millisecs,
                   rad_sound_state.data_poll_usec,rad_sound_state.channel_I,true);
}
void MsCore::setVDFftwStartStopFreq(int beg,int end)
{
    s_freq_beg = beg;
    s_freq_end = end; //qDebug()<<beg<<end;
}
void MsCore::Get_Graph(int smiter)
{
    int i, j, k, n;
    fft_data * ptFft;
    //double y_scale[DATA_WIDTH+50];//2.70 stop important 2.70 important signal/slot problem
    double d2;

    //qDebug()<<DATA_WIDTH;
    //if (!PyArg_ParseTuple (args, ""))
    //return NULL;
    // Look for an fft ready to run.  Throw the other away if there are two.
    if (FFT1 && FFT1->status == READY)
    {
        ptFft = FFT1;
        if (FFT2 && FFT2->status == READY)
            FFT2->status = EMPTY;
    }
    else if (FFT2 && FFT2->status == READY)
    {
        ptFft = FFT2;
        if (FFT1 && FFT1->status == READY)
            FFT1->status = EMPTY;
    }
    else
    {		// No data yet
        //Py_INCREF(Py_None);
        return;
    }
    for (i = 0; i < fft_size; ++i)	// multiply by window
        ptFft->samples[i] *= fft_window[i];
    //ptFft->samples[i] = ptFft->samples[i]*fft_window[i];

    fftw_execute(ptFft->plan_dsp);	// Calculate FFT
    // Average the fft data into the graph width

    //double kavg = 0.0;//2.46
    if (f_disp_v_h)
    {
        //int s_freq_end = 2200; //2150hz
        //int s_freq_beg = 200;  //200hz  1.0 koef
        double size_6000 = fft_size/2.0;
        int beg_size = ((double)s_freq_beg*size_6000)/((double)fftw_sample_rate/2.0);
        int end_size = ((double)s_freq_end*size_6000)/((double)fftw_sample_rate/2.0);
        double d_koef = (double)(end_size-beg_size)/(double)((double)s_data_width);//1.39 rem s_data_width/2
        bool retry = false;

        for (i = 0, k = 0; k < s_data_width; k++)//1.39 rem s_data_width/2
        {
            int c_kk = 1;
c6:
            if (!retry)//2.45
            {
                fft_avg[k] += cabs(ptFft->samples[i+beg_size]);
            }
            else
            {
                retry = false;
                double p0 = cabs(ptFft->samples[i+beg_size]);
                double p1 = cabs(ptFft->samples[i+beg_size+1]);
                fft_avg[k] = (p0+p1)/2.0;
            }
            if ((double)i>(double)k*d_koef)// tova pri nedostig
            {
                retry = true;
                continue;
            }
            i++;
            if ((double)i<(double)k*d_koef)// towa kogato ima mnogo
            {
                c_kk++;
                goto c6;
            }
            fft_avg[k]=(fft_avg[k]/(double)c_kk);
        }
    }
    else
    {
        n = fft_size / s_data_height /2 ; //HV za positie and negative bez -> / 2
        if ( n == 0 ) n=1;//HV za da risuva i pod 20 seconds ina4e n = 0 stava            
        for (i = 0, k = 0; k < s_data_height; ++k)
        {
            for (j = 0; j < n; ++j) fft_avg[k] += cabs(ptFft->samples[i++]);        	
       	}
    }
    //qDebug()<<"fft_avg="<<k;

    ptFft->status = EMPTY;
    if (++count_fft < average_count)
    {
        //Py_INCREF(Py_None);	// No data yet
        return;
    }
    // We have averaged enough fft's to return the graph data
    count_fft = 0;
    //tuple2 = PyTuple_New(DATA_WIDTH);

    i = 0;

    if (f_disp_v_h) i=(s_data_width)-1;//DATA_DSP_HEIGHT-0; //1.39 rem s_data_width/2        
    else i=s_data_dsp_height-1;//DATA_DSP_HEIGHT-0;
        
    //i=(150-46)-1;

    if (f_disp_v_h)
    {
        //kavg /= s_data_width; //qDebug()<<kavg;//qoter avg
        //if (kavg < 0.001) kavg = 0.001;	// //2.46
        //double avg_alcvd = 0.0;
        for (k = 0; k < s_data_width; ++k)//1.39 rem s_data_width/2
        {
            d2 = log10(fft_avg[k] * s_scale); //d2 = log(fft_avg[k] * (s_scale))*0.5;
            //avg_alcvd += fabs(d2);
            if (d2 < -10.0) d2 = -10.0;
            fft_avg[k] = 0;
            y_scale[i] = (50.0 * d2);
            if (i>0) i--;//1.36 i>0 pazi ot -1                
        }
        //avg_alcvd /= (double)s_data_width;
        //RefrAlcVD(avg_alcvd);
    }
    else
    {
        //for (k = FFT_CUT_LOW_FRQ; k < FFT_END_FRQ; k++/*, i--*/)//34
        for (k = s_fft_cut_low_frq; k < s_fft_end_frq; ++k/*, i--*/)
        {
            d2 = log10(fft_avg[k] * s_scale);
            if (d2 < -10.0) d2 = -10.0;
            fft_avg[k] = 0;
            y_scale[i] = (50.0 * d2);// 50.0 <- max display is 501
            if (i>0) i--;//1.36 i>0 pazi ot -1                
        }
    }
    //qDebug()<<"mmmmmmmmmmmmmmmmmmmmmmmmmmm";
    emit Set_Graph(y_scale, smiter);
    //delete y_scale;//rem 1.37
}
void MsCore::Refresh_t()
{
    if (sta_sto_flag && g_read_snd)
    {
        if (ftci)//tci
        {
            if (tci_read_ == 0)
            {
                tci_cr2_0 = 0;
                tci_read_ = 1;
            }
            if (tci_read_ == 2) ReadTci();
        }
        else
        {
#if defined _WIN32_
            ReadData();
#endif
#if defined _LINUX_
            alsa_read_sound();
#endif
        }
    }
    else
        tci_read_ = 0;

    emit Refresh_time();
}

