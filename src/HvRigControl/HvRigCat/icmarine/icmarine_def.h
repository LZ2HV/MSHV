/* MSHV
 *
 * By Hrisimir Hristov - LZ2HV
 * May be used under the terms of the GNU General Public License (GPL)
 */
#ifndef ICMARINE_DEF_H
#define ICMARINE_DEF_H

#include "../rigdef.h"
#define ICMARINE_ID 8
#define ICMARINE_COUNT 3
static RigSet rigs_icmarine[ICMARINE_COUNT] =
    {
        {"IC-M802",RIG_PTT_RIG,RIG_PORT_SERIAL,4800,4800,8,1,PAR_NONE,FLOW_OFF,0,0,500,0},
        {"IC-M710",RIG_PTT_RIG,RIG_PORT_SERIAL,4800,4800,8,1,PAR_NONE,FLOW_OFF,0,0,500,0},
        {"IC-M700PRO",RIG_PTT_RIG,RIG_PORT_SERIAL,4800,4800,8,1,PAR_NONE,FLOW_OFF,0,0,500,0},
    };
#endif
