/* MSHV Part from RigControl
 * Copyright 2019 Hrisimir Hristov, LZ2HV
 * May be used under the terms of the GNU General Public License (GPL)
 */
#include "../../../config.h"
#if defined _WIN32_
#ifndef OMNIRIGHV_H
#define OMNIRIGHV_H
#include "omnirig.h"
using namespace OmniRig;

#include <QWidget>
#include <QMessageBox>

#include "../rigdef.h"

class OmniRigHV : public QWidget
{
    Q_OBJECT
public:
    OmniRigHV(int ModelID,QWidget *parent = 0);
    virtual ~OmniRigHV();
    bool InitOmniRigHV();

signals:
    void EmitRigSet(RigSet);
    void EmitWriteCmd(char*data,int size);
    void EmitReadedInfo(CmdID,QString);
    void EmitOmniRigActive(bool,QString);
    void EmitOmniRigChanged();

public slots:
    void SetOmniRigPttMethod(bool*);

private slots:
    void SetCmd(CmdID,ptt_t,QString);
    //void SetReadyRead(QByteArray,int);
    void SetTypeChange(int);

private:
    int  s_ModelID;
    void set_ptt(ptt_t);

    int s_CmdID;
    //QByteArray s_read_array;
    QString s_rig_name;
    void set_freq(unsigned long long);
    void get_freq();
    void set_mode(QString);
    void get_mode();
    bool ptt_dtr;
    bool ptt_rts;
    bool ptt_cat;
    bool ptt_off;
    bool rts_on;
    bool dtr_on;
    int SetFreqMethod;
    int GetFreqMethod;//qint8

protected:
    //IOmniRigX *m_pOmniRig;
    OmniRigX *TOmniRig;
    RigX *TRigX;
    PortBits *TPortBits;

};
#endif
#endif