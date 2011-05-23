/**
 * \file debugSupport.c
 *
 * debug output.
 *
 * \addtogroup TBD
 * \{
 *//*
 * Copyright (C) 2011 Consolidated Resource Imaging LLC
 *
 *       1         2         3         4         5         6         7         8
 *345678901234567890123456789012345678901234567890123456789012345678901234567890
 *
 */

#include <stdarg.h>
#include <string.h>

#include "FreeRTOS.h"
#include "task.h"

#include "logger.h"

int   dbg_level = 0;
char *dbg_proc_name = "*";

void dbg_printf(const char *file,
		const int line,
		const char *function_name,
		const int  print_level,
		const char *fmt, ...)
{
	va_list argptr;

	va_start(argptr, fmt);

	/*
	 * print the debug information when
	 *         print_level is 0
	 *      or debug level is positive
	 *         print everything below the debug level
     *      or debug level is negative
     *           print just what equals abs( debug level )
	 */
	if ( 	  (print_level==00)
	        ||((dbg_level>=0) && (print_level <=  dbg_level))
	        ||((dbg_level<0)  && (print_level == -dbg_level)) ){

		portTickType now;
		now = xTaskGetTickCount();

        /*
         * pull of the directories out of file name
         */
		char *name = strrchr( file, '/');
        name = name ? name+1 : (char *) file;

		lprintf( "%010u:%s:%10s:%04d:%10s - ",
				(unsigned int)now, dbg_proc_name, name,
				line, function_name);
		lprintf(fmt, argptr);
		lprintf("\n");
	}

}
/** \} */
