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
//      osbdm_s08.h
// 
//
//  DESCRIPTION
//
//		OSBDM S08 Specific header
//
//
//----------------------------------------------------------------------------
//

#ifndef __OSBDM_S08_H__
#define __OSBDM_S08_H__

//
// definitions
//


//
// S08 register numbers
//
typedef enum {
    s08regID_a,		// 0
    s08regID_ccr,					
    s08regID_hx,					
    s08regID_sp,				
    s08regID_pc,
	s08regID_FIRST_DEBUG_REG = 0x2000, 
    s08regID_bdcscr = 0x2000,	// 5				
    s08regID_bdcbmr				
 }s08regID;
 
#endif //__OSBDM_S08_H__

