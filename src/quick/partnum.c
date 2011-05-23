/**
 * \file partnum.c
 *
 * Store and access configuration information in flash.
 *
 * \addtogroup util Utilities
 * \{
 *
 *//*
 * Copyright (C) 2011 Consolidated Resource Imaging LLC
 *
 *       1         2         3         4         5         6         7        
 *3456789012345678901234567890123456789012345678901234567890123456789012345678
 */

#include <stdint.h>

#include <FreeRTOSConfig.h>
#include <hw_types.h>
#include "../../StellarisWare/driverlib/flash.h"

#include "config.h"
#include "partnum.h"

/**
 * Default permanent configuration data.
 *
 * This is what is filled in the configuration data fields if the flash
 * area is invalid (e.g. not programmed).
 *
 * \req \req_config The \program \shall use default configuration values
 *   if they have not been set previously.
 */
static struct permcfg_s default_permcfg = {
	.length   = sizeof(struct permcfg_s),
	.version  = -1,
	.bd_pn    = "8A7W5 100569-001 Rev x1",
	.bd_sn    = "20110214001",
	.mac[0]   = 0xa8,
	.mac[1]   = 0xfc,
	.mac[2]   = 0xb7,
	.mac[3]   = 0x00,
	.mac[4]   = 0x00,
	.mac[5]   = 0x00,
	.mac[6]   = 0x00,
	.mac[7]   = 0x00,
	.checksum = 0,
};

#include <partnum-initial.c>

/*
 * Globally accessible configuration values.  This allows quick access
 * and should be treated as "read-only."
 */
struct permcfg_s permcfg;
struct usercfg_s usercfg;

/**
 * Calculate a 32 bit simple sum checksum.
 * \param ptr - Pointer to the memory area to checksum.
 * \param size - Size (bytes), should be a multiple of 4 bytes.
 * \param returns - signed 32 bit sum of the specified data area.
 */
static int32_t cksum(int32_t *ptr, int size)
{
	int32_t sum;
	int j;

	sum = 0;
	for (j= 0; j < (size / sizeof(int32_t)); j++)
		sum += *ptr++;
	return sum;
}

/*
 * Initialize the configuration utilities.
 */
int config_init(void)
{
	struct permcfg_s *permcfg_p = (struct permcfg_s*)PERMCFG_ADDR;
	struct usercfg_s *usercfg_p = (struct usercfg_s*)USERCFG_ADDR;

	if (permcfg_valid())
		permcfg = *permcfg_p;
	else
		permcfg = default_permcfg;

	if (usercfg_valid())
		usercfg = *usercfg_p;
	else
		usercfg = default_usercfg;

	return permcfg_valid();
}

/*
 * Check if the permanent configuration flash is erased.
 */
int permcfg_virgin(void)
{
	uint32_t *p = (uint32_t *)PERMCFG_ADDR;
	int j;

	/*
	 * Check if the save area is erased.
	 */
	for (j= 0; j < (sizeof(struct permcfg_s) / sizeof(int32_t)); j++) {
		if (*p++ != -1) {
			return FALSE;
		}
	}
	return TRUE;
}

/*
 * Check if the permanent configuration area is valid.
 */
int permcfg_valid(void)
{
	return cksum((int32_t *)PERMCFG_ADDR, sizeof(struct permcfg_s)) == -1;
}

/*
 * Check if the user configuration area is valid.
 */
int usercfg_valid(void)
{
	return cksum((int32_t *)USERCFG_ADDR, sizeof(struct usercfg_s)) == -1;
}

/*
 * Save the permanent configuration structure to flash.
 */
int permcfg_save(void)
{
	/*
	 * Verify that the permanent area is erased (unprogrammed).
	 */
	if (!permcfg_virgin())
		return FALSE;
	/*
	 * Make sure the constants are correct.  The checksum sums to -1.
	 */
	permcfg.length = sizeof(struct permcfg_s);
	permcfg.version = -1;
	permcfg.checksum = 0;
	permcfg.checksum =
		-1 - cksum((int32_t *)&permcfg, sizeof(struct permcfg_s));

	/*
	 * FLASH!
	 */
	FlashUsecSet(configCPU_CLOCK_HZ / 1000000);
	FlashProgram((long unsigned int *)&permcfg, PERMCFG_ADDR,
		sizeof(struct permcfg_s));

	return permcfg_valid();
}

/*
 * Save the user configuration structure to flash.
 */
int usercfg_save(void)
{
	uint32_t *p = (uint32_t *)USERCFG_ADDR;
	int j;

	FlashUsecSet(configCPU_CLOCK_HZ / 1000000);

	/*
	 * Check if the save area is erased, erase if not.
	 */
	for (j= 0; j < sizeof(struct usercfg_s) / sizeof(int32_t); j++) {
		if (*p++ != -1) {
			FlashErase(USERCFG_ADDR);
			break;
		}
	}

	/*
	 * Make sure the constants are correct.  The checksum sums to -1.
	 */
	usercfg.length = sizeof(struct usercfg_s);
	usercfg.version = -1;
	usercfg.checksum = 0;
	usercfg.checksum =
		-1 - cksum((int32_t *)&usercfg, sizeof(struct usercfg_s));

	/*
	 * FLASH!
	 */
	FlashProgram((long unsigned int *)&usercfg, USERCFG_ADDR,
		sizeof(struct usercfg_s));

	return usercfg_valid();
}
/** \} */
