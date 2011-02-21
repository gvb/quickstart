/*
    FreeRTOS V6.1.0 - Copyright (C) 2010 Real Time Engineers Ltd.

    ***************************************************************************
    *                                                                         *
    * If you are:                                                             *
    *                                                                         *
    *    + New to FreeRTOS,                                                   *
    *    + Wanting to learn FreeRTOS or multitasking in general quickly       *
    *    + Looking for basic training,                                        *
    *    + Wanting to improve your FreeRTOS skills and productivity           *
    *                                                                         *
    * then take a look at the FreeRTOS books - available as PDF or paperback  *
    *                                                                         *
    *        "Using the FreeRTOS Real Time Kernel - a Practical Guide"        *
    *                  http://www.FreeRTOS.org/Documentation                  *
    *                                                                         *
    * A pdf reference manual is also available.  Both are usually delivered   *
    * to your inbox within 20 minutes to two hours when purchased between 8am *
    * and 8pm GMT (although please allow up to 24 hours in case of            *
    * exceptional circumstances).  Thank you for your support!                *
    *                                                                         *
    ***************************************************************************

    This file is part of the FreeRTOS distribution.

    FreeRTOS is free software; you can redistribute it and/or modify it under
    the terms of the GNU General Public License (version 2) as published by the
    Free Software Foundation AND MODIFIED BY the FreeRTOS exception.
    ***NOTE*** The exception to the GPL is included to allow you to distribute
    a combined work that includes FreeRTOS without being obliged to provide the
    source code for proprietary components outside of the FreeRTOS kernel.
    FreeRTOS is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
    more details. You should have received a copy of the GNU General Public
    License and the FreeRTOS license exception along with FreeRTOS; if not it
    can be viewed here: http://www.freertos.org/a00114.html and also obtained
    by writing to Richard Barry, contact details for whom are available on the
    FreeRTOS WEB site.

    http://www.FreeRTOS.org - Documentation, latest information, license and
    contact details.

    http://www.SafeRTOS.com - A version that is certified for use in safety
    critical systems.

    http://www.OpenRTOS.com - Commercial support, development, porting,
    licensing and training services.
*/
/* Standard includes. */
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/* uip includes. */
#include "hw_types.h"

#include "uip.h"
#include "uip_arp.h"
#include "httpd.h"
#include "timer.h"
#include "clock-arch.h"
#include "hw_ethernet.h"
#include "ethernet.h"
#include "hw_memmap.h"
#include "lmi_flash.h"
#include "sysctl.h"

/* Demo includes. */
#include "emac.h"

#include "partnum.h"
#include "logger.h"
#include "io.h"

/*-----------------------------------------------------------*/

/* How long to wait before attempting to connect the MAC again. */
#define uipINIT_WAIT			100

/* Shortcut to the header within the Rx buffer. */
#define xHeader ((struct uip_eth_hdr *) &uip_buf[ 0 ])

/* Standard constant. */
#define uipTOTAL_FRAME_HEADER_SIZE	54

/*
 * We do a strncmp() against a fixed length string a lot in here.
 * The compiler is good enough to optimize the strlen() and turn it
 * into a constant.
 */
#define STRNCMP(a, b)	strncmp(a, b, strlen(b))

/*-----------------------------------------------------------*/

/*
 * Send the uIP buffer to the MAC.
 */
static void prvENET_Send(void);

/*
 * Setup the MAC address in the MAC itself, and in the uIP stack.
 */
static void prvSetMACAddress( void );

/*
 * Port functions required by the uIP stack.
 */
void clock_init( void );
clock_time_t clock_time( void );

/*-----------------------------------------------------------*/

/* The semaphore used by the ISR to wake the uIP task. */
extern xSemaphoreHandle xEMACSemaphore;

/*-----------------------------------------------------------*/

void clock_init(void)
{
	/* This is done when the scheduler starts. */
}
/*-----------------------------------------------------------*/

