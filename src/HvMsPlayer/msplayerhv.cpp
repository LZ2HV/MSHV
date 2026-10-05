/* MSHV MsPlayerHV
 * Copyright 2015 Hrisimir Hristov, LZ2HV
 * May be used under the terms of the GNU General Public License (GPL)
 */

#include "msplayerhv.h"
#include <QTime>

static const double SAVE_SAMPLE_RATE_11025 = 11025.0;
static const double SAVE_SAMPLE_RATE_12000 = 12000.0;

//#include <QtGui>
MsPlayerHV::MsPlayerHV(QString path)
{
    otp_tx_key  = "0#000000#0#0#0#0";
    sfma_tx_all = "#0#0#0#0#0#0#0#0";
    s_msf = false;
    s_mode=2;   //2.12
    filenamehv=(char*)"none"; //2.12
    s_period_time_sec = 30;

    SAVE_SAMPLE_RATE = SAVE_SAMPLE_RATE_11025;//HV important set to default mode fsk441  sample rate

    App_Path = path;
    //s_MyGridLoc = "GRID";
    s_tx_freq = 1200.0; //ft8 for the moment 1.43
    f_message_or_file = true;
    music.RealStop = true;
    music.pause = false;
    music.quit = false;
    //music.setframeflag = false;
    musics.stop = false;
    musics.restart = false;
    musics.errorflag = false;
    musics.error_repat = false;
    //music.setframenumber = 0;
    musics.move = 1;//???? 1 netragva na 32 ako e 0
    musics.currentrun = 0;
    musics.errorcode = 0;
    s_bitpersample = 16;
    s_offset_dt=0;
    /*int po = sched_getscheduler(0);
    qDebug()<<po<<sched_get_priority_max(po);
    qDebug()<<po<<sched_get_priority_min(po);*/
    /*
    #if defined _LINUX_
        Rawplayer::setvolume_lin(70);//pcm channel
    #endif
    */
    thtx = 0;
    startuplock  = PTHREAD_MUTEX_INITIALIZER;  //ok win and lin -> linux no TX error
    //pthread_mutex_init(&startuplock, NULL); //or this

    pthread_attr_t thread_attr;
    struct sched_param param;

    pthread_attr_init(&thread_attr);
    pthread_attr_getschedparam(&thread_attr, &param);//2.38
    param.sched_priority = param.sched_priority + 1;//2.38
    //param.sched_priority = 15;//2.38 stop

    pthread_attr_setschedpolicy(&thread_attr, SCHED_OTHER);  //2.38 stop <-no needed   need Linux test OTHER=0 FIFO=1 RR=2 policy
    pthread_attr_setschedparam(&thread_attr, &param);

    //pthread_create(&th,NULL,MsPlayerHV::ThreadEntry,(void*)this);
    pthread_create(&thtx,&thread_attr,MsPlayerHV::ThreadEntry,(void*)this);
    pthread_mutex_unlock(&startuplock); // za da ne tragva s puskanet

    /*int apoly,poly;
    pthread_getschedparam(thtx,&apoly,&param); 
    pthread_attr_getschedpolicy(&thread_attr,&poly);
    qDebug()<<"th="<<thtx<<param.sched_priority;*/

    pthread_attr_destroy(&thread_attr);
}
MsPlayerHV::~MsPlayerHV()
{}
void MsPlayerHV::SetModeForWavSaves(int mode)
{
    /*if (mode==0 || mode==7 || mode==8 || mode==9 || mode==10 || mode==11 || mode==12 || mode==13 ||
    	 mode==14 || mode==15 || mode==16 || mode==17)//jt65abc msk144 pi4 ft8 ft4
    {
        SAVE_SAMPLE_RATE = SAVE_SAMPLE_RATE_12000;
    #if defined _LINUX_
        Rawplayer::pa_sa_rate = 48000;
    #endif
    }
    else //if (mode>0 && mode<7)
    {
        SAVE_SAMPLE_RATE = SAVE_SAMPLE_RATE_11025;
    #if defined _LINUX_
        Rawplayer::pa_sa_rate = 44100;
    #endif
    }*/
    if (mode>0 && mode<7)//2.65
    {
        SAVE_SAMPLE_RATE = SAVE_SAMPLE_RATE_11025;
#if defined _LINUX_
        Rawplayer::pa_sa_rate = 44100;
#endif
    }
    else
    {
        SAVE_SAMPLE_RATE = SAVE_SAMPLE_RATE_12000;
#if defined _LINUX_
        Rawplayer::pa_sa_rate = 48000;
#endif
    }
}
void MsPlayerHV::SetSoundDevice(QString dev_in_number,int bpsamp,int buffer)
{
    strncpy(dev_in_p,dev_in_number.toUtf8(),127);//2.50 for pulse audio max is 512 ???
    Rawplayer::defaultdevice = dev_in_p;
    Rawplayer::buffering = buffer;
    s_bitpersample = bpsamp;
#if defined _LINUX_
    Rawplayer::s_bitpersamplelin = bpsamp;
#endif
    //qDebug()<<"MsPlayerHV========"<<Rawplayer::defaultdevice<<dev_in_number<<bpsamp;
}
void MsPlayerHV::music_move(int value)
{
    pthread_mutex_lock(&musics.movelock); // zada mal4i pri smiana na pesen veroiatno stawa i za pevartane
    musics.move+=value; //qDebug()<<"gore.move"<<musics.move;
    pthread_mutex_unlock(&musics.movelock);
}
void MsPlayerHV::music_done(void) // da preska4a pesen ako e error
{
    music.RealStop=true;// za stop start ot edno i sa6to kop4e
    music_move(1);
}
void MsPlayerHV::music_term(void)
{
    music.pause=false;
    music.quit=true;
}
void MsPlayerHV::seterrorcode(int errcode)
{
    musics.errorcode=errcode;
    musics.errorflag=true;
    musics.error_repat=true;
    music_done();
}
void MsPlayerHV::SaveFile(int*raw,int count,QString name)
{
    QString str = App_Path+"/RxWavs/"+name+".WAV"; //qDebug()<<str;
    Rawtofile_wr *recorder;
    recorder = Rawtofile_wr::opendevice(qstrdup(qPrintable(str)));
    if (recorder == NULL) return;
    (recorder && ((Rawtofile_wr*)recorder)->setfiletype(WAV));
    if (!recorder->setsoundtype(0,16,SAVE_SAMPLE_RATE))//SAVE_SAMPLE_RATE 6000.0
    {
        seterrorcode(recorder->geterrorcode()); //qDebug()<<"1111";
        return;
    }
    recorder->set8bitmode();
    byte01_ *f = new byte01_[count*2+8192];
    int j = 0;
    for (int i = 0; i < count; i++)
    {
        int z = raw[i];
        z /= 256;//z = z >> 8;//2.70 24 bit to 16 bit
        f[j]=(byte01_)(z & 0xff);
        f[j+1]=(byte01_)((z >> 8) & 0xff);
        j+=2;
    }
    recorder->putblock(f,count*2);
    delete [] f;
    delete recorder;
}
void MsPlayerHV::OpenFile(char *filename)
{
    bool splay_forcetomonoflag;
    splay_forcetomonoflag = false;
    Soundinputstream *loader;
    int err;
    Rawtodata *player;
    Wavetoraw *server;
    player=new Rawtodata;

    if (player==NULL)
    {
        seterrorcode(SOUND_ERROR_MEMORYNOTENOUGH);
        return;
    }
    if (!player->initialize(filename))
    {
        //2.12 error garmi
        //delete player;//2.13 no delete crash
        //qDebug()<<"player";
        seterrorcode(player->geterrorcode());
        return;
    }
    if ((loader=Soundinputstream::hopen(filename,&err))==NULL)
    {
        //qDebug()<<"loader";
        seterrorcode(err);
        delete loader;
        return;
    }
    if ((server=new Wavetoraw(loader,player))==NULL)
    {
        //qDebug()<<"server";
        seterrorcode(SOUND_ERROR_MEMORYNOTENOUGH);
        return;
    }

    server->initialize((int)SAVE_SAMPLE_RATE);//check for diferent samplerates

    server->setforcetomono(splay_forcetomonoflag);
    //Setsongname(stripfilename(filename)); izliza na displea i zaradi tova go skasiavat hv
    //music.pause=music.quit=music.setframeflag=false; //2.47 no needed in Open File

    if (!server->run())     // Initialize Wave player
    {
        //qDebug()<<"server->run";
        seterrorcode(server->geterrorcode());
        delete server;
        return;
    }
    emit SentFileClarDisplay();
    connect(player,SIGNAL(SentData(int*,int,bool)),this,SIGNAL(SentData(int*,int,bool)));

///////////////////////////////tova ne triabva ve4e hv /////////////////////////////
    int q=server->getfrequency()*(server->isstereo()+1);
    if (server->getsamplesize()==16)
    {
        if (!splay_forcetomonoflag)q<<=1;
    }
    q>>=1;
    player->setquota(q);
///////////////////////////////tova ne triabva ve4e hv /////////////////////////////

    for (;;)
    {
        if (!server->run())
        {
            // music_done();
            break;
        }
    }//qDebug()<<"reset_wav";
    delete server;
    delete player;
    delete loader;
    return;
}
void MsPlayerHV::SetTxFreq(double f)
{
    s_tx_freq = f;
}
void MsPlayerHV::SetSFMATxAll(QString s)//2.76sf
{
    sfma_tx_all = s; //qDebug()<<"TX sfox_tx_all="<<sfox_tx_all;
    //QStringList l0 = s.split("#");
    //qDebug()<<"TXT="<<l0.at(0)<<"SendTXT="<<l0.at(1)<<"SendCQ="<<l0.at(2)<<"MyBaseCall="<<l0.at(3)<<"MADX="<<l0.at(4);
}
void MsPlayerHV::SetOtpTxKey(QString s)//2.76sf
{
    otp_tx_key = s; //qDebug()<<"TX otp_tx_key="<<otp_tx_key;
    //QStringList l0 = s.split("#");
    //qDebug()<<"MADX="<<l0.at(0)<<"SFox="<<l0.at(1)<<"Key="<<l0.at(2);
}
void MsPlayerHV::SetMsf(bool f)
{
    s_msf = f; //printf("TX s_msf=%d\n",s_msf);
}
void MsPlayerHV::SetOffsetDt(int dt)//2.76.5
{
	s_offset_dt=dt;
}
void MsPlayerHV::GetSsMsAndOffset(int &ss_s,int &ms_s)
{
	QTime ctt;//QTime ctt = QTime::currentTime();
    if (s_offset_dt==0) ctt = QTime::currentTime();
    else ctt = QTime::currentTime().addMSecs(s_offset_dt);
    ss_s = ctt.toString("ss").toInt();
    ms_s = ctt.toString("zzz").toInt();//"z"<--- error QT > QT5 v5.9.x
}
void MsPlayerHV::xplaymessage(char *message,Rawplayer *player)
{
    bool splay_forcetomonoflag;
    splay_forcetomonoflag = false;
    GenMessage *server;

    //qDebug()<<"New Server"<<(QString)message<<ttt.elapsed();
    //QTime ttt; ttt.start();
    // Server
    if ((server = new GenMessage(player))==NULL)//App_Path,
    {
        seterrorcode(SOUND_ERROR_MEMORYNOTENOUGH);
        return;
    }
    //qDebug()<<"1 initialize="<<(QString)message<<ttt.elapsed();
    server->initialize(message,s_mode,s_tx_freq,s_period_time_sec,s_bitpersample,s_msf,sfma_tx_all,otp_tx_key);
    //qDebug()<<"2 initialize="<<(QString)message<<ttt.elapsed();
    QString new_tx_msg;
    if ((s_mode==0 || s_mode==12) && message[0]!='@')//msk144 1.31 for " R " msg is same but not after unpack
        new_tx_msg=server->TGenMsk->GetUnpackMsg();
    else if ((s_mode==7 || s_mode==8 || s_mode==9) && message[0]!='@')//jt65abc
    {
        new_tx_msg=server->TGen65->GetUnpackMsg();
    }
    else if (s_mode==11 && message[0]!='@')//zasega ft8 s tova ->msk144 1.31 for " R " msg is same but not after unpack
    {
        if (s_msf) new_tx_msg=server->TGenSFox->GetUnpackMsg();
        else new_tx_msg=server->TGenFt8->GetUnpackMsg();
    }
    else if (s_mode==13 && message[0]!='@')//zasega ft4 s tova ->msk144 1.31 for " R " msg is same but not after unpack
        new_tx_msg=server->TGenFt4->GetUnpackMsg();
    else if (s_mode==18 && message[0]!='@')//zasega ft4 s tova ->msk144 1.31 for " R " msg is same but not after unpack
        new_tx_msg=server->TGenFt2->GetUnpackMsg();
    else if ((s_mode==14 || s_mode==15 || s_mode==16 || s_mode==17) && message[0]!='@')//q64
        new_tx_msg=server->TGenQ65->GetUnpackMsg();
    else
        new_tx_msg=(QString)message;
    //qDebug()<<"Unpack"<<(QString)message<<ttt.elapsed();
    //qDebug()<<"ss_prev_msg="<<ss_prev_msg<<"NewREAL="<<real_tx_msg<<"RealIn="<<(QString)message;
    if (!music.RealStop)//s_prev_tx_msg!=new_tx_msg &&
    {
        QString tmp = new_tx_msg; //qDebug()<<new_tx_msg<<new_tx_msg.count()<<message<<strlen(message);
        if (tmp.count()>1)//2.76 protection from Gen`S return empty string
        {
            if (tmp[0]=='@')
            {
                if (tmp=="@TUNE") tmp="TUNE";
                if (tmp[1]=='A')  tmp="882 Hz";
                if (tmp[1]=='B')  tmp="1323 Hz";
                if (tmp[1]=='C')  tmp="1764 Hz";
                if (tmp[1]=='D')  tmp="2205 Hz";
                if (tmp[0]=='@')
                {
                    tmp.remove("@");
                    if (tmp.toInt()>=100 && tmp.toInt()<=3500) tmp= tmp+" Hz";
                } //SendTxMsgLabTxkkk(tmp);
            }
        }
        //else
        //emit SendTxMsgLabTx(tmp);
        emit SendTxMsgAllTxt(tmp,s_tx_freq);

        if (s_prev_tx_msg!=new_tx_msg)
        {
            emit SendTxMsgLabTx(tmp);
            if (new_tx_msg!="@TUNE")
            {
                s_prev_disp_msg=tmp;      //1.31
                s_prev_tx_msg=new_tx_msg; //1.31 za da ne izpra6ta ako massg e sa6toto
            }
        }
    }

    server->setforcetomono(splay_forcetomonoflag);

    //music.pause=music.quit=music.setframeflag=false; //2.47 stop move UP in xplay()

    if (!server->run())     // Initialize Wave player
    {
        //PlayerStatus(0,0,0,(char*)"none");
        seterrorcode(server->geterrorcode());
        delete server;
        return;
    }

    ///////////////////////////////tova ne triabva ve4e hv /////////////////////////////
    int q=server->getfrequency()*(server->isstereo()+1);
    if (server->getsamplesize()==16)
    {
        if (!splay_forcetomonoflag)q<<=1;
    }
    q>>=1;
    player->setquota(q);
    ///////////////////////////////tova ne triabva ve4e hv /////////////////////////////

    int min_delayft = 200;
    int delayft = min_delayft;

    if (s_mode==7 || s_mode==8 || s_mode==9) usleep(700000);// tested 1.35 //jt65 wheit 1s in begining       
    else if (s_mode==11)//ft8
    {
        int ss_s=0; int ms_s=0;
        GetSsMsAndOffset(ss_s,ms_s);
        if (ss_s % 15 == 0)
        {
            int xlat = 450 - ms_s;//500 real s wsjt2
            if (xlat>50)// min 100-50ms delay
            {
                delayft = xlat;
                //qDebug()<<"Start="<<ss_s<<ms_s<<xlat<<delayft8<<"FullLat="<<(ms_s + delayft8);
            }
            //else
            //qDebug()<<"ERROR > 50===="<<ss_s<<ms_s<<delayft8<<"FullLat="<<(delayft8);
        }
        else
        {
            delayft = min_delayft;
            //qDebug()<<"ERROR % 15===="<<ss_s<<ms_s<<delayft8<<"FullLat="<<(delayft8);
        } 
        int lindelayft = (delayft * 1000);
        usleep(lindelayft);//350000   400000 ft8 2.03
    }
    else if (s_mode==13)//ft4
    {
        int ss_s=0; int ms_s=0;
        GetSsMsAndOffset(ss_s,ms_s);
        if (ss_s == 7 || ss_s == 22 || ss_s == 37 || ss_s == 52)//if (ss_s % 15 == 7)  time_ms = 500;
        {
            ss_s=0;
            ms_s -= 500;
        }
        if (ss_s % 15 == 0)
        {
            int xlat = 350 - ms_s;//350 real s wsjt2 rc7 ???? my be =400ms
            if (xlat>50)// min 100-50ms delay
            {
                delayft = xlat; 
            }//else //qDebug()<<"ERROR > 50===="<<ss_s<<ms_s<<delayft8<<"FullLat="<<(delayft8);
        }
        else
        {
            delayft = min_delayft; //qDebug()<<"ERROR % 15===="<<ss_s<<ms_s<<delayft8<<"FullLat="<<(delayft8);
        } 
        int lindelayft = (delayft * 1000);
        usleep(lindelayft);//350000   400000 ft8 2.03
    }
    else if (s_mode==18)//ft2
    {
        int ss_s=0; int ms_s=0;
        GetSsMsAndOffset(ss_s,ms_s);
        if 		(ss_s == 3 || ss_s == 18 || ss_s == 33 || ss_s == 48)//if (ss_s % 15 == 3)  time_ms = 750;
        {
            ss_s=0;
            ms_s -= 750;
        }
        else if	(ss_s == 7 || ss_s == 22 || ss_s == 37 || ss_s == 52)//if (ss_s % 15 == 7)  time_ms = 500;
        {
            ss_s=0;
            ms_s -= 500;
        }
        else if (ss_s ==11 || ss_s == 26 || ss_s == 41 || ss_s == 56)//if (ss_s % 15 == 11)  time_ms = 250;
        {
            ss_s=0;
            ms_s -= 250;
        }//qDebug()<<"Start="<<ctt.toString("ss.zzz")<<"ss ms="<<ss_s<<ms_s;
        if (ss_s % 15 == 0)
        {
            int xlat = 350 - ms_s;//350 real s wsjt2 rc7 ???? my be =400ms
            if (xlat>50)// min 100-50ms delay
            {
                delayft = xlat; 
                if (delayft>700) delayft = min_delayft;//<- from TX ro RX error
                //qDebug()<<"Start="<<ss_s<<"ms_s="<<ms_s<<"delayft="<<delayft<<"FullLat="<<(ms_s + delayft)<<"xlat="<<xlat<<music.RealStop;
            }//else //qDebug()<<"ERROR > 50===="<<ss_s<<ms_s<<delayft8<<"FullLat="<<(delayft8);
        }
        else
        {
            delayft = min_delayft; //qDebug()<<"ERROR % 15===="<<ss_s<<ms_s<<delayft8<<"FullLat="<<(delayft8);
        }
        int lindelayft = (delayft * 1000);
        usleep(lindelayft);//350000   400000 ft8 2.03
    }
    else if (s_mode==14 || s_mode==15 || s_mode==16 || s_mode==17)
    {
        QTime ctt = QTime::currentTime();
        int mm_s = ctt.toString("mm").toInt();
        int ss_s = ctt.toString("ss").toInt();
        int ms_s = ctt.toString("zzz").toInt();
        int mmss = (mm_s*60)+ss_s;
        if (mmss % s_period_time_sec == 0)
        {
            int xlat = 950 - ms_s;
            if (s_period_time_sec<60) xlat = 450 - ms_s;
            if (xlat>50)
            {
                delayft = xlat;
                //qDebug()<<"Start="<<ss_s<<ms_s<<xlat<<delayft8<<"FullLat="<<(ms_s + delayft8);
            }
            //else
            //qDebug()<<"ERROR > 50===="<<ss_s<<ms_s<<delayft8<<"FullLat="<<(delayft8);
        }
        else
        {
            delayft = min_delayft;
            //qDebug()<<"ERROR % 15===="<<ss_s<<ms_s<<delayft8<<"FullLat="<<(delayft8);
        }
        int lindelayft = (delayft * 1000);
        usleep(lindelayft);
    }
    else usleep(250000);//wsjt-x 200ms mshv -> 250000us tx delay offset 250ms

    //qDebug()<<"Reall_TX==AUDIO=="<<QTime::currentTime().toString("ss:zzz");
    for (;;)
    {
        if (music.pause)
        {
            player->abort();
            player->resetsoundtype();
            while (music.pause)
            {
                usleep(1000); //1.27 32-64bit qt4 gcc492 =100  //qt5 gcc530 = 1000
                //#if defined _WIN64_
                //usleep(1000);//v1.14 HV problem pod 1000 ne raboti GCC4.9.2 win64
                //#endif
            }
        }

        if (music.RealStop)  //  slagam go tuk za da ne proswirva pri stop pri malka pauza v na4aloto
        {
            server->mute_hv(true);//player->mute_hv(true);
            //f_real_start = false;
            music.pause = true;
            //SentFrame(0/FRAMESIZE,server->gettotallength()/FRAMESIZE,FRAMESIZE,frequency);
        }
        else
        {
            server->mute_hv(false);//player->mute_hv(false);//tuk zastoto ina4e na vbr_wma Proswirwa w samia krai malko hv
        }

        if (music.quit)
        {
            player->getprocessed();
            player->abort();
            player->resetsoundtype();
            break;
        }

        if (!server->run())
        {
            music_done();
            break;
        }
    }
    delete server;

    if (new_tx_msg=="@TUNE") emit SendTxMsgLabTx(s_prev_disp_msg);

    //qDebug()<<"ELETE";
    return;
}
void MsPlayerHV::xplayfile(char *filename)
{
    char *device=Rawplayer::defaultdevice;
    if (f_message_or_file)
    {
        if (device==NULL) device=Rawplayer::defaultdevice;
        if (device[0]!='/') device=Rawplayer::defaultdevice;
        Rawplayer *player = new Rawplayer; //qDebug()<<"create player";
        if (player==NULL)
        {
            seterrorcode(SOUND_ERROR_MEMORYNOTENOUGH);
            usleep(2000);// 2.48
            return;
        }
        //qDebug()<<"initialize";
        if (!player->initialize(device))
        {
            //qDebug()<<"FALSE player->initialize";//<<errr;
            seterrorcode(player->geterrorcode());
            //2.12 error garmi
            //usleep(1000); 	// no 100% thread
            //delete player;//2.39     old->2.13 stoped no delete crash
            usleep(2000);// 2.48
            return;
        }
        //qDebug()<<"EEEEEE==TX="<<QTime::currentTime().toString("mm:ss:zzz");
        //qDebug()<<"initialize";
        xplaymessage(filename,player);
        usleep(1000); //no 100% thread
        delete player;
        usleep(1000); //2.39 no 100% thread
        //musics.stop=true;//hv 2.39 possyble but no recomended
        return;
    }
}
void MsPlayerHV::xplay()
{
    musics.restart=true;
    musics.errorflag=false;

    //qDebug()<<"pthread_mutex_init";
    pthread_mutex_init(&musics.movelock,NULL);//2.39 inportent crash win lin

    for (;;)
    {
        /*if (music.RealStop)
        {
            SendTxRx(false);
        }
        else if ((QString)filenamehv != "%")
        {
            SendTxRx(true);
        }*/

        if (musics.restart)
        {
            musics.stop=false;
            musics.restart=false;
            musics.currentrun=0;
        }

        if (musics.errorflag) musics.errorflag=false;

        if (musics.move!=0)
        {
            pthread_mutex_lock(&musics.movelock);//2.39 inportent crash win lin

            musics.currentrun+=musics.move;
            if (!musics.stop)
            {
                musics.stop=true;
            }
            musics.move=0;

            pthread_mutex_unlock(&musics.movelock);//2.39 inportent crash win lin
        }

        if (musics.stop)
        {
            usleep(2000); //2.37=2000  2.48=2000
        }
        else
        {
            if (filenamehv != NULL)  //     if(musics.currentrun<splay_listsize)
            {
                //qDebug()<<"START-------------------------------";
                musics.error_repat=false; // vazno hv da e to4no tuk
                music.pause=music.quit=false; //2.47 moved here KP4HF no the fly
                xplayfile(filenamehv);//2.37 w10 no Windows-1251 crash
            }
            else musics.restart=true;
        }
    }
}
void *MsPlayerHV::ThreadEntry(void *ar)
{
    MsPlayerHV* pt = (MsPlayerHV*)ar;
    pthread_mutex_lock(&pt->startuplock);
    pt->xplay();
    pthread_detach(pt->thtx);//2.48
    pthread_exit(NULL);
    return NULL;
}
void MsPlayerHV::Stop()
{
    //qDebug()<<"MsPlayerHV::Stop()===================== ";
    music.RealStop = true;
    music_term();
}
void MsPlayerHV::Restart() //promenena
{
    music.RealStop = false;
}
void MsPlayerHV::setfile_play(char *filename, bool msg_file, int mod_ident,int preiod_t)
{
    int nmsg = strlen(filename); //if (nmsg<1) return;
    if (nmsg<1) filename = (char*)" ";//2.37 protect system for message[0]!='@' in xplaymessage()

    f_message_or_file = msg_file;
    filenamehv = filename;
    s_mode = mod_ident;
    s_period_time_sec = preiod_t;

    if (msg_file)// not needed hv
    {
        //qDebug()<<"setfile_play"<<(QString)filename;
        music_term();
        musics.restart = true;
        Restart();  // zaradi parvoto puskane
    }
    /*else
    {
           if (!only_one_start_cw_gen)
           {
               music_term();
               musics.restart = true;
               Restart();
               only_one_start_cw_gen = true;
           }
           if (stop_cw_global || emidiatly_start_cw)
           {
               stop_cw_global = false;
               count_txt_1 = 0;
               count_read_letter_1=0;
               get_decode(filenamehv[count_txt_1],tex_digits_1);
               count_txt_1++;
           }
           else
               stop_cw_global = true;
    }*/
}
void MsPlayerHV::SetVolume(int volume)
{
    GenMessage::setvolume_all(volume);
}




