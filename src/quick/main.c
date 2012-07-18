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

#include "config.h"
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
#include "interrupt.h"
#include "ethernet.h"

#include "LWIPStack.h"
#include "ETHIsr.h"

#include "partnum.h"
#include "util.h"
#include "logger.h"
#include "io.h"
#include "debugSupport.h"
#include "buildDate.h"

#if (PART == LM3S8962)
#include "inc/lm3s8962.h"
#elif (PART == LM3S9B96)
#include "inc/lm3s9b96.h"
#elif (PART == LM3S2110)
#include "inc/lm3s2110.h"
#endif

#include "syslog.h"

#if (PART == LM3S8962)
#include "drivers/rit128x96x4.h"
#endif

#if USE_PROGRAM_STARTUP
#include "program-startup.h"
#else
void prvSetupHardware(void);

/*
 * The RIT display font is 5x7 in a 6x8 cell.
 */
#define RITCOL(col)	(col * 6)
#define RITLINE(ln)	(ln * 8)

#endif

/****************************************************************************/

extern void vSetupHighFrequencyTimer( void );

#if QUICK_ETHERNET
void ethernetThread(void *pParams);
#endif

/****************************************************************************/

/**
 * Main function
 *
 * This is the traditional C main().
 */
int main(void)
{

#if USE_PROGRAM_STARTUP
	program_startup();
#else
	char s[64];		/* sprintf string */
	unsigned long why;	/* Why did we get reset? Why? */

	prvSetupHardware();
	init_logger();

#if PART == LM3S8962
	RIT128x96x4Init(1000000);
#endif

	/*
	 * \todo maybe this needs to be earlier or later in the code.
	 * Enable fault handlers in addition to FaultIsr()
	 */
	NVIC_SYS_HND_CTRL_R |= NVIC_SYS_HND_CTRL_USAGE
			              |NVIC_SYS_HND_CTRL_BUS
			              |NVIC_SYS_HND_CTRL_MEM;

#if (PART != LM3S2110)
	/*
	 * Allow the following to erase the permanent configuration flash page:
	 *
	 * make PROTECT_PERMCFG="-D PROTECT_PERMCFG=0" \
	 * ERASE_PERMCFG="-D ERASE_PERMCFG=1"
	 *
	 * The program will continue to erase the permanent configuration structure
	 * at every powerup, so the program must be recompiled without the
	 * ERASE_PERMCFG=1 part and reloaded to allow a permanent configuration
	 * record to persist through power cycles.
	 */
#if ERASE_PERMCFG
#if !PROTECT_PERMCFG

	if (permcfg_erase()) {
#if (PART == LM3S8962)
		RIT128x96x4StringDraw("permcfg Blank",
							0, RITLINE(10), 15);
#endif
	}
	else {
#if (PART == LM3S8962)
		RIT128x96x4StringDraw("permcfg Not Blank",
							0, RITLINE(10), 15);
#endif
	}
#endif
#endif

	config_init();
#endif
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
	lstr("\r\nCRI Quickstart\r\n");
#if (PART == LM3S2110)
	lstr("LM3S2110 Eval Board\r\n");
#elif (PART == LM3S8962)
	lstr("LM3S8962 Eval Board\r\n");
#elif (PART == LM3S9B96)
	lstr("LM3S9B96 Eval Board\r\n");
#endif
	lstr("Copyright (C) 2011 Consolidated Resource Imaging\r\n");

	lprintf("   Software Build Date: %s\n", buildDate);
#if (PART != LM3S2110)
	lprintf("  Assembly Part Number: %s\n", usercfg.assy_pn);
	lprintf("Assembly Serial Number: %s\n", usercfg.assy_sn);
	lprintf("     Board Part Number: %s\n", permcfg.bd_pn);
	lprintf("   Board Serial Number: %s\n", permcfg.bd_sn);
	lprintf("Notes:\r\n %s\r\n", usercfg.notes);
#endif
#if (PART == LM3S8962)
	/*
	 * Display our configuration on the OLED display.
	 */
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
#endif

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

#endif

	util_init();

	/**
	 * \req \req_tcpip The \program \shall support TCP/IP communications.
	 *
	 * Create the LWIP task if running on a processor that includes a MAC
	 * and PHY.
	 */

#if QUICK_ETHERNET

	if( SysCtlPeripheralPresent( SYSCTL_PERIPH_ETH ) ) {
		xTaskCreate(ethernetThread,
			    (signed char *)"eth-init",
			    DEFAULT_STACK_SIZE,
			    NULL,
			    ETH_INIT_PRIORITY,
			    NULL);
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

#if !USE_PROGRAM_STARTUP
/*****************************************************************************/
/**
 * Initialize the processor hardware.
 *
 * \req \req_init The \program \shall initialize the hardware.
 */
void prvSetupHardware(void)
{
	/*
	 * If running on Rev A2 silicon, turn the LDO voltage up to 2.75V.
	 * This is a workaround to allow the PLL to operate reliably.
	 */
	if( REVISION_IS_A2 ) {
		SysCtlLDOSet( SYSCTL_LDO_2_75V );
	}

	/**
	 * Set the clocking to run from the PLL at 50 MHz
	 */
#if (PART == LM3S8962)

	SysCtlClockSet(
		SYSCTL_SYSDIV_4
		| SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_8MHZ );

#elif (PART == LM3S9B96)

	SysCtlClockSet(
		SYSCTL_SYSDIV_4
		| SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ );

	/* 80MHz operation, change FreeRTOSConfig.h too */
 /* 	SysCtlClockSet(
		SYSCTL_SYSDIV_2_5
		| SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ );
*/

#elif (PART == LM3S2110)
	//
	// Set the clocking to run directly from the PLL at 25MHz.
	//
	SysCtlClockSet(SYSCTL_SYSDIV_8 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN |
		       SYSCTL_XTAL_8MHZ);
#endif

	/*
	 * Initialize the ARM peripherals that are used.  All the GPIOs
	 * are initialized because the processor I/O page references
	 * all of them.
	 */
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOG);

	SysCtlPeripheralEnable(SYSCTL_PERIPH_WDOG0);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM);

#if (PART != LM3S2110)
	SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
#endif

#if (PART==LM3S8962)
	/*
	 * Configure the GPIOs used to read the on-board buttons.
	 */
	GPIOPinTypeGPIOInput(GPIO_PORTE_BASE,
		GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);
	GPIOPadConfigSet(GPIO_PORTE_BASE,
		GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3,
		GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
	GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_1);
	GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_1, GPIO_STRENGTH_2MA,
		GPIO_PIN_TYPE_STD_WPU);

	/*
	 * Configure the LED and speaker GPIOs.
	 */
	GPIOPinTypePWM(GPIO_PORTG_BASE, GPIO_PIN_1);
	GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_0);
	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, 0);
