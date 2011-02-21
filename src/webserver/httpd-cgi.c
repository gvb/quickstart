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

#include "uip.h"
#include "psock.h"
#include "httpd.h"
#include "httpd-cgi.h"
#include "httpd-fs.h"

#include <stdio.h>
#include <string.h>

#include <config.h>
#include <partnum.h>
#include <io.h>


HTTPD_CGI_CALL(file, "file-stats", file_stats);
HTTPD_CGI_CALL(tcp, "tcp-connections", tcp_stats);
HTTPD_CGI_CALL(net, "net-stats", net_stats);
HTTPD_CGI_CALL(rtos, "rtos-stats", rtos_stats);
HTTPD_CGI_CALL(run, "run-time", run_time);

/* Configuration */
HTTPD_CGI_CALL(perm_config_pg, "perm-config", perm_config);
HTTPD_CGI_CALL(user_config_pg, "user-config", user_config);

/* Page updates */
HTTPD_CGI_CALL(ctl_upd_pg, "ctl-upd", ctl_upd);
HTTPD_CGI_CALL(proc_upd_pg, "proc-upd", proc_upd);

static const struct httpd_cgi_call *calls[] = {
	&file, &tcp, &net, &rtos, &run,
	&perm_config_pg,
	&user_config_pg,
	&ctl_upd_pg,
	&proc_upd_pg,
	NULL };

/*
 * Some things to help make pages.
 */

#define ADCEXPAND(a) \
	adcxlate[a].str, adc(a, raw), \
	adcxlate[a].str, adc(a, millivolts), \
	adcxlate[a].str, adc(a, engineering)

static const char *greenball = "<img src='/green.png' />";
static const char *offball   = "<img src='/off.png' />";
static const char *redball   = "<img src='/red.png' />";

static void split_eng(int a, int *integer, int *fract)
{
	*fract = a;
	*integer = *fract / 1000;
	*fract = *fract - (*integer * 1000);
}

