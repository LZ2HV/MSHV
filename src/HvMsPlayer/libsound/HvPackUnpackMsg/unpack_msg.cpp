/* The algorithms, source code, look-and-feel of WSJT-X and related
 * programs, and protocol specifications for the modes FSK441, FT8, JT4,
 * JT6M, JT9, JT65, JTMS, QRA64, ISCAT, MSK144, are Copyright © 2001-2017
 * by one or more of the following authors: Joseph Taylor, K1JT; Bill
 * Somerville, G4WJS; Steven Franke, K9AN; Nico Palermo, IV3NWV; Greg Beam,
 * KI7MT; Michael Black, W9MDB; Edson Pereira, PY2SDR; Philip Karn, KA9Q;
 * and other members of the WSJT Development Group.
 *
 * MSHV UnpackMessage
 * Rewritten into C++ and modified by Hrisimir Hristov, LZ2HV 2015-2017
 * May be used under the terms of the GNU General Public License (GPL)
 */

#include "pack_unpack_msg.h"
#include "../../../pfx_sfx.h"

//#include <QtGui>
/*QString GenMsk::CharToQString(char* ch, int count)
{
    QString s;
    for (int j = 0; j<count; j++)
        s.append(ch[j]);
    return s;
}*/
QString PackUnpackMsg::RemBegEndWSpaces(QString str)
{
    QString s;
    /*int msg_count = 0;//2.64 stop
    for (msg_count = str.count()-1; msg_count>=0; msg_count--)
    {
        if (str.at(msg_count)!=' ')
            break;
    }
    s = str.mid(0,msg_count+1);
    msg_count = 0;
    for (msg_count = 0; msg_count<s.count(); msg_count++)
    {
        if (s.at(msg_count)!=' ')
            break;
    }
    s = s.mid(msg_count,(s.count()-msg_count));*/
	s = str.trimmed();
    return s;
}
QString PackUnpackMsg::unpacktext(int nc1,int nc2,int nc3,QString msg)
{
    /*character*22 msg
    character*44 c
    data c/'0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ +-./?'/*/
    int j = 0;

    nc3=(nc3 & 32767);                     //!Remove the "plain text" bit
    if ((nc1 & 1)!=0) nc3=nc3+32768;
    nc1=nc1/2;
    if ((nc2 & 1)!=0) nc3=nc3+65536;
    nc2=nc2/2;

    for (int i = 4; i >= 0; i--)
    {//do i=5,1,-1
        //j=fmod(nc1,42)+0;
        j=(nc1 % 42);
        msg[i]=C_PACK_UNPACK_JT[j];
        nc1=nc1/42;
    }

    for (int i = 9; i >= 5; i--)
    {//do i=10,6,-1
        //j=fmod(nc2,42)+0;
        j=(nc2 % 42);
        msg[i]=C_PACK_UNPACK_JT[j];
        nc2=nc2/42;
    }

    for (int i = 12; i >= 10; i--)
    {//do i=13,11,-1
        //j=fmod(nc3,42)+0;
        j=(nc3 % 42);
        msg[i]=C_PACK_UNPACK_JT[j];
        nc3=nc3/42;
    }
    for (int i = 13; i < 23; i++)
        msg[i] = ' ';


    return msg;
}
void PackUnpackMsg::unpackcall(int ncall,QString &word,int &iv2,QString &psfx)
{

    //parameter (NBASE=37*36*10*27*27*27)
    //character word*12,c*37,psfx*4

    //data c/'0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ '/
    char c[37]={'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E',
                'F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z',' '};
    int i = 0;

    word="......";
    psfx="    ";
    int n=ncall;
    iv2=0; //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    if (n>=262177560) goto c20;
    word="......";
    //!  if(n.ge.262177560) go to 999            !Plain text message ...262177561
    //i=fmod(n,27)+10;
    i=(n % 27)+10;
    word[5]=c[i];
    n=n/27;
    //i=fmod(n,27)+10;
    i=(n % 27)+10;
    word[4]=c[i];
    n=n/27;
    //i=fmod(n,27)+10;
    i=(n % 27)+10;
    word[3]=c[i];
    n=n/27;
    //i=fmod(n,10)+0;
    i=(n % 10);
    word[2]=c[i];
    n=n/10;
    //i=fmod(n,36)+0;
    i=(n % 36);
    word[1]=c[i];
    n=n/36;
    i=n+0;
    word[0]=c[i];

    for (i = 0; i < 4; i++)
    {//do i=1,4
        if (word[i]!=' ') goto c10;
    }
    goto c999;
c10:
    word=qstr_beg_end(word,i,word.count());
    goto c999;

c20:
    if (n>=267796946) goto c999;//262177561
//qDebug()<<"DDDDDDDDDDDDDDDDDDDDD";
//! We have a JT65v2 message//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    if ((n>=262178563) && (n<=264002071))
//! CQ with prefix
    {   iv2=1;
        n=n-262178563;
        //i=fmod(n,37)+0;
        i=(n % 37);
        psfx[3]=c[i];
        n=n/37;
        //i=fmod(n,37)+0;
        i=(n % 37);
        psfx[2]=c[i];
        n=n/37;
        //i=fmod(n,37)+0;
        i=(n % 37);
        psfx[1]=c[i];
        n=n/37;
        i=n+0;
        psfx[0]=c[i];
    }
    else if ((n>=264002072) && (n<=265825580))
//! QRZ with prefix
    {   iv2=2;
        n=n-264002072;
        //i=fmod(n,37)+0;
        i=(n % 37);
        psfx[3]=c[i];
        n=n/37;
        //i=fmod(n,37)+0;
        i=(n % 37);
        psfx[2]=c[i];
        n=n/37;
        //i=fmod(n,37)+0;
        i=(n % 37);
        psfx[1]=c[i];
        n=n/37;
        i=n+0;
        psfx[0]=c[i];
    }
    else if ((n>=265825581) && (n<=267649089))//262177561
//! DE with prefix
    {  iv2=3;
        n=n-265825581;
        //i=fmod(n,37)+0;
        i=(n % 37);
        psfx[3]=c[i];
        n=n/37;
        //i=fmod(n,37)+0;
        i=(n % 37);
        psfx[2]=c[i];
        n=n/37;
        //i=fmod(n,37)+0;
        i=(n % 37);
        psfx[1]=c[i];
        n=n/37;
        i=n+0;
        psfx[0]=c[i];
    }
    else if ((n>=267649090) && (n<=267698374))//262177561
//! CQ with suffix
    {   iv2=4;
        n=n-267649090;
        //i=fmod(n,37)+0;
        i=(n % 37);
        psfx[2]=c[i];
        n=n/37;
        //i=fmod(n,37)+0;
        i=(n % 37);
        psfx[1]=c[i];
        n=n/37;
        i=n+0;
        psfx[0]=c[i];
    }
    else if ((n>=267698375) && (n<=267747659))
//! QRZ with suffix
    {  iv2=5;
        n=n-267698375;
        //i=fmod(n,37)+0;
        i=(n % 37);
        psfx[2]=c[i];
        n=n/37;
        //i=fmod(n,37)+0;
        i=(n % 37);
        psfx[1]=c[i];
        n=n/37;
        i=n+0;
        psfx[0]=c[i];
    }
    else if ((n>=267747660) && (n<=267796944))
//! DE with suffix
    {   iv2=6;
        n=n-267747660;
        //i=fmod(n,37)+0;
        i=(n % 37);
        psfx[2]=c[i];
        n=n/37;
        //i=fmod(n,37)+0;
        i=(n % 37);
        psfx[1]=c[i];
        n=n/37;
        i=n+0;
        psfx[0]=c[i];  //qDebug()<<"DDDDDDDDDDDDDDDDDDDDD";
    }
    else if (n==267796945)
//! DE with no prefix or suffix
    { iv2=7;
        psfx = "    ";
    }

c999:
    if (qstr_beg_end(word,0,3)=="3D0") word="3DA0"+qstr_beg_end(word,3,word.count());//word(4:)
    if (qstr_beg_end(word,0,1)=="Q") word="3X"+qstr_beg_end(word,1,word.count());
    //if(word(1:1).eq.'Q') word='3X'//word(2:)

    word = RemBegEndWSpaces(word);
    psfx = RemBegEndWSpaces(psfx);
    //qDebug()<<"DDD="<<iv2<<psfx;


}

