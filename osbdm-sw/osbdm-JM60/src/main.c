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
	File:		main.c
	
	Purpose: OSBDM60 CodeWarrior Project
	
	adapted from CodeWarrior Project developed by Freescale and included with 
	AN3564, "Customize the USB Application Using the MC9S08JM60", Rev 0, dated
	Nov, 2007

******************************************************************************/ 

#include <hidef.h>					// for EnableInterrupts macro 
#include "derivative.h"				// include peripheral declarations  
#include "Usb_Drv.h"					// USB Main Driver  
#include "Usb_Config.h"				// USB Configuration header  
#include "MCU.h"						// MCU Initialization  
#include "USB_User_API.h"			// USB API for USB Module  
#include "commands.h"				// BDM commands header file
#include "cmd_processing.h"		// command processing structures
#include "timer.h"
#include "serial_io.h"
#include "board_id.h"


//---------------------------------------------------------------------------
// NVOPT/NVPROT Initialization
//---------------------------------------------------------------------------

const byte NVOPT_INIT  @0x0000FFBF = 0x02;    // vector redirect, flash unsecure
const byte NVPROT_INIT @0x0000FFBD = 0xFA;    // 0xFC00-0xFFFF are protected 

//---------------------------------------------------------------------------
// Function Prototypes
//---------------------------------------------------------------------------

extern void _Startup(void);
void interrupt IRQ_ISR(void);
void interrupt USB_ISR(void);
void interrupt VPP_ISR(void);

void t_debug_init(void);
void t_flash_stat(void);

void main(void) {

	// Initialize System
	read_board_id();
	read_osbdm_id();

	Mcu_Init();							// MCU general Initialization  
	MCG_Init();							// Initialize the MCG module to generate 24MHz bus clock  
	CDC_Init();


	t_debug_init();	// initialize Debug Port Settings


	Initialize_USBModule();			// Initialize USB Module  
	EnableInterrupts;			// Enable Interrupts Macro         
	Check_USBBus_Status();	// 1st time thru, enumerate and set state to ATTACHED

								// NOTE: may need to move this inside the loop, execute once
	bdm_vsw_en();	// enable BDM voltage and check for fault 
								 
//  while(Usb_Device_State != ADDRESS_STATE)   ;  
//  ConfigureUSBDevice();

	debug_cmd_pending = 0x00;		// clear pending flag
	//serial_cmd_pending = 0x00;		// clear pending flag

	for(;;){						// Main Loop

		Check_USBBus_Status();	// use polling, must remain in main loop
		
		CDC_Engine(); //Process any configuration commands for serial port

		if(debug_cmd_pending){				// if BDM command is ready, 
			debug_command_exec();			// process BDM command
			debug_cmd_pending = 0x00;		// clear pending flag
		}

		CheckTargetPower();

#ifdef __9RS08__  
		t_flash_stat();	// check status for flash programming
#endif	
		
	} 
}

// Dummy ISR
interrupt void Dummy_ISR(void) {
}

void (* volatile const _UserEntry[])()@0xFABC={
  0x9DCC,             // asm NOP(9D), asm JMP(CC)
  _Startup
};
  
