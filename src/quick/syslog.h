/**
 * \file syslog.h
 *
 * System Log definitions and declarations.
 *
 * \addtogroup syslog System Log
 * \{
 *//*
 * Copyright (C) 2011 Consolidated Resource Imaging LLC
 *
 *       1         2         3         4         5         6         7
 *3456789012345678901234567890123456789012345678901234567890123456789012345678
 */

#ifndef SYSLOG_H_
#define SYSLOG_H_

#include <lwip/netif.h>

enum facility_vals {
	facility_kern = 0,
	facility_user,
	facility_mail,
	facility_daemon,
	facility_auth,
	facility_syslog,
	facility_lpr,
	facility_news,
	facility_uucp,
	facility_cron,
	facility_authpriv,
	facility_ftp,
	facility_local0 = 16,
	facility_local1,
	facility_local2,
	facility_local3,
	facility_local4,
	facility_local5,
	facility_local6,
	facility_local7
};

enum level_vals {
	level_emerg = 0,
	level_alert,
	level_crit,
	level_err,
	level_warning,
	level_notice,
	level_info,
	level_debug
};

void syslogInit(void);
void syslog(enum facility_vals fac, enum level_vals lev, char * fmt, ...);

#endif