clock_time_t clock_time( void )
{
	return xTaskGetTickCount();
}


void vuIP_Task( void *pvParameters )
{
	portBASE_TYPE i;
	uip_ipaddr_t xIPAddr;
	struct timer periodic_timer, arp_timer;
	extern void ( vEMAC_ISR )( void );

	lprintf("Starting vuIP_Task\r\n");

	/* Enable/Reset the Ethernet Controller */
	SysCtlPeripheralEnable( SYSCTL_PERIPH_ETH );
	SysCtlPeripheralReset( SYSCTL_PERIPH_ETH );

	/* Create the semaphore used by the ISR to wake this task. */
	vSemaphoreCreateBinary( xEMACSemaphore );

	/* Initialise the uIP stack. */
	timer_set( &periodic_timer, configTICK_RATE_HZ / 2 );
	timer_set( &arp_timer, configTICK_RATE_HZ * 10 );
	uip_init();
	uip_ipaddr( xIPAddr,
		usercfg.ip[0], usercfg.ip[1], usercfg.ip[2], usercfg.ip[3]);
	uip_sethostaddr( xIPAddr );
	httpd_init();

	while( vInitEMAC() != pdPASS )
	{
		vTaskDelay( uipINIT_WAIT );
	}
	prvSetMACAddress();

	for( ;; )
	{
		/* Is there received data ready to be processed? */
		uip_len = uiGetEMACRxData( uip_buf );

		if( uip_len > 0 )
		{
			/* Standard uIP loop taken from the uIP manual. */

			if( xHeader->type == htons( UIP_ETHTYPE_IP ) )
			{
				uip_arp_ipin();
				uip_input();

				/*
				 * If the above function invocation
				 * resulted in data that should be sent
				 * out on the network, the global variable
				 * uip_len is set to a value > 0.
				 */
				if( uip_len > 0 )
				{
					uip_arp_out();
					prvENET_Send();
				}
			}
			else if( xHeader->type == htons( UIP_ETHTYPE_ARP ) )
			{
				uip_arp_arpin();

				/*
				 * If the above function invocation
				 * resulted in data that should be sent
				 * out on the network, the global variable
				 * uip_len is set to a value > 0.
				 */
				if( uip_len > 0 )
				{
					prvENET_Send();
				}
			}
		}
		else
		{
			if( timer_expired( &periodic_timer ) )
			{
				timer_reset( &periodic_timer );
				for( i = 0; i < UIP_CONNS; i++ )
				{
					uip_periodic( i );

					/*
					 * If the above function
					 * invocation resulted in data
					 * that should be sent out on the
					 * network, the global variable
					 * uip_len is set to a value > 0.
					 */
					if( uip_len > 0 )
					{
						uip_arp_out();
						prvENET_Send();
					}
				}

				/* Call the ARP timer every 10 seconds. */
				if( timer_expired( &arp_timer ) )
				{
					timer_reset( &arp_timer );
					uip_arp_timer();
				}
			}
			else
			{
				/*
				 * We did not receive a packet, and
				 * there was no periodic processing to
				 * perform.  Block for a fixed period.
				 * If a packet is received during this
				 * period we will be woken by the ISR
				 * giving us the Semaphore.
				 */
				xSemaphoreTake( xEMACSemaphore, configTICK_RATE_HZ / 2 );
			}
		}
	}
}
/*-----------------------------------------------------------*/

static void prvENET_Send(void)
{
	vInitialiseSend();
	vIncrementTxLength( uip_len );
	vSendBufferToMAC();
}
/*-----------------------------------------------------------*/

