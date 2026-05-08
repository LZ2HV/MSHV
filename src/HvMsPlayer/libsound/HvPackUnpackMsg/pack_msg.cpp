/* The algorithms, source code, look-and-feel of WSJT-X and related
 * programs, and protocol specifications for the modes FSK441, FT8, JT4,
 * JT6M, JT9, JT65, JTMS, QRA64, ISCAT, MSK144, are Copyright © 2001-2017
 * by one or more of the following authors: Joseph Taylor, K1JT; Bill
 * Somerville, G4WJS; Steven Franke, K9AN; Nico Palermo, IV3NWV; Greg Beam,
 * KI7MT; Michael Black, W9MDB; Edson Pereira, PY2SDR; Philip Karn, KA9Q;
 * and other members of the WSJT Development Group.
 *
 * MSHV PackMessage
 * Rewritten into C++ and modified by Hrisimir Hristov, LZ2HV 2015-2017
 * May be used under the terms of the GNU General Public License (GPL)
 */

#include "pack_unpack_msg.h"
#include "../../../pfx_sfx.h"

//#include <QtGui>
void PackUnpackMsg::copy_qstr(QString &to,QString from,int b_from,int e_from)
{
    for (int i = 0; i < e_from-b_from; i++)
        to[i]=from[i+b_from];
    for (int i = e_from-b_from; i < to.count(); i++)
        to[i]=' ';
}
int PackUnpackMsg::max_2int(int a,int b)
{
    int res = a;
    if (b>res)
        res=b;
    return res;
}

