/*********************  P r o g r a m  -  M o d u l e ***********************/
/*!
 *        \file  z17_drv.c
 *
 *      \author  ulrich.bogensperger@men.de 
 *        $Date: 2016/07/13 10:09:33 $
 *    $Revision: 1.12 $
 *
 *       \brief  Low-level driver for the 16Z034, 16Z037 and 16Z127
 *               GPIO controller
 *
 *     Required: OSS, DESC, DBG libraries
 *
 *    \switches  _ONE_NAMESPACE_PER_DRIVER_
 *               Z17_MODEL_Z127
 *               Z127_NOIRQ
 */
 /*-------------------------------[ History ]--------------------------------
 *
 * $Log: z17_drv.c,v $
 * Revision 1.12  2016/07/13 10:09:33  ts
 * R: m_open failed on EM10A00 GPIOs with 0x20 address space size.
 * M: previous default ADDRSPACE_SIZE of 0x100 failed to match, corrected back to 0x20
 *
 * Revision 1.11  2014/11/28 14:17:37  MRoth
 * R: 1.) no toggle function for GPIOs
 *    2.) Z127 variant always requested an IRQ even if it had none in Hardware
 *    3.) no support for MSI(x) - no IRQ should happen during ISR
 *    4.) GPIO reset at driver exit (m_close) not always wanted
 *    5.) cosmetics
 * M: 1.a) added setstats Z17_TOG_PORTS, Z17_TOG_HIGH, Z17_TOG_LOW
 *      b) added AlarmHandler() for timer
 *    2.) introduced Z127_NOIRQ variant to inhibit IRQs.
 *    3.) disable IRQs at the beginning and re-enable at the end of the ISR
 *    4.) added RESET_OFF descriptor key to init function
 *    5.) revised code completely
 *
 * Revision 1.10  2011/11/16 15:24:31  GLeonhardt
 * R: 1.) Interrupts for GPIO 4-7 did not work with Z34/Z37 core
 *        A preprocessor string comparison did not work.
 *        Driver always compiled as Z127 model with 32 bit registers.
 * M: 1.) Replace string comparison with driver switch from Makefile.
 *
 * Revision 1.9  2011/09/12 13:51:23  GLeonhardt
 * R: Compiler error, if MWRITE_D32 macro is a compound statement
 * M: Embed macro in braces
 *
 * Revision 1.8  2011/07/27 15:17:49  dpfeuffer
 * R: IRQ latency test initialization problem
 * M: IRQ latency test initialization revised
 *
 * Revision 1.7  2011/07/08 13:20:09  dpfeuffer
 * R: IRQ latency test required for 16G215-01 design test
 * M: IRQ latency test implemented
 *
 * Revision 1.6  2009/08/03 16:43:18  MRoth
 * R: no support for 32 bit Z127_GPIO IP core
 * M: a) added define Z17_MODEL_Z127
 *    b) added Z17_MODEL_Z127 section to setstat/getstat functions
 *
 * Revision 1.5  2009/07/10 10:15:49  CRuff
 * R: compiler warnings because of unused variables
 * M: commented out the unused variables
 *
 * Revision 1.4  2009/04/17 10:23:38  MRoth
 * R: no 64 Bit support
 * M: changed set/getstat to support 64 Bit
 *
 * Revision 1.3  2006/12/20 11:23:21  ufranke
 * added
 *  + getstat code Z17_IRQ_LAST_REQUEST
 *
 * Revision 1.2  2006/06/01 17:03:12  cs
 * changed ADDRSPACE_SIZE to 32 (real size is reported by Chameleon BBIS)
 *
 * Revision 1.1  2004/06/18 14:29:50  ub
 * Initial Revision
 *
 *---------------------------------------------------------------------------
 * (c) Copyright 2004 by MEN Mikro Elektronik GmbH, Nuernberg, Germany
 ****************************************************************************/

#define _NO_LL_HANDLE        /* ll_defs.h: don't define LL_HANDLE struct */

/*-----------------------------------------+
|  INCLUDES                                |
+-----------------------------------------*/
#include <MEN/men_typs.h>    /* system dependent definitions   */
#include <MEN/maccess.h>     /* hw access macros and types     */
#include <MEN/dbg.h>         /* debug functions                */
#include <MEN/oss.h>         /* oss functions                  */
#include <MEN/desc.h>        /* descriptor functions           */
#include <MEN/modcom.h>      /* ID PROM functions              */
#include <MEN/mdis_api.h>    /* MDIS global defs               */
#include <MEN/mdis_com.h>    /* MDIS common defs               */
#include <MEN/mdis_err.h>    /* MDIS error codes               */
#include <MEN/ll_defs.h>     /* low-level driver definitions   */
#include <MEN/arwen.h>       /* definitions for Arwen GPIO     */
#include <MEN/chameleon.h>   /* chameleon header               */

/*-----------------------------------------+
|  DEFINES                                 |
+-----------------------------------------*/
#if (defined(Z127V01) && defined(MDIS_MA_BB_INFO_PTR))
	#define Z127_INFO_PTR 1
#endif

/* general defines */
#define CH_NUMBER          1          /**< number of device channels      */
#ifdef Z127_NOIRQ
  #define USE_IRQ            FALSE      /**< no interrupt required          */
#else
  #define USE_IRQ            TRUE       /**< interrupt required             */
#endif  /* Z127_NOIRQ */

#ifdef Z127_INFO_PTR
	#define ADDRSPACE_COUNT    2          /**< nbr of required address spaces */
	#define ADDRSPACE_SIZE     0x100      /**< size of address space for 16Z127-01 */
#else
	#define ADDRSPACE_COUNT    1          /**< nbr of required address spaces */
	#define ADDRSPACE_SIZE     0x20       /**< size of address space for all other GPIOs */
#endif

/* debug defines */
#define DBG_MYLEVEL        llHdl->dbgLevel    /**< debug level  */
#define DBH                llHdl->dbgHdl      /**< debug handle */
#define OSH                llHdl->osHdl       /**< OS handle    */

/* toggle mode defines */
#define TOG_TIME_DEFAULT   1000       /**< default toggle time [ms]  */
#define TOG_TIME_MIN       100        /**< min toggle time (100ms)  */
#define TOG_TIME_MAX       60000      /**< max toggle time (1min)   */
#define TOG_CYCLIC         1          /**< endless timer flag        */

/* descriptor key defines*/
#define RESET_DEFAULT      0          /**< default arwen reset (enabled)     */
#define RESET_OFF          1          /**< disables the arwen reset function */

