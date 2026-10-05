/* MSHV
 *
 * By Hrisimir Hristov - LZ2HV
 * May be used under the terms of the GNU General Public License (GPL)
 */
#ifndef JRC_DEF_H
#define JRC_DEF_H

#include "../rigdef.h"
#define JRC_ID 6
#define JRC_COUNT 4
static RigSet rigs_jrc[JRC_COUNT] =
    {
        {"NRD-525",RIG_PTT_NONE,RIG_PORT_SERIAL,300,4800,8,1,PAR_NONE,FLOW_OFF,0,20,1000,0},
		{"NRD-535D",RIG_PTT_NONE,RIG_PORT_SERIAL,4800,4800,8,1,PAR_NONE,FLOW_OFF,0,20,200,3},
		{"NRD-545DSP",RIG_PTT_NONE,RIG_PORT_SERIAL,4800,4800,8,1,PAR_NONE,FLOW_OFF,0,0,200,3},
		{"JST-245",RIG_PTT_NONE,RIG_PORT_SERIAL,4800,4800,8,1,PAR_NONE,FLOW_OFF,0,20,200,3},
    };
#endif
