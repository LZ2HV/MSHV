/* MPEG/WAVE Sound library

   (C) 1997 by Jung woo-jae */

// Wavetoraw.cc
// Server which strips wave header.
#include <malloc.h>

#include "mpegsound.h"
#include <QMessageBox>
//#define MAX_READ_SIZE 11025*30*2 // only 30s samplerate*Sec*2
//#include <QtGui>
Wavetoraw::Wavetoraw(Soundinputstream *loader,Soundplayer *player/*,QWidget * parent*/)
//  : QWidget(parent)
{
    __errorcode=SOUND_ERROR_OK;
    initialized=false;
    buffer=NULL;
    this->loader=loader;
    this->player=player;
    //2.12
    s_max_period = 0;
    s_srate = 0;
    forcetomonoflag = false;
    buffersize = 0;
    samplesize = 0;
    speed = 0;
    stereo = 0;
    currentpoint = 0;
    size = 0;
    pcmsize = 1;
    //formatt = 0;
    //2.12
}
Wavetoraw::~Wavetoraw()
{
    if (buffer)free(buffer);
}
// Convert wave format to raw format class
bool Wavetoraw::initialize(int srate)
{
    s_srate=srate;
    if (!buffer)
    {
        buffersize=player->getblocksize();
        if ((buffer=(char *)malloc(buffersize))==NULL)
        {
            seterrorcode(SOUND_ERROR_MEMORYNOTENOUGH);
            return false;
        }
    }
    return true;
}
bool Wavetoraw::run(void)
{
    int c;
    if (initialized)
    {
        c=loader->getblock(buffer,buffersize);
        if (c==0)
        {
            seterrorcode(SOUND_ERROR_FILEREADFAIL);
            return false;
        }
        currentpoint+=c;
        if (player->putblock(buffer,buffersize)==false)return false;
        if (currentpoint>=size)
        {
            seterrorcode(SOUND_ERROR_FINISH);
            return false;
        }
    }
    else
    {
        c=loader->getblock(buffer,sizeof(WAVEHEADER));
        if (c==0)
        {
            seterrorcode(SOUND_ERROR_FILEREADFAIL);
            return false;
        }

        if (!testwave(buffer))return false;
        if (player->setsoundtype(stereo,samplesize,speed)==false)return false;
        //buffersize=player->getblocksize();//2.70 read all variants
        currentpoint=0;
        initialized=true;
    }
    return true;
}
/*void Wavetoraw::setcurrentpoint(int p)
{
    if (p*pcmsize>size)currentpoint=size;
    else currentpoint=p*pcmsize;
    loader->setposition(currentpoint+sizeof(WAVEHEADER));
}*/
bool Wavetoraw::testwave(char *buffer)
{
    WAVEHEADER *tmp=(WAVEHEADER *)buffer;  
    if (tmp->main_chunk==RIFF && tmp->chunk_type==WAVE && tmp->sub_chunk==FMT && tmp->data_chunk==DATA)
    {    	
        if ((tmp->format==PCM_CODE || tmp->format==IEEE_FLOAT) && tmp->modus<=2)
        {
            stereo=(tmp->modus==WAVE_STEREO) ? 1 : 0;           
            samplesize=(int)(tmp->bit_p_spl);       
            speed=(int)(tmp->sample_fq);
            pcmsize=1;
            if (stereo==1)pcmsize*=2;
            if (samplesize==16)pcmsize*=2;
            if (samplesize==24)pcmsize*=3;
            if (samplesize==32)pcmsize*=4;
            if (tmp->format==IEEE_FLOAT) samplesize=33;//33bit fictive secret to recognize float32

            //qDebug()<<pcmsize<<speed<<samplesize;//<<tmp->data_length;
            int max_size = speed*MAX_OPEN_WAW_TIME*pcmsize;//2.70 *pcmsize for q65=2min max old=speed*60*2;

            if ((int)tmp->data_length<max_size) size =(int)(tmp->data_length);
            else size = max_size;

            if (s_srate!=0)// 0 no emit this message hv
            {
                if (s_srate!=speed)
                {
                    QString text = "File sample rate is "+QString("%1").arg(speed)+
                                   " Hz.\nFor this reason this file may not be decoded"+
                                   " correctly.\nPlease, follow these steps:\n    1. Change mode.\n    2. "+
                                   "Open the file again.\n12000 Hz sample rate for mode MSK, FT8, JT65 and PI4 "+
                                   "files.\n11025 Hz sample rate for mode JTMS, FSK441, FSK315, ISCAT and JT6M files.";
                    QMessageBox::warning(this, "  Warning  ", text, QMessageBox::Close);
                }
            }

            return true;
        }
    }
    seterrorcode(SOUND_ERROR_BAD);
    return false;
}

