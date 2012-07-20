/**
 * \addtogroup httpd
 * @{
 */

/**
 * \file
 *         Web server script interface
 * \author
 *         Adam Dunkels <adam@sics.se>
 *
 */

/*
 * Copyright (c) 2001-2006, Adam Dunkels.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * This file is part of the uIP TCP/IP stack.
 *
 * $Id: httpd-cgi.c,v 1.2 2006/06/11 21:46:37 adam Exp $
 *
 */


#include <ustdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <lwip/opt.h>
#include <lwip/debug.h>
#include <lwip/stats.h>
#include <LWIPStack.h>
#include <httpd.h>
#include <httpd-cgi.h>

#include <hw_types.h>
#include <hw_memmap.h>
#include <io.h>
#include <gpio.h>

#include <config.h>
#include <partnum.h>

#include <fs.h>
#include <fsdata.h>
#include <../../obj/fsdata-stats.c>
#include <logger.h>
#include <quickstart-opts.h>

#define UIP_APPDATA_SIZE 2048

char uip_appdata[UIP_APPDATA_SIZE];

/*
 * Some things to help make pages.
 */
#define MKBUTTON	"<input type=\"submit\" name=\"%d\" value=\"%d\">"

#define ADCEXPAND(a) \
	adcxlate[a].str, adc(a, raw), \
	adcxlate[a].str, adc(a, millivolts), \
	adcxlate[a].str, adc(a, engineering)

static const char *greenball = "<img src='/green.png' />";
static const char *offball   = "<img src='/off.png' />";
static const char *redball   = "<img src='/red.png' />";

#ifdef NOT_USED_IN_QUICKSTART

/*
 * Split a fixed point number into the integer and fraction parts.
 */
static inline void split_eng(int a, int *integer, int *fract)
{
	*fract = a;
	*integer = *fract / 1000;
	*fract = *fract - (*integer * 1000);
}
#endif

/*---------------------------------------------------------------------------*/

unsigned int refreshCount = 0;
static char cCountBuf[32];

extern void vTaskGetRunTimeStats( signed char *pcWriteBuffer );

int run_time(int index, int iNumParams,
		char *pcParam[], char *pcValue[], char **resultBuffer)
{
	uip_appdata[0] = 0;
	*resultBuffer = uip_appdata;

	//\todo checkfor buffer overrun;
	refreshCount++;
	sprintf( cCountBuf, "<p><br>Refresh count = %u", refreshCount );
	vTaskGetRunTimeStats( (signed char*)uip_appdata );
	strcat( uip_appdata, cCountBuf );

	return strlen( uip_appdata );
}

/*---------------------------------------------------------------------------*/

extern void vTaskList( signed char *pcWriteBuffer );

static int rtos_stats(int index, int iNumParams,
		char *pcParam[], char *pcValue[], char **resultBuffer)
{
	uip_appdata[0] = 0;
	*resultBuffer = uip_appdata;

	//\todo checkfor buffer overrun;
	refreshCount++;
	sprintf( cCountBuf, "<p><br>Refresh count = %u", refreshCount );
	vTaskList( (signed char *)uip_appdata );
	strcat( uip_appdata, cCountBuf );

	return strlen( uip_appdata );
}

/*---------------------------------------------------------------------------*/

