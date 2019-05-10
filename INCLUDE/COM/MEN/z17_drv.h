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
 /*
 *---------------------------------------------------------------------------
 * Copyright (c) 2004-2019, MEN Mikro Elektronik GmbH
 ****************************************************************************/
/*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

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
#define Z17_SET_PORTS           M_DEV_OF+0x00    /**<   S: Set IO ports without affecting others - bit 0 for gpio[0] */
#define Z17_CLR_PORTS           M_DEV_OF+0x01    /**<   S: Clear IO ports without affecting others - bit 0 for gpio[0] */
#define Z17_DIRECTION           M_DEV_OF+0x02    /**< G,S: Get/Set direction of all ports - bit 0 for gpio[0] - value 0 is in - 1 is out  */
#define Z17_IRQ_SENSE           M_DEV_OF+0x03    /**< G,S: Get/set IRQ sense mode for models Z34/Z37 - 16 bit - and for model Z127 - 32 bit - bit 0 and 1 for gpio[0] - 0 no, 1 rising, 2 falling, 3 both edges */
#define Z17_OPEN_DRAIN          M_DEV_OF+0x04    /**< G,S: Get/set open drain mode - bit 0 for gpio[0] - value 0 is dis - 1 is enable */
#define Z17_DEBOUNCE            M_DEV_OF+0x05    /**< G,S: Get/set debouncing of all ports - bit 0 for gpio[0] - value 0 is dis - 1 is enable */
#define Z17_SET_SIGNAL          M_DEV_OF+0x06    /**<   S: Set signal sent on IRQ  */
#define Z17_CLR_SIGNAL          M_DEV_OF+0x07    /**<   S: Uninstall signal        */
#define Z17_IRQ_LAST_REQUEST    M_DEV_OF+0x08    /**< G  : Get last IRQ request - bit 0 for gpio[0] - value 1 is requested - clears the last request  */
#define Z17_IRQ_SENSE_16TO31    M_DEV_OF+0x09    /**< G,S: Get/set IRQ sense mode only for model Z127 - 32 bit - bit 0 and 1 for gpio[16] - 0 no, 1 rising, 2 falling, 3 both edges */
#define Z17_TOG_PORTS           M_DEV_OF+0x0a    /**<   S: Toggle IO ports without affecting others - bit 0 for gpio[0] */
#define Z17_TOG_HIGH            M_DEV_OF+0x0b    /**<   S: Toggle phase high - default 1000ms */
#define Z17_TOG_LOW             M_DEV_OF+0x0c    /**<   S: Toggle phase low - default 1000ms */

/* Z17 specific Getstat/Setstat block codes */
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
 
 
 
 
 
