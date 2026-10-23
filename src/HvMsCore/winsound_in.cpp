/* MSHV MsCore part of win sound in
 * Copyright 2015 Hrisimir Hristov, LZ2HV
 * May be used under the terms of the GNU General Public License (GPL)
 */
#include "mscore.h"
#if defined _WIN32_
//#include <QtGui>

QString ConvertToStr_tin(LPCTSTR name)
{
    QString s; 
    for (unsigned int i=0; i<wcslen(name); ++i) s.push_back(name[i]);
    //s.append(" Микрофон 128 で簡単に作れますが"); //for test coding utf-8
    return s.toUtf8();
}
static bool _infirst_ = false;
static LPGUID _inguid_;
static QString _inname_;//LPCTSTR=lpszDesc LPCTSTR=lpszDrvName=*.dll LPVOID=pContext
BOOL CALLBACK DSEnumProc_t_in(LPGUID lpGUID,LPCTSTR lpszDesc,LPCTSTR,LPVOID)
{
    if (!_infirst_)
    {
        if (_inname_ == ConvertToStr_tin(lpszDesc))
        {
            _infirst_ = true;
            _inguid_ = lpGUID; //qDebug()<<"Find In="<<_inname_<<_inguid_;
        }
    }
    return true;
}
bool MsCore::ReadData()
{
    // check to see if this is the first time data was requested from buffer
    if ( ( CapturePosition == (DWORD)-1 ) && ( ReadPosition == (DWORD)-1 ) )
    {
        lpdCaptureBuff->GetCurrentPosition( &CapturePosition, &ReadPosition );
        //PreviousReadPosition = ReadPosition;
        //qDebug()<<"FIRST RESET lpdCaptureBuff->GetCurrentPosition"<<CapturePosition<<ReadPosition;
        return false;
    }

    // Get former read and capture positions and acquire new ones
    //DWORD   PreviousCapturePosition;
    //PreviousCapturePosition = CapturePosition;
    DWORD   PreviousReadPosition = ReadPosition;
    //DWORD   temp_PreviousReadPosition = ReadPosition;
    //PreviousReadPosition = ReadPosition;

    //if( FAILED( hr = g_pDSBCapture->GetCurrentPosition( &dwCapturePos, &dwReadPos ) ) )
    // return hr;
    if ( FAILED(lpdCaptureBuff->GetCurrentPosition( &CapturePosition, &ReadPosition ) ) )
    {
        //qDebug()<<"********* FAILED lpdCaptureBuff->GetCurrentPosition";
        //ReadPosition = temp_PreviousReadPosition;
        return false;
    }
    //lpdCaptureBuff->GetCurrentPosition( &CapturePosition, &ReadPosition );

    LPVOID  CBAddress1, CBAddress2;
    DWORD   CBLength1, CBLength2;
    DWORD   LockSize;

    if ( ReadPosition < PreviousReadPosition ) LockSize = ( dscbd.dwBufferBytes - PreviousReadPosition ) + ReadPosition;        
    else LockSize = ReadPosition - PreviousReadPosition;        

    if ( LockSize != 0 )
    {
        if ( FAILED( lpdCaptureBuff->Lock(
                         PreviousReadPosition,           // Offset at which to start lock.
                         LockSize,                       // Size of lock.
                         &CBAddress1,                    // Gets address of first part of lock.
                         &CBLength1,                     // Gets size of first part of lock.
                         &CBAddress2,                    // Address of wraparound.
                         &CBLength2,                     // Size of wraparound.
                         0 ) ) )                         // Flag.
            return false;

        memcpy((unsigned char*)all_data,CBAddress1,CBLength1);//2.70 (unsigned char*) old=(char*) 
        if (CBAddress2) memcpy(((unsigned char*)all_data + CBLength1),CBAddress2,CBLength2);//2.70 if new data wraps around the capture buffer
            
        //if( CBAddress2 ) qDebug()<<"1Lock, CBLength1, CBLength2"<<LockSize<<CBLength1<<CBLength2<<CBLength1+CBLength2;
        //if ( CBAddress2 != NULL ) qDebug()<<"2Lock, CBLength1, CBLength2"<<LockSize<<CBLength1<<CBLength2<<CBLength1+CBLength2;

        if (LockSize <= 0) return false;

        int count = 0; //qDebug()<<in_bitpersample<<LockSize<<; //6000,9000,12000
		int bits = 2;
 		if (in_bitpersample == 24) bits = 3;
 		if (in_bitpersample == 32) bits = 4;
		int cha_ = rad_sound_state.channel_I*bits;
		//unsigned char b0=0;
		//unsigned char b1=0;
		//unsigned char b2=0;
        for (int i = 0; i < (int)LockSize; i+=2*bits)//2.70 2*2=2chan*2bits
        {
        	int z  = 0;        	
        	if 		(bits == 4) z = all_data[i+cha_] + (all_data[i+1+cha_] <<  8) + (all_data[i+2+cha_] << 16) + (all_data[i+3+cha_] << 24);
        	else if (bits == 3) z = (all_data[i+cha_] <<  8) + (all_data[i+1+cha_] << 16) + (all_data[i+2+cha_] << 24);
        	else 		   		z = (all_data[i+cha_] << 16) + (all_data[i+1+cha_] << 24);
       		z = (z >> 8);//to 24-bits	 			        	
            cSamples_mono[count] = z;
            count++;           
    		//b0 = all_data[0];
    		//b1 = all_data[1];
    		//if (bits == 3) b2 = all_data[2];
        }//qDebug()<<b0<<b1<<b2;

		ResampleAndFilter(cSamples_mono,count); //2.57

        if ( FAILED( lpdCaptureBuff->Unlock(
                         CBAddress1,                     // Gets address of first part of lock.
                         CBLength1,                      // Gets size of first part of lock.
                         CBAddress2,                     // Address of wraparound not needed.
                         CBLength2 ) ) )                 // Size of wraparound not needed.
            return false;
    }
    return true;
}
bool MsCore::stop_sound()
{
    if (FAILED(lpdCaptureBuff->Stop())) return false;
    CapturePosition = (DWORD)-1; // reset capture position
    ReadPosition = (DWORD)-1;    // reset read position
    //PreviousReadPosition = (DWORD)-1;
    return true;
}
bool MsCore::start_sound()
{
    if (FAILED(lpdCaptureBuff->Start(DSCBSTART_LOOPING))) return false;
    return true;
}
bool MsCore::select_device(bool immediately)
{
    static bool prev_res = false;
	static LPGUID prev_inguid = (LPGUID)-1;//2.50 protect at start no primary sound driver =(-1)
    DWORD pv;  // Can be any 32-bit type.
    _infirst_ = false;
    _inguid_ = (LPGUID)0;
    _inname_ = (QString)rad_sound_state.dev_capt_name;
    if ( DS_OK != DirectSoundCaptureEnumerate((LPDSENUMCALLBACK)DSEnumProc_t_in, (VOID*)&pv))
    {   	
    	prev_res = false; //qDebug()<<"EROR DirectSoundCaptureEnumerate";
        return false;
    }
   
    if (!immediately)  
    { 
    	if (prev_inguid != _inguid_) //2.49 exception GUID is Changed
    	{
    		close_sound(); //qDebug()<<"GUID Changed Close---->";
    		usleep(2000); 
   		}
   		else 
   		{
   			//qDebug()<<"GUID No Changed Return="<<prev_res;
    		return prev_res;  			
  		}    		
   	} //else qDebug()<<"RESET immediately";

    CapturePosition = (DWORD)-1; // reset capture position
    ReadPosition = (DWORD)-1;    // reset read position

    // Set up wave format structure.
    ZeroMemory( &wf, sizeof( wf ) );
    wf.wFormatTag = WAVE_FORMAT_PCM;
    wf.nChannels = CHANNELS_W;
    wf.wBitsPerSample = in_bitpersample;//2.70
    wf.nSamplesPerSec = in_sample_rate;//rad_sound_state.sample_rate;
    wf.nBlockAlign = ( wf.nChannels * wf.wBitsPerSample ) / 8;     // ( nChannels * wBitsPerSample ) / 8 bits per byte
    wf.nAvgBytesPerSec = ( wf.nSamplesPerSec * wf.nBlockAlign );
    //qDebug() << "wf.nAvgBytesPerSec"<<wf.nAvgBytesPerSec;

    // Set up DSBUFFERDESC structure: Buffer Control Options
    ZeroMemory(&dscbd, sizeof( dscbd ) );
    dscbd.dwSize = sizeof( DSCBUFFERDESC );
    dscbd.dwFlags = 0;
    dscbd.dwBufferBytes = wf.nAvgBytesPerSec * MAXDSBUFERTIME_W;  //max->3840000 buffer for 192000 ->5 second buffer
    dscbd.dwReserved = 0;
    dscbd.lpwfxFormat = &wf;
    dscbd.dwFXCount = 0;
    dscbd.lpDSCFXDesc = NULL;
    //qDebug() << "dscbd.dwBufferBytes"<<dscbd.dwBufferBytes;

	lpdCapture = NULL;
    if (FAILED( DirectSoundCaptureCreate(_inguid_,&lpdCapture,NULL)))
    {        
        prev_res = false; //qDebug()<<"EROR DirectSoundCaptureCreate"; // insert error handling
        return false;
    }
    
    lpdCaptureBuff = NULL; //qDebug()<<"----"<<in_bitpersample;
    if (FAILED(lpdCapture->CreateCaptureBuffer(&dscbd,&lpdCaptureBuff,NULL)))
    {        
        prev_res = false; //qDebug()<<"EROR CreateCaptureBuffer"<<in_bitpersample; //here is problem if not any device ON
        return false;
    }    
    prev_inguid = _inguid_;//2.50 need to be here if setup is ok save->prev_inguid
    
    // Set up DSBCAPS structure: Buffer Capabilities
    ZeroMemory( &dscbcaps, sizeof( dscbcaps ) );
    dscbcaps.dwSize = sizeof( DSCBCAPS );

    // Find the maximum playback offset position
    lpdCaptureBuff->GetCaps( &dscbcaps );
    //qDebug()<<dscbcaps.dwBufferBytes;
    
    prev_res = true;
    return true;
}
#endif

