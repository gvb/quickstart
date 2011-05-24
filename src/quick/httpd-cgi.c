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
#include <string.h>

#include <lwip/opt.h>
#include <lwip/debug.h>
#include <lwip/stats.h>
#include <httpd.h>
#include <httpd-cgi.h>

#include <config.h>
#include <io.h>
#include <partnum.h>

#include <fs.h>
#include <fsdata.h>
#include <fsdata-stats.h>
#include <logger.h>

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

static inline void split_eng(int a, int *integer, int *fract)
{
	*fract = a;
	*integer = *fract / 1000;
	*fract = *fract - (*integer * 1000);
}

/*---------------------------------------------------------------------------*/
static int file_stats(int index, int iNumParams,
		char *pcParam[], char *pcValue[], char **resultBuffer)
{
#ifdef NEED_TO_CONVERT_FROM_UIP_TO_LWIP
	uip_appdata[0]=0;
	*resultBuffer = uip_appdata;
	if (iNumParams>1) {
		return snprintf((char *)uip_appdata, UIP_APPDATA_SIZE, "%5u", httpd_fs_count(pcValue[0]));
	} else {
		return 0;
	}
#else
	*resultBuffer = "Need to port tcp_stats over to lwIP." ;
	return strlen(*resultBuffer);
#endif
}
/*---------------------------------------------------------------------------*/
static const char closed[] =   /*  "CLOSED",*/
{0x43, 0x4c, 0x4f, 0x53, 0x45, 0x44, 0};
static const char syn_rcvd[] = /*  "SYN-RCVD",*/
{0x53, 0x59, 0x4e, 0x2d, 0x52, 0x43, 0x56,
 0x44,  0};
static const char syn_sent[] = /*  "SYN-SENT",*/
{0x53, 0x59, 0x4e, 0x2d, 0x53, 0x45, 0x4e,
 0x54,  0};
static const char established[] = /*  "ESTABLISHED",*/
{0x45, 0x53, 0x54, 0x41, 0x42, 0x4c, 0x49, 0x53, 0x48,
 0x45, 0x44, 0};
static const char fin_wait_1[] = /*  "FIN-WAIT-1",*/
{0x46, 0x49, 0x4e, 0x2d, 0x57, 0x41, 0x49,
 0x54, 0x2d, 0x31, 0};
static const char fin_wait_2[] = /*  "FIN-WAIT-2",*/
{0x46, 0x49, 0x4e, 0x2d, 0x57, 0x41, 0x49,
 0x54, 0x2d, 0x32, 0};
static const char closing[] = /*  "CLOSING",*/
{0x43, 0x4c, 0x4f, 0x53, 0x49,
 0x4e, 0x47, 0};
static const char time_wait[] = /*  "TIME-WAIT,"*/
{0x54, 0x49, 0x4d, 0x45, 0x2d, 0x57, 0x41,
 0x49, 0x54, 0};
static const char last_ack[] = /*  "LAST-ACK"*/
{0x4c, 0x41, 0x53, 0x54, 0x2d, 0x41, 0x43,
 0x4b, 0};

static const char *states[] = {
  closed,
  syn_rcvd,
  syn_sent,
  established,
  fin_wait_1,
  fin_wait_2,
  closing,
  time_wait,
  last_ack};


static int
generate_tcp_stats(void *arg)
{
  struct uip_conn *conn;
  struct httpd_state *s = (struct httpd_state *)arg;

  return snprintf((char *)uip_appdata, UIP_APPDATA_SIZE, "Need to fix up.");

#ifdef NEED_TO_CONVERT_FROM_UIP_TO_LWIP

  conn = &uip_conns[s->count];
  return snprintf((char *)uip_appdata, UIP_APPDATA_SIZE,
		 "<tr><td>%d</td><td>%u.%u.%u.%u:%u</td><td>%s</td><td>%u</td><td>%u</td><td>%c %c</td></tr>\r\n",
		 htons(conn->lport),
		 htons(conn->ripaddr[0]) >> 8,
		 htons(conn->ripaddr[0]) & 0xff,
		 htons(conn->ripaddr[1]) >> 8,
		 htons(conn->ripaddr[1]) & 0xff,
		 htons(conn->rport),
		 states[conn->tcpstateflags & UIP_TS_MASK],
		 conn->nrtx,
		 conn->timer,
		 (uip_outstanding(conn))? '*':' ',
		 (uip_stopped(conn))? '!':' ');
#endif
}
/*---------------------------------------------------------------------------*/
static int tcp_stats(int index, int iNumParams,
		char *pcParam[], char *pcValue[], char **resultBuffer)
{
	*resultBuffer = "Need to port tcp_stats over to lwIP." ;
	return strlen(*resultBuffer);
}
/*---------------------------------------------------------------------------*/
#ifdef NEED_TO_CONVERT_FROM_UIP_TO_LWIP
static unsigned short
generate_net_stats(void *arg)
{
  struct httpd_state *s = (struct httpd_state *)arg;
  return snprintf((char *)uip_appdata, UIP_APPDATA_SIZE,
		  "%5u\n", ((uip_stats_t *)&uip_stat)[s->count]);
}
#endif

static int net_stats(int index, int iNumParams,
		char *pcParam[], char *pcValue[], char **resultBuffer)
{
	*resultBuffer = "Need to port net_stats over to lwIP." ;
	return strlen(*resultBuffer);

}
/*---------------------------------------------------------------------------*/

extern void vTaskList( signed char *pcWriteBuffer );
static char cCountBuf[ 32 ];
long lRefreshCount = 0;
static int
generate_rtos_stats(void)
{
	//\todo checkfor buffer overrun;
	lRefreshCount++;
	sprintf( cCountBuf, "<p><br>Refresh count = %ld", lRefreshCount );
	vTaskList( (signed char *)uip_appdata );
	strcat( uip_appdata, cCountBuf );

	return strlen( uip_appdata );
}
/*---------------------------------------------------------------------------*/

