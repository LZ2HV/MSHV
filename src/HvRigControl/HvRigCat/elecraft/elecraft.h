/* MSHV
 *
 * By Hrisimir Hristov - LZ2HV
 * May be used under the terms of the GNU General Public License (GPL)
 */

#ifndef ELECRAFT_H
#define ELECRAFT_H

#include <QWidget>

#include "../rigdef.h"

class Elecraft : public QWidget
{
    Q_OBJECT
public:
    Elecraft(int ModelID,QWidget *parent = 0);
    virtual ~Elecraft();

signals:
    void EmitRigSet(RigSet);
    void EmitWriteCmd(char*data,int size);
    void EmitReadedInfo(CmdID,QString);


public slots:
    ////////////////////////////////////////////////////////new read com
    void SetReadyRead(QByteArray,int);
    ////////////////////////////////////////////////////////end new read com

private slots:
    void SetCmd(CmdID,ptt_t,QString);
private:
    int  s_ModelID;
    void set_ptt(ptt_t);
    ////////////////////////////////////////////////////////new read com
    int s_CmdID;
    QByteArray s_read_array;
    QString s_rig_name;
    void set_freq(unsigned long long);
    void get_freq();
    void set_mode(QString);
    void get_mode();
    ////////////////////////////////////////////////////////end new read com
protected:

};
#endif
