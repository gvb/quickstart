/**
 * \file util.h
 *
 * Utility functions.
 * - Watchdog timer implementation.
 *
 * \addtogroup util Utility functions
 * \{
 *
 *//*
 * Copyright (C) 2011 Consolidated Resource Imaging LLC
 *
 *       1         2         3         4         5         6         7        
 *3456789012345678901234567890123456789012345678901234567890123456789012345678
 */

#ifndef UTIL_H_
#define UTIL_H_

/*
 * Public information.
 */
extern xTaskHandle wdt_task_handle;	/**< Watchdog task handle */

/**
 * Things we are monitoring.
 */
enum wdt_monitor_e {
	wdt_io,		/**< I/O task */
	wdt_util,	/**< Utility task */
	wdt_last	/**< last entry flag */
};

/**
 * Things we are monitoring need to reset their counter to zero
 * periodically, or we reset the system.  The WDT ISR increments
 * the counters atomically, since it is in an ISR, and the tasks
 * reset them to zero atomically since it is a simple store instruction.
 */
extern int wdt_checkin[];

#endif /* UTIL_H_ */
/** \} */
