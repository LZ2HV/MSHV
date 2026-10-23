#include "mscore.h"

#if defined _LINUX_
//#include <QtGui>

#include <pulse/pulseaudio.h>
#include <pulse/simple.h>
pa_simple *s_pulse_rx = NULL;

int MsCore::alsa_read_sound()
{ 
    int count = 0;
    if (!is_pulse_a_in)
    {
        //////////////////////tova go zabavia ne znam kak /////////////////////////////////////////////////////////////
        static unsigned long long int time0 = 0;		// times are in microseconds
        unsigned long long int time;
        struct timespec tspec, trem;

        int  timer;//, count;
        struct timeval tv;
#if DEBUG > 2
        static unsigned long long int stime = 0;
        static int samples;
        static int calls=0, waits=0;
#endif

        //	gettimeofday(&tv, NULL);
        tv.tv_sec = 200;
        tv.tv_usec = 200;
        time = tv.tv_sec * 1000000 + tv.tv_usec;

        timer = rad_sound_state.data_poll_usec - (int)(time - time0);	// time remaining from last poll
        if (timer > 1000)
        {	// see if enough time has elapsed
            // wait for the remainder of the poll interval
            tspec.tv_sec = 0;
            tspec.tv_nsec = timer * 1000;
            clock_nanosleep(CLOCK_MONOTONIC, 0, &tspec, &trem);
#if DEBUG > 2
            waits++;
        }
        calls++;
        if (calls % 400 == 0)
        {
#if DEBUG > 3
            printf("quisk_read_sound calls %d, waits %d, waits %.2f %%\n", calls, waits, 100.0 * waits / calls);
#endif
            calls = waits = 0;
        }
#else
        }
#endif
        //	gettimeofday(&tv, NULL);		// reset starting time value
        tv.tv_sec = 200;
        tv.tv_usec = 200;
        time0 = tv.tv_sec * 1000000 + tv.tv_usec;
        //////////////////////tova go zabavia ne znam kak /////////////////////////////////////////////////////////////

        count = read_alsa();// read from ALSA soundcard

		//printf("sample_bytes=%d\n",sample_bytes);
		int *dat_t = new int[count+10];
		for (int j = 0; j < count; ++j)
		{
			//if 	  (sample_bytes == 2  || sample_bytes == 4) dat_t[j] = cSamples_l[j]/65536; //int to short
			//else if (sample_bytes == 3) 					  dat_t[j] = cSamples_l[j]/256;   //int to short
			if 		(sample_bytes == 2  || sample_bytes == 4) dat_t[j] = cSamples_l[j]/256; //int to int24
			else if (sample_bytes == 3) 					  dat_t[j] = cSamples_l[j];     //int to int24
		}
		ResampleAndFilter(dat_t,count);
		delete [] dat_t;       
        //////////////////////tova go zabavia ne znam kak /////////////////////////////////////////////////////////////
#if DEBUG > 3
        if (stime == 0)
        {
            stime = time0;
            samples = 0;
        }
        else if (time0 - stime > 5000000)
        {
            samples += nSamples;
            printf("sample rate %.2f\n", 1e6 * samples / (time0 - stime));
            stime = 0;
        }
        else
        {
            samples += nSamples;
        }
#endif
        //////////////////////tova go zabavia ne znam kak /////////////////////////////////////////////////////////////
    }
    else
    {    
        /*const int buf_c = 1024;//4096;//2048;
		short buf[buf_c];
		int bufsize = buf_c * sizeof(short);
        //if (bufsize > buf_c) bufsize = buf_c;
        // Record some data ...
        int error;
        if (pa_simple_read(s_pulse_rx, buf, bufsize, &error) < 0)
        {
            //fprintf(stderr, __FILE__": pa_simple_read() failed: %s\n", pa_strerror(error));
            //goto finish;
            return 0;
        }
		int cnt = bufsize / sizeof(short);
        for (int i = 0; i<cnt; i += 2, count++)//chan_capt
        {
            short si = buf[i + channel_I];
            short sq = buf[i + channel_Q];
            if (si >=  CLIP16 || si <= -CLIP16) quisk_overrange++;	// assume overrange returns max int
            if (sq >=  CLIP16 || sq <= -CLIP16) quisk_overrange++;
            cSamples_l[count] = si;
            cSamples_r[count] = sq;
        }*/
        const int buf_c = 4200;
		unsigned char buf[buf_c];
		int bufsize = buf_c * sizeof(unsigned char);
        int error;
        if (pa_simple_read(s_pulse_rx, buf, bufsize, &error) < 0)
        {
            //fprintf(stderr, __FILE__": pa_simple_read() failed: %s\n", pa_strerror(error));
            //goto finish;
            return 0;
        }
        count = 0; //qDebug()<<LockSize<<SAMP_BUFFER_SIZE_WIN;
		int bits = 2;
 		if (in_bitpersample == 24) bits = 3;
 		if (in_bitpersample == 32) bits = 4;
		int cha_ = rad_sound_state.channel_I*bits;
        for (int i = 0; i < (int)bufsize; i+=2*bits)//2.70 2*2=2chan*2bits
        {
        	int z  = 0; 
        	if 		(bits == 4) z =  buf[i+cha_] + (buf[i+1+cha_] <<  8) + (buf[i+2+cha_] << 16) + (buf[i+3+cha_] << 24);       	
        	else if (bits == 3) z = (buf[i+cha_] <<  8) + (buf[i+1+cha_] << 16) + (buf[i+2+cha_] << 24);
        	else 		   		z = (buf[i+cha_] << 16) + (buf[i+1+cha_] << 24);
       		z = (z >> 8);//to 24-bits	 			        	
            cSamples_l[count] = z;
            count++;
        } //printf("SSS=%d,c=%d\n",cSamples_l[100],count);
		int *dat_t = new int[count+10];  //2.70 old count=512
		for (int j = 0; j < count; ++j) dat_t[j] = cSamples_l[j];//int to short
		ResampleAndFilter(dat_t,count);	
		delete [] dat_t;        
    }
    return count;
}

