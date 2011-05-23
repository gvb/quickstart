/*
 * utilwdtcfg.c
 *
 *  Created on: May 23, 2011
 *      Author: mikes
 */

/**
 * If we rack up more than this count, something has gone awry with the task.
 */
static int wdt_limit[wdt_last] = {
	1000,		/**< io_task: 10 Hz */
	1000,		/**< util_task: 100 Hz */
};
