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

/* This file was modified by P&E Microcomputer Systems in March, 2011
 * http://www.pemicro.com/osbdm
*/

//----------------------------------------------------------------------------
//
//
//  FILE
//
//      commands.h
// 
//
//  DESCRIPTION
//
//		OSBDM-JM60 command descriptor file
//
//
//----------------------------------------------------------------------------
//

#ifndef __COMMANDS_H__
#define __COMMANDS_H__

#include "osbdm_memtype.h"
#include "osbdm_configparam.h"

// cable status bit fields
#define RESET_DETECTED_MASK		0x0001
#define RSTO_STATE_MASK			0x0002

#define BDM_HW_VER				0		// 3bit field for HW version
#define BDM_FW_VER				0		// 3bit field for SW version

#define MAX_DATA_SIZE			64		// maximum size of data packets	
#define MEM_HEADER_SIZE			11		// number of bytes before data starts in memory read/write commands
#define CMD_HEADER_SIZE			2		// number of bytes before data starts in other commands
#define	STATUS_DATA_SIZE		10		// number of data bytes returned by status command
#define	STATUS_BLOCK_SIZE		CMD_HEADER_SIZE+STATUS_DATA_SIZE		// total number of bytes returned by status command
#define	VERSION_BLOCK_SIZE		12		// total number of bytes returned by get_ver command


// Target Types
#define TARGET_TYPE_CF_BDM		0		// Coldfire
#define TARGET_TYPE_JTAG		  1		// JTAG

// Each target has different register types.  The following defines make code more readable by defining
// CPU specific names for each register type number used by the firmware commmand
#define HC12_CPU_REG	MEM_REG
#define HC12_DEBUG_REG	MEM_DREG


// BDM Error Codes
#define BDMERROR_NOT_READY		0x10
#define BDMERROR_BERR			0x11
#define BDMERROR_ILLEGAL		0x12

// Return Status Codes
#define CMD_OK					0
#define CMD_FAILED				1
#define CMD_UNKNOWN				2

// System related commands
#define CMD_GET_STATUS			0x10	// returns a block of status information for the BDM and it's current state
#define CMD_BDM_INIT			  0x11	// initialize BDM

#define CMD_SET_TARGET			0x1A	// 8bit parameter; 0 = ColdFire(default), 1 = JTAG
#define CMD_RESET_TARGET		0x1B	// 8bit parameter; 0 = reset to BDM mode, 1 = reset to NORMAL mode 

#define CMD_HALT				0x20	// stop the CPU and bring it into BDM mode 
#define CMD_GO					0x21	// start code execution from current PC address 
#define CMD_STEP				0x22	// perform single step 
#define CMD_RESYNCHRONIZE		0x23	// resynchronize communication with the target (in case of noise, etc.) 
#define CMD_ASSERT_TA			0x24	// parameter: 8-bit number of 10us ticks - duration of the TA assertion 
#define CMD_SET_SPEED			0x25	// set target clock speed
#define CMD_GET_SPEED			0x26	// get target clock speed
#define CMD_SPECIAL_FEATURE             0x27    // Architecture Specific Command - See appropriate firmware file.

													
// CPU related commands 
#define CMD_READ_MEM			0x30	// parameters: 8-bit Memory Type, 32-bit address, 8-bit Size (8/16/32), 32-bit length (number to read)
#define CMD_WRITE_MEM			0x31 	// parameters: 8-bit Memory Type, 32-bit address, 8-bit Size (8/16/32), 32-bit length (number to write), data ...
#define CMD_READ_NEXT			0x32	// no parameters, reads the next block of data from the CMD_READ_MEM command
#define CMD_WRITE_NEXT			0x33 	// no parameters, writes the next block of data from the CMD_WRITE_MEM command
#define CMD_FILL				0x34 	// parameters: 8-bit Memory Type, 32-bit address, 8-bit Size (8/16/32), 32-bit length (number to write), data ...

#define CMD_FLASH_DLSTART		0x35 	// Start a flash programming sequence (needed for RS08 to turn voltage on)
#define CMD_FLASH_DLEND			0x36 	// Stop flash programming sequence
#define CMD_FLASH_PROG			0x37 	// Execute flash programming algorythm 


#define CMD_SCI_CONFIG			0x40	// configure serial port (baud rate etc.)
#define CMD_SCI_READ			0x41	// returns number of serial bytes received from the UART followed by the data
#define CMD_SCI_WRITE			0x42	// send a number of serial bytes out the UART 


// OSBDM-JM60 related commands
#define CMD_GET_VER       0x50  // returns 16 bit HW/SW version number,build version, firmware info

// Configuration related commands
#define CMD_CONFIG        0x70  // OSBDM-JM60 and target specific configuration. 
                                // parameters: config parameter and its value
              

// JTAG commands 
#define CMD_JTAG_GOTORESET		0x80	// no parameters, takes the TAP to TEST-LOGIC-RESET state, re-select the JTAG target to take TAP back to RUN-TEST/IDLE 
#define CMD_JTAG_GOTOSHIFT		0x81	// parameters 8-bit path option; path option ==0 : go to SHIFT-DR, !=0 : go to SHIFT-IR (requires the tap to be in RUN-TEST/IDLE) 
#define CMD_JTAG_WRITE			0x82	// parameters 8-bit exit option, 8-bit count of bits to shift in, and the data to be shifted in (shifted in LSB (last byte) first, unused bits (if any) are in the MSB (first) byte; exit option ==0 : stay in SHIFT-xx, !=0 : go to RUN-TEST/IDLE when finished 
#define CMD_JTAG_READ			0x83	// parameters 8-bit exit option, 8-bit count of bits to shift out; exit option ==0 : stay in SHIFT-xx, !=0 : go to RUN-TEST/IDLE when finished, returns the data read out of the device (first bit in LSB of the last byte in the buffer) 

#define CMD_JTAG_UNSECURE		0x90	// unsecure and erase flash

#endif // __COMMANDS_H__



