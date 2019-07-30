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



/*****************************************************************************\
*
*	Author:		Axiom Manufacturing
*
*	File:		targetAPI.h
*
*	Purpose: 	header file for the generic target module interface API
*
*
* 03/08/2011  : Modified by P&E Microcomputer Systems
*               http://www.pemicro.com/osbdm
*
*
*
\*****************************************************************************/

#ifndef _TARGETAPI_H_
#define _TARGETAPI_H_

//----------------------------------------------------------------------------
// Hardware and Firmware Version Numbers
//----------------------------------------------------------------------------
#define VERSION_SW  0x01	// Firmware Version 1.0
#define VERSION_HW  0x10	// Hardware Version 1.0

//---------------------------------------------------------------------------
// Firmware Build Info
//---------------------------------------------------------------------------
//
// Warning! New versions of Codewarrior will automatically update the firmware.
//          If you are building a custom firmware based on OSBDM, please set 
//          the BUILD_VER version number to a minimum of 100
//
#define BUILD_VER         31
#define BUILD_VER_REV     26

//----------------------------------------------------------------------------
// Reset Type Definition
//----------------------------------------------------------------------------
   
typedef enum  {			// type of reset mode
	eSoftReset_to_DebugMode, 		
	eSoftReset_to_NormalMode, 		
	eHardReset_to_DebugMode, 
	eHardReset_to_NormalMode, 
	ePowerReset_to_DebugMode,
	ePowerReset_to_NormalMode	// not implemented yet
} ResetT; 

//----------------------------------------------------------------------------
// Core Type Definition
//----------------------------------------------------------------------------

typedef enum  {
  eCFv234,
	eCFv1,
	eS08,
	eRS08,
  eS12,
	eDSC,
	eCoreTypeUnknown,
	eKinetis,
	ePPC,
	eS12Z,
} CoreT;

//----------------------------------------------------------------------------
// Firmware Type Definition
//----------------------------------------------------------------------------

typedef enum  {
  eGeneric,
	eEmbeddedGeneric
} FirmwareT;

//----------------------------------------------------------------------------
// target implementation module API functions
//----------------------------------------------------------------------------

byte t_init        (void);			// init BDM_CF and target
byte t_reset       (ResetT mode);		// resets target and leaves in selected mode;
word t_sync        (void);			// Returns the sync value in 24 MHz. clocks; 
byte t_resync      (void);
int  t_stat        (PUINT8 pData);	// Returns status
byte t_halt        (void);			// halt program execution at next command
byte t_go          (void);			// resume code execution from current PC												
byte t_step        (void);
int  t_get_ver        (PUINT8 pData);	// Returns OSBDM-JM60 version info

int t_config        (byte configType, byte configParam, UINT32 paramVal);

UINT32 t_get_clock (void);
void t_set_clock   (UINT32 interval);

int t_write_mem    (byte type, UINT32 addr, UINT8 width, int count, PUINT8 pData);
int t_soft_reset_halt    (byte type, UINT32 addr, UINT8 width, int count, PUINT8 pData);
int t_read_mem     (byte type, UINT32 addr, UINT8 width, int count, PUINT8 pData);
int t_fill_mem     (byte type, UINT32 addr, UINT8 width, int count, PUINT8 pData);

int t_write_ad     (UINT32 addr, PUINT8 pData);
int t_read_ad      (UINT32 addr, PUINT8 pData);

int t_write_creg   (UINT32 addr, PUINT8 pData);
int t_read_creg    (UINT32 addr, PUINT8 pData);

int t_write_dreg   (UINT32 addr, UINT8 uWidth, PUINT8 pData);
int t_read_dreg    (UINT32 addr, UINT8 uWidth, PUINT8 pData);

char t_unsecure    (byte lockr, byte lrcnt, byte clkdiv);
void t_assert_ta   (word dly_cnt);

int t_flash_power  (byte enable);
int t_flash_enable (byte enable);
int t_flash_prog	 (PUINT8 pData);

void t_flash_stat	 (void);

int t_enable_ack   (byte enable);	// 1 to enable, 0 to disable

int t_special_feature(unsigned char sub_cmd_num,  // Special feature number (sub_cmd_num)
	                     PUINT16 pInputLength,      // Length of Input Command
	                     PUINT8  pInputBuffer,      // Input Command Buffer
	                     PUINT16 pOutputLength,     // Length of Output Response
	                     PUINT8  pOutputBuffer);    // Output Response Buffer 

#endif // _TARGETAPI_H_
// --------------------------------- EOF -------------------------------------
