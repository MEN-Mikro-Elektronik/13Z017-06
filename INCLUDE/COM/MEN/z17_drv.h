/***********************  I n c l u d e  -  F i l e  ************************/
/*!
 *        \file  z17_drv.h
 *
 *      \author  ulrich.bogensperger@men.de
 *        $Date: 2014/11/28 14:17:53 $
 *    $Revision: 2.6 $
 *
 *       \brief  Header file for Z17 driver containing
 *               Z17 specific status codes and
 *               Z17 function prototypes
 *
 *    \switches  _ONE_NAMESPACE_PER_DRIVER_
 *               _LL_DRV_
 */
 /*-------------------------------[ History ]--------------------------------
 *
 * $Log: z17_drv.h,v $
 * Revision 2.6  2014/11/28 14:17:53  MRoth
 * R: cosmetics
 * M: revised code completely
 *
 * Revision 2.5  2011/07/08 13:20:17  dpfeuffer
 * R: IRQ latency test required for 16G215-01 design test
 * M: IRQ latency test implemented
 *
 * Revision 2.4  2009/08/03 16:43:29  MRoth
 * R: no support for 32bit Z127_GPIO
 * M: a) added define Z17_IRQ_SENSE_16TO31
 *    b) fixed doxygen comment of define Z17_IRQ_SENSE
 *
 * Revision 2.3  2009/07/10 10:18:06  CRuff
 * R: need backward compatibility to MDIS4 (Linux)
 * M: added defines for data types INT32_OR_64 and MDIS_PATH
 *
 * Revision 2.2  2006/12/20 11:19:13  ufranke
 * added
 *  + getstat code Z17_IRQ_LAST_REQUEST
 *  + set/getstat value documentation
 *
 * Revision 2.1  2004/06/18 14:30:03  ub
 * Initial Revision
 *
 *---------------------------------------------------------------------------
 * (c) Copyright 2004 by MEN mikro elektronik GmbH, Nuernberg, Germany
 ****************************************************************************/

#ifndef _Z17_DRV_H
#define _Z17_DRV_H

