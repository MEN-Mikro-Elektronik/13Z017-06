/****************************************************************************
 ************                                                    ************
 ************                    Z127_OUT                         ************
 ************                                                    ************
 ****************************************************************************/
/*!
 *        \file  z127_out.c
 *      \author  dieter.pfeuffer@men.de
 *
 *       \brief  Tool to control the 16Z127 (32-bit) outputs
 *
 *     Required: libraries: mdis_api, usr_oss, usr_utl
 *    \switches  (none)
 */
 /*
 *---------------------------------------------------------------------------
 * Copyright (c) 2016-2019, MEN Mikro Elektronik GmbH
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
#include <stdlib.h>
#include <MEN/men_typs.h>
#include <MEN/mdis_api.h>
#include <MEN/usr_oss.h>
#include <MEN/usr_utl.h>
#include <MEN/z17_drv.h>

static const char IdentString[]=MENT_XSTR(MAK_REVISION);

/*--------------------------------------+
|   DEFINES                             |
+--------------------------------------*/
#define ERR_OK		0
#define ERR_PARAM	1
#define ERR_FUNC	2

/*--------------------------------------+
|   GLOBALS                             |
+--------------------------------------*/
static MDIS_PATH G_path;
static u_int32	G_portm;
static u_int32	G_verbose;

/*--------------------------------------+
|  PROTOTYPES                           |
+--------------------------------------*/
static void usage(void);
static int PrintError(char *info);
static int WriteOutputs(int outMask);
static int ReadOutputs(void);

/********************************* usage ***********************************/
/**  Print program usage
 */
static void usage(void)
{
	printf("Usage:    z127_out <device> <opts> [<opts>]                            \n");
	printf("Function: Control the 16Z127 outputs (32-bit)                          \n");
	printf("Options:                                                   [default]   \n");
	printf("    device      device name (e.g. gpio_1)                              \n");
	printf("    -p=<mask>   32-bit output port mask....................[0xffffffff]\n");
	printf("    -g          get only output relevant settings and exit             \n");
	printf("    -o          open drain mode                                        \n");
	printf("    -r          read outputs back                                      \n");
	printf("    -s=0/1      set outputs 0=low 1=high                               \n");
	printf("    -t=<ms>     toggle outputs in turn, <ms> each                      \n");
	printf("    -T [<opt>]  toggle outputs together                                \n");
	printf("       -H=<ms>    toggle high time [ms]....................[1000]      \n");
	printf("       -L=<ms>    toggle low time [ms].....................[1000]      \n");
	printf("    -l=<n>      loop toggling n times (0=endless)..........[0]         \n");
	printf("                  can be aborted with keypress                         \n");
	printf("    -v          verbose output                                         \n");
	printf("\n");
	printf("Copyright (c) 2003-2019, MEN Mikro Elektronik GmbH\n%s\n", IdentString);
}

/***************************************************************************/
/** Program main function
 *
 *  \param argc       \IN  argument counter
 *  \param argv       \IN  argument vector
 *
 *  \return           success (0) or error code
 */