QString PackUnpackMsg::qstr_beg_end(QString s_in,int b,int e)
{
    QString str;
    str=s_in.mid(b,e-b);
    return str;
}
int PackUnpackMsg::index(QString in,char search)
{
    int res = -1; // if not search return -1

    for (int i = 0; i < in.count(); i++)
    {
        if (in[i]==search)
        {
            res = i;
            break;
        }
    }

    return res;
}
int PackUnpackMsg::nchar(char c)
{
//! Convert ascii number, letter, or space to 0-36 for callsign packing.
    int n=0;
    // n=0                                    //!Silence compiler warning
    //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    if (c>='0' && c<='9')
        n=(int)c-(int)'0';
    else if ((int)c>=(int)'A' && (int)c<=(int)'Z')
        n=(int)c-(int)'A' + 10;
    else if ((int)c>=(int)'a' && (int)c<=(int)'z')
        n=(int)c-(int)'a' + 10;
    else if ((int)c>=(int)' ')
        n=36;
    else
    {
        //printf ("%s \n", "Invalid character in callsign");
        //qDebug()<<"Invalid character in callsign"<<c<<(int)c;
    }
    //Print*,'Invalid character in callsign ',c,' ',ichar(c)
    //stop
    //endif
    //printf ("%s \n", "Invalid character in callsign");
    //qDebug()<<"Invalid character in callsign"<<c<<(int)c;
    return n;
}
void PackUnpackMsg::packcall(QString callsign,int &ncall,bool &text)
{
    //! Pack a valid callsign into a 28-bit integer.
    int NBASE=37*36*10*27*27*27;
    //character callsign*6,c*1,tmp*6
    QString tmp="                    ";//20char
    //char c;
    text = false;

    //qDebug()<<"CQ"<<callsign;
    //s.replace(QString("9"), QString("n"));
    //! Work-around for Swaziland prefix:
    if (qstr_beg_end(callsign,0,4)=="3DA0") callsign.replace("3DA0","3D0");//callsign(5:6)
    // ! Work-around for Guinea prefixes:
    if (qstr_beg_end(callsign,0,2)=="3X") callsign.replace("3X","Q");
    //if(callsign(1:2).eq.'3X') callsign='Q'//callsign(3:6)

    //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    if (qstr_beg_end(callsign,0,3)=="CQ ")
    {
        // qDebug()<<"CQ"<<callsign;
        ncall=NBASE + 1;
        if (callsign[3]>=(int)'0' && callsign[3]<=(int)'9' && callsign[4]>=(int)'0' && callsign[4]<=(int)'9' &&
                callsign[5]>=(int)'0' && callsign[5]<=(int)'9')
        {
            int nfreq = qstr_beg_end(callsign,3,6).toInt();//read(callsign(4:6),*) nfreq;
            ncall=NBASE + 3 + nfreq;
        }
        return;
    }
    else if (qstr_beg_end(callsign,0,4)=="QRZ ")
    {
        ncall=NBASE + 2;
        return;
    }
    else if (qstr_beg_end(callsign,0,3)=="DE ")
    {
        ncall=267796945;
        return;
    }

    //tmp="      ";
    if (callsign[2]>=(int)'0' && callsign[2]<=(int)'9')
        copy_qstr(tmp,callsign,0,callsign.count());//tmp=callsign;
    else if (callsign[1]>=(int)'0' && callsign[1]<=(int)'9')
    {
        if (callsign[5]!=' ')
        {
            text=true;
            return;
        }
        copy_qstr(tmp,callsign,0,5);//qDebug()<<"tmp"<<tmp;//tmp=" "+qstr_beg_end(callsign,0,5);//tmp=' '//callsign(:5)
        tmp.prepend(' ');//qDebug()<<"tmp"<<tmp;
    }
    else
    {
        text=true;
        return;
    }

    for (int i = 0; i < 6; i++)
    {//do i=1,6
        char c=tmp[i].toLatin1();
        if ((int)c>=(int)'a' && (int)c<=(int)'z') tmp[i]=char((int)c-(int)'a'+(int)'A');
    }
    int n1=0;
    if ((tmp[0]>=(int)'A' && tmp[0]<=(int)'Z') || tmp[0]==' ') n1=1;
    if (tmp[0]>=(int)'0' && tmp[0]<=(int)'9') n1=1;
    int n2=0;
    if (tmp[1]>='A' && tmp[1]<=(int)'Z') n2=1;
    if (tmp[1]>=(int)'0' && tmp[1]<=(int)'9') n2=1;
    int n3=0;
    if (tmp[2]>=(int)'0' && tmp[2]<=(int)'9') n3=1;
    int n4=0;
    if ((tmp[3]>=(int)'A' && tmp[3]<=(int)'Z') || tmp[3]==' ') n4=1;
    int n5=0;
    if ((tmp[4]>=(int)'A' && tmp[4]<=(int)'Z') || tmp[4]==' ') n5=1;
    int n6=0;
    if ((tmp[5]>=(int)'A' && tmp[5]<=(int)'Z') || tmp[5]==' ') n6=1;

    if (n1+n2+n3+n4+n5+n6 != 6)
    {
        text=true;
        return;
    }
    ncall=nchar(tmp[0].toLatin1());
    ncall=36*ncall+nchar(tmp[1].toLatin1());
    ncall=10*ncall+nchar(tmp[2].toLatin1());
    ncall=27*ncall+nchar(tmp[3].toLatin1())-10;
    ncall=27*ncall+nchar(tmp[4].toLatin1())-10;
    ncall=27*ncall+nchar(tmp[5].toLatin1())-10;
}

