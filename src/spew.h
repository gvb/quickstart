/**
 * @file spew.h
 *
 * Async serial debug output.
 *
 * @addtogroup io I/O
 * @{
 *//*
 * Copyright (C) 2011 Consolidated Resource Imaging LLC
 *
 *       1         2         3         4         5         6         7         8
 *345678901234567890123456789012345678901234567890123456789012345678901234567890
 *
 */

#ifndef SPEW_H_
#define SPEW_H_

#include "hw_memmap.h"

#ifndef SPEW_SERIAL
#define SPEW_SERIAL 1
#endif

#define SPEW_UART_BASE (UART1_BASE)

/**
 * Initialize the spew package.
 */
void spewInit(void);

/**
 * Put a single character (no port locking).
 */
void spewPutchar(char c);

/**
 * Just like printf, but out the spew port.
 */
void spewPrintf(const char *fmt, ...);

#endif /* USW_H_ */