/*-----------------------------------------+
|  TYPEDEFS                                |
+-----------------------------------------*/
/** low-level handle */
typedef struct {
	/* general */
	int32                   memAlloc;       /**< size allocated for the handle */
	OSS_HANDLE              *osHdl;         /**< oss handle        */
	OSS_IRQ_HANDLE          *irqHdl;        /**< irq handle        */
	DESC_HANDLE             *descHdl;       /**< desc handle       */
	MACCESS                 ma;             /**< hw access handle  */
#ifdef Z127_INFO_PTR
	CHAMELEONV2_UNIT        *chamu;         /**< chamv2 unit struct */
	u_int32                 vdt;            /**< 1=variable debounce time support */
#endif
	MDIS_IDENT_FUNCT_TBL    idFuncTbl;      /**< id function table */
	/* debug */
	u_int32                 dbgLevel;       /**< debug level  */
	DBG_HANDLE              *dbgHdl;        /**< debug handle */
#ifndef Z127_NOIRQ
	/* misc */
	OSS_SIG_HANDLE          *portChangeSig; /**< signal for port change */
	u_int32                 irqCount;       /**< interrupt counter      */
	u_int32                 lastReq;        /**< last interrupt request */
	/* IRQ latency test */
	u_int32                 irqTest;        /* test running flag (1=on) */
	int32                   outBit;         /* output bit 0..31       */
	u_int32                 outLast;        /* last set output value (1 or 0) */
	u_int32                 irqs2fire;      /* number of IRQs to fire */
	u_int32                 startTick;      /* start tick count       */
	u_int32                 stopTick;       /* stop tick count        */
#endif	/* Z127_NOIRQ */
	/* toggle mode */
	OSS_ALARM_HANDLE        *alarmHdl;      /**< alarm handle               */
	OSS_SEM_HANDLE          *devSemHdl;     /**< device semaphore handle    */
	u_int32                 togHigh;        /**< toggle mode                */
	u_int32                 togtimeHigh;    /**< toggle time high           */
	u_int32                 togtimeLow;     /**< toggle time low            */
	u_int32                 togBitMask;     /**< current toggling gpios     */
	u_int32                 togCount;       /**< counter for timer callback */
	/* arwen no reset */
	u_int32                 arwenResetOff;  /**< disable reset function (1=off) */
} LL_HANDLE;

/* include files which need LL_HANDLE */
#include <MEN/ll_entry.h>       /* low-level driver jump table */
#include <MEN/z17_drv.h>        /* Z17 driver header file      */

/*-----------------------------------------+
|  PROTOTYPES                              |
+-----------------------------------------*/
static int32 Z17_Init(DESC_SPEC *descSpec, OSS_HANDLE *osHdl,
						MACCESS *ma, OSS_SEM_HANDLE *devSemHdl,
						OSS_IRQ_HANDLE *irqHdl, LL_HANDLE **llHdlP);
static int32 Z17_Exit(LL_HANDLE **llHdlP);
static int32 Z17_Read(LL_HANDLE *llHdl, int32 ch, int32 *value);
static int32 Z17_Write(LL_HANDLE *llHdl, int32 ch, int32 value);
static int32 Z17_SetStat(LL_HANDLE *llHdl,int32 ch, int32 code,
							INT32_OR_64 value32_or_64);
static int32 Z17_GetStat(LL_HANDLE *llHdl, int32 ch, int32 code,
							INT32_OR_64 *value32_or64P);
static int32 Z17_BlockRead(LL_HANDLE *llHdl, int32 ch, void *buf, int32 size,
							int32 *nbrRdBytesP);
static int32 Z17_BlockWrite(LL_HANDLE *llHdl, int32 ch, void *buf, int32 size,
								int32 *nbrWrBytesP);
#ifndef Z127_NOIRQ
	static int32 Z17_Irq(LL_HANDLE *llHdl);
#endif	/* Z127_NOIRQ */
static int32 Z17_Info(int32 infoType, ...);
static char* Ident(void);
static int32 Cleanup(LL_HANDLE *llHdl, int32 retCode);
static void  arwenReset(LL_HANDLE *llHdl);

static void  AlarmHandler(void *arg);

/****************************** Z17_GetEntry ********************************/
/** Initialize driver's jump table
 *
 *  \param drvP     \OUT pointer to the initialized jump table structure
 */
#ifdef _ONE_NAMESPACE_PER_DRIVER_
	extern void LL_GetEntry(
		LL_ENTRY* drvP
	)
#else
	extern void __Z17_GetEntry(
		LL_ENTRY* drvP
	)
#endif
{
	drvP->init        = Z17_Init;
	drvP->exit        = Z17_Exit;
	drvP->read        = Z17_Read;
	drvP->write       = Z17_Write;
	drvP->blockRead   = Z17_BlockRead;
	drvP->blockWrite  = Z17_BlockWrite;
	drvP->setStat     = Z17_SetStat;
	drvP->getStat     = Z17_GetStat;
#ifndef Z127_NOIRQ
	drvP->irq         = Z17_Irq;
#endif	/* Z127_NOIRQ */
	drvP->info        = Z17_Info;
}

/******************************** Z17_Init **********************************/
/** Allocate and return low-level handle, initialize hardware
 *
 * The function initializes all channels with the definitions made
 * in the descriptor. The interrupt is disabled.
 *
 * The following descriptor keys are used:
 *
 * \code
 * Descriptor key        Default          Range
 * --------------------  ---------------  -------------
 * DEBUG_LEVEL_DESC      OSS_DBG_DEFAULT  see dbg.h
 * DEBUG_LEVEL           OSS_DBG_DEFAULT  see dbg.h
 * RESET_OFF             0                0..1
 * \endcode
 *
 *  \param descP      \IN  pointer to descriptor data
 *  \param osHdl      \IN  oss handle
 *  \param ma         \IN  hw access handle
 *  \param devSemHdl  \IN  device semaphore handle
 *  \param irqHdl     \IN  irq handle
 *  \param llHdlP     \OUT pointer to low-level driver handle
 *
 *  \return           \c 0 on success or error code
 */
