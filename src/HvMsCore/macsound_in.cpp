/* MSHV macOS PortAudio capture backend
 * Drop-in replacement for linsound_in.cpp on macOS.
 * Implements MsCore::rad_open_sound, rad_close_sound, alsa_read_sound on macOS
 * via PortAudio over CoreAudio.
 */

#include "mscore.h"

#if defined _MACOS_

#include <portaudio.h>
#include <QString>
#include <QByteArray>
#include <string.h>

static PaStream *s_pa_rx = NULL;
static int s_pa_rx_device = paNoDevice;
static bool s_pa_in_initialized = false;
static bool s_pa_in_is_float = false;
static int  s_pa_in_chan_capt = 2;

// Scratch buffer for the 24-bit->int24 normalisation below. alsa_read_sound()
// is driven by the 5 ms tick, so the old `new int[]`/`delete[]` here was a
// malloc/free pair a few hundred times a second on a path that must not stall
// (F7). nSamples is bounded by SAMP_BUFFER_SIZE, and cSamples_l/cSamples_r are
// already fixed-size members sized SAMP_BUFFER_SIZE+1000 — this one matches
// them. The capture path is single-threaded, so a file-static is safe.
static int s_scratch_dat_t[SAMP_BUFFER_SIZE + 1000];

static void ensure_pa_in_init()
{
    if (!s_pa_in_initialized)
    {
        Pa_Initialize();
        s_pa_in_initialized = true;
    }
}

static int pick_input_device(const char *device_name)
{
    QString needle = QString::fromUtf8(device_name ? device_name : "");
    if (needle.isEmpty() || needle == "default" || needle.startsWith("default:") || needle.startsWith("pulse: "))
        return Pa_GetDefaultInputDevice();

    int n = Pa_GetDeviceCount();
    for (int i = 0; i < n; ++i)
    {
        const PaDeviceInfo *info = Pa_GetDeviceInfo(i);
        if (info && info->maxInputChannels > 0 && needle == QString::fromUtf8(info->name))
            return i;
    }
    return Pa_GetDefaultInputDevice();
}

void MsCore::rad_close_sound()
{
    if (s_pa_rx)
    {
        Pa_StopStream(s_pa_rx);
        Pa_CloseStream(s_pa_rx);
        s_pa_rx = NULL;
    }
    strncpy(rad_sound_state.err_msg, CLOSED_TEXT, SC_SIZE_L);
    rad_sound_state.bad_device = 1;
}

