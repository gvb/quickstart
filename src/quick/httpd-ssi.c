/*
 * httpd-ssi.c
 *
 *  Created on: May 22, 2011
 *      Author: mikes
 */

#include <ustdlib.h>
#include <stdio.h>
#include <string.h>

#include <lwip/opt.h>
#include <lwip/debug.h>
#include <lwip/stats.h>
#include <httpd.h>
#include <httpd-ssi.h>

#include <fs.h>
#include <fsdata.h>
#include <fsdata-stats.h>

#define NUM_SSI_FUNCTIONS 5
#define NUM_SSI_ENTRIES (NUM_SSI_FUNCTIONS+FS_NUMFILES)

int fun1(char **pcInsert)
{
	*pcInsert = "1234567890";
	return strlen(*pcInsert);
}

int fun2(char **pcInsert)
{
	*pcInsert = "-userconfig-";
	return strlen(*pcInsert);
}

// \todo fix this hack of moving the CGI commands to SSI.
extern int perm_config(int index, int iNumParams,
		char *pcParam[], char *pcValue[], char **resultBuffer);

extern int user_config(int index, int iNumParams,
		char *pcParam[], char *pcValue[], char **resultBuffer);

extern int run_time(int index, int iNumParams,
		char *pcParam[], char *pcValue[], char **resultBuffer);

int perm_config_ssi(char **pcInsert)
{
	return perm_config(0, 0, NULL, NULL, pcInsert );
}

int user_config_ssi(char **pcInsert)
{
	return user_config(0, 0, NULL, NULL, pcInsert );
}

int run_time_ssi(char **pcInsert)
{
	return run_time(0, 0, NULL, NULL, pcInsert );
}

int (*ssiFunctions[NUM_SSI_FUNCTIONS])(char **pcInsert) = {
	perm_config_ssi,
	user_config_ssi,
	run_time_ssi,
	fun1,
	fun1
};

const char *ssiTags[NUM_SSI_ENTRIES] = {
		"permconfig",
		"userconfig",
		"runtime",
		"4",
		"5"
};

//*****************************************************************************
//
//! Handle a server side include.
//! \param TBD
//!
//! This function
//!
//! \return Number of characters copied to pcInsert.
//
//*****************************************************************************

int SSIHandler(int iIndex, char **pcInsert)
{
	if (iIndex<NUM_SSI_FUNCTIONS) {
		return ssiFunctions[iIndex](pcInsert);
	}
	else if (iIndex<NUM_SSI_ENTRIES) {
			struct fs_file *fs;

			fs = fs_open((char *)ssiTags[iIndex]);
			if (fs) {
				/*
				 * A read of the file without a call to read
				 */
				int len = fs->len;
				*pcInsert = fs->data;
				fs_close(fs);
				return len;
			}
	}

	*pcInsert = "";
	return 0;
}

void init_ssi_handlers(void)
{
	int i;
	const struct fsdata_file *f = FS_ROOT;

    LWIP_ASSERT( "Tables must be same size.",
    		sizeof(ssiFunctions) == sizeof(ssiTags) );

	/*
	 * Append the SSI File names to the ssiTags list.
	 */

	for(i=NUM_SSI_FUNCTIONS;i<NUM_SSI_ENTRIES;i++){
		LWIP_ASSERT("(f)", (f));
		if (f) {
			ssiTags[i] = (char *)f->name;
			f = f->next;
		}
		else {
			ssiTags[i] = "";
		}
	}

	http_set_ssi_handler(SSIHandler, ssiTags, NUM_SSI_ENTRIES);

}
