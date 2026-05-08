/* MSHV Part from RigControl
 * Copyright 2015 Hrisimir Hristov, LZ2HV
 * May be used under the terms of the GNU General Public License (GPL)
 */
#include "yaesu.h"
#include "yaesu_def.h"
#include "../hvutils.h"
#include <stdio.h>
#include <QTimer>

//#include <QtGui>

#define YAESU_CMD_LENGTH 5
#define NEWCAT 201
#define USECMD 202
#define PMR0   203
#define FI 0x3b  //end of fraze NEWCAT -> ;

enum native_cmd_e {
    NATIVE_CAT_PTT_ON = 0,
    NATIVE_CAT_PTT_OFF,
    NATIVE_CAT_SET_FREQ,
    NATIVE_CAT_GET_FREQ_MODE_STATUS,
    NATIVE_CAT_SET_MODE_USB,
    NATIVE_CAT_SET_MODE_DIGU,
    NATIVE_CAT_GET_MODE,
    CAT_ON,
    CAT_OFF
};

struct yaesu_cmd_set
{
    unsigned char ncomp;		        /* 1 = complete, 0 = incomplete, needs extra info */
    unsigned char nseq[YAESU_CMD_LENGTH];	/* native cmd sequence */
};
typedef struct yaesu_cmd_set yaesu_cmd_set_t;
static const yaesu_cmd_set_t ncmd[YAESU_COUNT][9] =
    {
        //FT-100
        {{ 1, { 0x00, 0x00, 0x00, 0x01, 0x0f } }, /* ptt on */
         { 1, { 0x00, 0x00, 0x00, 0x00, 0x0f } }, /* ptt off */
         { 0, { 0x00, 0x00, 0x00, 0x00, 0x0a } }, /* set freq */
         { 0, { 0x00, 0x00, 0x00, 0x00, 0x10 } }, /* get FREQ and MODE status 32 bites */
         { 1, { 0x00, 0x00, 0x00, 0x01, 0x0c } }, /* mode set main USB */
         { 1, { 0x00, 0x00, 0x00, 0x05, 0x0c } }, /* mode set main DIGU */
         { 0, { 0x00, 0x00, 0x00, 0x00, 0x10 } }, /* get FREQ and MODE status 32 bites */
         { USECMD, { 0x00, 0x00, 0x00, 0x01, 0xfa } }, /*  CAT = On */
         { 0, { 0x00, 0x00, 0x00, 0x00, 0x00 } },}, /* CAT = Off */
        //FT-847
        {{ 1, { 0x00, 0x00, 0x00, 0x00, 0x08 } }, /* ptt on */
         { 1, { 0x00, 0x00, 0x00, 0x01, 0x88 } }, /* ptt off */
         { 0, { 0x00, 0x00, 0x00, 0x00, 0x01 } }, /* set freq */
         { 1, { 0x00, 0x00, 0x00, 0x00, 0x03 } }, /* get FREQ and MODE status */
         { 1, { 0x01, 0x00, 0x00, 0x00, 0x07 } }, /* mode set main USB */
         { 1, { 0x01, 0x00, 0x00, 0x00, 0x07 } }, /* mode set main DIGU */
         { 1, { 0x00, 0x00, 0x00, 0x00, 0x03 } }, /* get FREQ and MODE status */
         { USECMD, { 0x00, 0x00, 0x00, 0x00, 0x00 } }, /* CAT = On */
         { USECMD, { 0x00, 0x00, 0x00, 0x00, 0x80 } },}, /* CAT = Off */
        //FT-990
        {{ 1, { 0x00, 0x00, 0x00, 0x01, 0x0f } }, /* ptt on */
         { 1, { 0x00, 0x00, 0x00, 0x00, 0x0f } }, /* ptt off */
         { 0, { 0x00, 0x00, 0x00, 0x00, 0x0a } }, /* Set Op Freq */
         { 1, { 0x00, 0x00, 0x00, 0x02, 0x10 } },/* Update Op Data return 32 bites */
         { 1, { 0x00, 0x00, 0x00, 0x01, 0x0c } }, /* OP Mode Set USB */
         { 1, { 0x00, 0x00, 0x00, 0x09, 0x0c } }, /* OP Mode Set DIGU */
         { 1, { 0x00, 0x00, 0x00, 0x02, 0x10 } },/* Update Op Data  return 32 bites */
         { USECMD, { 0x00, 0x00, 0x00, 0x00, 0x0e } }, /*  CAT = On */
         { 0, { 0x00, 0x00, 0x00, 0x00, 0x00 } },}, /* CAT = Off */
        //FT-991
        {{ NEWCAT, { "TX1;" } }, /*identif newcat NEWCAT TX1; ptt on */
         { NEWCAT, { "TX0;" } }, /*identif newcat NEWCAT TX0;  ptt off */
         { NEWCAT, { "FA" } },   /*identif newcat NEWCAT FA;  set freq */
         { NEWCAT, { "FA;" } }, /*identif newcat NEWCAT FA;  get freq */
         { NEWCAT, { "MD0" } } , /*identif newcat NEWCAT FA;  set mode USB ili C data-usb */
         { NEWCAT, { "MD0" } } , /*identif newcat NEWCAT FA;  set mode DIGU ili C data-usb */
         { NEWCAT, { "MD0;" } }, /*identif newcat NEWCAT FA;  get mode */
         { USECMD, { "AI0;" } }, /* ??? 0=no use    CAT = On */
         { 0, { "AI1;" } },}, /* ??? 0=no use  CAT = Off */
        //FT-857
        {{ 1, { 0x00, 0x00, 0x00, 0x00, 0x08 } }, /* ptt on */
         { 1, { 0x00, 0x00, 0x00, 0x01, 0x88 } }, /* ptt off */
         { 0, { 0x00, 0x00, 0x00, 0x00, 0x01 } }, /* set freq */
         { 1, { 0x00, 0x00, 0x00, 0x00, 0x03 } }, /* get FREQ and MODE status */
         { 1, { 0x01, 0x00, 0x00, 0x00, 0x07 } }, /* mode set main USB */
         { 1, { 0x0a, 0x00, 0x00, 0x00, 0x07 } }, /* mode set main DIGU */
         { 1, { 0x00, 0x00, 0x00, 0x00, 0x03 } }, /* get FREQ and MODE status */
         { 0, { 0x00, 0x00, 0x00, 0x00, 0x00 } }, /*  CAT = On */
         { 0, { 0x00, 0x00, 0x00, 0x00, 0x00 } },}, /* CAT = Off */
        //FT-1000D
        {{ 1, { 0x00, 0x00, 0x00, 0x01, 0x0f } }, /* ptt on */
         { 1, { 0x00, 0x00, 0x00, 0x00, 0x0f } }, /* ptt off */
         { 0, { 0x00, 0x00, 0x00, 0x00, 0x0a } }, /* Set Op Freq */
         { 1, { 0x00, 0x00, 0x00, 0x02, 0x10 } }, /* Update Op Data  return 16 bites */
         { 1, { 0x00, 0x00, 0x00, 0x01, 0x0c } }, /* OP Mode Set USB */
         { 1, { 0x00, 0x00, 0x00, 0x09, 0x0c } }, /* OP Mode Set DIGU */
         { 1, { 0x00, 0x00, 0x00, 0x02, 0x10 } },/* Update Op Data  return 16 bites */
         { USECMD, { 0x00, 0x00, 0x00, 0x00, 0x0e } }, /*  CAT = On */
         { 0, { 0x00, 0x00, 0x00, 0x00, 0x00 } },}, /* CAT = Off */
        //FT-1000MP
        {{ 1, { 0x00, 0x00, 0x00, 0x01, 0x0f } }, /* ptt on */
         { 1, { 0x00, 0x00, 0x00, 0x00, 0x0f } }, /* ptt off */
         { 0, { 0x00, 0x00, 0x00, 0x00, 0x0a } }, /* set VFOA freq */
         { 1, { 0x00, 0x00, 0x00, 0x02, 0x10 } }, /* Update Op Data */
         { 1, { 0x00, 0x00, 0x00, 0x01, 0x0c } }, /* vfo A mode set USB */
         { 1, { 0x00, 0x00, 0x00, 0x09, 0x0c } }, /* OP Mode Set DIGU */
         { 1, { 0x00, 0x00, 0x00, 0x02, 0x10 } },/* Update Op Data */
         { USECMD, { 0x00, 0x00, 0x00, 0x00, 0x0e } }, /*  CAT = On */
         { 0, { 0x00, 0x00, 0x00, 0x00, 0x00 } },}, /* CAT = Off */
        //FT-2000
        //{{ NEWCAT, { 0x54, 0x58, 0x31, 0x3b, 0x00 } }, /*identif newcat NEWCAT TX1; ptt on */
        // { NEWCAT, { 0x54, 0x58, 0x30, 0x3b, 0x00 } },}, /*identif newcat NEWCAT TX0;  ptt off */
        //FT-2000
        {{ NEWCAT, { "TX1;" } }, /*identif newcat NEWCAT TX1; ptt on */
         { NEWCAT, { "TX0;" } }, /*identif newcat NEWCAT TX0;  ptt off */
         { NEWCAT, { "FA" } },   /*identif newcat NEWCAT FA;  set freq */
         { NEWCAT, { "FA;" } }, /*identif newcat NEWCAT FA;  get freq */
         { NEWCAT, { "MD0" } } , /*identif newcat NEWCAT FA;  set mode USB ili C data-usb */
         { NEWCAT, { "MD0" } } , /*identif newcat NEWCAT FA;  set mode DIGU ili C data-usb */
         { NEWCAT, { "MD0;" } }, /*identif newcat NEWCAT FA;  get mode */
         { USECMD, { "AI0;" } }, /* ??? 0=no use    CAT = On */
         { 0, { "AI1;" } },}, /* ??? 0=no use  CAT = Off */
        //FT-DX3000  //AI0;=off AI1;-on Auto information  VS;vefo select
        {{ NEWCAT, { "TX1;" } }, /*identif newcat NEWCAT TX1; ptt on */
         { NEWCAT, { "TX0;" } }, /*identif newcat NEWCAT TX0;  ptt off */
         { NEWCAT, { "FA" } },   /*identif newcat NEWCAT FA;  set freq */
         { NEWCAT, { "FA;" } }, /*identif newcat NEWCAT FA;  get freq */
         { NEWCAT, { "MD0" } } , /*identif newcat NEWCAT FA;  set mode USB ili C data-usb */
         { NEWCAT, { "MD0" } } , /*identif newcat NEWCAT FA;  set mode DIGU ili C data-usb */
         { NEWCAT, { "MD0;" } }, /*identif newcat NEWCAT FA;  get mode */
         { USECMD, { "AI0;" } }, /* ??? 0=no use    CAT = On */
         { 0, { "AI1;" } },}, /* ??? 0=no use  CAT = Off */
        //FT-DX5000  //AI0;=off AI1;-on Auto information  VS;vefo select
        {{ NEWCAT, { "TX1;" } }, /*identif newcat NEWCAT TX1; ptt on */
         { NEWCAT, { "TX0;" } }, /*identif newcat NEWCAT TX0;  ptt off */
         { NEWCAT, { "FA" } },   /*identif newcat NEWCAT FA;  set freq */
         { NEWCAT, { "FA;" } }, /*identif newcat NEWCAT FA;  get freq */
         { NEWCAT, { "MD0" } } , /*identif newcat NEWCAT FA;  set mode USB ili C data-usb */
         { NEWCAT, { "MD0" } } , /*identif newcat NEWCAT FA;  set mode DIGU ili C data-usb */
         { NEWCAT, { "MD0;" } }, /*identif newcat NEWCAT FA;  get mode */
         { USECMD, { "AI0;" } }, /* ??? 0=no use    CAT = On */
         { 0, { "AI1;" } },}, /* ??? 0=no use  CAT = Off */
        //FTDX-9000
        {{ NEWCAT, { "TX1;" } }, /*identif newcat NEWCAT TX1; ptt on */
         { NEWCAT, { "TX0;" } }, /*identif newcat NEWCAT TX0;  ptt off */
         { NEWCAT, { "FA" } },   /*identif newcat NEWCAT FA;  set freq */
         { NEWCAT, { "FA;" } }, /*identif newcat NEWCAT FA;  get freq */
         { NEWCAT, { "MD0" } } , /*identif newcat NEWCAT FA;  set mode USB ili C data-usb */
         { NEWCAT, { "MD0" } } , /*identif newcat NEWCAT FA;  set mode DIGU ili C data-usb */
         { NEWCAT, { "MD0;" } }, /*identif newcat NEWCAT FA;  get mode */
         { USECMD, { "AI0;" } }, /* ??? 0=no use    CAT = On */
         { 0, { "AI1;" } },}, /* ??? 0=no use  CAT = Off */
        //FT-950
        {{ NEWCAT, { "TX1;" } }, /*identif newcat NEWCAT TX1; ptt on */
         { NEWCAT, { "TX0;" } }, /*identif newcat NEWCAT TX0;  ptt off */
         { NEWCAT, { "FA" } },   /*identif newcat NEWCAT FA;  set freq */
         { NEWCAT, { "FA;" } }, /*identif newcat NEWCAT FA;  get freq */
         { NEWCAT, { "MD0" } } , /*identif newcat NEWCAT FA;  set mode USB ili C data-usb */
         { NEWCAT, { "MD0" } } , /*identif newcat NEWCAT FA;  set mode DIGU ili C data-usb */
         { NEWCAT, { "MD0;" } }, /*identif newcat NEWCAT FA;  get mode */
         { USECMD, { "AI0;" } }, /* ??? 0=no use    CAT = On */
         { 0, { "AI1;" } },}, /* ??? 0=no use  CAT = Off */
        //FT-920
        {{ 1, { 0x00, 0x00, 0x00, 0x01, 0x0f } }, /* ptt on */
         { 1, { 0x00, 0x00, 0x00, 0x00, 0x0f } }, /* ptt off */
         { 0, { 0x00, 0x00, 0x00, 0x00, 0x0a } },  /* set vfo A freq */
         { 1, { 0x00, 0x00, 0x00, 0x02, 0x10 } },/* Status Update Data--Current operating data for VFO/Memory (28 bytes) */
         { 0, { 0x00, 0x00, 0x00, 0x01, 0x0c } },    /* mode set */ //#define MODE_SET_A_USB      0x01 P1=3bit
         { 0, { 0x00, 0x00, 0x00, 0x0a, 0x0c } },    /* mode set */ //#define MODE_SET_A_DIGU      0x01 P1=3bit
         { 1, { 0x00, 0x00, 0x00, 0x02, 0x10 } },  /* Status Update Data--Current operating data for VFO/Memory (28 bytes) */
         { USECMD, { 0x00, 0x00, 0x00, 0x00, 0x0e } }, /*  CAT = On */
         { 0, { 0x00, 0x00, 0x00, 0x00, 0x00 } },}, /* CAT = Off */
        //FT-900
        {{ 1, { 0x00, 0x00, 0x00, 0x01, 0x0f } }, /* ptt on */
         { 1, { 0x00, 0x00, 0x00, 0x00, 0x0f } }, /* ptt off */
         { 0, { 0x00, 0x00, 0x00, 0x00, 0x0a } }, /* set display freq */
         { 1, { 0x00, 0x00, 0x00, 0x02, 0x10 } }, /* Status Update Data--Current operating data for VFO/Memory (19 bytes) */
         { 0, { 0x00, 0x00, 0x00, 0x01, 0x0c } }, /* mode set *///#define MODE_SET_USB    0x01 P1=3bit
         { 0, { 0x00, 0x00, 0x00, 0x01, 0x0c } }, /* mode set *///#define MODE_SET_DIGU    0x01 P1=3bit
         { 1, { 0x00, 0x00, 0x00, 0x02, 0x10 } }, /* Status Update Data--Current operating data for VFO/Memory (19 bytes) */
         { USECMD, { 0x00, 0x00, 0x00, 0x00, 0x0e } }, /*  CAT = On */
         { 0, { 0x00, 0x00, 0x00, 0x00, 0x00 } },}, /* CAT = Off */
        //FT-897
        {{ 1, { 0x00, 0x00, 0x00, 0x00, 0x08 } }, /* ptt on */
         { 1, { 0x00, 0x00, 0x00, 0x01, 0x88 } }, /* ptt off */
         { 0, { 0x00, 0x00, 0x00, 0x00, 0x01 } }, /* set freq */
         { 1, { 0x00, 0x00, 0x00, 0x00, 0x03 } }, /* get FREQ and MODE status */
         { 1, { 0x01, 0x00, 0x00, 0x00, 0x07 } }, /* mode set main USB */
         { 1, { 0x0a, 0x00, 0x00, 0x00, 0x07 } }, /* mode set main DIGU */
         { 1, { 0x00, 0x00, 0x00, 0x00, 0x03 } }, /* get FREQ and MODE status */
         { 0, { 0x00, 0x00, 0x00, 0x00, 0x00 } }, /*  CAT = On */
         { 0, { 0x00, 0x00, 0x00, 0x00, 0x00 } },}, /* CAT = Off */
        //FT-890
        {{ 1, { 0x00, 0x00, 0x00, 0x01, 0x0f } }, /* ptt on */
         { 1, { 0x00, 0x00, 0x00, 0x00, 0x0f } }, /* ptt off */
         { 0, { 0x00, 0x00, 0x00, 0x00, 0x0a } }, /* set display freq */
         { 1, { 0x00, 0x00, 0x00, 0x02, 0x10 } },/* Status Update Data--Current operating data for VFO/Memory (19 bytes) */
         { 0, { 0x00, 0x00, 0x00, 0x01, 0x0c } }, /* mode set */  //    #define MODE_SET_USB    0x01   P1=3bit
         { 0, { 0x00, 0x00, 0x00, 0x01, 0x0c } }, /* mode set */  //    #define MODE_SET_DIGU    0x01   P1=3bit
         { 1, { 0x00, 0x00, 0x00, 0x02, 0x10 } },/* Status Update Data--Current operating data for VFO/Memory (19 bytes) */
         { USECMD, { 0x00, 0x00, 0x00, 0x00, 0x0e } }, /*  CAT = On */
         { 0, { 0x00, 0x00, 0x00, 0x00, 0x00 } },}, /* CAT = Off */
        //FT-840
        {{ 1, { 0x00, 0x00, 0x00, 0x01, 0x0f } }, /* ptt on */
         { 1, { 0x00, 0x00, 0x00, 0x00, 0x0f } }, /* ptt off */
         { 0, { 0x00, 0x00, 0x00, 0x00, 0x0a } }, /* set display freq */
         { 1, { 0x00, 0x00, 0x00, 0x02, 0x10 } },  /* Status Update Data--Current operating data for VFO/Memory (19 bytes) */
         { 0, { 0x00, 0x00, 0x00, 0x01, 0x0c } }, /* mode set */ // #define MODE_SET_USB    0x01
         { 0, { 0x00, 0x00, 0x00, 0x01, 0x0c } }, /* mode set */ // #define MODE_SET_DIGU    0x01
         { 1, { 0x00, 0x00, 0x00, 0x02, 0x10 } },  /* Status Update Data--Current operating data for VFO/Memory (19 bytes) */
         { USECMD, { 0x00, 0x00, 0x00, 0x00, 0x0e } }, /*  CAT = On */
         { 0, { 0x00, 0x00, 0x00, 0x00, 0x00 } },}, /* CAT = Off */
        //FT-817
        {{ 1, { 0x00, 0x00, 0x00, 0x00, 0x08 } }, /* ptt on */
         { 1, { 0x00, 0x00, 0x00, 0x01, 0x88 } }, /* ptt off */
         { 0, { 0x00, 0x00, 0x00, 0x00, 0x01 } }, /* set freq */
         { 1, { 0x00, 0x00, 0x00, 0x00, 0x03 } }, /* get FREQ and MODE status */
         { 1, { 0x01, 0x00, 0x00, 0x00, 0x07 } }, /* mode set main USB */
         { 1, { 0x0a, 0x00, 0x00, 0x00, 0x07 } }, /* mode set main DIGU */
         { 1, { 0x00, 0x00, 0x00, 0x00, 0x03 } }, /* get FREQ and MODE status */
         { 0, { 0x00, 0x00, 0x00, 0x00, 0x00 } }, /*  CAT = On */
         { 0, { 0x00, 0x00, 0x00, 0x00, 0x00 } },}, /* CAT = Off */
        //FT-747GX
        //#define FT747_SUMO_DISPLAYED_FREQ             0x01 from_bcd_be
        //#define FT747_SUMO_VFO_A_FREQ                 0x09 from_bcd_be
        {{ 1, { 0x00, 0x00, 0x00, 0x01, 0x0f } }, /* ptt on */
         { 1, { 0x00, 0x00, 0x00, 0x00, 0x0f } }, /* ptt off */
         { 0, { 0x00, 0x00, 0x00, 0x00, 0x0a } }, /* set freq */
         { 0, { 0x00, 0x00, 0x00, 0x00, 0x10 } },//2.46 get mode/frq =0x10 status #define FT747_SUMO_DISPLAYED_FREQ 0x01
         { 1, { 0x00, 0x00, 0x00, 0x01, 0x0c } }, /* mode set USB */
         { 1, { 0x00, 0x00, 0x00, 0x01, 0x0c } }, /* mode set DIGU */
         { 1, { 0x00, 0x00, 0x00, 0x00, 0x10 } },//2.46 get mode/frq =0x10 status #define FT747_SUMO_DISPLAYED_MODE 0x18
         { USECMD, { 0x00, 0x00, 0x00, 0x00, 0x0e } }, /*  CAT = On */
         { 0, { 0x00, 0x00, 0x00, 0x00, 0x00 } },}, /* CAT = Off */
        //FT-450
        {{ NEWCAT, { "TX1;" } }, /*identif newcat NEWCAT TX1; ptt on */
         { NEWCAT, { "TX0;" } }, /*identif newcat NEWCAT TX0;  ptt off */
         { NEWCAT, { "FA" } },   /*identif newcat NEWCAT FA;  set freq */
         { NEWCAT, { "FA;" } }, /*identif newcat NEWCAT FA;  get freq */
         { NEWCAT, { "MD0" } } , /*identif newcat NEWCAT FA;  set mode USB ili C data-usb */
         { NEWCAT, { "MD0" } } , /*identif newcat NEWCAT FA;  set mode DIGU ili C data-usb */
         { NEWCAT, { "MD0;" } }, /*identif newcat NEWCAT FA;  get mode */
         { USECMD, { "AI0;" } }, /* ??? 0=no use    CAT = On */
         { 0, { "AI1;" } },}, /* ??? 0=no use  CAT = Off */
        //FT-450D
        {{ NEWCAT, { "TX1;" } }, /*identif newcat NEWCAT TX1; ptt on */
         { NEWCAT, { "TX0;" } }, /*identif newcat NEWCAT TX0;  ptt off */
         { NEWCAT, { "FA" } },   /*identif newcat NEWCAT FA;  set freq */
         { NEWCAT, { "FA;" } }, /*identif newcat NEWCAT FA;  get freq */
         { NEWCAT, { "MD0" } } , /*identif newcat NEWCAT FA;  set mode USB ili C data-usb */
         { NEWCAT, { "MD0" } } , /*identif newcat NEWCAT FA;  set mode DIGU ili C data-usb */
         { NEWCAT, { "MD0;" } }, /*identif newcat NEWCAT FA;  get mode */
         { USECMD, { "AI0;" } }, /* ??? 0=no use    CAT = On */
         { 0, { "AI1;" } },}, /* ??? 0=no use  CAT = Off */
        //FT-767GX      NONE HV ?????????  get func
        {{ 1, { 0x00, 0x00, 0x00, 0x01, 0x0f } }, /* ptt on */
         { 1, { 0x00, 0x00, 0x00, 0x00, 0x0f } }, /* ptt off */
         { 1, { 0x00, 0x00, 0x00, 0x00, 0x08 } },	//#define CMD_FREQ_SET	0x08
         { 1, { 0x00, 0x00, 0x00, 0x00, 0x00 } },	//#define STATUS_CURR_FREQ	1	/* Operating Frequency */ //#define STATUS_VFOA_FREQ	14 ????
         { 1, { 0x00, 0x00, 0x00, 0x11, 0x0a } },	 //set mode //#define SUBCMD_MODE_USB	0x11 /* 8 bytes returned */
         { 1, { 0x00, 0x00, 0x00, 0x11, 0x0a } },	 //set mode //#define SUBCMD_MODE_DIGU	0x11 /* 8 bytes returned */
         { 1, { 0x00, 0x00, 0x00, 0x00, 0x00 } },//PROBLEM get mode   //#define STATUS_CURR_MODE	6      #define SUBCMD_MODE_USB	0x11 /* 8 bytes returned */
         { USECMD, { 0x00, 0x00, 0x00, 0x00, 0x00 } }, /*  CAT = On */
         { USECMD, { 0x00, 0x00, 0x00, 0x01, 0x00 } },}, /* CAT = Off */
        //FT-757GX      NONE HV
        {{ 1, { 0x00, 0x00, 0x00, 0x01, 0x0f } }, /* ptt on */
         { 1, { 0x00, 0x00, 0x00, 0x00, 0x0f } }, /* ptt off */
         { 0, { 0x00, 0x00, 0x00, 0x00, 0x0a } }, /* set freq */
         { 0, { 0x00, 0x00, 0x00, 0x00, 0x10 } }, //#define STATUS_CURR_FREQ    5   /* Operating Frequency *  /#define STATUS_VFOA_FREQ    10
         { 0, { 0x00, 0x00, 0x00, 0x01, 0x0c } },	//set mode USB
         { 0, { 0x00, 0x00, 0x00, 0x01, 0x0c } },	//set mode DIGU
         { 0, { 0x00, 0x00, 0x00, 0x00, 0x10} },	//get mode
         { 0, { 0x00, 0x00, 0x00, 0x00, 0x00 } }, /*  CAT = On */
         { 0, { 0x00, 0x00, 0x00, 0x00, 0x00 } },},
        /* CAT = Off */
        //FRG-9600      NONE HV
        {{ 1, { 0x00, 0x00, 0x00, 0x01, 0x0f } }, /* ptt on */
         { 1, { 0x00, 0x00, 0x00, 0x00, 0x0f } }, /* ptt off */
         { 1, { 0x0a, 0x00, 0x00, 0x00, 0x00 } }, /* set freq */ //-1  block
         { 1, { 0x00, 0x00, 0x00, 0x00, 0x00 } }, /* get freq */ //none
         { 1, { 0x00, 0x00, 0x00, 0x00, 0x00 } }, /* set mode */ //none
         { 1, { 0x00, 0x00, 0x00, 0x00, 0x00 } }, /* set mode */ //none
         { 1, { 0x00, 0x00, 0x00, 0x00, 0x00 } }, /* get mode */ //none
         { 0, { 0x00, 0x00, 0x00, 0x00, 0x00 } }, /*  CAT = On */
         { 0, { 0x00, 0x00, 0x00, 0x00, 0x00 } },}, /* CAT = Off */
        //FRG-8800      NONE HV
        {{ 1, { 0x00, 0x00, 0x00, 0x01, 0x0f } }, /* ptt on */
         { 1, { 0x00, 0x00, 0x00, 0x00, 0x0f } }, /* ptt off */
         { 1, { 0x00, 0x00, 0x00, 0x00, 0x01 } }, /* set freq */
         { 1, { 0x00, 0x00, 0x00, 0x00, 0x00 } }, /* get freq */ //none
         { 1, { 0x00, 0x00, 0x00, 0x02, 0x80 } }, /* set mode*/
         { 1, { 0x00, 0x00, 0x00, 0x02, 0x80 } }, /* set mode*/
         { 1, { 0x00, 0x00, 0x00, 0x00, 0x00 } }, /* get freq */ //none
         { USECMD, { 0x00, 0x00, 0x00, 0x00, 0x00 } }, /*  CAT = On */
         { 0, { 0x00, 0x00, 0x00, 0x00, 0x00 } },}, /* CAT = Off */
        //FRG-100      NONE HV
        {{ 1, { 0x00, 0x00, 0x00, 0x01, 0x0f } }, /* ptt on */
         { 1, { 0x00, 0x00, 0x00, 0x00, 0x0f } }, /* ptt off */
         { 0, { 0x00, 0x00, 0x00, 0x00, 0x0a } }, /* Set Op Freq */
         { 1, { 0x00, 0x00, 0x00, 0x00, 0x00 } }, /* get freq */ //none
         { 1, { 0x00, 0x00, 0x00, 0x01, 0x0c } }, /* OP Mode Set USB */
         { 1, { 0x00, 0x00, 0x00, 0x01, 0x0c } }, /* OP Mode Set DIGU */
         { 1, { 0x00, 0x00, 0x00, 0x00, 0x00 } }, /* get freq */ //none
         { USECMD, { 0x00, 0x00, 0x00, 0x00, 0x0e } }, /*  CAT = On */
         { 0, { 0x00, 0x00, 0x00, 0x00, 0x00 } },}, /* CAT = Off */
        //VR-5000      NONE HV
        {{ 1, { 0x00, 0x00, 0x00, 0x01, 0x0f } },   /* ptt on */
         { 1, { 0x00, 0x00, 0x00, 0x00, 0x0f } },   /* ptt off */
         { 1, { 0x00, 0x00, 0x00, 0x00, 0x00 } },   /* get freq */ //none
         { 1, { 0x00, 0x00, 0x00, 0x00, 0x00 } }, /* get freq */ //none
         { 1, { 0x00, 0x00, 0x00, 0x00, 0x00 } }, /* get freq */ //none
         { 1, { 0x00, 0x00, 0x00, 0x00, 0x00 } }, /* get freq */ //none
         { 1, { 0x00, 0x00, 0x00, 0x00, 0x00 } }, /* get freq */ //none
         { USECMD, { 0x00, 0x00, 0x00, 0x00, 0x00 } }, /*  CAT = On */
         { USECMD, { 0x00, 0x00, 0x00, 0x00, 0x80 } },}, /* CAT = Off */
        //FT-736R   ?
        {{ 1, { 0x00, 0x00, 0x00, 0x00, 0x08 } },   /* ptt on */
         { 1, { 0x00, 0x00, 0x00, 0x01, 0x88 } },   /* ptt off */
         { 1, { 0x00, 0x00, 0x00, 0x00, 0x01 } },   /* set freq */
         { 1, { 0x00, 0x00, 0x00, 0x00, 0x00 } }, /* get freq */ //none
         { 1, { 0x01, 0x00, 0x00, 0x00, 0x07 } },   /* set mode */
         { 1, { 0x01, 0x00, 0x00, 0x00, 0x07 } },   /* set mode */
         { 1, { 0x00, 0x00, 0x00, 0x00, 0x00 } }, /* get mode */ //none
         { USECMD, { 0x00, 0x00, 0x00, 0x00, 0x00 } }, /*  CAT = On */
         { USECMD, { 0x80, 0x80, 0x80, 0x80, 0x80 } },}, /* CAT = Off */
        //FT-980    TESTED HV
        {{ 1, { 0x00, 0x00, 0x00, 0x01, 0x0f } },   /* ptt on */
         { 1, { 0x00, 0x00, 0x00, 0x00, 0x0f } },   /* ptt off */
         { 1, { 0x00, 0x00, 0x00, 0x00, 0x08 } },   /* set freq */
         { 1, { 0x00, 0x00, 0x00, 0x00, 0x01 } }, /* get freq */ //
         { 1, { 0x00, 0x00, 0x00, 0x11, 0x0a } },   /* set mode */	//#define MD_USB  0x11
         { 1, { 0x00, 0x00, 0x00, 0x16, 0x0a } },   /* set mode */	//#define MD_DIGU  0x11
         { 1, { 0x00, 0x00, 0x00, 0x00, 0x01 } }, /* get mode */ //
         { USECMD, { 0x00, 0x00, 0x00, 0x00, 0x00 } }, /*  CAT = On */  //2.56
         { USECMD, { 0x00, 0x00, 0x00, 0x00, 0x00 } },}, /* CAT = Off */ //2.56
        //MARK-V FT-1000MP
        {{ 1, { 0x00, 0x00, 0x00, 0x01, 0x0f } },   /* ptt on */
         { 1, { 0x00, 0x00, 0x00, 0x00, 0x0f } }, /* ptt off */
         { 0, { 0x00, 0x00, 0x00, 0x00, 0x0a } }, /* set VFOA freq */
         { 1, { 0x00, 0x00, 0x00, 0x02, 0x10 } }, /* Update Op Data */
         { 1, { 0x00, 0x00, 0x00, 0x01, 0x0c } }, /* vfo A mode set USB */
         { 1, { 0x00, 0x00, 0x00, 0x09, 0x0c } }, /* vfo A mode set DIGU */
         { 1, { 0x00, 0x00, 0x00, 0x02, 0x10 } },/* Update Op Data */
         { USECMD, { 0x00, 0x00, 0x00, 0x00, 0x0e } }, /*  CAT = On */
         { 0, { 0x00, 0x00, 0x00, 0x00, 0x00 } },}, /* CAT = Off */
        //MARK-V Field FT-1000MP
        {{ 1, { 0x00, 0x00, 0x00, 0x01, 0x0f } },   /* ptt on */
         { 1, { 0x00, 0x00, 0x00, 0x00, 0x0f } }, /* ptt off */
         { 0, { 0x00, 0x00, 0x00, 0x00, 0x0a } }, /* set VFOA freq */
         { 1, { 0x00, 0x00, 0x00, 0x02, 0x10 } }, /* Update Op Data */
         { 1, { 0x00, 0x00, 0x00, 0x01, 0x0c } }, /* vfo A mode set USB */
         { 1, { 0x00, 0x00, 0x00, 0x09, 0x0c } }, /* vfo A mode set DIGU */
         { 1, { 0x00, 0x00, 0x00, 0x02, 0x10 } },/* Update Op Data */
         { USECMD, { 0x00, 0x00, 0x00, 0x00, 0x0e } }, /*  CAT = On */
         { 0, { 0x00, 0x00, 0x00, 0x00, 0x00 } },}, /* CAT = Off */
        //FTX-1F
        {{ NEWCAT, { "TX1;" } }, /*identif newcat NEWCAT TX1; ptt on */
         { NEWCAT, { "TX0;" } }, /*identif newcat NEWCAT TX0;  ptt off */
         { NEWCAT, { "FA" } },   /*identif newcat NEWCAT FA;  set freq */
         { NEWCAT, { "FA;" } }, /*identif newcat NEWCAT FA;  get freq */
         { NEWCAT, { "MD0" } } , /*identif newcat NEWCAT FA;  set mode USB ili C data-usb */
         { NEWCAT, { "MD0" } } , /*identif newcat NEWCAT FA;  set mode DIGU ili C data-usb */
         { NEWCAT, { "MD0;" } }, /*identif newcat NEWCAT FA;  get mode */
         { USECMD, { "AI0;" } }, /* ??? 0=no use    CAT = On */
         { 0, { "AI1;" } },}, /* ??? 0=no use  CAT = Off */
        //FT-710
        {{ NEWCAT, { "TX1;" } }, /*identif newcat NEWCAT TX1; ptt on */
         { NEWCAT, { "TX0;" } }, /*identif newcat NEWCAT TX0;  ptt off */
         { NEWCAT, { "FA" } },   /*identif newcat NEWCAT FA;  set freq */
         { NEWCAT, { "FA;" } }, /*identif newcat NEWCAT FA;  get freq */
         { NEWCAT, { "MD0" } } , /*identif newcat NEWCAT FA;  set mode USB ili C data-usb */
         { NEWCAT, { "MD0" } } , /*identif newcat NEWCAT FA;  set mode DIGU ili C data-usb */
         { NEWCAT, { "MD0;" } }, /*identif newcat NEWCAT FA;  get mode */
         { USECMD, { "AI0;" } }, /* ??? 0=no use    CAT = On */
         { 0, { "AI1;" } },}, /* ??? 0=no use  CAT = Off */
        //FT-891
        {{ NEWCAT, { "TX1;" } }, /*identif newcat NEWCAT TX1; ptt on */
         { NEWCAT, { "TX0;" } }, /*identif newcat NEWCAT TX0;  ptt off */
         { NEWCAT, { "FA" } },   /*identif newcat NEWCAT FA;  set freq */
         { NEWCAT, { "FA;" } }, /*identif newcat NEWCAT FA;  get freq */
         { NEWCAT, { "MD0" } } , /*identif newcat NEWCAT FA;  set mode USB ili C data-usb */
         { NEWCAT, { "MD0" } } , /*identif newcat NEWCAT FA;  set mode DIGU ili C data-usb */
         { NEWCAT, { "MD0;" } }, /*identif newcat NEWCAT FA;  get mode */
         { USECMD, { "AI0;" } }, /* ??? 0=no use    CAT = On */
         { 0, { "AI1;" } },}, /* ??? 0=no use  CAT = Off */
        //FT-DX10
        {{ NEWCAT, { "TX1;" } }, /*identif newcat NEWCAT TX1; ptt on */
         { NEWCAT, { "TX0;" } }, /*identif newcat NEWCAT TX0;  ptt off */
         { NEWCAT, { "FA" } },   /*identif newcat NEWCAT FA;  set freq */
         { NEWCAT, { "FA;" } }, /*identif newcat NEWCAT FA;  get freq */
         { NEWCAT, { "MD0" } } , /*identif newcat NEWCAT FA;  set mode USB ili C data-usb */
         { NEWCAT, { "MD0" } } , /*identif newcat NEWCAT FA;  set mode DIGU ili C data-usb */
         { NEWCAT, { "MD0;" } }, /*identif newcat NEWCAT FA;  get mode */
         { USECMD, { "AI0;" } }, /* ??? 0=no use    CAT = On */
         { 0, { "AI1;" } },}, /* ??? 0=no use  CAT = Off */
        //FT-DX101
        {{ NEWCAT, { "TX1;" } }, /*identif newcat NEWCAT TX1; ptt on */
         { NEWCAT, { "TX0;" } }, /*identif newcat NEWCAT TX0;  ptt off */
         { NEWCAT, { "FA" } },   /*identif newcat NEWCAT FA;  set freq */
         { NEWCAT, { "FA;" } }, /*identif newcat NEWCAT FA;  get freq */
         { NEWCAT, { "MD0" } } , /*identif newcat NEWCAT FA;  set mode USB ili C data-usb */
         { NEWCAT, { "MD0" } } , /*identif newcat NEWCAT FA;  set mode DIGU ili C data-usb */
         { NEWCAT, { "MD0;" } }, /*identif newcat NEWCAT FA;  get mode */
         { USECMD, { "AI0;" } }, /* ??? 0=no use    CAT = On */
         { 0, { "AI1;" } },}, /* ??? 0=no use  CAT = Off */
        //FT-DX1200
        {{ NEWCAT, { "TX1;" } }, /*identif newcat NEWCAT TX1; ptt on */
         { NEWCAT, { "TX0;" } }, /*identif newcat NEWCAT TX0;  ptt off */
         { NEWCAT, { "FA" } },   /*identif newcat NEWCAT FA;  set freq */
         { NEWCAT, { "FA;" } }, /*identif newcat NEWCAT FA;  get freq */
         { NEWCAT, { "MD0" } } , /*identif newcat NEWCAT FA;  set mode USB ili C data-usb */
         { NEWCAT, { "MD0" } } , /*identif newcat NEWCAT FA;  set mode DIGU ili C data-usb */
         { NEWCAT, { "MD0;" } }, /*identif newcat NEWCAT FA;  get mode */
         { USECMD, { "AI0;" } }, /* ??? 0=no use    CAT = On */
         { 0, { "AI1;" } },}, /* ??? 0=no use  CAT = Off */
        //Ailunce HS2/HS3
        {{ PMR0, { 0x07, 0x00, 0x00, 0x00, 0x00 } }, /* ptt on */
         { PMR0, { 0x07, 0x01, 0x00, 0x00, 0x00 } }, /* ptt off */
         { PMR0, { 0x09, 0x00, 0x00, 0x00, 0x00 } }, /* set freq */
         { PMR0, { 0x0b, 0x00, 0x00, 0x00, 0x00 } }, /* get FREQ and MODE status */
         { PMR0, { 0x0a, 0x00, 0x00, 0x00, 0x00 } }, /* mode set main USB */
         { PMR0, { 0x0a, 0x07, 0x00, 0x00, 0x00 } }, /* mode set main DIGU */
         { PMR0, { 0x0b, 0x00, 0x00, 0x00, 0x00 } }, /* get FREQ and MODE status */
         { USECMD, { 0x04, 0x27, 0x00, 0x8f, 0x2d } }, /*  CAT = On */
         { 0, { 0x00, 0x00, 0x00, 0x00, 0x00 } },}, /* CAT = Off */
        //Radioddity QR20
        {{ PMR0, { 0x07, 0x00, 0x00, 0x00, 0x00 } }, /* ptt on */
         { PMR0, { 0x07, 0x01, 0x00, 0x00, 0x00 } }, /* ptt off */
         { PMR0, { 0x09, 0x00, 0x00, 0x00, 0x00 } }, /* set freq */
         { PMR0, { 0x0b, 0x00, 0x00, 0x00, 0x00 } }, /* get FREQ and MODE status */
         { PMR0, { 0x0a, 0x00, 0x00, 0x00, 0x00 } }, /* mode set main USB */
         { PMR0, { 0x0a, 0x07, 0x00, 0x00, 0x00 } }, /* mode set main DIGU */
         { PMR0, { 0x0b, 0x00, 0x00, 0x00, 0x00 } }, /* get FREQ and MODE status */
         { USECMD, { 0x04, 0x27, 0x00, 0x8f, 0x2d } }, /*  CAT = On */
         { 0, { 0x00, 0x00, 0x00, 0x00, 0x00 } },}, /* CAT = Off */
        //Guohe Q900/PMR-171/TBR-119
        {{ PMR0, { 0x07, 0x00, 0x00, 0x00, 0x00 } }, /* ptt on */
         { PMR0, { 0x07, 0x01, 0x00, 0x00, 0x00 } }, /* ptt off */
         { PMR0, { 0x09, 0x00, 0x00, 0x00, 0x00 } }, /* set freq */
         { PMR0, { 0x0b, 0x00, 0x00, 0x00, 0x00 } }, /* get FREQ and MODE status */
         { PMR0, { 0x0a, 0x00, 0x00, 0x00, 0x00 } }, /* mode set main USB */
         { PMR0, { 0x0a, 0x07, 0x00, 0x00, 0x00 } }, /* mode set main DIGU */
         { PMR0, { 0x0b, 0x00, 0x00, 0x00, 0x00 } }, /* get FREQ and MODE status */
         { USECMD, { 0x04, 0x27, 0x00, 0x8f, 0x2d } }, /*  CAT = On */
         { 0, { 0x00, 0x00, 0x00, 0x00, 0x00 } },}, /* CAT = Off */
    };

