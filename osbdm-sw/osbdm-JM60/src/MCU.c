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
	OSBDM60 CodeWarrior Project
	
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

#include "typedef.h"
#include "MCU.h"				// include MCU header
#include "timer.h"			// timer functions
#include "serial_io.h"

#include "target.h"


//---------------------------------------------------------------------------
// Function Prototypes
//---------------------------------------------------------------------------
void Mcu_Init(void);
void MCG_Init(void);
void Blink_Led(byte rate);	// blinks Status LED at rate in ms to indicate error
byte tPower_Good(void);		// detect target power level
void bdm_vsw_en(void);		// turn on BDM voltage 
byte bdm_vtrg_en(void);		// check target power and enable Vout if necessary
void t_rsto(void);			// assert tRESET_OUT to target

// State Variables
byte Self_Power = 0;			// flag indicator for self_powered target 
byte Jtag_Mode = 0;			// JTAG flag set true if JTAG mode
byte Target_State;			// holds target state
byte FP_Stat;		// holds flash programming status
byte VPP_Stat;          // holds VPP status

/******************************************************************************
	MCU_Init()

	setup and configure JM60 for BDM functionality

******************************************************************************/
void Mcu_Init(){
  
	SOPT1= SOPT1_STOPE_MASK;
	SPMSC1=SPMSC1_LVWACK_MASK;

	PTBD = 0x08;			// init PTBD 
	PTCD = 0x00;			// init PTCD
	PTDD = 0x03;			// init PTDD
	PTED = 0x80;			// init PTED
	PTFD = 0x00;			// init PTFD
	PTGD = 0x00;	 		// init PTGD
	
	PTBDD = 0x08;			// PTBD[4,2:0] input, PTBD[4,2] inputs used for MCU BOARD ID, PTBD[7:5] not used, PT3 output
	PTCDD = 0x30;			// PTCD[5:4] output, PTCD[3:0] input, PTCD[2:0] input used for OSBDM ID, PTCD[7:6] not used 
	PTDDD = 0x03;			// PTDD[1:0] output, PTD2 input, others not used
	PTEDD = 0x00;			// PTED[7:0] input
	PTFDD = 0x21;			// PTFD[0] output, PTF[1] input, PTF[5] make sure BGND line is tri-stated 
	                    // others alternate functionality	
	PTGDD = 0x00;			// PTGD[3:2] input, PTGD[5:4] alternate functionality
	
	// TPM1 setup
	TPM1SC = 0x08;				// TPM1 enabled, bus rate clk, div by 1
	TPM1C0SC = 0x10;			// timer set for output compare, output is GPIO
	TPM1C1SC_ELS1x = 0x00;	// pin is GPIO	
	TPM1C2SC_ELS2x = 0x00;	// pin is GPIO	
	TPM1C3SC_ELS3x = 0x00;	// pin is GPIO	
	
	//TPM2 Setup
	TPM2SC_CLKSx = 0x00;		// TMP2 counter disabled
	TPM2C0SC = 0x00;			// TPM2C0 is toggle output on compare
	TPM2C1SC = 0x00;			// TPM2C1 is toggle output on compare 
	
	// ADC Setup 
	ADCSC1 = 0x1F;				// ADC1 disabled
	ADCSC2 = 0x00;				// SW trigger, compare function disabled
//	ADCCFG = 0x51;				// 8b result, bus clk div 2, input clk div 4 (3MHz clock)
	ADCCFG = 0x71;				// 8b result, bus clk div 2, input clk div 8 (1.5MHz clock)
	APCTL1 = 0x20;				// ADP5 enabled
	APCTL2 = 0x00;				// other ADP inputs disabled
	ADCSC1 = ADP5;				// enable ADP5 and start conversion

	// SPI1 Setup
	SPI1C1 = 0x10;				// SPI disabled, Master mode, active high, center aligned data, MSB first
	SPI1C2 = 0x40;				// 16b mode, required bit-bang for extra bit, data in/out on separate pins
	SPI1BR = 0x44;				// 150kHz baud rate default
	

#ifdef __DSC__  
	// this board passes the target micro's serial i/o thru the JM60's first serial port
  PTGDD = 0x04;       // set PTG2 output, rest input
  PTGD_PTGD2 = 1;     // enable serial bridge
#endif

	(void)bdm_vtrg_en();	// enable Vout to target

	// clear global status variables
	FP_Stat = FP_DONE;
	VPP_Stat = VPP_DISABLED;
}

