/* MSHV
 *
 * By Hrisimir Hristov - LZ2HV
 * May be used under the terms of the GNU General Public License (GPL)
 */
#ifndef ELECRAFT_DEF_H
#define ELECRAFT_DEF_H

#include "../rigdef.h"
#define ELECRAFT_ID 3
#define ELECRAFT_COUNT 3
static RigSet rigs_elecraft[ELECRAFT_COUNT] =
    {
        {"K2",RIG_PTT_RIG,RIG_PORT_SERIAL,4800,4800,8,2,PAR_NONE,FLOW_OFF,0,100,600,3},
        {"K3/KX3",RIG_PTT_RIG,RIG_PORT_SERIAL,4800,4800,8,2,PAR_NONE,FLOW_OFF,0,100,600,3},
        {"XG3",RIG_PTT_RIG,RIG_PORT_SERIAL,9600,9600,8,1,PAR_NONE,FLOW_OFF,0,25,25,3},
    };
#endif