extern void vTaskGetRunTimeStats( signed char *pcWriteBuffer );
static int
generate_runtime_stats(void)
{
	//\todo checkfor buffer overrun;
	lRefreshCount++;
	sprintf( cCountBuf, "<p><br>Refresh count = %ld", lRefreshCount );
	vTaskGetRunTimeStats( (signed char*)uip_appdata );
	strcat( uip_appdata, cCountBuf );

	return strlen( uip_appdata );
}
/*---------------------------------------------------------------------------*/

int run_time(int index, int iNumParams,
		char *pcParam[], char *pcValue[], char **resultBuffer)
{

	uip_appdata[0]=0;
	*resultBuffer = uip_appdata;

	return generate_runtime_stats();
}
/*---------------------------------------------------------------------------*/

static int rtos_stats(int index, int iNumParams,
		char *pcParam[], char *pcValue[], char **resultBuffer)
{
	uip_appdata[0]=0;
	*resultBuffer = uip_appdata;

	return generate_rtos_stats();
}
/*---------------------------------------------------------------------------*/

int perm_config(int index, int iNumParams,
		char *pcParam[], char *pcValue[], char **resultBuffer)
{

	int pcvalid = permcfg_valid();

	uip_appdata[0]=0;
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

		permcfg.bd_pn, pcvalid ? "disabled=\"disabled\"" : "",
		permcfg.bd_sn, pcvalid ? "disabled=\"disabled\"" : "",

		permcfg.mac[0], pcvalid ? "disabled=\"disabled\"" : "",
		permcfg.mac[1], pcvalid ? "disabled=\"disabled\"" : "",
		permcfg.mac[2], pcvalid ? "disabled=\"disabled\"" : "",
		permcfg.mac[3], pcvalid ? "disabled=\"disabled\"" : "",
		permcfg.mac[4], pcvalid ? "disabled=\"disabled\"" : "",
		permcfg.mac[5], pcvalid ? "disabled=\"disabled\"" : ""
	);
}
/*---------------------------------------------------------------------------*/

int user_config(int index, int iNumParams,
		char *pcParam[], char *pcValue[], char **resultBuffer)
{
	uip_appdata[0]=0;
	*resultBuffer = uip_appdata;

	return snprintf((char *)uip_appdata, UIP_APPDATA_SIZE,
"<tr>"
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
"<td> Notes: </td><td>"
"	<textarea name=\"NOTES\" rows=\"4\" cols=\"63\">%s</textarea></td>"
"</tr>",
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

		usercfg.notes
	);
}

/*
 * Process the form input sent by the config.shtml page.
 */
// static void process_form_config(portCHAR *pcInputString, portBASE_TYPE xInputLength)

static int process_form_config(int index, int iNumParams,
		char *pcParam[], char *pcValue[], char **resultBuffer)
{
	char *c;
	int  len;
	int  idx;

	/*
	 * Parse the board part number string, copy it into the
	 * config variable, truncating it and terminating it with
	 * a null in case the user entered string is too long.
	 */
/*	for (idx=0; idx<iNumParams;i++) {
		if (strcmp(pcParam[idx], "BDPN=") == 0) {
			c += 5;
			len = strncpy_html(permcfg.bd_pn, c,
				sizeof(permcfg.bd_pn) - 1);
			permcfg.bd_pn[len] = '\0';
			goto next_param;
		} */
		/*
		 * Ditto for the board serial number
		 */
/*		if (STRNCMP(c, "BDSN=") == 0) {
			c += 5;
			len = strncpy_html(permcfg.bd_sn, c,
				sizeof(permcfg.bd_sn) - 1);
			permcfg.bd_sn[len] = '\0';
			goto next_param;
		} */
		/*
		 * Ditto for the assembly part number
		 */
/*		if (STRNCMP(c, "AYPN=") == 0) {
			c += 5;
			len = strncpy_html(usercfg.assy_pn, c,
				sizeof(usercfg.assy_pn) - 1);
			usercfg.assy_pn[len] = '\0';
			goto next_param;
		} */
		/*
		 * Ditto for the assembly serial number
		 */
/*		if (STRNCMP(c, "AYSN=") == 0) {
			c += 5;
			len = strncpy_html(usercfg.assy_sn, c,
				sizeof(usercfg.assy_sn) - 1);
			usercfg.assy_sn[len] = '\0';
			goto next_param;
		}*/
		/*
		 * Parse the MAC addresses.
		 */
/*		if (STRNCMP(c, "MAC") == 0) {
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
		}*/
		/*
		 * Parse the IP addresses.
		 */
/*		if (STRNCMP(c, "IP") == 0) {
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
		} */
		/*
		 * Parse the netmask.
		 */
/*		if (STRNCMP(c, "NM") == 0) {
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
		} */
		/*
		 * Parse the gateway.
		 */
/*		if (STRNCMP(c, "GW") == 0) {
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
		} */
		/*
		 * Save the notes field.
		 */
/*		if (STRNCMP(c, "NOTES=") == 0) {
			c += 6;
			len = strncpy_html(usercfg.notes, c,
				sizeof(usercfg.notes) - 1);
			usercfg.notes[len] = '\0';
			goto next_param;
		}*/

		/*
		 * Advance to the next setting, if present.
		 */
/*next_param:
		c = strstr(c, "&");

	}
	c = strstr(pcInputString, "?");

	while (c)
	{
		c++;	*//* move past the "?" or "&" */
/*
	}

	if (permcfg_virgin())
		permcfg_save();
	usercfg_save(); */
	return 0;
}


