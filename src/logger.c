/**
 * \file logger.c
 *
 * Logger (debug) output.
 *
 * \addtogroup io I/O
 * \{
 *//*
 * Copyright (C) 2011 Consolidated Resource Imaging LLC
 *
 *       1         2         3         4         5         6         7
 *345678901234567890123456789012345678901234567890123456789012345678901234567
 *
 */

#include <stdio.h>
#include <stdarg.h>
#include <ustdlib.h>	/* vsnprintf() */

#include <FreeRTOS.h>
#include <semphr.h>

#include "hw_memmap.h"
#include "hw_types.h"
#include "hw_sysctl.h"
#include "sysctl.h"
#include "gpio.h"
#include "uart.h"

#include "logger.h"

/**
 * Which UART to use for the logger function.
 * - UART0 is the virtual comm port supplied by the USB debugger cable.`
 */
#define LOGGER_UART_BASE (UART0_BASE)

/**
 * Mutual exclusion to make logger multitask-safe.
 */
static xSemaphoreHandle loggerMutex;

/**
 * Buffer to hold the vsnprintf() output string.
 */
static char a[128];

/****************************************************************************/

void init_logger(void)
{
	/*
	 * Setup UART for Spew
	 */
	UARTConfigSetExpClk(LOGGER_UART_BASE, SysCtlClockGet(), 115200,
			( UART_CONFIG_WLEN_8
			| UART_CONFIG_STOP_ONE
			| UART_CONFIG_PAR_NONE));
	UARTEnable(LOGGER_UART_BASE);

	loggerMutex = xSemaphoreCreateMutex();
}

void lputchar(char c)
{
	UARTCharPut(LOGGER_UART_BASE, c);
}

int lgetchar(void)
{
	return UARTCharGetNonBlocking(LOGGER_UART_BASE);
}

void lhex(unsigned long hex)
{
	int i;

	for (i=0; i<8; i++) {
		/*
		 * Pull off a nibble, most significant first.
		 */
		unsigned int nibble = hex >>28;
		/*
		 * Convert the nibble from int to ASCII char
		 * relying that ASCII defines
		 * '0' to '9'
		 *  and also
		 *    'a' to 'f'
		 *  in increasing sequential order
		 */
		nibble += (unsigned int)'0';
		if (nibble > (unsigned int)'9')
			nibble += (unsigned int)'a' - (unsigned int)'9' - 1U;

		UARTCharPut(LOGGER_UART_BASE, (unsigned char)nibble);

		/*
		 * Get the next most significant nibble;
		 */
		hex<<=4;
	}

	/*
	 * Append a trailing space for ease of use.
	 */
	UARTCharPut(LOGGER_UART_BASE, ' ');
}

void lstr(const char *p)
{
	while (*p) {
		if (*p=='\n') {
			UARTCharPut(LOGGER_UART_BASE, '\r');
		}
		UARTCharPut(LOGGER_UART_BASE, *p);
		p++;
	}
}

void crlf(void){
	lstr("\n");
}

void lprintf(const char *fmt, ...)
{
	va_list argptr;
	va_start(argptr, fmt);

	xSemaphoreTake(loggerMutex, portMAX_DELAY);

	a[0] = '\0';

	vsnprintf(a, sizeof(a), fmt, argptr);
	lstr(a);

	xSemaphoreGive(loggerMutex);
}
/** \} */
