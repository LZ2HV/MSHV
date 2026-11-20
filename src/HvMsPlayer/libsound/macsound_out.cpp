/* MSHV macOS PortAudio playback backend
 * Drop-in replacement for linsound_out.cpp on macOS.
 * Implements Rawplayer::mac_* methods using PortAudio over CoreAudio.
 */

#include "mpegsound.h"

#if defined _MACOS_

#include <portaudio.h>
#include <QString>
#include <QByteArray>

static PaStream *s_pa_tx = NULL;
static int s_pa_tx_device = paNoDevice;
static bool s_pa_initialized = false;
static int  s_pa_tx_chans = 2;       // channels actually opened on the stream
static PaSampleFormat s_pa_tx_fmt = paInt16;
// Cached open params — used to skip the close+reopen dance if MSHV calls
// resetsoundtype() with the same configuration (which it does for every TX
// message block; CoreAudio close+open takes ~100 ms each → distorted TX).
static int s_pa_tx_open_speed = 0;
static int s_pa_tx_open_size  = 0;
static int s_pa_tx_open_chans = 0;
static int s_pa_tx_open_dev   = paNoDevice;

static void ensure_pa_init()
{
    if (!s_pa_initialized)
    {
        Pa_Initialize();
        s_pa_initialized = true;
    }
}

static int pick_output_device(const char *device_name)
{
    QString needle = QString::fromUtf8(device_name ? device_name : "");
    if (needle.isEmpty() || needle == "default" || needle.startsWith("default:") || needle.startsWith("pulse: "))
        return Pa_GetDefaultOutputDevice();

    int n = Pa_GetDeviceCount();
    for (int i = 0; i < n; ++i)
    {
        const PaDeviceInfo *info = Pa_GetDeviceInfo(i);
        if (info && info->maxOutputChannels > 0 && needle == QString::fromUtf8(info->name))
            return i;
    }
    return Pa_GetDefaultOutputDevice();
}

void Rawplayer::mac_destroy()
{
    // Deliberately keep s_pa_tx alive across Rawplayer destructor calls.
    // MSHV news/deletes a Rawplayer per TX message (msplayerhv.cpp:560/582),
    // and CoreAudio close+open takes ~100 ms each — re-doing it per message
    // produces audible flutter and chopped TX audio. The next message hits
    // mac_resetsoundtype's cache and reuses the existing open stream. The
    // stream gets a real close on process exit via Pa_Terminate (or never,
    // which CoreAudio handles fine).
}

bool Rawplayer::mac_initialize(char *device_name, int bpsmpl)
{
    ensure_pa_init();
    rawspeed       = 11025;
    rawsamplesize  = bpsmpl;
    rawstereo      = 1;
    rawchannels    = 2;
    s_pa_tx_device = pick_output_device(device_name);
    return s_pa_tx_device != paNoDevice;
}

