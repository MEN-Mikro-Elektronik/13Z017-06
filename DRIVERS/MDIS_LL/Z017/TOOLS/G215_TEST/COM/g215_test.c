/****************************************************************************
 ************                                                    ************
 ************                   G215_TEST                         ************
 ************                                                    ************
 ****************************************************************************/
/*!
 *         \file g215_test.c
 *       \author axel.morbach@men.de
 *
 *       \brief  Tool to test the G215
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
	printf("Usage: g215_test <tx-dev> <-t=port> <rx-dev> <-r=port> [-i=nbr] [-d=ms]\n");
	printf("Function: Access the 16Z034 I/Os\n");
	printf("Options:\n");
	printf("    tx-dev         device name transmitter    [none]\n");
	printf("    -t=<port>      i/o port 0..32 transmitter [none]\n");
	printf("    rx-dev         device name receiver       [none]\n");
	printf("    -r=<port>      i/o port 0..32 receiver    [none]\n");
	printf("    -i=<nbr>       instance number in message [none]\n");
	printf("    -d=<ms>        delay [ms] for loop        [500]\n");
	printf("\n");
	printf("Copyright (c) 2011-2019, MEN Mikro Elektronik GmbH\n%s\n", IdentString);
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
	int32	n;
	char	*deviceTx, *deviceRx, buf[40], *portStr, *errstr;
	int32	portTx, portRx, portBitTx, portBitRx;
    int32   dirTx, dirRx;
	int32	val, valRx;
	int32	errorcnt=0, errorsave=0, loopCnt=0, instance=0;
    int32	out = 0;
	u_int32 delay, errBunch;
	MDIS_PATH pathTx, pathRx;

	/*----------------------+
    |  check arguments      |
    +----------------------*/
	if( (errstr = UTL_ILLIOPT("t=r=n=i=d=?", buf)) ){
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
	for (deviceTx=NULL, deviceRx=NULL, n=1; n<argc; n++)
    {
        if ( NULL == deviceTx )
        {
            if (*argv[n] != '-') {
			    deviceTx = argv[n];
		    }
        }
        else
        {
            if (*argv[n] != '-') {
			    deviceRx = argv[n];
			    break;
		    }
        }
    }

	if ( (!deviceTx) || (!deviceRx) )
        {
		usage();
		return(1);
	}

	portTx = ((portStr = UTL_TSTOPT("t=")) ? atoi(portStr) : -1);
	portRx = ((portStr = UTL_TSTOPT("r=")) ? atoi(portStr) : -1);
	instance = ((portStr = UTL_TSTOPT("i=")) ? atoi(portStr) : -1);
	delay = ((portStr = UTL_TSTOPT("d=")) ? atoi(portStr) : 500);

	if( delay < 5 )
		errBunch = 1000;
	else if( delay < 50 )
		errBunch = 100;
	else if( delay < 500 )
		errBunch = 10;
	else 
		errBunch = 1;

	if( portTx != -1 ){
		portBitTx = 1 << portTx;
	}
	else{
		usage();
		return(1);
	}
	if( portRx != -1 ){
		portBitRx = 1 << portRx;
	}
	else{
		usage();
		return(1);
	}

	/*--------------------+
    |  open path          |
    +--------------------*/
	if ((pathTx = M_open(deviceTx)) < 0) 
    {
		PrintError("open TX");
		return(1);
	}
    if ( 0 == strcmp (deviceRx, deviceTx))
    {
       pathRx = pathTx;
    }
    else
    {
	    if ((pathRx = M_open(deviceRx)) < 0) 
        {
		    PrintError("open RX");
		    return(1);
	    }
    }

	/*----------------------+
	|  set port direction   |
	+----------------------*/
	if ((M_getstat(pathTx, Z17_DIRECTION, &dirTx)) < 0) 
    {
		PrintError("getstat Z17_DIRECTION");
		goto abort;
	}
	dirTx |= portBitTx;
	if ((M_setstat(pathTx, Z17_DIRECTION, dirTx)) < 0) 
    {
		PrintError("setstat Z17_DIRECTION");
		goto abort;
	}

	if ((M_getstat(pathRx, Z17_DIRECTION, &dirRx)) < 0) 
    {
		PrintError("getstat Z17_DIRECTION");
		goto abort;
	}
	dirRx &= ~portBitRx;
	if ((M_setstat(pathRx, Z17_DIRECTION, dirRx)) < 0) 
    {
		PrintError("setstat Z17_DIRECTION");
		goto abort;
	}

	/*----------------------+
	|  toggle output        |
	+----------------------*/
	/* repeat until keypress */
	do {
        loopCnt++;
        /* write port */
		out = !out;

		if ((M_setstat(pathTx, out ? Z17_SET_PORTS : Z17_CLR_PORTS, portBitTx)) < 0) 
        {
			PrintError("setstat Z17_SET_PORTS/Z17_CLR_PORTS");
			goto abort;
		}

        /* read port */
		if ((M_read(pathRx, &val ))  < 0 )
        {
			PrintError("read");
			goto abort;
		}
        if ( (val & portBitRx) == 0)
            valRx = 0;
        else
            valRx = 1;
				
        if ( ( out ) && ( valRx == 0 ) )
        {
            errorcnt++;
        }
        else if ( ( !out ) && ( valRx == 1 ) )
        {
            errorcnt++;
        }
        
        if ( (loopCnt % errBunch )== 0 )
        {
            if ( errorsave != errorcnt ){
				if( instance != -1 )
					printf("Instance %d: ", instance);
                printf("loop=%d, errors=%d\n", loopCnt, errorcnt );
			}

            errorsave = errorcnt;
        }

		UOS_Delay( delay );
	} while( UOS_KeyPressed() == -1 );		

	/*--------------------+
    |  cleanup            |
    +--------------------*/
	abort:
	if (M_close(pathTx) < 0)
		PrintError("close");
    if ( 0 != strcmp (deviceRx, deviceTx))
    {
		if (M_close(pathRx) < 0)
			PrintError("close");
    }

	return(0);
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

