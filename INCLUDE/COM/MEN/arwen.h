/***********************  I n c l u d e  -  F i l e  ************************
 *
 *         Name: arwen.h
 *
 *       Author: kp
 *        $Date: 2009/08/03 16:43:31 $
 *    $Revision: 2.5 $
 *
 *  Description: Header file for arwen GPIO in Chameleon FPGA
 *
 *     Switches: -
 *
 *---------------------------------------------------------------------------
 * (c) Copyright 2004 by MEN Mikro Elektronik GmbH, Nuremberg, Germany
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

#ifndef _ARWEN_H
#define _ARWEN_H

#ifdef __cplusplus
	extern "C" {
#endif

/*--------------------------------------+
|   DEFINES                             |
+--------------------------------------*/

/*--- register offsets for org. Arwen (and Arwen_01/DAK) ---*/

#define ARWEN_GPIO		0x00	/**< input/output data */

/** interrupt enable reg
 * Arwen 00: only bit 0 implemented (global irq enable)
 * Arwen 01: Bits 7..0 implemented
 */
#define ARWEN_00_IER		0x04

/** interrupt request reg
 * 00/01: same implementation as IER
 */
#define ARWEN_IRR			0x08

/** data direction register
 * Arwen 00: not implemented!
 * Arwen 01/02: implemented
 *
 * 0=input, 1=output
 */
#define ARWEN_DDR		0x0c

/*--- special register offsets for Arwen 02  ---*/

/** port state reg
 * read-only: reads the current state of all GPIO pins
 */
#define ARWEN_02_PSR   		0x04

/** interrupt enables for inputs 3..0 for Z34/Z37 and 15..0 for Z127
 * for each port pin, 2 bits in this reg:
 * 00 = irq off
 * 01 = irq on rising edge
 * 10 = irq on falling edge
 * 11 = irq on both edges
 */
#define ARWEN_02_IER1		0x10

/** interrupt enables for inputs 7..4 for Z34/Z37 and 31..16 for Z127
 * for each port pin, 2 bits in this reg.
 */
#define ARWEN_02_IER2		0x14

/** debounce enable reg
 * 0=debouncer disabled, 1=enabled
 */
#define ARWEN_02_DBER		0x18

/** open drain enable reg
 * 0=drive high and low, 1=drive only low
 */
#define ARWEN_02_ODER		0x1c

/** 16Z127-01: debounce compare regs
 * 16-bit, 50us unit: 0..3276750us 
 */
#define ARWEN_Z127V01_DBCR(port)		(0x80 + (0x04 * (port)))

#ifdef __cplusplus
	}
#endif

#endif	/* _ARWEN_H */

 
 
 
 
 