int perm_config(int index, int iNumParams,
		char *pcParam[], char *pcValue[], char **resultBuffer)
{
	/*
	 * Allow make PROTECT_PERMCFG="-D PROTECT_PERMCFG=0"
	 * to build application which will overwrite a valid
	 * permanent configuration.
	 */

	int pcvalid = permcfg_valid();
	int pcprot  = PROTECT_PERMCFG && pcvalid;

	*resultBuffer = uip_appdata;

	return snprintf((char *)uip_appdata, UIP_APPDATA_SIZE,
"<tr>"
"<td>Permanent Config Status:</td><td>%s, %s</td>"
"</tr><tr>"
"<td> Part Number: </td><td>"
"	<input type=\"text\" name=\"BDPN\" value=\"%s\" size=\"63\" %s>"
"</tr><tr>"
"<td> Serial Number: </td><td>"
"<form name=\"aForm\" action=\"/config.shtml\" method=\"get\">"
"	<input type=\"text\" name=\"BDSN\" value=\"%s\" size=\"63\" %s>"
"</tr><tr>"
"<td> MAC Address: </td><td>"
"	<input type=\"text\" name=\"MAC0\" value=\"%02x\" size=\"2\" %s>"
"	<input type=\"text\" name=\"MAC1\" value=\"%02x\" size=\"2\" %s>"
"	<input type=\"text\" name=\"MAC2\" value=\"%02x\" size=\"2\" %s>"
"	<input type=\"text\" name=\"MAC3\" value=\"%02x\" size=\"2\" %s>"
"	<input type=\"text\" name=\"MAC4\" value=\"%02x\" size=\"2\" %s>"
"	<input type=\"text\" name=\"MAC5\" value=\"%02x\" size=\"2\" %s>"
"</td>"
"</tr>",
		permcfg_virgin() ? "Unprogrammed" : "Programmed",
		pcvalid ? "valid" : "invalid",

		permcfg.bd_pn, pcprot ? "disabled=\"disabled\"" : "",
		permcfg.bd_sn, pcprot ? "disabled=\"disabled\"" : "",

		permcfg.mac[0], pcprot ? "disabled=\"disabled\"" : "",
		permcfg.mac[1], pcprot ? "disabled=\"disabled\"" : "",
		permcfg.mac[2], pcprot ? "disabled=\"disabled\"" : "",
		permcfg.mac[3], pcprot ? "disabled=\"disabled\"" : "",
		permcfg.mac[4], pcprot ? "disabled=\"disabled\"" : "",
		permcfg.mac[5], pcprot ? "disabled=\"disabled\"" : ""
	);
}

/*---------------------------------------------------------------------------*/

int user_config(int index, int iNumParams,
		char *pcParam[], char *pcValue[], char **resultBuffer)
{
	int ucvalid = usercfg_valid();

	*resultBuffer = uip_appdata;

	return snprintf((char *)uip_appdata, UIP_APPDATA_SIZE,
"<tr>"
"<td>User Config Status:</td><td>%s</td>"
"</tr><tr>"
"<td> Assembly Part Number: </td><td>"
"	<input type=\"text\" name=\"AYPN\" value=\"%s\" size=\"63\">"
"</tr><tr>"
"<td> Assembly Serial Number: </td><td>"
"	<input type=\"text\" name=\"AYSN\" value=\"%s\" size=\"63\">"
"</tr><tr>"
"<td> IP Address: </td><td>"
"	<input type=\"text\" name=\"IP0\" value=\"%d\" size=\"3\">"
"	<input type=\"text\" name=\"IP1\" value=\"%d\" size=\"3\">"
"	<input type=\"text\" name=\"IP2\" value=\"%d\" size=\"3\">"
"	<input type=\"text\" name=\"IP3\" value=\"%d\" size=\"3\"> </td>"
"</tr><tr>"
"<td> Netmask: </td><td>"
"	<input type=\"text\" name=\"NM0\" value=\"%d\" size=\"3\">"
"	<input type=\"text\" name=\"NM1\" value=\"%d\" size=\"3\">"
"	<input type=\"text\" name=\"NM2\" value=\"%d\" size=\"3\">"
"	<input type=\"text\" name=\"NM3\" value=\"%d\" size=\"3\"> </td>"
"</tr><tr>"
"<td> Gateway: </td><td>"
"	<input type=\"text\" name=\"GW0\" value=\"%d\" size=\"3\">"
"	<input type=\"text\" name=\"GW1\" value=\"%d\" size=\"3\">"
"	<input type=\"text\" name=\"GW2\" value=\"%d\" size=\"3\">"
"	<input type=\"text\" name=\"GW3\" value=\"%d\" size=\"3\"> </td>"
"</td></tr><tr>"
"</tr><tr>"
"<td> DHCP is not working, Must Select StaticIP. </td>"
"</tr><tr>"
"<td> StaticIP=0, DHCP=1, AUTOIP=2</td><td>"
"	<input type=\"text\" name=\"IPMD\" value=\"%d\" size=\"1\">"
"</td></tr><tr>"
"<td> Notes: </td><td>"
"	<textarea name=\"NOTES\" rows=\"4\" cols=\"63\">%s</textarea></td>"
"</tr>",
		ucvalid ? "Valid" : "Invalid",
		usercfg.assy_pn,
		usercfg.assy_sn,

		usercfg.ip[0],
		usercfg.ip[1],
		usercfg.ip[2],
		usercfg.ip[3],

		usercfg.netmask[0],
		usercfg.netmask[1],
		usercfg.netmask[2],
		usercfg.netmask[3],

		usercfg.gateway[0],
		usercfg.gateway[1],
		usercfg.gateway[2],
		usercfg.gateway[3],

		usercfg.IPMode,

		usercfg.notes
	);
}

