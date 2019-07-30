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
//      osbdm_rs08.h
// 
//
//  DESCRIPTION
//
//		OSBDM RS08 Specific header
//
//
//----------------------------------------------------------------------------
//

#ifndef __OSBDM_RS08_H__
#define __OSBDM_RS08_H__

//
// definitions
//


//
// RS08 register numbers
//


//
// definitions
//


//
// RS08 register numbers
//
typedef enum rs08regIDTag {
    rs08regID_a,		// 0	
	rs08regID_ccr,
    rs08regID_spc =3,
	rs08regID_pc,		// 4
 	rs08regID_x =		0xf,	
	rs08regID_page =	0x1f,
	rs08regID_FIRST_DEBUG_REG = 0x2000, 
    rs08regID_bdcscr = 0x2000,					
    rs08regID_bdcbmr				
 }rs08regID;
 

typedef enum rs08cfgTag {
	rs08cfg_enable_vpp_powersupply,  // set to 1 to enable the flash programming voltage power supply
									 // set to 0 to disable the flash programming voltage power supply
	rs08cfg_enable_vpp,				 // set to 1 to turn ON the flash programming voltage to the target
									 // set to 0 to turn OFF the flash programming voltage to the target
	rs08cfg_power_reset_on_connect,   // set to 1 to enable Power Reset on connect.

	rs08cfg_count
} rs08cfgType;

#endif //__OSBDM_RS08_H__

