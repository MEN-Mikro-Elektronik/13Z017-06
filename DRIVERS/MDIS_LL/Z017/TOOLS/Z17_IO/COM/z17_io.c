/****************************************************************************
 ************                                                    ************
 ************                     Z17_IO                         ************
 ************                                                    ************
 ****************************************************************************/
/*!
 *        \file  z17_io.c
 *      \author  dieter.pfeuffer@men.de
 *        $Date: 2014/11/28 15:20:05 $
 *    $Revision: 1.9 $
 *
 *       \brief  Tool to access the 16Z034/16Z037 (8-bit) I/Os
 *
 *     Required: libraries: mdis_api, usr_oss, usr_utl
 *    \switches  (none)
 */
 /*-------------------------------[ History ]--------------------------------
 *
 * $Log: z17_io.c,v $
 * Revision 1.9  2014/11/28 15:20:05  MRoth
 * R: modifications of revision 1.7 were lost during merge
 * M: re-added modifications of revision 1.7
 *
 * Revision 1.8  2014/11/28 14:17:47  MRoth
 * R: 1.) driver supports toggle function
 *    2.) cosmetics
 * M: 1.) changed toggle mode from loop to setstats
 *    2.) revised code completely
 *
 * Revision 1.7  2011/11/30 16:34:33  dpfeuffer
 * R: last modification was wrong
 * M: last modification fixed
 *
 * Revision 1.6  2011/11/28 12:02:02  dpfeuffer
 * R: too much CPU load with option -h
 * M: delay added
 *
 * Revision 1.5  2011/03/28 11:08:55  dpfeuffer
 * R: error in usage information
 * M: description fixed
 *
 * Revision 1.4  2009/07/10 13:40:54  CRuff
 * R: Porting to MDIS5
 * M: changed type of M_open Path variable to MDIS_PATH
 *
 * Revision 1.3  2008/05/07 11:17:49  ts
 * included <stdlib.h> to remove warning about implicitely defined func. atoi()
 *
 * Revision 1.2  2006/12/20 11:30:59  ufranke
 * removed
 *  - unused variable warning
 *
 * Revision 1.1  2006/08/02 08:31:52  DPfeuffer
 * Initial Revision
 *
 *---------------------------------------------------------------------------
 * (c) Copyright 2006 by MEN mikro elektronik GmbH, Nuernberg, Germany
 ****************************************************************************/
static const char RCSid[]="$Id: z17_io.c,v 1.9 2014/11/28 15:20:05 MRoth Exp $";

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
static void usage (void)
{
	printf("Usage:    z17_io <device> <opts> [<opts>]                    \n");
	printf("Function: Access the 16Z034 GPIOs (8-bit)                    \n");
	printf("Options:                                                     \n");
	printf("    device        device name (e.g. z17_1)                   \n");
	printf("    -p=<port>     i/o port(s) 0..7.....................[all] \n");
	printf("    -g            get state of input port(s)                 \n");
	printf("    -s=0/1        set output port(s) low/high                \n");
	printf("    -t            toggle all output ports in turn            \n");
	printf("    -G            get state of input port(s) in a loop       \n");
	printf("    -T [<opts>]   toggle output port(s) endlessly            \n");
	printf("        -o=<time>     toggle high (in ms)..............[1000]\n");
	printf("        -l=<time>     toggle low (in ms)...............[1000]\n");
	printf("    -h            hold path open until keypress              \n");
	printf("\n");
	printf("(c)Copyright 2006 by MEN Mikro Elektronik GmbH\n%s\n", RCSid);
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
	for (device=NULL, n=1; n<argc; n++) {
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
		portStr = "0..7";
		portBit = 0xff;
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
	if ((M_getstat(path, Z17_DIRECTION, &dir)) < 0) {
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
				printf("port : 7  6  5  4  3  2  1  0\n");
				printf("state: ");
				for (n=7; n>=0; n--)
					printf("%d  ", (val >> n) & 1);
				printf("\n\n");
			} else {
				printf("input state port #%d: %d (read-value=0x%x)\n",
							port, (val & portBit) ? 1 : 0, val);
			}
			if (getLoop)
				UOS_Delay(2000);
		} while (getLoop && (UOS_KeyPressed() == (-1)));
	}

	/*----------------------+
	|  set output           |
	+----------------------*/
	if (set != (-1)) {
		printf("set output state port #%s: %d\n", portStr, set);
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
		printf("toggle output state port 0..7 in turn\n");
		for (n=0; n<9; n++) {
			if ((M_write(path, 1<<n)) < 0) {
				PrintError("write");
				goto abort;
			}
			if (n < 8)
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
			printf("toggle output state port #%d\n", portBit);

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
	printf("*** can't %s: %s\n", info, M_errstring(UOS_ErrnoGet()));
} 
 
 
 