static int32 Z17_Init(
	DESC_SPEC       *descP,
	OSS_HANDLE      *osHdl,
	MACCESS         *ma,
	OSS_SEM_HANDLE  *devSemHdl,
	OSS_IRQ_HANDLE  *irqHdl,
	LL_HANDLE       **llHdlP
)
{
	LL_HANDLE *llHdl = NULL;
	u_int32 gotsize;
	int32 error;
	u_int32 value;

	/*------------------------------+
	|  prepare the handle           |
	+------------------------------*/
	*llHdlP = NULL;		/* set low-level driver handle to NULL */

	/* alloc */
	if ((llHdl = (LL_HANDLE*)OSS_MemGet(
					osHdl, sizeof(LL_HANDLE), &gotsize)) == NULL)
		return (ERR_OSS_MEM_ALLOC);

	/* clear */
	OSS_MemFill(osHdl, gotsize, (char*)llHdl, 0x00);

	/* init */
	llHdl->memAlloc    = gotsize;
	llHdl->osHdl       = osHdl;
#ifndef Z127_NOIRQ
	llHdl->irqHdl      = irqHdl;
#endif	/* Z127_NOIRQ */
	llHdl->ma          = *ma;
	llHdl->devSemHdl   = devSemHdl;
	llHdl->togtimeHigh = TOG_TIME_DEFAULT;
	llHdl->togtimeLow  = TOG_TIME_DEFAULT;
	llHdl->togBitMask  = 0;
	llHdl->togCount    = 0;
	llHdl->togHigh     = 0;

#ifdef Z127_INFO_PTR
	llHdl->chamu       = (CHAMELEONV2_UNIT*)ma[1];
#endif 

	/*------------------------------+
	|  init id function table       |
	+------------------------------*/
	/* driver's ident function */
	llHdl->idFuncTbl.idCall[0].identCall = Ident;
	/* library's ident functions */
	llHdl->idFuncTbl.idCall[1].identCall = DESC_Ident;
	llHdl->idFuncTbl.idCall[2].identCall = OSS_Ident;
	/* terminator */
	llHdl->idFuncTbl.idCall[3].identCall = NULL;

	/*------------------------------+
	|  prepare debugging            |
	+------------------------------*/
	DBG_MYLEVEL = OSS_DBG_DEFAULT;		/* set OS specific debug level */
	DBGINIT((NULL,&DBH));

	/*------------------------------+
	|  scan descriptor              |
	+------------------------------*/
	/* prepare access */
	if ((error = DESC_Init(descP, osHdl, &llHdl->descHdl)))
		return (Cleanup(llHdl, error));

	/* DEBUG_LEVEL_DESC */
	if ((error = DESC_GetUInt32(llHdl->descHdl, OSS_DBG_DEFAULT,
								&value, "DEBUG_LEVEL_DESC")) &&
		error != ERR_DESC_KEY_NOTFOUND)
		return (Cleanup(llHdl, error));

	DESC_DbgLevelSet(llHdl->descHdl, value);	/* set level */

	/* DEBUG_LEVEL */
	if ((error = DESC_GetUInt32(llHdl->descHdl, OSS_DBG_DEFAULT,
								&llHdl->dbgLevel, "DEBUG_LEVEL")) &&
		error != ERR_DESC_KEY_NOTFOUND)
		return (Cleanup(llHdl, error));

	DBGWRT_1((DBH, "LL - Z17_Init: base address = %08p\n", (void*)llHdl->ma));

#ifdef Z127_INFO_PTR
	/* print chameleon info */
	DBGWRT_1(( DBH, "	device ID:\t0x%02x \n",llHdl->chamu->devId));
	DBGWRT_1(( DBH, "	variant:  \t0x%02x \n",llHdl->chamu->variant));
	DBGWRT_1(( DBH, "	revision: \t0x%02x \n\n",llHdl->chamu->revision));
	
	if( llHdl->chamu->variant > 0 )
		llHdl->vdt = 1;
#endif

	/* RESET_OFF */
	if ((error = DESC_GetUInt32(llHdl->descHdl, RESET_DEFAULT,
								&llHdl->arwenResetOff, "RESET_OFF")) &&
		error != ERR_DESC_KEY_NOTFOUND)
		return (Cleanup(llHdl, error));

	/*------------------------------+
	|  init alarm                   |
	+------------------------------*/
	if ((error = OSS_AlarmCreate(llHdl->osHdl, AlarmHandler, llHdl,
									&llHdl->alarmHdl)))
		return (Cleanup(llHdl, error));

	/*------------------------------+
	|  init hardware                |
	+------------------------------*/
	arwenReset(llHdl);
	*llHdlP = llHdl;		/* set low-level driver handle */

	return (ERR_SUCCESS);
}

/****************************** Z17_Exit ************************************/
/** De-initialize hardware and clean up memory
 *
 *  The function deinitializes all channels by setting them as inputs.
 *  The interrupt is disabled.
 *
 *  \param llHdlP     \IN  pointer to low-level driver handle
 *
 *  \return           \c 0 on success or error code
 */
static int32 Z17_Exit(
	LL_HANDLE **llHdlP
)
{
	LL_HANDLE *llHdl = *llHdlP;
	int32 error = 0;

	DBGWRT_1((DBH, "LL - Z17_Exit\n"));

	/*------------------------------+
	|  de-init hardware             |
	+------------------------------*/
	if (llHdl->arwenResetOff == 0)
		arwenReset(llHdl);
	/*------------------------------+
	|  clean up memory              |
	+------------------------------*/
	*llHdlP = NULL;		/* set low-level driver handle to NULL */
	error = Cleanup(llHdl, error);

	return (error);
}

/****************************** Z17_Read ************************************/
/** Read a value from the device
 *
 *  The function reads the current state of all port pins.
 *
 *  \param llHdl      \IN  low-level handle
 *  \param ch         \IN  current channel
 *  \param valueP     \OUT read value
 *
 *  \return           \c 0 on success or error code
 */
static int32 Z17_Read(
	LL_HANDLE  *llHdl,
	int32      ch,
	int32      *valueP
)
{
	DBGWRT_1((DBH, "LL - Z17_Read: ch=%d\n", ch));

	*valueP = MREAD_D32(llHdl->ma, ARWEN_02_PSR);

	return (ERR_SUCCESS);
}

/****************************** Z17_Write ***********************************/
/** Description:  Write a value to the device
 *
 *  The function writes a value to the ports which are programmed as outputs.
 *
 *  \param llHdl      \IN  low-level handle
 *  \param ch         \IN  current channel
 *  \param value      \IN  value to write
 *
 *  \return           \c 0 on success or error code
 */
static int32 Z17_Write(
	LL_HANDLE  *llHdl,
	int32      ch,
	int32      value
)
{
	DBGWRT_1((DBH, "LL - Z17_Write: ch=%d  val=0x%x\n", ch, value));

	MWRITE_D32(llHdl->ma, ARWEN_GPIO, value);

	return (ERR_SUCCESS);
}

