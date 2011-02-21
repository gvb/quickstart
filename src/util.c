/**
 * \file util.c
 *
 * Utility functions.
 * - Watchdog timer implementation.
 *
 * \addtogroup util Utility functions
 * \{
 *//*
 * Copyright (C) 2011 Consolidated Resource Imaging LLC
 *
 *       1         2         3         4         5         6         7        
 *3456789012345678901234567890123456789012345678901234567890123456789012345678
 */

#include <string.h>
#include <stdint.h>

#include <FreeRTOS.h>
#include <task.h>

#include <hw_types.h>
#include <hw_memmap.h>
#include <hw_ints.h>
#include <interrupt.h>
#include <watchdog.h>

#include <config.h>
#include <util.h>
#include <io.h>
#include <logger.h>

#define DEBUG	1

#define WDT_RESET_MS	100	/* Watchdog resets us after this */
/*
 * How many clock cycles between WDT interrupts.
 */
#define WDT_INT_CLKS	((configCPU_CLOCK_HZ / (1000 * 2)) * WDT_RESET_MS)

/*
 * Public information.
 */
xTaskHandle util_task_handle;	/**< Utility task handle */

/*
 * Things we are monitoring need to reset their counter to zero
 * periodically, or we reset the system.  The WDT ISR increments
 * the counters atomically, since it is in an ISR, and the tasks
 * reset them to zero atomically since it is a simple store instruction.
 */
int wdt_checkin[wdt_last] = {0};

/**
 * If we rack up more than this count, something has gone awry with the task.
 */
static int wdt_limit[wdt_last] = {
	1000,		/**< io_task: 10 Hz */
	1000,		/**< util_task: 100 Hz */
};


/****************************************************************************/

#define POLL_HZ		100
#define POLL_DELAY	(configTICK_RATE_HZ / POLL_HZ)

#define MSEC2POLL(msec)	((msec) / POLL_DELAY)

/****************************************************************************/

/**
 * Watchdog interrupt service routine.
 *
 * \req \req_wdt The \program \shall implement a watchdog timer.
 * \req \req_wdt If the processor stops responding to interrupts, the
 *   watchdog timer \shall reset it.
 * \req \req_wdt If a monitored task fails to run in a timely fashion, the
 *   watchdog timer \shall reset it.
 */
void wdt_isr(void)
{
	int j;

	for (j = 0; j < wdt_last; j++) {
		if (++wdt_checkin[j] > wdt_limit[j]) {
			lputchar('A' + j);
			return;		/* this will cause us to die */
		}
	}
	/*
	 * Clear the WDT - if we don't do this, we get reset after a
	 * second timeout.
	 */
	WatchdogIntClear(WATCHDOG0_BASE);
}

/****************************************************************************/

/**
 * Utility task.
 *
 * This monitors the system.
 *
 * This also initializes the watchdog so it will reset the processor
 * if Something Goes Wrong[tm].
 *
 * \req \req_wdt The \program \shall implement a watchdog timer.
 */
static void util_task(void *params)
{
	portTickType last_wake_time;

#if (DEBUG > 0)
	lprintf("util_task() running, WDT is %d.\r\n", WDT_INT_CLKS);
#endif

	/*
	 * Registers and enables the watchdog interrupt.
	 */
	WatchdogUnlock(WATCHDOG0_BASE);
	WatchdogIntRegister(WATCHDOG0_BASE, wdt_isr);
	WatchdogIntEnable(WATCHDOG0_BASE);

	/*
	 * Set the watchdog timeout duration (processor clocks).
	 */
	WatchdogReloadSet(WATCHDOG0_BASE, WDT_INT_CLKS);
	/*
	 * Enable the WDT reset and then enable the WDT itself.
	 */
	WatchdogResetEnable(WATCHDOG0_BASE);
	WatchdogEnable(WATCHDOG0_BASE);

	/* Start our periodic time starting in 3. 2. 1. NOW! */
	last_wake_time = xTaskGetTickCount();

	while(1) {	/* forever loop */
		wdt_checkin[wdt_util] = 0;

		vTaskDelayUntil(&last_wake_time, POLL_DELAY);
	}
}

/****************************************************************************/

/*
 * Initialize the utility functions.
 */
int util_init(void)
{
	portBASE_TYPE ret;

	ret = xTaskCreate(util_task,
		(signed portCHAR *)"util",
		DEFAULT_STACK_SIZE,
		NULL,
		UTIL_TASK_PRIORITY,
		&util_task_handle);
	if (ret != pdPASS)
		lprintf("Creation of utilities task failed: %d\r\n", ret);

	return 0;
}

/** \} */
