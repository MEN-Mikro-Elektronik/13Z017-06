/****************************************************************************
 ************                                                    ************
 ************                    Z127_IO                         ************
 ************                                                    ************
 ****************************************************************************/
/*!
 *        \file  z127_io.c
 *      \author  michael.roth@men.de
 *        $Date: 2017/05/03 16:53:47 $
 *    $Revision: 1.9 $
 *
 *       \brief  Tool to access the 16Z127 (32-bit) I/Os
 *
 *     Required: libraries: mdis_api, usr_oss, usr_utl
 *    \switches  (none)
 */
 /*
 *---------------------------------------------------------------------------
 * Copyright (c) 2010-2019, MEN Mikro Elektronik GmbH
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

static const char RCSid[]="$Id: z127_io.c,v 1.9 2017/05/03 16:53:47 DPfeuffer Exp $";

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

/*--------------------------------------+
|  PROTOTYPES                           |
+--------------------------------------*/
static void usage(void);
static void PrintError(char *info);

/********************************* usage ***********************************/
/**  Print program usage
 */
static void usage(void)
{
	printf("Usage:    z127_io <device> <opts> [<opts>]                   \n");
	printf("Function: Access the 16Z127 GPIOs (32-bit)                   \n");
	printf("Options:                                                     \n");
	printf("    device        device name (e.g. z17_z127_1)              \n");
	printf("    -p=<port>     i/o port(s) 0..31....................[all] \n");
	printf("    -g            get state of input port(s)                 \n");
	printf("    -s=0/1        set output port(s) low/high                \n");
	printf("    -t            toggle all output ports in turn            \n");
	printf("    -G            get state of input port(s) in a loop       \n");
	printf("    -T [<opts>]   toggle output port(s) endlessly            \n");
	printf("        -o=<time>     toggle high (in ms)..............[1000]\n");
	printf("        -l=<time>     toggle low (in ms)...............[1000]\n");
	printf("    -h            hold path open until keypress              \n");
	printf("\n");
	printf("(c)Copyright 2010 by MEN Mikro Elektronik GmbH\n%s\n", RCSid);
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
	char      *device, *str, *portStr, *errstr, buf[40];
	int32     port, get, set, toggleRot, getLoop, toggleMode, hold;
	int32     timeHigh, timeLow;
	int32     portBit, dir, val, n;
	MDIS_PATH path;

	/*----------------------+
	|  check arguments      |
	+----------------------*/
	if ((errstr = UTL_ILLIOPT("p=gs=tGTo=l=h?", buf))) {
		printf("*** %s\n", errstr);
		return (1);
	}
	if (UTL_TSTOPT("?")) {
		usage();
		return (1);
	}
	if (argc < 3) {
		usage();
		return (1);
	}

	/*----------------------+
	|  get arguments        |
	+----------------------*/
	for (device = NULL, n = 1; n < argc; n++) {
		if (*argv[n] != '-') {
			device = argv[n];
			break;
		}
	}
	if (!device) {
		usage();
		return (1);
	}

	port = ((portStr = UTL_TSTOPT("p=")) ? atoi(portStr) : -1);
	if (port != (-1)) {
		portBit = 1 << port;
	} else {
		portStr = "0..31";
		portBit = 0xffffffff;
	}

	get        = (UTL_TSTOPT("g") ? 1 : 0);
	set        = ((str = UTL_TSTOPT("s=")) ? atoi(str) : -1);
	toggleRot  = (UTL_TSTOPT("t") ? 1 : 0);
	getLoop    = (UTL_TSTOPT("G") ? 1 : 0);
	toggleMode = (UTL_TSTOPT("T") ? 1 : 0);
	timeHigh   = ((str = UTL_TSTOPT("o=")) ? atoi(str) : 0);
	timeLow    = ((str = UTL_TSTOPT("l=")) ? atoi(str) : 0);
	hold       = (UTL_TSTOPT("h") ? 1 : 0);

	/*----------------------+
	|  open path            |
	+----------------------*/
	if ((path = M_open(device)) < 0) {
		PrintError("open");
		return (1);
	}

	/*----------------------+
	|  set port direction   |
	+----------------------*/
	if ((M_getstat (path, Z17_DIRECTION, &dir)) < 0) {
		PrintError("getstat Z17_DIRECTION");
		goto abort;
	}

	/* input? */
	if (get || getLoop)
		dir &= ~portBit;
	/* output */
	else
		dir |= portBit;

	if ((M_setstat(path, Z17_DIRECTION, dir)) < 0) {
		PrintError("setstat Z17_DIRECTION");
		goto abort;
	}

	/*----------------------+
	|  get input            |
	+----------------------*/
	if (get || getLoop) {

		/* repeat until keypress */
		do {
			if ((M_read(path, &val)) < 0) {
				PrintError("read");
				goto abort;
			}
			if (port == (-1)) {
				printf("port : 31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16\n");
				printf("state: ");
				for (n=31; n>=16; n--)
					printf("%ld  ", (val >> n) & 1);
				printf("\n\n");
				printf("port : 15 14 13 12 11 10 9  8  7  6  5  4  3  2  1  0\n");
				printf("state: ");
				for (n=15; n>=0; n--)
					printf("%ld  ", (val >> n) & 1);
				printf("\n\n");
			} else {
				printf("input state port #%d: %d (read-value=0x%x)\n",
							(int)port, (val & portBit) ? 1 : 0, (int)val);
			}
			if (getLoop)
				UOS_Delay(2000);
		} while (getLoop && (UOS_KeyPressed() == (-1)));
	}

	/*----------------------+
	|  set output           |
	+----------------------*/
	if (set != (-1)) {
		printf("set output state port #%s: %d\n", portStr, (int)set);
		if ((M_setstat(path, set ? Z17_SET_PORTS : Z17_CLR_PORTS, portBit)) < 0) {
			PrintError("setstat Z17_SET_PORTS/Z17_CLR_PORTS");
			goto abort;
		}
	}

	/*----------------------+
	|  toggle output        |
	+----------------------*/
	/* toggle all GPIOs once*/
	if (toggleRot) {
		printf("toggle output state port 0..31 in turn\n");
		for (n = 0; n < 32; n++) {
			if ((M_write(path, 1 << n)) < 0) {
				PrintError("write");
				goto abort;
			}
			if (n < 31)
				UOS_Delay(1000);
		}
	}

	/* toggle GPIO(s) endlessly*/
	if (toggleMode) {
		if (timeHigh) {
			if ((M_setstat(path, Z17_TOG_HIGH, timeHigh)) < 0) {
				PrintError("setstat Z17_TOG_HIGH");
				goto abort;
			}
		}
		if (timeLow) {
			if ((M_setstat(path, Z17_TOG_LOW, timeLow)) < 0) {
				PrintError("setstat Z17_TOG_LOW");
				goto abort;
			}
		}
		if (port == (-1))
			printf("toggle output state ports %s\n", portStr);
		else
			printf("toggle output state port #%d\n", (int)portBit);

		if ((M_setstat(path, Z17_TOG_PORTS, portBit)) < 0) {
			PrintError("setstat Z17_TOG_PORTS");
			goto abort;
		}
	}

	/*-----------------------+
	|  hold device path open |
	+-----------------------*/
	if (hold) {
		printf("holding path open until keypress\n");
		do {
			UOS_Delay(100);
		} while (UOS_KeyPressed() == -1);
	}

	/*----------------------+
	|  cleanup              |
	+----------------------*/
	abort:
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
	printf("*** can't %s: %s\n", info, M_errstring (UOS_ErrnoGet()));
}


 
 
 
 
