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
//      osbdm_api.c
// 
//
//  DESCRIPTION
//
//		DSC OSBDM opensourcebdm_jtag USB driver dll extensions
//
//
//
//----------------------------------------------------------------------------
//

#ifdef LINUX
#define  __declspec(dllexport)
#else
#include <windows.h>
#endif

#include "osbdm_base.h"
#include "osbdm_api.h"
#include "commands.h"
#include "osbdm_usb.h"

#include "log.h"  //define symbol "LOG" during compilation to produce a log file.

//
// OSBDM USB driver API target specific definitions
//

#include "osbdm_cfv234.h"	// Coldfire v2-4
#include "osbdm_cfv1.h"		// Coldfire v1 
#include "osbdm_s08.h"		// HCS08 
#include "osbdm_rs08.h"		// RS08 
#include "osbdm_s12.h"		// HCS12
#include "osbdm_dsc.h"		// DSC
#include "osbdm_kinetis.h"	// KINETIS

OsbdmInfoT gOSBDMInfo = {{RESET_INACTIVE, NO_CONNECTION, 0, FP_UNKNOWN}, eCoreTypeUnknown};

// 
// local declarations
//

typedef struct {
	OsbdmMemT	regtype;
	unsigned int regid;
	SizeT regsize;
} InternalReginfoT;

OsbdmErrT map_to_internal_reginfo(unsigned int regid, InternalReginfoT *reginfo);
OsbdmErrT dsc_read_mem(dscmem memType, unsigned int address, unsigned int numBytes, unsigned int accessType, unsigned char *buffer);
OsbdmErrT dsc_write_mem(dscmem memType, unsigned int address, unsigned int numBytes, unsigned int accessType, unsigned char *buffer);


__declspec(dllexport) unsigned char	osbdmAPI_api_version(void){
	return osbdm_DLL_VERSION();
}

__declspec(dllexport) OsbdmErrT osbdmAPI_open(unsigned char device_no){
	return osbdm_open(device_no);
}

__declspec(dllexport) OsbdmErrT osbdmAPI_close(void){
	osbdm_close();
	return osbdm_error_ok;
}

// USB is initialized in DLLMAIN() (see "osbdm_dll.c")
// Find the USB devices and returns number of devices found
__declspec(dllexport) OsbdmErrT osbdmAPI_init(unsigned char *device_count){
	*device_count = osbdm_init();
	if (*device_count == 0 || *device_count == 0xFF)
		return osbdm_error_usb_transport;
	else
		return osbdm_error_ok;
}

// Initialize OSBDM connection.
__declspec(dllexport) OsbdmErrT osbdmAPI_connect(CoreT core){
	OsbdmErrT retErr;

	if ((retErr = osbdm_init_hardware())) return retErr;

	gOSBDMInfo.coreid = core;
	retErr = osbdmAPI_get_status(&(gOSBDMInfo.connect_state));
	if (retErr && (retErr != osbdm_error_not_supported))
		return retErr;

	return osbdm_error_ok;
}

// Get version of the interface (HW and SW) in BCD format.
__declspec(dllexport) OsbdmErrT osbdmAPI_osbdmJM60_version(unsigned int *version){
	OsbdmErrT retErr;
	unsigned char data[20];

	retErr = osbdm_get_version(data);
	if (retErr != osbdm_error_ok) 
		return retErr;
		
	// VERSION = (VERSION_HW+256*VERSION_SW):  intel endianism
	*version = data[2] + (0x100 * data[3]); 
	
	return osbdm_error_ok;
}

__declspec(dllexport) OsbdmErrT osbdmAPI_get_status(ConnectStateT *bdm_status)
{
	OsbdmErrT retErr;
	unsigned char version_info[20], data[20];
	unsigned int version;

	retErr = osbdm_get_version(version_info);
	if (retErr != osbdm_error_ok) 
		return retErr;

	// VERSION = (VERSION_HW+256*VERSION_SW):  intel endianism
	version = version_info[2] + (0x100 * version_info[3]);

	bdm_status->osbdmJM60_version = version;
	bdm_status->reset_state = RESET_UNDEFINED;
	bdm_status->connect_state = CONNECT_UNDEFINED;
	bdm_status->flash_state = FP_UNKNOWN;
	bdm_status->Vpp_state = VPP_UNKNOWN;


	// DSC OSBDM firmware does not support get_status. CFv234 OSBDM firmware uses get_status for "core status" inquiry.
	if (gOSBDMInfo.coreid == eDSC || gOSBDMInfo.coreid == eCFv234 || gOSBDMInfo.coreid == eKinetis || gOSBDMInfo.coreid == ePPC){
		return osbdm_error_ok;
	}
	else{
		if ((retErr=osbdm_status(data)))
			return retErr;

	//	bdm_status->ackn_state = data[2];
		bdm_status->reset_state = data[3];
		bdm_status->connect_state = data[4];
		if (gOSBDMInfo.coreid == eRS08)
			bdm_status->Vpp_state = data[10];
		return osbdm_error_ok;
	}
}

