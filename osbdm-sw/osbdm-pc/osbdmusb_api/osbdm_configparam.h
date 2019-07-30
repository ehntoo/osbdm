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
//      osbdm_configparam.h
// 
//
//  DESCRIPTION
//
//		OSBDM configuration parameters and their values
//
//
//----------------------------------------------------------------------------
//
#ifndef __OSBDM_CONFIGPARAM_H__
#define __OSBDM_CONFIGPARAM_H__

//
// definitions
// 

typedef enum ConfigTypeTag {	
    osbdmcfg		=     0x10,		// osbdm-JM60 config
    tgtcfg_generic	=     0x40,		// generic target config 
    tgtcfg_specific	=     0x80		// target specific config
} ConfigT;



#endif //__OSBDM_CONFIGPARAM_H__
