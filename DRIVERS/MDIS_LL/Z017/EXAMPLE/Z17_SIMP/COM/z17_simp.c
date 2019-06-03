/****************************************************************************
 ************                                                    ************
 ************                   Z17_SIMP                         ************
 ************                                                    ************
 ****************************************************************************/
/*!
 *        \file  z17_simp.c
 *      \author  ulrich.bogensperger@men.de
 *
 *      \brief   Simple example program for the Z17 driver
 *
 *               Reads and writes some values from/to GPIO ports,
 *               generating interrupts.
 *               Interrupts will be generated only on inputs. Thatsway
 *               normaly an external loopback from the outputs gpio[0]..[4]
 *               to gpio[5]..[7] is required.
 *
 *     Required: libraries: mdis_api, usr_oss
 *    \switches  (none)
 */
 /*
 *---------------------------------------------------------------------------
 * Copyright (c) 2003-2019, MEN Mikro Elektronik GmbH
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


/*--------------------------------------+
|  INCLUDES                             |
+--------------------------------------*/
#include <stdio.h>
#include <string.h>
#include <MEN/men_typs.h>
#include <MEN/mdis_api.h>
#include <MEN/usr_oss.h>
#include <MEN/z17_drv.h>

static const char IdentString[]=MENT_XSTR(MAK_REVISION);

/*--------------------------------------+
|  GLOBALS                              |
+--------------------------------------*/
static int G_sigCount = 0;

/*--------------------------------------+
|  PROTOTYPES                           |
+--------------------------------------*/
static void usage(void);
static void PrintError(char *info);
static void __MAPILIB SignalHandler(u_int32 sig);
static char* bitString(char *s, u_int32 val, int nrBits);

/********************************* usage ***********************************/
/**  Print program usage
 */
static void usage(void)
{
	printf("Usage:    z17_simp <device>\n");
	printf("Function: Example program for the Z17 GPIO driver\n");
	printf("    Note:    Interrupts will be generated only on inputs.\n");
	printf("             That's why usually at least one external loopback\n");
	printf("             from the outputs gpio[0]..[4] to gpio[5]..[7] is required.\n");
	printf("             A direction switching and drift to high, low and firing interrupt\n");
	printf("             will be tried.\n");
	printf("Options:\n");
	printf("    device       device name (e.g. z17_1)\n");
	printf("\n");
	printf("Copyright (c) 2003-2019, MEN Mikro Elektronik GmbH\n%s\n", IdentString);
}

/***************************************************************************/
/** Program main function
 *
 *  \param argc       \IN  argument counter
 *  \param argv       \IN  argument vector
 *
 *  \return           success (0) or error (1)
 */
