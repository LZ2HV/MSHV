/* MSHV
 *
 * By Hrisimir Hristov - LZ2HV
 * May be used under the terms of the GNU General Public License (GPL)
 */
#ifndef SDRS_DEF_H
#define SDRS_DEF_H

#include "../rigdef.h"
#define SDRS_ID 12
#define SDRS_COUNT 11 
static RigSet rigs_sdrs[SDRS_COUNT] =
    {
    	{"FlexRadio/Apache PowerSDR",RIG_PTT_RIG,RIG_PORT_SERIAL,4800,9600,8,1,PAR_NONE,FLOW_OFF,0,0,200,3},
    	{"Thetis",RIG_PTT_RIG,RIG_PORT_SERIAL,4800,9600,8,1,PAR_NONE,FLOW_OFF,0,0,200,3},
        {"FlexRadio SmartSDR",RIG_PTT_RIG,RIG_PORT_SERIAL,4800,9600,8,1,PAR_NONE,FLOW_OFF,0,0,200,3},     		
        {"FlexRadio SmartSDR Slice A Ser",RIG_PTT_RIG,RIG_PORT_SERIAL,4800,9600,8,1,PAR_NONE,FLOW_OFF,0,0,200,3},
        {"FlexRadio SmartSDR Slice B Ser",RIG_PTT_RIG,RIG_PORT_SERIAL,4800,9600,8,1,PAR_NONE,FLOW_OFF,0,0,200,3},
        {"FlexRadio SmartSDR Slice C Ser",RIG_PTT_RIG,RIG_PORT_SERIAL,4800,9600,8,1,PAR_NONE,FLOW_OFF,0,0,200,3},
        {"FlexRadio SmartSDR Slice D Ser",RIG_PTT_RIG,RIG_PORT_SERIAL,4800,9600,8,1,PAR_NONE,FLOW_OFF,0,0,200,3},
        {"FlexRadio SmartSDR Slice E Ser",RIG_PTT_RIG,RIG_PORT_SERIAL,4800,9600,8,1,PAR_NONE,FLOW_OFF,0,0,200,3},
        {"FlexRadio SmartSDR Slice F Ser",RIG_PTT_RIG,RIG_PORT_SERIAL,4800,9600,8,1,PAR_NONE,FLOW_OFF,0,0,200,3},
        {"FlexRadio SmartSDR Slice G Ser",RIG_PTT_RIG,RIG_PORT_SERIAL,4800,9600,8,1,PAR_NONE,FLOW_OFF,0,0,200,3},
        {"FlexRadio SmartSDR Slice H Ser",RIG_PTT_RIG,RIG_PORT_SERIAL,4800,9600,8,1,PAR_NONE,FLOW_OFF,0,0,200,3},        
    };
#endif