void PackUnpackMsg::getpfx1(QString &callsign,int &k,int &nv2)
{
    QString callsign0;
    QString c;
    QString lof;
    QString rof;
    QString tpfx="                    ";//20char
    QString tsfx="                    ";//20char
    bool ispfx=false;
    bool issfx=false;
    bool invalid = true;

    //callsign0=qstr_beg_end(callsign,0,callsign.count());
    nv2=1;
    int iz=index(callsign,' ') - 1;//iz=index(callsign,' ') - 1
    callsign0=qstr_beg_end(callsign,0,iz+1);

    if (iz<0) iz=12; //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.

    int islash=index((qstr_beg_end(callsign,0,iz+1)),'/');//islash=index(callsign(1:iz),'/') may ako ne go nameri vra6ta -1
    k=0;
    //!  if(k.eq.0) go to 10     !Tnx to DL9RDZ for reminder:this was for tests only!
    //c="   ";
    //qDebug()<<"prefix"<<qstr_beg_end(callsign,0,islash-0)<<islash<<iz-4<<(iz-1);//c=callsign(1:islash-1)
    //qDebug()<<"sufix"<<qstr_beg_end(callsign,islash+1,iz+1);
    if (islash>-1 && islash<=(iz-4))//if(islash.gt.0 .and. islash.le.(iz-4)) then
    {
        //! Add-on prefix
        c=qstr_beg_end(callsign,0,islash-0);//c=callsign(1:islash-1)
        copy_qstr(callsign,callsign,islash+1,iz+1);//callsign=callsign(islash+1:iz)
        //qDebug()<<"PFX"<<c<<callsign;
        for (int i = 0; i < NZ; i++)
        {//do i=1,NZ
            if (pfx[i]==c)//pfx(i)(1:4)==c
            {
                k=i+1;
                nv2=2;
                goto c10;
            }
        }
        if (addpfx==c)
        {
            k=449;//448
            nv2=2;
            goto c10;
        }
    }
    else if (islash==(iz-1))//if(islash.eq.(iz-1)) then
    {
        //! Add-on suffix
        c=qstr_beg_end(callsign,islash+1,iz+1);//c=callsign(islash+1:iz)
        copy_qstr(callsign,callsign,0,islash-0);//callsign=callsign(1:islash-1)
        //qDebug()<<"SFX"<<c<<callsign;
        for (int i = 0; i < NZ2; i++)
        {//do i=1,NZ2
            if (sfx[i]==c[0])//if(sfx(i).eq.c(1:1)) then
            {
                k=400+i+1;
                nv2=3;
                goto c10;
            }
        }
    }
c10:
    if (islash!=-1 && k==0) //if(islash.ne.0 .and.k.eq.0)
    {
        //! Original JT65 would force this compound callsign to be treated as
        //! plain text.  In JT65v2, we will encode the prefix or suffix into nc1.
        //! The task here is to compute the proper value of k.
        lof=qstr_beg_end(callsign0,0,islash-0);//lof=callsign0(:islash-1)
        rof=qstr_beg_end(callsign0,islash+1,callsign0.count());//rof=callsign0(islash+1:)
        //qDebug()<<"LOV ROT="<<lof<<rof;
        //llof=len_trim(lof)//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
        //lrof=len_trim(rof)
        //qDebug()<<"IS PFX SFX="<<ispfx<<issfx;
        if (lof.count()>0 && lof.count()<=4) ispfx=true;//ispfx=(llof.gt.0 .and. llof.le.4)
        if (rof.count()>0 && rof.count()<=3) issfx=true;////issfx=(lrof.gt.0 .and. lrof.le.3)
        //qDebug()<<"IS PFX SFX="<<ispfx<<issfx;
        if (!ispfx || !issfx) invalid = false;//invalid=.not.(ispfx.or.issfx)
        if (ispfx && issfx) //if(ispfx.and.issfx)
        {
            if (lof.count()<3) issfx=false;//if(llof.lt.3) issfx=.false.
            if (rof.count()<3) ispfx=false;//if(lrof.lt.3) ispfx=.false.
            if (ispfx && issfx) //if(ispfx.and.issfx) then
            {
                int i=(int)callsign0[islash-1].toLatin1();//i=ichar(callsign0(islash-1:islash-1))
                if (i>=(int)'0' && i<=(int)'9') //if(i.ge.ichar('0') .and. i.le.ichar('9'))
                    issfx=false;
                else
                    ispfx=false;
            }
        }
        if (invalid)
            k=-1;
        else
        {
            //qDebug()<<"LOV ROT="<<ispfx<<issfx;
            if (ispfx)
            {
                lof.append("                ");
                copy_qstr(tpfx,lof,0,4);//tpfx=lof(1:4)
                k=nchar(tpfx[0].toLatin1());
                k=37*k + nchar(tpfx[1].toLatin1());
                k=37*k + nchar(tpfx[2].toLatin1());
                k=37*k + nchar(tpfx[3].toLatin1());
                nv2=4;
                int i=index(callsign0,'/');
                copy_qstr(callsign,callsign0,0,i-0);//callsign=callsign0(:i-1);
                copy_qstr(callsign,callsign0,i+1,callsign0.count());//callsign=callsign0(i+1:);
                //qDebug()<<"TPFX="<<tpfx;
            }
            if (issfx)
            {
                rof.append("                ");
                copy_qstr(tsfx,rof,0,3);//tsfx=rof(1:3)
                k=nchar(tsfx[0].toLatin1());
                k=37*k + nchar(tsfx[1].toLatin1());
                k=37*k + nchar(tsfx[2].toLatin1());
                nv2=5;
                int i=index(callsign0,'/');
                copy_qstr(callsign,callsign0,0,i-0);//callsign=callsign0(:i-1);
                //qDebug()<<"TSFX="<<tsfx;
            }
        }
    }


}

