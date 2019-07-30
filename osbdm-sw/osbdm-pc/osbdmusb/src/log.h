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
//      log.h
// 
//
//  DESCRIPTION
//
//      Functions related to message log 
//
//----------------------------------------------------



#ifndef __LOG_H__
#define __LOG_H__

#ifdef LOG

extern FILE *log_file;
extern char log_file_open;

#endif


void print(const char *format, ...);
void print_dump(unsigned char *data, unsigned int size);
void open_log_file();
void close_log_file();

#endif // __LOG_H__

