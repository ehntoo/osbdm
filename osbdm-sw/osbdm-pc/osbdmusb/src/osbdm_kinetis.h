/* OSBDM-JM60 Windows USB Library
 * Copyright (C) 2011  P&E Microcomputer Systems, Inc.
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
//      osbdm_kinetis.h
// 
//
//  DESCRIPTION
//
//		OSBDM Kinetis Specific header
//
//
//----------------------------------------------------------------------------
//

#ifndef __OSBDM_KINETIS_H__
#define __OSBDM_KINETIS_H__

//
// definitions
//
#define Reg_CoreDebug_DHCSR_address      0xE000EDF0
#define Reg_CoreDebug_DCRSR_address      0xE000EDF4
#define Reg_CoreDebug_DCRDR_address      0xE000EDF8
#define Reg_CoreDebug_DEMCR_address      0xE000EDFC

//
// Kinetis register numbers
//



// for MEM_REG register addresses 0 - 95 :  Read/Write Core Registers
//
// for MEM_REG register address == 1000  :  Read/Write AP MDM Status Register
//
// for MEM_REG register address == 1001  :  Read/Write AP MDM Control Register


#endif //__OSBDM_S08_H__