/*---------------------------------------------------------------------------*/

#define STRNCMP(a, b)	strncmp(a, b, strlen(b))

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

/*---------------------------------------------------------------------------*/

/*
 * Process the form input sent by the config.shtml page.
 */
static int config_form(int index, int iNumParams,
		char *pcParam[], char *pcValue[], char **resultBuffer)
{
	char *c;
	int  len;
	int  idx;
	int  i;

	uip_appdata[0]=0;
	*resultBuffer = uip_appdata;

	/*
	 * Parse the board part number string, copy it into the
	 * config variable, truncating it and terminating it with
	 * a null in case the user entered string is too long.
	 */
	for (i=0; i<iNumParams;i++) {
		lstr("<");lhex(i);
		lstr(".");lstr(pcParam[i]);
		lstr(".");lstr(pcValue[i]);lstr(">\n");
		if (strcmp(pcParam[i], "BDPN") == 0) {
			len = strncpy_html(permcfg.bd_pn, pcValue[i],
				sizeof(permcfg.bd_pn) - 1);
			permcfg.bd_pn[len] = '\0';
			lstr("bd_pn=");lstr(permcfg.bd_pn);
			continue;
		}
		/*
		 * Ditto for the board serial number
		 */
		if (STRNCMP(pcParam[i], "BDSN") == 0) {
			len = strncpy_html(permcfg.bd_sn, pcValue[i],
				sizeof(permcfg.bd_sn) - 1);
			permcfg.bd_sn[len] = '\0';
			continue;
		}
		/*
		 * Ditto for the assembly part number
		 */
		if (STRNCMP(pcParam[i], "AYPN=") == 0) {
			len = strncpy_html(usercfg.assy_pn, pcValue[i],
				sizeof(usercfg.assy_pn) - 1);
			usercfg.assy_pn[len] = '\0';
			continue;
		}
		/*
		 * Ditto for the assembly serial number
		 */
		if (STRNCMP(pcParam[i], "AYSN=") == 0) {
			len = strncpy_html(usercfg.assy_sn, pcValue[i],
				sizeof(usercfg.assy_sn) - 1);
			usercfg.assy_sn[len] = '\0';
			continue;
		}
		/*
		 * Parse the MAC addresses.
		 */
		if (STRNCMP(pcParam[i], "MAC") == 0) {
			c = pcParam[i]+ 3;
			if (isdigit(*c))
				idx = *c - '0';
			else
				return 0;
			if (idx > 5)
				return 0;

			if (isxdigit(*pcValue[i]))
				permcfg.mac[idx] = strtol(pcValue[i], NULL, 16) & 0xFF;
			continue;
		}
		/*
		 * Parse the IP Mode (Static, DHCP, AUTOIP).
		 *
		 * "IPMD" must be in front of "IP" so STRNCMP
		 * finds "IPMD" instead of the substring "IP".
		 */
		if (STRNCMP(pcParam[i], "IPMD") == 0) {
			lstr("<IPMD.");lstr(pcValue[i]);
			lstr(".");lhex(*pcValue[i]);
			if (isdigit(*pcValue[i])) {
				unsigned long IPMode = strtol(pcValue[i], NULL, 10) & 0x3;
				lstr(".");lhex(IPMode);
				if ( IPMode > IPADDR_USE_AUTOIP ) {
					lstr(".err>");
					return 0;
				}
				usercfg.IPMode = IPMode;
			}
			lstr(">");
			continue;
		}
		/*
		 * Parse the IP addresses.
		 */
		if (STRNCMP(pcParam[i], "IP") == 0) {
			lstr("IP:");lstr(pcValue[i]);
			c = pcParam[i] + 2;
			if (isdigit(*c))
				idx = *c - '0';
			else
				return 0;
			if (idx > 3)
				return 0;
			if (isdigit(*pcValue[i]))
				usercfg.ip[idx] = strtol(pcValue[i], NULL, 10) & 0xFF;
			continue;
		}
		/*
		 * Parse the netmask.
		 */
		if (STRNCMP(pcParam[i], "NM") == 0) {
			c = pcParam[i] + 2;
			if (isdigit(*c))
				idx = *c - '0';
			else
				return 0;
			if (idx > 3)
				return 0;
			if (isdigit(*pcValue[i]))
				usercfg.netmask[idx] = strtol(pcValue[i], NULL, 10) & 0xFF;
			continue;
		}
		/*
		 * Parse the gateway.
		 */
		if (STRNCMP(pcParam[i], "GW") == 0) {
			c = pcParam[i] + 2;
			if (isdigit(*c))
				idx = *c - '0';
			else
				return 0;
			if (idx > 3)
				return 0;
			if (isdigit(*pcValue[i]))
				usercfg.gateway[idx] = strtol(pcValue[i], NULL, 10) & 0xFF;
			continue;
		}
		/*
		 * Save the notes field.
		 */
		if (STRNCMP(pcParam[i], "NOTES") == 0) {
			len = strncpy_html(usercfg.notes, pcValue[i],
				sizeof(usercfg.notes) - 1);
			usercfg.notes[len] = '\0';
			continue;
		}
	}
/*
 * #define PROTECT_PERMCFG 0
 * to make a build that will overwrite permcfg.
 */
#if PROTECT_PERMCFG
	if (permcfg_virgin())
#endif
	{
		permcfg_save();
	}
	usercfg_save();

	/*
	 * Return a trivial save confirmation page with a button that
	 * links back to the config.shtml page.  Other "tricks" to
	 * return the user to the config page resulted in the browser
	 * not actually re-reading and re-rendering the config page
	 * (permissible behavior even with cache disabled for the pages).
	 */
	return snprintf((char *)uip_appdata, UIP_APPDATA_SIZE,
		"HTTP/1.1 200 OK\r\n"
		"Server: lwIP/CGI (FreeRTOS)\r\n"
		"Content-type: text/html\r\n"
		"Cache-control: no-cache\r\n\r\n"

		"<html>"
		"<head>"
		"<title>Saving Configuration</title>"
//no//		"<meta http-equiv=\"REFRESH\" content=\"1;url=/config.shtml\">"
		"</head>"
		"<body>"
		"<center>"
		"<p>Configuration saved.</p>"
		"<button type=\"button\" OnClick=\"window.location.href = '/config.shtml';\">OK</button>"
		"</center>"
		"</body>"
		"</html>"
	);
}

