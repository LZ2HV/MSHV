/* The algorithms, source code, look-and-feel of WSJT-X and related
 * programs, and protocol specifications for the modes FSK441, FT8, JT4,
 * JT6M, JT9, JT65, JTMS, QRA64, ISCAT, MSK144, are Copyright © 2001-2017
 * by one or more of the following authors: Joseph Taylor, K1JT; Bill
 * Somerville, G4WJS; Steven Franke, K9AN; Nico Palermo, IV3NWV; Greg Beam,
 * KI7MT; Michael Black, W9MDB; Edson Pereira, PY2SDR; Philip Karn, KA9Q;
 * and other members of the WSJT Development Group.
 *
 * MSHV AstroDataW
 * Rewritten into C++ and modified by Hrisimir Hristov, LZ2HV 2015-2017
 * May be used under the terms of the GNU General Public License (GPL)
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define RADS 0.0174532925199433
#define DEGS 57.2957795130823
#define TPI 6.28318530717959
//#define PI 3.1415927

/* ratio of     earth radius to astronomical unit */
#define ER_OVER_AU 0.0000426352325194252
 
/* all prototypes here */

double getcoord(int coord);
void getargs(int argc, char *argv[], int *y, int *m, double *tz, double *glong, double *glat);
double range(double y);
double rangerad(double y);
double days(int y, int m, int dn, double hour);
double days_(int *y, int *m, int *dn, double *hour);
void moonpos(double, double *, double *, double *);
void sunpos(double , double *, double *, double *);
double moontransit(int y, int m, int d, double timezone, double glat, double glong, int *nt);
double atan22(double y, double x);
double epsilon(double d);
void equatorial(double d, double *lon, double *lat, double *r);
void ecliptic(double d, double *lon, double *lat, double *r);
double gst(double d);
void topo(double lst, double glat, double *alp, double *dec, double *r);
double alt(double glat, double ha, double dec);
void libration(double day, double lambda, double beta, double alpha, double *l, double *b, double *p);
void illumination(double day, double lra, double ldec, double dr, double sra, double sdec, double *pabl, double *ill);
int daysinmonth(int y, int m);
int isleap(int y);
//void tmoonsub_(double *day, double *glat, double *glong, double *moonalt,
//double *mrv, double *l, double *b, double *paxis);

/*
static const char
*usage = "  Usage: tmoon date[yyyymm] timz[+/-h.hh] long[+/-dddmm] lat[+/-ddmm]\n"
            "example: tmoon 200009 0 -00155 5230\n";
*/

/*
  getargs() gets the arguments from the command line, does some basic error
  checking, and converts arguments into numerical form. Arguments are passed
  back in pointers. Error messages print to stderr so re-direction of output
  to file won't leave users blind. Error checking prints list of all errors
  in a command line before quitting.
*/
void getargs(int /*argc*/, char *argv[], int *y, int *m, double *tz,
             double *glong, double *glat)
{

    int date, latitude, longitude;
    int mflag = 0, yflag = 0, longflag = 0, latflag = 0, tzflag = 0;
    int longminflag = 0, latminflag = 0, dflag = 0;

    /* if not right number of arguments, then print example command line */

    /*
    if (argc !=5) {
      fprintf(stderr, usage);
      exit(EXIT_FAILURE);
    }
    */

    date = atoi(argv[1]);
    *y = date / 100;
    *m = date - *y * 100;
    *tz = (double) atof(argv[2]);
    longitude = atoi(argv[3]);
    latitude = atoi(argv[4]);
    *glong = RADS * getcoord(longitude);
    *glat = RADS * getcoord(latitude);

    /* set a flag for each error found */

    if (*m > 12 || *m < 1) mflag = 1;
    if (*y > 2500) yflag = 1;
    if (date < 150001) dflag = 1;
    if (fabs((float) *glong) > 180 * RADS) longflag = 1;
    if (abs(longitude) % 100 > 59) longminflag = 1;
    if (fabs((float) *glat) > 90 * RADS) latflag = 1;
    if (abs(latitude) % 100 > 59) latminflag = 1;
    if (fabs((float) *tz) > 12) tzflag = 1;

    /* print all the errors found */

    if (dflag == 1)
    {
        fprintf(stderr, "date: dates must be in form yyyymm, gregorian, and later than 1500 AD\n");
    }
    if (yflag == 1)
    {
        fprintf(stderr, "date: too far in future - accurate from 1500 to 2500\n");
    }
    if (mflag == 1)
    {
        fprintf(stderr, "date: month must be in range 0 to 12, eg - August 2000 is entered as 200008\n");
    }
    if (tzflag == 1)
    {
        fprintf(stderr, "timz: must be in range +/- 12 hours, eg -6 for Chicago\n");
    }
    if (longflag == 1)
    {
        fprintf(stderr, "long: must be in range +/- 180 degrees\n");
    }
    if (longminflag == 1)
    {
        fprintf(stderr, "long: last two digits are arcmin - max 59\n");
    }
    if (latflag == 1)
    {
        fprintf(stderr, " lat: must be in range +/- 90 degrees\n");
    }
    if (latminflag == 1)
    {
        fprintf(stderr, " lat: last two digits are arcmin - max 59\n");
    }

    /* quits if one or more flags set */

    if (dflag + mflag + yflag + longflag + latflag + tzflag + longminflag + latminflag > 0)
    {
        exit(EXIT_FAILURE);
    }

}

/*
   returns coordinates in decimal degrees given the
   coord as a ddmm value stored in an integer.
*/
double getcoord(int coord)
{
    int west = 1;
    double glg, deg;
    if (coord < 0) west = -1;
    glg = fabs((double) coord/100);
    deg = floor(glg);
    glg = west* (deg + (glg - deg)*100 / 60);
    return(glg);
}

/*
  days() takes the year, month, day in the month and decimal hours
  in the day and returns the number of days since J2000.0.
  Assumes Gregorian calendar.
*/
double days(int y, int m, int d, double h)
{
    int a, b;
    double day;

    /*
      The lines below work from 1900 march to feb 2100
      a = 367 * y - 7 * (y + (m + 9) / 12) / 4 + 275 * m / 9 + d;
      day = (double)a - 730531.5 + hour / 24;
    */

    /*  These lines work for any Gregorian date since 0 AD */
    if (m ==1 || m==2)
    {
        m +=12;
        y -= 1;
    }
    a = y / 100;
    b = 2 - a + a/4;
    day = floor(365.25*(y + 4716)) + floor(30.6001*(m + 1))
          + d + b - 1524.5 - 2451545 + h/24;
    return(day);
}
double days_(int *y0, int *m0, int *d0, double *h0)
{
    return days(*y0,*m0,*d0,*h0);
}

/*
Returns 1 if y a leap year, and 0 otherwise, according
to the Gregorian calendar
*/
int isleap(int y)
{
    int a = 0;
    if (y % 4 == 0) a = 1;
    if (y % 100 == 0) a = 0;
    if (y % 400 == 0) a = 1;
    return(a);
}

/*
Given the year and the month, function returns the
number of days in the month. Valid for Gregorian
calendar.
*/
int daysinmonth(int y, int m)
{
    int b = 31;
    if (m == 2)
    {
        if (isleap(y) == 1) b= 29;
        else b = 28;
    }
    if (m == 4 || m == 6 || m == 9 || m == 11) b = 30;
    return(b);
}

/*
moonpos() takes days from J2000.0 and returns ecliptic coordinates
of moon in the pointers. Note call by reference.
This function is within a couple of arcminutes most of the time,
and is truncated from the Meeus Ch45 series, themselves truncations of
ELP-2000. Returns moon distance in earth radii.
Terms have been written out explicitly rather than using the
table based method as only a small number of terms is
retained.
*/
void moonpos(double d, double *lambda, double *beta, double *rvec)
{
    double dl, dB, dR, L, D, M, M1, F, e, lm, bm, rm, t;

    t = d / 36525;

    L = range(218.3164591  + 481267.88134236  * t) * RADS;
    D = range(297.8502042  + 445267.1115168  * t) * RADS;
    M = range(357.5291092  + 35999.0502909  * t) * RADS;
    M1 = range(134.9634114  + 477198.8676313  * t - .008997 * t * t) * RADS;
    F = range(93.27209929999999  + 483202.0175273  * t - .0034029*t*t)*RADS;
    e = 1 - .002516 * t;

    dl =      6288774 * sin(M1);
    dl +=     1274027 * sin(2 * D - M1);
    dl +=      658314 * sin(2 * D);
    dl +=      213618 * sin(2 * M1);
    dl -=  e * 185116 * sin(M);
    dl -=      114332 * sin(2 * F) ;
    dl +=       58793 * sin(2 * D - 2 * M1);
    dl +=   e * 57066 * sin(2 * D - M - M1) ;
    dl +=       53322 * sin(2 * D + M1);
    dl +=   e * 45758 * sin(2 * D - M);
    dl -=   e * 40923 * sin(M - M1);
    dl -=       34720 * sin(D) ;
    dl -=   e * 30383 * sin(M + M1) ;
    dl +=       15327 * sin(2 * D - 2 * F) ;
    dl -=       12528 * sin(M1 + 2 * F);
    dl +=       10980 * sin(M1 - 2 * F);
    lm = rangerad(L + dl / 1000000 * RADS);

    dB =   5128122 * sin(F);
    dB +=   280602 * sin(M1 + F);
    dB +=   277693 * sin(M1 - F);
    dB +=   173237 * sin(2 * D - F);
    dB +=    55413 * sin(2 * D - M1 + F);
    dB +=    46271 * sin(2 * D - M1 - F);
    dB +=    32573 * sin(2 * D + F);
    dB +=    17198 * sin(2 * M1 + F);
    dB +=     9266 * sin(2 * D + M1 - F);
    dB +=     8822 * sin(2 * M1 - F);
    dB += e * 8216 * sin(2 * D - M - F);
    dB +=     4324 * sin(2 * D - 2 * M1 - F);
    bm = dB / 1000000 * RADS;

    dR =    -20905355 * cos(M1);
    dR -=     3699111 * cos(2 * D - M1);
    dR -=     2955968 * cos(2 * D);
    dR -=      569925 * cos(2 * M1);
    dR +=   e * 48888 * cos(M);
    dR -=        3149 * cos(2 * F);
    dR +=      246158 * cos(2 * D - 2 * M1);
    dR -=  e * 152138 * cos(2 * D - M - M1) ;
    dR -=      170733 * cos(2 * D + M1);
    dR -=  e * 204586 * cos(2 * D - M);
    dR -=  e * 129620 * cos(M - M1);
    dR +=      108743 * cos(D);
    dR +=  e * 104755 * cos(M + M1);
    dR +=       79661 * cos(M1 - 2 * F);
    rm = 385000.56  + dR / 1000;

    *lambda = lm;
    *beta = bm;
    /* distance to Moon must be in Earth radii */
    *rvec = rm / 6378.14;
}

