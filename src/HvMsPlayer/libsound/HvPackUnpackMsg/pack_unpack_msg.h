#ifndef PACK_UNPACK_MSG_H
#define PACK_UNPACK_MSG_H

#include <QString>
//#include <QStringList>
#include <math.h>       /* fmod */
#include "../../../HvTxW/hvqthloc.h"

static const char C_PACK_UNPACK_JT[42]=
    {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F','G','H','I',
     'J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z',' ','+',
     '-','.','/','?'
     };

class PackUnpackMsg 
{
public: 
	void packmsg(char *message,int *i,int &itype,bool msk144ms); 
	QString unpackmsg144(int *dat,char &ident,QString &c1,QString &c2,bool msk144ms);          
    QString unpackmsg(int *dat,char &ident,bool msk144ms);
    int index(QString,char search);
    void copy_qstr(QString &to,QString from,int b_from,int e_from);
    void grid2deg(QString grid0,double &dlong,double &dlat);
    QString qstr_beg_end(QString s_in,int b,int e);
    void deg2grid(double dlong0,double dlat,QString &grid);
    void packgrid(QString grid,int &ng,bool &text,bool msk144ms);
    void unpackgrid(int ng,QString &grid,bool msk144ms);//2.76

private:
    int CalcDistance(QString c_my_loc,QString c_test_loc);
    HvQthLoc THvQthLoc;
    QString RemBegEndWSpaces(QString str);
    int max_2int(int a,int b);
    int nchar(char c);    
    void packcall(QString c1,int &nc1,bool &text1);
    void getpfx1(QString &callsign,int &k,int &nv2);
    void packtext(QString msg,int &nc1,int &nc2,int &nc3);
    void k2grid(int k,QString &grid);
    //void packgrid(QString grid,int &ng,bool &text,bool msk144ms);
	///Pack///
	///UnPack///
    QString unpacktext(int nc1,int nc2,int nc3,QString msg);
    void unpackcall(int nc1,QString &c1,int &iv2,QString &psfx);
    void grid2k(QString grid6,int &k);
    //void unpackgrid(int ng,QString &grid,bool msk144ms);
    void getpfx2(int k0,QString &callsign);
	///UnPack///
	QString addpfx;
	
};
#endif
