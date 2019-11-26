/****************************************************************************
 ************                                                    ************
 ************                G215_190029_ET                      ************
 ************                                                    ************
 ****************************************************************************/
/*!
 *         \file g215_190029_et.c
 *       \author dieter.pfeuffer@men.de
 *
 *       \brief  G215 test tool for 190029 env-test (and 16G215 design-test)
 *
 *               HW configuration according 190029-_HwTestSpec
 *               (or 16G215-01_IcTestSpec). 
 *
 *     Required: libraries: mdis_api, usr_oss, usr_utl
 *     \switches (none)
 */
 /*

 *---------------------------------------------------------------------------
 * Copyright 2011-2019, MEN Mikro Elektronik GmbH
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
#include <MEN/usr_oss.h>
#include <MEN/usr_utl.h>
#include <MEN/z17_drv.h>

static const char IdentString[]=MENT_XSTR(MAK_REVISION);

/*--------------------------------------+
|  DEFINES                              |
+--------------------------------------*/
#define MAX_DEVS	5

/* gpio#0 ports */
#define G0_LED1	(1<<1)
#define G0_LED2	(1<<2)
#define G0_LED3	(1<<3)
#define G0_LED4	(1<<0)
#define G0_EMG	(1<<4)

/* gpio#3 ports */
#define G3_P3B3	(1<<8)

/*--------------------------------------+
|  TYPEDEFS                             |
+--------------------------------------*/
typedef struct {
	char name[100];
	MDIS_PATH hdl;
} MDEV;

typedef struct {
	char	txPortLoc[50];
	u_int8	txDev;
	u_int8	txPort;
	
	char	rxPortLoc[50];
	u_int8	rxDev;
	u_int8	rxPort;

	int32	rxLast;
	u_int8	errFlag;
	int 	errCount;
} LCFG;

/* ========== 190029 env-test ========== */
LCFG G_lcfgEnvTest[] = 
{
	/* 0*/ { ""					,99, 99,		""					,99, 99 },
	/* 1*/ { "#12 F220/1 TX-1"	, 1,  0,		"#13 F221/1 RX-1"	, 2, 31 },
	/* 2*/ { "#12 F220/1 TX-8"	, 1,  3,		"#13 F221/1 RX-8"	, 3,  4 },
	/* 3*/ { "#14 F220/2 TX-2"	, 1, 24,		"#15 F221/2 RX-2"	, 3, 18 },
	/* 4*/ { "#14 F220/2 TX-7"	, 1, 27,		"#15 F221/2 RX-7"	, 3, 21 },
	/* 5*/ { "#16 F220/3 TX-3"	, 2,  5,		"#18 F221/3 RX-3"	, 1,  9 },
	/* 6*/ { "#16 F220/3 TX-6"	, 2,  6,		"#18 F221/3 RX-6"	, 1, 10 },
	/* 7*/ { "#17 F220/4 TX-4"	, 4,  3,		"#20 F221/4 RX-4"	, 4, 25 },
	/* 8*/ { "#17 F220/4 TX-5"	, 4,  4,		"#20 F221/4 RX-5"	, 4, 26 },
	/* 9*/ { "#19 F220/5 TX-1"	, 4, 12,		"#13 F221/1 RX-4"	, 3,  6 },
	/*10*/ { "#19 F220/5 TX-8"	, 4, 17,		"#13 F221/1 RX-5"	, 3,  7 },
	/*11*/ { "#21 G215   TX  "	, 4, 28,		"#15 F221/2 RX-1"	, 3, 12 },
	/*12*/ { "#14 F220/2 TX-1"	, 1, 20,		"#21 G215   RX  "	, 4, 29 },
	/*13*/ { "#21 G215 SERTX "	, 0, 16,		"#21 G215 SERRX "	, 0, 17 },
	/*14*/ { "#21 G215 SERDTR"	, 0, 18,		"#21 G215 SERCTS"	, 0, 21 },
};