/****************************** Z17_SetStat *********************************/
/** Set the driver status
 *
 *  The driver supports \ref getstat_setstat_codes "these status codes"
 *  in addition to the standard codes (see mdis_api.h).
 *  Note: only inputs are able fire an interrupt
 *
 *  \param llHdl         \IN  low-level handle
 *  \param code          \IN  \ref getstat_setstat_codes "status code"
 *  \param ch            \IN  current channel
 *  \param value32_or_64 \IN  data or pointer to block data structure
 *                            (M_SG_BLOCK) for block status codes
 *  \return              \c 0 on success or error code
 */
static int32 Z17_SetStat(
	LL_HANDLE   *llHdl,
	int32       code,
	int32       ch,
	INT32_OR_64 value32_or_64
)
{
	int32 value = (int32)value32_or_64;		/* 32bit value */
#ifndef Z127_NOIRQ
	INT32_OR_64 valueP = value32_or_64;		/* stores 32/64bit pointer */
	M_SG_BLOCK *blk = (M_SG_BLOCK*)valueP;	/* stores block struct pointer */
#endif
	MACCESS ma = llHdl->ma;
	int32 error = ERR_SUCCESS;

	DBGWRT_1((DBH, "LL - Z17_SetStat: ch=%d code=0x%04x value=0x%x\n",
				ch, code, value));

	switch (code) {
		/*--------------------------+
		|  debug level              |
		+--------------------------*/
		case M_LL_DEBUG_LEVEL:
			llHdl->dbgLevel = value;
			break;
		/*--------------------------+
		|  enable interrupts        |
		+--------------------------*/
		case M_MK_IRQ_ENABLE:
			break;
		/*--------------------------+
		|  set irq counter          |
		+--------------------------*/
		case M_LL_IRQ_COUNT:
#ifndef Z127_NOIRQ
			llHdl->irqCount = value;
#endif
			break;
		/*--------------------------+
		|  channel direction        |
		+--------------------------*/
		case M_LL_CH_DIR:
			if (value != M_CH_INOUT) {
				error = ERR_LL_ILL_DIR;
			}
			break;
		/*--------------------------+
		|  set IO ports             |
		+--------------------------*/
		case Z17_SET_PORTS:
			MWRITE_D32(ma, ARWEN_GPIO, MREAD_D32(ma, ARWEN_GPIO) | value);
			/* remove bit from toggle mode */
			if (llHdl->togBitMask) {
				llHdl->togBitMask &= ~value;
				/* stop alarm if last bit is set */
				if (!llHdl->togBitMask)
					error = OSS_AlarmClear(llHdl->osHdl, llHdl->alarmHdl);
			}
			break;
		/*--------------------------+
		|  clear IO ports           |
		+--------------------------*/
		case Z17_CLR_PORTS:
			MWRITE_D32(ma, ARWEN_GPIO, MREAD_D32(ma, ARWEN_GPIO) & ~value);
			/* remove bit from toggle mode */
			if (llHdl->togBitMask) {
				llHdl->togBitMask &= ~value;
				/* stop alarm if last bit is cleared */
				if (!llHdl->togBitMask)
					error = OSS_AlarmClear(llHdl->osHdl, llHdl->alarmHdl);
			}
			break;
		/*--------------------------+
		|  port direction           |
		+--------------------------*/
		case Z17_DIRECTION:
			MWRITE_D32(ma, ARWEN_DDR, value);
			break;
#ifndef Z127_NOIRQ
		/*--------------------------+
		|  edge for interrupt       |
		+--------------------------*/
		case Z17_IRQ_SENSE:
#ifndef Z17_MODEL_Z127
		/* Z34/Z37 - 8 GPIOs */
			MWRITE_D32(ma, ARWEN_02_IER1, value & 0xff);
			MWRITE_D32(ma, ARWEN_02_IER2, (value >> 8) & 0xff);
			break;
#else
		/* Z127: GPIOs 0..15 - two bits per GPIO */
			MWRITE_D32(ma, ARWEN_02_IER1, value);
			break;
		/* Z127: GPIOs 16..31 - two bits per GPIO */
		case Z17_IRQ_SENSE_16TO31:
			MWRITE_D32(ma, ARWEN_02_IER2, value);
			break;
#endif	/* Z17_MODEL_Z127 */
#endif	/* Z127_NOIRQ */
		/*--------------------------+
		|  open drain               |
		+--------------------------*/
		case Z17_OPEN_DRAIN:
			MWRITE_D32(ma, ARWEN_02_ODER, value);
			break;
		/*--------------------------+
		|  debouncer                |
		+--------------------------*/
		case Z17_DEBOUNCE:
			MWRITE_D32(ma, ARWEN_02_DBER, value);
			break;
			
#ifdef Z127_INFO_PTR			
		case Z17_BLK_DEBOUNCE_TIME:
		{
			int8 port;
			Z17_BLK_DEBTIME *dbt = (Z17_BLK_DEBTIME*)blk->data;			
		
			/* prevent access to not implemented DBCR registers */		
			if( llHdl->vdt == 0 ){
				DBGWRT_ERR((DBH, "*** LL - Z17_SetStat(Z17_BLK_DEBOUNCE_TIME): "
					" requires at least 16Z127 variant 1 IP core\n"));
				return ERR_LL_ILL_FUNC;
			}
					
			for( port=0; port<32; port++ ){
				if( (dbt->portMask & (1<<port)) ){
					MWRITE_D32(ma, ARWEN_Z127V01_DBCR(port), dbt->timeUs/50);
				}
			}		

			break;
		}
#endif
					
#ifndef Z127_NOIRQ
		/*--------------------------+
		|  register signal          |
		+--------------------------*/
		case Z17_SET_SIGNAL:
			/* signal already installed ? */
			if (llHdl->portChangeSig) {
				error = ERR_OSS_SIG_SET;
				break;
			}
			error = OSS_SigCreate(OSH, value, &llHdl->portChangeSig);
			break;
		/*--------------------------+
		|  unregister signal        |
		+--------------------------*/
		case Z17_CLR_SIGNAL:
			/* signal already installed ? */
			if (llHdl->portChangeSig == NULL) {
				error = ERR_OSS_SIG_CLR;
				break;
			}
			error = OSS_SigRemove(OSH, &llHdl->portChangeSig);
			break;
		/*--------------------------+
		|  init IRQ latency test    |
		+--------------------------*/
		case Z17_BLK_IRQLAT_START:
		{
			int32 value;
			Z17_BLK_IRQLAT *irqlat = (Z17_BLK_IRQLAT*)blk->data;

			/* clear out values */
			irqlat->startTick = 0;
			irqlat->stopTick = 0;
			irqlat->irqsRcved = 0;

			/* init */
			llHdl->irqTest = 1;
			llHdl->irqs2fire = irqlat->irqs2fire-1;
			llHdl->irqCount = 0;
			llHdl->stopTick = 0;

			/* compute and save output bit */
			llHdl->outBit = 1 << irqlat->outPort;

			/* disable all interrupts */
			MWRITE_D32(ma, ARWEN_02_IER1, 0);
			MWRITE_D32(ma, ARWEN_02_IER2, 0);

			/* program all ports as inputs */
			MWRITE_D32(ma, ARWEN_DDR, 0);

			/* determine actual state of output port */
			value = MREAD_D32(ma, ARWEN_02_PSR);
			llHdl->outLast = (value & llHdl->outBit) ? 1 : 0;

			/* preset output port value */
			MWRITE_D32(ma, ARWEN_GPIO, value);

			/* enable IRQ for rising+falling edge of input port */
/* Z34/Z37 - 8 GPIOs */
#ifndef Z17_MODEL_Z127
			if (irqlat->inPort < 4)	/* port 3..0 */
				MWRITE_D32(ma, ARWEN_02_IER1, 3 << (irqlat->inPort * 2));
			else	/* port 8..4 */
				MWRITE_D32(ma, ARWEN_02_IER2, 3 << ((irqlat->inPort-4) * 2));
/* Z127 - 32 GPIOs */
#else
			if (irqlat->inPort < 16)	/* port 15..0 */
				MWRITE_D32(ma, ARWEN_02_IER1, 3 << (irqlat->inPort * 2));
			else	/* port 32..16 */
				MWRITE_D32(ma, ARWEN_02_IER2, 3 << ((irqlat->inPort-16) * 2));
#endif	/* Z17_MODEL_Z127 */
			/* config port as output (default is input) */
			MWRITE_D32(ma, ARWEN_DDR, llHdl->outBit);

			/* set output (this fires the first IRQ) */
			llHdl->outLast = !llHdl->outLast;
			DBGWRT_3((DBH, "set output=0x%x, irqs2fire=%d\n",
						llHdl->outLast, llHdl->irqs2fire));
			llHdl->startTick = OSS_TickGet(OSH);
			MWRITE_D32(ma, ARWEN_GPIO, llHdl->outLast ? llHdl->outBit : 0);
			break;
		}
#endif	/* Z127_NOIRQ */
		/*--------------------------+
		|  Toggle mode              |
		+--------------------------*/
		case Z17_TOG_PORTS:
		{
			u_int32 realMsec;

			/* start timer initially */
			if (!llHdl->togBitMask) {
				llHdl->togBitMask = value;
				error = OSS_AlarmSet(llHdl->osHdl, llHdl->alarmHdl,
										TOG_TIME_MIN, TOG_CYCLIC, &realMsec);
			} else {
				llHdl->togBitMask |= value;
				/* reset timer */
				error = OSS_AlarmClear(llHdl->osHdl, llHdl->alarmHdl);
				error = OSS_AlarmSet(llHdl->osHdl, llHdl->alarmHdl,
										TOG_TIME_MIN, TOG_CYCLIC, &realMsec);
			}
			break;
		}
		/*--------------------------+
		|  Toggle mode - high       |
		+--------------------------*/
		case Z17_TOG_HIGH:
			/* adjust time value (100ms resolution) */
			llHdl->togtimeHigh = (value / TOG_TIME_MIN) * TOG_TIME_MIN;
			if (llHdl->togtimeHigh > TOG_TIME_MAX)
				llHdl->togtimeHigh = TOG_TIME_MAX;
			else if (llHdl->togtimeHigh < TOG_TIME_MIN)
				llHdl->togtimeHigh = TOG_TIME_MIN;
			break;
		/*--------------------------+
		|  Toggle mode - low        |
		+--------------------------*/
		case Z17_TOG_LOW:
			/* adjust time value (100ms resolution) */
			llHdl->togtimeLow = (value / TOG_TIME_MIN) * TOG_TIME_MIN;
			if (llHdl->togtimeLow > TOG_TIME_MAX)
				llHdl->togtimeLow = TOG_TIME_MAX;
			else if (llHdl->togtimeLow < TOG_TIME_MIN)
				llHdl->togtimeLow = TOG_TIME_MIN;
			break;
		/*--------------------------+
		|  (unknown)                |
		+--------------------------*/
		default:
			error = ERR_LL_UNK_CODE;
	}

	return (error);
}