/******************************************************************************
	MCG_Init()

	configure JM60 MCG to PEE mode using 4Mhz XTAL input to generate the 24MHz
	internal bus frequency for USB functionality
 
******************************************************************************/
void MCG_Init(){
	// Switch from FEI mode to FBE mode
	MCGC2 = 0x36;						// select hi-freq, hi-gain ext OSC,
											// enable ext OSC
	while(!(MCGSC_OSCINIT));		// wait until ext OSC is stable
	MCGC1 = 0x88;						// ext OSC is system clock
											// set ref divider to 2 for PLL input = 2.0MHz
	while(!(MCGSC_CLKST1));			// wait until external reference is selected

	// Switch from FBE mode to PBE mode
	MCGC3 = 0x46;						// select PLL, 24mhz
//	MCGC3 = 0x45;						// select PLL, 20mhz
//	MCGC3 = 0x44;						// select PLL, 16mhz
	while (!(MCGSC & 0x6A));		// wait until PLL is locked

	// Switch from PBE mode to PEE mode
	MCGC1 &= 0x3F;						// select PLL output 
	while(!(MCGSC & 0x6C));			// wait until locked, PLL selected, and 
	return;
}

/******************************************************************************
	Blink_Led()

	blinks Status LED 
 
******************************************************************************/
void Blink_Led(byte rate){
	for(;;){
		STATUS_LED ^= 1;				// toggle Status LED
		wait_ms(rate);					// wait for rate ms
	}
}

/******************************************************************************
	bdm_vsw_en()

	enable BDM voltage and check for fault
	
	Return:	this does not return if it fails
	
******************************************************************************/
void bdm_vsw_en(void){
	VSW_EN = 1;						// enable BDM voltage swtich
	VSW_EN_DIR = 1;				// pin is output
	wait_ms(500);					// wait 50ms
    #if   defined __CFV2__
    // do not check overcurrent due to a flaw in one of the modelo boards
    #else
    //	if(VSW_FAULT == 1){			// if circuit is in overcurrent
    //	VSW_EN = 0;					// disable BDM voltage switch
//		return(1);					// return failed
		// here if power enable returns fail 
		// the next function never returns, so this is an endless loop
    //		Blink_Led(255);	// blink status LED at 1/3 Hz to indicate an error
    //	}
    #endif
	// here if power enabled OK
	STATUS_LED = 0;	// STATUS LED ON to show enumeration and BDM power good
}

/******************************************************************************
	bdm_vtrg_en()

	enable check for target power and enable Vout to target if necessary
	
	Return:	0 on success, 1 on fail 
	
******************************************************************************/
byte bdm_vtrg_en(void){
	// enable target power 
	if(!(tPower_Good())) {		// check to see if target is self-powered
		VTRG_EN = 1;				// enable BDM voltage switch
		VTRG_EN_DIR = 1;			// pin is output
		wait_ms(50);				// wait 50ms
    #if   defined __CFV2__
    // do not check overcurrent due to a flaw in one of the modelo boards
    #else
		if(VTRG_FAULT == 1){		// if circuit is in overcurrent
			VTRG_EN = 0;			// disable BDM voltage switch
			return(1);				// return failed
		}
		#endif
	}
	if(tPower_Good()) {			// check again to ensure power is good
		tPWR_LED = 0;				// TPWR LED indicates target is powered
	}
	return(0);						// return OK					 	
}

/******************************************************************************
	tPower_Good()

	tests target power voltage rail. 
	
	OUTPUT:		1 if VTRG is greater than 3.0V
					0 if VTRG is less than 3.0V 
 
******************************************************************************/
byte tPower_Good(void){
	ADCSC1 = ADP5;						// enable ADP5 and start conversion
	while(!(ADCSC1_COCO));			// wait until conversion completes
	if (ADCRL > 0x4C){
		return (1);						// target is self-powered
	} else return (0);				// target is unpowered
}


/******************************************************************************
	CheckTargetPower()

	checks the power status of the target and sets/clears power LED accordingly
	
******************************************************************************/
void	CheckTargetPower(void){
		if(!(tPower_Good())) {			// check if target power is still good
			tPWR_LED = 1;					// TPWR LED off if target not powered
		} else tPWR_LED = 0;			// TPWR LED on if target power is OK
}

/******************************************************************************
	t_rsto()

	asserts reset to target for 50ms then waits 300ms to return. 
	
******************************************************************************/
void t_rsto(void){
	tRSTO = 1;						// assert RESET signal
	wait_ms(50);					// wait 50ms 
	tRSTO = 0;						// de-assert RESET signal
	wait_ms(300);					// wait 300ms for reset pin to go high
}

/******************************************************************************
	t_rsti()

	returns 1 if target is being reset (RESET_IN pin low) otherwise returns 0 
	
******************************************************************************/
byte t_rsti(void){
	if(tRSTI == 1)	return 0;
	return 1;
}
				
