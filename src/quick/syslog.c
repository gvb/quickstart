/*
 * see http://tools.ietf.org/html/rfc3164
 */

#include <stdarg.h>
#include <syslog.h>
#include <lwip/udp.h>
#include <LWIPStack.h>

static const int rfc3164_max_packet_size = 1024;

static struct sysLogIniStruct {
	int initialized;
	ip_addr_t localIp;
	ip_addr_t remotIp;
	u16_t localPort;
	u16_t remotPort;
} sysLogIni = {0};

void syslogInit(void)
{
	// \todo define constants for IP address and port

	IP_CONFIG currentIPConfig;

	LWIPServiceTaskIPConfigGet(lwip_netif, &currentIPConfig);

	if (!sysLogIni.initialized) {
		sysLogIni.initialized = 1;
		sysLogIni.localIp.addr = currentIPConfig.IPAddr;
		sysLogIni.remotIp.addr = htonl((192<<24|168<<16|8<<8|160));
		sysLogIni.localPort = 6719;
		sysLogIni.remotPort = 6719;
	}
}

void syslog(enum facility_vals fac, enum level_vals lev, char * fmt, ...)
{
	struct udp_pcb *pcb;
	struct pbuf *p;
	va_list argptr;
	va_start(argptr, fmt);
	char *eos;

	pcb = udp_new();

	udp_bind(pcb, &sysLogIni.localIp, sysLogIni.localPort);

	p = pbuf_alloc(PBUF_TRANSPORT,rfc3164_max_packet_size,PBUF_RAM);

	eos = p->payload;

	eos += sprintf(eos,"<%d>", lev+fac*8 );

	vsnprintf(eos, rfc3164_max_packet_size-(eos-(char *)p->payload), fmt, argptr);

	udp_sendto(pcb, p, &sysLogIni.remotIp, sysLogIni.remotPort);

	pbuf_free(p);
	udp_remove(pcb);

}
