#ifndef GEN65_H
#define GEN65_H

//#include <QString>
//#include <QStringList>
//#include <math.h>       /* fmod */
#include "../HvPackUnpackMsg/pack_unpack_msg.h"


class Gen65 //: public QObject
{
	//Q_OBJECT // hv
public: 
    Gen65();
    ~Gen65(); 
	int gen65(char *cmsg,int mode65,int nfast,double samfac,double gensamplerate,int *t_iwave);//double ntxdf,double koef_srate,
	QString GetUnpackMsg(){return s_unpack_msg;};  
	double pr[126];
	int mdat[126]; 
	int mdat2[126];     
	void setup65();
	void interleave63(int *d1,int idir);
	void graycode(int *ia,int n,int idir,int *ib);
	void graycode65(int *dat,int n,int idir);
	QString unpackmsg(int *dat,char &ident);
	void rs_encode(int *dgen, int *sent);// for deep search 65
	void packmsg(char *cmsg,int *dgen,int &itype);// for deep search 65

private:
   //int count_1s;
   int sendingsh;
   QString s_unpack_msg;
   PackUnpackMsg TPackUnpackMsg;
   
   int mref_[2][126];
   int mref2_[2][126];
   void *rs;
   bool first;
   //void rs_encode(int *dgen, int *sent);
   void move(int *x,int *y, int n);
   int igray(int n0, int idir);
   void chkmsg(QString &message,QString &cok,int &nspecial,double &flip);
   double twopi;

};
#endif
