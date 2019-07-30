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
//      osbdm_utils.h
// 
//
//  DESCRIPTION
//
//		Utility functions
//
//
//----------------------------------------------------------------------------
//

#ifndef __OSBDM_UTILS_H__
#define __OSBDM_UTILS_H__


#ifdef LINUX
#define  __declspec(dllexport)
#endif

__declspec(dllexport) unsigned long		ByteSwap32(unsigned long value);
__declspec(dllexport) unsigned short	ByteSwap16(unsigned short value);
__declspec(dllexport) unsigned long		BufToUlong(unsigned char *buf);
#endif //__OSBDM_UTILS_H__