int MsCore::read_alsa()
{	// read sound samples from the ALSA soundcard
    int i;
    snd_pcm_sframes_t frames, avail;
    short si, sq;
    int ii, qq;
    int nSamples;

#ifdef FIX_H101
    static short save_sq = 0;
#endif

    if (!hCapture)
        return -1;
    avail = 0;
    snd_pcm_delay(hCapture, &avail);	// samples available in the capture buffer

    nSamples = 0;
    if (avail > SAMP_BUFFER_SIZE / chan_capt)		// limit read request to buffer size
        avail = SAMP_BUFFER_SIZE / chan_capt;
    switch (sample_bytes)
    {
    case 2:
        frames = snd_pcm_readi (hCapture, buffer2, avail);	// read available samples
        if (frames == -EAGAIN)
        {	// no samples available
            break;
        }
        else if (frames <= 0)
        {		// error
            rad_sound_state.read_error++;
            snd_pcm_prepare (hCapture);
            snd_pcm_start (hCapture);
            break;
        }
        for (i = 0; frames; i += chan_capt, nSamples++, frames--)
        {
            si = buffer2[i + channel_I];
            sq = buffer2[i + channel_Q];
            if (si >=  CLIP16 || si <= -CLIP16)
                quisk_overrange++;	// assume overrange returns max int
            if (sq >=  CLIP16 || sq <= -CLIP16)
                quisk_overrange++;
            ii = si << 16;
#ifdef FIX_H101
            qq = save_sq << 16;
            save_sq = sq;
#else
            qq = sq << 16;
#endif
            cSamples_l[nSamples] = ii;
            cSamples_r[nSamples] = qq;
        }
        break;
    case 3:
        frames = snd_pcm_readi (hCapture, buffer3, avail);	// read available samples
        if (frames == -EAGAIN)
        {	// no samples available
            break;
        }
        else if (frames <= 0)
        {		// error
            rad_sound_state.read_error++;
            snd_pcm_prepare (hCapture);
            snd_pcm_start (hCapture);
            break;
        }
        for (i = 0; frames; i += chan_capt, nSamples++, frames--)
        {
            ii = qq = 0;
            if (convert_sample_bytes == ThreeLittle2Big)
            {
                *((unsigned char *)&ii    ) = buffer3[(i + channel_I) * 3 + 2];
                *((unsigned char *)&ii + 1) = buffer3[(i + channel_I) * 3 + 1];
                *((unsigned char *)&ii + 2) = buffer3[(i + channel_I) * 3    ];
                *((unsigned char *)&qq    ) = buffer3[(i + channel_Q) * 3 + 2];
                *((unsigned char *)&qq + 1) = buffer3[(i + channel_Q) * 3 + 1];
                *((unsigned char *)&qq + 2) = buffer3[(i + channel_Q) * 3    ];
            }
            else
            {		// ThreeLittle2Little
                memcpy((unsigned char *)&ii + 1, buffer3 + (i + channel_I) * 3, 3);
                memcpy((unsigned char *)&qq + 1, buffer3 + (i + channel_Q) * 3, 3);
            }
            if (ii >=  CLIP32 || ii <= -CLIP32)
                quisk_overrange++;	// assume overrange returns max int
            if (qq >=  CLIP32 || qq <= -CLIP32)
                quisk_overrange++;
            cSamples_l[nSamples] = ii;
            cSamples_r[nSamples] = qq;
        }
        break;
    case 4:
        frames = snd_pcm_readi (hCapture, buffer4, avail);	// read available samples
        if (frames == -EAGAIN)
        {	// no samples available
            break;
        }
        else if (frames <= 0)
        {		// error
            rad_sound_state.read_error++;
            snd_pcm_prepare (hCapture);
            snd_pcm_start (hCapture);
            break;
        }
        for (i = 0; frames; i += chan_capt, nSamples++, frames--)
        {
            ii = buffer4[i + channel_I];
            qq = buffer4[i + channel_Q];
            if (ii >=  CLIP32 || ii <= -CLIP32)
                quisk_overrange++;	// assume overrange returns max int
            if (qq >=  CLIP32 || qq <= -CLIP32)
                quisk_overrange++;
            cSamples_l[nSamples] = ii;
            cSamples_r[nSamples] = qq;
        }
        break;
    }
    return nSamples;
}
void MsCore::rad_open_sound()
{
    rad_close_sound();

    if (!is_pulse_a_in)
    {
        //rad_close_sound();

        is_little_endian = 1;	// Test machine byte order
        if (*(char *)&is_little_endian == 1)
            is_little_endian = 1;
        else
            is_little_endian = 0;

        rad_sound_state.read_error = 0;
        rad_sound_state.write_error = 0;
        rad_sound_state.underrun_error = 0;
        rad_sound_state.interupts = 0;
        rad_sound_state.rate_min = rad_sound_state.rate_max = -99;
        rad_sound_state.chan_min = rad_sound_state.chan_max = -99;
        rad_sound_state.msg1[0] = 0;
        rad_sound_state.err_msg[0] = 0;
        rad_sound_state.bad_device = 1;
        //	rad_sound_state.sdriq_audio_decim = 1.0;	// No audio decimation

        open_alsa_capture();

        //qDebug()<<"------------"<<hCapture<<(bool)rad_sound_state.err_msg[0];
        if (rad_sound_state.err_msg[0])
            return;		// error return
        if (hCapture)
            snd_pcm_start(hCapture);
    }
    else
    {
        // The sample type to use
        /*is_little_endian = 1;	// Test machine byte order
        if (*(char *)&is_little_endian == 1)
          is_little_endian = 1;
        else
          is_little_endian = 0; 
        //is_little_endian = 0; // test*/

        pa_sample_format_t Format_LE_BE = PA_SAMPLE_S16LE;
        if (in_bitpersample==24) Format_LE_BE = PA_SAMPLE_S24LE;
        if (in_bitpersample==32) Format_LE_BE = PA_SAMPLE_S32LE;
        //if (!is_little_endian) Format_LE_BE = PA_SAMPLE_S16BE;

        pa_sample_spec ss;
        ss.format = Format_LE_BE;
        ss.rate = (uint32_t)in_sample_rate;
        ss.channels = 2;

        int latency = 200000;//in useconds  1000000;
        pa_buffer_attr bufattr;
        //memset(&bufattr, 0, sizeof(bufattr));
        bufattr.fragsize = (uint8_t)-1;//=255
        bufattr.maxlength = pa_usec_to_bytes(latency,&ss);
        bufattr.minreq = pa_usec_to_bytes(0,&ss);
        bufattr.prebuf = (uint8_t)-1;//=255
        bufattr.tlength = pa_usec_to_bytes(latency,&ss);
        //qDebug()<<"-----------------"<<(uint8_t)-1;
        /*bufattr.maxlength = (uint32_t)-1;
        bufattr.tlength   = (uint32_t)-1;
        bufattr.prebuf    = (uint32_t)-1;
        bufattr.minreq    = (uint32_t)-1;
        bufattr.fragsize  = (uint32_t)pa_usec_to_bytes(1 * 10, &ss);*/
        /*bufattr.tlength = (uint32_t) latency;
        bufattr.minreq = (uint32_t) 1000;
        bufattr.maxlength = (uint32_t) -1;
        bufattr.prebuf = (uint32_t) -1;
        bufattr.fragsize = (uint32_t) latency;*/

        int error;
        QString str_device_name = (QString)rad_sound_state.dev_capt_name;
        str_device_name.remove("pulse: ");
        if (str_device_name == "default")
        {
        	//qDebug()<<"-----------------------"<<"default";
            if (!(s_pulse_rx = pa_simple_new(NULL, "MSHV", PA_STREAM_RECORD, NULL, "Receive", &ss, NULL, &bufattr, &error)))
            {
                //fprintf(stderr, __FILE__": pa_simple_new() failed: %s\n", pa_strerror(error));
                return;
            }
        }
        else
        {
            //char *ddevv = q-strdup(qPrintable(str_device_name));
			static char ddevv[128];//2.50 
			strncpy(ddevv,str_device_name.toUtf8(),127);
            //qDebug()<<"-----------------------"<<ddevv;
            if (!(s_pulse_rx = pa_simple_new(NULL, "MSHV", PA_STREAM_RECORD, ddevv, "Receive", &ss, NULL, &bufattr, &error)))
            {
                //fprintf(stderr, __FILE__": pa_simple_new() failed: %s\n", pa_strerror(error));
                return;
            }
        }
    }

}
void MsCore::rad_close_sound()
{
    if (hCapture)
    {
        snd_pcm_drop(hCapture);
        snd_pcm_close(hCapture);
        hCapture = NULL;
    }

    strncpy (rad_sound_state.err_msg, CLOSED_TEXT, SC_SIZE_L);
    rad_sound_state.bad_device = 1;
    //qDebug()<<"close handles";

    if (s_pulse_rx)
    {
        pa_simple_free(s_pulse_rx);
        //pa_simple_drain(s_pulse, NULL);
        s_pulse_rx = NULL;
        //usleep(5000000);
    }
}
void MsCore::open_alsa_capture()
{	// Open the ALSA soundcard for capture and perhaps playback.  Detect the
    // capture parameters and make the playback parameters the same.
    int chan_needed, err, dir;
    int poll_size;
    //unsigned int ui;
    snd_pcm_hw_params_t *hware;
    snd_pcm_sw_params_t *sware;
    snd_pcm_uframes_t frames;
    snd_pcm_format_t format;
    //qDebug()<<"format="<<hCapture<<rad_sound_state.dev_capt_name<<SND_PCM_STREAM_CAPTURE<<SND_PCM_NONBLOCK;
    if ((err = snd_pcm_open (&hCapture, rad_sound_state.dev_capt_name, SND_PCM_STREAM_CAPTURE, SND_PCM_NONBLOCK)) < 0)
    {
        //stop 2.26 qt5 gcc7.3.1
        //snprintf(rad_sound_state.err_msg, SC_SIZE_L, "Cannot open capture device %s (%s)",
        //         rad_sound_state.dev_capt_name, snd_strerror (err));
        //qDebug()<<"------------"<<hCapture;
        return;
    }
    if ((err = snd_pcm_sw_params_malloc (&sware)) < 0)
    {
        snprintf (rad_sound_state.err_msg, SC_SIZE_L, "Cannot allocate software parameter structure (%s)\n",
                  snd_strerror (err));
        return;
    }
    if ((err = snd_pcm_hw_params_malloc (&hware)) < 0)
    {
        snprintf (rad_sound_state.err_msg, SC_SIZE_L, "Cannot allocate hardware parameter structure (%s)\n",
                  snd_strerror (err));
        snd_pcm_sw_params_free (sware);
        return;
    }
    if ((err = snd_pcm_hw_params_any (hCapture, hware)) < 0)
    {
        snprintf (rad_sound_state.err_msg, SC_SIZE_L, "Cannot initialize capture parameters (%s)\n",
                  snd_strerror (err));
        goto errend;
    }

    /* UNAVAILABLE
    if ((err = snd_pcm_hw_params_set_rate_resample (hCapture, hware, 0)) < 0) {
    	snprintf (quisk_sound_state.err_msg, SC_SIZE_L, "Cannot disable resampling (%s)\n",
    		snd_strerror (err));
    	goto errend;
    }
    */
    rad_sound_state.bad_device = 0;
    // Get some parameters to send back
    if (snd_pcm_hw_params_get_rate_min(hware, &rad_sound_state.rate_min, &dir) != 0)
        rad_sound_state.rate_min = 0;	// Error
    if (snd_pcm_hw_params_get_rate_max(hware, &rad_sound_state.rate_max, &dir) != 0)
        rad_sound_state.rate_max = 0;	// Error
    if (snd_pcm_hw_params_get_channels_min(hware, &rad_sound_state.chan_min) != 0)
        rad_sound_state.chan_min= 0;	// Error
    if (snd_pcm_hw_params_get_channels_max(hware, &rad_sound_state.chan_max) != 0)
        rad_sound_state.chan_max= 0;	// Error
    // Set the capture parameters
    format = check_formats(hCapture, hware);
    //qDebug()<<"format================="<<format<<SND_PCM_FORMAT_UNKNOWN;
    if (format == SND_PCM_FORMAT_UNKNOWN)
    {
        strncpy (rad_sound_state.err_msg, "MSHV does not support your format.", SC_SIZE_L);
        goto errend;
    }
    //sample_rate = 44100;// rad_sound_state.sample_rate;
    //qDebug()<<sample_rate;
    if (snd_pcm_hw_params_set_rate (hCapture, hware, in_sample_rate, 0) < 0)
    {
        snprintf (rad_sound_state.err_msg, SC_SIZE_L, "Can not set sample rate %d",
                  in_sample_rate);
        goto errend;
    }
    if (snd_pcm_hw_params_set_access (hCapture, hware, SND_PCM_ACCESS_RW_INTERLEAVED) < 0)
    {
        strncpy(rad_sound_state.err_msg, "Interleaved access is not available", SC_SIZE_L);
        goto errend;
    }
    channel_I = rad_sound_state.channel_I;
    channel_Q = rad_sound_state.channel_Q;
    chan_needed = channel_I;	// Set number of channels to highest index + 1
    if (chan_needed < channel_Q)
        chan_needed = channel_Q;
    chan_needed++;
    chan_capt = chan_needed;
    if (chan_capt < (int)rad_sound_state.chan_min)//hv
        chan_capt = rad_sound_state.chan_min;
    if (snd_pcm_hw_params_set_channels (hCapture, hware, chan_capt) < 0)
    {
        snprintf (rad_sound_state.err_msg, SC_SIZE_L, "Can not set channels to %d", chan_capt);
        goto errend;
    }
    // Try to set a capture buffer larger than needed
    frames = in_sample_rate * 200 / 1000;	// buffer size in milliseconds
    if (snd_pcm_hw_params_set_buffer_size_near (hCapture, hware, &frames) < 0)
    {
        snprintf (rad_sound_state.err_msg, SC_SIZE_L, "Can not set capture buffer size");
        goto errend;
    }
    poll_size = in_sample_rate * rad_sound_state.data_poll_usec / 1000000;
    if (frames < (snd_pcm_uframes_t)poll_size * 3)
    {		// buffer size is too small, reduce poll time
        rad_sound_state.data_poll_usec = frames * 1000000 / in_sample_rate / 3;
#if DEBUG
        printf("Reduced data_poll_usec %d for small sound capture buffer\n",
               rad_sound_state.data_poll_usec);
#endif
    }
#if DEBUG
    printf("Capture buffer size %d\n", (int)frames);
    if (frames > SAMP_BUFFER_SIZE / chan_capt)
        printf("Capture buffer exceeds size of sample buffers\n");
#endif
    if ((err = snd_pcm_hw_params (hCapture, hware)) < 0)
    {
        snprintf (rad_sound_state.err_msg, SC_SIZE_L, "Cannot set hw capture parameters (%s)\n",
                  snd_strerror (err));
        goto errend;
    }
    if ((err = snd_pcm_sw_params_current (hCapture, sware)) < 0)
    {
        snprintf (rad_sound_state.err_msg, SC_SIZE_L, "Cannot get software capture parameters (%s)\n",
                  snd_strerror (err));
        goto errend;
    }
    if (chan_capt <= 0) // ima problem po niakoga go dava 0 igarmi hv
        chan_capt = 2;  // ima problem po niakoga go dava 0 igarmi hv
    //qDebug()<<chan_capt;

    if ((err = snd_pcm_prepare (hCapture)) < 0)
    {
        snprintf (rad_sound_state.err_msg, SC_SIZE_L, "Cannot prepare capture interface for use (%s)\n",
                  snd_strerror (err));
        goto errend;
    }
    // Now open the microphone
	//if (rad_sound_state.mic_dev_name[0])
		//open_microphone();

errend:
    snd_pcm_hw_params_free (hware);
    snd_pcm_sw_params_free (sware); ///qDebug()<<sample_bytes;//qDebug()<<"read";
    return;
}