__declspec(dllexport) OsbdmErrT	osbdmAPI_get_speed(unsigned long *speed){

	switch (gOSBDMInfo.coreid)
	{
	case eDSC:
	case eCFv234:
		return osbdm_error_not_supported;
	default:
		return osbdm_get_speed(speed);
	}
}

__declspec(dllexport) OsbdmErrT	osbdmAPI_set_speed(unsigned long speed){	// in KHz
	return osbdm_set_speed(speed);
}

__declspec(dllexport) OsbdmErrT	osbdmAPI_config(unsigned char config_type, unsigned char config_param, unsigned long param_value)
{
	return osbdm_config(config_type, config_param, param_value);
}

__declspec(dllexport) OsbdmErrT osbdmAPI_reset(ResetT reset_type){
	return osbdm_target_reset(reset_type);
}

__declspec(dllexport) OsbdmErrT osbdmAPI_stop(void){
	return osbdm_target_halt();
}
__declspec(dllexport) OsbdmErrT osbdmAPI_run(void){
	return osbdm_target_go();
}
__declspec(dllexport) OsbdmErrT osbdmAPI_step(void){
	return osbdm_target_step();
}

__declspec(dllexport) OsbdmErrT osbdmAPI_read_reg(unsigned int regid, SizeT regsize, unsigned long *value){

	OsbdmErrT retErr;
	InternalReginfoT reginfo;
	unsigned char tmpValue[4];

	reginfo.regtype = MEM_REG;  // default
	reginfo.regid = regid;
	reginfo.regsize = regsize;

	if ((retErr = map_to_internal_reginfo(regid, &reginfo)))
		return retErr;

	switch(reginfo.regsize){
		 case eLong:
			//	return osbdm_read_32(reginfo.regtype, reginfo.regid, (unsigned long *)value);
			retErr = osbdm_read_block((unsigned char)reginfo.regtype, 32, (unsigned long)reginfo.regid, (unsigned char *)tmpValue, 4);
			*value = *(unsigned long *)tmpValue;
			return retErr;
 
		 case eWord:
			// return osbdm_read_16(reginfo.regtype, reginfo.regid, (unsigned short *)value);
			retErr = osbdm_read_block((unsigned char)reginfo.regtype, 16, (unsigned long)reginfo.regid, (unsigned char *)tmpValue, 2);
			*value = *(unsigned short *)tmpValue;
			return retErr;

		 case eByte:
			return osbdm_read_8((unsigned char)reginfo.regtype, reginfo.regid, (unsigned char *)value);

		 default:
			return osbdm_error_invalid_parameter;
	}
}


__declspec(dllexport) OsbdmErrT osbdmAPI_write_reg(unsigned int regid, SizeT regsize, unsigned long value){

	OsbdmErrT retErr;
	InternalReginfoT reginfo;

	reginfo.regtype = MEM_REG;  // default
	reginfo.regid = regid;
	reginfo.regsize = regsize;

	if ((retErr = map_to_internal_reginfo(regid, &reginfo)))
		return retErr;

	switch(reginfo.regsize){
		 case eLong:
			return osbdm_write_32((unsigned char)reginfo.regtype, reginfo.regid, (unsigned long)value);

		 case eWord:
			return osbdm_write_16((unsigned char)reginfo.regtype, reginfo.regid, (unsigned short)value);

		 case eByte:
			return osbdm_write_8((unsigned char)reginfo.regtype, reginfo.regid, (unsigned char)value);

		 default:
			return osbdm_error_invalid_parameter;
	}
}


__declspec(dllexport) 
OsbdmErrT osbdmAPI_read_mem(unsigned char mem_space, unsigned int addr, unsigned int byte_count, SizeT accessSize, unsigned char *buffer){

	switch (gOSBDMInfo.coreid)
	{
	case eDSC:
		return dsc_read_mem((dscmem)mem_space, addr, byte_count, accessSize, buffer);
	default:
		return osbdm_read_block((unsigned char)MEM_RAM, (unsigned char)(8<<accessSize), addr, buffer, byte_count);
	}
}

