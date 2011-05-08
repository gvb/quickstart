/**
 * \file debugSupport.h
 *
 * debug output.
 *
 * \addtogroup mcm TBDmcm
 * \{
 *//*
 * Copyright (C) 2011 Consolidated Resource Imaging LLC
 *
 *       1         2         3         4         5         6         7         8
 *345678901234567890123456789012345678901234567890123456789012345678901234567890
 *
 */
#ifndef __DEBUG_SUPPORT_INCLUDE_H
#define __DEBUG_SUPPORT_INCLUDE_H

#define DBG_MAX_PRINT_LEVEL 499

#define DPRINTF( PRINT_LEVEL, ... ) if ( PRINT_LEVEL <= DBG_MAX_PRINT_LEVEL) dbg_printf( __FILE__, __LINE__, __FUNCTION__, PRINT_LEVEL, __VA_ARGS__ )

#ifdef __cplusplus
extern "C" {
#endif

extern int   dbg_level;
extern char *dbg_proc_name;

extern void dbg_printf(const char *file,
		const int line,
		const char *function_name,
		const int  print_level,
		const char *fmt, ...);

#ifdef __cplusplus
}
#endif


#endif

/** \} */
