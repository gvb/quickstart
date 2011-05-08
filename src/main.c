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

#include <stdint.h>
#include <ustdlib.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "hw_memmap.h"
#include "hw_types.h"
#include "hw_sysctl.h"
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
/*
 * The RIT display font is 5x7 in a 6x8 cell.
 */
#define RITCOL(col)	(col * 6)
#define RITLINE(ln)	(ln * 8)

/****************************************************************************/

static void prvSetupHardware(void);
extern void vSetupHighFrequencyTimer( void );
void ethernetThread(void *pvParameters);

/****************************************************************************/

/**
 * Main function
 *
 * This is the traditional C main().
 */
int main(void)
{
	char s[64];		/* sprintf string */
	unsigned long why;	/* Why did we get reset? Why? */

	prvSetupHardware();
	init_logger();

	config_init();
	io_init();
	util_init();

	/*
	 * Enable interrupts...
	 */
	IntMasterEnable();

	/**
	 * \req \req_tcpip The \program \shall support TCP/IP communications.
	 *
	 * Create the LWIP task if running on a processor that includes a MAC
	 * and PHY.
	 */
	if( SysCtlPeripheralPresent( SYSCTL_PERIPH_ETH ) ) {
		xTaskCreate( ethernetThread,(signed char *)"ethernet", 1000, NULL, 3, NULL);
	}

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
	lprintf("  Assembly Part Number: %s\r\n", usercfg.assy_pn);
	lprintf("Assembly Serial Number: %s\r\n", usercfg.assy_sn);
	lprintf("     Board Part Number: %s\r\n", permcfg.bd_pn);
	lprintf("   Board Serial Number: %s\r\n", permcfg.bd_sn);
	lprintf("                   MAC: %02x:%02x:%02x:%02x:%02x:%02x\r\n",
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

	sprintf(s, "MAC %02x:%02x:%02x:%02x:%02x:%02x", 
		permcfg.mac[0], permcfg.mac[1], permcfg.mac[2],
		permcfg.mac[3], permcfg.mac[4], permcfg.mac[5]);
	RIT128x96x4StringDraw(s, 0, RITLINE(3), 15);

	sprintf(s, "IP  %d.%d.%d.%d\r\n",
		usercfg.ip[0], usercfg.ip[1], usercfg.ip[2], usercfg.ip[3]);
	RIT128x96x4StringDraw(s, 0, RITLINE(4), 15);

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

	vSetupHighFrequencyTimer();
	vTaskStartScheduler();

	/*
	 * Will only get here if there was insufficient memory to create the
	 * idle task.
	 */

	for( ;; );
	return 0;
}

/*****************************************************************************/

/**
 * Initialize the processor hardware.
 *
 * \req \req_init The \program \shall initialize the hardware.
 */
static void prvSetupHardware(void)
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
	SysCtlClockSet(
		SYSCTL_SYSDIV_4
		| SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_8MHZ );

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
	SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);

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

/****************************************************************************/

/**
 * Tick hook.
 *
 * This is called every operating system tick even if the scheduler is
 * not running.
 */


void ethernetThread(void *pvParameters)
{
	IP_CONFIG ipconfig;

	ETHServiceTaskInit(0);
	ETHServiceTaskFlush(0,ETH_FLUSH_RX | ETH_FLUSH_TX);

	ipconfig.IPMode = IPADDR_USE_STATIC;
	ipconfig.IPAddr=0xC0A80064;
	ipconfig.NetMask=0xFFFFFF00;
	ipconfig.GWAddr=0xC0A80001;

	LWIPServiceTaskInit((void *)&ipconfig);

	for(;;)
	{
	}
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
	for( ;; );
}
/** \} */