int main(int argc, char *argv[])
{
	char	*device, *str, *errstr, buf[40];
	u_int32	p, mask;
	int32	openDrain, get, readBack, set, togTurn, togAll, togAllH, togAllL, loop;
	int32	dir, loopcnt, outMask, run, old;
	int		ret, n;

	/*----------------------+
	|  check arguments      |
	+----------------------*/
	if ((errstr = UTL_ILLIOPT("p=ogrs=t=TH=L=l=v?", buf))) {
		printf("*** %s\n", errstr);
		return ERR_PARAM;
	}
	if (UTL_TSTOPT("?")) {
		usage();
		return ERR_PARAM;
	}
	if (argc < 3) {
		usage();
		return ERR_PARAM;
	}

	/*----------------------+
	|  get arguments        |
	+----------------------*/
	for (device = NULL, n=1; n<argc; n++) {
		if (*argv[n] != '-') {
			device = argv[n];
			break;
		}
	}
	if (!device) {
		usage();
		return ERR_PARAM;
	}

	G_portm	  = ((str = UTL_TSTOPT("p=")) ? strtoul(str, NULL, 16) : 0xffffffff);
	openDrain = (UTL_TSTOPT("o") ? 1 : 0);
	get       = (UTL_TSTOPT("g") ? 1 : 0);
	readBack  = (UTL_TSTOPT("r") ? 1 : 0);
	set       = ((str = UTL_TSTOPT("s=")) ? atoi(str) : -1);
	togTurn   = ((str = UTL_TSTOPT("t=")) ? atoi(str) : -1);
	togAll    = (UTL_TSTOPT("T") ? 1 : 0);
	togAllH	  = ((str = UTL_TSTOPT("H=")) ? atoi(str) : 1000);
	togAllL   = ((str = UTL_TSTOPT("L=")) ? atoi(str) : 1000);
	loop      = ((str = UTL_TSTOPT("l=")) ? atoi(str) : 0);
	G_verbose = (UTL_TSTOPT("v") ? 1 : 0);

	/* further parameter checking */
	if ((togTurn != -1) && togAll) {
		printf("*** error: -t= together with -T not possible\n");
		return ERR_PARAM;
	}

	/*----------------------+
	|  open path            |
	+----------------------*/
	if ((G_path = M_open(device)) < 0) {
		return PrintError("open");
	}

	/*----------------------+
	|  get settings         |
	+----------------------*/
	if (get) {
		printf("_____Port Settings_____\n");

		/* DIRECTION */
		if ((M_getstat(G_path, Z17_DIRECTION, &dir)) < 0) {
			ret = PrintError("getstat Z17_DIRECTION");
			goto ABORT;
		}

		/* OPEN DRAIN ON/OFF */
		if ((M_getstat(G_path, Z17_OPEN_DRAIN, &old)) < 0) {
			ret = PrintError("getstat Z17_DEBOUNCE");
			goto ABORT;
		}

		mask = 0;
		for (p = 0; p<32; p++) {
			if (G_portm & (1 << p)) {

				printf("Port %02d: dir=%s, open-drain=%s\n",
					p,
					(dir & (1 << p)) ? "out" : "in",
					(old & (1 << p)) ? "on" : "off");
			}
		}
		goto CLEANUP;
	}

	/*----------------------+
	|  set port direction   |
	+----------------------*/
	if ((M_getstat(G_path, Z17_DIRECTION, &dir)) < 0) {
		ret = PrintError("getstat Z17_DIRECTION");
		goto ABORT;
	}

	dir |= G_portm;

	if ((M_setstat(G_path, Z17_DIRECTION, dir)) < 0) {
		ret = PrintError("setstat Z17_DIRECTION");
		goto ABORT;
	}

	/*----------------------+
	|  configure open drain |
	+----------------------*/
	if ((M_getstat(G_path, Z17_OPEN_DRAIN, &old)) < 0) {
		ret = PrintError("getstat Z17_DEBOUNCE");
		goto ABORT;
	}

	mask = openDrain ? G_portm : 0;
	mask |= (~G_portm) & old;

	if ((M_setstat(G_path, Z17_OPEN_DRAIN, mask)) < 0) {
		ret = PrintError("setstat Z17_DEBOUNCE");
		goto ABORT;
	}
	
	/*----------------------+
	|  set output           |
	+----------------------*/
	if (set != -1) {
		printf("set ports 0x%08x to %d\n", G_portm, set);
		if ((ret = WriteOutputs(set ? G_portm : 0)))
			goto ABORT;
	}

	/*----------------------+
	| read back             |
	+----------------------*/
	if (readBack) {
		if ((ret = ReadOutputs()))
			goto ABORT;
	}

	/*----------------------+
	|  loop                 |
	+----------------------*/
	loopcnt = 0;
	run = 1;
	do {
		/* toggle in turn */
		if (togTurn != -1) {
			if(loopcnt == 0)
				printf("toggle ports 0x%08x in turn (each for %dms)\n",
					G_portm, togTurn);
	
			if(!G_verbose){
				printf(".");
				fflush(stdout);
			}

			for (p = 0; p<32; p++) {

				/* skip unconfigured ports */
				if(!(G_portm & (1 << p)))
					continue;

				outMask = 1 << p;

				if ((ret = WriteOutputs(outMask)))
					goto ABORT;

				/* repeat until keypress */
				if (UOS_KeyPressed() != -1)
					goto CLEANUP;

				if (p < 31)
					UOS_Delay(togTurn);
			}
		}
		/* toggle together */
		else if (togAll) {
			if (loopcnt == 0)
				printf("toggle ports 0x%08x together 1/0 (high=%dms, low=%dms)\n",
					G_portm, togAllH, togAllL);

			if(!G_verbose){
				printf(".");
				fflush(stdout);
			}

			if ((ret = WriteOutputs(G_portm)))
				goto ABORT;

			UOS_Delay(togAllH);

			if ((ret = WriteOutputs(0)))
				goto ABORT;

			/* repeat until keypress */
			if (UOS_KeyPressed() != -1)
				goto CLEANUP;

			UOS_Delay(togAllL);
		}
		else
			run = 0;

		/* abort after n turns */
		loopcnt++;
		if (loop && (loopcnt == loop))
			run = 0;
					
	} while (run);

	/*----------------------+
	|  cleanup              |
	+----------------------*/
CLEANUP:
	printf("\n");
	ret=ERR_OK;
	
ABORT:
	if (M_close(G_path) < 0)
		ret = PrintError("close");

	return ret;
}

