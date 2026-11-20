#ifndef GEN_SFOX_H
#define GEN_SFOX_H
//#include <QObject> 
#include <QString>
//#include <QApplication>
//#include <QCoreApplication>
//#include <stdio.h>      /* printf */
//#include <math.h>       /* fmod */

// JTMSK144 ///////////////////////
//#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>

//#include "../HvPackUnpackMsg/pack_unpack_msg77.h"
#include "../genpom.h"
#include "../HvPackUnpackMsg/pack_unpack_msg.h"
#include "../HvPackUnpackMsg/pack_unpack_msg77.h"
#include <QStringList>


//#include <complex.h> // gnu++11 c++11
//#define complex		_Complex
class GenSFox //: public QObject
{
	//Q_OBJECT // hv  
public: 
    explicit GenSFox(bool fl);//f_dec_gen = dec=true gen=false
    ~GenSFox();
 
    int gensfox(QString,int *t_iwave,double samp_rate,double f0,QString,QString);//,int i3b ,int &ntxslot 
    /*void make_c77_i4tone(bool *c77,int *i4tone); 
    void make_c77_i4tone_codeword(bool *c77,int *i4tone,bool*);*/                          
    QString GetUnpackMsg(){return s_unpack_msg;};//s_unpack_msg
    QString sfox_unpack(unsigned char *,bool);   
    /*void save_hash_call_from_dec(QString c13,int n10,int n12,int n22);
    void save_hash_call_my_his_r1_r2(QString call,int pos);
    //void save_hash_call_mam(QStringList ls);
    QString unpack77(bool *c77,bool &unpk77_success); 
    void pack77(QString msgs,int &i3,int n3,bool *c77); 
    void split77(QString &msg,int &nwords,QString *w);*/ 
    	
private:  
    //bool gen_dec;
    GenPomFt genPomFt;
    QString s_unpack_msg;
    double twopi;
    //QString format_msg(char *message_in, int cmsg);
    //void make_c77_i4tone(bool *c77,int *i4tone);//,bool f_gen,bool f_addc
    PackUnpackMsg77 TPackUnpackMsg77;
    PackUnpackMsg TPackUnpackMsg;
    //QString mycall;
	QString msg1;
	QString rpt2[5];
	QString hiscall[10];
	int nmsg[4];
	int nb_mycall;
	int nbits;
    QString sfox_assemble(int,int,QString,QString/*,QString*/);
    QString foxgen2(int,QStringList);
    //QString sfox_unpack(unsigned char *);
    void sfox_pack(QString line,QString ckey,bool bMoreCQs,bool bSendMsg,QString freeTextMsg,unsigned char *xin);
    void SetArrayBits(int in,int in_bits,bool *ar,int &co);
    int BinToInt32(bool*a,int b_a,int bits_sz);
    void u8shift1(unsigned char *a,int cou_a,int ish);
    //otp//
    /*QByteArray generateHOTP(const QByteArray &rawSecret, quint64 counter, int length);
    QByteArray fromBase32(const QString &input);
    QString generateHOTP(const QString &secret, quint64 counter, int length);
    QByteArray generateTOTP(const QByteArray &rawSecret, int length);
    QString generateTOTP(const QString &secret, int length);
    QString generateTOTP(const QString &secret, QDateTime dt, int length);
    QString foxOTPcode(QString);*/
    // end otp//
   
};
#endif