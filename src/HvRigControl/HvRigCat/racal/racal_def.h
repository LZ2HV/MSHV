/* MSHV
 *
 * By Hrisimir Hristov - LZ2HV
 * May be used under the terms of the GNU General Public License (GPL)
 */

#ifndef RACAL_DEF_H
#define RACAL_DEF_H

#include "../rigdef.h"
#define RACAL_ID 10
#define RACAL_COUNT 2
static RigSet rigs_racal[RACAL_COUNT] =
    {
        {"RA6790/GM",RIG_PTT_NONE,RIG_PORT_SERIAL,300,9600,7,2,PAR_EVEN,FLOW_XONXOFF,0,10,2000,3},
		{"RA3702",RIG_PTT_NONE,RIG_PORT_SERIAL,300,9600,7,1,PAR_EVEN,FLOW_XONXOFF,0,0,1000,2},
    };
#endif