/*---------------------------------------------------------------------------*/
#if LRU_IS_GCU
static int exp1_upd(int index, int iNumParams,
		char *pcParam[], char *pcValue[], char **resultBuffer)
{
	uip_appdata[0]=0;
	*resultBuffer = uip_appdata;

	return snprintf((char *)uip_appdata, UIP_APPDATA_SIZE,
		"\"%s\": \"%s\","
		"\"%s\": \"%s\","
		"\"%s\": \"%s\","
		"\"%s\": \"%s\","
		"\"%s\": \"%s\","
		"\"%s\": \"%s\","
		"\"%s\": \"%s\","
		"\"%s\": \"%s\","
		"\"%s\": \"%s\","
		"\"%s\": \"%s\","
		"\"%s\": \"%s\","
		"\"%s\": \"%s\","
		"\"%s\": \"%s\","
		"\"%s\": \"%s\","
		"\"%s\": \"%s\","
		"\"%s\": \"%s\",",
		dioxlate[dioGpsReset_].str,
			dio(dioGpsReset_) ? offball : greenball,
		dioxlate[dioCpuLed1].str,
			dio(dioCpuLed1) ? greenball : offball,
		dioxlate[dioCpuLed2].str,
			dio(dioCpuLed2) ? greenball : offball,
		dioxlate[dioArmPc4].str,
			dio(dioArmPc4) ? greenball : offball,
		dioxlate[dioArmPc6].str,
			dio(dioArmPc6) ? greenball : offball,
		dioxlate[dioArmProc].str,
			dio(dioArmProc) ? greenball : offball,
		dioxlate[dioGpsPosValid].str,
			dio(dioGpsPosValid) ? greenball : offball,
		dioxlate[dioOpenGnd2IsOn].str,
			dio(dioOpenGnd2IsOn) ? greenball : offball,
		dioxlate[dioVicor3xBInputPower].str,
			dio(dioVicor3xBInputPower) ? greenball : offball,
		dioxlate[dioVicor3xCInputPower].str,
			dio(dioVicor3xCInputPower) ? greenball : offball,
		dioxlate[dioUnused1].str,
			dio(dioUnused1) ? greenball : offball,
		dioxlate[dioVicor1xStatus].str,
			dio(dioVicor1xStatus) ? greenball : offball,
		dioxlate[dioVicorSpareEnable].str,
			dio(dioVicorSpareEnable) ? greenball : offball,
		dioxlate[dioVicorSpareStatus].str,
			dio(dioVicorSpareStatus) ? greenball : offball,
		dioxlate[dioVicor1xInputPower].str,
			dio(dioVicor1xInputPower) ? greenball : offball,
		dioxlate[dioVicor3xAInputPower].str,
			dio(dioVicor3xAInputPower) ? greenball : offball
	);
}
/*---------------------------------------------------------------------------*/

static int exp2_upd(int index, int iNumParams,
		char *pcParam[], char *pcValue[], char **resultBuffer)
{
	uip_appdata[0]=0;
	*resultBuffer = uip_appdata;

	return snprintf((char *)uip_appdata, UIP_APPDATA_SIZE,
		"\"%s\": \"%s\","
		"\"%s\": \"%s\","
		"\"%s\": \"%s\","
		"\"%s\": \"%s\","
		"\"%s\": \"%s\","
		"\"%s\": \"%s\","
		"\"%s\": \"%s\","
		"\"%s\": \"%s\","
		"\"%s\": \"%s\","
		"\"%s\": \"%s\","
		"\"%s\": \"%s\","
		"\"%s\": \"%s\","
		"\"%s\": \"%s\","
		"\"%s\": \"%s\","
		"\"%s\": \"%s\","
		"\"%s\": \"%s\",",
		dioxlate[dioFan].str,
			dio(dioFan) ? greenball : offball,
		dioxlate[dioFanStatus].str,
			dio(dioFanStatus) ? greenball : offball,
		dioxlate[dioPayloadReset].str,
			dio(dioPayloadReset) ? greenball : offball,
		dioxlate[dioUnused2].str,
			dio(dioUnused2) ? greenball : offball,
		dioxlate[dioPc104Power].str,
			dio(dioPc104Power) ? greenball : offball,
		dioxlate[dioGpsPower].str,
			dio(dioGpsPower) ? greenball : offball,
		dioxlate[dioVicor3xEnable].str,
			dio(dioVicor3xEnable) ? greenball : offball,
		dioxlate[dioVicor1xEnable].str,
			dio(dioVicor1xEnable) ? greenball : offball,
		dioxlate[dioLedPowerOK].str,
			dio(dioLedPowerOK) ? greenball : offball,
		dioxlate[dioLedGpsOK].str,
			dio(dioLedGpsOK) ? greenball : offball,
		dioxlate[dioLedGimbalLockOK].str,
			dio(dioLedGimbalLockOK) ? greenball : offball,
		dioxlate[dioLedGimbalLockFault].str,
			dio(dioLedGimbalLockFault) ? greenball : offball,
		dioxlate[dioLedInputPowerFault].str,
			dio(dioLedInputPowerFault) ? greenball : offball,
		dioxlate[dioLedOutputPowerFault].str,
			dio(dioLedOutputPowerFault) ? greenball : offball,
		dioxlate[dioSwPowerOn].str,
			dio(dioSwPowerOn) ? greenball : offball,
		dioxlate[dioSwLdgOn].str,
			dio(dioSwLdgOn) ? greenball : offball
	);
}
/*---------------------------------------------------------------------------*/

