/**
 * \file util.c
 *
 * Utility functions.
 * - Watchdog timer implementation.

\page utilpage1 Utilities Overview

\addindex Watchdog Timer

\section wdt Watchdog Timer

A watchdog timer (WDT) is very useful, monitoring the correct execution of
the program and resetting the system if something goes awry.  While they
are not a panacea, they form a useful layer in the "defense in depth"
philosophy of making system resiliant to failure.

The simplest way to handle a watchdog timer is to reset it periodically.
If this is done in an interrupt service routine (ISR), it guarantees that
interrupts are working, but gives no guarantee that the tasks are
operating properly.  If this is done in a "watchdog reset" task which
is run at a very low priority, it ensures that the system will be reset if:
-# Interrupts are not handled correctly.  If the timer tick interrupt stops
     happening, tasking will likely stop functioning properly, starving
     the watchdog timer reset task and causing the timer to expire and
     reset the system.
-# Higher prioritiy (worker) tasks get "confused" and do not complete.
     If this occurs, the malfunctioning higher priority task will starve
     the watchdog timer task, again causing the timer to expire and reset
     the system.

There is also a tradeoff with watchdog timers: how long should the timer
be?  If it is short, it will reset and recover the system quickly in the
case of a failure, but it will cause more overhead and cause problems
if parts of the system really \em are slow relative to the timer period.

The Stellaris watchdog timer causes an interrupt when it times out.
If the watchdog is \em not reset, the \em next watchdog interrupt will
reset the system.  Thus the fault discovery time is twice the watchdog
timer setting.

This program takes a hybrid approach.  When the watchdog interrupt fires,
the watchdog is normally reset in the interrupt service routine.  Thus,
if interrupts fail, the system will be reset.  Layered on top of this
is a task monitoring system.  Each task that is monitored has a counter
that is held in an array, indexed by an emun.  The WDT ISR increments
the counter array on every interrupt and verifies that it does \em not
exceed a threshold value.  Each monitored task resets its counter to
zero every time through its "main loop."  If a task stops running, the
WDT ISR will see the count increment past the error detection threshold
and reset the system.

Notes:
- Since the increment is done in the WDT ISR, the interrupt handling
    becomes our atomic locking mechanism: the read/modify/write
    operation is atomic because a task cannot interrupt an interrupt
    service routine.
- Zeroing an integer (the operation done in a monitored task) is atomic
    because it is a single memory write operation.  There is no race
    condition with respect to the WDT ISR because the zeroing will either
    happen before the WDT ISR runs or after: there is no possibility that
    the WDT ISR will "split" a zeroing operation or that the zeroing will
    occur during the WDT ISR read/modify/write incrementing operation.

The following snippets of code were extracted via objdump:
\verbatim
arm-none-eabi-objdump -Sd obj/util.o | less
\endverbatim

This is the main watchdog timer task loop.  It illustrates the zeroing
of the watchdog counter array, followed by a timed delay.  Note that
the zeroing is an atomic operation: the "str r2, [r3, #4]" is a single
uninterruptable instruction.
\code
        while(1) {
                wdt_checkin[wdt_util] = 0;
  44:   4b06            ldr     r3, [pc, #24]   ; (60 <util_task+0x60>)
  46:   2200            movs    r2, #0
  48:   605a            str     r2, [r3, #4]

                vTaskDelayUntil(&last_wake_time, POLL_DELAY);
  4a:   a801            add     r0, sp, #4
  4c:   210a            movs    r1, #10
  4e:   f7ff fffe       bl      0 <vTaskDelayUntil>
  52:   e7f7            b.n     44 <util_task+0x44>
  54:   002625a0        .word   0x002625a0
  58:   0000002d        .word   0x0000002d
\endcode

The watchdog timer interrupt service routine (ISR) loops through
the counters, checking to see if a counter exceeds the limit, and
incrementing the counters as it goes.  Note that the increment operation
is a read/modify/write (three separate operations) that need to be atomic.
Because of the nature of our system, this is atomic because it is an ISR.
\code
void wdt_isr(void)
{
        int j;

        for (j = 0; j < wdt_last; j++) {
                if (++wdt_checkin[j] > wdt_limit[j]) {
   0:   4b0b            ldr     r3, [pc, #44]   ; (30 <wdt_isr+0x30>)
   2:   681a            ldr     r2, [r3, #0]
   4:   3201            adds    r2, #1
   6:   f5b2 7f7a       cmp.w   r2, #1000       ; 0x3e8
   a:   601a            str     r2, [r3, #0]
   c:   dd01            ble.n   12 <wdt_isr+0x12>
   e:   2000            movs    r0, #0
  10:   e00a            b.n     28 <wdt_isr+0x28>
  12:   685a            ldr     r2, [r3, #4]    ; Read current count
  14:   3201            adds    r2, #1          ; Increment the count
  16:   f5b2 7f7a       cmp.w   r2, #1000       ; Check for a timeout
  1a:   605a            str     r2, [r3, #4]    ; Write the count back
  1c:   dc03            bgt.n   26 <wdt_isr+0x26>
\endcode

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