void PackUnpackMsg::unpackgrid(int ng,QString &grid,bool msk144ms)// rpt_db_msk
{
    int NGBASE=180*180;
    QString grid6="      ";//vazno da e taka hv
    int n;
    grid="    ";//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    double dlat=fmod(ng,180)-90;
    double dlong=(ng/180)*2 - 180 + 2;

    if (ng>=32400) goto c10;

    deg2grid(dlong,dlat,grid6);
    grid=qstr_beg_end(grid6,0,4);

    if (qstr_beg_end(grid,0,2)=="KA")// old loc
    {
        //read(grid(3:4),*) n
        n=qstr_beg_end(grid,2,4).toInt();
        //n=n-50;  //qDebug()<<"NNNNNN1="<<n;
        if (msk144ms)
        {
            n=n-30;
            grid = QString("%1").arg(n,2,10,QChar('0'))+" ";
        }
        else
        {
            n=n-50;
            grid = "+"+QString("%1").arg(n,2,10,QChar('0'))+" ";
        }
        //if (grid[0]==' ') grid[0]='+';
    }
    else if (qstr_beg_end(grid,0,2)=="LA")// old loc
    {
        //read(grid(3:4),*) n
        n=qstr_beg_end(grid,2,4).toInt();
        //n=n-50; //qDebug()<<"NNNNNN2="<<n;
        if (msk144ms)
        {
        	n=n-30;
            grid = "R"+QString("%1").arg(n,2,10,QChar('0'));
        }
        else
        {
        	n=n-50;
            grid = "R+"+QString("%1").arg(n,2,10,QChar('0'));
        }
        //if (grid[1]==' ') grid[1]='+';
    }
    goto c900;
c10:
    n=ng-NGBASE-1;//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    if (n>=1 && n<=30)
    {
        //grid.replace(2,2,QString("%1").arg((n)));
        grid = "-"+QString("%1").arg(n,2,10,QChar('0'))+" ";
        //write(grid,1012) -n
        //1012 format(i3.2)
    }
    else if (n>=31 && n<=60)
    {
        n=n-30;
        //grid.replace(2,2,QString("%1").arg((n)));
        grid = "R-"+QString("%1").arg(n,2,10,QChar('0'));
        //write(grid,1022) -n
        //1022 format('R',i3.2)
    }
    else if (n==61)
    {
        grid="RO  ";
    }
    else if (n==62)
    {
        grid="RRR ";
    }
    else if (n==63)
    {
        grid="73  ";
    }
c900:
    return;
}