void MsCore::rad_open_sound()
{
    rad_close_sound();
    ensure_pa_in_init();

    rad_sound_state.read_error      = 0;
    rad_sound_state.write_error     = 0;
    rad_sound_state.underrun_error  = 0;
    rad_sound_state.interupts       = 0;
    rad_sound_state.rate_min        = (unsigned)-99;
    rad_sound_state.rate_max        = (unsigned)-99;
    rad_sound_state.chan_min        = (unsigned)-99;
    rad_sound_state.chan_max        = (unsigned)-99;
    rad_sound_state.msg1[0]         = 0;
    rad_sound_state.err_msg[0]      = 0;
    rad_sound_state.bad_device      = 1;
    is_little_endian                = 1;

    s_pa_rx_device = pick_input_device(rad_sound_state.dev_capt_name);
    if (s_pa_rx_device == paNoDevice)
    {
        snprintf(rad_sound_state.err_msg, SC_SIZE_L, "No CoreAudio input device available");
        return;
    }

    PaSampleFormat fmt = paInt16;
    sample_bytes = 2;
    if (in_bitpersample == 24) { fmt = paInt24; sample_bytes = 3; }
    else if (in_bitpersample == 32) { fmt = paInt32; sample_bytes = 4; }

    chan_capt = 2;
    channel_I = rad_sound_state.channel_I;
    channel_Q = rad_sound_state.channel_Q;
    int chan_needed = channel_I;
    if (chan_needed < channel_Q) chan_needed = channel_Q;
    chan_needed++;
    if (chan_needed > chan_capt) chan_capt = chan_needed;

    const PaDeviceInfo *info = Pa_GetDeviceInfo(s_pa_rx_device);

    // Clamp channel count to what the device actually supports. Virtual
    // SDR audio devices (FlexRadio DAX, AetherSDR CommonRadioAudio Out N,
    // SignaLink-style mono interfaces) expose 1 input channel only — a
    // hardcoded 2-channel open silently fails and we get a blank waterfall.
    if (info && info->maxInputChannels > 0 && info->maxInputChannels < chan_capt)
    {
        chan_capt = info->maxInputChannels;
        if (chan_capt == 1) { channel_I = 0; channel_Q = 0; }
    }

    PaStreamParameters params;
    params.device                    = s_pa_rx_device;
    params.channelCount              = chan_capt;
    params.sampleFormat              = fmt;
    // Ask CoreAudio for ~1 s of buffering. Qt's QTimer fires the 5 ms tick
    // that calls alsa_read_sound(), but on Mac it can drift much further
    // under GUI load — a fresh decode + waterfall redraw can stall the main
    // thread for 200–500 ms easily. 1 s gives us comfortable headroom; the
    // FT8 decoder doesn't care about audio latency, only about not losing
    // samples (which would break 15-second period alignment).
    params.suggestedLatency          = 1.0;
    params.hostApiSpecificStreamInfo = NULL;

    if (info)
    {
        rad_sound_state.rate_min = (unsigned)in_sample_rate;
        rad_sound_state.rate_max = (unsigned)in_sample_rate;
        rad_sound_state.chan_min = 1;
        rad_sound_state.chan_max = info->maxInputChannels;
        snprintf(rad_sound_state.msg1, SC_SIZE_L, "CoreAudio: %s (%d ch)", info->name, chan_capt);
    }

    // Try the user's requested rate first, then the device's native rate
    // if that fails (e.g. CommonRadioAudio is locked at 48k, DAX at 24k).
    double rates[3] = { (double)in_sample_rate,
                        info ? info->defaultSampleRate : 0.0,
                        0.0 };
    // Try the requested format first, then float32 / int16 / int32 fallbacks
    // (some virtual drivers only expose Float32 internally).
    PaSampleFormat formats[4] = { fmt, paFloat32, paInt16, paInt32 };

    bool opened_ok = false;
    PaError last_err = paUnanticipatedHostError;
    for (int ri = 0; ri < 3 && !opened_ok; ++ri)
    {
        if (rates[ri] <= 0.0) continue;
        for (int fi = 0; fi < 4 && !opened_ok; ++fi)
        {
            params.sampleFormat = formats[fi];
            if (Pa_IsFormatSupported(&params, NULL, rates[ri]) != paFormatIsSupported)
                continue;
            PaError err = Pa_OpenStream(&s_pa_rx, &params, NULL, rates[ri],
                                        paFramesPerBufferUnspecified, paNoFlag, NULL, NULL);
            last_err = err;
            if (err == paNoError && s_pa_rx != NULL)
            {
                fmt = formats[fi];
                s_pa_in_is_float = (fmt == paFloat32);
                if (fmt == paInt16)   sample_bytes = 2;
                if (fmt == paInt24)   sample_bytes = 3;
                if (fmt == paInt32 || fmt == paFloat32) sample_bytes = 4;
                s_pa_in_chan_capt = chan_capt;
                fprintf(stderr, "[MSHV mac audio in] %s: %.0f Hz %d ch fmt=0x%lx float=%d\n",
                        info ? info->name : "?", rates[ri], chan_capt, (unsigned long)fmt, s_pa_in_is_float);
                opened_ok = true;
            }
        }
    }
    if (!opened_ok)
    {
        snprintf(rad_sound_state.err_msg, SC_SIZE_L,
                 "Pa_OpenStream failed: %s (dev=%s req=%dch %dHz)",
                 Pa_GetErrorText(last_err), info ? info->name : "?",
                 chan_capt, in_sample_rate);
        fprintf(stderr, "[MSHV mac audio in] OPEN FAILED: %s\n", rad_sound_state.err_msg);
        s_pa_rx = NULL;
        return;
    }
    PaError err = paNoError;
    err = Pa_StartStream(s_pa_rx);
    if (err != paNoError)
    {
        snprintf(rad_sound_state.err_msg, SC_SIZE_L, "Pa_StartStream failed: %s", Pa_GetErrorText(err));
        fprintf(stderr, "[MSHV mac audio in] START FAILED: %s\n", rad_sound_state.err_msg);
        Pa_CloseStream(s_pa_rx);
        s_pa_rx = NULL;
        return;
    }
    rad_sound_state.bad_device = 0;
}

