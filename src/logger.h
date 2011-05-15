/**
 * \file logger.h
 *
 * Async serial debug output.
 *
 * \addtogroup io I/O
 * \{
 *//*
 * Copyright (C) 2011 Consolidated Resource Imaging LLC
 *
 *       1         2         3         4         5         6         7
 *3456789012345678901234567890123456789012345678901234567890123456789012345678
 *
 */

#ifndef LOGGER_H_
#define LOGGER_H_

/**
 * Initialize the logger package.
 */
void init_logger(void);

/**
 * Get a single character
 * - This does \em not do port locking.
 * - This does \em not block
 *
 * \return -1 if no character is available.
 */
int lgetchar(void);

/**
 * Put a single character (no port locking).
 * - This does \em not do port locking.
 *
 * \param c The character to put out the port.
 */
void lputchar(char c);

/**
 * Just like printf, but out the logger port.
 *
 * \param fmt The printf format string.
 * \param ... The variable number of args for the fmt string.
 */
void lprintf(const char *fmt, ...);

/**
 * Like puts, but no trailing newline, and out the logger port.
 * May be called from an ISR.
 *
 * '\n' characters are expanded to '\r' '\n'.
 *
 * \param p The null terminated string to send out the logger port.
 */
void lstr(const char *p);

/**
 * Put a 8 nibble hex string out the logger port.
 * May be called from an ISR.
 *
 * \param hex the integer to convert to ASCII hex and put out the logger port.
 */
void lhex(unsigned long hex);

/**
 * Put a Carriage Return and New Line out the log port;
 * May be called from an ISR.
 *
 */
void crlf(void);


#endif /* LOGGER_H_ */