/*---------------------------------------------------------------------------*/

static int proc_io_upd(int index, int iNumParams,
		char *pcParam[], char *pcValue[], char **resultBuffer)
{
	*resultBuffer = uip_appdata;

	return snprintf((char *)uip_appdata, UIP_APPDATA_SIZE,
		"HTTP/1.1 200 OK\r\n"
		"Server: lwIP/CGI (FreeRTOS)\r\n"
		"Content-type: application/json\r\n"
		"Cache-control: no-cache\r\n\r\n"

		"{"
		/* Port A */
		"\"pA0\": \"%d\""
		",\"pA1\": \"%d\""
		",\"pA2\": \"%d\""
		",\"pA3\": \"%d\""
		",\"pA4\": \"%d\""
		",\"pA5\": \"%d\""
		",\"pA6\": \"%d\""
		",\"pA7\": \"%d\""
		/* Port B */
		",\"pB0\": \"%d\""
		",\"pB1\": \"%d\""
		",\"pB2\": \"%d\""
		",\"pB3\": \"%d\""
		",\"pB4\": \"%d\""
		",\"pB5\": \"%d\""
		",\"pB6\": \"%d\""
		",\"pB7\": \"%d\""
		/* Port C */
		",\"pC0\": \"%d\""
		",\"pC1\": \"%d\""
		",\"pC2\": \"%d\""
		",\"pC3\": \"%d\""
		",\"pC4\": \"%d\""
		",\"pC5\": \"%d\""
		",\"pC6\": \"%d\""
		",\"pC7\": \"%d\""
		/* Port D */
		",\"pD0\": \"%d\""
		",\"pD1\": \"%d\""
		",\"pD2\": \"%d\""
		",\"pD3\": \"%d\""
		",\"pD4\": \"%d\""
		",\"pD5\": \"%d\""
		",\"pD6\": \"%d\""
		",\"pD7\": \"%d\""
		/* Port E */
		",\"pE0\": \"%d\""
		",\"pE1\": \"%d\""
		",\"pE2\": \"%d\""
		",\"pE3\": \"%d\""
		",\"pE4\": \"%d\""
		",\"pE5\": \"%d\""
		",\"pE6\": \"%d\""
		",\"pE7\": \"%d\""
		/* Port F */
		",\"pF0\": \"%d\""
		",\"pF1\": \"%d\""
		",\"pF2\": \"%d\""
		",\"pF3\": \"%d\""
		",\"pF4\": \"%d\""
		",\"pF5\": \"%d\""
		",\"pF6\": \"%d\""
		",\"pF7\": \"%d\""
		/* Port G */
		",\"pG0\": \"%d\""
		",\"pG1\": \"%d\""
		",\"pG2\": \"%d\""
		",\"pG3\": \"%d\""
		",\"pG4\": \"%d\""
		",\"pG5\": \"%d\""
		",\"pG6\": \"%d\""
		",\"pG7\": \"%d\""
		"}",

		GPIOPinRead(GPIO_PORTA_BASE, GPIO_PIN_0) ? 1 : 0,
		GPIOPinRead(GPIO_PORTA_BASE, GPIO_PIN_1) ? 1 : 0,
		GPIOPinRead(GPIO_PORTA_BASE, GPIO_PIN_2) ? 1 : 0,
		GPIOPinRead(GPIO_PORTA_BASE, GPIO_PIN_3) ? 1 : 0,
		GPIOPinRead(GPIO_PORTA_BASE, GPIO_PIN_4) ? 1 : 0,
		GPIOPinRead(GPIO_PORTA_BASE, GPIO_PIN_5) ? 1 : 0,
		GPIOPinRead(GPIO_PORTA_BASE, GPIO_PIN_6) ? 1 : 0,
		GPIOPinRead(GPIO_PORTA_BASE, GPIO_PIN_7) ? 1 : 0,

		GPIOPinRead(GPIO_PORTB_BASE, GPIO_PIN_0) ? 1 : 0,
		GPIOPinRead(GPIO_PORTB_BASE, GPIO_PIN_1) ? 1 : 0,
		GPIOPinRead(GPIO_PORTB_BASE, GPIO_PIN_2) ? 1 : 0,
		GPIOPinRead(GPIO_PORTB_BASE, GPIO_PIN_3) ? 1 : 0,
		GPIOPinRead(GPIO_PORTB_BASE, GPIO_PIN_4) ? 1 : 0,
		GPIOPinRead(GPIO_PORTB_BASE, GPIO_PIN_5) ? 1 : 0,
		GPIOPinRead(GPIO_PORTB_BASE, GPIO_PIN_6) ? 1 : 0,
		GPIOPinRead(GPIO_PORTB_BASE, GPIO_PIN_7) ? 1 : 0,

		GPIOPinRead(GPIO_PORTC_BASE, GPIO_PIN_0) ? 1 : 0,
		GPIOPinRead(GPIO_PORTC_BASE, GPIO_PIN_1) ? 1 : 0,
		GPIOPinRead(GPIO_PORTC_BASE, GPIO_PIN_2) ? 1 : 0,
		GPIOPinRead(GPIO_PORTC_BASE, GPIO_PIN_3) ? 1 : 0,
		GPIOPinRead(GPIO_PORTC_BASE, GPIO_PIN_4) ? 1 : 0,
		GPIOPinRead(GPIO_PORTC_BASE, GPIO_PIN_5) ? 1 : 0,
		GPIOPinRead(GPIO_PORTC_BASE, GPIO_PIN_6) ? 1 : 0,
		GPIOPinRead(GPIO_PORTC_BASE, GPIO_PIN_7) ? 1 : 0,

		GPIOPinRead(GPIO_PORTD_BASE, GPIO_PIN_0) ? 1 : 0,
		GPIOPinRead(GPIO_PORTD_BASE, GPIO_PIN_1) ? 1 : 0,
		GPIOPinRead(GPIO_PORTD_BASE, GPIO_PIN_2) ? 1 : 0,
		GPIOPinRead(GPIO_PORTD_BASE, GPIO_PIN_3) ? 1 : 0,
		GPIOPinRead(GPIO_PORTD_BASE, GPIO_PIN_4) ? 1 : 0,
		GPIOPinRead(GPIO_PORTD_BASE, GPIO_PIN_5) ? 1 : 0,
		GPIOPinRead(GPIO_PORTD_BASE, GPIO_PIN_6) ? 1 : 0,
		GPIOPinRead(GPIO_PORTD_BASE, GPIO_PIN_7) ? 1 : 0,

		GPIOPinRead(GPIO_PORTE_BASE, GPIO_PIN_0) ? 1 : 0,
		GPIOPinRead(GPIO_PORTE_BASE, GPIO_PIN_1) ? 1 : 0,
		GPIOPinRead(GPIO_PORTE_BASE, GPIO_PIN_2) ? 1 : 0,
		GPIOPinRead(GPIO_PORTE_BASE, GPIO_PIN_3) ? 1 : 0,
		GPIOPinRead(GPIO_PORTE_BASE, GPIO_PIN_4) ? 1 : 0,
		GPIOPinRead(GPIO_PORTE_BASE, GPIO_PIN_5) ? 1 : 0,
		GPIOPinRead(GPIO_PORTE_BASE, GPIO_PIN_6) ? 1 : 0,
		GPIOPinRead(GPIO_PORTE_BASE, GPIO_PIN_7) ? 1 : 0,

		GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_0) ? 1 : 0,
		GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_1) ? 1 : 0,
		GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_2) ? 1 : 0,
		GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_3) ? 1 : 0,
		GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_4) ? 1 : 0,
		GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_5) ? 1 : 0,
		GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_6) ? 1 : 0,
		GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_7) ? 1 : 0,

		GPIOPinRead(GPIO_PORTG_BASE, GPIO_PIN_0) ? 1 : 0,
		GPIOPinRead(GPIO_PORTG_BASE, GPIO_PIN_1) ? 1 : 0,
		GPIOPinRead(GPIO_PORTG_BASE, GPIO_PIN_2) ? 1 : 0,
		GPIOPinRead(GPIO_PORTG_BASE, GPIO_PIN_3) ? 1 : 0,
		GPIOPinRead(GPIO_PORTG_BASE, GPIO_PIN_4) ? 1 : 0,
		GPIOPinRead(GPIO_PORTG_BASE, GPIO_PIN_5) ? 1 : 0,
		GPIOPinRead(GPIO_PORTG_BASE, GPIO_PIN_6) ? 1 : 0,
		GPIOPinRead(GPIO_PORTG_BASE, GPIO_PIN_7) ? 1 : 0
	);
}

