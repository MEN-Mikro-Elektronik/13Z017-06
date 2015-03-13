/****************************************************************************
 ************                                                    ************
 ************                   Z17_SIMP                         ************
 ************                                                    ************
 ****************************************************************************/
/*!
 *        \file  z17_simp.c
 *      \author  ulrich.bogensperger@men.de
 *        $Date: 2014/11/28 14:17:45 $
 *    $Revision: 1.6 $
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
 /*-------------------------------[ History ]--------------------------------
 *
 * $Log: z17_simp.c,v $
 * Revision 1.6  2014/11/28 14:17:45  MRoth
 * R: cosmetics
 * M: revised code completely
 *
 * Revision 1.5  2009/07/10 13:40:12  CRuff
 * R: VxWorks compiler warnings
 * M: changed type of local variables to avoid implicit type conversions
 *
 * Revision 1.4  2009/04/17 10:23:40  MRoth
 * R: stack overflow at signalHandler call for Windows
 * M: Fixed calling convention for UOS_SigInit
 *
 * Revision 1.3  2006/12/20 11:30:39  ufranke
 * added
 *  + getstat code Z17_IRQ_LAST_REQUEST
 *  + program try to fire interrupts without external loopback
 *
 * Revision 1.2  2005/04/27 14:10:12  kp
 * bug fix: enable global device interrupt
 *
 * Revision 1.1  2004/06/18 14:30:01  ub
 * Initial Revision
 *
 *---------------------------------------------------------------------------
 * (c) Copyright 2003 by MEN mikro elektronik GmbH, Nuernberg, Germany
 ****************************************************************************/
static const char RCSid[]="$Id: z17_simp.c,v 1.6 2014/11/28 14:17:45 MRoth Exp $";

/*--------------------------------------+
|  INCLUDES                             |
+--------------------------------------*/
#include <stdio.h>
#include <string.h>
#include <MEN/men_typs.h>
#include <MEN/mdis_api.h>
#include <MEN/usr_oss.h>
#include <MEN/z17_drv.h>

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
	printf("(c)Copyright 2004 by MEN Mikro Elektronik GmbH\n%s\n", RCSid);
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
	u_int32   out;
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
		M_read(path, &inp);

		printf("%2d    |   Out: %s     In: %s (0x%x) IRQs: %d\n",
					out, bitString(buf1, out, 5), bitString(buf2, inp, 8),
					inp, G_sigCount);

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
	printf("    Out: %02x\n", inp1);
	M_setstat(path, Z17_DIRECTION, 0x00);
	UOS_Delay(1000);	/* drift to low with pull downs */
	M_read(path, &inp2);
	printf("     In: %02x\n", inp2);
	M_getstat(path, M_LL_IRQ_COUNT, &irqCount);
	printf("   IRQs: %d  total %d  %s\n", G_sigCount, irqCount, G_sigCount ? "--> fired" : "");
	M_getstat(path, Z17_IRQ_LAST_REQUEST, &lastRequest); /* get lastRequest */
	printf("   IREQ: %02x\n", lastRequest);

	/* 1. */
	printf("\n set to low, switch to input and try to drift to high\n");
	G_sigCount = 0;
	M_setstat(path, Z17_DIRECTION, 0xff);
	M_setstat(path, Z17_CLR_PORTS, 0xAA);
	UOS_Delay(10);
	M_read(path, &inp1);
	printf("    Out: %02x\n", inp1);
	M_setstat(path, Z17_DIRECTION, 0x00);
	UOS_Delay(1000);	/* drift to low with pull downs */
	M_read(path, &inp2);
	printf("     In: %02x\n", inp2);
	M_getstat(path, M_LL_IRQ_COUNT, &irqCount);
	printf("   IRQs: %d  total %d  %s\n", G_sigCount, irqCount, G_sigCount ? "--> fired" : "");
	M_getstat(path, Z17_IRQ_LAST_REQUEST, &lastRequest);	/* get lastRequest */
	printf("   IREQ: %02x\n", lastRequest);

	/* 2. */
	printf("\n set to high, switch to input and try to drift to low\n");
	G_sigCount = 0;
	M_setstat(path, Z17_DIRECTION, 0xff);
	M_setstat(path, Z17_SET_PORTS, 0xff);
	UOS_Delay(10);
	M_read(path, &inp1);
	printf("    Out: %02x\n", inp1);
	M_setstat(path, Z17_DIRECTION, 0x00);
	UOS_Delay(1000);	/* drift to low with pull downs */
	M_read(path, &inp2);
	printf("     In: %02x\n", inp2);
	M_getstat(path, M_LL_IRQ_COUNT, &irqCount);
	printf("   IRQs: %d  total %d  %s\n", G_sigCount, irqCount, G_sigCount ? "--> fired" : "");
	M_getstat(path, Z17_IRQ_LAST_REQUEST, &lastRequest);	/* get lastRequest */
	printf("   IREQ: %02x\n", lastRequest);

	/* 1. */
	printf("\n set to low, switch to input and try to drift to high\n");
	G_sigCount = 0;
	M_setstat(path, Z17_DIRECTION, 0xff);
	M_setstat(path, Z17_CLR_PORTS, 0x55);
	UOS_Delay(10);
	M_read(path, &inp1);
	printf("    Out: %02x\n", inp1);
	M_setstat(path, Z17_DIRECTION, 0x00);
	UOS_Delay(1000);	/* drift to low with pull downs */
	M_read(path, &inp2);
	printf("     In: %02x\n", inp2);
	M_getstat(path, M_LL_IRQ_COUNT, &irqCount);
	printf("   IRQs: %d  total %d  %s\n", G_sigCount, irqCount, G_sigCount ? "--> fired" : "");
	M_getstat(path, Z17_IRQ_LAST_REQUEST, &lastRequest);	/* get lastRequest */
	printf("   IREQ: %02x\n", lastRequest);

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