/****************************** Z17_GetStat *********************************/
/** Get the driver status
 *
 *  The driver supports \ref getstat_setstat_codes "these status codes"
 *  in addition to the standard codes (see mdis_api.h).
 *  Getstat Z17_IRQ_LAST_REQUEST will return the last IRQ request mask
 *  and clear it than.
 *  \param llHdl             \IN  low-level handle
 *  \param code              \IN  \ref getstat_setstat_codes "status code"
 *  \param ch                \IN  current channel
 *  \param value32_or_64P    \IN  pointer to block data structure (M_SG_BLOCK) for
 *                                block status codes
 *  \param value32_or_64P    \OUT data pointer or pointer to block data structure
 *                                (M_SG_BLOCK) for block status codes
 *
 *  \return                  \c 0 on success or error code
 */
static int32 Z17_GetStat(
	LL_HANDLE   *llHdl,
	int32       code,
	int32       ch,
	INT32_OR_64 *value32_or_64P
)
{
	int32 *valueP = (int32*)value32_or_64P;		/* pointer to 32bit value */
	INT32_OR_64 *value64P = value32_or_64P;		/* stores 32/64bit pointer */
	MACCESS ma = llHdl->ma;
	int32 error = ERR_SUCCESS;
#ifndef Z127_NOIRQ
	M_SG_BLOCK *blk = (M_SG_BLOCK*)value32_or_64P;	/* stores block struct pointer; not needed here */
	OSS_IRQ_STATE irqState;
#endif	/* Z127_NOIRQ */

	DBGWRT_1((DBH, "LL - Z17_GetStat: ch=%d code=0x%04x\n", ch, code));

	switch (code) {
		/*--------------------------+
		|  debug level              |
		+--------------------------*/
		case M_LL_DEBUG_LEVEL:
			*valueP = llHdl->dbgLevel;
			break;
		/*--------------------------+
		|  number of channels       |
		+--------------------------*/
		case M_LL_CH_NUMBER:
			*valueP = CH_NUMBER;
			break;
		/*--------------------------+
		|  channel direction        |
		+--------------------------*/
		case M_LL_CH_DIR:
			*valueP = M_CH_INOUT;
			break;
		/*--------------------------+
		|  channel length [bits]    |
		+--------------------------*/
		case M_LL_CH_LEN:
#ifndef Z17_MODEL_Z127
			/* Z34 / Z37 */
			*valueP = 8;
			break;
#else
			/* Z127 */
			*valueP = 32;
			break;
#endif	/* Z17_MODEL_Z127 */
		/*--------------------------+
		|  channel type info        |
		+--------------------------*/
		case M_LL_CH_TYP:
			*valueP = M_CH_BINARY;
			break;
		/*--------------------------+
		|  irq counter              |
		+--------------------------*/
		case M_LL_IRQ_COUNT:
#ifndef Z127_NOIRQ
			*valueP = llHdl->irqCount;
#endif
			break;
		/*--------------------------+
		|  ID PROM check enabled    |
		+--------------------------*/
		case M_LL_ID_CHECK:
			*valueP = 0;
			break;
		/*--------------------------+
		|  ident table pointer      |
		|  (treat as non-block!)    |
		+--------------------------*/
		case M_MK_BLK_REV_ID:
			*value64P = (INT32_OR_64)&llHdl->idFuncTbl;
			break;
		/*--------------------------+
		|  port direction           |
		+--------------------------*/
		case Z17_DIRECTION:
			*valueP = MREAD_D32(ma, ARWEN_DDR);
			break;
#ifndef Z127_NOIRQ
		/*--------------------------+
		|  edge for interrupt       |
		+--------------------------*/
		case Z17_IRQ_SENSE:
#ifndef Z17_MODEL_Z127
			/* Z34/Z37 */
			*valueP = MREAD_D32(ma, ARWEN_02_IER1) |
						MREAD_D32(ma, ARWEN_02_IER2) << 8;
			break;
#else
			/* Z127: GPIOs 0..15 - two bits per GPIO */
			*valueP = MREAD_D32(ma, ARWEN_02_IER1);
			break;
		case Z17_IRQ_SENSE_16TO31:
			/* Z127: GPIOs 16..31 - two bits per GPIO */
			*valueP = MREAD_D32(ma, ARWEN_02_IER2);
			break;
#endif	/* Z17_MODEL_Z127 */
#endif	/* Z127_NOIRQ */
		/*--------------------------+
		|  open drain               |
		+--------------------------*/
		case Z17_OPEN_DRAIN:
			*valueP = MREAD_D32(ma, ARWEN_02_ODER);
			break;
		/*--------------------------+
		|  debouncer                |
		+--------------------------*/
		case Z17_DEBOUNCE:
			*valueP = MREAD_D32(ma, ARWEN_02_DBER);
			break;

#ifdef Z127_INFO_PTR			
		case Z17_BLK_DEBOUNCE_TIME:
		{
			int8 	port, first=0;
			u_int32 timeUs=0;
			Z17_BLK_DEBTIME *dbt = (Z17_BLK_DEBTIME*)blk->data;			
			
			/* prevent access to not implemented DBCR registers */		
			if( llHdl->vdt == 0 ){
				DBGWRT_ERR((DBH, "*** LL - Z17_GetStat(Z17_BLK_DEBOUNCE_TIME): "
					" requires at least 16Z127 variant 1 IP core\n"));
				return ERR_LL_ILL_FUNC;
			}

			if( dbt->portMask == 0 ){
				DBGWRT_ERR((DBH, "*** LL - Z17_GetStat(Z17_BLK_DEBOUNCE_TIME): "
					"no port specified\n"));
				return ERR_LL_ILL_PARAM;
			}
		
			for( port=0; port<32; port++ ){
				if( (dbt->portMask & (1<<port)) ){
					if( first>0 ){
						DBGWRT_ERR((DBH, "*** LL - Z17_GetStat(Z17_BLK_DEBOUNCE_TIME): "
							"more than one port specified\n"));
		                return ERR_LL_ILL_PARAM;
					}
					timeUs = 50 * MREAD_D32(ma, ARWEN_Z127V01_DBCR(port));
					first++;
				}
			}
			
			dbt->timeUs = timeUs;		
			break;
		}
#endif					
			
#ifndef Z127_NOIRQ
		/*--------------------------+
		|  last IRQ request         |
		+--------------------------*/
		case Z17_IRQ_LAST_REQUEST:
			irqState = OSS_IrqMaskR(llHdl->osHdl, llHdl->irqHdl);
			*valueP = llHdl->lastReq;
			llHdl->lastReq = 0;
			OSS_IrqRestore(llHdl->osHdl, llHdl->irqHdl, irqState);
			break;
		/*--------------------------+
		|  IRQ latency test result  |
		+--------------------------*/
		case Z17_BLK_IRQLAT_RESULT:
		{
			Z17_BLK_IRQLAT *irqlat = (Z17_BLK_IRQLAT*)blk->data;

			/* IRQ latency test not started? */
			if (!llHdl->irqTest) {
				error = ERR_LL_DEV_NOTRDY;
				break;
			}
			/* IRQ latency test not yet finished? */
			if (!llHdl->stopTick) {
				error = ERR_LL_DEV_BUSY;
				break;
			}
			llHdl->irqTest = 0;

			/* disable all IRQs */
			MWRITE_D32(ma, ARWEN_02_IER1, 0);
			MWRITE_D32(ma, ARWEN_02_IER2, 0);

			/* reset all outputs */
			MWRITE_D32(ma, ARWEN_GPIO, 0);

			/* program all ports as inputs */
			MWRITE_D32(ma, ARWEN_DDR, 0);

			irqlat->tickRate = OSS_TickRateGet(OSH);
			irqlat->startTick = llHdl->startTick;
			irqlat->stopTick = llHdl->stopTick;
			irqlat->irqsRcved = llHdl->irqCount;
			break;
		}
#endif	/* Z127_NOIRQ */
		/*--------------------------+
		|  (unknown)                |
		+--------------------------*/
		default:
			error = ERR_LL_UNK_CODE;
	}

	return (error);
}