static int exp3_upd(int index, int iNumParams,
		char *pcParam[], char *pcValue[], char **resultBuffer)
{
	uip_appdata[0]=0;
	*resultBuffer = uip_appdata;

	return snprintf((char *)uip_appdata, UIP_APPDATA_SIZE,
		"\"%s\": \"%s\","
		"\"%s\": \"%s\","
		"\"%s\": \"%s\","
		"\"%s\": \"%s\","
		"\"%s\": \"%s\","
		"\"%s\": \"%s\","
		"\"%s\": \"%s\","
		"\"%s\": \"%s\","
		"\"%s\": \"%s\","
		"\"%s\": \"%s\","
		"\"%s\": \"%s\","
		"\"%s\": \"%s\","
		"\"%s\": \"%s\","
		"\"%s\": \"%s\","
		"\"%s\": \"%s\","
		"\"%s\": \"%s\",",
		dioxlate[dioNe28vConnect].str,
			dio(dioNe28vConnect) ? greenball : offball,
		dioxlate[dioNe28vCapConnect].str,
			dio(dioNe28vCapConnect) ? greenball : offball,
		dioxlate[dio5vAuxA].str,
			dio(dio5vAuxA) ? greenball : offball,
		dioxlate[dio5vAuxB].str,
			dio(dio5vAuxB) ? greenball : offball,
		dioxlate[dio12vAuxAOn_].str,
			dio(dio12vAuxAOn_) ? offball : greenball,
		dioxlate[dio12vAuxBOn_].str,
			dio(dio12vAuxBOn_) ? offball : greenball,
		dioxlate[dio12vAuxCOn_].str,
			dio(dio12vAuxCOn_) ? offball : greenball,
		dioxlate[dio41v_].str,
			dio(dio41v_) ? offball : greenball,
		dioxlate[dio15vAux_].str,
			dio(dio15vAux_) ? offball : greenball,
		dioxlate[dio19vAux_].str,
			dio(dio19vAux_) ? offball : greenball,
		dioxlate[dioLdgBusCapConnect].str,
			dio(dioLdgBusCapConnect) ? greenball : offball,
		dioxlate[dioSuPowerOn_].str,
			dio(dioSuPowerOn_) ? offball : greenball,
		dioxlate[dioServoPowerOn_].str,
			dio(dioServoPowerOn_) ? offball : greenball,
		dioxlate[dio28vAuxCOn_].str,
			dio(dio28vAuxCOn_) ? offball : greenball,
		dioxlate[dio28vAuxDOn_].str,
			dio(dio28vAuxDOn_) ? offball : greenball,
		dioxlate[dioOpenGnd1On_].str,
			dio(dioOpenGnd1On_) ? offball : greenball
	);
}
/*---------------------------------------------------------------------------*/

static int exp4_upd(int index, int iNumParams,
		char *pcParam[], char *pcValue[], char **resultBuffer)
{
	uip_appdata[0]=0;
	*resultBuffer = uip_appdata;

	return snprintf((char *)uip_appdata, UIP_APPDATA_SIZE,
		"\"%s\": \"%s\","
		"\"%s\": \"%s\","
		"\"%s\": \"%s\","
		"\"%s\": \"%s\","
		"\"%s\": \"%s\","
		"\"%s\": \"%s\","
		"\"%s\": \"%s\","
		"\"%s\": \"%s\","
		"\"%s\": \"%s\","
		"\"%s\": \"%s\","
		"\"%s\": \"%s\","
		"\"%s\": \"%s\","
		"\"%s\": \"%s\","
		"\"%s\": \"%s\","
		"\"%s\": \"%s\","
		"\"%s\": \"%s\",",
		dioxlate[dio5vAuxAIsOn_].str,
			dio(dio5vAuxAIsOn_) ? offball : greenball,
		dioxlate[dio5vAuxBIsOn_].str,
			dio(dio5vAuxBIsOn_) ? offball : greenball,
		dioxlate[dioLdgBusPrecharge_].str,
			dio(dioLdgBusPrecharge_) ? offball : greenball,
		dioxlate[dioSuPowerIsOn].str,
			dio(dioSuPowerIsOn) ? greenball : offball,
		dioxlate[dioServoPowerIsOn].str,
			dio(dioServoPowerIsOn) ? greenball : offball,
		dioxlate[dio28vAuxCIsOn].str,
			dio(dio28vAuxCIsOn) ? greenball : offball,
		dioxlate[dio28vAuxDIsOn].str,
			dio(dio28vAuxDIsOn) ? greenball : offball,
		dioxlate[dioOpenGnd1IsOn].str,
			dio(dioOpenGnd1IsOn) ? greenball : offball,
		dioxlate[dioSuPowerOff_].str,
			dio(dioSuPowerOff_) ? offball : greenball,
		dioxlate[dioServoPowerOff_].str,
			dio(dioServoPowerOff_) ? offball : greenball,
		dioxlate[dio28vAuxCOff_].str,
			dio(dio28vAuxCOff_) ? offball : greenball,
		dioxlate[dio28vAuxDOff_].str,
			dio(dio28vAuxDOff_) ? offball : greenball,
		dioxlate[dioOpenGnd1Off_].str,
			dio(dioOpenGnd1Off_) ? offball : greenball,
		dioxlate[dioRccbTripped].str,
			dio(dioRccbTripped) ? redball : greenball,
		dioxlate[dioSpare2].str,
			dio(dioSpare2) ? greenball : offball,
		dioxlate[dioSpare3].str,
			dio(dioSpare3) ? greenball : offball
	);
}
/*---------------------------------------------------------------------------*/