/*---------------------------------------------------------------------------*/
static
PT_THREAD(nullfunction(struct httpd_state *s, char *ptr))
{
  PSOCK_BEGIN(&s->sout);
  PSOCK_END(&s->sout);
}
/*---------------------------------------------------------------------------*/
httpd_cgifunction
httpd_cgi(char *name)
{
  const struct httpd_cgi_call **f;

  /* Find the matching name in the table, return the function. */
  for(f = calls; *f != NULL; ++f) {
    if(strncmp((*f)->name, name, strlen((*f)->name)) == 0) {
      return (*f)->function;
    }
  }
  return nullfunction;
}
/*---------------------------------------------------------------------------*/
static unsigned short
generate_file_stats(void *arg)
{
  char *f = (char *)arg;
  return snprintf((char *)uip_appdata, UIP_APPDATA_SIZE, "%5u", httpd_fs_count(f));
}
/*---------------------------------------------------------------------------*/
static
PT_THREAD(file_stats(struct httpd_state *s, char *ptr))
{
  PSOCK_BEGIN(&s->sout);

  PSOCK_GENERATOR_SEND(&s->sout, generate_file_stats, strchr(ptr, ' ') + 1);

  PSOCK_END(&s->sout);
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


static unsigned short
generate_tcp_stats(void *arg)
{
  struct uip_conn *conn;
  struct httpd_state *s = (struct httpd_state *)arg;

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
}
/*---------------------------------------------------------------------------*/
static
PT_THREAD(tcp_stats(struct httpd_state *s, char *ptr))
{

  PSOCK_BEGIN(&s->sout);

  for(s->count = 0; s->count < UIP_CONNS; ++s->count) {
    if((uip_conns[s->count].tcpstateflags & UIP_TS_MASK) != UIP_CLOSED) {
      PSOCK_GENERATOR_SEND(&s->sout, generate_tcp_stats, s);
    }
  }

  PSOCK_END(&s->sout);
}
/*---------------------------------------------------------------------------*/
static unsigned short
generate_net_stats(void *arg)
{
  struct httpd_state *s = (struct httpd_state *)arg;
  return snprintf((char *)uip_appdata, UIP_APPDATA_SIZE,
		  "%5u\n", ((uip_stats_t *)&uip_stat)[s->count]);
}

static
PT_THREAD(net_stats(struct httpd_state *s, char *ptr))
{
  PSOCK_BEGIN(&s->sout);

#if UIP_STATISTICS

  for(s->count = 0; s->count < sizeof(uip_stat) / sizeof(uip_stats_t);
      ++s->count) {
    PSOCK_GENERATOR_SEND(&s->sout, generate_net_stats, s);
  }

#endif /* UIP_STATISTICS */

  PSOCK_END(&s->sout);
}
/*---------------------------------------------------------------------------*/

extern void vTaskList( signed char *pcWriteBuffer );
static char cCountBuf[ 32 ];
long lRefreshCount = 0;
static unsigned short
generate_rtos_stats(void *arg)
{
	lRefreshCount++;
	sprintf( cCountBuf, "<p><br>Refresh count = %ld", lRefreshCount );
	vTaskList( uip_appdata );
	strcat( uip_appdata, cCountBuf );

	return strlen( uip_appdata );
}
/*---------------------------------------------------------------------------*/

extern void vTaskGetRunTimeStats( signed char *pcWriteBuffer );
static unsigned short
generate_runtime_stats(void *arg)
{
	lRefreshCount++;
	sprintf( cCountBuf, "<p><br>Refresh count = %ld", lRefreshCount );
	vTaskGetRunTimeStats( uip_appdata );
	strcat( uip_appdata, cCountBuf );

	return strlen( uip_appdata );
}
/*---------------------------------------------------------------------------*/

static unsigned short gen_proc_upd_state( void *arg )
{
	/*
	 * The following are used to generate engineering units in integer
	 * units plus milli-units to avoid floating point calculations.
	 */
	int adcProc0v, adcProc0mv;
	int adcProc1v, adcProc1mv;
	int adcProc2v, adcProc2mv;
	int adcProc3v, adcProc3mv;

	split_eng(adc(adcProc0, engineering), &adcProc0v, &adcProc0mv);
	split_eng(adc(adcProc1, engineering), &adcProc1v, &adcProc1mv);
	split_eng(adc(adcProc2, engineering), &adcProc2v, &adcProc2mv);
	split_eng(adc(adcProc3, engineering), &adcProc3v, &adcProc3mv);

	return snprintf((char *)uip_appdata, UIP_APPDATA_SIZE,
		"{"
		"\"%s\": \"%s\","
		"\"%s\": \"%s\","
		"\"%s\": \"%s\","
		"\"%s\": \"%s\","
		"\"%s\": \"%s\","
		"\"%s\": \"%s\","
		"\"%s\": \"%d\"," "\"%smV\": \"%d\","
		"\"%s\": \"%d\"," "\"%smV\": \"%d\","
		"\"%s\": \"%d\"," "\"%smV\": \"%d\","
		"\"%s\": \"%d\"," "\"%smV\": \"%d\","
		"\"%s\": \"%d\"," "\"%sEng\": \"%d\","
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
			adcxlate[adcProc0].str,
			adcProc0v, adcProc0mv,
		adcxlate[adcProc1].str,
			adc(adcProc1, raw),
			adcxlate[adcProc1].str,
			adc(adcProc1, millivolts),
			adcxlate[adcProc1].str,
			adcProc1v, adcProc1mv,
		adcxlate[adcProc2].str,
			adc(adcProc2, raw),
			adcxlate[adcProc2].str,
			adc(adcProc2, millivolts),
			adcxlate[adcProc2].str,
			adcProc2v, adcProc2mv,
		adcxlate[adcProc3].str,
			adc(adcProc3, raw),
			adcxlate[adcProc3].str,
			adc(adcProc3, millivolts),
			adcxlate[adcProc3].str,
			adcProc3v, adcProc3mv,
		adcxlate[adcProcTemp].str,
			adc(adcProcTemp, raw),
			adcxlate[adcProcTemp].str,
			adc(adcProcTemp, engineering) / 1000
	);
}
/*---------------------------------------------------------------------------*/

static unsigned short gen_ctl_upd_state( void *arg )
{
	return snprintf((char *)uip_appdata, UIP_APPDATA_SIZE,
		"{"
		"\"%s\": \"%s\","
		"\"%s\": \"%s\","
		"\"%s\": \"%s\","
		"\"%s\": \"%s\","
		"\"%s\": \"%s\","
		"\"%s\": \"%s\","
		"\"%s\": \"%d\"," "\"%smV\": \"%d\","
		"\"%s\": \"%d\"," "\"%smV\": \"%d\","
		"\"%s\": \"%d\"," "\"%smV\": \"%d\","
		"\"%s\": \"%d\"," "\"%smV\": \"%d\","
		"\"%s\": \"%d\"," "\"%sEng\": \"%d\","
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

static unsigned short generate_perm_config_state( void *arg )
{
	int pcvalid = permcfg_valid();

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

static unsigned short generate_user_config_state( void *arg )
{
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
/*---------------------------------------------------------------------------*/

static
PT_THREAD(run_time(struct httpd_state *s, char *ptr))
{
  PSOCK_BEGIN(&s->sout);
  PSOCK_GENERATOR_SEND(&s->sout, generate_runtime_stats, NULL);
  PSOCK_END(&s->sout);
}
/*---------------------------------------------------------------------------*/

static
PT_THREAD(rtos_stats(struct httpd_state *s, char *ptr))
{
  PSOCK_BEGIN(&s->sout);
  PSOCK_GENERATOR_SEND(&s->sout, generate_rtos_stats, NULL);
  PSOCK_END(&s->sout);
}
/*---------------------------------------------------------------------------*/

static PT_THREAD(perm_config(struct httpd_state *s, char *ptr))
{
  PSOCK_BEGIN(&s->sout);
  PSOCK_GENERATOR_SEND(&s->sout, generate_perm_config_state, NULL);
  PSOCK_END(&s->sout);
}
/*---------------------------------------------------------------------------*/

static PT_THREAD(user_config(struct httpd_state *s, char *ptr))
{
  PSOCK_BEGIN(&s->sout);
  PSOCK_GENERATOR_SEND(&s->sout, generate_user_config_state, NULL);
  PSOCK_END(&s->sout);
}
/*---------------------------------------------------------------------------*/

static PT_THREAD(ctl_upd(struct httpd_state *s, char *ptr))
{
  PSOCK_BEGIN(&s->sout);
  PSOCK_GENERATOR_SEND(&s->sout, gen_ctl_upd_state, NULL);
  PSOCK_END(&s->sout);
}
/*---------------------------------------------------------------------------*/

static PT_THREAD(proc_upd(struct httpd_state *s, char *ptr))
{
  PSOCK_BEGIN(&s->sout);
  PSOCK_GENERATOR_SEND(&s->sout, gen_proc_upd_state, NULL);
  PSOCK_END(&s->sout);
}
/*---------------------------------------------------------------------------*/

/** @} */