/*
topomoon() takes the local siderial time, the geographical
latitude of the observer, and pointers to the geocentric
equatorial coordinates. The function overwrites the geocentric
coordinates with topocentric coordinates on a simple spherical
earth model (no polar flattening). Expects Moon-Earth distance in
Earth radii.    Formulas scavenged from Astronomical Almanac 'low
precision formulae for Moon position' page D46.
*/

void topo(double lst, double glat, double *alp, double *dec, double *r)
{
    double x, y, z, r1;
    x = *r * cos(*dec) * cos(*alp) - cos(glat) * cos(lst);
    y = *r * cos(*dec) * sin(*alp) - cos(glat) * sin(lst);
    z = *r * sin(*dec)  - sin(glat);
    r1 = sqrt(x*x + y*y + z*z);
    *alp = atan22(y, x);
    *dec = asin(z / r1);
    *r = r1;
}

/*
moontransit() takes date, the time zone and geographic longitude
of observer and returns the time (decimal hours) of lunar transit
on that day if there is one, and sets the notransit flag if there
isn't. See Explanatory Supplement to Astronomical Almanac
section 9.32 and 9.31 for the method.
*/

double moontransit(int y, int m, int d, double tz, double glat, double glong, int *notransit)
{
    double hm, ht, ht1, lon, lat, rv, dnew, lst;
    int itcount;

    hm=0.0;
    dnew=0.0;
    lst=0.0;
    ht1 = 180 * RADS;
    ht = 0;
    itcount = 0;
    *notransit = 0;
    do
    {
        ht = ht1;
        itcount++;
        dnew = days(y, m, d, ht * DEGS/15) - tz/24;
        lst = gst(dnew) + glong;
        /* find the topocentric Moon ra (hence hour angle) and dec */
        moonpos(dnew, &lon, &lat, &rv);
        equatorial(dnew, &lon, &lat, &rv);
        topo(lst, glat, &lon, &lat, &rv);
        hm = rangerad(lst -  lon);
        ht1 = rangerad(ht - hm);
        /* if no convergence, then no transit on that day */
        if (itcount > 30)
        {
            *notransit = 1;
            break;
        }
    }
    while (fabs(ht - ht1) > 0.04 * RADS);
    return(ht1);
}

/*
  Calculates the selenographic coordinates of either the sub Earth point
  (optical libration) or the sub-solar point (selen. coords of centre of
  bright hemisphere).  Based on Meeus chapter 51 but neglects physical
  libration and nutation, with some simplification of the formulas.
*/
void libration(double day, double lambda, double beta, double alpha, double *l, double *b, double *p)
{
    double i, f, omega, w, y, x, a, t, eps;
    t = day / 36525;
    i = 1.54242 * RADS;
    eps = epsilon(day);
    f = range(93.2720993 + 483202.0175273 * t - .0034029 * t * t) * RADS;
    omega = range(125.044555 - 1934.1361849 * t + .0020762 * t * t) * RADS;
    w = lambda - omega;
    y = sin(w) * cos(beta) * cos(i) - sin(beta) * sin(i);
    x = cos(w) * cos(beta);
    a = atan22(y, x);
    *l = a - f;

    /*  kludge to catch cases of 'round the back' angles  */
    if (*l < -90 * RADS) *l += TPI;
    if (*l > 90 * RADS)  *l -= TPI;
    *b = asin(-sin(w) * cos(beta) * sin(i) - sin(beta) * cos(i));

    /*  pa pole axis - not used for Sun stuff */
    x = sin(i) * sin(omega);
    y = sin(i) * cos(omega) * cos(eps) - cos(i) * sin(eps);
    w = atan22(x, y);
    *p = rangerad(asin(sqrt(x*x + y*y) * cos(alpha - w) / cos(*b)));
}

/*
  Takes: days since J2000.0, eq coords Moon, ratio of moon to sun distance,
  eq coords Sun
  Returns: position angle of bright limb wrt NCP, percentage illumination
  of Sun
*/
void illumination(double /*day*/, double lra, double ldec, double dr, double sra, double sdec, double *pabl, double *ill)
{
    double x, y, phi, i;
    y = cos(sdec) * sin(sra - lra);
    x = sin(sdec) * cos(ldec) - cos(sdec) * sin(ldec) * cos (sra - lra);
    *pabl = atan22(y, x);
    phi = acos(sin(sdec) * sin(ldec) + cos(sdec) * cos(ldec) * cos(sra-lra));
    i = atan22(sin(phi) , (dr - cos(phi)));
    *ill = 0.5*(1 + cos(i));
}

/*
sunpos() takes days from J2000.0 and returns ecliptic longitude
of Sun in the pointers. Latitude is zero at this level of precision,
but pointer left in for consistency in number of arguments.
This function is within 0.01 degree (1 arcmin) almost all the time
for a century either side of J2000.0. This is from the 'low precision
fomulas for the Sun' from C24 of Astronomical Alamanac
*/
void sunpos(double d, double *lambda, double *beta, double *rvec)
{
    double L, g, ls, bs, rs;

    L = range(280.461 + .9856474 * d) * RADS;
    g = range(357.528 + .9856003 * d) * RADS;
    ls = L + (1.915 * sin(g) + .02 * sin(2 * g)) * RADS;
    bs = 0;
    rs = 1.00014 - .01671 * cos(g) - .00014 * cos(2 * g);
    *lambda = ls;
    *beta = bs;
    *rvec = rs;
}

/*
this routine returns the altitude given the days since J2000.0
the hour angle and declination of the object and the latitude
of the observer. Used to find the Sun's altitude to put a letter
code on the transit time, and to find the Moon's altitude at
transit just to make sure that the Moon is visible.
*/
double alt(double glat, double ha, double dec)
{
    return(asin(sin(dec) * sin(glat) + cos(dec) * cos(glat) * cos(ha)));
}

/* returns an angle in degrees in the range 0 to 360 */
double range(double x)
{
    double a, b;
    b = x / 360;
    a = 360 * (b - floor(b));
    if (a < 0)
        a = 360 + a;
    return(a);
}

/* returns an angle in rads in the range 0 to two pi */
double rangerad(double x)
{
    double a, b;
    b = x / TPI;
    a = TPI * (b - floor(b));
    if (a < 0)
        a = TPI + a;
    return(a);
}

/*
gets the atan2 function returning angles in the right
order and  range
*/
double atan22(double y, double x)
{
    double a;

    a = atan2(y, x);
    if (a < 0) a += TPI;
    return(a);
}

/*
returns mean obliquity of ecliptic in radians given days since
J2000.0.
*/
double epsilon(double d)
{
    double t = d/ 36525;
    return((23.4392911111111 - (t* (46.8150 + 0.00059*t)/3600)) *RADS);
}

/*
replaces ecliptic coordinates with equatorial coordinates
note: call by reference destroys original values
R is unchanged.
*/
void equatorial(double d, double *lon, double *lat, double */*r*/)
{
    double  eps, ceps, seps, l, b;

    l = *lon;
    b = * lat;
    eps = epsilon(d);
    ceps = cos(eps);
    seps = sin(eps);
    *lon = atan22(sin(l)*ceps - tan(b)*seps, cos(l));
    *lat = asin(sin(b)*ceps + cos(b)*seps*sin(l));
}

/*
replaces equatorial coordinates with ecliptic ones. Inverse
of above, but used to find topocentric ecliptic coords.
*/
void ecliptic(double d, double *lon, double *lat, double */*r*/)
{
    double  eps, ceps, seps, alp, dec;
    alp = *lon;
    dec = *lat;
    eps = epsilon(d);
    ceps = cos(eps);
    seps = sin(eps);
    *lon = atan22(sin(alp)*ceps + tan(dec)*seps, cos(alp));
    *lat = asin(sin(dec)*ceps - cos(dec)*seps*sin(alp));
}

/*
returns the siderial time at greenwich meridian as
an angle in radians given the days since J2000.0
*/
double gst( double d)
{
    double t = d / 36525;
    double theta;
    theta = range(280.46061837 + 360.98564736629 * d + 0.000387933 * t * t);
    return(theta * RADS);
}

void tmoonsub_(double *day, double *glat, double *glong, double *moonalt,
               double *mrv, double *l, double *b, double *paxis)
{
    double mlambda, mbeta;
    double malpha, mdelta;
    double lst, mhr;
    double tlambda, tbeta, trv;

    lst = gst(*day) + *glong;

    /* find Moon topocentric coordinates for libration calculations */

    moonpos(*day, &mlambda, &mbeta, mrv);
    malpha = mlambda;
    mdelta = mbeta;
    equatorial(*day, &malpha, &mdelta, mrv);
    topo(lst, *glat, &malpha, &mdelta, mrv);
    mhr = rangerad(lst - malpha);
    *moonalt = alt(*glat, mhr, mdelta);

    /* Optical libration and Position angle of the Pole */

    tlambda = malpha;
    tbeta = mdelta;
    trv = *mrv;
    ecliptic(*day, &tlambda, &tbeta, &trv);
    libration(*day, tlambda, tbeta, malpha,  l, b, paxis);
}



#include "hvastrodataw.h"
#include <QFile>
#include <QTextStream>
#include <QCoreApplication>
//#include <QtGui>