static void prvSetMACAddress( void )
{
	struct uip_eth_addr xAddr;

	/* Program the MAC address. */
	EthernetMACAddrSet(ETH_BASE, permcfg.mac);

	xAddr.addr[0] = permcfg.mac[0];
	xAddr.addr[1] = permcfg.mac[1];
	xAddr.addr[2] = permcfg.mac[2];
	xAddr.addr[3] = permcfg.mac[3];
	xAddr.addr[4] = permcfg.mac[4];
	xAddr.addr[5] = permcfg.mac[5];
	uip_setethaddr( xAddr );
}
/*-----------------------------------------------------------*/

/*
 * Process the button press information.
 */
static void process_button(portCHAR *pcInputString, portBASE_TYPE xInputLength)
{
	char *c;
#ifdef COMMENT_OUT
	enum pwr_sel whichpwr;
#endif // COMMENT_OUT
	enum dio_sel whichdio;

	c = strstr(pcInputString, "?");

	while (c)
	{
		c++;
#ifdef COMMENT_OUT
		whichpwr = strtopwr(c);
		if (whichpwr != pwrInvalid) {
			pwr_ctl(whichpwr, !pwr_state(whichpwr));
			goto next;
		}
#endif // COMMENT_OUT
		whichdio = strtodio(c);
		if (whichdio != dioInvalid) {
			dio_set(whichdio, !dio(whichdio));
			goto next;
		}
		/*
		 * Advance to the next setting, if present.
		 */
next:		c = strstr(c, "&");
	}
}
/*-----------------------------------------------------------*/

/*
 * Process the form input sent by the control.shtml page.
 */
static void process_form_control(portCHAR *pcInputString, portBASE_TYPE xInputLength)
{
	char *c;
	int  which;

	c = strstr(pcInputString, "?");

	while (c)
	{
		c++;	/* move past the "?" or "&" */

		/*
		 * Parse <number>=<string> but we only care about the
		 * number, not the string (which is the button label).
		 */
		if (isdigit(*c)) {
			which = strtol(c, &c, 10);
			/*
		 	* Set the output opposite of its current setting.
		 	*/
#ifdef COMMENT_OUT
			pwr_ctl(which, !pwr_state(which));
#endif // COMMENT_OUT
		}
		/*
		 * Advance to the next setting, if present.
		 */
		c = strstr(c, "&");
	}
}
/*-----------------------------------------------------------*/

/*
 * Copies the string while unescaping HTML encodings.  Returns the length,
 * limited to the max.
 */
static int strncpy_html(char *dp, char *cp, int max)
{
	int  len;

	len = 0;

	while (*cp && (*cp != '&') && len < max) {
		if (*cp == '+') {		/* spaces are encoded as '+' */
			*dp++ = ' ';
			cp++;
		} else if (*cp == '%') {	/* %xx encoding */
			cp++;
			if (isxdigit(cp[0]) && isxdigit(cp[1])) {
				char tmp[4];
				tmp[0] = *cp++;
				tmp[1] = *cp++;
				tmp[2] = '\0';
				*dp++ = strtol(tmp, NULL, 16);
			} else {
				/*
				 * Don't recognize the encoding, just
				 * copy it verbatim.
				 */
				*dp++ = '%';
				len++;
				*dp++ = *cp++;
			}
		} else
			*dp++ = *cp++;
		len++;
	}
	return len;
}
/*-----------------------------------------------------------*/

/*
 * Process the form input sent by the config.shtml page.
 */
