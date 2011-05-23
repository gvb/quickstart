/**
 * \file config.h
 *
 * Main software configuration file.
 *
 * \addtogroup util Utilities
 * \{
 *
 *//*
 * Copyright (C) 2011 Consolidated Resource Imaging LLC
 *
 *       1         2         3         4         5         6         7
 *3456789012345678901234567890123456789012345678901234567890123456789012345678
 *
 */

#ifndef CONFIG_H_
#define CONFIG_H_

#ifndef TRUE
#define FALSE	0
#define TRUE	(!FALSE)
#endif

/*
 * RTOS configuration choices.
 */
#define IO_TASK_PRIORITY		3	/* I/O polling */
#define WEB_TASK_PRIORITY		2	/* Web server */
#define UIP_TASK_PRIORITY		1	/* ethernet / MAC */
#define UTIL_TASK_PRIORITY		1	/* Utility, including WDT */
#define IDLE_TASK_PRIORITY		0	/* for completeness */

#define DEFAULT_STACK_SIZE		512	/* unless otherwise */
#define IDLE_STACK_SIZE			120
#define WEB_STACK_SIZE			512


/* The period of the system clock in nano seconds. */

#define NS_PER_CLOCK  \
	((unsigned portLONG) \
		((1.0 / (double)configCPU_CLOCK_HZ) * 1000000000.0))

/*
 * Stellaris built-in reference.
 */
#define VREF 		3000	/* millivolts */

#endif /* CONFIG_H_ */
/** \} */
