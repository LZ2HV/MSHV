/* MSHV
 *
 * By Hrisimir Hristov - LZ2HV
 * May be used under the terms of the GNU General Public License (GPL)
 */
#ifndef YAESU_H
#define YAESU_H

#include <QWidget>
#include <QTimer>

#include "../rigdef.h"

class Yaesu : public QWidget
{
    Q_OBJECT
public:
    Yaesu(int ModelID,QWidget *parent = 0);
    virtual ~Yaesu();

signals:
    void EmitRigSet(RigSet);
    void EmitWriteCmd(char*data,int size);  
    void EmitReadedInfo(CmdID,QString);
    
public slots:

private slots:
    void SetCmd(CmdID,ptt_t,QString);
    void SetReadyRead(QByteArray,int);
    void SetOnOffCatCommand(bool,int,int);
	void SetCmdOk();
	void SetExtCntl();//2.57
	
private:
    int  s_ModelID;
    void set_ptt(ptt_t);

    bool f_echo_back;
    int oldcat_read_ar_size;
    int s_bcd_size;   
    int s_pos_frq;
    int s_method_frq;
    double s_multypl;
    int s_pos_mod;
    unsigned char s_id_mod_usb;
    unsigned char s_id_mod_digu;
       
    int s_CmdID;
    QByteArray s_read_array;
    QString s_rig_name;
    unsigned char s_ncomp;
    QTimer *timer_cmd_ok;
    int i_ext_cntl;
    bool WaitSetExtCntl();
    void SetWriteCmd(char*,int);
    void set_freq(unsigned long long);
    void get_freq();
    void set_mode(QString);  
    void get_mode();
    
    unsigned long long pmr171fB;
    unsigned char pmr171mB;
    uint16_t CRC16Check(const unsigned char *buf, int len);

protected:

};
#endif
