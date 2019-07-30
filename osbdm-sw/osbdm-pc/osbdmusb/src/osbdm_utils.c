/* OSBDM-JM60 Target Interface Software Package
 * Copyright (C) 2009  Freescale
 *
 * This software package is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this software; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

//----------------------------------------------------------------------------
//
//
//  FILE
//
//      osbdm_utils.c
// 
//
//  DESCRIPTION
//
//		Utility functions
//
//
//----------------------------------------------------------------------------
//


#ifdef LINUX
#define  __declspec(dllexport)
#else
#include <windows.h>
#endif

#include "osbdm_utils.h"

// reverses high and low words or a 32-bit value
__declspec(dllexport) unsigned long ByteSwap32(unsigned long value){
	unsigned long a, b, c, d;

	a = value << 24;
	b = (value << 8) & 0x00FF0000;
	c = (value >> 8) & 0x0000FF00;
	d = value >> 24;

	return(a + b + c + d);
}

// reverses high and low words or a 32-bit value
__declspec(dllexport) unsigned short ByteSwap16(unsigned short value){
	unsigned short a, b;

	a = value >> 8;
	b = value << 8;

	return(a + b);
}


//   return a 32-bit value taking 4 bytes from an 8-bit buffer, LSB first
__declspec(dllexport) unsigned long BufToUlong(unsigned char *buf){
	unsigned long val=0;
	int i;
	for(i=0; i<4; i++){
		val <<=8;
		val += *(buf++);
	}
	return val;
}

