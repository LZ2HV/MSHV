/* MSHV Part from RigControl
 * Copyright 2019 Hrisimir Hristov, LZ2HV
 * May be used under the terms of the GNU General Public License (GPL)
 */
#include "../../../config.h"
#if defined _WIN32_
#ifndef OMNIRIGHV_DEF_H
#define OMNIRIGHV_DEF_H

#include "../rigdef.h"
#define OMNIRIGHV_ID 11
#define OMNIRIGHV_COUNT 2
static RigSet rigs_omnirighv[OMNIRIGHV_COUNT] =
    {
        {"Rig 1",RIG_PTT_RIG,RIG_PORT_SERIAL,4800,4800,8,2,PAR_NONE,FLOW_OFF,0,0,200,3},
        {"Rig 2",RIG_PTT_RIG,RIG_PORT_SERIAL,4800,4800,8,2,PAR_NONE,FLOW_OFF,0,0,200,3},
        /*{"Rig 3",RIG_PTT_RIG,RIG_PORT_SERIAL,4800,4800,8,2,PAR_NONE,FLOW_OFF,0,0,200,3},
        {"Rig 4",RIG_PTT_RIG,RIG_PORT_SERIAL,4800,4800,8,2,PAR_NONE,FLOW_OFF,0,0,200,3},*/
    };
#endif
#endif