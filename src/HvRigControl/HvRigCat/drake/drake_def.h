/* MSHV
 *
 * By Hrisimir Hristov - LZ2HV
 * May be used under the terms of the GNU General Public License (GPL)
 */

#ifndef DRAKE_DEF_H
#define DRAKE_DEF_H

#include "../rigdef.h"
#define DRAKE_ID 7
#define DRAKE_COUNT 2
static RigSet rigs_drake[DRAKE_COUNT] =
    {
        {"R-8A",RIG_PTT_NONE,RIG_PORT_SERIAL,9600,9600,8,1,PAR_NONE,FLOW_HARDWARE,0,1,200,3},
		{"R-8B",RIG_PTT_NONE,RIG_PORT_SERIAL,9600,9600,8,1,PAR_NONE,FLOW_HARDWARE,0,1,200,3},
    };
#endif
