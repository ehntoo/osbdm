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
//      osbdm_s12.h
// 
//
//  DESCRIPTION
//
//		OSBDM S12 Specific header
//
//
//----------------------------------------------------------------------------
//
#ifndef __OSBDM_S12_H__
#define __OSBDM_S12_H__

//
// definitions
//

typedef enum {
	BDMSTS_ENBDM_MASK  = 0x80, // R/W
	BDMSTS_BDMACT_MASK = 0x40, // RONLY
	BDMSTS_SDV_MASK    = 0x10, // DO NOT ALTER
	BDMSTS_TRACE_MASK  = 0x08, // DO NOT ALTER
	BDMSTS_CLKSW_MASK  = 0x04, // R/W
	BDMSTS_UNSEC_MASK  = 0x02  // RONLY
}bdmstsBitMask;


//
// S12 register numbers
//
typedef enum {
    s12regID_d,			// 0		
    s12regID_x,					
    s12regID_y,					
    s12regID_sp,					
    s12regID_pc,					
	s12regID_FIRST_DEBUG_REG = 0xFF00, 
    s12regID_bdmsts = 0xFF01,	
	s12regID_bdmccr = 0xFF06,
 }s12regID;

 
#endif //__OSBDM_S12_H__

