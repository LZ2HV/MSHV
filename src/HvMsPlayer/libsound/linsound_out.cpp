#include "mpegsound.h"
#if defined _LINUX_
//#include <QtGui>

#include <pulse/pulseaudio.h>
#include <pulse/simple.h>
pa_simple *s_pulse_tx = NULL;
void Rawplayer::lin_destroy()
{
    if (playback_handle) //2.37 protect handle
    {
        snd_pcm_close(playback_handle);
        playback_handle = NULL;
        //qDebug()<<"---------------"<<"Destroy ALSA";
    }

    if (s_pulse_tx)
    {
        /* Make sure that every single sample was played */
        int error = 0;
        if (pa_simple_drain(s_pulse_tx, &error) < 0)
        {
            //fprintf(stderr, __FILE__": pa_simple_drain() failed: %s\n", pa_strerror(error));
            //goto finish;
        }
        pa_simple_free(s_pulse_tx);
        s_pulse_tx = NULL;
        //qDebug()<<"---------------"<<"Destroy PULSE";
    }
}
bool is_pulse_a_out = true;//2.65 by default
bool Rawplayer::lin_initialize(char *device_name,int bpsmpl)
{
    s_pulse_tx = NULL;
    //if ((QString)device_name=="pulse") is_pulse_a_out = true;
    //else is_pulse_a_out = false;
    QString str_device_name = (QString)device_name;
    QString str_device_name0 = str_device_name+"xxxxxxx";//2.70
    if (str_device_name0.mid(0,7)=="pulse: ") is_pulse_a_out = true;
    else is_pulse_a_out = false;

    rawspeed = 11025;// tova e nai niskoto
    rawsamplesize = bpsmpl; //printf("sample_bytes=%d\n",rawsamplesize);
    rawstereo = 1;
    rawchannels = 2;

    int err = 0;
    playback_handle = NULL;
    if (!is_pulse_a_out)
    {
        /*if ((err = snd_pcm_open (&playback_handle, device_name, SND_PCM_STREAM_PLAYBACK,0)) < 0)
        {
            fprintf (stderr, "cannot open audio device %s (%s)\n", device_name, snd_strerror (err));
            return false;
        }*/
        bool errtagn = true;
        if ((err = snd_pcm_open (&playback_handle, device_name, SND_PCM_STREAM_PLAYBACK,0)) < 0)
        {
            errtagn = false;
        }
        int c_retry = 0;
        while (!errtagn)
        {
            usleep(100000);
            if ((err = snd_pcm_open (&playback_handle, device_name, SND_PCM_STREAM_PLAYBACK,0)) < 0)
                errtagn = false;
            else
                errtagn = true;
            c_retry++;
            if (c_retry>120) break;
        }
        if (!errtagn)
        {
            fprintf (stderr, "cannot open audio device %s (%s)\n", device_name, snd_strerror (err));
            return false;
        }
        //qDebug()<<"---------------"<<"initialize ALSA"<<playback_handle<<errtagn<<device_name;
    }
    else
    {
        /*int is_little_endian = 1;	// Test machine byte order
        if (*(char *)&is_little_endian == 1)
          is_little_endian = 1;
        else
          is_little_endian = 0; 
        //is_little_endian = 0; // test*/

        pa_sample_format_t Format_LE_BE = PA_SAMPLE_S16LE;
        if (rawsamplesize==24) Format_LE_BE = PA_SAMPLE_S24LE;
        if (rawsamplesize==32) Format_LE_BE = PA_SAMPLE_S32LE;
        //if (!is_little_endian) Format_LE_BE = PA_SAMPLE_S16BE;

        pa_sample_spec ss;
        ss.format = Format_LE_BE;
        ss.rate = (uint32_t)Rawplayer::pa_sa_rate;
        ss.channels = 2;

        /*int latency = 200000;//in useconds  1000000;
        pa_buffer_attr bufattr;
        //memset(&bufattr, 0, sizeof(bufattr));
        bufattr.fragsize = (uint32_t)-1;//=255
        bufattr.maxlength = pa_usec_to_bytes(latency,&ss);
        bufattr.minreq = pa_usec_to_bytes(0,&ss);
        bufattr.prebuf = (uint32_t)-1;//=255
        bufattr.tlength = pa_usec_to_bytes(latency,&ss);*/

        int error = 0;
        str_device_name.remove("pulse: ");
        if (str_device_name == "default")
        {
        	//qDebug()<<"-----------------------"<<"default";
            if (!(s_pulse_tx = pa_simple_new(NULL, "MSHV", PA_STREAM_PLAYBACK, NULL, "Transmit", &ss, NULL, NULL, &error)))
            {
                //fprintf(stderr, __FILE__": pa_simple_new() failed: %s\n", pa_strerror(error));
                return false;
            }
        }
        else
        {
            //char *ddevv = q-strdup(qPrintable(str_device_name));
			static char ddevv[128];//2.50 
			strncpy(ddevv,str_device_name.toUtf8(),127);
            //qDebug()<<"-----------------------"<<ddevv;
            if (!(s_pulse_tx = pa_simple_new(NULL, "MSHV", PA_STREAM_PLAYBACK, ddevv, "Transmit", &ss, NULL, NULL, &error)))
            {
                //fprintf(stderr, __FILE__": pa_simple_new() failed: %s\n", pa_strerror(error));
                return false;
            }
        } //qDebug()<<"---------------"<<"initialize PULSE"<<(uint32_t)Rawplayer::pa_sa_rate;
    }
	//printf("sample_bytes=%d\n",rawsamplesize);
    audiobuffersize = 8192;//2.70 kratno na 3*2bit  old=4096;//vazno ina4e garmi wav

    return true;
}
bool Rawplayer::lin_resetsoundtype()
{
    if (!is_pulse_a_out)
    {
        //qDebug()<<"lin_out"<<rawchannels<<rawsamplesize<<rawspeed;
        int err = 0;
        snd_pcm_hw_params_t *hw_params = NULL;//alsa
        snd_pcm_drop(playback_handle);// iztriva bufera immediatly vazno hv
        snd_pcm_drain(playback_handle); // vazno hv
        //snd_pcm_format_t format;
        //snd_pcm_reset(playback_handle);
        //snd_pcm_hwsync(playback_handle);

        if ((err = snd_pcm_hw_params_malloc (&hw_params)) < 0)
        {
            fprintf (stderr, "cannot allocate hardware parameter structure (%s)\n",snd_strerror (err));
            return false;
        }
        if ((err = snd_pcm_hw_params_any(playback_handle, hw_params)) < 0)
        {
            fprintf (stderr, "cannot initialize hardware parameter structure (%s)\n",snd_strerror (err));
            return false;
        }

        //SND_PCM_ACCESS_RW_INTERLEAVED  SND_PCM_ACCESS_MMAP_INTERLEAVED
        if ((err = snd_pcm_hw_params_set_access(playback_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0)
        {
            fprintf (stderr, "cannot set access type (%s)\n",snd_strerror (err));
            return false;
        }
        if ((err = snd_pcm_hw_params_set_rate_resample(playback_handle, hw_params, 1)) < 0)
        {
            fprintf (stderr, "cannot resample (%s)\n",snd_strerror (err));
            return false;
        }

        snd_pcm_format_t fmt = SND_PCM_FORMAT_S16_LE;
        if (rawsamplesize == 24) fmt =  SND_PCM_FORMAT_S24_LE;
        if (rawsamplesize == 32) fmt =  SND_PCM_FORMAT_S32_LE;
        if ((err = snd_pcm_hw_params_set_format (playback_handle, hw_params, fmt)) < 0)
        {
            fprintf (stderr, "cannot set sample format (%s)\n",snd_strerror (err));
            return false;
        }

        //rawspeed = 48000;//(unsigned*)&rawspeed
        if ((err = snd_pcm_hw_params_set_rate(playback_handle, hw_params, rawspeed, 0)) < 0)
            //if ((err = snd_pcm_hw_params_set_rate_near (playback_handle, hw_params, (unsigned*)&rawspeed, 0)) < 0)
        {
            fprintf (stderr, "cannot set sample rate (%s)\n",snd_strerror (err));
            return false;
        }

        //qDebug()<<"Chanels"<<rawchannels;
        //rawchannels = 1;
        if ((err = snd_pcm_hw_params_set_channels(playback_handle, hw_params, rawchannels)) < 0)
        {
            fprintf (stderr, "cannot set channel count (%s)\n",snd_strerror (err));
            return false;
        }
        /////////////////////////////// seutp buffers hv /////////////////////////////////////////
        unsigned periodss = 12; // x-fi via ich5
        snd_pcm_uframes_t buffer_size = 8192; //2.70 4096; // x-fi via ich5
        snd_pcm_uframes_t period_size = 1800; // x-fi via ich5

        if ((err = snd_pcm_hw_params_set_periods_near(playback_handle, hw_params, &periodss, 0)) < 0)
        {
            fprintf (stderr, "cannot set number of periods (%s)\n", snd_strerror (err));
            return false;
        }
        if ((err = snd_pcm_hw_params_set_period_size_near (playback_handle, hw_params, &period_size, 0)) < 0)
        {
            fprintf (stderr, "cannot set period size (%s)\n", snd_strerror (err));
            return false;
        }
        if ((err = snd_pcm_hw_params_set_buffer_size_near (playback_handle, hw_params, &buffer_size)) < 0)
        {
            fprintf (stderr, "cannot set buffer size (%s)\n", snd_strerror (err));
            return false;
        }

        if ((err = snd_pcm_hw_params (playback_handle, hw_params)) < 0)
        {
            fprintf (stderr, "cannot set parameters (%s)\n",snd_strerror (err));
            return false;
        }
        snd_pcm_hw_params_free(hw_params);

        snd_pcm_sw_params_t *sw_params;
        if ((err = snd_pcm_sw_params_malloc (&sw_params)) < 0)
        {
            fprintf (stderr, "cannot allocate software parameters structure (%s)\n", snd_strerror (err));
            return false;
        }
        if ((err = snd_pcm_sw_params_current (playback_handle, sw_params)) < 0)
        {
            fprintf (stderr, "cannot initialize software parameters structure (%s)\n", snd_strerror (err));
            return false;
        }
        if ((err = snd_pcm_sw_params_set_avail_min (playback_handle, sw_params, 1024)) < 0)
        {
            fprintf (stderr, "cannot set minimum available count (%s)\n", snd_strerror (err));
            return false;
        }
        if ((err = snd_pcm_sw_params_set_start_threshold (playback_handle, sw_params, 0U)) < 0)
        {
            fprintf (stderr, "cannot set start mode (%s)\n", snd_strerror (err));
            return false;
        }
        if ((err = snd_pcm_sw_params (playback_handle, sw_params)) < 0)
        {
            fprintf (stderr, "cannot set software parameters (%s)\n", snd_strerror (err));
            return false;
        }
        snd_pcm_sw_params_free(sw_params);

        snd_pcm_prepare (playback_handle);

        //unsigned int tmp;
        //snd_pcm_hw_params_get_rate(hw_params, &tmp, 0);
        //qDebug()<<"---------------"<<"resetsoundtype ALSA";
    }
    return true;
}
bool Rawplayer::lin_putblock(void *buffer,int size)
{
    if (!is_pulse_a_out)
    {
        int err;
        int bits = 2;
        if (rawsamplesize == 24) bits = 3;
        if (rawsamplesize == 32) bits = 4;
        int frames = (size / rawchannels / (sizeof(unsigned char)*bits));// frames=1024  size=4096
        unsigned char *framedata = (unsigned char*)buffer;
        if ((err = snd_pcm_writei (playback_handle,framedata,frames)) != frames)
        {
            snd_pcm_prepare (playback_handle);
            //fprintf (stderr, "write to audio interface failed (%s)\n",snd_strerror (err));
            return false;
        }
    }
    else
    {
        int error = 0;  
        if (pa_simple_write(s_pulse_tx, buffer, size, &error) < 0)
        {
            //fprintf(stderr, __FILE__": pa_simple_write() failed: %s\n", pa_strerror(error));
            return false;
        }
    }
    return true;
}
#endif