typedef struct
{
    int cat_read_ar_size;
    int pos_frq;
    int bcd_size;//bcd size    in NEWCAT identif word size 8 or 9
    int method_frq; //s_method_frq->  0=from_bcd,  1=from_bcd_be,  2=3int,  3=4int,  4=ft100
    double multypl;
    int pos_mod;
    unsigned char id_mod_usb;
    unsigned char id_mod_digu;
}
YaesuReadParms;
static YaesuReadParms read_parms[YAESU_COUNT] = //{0,     0,     0,        0,      1.0,    0,       0x00},//NEWCAT
    {
        //size  p frq  bcd_size  method  multyp  pos_mod  mod_usb  mod_digu->0x7e=no suport=int->126 NEWCAT=0x00
        {32,    1,     4,        4,      1.25,   5,       0x01,    0x05},//FT-100
        {5,     0,     4,        1,      10.0,   4,       0x01,    0x7e},//FT-847
        {19,    1,     3,        2,      10.0,   7,       0x01,    0x05},//FT-990  0x05=rtty
        {0,     0,     9,        0,      1.0,    0,       0x00,    0x00},//FT-991 NEWCAT
        {5,     0,     4,        1,      10.0,   4,       0x01,    0x0a},//FT-857
        {16,    1,     3,        2,      10.0,   7,       0x01,    0x05},//FT-1000D  0x05=rtty
        {16,    1,     4,        3,      0.625,  7,       0x01,    0x05},//FT-1000MP" 0.625 0x05=rtty
        {0,     0,     8,        0,      1.0,    0,       0x00,    0x00},//FT-2000 NEWCAT  bcd_size special!!!
        {0,     0,     8,        0,      1.0,    0,       0x00,    0x00},//FT-DX3000 NEWCAT  bcd_size special!!!
        {0,     0,     8,        0,      1.0,    0,       0x00,    0x00},//FT-DX5000 NEWCAT  bcd_size special!!!
        {0,     0,     8,        0,      1.0,    0,       0x00,    0x00},//FTDX-9000 NEWCAT  bcd_size special!!!
        {0,     0,     8,        0,      1.0,    0,       0x00,    0x00},//FT-950 NEWCAT  bcd_size special!!!
        {28,    1,     4,        3,      1.0,    7,       0x40,    0x05},//FT-920
        {19,    2,     3,        2,      10.0,   7,       0x01,    0x7e},//FT-900
        {5,     0,     4,        1,      10.0,   4,       0x01,    0x0a},//FT-897
        {19,    2,     3,        2,      10.0,   7,       0x01,    0x7e},//FT-890
        {19,    2,     3,        2,      10.0,   7,       0x01,    0x7e},//FT-840
        {5,     0,     4,        1,      10.0,   4,       0x01,    0x0a},//FT-817
        {344,   1,     5,        1,      1.0,    24,      0x08,    0x7e},//FT-747GX   //2.46 = pos_mode=24
        {0,     0,     8,        0,      1.0,    0,       0x00,    0x00},//FT-450 NEWCAT  bcd_size special!!!
        {0,     0,     8,        0,      1.0,    0,       0x00,    0x00},//FT-450D NEWCAT  bcd_size special!!!
        {86,    1,     4,        1,      10.0,   6,       0x10,    0x7e},//FT-767GX
        {75,    5,     4,        0,      10.0,   9,       0x01,    0x7e},//FT-757GX
        {0,     0,     0,        0,      1.0,    0,       0x00,    0x7e},//FRG-9600 NO
        {0,     0,     0,        0,      1.0,    0,       0x00,    0x7e},//FRG-8800 NO
        {0,     0,     0,        0,      1.0,    0,       0x00,    0x7e},//FRG-100 NO
        {0,     0,     0,        0,      1.0,    0,       0x00,    0x7e},//VR-5000 NO
        {0,     0,     0,        0,      1.0,    0,       0x00,    0x7e},//FT-736R NO READ
        {148,   143,   4,        0,      10.0,   142,     0x01,    0x06},//FT-980 pos EXT CTRL=121
        {16,    1,     4,        3,      0.625,  7,       0x01,    0x05},//MARK-V FT-1000MP
        {16,    1,     4,        3,      0.625,  7,       0x01,    0x05},//MARK-V Field FT-1000MP
        {0,     0,     9,        0,      1.0,    0,       0x00,    0x00},//FTX-1F NEWCAT
        {0,     0,     9,        0,      1.0,    0,       0x00,    0x00},//FT-710 NEWCAT
        {0,     0,     9,        0,      1.0,    0,       0x00,    0x00},//FT-891 NEWCAT
        {0,     0,     9,        0,      1.0,    0,       0x00,    0x00},//FT-DX10 NEWCAT
        {0,     0,     9,        0,      1.0,    0,       0x00,    0x00},//FT-DX101 NEWCAT
        {0,     0,     9,        0,      1.0,    0,       0x00,    0x00},//FT-DX1200 NEWCAT
        {32,    9,     4,        3,      1.0,    7,       0x00,    0x07},//Ailunce HS2/HS3
        {32,    9,     4,        3,      1.0,    7,       0x00,    0x07},//Radioddity QR20
        {32,    9,     4,        3,      1.0,    7,       0x00,    0x07},//Guohe Q900/PMR-171/TBR-119
    };