/*---------------------------------------------------------------------------*/

static int control_upd(int index, int iNumParams,
		char *pcParam[], char *pcValue[], char **resultBuffer)
{
	*resultBuffer = uip_appdata;

	return snprintf((char *)uip_appdata, UIP_APPDATA_SIZE,
		"HTTP/1.1 200 OK\r\n"
		"Server: lwIP/CGI (FreeRTOS)\r\n"
		"Content-type: application/json\r\n"
		"Cache-control: no-cache\r\n\r\n"

		"{"
		"\"%s\": \"%s\""
		",\"%s\": \"%s\""
		",\"%s\": \"%s\""
		",\"%s\": \"%s\""
		",\"%s\": \"%s\""
		",\"%s\": \"%s\""
		",\"%s\": \"%d\"," "\"%smV\": \"%d\""
		",\"%s\": \"%d\"," "\"%smV\": \"%d\""
		",\"%s\": \"%d\"," "\"%smV\": \"%d\""
		",\"%s\": \"%d\"," "\"%smV\": \"%d\""
		",\"%s\": \"%d\"," "\"%sEng\": \"%d\""
		"}",

		dioxlate[dioUp].str, dio(dioUp) ? offball : greenball,
		dioxlate[dioDown].str, dio(dioDown) ? offball : greenball,
		dioxlate[dioLeft].str, dio(dioLeft) ? offball : greenball,
		dioxlate[dioRight].str, dio(dioRight) ? offball : greenball,
		dioxlate[dioSelect].str, dio(dioSelect) ? offball : greenball,
		dioxlate[dioLed0].str, dio(dioLed0) ? greenball : offball,

		adcxlate[adcProc0].str,
			adc(adcProc0, raw),
			adcxlate[adcProc0].str,
			adc(adcProc0, millivolts),
		adcxlate[adcProc1].str,
			adc(adcProc1, raw),
			adcxlate[adcProc1].str,
			adc(adcProc1, millivolts),
		adcxlate[adcProc2].str,
			adc(adcProc2, raw),
			adcxlate[adcProc2].str,
			adc(adcProc2, millivolts),
		adcxlate[adcProc3].str,
			adc(adcProc3, raw),
			adcxlate[adcProc3].str,
			adc(adcProc3, millivolts),
		adcxlate[adcProcTemp].str,
			adc(adcProcTemp, raw),
			adcxlate[adcProcTemp].str,
			adc(adcProcTemp, engineering) / 1000
	);
}

