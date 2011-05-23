/**
 * \file main.c
 *
 * Mainline.
 * \addtogroup main Mainline
 * \{
 *//*
 * Copyright (C) 2011 Consolidated Resource Imaging LLC
 *
 *       1         2         3         4         5         6         7        
 *3456789012345678901234567890123456789012345678901234567890123456789012345678
 */

/**
\mainpage Stellaris + FreeRTOS + lwIP (Linux host) Quickstart

\section Overview Overview

The evaluation board is the small, successful program that spawns many
large successful programs.

\section Building from Source

-# Set up an ARM build environment (out of scope for this document).

-# Create build tree

     mkdir quickstart-dev
     cd quickstart-dev

-# Use git to clone the source.

     git clone gitosis@share.cri.us.com:LM3S8962ek.git
     git clone gitosis@share.cri.us.com:StellarisWare.git
     git clone gitosis@share.cri.us.com:FreeRTOS.git
     git clone gitosis@share.cri.us.com:lwip.git
     git clone gitosis@share.cri.us.com:lwip-contrib.git

-# Build

     pushd StellarisWare; make; popd;
     pushd lwip-contrib/ports/cross; make; popd;
     pushd LM3S8962ek; make; popd;

\section See Also

\sa \ref iopage1
\sa \ref utilpage1

\section Rights Proprietary and Confidential

\todo Revise the copyright: use the BSD/MIT license: http://www.opensource.org/licenses/bsd-license


*****************************************************************************/

#include "quickstart-opts.h"

#include <stdint.h>
#include <ustdlib.h>
#include <string.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "hw_memmap.h"
#include "hw_types.h"
#include "hw_sysctl.h"
#include "hw_ints.h"
#include "sysctl.h"
#include "gpio.h"
#include "uart.h"
#include "drivers/rit128x96x4.h"
#include "interrupt.h"

#include "LWIPStack.h"
#include "ETHIsr.h"

#include "config.h"
#include "partnum.h"
#include "util.h"
#include "logger.h"
#include "io.h"
#include "debugSupport.h"
#include "buildDate.h"
#include "inc/lm3s8962.h"

#ifdef USE_PROGRAM_STARTUP
#include "program-startup.h"
#endif

/*
 * The RIT display font is 5x7 in a 6x8 cell.
 */
#define RITCOL(col)	(col * 6)
#define RITLINE(ln)	(ln * 8)

/****************************************************************************/

extern void vSetupHighFrequencyTimer( void );
void ethernetThread(void);

/****************************************************************************/

/**
 * Main function
 *
 * This is the traditional C main().
 */
