/* The algorithms, source code, look-and-feel of WSJT-X and related
 * programs, and protocol specifications for the modes FSK441, FT8, JT4,
 * JT6M, JT9, JT65, JTMS, QRA64, ISCAT, MSK144, are Copyright © 2001-2017
 * by one or more of the following authors: Joseph Taylor, K1JT; Bill
 * Somerville, G4WJS; Steven Franke, K9AN; Nico Palermo, IV3NWV; Greg Beam,
 * KI7MT; Michael Black, W9MDB; Edson Pereira, PY2SDR; Philip Karn, KA9Q;
 * and other members of the WSJT Development Group.
 *
 * MSHV Q65 Generator
 * Rewritten into C++ and modified by Hrisimir Hristov, LZ2HV 2015-2021
 * May be used under the terms of the GNU General Public License (GPL)
 */
#ifndef GEN_Q65_H
#define GEN_Q65_H
//#include <QObject> 
#include <QString>
 
#include "../HvPackUnpackMsg/pack_unpack_msg77.h"
#include "q65_subs.h"

class GenQ65   
{
	//Q_OBJECT // hv
public: 
    explicit GenQ65(bool fl);//f_dec_gen = dec=true gen=false
    ~GenQ65();

 	void genq65itone(QString msg0,int *itone,bool);
    int genq65(QString,int *t_iwave,double samp_rate,double f0,int mq65,int period_t);//,int i3b ,int &ntxslot 
    QString GetUnpackMsg(){return s_unpack_msg;}; 
    QString unpack77(bool *c77,bool &unpk77_success);  
    void pack77(QString msgs,int &i3,int n3,bool *c77);          
    	
private:   
	QString s_unpack_msg;
	PackUnpackMsg77 TPackUnpackMsg77;
	q65subs q65S;
	int BinToInt32(bool*a,int b_a,int bits_sz);
    //QString format_msg(char *message_in, int cmsg);
};
#endif