static int ctl_upd(int index, int iNumParams,
		char *pcParam[], char *pcValue[], char **resultBuffer)
{
	int vicor1x_currentv, vicor1x_currentmv;
	int vicor3xa_currentv, vicor3xa_currentmv;
	int vicor3xb_currentv, vicor3xb_currentmv;
	int vicor3xc_currentv, vicor3xc_currentmv;
	int ne28v_currentv, ne28v_currentmv;
	int vicor1x_voltagev, vicor1x_voltagemv;
	int vicor3x_voltagev, vicor3x_voltagemv;
	int ne28v_voltagev, ne28v_voltagemv;

	split_eng(adc(adcVicor1xI, engineering),
			&vicor1x_currentv, &vicor1x_currentmv);
	split_eng(adc(adcVicor3xAI, engineering),
			&vicor3xa_currentv, &vicor3xa_currentmv);
	split_eng(adc(adcVicor3xBI, engineering),
			&vicor3xb_currentv, &vicor3xb_currentmv);
	split_eng(adc(adcVicor3xCI, engineering),
			&vicor3xc_currentv, &vicor3xc_currentmv);
	split_eng(adc(adcNe28vInI, engineering),
			&ne28v_currentv, &ne28v_currentmv);
	split_eng(adc(adcVicor1x, engineering),
			&vicor1x_voltagev, &vicor1x_voltagemv);
	split_eng(adc(adcVicor3x, engineering),
			&vicor3x_voltagev, &vicor3x_voltagemv);
	split_eng(adc(adcNe28vIn, engineering),
			&ne28v_voltagev, &ne28v_voltagemv);

	/*
	 * Suppress the distracting noise when unpowered.
	 */
	if (vicor1x_voltagev <= 20)
		vicor1x_currentmv = 0;
	if (vicor3x_voltagev <= 20) {
		vicor3xb_currentmv = 0;
		vicor3xc_currentmv = 0;
		vicor3xa_currentmv = 0;
	}

	uip_appdata[0]=0;
	*resultBuffer = uip_appdata;

	return snprintf((char *)uip_appdata, UIP_APPDATA_SIZE,
		"{"
		/**** Power Control Buttons ****/
		"\"%s\": \"%s\","
		"\"%s\": \"%s\","
		"\"%s\": \"%s\","
		"\"%s\": \"%s\","
		"\"%s\": \"%s\","
		"\"%s\": \"%s\","
		"\"%s\": \"%s\","
		"\"%s\": \"%s\","
		"\"%s\": \"%s\","
		"\"%s\": \"%s\","
		"\"%s\": \"%s\","
		"\"%s\": \"%s\","
		"\"%s\": \"%s\","
		"\"%s\": \"%s\","
		"\"%s\": \"%s\","
		/**** Input Power ****/
		"\"%s\": \"%s\","
		"\"%s\": \"%s\","
		"\"%s\": \"%s\","
		"\"%s\": \"%s\","
		/**** Output Control ****/
		"\"%s\": \"%s\","
		"\"%s\": \"%s\","
		"\"%s\": \"%s\","
		"\"%s\": \"%s\","
		"\"%s\": \"%s\","
		"\"%s\": \"%s\","
		"\"%s\": \"%s\","
		"\"%s\": \"%s\","
		/**** Voltage and Current ****/
		"\"%s\": %d.%03d, \"%s\": %d.%03d,"
		"\"%s\": %d.%03d, \"%s\": %d.%03d,"
		"\"%s\": %d.%03d, \"%s\": %d.%03d,"
		"\"%s\": %d.%03d,"
		"\"%s\": %d.%03d"
		"}",
		/* Power Control Buttons */
		pwrxlate[pwrMaster].str,
			pwr_state(pwrMaster) ? greenball : offball,
		pwrxlate[pwrGCC].str,
			pwr_state(pwrGCC) ? greenball : offball,
		pwrxlate[pwrLdg].str,
			pwr_state(pwrLdg) ? greenball : offball,
		pwrxlate[pwrSu].str,
			pwr_state(pwrSu) ? greenball : offball,
		pwrxlate[pwr5vAuxA].str,
			pwr_state(pwr5vAuxA) ? greenball : offball,
		pwrxlate[pwr5vAuxB].str,
			pwr_state(pwr5vAuxB) ? greenball : offball,
		pwrxlate[pwr12vAuxA].str,
			pwr_state(pwr12vAuxA) ? greenball : offball,
		pwrxlate[pwr12vAuxB].str,
			pwr_state(pwr12vAuxB) ? greenball : offball,
		pwrxlate[pwr12vAuxC].str,
			pwr_state(pwr12vAuxC) ? greenball : offball,
		pwrxlate[pwr15vAux].str,
			pwr_state(pwr15vAux) ? greenball : offball,
		pwrxlate[pwr19vAux].str,
			pwr_state(pwr19vAux) ? greenball : offball,
		pwrxlate[pwr28vAuxC].str,
			pwr_state(pwr28vAuxC) ? greenball : offball,
		pwrxlate[pwr28vAuxD].str,
			pwr_state(pwr28vAuxD) ? greenball : offball,
		pwrxlate[pwrOG1].str,
			pwr_state(pwrOG1) ? greenball : offball,
		pwrxlate[pwrOG2].str,
			pwr_state(pwrOG2) ? greenball : offball,

		/* Input Power */
		dioxlate[dioVicor1xInputPower].str,
			dio(dioVicor1xInputPower) ? greenball : offball,
		dioxlate[dioVicor3xAInputPower].str,
			dio(dioVicor3xAInputPower) ? greenball : offball,
		dioxlate[dioVicor3xBInputPower].str,
			dio(dioVicor3xBInputPower) ? greenball : offball,
		dioxlate[dioVicor3xCInputPower].str,
			dio(dioVicor3xCInputPower) ? greenball : offball,

		/* Output Control */
		dioxlate[dioSuPowerIsOn].str,
			dio(dioSuPowerIsOn) ? greenball : offball,
		dioxlate[dioServoPowerIsOn].str,
			dio(dioServoPowerIsOn) ? greenball : offball,
		dioxlate[dio5vAuxAIsOn_].str,
			dio(dio5vAuxAIsOn_) ? offball :	greenball,
		dioxlate[dio5vAuxBIsOn_].str,
			dio(dio5vAuxBIsOn_) ? offball : greenball,
		dioxlate[dioOpenGnd1IsOn].str,
			dio(dioOpenGnd1IsOn) ? greenball : offball,
		dioxlate[dioOpenGnd2IsOn].str,
			dio(dioOpenGnd2IsOn) ? greenball : offball,
		dioxlate[dioFanStatus].str,
			dio(dioFanStatus) ? greenball : offball,
		dioxlate[dioRccbTripped].str,
			dio(dioRccbTripped) ? redball : greenball,

		/* ADC */
		adcxlate[adcNe28vIn].str,
        		ne28v_voltagev,
			ne28v_voltagemv,
		adcxlate[adcNe28vInI].str,
        		ne28v_currentv,
			ne28v_currentmv,
		adcxlate[adcVicor1x].str,
        		vicor1x_voltagev,
			vicor1x_voltagemv,
		adcxlate[adcVicor1xI].str,
        		vicor1x_currentv,
			vicor1x_currentmv,
		adcxlate[adcVicor3x].str,
        		vicor3x_voltagev,
			vicor3x_voltagemv,
		adcxlate[adcVicor3xAI].str,
        		vicor3xa_currentv,
			vicor3xa_currentmv,
		adcxlate[adcVicor3xBI].str,
        		vicor3xb_currentv,
			vicor3xb_currentmv,
		adcxlate[adcVicor3xCI].str,
        		vicor3xc_currentv,
			vicor3xc_currentmv
	);
}
/*---------------------------------------------------------------------------*/

