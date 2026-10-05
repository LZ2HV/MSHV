/* MSHV Qth Loc Distance Validations
 * Copyright 2015 Hrisimir Hristov, LZ2HV
 * May be used under the terms of the GNU General Public License (GPL)
 */
#include "hvqthloc.h"

static double eltab[22]=
    {
        18.0,15.0,13.0,11.0,9.0,8.0,7.0,6.0,5.3,4.7,4.0,3.3,2.7,2.0,1.5,1.0,0.8,0.6,0.4,0.2,0.0,0.0
    };
static double daztab[22]=
    {
        21.0,18.0,16.0,15.0,14.0,13.0,12.0,11.0,10.7,10.3,10.0,10.0,10.0,10.0,10.0,10.0,10.0,9.0,9.0,9.0,8.0,8.0
    };
//data eltab/18.,15.,13.,11.,9.,8.,7.,6.,5.3,4.7,4.,3.3,2.7,          &
//       2.,1.5,1.,0.8,0.6,0.4,0.2,0.0,0.0/

#define PI_h 3.14159265359//3.141592654
#define EARTH_RADIUS 6378.2064//6378.0

bool HvQthLoc::isValidCallsign(QString callsign)
{
    bool valid = true;
    QRegExp rx("(\\d+)");

    if (callsign.count()< 3) // A callsign must be longer than two characters
        valid = false;
    else
    {
        if ((callsign.at(0) == 'Q') || (callsign.at(0) == '0')) // Do not accept callsigns begining with Q or 0
            valid = false;
        else
        {
            if (callsign.contains("//")) // Do not accept callsigns with //
                valid = false;
            else
                if ((callsign.at(0) == '/') || (callsign.at(callsign.count() - 1) == '/')) // Do not accept callsigns with / in the beginning or at the end
                    valid = false;
                else
                    //if (DigitPresent(callsign) == false)
                    if (!callsign.contains(rx))
                        valid = false;
                    else
                    {
                        foreach (QChar Char , callsign)
                        {
                            if (((Char >= 'A') && (Char <= 'Z')) ||
                                    ((Char >= 'a') && (Char <= 'z')) ||
                                    ((Char >= '0') && (Char <= '9')) ||
                                    (Char == '/'))
                            {
                            	// noting
                            }
                            else
                                valid = false;
                        }
                    }
        }
    }
    return valid;
}
QString HvQthLoc::CorrectLocator(QString corr_loc)
{
    if (corr_loc.length() == 4) corr_loc = corr_loc +"LM";
    if (corr_loc.length() == 5) corr_loc = corr_loc +"M";
    //if (corr_loc.length() == 4) corr_loc = corr_loc +"MM";// "MM"
    //if (corr_loc.length() == 5) corr_loc = corr_loc +"M";
    return corr_loc;
}
bool HvQthLoc::isValidLocator(const QString& tlocator)
{
    /* -------------- Subroutine -----------------------
          Check valid locator
          Input : char *locator = 4 or 6 characters word wide locator.
          returned value ==  -1 No error. (Valid locator).
          returned value ==  0 Error.   (Invalid locator).
          Note: also string "END" is considered a valid locator, but returned value is -2.
       ------------------------------------------------- */
    //cout << "Locator::isValidLocator: " << tlocator << endl;
    QChar theChar;
    //int lenght_of_locator;
    QString testLocator = CorrectLocator(tlocator.toUpper());

    if (testLocator.length() != 6)
    {
        return false;
    }
    else
    {
        theChar = testLocator.at(0);
        //qDebug()<<bool(theChar>'A');
        if (!theChar.isLetter())
        {  //First letter is not a valid letter
            return false;
        }
        if (theChar>'R') //A B C D E F G H I J K L M N O P Q R S T U V W X Y Z
        {  //First letter is not a valid letter
            return false;
        }
        theChar = testLocator.at(1);
        if (!theChar.isLetter())
        {  //Second letter is not a valid letter
            return false;
        }
        if (theChar>'R')
        {  //Second letter is not a valid letter
            return false;
        }
        theChar = testLocator.at(2);
        if (!theChar.isDigit())
        {  //Second letter is not a number
            return false;
        }
        theChar = testLocator.at(3);
        if (!theChar.isDigit())
        {  //Second letter is not a number
            return false;
        }
        theChar = testLocator.at(4);
        if (!theChar.isLetter())
        {  //First letter is not a valid letter
            return false;
        }
        if (theChar>'X')
        {  //First letter is not a valid letter
            return false;
        }
        theChar = testLocator.at(5);
        if (!theChar.isLetter())
        {  //Second letter is not a valid letter
            return false;
        }
        if (theChar>'X')
        {  //Second letter is not a valid letter
            return false;
        }
    }
    return true;
}
int HvQthLoc::getDistanceKilometres(const double lon1, const double lat1, const double lon2, const double lat2)
{
    double lo1,la1,lo2,la2;
    lo1=lon1*PI_h/180.0;   // Convert degrees to radians
    la1=lat1*PI_h/180.0;
    lo2=lon2*PI_h/180.0;
    la2=lat2*PI_h/180.0;
    // Calculates distance in km
    return  (int)(acos(cos(la1)*cos(lo1)*cos(la2)*cos(lo2)+cos(la1)*sin(lo1)*cos(la2)*sin(lo2)+sin(la1)*sin(la2)) * EARTH_RADIUS);
}
int HvQthLoc::getDistanceMilles(const double lon1, const double lat1, const double lon2, const double lat2)
{
    return  (int)(getDistanceKilometres(lon1, lat1, lon2, lat2)/1.609344) ;
}
double HvQthLoc::getLat(const QString& tlocator)
{
    return -90+((double)tlocator.at(1).toLatin1()-65.0)*10.0 + ((double)tlocator.at(3).toLatin1()-48.0) +((double)tlocator.at(5).toLatin1()-64.5)/24.0;
}
double HvQthLoc::getLon(const QString& tlocator)
{
    return -(-180+((double)tlocator.at(0).toLatin1()-65.0)*20.0 + ((double)tlocator.at(2).toLatin1()-48.0)*2.0 +((double)tlocator.at(4).toLatin1()-64.5)/12.0);
}
int HvQthLoc::getBeam(const double lon1, const double lat1, const double lon2, const double lat2)
{
    double lon_a,lat_a,lon_b,lat_b, bearing;
    lon_a=lon1*PI_h/180.0;   // Convert degrees to radians
    lat_a=lat1*PI_h/180.0;
    lon_b=lon2*PI_h/180.0;
    lat_b=lat2*PI_h/180.0;

    //earing_Distance( double lon_a, double lat_a, /* Lon/Lat of point A */
    //                  double lon_b, double lat_b, /* Lon/Lat of point B */
    //                  double *bearing, double *distance )/* From A to B */
    //{
    double  //gc_arc,
    cos_gc_arc,       /* Great circle arc   A to B */
    cos_bearing, sin_bearing, /* cos/sin of bearing A to B */
    lon_diff;                 /* Difference in longitude of B from A */
    //gc_arc = 0.0;

    /* Longitude differnce of B from A */
    lon_diff = lon_b - lon_a;

    /* Calculate great circle distance A to B */
    cos_gc_arc = cos(lon_diff)*cos(lat_a)*cos(lat_b) + sin(lat_a)*sin(lat_b);
    //gc_arc = acos( cos_gc_arc );

    /* Distance in km */
    //  *distance = eradius * gc_arc;

    /* Calculate bearing A to B */
    cos_bearing  = sin(lat_b) - sin(lat_a) * cos_gc_arc;
    sin_bearing  = sin(lon_diff) * cos(lat_a) * cos(lat_b);
    bearing = atan2(sin_bearing, cos_bearing);

    /* Correct negative (anticlockwise) bearings */

    if ( bearing < 0.0 )
        bearing = (2*PI_h) + bearing;
    bearing = 360-(180/PI_h*bearing);

    /* Convert to degrees */
    return (int)bearing;
}
QStringList HvQthLoc::getElAndHot(double UTChours,int dist_km,int az, double dlong1,double dlat1,double dlong2,double dlat2)
{
    //QString res = "Hot B: ";
    QStringList res;
    //res << "0" << "Hot B: ";
    double el=0.0;
    double HotA =  0.0;
    double HotB =  0.0;
    //double u = 0.0;
    bool HotABetter=true;

    if (dist_km!=0)
    {
    	double u = 0.0;
        //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
        double ndkm=(double)dist_km/100.0;
        int j=int(ndkm-4.0);
        if (j<1) j=1;
        if (j>21)j=21;
        if (dist_km<500.0)
            el=18.0;
        else
        {
            u=((double)dist_km-100.0*ndkm)/100.0;
            //El=(1.0-u)*eltab(j) + u*eltab(j+1)
            el=(1.0-u)*eltab[j-1] + u*eltab[j+0];
        }

        //daz=daztab(j) + u * (daztab(j+1)-daztab(j))
        double daz=daztab[j-1] + u * (daztab[j+0]-daztab[j-1]);
        //Dmiles=Dkm/1.609344

        double tmid=fmod(UTChours-0.5*(dlong1+dlong2)/15.0+48.0,24.0);
        bool IamEast=false;
        if (dlong1<dlong2) IamEast=true;
        if (dlong1==dlong2 && dlat1>dlat2) IamEast=false;

        int azEast;//baz;
        if (az<180)
            azEast=az+180;
        else
            azEast=az-180;

        if (IamEast) azEast=az;
        if ((azEast>=45.0 && azEast<135.0) || (azEast>=225.0 && azEast<315.0))
        {
            //! The path will be taken as "east-west".
            HotABetter=true;
            if (fabs(tmid-6.0)<6.0) HotABetter=false;
            if ((dlat1+dlat2)/2.0 < 0.0)
            {
                //if((dlat1+dlat2)/2.0 .lt. 0.0) HotABetter=.not.HotABetter
                if (!HotABetter)
                    HotABetter = true;
                else
                    HotABetter = false;
            }
        }
        else
        {
            //! The path will be taken as "north-south".
            HotABetter=false;
            if (fabs(tmid-12.0)<6.0) HotABetter=true;
        }
        if (IamEast)
        {
            HotA = az - daz;
            HotB = az + daz;
        }
        else
        {
            HotA = az + daz;
            HotB = az - daz;
        }
        //c++   ==.EQ. !=.NE. >.GT. <.LT. >=.GE. <=.LE.
        if (HotA<0.0)   HotA=HotA+360.0;
        if (HotA>360.0) HotA=HotA-360.0;
        if (HotB<0.0)   HotB=HotB+360.0;
        if (HotB>360.0) HotB=HotB-360.0;

        //900 continue
        //naz=nint(Az)
        //nel=nint(el)
        //nDmiles=nint(Dmiles)
        //nDkm=nint(Dkm)
        //int nHotAz=(int)HotB;
        //res.at(1).append(QString("%1").arg((int)HotB));
        //bool nHotABetter=false;
    }
    res << QString("%1").arg((int)el);
    if (HotABetter) res << "Hot A: "+(QString("%1").arg((int)HotA));
    else res << "Hot B: "+(QString("%1").arg((int)HotB));

    return res;
}