HvAstroDataW::HvAstroDataW(int x,int y,QWidget *parent)
        : QWidget(parent)
{
    setWindowTitle(tr("Astronomical Data"));
    //setWindowFlags(windowFlags() ^ Qt::WindowContextHelpButtonHint);
    setWindowFlags(windowFlags() &~ Qt::WindowMaximizeButtonHint &~ Qt::WindowContextHelpButtonHint);
    setWindowIcon(QPixmap(":pic/ms_ico.png"));


	azel_path = (QCoreApplication::applicationDirPath())+"/settings/azel.dat";
	f_txrx = false;
	
    //setFixedSize(240,390);
    //setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);

    //this->setMinimumSize(230,370);
    //this->setBackgroundColor(QColor(255,255,255));
    setStyleSheet("QWidget{font-size:13pt;font-weight:bold;};");//background-color:rgb(255,255,222); background-color:rgb(255,255,200);
    QFont f_t = font();
    //f_t.setPointSize(13);
    //this->setFont(f_t);
    QFontMetrics *fm_X = new QFontMetrics(f_t);
    int fm_y = fm_X->height();

    uth8z = 0.0;
    dopplerz = 0.0;
    doppler00z = 0.0;
    poloffset1 = 0.0;
    poloffset2 = 0.0;

    s_mygrid = "KN23SF";
    s_hisgrid = "KN23SF";
    s_nfreq = 70;

    QLabel *az_t = new QLabel("Az  ");
    QLabel *el_t = new QLabel("El  ");
    QLabel *moon_t = new QLabel("Moon:");
    moon_t->setContentsMargins(0,fm_y+4,0,0);
    moon_az = new QLabel;
    moon_az->setText("0");
    moon_el = new QLabel;
    moon_el->setText("0");

    QLabel *moondx_t = new QLabel("Moon/DX:");
    moondx_az = new QLabel;
    moondx_az->setText("0");
    moondx_el = new QLabel;
    moondx_el->setText("0");

    QLabel *sun_t = new QLabel("Sun:");
    sun_az = new QLabel;
    sun_az->setText("0");
    sun_el = new QLabel;
    sun_el->setText("0");

    QLabel *sou_t = new QLabel("Source:");
    sou_az = new QLabel;
    sou_az->setText("0");
    sou_el = new QLabel;
    sou_el->setText("0");

    QLabel *dx_t = new QLabel("DX  ");
    dx_t->setContentsMargins(0,fm_y,0,0);
    QLabel *self_t = new QLabel("Self  ");
    self_t->setContentsMargins(0,fm_y,0,0);
    QLabel *dop_t = new QLabel("Dop:");
    dop_t->setContentsMargins(0,fm_y*2,0,0);
    dop_dx =  new QLabel;
    dop_dx->setText("0");
    dop_self = new QLabel;
    dop_self->setText("0");
    QLabel *dfdt_t = new QLabel("df/dt:");
    dfdt_dx =  new QLabel;
    dfdt_dx->setText("0");
    dfdt_self = new QLabel;
    dfdt_self->setText("0");
    QLabel *sp_t = new QLabel("Spread:");
    sp_dx =  new QLabel;
    sp_dx->setText("0");
    sp_self = new QLabel;
    sp_self->setText("0");
    QLabel *w50_t = new QLabel("w50:");
    w50_dx =  new QLabel;
    w50_dx->setText("0");
    w50_self = new QLabel;
    w50_self->setText("0");

    QLabel *ra_t = new QLabel("RA  ");
    ra_t->setContentsMargins(0,fm_y,0,0);
    QLabel *dec_t = new QLabel("DEC  ");
    dec_t->setContentsMargins(0,fm_y,0,0);
    QLabel *moon_rd_t = new QLabel("Moon:");
    moon_rd_t->setContentsMargins(0,fm_y*2,0,0);
    moon_ra =  new QLabel;
    moon_ra->setText("0");
    moon_dec = new QLabel;
    moon_dec->setText("0");
    QLabel *sou_rd_t = new QLabel("Source:");
    sou_ra =  new QLabel;
    sou_ra->setText("0");
    sou_dec = new QLabel;
    sou_dec->setText("0");

    QVBoxLayout *V_l_txt = new QVBoxLayout();
    V_l_txt->setContentsMargins ( 0, 0, 0, 0);
    V_l_txt->setSpacing(0);
    V_l_txt->addWidget(moon_t);
    V_l_txt->addWidget(moondx_t);
    V_l_txt->addWidget(sun_t);
    V_l_txt->addWidget(sou_t);
    V_l_txt->addWidget(dop_t);
    V_l_txt->addWidget(dfdt_t);
    V_l_txt->addWidget(sp_t);
    V_l_txt->addWidget(w50_t);
    V_l_txt->addWidget(moon_rd_t);
    V_l_txt->addWidget(sou_rd_t);
    QVBoxLayout *V_l_0 = new QVBoxLayout();
    V_l_0->setContentsMargins ( 0, 0, 0, 0);
    V_l_0->setSpacing(0);
    V_l_0->addWidget(az_t);
    V_l_0->setAlignment(az_t,Qt::AlignRight);//
    V_l_0->addWidget(moon_az);
    V_l_0->setAlignment(moon_az,Qt::AlignRight);
    V_l_0->addWidget(moondx_az);
    V_l_0->setAlignment(moondx_az,Qt::AlignRight);
    V_l_0->addWidget(sun_az);
    V_l_0->setAlignment(sun_az,Qt::AlignRight);
    V_l_0->addWidget(sou_az);
    V_l_0->setAlignment(sou_az,Qt::AlignRight);
    V_l_0->addWidget(dx_t);
    V_l_0->setAlignment(dx_t,Qt::AlignRight);//
    V_l_0->addWidget(dop_dx);
    V_l_0->setAlignment(dop_dx,Qt::AlignRight);
    V_l_0->addWidget(dfdt_dx);
    V_l_0->setAlignment(dfdt_dx,Qt::AlignRight);
    V_l_0->addWidget(sp_dx);
    V_l_0->setAlignment(sp_dx,Qt::AlignRight);
    V_l_0->addWidget(w50_dx);
    V_l_0->setAlignment(w50_dx,Qt::AlignRight);
    V_l_0->addWidget(ra_t);
    V_l_0->setAlignment(ra_t,Qt::AlignRight);//
    V_l_0->addWidget(moon_ra);
    V_l_0->setAlignment(moon_ra,Qt::AlignRight);
    V_l_0->addWidget(sou_ra);
    V_l_0->setAlignment(sou_ra,Qt::AlignRight);

    QVBoxLayout *V_l_1 = new QVBoxLayout();
    V_l_1->setContentsMargins ( 0, 0, 0, 0);
    V_l_1->setSpacing(0);
    V_l_1->addWidget(el_t);
    V_l_1->setAlignment(el_t,Qt::AlignRight);//
    V_l_1->addWidget(moon_el);
    V_l_1->setAlignment(moon_el,Qt::AlignRight);
    V_l_1->addWidget(moondx_el);
    V_l_1->setAlignment(moondx_el,Qt::AlignRight);
    V_l_1->addWidget(sun_el);
    V_l_1->setAlignment(sun_el,Qt::AlignRight);
    V_l_1->addWidget(sou_el);
    V_l_1->setAlignment(sou_el,Qt::AlignRight);
    V_l_1->addWidget(self_t);
    V_l_1->setAlignment(self_t,Qt::AlignRight);//
    V_l_1->addWidget(dop_self);
    V_l_1->setAlignment(dop_self,Qt::AlignRight);
    V_l_1->addWidget(dfdt_self);
    V_l_1->setAlignment(dfdt_self,Qt::AlignRight);
    V_l_1->addWidget(sp_self);
    V_l_1->setAlignment(sp_self,Qt::AlignRight);
    V_l_1->addWidget(w50_self);
    V_l_1->setAlignment(w50_self,Qt::AlignRight);
    V_l_1->addWidget(dec_t);
    V_l_1->setAlignment(dec_t,Qt::AlignRight);//
    V_l_1->addWidget(moon_dec);
    V_l_1->setAlignment(moon_dec,Qt::AlignRight);
    V_l_1->addWidget(sou_dec);
    V_l_1->setAlignment(sou_dec,Qt::AlignRight);

    QHBoxLayout *H_l0 = new QHBoxLayout();
    H_l0->setContentsMargins ( 0, 0, 0, 0);
    H_l0->setSpacing(0);
    H_l0->addLayout(V_l_txt);
    H_l0->addLayout(V_l_0);
    H_l0->addLayout(V_l_1);
    //H_l0->setAlignment(Qt::AlignCenter);


    QLabel *freq_t = new QLabel("Freq:");
    freq_t->setContentsMargins(0,fm_y,0,0);
    QLabel *tasky_t = new QLabel("Tsky:");
    tasky_t->setContentsMargins(0,fm_y,0,0);
    freq_0 =  new QLabel;
    freq_0->setContentsMargins(0,fm_y,0,0);
    freq_0->setText("0");
    tasky_0 = new QLabel;
    tasky_0->setContentsMargins(0,fm_y,0,0);
    tasky_0->setText("0");

    QLabel *mnr_t = new QLabel("MNR:");
    mnr_0 = new QLabel;
    mnr_0->setText("0");
    QLabel *dgrd_t = new QLabel("Dgrd:");
    dgrd_0 = new QLabel;
    dgrd_0->setText("0");

    QLabel *dpol_t = new QLabel("DPol:");
    dpol_0 = new QLabel;
    dpol_0->setText("0");
    QLabel *sd_t = new QLabel("SD:");
    sd_0 = new QLabel;
    sd_0->setText("0");

    QVBoxLayout *V_ld_t1 = new QVBoxLayout();
    V_ld_t1->setContentsMargins ( 0, 0, 0, 0);
    V_ld_t1->setSpacing(0);
    V_ld_t1->addWidget(freq_t);
    V_ld_t1->addWidget(mnr_t);
    V_ld_t1->addWidget(dpol_t);

    QVBoxLayout *V_ld_0 = new QVBoxLayout();
    V_ld_0->setContentsMargins ( 0, 0, 8, 0);
    V_ld_0->setSpacing(0);
    V_ld_0->addWidget(freq_0);
    V_ld_0->setAlignment(freq_0,Qt::AlignRight);
    V_ld_0->addWidget(mnr_0);
    V_ld_0->setAlignment(mnr_0,Qt::AlignRight);
    V_ld_0->addWidget(dpol_0);
    V_ld_0->setAlignment(dpol_0,Qt::AlignRight);
    //V_ld_0->setAlignment(Qt::AlignRight);

    QVBoxLayout *V_ld_t2 = new QVBoxLayout();
    V_ld_t2->setContentsMargins ( 8, 0, 0, 0);
    V_ld_t2->setSpacing(0);
    V_ld_t2->addWidget(tasky_t);
    V_ld_t2->addWidget(dgrd_t);
    V_ld_t2->addWidget(sd_t);

    QVBoxLayout *V_ld_1 = new QVBoxLayout();
    V_ld_1->setContentsMargins ( 0, 0, 0, 0);
    V_ld_1->setSpacing(0);
    V_ld_1->addWidget(tasky_0);
    V_ld_1->setAlignment(tasky_0,Qt::AlignRight);
    V_ld_1->addWidget(dgrd_0);
    V_ld_1->setAlignment(dgrd_0,Qt::AlignRight);
    V_ld_1->addWidget(sd_0);
    V_ld_1->setAlignment(sd_0,Qt::AlignRight);
    //V_ld_1->setAlignment(Qt::AlignRight);

    QHBoxLayout *H_l1= new QHBoxLayout();
    H_l1->setContentsMargins ( 0, 0, 0, 0);
    H_l1->setSpacing(0);
    H_l1->addLayout(V_ld_t1);
    H_l1->addLayout(V_ld_0);
    H_l1->addLayout(V_ld_t2);
    H_l1->addLayout(V_ld_1);
    H_l1->setAlignment(Qt::AlignTop);


    s_radeceg = new QLabel("For eg. hh:mm:ss (1:30:00 or 1.5)\nor (-1:30:00 or -1.5)");
    s_radeceg->setStyleSheet("QLabel{font-size:10pt;color:green};");
    s_radeceg->setContentsMargins(0,fm_y,0,0);
    s_radeceg->setAlignment(Qt::AlignCenter);
    //QLabel *s_raeg = new QLabel("For eg. hh:mm:ss (1:30:00 or 1.5)");
    //s_raeg->setStyleSheet("QLabel{font-size:10pt; color:green};");
    //s_raeg->setContentsMargins(0,fm_y,0,0);
    QLabel *s_ra = new QLabel("Source RA:");
    le_auxra = new QLineEdit();
    le_auxra->setMaxLength(9);
    le_auxra->setMaximumWidth(110);
    le_auxra->setText("0");

    QLabel *s_dec = new QLabel("Source DEC:");
    le_auxdec = new QLineEdit();
    le_auxdec->setMaxLength(9);
    le_auxdec->setMaximumWidth(110);
    le_auxdec->setText("0");

    QHBoxLayout *H_ra= new QHBoxLayout();
    H_ra->setContentsMargins ( 0, 0, 0, 0);
    H_ra->setSpacing(0);
    H_ra->addWidget(s_ra);
    H_ra->addWidget(le_auxra);
    //H_ra->setAlignment(le_auxra,Qt::AlignRight);
    //H_ra->setAlignment(Qt::AlignHCenter);
    QHBoxLayout *H_dec= new QHBoxLayout();
    H_dec->setContentsMargins ( 0, 0, 0, 0);
    H_dec->setSpacing(0);
    H_dec->addWidget(s_dec);
    H_dec->addWidget(le_auxdec);
    //H_dec->setAlignment(le_auxdec,Qt::AlignRight);
    //H_dec->setAlignment(Qt::AlignHCenter);
    //H_dec->setAlignment(Qt::AlignTop);

    /*QVBoxLayout *V_lss1 = new QVBoxLayout();
    V_lss1->setContentsMargins ( 0, 15, 0, 0);
    V_lss1->setSpacing(0);
    V_lss1->addWidget(s_raeg);
    V_lss1->addLayout(H_ra);
    //V_lss1->setAlignment(Qt::AlignTop);
    QVBoxLayout *V_lss2 = new QVBoxLayout();
    V_lss2->setContentsMargins ( 0, 0, 0, 0);
    V_lss2->setSpacing(0);
    V_lss2->addWidget(s_deceg);
    V_lss2->addLayout(H_dec); 
    //V_lss2->setAlignment(Qt::AlignTop);*/


    QVBoxLayout *V_l = new QVBoxLayout(this);
    V_l->setContentsMargins ( 8, 8, 8, 8);
    V_l->setSpacing(0);
    V_l->addLayout(H_l0);
    V_l->addLayout(H_l1);
    //V_l->addLayout(V_lss1);
    //V_l->addLayout(V_lss2);
    V_l->addWidget(s_radeceg);
    //V_l->setAlignment(s_radeceg,Qt::AlignRight);
    V_l->addLayout(H_ra);
    //V_l->addWidget(s_deceg);
    V_l->addLayout(H_dec);
    V_l->setAlignment(Qt::AlignTop);

    //V_l->setAlignment(Qt::AlignCenter);

    this->setLayout(V_l);

    //this->setMaximumSize(300,600);
    //this->setMinimumSize(230,465);

    move(x,y);

    timer_ref_ = new QTimer();
    connect(timer_ref_, SIGNAL(timeout()), this, SLOT(astro0()));
    //timer_ref_->start(1000);//5ms refresh time
    //astro0();
}
HvAstroDataW::~HvAstroDataW()
{}
void HvAstroDataW::SetPosXY(QString s)
{
	QStringList list_s = s.split("#");
	if(list_s.count()==2)
	{
		move(list_s[0].toInt(),list_s[1].toInt());
	}	
}
void HvAstroDataW::SetFont(QFont f)
{
    setFont(f);
    QString fs = "font-size:"+QString("%1").arg(f.pointSize()+4)+"pt;";//13pt
    setStyleSheet("QWidget{"+fs+"font-weight:bold;};");
    QString fs2 = "font-size:"+QString("%1").arg(f.pointSize()+1)+"pt;";//10pt
    s_radeceg->setStyleSheet("QLabel{"+fs2+"color:green};");
    /*QFont f_t = f;
       //f_t.setPointSize(13);
       //this->setFont(f_t);
       QFontMetrics *fm_X = new QFontMetrics(f_t);
       int fm_y = fm_X->height();*/
}
void HvAstroDataW::StartStopTimer(bool f)
{
    //qDebug()<<"StartStopTimerAstro=================="<<f;
    if (f) timer_ref_->start(2000);
    else timer_ref_->stop();
}
void HvAstroDataW::SetMyLocHisLocBand(QString mloc,QString hisloc,QString strfreq)
{
	//qDebug()<<"NeedLoc="<<s_mygrid<<mloc<<s_hisgrid<<hisloc<<strfreq;
	if (mloc.isEmpty() || strfreq.isEmpty()) return; //2.10 protection	 
	if (hisloc.isEmpty())//2.11
		 hisloc=mloc;
				
    s_mygrid = mloc;
    s_hisgrid = hisloc;
    int frq_mhz = 70;//default 70 MHz
    if (strfreq.contains(" kHz")) frq_mhz = 1; //2.65
    if (strfreq.contains(" MHz"))
    {
        strfreq.remove(" MHz");
        frq_mhz=(int)strfreq.toDouble();// v 1.44 for 1.8 3.5
    }
    else if (strfreq.contains(" GHz"))
    {
        strfreq.remove(" GHz");
        frq_mhz = strfreq.toDouble()*1000.0;
    }
    s_nfreq = frq_mhz;
    //qDebug()<<"SF="<<s_mygrid<<s_hisgrid<<s_nfreq<<strfreq;
}
void HvAstroDataW::tm2(double day,double xlat4,double xlon4,double &xl4,double &b4)
{
    /*implicit real*8 (a-h,o-z)
    parameter (RADS=0.0174532925199433d0)
    real*4 xlat4,xlon4,xl4,b4*/
    //double RADS=0.0174532925199433d;
    double xl=0.0;
    double b=0.0;
    double el=0.0;
    double rv=0.0;
    double pax=0.0;

    double glat=xlat4*RADS;
    double glong=xlon4*RADS;
    tmoonsub_(&day,&glat,&glong,&el,&rv,&xl,&b,&pax);
    //tmoonsub_(day,glat,glong,el,rv,xl,b,pax);
    xl4=xl;
    b4=b;
}
double HvAstroDataW::dot(double *x,double *y)
{
    //real*8 x(3),y(3)
    double dot=0.0;
    for (int i = 0; i<3; i++)
    {//do i=1,3
        dot=dot+x[i]*y[i];
    }
    return dot;
}
void HvAstroDataW::toxyz(double alpha,double delta,double r,double *vec)
{
    /*implicit real*8 (a-h,o-z)
    real*8 vec(3)*/
    vec[0]=r*cos(delta)*cos(alpha);
    vec[1]=r*cos(delta)*sin(alpha);
    vec[2]=r*sin(delta);
}
void HvAstroDataW::fromxyz(double *vec,double &alpha,double &delta,double &r)
{
    /*implicit real*8 (a-h,o-z)
    real*8 vec(3)*/
    //double twopi = 6.2831853070;
    r=sqrt(vec[0]*vec[0] + vec[1]*vec[1] + vec[2]*vec[2]);
    alpha=atan2(vec[1],vec[0]);
    if (alpha<0.0) alpha=alpha+6.2831853070;  //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    delta=asin(vec[2]/r);
}
void HvAstroDataW::coord(double A0,double B0,double AP,double BP,double A1,double B1,double &A2,double &B2)
{
    /*! Examples:
    ! 1. From ha,dec to az,el:
    !      call coord(pi,pio2-lat,0.,lat,ha,dec,az,el)
    ! 2. From az,el to ha,dec:
    !      call coord(pi,pio2-lat,0.,lat,az,el,ha,dec)
    ! 3. From ra,dec to l,b
    !      call coord(4.635594495,-0.504691042,3.355395488,0.478220215,
    !        ra,dec,l,b)
    ! 4. From l,b to ra,dec
    !      call coord(1.705981071d0,-1.050357016d0,2.146800277d0,
    !        0.478220215d0,l,b,ra,dec)
    ! 5. From ecliptic latitude (eb) and longitude (el) to ra, dec:
    !      call coord(0.e0,0.e0,-pio2,pio2-23.443*pi/180,ra,dec,el,eb)*/

    double SB0=sin(B0);
    double CB0=cos(B0);
    double SBP=sin(BP);
    double CBP=cos(BP);
    double SB1=sin(B1);
    double CB1=cos(B1);
    double SB2=SBP*SB1 + CBP*CB1*cos(AP-A1);
    double CB2=sqrt(1.0-(SB2*SB2));
    B2=atan(SB2/CB2);
    double SAA=sin(AP-A1)*CB1/CB2;
    double CAA=(SB1-SB2*SBP)/(CB2*CBP);
    double CBB=SB0/CBP;
    double SBB=sin(AP-A0)*CB0;
    double SA2=SAA*CBB-CAA*SBB;
    double CA2=CAA*CBB+SAA*SBB;
    double TA2O2=0.0; //!Shut up compiler warnings. -db
    if (CA2<=0.0) TA2O2=(1.0-CA2)/SA2; //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    if (CA2>0.0) TA2O2=SA2/(1.0+CA2);
    A2=2.0*atan(TA2O2);
    if (A2<0.0) A2=A2+6.2831853;
}
void HvAstroDataW::dcoord(double A0,double B0,double AP,double BP,double A1,double B1,double &A2,double &B2)
{
    /*implicit real*8 (a-h,o-z)
    ! Examples:
    ! 1. From ha,dec to az,el:
    !      call coord(pi,pio2-lat,0.,lat,ha,dec,az,el)
    ! 2. From az,el to ha,dec:
    !      call coord(pi,pio2-lat,0.,lat,az,el,ha,dec)
    ! 3. From ra,dec to l,b
    !      call coord(4.635594495,-0.504691042,3.355395488,0.478220215,
    !        ra,dec,l,b)
    ! 4. From l,b to ra,dec
    !      call coord(1.705981071d0,-1.050357016d0,2.146800277d0,
    !        0.478220215d0,l,b,ra,dec)
    ! 5. From ecliptic latitude (eb) and longitude (el) to ra, dec:
    !      call coord(0.d0,0.d0,-pio2,pio2-23.443*pi/180,ra,dec,el,eb)*/

    double SB0=sin(B0);
    double CB0=cos(B0);
    double SBP=sin(BP);
    double CBP=cos(BP);
    double SB1=sin(B1);
    double CB1=cos(B1);
    double SB2=SBP*SB1 + CBP*CB1*cos(AP-A1);
    double CB2=sqrt(1.0-(SB2*SB2));//CB2=SQRT(1.D0-SB2**2)
    B2=atan(SB2/CB2);
    double SAA=sin(AP-A1)*CB1/CB2;
    double CAA=(SB1-SB2*SBP)/(CB2*CBP);
    double CBB=SB0/CBP;
    double SBB=sin(AP-A0)*CB0;
    double SA2=SAA*CBB-CAA*SBB;
    double CA2=CAA*CBB+SAA*SBB;
    double TA2O2=0.0; //!Shut up compiler warnings. -db
    if (CA2<=0.0) TA2O2=(1.0-CA2)/SA2; //1.76 c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    if (CA2>0.0) TA2O2=SA2/(1.0+CA2);
    A2=2.0*atan(TA2O2);
    if (A2<0.0) A2=A2+6.28318530717958640;
}
void HvAstroDataW::moon2(int y,int m,int Day,double UT,double lon,double lat,double &RA,double &Dec,
                         double &topRA,double &topDec,double &LST,double &HA,double &Az,double &El,double &dist)//Az0,El0,
{
    /*implicit none
    integer y                           !Year
    integer m                           !Month
    integer Day                         !Day
    real*8 UT                           !UTC in hours
    real*8 RA,Dec                       !RA and Dec of moon
    //! NB: Double caps are single caps in the writeup.
    real*8 NN                           !Longitude of ascending node
    real*8 i                            !Inclination to the ecliptic
    real*8 w                            !Argument of perigee
    real*8 a                            !Semi-major axis
    real*8 e                            !Eccentricity
    real*8 MM                           !Mean anomaly
    real*8 v                            !True anomaly
    real*8 EE                           !Eccentric anomaly
    real*8 ecl                          !Obliquity of the ecliptic
    real*8 d                            !Ephemeris time argument in days
    real*8 r                            !Distance to sun, AU
    real*8 xv,yv                        !x and y coords in ecliptic
    real*8 lonecl,latecl                !Ecliptic long and lat of moon
    real*8 xg,yg,zg                     !Ecliptic rectangular coords
    real*8 Ms                           !Mean anomaly of sun
    real*8 ws                           !Argument of perihelion of sun
    real*8 Ls                           !Mean longitude of sun (Ns=0)
    real*8 Lm                           !Mean longitude of moon
    real*8 DD                           !Mean elongation of moon
    real*8 FF                           !Argument of latitude for moon
    real*8 xe,ye,ze                     !Equatorial geocentric coords of moon
    real*8 mpar                         !Parallax of moon (r_E / d)
    real*8 lat,lon                      !Station coordinates on earth
    real*8 gclat                        !Geocentric latitude
    real*8 rho                          !Earth radius factor
    real*8 GMST0,LST,HA
    real*8 g
    real*8 topRA,topDec                 !Topocentric coordinates of Moon
    real*8 Az,El
    real*8 dist
    real*8 rad,twopi,pi,pio2
    data rad/57.2957795131d0/,twopi/6.283185307d0/*/

    double rad = 57.29577951310;
    double twopi = 6.2831853070;

    double d=367*y - 7*(y+(m+9)/12)/4 + 275*m/9 + Day - 730530 + UT/24.0;
    //double d=367.0*(double)y - 7.0*((double)y+((double)m+9.0)/12.0)/4.0 + 275.0*(double)m/9.0 + (double)Day - 730530.0 + UT/24.0;

    double ecl = 23.43930 - 3.563e-7 * d;//ecl = 23.4393d0 - 3.563d-7 * d
    //qDebug()<<"moon2"<<d;

    //! Orbital elements for Moon:
    double NN = 125.12280 - 0.05295380830 * d;
    double ii = 5.14540;
    double w = fmod(318.06340 + 0.16435732230 * d + 360000.0,360.0);
    double a = 60.26660;
    double e = 0.0549000;
    double MM = fmod(115.36540 + 13.06499295090 * d + 360000.0,360.0);

    double EE = MM + e*rad*sin(MM/rad) * (1.0 + e*cos(MM/rad));
    EE = EE - (EE - e*rad*sin(EE/rad)-MM) / (1.0 - e*cos(EE/rad));
    EE = EE - (EE - e*rad*sin(EE/rad)-MM) / (1.0 - e*cos(EE/rad));

    double xv = a * (cos(EE/rad) - e);
    double yv = a * (sqrt(1.0-e*e) * sin(EE/rad));

    double v = fmod(rad*atan2(yv,xv)+720.0,360.0);
    double r = sqrt(xv*xv + yv*yv);

    //! Get geocentric position in ecliptic rectangular coordinates:
    double xg = r * (cos(NN/rad)*cos((v+w)/rad) - sin(NN/rad)*sin((v+w)/rad)*cos(ii/rad));
    double yg = r * (sin(NN/rad)*cos((v+w)/rad) + cos(NN/rad)*sin((v+w)/rad)*cos(ii/rad));
    double zg = r * (sin((v+w)/rad)*sin(ii/rad));

    //! Ecliptic longitude and latitude of moon:
    double lonecl = fmod(rad*atan2(yg/rad,xg/rad)+720.0,360.0);
    double latecl = rad*atan2(zg/rad,sqrt(xg*xg + yg*yg)/rad);

    //! Now include orbital perturbations:
    double Ms = fmod(356.04700 + 0.98560025850 * d + 3600000.0,360.0);
    double ws = 282.94040 + 4.70935e-5 * d;//ws = 282.9404d0 + 4.70935d-5*d
    double Ls = fmod(Ms + ws + 720.0,360.0);
    double Lm = fmod(MM + w + NN+720.0,360.0);
    double DD = fmod(Lm - Ls + 360.0,360.0);
    double FF = fmod(Lm - NN + 360.0,360.0);
    //qDebug()<<"1="<<dist<<latecl<<lonecl;
    lonecl = lonecl
             -1.2740 * sin((MM-2.0*DD)/rad)
             +0.6580 * sin(2.0*DD/rad)
             -0.1860 * sin(Ms/rad)
             -0.0590 * sin((2.0*MM-2.0*DD)/rad)
             -0.0570 * sin((MM-2.0*DD+Ms)/rad)
             +0.0530 * sin((MM+2.0*DD)/rad)
             +0.0460 * sin((2.0*DD-Ms)/rad)
             +0.0410 * sin((MM-Ms)/rad)
             -0.0350 * sin(DD/rad)
             -0.0310 * sin((MM+Ms)/rad)
             -0.0150 * sin((2.0*FF-2.0*DD)/rad)
             +0.0110 * sin((MM-4.0*DD)/rad);

    latecl = latecl
             -0.1730 * sin((FF-2.0*DD)/rad)
             -0.0550 * sin((MM-FF-2.0*DD)/rad)
             -0.0460 * sin((MM+FF-2.0*DD)/rad)
             +0.0330 * sin((FF+2.0*DD)/rad)
             +0.0170 * sin((2.0*MM+FF)/rad);

    r = 60.362980
        - 3.277460*cos(MM/rad)
        - 0.579940*cos((MM-2.0*DD)/rad)
        - 0.463570*cos(2.0*DD/rad)
        - 0.089040*cos(2.0*MM/rad)
        + 0.038650*cos((2.0*MM-2.0*DD)/rad)
        - 0.032370*cos((2.0*DD-Ms)/rad)
        - 0.026880*cos((MM+2.0*DD)/rad)
        - 0.023580*cos((MM-2.0*DD+Ms)/rad)
        - 0.020300*cos((MM-Ms)/rad)
        + 0.017190*cos(DD/rad)
        + 0.016710*cos((MM+Ms)/rad);

    dist=r*6378.1400;
    //qDebug()<<"2="<<dist<<latecl<<lonecl;
    //! Geocentric coordinates:
    //! Rectangular ecliptic coordinates of the moon:
    xg = r * cos(lonecl/rad)*cos(latecl/rad);
    yg = r * sin(lonecl/rad)*cos(latecl/rad);
    zg = r *                 sin(latecl/rad);

    //! Rectangular equatorial coordinates of the moon:
    double xe = xg;
    double ye = yg*cos(ecl/rad) - zg*sin(ecl/rad);
    double ze = yg*sin(ecl/rad) + zg*cos(ecl/rad);

    //! Right Ascension, Declination:
    RA = fmod(rad*atan2(ye,xe)+360.0,360.0);
    Dec = rad*atan2(ze,sqrt(xe*xe + ye*ye));

    //! Now convert to topocentric system:
    double mpar=rad*asin(1.0/r);
    //!     alt_topoc = alt_geoc - mpar*cos(alt_geoc)
    double gclat = lat - 0.19240*sin(2.0*lat/rad);
    double rho = 0.998830 + 0.001670*cos(2.0*lat/rad);
    double GMST0 = (Ls + 180.0)/15.0;
    LST = fmod(GMST0+UT+lon/15.0+48.0,24.0);    //!LST in hours
    HA = 15.0*LST - RA;                          //!HA in degrees
    double g = rad*atan(tan(gclat/rad)/cos(HA/rad));
    topRA = RA - mpar*rho*cos(gclat/rad)*sin(HA/rad)/cos(Dec/rad);
    topDec = Dec - mpar*rho*sin(gclat/rad)*sin((g-Dec)/rad)/sin(g/rad);
    //qDebug()<<"topoRAMoon"<<topRA<<topDec;

    HA = 15.0*LST - topRA; //1.76                      //!HA in degrees
    if (HA > 180.0) HA=HA-360.0; //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    if (HA < -180.0) HA=HA+360.0;

    double pi=0.5*twopi;//1.76
    double pio2=0.5*pi;//1.76

    //qDebug()<<"RAMoon"<<HA*twopi/360.0<<topDec/rad;

    dcoord(pi,pio2-(lat/rad),0.0,lat/rad,HA*twopi/360.0,topDec/rad,Az,El);

    Az=Az*rad;
    El=El*rad;

}
void HvAstroDataW::geocentric(double alat,double elev,double &hlt,double &erad)
{
    //implicit real*8 (a-h,o-z)
    //! IAU 1976 flattening f, equatorial radius a
    double f = 1.0/298.2570;
    double a = 6378140.0;
    double c = 1.0/sqrt(1.0 + (-2.0 + f)*f*sin(alat)*sin(alat));
    double arcf = (a*c + elev)*cos(alat);
    double arsf = (a*(1.0 - f)*(1.0 - f)*c + elev)*sin(alat);
    hlt = atan2(arsf,arcf);//hlt = datan2(arsf,arcf);
    erad = sqrt(arcf*arcf + arsf*arsf);
    erad = 0.0010*erad;
}
void HvAstroDataW::MoonDop(int nyear,int month,int nday,double uth4,double lon4,double lat4,double &RAMoon4,
                           double &DecMoon4,double &LST4,double &HA4,double &AzMoon4,double &ElMoon4,double &ldeg4,
                           double &bdeg4,double &vr4,double &dist4)
{
    /*implicit real*8 (a-h,o-z)
    real*4 uth4                    !UT in hours
    real*4 lon4                    !West longitude, degrees
    real*4 lat4                    !Latitude, degrees
    real*4 RAMoon4                 !Topocentric RA of moon, hours
    real*4 DecMoon4                !Topocentric Dec of Moon, degrees
    real*4 LST4                    !Locat sidereal time, hours
    real*4 HA4                     !Local Hour angle, degrees
    real*4 AzMoon4                 !Topocentric Azimuth of moon, degrees
    real*4 ElMoon4                 !Topocentric Elevation of moon, degrees
    real*4 ldeg4                   !Galactic longitude of moon, degrees
    real*4 bdeg4                   !Galactic latitude of moon, degrees
    real*4 vr4                     !Radial velocity of moon wrt obs, km/s
    real*4 dist4                   !Echo time, seconds
    real*8 LST
    real*8 RME(6)                  !Vector from Earth center to Moon
    real*8 RAE(6)                  !Vector from Earth center to Obs
    real*8 RMA(6)                  !Vector from Obs to Moon
    real*8 rme0(6)
    real*8 lrad
    data rad/57.2957795130823d0/,twopi/6.28310530717959d0/
    */
    double twopi = 6.283105307179590;
    double rme0[6];
    double rme[6];
    double rae[6];
    double rma[6];
    double dlat1 = 0.0;
    double erad1 = 0.0;
    double rad = 57.29577951308230;

    double dt=100.0;                      //!For numerical derivative, in seconds
    double UT=uth4;
    double RA = 0.0;
    double Dec = 0.0;
    double topRA = 0.0;
    double topDec = 0.0;
    double LST = 0.0;
    double HA = 0.0;
    double dist = 0.0;
    double Az0 = 0.0;
    double El0 = 0.0;
    double Az = 0.0;
    double El = 0.0;

    double dlat=lat4/rad;
    double dlong1=lon4/rad;
    double elev1=200.0;

    //qDebug()<<lat4<<lon4;

    geocentric(dlat,elev1,dlat1,erad1);
    //! NB: geodetic latitude used here, but geocentric latitude used when
    //! determining Earth-rotation contribution to Doppler.
    moon2(nyear,month,nday,UT-(dt/3600.0),dlong1*rad,dlat*rad,RA,Dec,topRA,topDec,LST,HA,Az0,El0,dist);//Az0,El0,
    //qDebug()<<"MOON0="<<Az0<<El0<<dist;

    toxyz(RA/rad,Dec/rad,dist,rme0);      //!Convert to rectangular coords

    moon2(nyear,month,nday,UT,dlong1*rad,dlat*rad,RA,Dec,topRA,topDec,LST,HA,Az,El,dist);
    //moon2(nyear,month,nday,UT,lon4,lat4,RA,Dec,topRA,topDec,LST,HA,Az,El,dist);

    //qDebug()<<"MOON="<<Az<<El<<dist;

    toxyz(RA/rad,Dec/rad,dist,rme);       //!Convert to rectangular coords

    double phi=LST*twopi/24.0;
    toxyz(phi,dlat1,erad1,rae);           //!Gencentric numbers used here!
    double radps=twopi/(86400.0/1.0027379090);
    rae[3]=-rae[1]*radps;                      //!Vel of Obs wrt Earth center
    rae[4]=rae[0]*radps;
    rae[5]=0.0;

    for (int i = 0; i<3; i++)
    {//do i=1,3
        rme[i+3]=(rme[i]-rme0[i])/dt;
        rma[i]=rme[i]-rae[i];
        rma[i+3]=rme[i+3]-rae[i+3];
    }
    double alpha1 = 0.0;
    double delta1 = 0.0;
    double dtopo0 = 0.0;
    fromxyz(rma,alpha1,delta1,dtopo0);     //!Get topocentric coords

    double rma_t[3];
    for (int i = 0; i<3; i++)
        rma_t[i]=rma[i+3];

    if (dtopo0==0.0) dtopo0=0.00001;//2.10 no div by 0
    double vr=dot(rma_t,rma)/dtopo0;//vr=dot(rma(4),rma)/dtopo0

    double rarad=RA/rad;
    double decrad=Dec/rad;
    double lrad = 0.0;
    double brad = 0.0;
    dcoord(4.6355944950,-0.5046910420,3.3553954880,0.4782202150,rarad,decrad,lrad,brad);

    RAMoon4=topRA;
    DecMoon4=topDec;
    LST4=LST;
    HA4=HA;
    AzMoon4=Az;
    ElMoon4=El;
    ldeg4=lrad*rad;
    bdeg4=brad*rad;
    vr4=vr;
    dist4=dist;
}
void HvAstroDataW::sun(int y,int m,int DD,double UT,double lon,double lat,double &LST,double &Az,
                       double &El,double &day)
{
    /*implicit none

    integer y                         !Year
    integer m                         !Month
    integer DD                        !Day
    integer mjd                       !Modified Julian Date
    real UT                           !UTC in hours
    real RA,Dec                       !RA and Dec of sun

    //! NB: Double caps here are single caps in the writeup.

    //! Orbital elements of the Sun (also N=0, i=0, a=1):
    real w                            !Argument of perihelion
    real e                            !Eccentricity
    real MM                           !Mean anomaly
    real Ls                           !Mean longitude

    //! Other standard variables:
    real v                            !True anomaly
    real EE                           !Eccentric anomaly
    real ecl                          !Obliquity of the ecliptic
    real d                            !Ephemeris time argument in days
    real r                            !Distance to sun, AU
    real xv,yv                        !x and y coords in ecliptic
    real lonsun                       !Ecliptic long and lat of sun
    //!Ecliptic coords of sun (geocentric)
    real xs,ys
    //!Equatorial coords of sun (geocentric)
    real xe,ye,ze
    real lon,lat
    real GMST0,LST,HA
    real xx,yy,zz
    real xhor,yhor,zhor
    real Az,El

    real day
    real rad
    data rad/57.2957795/
    */
    double rad = 57.2957795;
    double RA,Dec;                       //!RA and Dec of sun

    //! Time in days, with Jan 0, 2000 equal to 0.0:

    double d=367*y - 7*(y+(m+9)/12)/4 + 275*m/9 + DD - 730530 + UT/24.0;
    //double d=367.0*(double)y - 7.0*((double)y+((double)m+9.0)/12.0)/4.0 + 275.0*(double)m/9.0 + (double)DD - 730530.0 + UT/24.0;

    //double mjd=(double)d + 51543.0;
    double ecl = 23.4393 - 3.563e-7 * d;
    //qDebug()<<"sun"<<d;

    //! Compute updated orbital elements for Sun:
    double w = 282.9404 + 4.70935e-5 * d;
    double e = 0.016709 - 1.151e-9 * d;
    double MM = fmod(356.04700 + 0.98560025850 * d + 360000.0,360.0);
    double Ls = fmod(w+MM+720.0,360.0);

    double EE = MM + e*rad*sin(MM/rad) * (1.0 + e*cos((double)m/rad));
    EE = EE - (EE - e*rad*sin(EE/rad)-MM) / (1.0 - e*cos(EE/rad));

    double xv = cos(EE/rad) - e;
    double yv = sqrt(1.0-e*e) * sin(EE/rad);
    double v = rad*atan2(yv,xv);
    double r = sqrt(xv*xv + yv*yv);
    double lonsun = fmod(v + w + 720.0,360.0);
    //! Ecliptic coordinates of sun (rectangular):
    double xs = r * cos(lonsun/rad);
    double ys = r * sin(lonsun/rad);

    //! Equatorial coordinates of sun (rectangular):
    double xe = xs;
    double ye = ys * cos(ecl/rad);
    double ze = ys * sin(ecl/rad);

    //! RA and Dec in degrees:
    RA = rad*atan2(ye,xe);
    Dec = rad*atan2(ze,sqrt(xe*xe + ye*ye));

    double GMST0 = (Ls + 180.0)/15.0;
    LST = fmod(GMST0+UT+lon/15.0+48.0,24.0);    //!LST in hours
    double HA = 15.0*LST - RA;                          //!HA in degrees
    double xx = cos(HA/rad)*cos(Dec/rad);
    double yy = sin(HA/rad)*cos(Dec/rad);
    double zz =             sin(Dec/rad);
    double xhor = xx*sin(lat/rad) - zz*cos(lat/rad);
    double yhor = yy;
    double zhor = xx*cos(lat/rad) + zz*sin(lat/rad);
    Az = fmod(rad*atan2(yhor,xhor) + 180.0 + 360.0,360.0);
    El = rad*asin(zhor);
    day=d-1.5;
}
void HvAstroDataW::grid2deg(QString grid0,double &dlong,double &dlat)
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

    double nlong = 180.0 - 20.0*((int)g1-(int)'A');
    double n20d = 2.0*((int)g3-(int)'0');
    double xminlong = 5.0*((int)g5-(int)'a'+0.5);
    dlong = nlong - n20d - xminlong/60.0;
    double nlat = -90.0+10.0*((int)g2-(int)'A') + (int)g4-(int)'0';
    double xminlat = 2.5*((int)g6-(int)'a'+0.5);
    dlat = nlat + xminlat/60.0;
}

