/* MSHV
 *
 * By Hrisimir Hristov - LZ2HV
 * May be used under the terms of the GNU General Public License (GPL)
 */
#ifndef MITS_H
#define MITS_H
 
#include <QWidget> 

#include "../rigdef.h"

class Mits : public QWidget
{
    Q_OBJECT
public:
    Mits(int ModelID,QWidget *parent = 0);
    virtual ~Mits();

signals:
    void EmitRigSet(RigSet);
    void EmitWriteCmd(char*data,int size);
    void EmitReadedInfo(CmdID,QString);
    void EmitPttDtr(bool);
    
public slots:
	void SetOnOffCatCommand(bool f, int model_id, int fact_id);
	
private slots:
    void SetCmd(CmdID,ptt_t,QString);
    void SetReadyRead(QByteArray,int);
    
private:
	void MakeCommandAndSend(QString);
    int  s_ModelID;
    void set_ptt(ptt_t);
    int s_CmdID;
    QByteArray s_read_array;
    void set_freq(unsigned long long);
    void get_freq();
    void set_mode(QString);
    void get_mode();
    
protected:

};
#endif