static int proc_upd(int index, int iNumParams,
		char *pcParam[], char *pcValue[], char **resultBuffer)
{
	/*
	 * The following are used to generate engineering units in integer
	 * units plus milli-units to avoid floating point calculations.
	 */
	int ne28v, ne28mv;
	int vref_2048v, vref_2048mv;
	int bootstrap_p5v, bootstrap_p5mv;

	split_eng(adc(adcNe28vProc, engineering),
			&ne28v, &ne28mv);
	split_eng(adc(adcVref2048, engineering),
			&vref_2048v, &vref_2048mv);
	split_eng(adc(adcBootStrap5v, engineering),
			&bootstrap_p5v, &bootstrap_p5mv);

	uip_appdata[0]=0;
	*resultBuffer = uip_appdata;

	return snprintf((char *)uip_appdata, UIP_APPDATA_SIZE,
		"{"
		"\"%s\": \"%s\","
		"\"%s\": \"%s\","
		"\"%s\": \"%s\","
		"\"%s\": \"%s\","
		"\"%s\": \"%s\","
		"\"%s\": \"%s\","
		"\"%s\": \"%s\","
		"\"%s\": \"%s\","
		"\"%s\": \"%s\","
		"\"%s\": \"%s\","
		"\"%s\": \"%s\","
		"\"%s\": \"%s\","
		"\"%s\": \"%s\","
		"\"%s\": \"%s\","
		"\"%s\": \"%s\","
		"\"%s\": \"%s\","
		"\"%s\": \"%d\"," "\"%smV\": \"%d\"," "\"%sEng\": \"%d.%01d\","
		"\"%s\": \"%d\"," "\"%smV\": \"%d\"," "\"%sEng\": \"%d.%03d\","
		"\"%s\": \"%d\"," "\"%smV\": \"%d\"," "\"%sEng\": \"%d.%03d\","
		"\"%s\": \"%d\"," "\"%smV\": \"%d\"," "\"%sEng\": \"%d\","
		"\"%s\": \"%d\"," "\"%smV\": \"%d\"," "\"%sEng\": \"%d\""
		"}",
		dioxlate[dioGpsPps].str,
			dio(dioGpsPps) ? greenball : offball,
		dioxlate[dioOpenGnd1OverI].str,
			dio(dioOpenGnd1OverI) ? greenball : offball,
		dioxlate[dioOpenGnd2Off_].str,
			dio(dioOpenGnd2Off_) ? offball : greenball,
		dioxlate[dioNe28vOverVInt].str,
			dio(dioNe28vOverVInt) ? greenball : offball,
		dioxlate[dioNe28vCapConnected_].str,
			dio(dioNe28vCapConnected_) ? offball : greenball,
		dioxlate[dioNe28vOverV_].str,
			dio(dioNe28vOverV_) ? greenball : redball,
		dioxlate[dioSuOverI].str,
			dio(dioSuOverI) ? redball : greenball,
		dioxlate[dioVicor3xLossedBus_].str,
			dio(dioVicor3xLossedBus_) ? greenball : redball,
		dioxlate[dioRccbOverI].str,
			dio(dioRccbOverI) ? redball : greenball,
		dioxlate[dioVicor1xLossedBus_].str,
			dio(dioVicor1xLossedBus_) ? greenball : redball,
		dioxlate[dio28vAuxBOverI].str,
			dio(dio28vAuxBOverI) ? redball : greenball,
		dioxlate[dioNe28vUnderV_].str,
			dio(dioNe28vUnderV_) ? greenball : redball,
		dioxlate[dio28vAuxAOverI].str,
			dio(dio28vAuxAOverI) ? redball : greenball,
		dioxlate[dioMasterArm_].str,
			dio(dioMasterArm_) ? offball : greenball,
		dioxlate[dioOpenGnd2OverI].str,
			dio(dioOpenGnd2OverI) ? redball : greenball,
		dioxlate[dioOpenGnd2On_].str,
			dio(dioOpenGnd2On_) ? offball : greenball,

		adcxlate[adcNe28vProc].str,
			adc(adcNe28vProc, raw),
			adcxlate[adcNe28vProc].str,
			adc(adcNe28vProc, millivolts),
			adcxlate[adcNe28vProc].str,
			ne28v, ne28mv / 100,
		adcxlate[adcVref2048].str,
			adc(adcVref2048, raw),
			adcxlate[adcVref2048].str,
			adc(adcVref2048, millivolts),
			adcxlate[adcVref2048].str,
			vref_2048v, vref_2048mv,
		adcxlate[adcBootStrap5v].str,
			adc(adcBootStrap5v, raw),
			adcxlate[adcBootStrap5v].str,
			adc(adcBootStrap5v, millivolts),
			adcxlate[adcBootStrap5v].str,
			bootstrap_p5v, bootstrap_p5mv,
		adcxlate[adcBoardTemp].str,
			adc(adcBoardTemp, raw),
			adcxlate[adcBoardTemp].str,
			adc(adcBoardTemp, millivolts),
			adcxlate[adcBoardTemp].str,
			adc(adcBoardTemp, engineering) / 1000,
		adcxlate[adcProcTemp1].str,
			adc(adcProcTemp1, raw),
			adcxlate[adcProcTemp1].str,
			adc(adcProcTemp1, millivolts),
			adcxlate[adcProcTemp1].str,
			adc(adcProcTemp1, engineering) / 1000
	);
}
/*---------------------------------------------------------------------------*/

