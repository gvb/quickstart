/**
 * \file partnum.h
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

#ifndef PARTNUM_H_
#define PARTNUM_H_

#include <stdint.h>

/*
 * Where to store our configuration data.  Store in the last and
 * second-to-last pages in flash.  Pages are 1K in this processor.
 */
#define PERMCFG_ADDR	(0x00040000 - 0x0400)	/**< Permanent config */
#define USERCFG_ADDR	(0x00040000 - 0x0800)	/**< User config */

/**
 * struct permcfg_s - Permanent configuration data structure (write once).
 */
struct permcfg_s {
	int32_t length;		/**< sizeof(struct permcfg_s) */
	int32_t version;	/**< structure version */
	char    bd_pn[64];	/**< string: board part number */
	char    bd_sn[64];	/**< string: board serial number */
	uint8_t mac[8];		/**< first six are used, the rest are padding */
	int32_t checksum;	/**< signed 32 bit sum of the data, totals -1 */
};

/**
 * User modifiable configuration data structure.
 */
struct usercfg_s {
	int32_t length;			/**< sizeof(struct usercfg_s) */
	int32_t version;		/**< structure version */
	char    assy_pn[64];	/**< string: assembly part number */
	char    assy_sn[64];	/**< string: assembly serial number */
	uint8_t ip[4];			/**< IP address */
	uint8_t netmask[4];		/**< IP netmask */
	uint8_t gateway[4];		/**< IP gateway */
	unsigned long IPMode; 	/**< IP Address Mode: STATIC DHCP or AUTO */
	char    notes[256];		/**< free form notes */
	int32_t checksum;		/**< signed 32 bit sum of the data, totals -1 */
};

/*
 * Permanent and user modifiable configurations.
 */
extern struct permcfg_s permcfg;	/**< Permanent configuration data */
extern struct usercfg_s usercfg;	/**< User configuration data */

/*
 * Utilities
 */

/**
 * Initialize the configuration utilities.
 */
int config_init(void);

/**
 * Check if the permanent configuration flash is erased.
 * \returns true => permanent config flash is erased.
 */
int permcfg_virgin(void);

/**
 * Check if the permanent configuration area is valid.
 * \returns true => permanent config in flash is valid.
 */
int permcfg_valid(void);

/**
 * Save the permanent configuration structure to flash.
 * \returns
 *  - TRUE => configuration saved.
 *  - FALSE => configuration not saved.
 *
 * If the permanent configuration contains data, this refuses to save
 * new data over top the existing data.  The "permanent" configuration is
 * "write-once."
 */
int permcfg_save(void);

/**
 * Check if the user configuration area is valid.
 * \returns true => user config in flash is valid.
 */
int usercfg_valid(void);

/**
 * Save the user configuration structure to flash.
 * \returns
 *  - TRUE => configuration saved.
 *  - FALSE => configuration not saved.
 *
 * If the user configuration contains data, this erases it first then
 * stores the new data.
 */
int usercfg_save(void);

/**
 * Erase the permanent configuration structure in flash
 * \returns
 *  - TRUE => perm configuration page erased/blank.
 *  - FALSE => perm configuration page not erased/blank.
 */
int permcfg_erase(void);

#endif /* PARTNUM_H_ */
/** \} */