bool Rawplayer::mac_resetsoundtype()
{
    if (s_pa_tx_device == paNoDevice)
    {
        s_pa_tx_device = Pa_GetDefaultOutputDevice();
        if (s_pa_tx_device == paNoDevice) return false;
    }

    // Skip the open dance if nothing actually changed. MSHV calls
    // setsoundtype/resetsoundtype on every TX message block; closing and
    // reopening a CoreAudio stream takes ~100 ms each and produces audible
    // clicks plus phase discontinuities → "low + distorted" TX.
    if (s_pa_tx
        && s_pa_tx_open_speed == rawspeed
        && s_pa_tx_open_size  == rawsamplesize
        && s_pa_tx_open_chans == rawchannels
        && s_pa_tx_open_dev   == s_pa_tx_device)
    {
        return true;
    }

    if (s_pa_tx)
    {
        Pa_StopStream(s_pa_tx);
        Pa_CloseStream(s_pa_tx);
        s_pa_tx = NULL;
    }

    PaSampleFormat fmt = paInt16;
    if (rawsamplesize == 24) fmt = paInt24;
    else if (rawsamplesize == 32) fmt = paInt32;

    // For 24-bit on Mac, force paFloat32 stream format. PortAudio's internal
    // int24 → CoreAudio-native-Float32 conversion produces audible
    // distortion against virtual SDR audio devices (CommonRadioAudio,
    // observed). We do the int24 → float32 conversion ourselves in
    // mac_putblock — same byte budget, fewer surprises.
    if (rawsamplesize == 24) fmt = paFloat32;

    const PaDeviceInfo *info = Pa_GetDeviceInfo(s_pa_tx_device);

    // Clamp channel count to what the device exposes. Virtual SDR sinks
    // (FlexRadio DAX TX, AetherSDR CommonRadioAudio In N) accept mono only;
    // a hardcoded stereo open silently fails and TX produces no audio.
    int chans = rawchannels;
    if (info && info->maxOutputChannels > 0 && info->maxOutputChannels < chans)
        chans = info->maxOutputChannels;

    PaStreamParameters params;
    params.device                    = s_pa_tx_device;
    params.channelCount              = chans;
    params.sampleFormat              = fmt;
    // Match the RX path: ~200 ms of CoreAudio output buffer. The default
    // (~5 ms) underflows between MSHV's putblock calls (~57 ms each at 24-
    // bit stereo 48k), producing periodic silence gaps that sound like
    // fluttering. 200 ms gives comfortable headroom; FT8 TX has no latency
    // requirement tighter than that.
    params.suggestedLatency          = 0.2;
    params.hostApiSpecificStreamInfo = NULL;

    double rates[3]            = { (double)rawspeed,
                                   info ? info->defaultSampleRate : 0.0,
                                   0.0 };
    PaSampleFormat formats[4]  = { fmt, paFloat32, paInt16, paInt32 };

    bool opened_ok = false;
    PaError last_err = paUnanticipatedHostError;
    for (int ri = 0; ri < 3 && !opened_ok; ++ri)
    {
        if (rates[ri] <= 0.0) continue;
        for (int fi = 0; fi < 4 && !opened_ok; ++fi)
        {
            params.sampleFormat = formats[fi];
            if (Pa_IsFormatSupported(NULL, &params, rates[ri]) != paFormatIsSupported)
                continue;
            PaError err = Pa_OpenStream(&s_pa_tx, NULL, &params, rates[ri],
                                        paFramesPerBufferUnspecified, paClipOff, NULL, NULL);
            last_err = err;
            if (err == paNoError && s_pa_tx != NULL)
            {
                s_pa_tx_chans = chans;
                s_pa_tx_fmt   = formats[fi];
                s_pa_tx_open_speed = rawspeed;
                s_pa_tx_open_size  = rawsamplesize;
                s_pa_tx_open_chans = rawchannels;
                s_pa_tx_open_dev   = s_pa_tx_device;
                fprintf(stderr, "[MSHV mac audio out] %s: %.0f Hz %d ch fmt=0x%lx\n",
                        info ? info->name : "?", rates[ri], chans, (unsigned long)formats[fi]);
                opened_ok = true;
            }
        }
    }
    if (!opened_ok)
    {
        fprintf(stderr, "[MSHV mac audio out] OPEN FAILED: %s (dev=%s req=%dch %dHz)\n",
                Pa_GetErrorText(last_err), info ? info->name : "?", chans, rawspeed);
        s_pa_tx = NULL;
        return false;
    }
    PaError err = paNoError;
    err = Pa_StartStream(s_pa_tx);
    if (err != paNoError)
    {
        fprintf(stderr, "[MSHV mac audio out] START FAILED: %s\n", Pa_GetErrorText(err));
        Pa_CloseStream(s_pa_tx);
        s_pa_tx = NULL;
        return false;
    }
    audiobuffersize = 8192;
    return true;
}

