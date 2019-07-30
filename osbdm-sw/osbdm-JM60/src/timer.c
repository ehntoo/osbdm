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
*
*	Author:		Axiom Manufacturing
*
*	File:			timer.c
*
*	Purpose: 	Timer and loop delay routines
*
******************************************************************************/ 

#include "derivative.h"				// include peripheral declarations  
#include "timer.h"

/******************************************************************************
*	void wait_ms(ticks)
*
*	delay routine in 1ms increments.  actual delay measures ~ 1.03ms
*	
*	Input:	ticks - # of 1ms delay units
*
******************************************************************************/
void wait_ms(word ticks){

	word n;  
	
	for(n=0; n<ticks; n++){	// 32 cycles * (3 * 249) loops + 24 cycles (setup) + 72 cycles (padding) = 24000 ticks 
									// 24000 ticks @ 24MHz = 1ms 
		asm {
		LDA #0x03				// outer loop
		loop1:
		  LDX #0xF9			// inner loop - 249 loops
		loop:
			BRSET 0,0,0			// 5 cycles   
			BRSET 0,0,0			// 5 cycles   
			BRSET 0,0,0			// 5 cycles   
			BRSET 0,0,0			// 5 cycles   
			BRSET 0,0,0			// 5 cycles   
			NOP					// 1 cycle 
			NOP					// 1 cycle 
			NOP				     	// 1 cycle 
			DBNZX loop			// 4 cycles 
									// total: 5 * 5 + 3 + 4 = 32 cycles per loop
		DBNZA loop1   
		
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		}
	}

}



/******************************************************************************
*	void spi_bit(word mod)	add delay 
*
*	INPUT:	mod delay count
*	OUTPUT:	none, returns after delay
*	
******************************************************************************/
void dly(word mod){

	TPM1C0V = TPM1CNT + mod;
	TPM1C0SC = 0x10;					// timer set for output compare, output is GPIO
	while(!(TPM1C0SC_CH0F));			// wait for channel to overflow
}


/******************************************************************************
*	void dly_10us(void)	add delay 
*
*	delay routine in 10us increments,
*	
*	Input:	ticks - # of 1ms delay units
*
******************************************************************************/
void dly_10us(word ticks){

	byte n;  
	
	for(n=0; n<ticks; n++){	// 40 cycles * (1 * 6) loops = 240 ticks 
									// 240 ticks @ 24MHz = 10us 
		asm {
		LDA #0x01				// outer loop executes 1 time
		loop1:
			LDX #0x6				// inner loop - 6 loops
		loop:
			BRSET 0,0,0			// 5 cycles   
			BRSET 0,0,0			// 5 cycles   
			BRSET 0,0,0			// 5 cycles   
			BRSET 0,0,0			// 5 cycles   
			BRSET 0,0,0			// 5 cycles   
			BRSET 0,0,0			// 5 cycles   
			BRSET 0,0,0			// 5 cycles   
			NOP					// 1 cycle 
			NOP					// 1 cycle 
			DBNZX loop			// 3 cycles 
									// total: 7 * 5 + 2 + 3 = 40 cycles per loop
		DBNZA loop1   
		}
	}
}