/***************************************************************************/
/** Print MDIS error message
 *
 *  \param info       \IN  info string
 *
 *  \return           ERR_FUNC
 */
static int PrintError(char *info)
{
	printf("*** can't %s: %s\n", info, M_errstring (UOS_ErrnoGet()));
	return ERR_FUNC;
}

/***************************************************************************/
/** Write output values
 *
 *  \param outMask    \IN  output mask
 *
 *  \return           success (0) or error code
 */
static int WriteOutputs( int outMask )
{
	int32	old, p, mask=0;

	if ((M_read(G_path, &old)) < 0) {
		return PrintError("read");
	}

	mask = outMask;
	mask |= (~G_portm) & old;

	if ((M_write(G_path, mask)) < 0) {
		return PrintError("write");
	}

	if(G_verbose){
		printf("Port 15...0: 15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00\n");
		printf("State      : ");
		for (p = 15; p >= 0; p--)
			printf("%c%d ",
			(G_portm & (1 << p) ? ' ' : '*'),
				(mask >> p) & 1);
		printf("\n");

		printf("       === SET PORT === (*=un-configured port)\n");

		printf("Port 31..16: 31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16\n");
		printf("State      : ");
		for (p = 31; p >= 16; p--)
			printf("%c%d ",
			(G_portm & (1 << p) ? ' ' : '*'),
				(mask >> p) & 1);
		printf("\n\n");
	}
	
	return ERR_OK;
}

/***************************************************************************/
/** Read output values
*
*  \return           success (0) or error code
*/
static int ReadOutputs(void)
{
	int32	val, p;

	if ((M_read(G_path, &val)) < 0) {
		return PrintError("read");
	}

	printf("Port 15...0: 15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00\n");
	printf("State      : ");
	for (p = 15; p >= 0; p--)
		printf("%c%d ",
		(G_portm & (1 << p) ? ' ' : '*'),
			(val >> p) & 1);
	printf("\n");

	printf("       === READ PORT BACK === (*=un-configured port)\n");

	printf("Port 31..16: 31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16\n");
	printf("State      : ");
	for (p = 31; p >= 16; p--)
		printf("%c%d ",
		(G_portm & (1 << p) ? ' ' : '*'),
			(val >> p) & 1);
	printf("\n\n");

	return ERR_OK;
}