/******************************* Z17_BlockRead ******************************/
/** Read a data block from the device
 *
 *  \param llHdl       \IN  low-level handle
 *  \param ch          \IN  current channel
 *  \param buf         \IN  data buffer
 *  \param size        \IN  data buffer size
 *  \param nbrRdBytesP \OUT number of read bytes
 *
 *  \return            \c 0 on success or error code
 */
static int32 Z17_BlockRead(
	LL_HANDLE *llHdl,
	int32     ch,
	void      *buf,
	int32     size,
	int32     *nbrRdBytesP
)
{
	DBGWRT_1((DBH, "LL - Z17_BlockRead: ch=%d, size=%d\n", ch, size));

	/* return number of read bytes */
	*nbrRdBytesP = 0;

	return (ERR_LL_ILL_FUNC);
}

/****************************** Z17_BlockWrite *****************************/
/** Write a data block from the device
 *
 *  \param llHdl       \IN  low-level handle
 *  \param ch          \IN  current channel
 *  \param buf         \IN  data buffer
 *  \param size        \IN  data buffer size
 *  \param nbrWrBytesP \OUT number of written bytes
 *
 *  \return            \c 0 on success or error code
 */
static int32 Z17_BlockWrite(
	LL_HANDLE *llHdl,
	int32     ch,
	void      *buf,
	int32     size,
	int32     *nbrWrBytesP
)
{
	DBGWRT_1((DBH, "LL - Z17_BlockWrite: ch=%d, size=%d\n", ch, size));

	/* return number of written bytes */
	*nbrWrBytesP = 0;

	return (ERR_LL_ILL_FUNC);
}

