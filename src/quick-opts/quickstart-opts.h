/*
 * quickstart-opts.h
 *
 *  Created on: May 13, 2011
 *      Author: cri_user
 */

#ifndef QUICKSTARTOPTS_H_
#define QUICKSTARTOPTS_H_

#define USE_PROGRAM_STARTUP 0

#if (PART == LM3S2110)
#define QUICK_ETHERNET 0
#else
#define QUICK_ETHERNET 1
#endif

#ifndef PROTECT_PERMCFG
#define PROTECT_PERMCFG 1
#endif

#ifndef ERASE_PERMCFG
#define ERASE_PERMCFG 0
#endif

#endif /* QUICKSTARTOPTS_H_ */