#ifdef __cplusplus
	extern "C" {
#endif

/*-----------------------------------------+
|  TYPEDEFS                                |
+-----------------------------------------*/
/* structure for the Z17_BLK_IRQLAT_X setstat/getstat */
typedef struct {
	/* in */
	u_int32 outPort;    /* output port (0..31) */
	u_int32 inPort;     /* input port (0..31) */
	u_int32 irqs2fire;  /* number of interrupts to fire */
	/* out */
	u_int32 tickRate;   /* tick rate from OSS_TickRateGet */
	u_int32 startTick;  /* start tick count */
	u_int32 stopTick;   /* stop tick count */	
	u_int32 irqsRcved;  /* received IRQs */
} Z17_BLK_IRQLAT;

/* structure for the Z17_BLK_DEBOUNCE_TIME setstat/getstat */
typedef struct {
	/* in */
	u_int32 portMask;   /* port mask (0x0..0xffffffff) */
	/* in/out */
	u_int32 timeUs;   	/* time in us (in 50us steps) */
} Z17_BLK_DEBTIME;

/*-----------------------------------------+
|  DEFINES                                 |
+-----------------------------------------*/
/** \name Z17 specific Getstat/Setstat standard codes
 *  \anchor getstat_setstat_codes
 */
/**@{*/
#define Z17_SET_PORTS           M_DEV_OF+0x00    /**<   S: Set IO ports without affecting others - 8 bit mask - bit 0 for gpio[0] */
#define Z17_CLR_PORTS           M_DEV_OF+0x01    /**<   S: Clear IO ports without affecting others - 8 bit mask - bit 0 for gpio[0] */
#define Z17_DIRECTION           M_DEV_OF+0x02    /**< G,S: Get/Set direction of all ports - 8bit - bit 0 for gpio[0] - value 0 is in - 1 is out  */
#define Z17_IRQ_SENSE           M_DEV_OF+0x03    /**< G,S: Get/set IRQ sense mode for models Z34/Z37 - 16 bit - and for model Z127 - 32 bit - bit 0 and 1 for gpio[0] - 0 no, 1 rising, 2 falling, 3 both edges */
#define Z17_OPEN_DRAIN          M_DEV_OF+0x04    /**< G,S: Get/set open drain mode - 8bit - bit 0 for gpio[0] - value 0 is dis - 1 is enable */
#define Z17_DEBOUNCE            M_DEV_OF+0x05    /**< G,S: Get/set debouncing of all ports - 8bit - bit 0 for gpio[0] - value 0 is dis - 1 is enable */
#define Z17_SET_SIGNAL          M_DEV_OF+0x06    /**<   S: Set signal sent on IRQ  */
#define Z17_CLR_SIGNAL          M_DEV_OF+0x07    /**<   S: Uninstall signal        */
#define Z17_IRQ_LAST_REQUEST    M_DEV_OF+0x08    /**< G  : Get last IRQ request - 8bit - bit 0 for gpio[0] - value 1 is requested - clears the last request  */
#define Z17_IRQ_SENSE_16TO31    M_DEV_OF+0x09    /**< G,S: Get/set IRQ sense mode only for model Z127 - 32 bit - bit 0 and 1 for gpio[16] - 0 no, 1 rising, 2 falling, 3 both edges */
#define Z17_TOG_PORTS           M_DEV_OF+0x0a    /**<   S: Toggle IO ports without affecting others - 8/32 bit mask - bit 0 for gpio[0] */
#define Z17_TOG_HIGH            M_DEV_OF+0x0b    /**<   S: Toggle phase high - default 1000ms */
#define Z17_TOG_LOW             M_DEV_OF+0x0c    /**<   S: Toggle phase low - default 1000ms */

/* Z17 specific Getstat/Setstat block codes (for test purposes) */
#define Z17_BLK_IRQLAT_START    M_DEV_BLK_OF+0x00	/*   S: Initialize IRQ latency test */
#define Z17_BLK_IRQLAT_RESULT   M_DEV_BLK_OF+0x01	/* G  : Get result of IRQ latency test */
#define Z17_BLK_DEBOUNCE_TIME   M_DEV_BLK_OF+0x02	/* G,S: Get/set debounce time (us) for specified port(s) */
													/*      Get: specify only one port */
													/*      Set: specify one ore more ports */			
/**@}*/

#ifndef  Z17_VARIANT
  #define Z17_VARIANT    Z17
#endif

#define _Z17_GLOBNAME(var,name) var##_##name

#ifndef _ONE_NAMESPACE_PER_DRIVER_
  #define Z17_GLOBNAME(var,name)    _Z17_GLOBNAME(var,name)
#else
  #define Z17_GLOBNAME(var,name)    _Z17_GLOBNAME(Z17,name)
#endif

#define __Z17_GetEntry    Z17_GLOBNAME(Z17_VARIANT, GetEntry)

/*-----------------------------------------+
|  PROTOTYPES                              |
+-----------------------------------------*/
#ifdef _LL_DRV_
#ifndef _ONE_NAMESPACE_PER_DRIVER_
	extern void __Z17_GetEntry(LL_ENTRY* drvP);
#endif
#endif /* _LL_DRV_ */

/*-----------------------------------------+
|  BACKWARD COMPATIBILITY TO MDIS4         |
+-----------------------------------------*/
#ifndef U_INT32_OR_64
  /* we have an MDIS4 men_types.h and mdis_api.h included */
  /* only 32bit compatibility needed!                     */
  #define INT32_OR_64    int32
  #define U_INT32_OR_64  u_int32
  typedef INT32_OR_64    MDIS_PATH;
#endif /* U_INT32_OR_64 */

#ifdef __cplusplus
	}
#endif

#endif /* _Z17_DRV_H */
 
 