static int adc_spi_upd(int index, int iNumParams,
		char *pcParam[], char *pcValue[], char **resultBuffer)
{
	/*
	 * SPI ADS7844
	 */
	uip_appdata[0]=0;
	*resultBuffer = uip_appdata;

	return snprintf((char *)uip_appdata, UIP_APPDATA_SIZE,
		"\"%s\": \"%d\"," "\"%smV\": \"%d\"," "\"%sEng\": \"%d\","
		"\"%s\": \"%d\"," "\"%smV\": \"%d\"," "\"%sEng\": \"%d\","
		"\"%s\": \"%d\"," "\"%smV\": \"%d\"," "\"%sEng\": \"%d\","
		"\"%s\": \"%d\"," "\"%smV\": \"%d\"," "\"%sEng\": \"%d\","
		"\"%s\": \"%d\"," "\"%smV\": \"%d\"," "\"%sEng\": \"%d\","
		"\"%s\": \"%d\"," "\"%smV\": \"%d\"," "\"%sEng\": \"%d\","
		"\"%s\": \"%d\"," "\"%smV\": \"%d\"," "\"%sEng\": \"%d\","
		"\"%s\": \"%d\"," "\"%smV\": \"%d\"," "\"%sEng\": \"%d\","
		,
		ADCEXPAND(adcVicor1xI),
		ADCEXPAND(adcVicor3xAI),
		ADCEXPAND(adcVicor3xBI),
		ADCEXPAND(adcVicor3xCI),
		ADCEXPAND(adcNe28vInI),
		ADCEXPAND(adcVicor1x),
		ADCEXPAND(adcVicor3x),
		ADCEXPAND(adcNe28vIn)
	);
}
/*---------------------------------------------------------------------------*/

static int adc0_upd(int index, int iNumParams,
		char *pcParam[], char *pcValue[], char **resultBuffer)
{
	/*
	 * SPI ADS7844 Mux 0
	 */
	uip_appdata[0]=0;
	*resultBuffer = uip_appdata;

	return snprintf((char *)uip_appdata, UIP_APPDATA_SIZE,
		"\"%s\": \"%d\"," "\"%smV\": \"%d\"," "\"%sEng\": \"%d\","
		"\"%s\": \"%d\"," "\"%smV\": \"%d\"," "\"%sEng\": \"%d\","
		"\"%s\": \"%d\"," "\"%smV\": \"%d\"," "\"%sEng\": \"%d\","
		"\"%s\": \"%d\"," "\"%smV\": \"%d\"," "\"%sEng\": \"%d\","
		"\"%s\": \"%d\"," "\"%smV\": \"%d\"," "\"%sEng\": \"%d\","
		"\"%s\": \"%d\"," "\"%smV\": \"%d\"," "\"%sEng\": \"%d\","
#if (LRU_IS_GCU == 2)
		"\"%s\": \"%d\"," "\"%smV\": \"%d\"," "\"%sEng\": \"%d\","
		"\"%s\": \"%d\"," "\"%smV\": \"%d\"," "\"%sEng\": \"%d\","
#endif
		,
#if (LRU_IS_GCU == 2)
		ADCEXPAND(adcExpADC0),
#endif
		ADCEXPAND(adc2048vRefB),
		ADCEXPAND(adcNe28vIn1),
		ADCEXPAND(adc19vAux),
		ADCEXPAND(adcAircraft28v),
		ADCEXPAND(adcSuI),
#if (LRU_IS_GCU == 2)
		ADCEXPAND(adcSu),
#endif
		ADCEXPAND(adc12vAuxB)
	);
}
/*---------------------------------------------------------------------------*/

static int adc1_upd(int index, int iNumParams,
		char *pcParam[], char *pcValue[], char **resultBuffer)
{
	/*
	 * SPI ADS7844 Mux 1
	 */
	uip_appdata[0]=0;
	*resultBuffer = uip_appdata;

	return snprintf((char *)uip_appdata, UIP_APPDATA_SIZE,
		"\"%s\": \"%d\"," "\"%smV\": \"%d\"," "\"%sEng\": \"%d\","
		"\"%s\": \"%d\"," "\"%smV\": \"%d\"," "\"%sEng\": \"%d\","
		"\"%s\": \"%d\"," "\"%smV\": \"%d\"," "\"%sEng\": \"%d\","
		"\"%s\": \"%d\"," "\"%smV\": \"%d\"," "\"%sEng\": \"%d\","
		"\"%s\": \"%d\"," "\"%smV\": \"%d\"," "\"%sEng\": \"%d\","
		"\"%s\": \"%d\"," "\"%smV\": \"%d\"," "\"%sEng\": \"%d\","
#if (LRU_IS_GCU == 2)
		"\"%s\": \"%d\"," "\"%smV\": \"%d\"," "\"%sEng\": \"%d\","
		"\"%s\": \"%d\"," "\"%smV\": \"%d\"," "\"%sEng\": \"%d\","
#endif
		,
#if (LRU_IS_GCU == 2)
		ADCEXPAND(adcExpADC1),
		ADCEXPAND(adcExpADC2),
#endif
		ADCEXPAND(adc28vHoldup),
		ADCEXPAND(adc15vAux),
		ADCEXPAND(adcLdgBusCap),
		ADCEXPAND(adcRccbI),
		ADCEXPAND(adc28vAuxDI),
		ADCEXPAND(adc12vAuxAI)
	);
}
/*---------------------------------------------------------------------------*/

