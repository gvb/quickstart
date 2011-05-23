/*
 * Copyright (c) 2001-2003 Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 *
 * Author: Adam Dunkels <adam@sics.se>
 *
 */
#ifndef __LWIPOPTS_H__
#define __LWIPOPTS_H__

#include "logger.h"

#define TCPIP_THREAD_NAME               "tcp-ip"
#define TCPIP_THREAD_STACKSIZE          7000
#define TCPIP_THREAD_PRIO               3

#define DEFAULT_THREAD_STACKSIZE        200
#define DEFAULT_THREAD_PRIO             3

#define ETH_PAD_SIZE 					2

#define NOT_LWIP_DEBUG                  0
#define DBG_TYPES_ON                    0x00
#define LWIP_DBG_TYPES_ON               0
/*
 * To turn on debugging:
 * #define LWIP_DBG_TYPES_ON               LWIP_DBG_ON
 */
#define LWIP_DBG_MIN_LEVEL              0
#define MEMP_SANITY_CHECK               1
#define ARP_QUEUEING                    1

#define LWIP_DHCP_AUTOIP_COOP           ((LWIP_DHCP) && (LWIP_AUTOIP))

#define LWIP_TCP                        1

#define LWIP_PLATFORM_DIAG(x) {lprintf x;}

#define LWIP_DEBUG						1

#define ETHARP_DEBUG                   LWIP_DBG_ON
#define NETIF_DEBUG                    LWIP_DBG_ON
#define PBUF_DEBUG                     LWIP_DBG_ON
#define API_LIB_DEBUG                  LWIP_DBG_ON
#define API_MSG_DEBUG                  LWIP_DBG_ON
#define SOCKETS_DEBUG                  LWIP_DBG_ON
#define ICMP_DEBUG                     LWIP_DBG_ON
#define IGMP_DEBUG                     LWIP_DBG_ON
#define INET_DEBUG                     LWIP_DBG_ON
#define IP_DEBUG                       LWIP_DBG_ON
#define IP_REASS_DEBUG                 LWIP_DBG_ON
#define RAW_DEBUG                      LWIP_DBG_ON
#define MEM_DEBUG                      LWIP_DBG_ON
#define MEMP_DEBUG                     LWIP_DBG_ON
#define SYS_DEBUG                      LWIP_DBG_ON
#define TCP_DEBUG                      LWIP_DBG_ON
#define TCP_INPUT_DEBUG                LWIP_DBG_ON
#define TCP_FR_DEBUG                   LWIP_DBG_ON
#define TCP_RTO_DEBUG                  LWIP_DBG_ON
#define TCP_CWND_DEBUG                 LWIP_DBG_ON
#define TCP_WND_DEBUG                  LWIP_DBG_ON
#define TCP_OUTPUT_DEBUG               LWIP_DBG_ON
#define TCP_RST_DEBUG                  LWIP_DBG_ON
#define TCP_QLEN_DEBUG                 LWIP_DBG_ON
#define UDP_DEBUG                      LWIP_DBG_ON
#define TCPIP_DEBUG                    LWIP_DBG_ON
#define PPP_DEBUG                      LWIP_DBG_ON
#define SLIP_DEBUG                     LWIP_DBG_ON
#define DHCP_DEBUG                     LWIP_DBG_ON
#define AUTOIP_DEBUG                   LWIP_DBG_ON
#define SNMP_MSG_DEBUG                 LWIP_DBG_ON
#define SNMP_MIB_DEBUG                 LWIP_DBG_ON
#define DNS_DEBUG                      LWIP_DBG_ON

#define U8_F "c"
#define S8_F "c"
#define X8_F "x"
#define U16_F "u"
#define S16_F "d"
#define X16_F "x"
#define U32_F "u"
#define S32_F "d"
#define X32_F "x"

#define ETHARP_TRUST_IP_MAC				1

/**
 * SYS_LIGHTWEIGHT_PROT==1: if you want inter-task protection for certain
 * critical regions during buffer allocation, deallocation and memory
 * allocation and deallocation.
 */
#define SYS_LIGHTWEIGHT_PROT            1

/*
   ------------------------------------
   ---------- Memory options ----------
   ------------------------------------
*/

/**
 * MEM_ALIGNMENT: should be set to the alignment of the CPU
 *    4 byte alignment -> #define MEM_ALIGNMENT 4
 *    2 byte alignment -> #define MEM_ALIGNMENT 2
 */
