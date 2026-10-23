/* MSHV
 *
 * By Hrisimir Hristov - LZ2HV
 * May be used under the terms of the GNU General Public License (GPL)
 */

#ifndef HVQTHLOC_H
#define HVQTHLOC_H

#include <QStringList>
#include <QMessageBox>
#include <math.h>	//los fabs
//#include <QVBoxLayout>
//#include <QLineEdit>
//#include <QPushButton>
//#include <QRadioButton>
//#define PI 				3.14159265359//3.141592654
//#define EARTH_RADIUS 	6378.2064//6378.0

//#include <QtGui>

class HvQthLoc
{
public:
    bool isValidCallsign(QString callsign);
    QString CorrectLocator(QString);
    bool isValidLocator(const QString& tlocator);
    double getLon(const QString& tlocator);
    double getLat(const QString& tlocator);
    int getDistanceKilometres(const double lon1, const double lat1, const double lon2, const double lat2);
    int getDistanceMilles(const double lon1, const double lat1, const double lon2, const double lat2);
    int getBeam(const double lon1, const double lat1, const double lon2, const double lat2);
    QStringList getElAndHot(double UTChours,int dist_km,int az, double dlong1,double dlat1,double dlong2,double dlat2);
};
#endif