/* ========== 16G215 design-test ========== */
LCFG G_lcfgDesTest[] =
{
	/* ===== P3 ===== */
	/* 0*/ { ""	   		,99,  99,	""		,99, 99 },
	/* 1*/ { "P3:A7"	, 2,  31,	"P3:A5"	, 3,  0 },
	/* 2*/ { "P3:A3"	, 3,   1,	"P3:A1"	, 3,  2 },
	/* 3*/ { "P3:B8"	, 3,   3,	"P3:B6"	, 3,  5 },
	/* 4*/ { "P3:B7"	, 3,   4,	"P3:B5"	, 3,  6 },
	/* 5*/ { "P3:B4"	, 3,   7,	"P3:B2"	, 3,  9 },
	/*     { "P3:B1"	, 0,   4,	"P3:B3"	, 3,  8 }, special test with EMG_OFF */
	/* 6*/ { "P3:C4"	, 3,  10,	"P3:C2"	, 3, 11 },
	/* 7*/ { "P3:D7"	, 3,  12,	"P3:D5"	, 3, 13 },
	/* 8*/ { "P3:D3"	, 3,  14,	"P3:D1"	, 3, 15 },
	/* 9*/ { "P3:E8"	, 3,  16,	"P3:E6"	, 3, 18 },
	/*10*/ { "P3:E7"	, 3,  17,	"P3:E5"	, 3, 19 },
	/*11*/ { "P3:E4"	, 3,  20,	"P3:E2"	, 3, 22 },
	/*12*/ { "P3:E3"	, 3,  21,	"P3:E1"	, 3, 23 },
	/*13*/ { "P3:F8"	, 3,  24,	"P3:F6"	, 3, 25 },
	/*14*/ { "P3:F4"	, 3,  26,	"P3:F2"	, 3, 27 },
	/*15*/ { "P3:G7"	, 3,  28,	"P3:G5"	, 3, 29 },
	/*16*/ { "P3:G3"	, 3,  30,	"P3:G1"	, 3, 31 },
	/*17*/ { "P3:H8"	, 4,   0,	"P3:H6"	, 4,  2 },
	/*18*/ { "P3:H7"	, 4,   1,	"P3:H5"	, 4,  3 },
	/*19*/ { "P3:H4"	, 4,   4,	"P3:H2"	, 4,  6 },
	/*20*/ { "P3:H3"	, 4,   5,	"P3:H1"	, 4,  7 },
	/*21*/ { "P3:I8"	, 4,   8,	"P3:I6"	, 4,  9 },
	/*22*/ { "P3:I4"	, 4,  10,	"P3:I2"	, 4, 11 },
	/*23*/ { "P3:J7"	, 4,  12,	"P3:J5"	, 4, 13 },
	/*24*/ { "P3:J3"	, 4,  14,	"P3:J1"	, 4, 15 },
	/*25*/ { "P3:K8"	, 4,  16,	"P3:K6"	, 4, 18 },
	/*26*/ { "P3:K7"	, 4,  17,	"P3:K5"	, 4, 19 },
	/*27*/ { "P3:K4"	, 4,  20,	"P3:K2"	, 4, 22 },
	/*28*/ { "P3:K3"	, 4,  21,	"P3:K1"	, 4, 23 },
	/*29*/ { "P3:L8"	, 4,  24,	"P3:L6"	, 4, 25 },        
	/*30*/ { "P3:L4"	, 4,  26,	"P3:L2"	, 4, 27 },

	/* ===== P4 ===== */
	/*31*/ { "P4:A7"	, 1,   0,	"P4:A5"	, 1,  1 },
	/*32*/ { "P4:A3"	, 1,  29,	"P4:A1"	, 1, 31 },
	/*33*/ { "P4:B8"	, 1,   2,	"P4:B6"	, 1,  4 },
	/*34*/ { "P4:B7"	, 1,   3,	"P4:B5"	, 1,  5 },
	/*35*/ { "P4:B4"	, 1,   6,	"P4:B2"	, 2,  9 },
	/*36*/ { "P4:B3"	, 1,   7,	"P4:B1"	, 2, 16 },
	/*37*/ { "P4:C8"	, 2,  17,	"P4:C6"	, 2, 18 },
	/*38*/ { "P4:C4"	, 2,  19,	"P4:C2"	, 2, 10 },
	/*39*/ { "P4:D7"	, 1,  20,	"P4:D5"	, 1, 21 },
	/*40*/ { "P4:D3"	, 1,  30,	"P4:D1"	, 2, 11 },
	/*41*/ { "P4:E8"	, 1,  22,	"P4:E6"	, 1, 24 },
	/*42*/ { "P4:E7"	, 1,  23,	"P4:E5"	, 1, 25 },
	/*43*/ { "P4:E4"	, 1,  26,	"P4:E2"	, 2, 22 },
	/*44*/ { "P4:E3"	, 1,  27,	"P4:E1"	, 2,  0 },
	/*45*/ { "P4:F8"	, 1,  28,	"P4:F6"	, 2,  8 },
	/*46*/ { "P4:F4"	, 2,   1,	"P4:F2"	, 2, 23 },
	/*47*/ { "P4:G7"	, 2,   4,	"P4:G5"	, 2,  5 },
	/*48*/ { "P4:G3"	, 0,  28,	"P4:G1"	, 0, 29 },
	/*49*/ { "P4:H8"	, 2,   6,	"P4:H6"	, 0, 24 },
	/*50*/ { "P4:H7"	, 2,   7,	"P4:H5"	, 0, 25 },
	/*51*/ { "P4:H4"	, 0,  26,	"P4:H2"	, 0, 30 },
	/*52*/ { "P4:H3"	, 0,  27,	"P4:H1"	, 2,  2 },
	/*53*/ { "P4:I8"	, 2,   3,	"P4:I6"	, 2, 20 },
	/*54*/ { "P4:I4"	, 2,  21,	"P4:I2"	, 0, 31 },
	/*55*/ { "P4:J7"	, 1,   8,	"P4:J5"	, 1,  9 },
	/*56*/ { "P4:J3"	, 1,  16,	"P4:J1"	, 1, 17 },
	/*57*/ { "P4:K8"	, 1,  10,	"P4:K6"	, 1, 12 },
	/*58*/ { "P4:K7"	, 1,  11,	"P4:K5"	, 1, 13 },
	/*59*/ { "P4:K4"	, 1,  14,	"P4:K2"	, 1, 18 },
	/*60*/ { "P4:K3"	, 1,  15,	"P4:K1"	, 1, 19 },
	/*61*/ { "P4:L8"	, 2,  12,	"P4:L6"	, 2, 13 },
	/*62*/ { "P4:L4"	, 2,  14,	"P4:L2"	, 2, 15 },
	/*63*/ { "G215 TX"	  , 4,  28,	"G215 RX"    , 4, 29 },
	/*64*/ { "G215 SERTX" , 0, 16,	"G215 SERRX" , 0, 17 },
	/*65*/ { "G215 SERDTR", 0, 18,	"G215 SERCTS", 0, 21 },	
};

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
	printf("Usage: g215_190029_et <dev> <-i=nbr>\n");
	printf("Function: G215 test tool for I/Os\n");
	printf("Options:\n");
	printf("    dev       z127 base device name without instance number\n");
	printf("                e.g. z17_z127_\n");
	printf("    -i=<nbr>  device instance number of first gpio (gpio instance 0)\n");
	printf("                e.g. -i=1\n");
	printf("    -d=<ms>   delay between each tx->rx, optional\n");
	printf("    -t=<nbr>  select test:\n");
	printf("                0 = 190029 env-test (default)\n");
	printf("                1 = 16G215 design-test\n");
	printf("    -n=<nbr>  number of runs:\n");
	printf("                 0 = endless until keypress (default)\n");
	printf("                >0 = runs 2x<nbr>\n");
	printf("    -e        show errors only\n");
	printf("\n");
	printf("LED indication:\n");
	printf("LED#1 : Error occured, test failed\n");
	printf("LED#2 : Any connection lost\n");
	printf("LED#3 : Test live indicator (toggling)\n");
	printf("LED#4 : Test running\n");

	printf("Copyright 2011-2019, MEN Mikro Elektronik GmbH\n%s\n", IdentString);
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
	int32	val, valRx;
	int32	ret=1;
	int 	n, loopCnt=0, allErrs=0, runs, maxLines;
	char	*devBaseName, *str, buf[100], *errstr;
	int32	out=0, liveOut=0, dir, delay, test, onlyErr;
	int8	firstInstanceNbr, connLost;
	MDEV	mdev[MAX_DEVS];
	LCFG 	*G_lcfgP=NULL; 		/* G_lcfgP[0] unused */
	int 	emg_p3b3_errCount=0;


	/*----------------------+
    |  check arguments      |
    +----------------------*/
	if( (errstr = UTL_ILLIOPT("i=d=t=n=e?", buf)) ){
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
	for (devBaseName=NULL, n=1; n<argc; n++)
		if (*argv[n] != '-') {
			devBaseName = argv[n];
			break;
		}

	firstInstanceNbr = (int8)((str = UTL_TSTOPT("i=")) ? atoi(str) : -1);
	delay   = (str = UTL_TSTOPT("d=")) ? atoi(str) : 0;
	test    = (str = UTL_TSTOPT("t=")) ? atoi(str) : 0;
	runs    = 2 * ((str = UTL_TSTOPT("n=")) ? atoi(str) : 0);
	onlyErr = (UTL_TSTOPT("e") ? 1 : 0);

	if ( !devBaseName || (firstInstanceNbr==-1) ) {
		usage();
		return(1);
	}

	/* select test */ 
	switch( test ){
	/* ========== 190029 env-test ========== */
	case 0: 
		G_lcfgP = (LCFG*)&G_lcfgEnvTest;
		maxLines = sizeof(G_lcfgEnvTest)/sizeof(LCFG);
		break;
	/* ========== 16G215 design-test ========== */	
	case 1: 
		G_lcfgP = (LCFG*)&G_lcfgDesTest;
		maxLines = sizeof(G_lcfgDesTest)/sizeof(LCFG);
		break;
	default:
		usage();
		return(1);
	}		

	/*---------------------------+
	|  for each device instance  |
	+---------------------------*/
	for( n=0; n<MAX_DEVS; n++ ){

		/* assemble device instance name */
		sprintf( mdev[n].name, "%s%i", devBaseName, (int)(firstInstanceNbr+n) );

		/*--------------------+
		|  open path          |
		+--------------------*/
		if ((mdev[n].hdl = M_open(mdev[n].name)) < 0) 
		{
			sprintf( buf, "open %s", mdev[n].name );
			PrintError(buf);
			return(1);
		}
	} /* for */

	if( runs )
		printf("%d runs\n", runs);
	else
		printf("runs endless until keypress\n");
	
	/*----------------------------------+
	|  set port direction for EMG, LEDs |
	+----------------------------------*/
	if ((M_setstat(mdev[0].hdl, Z17_DIRECTION,
		G0_LED1 | G0_LED2 | G0_LED3 | G0_LED4 | G0_EMG)) < 0){
		PrintError("setstat Z17_DIRECTION tx");
		goto abort;
	}

	/*--------------------+
	|  switch LEDs        |
	+--------------------*/
	/* switch on LED#1..4 */ 
	if ((M_setstat(mdev[0].hdl, Z17_CLR_PORTS,
		G0_LED1 | G0_LED2 | G0_LED3 | G0_LED4)) < 0){
		PrintError("setstat Z17_CLR_PORTS");
		goto abort;
	}

	/* switch off LED#1..3 (step by step) */ 
	for( n=1; n<=3; n++ ){
		switch( n ){
			case 1: out = G0_LED1; break;
			case 2: out = G0_LED2; break;
			case 3: out = G0_LED3; break;
		}

		UOS_Delay( 500 );

		if ((M_setstat(mdev[0].hdl, Z17_SET_PORTS, out)) < 0){
			PrintError("setstat Z17_SET_PORTS tx");
			goto abort;
		}
	}

	/* ========== 16G215 design-test ========== */
	if( test == 1 ){
		/* set P3B3 as input */		
		M_getstat(mdev[3].hdl, Z17_DIRECTION, &dir);
		M_setstat(mdev[3].hdl, Z17_DIRECTION, dir & ~G3_P3B3);
					
		out = 0;
		for( n=0; n<(runs?runs:10); n++ ){
		
			/* set/clear emergency bit */
			M_setstat(mdev[0].hdl, out ? Z17_SET_PORTS : Z17_CLR_PORTS, G0_EMG);
	
			/* read P3B3 */
			M_read(mdev[3].hdl, &val );
			valRx = (val & G3_P3B3) ? 1 : 0;
				
			/*
			 * transmission error?
			 * Note: emergency bit is inverted!
			 */
			if( out == valRx )
				emg_p3b3_errCount++;
		}/* for */	
				
		printf("%s: P3:B1 (0:04) (!em_fpga&ICE=em_off) --> P3:B3 (3:08), err=%d\n",
			emg_p3b3_errCount ? "*ERR*" : "STATE", emg_p3b3_errCount);
				
		allErrs += emg_p3b3_errCount;			
	}/* if */

	/*---------------------------+
	|  for each line             |
	+---------------------------*/
	out = 1;
	for( n=1; n<maxLines; n++ ){

		/* clear errors */
		G_lcfgP[n].errFlag = 0;
		G_lcfgP[n].errCount = 0;
		G_lcfgP[n].rxLast = out;
		
		/*----------------------+
		|  set port direction   |
		+----------------------*/
		if ((M_getstat(mdev[G_lcfgP[n].txDev].hdl, Z17_DIRECTION, &dir)) < 0){
			PrintError("getstat Z17_DIRECTION tx");
			goto abort;
		}

		dir |= 1<<G_lcfgP[n].txPort;


		if ((M_setstat(mdev[G_lcfgP[n].txDev].hdl, Z17_DIRECTION, dir)) < 0){
			PrintError("setstat Z17_DIRECTION tx");
			goto abort;
		}

		if ((M_getstat(mdev[G_lcfgP[n].rxDev].hdl, Z17_DIRECTION, &dir)) < 0){
			PrintError("getstat Z17_DIRECTION rx");
			goto abort;
		}

		dir &= ~(1<<G_lcfgP[n].rxPort);

		if ((M_setstat(mdev[G_lcfgP[n].rxDev].hdl, Z17_DIRECTION, dir)) < 0){
			PrintError("setstat Z17_DIRECTION rx");
			goto abort;
		}
	}/* for */

	/*------------------------+
	|  clear emergency bit    |
	+------------------------*/
	if ((M_setstat(mdev[0].hdl, Z17_CLR_PORTS, G0_EMG)) < 0){
		PrintError("setstat Z17_CLR_PORTS");
		goto abort;
	}

	/*----------------------+
	|  toggle output        |
	+----------------------*/
	/* repeat until keypress */
	do {
        loopCnt++;
		out = !out;
		connLost = 0;

		if( runs && delay )
			printf("--- run #%d ---\n", loopCnt);

		/*---------------------------+
		|  for each line             |
		+---------------------------*/
		for( n=1; n<maxLines; n++ ){

			/* write port */
			if ((M_setstat(mdev[G_lcfgP[n].txDev].hdl,
				out ? Z17_SET_PORTS : Z17_CLR_PORTS, ((U_INT32_OR_64)1)<<G_lcfgP[n].txPort)) < 0) {
				PrintError("setstat Z17_SET_PORTS/Z17_CLR_PORTS");
				goto abort;
			}

			/* delay required for serial interface via SA adapter */
			UOS_Delay( delay );

			/* read port */
			if ((M_read(mdev[G_lcfgP[n].rxDev].hdl, &val ))  < 0 ){
				PrintError("read");
				goto abort;
			}

			valRx = (val & (1<<G_lcfgP[n].rxPort)) ? 1 : 0;

			/*
			 * transmission success?
			 * Note: To evaluate edges, the actual read value
			 *       must be the inverted last read value!  
			 */
			if ( valRx != G_lcfgP[n].rxLast ){

				if( (G_lcfgP[n].errFlag) || (loopCnt==1 && !onlyErr) ){
					printf("STATE: ln%2d: %s (%d:%02d)-->%s (%d:%02d), err=%d, n=%d\n",
						n,
						G_lcfgP[n].txPortLoc, G_lcfgP[n].txDev, G_lcfgP[n].txPort,
						G_lcfgP[n].rxPortLoc, G_lcfgP[n].rxDev, G_lcfgP[n].rxPort,
						G_lcfgP[n].errCount, loopCnt );

					G_lcfgP[n].errFlag = 0;
				}

			/* transmission error */
			} else {

				G_lcfgP[n].errCount++;
				connLost = 1;

				if( (!G_lcfgP[n].errFlag) || loopCnt==1 ){
					printf("*ERR*: ln%2d: %s (%d:%02d)-->%s (%d:%02d), err=%d, n=%d\n",
						n,
						G_lcfgP[n].txPortLoc, G_lcfgP[n].txDev, G_lcfgP[n].txPort,
						G_lcfgP[n].rxPortLoc, G_lcfgP[n].rxDev, G_lcfgP[n].rxPort,
						G_lcfgP[n].errCount, loopCnt );

					G_lcfgP[n].errFlag = 1;
				}

			} /* if/else */
        
			G_lcfgP[n].rxLast = valRx;

		}/* for */

		/* any connection lost? */
		if( connLost ){
			/*
			* - switch LED#1 on to indicate test failed
			* - switch LED#2 on to indicate connection lost
			*/
			if ((M_setstat(mdev[0].hdl, Z17_CLR_PORTS,
					G0_LED1 | G0_LED2)) < 0){
				PrintError("setstat Z17_CLR_PORTS tx");
				goto abort;
			}
		}
		else {
			/* switch LED#2 off to indicate connection */
			if ((M_setstat(mdev[0].hdl, Z17_SET_PORTS, G0_LED2)) < 0){
				PrintError("setstat Z17_SET_PORTS tx");
				goto abort;
			}
		}

		/* toggle LED#3 as live indicator */
		if( !(loopCnt % 10) ){
			liveOut = !liveOut;

			if ((M_setstat(mdev[0].hdl, 
				liveOut ? Z17_SET_PORTS : Z17_CLR_PORTS, G0_LED3)) < 0){
				PrintError("setstat Z17_SET_PORTS/Z17_CLR_PORTS tx");
				goto abort;
			}
		}

	} while( runs?(loopCnt<runs):(UOS_KeyPressed() == -1) );		

	/*---------------------------+
	|  for each line             |
	+---------------------------*/
	printf("List of Errors:\n");
	for( n=1; n<maxLines; n++ ){
		if( G_lcfgP[n].errCount ){
			printf("line %2d: TX %s --> RX %s, errors=%d\n",
				n, G_lcfgP[n].txPortLoc, G_lcfgP[n].rxPortLoc, G_lcfgP[n].errCount );
			allErrs += G_lcfgP[n].errCount;
		}
	}/* for */

	if( allErrs ){
		printf("*** TEST FAILED with %d errors ***\n", allErrs);
	} else {
		printf("=== TEST SUCCEEDED ===\n");
		ret = 0;
	}

	/*--------------------+
    |  cleanup            |
    +--------------------*/
abort:

	/* switch LED#4 off to indicate test ended */
	if ((M_setstat(mdev[0].hdl, Z17_SET_PORTS, G0_LED4)) < 0){
		PrintError("setstat Z17_SET_PORTS tx");
		goto abort;
	}

	/*---------------------------+
	|  for each device instance  |
	+---------------------------*/
	for( n=0; n<MAX_DEVS; n++ ){
		if (M_close(mdev[n].hdl) < 0)
			PrintError("close");
	}

	return(ret);
}

/********************************* PrintError ******************************/
/** Print MDIS error message
 *
 *  \param info       \IN  info string
*/
static void PrintError(char *info)
{
	printf("***  can't %s: %s\n", info, M_errstring(UOS_ErrnoGet()));
}



