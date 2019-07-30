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

#ifndef __MCU_H__
#define __MCU_H__


/* Includes */
#include "derivative.h"     /* include peripheral declarations */


//---------------------------------------------------------------------------
// Function Prototypes
//---------------------------------------------------------------------------
extern void Mcu_Init(void);
extern void MCG_Init(void);

//extern void tReset(byte bkpt);	// resets target and leaves in selected mode;
												// 0 = BDM mode, 1 = Normal mode
extern void Blink_Led(byte rate);	// blinks Status LED to indicate error
extern byte tPower_Good(void);		// detect target power level
extern void CheckTargetPower(void);
extern void bdm_vsw_en(void);			// turn on BDM voltage 
extern byte bdm_vtrg_en(void);		// check target power and enable Vout if necessary
extern void t_rsto(void);				// assert tRESET_OUT to target
extern byte t_rsti(void);				// get target reset status



// State Variables
extern byte Self_Power;
extern byte Jtag_Mode;				// JTAG flag set true if JTAG mode
extern byte Target_State;			// holds target state

extern byte FP_Stat;		// holds flash programming status
#define FP_DONE			0
#define FP_RUNNING	1
#define FP_TIMEOUT	2

extern byte VPP_Stat;	        	          // holds VPP flash programming voltage status
#define VPP_DISABLED  0                   // VPP is not supplied to the target.
#define VPP_ENABLED   1                   // VPP is supplied to the target.
#define VPP_TIMEOUT   2                   // VPP is turned off automatically due to timeout (20 sec).

// BDM State Defines
//#define ADP5		0x05	// single scan
#define ADP5		0x25	// continuous scan
#define RESET		0x01				// target is reset
//#define HALTED		0x02				// target is halted





#endif //  __MCU_H_