void HvAstroDataW::astro(int nyear,int month,int nday,double uth,int nfreq,QString Mygrid,int NStation,int mode,
                         int MoonDX,double &AzSun,double &ElSun,double &AzMoon0,double &ElMoon0,int &ntsky,double &doppler00,
                         double &doppler,double &dbMoon,double &RAMoon,double &DecMoon,double &HA,double &Dgrd, double &sd,
                         double &poloffset,double &xnr,double auxra,double auxdec,double &AzAux,double &ElAux,double &day,
                         double &lon,double &lat,double &LST)
{

    //! Computes astronomical quantities for display in JT65, CW, and EME Echo mode.
	//! NB: may want to smooth the Tsky map to 10 degrees or so.

    /*character*6 MyGrid,HisGrid
    real LST
    real lat,lon
    real ldeg
    integer*2 nt144(180)
    common/echo/xdop(2),techo,AzMoon,ElMoon,mjd
    data rad/57.2957795/*/
    double xdop[3];
    int nt144[180]={
                       234, 246, 257, 267, 275, 280, 283, 286, 291, 298,
                       305, 313, 322, 331, 341, 351, 361, 369, 376, 381,
                       383, 382, 379, 374, 370, 366, 363, 361, 363, 368,
                       376, 388, 401, 415, 428, 440, 453, 467, 487, 512,
                       544, 579, 607, 618, 609, 588, 563, 539, 512, 482,
                       450, 422, 398, 379, 363, 349, 334, 319, 302, 282,
                       262, 242, 226, 213, 205, 200, 198, 197, 196, 197,
                       200, 202, 204, 205, 204, 203, 202, 201, 203, 206,
                       212, 218, 223, 227, 231, 236, 240, 243, 247, 257,
                       276, 301, 324, 339, 346, 344, 339, 331, 323, 316,
                       312, 310, 312, 317, 327, 341, 358, 375, 392, 407,
                       422, 437, 451, 466, 480, 494, 511, 530, 552, 579,
                       612, 653, 702, 768, 863,1008,1232,1557,1966,2385,
                       2719,2924,3018,3038,2986,2836,2570,2213,1823,1461,
                       1163, 939, 783, 677, 602, 543, 494, 452, 419, 392,
                       373, 360, 353, 350, 350, 350, 350, 350, 350, 348,
                       344, 337, 329, 319, 307, 295, 284, 276, 272, 272,
                       273, 274, 274, 271, 266, 260, 252, 245, 238, 231
                   };
    //save
    double elon = 0.0;
    double bdeg = 0.0;
    double ldeg = 0.0;
    double vr = 0.0;
    double dist = 0.0;
    double AzMoon = 0.0;
    double ElMoon = 0.0;
    double rad = 57.2957795;
    //double poloffset1 = 0.0;
    //double poloffset2 = 0.0;
    QString HisGrid = s_hisgrid;
    double auxHA;
    double pi;
    double pio2;
    double tr;
    double tskymin;
    double tsysmin;
    double tsys;

    grid2deg(Mygrid,elon,lat);
    lon = - elon;
    //lat=43.19941978;
    //lon=25.554885;
    sun(nyear,month,nday,uth,lon,lat,LST,AzSun,ElSun,day);//no use RASun,DecSun,mjd
    //qDebug()<<Mygrid<<"SUN="<<LST<<AzSun<<ElSun<<day;
    if (nfreq==0) nfreq=1;//2.10 no div by 0
    double freq=nfreq*1.0e6;
    /*if(nfreq.eq.2) freq=1.8e6 // 1.8mhz
    if(nfreq.eq.4) freq=3.5e6 // 3.5mhz  */
    //qDebug()<<"start================="<<Mygrid;
    MoonDop(nyear,month,nday,uth,lon,lat,RAMoon,DecMoon,LST,HA,AzMoon,ElMoon,ldeg,bdeg,vr,dist);
    //qDebug()<<"stop=================="<<Mygrid;

    //! Compute spatial polarization offset
    double xx=sin(lat/rad)*cos(ElMoon/rad) - cos(lat/rad)*cos(AzMoon/rad)*sin(ElMoon/rad);
    double yy=cos(lat/rad)*sin(AzMoon/rad);
    if (NStation==1) poloffset1=rad*atan2(yy,xx);
    if (NStation==2) poloffset2=rad*atan2(yy,xx);

    //double techo=2.0 * dist/2.99792458e5;                 //!Echo delay time
    doppler=-freq*vr/2.99792458e5;                 //!One-way Doppler
    double el,eb;
    coord(0.0,0.0,-1.570796,1.161639,RAMoon/rad,DecMoon/rad,el,eb);
    int longecl_half=int(rad*el/2.0);
    if (longecl_half<0 || longecl_half>179) longecl_half=179;//c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    double t144=nt144[longecl_half];
    double tsky=(t144-2.7)*pow((144.0/nfreq),2.6) + 2.7;   //tsky=(t144-2.7)*(144.0/nfreq)**2.6 + 2.7      !Tsky for obs freq     //!Tsky for obs freq

    //!  t408=ftsky(ldeg,bdeg)                         //!Read sky map
    //!  tsky=t408*(408.0/nfreq)**2.6                  //!Tsky for obs freq
    //!  if(ltsky.and.(tsky.lt.3.0)) tsky=3.0          //!Minimum = 3 Kelvin

    xdop[NStation]=doppler;
    if (NStation==2)
    {
        HisGrid=Mygrid;
        goto c900;
    }

    doppler00=2.0*xdop[1];
    //!      if(mode.eq.2 .or. mode.eq.5) doppler=xdop(1)+xdop(2)
    doppler=xdop[1]+xdop[2];
    if (mode==3) doppler=2.0*xdop[1];            //!Echo mode
    if (dist<0.0001)	dist=0.0001;//1.78 div by0
    dbMoon=-40.0*log10(dist/356903.0);
    sd=16.23*370152.0/dist;

    //!      if(NStation.eq.1 .and. MoonDX.ne.0 .and.
    //!     +    (mode.eq.2 .or. mode.eq.5)) then
    if (NStation==1 && MoonDX!=0)
    {
        poloffset=fmod(poloffset2-poloffset1+720.0,180.0);
        if (poloffset>90.0) poloffset=poloffset-180.0;
        double x1=fabs(cos(2.0*poloffset/rad));
        if (x1<0.056234) x1=0.056234;  //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
        xnr=-20.0*log10(x1);
        //qDebug()<<"xnr1=============="<<x1;
        if ((int)HisGrid[0].toLatin1()<(int)'A' || (int)HisGrid[0].toLatin1()>(int)'Z') xnr=0.0;//if(HisGrid(1:1).lt.'A' .or. HisGrid(1:1).gt.'Z') xnr=0
        ///qDebug()<<"xnr2=============="<<xnr<<HisGrid[0]<<(int)HisGrid[0].toLatin1()<<(int)'A';
    }

    tr=80.0 ;                             //!Good preamp
    tskymin=13.0*pow((408.0/nfreq),2.6);//tskymin=13.0*(408.0/nfreq)**2.6      //!Cold sky temperature
    tsysmin=tskymin+tr;
    if (tsysmin==0.0) tsysmin=0.000001;//1.78 div by0
    tsys=tsky+tr;
    if (tsys<0.000001) tsys=0.000001;//1.78 div by0
    Dgrd=-10.0*log10(tsys/tsysmin) + dbMoon;

c900:

    AzMoon0=AzMoon;
    ElMoon0=ElMoon;
    ntsky=int(tsky);

    auxHA = 15.0*(LST-auxra);                      //!HA in degrees
    pi=3.14159265;
    pio2=0.5*pi;
    coord(pi,pio2-(lat/rad),0.0,lat/rad,auxHA*pi/180.0,auxdec/rad,AzAux,ElAux);
    AzAux=AzAux*rad;
    ElAux=ElAux*rad;

}