#ifndef Z127_NOIRQ
/****************************** Z17_Irq ************************************/
/** Interrupt service routine
 *
 *  The interrupt is triggered when one or more input ports change state.
 *  Depends on current setting via Z16_IRQ_SENSE.
 *  If the unit is requesting an interrupt, this mask will be stored
 *  for getstat Z17_IRQ_LAST_REQUEST.
 *
 *  If the driver can detect the interrupt's cause it returns
 *  LL_IRQ_DEVICE or LL_IRQ_DEV_NOT, otherwise LL_IRQ_UNKNOWN.
 *  
 *  For MSI(x) it is necessary to disable all IRQs and enable them again
 *  at the end of the ISR.
 *
 *  \param llHdl       \IN  low-level handle
 *  \return LL_IRQ_DEVICE   irq caused by device
 *          LL_IRQ_DEV_NOT  irq not caused by device
 *          LL_IRQ_UNKNOWN  unknown
 */
static int32 Z17_Irq(
	LL_HANDLE *llHdl
)
{
	u_int32 irqReq;
	u_int32 ier1, ier2;

	/* interrupt caused by Arwen ? */
	irqReq = MREAD_D32(llHdl->ma, ARWEN_IRR);

	IDBGWRT_1((DBH, ">>> LL - Z17_Irq: request %08x\n", irqReq));
	
	if (irqReq) {

		/* save IERs */
		ier1 = MREAD_D32(llHdl->ma, ARWEN_02_IER1);
		ier2 = MREAD_D32(llHdl->ma, ARWEN_02_IER2);
		/* disable all IRQs (for MSI(x)) */
		MWRITE_D32(llHdl->ma, ARWEN_02_IER1, 0);
		MWRITE_D32(llHdl->ma, ARWEN_02_IER2, 0);

		/* clear interrupt */
		MWRITE_D32(llHdl->ma, ARWEN_IRR, irqReq);
		
		/* if requested send signal to application */
		if (llHdl->portChangeSig)
			OSS_SigSend(OSH, llHdl->portChangeSig);
		
		llHdl->irqCount++;
		llHdl->lastReq = irqReq;
		
		/* IRQ latency test */
		if (llHdl->irqTest) {
			/* pending IRQs to fire? */
			IDBGWRT_3((DBH, ">>> Z17_Irq: irqs2fire=%d\n", llHdl->irqs2fire));
			if (llHdl->irqs2fire) {
				/* set output (this fires the next IRQ) */
				llHdl->outLast = !llHdl->outLast;
				llHdl->irqs2fire--;
				IDBGWRT_3((DBH, ">>> Z17_Irq: set output 0x%x\n", llHdl->outLast));
				MWRITE_D32(llHdl->ma, ARWEN_GPIO, llHdl->outLast ? llHdl->outBit : 0);
			} else {
				/* this was the last IRQ */
				llHdl->stopTick = OSS_TickGet(OSH);
			}
		}/* IRQ latency test */
		
		/* enable all IRQs (for MSI(x)) */
		MWRITE_D32(llHdl->ma, ARWEN_02_IER1, ier1);
		MWRITE_D32(llHdl->ma, ARWEN_02_IER2, ier2);

		return (LL_IRQ_DEVICE);
	}

	return (LL_IRQ_DEV_NOT);
}
#endif	/* Z127_NOIRQ */

/****************************** Z17_Info ***********************************/
/** Get information about hardware and driver requirements
 *
 *  The following info codes are supported:
 *
 * \code
 *  Code                      Description
 *  ------------------------  -----------------------------
 *  LL_INFO_HW_CHARACTER      hardware characteristics
 *  LL_INFO_ADDRSPACE_COUNT   nr of required address spaces
 *  LL_INFO_ADDRSPACE         address space information
 *  LL_INFO_IRQ               interrupt required
 *  LL_INFO_LOCKMODE          process lock mode required
 * \endcode
 *
 *  The LL_INFO_HW_CHARACTER code returns all address and
 *  data modes (ORed) which are supported by the hardware
 *  (MDIS_MAxx, MDIS_MDxx).
 *
 *  The LL_INFO_ADDRSPACE_COUNT code returns the number
 *  of address spaces used by the driver.
 *
 *  The LL_INFO_ADDRSPACE code returns information about one
 *  specific address space (MDIS_MAxx, MDIS_MDxx). The returned
 *  data mode represents the widest hardware access used by
 *  the driver.
 *
 *  The LL_INFO_IRQ code returns whether the driver supports an
 *  interrupt routine (TRUE or FALSE).
 *
 *  The LL_INFO_LOCKMODE code returns which process locking
 *  mode the driver needs (LL_LOCK_xxx).
 *
 *  \param infoType    \IN  info code
 *  \param ...         \IN  argument(s)
 *
 *  \return            \c 0 on success or error code
 */
