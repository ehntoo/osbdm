/* OSBDM-JM60 Windows USB Library
 * Copyright (C) 2009  Freescale
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

//----------------------------------------------------------------------------
//
//
//  FILE
//
//      log.c
// 
//
//  DESCRIPTION
//
//      Functions related to message log 
//
//----------------------------------------------------



#include <windows.h>
#include <stdio.h>
#include "log.h"
#ifdef LOG
	FILE *log_file=NULL;
char log_file_open=0;
#endif
#define LOGPATH	".\\osbdm.log"

/* provides print function which prints data into a log file */

#ifdef LOG
void open_log_file(){
	char path[MAX_PATH];
	if(log_file_open == 0){
		strcpy(path, LOGPATH);
		log_file=fopen(path,"wb");
		if(log_file != NULL){
			log_file_open = 1;
			print("Log file path: %s\r\n",path);
		}
	}
}
void close_log_file(){
	if(log_file_open != 0){
		fclose(log_file);
		log_file_open = 0;
	}
}
void print(const char *format, ...) {
	va_list list;
	if(log_file_open == 0)	return;
	va_start(list, format);
//		fprintf(log_file, "%04.3f: ",1.0*GetTickCount()/1000);
	vfprintf(log_file, format, list);
	va_end(list);
}
void print_dump(unsigned char *data, unsigned int size) {
	unsigned int i=0;
	if(log_file_open == 0)	return;
	while(size--) {
		fprintf(log_file,"%02X ",*(data++));
		if (((++i)%16)==0) fprintf(log_file,"\r\n");
	}
	if (((i)%16)!=0) fprintf(log_file,"\r\n");
}
#else
	void open_log_file(){
		return;
	}
	void close_log_file(){
		return;
	}
	void print(const char *format, ...) {
		return;
	}
	void print_dump(unsigned char *data, unsigned int size) {
		return;
	}
#endif