snd_pcm_format_t MsCore::check_formats(snd_pcm_t *h, snd_pcm_hw_params_t *hware)
{
    snd_pcm_format_t format = SND_PCM_FORMAT_UNKNOWN;
    sample_bytes = 0;

    strncpy (rad_sound_state.msg1, "Available formats: ", SC_SIZE_L);
    if (snd_pcm_hw_params_test_format (h, hware, SND_PCM_FORMAT_S32) == 0)
    {
        if (!sample_bytes)
        {
            strncat(rad_sound_state.msg1, "*", SC_SIZE_L);
            sample_bytes = 4;
            format = SND_PCM_FORMAT_S32;
        }
        strncat(rad_sound_state.msg1, "S32 ", SC_SIZE_L); //printf("SSS=%d\n",1);
    }
    if (snd_pcm_hw_params_test_format (h, hware, SND_PCM_FORMAT_U32) == 0)
    {
        strncat(rad_sound_state.msg1, "U32 ", SC_SIZE_L); //printf("SSS=%d\n",2);
    }
    if (snd_pcm_hw_params_test_format (h, hware, SND_PCM_FORMAT_S24) == 0)
    {
        strncat(rad_sound_state.msg1, "S24 ", SC_SIZE_L); //printf("SSS=%d\n",3);
    }
    if (snd_pcm_hw_params_test_format (h, hware, SND_PCM_FORMAT_U24) == 0)
    {
        strncat(rad_sound_state.msg1, "U24 ", SC_SIZE_L); //printf("SSS=%d\n",4);
    }
    if (snd_pcm_hw_params_test_format (h, hware, SND_PCM_FORMAT_S24_3LE) == 0)
    {
        if (!sample_bytes)
        {
            strncat(rad_sound_state.msg1, "*", SC_SIZE_L);
            sample_bytes = 3;
            format = SND_PCM_FORMAT_S24_3LE;
            if (is_little_endian)
                convert_sample_bytes = ThreeLittle2Little;	// Convert little endian to little
            else
                convert_sample_bytes = ThreeLittle2Big;	// Convert little endian to big
        }
        strncat(rad_sound_state.msg1, "S24_3LE ", SC_SIZE_L); //printf("SSS=%d\n",5);
    }
    if (snd_pcm_hw_params_test_format (h, hware, SND_PCM_FORMAT_S16) == 0)
    {
        if (!sample_bytes)
        {
            strncat(rad_sound_state.msg1, "*", SC_SIZE_L);
            sample_bytes = 2;
            format = SND_PCM_FORMAT_S16;
        }
        strncat(rad_sound_state.msg1, "S16 ", SC_SIZE_L); //printf("SSS=%d\n",6);
    }
    if (snd_pcm_hw_params_test_format (h, hware, SND_PCM_FORMAT_U16) == 0)
    {
        strncat(rad_sound_state.msg1, "U16 ", SC_SIZE_L); //printf("SSS=%d\n",7);
    }
    if (format == SND_PCM_FORMAT_UNKNOWN)
        strncat(rad_sound_state.msg1, "*UNSUPPORTED", SC_SIZE_L);
    else
        snd_pcm_hw_params_set_format (h, hware, format);

    return format;
}
#endif