#define MEM_ALIGNMENT                   4

/**
 * MEM_SIZE: the size of the heap memory. If the application will send
 * a lot of data that needs to be copied, this should be set high.
 */
#define MEM_SIZE                        (4*1024)

/*
   ------------------------------------------------
   ---------- Internal Memory Pool Sizes ----------
   ------------------------------------------------
*/
/**
 * MEMP_NUM_PBUF: the number of memp struct pbufs (used for PBUF_ROM and PBUF_REF).
 * If the application sends a lot of data out of ROM (or other static memory),
 * this should be set high.
 */

#define MEMP_NUM_PBUF                   20

/**
 * MEMP_NUM_TCP_PCB: the number of simulatenously active TCP connections.
 * (requires the LWIP_TCP option)
 */
#define MEMP_NUM_TCP_PCB                10

/**
 * MEMP_NUM_TCP_SEG: the number of simultaneously queued TCP segments.
 * (requires the LWIP_TCP option)
 */
#define MEMP_NUM_TCP_SEG                8

/**
 * MEMP_NUM_SYS_TIMEOUT: the number of simulateously active timeouts.
 * (requires NO_SYS==0)
 */
#define MEMP_NUM_SYS_TIMEOUT            5

/**
 * MEMP_NUM_NETBUF: the number of struct netbufs.
 * (only needed if you use the sequential API, like api_lib.c)
 */
#define MEMP_NUM_NETBUF                 4

/**
 * PBUF_POOL_SIZE: the number of buffers in the pbuf pool.
 */
#define PBUF_POOL_SIZE                  4


/*
   ----------------------------------
   ---------- Pbuf options ----------
   ----------------------------------
*/

/**
 * PBUF_POOL_BUFSIZE: the size of each pbuf in the pbuf pool. The default is
 * designed to accomodate single full size TCP frame in one pbuf, including
 * TCP_MSS, IP header, and link header.
 */
#define PBUF_POOL_BUFSIZE               1500

/*
 ---------------------------------
 ---------- TCP options ----------
 ---------------------------------
*/
/**
 * LWIP_TCP==1: Turn on TCP.
 */
#define LWIP_TCP                        1

/*
 * \todo Why do we have to turn on CORE_LOCKING
 * and CORE_LOCKING_INPUT to get TCP/IP to work?
 */

#define LWIP_TCPIP_CORE_LOCKING         1
#define LWIP_TCPIP_CORE_LOCKING_INPUT   1

/* TCP Maximum segment size. */
#define TCP_MSS                         1500

/* TCP sender buffer space (bytes). */
#define TCP_SND_BUF                     3000

/**
 * TCP_WND: The size of a TCP window.
 */
#define TCP_WND                         1500

/**
 * TCP_SYNMAXRTX: Maximum number of retransmissions of SYN segments.
 */
#define TCP_SYNMAXRTX                   4

/*
   ---------------------------------
   ---------- RAW options ----------
   ---------------------------------
*/
/**
 * LWIP_RAW==1: Enable application layer to hook into the IP layer itself.
 */
#define LWIP_RAW                        0


/*
   ------------------------------------
   ---------- Socket options ----------
   ------------------------------------
*/
/**
 * LWIP_SOCKET==1: Enable Socket API (require to use sockets.c)
 */
#define LWIP_SOCKET                     0


/*
   ----------------------------------------
   ---------- Statistics options ----------
   ----------------------------------------
*/
/**
 * LWIP_STATS==1: Enable statistics collection in lwip_stats.
 */
#define LWIP_STATS                      0

/*
   ----------------------------------
   ---------- DHCP options ----------
   ----------------------------------
*/
/**
 * LWIP_DHCP==1: Enable DHCP module.
 */
#define LWIP_DHCP                       0


#define LWIP_PROVIDE_ERRNO				0

/*
   ----------------------------------
   --------- HTTPD options ----------
   ----------------------------------
 */

#define INCLUDE_HTTPD_CGI 				1
#define INCLUDE_HTTPD_SSI				1
#define USER_PROVIDES_ZERO_COPY_STATIC_TAGS 1
#define HTTPD_CGI_USE_STATIC_BUFFER     1

#endif /* __LWIPOPTS_H__ */
