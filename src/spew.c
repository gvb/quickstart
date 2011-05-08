/**
 * \file spew.c
 *
 * Async serial debug output.
 *
 * \addtogroup io I/O
 * \{
 *//*
 * Copyright (C) 2011 Consolidated Resource Imaging LLC
 *
 *       1         2         3         4         5         6         7         8
 *345678901234567890123456789012345678901234567890123456789012345678901234567890
 *
 */

#include <stdio.h>
#include <stdarg.h>
#include <ustdlib.h>	/* vsnprintf() */

#include <FreeRTOS.h>
//#include <task.h>
#include <semphr.h>

#include "hw_memmap.h"
#include "hw_types.h"
#include "hw_sysctl.h"
#include "sysctl.h"
#include "gpio.h"
#include "uart.h"

#include "spew.h"

/**
 * Mutual exclusion to make spew multitask-safe.
 */
static xSemaphoreHandle spewMutex;
/**
 * Buffer to hold the vsnprintf() output string.
 */
static char a[128];

/****************************************************************************/

void spewInit(void)
{
	spewMutex = xSemaphoreCreateMutex();
}

void spewPutchar(char c)
{
	UARTCharPut(SPEW_UART_BASE, c);
}

void spewPrintf(const char *fmt, ...)
{
	char *p = a;
	va_list argptr;

	xSemaphoreTake(spewMutex, portMAX_DELAY);

	a[0] = '\0';

	va_start(argptr, fmt);
	vsnprintf(a, sizeof(a), fmt, argptr);

	while (*p) {
		UARTCharPut(SPEW_UART_BASE, *p);
		p++;
	}
	xSemaphoreGive(spewMutex);
}
/** \} */