/*---------------------------------------------------------------------------*/

static int button(int index, int iNumParams,
		char *pcParam[], char *pcValue[], char **resultBuffer)
{
	int j;

	enum dio_sel whichdio;

	uip_appdata[0]=0;
	*resultBuffer = uip_appdata;

	/*
	 * Parse the button string.
	 */
	for (j = 0; j < iNumParams; j++) {
		whichdio = strtodio(pcParam[j]);
		if (whichdio != dioInvalid)
			dio_set(whichdio, !dio(whichdio));
	}
	return 0;
}

/*---------------------------------------------------------------------------*/

static const tCGI ssi_cgi_funcs[] = {

		{ "/rtos_stats", rtos_stats },
		{ "/run_time", run_time },

		/* Configuration */
		{ "/perm_config", perm_config },
		{ "/user_config", user_config },
		{ "/config_form", config_form },

		/* AJAX page updates */
		{ "/control_upd", control_upd },
		{ "/proc_io_upd", proc_io_upd },

		/* Button press reports */
		{ "/button", button },
};
#define NUM_SSI_CGI_FUNCTIONS (sizeof(ssi_cgi_funcs) / sizeof(ssi_cgi_funcs[0]))
#define NUM_SSI_CGI_ENTRIES (NUM_SSI_CGI_FUNCTIONS+FS_NUMFILES)