void PackUnpackMsg::grid2k(QString grid,int &k)
{
    /*double xlong,xlat;
    grid2deg(grid,xlong,xlat);
    int nlong=(int)xlong;
    int nlat=(int)xlat; //qDebug()<<"LONGLAT="<<xlong<<xlat<<grid;
    k=0;
    if (nlat>=85) k=5*(nlong+179)/2 + nlat-84+0;//if(nlat.ge.85) k=5*(nlong+179)/2 + nlat-84*/
    double xlong,xlat;
    grid2deg(grid,xlong,xlat);
    k=0;
    if (xlat>=85.0) k=5.0*(xlong+179.0)/2.0 + xlat-84.0+1;
    //qDebug()<<"KK="<<k;
}

void PackUnpackMsg::getpfx2(int k0,QString &callsign)
{
    //character callsign*12
    //include 'pfx.f90'
    //character addpfx*8
    //common/pfxcom/addpfx
    //int iz;
    int k=k0;//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.

    if (k>450) k=k-450;//if(k.gt.450) k=k-450
    //qDebug()<<"K="<<k;
    if (k>=1 && k<=NZ) //if(k.ge.1 .and. k.le.NZ) then
    {
        //iz=index(pfx[k],' ') - 1;
        callsign=pfx[k-1]+"/"+callsign;//callsign=pfx(k)(1:iz)//'/'//callsign
        //qDebug()<<"PFX="<<pfx[k];
    }
    else if (k>=401 && k<=400+NZ2) //else if(k.ge.401 .and. k.le.400+NZ2) then
    {
        //iz=index(callsign,' ') - 1
        callsign=callsign+"/"+sfx[k-400-1];//callsign=callsign(1:iz)//'/'//sfx(k-400)
    }
    else if (k==449) //else if(k.eq.449) then
    {
        //iz=index(addpfx,' ') - 1
        //if(iz.lt.1) iz=8
        callsign=addpfx+"/"+callsign;//callsign=addpfx(1:iz)//'/'//callsign
    }
}

