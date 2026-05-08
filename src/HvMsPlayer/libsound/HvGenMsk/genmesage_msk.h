#ifndef GENMESAGE_MSK_H
#define GENMESAGE_MSK_H
#include <QObject> 
#include <QString>
//#include <QApplication>
//#include <QCoreApplication>
//#include <stdio.h>      /* printf */
#include <math.h>       /* fmod */

// JTMSK144 ///////////////////////
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../HvPackUnpackMsg/pack_unpack_msg.h"
#include "../HvPackUnpackMsg/pack_unpack_msg77.h"

// JTMSK144 ///////////////////////

class GenMsk //: public QObject
{
	//Q_OBJECT // hv
public: 
    GenMsk(bool f);//f_dec_gen = dec=true gen=false
    ~GenMsk();

    int genmsk(char *msg0,double samfac,int *i4tone,bool f_generate,int *t_iwave,double samp_rate,
                double k_srate,int ichk,int mode,bool rpt_db_msk);//QString mygridloc,
        
    // vremenno            
    QString unpackmsg144(int *dat,char &ident,QString &c1,QString &c2,bool rpt_db_msk);            
    //QString unpackmsg(int *dat,char &ident);

    QString GetUnpackMsg(){return s_unpack_msg;};	
    
    // JTMSK144 ///////////////////////
    int hash_msk40(QString s);

    //void ldpc_decode( double lratio[], char decoded[], int max_iterations, int &niterations, int max_dither, int &ndither); 
    void bpdecode144(double *llr,int maxiterations,char *decoded,int &niterations);
    void bpdecode40(double *llr,int maxiterations,char *decoded,int &niterations);
    void encode_msk40(char *message,char *codeword);		
	
	void save_hash_call_my_his_r1_r2(QString call,int pos);
	QString unpack77(bool *c77,bool &unpk77_success);
	void chkcrc13a(bool *decoded,int &nbadcrc);
	void bpdecode128_90(double *llr,int max_iterations,bool *decoded77,int &nharderror);//,bool *cw ,bool *apmask, int &niterations
	//QString Get_c1_msk40_hash(){return TPackUnpackMsg77.c1_msk40_hash;};
	//QString Get_c2_msk40_hash(){return TPackUnpackMsg77.c2_msk40_hash;};
	void Get_C1_C2_RX_Calls(QString &c1,QString &c2);

private:  
	int hash(QString s, int len, int ihash);
	// MSK40 ///////////////////////
	//char gen40_[16][16];//integer*1 gen144(48,80)
	char gen40_[20][20]; 
	bool first_msk40_enc;
	//void encode_msk40(char *message,char *codeword);	
	
	QString genmsk40(QString msg,int ichk,int *itone,int &itype);
	//bool first_msk40;
    //int ig24_msk32[4096];
	// MSK32 ///////////////////////	
	
	// MSK144 ///////////////////////	
	void platanh(double x,double &y);
	
    //void ldpc_write_qpc_temp_file(char *qrc_f, char *file_t);
	double pp[12];	
	
	void genmsk144(int *i4, unsigned char *i8, int *itone,bool *c77,bool msk144ms);
	bool first_msk144;
	void copy_char_ar(char*a,int a_beg,int a_end,char*b,int b_beg,int b_odd);
	void copy_double_ar(double*a,int a_beg,int a_end,double*b,int b_beg);
	
	//char gen144_[80][48];//integer*1 gen144(48,80)
	char gen144_[82][50];
	bool first_msk144_enc;
	void encode_msk144(char *msg, char *cdw);// makms
	
	short crc13(unsigned char const * data, int length);
	bool gen144_v2_[95][43];//integer*1 gen(M,K)  N=128, K=90, M=N-K=38
	bool first_msk144_enc_v2;
	void encode_128_90(bool *message77,bool* codeword);// msk144
	
	QString RemWSpacesInside(QString s);// fmtmsg(QString msg,int iz); 
	// MSK144 ///////////////////////
	
	QString s_unpack_msg;
	PackUnpackMsg TPackUnpackMsg;
	PackUnpackMsg77 TPackUnpackMsg77;
	
    double twopi;
    //QString addpfx;
};
#endif