// redirect vector 0xFFC0-0xFFFD to 0xFBC0-0xFBFD
void (* volatile const _Usr_Vector[])()@0xFBC4= {
    VPP_ISR,        // Int.no.29 RTC               (at FBC4) (at FFC4)
    Dummy_ISR,        // Int.no.28 IIC               (at FBC6) (at FFC6)
    Dummy_ISR,        // Int.no.27 ACMP              (at FBC8) (at FFC8)
    Dummy_ISR,        // Int.no.26 ADC               (at FBCA) (at FFCA)
    Dummy_ISR,        // Int.no.25 KBI               (at FBCC) (at FFCC)
    Dummy_ISR,        // Int.no.24 SCI2 Transmit     (at FBCE) (at FFCE)
    Dummy_ISR,        // Int.no.23 SCI2 Receive      (at FBD0) (at FFD0)
    Dummy_ISR,        // Int.no.22 SCI2 Error        (at FBD2) (at FFD2)
    Dummy_ISR,        // Int.no.21 SCI1 Transmit     (at FBD4) (at FFD4)
    SCI_ReceiveISR,        // Int.no.20 SCI1 Receive      (at FBD6) (at FFD6)
    SCI_OverrunISR,        // Int.no.19 SCI1 error        (at FBD8) (at FFD8)
    Dummy_ISR,        // Int.no.18 TPM2 Overflow     (at FBDA) (at FFDA)
    Dummy_ISR,        // Int.no.17 TPM2 CH1          (at FBDC) (at FFDC)
    Dummy_ISR,        // Int.no.16 TPM2 CH0          (at FBDE) (at FFDE)
    Dummy_ISR,        // Int.no.15 TPM1 Overflow     (at FBE0) (at FFE0)
    Dummy_ISR,        // Int.no.14 TPM1 CH5          (at FBE2) (at FFE2)
    Dummy_ISR,        // Int.no.13 TPM1 CH4          (at FBE4) (at FFE4)
    Dummy_ISR,        // Int.no.12 TPM1 CH3          (at FBE6) (at FFE6)
    Dummy_ISR,        // Int.no.11 TPM1 CH2          (at FBE8) (at FFE8)
    Dummy_ISR,        // Int.no.10 TPM1 CH1          (at FBEA) (at FFEA)
    Dummy_ISR,        // Int.no.9  TPM1 CH0          (at FBEC) (at FFEC)
    Dummy_ISR,        // Int.no.8  Reserved          (at FBEE) (at FFEE)
    USB_ISR,	        // Int.no.7  USB Statue        (at FBF0) (at FFF0)
    Dummy_ISR,        // Int.no.6  SPI2              (at FBF2) (at FFF2)
    Dummy_ISR,        // Int.no.5  SPI1              (at FBF4) (at FFF4)
    Dummy_ISR,        // Int.no.4  Loss of lock      (at FBF6) (at FFF6)
    Dummy_ISR,        // Int.no.3  LVI               (at FBF8) (at FFF8)
    IRQ_ISR,	        // Int.no.2  IRQ               (at FBFA) (at FFFA)
    Dummy_ISR,        // Int.no.1  SWI               (at FBFC) (at FFFC) 
};

#pragma CODE_SEG Bootloader_ROM

void Bootloader_Main(void);
void bootloader_dly_20us(void);


void _Entry(void) {

  bootloader_dly_20us();		// delay for lines to settle
	
  IRQSC_IRQPE = 1;          // enable IRQ pin which is used for bootload select jumper
   
  bootloader_dly_20us();		// delay for lines to settle

 
 	
    // If IRQ Jumper disabled, Enter User mode
    asm {
        BIL GotoBootloader;       // Branch if IRQ pin low
        JMP _UserEntry;           // Enter User mode
GotoBootloader:
    }
    // Otherwise enter bootloader mode
    
    // MCG clock initialization, fBus=24MHz
    MCGC2 = 0x36;
    while(!(MCGSC & 0x02));		      //wait for the OSC stable
    MCGC1 = 0x0B;					// 
    MCGC3 = 0x46;
    while ((MCGSC & 0x48) != 0x48);	//wait for the PLL is locked

    // Flash clock
    FCDIV=0x4E;                  // PRDIV8=1; DIV[5:0]=14, flash clock should be 150-200kHz
                                 // bus clock=24M, flash clock=fbus/8/(DIV[5:0]+1) 
        
    SOPT1 = 0x20;                 // disable COP 
    USBCTL0=0x44;                 // configure USB
    Bootloader_Main();            // Bootloader mode
}



/******************************************************************************
*	void bootloader_dly_20us(void)	add delay 20 us 
*
******************************************************************************/

void bootloader_dly_20us(void){
	 // 2* 8 cycles *30 = 2 * 240 ticks @ 24MHz = 20us 
	 asm {
	 	LDX #0x3C			// loop - 2* 30 loops
		loop1:
			BRSET 0,0,0			// 5 cycles   
			DBNZX loop1			// 3 cycles 
	}
}

#pragma CODE_SEG default


 