int PackUnpackMsg::CalcDistance(QString c_my_loc,QString c_test_loc)
{
    int dist_km = 0;
    double dlong1 = THvQthLoc.getLon(c_my_loc);
    double dlat1  = THvQthLoc.getLat(c_my_loc);
    double dlong2 = THvQthLoc.getLon(c_test_loc);
    double dlat2 = THvQthLoc.getLat(c_test_loc);

    dist_km = THvQthLoc.getDistanceKilometres(dlong1,dlat1,dlong2,dlat2);

    return dist_km;
}
QString PackUnpackMsg::unpackmsg144(int *dat,char &ident,QString &c1,QString &c2,bool rpt_db_msk)//false rpt_db_msk
{
    QString msg;
    int nc1,nc2,ng;//,i;
    //QString c1,c2,psfx,junk2,grid,grid6;
    QString psfx,junk2,grid,grid6;
    int iv2,junk1;
    bool cqnnn=false;
    int NBASE=37*36*10*27*27*27;
    int k =0;
    //int j = 0;
    //QString ss="aA";
    //char aa = 'A';

    nc1=(dat[0]<<22) + (dat[1]<<16) + (dat[2]<<10)+(dat[3]<<4) + ((dat[4]>>2)&15);
    nc2=((dat[4]&3)<<26) + (dat[5]<<20) +(dat[6]<<14) + (dat[7]<<8) + (dat[8]<<2) +((dat[9]>>4)&3);
    ng=((dat[9]&15)<<12) + (dat[10]<<6) + dat[11];

    //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    if (ng>=32768)
    {
        msg = unpacktext(nc1,nc2,ng,msg);
        c1="";
        c2="";
        ident = '$';
        goto c100;
    }
    //nc1=262178563;

    unpackcall(nc1,c1,iv2,psfx);
    ident = '*';

    if (iv2==0)
        //! This is an "original JT65" message
    {
        if (nc1==NBASE+1) c1="CQ";
        if (nc1==NBASE+2) c1="QRZ";
        int nfreq=nc1-NBASE-3;
        if (nfreq>=0 && nfreq<=999)
        {
            //write(c1,1002) nfreq
            //1002    format('CQ ',i3.3)
            //
            c1=("CQ "+QString("%1").arg(nfreq));
            cqnnn=true;
        }
    }
    unpackcall(nc2,c2,junk1,junk2);
    unpackgrid(ng,grid,rpt_db_msk);//false rpt_db_msk
    //qDebug()<<"psfx="<<iv2<<psfx<<grid;
    if (iv2>0) //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    {
        //! This is a JT65v2 message
        /*for (i = 0; i < 4; i++)
        {//do i=1,4
            if (((int)psfx[i].toLatin1())==0) psfx[i]=' ';
        }*/

        //n1=len_trim(psfx)
        //n2=len_trim(c2)
        if (iv2==1) msg="CQ "+psfx+"/"+c2+" "+grid;   //psfx(:n1)//'/'//c2(:n2)//' '//grid
        if (iv2==2) msg="QRZ "+psfx+"/"+c2+" "+grid;  //psfx(:n1)//'/'//c2(:n2)//' '//grid
        if (iv2==3) msg="DE "+psfx+"/"+c2+" "+grid;   //psfx(:n1)//'/'//c2(:n2)//' '//grid
        if (iv2==4) msg="CQ "+c2+"/"+psfx+" "+grid;   //c2(:n2)//'/'//psfx(:n1)//' '//grid
        if (iv2==5) msg="QRZ "+c2+"/"+psfx+" "+grid;  //c2(:n2)//'/'//psfx(:n1)//' '//grid
        if (iv2==6) msg="DE "+c2+"/"+psfx+" "+grid;   //c2(:n2)//'/'//psfx(:n1)//' '//grid
        if (iv2==7) msg="DE "+c2+" "+grid;            //c2(:n2)//' '//grid
        if (iv2==8) msg=" ";
        goto c100;
        //else
    }
    //qDebug()<<"DDD="<<c1<<c2<<grid<<psfx;//<<junk2;

    grid6=grid+"ma";
    //grid6="IR94ma";   //ima->"IR95ma"; niama->"IR94ma";
    //qDebug()<<"Grid K_in="<<grid<<k;
    grid2k(grid6,k);
    //qDebug()<<"K_out="<<k;

    if (k>=1 && k<=450)   getpfx2(k,c1);//if(k.ge.1 .and. k.le.450)   call getpfx2(k,c1)
    if (k>=451 && k<=900) getpfx2(k,c2);//if(k.ge.451 .and. k.le.900) call getpfx2(k,c2)

    //qDebug()<<"addpfx1="<<addpfx1;
    //qDebug()<<"C1="<<c1<<"C2="<<c2;
    //k=0;

    //i=index(c1,char(0))
    //if(i>=3) c1=c1(1:i-1)//'            '
    //i=index(c2,char(0))
    //if(i>=3) c2=c2(1:i-1)//'            '

    //msg="                      ";
    //j=-1;
    if (cqnnn)
    {
        //msg=c1+"          ";
        //j=7;
        msg.append(c1);
        msg.append(" ");
        goto c10;
    }

    /*for (i = 0; i < 12; i++)
    {//do i=1,12
        j++;//j=j+1
        msg[j]=c1[i];
        if (c1[i]==' ') goto c10;
    }
    j++;//j=j+1
    msg[j]=' ';*/
    msg.append(c1);
    msg.append(" ");
