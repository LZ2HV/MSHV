/* MSHV
 *
 * By Hrisimir Hristov - LZ2HV
 * May be used under the terms of the GNU General Public License (GPL)
 */
#ifndef TENTEC_H
#define TENTEC_H

#include <QWidget>

#include "../rigdef.h"

class Tentec : public QWidget
{
    Q_OBJECT
public:
    Tentec(int ModelID,QWidget *parent = 0);
    virtual ~Tentec();

signals:
    void EmitRigSet(RigSet);
    void EmitWriteCmd(char*data,int size); 
    void EmitReadedInfo(CmdID,QString);

public slots:

private slots:
    void SetCmd(CmdID,ptt_t,QString);
    ////////////////////////////////////////////////////////new read com
    void SetReadyRead(QByteArray,int);
    ////////////////////////////////////////////////////////end new read com
private:
    int  s_ModelID;
    void set_ptt(ptt_t);
    ////////////////////////////////////////////////////////new read com
    int s_CmdID;
    QByteArray s_read_array;
    unsigned char EOM;
    unsigned char s_model_addr;
    void set_freq(unsigned long long);
    void get_freq();
    void set_mode(QString);
    void get_mode();
    ////////////////////////////////////////////////////////end new read com
protected:

};
#endif
