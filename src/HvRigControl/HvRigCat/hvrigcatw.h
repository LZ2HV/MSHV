/* MSHV HvRigCat
 * Copyright 2015 Hrisimir Hristov, LZ2HV
 * May be used under the terms of the GNU General Public License (GPL)
 */

#ifndef HVRIGCAT_H
#define HVRIGCAT_H

#include <QWidget>
#include <QComboBox>
#include <QLabel>
#include <QHBoxLayout>
//#include <QStandardItemMode_l>
#include <QRadioButton>
#include <QCheckBox>
#include <QTimer>
#include <QSpinBox>
#include <QLineEdit>

//#include <QtGui>

//#include "../qextserialport_1_2rc/qextserialport.h"
//#include "rigdef.h"
//#include "../qextserialport_1_2rc/qextserialport.h"


#include "../../config.h"
//#include <unistd.h>   // uslepp Linux and windows
//#include <windows.h>  // Slepp windows

/*
#if defined _LINUX_
#include <unistd.h>
#endif
#if defined _WIN32_
#include <windows.h>
#endif
*/

#include "rigdef.h"

/*
#if defined _LINUX_
#include <alsa/asoundlib.h>
#include <pthread.h> //	pthread_create, pthread_mutex_unlock
#endif
*/

/*
#if defined _LINUX_
#ifdef PTHREADEDMPEG
#ifdef HAVE_PTHREAD_H
#include <pthread.h>
#else
#ifdef HAVE_PTHREAD_MIT_PTHREAD_H
#include <pthread/mit/pthread.h>
#endif
#endif
#endif
#endif
*/
class HvRigCat : public QWidget
{
    Q_OBJECT
public:
    HvRigCat(QWidget *parent = 0);
    virtual ~HvRigCat();

    void set_ptt(bool,bool);
    QString get_rig_name()
    {
        return RigsBoxs->currentText();
    };
    QString get_ptt_type()
    {
        QString s = "0";
        if (rb_ptt_mic->isChecked()) s = "1";
        if (rb_ptt_data->isChecked()) s = "2";
        s.append("#"+QString("%1").arg(sb_pollint->value()));
        return s;
    };
    void SetRigName(QString);
    void SetPttCatType(QString);

    ////////////////////////////////////////////////////////new read com
    void set_freq(QString);
    void SetReadyRead(QByteArray,int);
    void StartStopPollingTimer(bool popen);/*, bool read_data_rts_on*/
    void set_mode(QString);
    ////////////////////////////////////////////////////////end new read com
    void get_freq();
    //void SetMode(int);//tci
    /*void SetTciRig(int);//tci
    int get_tci_rig()//tci
    {
        return TciRigBox->currentIndex();
    };*/
    void SetTciSelect(int);
    //void SetFont(QFont);
    

signals:
    void EmitCatAactiveAndRead(bool,bool);
////// omnirig /////////////
#if defined _WIN32_
    void EmitOmniRigActive(bool,QString);
    void EmitOmniRigPttMethod(bool*);
#endif
////// omnirig end /////////////
    void EmitNetSP(QString);
    void EmitNetConnInfo(QString,bool,bool);
    void DesleteRig();
    void SetCmd(CmdID,ptt_t,QString);
    void EmitWriteCmd(char*data,int size);
    void EmitSetRigSet(RigSet,int,int);
    //void EmitMode(int);//tci

    ////////////////////////////////////////////////////////new read com
    void EmitReadyRead(QByteArray,int);
    void EmitGetedFreq(QString);
    void EmitGetedMode(QString);
    void EmitOnOffCatCommand(bool,int,int);
    void EmitRigCatActiveAndReadF(bool);
    //void EmitReadDataRtsOnTrue(bool);
    ////////////////////////////////////////////////////////end new read com
    void EmitPttDtr(bool);// sea-235
    void EmitTciSelect(int);
    void EmitFullRigInfo(QString);//2.76.1

private slots:
////// omnirig /////////////
#if defined _WIN32_
    void SetOmniRigChanged();
    void SetOmniRigReinit();
#endif
////// omnirig end /////////////
    void SetRig(int);
    //void SetRig1(int);
    void SetRigSet(RigSet);
    void SetWriteCmd(char*data,int size);
    void PttChanget(bool f);
    ////////////////////////////////////////////////////////new read com
    void SetReadedInfo(CmdID,QString);
    //void get_freq();
    //void get_mode();
    void PollingTimerReadRig();
    void StartPttTimer();
    ////////////////////////////////////////////////////////end new read com
    void SBPollIntChanged(int);

private:
    //int s_mode;//tci
    //QComboBox *TciRigBox;//tci
    void get_mode();
    int s_active_model_id;
    int s_active_fact_id;
    //void InsertItemModel(QStringList list);
    //QStandardItemMode_l model;
    /*typedef struct
	{
		QString name;
    	int facid;
    	int model;
	} 
	rigstruct;
    QList<rigstruct>lmodel;
    bool hv_sort_nameorder(const rigstruct &d1,const rigstruct &d2);*/
    QComboBox *RigsBoxs;
    bool f_rig_activ;
    void DestroyRig();
    QRadioButton *rb_ptt;
    QRadioButton *rb_ptt_mic;
    QRadioButton *rb_ptt_data;
    QSpinBox *sb_pollint;

    ptt_t s_ptt_on_type;
    void SetPttCaps(bool);
    rig_port_t s_port_type;
    bool s_port_poen;
    int c_poll_comm;
    int c_rig_cat_active_and_read;
    //void StartTimerPrivate();
    int have_read_data_rts_on;
    //bool s_cb_read_data_rts_on;
//////////////////////////////////////////////
    //QextSerialPort *port;
//////////////////////////////////////////////
    bool s_f_ptt;
    int tci_select;
    int max4min;
    bool f_rig_active_never_stop;//2.76.1 Flex Slice
    
protected:
////// omnirig /////////////
#if defined _WIN32_
    QTimer *omnirig_reinit_timer;
#endif
////// omnirig end /////////////
    QTimer *polling_timer;
    QTimer *tx_rx_timer;

};

#endif