c10:
    /*for (i = 0; i < 12; i++) //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    {//do i=1,12
        if (j<=21) j++;
        msg[j]=c2[i];
        if (c2[i]==' ') goto c20;
    }
    if (j<=21) j++;//j=j+1
    msg[j]=' ';   */

    msg.append(c2);
    if (grid!="    ")
        msg.append(" ");
//c20:
    if (k==0)
    {
        /*for (i = 0; i < 4; i++)
        {//do i=1,4
            if (j<=21) j++;
            msg[j]=grid[i];//qDebug()<<"DDD1="<<c1<<c2<<grid[i]<<psfx<<"MSG="<<j;//<<junk2;
        }
        if (j<=21) j++;
        msg[j]=' ';*/
        if (grid!="    ")
            msg.append(grid);
    }
c100: //continue
    if (qstr_beg_end(msg,0,6)=="CQ9DX ") msg[2]=' ';
    //qDebug()<<"DDD2="<<c1<<c2<<grid<<psfx<<"MSG="<<msg<<k;//<<junk2;
    /* v1.27
    if(msg(1:2).eq.'E9' .and.                                          &
    msg(3:3).ge.'A' .and. msg(3:3).le.'Z' .and.                   &
    msg(4:4).ge.'A' .and. msg(4:4).le.'Z' .and.                   &
    msg(5:5).eq.' ') msg='CQ '//msg(3:)*/
    if (qstr_beg_end(msg,0,2)=="E9" &&
            msg[2]>='A' && msg[2]<='Z' &&
            msg[3]>='A' && msg[3]<='Z' &&
            msg[4]==' ') msg="CQ "+qstr_beg_end(msg,2,msg.count());

    msg = RemBegEndWSpaces(msg);

    return msg;
}
QString PackUnpackMsg::unpackmsg(int *dat,char &ident,bool rpt_db_msk)//false rpt_db_msk
{
    QString msg;
    int nc1,nc2,ng;//,i;
    QString c1,c2,psfx,junk2,grid,grid6;
    int iv2,junk1;
    bool cqnnn=false;
    int NBASE=37*36*10*27*27*27;
    int k =0;
    //int j = 0;
    //QString ss="aA";
    //char aa = 'A';

    nc1=(dat[0]<<22) + (dat[1]<<16) + (dat[2]<<10)+(dat[3]<<4) + ((dat[4]>>2)&15);
    nc2=((dat[4]&3)<<26) + (dat[5]<<20) +(dat[6]<<14) + (dat[7]<<8) + (dat[8]<<2) +((dat[9]>>4)&3);
    ng=((dat[9]&15)<<12) + (dat[10]<<6) + dat[11];

    //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    if (ng>=32768)
    {
        msg = unpacktext(nc1,nc2,ng,msg);
        ident = '$';
        goto c100;
    }
    //nc1=262178563;

    unpackcall(nc1,c1,iv2,psfx);
    ident = '*';

    if (iv2==0)
        //! This is an "original JT65" message
    {
        if (nc1==NBASE+1) c1="CQ";
        if (nc1==NBASE+2) c1="QRZ";
        int nfreq=nc1-NBASE-3;
        if (nfreq>=0 && nfreq<=999)
        {
            //write(c1,1002) nfreq
            //1002    format('CQ ',i3.3)
            //
            c1=("CQ "+QString("%1").arg(nfreq));
            cqnnn=true;
        }
    }
    unpackcall(nc2,c2,junk1,junk2);
    unpackgrid(ng,grid,rpt_db_msk);//false rpt_db_msk
    //qDebug()<<"psfx="<<iv2<<psfx<<grid;
    if (iv2>0) //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    {
        //! This is a JT65v2 message
        /*for (i = 0; i < 4; i++)
        {//do i=1,4
            if (((int)psfx[i].toLatin1())==0) psfx[i]=' ';
        }*/

        //n1=len_trim(psfx)
        //n2=len_trim(c2)
        if (iv2==1) msg="CQ "+psfx+"/"+c2+" "+grid;   //psfx(:n1)//'/'//c2(:n2)//' '//grid
        if (iv2==2) msg="QRZ "+psfx+"/"+c2+" "+grid;  //psfx(:n1)//'/'//c2(:n2)//' '//grid
        if (iv2==3) msg="DE "+psfx+"/"+c2+" "+grid;   //psfx(:n1)//'/'//c2(:n2)//' '//grid
        if (iv2==4) msg="CQ "+c2+"/"+psfx+" "+grid;   //c2(:n2)//'/'//psfx(:n1)//' '//grid
        if (iv2==5) msg="QRZ "+c2+"/"+psfx+" "+grid;  //c2(:n2)//'/'//psfx(:n1)//' '//grid
        if (iv2==6) msg="DE "+c2+"/"+psfx+" "+grid;   //c2(:n2)//'/'//psfx(:n1)//' '//grid
        if (iv2==7) msg="DE "+c2+" "+grid;            //c2(:n2)//' '//grid
        if (iv2==8) msg=" ";
        goto c100;
        //else
    }
    //qDebug()<<"DDD="<<c1<<c2<<grid<<psfx;//<<junk2;

    grid6=grid+"ma";
    //grid6="IR94ma";   //ima->"IR95ma"; niama->"IR94ma";
    //qDebug()<<"Grid K_in="<<grid<<k;
    grid2k(grid6,k);
    //qDebug()<<"K_out="<<k;

    if (k>=1 && k<=450)   getpfx2(k,c1);//if(k.ge.1 .and. k.le.450)   call getpfx2(k,c1)
    if (k>=451 && k<=900) getpfx2(k,c2);//if(k.ge.451 .and. k.le.900) call getpfx2(k,c2)

    //qDebug()<<"addpfx1="<<addpfx1;
    //qDebug()<<"C1="<<c1<<"C2="<<c2;
    //k=0;

    //i=index(c1,char(0))
    //if(i>=3) c1=c1(1:i-1)//'            '
    //i=index(c2,char(0))
    //if(i>=3) c2=c2(1:i-1)//'            '

    //msg="                      ";
    //j=-1;
    if (cqnnn)
    {
        //msg=c1+"          ";
        //j=7;
        msg.append(c1);
        msg.append(" ");
        goto c10;
    }

    /*for (i = 0; i < 12; i++)
    {//do i=1,12
        j++;//j=j+1
        msg[j]=c1[i];
        if (c1[i]==' ') goto c10;
    }
    j++;//j=j+1
    msg[j]=' ';*/
    msg.append(c1);
    msg.append(" ");
