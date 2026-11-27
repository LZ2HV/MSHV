/* MSHV
 *
 * By Hrisimir Hristov - LZ2HV
 * May be used under the terms of the GNU General Public License (GPL)
 */

#ifndef MSPLAYERHV_H
#define MSPLAYERHV_H
//
//#include <QtGui>

#include "../config.h"

#include <QObject>


#include "libsound/mpegsound.h"
#include <unistd.h> // zaradi usleep

//#include <QTextCodec> //zaradi cp1251 za da sviri pod windows

typedef unsigned char byte01_;

//#define SAVE_SAMPLE_RATE 11025
// void Setcurrentmpegstatus(int version,int layer,
//			  int frequency,int bitrate,
//			  bool crc,int mode,
//			  bool forcetomono,int downfreq); // neraz

//bool getquotaflag(void); // neraz
//void Unlockframeslider(void);  // neraz
//void Setframestatus(int frame,int maxframe,int pcmperframe,int frequency); //neznam
//void Clearcurrentstatus(void); // neznam

/////////////////////////Ststic Structure//////////////////////////////////

class MsPlayerHV : public QObject
{
    Q_OBJECT
public:
    MsPlayerHV(QString);
    virtual~MsPlayerHV();
    void OpenFile(char*);
    void SetModeForWavSaves(int);
    bool Is_RealStop()// 2.56 old=slots
    {
        return music.RealStop;
    };
    void setfile_play(char *,bool,int mod_ident,int);
    void Stop();
    void SetTxFreq(double);// ft8 for the moment 1.43
    void SetMsf(bool);
    void SetOffsetDt(int dt);//2.76.5

signals:
    //void SendTxRx(bool);
    void SentData(int*,int,bool); //1.27 psk rep fopen
    void SentFileClarDisplay();
    void SendTxMsgLabTx(QString);
    void SendTxMsgAllTxt(QString,double);

public slots:
    //void setfile_play(char *,bool,int mod_ident,int); //int tx_dilay
    //void Stop();
    //void Restart();
    void SetVolume(int);
    void SetSoundDevice(QString dev,int,int buffer);
    void SaveFile(int*,int,QString);
    void SetSFMATxAll(QString);
    void SetOtpTxKey(QString);

private:
	int s_offset_dt;
	int s_bitpersample;
    char dev_in_p[128];//2.50
    QString s_prev_disp_msg;
    QString s_prev_tx_msg;
    
    struct msui
    {
        bool RealStop;
        bool pause;
        bool quit;
        //bool setframeflag;
        //int  setframenumber;
    };

    struct	mus
    {
        bool stop;
        bool restart;
        int  move;
        int  currentrun;
        pthread_mutex_t movelock;
        bool errorflag;
        int  errorcode;
        bool error_repat;
    };

    mus musics;
    msui music;
    bool f_message_or_file;
    int s_mode;
    QString App_Path;
    double s_tx_freq;// = 1200.0 //ft8 for the moment
    double SAVE_SAMPLE_RATE;   
    char *filenamehv;
    int s_period_time_sec;
    void Restart();
    bool s_msf;
    QString sfma_tx_all;
    QString otp_tx_key;
    void GetSsMsAndOffset(int &,int &);

protected:
    void xplay();
    void xplayfile(char *filename);
    void xplaymessage(char *mess,Rawplayer *player);
    void music_term(void);
    void music_move(int value);
    void music_done(void);
    void seterrorcode(int errcode);

	pthread_t thtx;
    pthread_mutex_t startuplock;  // za 64bit qt4 4.4.0 da tragva s puskaneto iska da e tuk
    static void *ThreadEntry(void*);
    //short ShortSwap(short s);
};
#endif
