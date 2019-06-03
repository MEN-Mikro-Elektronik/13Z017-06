/****************************************************************************
 ************                                                    ************
 ************                   Z127_IRQLAT                      ************
 ************                                                    ************
 ****************************************************************************/
/*!
 *         \file z127_irqlat.c
 *       \author dieter.pfeuffer@men.de
 *
 *       \brief  Tool to test IRQ latency with 16Z127 (32-bit) I/Os
 *
 *     Required: libraries: mdis_api, usr_oss, usr_utl
 *     \switches (none)
 */
 /*
 *---------------------------------------------------------------------------
 * Copyright (c) 2011-2019, MEN Mikro Elektronik GmbH
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <MEN/men_typs.h>
#include <MEN/mdis_api.h>
#include <MEN/mdis_err.h>
#include <MEN/usr_oss.h>
#include <MEN/usr_utl.h>
#include <MEN/z17_drv.h>

static const char IdentString[]=MENT_XSTR(MAK_REVISION);

/*--------------------------------------+
|   PROTOTYPES                          |
+--------------------------------------*/
static void usage(void);
static void PrintError(char *info);

/********************************* usage ***********************************/
/**  Print program usage
 */
static void usage(void)
{
	printf("Usage: z127_irqlat [<opts>] <device> [<opts>]\n");
	printf("Function: Test IRQ latency with 16Z127 (32-bit) I/Os\n");
	printf("Options:\n");
	printf("    device    device name                         \n");
	printf("    -o=<port> output port 0..31                   \n");
	printf("    -i=<port> input port 0..31                    \n");
	printf("    -n=<nbr>  number of IRQs to fire        [1000]\n");
	printf("    -H        hold path open until keypress       \n");
	printf("    -L        loop until keypress                 \n");
	printf("Copyright (c) 2010-2019, MEN Mikro Elektronik GmbH\n%s\n", IdentString);
}

/***************************************************************************/
/** Program main function
 *
 *  \param argc       \IN  argument counter
 *  \param argv       \IN  argument vector
 *
 *  \return	          success (0) or error (1)
 */
int main(int argc, char *argv[])
{
	int32	n, timeout;
	char	*device, *str, *errstr, buf[40];
	int32	outPort, inPort, irqs2fire, hold, loop, err;
	u_int32	tickDiff;
	double	time;	
	int		ret=1;
	
	MDIS_PATH path;
	M_SG_BLOCK blk;
	Z17_BLK_IRQLAT irqlat;

	/*----------------------+
    |  check arguments      |
    +----------------------*/
	if( (errstr = UTL_ILLIOPT("o=i=n=HLh?", buf)) ){
		printf("*** %s\n", errstr);
		return(1);
	}

	if( UTL_TSTOPT("?") ){
		usage();
		return(1);
	}

	if( argc < 3 ){
		usage();
		return(1);
	}

	/*----------------------+
    |  get arguments        |
    +----------------------*/
	for (device=NULL, n=1; n<argc; n++)
		if (*argv[n] != '-') {
			device = argv[n];
			break;
		}

	if (!device) {
		usage();
		return(1);
	}

	outPort		= ((str = UTL_TSTOPT("o=")) ? atoi(str) : -1);
	inPort		= ((str = UTL_TSTOPT("i=")) ? atoi(str) : -1);
	irqs2fire	= ((str = UTL_TSTOPT("n=")) ? atoi(str) : 1000);
	hold        = (UTL_TSTOPT("H") ? 1 : 0);
	loop	    = (UTL_TSTOPT("L") ? 1 : 0);

	if( (outPort==-1) || (inPort==-1) ){
		usage();
		return(1);
	}

	/*--------------------+
    |  open path          |
    +--------------------*/
	if ((path = M_open(device)) < 0) {
		PrintError("open");
		return(1);
	}

	/*--------------------------+
	|  config IRQ latency test  |
	+--------------------------*/
	blk.size = sizeof(Z17_BLK_IRQLAT);
	blk.data = (void*)&irqlat;
	
	irqlat.outPort 		= outPort;
	irqlat.inPort 		= inPort;
	irqlat.irqs2fire 	= irqs2fire;

	/* enable global device interrupt */
	if ((M_setstat(path, M_MK_IRQ_ENABLE, TRUE)) < 0) {
		PrintError("setstat M_MK_IRQ_ENABLE TRUE");
		goto abort;
	}

	/* do once or repeat until keypress */
	do {
		/*------------------------+
		|  init IRQ latency test  |
		+------------------------*/
		if ((M_setstat(path, Z17_BLK_IRQLAT_START, (INT32_OR_64)&blk)) < 0) {
			PrintError("setstat Z17_BLK_IRQLAT_START");
			goto abort;
		}

		/*------------------------+
		|  get result             |
		+------------------------*/
		timeout = 0;
		do {
			UOS_Delay( 100 );
			err = M_getstat(path, Z17_BLK_IRQLAT_RESULT, (int32*)&blk);
			timeout++;
		} while( (err < 0) && (UOS_ErrnoGet() == ERR_LL_DEV_BUSY) && (timeout<100) );
		
		if( err < 0 ){
			PrintError("getstat Z17_BLK_IRQLAT_RESULT");
			goto abort;
		}

		/*------------------------+
		|  compute latency        |
		+------------------------*/
		tickDiff = irqlat.stopTick - irqlat.startTick;
		time = (double)tickDiff / (double)irqlat.tickRate;
				
		printf("%d IRQs fired, %d IRQs received\n",
			irqs2fire, irqlat.irqsRcved ); 
		printf(" tickRate  = %d t/s\n", irqlat.tickRate); 
		printf(" startTick = %d\n", irqlat.startTick); 
		printf(" stopTick  = %d\n", irqlat.stopTick); 
		printf(" tickDiff  = %d\n", tickDiff); 
		printf(" time      = %8.3fs\n", time); 
		printf(" time/IRQ  = %8.3fus\n", ((double)1000000 * time) / (double)irqs2fire); 

	} while( loop && (UOS_KeyPressed() == -1) );

	/* hold path open until keypress */
	if( hold ){
		printf("holding path open until keypress\n");
		UOS_KeyWait();
	}

	ret = 0;
	
	/*--------------------+
    |  cleanup            |
    +--------------------*/
	abort:
		
	/* disable global device interrupt */
	if ((M_setstat(path, M_MK_IRQ_ENABLE, FALSE)) < 0) {
		PrintError("setstat M_MK_IRQ_ENABLE FALSE");
		goto abort;
	}
		
	if (M_close(path) < 0)
		PrintError("close");

	return ret;
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