c10:
    /*for (i = 0; i < 12; i++) //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    {//do i=1,12
        if (j<=21) j++;
        msg[j]=c2[i];
        if (c2[i]==' ') goto c20;
    }
    if (j<=21) j++;//j=j+1
    msg[j]=' ';   */

    msg.append(c2);
    if (grid!="    ")
        msg.append(" ");
//c20:
    if (k==0)
    {
        /*for (i = 0; i < 4; i++)
        {//do i=1,4
            if (j<=21) j++;
            msg[j]=grid[i];//qDebug()<<"DDD1="<<c1<<c2<<grid[i]<<psfx<<"MSG="<<j;//<<junk2;
        }
        if (j<=21) j++;
        msg[j]=' ';*/
        if (grid!="    ")
            msg.append(grid);
    }
c100: //continue
    if (qstr_beg_end(msg,0,6)=="CQ9DX ") msg[2]=' ';
    //qDebug()<<"DDD2="<<c1<<c2<<grid<<psfx<<"MSG="<<msg<<k;//<<junk2;
    /* v1.27
    if(msg(1:2).eq.'E9' .and.                                          &
    msg(3:3).ge.'A' .and. msg(3:3).le.'Z' .and.                   &
    msg(4:4).ge.'A' .and. msg(4:4).le.'Z' .and.                   &
    msg(5:5).eq.' ') msg='CQ '//msg(3:)*/
    if (qstr_beg_end(msg,0,2)=="E9" &&
            msg[2]>='A' && msg[2]<='Z' &&
            msg[3]>='A' && msg[3]<='Z' &&
            msg[4]==' ') msg="CQ "+qstr_beg_end(msg,2,msg.count());
       
        
    //if(msg(1:5).eq.'CQ 00' .and. msg(6:6).ge.'0' .and.                 &
    //msg(6:6).le.'9') msg='CQ '//msg(6:)
    //and CQ 0..9 HV  
    if(qstr_beg_end(msg,0,5)=="CQ 00" && msg[5]>='0' &&  msg[5]<='9') msg="CQ "+qstr_beg_end(msg,5,msg.count());

    msg = RemBegEndWSpaces(msg);

    return msg;
}
