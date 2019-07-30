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



/******************************************************************************
	File:		cmd_processing.h
	
	Purpose: OSBDM60 CodeWarrior Project
	
	Copyright (c) 2008 Axiom Manufacturing

	adapted from CodeWarrior Project developed by Freescale and included with 
	AN3564, "Customize the USB Application Using the MC9S08JM60", Rev 0, dated
	Nov, 2007

*
* 03/08/2011  : Modified by P&E Microcomputer Systems
*               http://www.pemicro.com/osbdm
*
*
*
******************************************************************************/ 

#ifndef _CMD_PROCESSING_H_
#define _CMD_PROCESSING_H_

// Global Variables
extern word mod;						// SPI modulus count
extern byte sStatus;					// SPI transmit status
extern byte eStatus;					// BDMERROR_ status.  0 if no error
extern byte debug_cmd_pending;			// input BDM command pending
//extern byte serial_cmd_pending;			// input serial command pending


/*
typedef enum {
	CF = TARGET_TYPE_CF_BDM,		// target is ColdFire
	JTAG = TARGET_TYPE_JTAG			// target is JTAG
}target_type_e;


typedef enum {
	NO_RESET_ACTIVITY = 0,
	RESET_DETECTED = 1
}reset_e;
*/

typedef struct {
	byte target_type	:3;			// connected target type
	byte reset			:1;			// reset
} _tCable_Status;


extern _tCable_Status tCable_Status;


// Functions

extern void debug_command_exec(void);	// command exec function
extern void serial_command_exec(void);
extern void serial_send_data_if_pending(void);

#endif // _CMD_PROCESSING_H_
