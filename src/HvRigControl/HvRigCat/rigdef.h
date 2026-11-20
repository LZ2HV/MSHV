#ifndef RIGDEF_H
#define RIGDEF_H
 
typedef enum {
  RIG_PTT_OFF = 0,		/*!< PTT desactivated */
  RIG_PTT_ON,			/*!< PTT activated */
  RIG_PTT_ON_MIC,		/*!< PTT Mic only, fallbacks on RIG_PTT_ON if unavailable */
  RIG_PTT_ON_DATA		/*!< PTT Data (Mic-muted), fallbacks on RIG_PTT_ON if unavailable */
} ptt_t;

typedef enum rig_port_e {
    RIG_PORT_NONE = 0,		/*!< No port */
    RIG_PORT_SERIAL,		/*!< Serial */
    RIG_PORT_NETWORK,		/*!< Network socket type */
    RIG_PORT_DEVICE,		/*!< Device driver, like the WiNRADiO */
    RIG_PORT_PACKET,		/*!< AX.25 network type, e.g. SV8CS protocol */
    RIG_PORT_DTMF,		/*!< DTMF protocol bridge via another rig, eg. Kenwood Sky Cmd System */
    RIG_PORT_ULTRA,		/*!< IrDA Ultra protocol! */
    RIG_PORT_RPC,			/*!< RPC wrapper */
    RIG_PORT_PARALLEL,		/*!< Parallel port */
    RIG_PORT_USB,			/*!< USB port */
    RIG_PORT_UDP_NETWORK,		/*!< UDP Network socket type */
    RIG_PORT_CM108		/*!< CM108 GPIO */
} rig_port_t;

typedef enum {
  RIG_PTT_NONE = 0,		/*!< No PTT available */
  RIG_PTT_RIG,			/*!< Legacy PTT */
  RIG_PTT_SERIAL_DTR,		/*!< PTT control through serial DTR signal */
  RIG_PTT_SERIAL_RTS,		/*!< PTT control through serial RTS signal */
  RIG_PTT_PARALLEL,		/*!< PTT control through parallel port */
  RIG_PTT_RIG_MICDATA,		/*!< Legacy PTT, supports RIG_PTT_ON_MIC/RIG_PTT_ON_DATA */
  RIG_PTT_CM108		/*!< PTT control through CM108 GPIO pin */
} ptt_type_t;

#include "../qexsp_1_2rc/qextserialport.h"
/*enum ParityType_// napravo ot sorsa na qtserialport
{
    PAR_NONE,
    PAR_ODD,
    PAR_EVEN,
    PAR_MARK,               //WINDOWS ONLY
    PAR_SPACE
};

enum FlowType_
{
    FLOW_OFF,
    FLOW_HARDWARE,
    FLOW_XONXOFF
};*/

typedef struct
{
    QString       name;
    ptt_type_t    ptt_type;
    rig_port_t    port_type;
    int           serial_rate_min;		/*!< Minimum serial speed. */
    int           serial_rate_max;		/*!< Maximum serial speed. */
    int           serial_data_bits;		/*!< Number of data bits. */
    int           serial_stop_bits;		/*!< Number of stop bits. */
    ParityType    serial_parity;
    FlowType      serial_flow_type;
    int write_delay;		/*!< Delay between each byte sent out, in mS */
    int post_write_delay;		/*!< Delay between each commands send out, in mS */
    int timeout;			/*!< Timeout, in mS */
    int retry;			/*!< Maximum number of retries if command fails, 0 to disable */
}
RigSet;
 
typedef enum 
{
	GET_SETT = 0,
    SET_PTT,          
    SET_FREQ,
    GET_FREQ,
    SET_MODE,
    GET_MODE,          
}CmdID;

#endif