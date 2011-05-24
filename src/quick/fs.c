/*
 * Copyright (c) 2001-2003 Swedish Institute of Computer Science.
 * All rights reserved. 
 * 
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED 
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT 
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT 
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING 
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 * 
 * Author: Adam Dunkels <adam@sics.se>
 *
 */
#include "lwip/opt.h"
#include "lwip/def.h"
#include "fs.h"
#include "fsdata.h"
#include "fsdata-stats.h"
#include "../../obj/fsdata.c"
#include <string.h>
#include "logger.h"
/*-----------------------------------------------------------------------------------*/
/* Define the number of open files that we can support. */
#ifndef LWIP_MAX_OPEN_FILES
#define LWIP_MAX_OPEN_FILES     10
#endif

/* Define the file system memory allocation structure. */
struct fs_table {
  struct fs_file file;
  int inuse;
};

/* Allocate file system memory */
struct fs_table fs_memory[LWIP_MAX_OPEN_FILES];

#define FS_IDX(f) ((const struct fs_table*)(f)-fs_memory)

#if 0
char *fsIStr(const struct fs_file* f)
{
	static char s[2];
	s[0]='0'+(char)FS_IDX(f);
	s[1]=(char)0;
	return s;
}
#endif

/*-----------------------------------------------------------------------------------*/
static struct fs_file *
fs_malloc(void)
{
  int i;
  for(i = 0; i < LWIP_MAX_OPEN_FILES; i++) {
    if(fs_memory[i].inuse == 0) {
      fs_memory[i].inuse = 1;
      return(&fs_memory[i].file);
    }
  }
  return(NULL);
}

/*-----------------------------------------------------------------------------------*/
static void
fs_free(struct fs_file *file)
{
  int i;
  for(i = 0; i < LWIP_MAX_OPEN_FILES; i++) {
    if(&fs_memory[i].file == file) {
      fs_memory[i].inuse = 0;
      break;
    }
  }
  return;
}

/*-----------------------------------------------------------------------------------*/
/**
 * open a file.
 *
 * Because this is a read only file system for a small efficient
 * webserver, callers expect that they may peruse the returned
 * fs_file structure and
 * have:
 *  f = fs_open_get_access("filename");
 *  f->name
 *  f->data
 *  *(f->data)
 *  f->len
 * remain valid until fs_close(f);.
 *  f->index will change based upon calls fs_read.
 *
 *  This fs_open_get_access operates a the
 *  f=open(),"user may peruse *(f->data)",close(f)
 *  paradigm.  Rather than just the
 *  f=open(),read(f),close(f) paradigm.
 *
 */
struct fs_file *
fs_open_get_access(char *name)
{
  struct fs_file *file;
  const struct fsdata_file *f;

  //lstr("<Ot.");lstr(name);lstr("|");

  file = fs_malloc();
  if(file == NULL) {
	//lstr("X>");
    return NULL;
  }

  for(f = FS_ROOT; f != NULL; f = f->next) {
    if (!strcmp(name, (char *)f->name)) {
      file->data = (char *)f->data;
      file->len = f->len;
#if !USER_PROVIDES_ZERO_COPY_STATIC_TAGS
      file->index = 0;  // was: f->len;  shouldbe 0
#endif
#if USE_PEXTENSION
      file->pextension = NULL;
#endif
      //lstr(fsIStr(file));
      //lstr(name);lstr(">,name=<");lstr(f->name);
      //lstr(">,file=");lhex((int)file);lstr(",data=");lhex(file->data);
      //lstr("|len=");lhex(file->len);
      //lstr(",index=");lhex(file->index);
      //lstr(">");
      return file;
    }
  }
  //lstr("n>");
  fs_free(file);
  return NULL;
}

/*-----------------------------------------------------------------------------------*/
void
fs_close(struct fs_file *file)
{
  //lstr("c<");lstr(fsIStr(file));lstr(">");
  fs_free(file);
}
/*-----------------------------------------------------------------------------------*/
#if !USER_PROVIDES_ZERO_COPY_STATIC_TAGS
int
fs_read(struct fs_file *file, char *buffer, int count)
{
  int read;

  //lstr("r<");lstr(fsIStr(file));lstr("|");
  //lhex((int)buffer);lstr("|");lhex(count);lstr("|");
  //lstr("READ:file=");lhex((int)file);lstr(",data=");lhex(file->data);
  //lstr(",len=");lhex(file->len);lstr(",index=");lhex(file->index);
  //lstr(",count=");lhex(count);crlf();

  if(file->index == file->len) {
	  //lstr("READ@end:file=");lhex((int)file);lstr(",data=");lhex(file->data);
	  //lstr(",len=");lhex(file->len);lstr(",index=");lhex(file->index);
	  //lstr(",read=");
	  //lstr("-1");
	  //crlf();
	  //lstr(">");
    return -1;
  }

  read = file->len - file->index;
  if(read > count) {
    read = count;
  }

  memcpy(buffer, (file->data + file->index), read);
  file->index += read;

  //lhex(read);lstr(">");
  //lstr("READ@end:file=");lhex((int)file);lstr(",data=");lhex(file->data);
  //lstr(",len=");lhex(file->len);lstr(",index=");lhex(file->index);
  //lstr(",read=");lhex(read);crlf();
  return(read);
}
#endif