void HvAstroDataW::SetRaDec(QString radec)
{
    QStringList l_radec = radec.split("#");
    if (l_radec.count()==2)
    {
        le_auxra->setText(l_radec.at(0));
        le_auxdec->setText(l_radec.at(1));
    }
}
QString HvAstroDataW::GetRaDec()
{
    QString res =le_auxra->text()+"#"+le_auxdec->text();
    return res;
}
void HvAstroDataW::SetTx(bool f)
{
	f_txrx = f;
}
void HvAstroDataW::astro0()
{
    //QString hisgrid = "JO90NH";
    //QString grid = "KN23SF";
    //int nfreq = 144; //50,70,144.....
    QDateTime utc_t = QDateTime::currentDateTimeUtc();

    int nyear = utc_t.toString("yyyy").toInt();
    int month = utc_t.toString("MM").toInt();
    int nday  = utc_t.toString("dd").toInt();
    int hh    = utc_t.toString("hh").toInt();
    int mm    = utc_t.toString("mm").toInt();
    int ss    = utc_t.toString("ss").toInt();

    QString str_auxra = le_auxra->text();
    QString str_auxdec =le_auxdec->text();
    double auxra = 0.0;  // from ini =0
    if (!str_auxra.contains(":"))
        auxra = str_auxra.toDouble();
    else
    {
        QStringList l_hms = str_auxra.split(":");
        if (l_hms.count()>2)
        {
            double ih = l_hms.at(0).toInt();
            double im = l_hms.at(1).toInt();
            double is = l_hms.at(2).toInt();
            auxra=ih + im/60.0 + is/3600.0;
        }
    }

    double auxdec= 0.0;  // from ini =0
    if (!str_auxdec.contains(":"))
        auxdec = str_auxdec.toDouble();
    else
    {
        QStringList l_hms = str_auxdec.split(":");
        if (l_hms.count()>2)
        {
            double id = l_hms.at(0).toInt();
            double im = l_hms.at(1).toInt();
            double is = l_hms.at(2).toInt();
            auxdec=fabs(id) + im/60.0 + is/3600.0;
            if (str_auxdec.mid(0,1)=="-") auxdec=-auxdec;
        }
    }
    //qDebug()<<str_auxra<<auxra<<str_auxdec<<auxdec;

    double uth8=(double)hh + (double)mm/60.0 + (double)ss/3600.0;
    //qDebug()<<uth8;

    double uth = uth8;
    double AzSun = 0.0;
    double ElSun = 0.0;
    double AzMoon = 0.0;
    double ElMoon = 0.0;
    int ntsky = 0;
    double doppler00 = 0.0;
    double doppler = 0.0;
    double dbMoon = 0.0;
    double RAMoon = 0.0;
    double DecMoon = 0.0;
    double HA = 0.0;
    double Dgrd = 0.0;
    double sd = 0.0;
    double poloffset = 0.0;
    double xnr = 0.0;
    double AzAux = 0.0;
    double ElAux = 0.0;
    double day = 0.0;
    double xlon2 = 0.0;
    double xlat2 = 0.0;
    double xlon1 = 0.0;
    double xlat1 = 0.0;
    double xlst = 0.0;
    int nmode = 2;// 2=jt65
    double AzMoonB8 = 0.0;
    double ElMoonB8 = 0.0;
    //double uth8z = 0.0;

    astro(nyear,month,nday,uth,s_nfreq,s_hisgrid,2,nmode,1,AzSun,ElSun,AzMoon,ElMoon,ntsky,doppler00,doppler,
          dbMoon,RAMoon,DecMoon,HA,Dgrd,sd,poloffset,xnr,auxra,auxdec,AzAux,ElAux,day,xlon2,xlat2,xlst);
    AzMoonB8=AzMoon;
    ElMoonB8=ElMoon;
    astro(nyear,month,nday,uth,s_nfreq,s_mygrid,1,nmode,1,AzSun,ElSun,AzMoon,ElMoon,ntsky,doppler00,doppler,
          dbMoon,RAMoon,DecMoon,HA,Dgrd,sd,poloffset,xnr,auxra,auxdec,AzAux,ElAux,day,xlon1,xlat1,xlst);

    int ndop=int(doppler);
    emit EmitAstroData(AzMoon,ElMoon,ndop,Dgrd);
    //qDebug()<<"NO ALLLL";

    if (!isVisible()) return;

    //qDebug()<<"xlat1="<<xlon1<<xlat1<<xlon2<<xlat2<<day;
    //xlon1=-xlon1;
    //xlon2=-xlon2;
    double day8=day;
    //double xlst8=xlst;
    double xl1,b1,xl2,b2,xl1a,b1a,xl2a,b2a;
    tm2(day8,xlat1,xlon1,xl1,b1);
    tm2(day8,xlat2,xlon2,xl2,b2);
    tm2(day8+1.0/1440.0,xlat1,xlon1,xl1a,b1a);
    tm2(day8+1.0/1440.0,xlat2,xlon2,xl2a,b2a);
    //qDebug()<<"gridRAMoon DecMoon="<<xl1<<b1<<xl2<<b2<<xl1a<<b1a<<xl2a<<b2a<<day;

    double fghz=0.001*(double)s_nfreq;
    if (fghz<=0.0) fghz=0.000001;//1.78 div by0
    double dldt1=DEGS*(xl1a-xl1);
    double dbdt1=DEGS*(b1a-b1);
    double dldt2=DEGS*(xl2a-xl2);
    double dbdt2=DEGS*(b2a-b2);
    double rate1=2.0*sqrt(dldt1*dldt1 + dbdt1*dbdt1);
    double width1=0.5*6741.0*fghz*rate1;
    double rate2=sqrt((dldt1+dldt2)*(dldt1+dldt2) + (dbdt1+dbdt2)*(dbdt1+dbdt2));
    double width2=0.5*6741.0*fghz*rate2;

    double fbend=0.7;
    double a2=0.0045*log(fghz/fbend)/log(1.05);
    if (fghz<fbend) a2=0.0;  //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    double f50=0.19 * pow((fghz/fbend),a2);
    if (f50>1.0) f50=1.0;
    double w501=f50*width1;
    double w502=f50*width2;

    /*RaAux8=auxra
    DecAux8=auxdec
    AzSun8=AzSun
    ElSun8=ElSun
    AzMoon8=AzMoon
    ElMoon8=ElMoon
    dbMoon8=dbMoon
    RAMoon8=RAMoon/15.0
    DecMoon8=DecMoon
    HA8=HA
    Dgrd8=Dgrd
    sd8=sd
    poloffset8=poloffset
    xnr8=xnr
    AzAux8=AzAux
    ElAux8=ElAux*/
    RAMoon=RAMoon/15.0;
    int irah = int(RAMoon);
    int iram = int(60.0*(RAMoon-irah));

    int irahx = int(auxra);
    int iramx = int(60.0*(auxra-irahx));

    int ndop00=int(doppler00);

    if (uth8z==0.0)
    {
        uth8z=uth8-1.0/3600.0;
        dopplerz=doppler;
        doppler00z=doppler00;
    }

    double dt=60.0*(uth8-uth8z);
    if (dt<=0.0) dt=1.0/60.0;  //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
    double dfdt=(doppler-dopplerz)/dt;
    double dfdt0=(doppler00-doppler00z)/dt;

    //qDebug()<<"ssss dx="<<width1<<"ssss self="<<width2;
    //qDebug()<<"w50 dx="<<w501<<"w50 self="<<w502;
    //qDebug()<<"ALLLL";

    uth8z=uth8;
    dopplerz=doppler;
    doppler00z=doppler00;

    moon_az->setText(QString("%1").arg(AzMoon,0,'f',2));
    moon_el->setText(QString("%1").arg(ElMoon,0,'f',2));
    moondx_az->setText(QString("%1").arg(AzMoonB8,0,'f',2));
    moondx_el->setText(QString("%1").arg(ElMoonB8,0,'f',2));
    sun_az->setText(QString("%1").arg(AzSun,0,'f',2));
    sun_el->setText(QString("%1").arg(ElSun,0,'f',2));
    sou_az->setText(QString("%1").arg(AzAux,0,'f',2));
    sou_el->setText(QString("%1").arg(ElAux,0,'f',2));
    dop_dx->setText(QString("%1").arg(ndop));
    dop_self->setText(QString("%1").arg(ndop00));
    dfdt_dx->setText(QString("%1").arg(dfdt,0,'f',2));
    dfdt_self->setText(QString("%1").arg(dfdt0,0,'f',2));
    sp_dx->setText(QString("%1").arg(width2,0,'f',1));
    sp_self->setText(QString("%1").arg(width1,0,'f',1));
    w50_dx->setText(QString("%1").arg(w502,0,'f',1));
    w50_self->setText(QString("%1").arg(w501,0,'f',1));
    moon_ra->setText(QString("%1").arg(irah,2,10, QChar('0'))+":"+QString("%1").arg(iram,2,10, QChar('0')));
    moon_dec->setText(QString("%1").arg(DecMoon,0,'f',2));
    sou_ra->setText(QString("%1").arg(irahx,2,10, QChar('0'))+":"+QString("%1").arg(iramx,2,10, QChar('0')));
    sou_dec->setText(QString("%1").arg(auxdec,0,'f',2));
    freq_0->setText(QString("%1").arg(s_nfreq));
    tasky_0->setText(QString("%1").arg(ntsky));
    mnr_0->setText(QString("%1").arg(xnr,0,'f',1));
    dgrd_0->setText(QString("%1").arg(Dgrd,0,'f',1));
    //dpol_0->setText(QString("%1").arg(poloffset,0,'f',1));
    dpol_0->setText(QString("%1").arg((int)poloffset));
    sd_0->setText(QString("%1").arg(sd,0,'f',2));
    
    
    QFile file(azel_path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return;    
    QString hms =  utc_t.toString("hh:mm:ss");
    QString am8 = QString("%1").arg(AzMoon,0,'f',1);
    QString em8 = QString("%1").arg(ElMoon,0,'f',1);
    QString as8 = QString("%1").arg(AzSun,0,'f',1);
    QString es8 = QString("%1").arg(ElSun,0,'f',1);
    QString aau = "0.0";//QString("%1").arg(0.0,0,'f',1);
    QString eau = "0.0";//QString("%1").arg(0.0,0,'f',1);    
    QString frq = QString("%1").arg(s_nfreq);    
    QString ndp = QString("%1").arg(doppler,0,'f',1);
    //QString ndp = QString("%1").arg((double)ndop,0,'f',1);
    QString dft = QString("%1").arg(dfdt,0,'f',2);    
    QString ndp0 = QString("%1").arg(doppler00,0,'f',1);
    //QString ndp0 = QString("%1").arg((double)ndop00,0,'f',1);
    QString dft0 = QString("%1").arg(dfdt0,0,'f',2);    
    QString rt = "R";//or T TX RX
    if (f_txrx) rt = "T";
	QString sout;
	sout.append(hms+","+am8.rightJustified(5,' ')+","+em8.rightJustified(5,' ')+",Moon\n"); 
    sout.append(hms+","+as8.rightJustified(5,' ')+","+es8.rightJustified(5,' ')+",Sun\n");
    sout.append(hms+","+aau.rightJustified(5,' ')+","+eau.rightJustified(5,' ')+",Source\n");
    sout.append(frq.rightJustified(5,' ')+","+
    			   ndp.rightJustified(8,' ')+","+dft.rightJustified(8,' ')+","+
    			   ndp0.rightJustified(8,' ')+","+dft0.rightJustified(8,' ')+",Doppler, "+rt+"\n");    		   			   
    /*QStringList l_sout = sout.split("\n");
    for (int i = 0; i < l_sout.count()-1; ++i)
    {
    	qDebug() << l_sout.at(i);
   	}
    qDebug() <<"----------------------------------------------------------";*/     
    QTextStream out(&file);	
    out << sout;	    
    file.close();
}
