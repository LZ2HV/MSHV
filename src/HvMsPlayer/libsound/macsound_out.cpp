#include "mpegsound.h"

#if defined _MACOS_

#include <portaudio.h>
#include <string.h>

static PaStream *s_pa_play_stream = NULL;
static int s_pa_play_fmt_bytes = 2;
static int s_conv_buf[8192];

static int find_output_device(const char *name)
{
    int numDevices = Pa_GetDeviceCount();
    if (numDevices < 0) return -1;
    for (int i = 0; i < numDevices; i++)
    {
        const PaDeviceInfo *info = Pa_GetDeviceInfo(i);
        if (info && info->maxOutputChannels > 0)
        {
            if (strstr(info->name, name) != NULL)
                return i;
        }
    }
    return -1;
}

static char s_mac_play_device[128];

void Rawplayer::lin_destroy()
{
    if (s_pa_play_stream)
    {
        Pa_StopStream(s_pa_play_stream);
        Pa_CloseStream(s_pa_play_stream);
        s_pa_play_stream = NULL;
    }
}

bool Rawplayer::lin_initialize(char *device_name, int bpsmpl)
{
    s_pa_play_stream = NULL;

    PaError err = Pa_Initialize();
    if (err != paNoError)
        return false;

    strncpy(s_mac_play_device, device_name, 127);
    s_mac_play_device[127] = 0;

    rawspeed = 11025;
    rawsamplesize = bpsmpl;
    rawstereo = 1;
    rawchannels = 2;

    audiobuffersize = 8192;

    return true;
}

bool Rawplayer::lin_resetsoundtype()
{
    if (s_pa_play_stream)
    {
        Pa_StopStream(s_pa_play_stream);
        Pa_CloseStream(s_pa_play_stream);
        s_pa_play_stream = NULL;
    }

    PaSampleFormat fmt = paInt16;
    s_pa_play_fmt_bytes = 2;
    if (rawsamplesize == 24)
    {
        fmt = paInt32;
        s_pa_play_fmt_bytes = 3;
    }
    else if (rawsamplesize == 32)
    {
        fmt = paInt32;
        s_pa_play_fmt_bytes = 4;
    }

    PaStreamParameters outputParams;
    memset(&outputParams, 0, sizeof(outputParams));
    outputParams.device = Pa_GetDefaultOutputDevice();
    outputParams.channelCount = rawchannels;
    outputParams.sampleFormat = fmt;
    outputParams.suggestedLatency = 0.1;
    outputParams.hostApiSpecificStreamInfo = NULL;

    QString str_device_name = (QString)s_mac_play_device;
    str_device_name.remove("pulse: ");
    if (str_device_name != "default" && !str_device_name.isEmpty())
    {
        int devIdx = find_output_device(str_device_name.toUtf8().constData());
        if (devIdx >= 0)
            outputParams.device = devIdx;
    }

    if (outputParams.device == paNoDevice)
        return false;

    PaError err = Pa_OpenStream(&s_pa_play_stream, NULL, &outputParams,
                                rawspeed, paFramesPerBufferUnspecified,
                                paClipOff, NULL, NULL);
    if (err != paNoError)
    {
        s_pa_play_stream = NULL;
        return false;
    }

    err = Pa_StartStream(s_pa_play_stream);
    if (err != paNoError)
    {
        Pa_CloseStream(s_pa_play_stream);
        s_pa_play_stream = NULL;
        return false;
    }

    return true;
}

bool Rawplayer::lin_putblock(void *buffer, int size)
{
    if (!s_pa_play_stream)
        return false;

    if (s_pa_play_fmt_bytes == 3)
    {
        int num_samples = size / 3;
        if (num_samples > 8192) num_samples = 8192;
        unsigned char *src = (unsigned char *)buffer;
        for (int i = 0; i < num_samples; i++)
        {
            s_conv_buf[i] = (int)(
                ((unsigned int)src[i * 3] << 8) |
                ((unsigned int)src[i * 3 + 1] << 16) |
                ((unsigned int)src[i * 3 + 2] << 24));
        }
        int frames = num_samples / rawchannels;
        PaError err = Pa_WriteStream(s_pa_play_stream, s_conv_buf, frames);
        if (err != paNoError)
            return false;
    }
    else
    {
        int bytes_per_sample = s_pa_play_fmt_bytes;
        int frames = size / (rawchannels * bytes_per_sample);
        PaError err = Pa_WriteStream(s_pa_play_stream, buffer, frames);
        if (err != paNoError)
            return false;
    }

    return true;
}

#endif