static void process_form_config(portCHAR *pcInputString, portBASE_TYPE xInputLength)
{
	char *c;
	int  len;
	int  idx = 0;

	c = strstr(pcInputString, "?");

	while (c)
	{
		c++;	/* move past the "?" or "&" */

		/*
		 * Parse the board part number string, copy it into the
		 * config variable, truncating it and terminating it with
		 * a null in case the user entered string is too long.
		 */
		if (STRNCMP(c, "BDPN=") == 0) {
			c += 5;
			len = strncpy_html(permcfg.bd_pn, c,
				sizeof(permcfg.bd_pn) - 1);
			permcfg.bd_pn[len] = '\0';
			goto next_param;
		}
		/*
		 * Ditto for the board serial number
		 */
		if (STRNCMP(c, "BDSN=") == 0) {
			c += 5;
			len = strncpy_html(permcfg.bd_sn, c,
				sizeof(permcfg.bd_sn) - 1);
			permcfg.bd_sn[len] = '\0';
			goto next_param;
		}
		/*
		 * Ditto for the assembly part number
		 */
		if (STRNCMP(c, "AYPN=") == 0) {
			c += 5;
			len = strncpy_html(usercfg.assy_pn, c,
				sizeof(usercfg.assy_pn) - 1);
			usercfg.assy_pn[len] = '\0';
			goto next_param;
		}
		/*
		 * Ditto for the assembly serial number
		 */
		if (STRNCMP(c, "AYSN=") == 0) {
			c += 5;
			len = strncpy_html(usercfg.assy_sn, c,
				sizeof(usercfg.assy_sn) - 1);
			usercfg.assy_sn[len] = '\0';
			goto next_param;
		}
		/*
		 * Parse the MAC addresses.
		 */
		if (STRNCMP(c, "MAC") == 0) {
			c += 3;
			if (isdigit(*c))
				idx = *c++ - '0';
			if (idx > 5)
				return;
			if (*c++ != '=')
				return;

			if (isxdigit(*c))
				permcfg.mac[idx] = strtol(c, &c, 16) & 0xFF;
			goto next_param;
		}
		/*
		 * Parse the IP addresses.
		 */
		if (STRNCMP(c, "IP") == 0) {
			c += 2;
			if (isdigit(*c))
				idx = *c++ - '0';
			if (idx > 3)
				return;
			if (*c++ != '=')
				return;
			if (isdigit(*c))
				usercfg.ip[idx] = strtol(c, &c, 10) & 0xFF;
			goto next_param;
		}
		/*
		 * Parse the netmask.
		 */
		if (STRNCMP(c, "NM") == 0) {
			c += 2;
			if (isdigit(*c))
				idx = *c++ - '0';
			if (idx > 3)
				return;
			if (*c++ != '=')
				return;
			if (isdigit(*c))
				usercfg.netmask[idx] = strtol(c, &c, 10) & 0xFF;
			goto next_param;
		}
		/*
		 * Parse the gateway.
		 */
		if (STRNCMP(c, "GW") == 0) {
			c += 2;
			if (isdigit(*c))
				idx = *c++ - '0';
			if (idx > 3)
				return;
			if (*c++ != '=')
				return;
			if (isdigit(*c))
				usercfg.gateway[idx] = strtol(c, &c, 10) & 0xFF;
			goto next_param;
		}
		/*
		 * Save the notes field.
		 */
		if (STRNCMP(c, "NOTES=") == 0) {
			c += 6;
			len = strncpy_html(usercfg.notes, c,
				sizeof(usercfg.notes) - 1);
			usercfg.notes[len] = '\0';
			goto next_param;
		}

		/*
		 * Advance to the next setting, if present.
		 */
next_param:
		c = strstr(c, "&");
	}

	if (permcfg_virgin())
		permcfg_save();
	usercfg_save();
}

/*-----------------------------------------------------------*/

/*
 * Figure out which page submitted a form and dispatch to it.
 */
void vApplicationProcessFormInput( portCHAR *pcInputString, portBASE_TYPE xInputLength )
{
	/*
	 * Dispatch to the proper page's handler.
	 */
	if (STRNCMP(pcInputString, "/button.html?") == 0)
		process_button(pcInputString, xInputLength);
	else if (STRNCMP(pcInputString, "/control.shtml?") == 0)
		process_form_control(pcInputString, xInputLength);
	else if (STRNCMP(pcInputString, "/config.shtml?") == 0)
		process_form_config(pcInputString, xInputLength);
	else if (STRNCMP(pcInputString, "/config.shtml?") == 0)
		process_form_config(pcInputString, xInputLength);
}