void PackUnpackMsg::packtext(QString msg,int &nc1,int &nc2,int &nc3)
{
    /*parameter (MASK28=2**28 - 1)
    character*13 msg
    character*42 c
    data c/'0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ +-./?'/
    cc_JTMSK_TX*/

    nc1=0;
    nc2=0;
    nc3=0;
    int j=0;

    for (int i = 0; i < 5; i++)
    {//do i=1,5                              //!First 5 characters in nc1
        for (j = 0; j < 42; j++)//max=41=?
        {//do j=1,42                            //!Get character code
            if (msg[i]==C_PACK_UNPACK_JT[j]) goto c10;
        }
        j=36;//j=37;pouse
c10:
        //j=j-1;                                //!Codes should start at zero
        nc1=42*nc1 + j;
        //qDebug()<<"ppp="<<cc_JTMSK_TX[41];
    }
    for (int i = 5; i < 10; i++)
    {//do i=6,10
        for (j = 0; j < 42; j++) //max=41=?                            //!Characters 6-10 in nc2
        {//do j=1,42                            //!Get character code
            if (msg[i]==C_PACK_UNPACK_JT[j]) goto c20;
        }
        j=36;//j=37; pouse
c20:
        //j=j-1;                                //!Codes should start at zero
        nc2=42*nc2 + j;
    }

    for (int i = 10; i < 13; i++)
    {//do i=11,13                              //!Characters 11-13 in nc3
        for (j = 0; j < 42; j++)//max=41=?
        {//do j=1,42                            //!Get character code
            if (msg[i]==C_PACK_UNPACK_JT[j]) goto c30;
        }
        j=36;//j=37; pouse
c30:
        //j=j-1;                                //!Codes should start at zero
        nc3=42*nc3 + j;
    }

    //! We now have used 17 bits in nc3.  Must move one each to nc1 and nc2.
    //qDebug()<<nc1;
    nc1=nc1+nc1; //qDebug()<<nc1;
    if ((nc3 & 32768)!=0) nc1=nc1+1;
    nc2=nc2+nc2;
    if ((nc3 & 65536)!=0) nc2=nc2+1;
    nc3=(nc3 & 32767);
    //qDebug()<<nc1<<nc2<<nc3;
}
void PackUnpackMsg::deg2grid(double dlong0,double dlat,QString &grid)
{
    double dlong;                      //!West longitude (deg)
    //double dlat                         //!Latitude (deg)

    dlong=dlong0;//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    if (dlong<-180.0) dlong=dlong+360.0;
    if (dlong>180.0) dlong=dlong-360.0;

    //! Convert to units of 5 min of longitude, working east from 180 deg.
    int nlong=int(60.0*(180.0-dlong)/5.0);
    int n1=nlong/240;                      //!20-degree field
    int n2=(nlong-240*n1)/24;              //!2 degree square
    int n3=nlong-240*n1-24*n2;             //!5 minute subsquare
    grid[0]=char((int)('A')+n1);
    grid[2]=char((int)('0')+n2);
    grid[4]=char((int)('a')+n3);

    //! Convert to units of 2.5 min of latitude, working north from -90 deg.
    int nlat=int(60.0*(dlat+90)/2.5);
    n1=nlat/240;                       //!10-degree field
    n2=(nlat-240*n1)/24;               //!1 degree square
    n3=nlat-240*n1-24*n2;              //!2.5 minuts subsquare
    grid[1]=char((int)('A')+n1);
    grid[3]=char((int)('0')+n2);
    grid[5]=char((int)('a')+n3);
}
void PackUnpackMsg::k2grid(int k,QString &grid)
{//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    double nlong=2*fmod((k-1)/5,90)-179;
    if (k>450) nlong=nlong+180;
    double nlat=fmod(k-1,5)+ 85;
    double dlat=nlat;
    double dlong=nlong;
    deg2grid(dlong,dlat,grid);
}
void PackUnpackMsg::grid2deg(QString grid0,double &dlong,double &dlat)
{
    //! Converts Maidenhead grid locator to degrees of West longitude
    //! and North latitude.

    QString grid;
    char g1,g2,g3,g4,g5,g6;

    grid=grid0;
    int i=(int)grid[4].toLatin1();//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    if (grid[4]==' ' || i<=64 || i>=128) grid.replace(4,2,"mm"); //qDebug()<<"GRID="<<grid;

    /*if ((int)grid[0].toLatin1()>=(int)'a' && grid[0]<=(int)'z') grid[0]=char((int)((int)grid[0].toLatin1())+(int)('A')-(int)('a'));
    if ((int)grid[1].toLatin1()>=(int)'a' && grid[1]<=(int)'z') grid[1]=char((int)((int)grid[1].toLatin1())+(int)('A')-(int)('a'));
    if ((int)grid[4].toLatin1()>=(int)'A' && grid[4]<=(int)'Z') grid[4]=char((int)((int)grid[4].toLatin1())-(int)('A')+(int)('a'));
    if ((int)grid[5].toLatin1()>=(int)'A' && grid[5]<=(int)'Z') grid[5]=char((int)((int)grid[5].toLatin1())-(int)('A')+(int)('a'));*/
    if (grid[0]>=(int)'a' && grid[0]<=(int)'z') grid[0]=char((int)grid[0].toLatin1()+(int)'A'-(int)'a');
    if (grid[1]>=(int)'a' && grid[1]<=(int)'z') grid[1]=char((int)grid[1].toLatin1()+(int)'A'-(int)'a');
    if (grid[4]>=(int)'A' && grid[4]<=(int)'Z') grid[4]=char((int)grid[4].toLatin1()-(int)'A'+(int)'a');
    if (grid[5]>=(int)'A' && grid[5]<=(int)'Z') grid[5]=char((int)grid[5].toLatin1()-(int)'A'+(int)'a');

    g1=grid[0].toLatin1();
    g2=grid[1].toLatin1();
    g3=grid[2].toLatin1();
    g4=grid[3].toLatin1();
    g5=grid[4].toLatin1();
    g6=grid[5].toLatin1();
    //qDebug()<<"G="<<g1<<g2<<g3<<g4<<g5<<g6;

    double nlong = 180 - 20*((int)g1-(int)'A');
    double n20d = 2*((int)g3-(int)'0');
    double xminlong = 5*((int)g5-(int)'a'+0.5);
    dlong = nlong - n20d - xminlong/60.0;
    double nlat = -90+10*((int)g2-(int)'A') + (int)g4-(int)'0';
    double xminlat = 2.5*((int)g6-(int)'a'+0.5);
    dlat = nlat + xminlat/60.0;
}
void PackUnpackMsg::packgrid(QString grid,int &ng,bool &text,bool msk144ms)
{
    int NGBASE=180*180;
    char c1;
    int n=0;
    text=false;
    double dlong = 0.0;
    double dlat = 0.0;
    int long1=0;
    int lat=0;

    //if(grid.isEmpty()) goto c90;//if(grid=='    ') goto c90;               //!Blank grid is OK
    if (qstr_beg_end(grid,0,4)=="    ") goto c90;

    //! First, handle signal reports in the original range, -01 to -30 dB
    //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    if (grid[0]=='-')
    {
        //if (grid[1]>=(int)'0' && grid[1]<=(int)'9' && grid[2]>=(int)'0' && grid[2]<=(int)'9')
        if (!grid[1].isLetter() && !grid[2].isLetter())
            n =qstr_beg_end(grid,1,3).toInt();//read(grid(2:3),*,err=800,end=800) n
        else
            goto c800;
        if (n>=1 && n<=30)
        {
            ng=NGBASE+1+n;
            goto c900;
        }
        goto c10;
    }
    else if (qstr_beg_end(grid,0,2)=="R-")
    {
        //grid[2].isDigit()
        //if (grid[2]>=(int)'0' && grid[2]<=(int)'9' && grid[3]>=(int)'0' && grid[3]<=(int)'9')
        if (!grid[2].isLetter() && !grid[3].isLetter())
            n =qstr_beg_end(grid,2,4).toInt();//read(grid(3:4),*,err=800,end=800) n
        else
            goto c800;
        if (n>=1 && n<=30)
        {
            ng=NGBASE+31+n;
            goto c900;
        }
        goto c10;
    }
    //! Now check for RO, RRR, or 73 in the message field normally used for grid
    else if (qstr_beg_end(grid,0,4)=="RO  ")
    {
        ng=NGBASE+62;
        goto c900;
    }
    else if (qstr_beg_end(grid,0,4)=="RRR ")
    {
        ng=NGBASE+63;
        goto c900;
    }
    else if (qstr_beg_end(grid,0,4)=="73  ")
    {
        ng=NGBASE+64;
        goto c900;
    }

    //! Now check for extended-range signal reports: -50 to -31, and 0 to +49.
c10:
    n=99;
    c1=grid[0].toLatin1();
    //qDebug()<<"GRID="<<grid<<grid.toInt();

    if (c1=='R' && !grid[1].isLetter() && !grid[2].isLetter() && !grid[3].isLetter())//+ ne naru6awa rezultata
    {
        n =qstr_beg_end(grid,1,4).toInt();
        goto c30;
    }
    else if (!grid[0].isLetter() && !grid[1].isLetter() && !grid[2].isLetter() && !grid[3].isLetter())
    {
        n=grid.toInt();
    }

    /*if (grid[0].isDigit()&&grid[1].isDigit()&&grid[2].isDigit()&&grid[3].isDigit())
        n=grid.toInt();//read(grid,*,err=20,end=20) n
    else
        goto c20;

    goto c30;
    c20:
    if (grid[1].isDigit()&&grid[2].isDigit()&&grid[3].isDigit())
        n =qstr_beg_end(grid,1,4).toInt();//read(grid(2:4),*,err=30,end=30) n
    else
        goto c30;*/
c30:
    
    if (msk144ms)
    {
        if (n>=-30 && n<=69)//for msk144ms
        {
            if (c1=='R')
            {
                grid="LA"+QString("%1").arg(n+30);
                //grid="LA"+QString("%1").arg(n+50,2,10,QChar('0'));
            }
            else
            {
                grid="KA"+QString("%1").arg(n+30);
                //grid="KA"+QString("%1").arg(n+50,2,10,QChar('0'));
            }
            goto c40;
        }
    }
    else //old variant
    {
        if (n>=-50 && n<=49)//old variant
        {
            if (c1=='R')
            {
                grid="LA"+QString("%1").arg(n+50);
                //grid="LA"+QString("%1").arg(n+50,2,10,QChar('0'));
            }
            else
            {
                grid="KA"+QString("%1").arg(n+50);
                //grid="KA"+QString("%1").arg(n+50,2,10,QChar('0'));
            }
            goto c40;
        }
    }

    //! Maybe it's free text ?//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    if (grid[0]<(int)'A' || grid[0]>(int)'R') text=true;
    if (grid[1]<(int)'A' || grid[1]>(int)'R') text=true;
    if (grid[2]<(int)'0' || grid[2]>(int)'9') text=true;
    if (grid[3]<(int)'0' || grid[3]>(int)'9') text=true;
    if (text) goto c900;

    //! OK, we have a properly formatted grid locator

c40:
    grid2deg(QString(grid+"mm"),dlong,dlat);                      //grid2deg(grid//'mm',dlong,dlat)
    long1=int(dlong);
    lat=int(dlat+ 90.0);
    ng=((long1+180)/2)*180 + lat;
    goto c900;

c90:
    ng=NGBASE + 1;
    goto c900;

c800:
    text=true;
c900:
    return;
}
void PackUnpackMsg::packmsg(char *msg0,int *dat,int &itype,bool msk144ms)
{
    /*! Packs a JT4/JT9/JT65 message into twelve 6-bit symbols

         ! itype Message Type
         !--------------------
         !   1   Standardd message
         !   2   Type 1 prefix
         !   3   Type 1 suffix
         !   4   Type 2 prefix
         !   5   Type 2 suffix
         !   6   Free text
         !  -1   Does not decode correctly*/

    //parameter (NBASE=37*36*10*27*27*27)
    //parameter (NBASE2=262178562)
    //character*22 msg0,msg
    QString msg;
    //char cmsg[50];
    //integer dat(12)
    //character*12 c1,c2
    QString c1="                    ";//20char
    QString c2="                    ";//20char
    QString c3="                    ";//20char
    QString grid6="                    ";//20char
    //character*4 c3
    //character*6 grid6
    //logical text1,text2,text3
    int ia;
    int ib;
    int ic;
    int k1=0;
    int k2=0;
    int nv2a = 0;
    int nv2b = 0;
    int nc1=0;
    int nc2=0;
    bool text1=true;
    bool text2=true;
    bool text3=true;
    int ng =0;


    int count = strlen(msg0);//qDebug()<<"count="<<count;
    for (int j = 0; j<count; j++)
        msg.append(msg0[j]);

    itype=1; //v2.12 in ima smisal   msk itype=0 nqma smisal 
    //call fmtmsg(msg,iz)
    
    //if(msg(1:3).eq.'CQ ' .and. msg(4:4).ge.'0' .and. msg(4:4).le.'9'   &
        //.and. msg(5:5).eq.' ') msg='CQ 00'//msg(4:)
    //and CQ 0..9 HV 
    if(qstr_beg_end(msg,0,3) =="CQ " && msg[3]>='0' && msg[3]<='9' && msg[4]==' ') msg="CQ 00"+qstr_beg_end(msg,3,msg.count());

    if (qstr_beg_end(msg,0,6) == "CQ DX ") msg[2]='9';//if(msg(1:6).eq.'CQ DX ') msg(3:3)='9'
    //qDebug()<<"new="<<msg;

    /*v1.27 //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    if(msg(1:3).eq."CQ " .and.                                         &
         msg(4:4).ge.'A' .and. msg(4:4).le.'Z' .and.                   &
         msg(5:5).ge.'A' .and. msg(5:5).le.'Z' .and.                   &
         msg(6:6).eq.' ') msg='E9'//msg(4:)*/
    if (qstr_beg_end(msg,0,3)=="CQ " &&
            msg[3]>='A' && msg[3]<='Z' &&
            msg[4]>='A' && msg[4]<='Z' &&
            msg[5]==' ') msg="E9"+qstr_beg_end(msg,3,msg.count());

    //qDebug()<<"new="<<msg<<msg[3]<<(int)'W';


    //! See if it's a CQ message
    int i = 0;
    if (qstr_beg_end(msg,0,3) == "CQ ")
    {/*if(msg(1:3).eq.'CQ ') then
                                                                                                                                                                                                                 go to 1 endif*/
        i=2; //mai 2 //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
        //! ... and if so, does it have a reply frequency?
        if (msg[3]>='0' && msg[3]<='9' && msg[4]>='0' && msg[4]<='9' && msg[5]>='0' && msg[5]<='9')
            i=6;//mai 6
        goto cc1;
    }


    for (i = 0; i < count; i++)
    {//do i=1,22
        if (msg[i]==' ') goto cc1;      //!Get 1st blank
    }
    goto c10;                             //!Consider msg as plain text

cc1:
    ia=i;


    copy_qstr(c1,msg,0,ia);//c1=qstr_beg_end(msg,0,ia);//c1=msg(1:ia-1)

    for (i = ia+1; i < count; i++)
    {//do i=ia+1,22
        if (msg[i]==' ') goto cc2;      //!Get 2nd blank
    }
    goto c10;                             //!Consider msg as plain text

cc2:
    ib=i;

    copy_qstr(c2,msg,ia+1,ib);//c2=qstr_beg_end(msg,ia+1,ib);//c2=msg(ia+1:ib-1)

    for (i = ib+1; i < count; i++)
    {//do i=ib+1,22
        if (msg[i]==' ') goto cc3;      //!Get 3rd blank
    }
    goto c10;                             //!Consider msg as plain text

cc3:
    ic=i; //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.

    //c3='    '
    if (ic>=ib+1) copy_qstr(c3,msg,ib+1,ic);//c3=qstr_beg_end(msg,ib+1,ic); if(ic.ge.ib+1) c3=msg(ib+1:ic)
    if (qstr_beg_end(c3,0,4)=="OOO ") c3.replace("OOO ","    ");//if(c3.eq.'OOO ') c3='    ' !Strip out the OOO flag
    //qDebug()<<"c1c2c3="<<c1<<c2<<c3;
    getpfx1(c1,k1,nv2a);//za sega se wy6tat k1=0,nv2a=1
    //qDebug()<<"getpfx1="<<c1<<"K="<<k1<<"nv2a="<<nv2a;
    if (nv2a>=4) goto c10;
    packcall(c1,nc1,text1);
    //qDebug()<<"packcall_1="<<c1<<"nc1="<<nc1<<"text1="<<text1;
    if (text1) goto c10;
    getpfx1(c2,k2,nv2b);
    packcall(c2,nc2,text2);
    //qDebug()<<"packcall_2="<<c2<<"nc2="<<nc2<<"text2="<<text2;
    if (text2) goto c10; //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    if (nv2a==2 || nv2a==3 || nv2b==2 || nv2b==3)
    {
        if (k1<0 || k2<0 || k1*k2!=0) goto c10;
        if (k2>0) k2=k2+450;
        int k=max_2int(k1,k2);
        if (k>0)
        {
            //k=449;
            k2grid(k,grid6);
            copy_qstr(c3,grid6,0,4);//grid6(:4)
            //qDebug()<<"c3="<<c3;
        }
    }
    //qDebug()<<"c3="<<c3;
    packgrid(c3,ng,text3,msk144ms);

    if (nv2a<4 && nv2b<4 && (!text1) && (!text2) && (!text3)) goto c20;

    nc1=0;
    if (nv2b==4)
    {
        if (qstr_beg_end(c1,0,3)=="CQ ")  nc1=262178563 + k2;
        if (qstr_beg_end(c1,0,4)=="QRZ ") nc1=264002072 + k2;
        if (qstr_beg_end(c1,0,3)=="DE ")  nc1=265825581 + k2;
    }
    else if (nv2b==5)
    {
        if (qstr_beg_end(c1,0,3)=="CQ ")  nc1=267649090 + k2;
        if (qstr_beg_end(c1,0,4)=="QRZ ") nc1=267698375 + k2;
        if (qstr_beg_end(c1,0,3)=="DE ")  nc1=267747660 + k2;
    }
    if (nc1!=0) goto c20;
    //! The message will be treated as plain text.
c10:
    itype=6;
    packtext(msg,nc1,nc2,ng);
    ng=ng+32768;
    //qDebug()<<msg<<nc1<<nc2<<ng;

    //! Encode data into 6-bit words
c20:
    //continue;
    //qDebug()<<"nv2a="<<nv2a<<"nv2a="<<nv2b<<"text1="<<text1<<"text2="<<text2<<"text3="<<text3;
    //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    //sent(k)=iand(1,ishft(i-1,-n)) -> sent[k]=1 & (i >> n)
    //Left Shift 	ISHFT 	ISHFT(N,M) (M > 0) 	<< 	n<<m 	n shifted left by m bits
    //Right Shift 	ISHFT 	ISHFT(N,M) (M < 0) 	>> 	n>>m 	n shifted right by m bits
    if (itype!=6) itype=max_2int(nv2a,nv2b);//moze bi -> nv2b=1;//2hv
    dat[0]=((nc1 >> 22) & 63);   //dat(1)=iand(ishft(nc1,-22),63)                !6 bits //VNIMANIE -22  ????           //!6 bits
    dat[1]=((nc1 >> 16) & 63);    //!6 bits
    dat[2]=((nc1 >> 10) & 63);       //!6 bits
    dat[3]=((nc1 >> 4) & 63);           //!6 bits
    dat[4]=4*(nc1 & 15)+((nc2 >> 26) & 3);//qDebug()<<"nc1="<<nc1<<"dat="<<dat[0]<<dat[1]<<dat[2]<<dat[3]<<dat[4]; //!4+2 bits
    dat[5]=((nc2 >> 20) & 63);           //!6 bits
    dat[6]=((nc2 >> 14) & 63);           //!6 bits
    dat[7]=((nc2 >> 8) & 63);          //!6 bits
    dat[8]=((nc2 >> 2) & 63);         //!6 bits
    dat[9]=16*(nc2 & 3)+((ng >>12) & 15);  //!2+4 bits
    dat[10]=((ng >> 6) & 63);
    dat[11]=(ng & 63);//dat(12)=iand(ng,63)

}
