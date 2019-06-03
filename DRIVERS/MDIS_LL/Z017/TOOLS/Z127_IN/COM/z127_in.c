/****************************************************************************
 ************                                                    ************
 ************                    Z127_IN                         ************
 ************                                                    ************
 ****************************************************************************/
/*!
 *        \file  z127_in.c
 *      \author  dieter.pfeuffer@men.de
 *
 *       \brief  Tool to control the 16Z127 (32-bit) inputs
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
static u_int8 G_sigInstall = FALSE;
static u_int32 G_sigCount = 0;
static int32 G_mode;

static u_int32	G_portm;
static MDIS_PATH G_path;

/*--------------------------------------+
|  PROTOTYPES                           |
+--------------------------------------*/
static void usage(void);
static int PrintError(char *info);
static int PrintUosError(char *info);
static int ReadInputs( void );
static void __MAPILIB SignalHandler( u_int32 sig );

/********************************* usage ***********************************/
/**  Print program usage
 */
static void usage(void)
{
	printf("Usage:    z127_in <device> <opts> [<opts>]                            \n");
	printf("Function: Control the 16Z127 Inputs (32-bit)                          \n");
	printf("Options:                                                  [default]   \n");
	printf("    device     device name (e.g. gpio_1)                              \n");
	printf("    -p=<mask>  32-bit input port mask.....................[0xffffffff]\n");
	printf("    -g         get only input relevant settings and exit              \n");
	printf("    -m=0..3    input mode:................................[0]         \n");
	printf("               0: read inputs (poll mode)                             \n");
	printf("               1: compute interrupt rate                              \n");
	printf("               2: show interrupt signal                               \n");
	printf("               3: read inputs after interrupt signal                  \n");
	printf("    -s=0..3    configure interrupt sense (for -m=1..3):...[0]         \n");
	printf("               0=no, 1=rising, 2=falling, 3=both edges                \n");
	printf("    -d         enable debouncer                                       \n");
	printf("    -t=<us>    set debounce time in us (*1)                           \n");
	printf("    -l=<ms>    loop all ms until keypress                             \n");
	printf("    -a=<n>     abort loop after n reads/signals (requires -l=<ms>)    \n");
	printf("\n");
	printf("(*1) requires 16Z127-01 (or compatible) IP core\n");
	printf("\n");
	printf("Copyright (c) 2016-2019, MEN Mikro Elektronik GmbH\n%s\n", IdentString);
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
	char	*device, *str, *errstr, buf[40], *sensStr;
	u_int32	p, mask;
	int32	isens, deb, debts, get, loopt, abort, old, sens, sens1, sens2;
	int32	dir, loop, loopcnt;
	int		ret, n;
	u_int32	timeLast=0, timeCurrent, timeDiff;
	u_int32	icntLast=0, icntCurrent, icntDiff;
	M_SG_BLOCK blk;
	Z17_BLK_DEBTIME dbt;
	
	blk.size = sizeof(Z17_BLK_DEBTIME);
	blk.data = (void*)&dbt;

	/*----------------------+
	|  check arguments      |
	+----------------------*/
	if ((errstr = UTL_ILLIOPT("p=m=s=dt=gl=a=?", buf))) {
		printf("*** %s\n", errstr);
		return ERR_PARAM;
	}
	if (UTL_TSTOPT("?")) {
		usage();
		return ERR_PARAM;
	}
	if (argc < 2) {
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

	G_portm	= ((str = UTL_TSTOPT("p=")) ? strtoul(str, NULL, 16) : 0xffffffff);
	G_mode	= ((str = UTL_TSTOPT("m=")) ? atoi(str) : 0);
	isens	= ((str = UTL_TSTOPT("s=")) ? atoi(str) : 0);
	deb		= (UTL_TSTOPT("d") ? 1 : 0);
	debts	= ((str = UTL_TSTOPT("t=")) ? atoi(str) : -1);
	get	    = (UTL_TSTOPT("g") ? 1 : 0);
	loopt	= ((str = UTL_TSTOPT("l=")) ? atoi(str) : -1);
	abort	= ((str = UTL_TSTOPT("a=")) ? atoi(str) : -1);
	
	/* further parameter checking */
	if (G_mode > 3) {
		usage();
		return ERR_PARAM;
	}
	if (isens > 3) {
		printf("*** error: -s=%d is an illegal parameter\n", isens);
		return ERR_PARAM;
	}

	if ((abort != -1) && (loopt == -1)) {
		printf("*** error: -a= only possible with -l= parameter\n");
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

		/* SENSE */
		/* port 0..15 */
		if (G_portm & 0x0000ffff) {

			if ((M_getstat(G_path, Z17_IRQ_SENSE, &sens1)) < 0) {
				ret = PrintError("getstat Z17_IRQ_SENSE");
				goto ABORT;
			}
		}

		/* port 16..31 */
		if (G_portm & 0xffff0000) {

			if ((M_getstat(G_path, Z17_IRQ_SENSE_16TO31, &sens2)) < 0) {
				ret = PrintError("getstat Z17_IRQ_SENSE_16TO31");
				goto ABORT;
			}
		}

		/* DEBOUNCE ON/OFF */
		if ((M_getstat(G_path, Z17_DEBOUNCE, &old)) < 0) {
			ret = PrintError("getstat Z17_DEBOUNCE");
			goto ABORT;
		}

		mask = 0;
		for (p = 0; p<32; p++) {
			if (G_portm & (1 << p)) {

				/* SENSE */
				if (p<15)
					sens = 0x03 & (sens1 >> (2 * p));
				else
					sens = 0x03 & (sens2 >> (2 * p));

				switch (sens) {
				case 0: sensStr = "no"; break;
				case 1: sensStr = "rising"; break;
				case 2: sensStr = "falling"; break;
				case 3: sensStr = "both"; break;
				default:
					sensStr = "illegal";
				}

				/* DEBOUNCE TIME */
				dbt.portMask = 1 << p;
				dbt.timeUs = 0;

				ret = M_getstat(G_path, Z17_BLK_DEBOUNCE_TIME, (int32*)&blk);

				printf("Port %02d: dir=%s, irq-sens=%s, debounce=%s, debounce-time",
					p,
					(dir & (1 << p)) ? "out" : "in",
					sensStr,
					(old & (1 << p)) ? "on" : "off");

				if (ret < 0)
					printf(" unsupported\n");
				else
					printf("=%uus\n", dbt.timeUs);
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

	dir &= ~G_portm;

	if ((M_setstat(G_path, Z17_DIRECTION, dir)) < 0) {
		ret = PrintError("setstat Z17_DIRECTION");
		goto ABORT;
	}

	/*----------------------+
	|  configure interrupt  |
	+----------------------*/
	if (G_mode>0){
		
		switch (G_mode) {		
		/* compute interrupt rate */
		case 1:
			/* reset irq count */
			if ((M_setstat(G_path, M_LL_IRQ_COUNT, 0)) < 0) {
				ret = PrintError("setstat M_LL_IRQ_COUNT");
				goto ABORT;
			}	
		break;		
					
		/* show interrupt signal */
		case 2:
		/* read inputs after interrupt signal */
		case 3:
			/* install signal handler */
			G_sigInstall = TRUE;

			if ((UOS_SigInit(SignalHandler)) != 0) {
				ret = PrintUosError("UOS_SigInit");
				goto ABORT;
			}

			if ((UOS_SigInstall(UOS_SIG_USR1)) != 0) {
				ret = PrintUosError("UOS_SigInstall");
				goto ABORT;
			}
			
			if ((M_setstat(G_path, Z17_SET_SIGNAL, UOS_SIG_USR1)) < 0) {
				ret = PrintError("setstat Z17_SET_SIGNAL");
				goto ABORT;
			}	
		break;
		}
			
		/* port 0..15 */
		if (G_portm & 0x0000ffff){
			
			if ((M_getstat(G_path, Z17_IRQ_SENSE, &old)) < 0) {
				ret = PrintError("getstat Z17_IRQ_SENSE");
				goto ABORT;
			}		
			
			mask = 0;
			for (p=0; p<16; p++){
				if (G_portm & (1<<p))
					mask |= isens << (2*p);
				else
					mask |= old & (0x3 << (2*p));
			} 
		
			if ((M_setstat(G_path, Z17_IRQ_SENSE, mask)) < 0) {
				ret = PrintError("setstat Z17_IRQ_SENSE");
				goto ABORT;
			}
		}

		/* port 16..31 */
		if( G_portm & 0xffff0000 ){		

			if ((M_getstat(G_path, Z17_IRQ_SENSE_16TO31, &old)) < 0) {
				ret = PrintError("getstat Z17_IRQ_SENSE_16TO31");
				goto ABORT;
			}		

			mask = 0;
			for (p=0; p<16; p++){
				if ((G_portm >> 16) & (1<<p))
					mask |= isens << (2*p);
				else
					mask |= old & (0x3 << (2*p));
			} 
		
			if ((M_setstat(G_path, Z17_IRQ_SENSE_16TO31, mask)) < 0) {
				ret = PrintError("setstat Z17_IRQ_SENSE_16TO31");
				goto ABORT;
			}
		}

		/* enable interrupt */
		if ((M_setstat(G_path, M_MK_IRQ_ENABLE, TRUE)) < 0) {
			ret = PrintError("setstat M_MK_IRQ_ENABLE");
			goto ABORT;
		}
	}

	/*----------------------+
	|  configure debouncer  |
	+----------------------*/
	if ((M_getstat(G_path, Z17_DEBOUNCE, &old)) < 0) {
		ret = PrintError("getstat Z17_DEBOUNCE");
		goto ABORT;
	}		

	mask  = deb ? G_portm : 0;
	mask |= (~G_portm) & old;

	if ((M_setstat(G_path, Z17_DEBOUNCE, mask)) < 0) {
		ret = PrintError("setstat Z17_DEBOUNCE");
		goto ABORT;
	}

	/* set debounce time */
	if (debts){
		dbt.portMask = G_portm;
		dbt.timeUs = debts;
		
		if ((M_setstat(G_path, Z17_BLK_DEBOUNCE_TIME, (INT32_OR_64)&blk)) < 0) {
			ret = PrintError("setstat Z17_BLK_DEBOUNCE_TIME");
			goto ABORT;
		}		
	}

	/*----------------------+
	|  loop                 |
	+----------------------*/
	loopcnt = 0;
	do {
	
		switch (G_mode) {
		
		/* poll mode */
		case 0:
			printf("_____Poll Mode_____\n");
			ret = ReadInputs();
			if( ret )
				goto ABORT;
		break;

		/* compute interrupt rate */
		case 1:
			timeCurrent = UOS_MsecTimerGet();

			if ((M_getstat(G_path, M_LL_IRQ_COUNT, (int32*)&icntCurrent)) < 0) {
				ret = PrintError("getstat M_LL_IRQ_COUNT");
				goto ABORT;
			}		

			/* not the first run */
			if( timeLast ){

				/* consider wrap around */				
				if( timeCurrent <  timeLast ){
					timeDiff = (0xffffffff - timeLast) + timeCurrent;
					printf("timeCurrent=%u, timeLast=%u\n", timeCurrent, timeLast);
				}
				else
					timeDiff = timeCurrent - timeLast;
				
				/* consider wrap around */				
				if( icntCurrent <  icntLast )
					icntDiff = (0xffffffff - icntLast) + icntCurrent;
				else
					icntDiff = icntCurrent - icntLast;
				
				printf("Interrupt rate: %dHz (#%u interrupts)\n",
					   (icntDiff * 1000) / timeDiff, icntDiff);
			}
			else{
				if (abort)
					printf("Interrupt rate: get initial value for computation\n");
			}
			
			timeLast = timeCurrent;
			icntLast = icntCurrent;
		
		break;		
		}			
		
		/* loop? */
		loop = 0;
		if (loopt != -1){
			UOS_Delay(loopt);

			/* repeat until keypress */
			if (UOS_KeyPressed() == -1)
				loop = 1;

			/* abort after n reads/signals */
			if (abort){
				loopcnt++;
				if (loopcnt==abort)
					loop = 0;
			}
		}
					
	} while (loop);

	/*----------------------+
	|  cleanup              |
	+----------------------*/
CLEANUP:
	ret=ERR_OK;
	
ABORT:
	if (G_sigInstall) {
		if ((M_setstat(G_path, Z17_CLR_SIGNAL, UOS_SIG_USR1)) < 0)
			PrintError("setstat Z17_CLR_SIGNAL");

		if ((UOS_SigRemove(UOS_SIG_USR1)) != 0)
			PrintUosError("UOS_SigRemove");

		if ((UOS_SigExit()) != 0)
			ret = PrintUosError("UOS_SigExit");
	}

	if (M_close(G_path) < 0)
		PrintError("close");

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
	printf("*** can't %s: %s\n", info, M_errstring(UOS_ErrnoGet()));
	return ERR_FUNC;
}

/********************************* PrintUosError ***************************/
/** Print User OSS error message
*
*  \param info       \IN  info string
*
*  \return           ERR_FUNC
*/
static int PrintUosError(char *info)
{
	printf("*** can't %s: %s\n", info, UOS_ErrString(UOS_ErrnoGet()));
	return ERR_FUNC;
}

/***************************************************************************/
/** Read and print input values
 *
 *  \return           success (0) or error code
 */
static int ReadInputs( void )
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

	printf("       === READ PORT === (*=un-configured port)\n");

	printf("Port 31..16: 31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16\n");
	printf("State      : ");
	for (p = 31; p >= 16; p--)
		printf("%c%d ",
		(G_portm & (1 << p) ? ' ' : '*'),
			(val >> p) & 1);
	printf("\n");

	return ERR_OK;
}

/***************************************************************************/
/** Signal handler
 *
 *  \param  sig    \IN   received signal
 */
static void __MAPILIB SignalHandler( u_int32 sig )
{
    if( sig == UOS_SIG_USR1 ) {
        ++G_sigCount;
		
		switch (G_mode) {
			/* show interrupt signal */
			case 2:
				printf("==> Interrupt signal #%d received\n", G_sigCount);
			break;

			/* read inputs after interrupt signal */
			case 3:
				printf("_____Interrupt Signal #%d_____\n", G_sigCount);
				ReadInputs();
			break;
		}		
    }
}
 
 
 
 
 