static char cmd_OK_ft[YAESU_CMD_LENGTH] =
    {
        0x00,0x00,0x00,0x00,0x0b
    };
static char echo_back_ft[YAESU_CMD_LENGTH];

Yaesu::Yaesu(int ModelID,QWidget *parent)
        : QWidget(parent)
{
    s_ModelID = ModelID;
    s_rig_name = rigs_yeasu[s_ModelID].name;
    s_ncomp = ncmd[s_ModelID][NATIVE_CAT_PTT_ON].ncomp;
    if (s_rig_name=="FT-980" || s_rig_name=="FT-767GX") f_echo_back = true;
    else f_echo_back = false;
    i_ext_cntl = 0;
    oldcat_read_ar_size = read_parms[s_ModelID].cat_read_ar_size;
    s_pos_frq = read_parms[s_ModelID].pos_frq;
    s_bcd_size = read_parms[s_ModelID].bcd_size;
    s_method_frq = read_parms[s_ModelID].method_frq;
    s_multypl = read_parms[s_ModelID].multypl;
    s_pos_mod = read_parms[s_ModelID].pos_mod;
    s_id_mod_usb = read_parms[s_ModelID].id_mod_usb;
    s_id_mod_digu= read_parms[s_ModelID].id_mod_digu;
    s_CmdID = -1;
    s_read_array.clear();
    timer_cmd_ok = new QTimer();
    connect(timer_cmd_ok, SIGNAL(timeout()), this, SLOT(SetCmdOk()));
    timer_cmd_ok->setSingleShot(true);
    pmr171fB = 14999000;//max = 2.000.000.000 GHz
    pmr171mB = 0x00;//=USB
    //qDebug()<<"NEW RESET "<<s_rig_name<<s_ncomp;
}
Yaesu::~Yaesu()
{
    //qDebug()<<"Delete"<<rigs_yeasu[s_ModelID].name;
}
#include <unistd.h>
void Yaesu::SetCmdOk()
{
    emit EmitWriteCmd(cmd_OK_ft,YAESU_CMD_LENGTH);
    timer_cmd_ok->stop();
}
void Yaesu::SetWriteCmd(char *cmdd,int lenn)
{
    if (f_echo_back)
    {
        for (int i = 0; i<YAESU_CMD_LENGTH; ++i) echo_back_ft[i]=cmdd[i];
    }
    emit EmitWriteCmd(cmdd,lenn);
}
void Yaesu::SetExtCntl()//2.57
{
    unsigned char cmd[8];
    for (int i = 0; i < YAESU_CMD_LENGTH; i++) cmd[i]=ncmd[s_ModelID][CAT_ON].nseq[i];
    SetWriteCmd((char*)cmd,YAESU_CMD_LENGTH);
}
void Yaesu::SetOnOffCatCommand(bool f, int model_id, int fact_id)
{
    if (model_id!=s_ModelID || fact_id!=YAESU_ID) return;
    i_ext_cntl = 0;
    if (f && ncmd[s_ModelID][CAT_ON].ncomp==USECMD)
    {
        unsigned char cmd[50];//qDebug()<<"Yaesu CAT ON SetOnOffCatCommand="<<rigs_yeasu[model_id].name<<rigs_yeasu[s_ModelID].name<<f;
        int len = YAESU_CMD_LENGTH;
        if (s_ncomp == NEWCAT) len = strlen((char*)ncmd[s_ModelID][CAT_ON].nseq);
        if (s_ncomp == PMR0)
        {
            len = 9;
            for (int i = 0; i < len; ++i) //0:q900 0:HS2 0:TBR-119 ??? -> A5 A5 A5 A5 04 27 00 8F 2D  A5 A5 A5 A5 04 27 01 9F 0C
            {
                if (i<4) cmd[i] = 0xa5;
                else cmd[i]=ncmd[s_ModelID][CAT_ON].nseq[i-4];
            }
        }
        else
        {
            for (int i = 0; i < len; i++) cmd[i]=ncmd[s_ModelID][CAT_ON].nseq[i];
        }
        if (s_rig_name=="FT-980") i_ext_cntl = 2;//2.57 special ID for ft980
        else SetWriteCmd((char*)cmd,len); //2.57 else ,EmitWriteCmd((char*)cmd,len);
    }
    else if (!f && ncmd[s_ModelID][CAT_OFF].ncomp==USECMD)
    {
        unsigned char cmd[50];//qDebug()<<"Yaesu CAT OFF SetOnOffCatCommand<========"<<rigs_yeasu[model_id].name<<rigs_yeasu[s_ModelID].name<<f;
        int len = YAESU_CMD_LENGTH;
        if (s_ncomp == NEWCAT) len = strlen((char*)ncmd[s_ModelID][CAT_OFF].nseq);
        for (int i = 0; i < len; i++) cmd[i]=ncmd[s_ModelID][CAT_OFF].nseq[i];
        emit EmitWriteCmd((char*)cmd,len);
        if (f_echo_back)// no from timer
        {
            usleep(80000);//for (int i = 0; i<YAESU_CMD_LENGTH; ++i) echo_back_ft980[i]=cmd[i];
            emit EmitWriteCmd(cmd_OK_ft,YAESU_CMD_LENGTH);
        }
    }
}
void Yaesu::SetCmd(CmdID i,ptt_t ptt,QString str)
{
    switch (i)
    {
    case GET_SETT:
        emit EmitRigSet(rigs_yeasu[s_ModelID]);
        break;
    case SET_PTT:
        set_ptt(ptt);
        break;
    case SET_FREQ:
        set_freq(str.toLongLong());
        break;
    case GET_FREQ:
        s_CmdID = GET_FREQ;
        get_freq();
        break;
    case SET_MODE:
        set_mode(str);
        break;
    case GET_MODE:
        s_CmdID = GET_MODE;
        get_mode();
        break;
    }
}
/*int crc16(const unsigned char *bytes, int len)
{
	int crc = 0xFFFF;
    for (int j = 0; j < len; ++j)
    {
    	crc = ((crc >> 8) | (crc << 8)) & 0xffff;
        crc ^= (bytes[j] & 0xff);
        crc ^= ((crc & 0xff) >> 4);
        crc ^= (crc << 12) & 0xffff;
        crc ^= ((crc & 0xFF) << 5) & 0xffff;
    }
    crc &= 0xffff;
    return crc;
}*/
uint16_t Yaesu::CRC16Check(const unsigned char *buf, int len)
{
    uint16_t crc = 0xFFFF;
    uint16_t polynomial = 0x1021;
    for (int i = 0; i < len; i++)
    {
        crc ^= ((uint16_t)buf[i] << 8);
        for (int j = 0; j < 8; j++)
        {
            if (crc & 0x8000) crc = (crc << 1) ^ polynomial;
            else crc = crc << 1;
        }
    } //qDebug()<<"New0="<<crc<<"New1="<<crc16(buf,len);
    return crc;
}
void Yaesu::set_ptt(ptt_t ptt)
{
    unsigned char *cmd;/* points to sequence to send */
    if (ptt==RIG_PTT_ON) cmd = (unsigned char *) &ncmd[s_ModelID][NATIVE_CAT_PTT_ON].nseq; /* get native sequence */
    else cmd = (unsigned char *) &ncmd[s_ModelID][NATIVE_CAT_PTT_OFF].nseq;
    if (s_ncomp == NEWCAT)
    {
        int len = strlen((const char*)cmd);
        emit EmitWriteCmd((char *)cmd,len); /* get native sequence */
    }
    else if (s_ncomp == PMR0)
    {
        unsigned char buf[11]={0xa5,0xa5,0xa5,0xa5,0x04,0x00,0x00,0x00,0x00, 0x00};//9
        buf[5]=cmd[0];
        buf[6]=cmd[1];
        unsigned int crc = CRC16Check(&buf[4],3);
        buf[7] = crc >> 8;
        buf[8] = crc & 0xff;
        SetWriteCmd((char *)buf,9);
    }
    else SetWriteCmd((char *)cmd,YAESU_CMD_LENGTH);//EmitWriteCmd((char *)cmd,YAESU_CMD_LENGTH); /* get native sequence */
}
void Yaesu::set_freq(unsigned long long freq)
{
    if (s_ncomp == NEWCAT)
    {
        ////991,891  -> 14.250.000          FA;<-read ansver->FA014250000;
        //FT2000,5000,9000,950,450,450d /10 FA;<-read ansver->FA14250000;
        char cmdnc[40];//1.75 [30 to 40] out of baunds  O4
        QString frq = (char*)ncmd[s_ModelID][NATIVE_CAT_SET_FREQ].nseq;
        if (s_bcd_size==8) frq.append(QString("%1").arg(freq,8,10,QChar('0'))); //FT2000,5000,9000,950,450,450d /10 //FA;<-read ansver->FA14250000;
        else frq.append(QString("%1").arg(freq,9,10,QChar('0'))); //FA;<-read ansver->FA014250000;
        frq.append(";");
        for (int i = 0; i < frq.count(); i++) cmdnc[i]=frq.at(i).toLatin1();
        emit EmitWriteCmd(cmdnc,frq.count()); /* get native sequence */
        //qDebug()<<"FREQ==========="<<frq;
    }
    else if (s_ncomp == PMR0)
    {
        if (freq > 2000000000) return; //qDebug()<<"FREQ==========="<<freq;
        unsigned char *cmd = (unsigned char *)ncmd[s_ModelID][NATIVE_CAT_SET_FREQ].nseq;
        unsigned char buf[18]={0xa5,0xa5,0xa5,0xa5,0x0b,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0x00};//16
        buf[5]=cmd[0];
        buf[6]=((freq >> 24) & 0xff);//to_be(&buf[6],freq,4);
        buf[7]=((freq >> 16) & 0xff);
        buf[8]=((freq >>  8) & 0xff);
        buf[9]=( freq & 0xff);
        buf[10]=((pmr171fB >> 24) & 0xff);//to_be(&buf[10],freq,4);
        buf[11]=((pmr171fB >> 16) & 0xff);
        buf[12]=((pmr171fB >>  8) & 0xff);
        buf[13]=( pmr171fB & 0xff);
        unsigned int crc = CRC16Check(&buf[4],10);//unsigned int crc = CRC16Check(&buf[4],12);
        buf[14] = crc >> 8;
        buf[15] = crc & 0xff;
        SetWriteCmd((char *)buf,16);
    }
    else
    {
        unsigned char cmd[50];
        for (int i = 0; i < YAESU_CMD_LENGTH; i++) cmd[i]=ncmd[s_ModelID][NATIVE_CAT_SET_FREQ].nseq[i];
        //to_bcd_be(data, (freq + 5) / 10, 8);//847,857,897,817
        //to_bcd(p->p_cmd, (freq+12)/10, 8);//747
        //to_bcd_be(cmd+1, freq/10, 8); frg9600
        if (s_rig_name=="FT-847" || s_rig_name=="FT-857" || s_rig_name=="FT-897" || s_rig_name=="FT-817")
        {
            if (s_rig_name=="FT-847") freq = (int)(freq)/10;
            else freq = (int)(freq+5)/10;
            to_bcd_be_(cmd,freq,8);
        }
        else if (s_rig_name=="FT-747GX")
        {
            //freq = (int)(freq+12)/10;
            freq = (int)(freq+5)/10;//2.46
            to_bcd_(cmd,freq,8);//??? be triabva da e
        }
        else
        {
            freq = (int)(freq)/10;
            to_bcd_(cmd,freq,8);
        }
        //to_bcd_(cmd,freq,8);	/*store bcd format in in cmd*/
        SetWriteCmd((char*)cmd,YAESU_CMD_LENGTH);//EmitWriteCmd((char*)cmd,YAESU_CMD_LENGTH); /* get native sequence */
    }
}
void Yaesu::set_mode(QString str)
{
    if (s_ncomp == NEWCAT)//{ NEWCAT, { "MD0;" } }, /*identif newcat NEWCAT FA;  set mode USB = 2 moze i da e C -> data-usb*/
    {
        unsigned char cmdnc[11];//MD0 2; <-USB
        QString mod = (char*)ncmd[s_ModelID][NATIVE_CAT_SET_MODE_USB].nseq;
        if 		(str=="LSB" ) mod.append("1");
        else if (str=="USB" ) mod.append("2");
        else if (str=="DIGU") mod.append("C");//DIGU
        else return;//mod.append("2");//usb
        mod.append(";");
        for (int i = 0; i < mod.count(); i++) cmdnc[i]=mod.at(i).toLatin1();
        emit EmitWriteCmd((char *)cmdnc,mod.count());//qDebug()<<"MOD==========="<<mod;
    }
    else if (s_ncomp == PMR0)
    {
        unsigned char *cmd; //From Guohe PMR-171 v.1.5 protocol
        if (str=="USB") cmd = (unsigned char *) &ncmd[s_ModelID][NATIVE_CAT_SET_MODE_USB].nseq;
        else if (str=="DIGU") cmd = (unsigned char *) &ncmd[s_ModelID][NATIVE_CAT_SET_MODE_DIGU].nseq;
        else return;
        unsigned char buf[12]={0xa5,0xa5,0xa5,0xa5,0x05,0x00,0x00,0x00,0x00,0x00, 0x00};//
        buf[5]=cmd[0];
        buf[6]=cmd[1];
        buf[7]=pmr171mB;
        unsigned int crc = CRC16Check(&buf[4],4);
        buf[8] = crc >> 8;
        buf[9] = crc & 0xff;
        SetWriteCmd((char *)buf,10);
        //From CNSDR v.1.?? protocol
        /*unsigned char buf[11]={0xa5,0xa5,0xa5,0xa5,0x04,0x00,0x00,0x00,0x00, 0x00};//
        buf[5]=cmd[0];
        buf[6]=cmd[1];
        unsigned int crc = CRC16Check(&buf[4],3);
        buf[7] = crc >> 8;
        buf[8] = crc & 0xff;
        SetWriteCmd((char *)buf,9);*/
    }
    else
    {
        unsigned char cmd[20];
        if 		(str=="USB")
        {
            for (int i = 0; i < YAESU_CMD_LENGTH; i++) cmd[i]=ncmd[s_ModelID][NATIVE_CAT_SET_MODE_USB].nseq[i];//set USB
        }
        else if (str=="DIGU" && s_id_mod_digu!=0x7e)
        {
            for (int i = 0; i < YAESU_CMD_LENGTH; i++) cmd[i]=ncmd[s_ModelID][NATIVE_CAT_SET_MODE_DIGU].nseq[i];//set DIGU
        }
        else return;
        SetWriteCmd((char*)cmd,YAESU_CMD_LENGTH);//EmitWriteCmd((char*)cmd,YAESU_CMD_LENGTH);
    }
}
bool Yaesu::WaitSetExtCntl() //2.57 ft980
{
    if (i_ext_cntl < 1) return false;
    if (i_ext_cntl < 2)
    {
        s_CmdID = 200;//special id
        SetExtCntl();
    }
    i_ext_cntl--;
    return true;
}
void Yaesu::get_freq()
{
    if (s_ncomp == NEWCAT) //osobenno -> FT-767GX FT747 from_bcd_be "FT-897" FT-817
    {
        char *cmdnc; //FA;<-read ansver->FA014250000;
        cmdnc =  (char *)ncmd[s_ModelID][NATIVE_CAT_GET_FREQ_MODE_STATUS].nseq;
        int len = strlen(cmdnc);
        emit EmitWriteCmd(cmdnc,len);
    }
    else if (s_ncomp == PMR0)
    {
        s_read_array.clear(); //for error corection no word end
        unsigned char *cmd =(unsigned char *)&ncmd[s_ModelID][NATIVE_CAT_GET_FREQ_MODE_STATUS].nseq;
        unsigned char buf[10]={0xa5,0xa5,0xa5,0xa5,0x03,0x00,0x00,0x00, 0x00};//
        buf[5] = cmd[0];
        unsigned int crc = CRC16Check(&buf[4],2);//3
        buf[6] = crc >> 8;
        buf[7] = crc & 0xff;
        SetWriteCmd((char *)buf,8);
    }
    else
    {
        if (WaitSetExtCntl()) return; //2.57 //qDebug()<<"FR COMMAND="<<i_ext_cntl;
        s_read_array.clear(); //for error corection no word end
        unsigned char cmd[10];
        for (int i = 0; i < YAESU_CMD_LENGTH; i++) cmd[i]=ncmd[s_ModelID][NATIVE_CAT_GET_FREQ_MODE_STATUS].nseq[i];
        SetWriteCmd((char *)cmd,YAESU_CMD_LENGTH);//EmitWriteCmd((char *)cmd,YAESU_CMD_LENGTH);
    }
}
void Yaesu::get_mode()
{
    if (s_ncomp == NEWCAT) //osobenno -> FT-767GX FT747 from_bcd_be "FT-897" FT-817
    {
        char *cmdnc; //MD0;<-read ansver->MD02;
        cmdnc =  (char *)ncmd[s_ModelID][NATIVE_CAT_GET_MODE].nseq;
        int len = strlen(cmdnc);
        emit EmitWriteCmd(cmdnc,len);
    }
    else if (s_ncomp == PMR0)
    {
        s_read_array.clear(); //for error corection no word end
        unsigned char *cmd =(unsigned char *)&ncmd[s_ModelID][NATIVE_CAT_GET_MODE].nseq;
        unsigned char buf[10]={0xa5,0xa5,0xa5,0xa5,0x03,0x00,0x00,0x00, 0x00};//
        buf[5] = cmd[0];
        unsigned int crc = CRC16Check(&buf[4],2);//3
        buf[6] = crc >> 8;
        buf[7] = crc & 0xff;
        SetWriteCmd((char *)buf,8);
    }
    else
    {
        if (WaitSetExtCntl()) return; //2.57 qDebug()<<"MD COMMAND="<<i_ext_cntl;
        s_read_array.clear(); //for error corection no word end
        unsigned char cmd[10];
        for (int i = 0; i < YAESU_CMD_LENGTH; i++) cmd[i]=ncmd[s_ModelID][NATIVE_CAT_GET_MODE].nseq[i];
        SetWriteCmd((char *)cmd,YAESU_CMD_LENGTH);//EmitWriteCmd((char *)cmd,YAESU_CMD_LENGTH);
    }
}
void Yaesu::SetReadyRead(QByteArray ar,int size0)
{
    for (int i = 0; i < size0; i++)
    {
        if (s_ncomp == NEWCAT)
        {
            if ((unsigned char)ar[i]==FI)//;=hex value ??? end of word EOM
            {
                //qDebug()<<"YESU READ ALL COMMAND="<<(QString(s_read_array.toHex()))<<s_read_array.size();
                //s_read_array.append(ar[i]); // no ;=fi added
                int size = s_read_array.size();
                //rejekt small array
                if (size < 4)//to get mde ansver->MD02;  ansver->FA014250000;
                {
                    s_read_array.clear();//qDebug()<<"NO INAF SIZE";
                    return;
                }//F=0x46 A=0x41
                if (s_CmdID==GET_FREQ && s_read_array[0]==(char)0x46 && s_read_array[1]==(char)0x41)//my qestion for freq and freq filter vfoA
                {
                    if (size==11 || size==10)//bez fd=fi (size==11)
                    {
                        QByteArray tfreq;
                        if (s_bcd_size==8) tfreq.append(s_read_array.mid(2,8)); //FA;<-read ansver->FA14250000   ;//FT2000,5000,9000,950,450,450d /10
                        else tfreq.append(s_read_array.mid(2,9)); //FA;<-read ansver->FA014250000  ;
                        unsigned long long f = tfreq.toLongLong();
                        emit EmitReadedInfo(GET_FREQ,QString("%1").arg(f));
                        s_CmdID = -1;//I Find my answer no need more
                    }
                }//M=0x4d D=0x44 0=0x30
                if (s_CmdID==GET_MODE && s_read_array[0]==(char)0x4d && s_read_array[1]==(char)0x44 && s_read_array[2]==(char)0x30)//my qestion for mode and mode filter
                {
                    if (size==4)//no fd=fi 4
                    {
                        QString smode = "WRONG_MODE";
                        if 		(s_read_array[3]==(char)0x31) smode = "LSB"; //0x31=LSB=1
                        else if (s_read_array[3]==(char)0x32) smode = "USB"; //0x32=USB=2
                        else if (s_read_array[3]==(char)0x33) smode = "CWU";//0x33=CW-U=3
                        else if (s_read_array[3]==(char)0x34) smode = "FM";  //0x34=FM=4
                        else if (s_read_array[3]==(char)0x35) smode = "AM";  //0x35=AM=5
                        else if (s_read_array[3]==(char)0x36) smode = "R-L"; //0x36=RTTY-LSB=6
                        else if (s_read_array[3]==(char)0x37) smode = "CWL";//0x37=CW-L=7
                        else if (s_read_array[3]==(char)0x38) smode = "DIGL";//0x38=DATA-LSB=8
                        else if (s_read_array[3]==(char)0x39) smode = "R-U"; //0x39=RTTY-USB=9
                        else if (s_read_array[3]==(char)0x41) smode = "FM-D"; //0x41=DATA-FM=A
                        //else if (s_read_array[3]==(char)0x42) smode = "FM-N";//0x42=FM-N=B
                        else if (s_read_array[3]==(char)0x43) smode = "DIGU";//0x43=DATA-USB=C
                        //else if (s_read_array[3]==(char)0x44) smode = "AM-N";//0x44=AM-N=D
                        else if (s_read_array[3]==(char)0x45) smode = "C4FM";//0x44=AM-N=D
                        emit EmitReadedInfo(GET_MODE,smode);
                        s_CmdID = -1;//I Find my answer no need more
                    }
                }
                s_read_array.clear();
            }
            else s_read_array.append(ar[i]);
        }
        else if (s_ncomp == PMR0)//Guohe Q900/PMR-171/TBR-119, Ailunce HS2, Radioddity QR20, Retevis Ailunce HS3
        {	//a5 a5 a5 a5 1b 0b 00 07 07 00 D6 C0 90 00 E4 DD D8 00 00 3c 3c 00 00 00 00 00 00 00 00 00 49 3d //DIGU 14.074.000
            //a5 a5 a5 a5 1b 0b 00 00 00 00 D7 C0 90 00 D5 9F 80 00 00 3c 3c 00 00 00 00 00 00 00 00 00 EA 7E // USB 14.139.536
            //a5 a5 a5 a5 1b 0b 00 07 07 00 D6 C1 58 00 E4 DD D8 00 00 3c 3c 00 00 00 00 00 00 00 00 00 7A CD //DIGU 14.074.200
            s_read_array.append(ar[i]);
            int size = s_read_array.size();
            bool f0 = true; //LZ2HV method for recognizing truncated (broken) message (have begin identification not have end character)
            if 		(size == 1)
            {
                if (s_read_array[0]!=(char)0xa5) f0 = false;
            }
            else if (size == 2)
            {
                if (s_read_array[1]!=(char)0xa5) f0 = false;
            }
            else if (size == 3)
            {
                if (s_read_array[2]!=(char)0xa5) f0 = false;
            }
            else if (size == 4)
            {
                if (s_read_array[3]!=(char)0xa5) f0 = false;
            }
            else if (size == 5)
            {
                if (s_read_array[4]!=(char)0x1b) f0 = false;
            }
            else if (size == 6)
            {
                if (s_read_array[5]!=(char)0x0b) f0 = false;
            }
            if (!f0)
            {
                s_read_array.clear();
                continue;
            }
            if (size >= oldcat_read_ar_size)//oldcat_read_ar_size &&//17 to full=32 
            {	//qDebug()<<"YESU READ ALL COMMAND="<<(QString(s_read_array.toHex()))<<s_read_array.size();
                unsigned char *fbcd = new unsigned char[oldcat_read_ar_size+2];
                for (int j = 0; j < oldcat_read_ar_size; ++j) fbcd[j]=(unsigned char)s_read_array[j];
                unsigned int end = ((fbcd[30]&0xff)<<8)|(fbcd[31]&0xff);
                unsigned int crc = CRC16Check(&fbcd[4],26);//qDebug()<<end<<crc;
                if ((s_CmdID==GET_FREQ || s_CmdID==GET_MODE) && end==crc)
                {
                    unsigned long long freq = 0;
                    freq = (unsigned long long)((((((fbcd[s_pos_frq]<<8) + fbcd[s_pos_frq+1])<<8) + fbcd[s_pos_frq+2])<<8) + fbcd[s_pos_frq+3]);//from_be(fbcd,4);
                    emit EmitReadedInfo(GET_FREQ,QString("%1").arg(freq));
                    pmr171fB = (unsigned long long)((((((fbcd[s_pos_frq+4]<<8) + fbcd[s_pos_frq+5])<<8) + fbcd[s_pos_frq+6])<<8) + fbcd[s_pos_frq+7]);//from_be(fbcd,4);
                    unsigned char mod = (unsigned char)s_read_array[s_pos_mod];
                    QString smode = "WRONG_MODE";
                    if 		(mod==s_id_mod_usb) smode = "USB";//USB
                    else if (mod==s_id_mod_digu) smode = "DIGU";//DATA-USB
                    else if (mod==0x01) smode = "LSB";//LSB
                    else if (mod==0x02) smode = "CWR";//2:CWR
                    else if (mod==0x03) smode = "CWL";//3:CWL
                    else if (mod==0x04) smode = "AM"; //4:AM
                    else if (mod==0x05) smode = "WFM";//5:WFM
                    else if (mod==0x06) smode = "NFM";//6:NFM
                    else if (mod==0x08) smode = "PKT";//8:PKT
                    emit EmitReadedInfo(GET_MODE,smode);
                    pmr171mB = (unsigned char)s_read_array[s_pos_mod+1]; //qDebug()<<"MODE-B="<<pmr171mB<<"FREQ-B="<<pmr171fB;
                }
                s_CmdID = -1;
                s_read_array.clear();
                delete [] fbcd;
            }
        }
        else
        {
            s_read_array.append(ar[i]);
            if (f_echo_back)//2.56 ft980 ft767gx
            {
                if (s_read_array.size()==5 && size0==5)
                {
                    if (    s_read_array[0]==(char)echo_back_ft[0] && s_read_array[1]==(char)echo_back_ft[1] &&
                            s_read_array[2]==(char)echo_back_ft[2] && s_read_array[3]==(char)echo_back_ft[3] &&
                            s_read_array[4]==(char)echo_back_ft[4])
                    {
                        s_read_array.clear();
                        timer_cmd_ok->start(20);
                        continue;
                    }
                }
            }
            if (s_read_array.size()==oldcat_read_ar_size)
            {   //qDebug()<<"YAESU READ ALL COMMAND="<<(QString(s_read_array.toHex()))<<s_read_array.size();
                if (s_CmdID==200) //2.57 ft980 special ID for ft980
                {
                    unsigned char ext_cntl = (unsigned char)s_read_array[121];//qDebug()<<"COMMAND="<<s_CmdID<<(unsigned char)s_read_array[121];
                    if (ext_cntl==0x00) QTimer::singleShot(20, this, SLOT(SetExtCntl()));
                }
                if (s_CmdID==GET_FREQ || s_CmdID==GET_MODE)
                {
                    unsigned char *fbcd = new unsigned char[s_bcd_size+2];//fbcd[s_bcd_size+2];
                    for (int j=0; j < s_bcd_size; j++) fbcd[j]=(unsigned char)s_read_array[j+s_pos_frq];//2.12

                    unsigned long long freq = 0;//s_method_frq-> 0=from_bcd,1=from_bcd_be,2=3int,3=4int,4=ft100
                    if      (s_method_frq==0) freq = from_bcd_(fbcd, s_bcd_size*2)*s_multypl;
                    else if (s_method_frq==1) freq = from_bcd_be_(fbcd, s_bcd_size*2)*s_multypl;
                    else if (s_method_frq==2) freq = ((((fbcd[0]<<8) + fbcd[1])<<8) + fbcd[2])*s_multypl;
                    else if (s_method_frq==3) freq = (int)((double)((((((fbcd[0]<<8) + fbcd[1])<<8) + fbcd[2])<<8) + fbcd[3])*s_multypl);
                    else if (s_method_frq==4)
                    {
                        char freq_str[20];
                        sprintf(freq_str, "%02X%02X%02X%02X",fbcd[0],fbcd[1],fbcd[2],fbcd[3]);
                        unsigned long long d1=strtol(freq_str,NULL,16);
                        freq =(int)((double)d1*s_multypl);
                    }
                    emit EmitReadedInfo(GET_FREQ,QString("%1").arg(freq));

                    unsigned char mod = (unsigned char)s_read_array[s_pos_mod];
                    if (s_rig_name=="FT-747GX")//2.46
                    {
                        mod &= 0x9f;
                        mod = (mod & 0x1f);  //qDebug()<<"MODE="<<mod<<s_rig_name;
                    }
                    QString smode = "WRONG_MODE";
                    if 		(mod==s_id_mod_usb) smode = "USB";//USB
                    else if (mod==s_id_mod_digu && s_id_mod_digu!=0x7e) smode = "DIGU";//DATA-USB  no suported 0x7e
                    emit EmitReadedInfo(GET_MODE,smode);

                    delete [] fbcd;
                }
                s_CmdID = -1;
                s_read_array.clear();
            }
        }
    } //qDebug()<<s_read_array.size();
    if (s_read_array.size()>1024) s_read_array.clear(); //2.55 protection  max>344=FT-747GX=512  something is wrong
}




