/* MSHV part of win sound out
 * Copyright 2015 Hrisimir Hristov, LZ2HV
 * May be used under the terms of the GNU General Public License (GPL)
 */
#include "mpegsound.h"
#if defined _WIN32_
#include <unistd.h>

//#include <QtGui>

QString ConvertToStr_tout(LPCTSTR name)
{
    QString s;
    for (unsigned int i=0; i<wcslen(name); ++i) s.push_back(name[i]);
    //s.append(" Микрофон 128 で簡単に作れますが"); //for test coding utf-8
    return s.toUtf8();
}
static bool _outfirst_ = false;
static LPGUID _outguid_;
static QString _outname_;//LPCTSTR=lpszDesc LPCTSTR=lpszDrvName=*.dll LPVOID=pContext
BOOL CALLBACK DSEnumProc_t_out(LPGUID lpGUID,LPCTSTR lpszDesc,LPCTSTR,LPVOID)
{
    if (!_outfirst_)
    {
        if (_outname_ == ConvertToStr_tout(lpszDesc))
        {
            _outfirst_ = true;
            _outguid_ = lpGUID; //qDebug()<<"Find Out="<<_outname_<<_outguid_;
        }
    }
    return true;
}
static bool ds_play_flag = false;
void Rawplayer::win_abort()
{
    if (ds_play_flag)
    {
        if (lpDirectSoundBuffer) IDirectSoundBuffer_Stop(lpDirectSoundBuffer);
        ds_play_flag = false;
    }
}
void Rawplayer::win_destroy()
{
    win_abort();
    if (lpDirectSoundBuffer)
    { 
        IDirectSoundBuffer_Release(lpDirectSoundBuffer); 
        //lpDirectSoundBuffer = NULL;   	
   	}
    if (lpDirectSound)
    {
        IDirectSound_Release(lpDirectSound);
        //lpDirectSound = NULL;  	
   	}
}
bool Rawplayer::win_initialize(char *device_name)
{
    //qDebug()<<"win_initialize";
	//QTime ttt; ttt.start();
    lpDirectSoundBuffer = NULL;//2.37 w10
    lpDirectSound = NULL;

    HWND hwnd1 = NULL;
    hwnd1 = GetForegroundWindow();
    if (hwnd1 == NULL)//2.39 100% correct initialize error
    {
        //qDebug()<<"Error GetForegroundWindow"<<hwnd1;
        hwnd1 = GetDesktopWindow();
    }
    if (hwnd1 == NULL)
    {
        //qDebug()<<"Final error="<<hwnd1;
        return false;
    }

    DWORD pv;  // Can be any 32-bit type.
    _outfirst_ = false;
    _outguid_ = (LPGUID)0;
    _outname_ = (QString)device_name;   
    if (DS_OK != DirectSoundEnumerate((LPDSENUMCALLBACK)DSEnumProc_t_out, (VOID*)&pv))
    {
    	//qDebug()<<"Failed to Enumerate";
        return false;
    }

    HRESULT hr;
    hr =DirectSoundCreate(_outguid_, &lpDirectSound, NULL);	// create dsound object
    if (hr != DS_OK)
    {
        //qDebug()<<"Failed to create dsound interface";
        return false;
    }
    hr =IDirectSound_SetCooperativeLevel(lpDirectSound, hwnd1, DSSCL_PRIORITY);	// Set coop level
    if (hr != DS_OK)
    {
        //qDebug()<<"Failed to create playback buffer";
        return false;
    }
    //qDebug()<<"create playback buffer"<<hwnd1<<DevHandles_t_out[dev.toInt()]<<hwnd1;
    //IDirectSound_Compact(lpDirectSound);
	//qDebug()<<ttt.elapsed();

    //return false;
    return true;
}
bool Rawplayer::win_resetsoundtype(int channels,int samplesize,int speed, int buffer_in)
{
    lpDirectSoundBuffer = NULL;

    //int buffer = (176400*buffer_in)/1000.0;
    //ds bufer VAZNO HV ot ms v golemina
    int buffer = (speed * ((double)( channels * samplesize ) / 8.0))*((double)buffer_in/1000.0);
    //qDebug()<<channels<<samplesize<<speed<<buffer;

    HRESULT hr;
    PCMWAVEFORMAT	pcmwf;

    int SAMPLERATE = speed;
    int AUDIOCHANNELS = channels;
    int BITSPERSAMPLE = samplesize;
    int SAMPLETIME = 4;//2.49 =4sec old=2sec

    memset(&pcmwf, 0, sizeof(PCMWAVEFORMAT));									// Set up wave format structure.
    pcmwf.wf.wFormatTag			= WAVE_FORMAT_PCM;
    pcmwf.wf.nChannels			= AUDIOCHANNELS;
    pcmwf.wf.nSamplesPerSec		= SAMPLERATE;
    pcmwf.wf.nBlockAlign		= (AUDIOCHANNELS * BITSPERSAMPLE) / 8; //BITSPERSAMPLE 16
    pcmwf.wf.nAvgBytesPerSec	= SAMPLERATE * (BITSPERSAMPLE /8) * AUDIOCHANNELS  ;	//pcmwf.wf.nSamplesPerSec * pcmwf.wf.nBlockAlign;
    pcmwf.wBitsPerSample		= BITSPERSAMPLE;

    memset(&playbackBuff, 0, sizeof(DSBUFFERDESC));								// Set up DSBUFFERDESC structure. Zero it out.
    playbackBuff.dwSize			= sizeof(DSBUFFERDESC);
    playbackBuff.dwFlags		= DSBCAPS_CTRLVOLUME | DSBCAPS_GLOBALFOCUS  ; //DSBCAPS_CTRLDEFAULT;	// Need default controls (pan, volume, frequency).
    //  taka spira kogato ne e na fokus  DSBCAPS_CTRLFREQUENCY| DSBCAPS_CTRLPAN | DSBCAPS_CTRLVOLUME
    // | DSBCAPS_GETCURRENTPOSITION2

    playbackBuff.dwBufferBytes	= SAMPLETIME * pcmwf.wf.nAvgBytesPerSec;		// SAMPLETIME-second buffer.
    playbackBuff.lpwfxFormat= (LPWAVEFORMATEX)&pcmwf;
    //IDirectSoundBuffer_Release(lpDirectSoundBuffer);
    ds_read_wr_offset = buffer; // 176400=1000ms  88200=500ms 150000=800ms
    write_position = 0;
    buffer_size = (int)playbackBuff.dwBufferBytes;
    //qDebug()<<"DS"<<ds_read_wr_offset<<buffer_size<<SAMPLERATE;
    hr = lpDirectSound->CreateSoundBuffer( &playbackBuff, &lpDirectSoundBuffer, NULL);

    if (hr != DS_OK)
    {
        //DBPRINTF(("NETPLAY: Failed to create playback buffer.\n"));
        lpDirectSoundBuffer = NULL;												// Failed.
        return false;
    }
    return true;
}
bool Rawplayer::win_putblock(void *buffer,int size)
{
    LPVOID lpvPtr1,lpvPtr2=NULL;
    DWORD dwBytes1,dwBytes2=0;
    HRESULT hr;

    hr = IDirectSoundBuffer_Lock(lpDirectSoundBuffer,
                                 write_position,			//offset
                                 size,		//size
                                 &lpvPtr1,&dwBytes1,
                                 &lpvPtr2,&dwBytes2,
                                 0 );//DSBLOCK_FROMWRITECURSOR

    memcpy(lpvPtr1,(unsigned char*)buffer,dwBytes1);
    if (NULL != lpvPtr2 ) memcpy(lpvPtr2,(unsigned char*)buffer+dwBytes1,dwBytes2);
    write_position += (int)(dwBytes1+dwBytes2);
    if (write_position >= buffer_size)
    {
        // prev_write_position = prev_write_position - buffer_size;
        write_position=(int)dwBytes2;
    }

    hr = IDirectSoundBuffer_Unlock(lpDirectSoundBuffer, lpvPtr1, dwBytes1, lpvPtr2,dwBytes2);

    if (SUCCEEDED(hr))
    { // s towa mu dava play samo kogato ima pritok na (buffer) i to samo vednaz
        DWORD status;
        IDirectSoundBuffer_GetStatus(lpDirectSoundBuffer, &status);
        if (!(status & DSBSTATUS_PLAYING))
        {
            //qDebug()<<"play";
            IDirectSoundBuffer_Play(lpDirectSoundBuffer, 0, 0, DSBPLAY_LOOPING);
            ds_play_flag = true;
        }
    }

    int play_position = 0;

    while (1)
    {
        IDirectSoundBuffer_GetCurrentPosition(lpDirectSoundBuffer, (DWORD*)&play_position, NULL);

        if ( write_position - play_position < 0 )
        {
            if (write_position + buffer_size - play_position < ds_read_wr_offset  )
                return true;
        }
        else
        {
            if (write_position - play_position < ds_read_wr_offset )
                return true;
        }
        usleep(1000);//v1.27 32-64bit qt4 gcc492 =20  //qt5 gcc530 = 1000
    }
    return true;
}
#endif