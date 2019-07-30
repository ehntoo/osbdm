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
/*
* 03/08/2011  : Modified by P&E Microcomputer Systems
*               http://www.pemicro.com/osbdm
*
*
*/


//----------------------------------------------------------------------------
//
//
//  FILE
//
//      osbdm_api.h
// 
//
//  DESCRIPTION
//
//		OSBDM USB driver API
//
//
//
//----------------------------------------------------------------------------
//

#ifndef __OSBDM_API_H__
#define __OSBDM_API_H__


#ifdef LINUX
#define  __declspec(dllexport)
#endif

//
// OSBDM USB Driver Definitions
//
#include "osbdm_def.h"		

//
// Interface
//
__declspec(dllexport) unsigned char		osbdmAPI_api_version(void); 						// returns OSBDM Driver dll version in BCD format
__declspec(dllexport) OsbdmErrT			osbdmAPI_open(unsigned char device_no);
__declspec(dllexport) OsbdmErrT			osbdmAPI_close(void);
__declspec(dllexport) OsbdmErrT			osbdmAPI_init(unsigned char *device_count);
__declspec(dllexport) OsbdmErrT			osbdmAPI_connect(CoreT core_type);
__declspec(dllexport) OsbdmErrT			osbdmAPI_osbdmJM60_version(unsigned int *version);			// returns version of HW (MSB) and SW (LSB) of the Firmware USB interface in BCD format 
__declspec(dllexport) OsbdmErrT			osbdmAPI_get_status(ConnectStateT *status);
__declspec(dllexport) OsbdmErrT			osbdmAPI_get_speed(unsigned long *speed);			// speed in KHz
__declspec(dllexport) OsbdmErrT			osbdmAPI_set_speed(unsigned long speed);			// speed in KHz
__declspec(dllexport) OsbdmErrT			osbdmAPI_stop(void);
__declspec(dllexport) OsbdmErrT			osbdmAPI_run(void);
__declspec(dllexport) OsbdmErrT			osbdmAPI_step(void);
__declspec(dllexport) OsbdmErrT			osbdmAPI_reset(ResetT type);
__declspec(dllexport) OsbdmErrT			osbdmAPI_read_mem(unsigned char mem_space, unsigned int addr, unsigned int byte_count, SizeT access_size, unsigned char *buffer); 
__declspec(dllexport) OsbdmErrT			osbdmAPI_write_mem(unsigned char mem_space, unsigned int addr, unsigned int byte_count, SizeT access_size, unsigned char *buffer);
__declspec(dllexport) OsbdmErrT			osbdmAPI_read_reg(unsigned int regid, SizeT regsize, unsigned long *value);    // read core register
__declspec(dllexport) OsbdmErrT			osbdmAPI_write_reg(unsigned int regid,  SizeT regsize, unsigned long value);	// write core register
__declspec(dllexport) OsbdmErrT		    osbdmAPI_config(unsigned char config_type, unsigned char config_param, unsigned long param_value);

__declspec(dllexport) OsbdmErrT	osbdmAPI_exchange_special_feature_command(unsigned char sub_cmd_num, unsigned int *length, unsigned char *data);

__declspec(dllexport) OsbdmErrT			osbdmAPI_core_mode(CoreModeT *core_mode);  
__declspec(dllexport) OsbdmErrT			osbdmAPI_secure_mode(SecureModeT *secure_mode);  

#endif //__OSBDM_API_H__