__declspec(dllexport) OsbdmErrT	
osbdmAPI_write_mem(unsigned char mem_space, unsigned int addr, unsigned int byte_count, SizeT accessSize, unsigned char *buffer){

	switch (gOSBDMInfo.coreid)
	{
	case eDSC:
		return dsc_write_mem((dscmem)mem_space, addr, byte_count, accessSize, buffer);
	default:
		return osbdm_write_block((unsigned char)MEM_RAM, (unsigned char)(8<<accessSize), addr, buffer, byte_count);
	}
}

__declspec(dllexport) OsbdmErrT osbdmAPI_core_mode(CoreModeT *core_mode){
	OsbdmErrT retErr;
	unsigned long osr, xcsr, bdmsts, temp_dhcsr;
	unsigned char mode, xcsrStat,PSTstat,bdcscr;
	unsigned int regid;
	unsigned char buffer[20];

	*core_mode = eCoreModeUndefined;
	switch (gOSBDMInfo.coreid){
	case eDSC:
		if ((retErr=osbdm_read_32(MEM_CREG, dscregID_osr, &osr))) return retErr;

		if ((osr & 0xffff) == 0xffff){
			*core_mode = eWaitOrStopped;
			return osbdm_error_ok;
		}


		mode = osr & 0x30;
		switch(mode){
		  case 0x0:
			*core_mode = eRunning;
			return osbdm_error_ok;

		  case 0x10:
			*core_mode = eWaitOrStopped;
			return osbdm_error_ok;

		  case 0x20:
			*core_mode = eBusy;
			return osbdm_error_ok;

		  case 0x30:
			*core_mode = eDebug;
			return osbdm_error_ok;
		}

	case eCFv1:
		retErr = osbdmAPI_read_reg(cfv1regID_xcsr_byte, eByte, &xcsr);
		if (retErr != osbdm_error_ok) return retErr;

		if (xcsr&0x01) {  // BDM enabled
			xcsrStat = xcsr&0xC0;

			switch(xcsrStat){
			  case 0: 
				*core_mode = eRunning;
				return osbdm_error_ok;

			  case 0x40:
				*core_mode = eStopped;
				return osbdm_error_ok;

			  case 0x80:
				*core_mode = eDebug;
				return osbdm_error_ok;
				
			  default:
				return osbdm_error_fail;
			} 
		}
		else{
			return osbdm_error_bdm_not_enabled;
		}

	case eCFv234:
		if ((retErr=osbdm_status(buffer))) return retErr;

		// NOTE PST[0:3] must be enabled for this to work
		PSTstat = buffer[7] & 0x0F;
		if (PSTstat == 0x0F) *core_mode = eDebug;
		else *core_mode = eRunning;
		return osbdm_error_ok;

	case eRS08:
	case eS08:

		if(gOSBDMInfo.coreid == eRS08) regid = rs08regID_bdcscr & ~rs08regID_FIRST_DEBUG_REG;
		else regid = s08regID_bdcscr & ~s08regID_FIRST_DEBUG_REG;

		if ((retErr=osbdm_read_8(MEM_DREG, regid, &bdcscr))) return retErr;

		if (bdcscr&0x80) {  // BDM enabled
			if (bdcscr&0x40) *core_mode = eDebug;
			else *core_mode = eRunning;
			return osbdm_error_ok;
		}
		else{
			return osbdm_error_bdm_not_enabled;
		}
	
	case eS12:
		retErr = osbdmAPI_read_reg(s12regID_bdmsts, eByte, &bdmsts);
		if (retErr != osbdm_error_ok) return retErr;

		if (bdmsts&0x80) {  // BDM enabled
			if (bdmsts&0x40) *core_mode = eDebug;
			else *core_mode = eRunning;
			return osbdm_error_ok;
		}
		else{
			return osbdm_error_bdm_not_enabled;
		}
    case eKinetis:
		if ((retErr=osbdm_read_32(MEM_RAM, Reg_CoreDebug_DHCSR_address, &temp_dhcsr))) return retErr;
        if((temp_dhcsr&0x00020003)!=0x00020003)
		{
		   *core_mode = eRunning;
			return osbdm_error_ok;
		} else
		{
		   *core_mode = eDebug;
			return osbdm_error_ok;
		}


	default:
		return osbdm_error_invalid_target;
	}

	return osbdm_error_ok;
}
	
