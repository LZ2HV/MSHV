#include "mscore.h"

#if defined _MACOS_

#include <portaudio.h>
#include <string.h>

static PaStream *s_pa_cap_stream = NULL;

static int find_input_device(const char *name)
{
    int numDevices = Pa_GetDeviceCount();
    if (numDevices < 0) return -1;
    for (int i = 0; i < numDevices; i++)
    {
        const PaDeviceInfo *info = Pa_GetDeviceInfo(i);
        if (info && info->maxInputChannels > 0)
        {
            if (strstr(info->name, name) != NULL)
                return i;
        }
    }
    return -1;
}

int MsCore::alsa_read_sound()
{
    if (!s_pa_cap_stream)
        return 0;

    int count = 0;

    if (sample_bytes == 2)
    {
        const int frames_to_read = 1050;
        short buf[frames_to_read * 2];
        PaError err = Pa_ReadStream(s_pa_cap_stream, buf, frames_to_read);
        if (err != paNoError)
            return 0;

        for (int i = 0; i < frames_to_read; i++, count++)
        {
            int z = (int)buf[i * 2 + channel_I];
            z = z << 8;
            cSamples_l[count] = z;
        }
    }
    else
    {
        const int frames_to_read = 1050;
        int buf32[frames_to_read * 2];
        PaError err = Pa_ReadStream(s_pa_cap_stream, buf32, frames_to_read);
        if (err != paNoError)
            return 0;

        for (int i = 0; i < frames_to_read; i++, count++)
        {
            int z = buf32[i * 2 + channel_I];
            z = z >> 8;
            cSamples_l[count] = z;
        }
    }

    int *dat_t = new int[count + 10];
    for (int j = 0; j < count; ++j)
        dat_t[j] = cSamples_l[j];
    ResampleAndFilter(dat_t, count);
    delete [] dat_t;

    return count;
}

void MsCore::rad_open_sound()
{
    rad_close_sound();

    rad_sound_state.read_error = 0;
    rad_sound_state.write_error = 0;
    rad_sound_state.underrun_error = 0;
    rad_sound_state.interupts = 0;
    rad_sound_state.bad_device = 1;
    rad_sound_state.err_msg[0] = 0;

    PaError err = Pa_Initialize();
    if (err != paNoError)
        return;

    channel_I = rad_sound_state.channel_I;
    channel_Q = rad_sound_state.channel_Q;

    PaSampleFormat fmt = paInt16;
    sample_bytes = 2;
    if (in_bitpersample == 24)
    {
        fmt = paInt32;
        sample_bytes = 4;
    }
    if (in_bitpersample == 32)
    {
        fmt = paInt32;
        sample_bytes = 4;
    }

    QString str_device_name = (QString)rad_sound_state.dev_capt_name;
    str_device_name.remove("pulse: ");

    PaStreamParameters inputParams;
    memset(&inputParams, 0, sizeof(inputParams));
    inputParams.device = Pa_GetDefaultInputDevice();
    inputParams.channelCount = 2;
    inputParams.sampleFormat = fmt;
    inputParams.suggestedLatency = 0.1;
    inputParams.hostApiSpecificStreamInfo = NULL;

    if (str_device_name != "default" && !str_device_name.isEmpty())
    {
        int devIdx = find_input_device(str_device_name.toUtf8().constData());
        if (devIdx >= 0)
            inputParams.device = devIdx;
    }

    if (inputParams.device == paNoDevice)
        return;

    err = Pa_OpenStream(&s_pa_cap_stream, &inputParams, NULL,
                        in_sample_rate, paFramesPerBufferUnspecified,
                        paClipOff, NULL, NULL);
    if (err != paNoError)
    {
        s_pa_cap_stream = NULL;
        return;
    }

    err = Pa_StartStream(s_pa_cap_stream);
    if (err != paNoError)
    {
        Pa_CloseStream(s_pa_cap_stream);
        s_pa_cap_stream = NULL;
        return;
    }

    rad_sound_state.bad_device = 0;
}

void MsCore::rad_close_sound()
{
    if (s_pa_cap_stream)
    {
        Pa_StopStream(s_pa_cap_stream);
        Pa_CloseStream(s_pa_cap_stream);
        s_pa_cap_stream = NULL;
    }

    strncpy(rad_sound_state.err_msg, CLOSED_TEXT, SC_SIZE_L);
    rad_sound_state.bad_device = 1;
}

#endif
