/*
 * timerconfig.h
 *
 *  Created on: May 13, 2011
 *      Author: cri_user
 */

#ifndef TIMERCONFIG_H_
#define TIMERCONFIG_H_

#include "FreeRTOS.h"

/* The set frequency of the interrupt.  Deviations from this are measured as
the jitter. */
#define timerINTERRUPT_FREQUENCY		( 20000UL )

/* The expected time between each of the timer interrupts - if the jitter was
zero. */
#define timerEXPECTED_DIFFERENCE_VALUE	( configCPU_CLOCK_HZ / timerINTERRUPT_FREQUENCY )

/* The highest available interrupt priority. */
#define timerHIGHEST_PRIORITY			( 0 )

/* Misc defines. */
#define timerMAX_32BIT_VALUE			( 0xffffffffUL )
#define timerTIMER_1_COUNT_VALUE		( * ( ( unsigned long * ) ( TIMER1_BASE + 0x48 ) ) )

#define GET_TIME_USEC() (timerTIMER_1_COUNT_VALUE / (configCPU_CLOCK_HZ/1000000) )

#endif /* TIMERCONFIG_H_ */
