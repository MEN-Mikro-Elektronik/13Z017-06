/****************************************************************************
 ************                                                    ************
 ************                   Z17EC0400                         ************
 ************                                                    ************
 ****************************************************************************/
/*!
 *         \file z17ec0400.c
 *       \author klaus.popp@men.de
 *        $Date: 2009/07/10 13:38:54 $
 *    $Revision: 1.2 $
 *
 *       \brief  Simple demo for buttons on EC04-00
 *
 *     Required: libraries: mdis_api, usr_oss
 *     \switches (none)
 */
 /*-------------------------------[ History ]--------------------------------
 *
 * $Log: z17_ec0400.c,v $
 * Revision 1.2  2009/07/10 13:38:54  CRuff
 * R: Porting to MDIS5
 * M: 1. changed type of M_open Path to MDIS_PATH
 *    2. added __MAPILIB macro to Signal Handler signature
 *
 * Revision 1.1  2005/04/27 14:10:14  kp
 * Initial Revision
 *
 *---------------------------------------------------------------------------
 * (c) Copyright 2005 by MEN mikro elektronik GmbH, Nuernberg, Germany
 ****************************************************************************/

static const char RCSid[]="$Id: z17_ec0400.c,v 1.2 2009/07/10 13:38:54 CRuff Exp $";

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
 
 