static tCGI calls[NUM_SSI_CGI_ENTRIES];


//*****************************************************************************
//
//! Handle a server side include.
//! \param TBD
//!
//! This function
//!
//! \return Number of characters copied to pcInsert.
//
//*****************************************************************************

int SSIHandler(int iIndex, int iNumParams,
		char *pcParam[], char *pcValue[], char **resultBuffer)
{
	//lstr("<SSI.");lhex(iIndex);lstr(">");
	if (iIndex<NUM_SSI_CGI_FUNCTIONS) {
		return calls[iIndex].pfnCGIHandler(iIndex, iNumParams,
				pcParam, pcValue, resultBuffer);
	}
	else if (iIndex<NUM_SSI_CGI_ENTRIES) {
			struct fs_file *fs;

			fs = fs_open_get_access((char *)calls[iIndex].pcCGIName);
			if (fs) {
				/*
				 * A read of the file without a call to read
				 */
				int len = fs->len;
				*resultBuffer = fs->data;
				fs_close(fs);
				return len;
			}
	}

	*resultBuffer = "";
	return 0;
}

void init_ssi_cgi_handlers(void)
{
	int i;
	const struct fsdata_file *f;

	for(i=0;i<NUM_SSI_CGI_FUNCTIONS;i++){
		calls[i].pcCGIName     = ssi_cgi_funcs[i].pcCGIName;
		calls[i].pfnCGIHandler = ssi_cgi_funcs[i].pfnCGIHandler;
	}

	/*
	 * Append the SSI File names to the calls list.
	 */
	f = FS_ROOT;
	for(i=NUM_SSI_CGI_FUNCTIONS;i<NUM_SSI_CGI_ENTRIES;i++){
		LWIP_ASSERT("(f)", (f));
		if (f) {
			calls[i].pcCGIName     = (char *)f->name;
			calls[i].pfnCGIHandler = NULL;
			f = f->next;
		}
		else {
			calls[i].pcCGIName = "";
			calls[i].pfnCGIHandler = NULL;
		}
	}

	// \to combine ssi_handlers and cgi_handlers
	http_set_ssi_handler(SSIHandler, calls, NUM_SSI_CGI_ENTRIES);
	http_set_cgi_handlers(calls, NUM_SSI_CGI_FUNCTIONS);
}

/** @} */