bool Rawplayer::mac_putblock(void *buffer, int size)
{
    if (!s_pa_tx) return false;
    int bits = 2;
    if (rawsamplesize == 24) bits = 3;
    if (rawsamplesize == 32) bits = 4;
    unsigned long frames = (unsigned long)(size / rawchannels / bits);

    // 24-bit MSHV → Float32 PortAudio conversion path.
    // We force paFloat32 in mac_resetsoundtype for 24-bit because
    // PortAudio's own int24→Float32 conversion produces distortion on
    // CommonRadioAudio. Unpack each MSHV int24 (3 bytes LE, signed) into a
    // float in [-1, 1] and write to the stream as float32. Downmix to mono
    // at the same time if the device is mono.
    if (rawsamplesize == 24 && s_pa_tx_fmt == paFloat32)
    {
        static float fbuf[8192];
        unsigned long out_frames = frames;
        unsigned long buf_floats = out_frames * (unsigned long)s_pa_tx_chans;
        if (buf_floats > sizeof(fbuf)/sizeof(fbuf[0]))
        {
            buf_floats = sizeof(fbuf)/sizeof(fbuf[0]);
            out_frames = buf_floats / (unsigned long)s_pa_tx_chans;
        }
        const unsigned char *src = (const unsigned char *)buffer;
        const float scale = 1.0f / 8388608.0f;   // int24 max+1
        for (unsigned long f = 0; f < out_frames; ++f)
        {
            for (int ch = 0; ch < s_pa_tx_chans; ++ch)
            {
                int src_ch = ch;
                if (src_ch >= rawchannels) src_ch = 0;
                const unsigned char *p = src + (f*rawchannels + src_ch)*3;
                // 3-byte LE int24, sign-extended into int32.
                int32_t s = (int32_t)((uint32_t)p[0] |
                                      ((uint32_t)p[1] << 8) |
                                      ((uint32_t)p[2] << 16));
                if (s & 0x800000) s |= 0xFF000000;   // sign extend
                fbuf[f*s_pa_tx_chans + ch] = (float)s * scale;
            }
        }
        PaError fe = Pa_WriteStream(s_pa_tx, fbuf, out_frames);
        if (fe != paNoError && fe != paOutputUnderflowed) return false;
        return true;
    }

    PaError err;
    // MSHV always generates stereo TX audio (rawchannels = 2). If the device
    // we opened is mono (e.g. AetherSDR/FlexRadio CommonRadioAudio In N),
    // writing stereo data byte-for-byte produces interleaved L/R samples
    // played as if they were a mono stream — that's the "distorted" sound.
    // Downmix to the device channel count first.
    if (s_pa_tx_chans == rawchannels)
    {
        err = Pa_WriteStream(s_pa_tx, buffer, frames);
    }
    else if (s_pa_tx_chans == 1 && rawchannels == 2)
    {
        // Take channel 0 from each stereo frame. MSHV duplicates TX audio
        // across both channels so dropping the right is lossless.
        static unsigned char downmix[16384]; // > audiobuffersize=8192
        unsigned long out_bytes = frames * (unsigned long)bits;
        if (out_bytes > sizeof(downmix)) out_bytes = sizeof(downmix);
        const unsigned char *src = (const unsigned char *)buffer;
        for (unsigned long f = 0; f < frames; ++f)
            memcpy(downmix + f*bits, src + f*rawchannels*bits, bits);
        err = Pa_WriteStream(s_pa_tx, downmix, frames);
    }
    else
    {
        // Unhandled channel mismatch — bail rather than send junk.
        fprintf(stderr, "[MSHV mac audio out] channel mismatch: stream=%d raw=%d\n",
                s_pa_tx_chans, rawchannels);
        return false;
    }

    // paOutputUnderflowed is non-fatal; we just keep going.
    if (err != paNoError && err != paOutputUnderflowed)
        return false;
    return true;
}

#endif // _MACOS_