__declspec(dllexport) OsbdmErrT osbdmAPI_secure_mode(SecureModeT *secure_mode){
	OsbdmErrT retErr;
	unsigned long xcsr,temp_ap_mdm_status;
	unsigned char bdcscr, fopt;
	unsigned short fopt_addr = 0x1821;  // S08/RS08 FOPT register address. FIXME: target code hard-coded.
	unsigned int regid;

	*secure_mode = eSecureModeUndefined;
	switch (gOSBDMInfo.coreid){

	case eCFv1:
		retErr = osbdmAPI_read_reg(cfv1regID_xcsr_byte, eByte, &xcsr);
		if (retErr != osbdm_error_ok) return retErr;
		
		if (xcsr&0x02)   // Flash Security enabled
			*secure_mode = eSecured;
		else
			*secure_mode = eUnsecured;

		return osbdm_error_ok;

	case eRS08:
	case eS08:
		if(gOSBDMInfo.coreid == eRS08) regid = rs08regID_bdcscr & ~rs08regID_FIRST_DEBUG_REG;
		else regid = s08regID_bdcscr & ~s08regID_FIRST_DEBUG_REG;

		if ((retErr=osbdm_read_8(MEM_DREG, regid, &bdcscr))) 
			return retErr;

		if (bdcscr&0x80) {  // BDM enabled
			if ((retErr=osbdm_read_8(MEM_RAM, fopt_addr, &fopt))) return retErr;
			if (fopt&0x02) *secure_mode = eUnsecured;
			else *secure_mode = eSecured;
			return osbdm_error_ok;
		}
		else{
			return osbdm_error_bdm_not_enabled;
		}
	
	case eS12:

	//TODO: ADD core mode routine here
		break;

	case eKinetis:
		if ((retErr=osbdm_read_32(MEM_REG, 1000, &temp_ap_mdm_status))) return retErr;
        if((temp_ap_mdm_status&0x00000004)==0x00000004)
		{
		   *secure_mode = eSecured;
			return osbdm_error_ok;
		} else
		{
		   *secure_mode = eUnsecured;
			return osbdm_error_ok;
		}

	default:
		return osbdm_error_not_supported;
	}

	return osbdm_error_ok;
}
	
	

__declspec(dllexport) OsbdmErrT	osbdmAPI_exchange_special_feature_command(unsigned char sub_cmd_num, unsigned int *length, unsigned char *data)
{
	unsigned int i;

	if (*length > MAX_DATA_SIZE-4)
		return osbdm_error_invalid_parameter;

	usb_data[0] = CMD_SPECIAL_FEATURE;
	usb_data[1] = sub_cmd_num;
	usb_data[2] = *length >> 8;
	usb_data[3] = *length & 0xff;
	for (i = 0; i<*length;i++) 
	{
		usb_data[4+i] = data[i];
	}

	if(osbdm_usb_send_ep1(MAX_DATA_SIZE, usb_data)) return osbdm_error_usb_transport;	// tx data
	if(osbdm_usb_recv_ep2(MAX_DATA_SIZE, usb_data2)) return osbdm_error_usb_transport;	// receive result
	if (usb_data2[0] != CMD_SPECIAL_FEATURE) return osbdm_error_cmd_failed;
	if (usb_data2[1] != MAX_DATA_SIZE) return osbdm_error_cmd_failed;
	*length = ((unsigned int) (usb_data2[2]) << 8) |  usb_data2[3];
	if (*length > MAX_DATA_SIZE-4)
		{
			*length = 0;
			return osbdm_error_invalid_parameter;
	}
	for (i = 0; i<*length;i++) 
	{
		data[i] = usb_data2[i+4];
	}
		
	return osbdm_error_ok;

}



//---------------------------------------------------------------
// local functions
//---------------------------------------------------------------
//

