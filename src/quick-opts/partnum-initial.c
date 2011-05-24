/*
 * partnum-initial.c
 *
 *  Created on: May 23, 2011
 *      Author: mikes
 */

/**
 * Default user configuration data.
 *
 * This is what is filled in the configuration data fields if the flash
 * area is invalid (e.g. not programmed).
 *
 * \req \req_config The \program \shall use default configuration values
 *   if they have not been set previously.
 */
static struct usercfg_s default_usercfg = {
	.length = sizeof(struct usercfg_s),
	.version = 1,

	.assy_pn = "8A7W5 100542-001 Rev x1",
	.assy_sn = "20110214001",

	.ip[0] = 192,
	.ip[1] = 168,
	.ip[2] = 8,
	.ip[3] = 29,

	.netmask[0] = 255,
	.netmask[1] = 255,
	.netmask[2] = 255,
	.netmask[3] = 0,

	.gateway[0] = 192,
	.gateway[1] = 168,
	.gateway[2] = 8,
	.gateway[3] = 1,

	.IPMode = IPADDR_USE_DHCP,

	.notes = {0},

	.checksum = 0,
};