int main(void)
{
#ifndef USE_PROGRAM_STARTUP
	char s[64];		/* sprintf string */
	unsigned long why;	/* Why did we get reset? Why? */
#endif

	init_logger();


	/*
	 * \todo maybe this needs to be earlier or later in the code.
	 * Enable fault handlers in addition to FaultIsr()
	 */


	NVIC_SYS_HND_CTRL_R |= NVIC_SYS_HND_CTRL_USAGE
			              |NVIC_SYS_HND_CTRL_BUS
			              |NVIC_SYS_HND_CTRL_MEM;

	config_init();

#ifdef USE_PROGRAM_STARTUP

	program-startup();

#else
	/**
	 * \req \req_id The \program \shall identify:
	 * - The program version.
	 * - A copyright string.
	 * - The board identification.
	 * - The assembly identification.
	 * - Network configuration information.
	 *
	 * \todo Issue #1175 Add software build time, git hash, software
	 *    version to build.
	 */
	lprintf("\r\nCRI Quickstart\r\n"
		"LM3S8962 Eval Board\r\n"
		"Copyright (C) 2011 Consolidated Resource Imaging\r\n");
	lprintf("   Software Build Date: %s\n", buildDate);
	lprintf("  Assembly Part Number: %s\n", usercfg.assy_pn);
	lprintf("Assembly Serial Number: %s\n", usercfg.assy_sn);
	lprintf("     Board Part Number: %s\n", permcfg.bd_pn);
	lprintf("   Board Serial Number: %s\n", permcfg.bd_sn);
	lprintf("                   MAC: %02x:%02x:%02x:%02x:%02x:%02x\n",
		permcfg.mac[0], permcfg.mac[1], permcfg.mac[2],
		permcfg.mac[3], permcfg.mac[4], permcfg.mac[5]);
	lprintf("                    IP: %d.%d.%d.%d\r\n",
		usercfg.ip[0], usercfg.ip[1], usercfg.ip[2], usercfg.ip[3]);

	lprintf("Notes:\r\n %s\r\n", usercfg.notes);

	/*
	 * Display our configuration on the OLED display.
	 */
	RIT128x96x4Init(1000000);

	RIT128x96x4StringDraw("CRI Quickstart", 0, RITLINE(0), 15);
	RIT128x96x4StringDraw("LM3S8962", 0, RITLINE(1), 15);

	/*
	 * Split date
	 * 0123456789012345678901234567890
	 * Sun, 08 May 2011 19:05:42 -0400
	 *
	 * into:
	 * 0123456789012345678901234567890
	 * Sun, 08 May 2011
	 *
	 * and
	 *
	 * 0123456789012345678901234567890
	 *                  19:05:42 -0400
	 */
	strcpy(s,buildDate);
	s[16]=0;
	RIT128x96x4StringDraw(s, 0, RITLINE(2), 15);
	RIT128x96x4StringDraw(&s[17], 0, RITLINE(3), 15);

	sprintf(s, "MAC %02x:%02x:%02x:%02x:%02x:%02x", 
		permcfg.mac[0], permcfg.mac[1], permcfg.mac[2],
		permcfg.mac[3], permcfg.mac[4], permcfg.mac[5]);
	RIT128x96x4StringDraw(s, 0, RITLINE(5), 15);

	sprintf(s, "IP  %d.%d.%d.%d\r\n",
		usercfg.ip[0], usercfg.ip[1], usercfg.ip[2], usercfg.ip[3]);
	RIT128x96x4StringDraw(s, 0, RITLINE(6), 15);

	/**
	 * \req \req_id The \program \shall identify:
	 * - The reason for the reset.
	 */
	why = SysCtlResetCauseGet();
	if (why != 0) {
		SysCtlResetCauseClear(why);

		lprintf("Reset reason: ");
		if (why & SYSCTL_CAUSE_LDO)
			lprintf("LDO ");
		if (why & SYSCTL_CAUSE_SW)
			lprintf("SW ");
		if (why & SYSCTL_CAUSE_WDOG)
			lprintf("WDOG ");
		if (why & SYSCTL_CAUSE_BOR)
			lprintf("Brown-out ");
		if (why & SYSCTL_CAUSE_POR)
			lprintf("Power-on ");
		if (why & SYSCTL_CAUSE_EXT)
			lprintf("External ");
		lprintf("\r\n");
	}

	io_init();
	util_init();

#endif


	/**
	 * \req \req_tcpip The \program \shall support TCP/IP communications.
	 *
	 * Create the LWIP task if running on a processor that includes a MAC
	 * and PHY.
	 */

#if 0
	if( SysCtlPeripheralPresent( SYSCTL_PERIPH_ETH ) ) {
		xTaskCreate( ethernetThread,(signed char *)"ethernet",
				5000, NULL, 3, NULL);
	}
#endif

	/*
	 * Enable interrupts...
	 */
	IntMasterEnable();

	vSetupHighFrequencyTimer();
	vTaskStartScheduler();
	DPRINTF(0,"Idle Task Create Failed.");

	/*
	 * Will only get here if there was insufficient memory to create the
	 * idle task.
	 */

	for( ;; );
	return 0;
}

/****************************************************************************/

/**
 * Tick hook.
 *
 * This is called every operating system tick even if the scheduler is
 * not running.
 */


void ethernetThread(void)
{
	IP_CONFIG ipconfig;

	ETHServiceTaskInit(0);
	ETHServiceTaskFlush(0,ETH_FLUSH_RX | ETH_FLUSH_TX);

	ipconfig.IPMode = IPADDR_USE_STATIC;
	ipconfig.IPAddr =
			IP2LONG(usercfg.ip[0],
					usercfg.ip[1],
					usercfg.ip[2],
					usercfg.ip[3]);
	ipconfig.NetMask =
			IP2LONG(usercfg.netmask[0],
					usercfg.netmask[1],
					usercfg.netmask[2],
					usercfg.netmask[3]);
	ipconfig.GWAddr=
			IP2LONG(usercfg.gateway[0],
					usercfg.gateway[1],
					usercfg.gateway[2],
					usercfg.gateway[3]);

	LWIPServiceTaskInit(&ipconfig);

	/* We should not get here. */
	return;
}

/****************************************************************************/

/**
 * Tick hook.
 *
 * This is called every operating system tick even if the scheduler is
 * not running.
 */
void vApplicationTickHook( void )
{
}

/****************************************************************************/

/**
 * Catches a stack overflow error.
 *
 * \req \req_ehandle The \program \shall catch and log errors.
 *
 * \todo Log errors.
 */
void vApplicationStackOverflowHook(
	xTaskHandle *pxTask,
	signed portCHAR *pcTaskName)
{
	lstr("\n\nvApplicationStackOverflowHook:");
	lhex((unsigned int)pxTask);lstr((char*)pcTaskName);crlf();
	for( ;; );
}
/** \} */