int main(int argc, char *argv[])
{
	char      *device;
	int32     inp, inp1, inp2;
	int32     lastRequest;
	int32     irqCount;
	unsigned int   out;
	MDIS_PATH path;

	if (argc < 2 || strcmp(argv[1], "-?") == 0) {
		usage();
		return (1);
	}

	device = argv[1];

	/*--------------------+
	|  open path          |
	+--------------------*/
	if ((path = M_open(device)) < 0) {
		PrintError("open");
		return (1);
	}

	/*--------------------+
	|  config             |
	+--------------------*/
	/* install signal which will be received at change of input ports */
	UOS_SigInit(SignalHandler);
	UOS_SigInstall(UOS_SIG_USR1);
	M_setstat(path, Z17_SET_SIGNAL, UOS_SIG_USR1);

	/* program lower five ports as outputs, others as inputs */
	M_write(path, 0x00);
	M_setstat(path, Z17_DIRECTION, 0x1f);

	/* generate interrupts on rising edge of inputs */
	M_setstat(path, Z17_IRQ_SENSE, 0x5400);

	/* enable global device interrupt */
	M_setstat(path, M_MK_IRQ_ENABLE, TRUE);

	/*--------------------+
	|  output values      |
	+--------------------*/
	printf("\n loop | GPIO:  4 3 2 1 0    GPIO: 7 6 5 4 3 2 1 0\n");
	printf(" ---- |        ---------          ---------------\n");
	for (out = 0; out < 32; ++out) {
		char buf1[40], buf2[40];

		M_write(path, out);
		UOS_Delay(10);
		M_read(path, (int32*)&inp);

		printf("%2d    |   Out: %s     In: %s (0x%x) IRQs: %d\n",
					out, bitString(buf1, out, 5), bitString(buf2, inp, 8),
					(unsigned int)inp, (int)G_sigCount);

		UOS_Delay(100);
	}

	/* clear ports 0 and 1 */
	M_setstat(path, Z17_CLR_PORTS, 0x03);
	UOS_Delay(10);
	M_read(path, &inp);

	/* set port 0 */
	M_setstat(path, Z17_SET_PORTS, 0x01);
	UOS_Delay(10);
	M_read(path, &inp);

	/*------------------------------------------------+
	|  try to generate interrupts by                  |
	|   1. setting low and switch to input and wait   |
	|   2. setting high and switch to input and wait  |
	+------------------------------------------------*/
	printf("\n\ntry to generate interrupts without external loopback\n");
	M_setstat(path, Z17_IRQ_SENSE, 0xFFFF);	/* generate interrupts for all channels on rising and falling edge */

	/* 2. */
	printf("\n set to high, switch to input and try to drift to low\n");
	G_sigCount = 0;
	M_setstat(path, Z17_DIRECTION, 0xff);
	M_setstat(path, Z17_SET_PORTS, 0xff);
	UOS_Delay(10);
	M_read(path, &inp1);
	printf("    Out: %02x\n", (unsigned int)inp1);
	M_setstat(path, Z17_DIRECTION, 0x00);
	UOS_Delay(1000);	/* drift to low with pull downs */
	M_read(path, &inp2);
	printf("     In: %02x\n", (unsigned int)inp2);
	M_getstat(path, M_LL_IRQ_COUNT, &irqCount);
	printf("   IRQs: %d  total %d  %s\n", (int)G_sigCount, (int)irqCount, G_sigCount ? "--> fired" : "");
	M_getstat(path, Z17_IRQ_LAST_REQUEST, &lastRequest); /* get lastRequest */
	printf("   IREQ: %02x\n", (unsigned int)lastRequest);

	/* 1. */
	printf("\n set to low, switch to input and try to drift to high\n");
	G_sigCount = 0;
	M_setstat(path, Z17_DIRECTION, 0xff);
	M_setstat(path, Z17_CLR_PORTS, 0xAA);
	UOS_Delay(10);
	M_read(path, &inp1);
	printf("    Out: %02x\n", (unsigned int)inp1);
	M_setstat(path, Z17_DIRECTION, 0x00);
	UOS_Delay(1000);	/* drift to low with pull downs */
	M_read(path, &inp2);
	printf("     In: %02x\n", (unsigned int)inp2);
	M_getstat(path, M_LL_IRQ_COUNT, &irqCount);
	printf("   IRQs: %d  total %d  %s\n", (int)G_sigCount, (int)irqCount, G_sigCount ? "--> fired" : "");
	M_getstat(path, Z17_IRQ_LAST_REQUEST, &lastRequest);	/* get lastRequest */
	printf("   IREQ: %02x\n", (unsigned int)lastRequest);

	/* 2. */
	printf("\n set to high, switch to input and try to drift to low\n");
	G_sigCount = 0;
	M_setstat(path, Z17_DIRECTION, 0xff);
	M_setstat(path, Z17_SET_PORTS, 0xff);
	UOS_Delay(10);
	M_read(path, &inp1);
	printf("    Out: %02x\n", (unsigned int)inp1);
	M_setstat(path, Z17_DIRECTION, 0x00);
	UOS_Delay(1000);	/* drift to low with pull downs */
	M_read(path, &inp2);
	printf("     In: %02x\n", (unsigned int)inp2);
	M_getstat(path, M_LL_IRQ_COUNT, &irqCount);
	printf("   IRQs: %d  total %d  %s\n", (int)G_sigCount, (int)irqCount, G_sigCount ? "--> fired" : "");
	M_getstat(path, Z17_IRQ_LAST_REQUEST, &lastRequest);	/* get lastRequest */
	printf("   IREQ: %02x\n", (unsigned int)lastRequest);

	/* 1. */
	printf("\n set to low, switch to input and try to drift to high\n");
	G_sigCount = 0;
	M_setstat(path, Z17_DIRECTION, 0xff);
	M_setstat(path, Z17_CLR_PORTS, 0x55);
	UOS_Delay(10);
	M_read(path, &inp1);
	printf("    Out: %02x\n", (unsigned int)inp1);
	M_setstat(path, Z17_DIRECTION, 0x00);
	UOS_Delay(1000);	/* drift to low with pull downs */
	M_read(path, &inp2);
	printf("     In: %02x\n", (unsigned int)inp2);
	M_getstat(path, M_LL_IRQ_COUNT, &irqCount);
	printf("   IRQs: %d  total %d  %s\n", (int)G_sigCount, (int)irqCount, G_sigCount ? "--> fired" : "");
	M_getstat(path, Z17_IRQ_LAST_REQUEST, &lastRequest);	/* get lastRequest */
	printf("   IREQ: %02x\n", (unsigned int)lastRequest);

	/*--------------------+
	|  cleanup            |
	+--------------------*/
	M_setstat(path, Z17_CLR_SIGNAL, 0);
	UOS_SigRemove(UOS_SIG_USR1);
	UOS_SigExit();

	if (M_close(path) < 0)
		PrintError("close");

	return (0);
}

/********************************* PrintError ******************************/
/** Print MDIS error message
 *
 *  \param info       \IN  info string
*/
static void PrintError(char *info)
{
	printf("*** can't %s: %s\n", info, M_errstring(UOS_ErrnoGet()));
}

/**********************************************************************/
/** Signal handler
 *
 *  \param  sig    \IN   received signal
 */
static void __MAPILIB SignalHandler(u_int32 sig)
{
	if (sig == UOS_SIG_USR1)
		++G_sigCount;
}

/**********************************************************************/
/** Convert an integer into its binary string representation
 *
 *  e.g. 0x4f is converted to "1 0 0 1 1 1 1 "
 *
 *  \param  s      \IN   pointer to where result is written
 *  \param  val    \IN   value to convert
 *  \param  nrBits \IN   number of bits to convert (max 32)
 *
 *  \return pointer to resulting string
 */
static char* bitString(char *s, u_int32 val, int nrBits)
{
	u_int32 bit;
	int i;

	*s = '\0';

	bit = 1 << (nrBits-1);

	for (i = 0; i < nrBits; ++i, bit >>= 1)
		strcat(s, val & bit ? "1 " : "0 ");

	return (s);
}
 
 
 
 
 
