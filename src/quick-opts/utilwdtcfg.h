/*
 * utilwdtcfg.h
 *
 *  Created on: May 23, 2011
 *      Author: mikes
 */

#ifndef __UTILWDTCFG_H__
#define __UTILWDTCFG_H__

extern int wdt_limit[];

/**
 * Things we are monitoring.
 */
enum wdt_monitor_e {
	wdt_io,		/**< I/O task */       //!< wdt_io
	wdt_util,	/**< Utility task */  //!< wdt_util
	wdt_last	/**< last entry flag *///!< wdt_last
};

#endif