OsbdmErrT map_to_internal_reginfo(unsigned int regid, InternalReginfoT *reginfo){

	switch (gOSBDMInfo.coreid)
	{
	case eDSC: 
		reginfo->regtype = MEM_CREG; 
		break;
	case eCFv1:
		if (regid >= cfv1regID_d0 && regid <= cfv1regID_a7)
			reginfo->regtype = MEM_REG;
		else if (regid == cfv1regID_pc || regid == cfv1regID_sr || 
					(regid >= cfv1regID_FIRST_CONTROL_REG && regid < cfv1regID_FIRST_DEBUG_REG)){

			reginfo->regtype = MEM_CREG;

			if (regid == cfv1regID_pc)	reginfo->regid = 0x0F;
			else if (regid == cfv1regID_sr) reginfo->regid = 0x0E;
			else reginfo->regid = regid - cfv1regID_FIRST_CONTROL_REG;
		}
		else if (regid >= cfv1regID_FIRST_DEBUG_REG){

			reginfo->regtype = MEM_DREG;

			if ((regid >= cfv1regID_xcsr_byte) && (regid <= cfv1regID_csr3_byte)){
				reginfo->regsize = eByte;
				reginfo->regid = regid - cfv1regID_FIRST_DEBUG_regID_BYTE +1;	
			}
			else{
				reginfo->regsize = eLong;
				reginfo->regid = regid - cfv1regID_FIRST_DEBUG_REG;
			}
		}
		else 
			 return osbdm_error_invalid_parameter;

		// TODO: Error handling for the invalid regid
		break;

	case eCFv234:
		if (cfv234regID_d0 <= regid && regid <= cfv234regID_a7) 
			reginfo->regtype = MEM_REG;
		else if (regid == cfv234regID_pc || regid == cfv234regID_sr || 
			(cfv234regID_FIRST_CONTROL_REG  <= regid && regid < cfv234regID_FIRST_DEBUG_REG)){

			reginfo->regtype = MEM_CREG;

			if (regid == cfv234regID_pc)	reginfo->regid = 0x0F;
			else if (regid == cfv234regID_sr) reginfo->regid = 0x0E;
			else reginfo->regid = regid - cfv234regID_FIRST_CONTROL_REG;
		}
		else if (regid >= cfv234regID_FIRST_DEBUG_REG){

			reginfo->regtype = MEM_DREG;
			reginfo->regsize = eLong;
			reginfo->regid = regid - cfv234regID_FIRST_DEBUG_REG;
		}
		else
			return osbdm_error_invalid_parameter;

		// TODO: Error handling for the invalid regid
		break;

	case eS08:
		if (regid >= s08regID_a && regid <= s08regID_pc) 
			reginfo->regtype = MEM_REG;
		else if (regid >= s08regID_FIRST_DEBUG_REG){
			reginfo->regtype = MEM_DREG;
			reginfo->regid = regid - s08regID_FIRST_DEBUG_REG;
		}
		else
			return osbdm_error_invalid_parameter;

		// TODO: Error handling for the invalid regid
		break;


	case eRS08:
		if (regid == rs08regID_x || regid == rs08regID_page)
			reginfo->regtype = MEM_RAM;
		else if ( regid >= rs08regID_a && regid <=rs08regID_pc)
			reginfo->regtype = MEM_REG;
		else if (regid >= rs08regID_FIRST_DEBUG_REG){
			reginfo->regtype = MEM_DREG;
			reginfo->regid = regid - rs08regID_FIRST_DEBUG_REG;
		}		
		else
			return osbdm_error_invalid_parameter;

		// TODO: Error handling for the invalid regid
		break;

	case eS12:
		if (s12regID_d <= regid && regid <= s12regID_pc) 
			reginfo->regtype = MEM_REG;
		else if (regid >= s12regID_FIRST_DEBUG_REG){
			reginfo->regtype = MEM_DREG;
			reginfo->regid = regid - s12regID_FIRST_DEBUG_REG;
		}
		else
			return osbdm_error_invalid_parameter;

		// TODO: Error handling for the invalid regid
		break;

	default:
		return osbdm_error_invalid_parameter;
	}

	return osbdm_error_ok;
}





OsbdmErrT dsc_read_mem(dscmem memType, unsigned int address, unsigned int numBytes, unsigned int accessType, unsigned char *buffer){
	print("osbdmdsc_read_mem: %d bytes at %0x\r\n", numBytes, address);
	switch(memType)
	{
	  case dscmem_p:
		return osbdm_read_block((unsigned char)MEM_P, 16, address, buffer, numBytes);

	  case dscmem_x:
		return osbdm_read_block((unsigned char)MEM_X, (unsigned char)(8<<accessType), address, buffer, numBytes);

	  case dscmem_p_flash:
		return osbdm_read_block((unsigned char)MEM_P_FLASH, 16, address, buffer, numBytes);

	  case dscmem_x_flash:
	  default:
			return osbdm_error_invalid_parameter;
	}
}

OsbdmErrT dsc_write_mem(dscmem memType, unsigned int address, unsigned int numBytes, unsigned int accessType, unsigned char *buffer){
	print("osbdmdsc_write_mem\r\n");
	switch(memType)
	{
	  case dscmem_p:
		return osbdm_write_block((unsigned char)MEM_P, 16, address, buffer, numBytes);

	  case dscmem_x:
		return osbdm_write_block((unsigned char)MEM_X, (unsigned char)(8<<accessType), address, buffer, numBytes);

	  case dscmem_p_flash:
		return osbdm_write_block((unsigned char)MEM_P_FLASH, 16, address, buffer, numBytes);

	  case dscmem_x_flash:
	  default:
			return osbdm_error_invalid_parameter;
	}
}