static int adc2_upd(int index, int iNumParams,
		char *pcParam[], char *pcValue[], char **resultBuffer)
{
	/*
	 * SPI ADS7844 Mux 2
	 */
	uip_appdata[0]=0;
	*resultBuffer = uip_appdata;

	return snprintf((char *)uip_appdata, UIP_APPDATA_SIZE,
		"\"%s\": \"%d\"," "\"%smV\": \"%d\"," "\"%sEng\": \"%d\","
		"\"%s\": \"%d\"," "\"%smV\": \"%d\"," "\"%sEng\": \"%d\","
		"\"%s\": \"%d\"," "\"%smV\": \"%d\"," "\"%sEng\": \"%d\","
		"\"%s\": \"%d\"," "\"%smV\": \"%d\"," "\"%sEng\": \"%d\","
		"\"%s\": \"%d\"," "\"%smV\": \"%d\"," "\"%sEng\": \"%d\","
		"\"%s\": \"%d\"," "\"%smV\": \"%d\"," "\"%sEng\": \"%d\","
#if (LRU_IS_GCU == 2)
		"\"%s\": \"%d\"," "\"%smV\": \"%d\"," "\"%sEng\": \"%d\","
		"\"%s\": \"%d\"," "\"%smV\": \"%d\"," "\"%sEng\": \"%d\","
#endif
		,
		ADCEXPAND(adcBoardTemp1),
#if (LRU_IS_GCU == 2)
		ADCEXPAND(adcOpenGnd1I),
#endif
		ADCEXPAND(adcCapFetGate),
		ADCEXPAND(adc2048vRefA),
		ADCEXPAND(adc5vAuxAbcdI),
#if (LRU_IS_GCU == 2)
		ADCEXPAND(adc28vAuxCI),
#endif
		ADCEXPAND(adcChassisFans),
		ADCEXPAND(adc12vAuxA)
	);
}
/*---------------------------------------------------------------------------*/

int adc3_upd(int index, int iNumParams,
		char *pcParam[], char *pcValue[], char **resultBuffer)
{
	/*
	 * SPI ADS7844 Mux 3
	 */
	uip_appdata[0]=0;
	*resultBuffer = uip_appdata;

	return snprintf((char *)uip_appdata, UIP_APPDATA_SIZE,
		"\"%s\": \"%d\"," "\"%smV\": \"%d\"," "\"%sEng\": \"%d\","
		"\"%s\": \"%d\"," "\"%smV\": \"%d\"," "\"%sEng\": \"%d\","
		"\"%s\": \"%d\"," "\"%smV\": \"%d\"," "\"%sEng\": \"%d\","
		"\"%s\": \"%d\"," "\"%smV\": \"%d\"," "\"%sEng\": \"%d\","
		"\"%s\": \"%d\"," "\"%smV\": \"%d\"," "\"%sEng\": \"%d\","
		"\"%s\": \"%d\"," "\"%smV\": \"%d\"," "\"%sEng\": \"%d\","
#if (LRU_IS_GCU == 2)
		"\"%s\": \"%d\"," "\"%smV\": \"%d\"," "\"%sEng\": \"%d\","
		"\"%s\": \"%d\"," "\"%smV\": \"%d\"," "\"%sEng\": \"%d\","
#endif
		,
#if (LRU_IS_GCU == 2)
		ADCEXPAND(adc28vAuxC),
		ADCEXPAND(adcOpenGnd2I),
#endif
		ADCEXPAND(adc19vAuxI),
		ADCEXPAND(adc15vAuxI),
		ADCEXPAND(adcBoardTemp2),
		ADCEXPAND(adc5vAuxEfgI),
		ADCEXPAND(adc12vAuxCI),
		ADCEXPAND(adc12vAuxBI)
	);
}
#endif
/*---------------------------------------------------------------------------*/


static const tCGI ssi_cgi_funcs[] = {
		{ "/file_stats", file_stats },
		{ "/tcp_connections", tcp_stats },
		{ "/net_stats", net_stats },
		{ "/rtos_stats", rtos_stats },
		{ "/run_time", run_time },

		/* Configuration */
		{ "/perm_config", perm_config },
		{ "/user_config", user_config },
		{ "/config_form", process_form_config },
#if LRU_IS_GCU
								,
		/* Page updates */
		{ "/ctl_upd", ctl_upd },
		{ "/exp1_upd", exp1_upd },
		{ "/exp2_upd", exp2_upd },
		{ "/exp3_upd", exp3_upd },
		{ "/exp4_upd", exp4_upd },
		{ "/adc_spi_upd", adc_spi_upd },
		{ "/adc0_upd", adc0_upd },
		{ "/adc1_upd", adc1_upd },
		{ "/adc2_upd", adc2_upd },
		{ "/adc3_upd", adc3_upd },
		{ "/proc_upd", proc_upd }
#endif
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
	lstr("<SSI.");lhex(iIndex);lstr(">");
	if (iIndex<NUM_SSI_CGI_FUNCTIONS) {
		return calls[iIndex].pfnCGIHandler(iIndex, iNumParams,
				pcParam, pcValue, resultBuffer);
	}
	else if (iIndex<NUM_SSI_CGI_ENTRIES) {
			struct fs_file *fs;

			fs = fs_open((char *)calls[iIndex].pcCGIName);
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