static int32 Z17_Info(
	int32 infoType,
	...
)
{
	int32   error = ERR_SUCCESS;
	va_list argptr;

	va_start(argptr, infoType);

	switch (infoType) {
		/*-------------------------------+
		|  hardware characteristics      |
		|  (all addr/data modes ORed)    |
		+-------------------------------*/
		case LL_INFO_HW_CHARACTER:
		{
			u_int32 *addrModeP = va_arg(argptr, u_int32*);
			u_int32 *dataModeP = va_arg(argptr, u_int32*);
		
			*addrModeP = MDIS_MA08;
			*dataModeP = MDIS_MD08 | MDIS_MD16;
			break;
		}
		/*-------------------------------+
		|  nr of required address spaces |
		|  (total spaces used)           |
		+-------------------------------*/
		case LL_INFO_ADDRSPACE_COUNT:
		{
			u_int32 *nbrOfAddrSpaceP = va_arg(argptr, u_int32*);
		
			*nbrOfAddrSpaceP = ADDRSPACE_COUNT;
			break;
		}
		/*-------------------------------+
		|  address space type            |
		|  (widest used data mode)       |
		+-------------------------------*/
		case LL_INFO_ADDRSPACE:
		{
			u_int32 addrSpaceIndex = va_arg(argptr, u_int32);
			u_int32 *addrModeP = va_arg(argptr, u_int32*);
			u_int32 *dataModeP = va_arg(argptr, u_int32*);
			u_int32 *addrSizeP = va_arg(argptr, u_int32*);
		
            switch( addrSpaceIndex ){
                case 0:
					*addrModeP = MDIS_MA08;
					*dataModeP = MDIS_MD16;
					*addrSizeP = ADDRSPACE_SIZE;
                    break;
#ifdef Z127_INFO_PTR
                case 1:
                    *addrModeP = MDIS_MA_BB_INFO_PTR;
                    *dataModeP = MDIS_MD_CHAM_0;
                    *addrSizeP = sizeof(CHAMELEONV2_UNIT);
                    break;
#endif

                default:
                    error = ERR_LL_ILL_PARAM;
            }
			break;
		}
		/*-------------------------------+
		|  interrupt required            |
		+-------------------------------*/
		case LL_INFO_IRQ:
		{
			u_int32 *useIrqP = va_arg(argptr, u_int32*);

			*useIrqP = USE_IRQ;
			break;
		}
		/*-------------------------------+
		|  process lock mode             |
		+-------------------------------*/
		case LL_INFO_LOCKMODE:
		{
			u_int32 *lockModeP = va_arg(argptr, u_int32*);

			*lockModeP = LL_LOCK_CALL;
			break;
		}
		/*-------------------------------+
		|  (unknown)                     |
		+-------------------------------*/
		default:
			error = ERR_LL_ILL_PARAM;
	}

	va_end(argptr);

	return (error);
}

/******************************** Ident ************************************/
/** Return ident string
 *
 *  \return            pointer to ident string
 */
static char* Ident(void)
{
	return ("Z17 - Z17 low level driver: $Id: z17_drv.c,v 1.12 2016/07/13 10:09:33 ts Exp $"
			#ifdef Z17_MODEL_Z127
				" Z127 model"
			#else
				" Z34/Z37 model"
			#endif
			#ifdef Z127_NOIRQ
				" - no IRQs"
			#endif
			);
}

/********************************* Cleanup *********************************/
/** Close all handles, free memory and return error code
 *
 *  \warning The low-level handle is invalid after this function is called.
 *
 *  \param llHdl      \IN  low-level handle
 *  \param retCode    \IN  return value
 *
 *  \return           \IN  retCode
 */
static int32 Cleanup(
	LL_HANDLE *llHdl,
	int32     retCode
)
{
	/*------------------------------+
	|  close handles                |
	+------------------------------*/
	/* clean up desc */
	if (llHdl->descHdl)
		DESC_Exit(&llHdl->descHdl);

	/* clean up alarm */
	if (llHdl->alarmHdl)
		OSS_AlarmRemove(llHdl->osHdl, &llHdl->alarmHdl);

	/* clean up debug */
	DBGEXIT((&DBH));

	/*------------------------------+
	|  free memory                  |
	+------------------------------*/
	/* free my handle */
	OSS_MemFree(llHdl->osHdl, (int8*)llHdl, llHdl->memAlloc);

	/*return error code */
	return (retCode);
}

/******************************* arwenReset ********************************/
/** Reset GPIO controller
 *
 *  Sets the controller registers to default values:
 *  - all interrupt disabled
 *  - all ports are inputs
 *  - open drain and debouncing disabled
 *
 *  \param llHdl      \IN  low-level handle
 */
static void arwenReset(
	LL_HANDLE *llHdl
)
{
	MACCESS ma = llHdl->ma;

	/* disable all interrupts */
	MWRITE_D32(ma, ARWEN_02_IER1, 0);
	MWRITE_D32(ma, ARWEN_02_IER2, 0);

	/* program all ports as inputs */
	MWRITE_D32(ma, ARWEN_DDR, 0);

	/* disable open drain and debouncer */
	MWRITE_D32(ma, ARWEN_02_ODER, 0);
	MWRITE_D32(ma, ARWEN_02_DBER, 0);
}

/******************************* AlarmHandler ******************************/
/** Alarm handler
 *
 *  Callback function for toggle mode
 *
 *  \param arg      \IN  low-level handle
 */
static void AlarmHandler(
	void *arg
)
{
	LL_HANDLE *llHdl = (LL_HANDLE*)arg;
	MACCESS ma = llHdl->ma;

	DBGWRT_3((DBH, ">>> LL - Z17_Z127 AlarmHandler:\n"));

	llHdl->togCount += TOG_TIME_MIN;

	/* reset counter */
	if (llHdl->togCount > ((llHdl->togtimeHigh + llHdl->togtimeLow)))
		llHdl->togCount = 0;

	/* toggle high */
	if (llHdl->togHigh && (llHdl->togCount <= (llHdl->togtimeHigh))) {

		/* lock device */
		if (llHdl->devSemHdl)
			OSS_SemWait(llHdl->osHdl, llHdl->devSemHdl, OSS_SEM_WAITFOREVER);

		MWRITE_D32(ma, ARWEN_GPIO, MREAD_D32(ma, ARWEN_GPIO)
									| llHdl->togBitMask);

		/* release device (for other processes) */
		if (llHdl->devSemHdl)
			OSS_SemSignal(llHdl->osHdl, llHdl->devSemHdl);

		/* reset flag */
		llHdl->togHigh = 0;
	}

	/* toggle low */
	if ((!llHdl->togHigh) && (llHdl->togCount > (llHdl->togtimeHigh))) {

		/* lock device */
		if (llHdl->devSemHdl)
			OSS_SemWait(llHdl->osHdl, llHdl->devSemHdl, OSS_SEM_WAITFOREVER);

		MWRITE_D32(ma, ARWEN_GPIO, MREAD_D32(ma, ARWEN_GPIO)
									& ~(llHdl->togBitMask));

		/* release device (for other processes) */
		if (llHdl->devSemHdl)
			OSS_SemSignal(llHdl->osHdl, llHdl->devSemHdl);

		/* reset flag */
		llHdl->togHigh = 1;
	}
}