#endif

	/*
	 * UART0 is our debug ("spew") I/O.  Configure it for 115200,
	 * 8-N-1 operation.
	 */
	GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
	UARTConfigSetExpClk(UART0_BASE, SysCtlClockGet(), 115200,
		( UART_CONFIG_WLEN_8
		| UART_CONFIG_STOP_ONE
		| UART_CONFIG_PAR_NONE ));
}
#endif

/****************************************************************************/

#if QUICK_ETHERNET

/**
 * Start the ethernet
 *
 */


void ethernetThread(void *pParams)
{
	IP_CONFIG ipconfig;
	unsigned char hwaddr[6] = {0, 0, 0, 0, 0, 0};
	char s[64];		/* sprintf string */

	ETHServiceTaskInit(0);
	ETHServiceTaskFlush(0,ETH_FLUSH_RX | ETH_FLUSH_TX);

/*
 * Allow make SET_IP_ADR="-D SET_IP_ADR=\"(192<<24|168<<16|98<<8|29)\""
 * to build application with a recovery IP address.
 */
#ifndef SET_IP_ADR
#define SET_IP_ADR 0
#endif
	if (SET_IP_ADR) {
		ipconfig.IPMode = IPADDR_USE_STATIC;
		ipconfig.IPAddr    = SET_IP_ADR;
#ifndef SET_NET_MASK
#define SET_NET_MASK 0xffffff00
#endif
		ipconfig.NetMask   = SET_NET_MASK;
#ifndef SET_GW_ADR
#define SET_GW_ADR (((SET_IP_ADR)&(SET_NET_MASK))|0x00000001)
#endif
		ipconfig.GWAddr    = SET_GW_ADR;
	} else {
		ipconfig.IPMode = usercfg.IPMode;
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
	}

	LWIPServiceTaskInit(&ipconfig);

	/*
	 * Get actual MAC and IP address programmed
	 */
	EthernetMACAddrGet(ETH_BASE, &hwaddr[0]);
	LWIPServiceTaskIPConfigGet(&lwip_netif, &ipconfig);

	/*
	 * Print Ethernet configuration to serial
	 */
	lprintf("\r\n");
	lprintf("MAC: %02X:%02X:%02X:%02X:%02X:%02X\r\n", hwaddr[0],
		hwaddr[1], hwaddr[2], hwaddr[3], hwaddr[4], hwaddr[5]);

	lprintf(" IP: %d.%d.%d.%d\r\n",
		ipconfig.IPAddr>>0 & 0xff,
		ipconfig.IPAddr>>8 & 0xff,
		ipconfig.IPAddr>>16 & 0xff,
		ipconfig.IPAddr>>24 & 0xff	);

	lprintf("\r\n");

#if (PART == LM3S8962)
	/*
	 * Print Ethernet configuration to OLED screen
	 */
	sprintf(s, "MAC %02X:%02X:%02X:%02X:%02X:%02X", hwaddr[0],
		hwaddr[1], hwaddr[2], hwaddr[3], hwaddr[4], hwaddr[5]);
	RIT128x96x4StringDraw(s, 0, RITLINE(5), 15);

	sprintf(s, "IP  %d.%d.%d.%d",
			ipconfig.IPAddr>>0 & 0xff,
			ipconfig.IPAddr>>8 & 0xff,
			ipconfig.IPAddr>>16 & 0xff,
			ipconfig.IPAddr>>24 & 0xff	);
	RIT128x96x4StringDraw(s, 0, RITLINE(6), 15);
#endif

	syslogInit();
	syslog(facility_local0 , level_err, "A message from QuickStart" );

	/* Nothing else to do.  No point hanging around. */
	vTaskDelete( NULL);

	/* We should not get here. */
	return;
}
#endif
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
