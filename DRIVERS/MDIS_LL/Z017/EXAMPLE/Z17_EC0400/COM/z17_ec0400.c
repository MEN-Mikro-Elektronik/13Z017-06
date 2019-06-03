/****************************************************************************
 ************                                                    ************
 ************                   Z17EC0400                         ************
 ************                                                    ************
 ****************************************************************************/
/*!
 *         \file z17ec0400.c
 *       \author klaus.popp@men.de
 *
 *       \brief  Simple demo for buttons on EC04-00
 *
 *     Required: libraries: mdis_api, usr_oss
 *     \switches (none)
 */
 /*
 *---------------------------------------------------------------------------
 * Copyright (c) 2005-2019, MEN Mikro Elektronik GmbH
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
#include <MEN/men_typs.h>
#include <MEN/mdis_api.h>
#include <MEN/usr_oss.h>
#include <MEN/z17_drv.h>

/*--------------------------------------+
|   DEFINES                             |
+--------------------------------------*/
/* none */

/*--------------------------------------+
|   TYPDEFS                             |
+--------------------------------------*/
/* none */

/*--------------------------------------+
|   EXTERNALS                           |
+--------------------------------------*/
/* none */

/*--------------------------------------+
|   GLOBALS                             |
+--------------------------------------*/
static int G_sigCount = 0;

/*--------------------------------------+
|   PROTOTYPES                          |
+--------------------------------------*/
static void PrintError(char *info);
static void __MAPILIB SignalHandler( u_int32 sig );
static char* bitString( char *s, u_int32 val, int nrBits );

/********************************* main ************************************/
/** Program main function
 *
 *  \param argc       \IN  argument counter
 *  \param argv       \IN  argument vector
 *
 *  \return	          success (0) or error (1)
 */
int main(int argc, char *argv[])
{
	MDIS_PATH	path;
    u_int32 inp, out;
	char	*device;

	if (argc < 2 || strcmp(argv[1],"-?")==0) {
		printf("Syntax: z17ec0400 <device>\n");
		printf("Function: Example program for using the Z17 GPIO driver\n");
		printf("Options:\n");
		printf("    device       device name\n");
		printf("\n");
		return(1);
	}

	device = argv[1];

	/*--------------------+
    |  open path          |
    +--------------------*/
	if ((path = M_open(device)) < 0) {
		PrintError("open");
		return(1);
	}

	/*--------------------+
    |  config             |
    +--------------------*/

    /* install signal which will be received at change of input ports */
    UOS_SigInit( SignalHandler );
    UOS_SigInstall( UOS_SIG_USR1 );
    M_setstat( path, Z17_SET_SIGNAL, UOS_SIG_USR1 );

    /* program all ports as inputs, enable debouncer for keys */
    M_setstat( path, Z17_DIRECTION, 0x00 );
    M_setstat( path, Z17_DEBOUNCE, 0xFF );

    /* generate interrupts on falling edge of inputs */
    M_setstat( path, Z17_IRQ_SENSE, 0xAAAA );
    M_setstat( path, M_MK_IRQ_ENABLE, TRUE );

	/*--------------------+
    |  read values        |
    +--------------------*/
    for( out=0; out<32; ++out ) {
        char    buf[40];

        UOS_Delay( 10 );
        M_read( path, (int32 *)&inp );

        printf( "%d: In: %s (0x%x) IRQs: %d\n",
                out, bitString( buf, inp, 8 ),
                inp, G_sigCount );

        UOS_Delay( 1000 );
    }


	/*--------------------+
    |  cleanup            |
    +--------------------*/

    M_setstat( path, Z17_CLR_SIGNAL, 0 );
    UOS_SigRemove( UOS_SIG_USR1 );
    UOS_SigExit();

	if (M_close(path) < 0)
		PrintError("close");

	return(0);
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
static void __MAPILIB SignalHandler( u_int32 sig )
{
    if( sig == UOS_SIG_USR1 ) {
        ++G_sigCount;
    }
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

static char* bitString( char *s, u_int32 val, int nrBits )
{
    u_int32 i, bit;

    *s = '\0';

    bit = 1 << (nrBits-1);

    for( i=0; i<nrBits; ++i, bit>>=1 ) {
        strcat( s, val & bit ? "1 " : "0 " );
    }

    return( s );
}
 
 
 
 
 