// Called from MsCore::Refresh_t() the same way alsa_read_sound() is on Linux.
// Reads any available frames from the PortAudio input stream, demuxes I/Q
// channels, and feeds the result into ResampleAndFilter — matching the Linux
// dispatch in linsound_in.cpp.
int MsCore::alsa_read_sound()
{
    if (!s_pa_rx) return 0;
    long avail = Pa_GetStreamReadAvailable(s_pa_rx);
    if (avail <= 0) return 0;
    long max_frames = SAMP_BUFFER_SIZE / chan_capt;
    if (avail > max_frames) avail = max_frames;

    int nSamples = 0;
    PaError err  = paNoError;

    if (sample_bytes == 2)
    {
        err = Pa_ReadStream(s_pa_rx, buffer2, (unsigned long)avail);
        if (err == paInputOverflowed)
        {
            static int s_overflow_log_n = 0;
            if (++s_overflow_log_n < 20 || (s_overflow_log_n % 200) == 0)
                fprintf(stderr, "[MSHV mac audio in] paInputOverflowed (n=%d)\n", s_overflow_log_n);
        }
        else if (err != paNoError)
        {
            rad_sound_state.read_error++;
            return 0;
        }
        for (long i = 0; i < avail; ++i)
        {
            short si = buffer2[i*chan_capt + channel_I];
            short sq = buffer2[i*chan_capt + channel_Q];
            if (si >=  CLIP16 || si <= -CLIP16) quisk_overrange++;
            if (sq >=  CLIP16 || sq <= -CLIP16) quisk_overrange++;
            cSamples_l[nSamples] = ((int)si) << 16;
            cSamples_r[nSamples] = ((int)sq) << 16;
            nSamples++;
        }
    }
    else if (sample_bytes == 3)
    {
        err = Pa_ReadStream(s_pa_rx, buffer3, (unsigned long)avail);
        if (err == paInputOverflowed)
        {
            static int s_overflow_log_n = 0;
            if (++s_overflow_log_n < 20 || (s_overflow_log_n % 200) == 0)
                fprintf(stderr, "[MSHV mac audio in] paInputOverflowed (n=%d)\n", s_overflow_log_n);
        }
        else if (err != paNoError)
        {
            rad_sound_state.read_error++;
            return 0;
        }
        for (long i = 0; i < avail; ++i)
        {
            int ii = 0, qq = 0;
            unsigned char *base = buffer3 + i*chan_capt*3;
            // Little-endian 24-bit packed -> int32 (sign-extended in upper 8 bits).
            memcpy((unsigned char*)&ii + 1, base + channel_I*3, 3);
            memcpy((unsigned char*)&qq + 1, base + channel_Q*3, 3);
            if (ii >=  CLIP32 || ii <= -CLIP32) quisk_overrange++;
            if (qq >=  CLIP32 || qq <= -CLIP32) quisk_overrange++;
            cSamples_l[nSamples] = ii;
            cSamples_r[nSamples] = qq;
            nSamples++;
        }
    }
    else /* sample_bytes == 4 (paInt32 or paFloat32) */
    {
        err = Pa_ReadStream(s_pa_rx, buffer4, (unsigned long)avail);
        if (err == paInputOverflowed)
        {
            static int s_overflow_log_n = 0;
            if (++s_overflow_log_n < 20 || (s_overflow_log_n % 200) == 0)
                fprintf(stderr, "[MSHV mac audio in] paInputOverflowed (n=%d)\n", s_overflow_log_n);
        }
        else if (err != paNoError)
        {
            rad_sound_state.read_error++;
            return 0;
        }
        // If we opened with paFloat32, the buffer holds normalised floats
        // in [-1, 1]. Convert in-place to int32 with the same fixed-point
        // scaling everything downstream expects.
        if (s_pa_in_is_float)
        {
            float *f = (float*)buffer4;
            long n = avail * (long)s_pa_in_chan_capt;
            for (long i = 0; i < n; ++i)
            {
                float v = f[i];
                if (v >  1.0f) v =  1.0f;
                if (v < -1.0f) v = -1.0f;
                buffer4[i] = (int)(v * 2147483647.0f);
            }
        }
        for (long i = 0; i < avail; ++i)
        {
            int ii = buffer4[i*chan_capt + channel_I];
            int qq = buffer4[i*chan_capt + channel_Q];
            if (ii >=  CLIP32 || ii <= -CLIP32) quisk_overrange++;
            if (qq >=  CLIP32 || qq <= -CLIP32) quisk_overrange++;
            cSamples_l[nSamples] = ii;
            cSamples_r[nSamples] = qq;
            nSamples++;
        }
    }

    // Match the 16-bit/32-bit paths: divide by 256 so dat_t lands at
    // ~int24 scale regardless of bit depth. Upstream MSHV's 24-bit branch
    // skips this divide (comment claims "int to int24" but the math leaves
    // values 256x larger), and the FFT stage in mscore.cpp multiplies by
    // 0.0000390625 = 1/25600 with no bit-depth awareness — so 24-bit
    // input arrives 256x hotter than 16-bit, pegging the waterfall and
    // saturating the decoder. Normalising here brings 24-bit to parity
    // with 16-bit downstream without touching upstream code.
    int *dat_t = s_scratch_dat_t;   // reused each tick — see decl (F7)
    for (int j = 0; j < nSamples; ++j)
        dat_t[j] = cSamples_l[j] / 256;
    ResampleAndFilter(dat_t, nSamples);

    return nSamples;
}

#endif // _MACOS_
