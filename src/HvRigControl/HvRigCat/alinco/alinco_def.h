/* MSHV
 *
 * By Hrisimir Hristov - LZ2HV
 * May be used under the terms of the GNU General Public License (GPL)
 */
#ifndef ALINCO_DEF_H
#define ALINCO_DEF_H

#include "../rigdef.h"
#define ALINCO_ID 5
#define ALINCO_COUNT 1
static RigSet rigs_alinco[ALINCO_COUNT] =
    {
        {"DX-77",RIG_PTT_NONE,RIG_PORT_SERIAL,9600,9600,8,2,PAR_NONE,FLOW_OFF,0,0,200,3},
    };
#endif
