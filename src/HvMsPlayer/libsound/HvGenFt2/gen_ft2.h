#ifndef GEN_FT2_H
#define GEN_FT2_H
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
 
#include "../HvPackUnpackMsg/pack_unpack_msg77.h"
#include "../genpom.h"
//#include <QStringList>

/*
#include <complex.h> // gnu++11 c++11
#define complex		_Complex
#include "../../../Hv_Lib_fftw/fftw3.h"
*/
//#include <complex.h> // gnu++11 c++11
//#define complex		_Complex
class GenFt2  
{
	//Q_OBJECT // hv
public: 
    explicit GenFt2(bool fl);//f_dec_gen = dec=true gen=false
    ~GenFt2();
 
    int genft2(QString,int *t_iwave,double samp_rate,double f0);//,int i3b ,int &ntxslot 
    void make_c77_i4tone_codeword(bool *c77,int *i4tone,bool *codeword);  
    void make_c77_i4tone(bool *c77,int *i4tone);//,bool f_gen,bool f_addc           
    QString GetUnpackMsg(){return s_unpack_msg;};
    void save_hash_call_mam(QStringList ls);
    QString unpack77(bool *c77,bool &unpk77_success); 
    void pack77(QString msgs,int &i3,int n3,bool *c77);
    void split77(QString &msg,int &nwords,/*int *nw,*/QString *w);
    void encode174_91(bool *message,bool *codeword); 
    	
private: 
    GenPomFt genPomFt;       
    PackUnpackMsg77 TPackUnpackMsg77;
    QString s_unpack_msg;
    double twopi;
    //QString format_msg(char *message_in, int cmsg);
};
#endif