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
*	File:		bdm_bgnd_driver.c
*
*	Purpose: 	BDM BGND driver file
*				Does not support the hardware handshake protocol
*
*
* 03/08/2011  : Modified by P&E Microcomputer Systems
*               http://www.pemicro.com/osbdm
*
*
*
\*****************************************************************************/

#include "derivative.h"			// Include peripheral declarations
#include "typedef.h"			// Type definitions
#include "MCU.h"				// MCU header
#include "timer.h"				// Timer functions
#include "bdm_bgnd_driver.h"    // Driver header
#include "target.h"				// Target specific definitions

// ---------------------------------------------------------------------------
// Internal definitions
// ---------------------------------------------------------------------------

// BgndReset() support definitions


// Internal prototypes

void BgndDetect (void);
void BgndTimerCompute (void);

// ---------------------------------------------------------------------------
// Internal variables
// ---------------------------------------------------------------------------

static byte   TimersInited = FALSE;	// 1 when timers are usable by the driver
static byte   ClockSet     = FALSE;	// TRUE when rate set manually, no sync permitted
static UINT16 syncTime;	// Sync interval in 24 MHz timer ticks
static UINT16 interval;			// The returned interval, assuming timeout

#pragma DATA_SEG __SHORT_SEG _DATA_ZEROPAGE
UINT16 bit1Time;	// Timer ticks representing the 1 bit pulse length
UINT16 bitThreshold;// Timer ticks representing the 0/1 threshold
UINT16 bit0Time;	// Timer ticks representing the 0 bit pulse length
UINT16 bitTime;		// Timer ticks representing the 16 target clocks interval
Pbyte  pBytes;		// Pointer into the TX/RX buffers
UINT8  bitsLeft;	// Number of remaining bits to be sent/received
UINT8  bytesLeft;	// Number of remaining bytes to be sent/received

UINT16 bitSTime;	// 


#pragma DATA_SEG DEFAULT

// ---------------------------------------------------------------------------
// Initialize the driver
// ---------------------------------------------------------------------------
	void
BgndInit (void)

	{
	TimersInited = FALSE;	// Ensure timers will be programmed before use
	ClockSet     = FALSE;	// Show the BDM clock is not manually configured
	BgndProgPower(0);		// Ensure programming power supply is off
	BgndProgEnable(0);		// Ensure programming voltage is not applied
	}

// ---------------------------------------------------------------------------
// Enable/Disable the 12 volt programming power (only used for 9RS08)
// ---------------------------------------------------------------------------
	void
BgndProgPower (
	byte  enable)

	{
	PTFD_PTFD0   = 0;		// Disable the programming voltage to the chip
#if 1
	PTFDD_PTFDD0 = 1;		// Make the enable pin an output
#endif
	TPM1C2SC     = 0;		// Disable the timer channel (release the pin)
	PTCD_PTCD5   = ((enable) ? (1) : (0));
#if 1
	PTCDD_PTCDD5 = 1;		// Make the programming power pin an output
#endif
	wait_ms(2);				// Time for power to settle
	}

// ---------------------------------------------------------------------------
// Enable/Disable the 12 volt programming power to the chip (restartable)
// ---------------------------------------------------------------------------
	void
BgndProgEnable (
	byte  enable)

	{
	PTFD_PTFD0     = 0;		// Disable the programming voltage to the chip
	RTCSC          = 0x80;	// Disable the timer (dismisses the flag)

	if (enable)
        {
		PTFD_PTFD0 = 1;		// Enable the programming voltage to the chip
		RTCMOD     = 5;		// Turn on the timer for 6 mSec
		RTCSC      = 0x33;	// RTCLKS = 01, RTIE = 1, RTCPS = 3
        }
	}

// ---------------------------------------------------------------------------
// VPP timer ISR, to ensure programming power disable
// ---------------------------------------------------------------------------
	void interrupt
VPP_ISR (void)
	{
	PTFD_PTFD0 = 0;			// Disable the programming voltage to the chip
	RTCSC      = 0x80;		// Disable the timer (dismisses the interrupt)
	if(FP_Stat == FP_RUNNING){	// if we're programming flash
		FP_Stat	= FP_TIMEOUT;	// set status - timeout waiting for flash algo to finish
		}
		
	VPP_Stat = VPP_TIMEOUT;

	}

// ---------------------------------------------------------------------------
// Get the current sync interval (in 60 MHz ticks)
// ---------------------------------------------------------------------------
	UINT16				// Returns the detected/configured sync interval
BgndGetClock (void)		// in 60 MHz ticks

	{
	UINT32  temp = syncTime;

	if (syncTime == 0)
		return (0);								// Not set yet, report don't know
	return (UINT16)(((temp * 5) + 1) >> 1);		// Set already, convert it
	}

/* ---------------------------------------------------------------------------
 Set the interface clock externally, using the interval (in 60 MHz ticks)
 This can be called in place of BgndSync() to configure for a specified rate

	Example - to set oscillator at 4Mhz:
	
	(1 / 4,000,000) * 256 / (1/60,000,000) = freq to set in 60Mhz ticks
	(1 / 4,000,000) * 15360000000 = 3840 (0xF00, which is the value sent from the PC)

	3840 / 2.5 = 1536 (Synctime)
	or
	(3840 * 2) / 5
	
	NOTE: lowest frequency this math will support is 500Khz
	
*/

// ---------------------------------------------------------------------------
	void
BgndSetClock (
	UINT16  request)	// The duration of the detected interval, in 60 MHz ticks

	{
	UINT32  temp = request;

	if (request != 0)
        {
		ClockSet = TRUE;
		syncTime = (UINT16)( (temp * 2) / 5);

        }
	else /* request == 0 */
		{
		ClockSet = FALSE;
		syncTime = BgndSync();
		}
	BgndTimerCompute();
	}

// ---------------------------------------------------------------------------
// Sync to the target interface bit clock by detecting the rate
// Also configures the driver parameters for the detected target bit clock
// ---------------------------------------------------------------------------
	UINT16			// Returns the detected sync interval, or 0  for timeout
BgndSync (void)

	{
        BgndDetect();			// Try to determine the target clock
	BgndTimerCompute();
	return (BgndGetClock());	// Return the current value
	}

// ---------------------------------------------------------------------------
//	Get the target bit clock timing information
// ---------------------------------------------------------------------------
	void
BgndDetect (void)	// Returns 0 if no target, or detection timeout

	{
	
	interval = 0; 					// clear interval timer
  
	TPM1SC        = 0;				// Clear/reset all timers
	TPM2SC        = 0;
	TPM1C3SC      = 0;
	TPM2C0SC      = 0;
	TPM2C1SC      = 0;

	TBGND_OUT     = 1;				// Set TBGND_OUT active high
	TBGND_OUT_DD  = 1;				// Signal is output

	TBGND_EN      = 1;				// Set TBGND_EN direction A to B
	TBGND_EN_DD   = 1;				// Signal is output, dir set A to B

	TPM1MOD       =					// Set timer modulus
	TPM2MOD       = 0x4000;			// Set timer modulus
	TPM1CNTH      = 0;				// Reset the counters so they will restart
	TPM2CNTH      = 0;				// into the modulus range, delta of 5 ticks

	TPM2C0V       = 0;				// Hold TBGND_OUT high when not pulsed
	TPM2C1V       = 0xFFFF;			// Hold TBGND_EN  low  when not pulsed

	TPM2C0SC      = 0x24;			// Enable the TBGND_OUT timer channel, pulses low
	TPM2C1SC      = 0x28;			// Enable the TBGND_EN  timer channel, pulses high
	TPM1C3SC      = 0x0c;				// Capture both edges

	TPM1SC        = 0x08;			// Start timer 1
	TPM2SC        = 0x08;			// Start timer 2

	TPM2C0V       = 0x20D0;			// Set TBGND_OUT pulse low  width
	TPM2C1V       = 0x20D2;			// Set TBGND_EN  pulse high width

	if (TPM2SC_TOF)
		TPM2SC_TOF = 0;

	while ( ! TPM2SC_TOF)			// Wait for rollover on timer 2 (start of sync pulse)
		;
	TPM2SC_TOF = 0;

	TPM2C0V       = 0;				// No second pulse on T2C0
	TPM2C1V       = 0xFFFF;			// No second pulse on T2C1

	if (TPM2C0SC_CH0F)
		TPM2C0SC_CH0F = 0;

	while ( ! TPM2C0SC_CH0F)		// Wait for end of sync pulse on TBGND_OUT
		;

	if (TPM1C3SC_CH3F){
    TPM1C3SC      = 0x0c;				// Capture both edges
	}

	TPM2C0SC      = 0;				// (5) Reset the falling transition capture channel


	while ( ! TPM2SC_TOF){	  // Wait for rollover on timer 2 (end of measurement)
  	if (TPM1C3SC_CH3F){
  	  interval = TPM1C3V;
      TPM1C3SC      = 0x0c;				// Capture both edges
      break;
  	}
	}
	while ( ! TPM2SC_TOF){	  // Wait for rollover on timer 2 (end of measurement)
    ;
	}
	if (TPM1C3SC_CH3F){
	  syncTime = TPM1C3V;
	}
	
	TPM2C0SC      = 0;				// Clear/reset all timers
	TPM2C1SC      = 0;
	TPM2SC        = 0;
	TPM1C3SC      = 0;
	TPM1SC        = 0;
	TBGND_EN      = 0;				// Set the bus transceiver to B to A (Hi-Z)

	TimersInited  = FALSE;

	if (syncTime != 0){
		syncTime      = (syncTime - interval);
	}

	}

// ---------------------------------------------------------------------------
// Assert hardware reset to target device
// ---------------------------------------------------------------------------
	byte				// Returns 0 for success; can't fail
BgndReset (
	byte  mode)			// 0 = Hard reset to BDM mode;
						// 1 = Hard reset to Running Mode;
						// 2 = Voltage reset to BDM mode
	{
	TPM1SC        = 0;			// Clear/reset all timers
	TPM2SC        = 0;
	TPM1C3SC      = 0;
	TPM2C0SC      = 0;
	TPM2C1SC      = 0;

	TBGND_EN      = 0;			// Prepare to set BGND pin hi-z, (B to A)
	TBGND_EN_DD   = 1;			// Set pin as out to transceiver

	TBGND_OUT     = 0;			// Prepare to set TBGND_OUT signal to low
	TBGND_OUT_DD  = 1;			// Set pin as out low to transceiver

	TRESET_OUT    = 0;			// Prepare to assert reset hi-Z to target
	TRESET_OUT_DD = 1; 			// Set pin as out to target (hi-Z)

	TPM2SC        = 0;			// Clear timer 2 TOF, set divider to 1
	TPM2C0SC      = 0;			// Set timer2 channel 0 pin as GPIO
	TPM2MOD       = 0x0800;		// Load timer2 count (85.3us pulse width) how long reset is active

	if (mode == 2)				// If voltage reset mode
		VTRG_EN   = 0;			// remove power from the target
	else
		TRESET_OUT = 1;			// Assert reset active low to target

	if (mode == 0 || mode == 2)	// If BDM active mode:
		TBGND_EN  = 1;			// Swap the transceiver to assert BGND low to target
								// Otherwise, it floats high to target
	TPM2SC        = 0x08;		// Start counting at bus rate clk
	while ( ! TPM2SC_TOF)		// Wait for timer2 TOF to set
		;

	TPM2SC_TOF    = 0;			// Clear TOF bit

	wait_ms(30); 				// Allow time for power off

//	if delay above is moved to only voltage reset mode, 
//	then this delay is required by the DEMO9S12XSFAME, no sure why
//	wait_ms(8); 				

	// If voltage reset mode
	if (mode == 2){
		VTRG_EN   = 1;			// restore power to the target
	}
	else
		TRESET_OUT = 0;			// De-assert reset out (back to hi-Z)

	if (mode == 0 || mode == 2)	// If BDM active mode:
		{						// Hold BGND low as target exits reset
                wait_ms(5);  // wait 5 ms after reset tristated, before bgnd tristated
		TBGND_OUT = 1;			// Raise the TBGND_OUT signal to high
		TBGND_EN  = 0;			// Swap the transceiver to float BGND back to hi-z 
		}

	TPM2SC        = 0;			// Disable timer 2
	TBGND_EN      = 0;			// Set the bus transceiver to B to A (Hi-Z)
	TimersInited  = FALSE;
	wait_ms(10); 				// Allow time to get running
	BgndSync();                             // Calculate timing parameters
	return (0);					// Return unconditional success
	}

// ---------------------------------------------------------------------------
// Internal definitions for the BGND W/R driver
// ---------------------------------------------------------------------------

#define  TPMINIT   0x08	// Enabled, bus clock div 1		used by the W/R driver
#define  TPM13INIT 0x04	// Enabled, input capture mode
#define  TPM20INIT 0x24	// Enabled, BGNDOUT pulses low
#define  TPM21INIT 0x28	// Enabled, A -> B

// Target bit clock support definitions for 24 MHz target clock

UINT16 const bit1Time24  =  4;	// Timer ticks representing the 1 bit pulse length @24MHz
UINT16 const bitThresh24 = 10;	// Timer ticks representing the threshold time
UINT16 const bit0Time24  = 13;	// Timer ticks representing the 0 bit pulse length @24MHz
UINT16 const bitTime24   = 50;	// Timer ticks representing the minimum bit time
//								// (required by the driver code)

// ---------------------------------------------------------------------------
// Sync to the target interface bit clock using the syncTime value
// ---------------------------------------------------------------------------
	void
BgndTimerCompute (void)

	{
/*
	
//	if (syncTime != 0)
	if (syncTime >= 0x83)     // 24Mhz is as fast as we can run
        {
			// Bit1 time is 4/16 of a bit time, rounded
		bit1Time     = (syncTime + 16) / 32;
			// Bit threshold is 10/16 of a bit time, rounded
		bitThreshold = ((((syncTime + 8) / 16) * 10) + 4) / 8;
			// Bit0 time is 13/16 of a bit time, rounded
		bit0Time     = ((((syncTime + 8) / 16) * 13) + 4) / 8;
			// Bit time is rounded, and must be slightly > target bit time
		bitTime      = ((syncTime + 4) / 8) + 1;

		if (bitTime < bitTime24)
			bitTime = bitTime24;		// Minimum value needed by firmware

  	// In this computation, the +5 is because TPM1 is 5 ticks higher than TPM2
	  // In this computation, the +1 is because TPM1 capture is low by one
    bitThreshold += 5 + 1;  
        }

	else // Default initialization; for 24 MHz target bit clock
		{
		bit1Time     = bit1Time24;
//		bitThreshold = bitThresh24;
		bitThreshold = 14;
		bit0Time     = bit0Time24;
		bitTime      = bitTime24;
		}
*/

#asm
// Divide by 8 with round up
Div8r:
  ldhx syncTime
  pshh 
  txa
  pulx
  
  lsrx  ; divide by 8
  rora
  lsrx 
  rora
  lsrx 
  rora
  adc #0
  bcc bgndTC01  ; round up
  aix #1        ; save carry to high byte
bgndTC01:
  pshx			; save bitTime, 16 target BDC clock periods
  psha

  lsrx			; compute 8 clock time 
  rora

  adc #0
  bcc bgndTC02  ; round up
  aix #1
bgndTC02:

  pshx  ; save 8 cycle time
  psha  
  lsrx 
  rora

  adc #0
  bcc bgndTC03  ; round up
  aix #1
bgndTC03:

  pshx  ; save 4 cycle time
  psha
  
  lsrx  ; compute 2 cycle time
  rora

  adc #0
  bcc bgndTC04  ; round up
  aix #1        ; add carry to high byte
bgndTC04:
  
  pshx  ; save 2 cycle time
  psha

  pulx  ; pull 2 cycle time
  pulh
  sthx	bitThreshold	; save 2 cycle time

  pulx  ; pull 4 cycle time
  pulh
  sthx	bit1Time	; save 4 cycle time

  pulx  ; pull 8 cycle time
  pulh
  sthx bit0Time ; save 8 cycle time

  pulx         ; pull 16 cycle time
  pulh
  sthx bitTime ; save 16 cycle time
#endasm

  bitThreshold += ((bit1Time*2) + bitThreshold); // bitThreshold = 2 + 8 + 2 = 12 cycles
  bit0Time += bit1Time;     // Bit0 = 4 + 8 = 12 cycles
//	bitSTime = bit0Time + 10;  

	bitTime = bitTime + (bit1Time/2); // add 2BDC cycles at the end of each bit read }

	if (bitTime < bitTime24){
		bitTime = bitTime24;		// Minimum value needed by firmware
	}
	bitSTime = bitTime - 10;  


//  bitThreshold += 5 + 2;  // this worked with gold version
    bitThreshold +=6; // This compensates for a mismatch between the 1st and the second timer

	TimersInited  = FALSE;
	}

// ---------------------------------------------------------------------------
// Initialize the driver timers
// ---------------------------------------------------------------------------
	void
BgndInitTimers (void)

	{
	TPM1SC    = 0;			// Stop all the timers
	TPM2SC    = 0;
	TPM1C3SC  = 0;
	TPM2C0SC  = 0;
	TPM2C1SC  = 0;

	TPM1MOD   = 1; 			// Set modulus low for fast register update
	TPM2MOD   = 1;

	TPM1CNTH  = 0; 			// Reset the counters so they will restart
	TPM2CNTH  = 0;			// into the modulus range

	// Resetting the timers as follows results in TPM1 being 5 counts higher
	// than TPM2, which is compensated in the computation of bitThreshold.
	// See BgndRate() for this computation.

	TPM1C3SC  = TPM13INIT;	// Initialize the receive capture mode
	TPM2C0SC  = TPM20INIT;	// Enable the BGND out timer channel, pulses low
	TPM2C1SC  = TPM21INIT;	// Set the XCVR out timer channel HIGH (Output to target)

	TPM2C0V   = 0;			// Output will start after setting TPMnCmV != 0
	TPM2C1V   = 0xFFFF;		// Set to hold XCVR in Output mode (hi level)

	TPM1MOD   = bitTime; 	// Set the real bit time periods
	TPM2MOD   = bitTime;

	TPM1SC    = TPMINIT;	// Start the RX timer counter (must be first)
	TPM2SC    = TPMINIT;	// Start the TX timer counter (must be second)

	TPM1CNTH  = 0; 			// Reset the counters so they will restart
	TPM2CNTH  = 0;			// into the current modulus range

	wait_ms(1);
	TimersInited = TRUE;
	}

// ---------------------------------------------------------------------------
// Report whether the device/driver supports the ACK protocol
// ACK support:  WAIT = 0,  ACKN = 1
// ---------------------------------------------------------------------------
	byte
BgndAckSupported (void)

	{
	return (0);		// This driver does not support the ACK protocol
	}

// ---------------------------------------------------------------------------
// Report any detected reset occurences
// Reset detect:  No = 0,  Yes = 1
// ---------------------------------------------------------------------------
	byte
BgndResetDetect (void)

	{
	return t_rsti();	// check if target is in reset
	}

// ---------------------------------------------------------------------------
// Report the state of the BDM clock configuration
// Clock mode:  NO connection = 0,  SYNC = 1,  Manual set = 3
// ---------------------------------------------------------------------------
	byte
BgndClockState (void)

	{
	return ((syncTime == 0) ? 0 : (ClockSet ? 3 : 1));
	}

// ---------------------------------------------------------------------------
// Unsecure flash by erasing it and resetting the security code
// ---------------------------------------------------------------------------
	void
BgndUnsecure (void)		// This is implemented a different way elsewhere

	{
	}

// ---------------------------------------------------------------------------
// drive BGND line high
// ---------------------------------------------------------------------------
byte BgndHigh() 
{
	TPM2SC        = 0;        // release BGND control pins from the timer
	                          // timer will be reconfigured during next data 
	                          // rxtx  
  TimersInited = FALSE;     // make sure timers get re-initialized during the 
                            // next transmission  
	TBGND_OUT     = 1;				// Set TBGND_OUT active high
	TBGND_OUT_DD  = 1;				// Signal is output

	TBGND_EN      = 1;				// Set TBGND_EN direction A to B
	TBGND_EN_DD   = 1;				// Signal is output, dir set A to B  
}

// ---------------------------------------------------------------------------
// drive BGND line low
// ---------------------------------------------------------------------------
byte BgndLow() 
{
	TPM2SC        = 0;        // release BGND control pins from the timer
	                          // timer will be reconfigured during next data 
	                          // rxtx  
  TimersInited = FALSE;     // make sure timers get re-initialized during the 
                            // next transmission  
	TBGND_OUT     = 0;				// Set TBGND_OUT active low
	TBGND_OUT_DD  = 1;				// Signal is output

	TBGND_EN      = 1;				// Set TBGND_EN direction A to B
	TBGND_EN_DD   = 1;				// Signal is output, dir set A to B  

  return (0);
}

// ---------------------------------------------------------------------------
// set RESET line to Hi-Z
// ---------------------------------------------------------------------------
byte ResetHiz() 
{
	TRESET_OUT    = 0;			// Prepare to assert reset hi-Z to target
	TRESET_OUT_DD = 1; 			// Set pin as out to target (hi-Z)
}

// ---------------------------------------------------------------------------
// drive RESET line low
// ---------------------------------------------------------------------------
byte ResetLow() 
{
	TRESET_OUT    = 1;			// Prepare to drive reset line low
	TRESET_OUT_DD = 1; 			// Drive reset line low 
}


// ---------------------------------------------------------------------------
// set BGND line to Hi-Z
// ---------------------------------------------------------------------------
byte BgndHiz() 
{
  TBGND_EN      = 0;			// Prepare to set BGND pin hi-z, (B to A)
	TBGND_EN_DD   = 1;			// Set pin as out to transceiver
  return (0);
}
// ---------------------------------------------------------------------------
// separate delay for individual Read/Write byte transmissions
// ---------------------------------------------------------------------------

byte BgndDelay() 
{
#asm	
lda	#BDMRXDELAY		; 2, Load the RX extra delay count, **!! never 0
rxdelay:
	brclr	7, TPM2SC, rxdelay	; 5, Await the current bit period end
	mov	#TPMINIT, TPM2SC	; 4, Reset the flag bit
	dbnza	rxdelay			; 7, Wait until exit delay is complete,
#endasm
return (0);
}
// ---------------------------------------------------------------------------
// Perform a requested write followed by read via the hardware
// --------------------------------------------------------------------------- 

	byte				// Returns 0 for success, else 1 for ACK timeout
BgndTxRx (
	Pbyte  pTxData,
	Pbyte  pRxData,
	byte    TxCount,
	byte    RxCount)

	{
	byte    ccr;			// Saved interrupt status
	byte    status = 0;		// Returned status

	if ( ! TimersInited)		// Ensure sure the timers are running
		BgndInitTimers();

// set up Reset input detect, just in case Soft reset to BGND mode...

	KBIES 	= 0x04;		//  Rising edge detect
	KBIPE	= 0x04;			//  enable Port D2...
	KBISC	= 0x04;			//  set mode and clear flag...

#asm
	tpa				; Condition codes to A
	sta	ccr			; Save old interrupt status
	sei				; Disable interrupts

// Perform the TxData write, if requested

	ldhx	#$FFFF			; Ensure bus transceiver is constant Output mode (A to B)
	sthx	TPM2C1VH

	ldx	TxCount			; Init bytesLeft (in DIRECT space)
	beq	txdone			; Write not requested (never legit)

	stx	bytesLeft
	ldhx	pTxData			; Init pBytes (in DIRECT space)
wloop1:
	brclr	7, TPM2SC, wloop1	; 5, Synchronize to the bit clock
	mov	#TPMINIT, TPM2SC	; 4, Reset the flag bit
	bra	entertx			; 3, Enter the main TX loop; x:h->TxData

; Send buffer loop

nexttxbyte:
	ldhx	pBytes			; 4, Get and increment the data pointer
	aix	#1			; 2

entertx:
	sthx	pBytes			; 4, Save the data pointer
	lda	,x			; 3, Get the first/next byte
	ldx	#8			; 2, Set the remaining bit loop count
	stx	bitsLeft		; 3

; Send byte loop

nexttxbit:
	lsla				; 1, Test the MSB and shift it one
	bcs	use1bit			; 3

	ldhx	bit0Time		; 4, Use the 0-bit timer value
wloop3:
	brclr	7, TPM2SC, wloop3	; 5, Await the current bit pulse end
	mov	#TPMINIT, TPM2SC	; 4, Reset the flag bit
	sthx	TPM2C0VH		; 4, Set the next bit time
	dbnz	bitsLeft,  nexttxbit	; 7, Continue while 7 bits not done
	bra	wbit8			; 3, Go complete the eight bit

use1bit:
	ldhx	bit1Time		; 4, Use the 1-bit timer value
wloop4:
	brclr	7, TPM2SC, wloop4	; 5, Await the current bit pulse end
	mov	#TPMINIT, TPM2SC	; 4, Reset the flag bit
	sthx	TPM2C0VH		; 4, Set the next bit time
	dbnz	bitsLeft, nexttxbit	; 7, Continue while 7 bits not done

; Await the bit 7 completion, and the start of bit 8

wbit8:
	clrh				; 1, Set zero pulse length to hold
	clrx   				; 1, the output high for a bit time
wloop5:
	brclr	7, TPM2SC, wloop5	; 5, Await the bit 7 -8 transition
	mov	#TPMINIT, TPM2SC	; 4, Reset the flag bit
	sthx	TPM2C0VH		; 4, Set the idle bit value (0), BGND output = 1
	dbnz	bytesLeft, nexttxbyte	; 7, Continue while buffer not done

txdone:	

; Prepare to receive RX data input: initiate reception, if requested

	ldx	RxCount			; 3, Init bytesLeft (in DIRECT space)
	bpl	BgndRx			; 3, Go receive with valid byte count

;*** Soft Reset to BDM mode here, RxCount = 0x80 or higher....
; Force BGND low now....
	
	ldhx	#0xFFFF			; 3, set the timer to do it
	sthx	TPM2C0VH		; 4, BGND low on next bit timer out

SR_BDM1:
	brclr	7, TPM2SC, SR_BDM1	; 5, Await the current bit period end

; setup timeout
	clrh				; set timer to free running
	clrx
	stx	TPM1SC		; Timer 1 is off
	sthx	TPM1MODH	; set max timer (2.73 ms) (low timer byte)
	mov	#TPMINIT, TPM1SC	; 4, Start the timer again
	stx	TPM1CNTH	; Reset timer counter to 0 for msx time

;	clra				; set max timeout = 700 ms  (high timer byte)
	lda	#0x70	; set timeout = 300 ms  (high timer byte)

; BGND is low now, wait for Reset edge...
SR_BDM2:
;	BRSET	3, KBISC, SR_BDM2a	; finish if reset high	

; check for timeout
	BRCLR 7, TPM1SC, SR_BDM2	; check timout timer (LB)
	mov	#TPMINIT, TPM1SC	; 4, Reset the flag bit
	dbnza 	SR_BDM2				; check timeout timer (HB)
SR_BDM2a:

	mov	#TPMINIT, TPM2SC	; 4, Reset the flag bit
; already clear
;	clrh				; 1, Set zero pulse length to hold
;	clrx   				; 1, the output high for a bit time
SR_BDM3:
	brclr	7, TPM2SC, SR_BDM3	; 5, Await the current bit period end
	mov	#TPMINIT, TPM2SC	; 4, Reset the flag bit
	sthx	TPM2C0VH		; 4, Set the idle bit value (0), BGND output = 1

SR_BDM4:
	brclr	7, TPM2SC, SR_BDM4	; 5, Await the current bit period end
	mov	#TPMINIT, TPM2SC	; 4, Reset the flag bit
	bra	rxdone			; finsihed

;............................................................
; Receive bytes here....

BgndRx:
	bne	bgndrxstart			; 3, Read requested

#if defined __9S12__
	; need to delay 150 cycles at the end of a command if it is not a read (9S12G128)
	lda	#BDMRXDELAY		; 2, Load the RX extra delay count, **!! never 0
writeenddelay:
	brclr	7, TPM2SC, writeenddelay	; 5, Await the current bit period end
	mov	#TPMINIT, TPM2SC	; 4, Reset the flag bit
	dbnza	writeenddelay			; 7, Wait until exit delay is complete,
#endif


	bra	rxdone			; 3, Read not requested; just return

bgndrxstart:

	stx	bytesLeft		; 3
	ldhx	pRxData			; 4, Init pBytes (in DIRECT space)
	sthx	pBytes			; 4, 

; Receive buffer loop

enterRX:
	lda	#BDMRXDELAY		; 2, Load the RX extra delay count, **!! never 0
rxdelay:
	brclr	7, TPM2SC, rxdelay	; 5, Await the current bit period end
	mov	#TPMINIT, TPM2SC	; 4, Reset the flag bit
	dbnza	rxdelay			; 7, Wait until exit delay is complete,

	ldhx	bit1Time		; 4, Start alternating the bus transceiver for
	sthx	TPM2C1VH		; 4, a pulsed turnaround of out to in (B to A) on next TOF

	ldx	#6			; 2, Set/Reset the bit count
	stx	bitsLeft		; 3

; Synchronize the receive byte process to the bit clock TOF

rloop1:
	brclr	7, TPM2SC, rloop1	; 5, Await the current bit period end
	mov	#TPMINIT, TPM2SC	; 4, Reset the flag bit

	ldhx	bitSTime		; 4, Set the TPM2C0V register to start
	sthx	TPM2C0VH		; 4, generating TBGND_OUT pulses starting next TOF

rloop2:
	brclr	7, TPM2SC, rloop2	; 5, Await the start of the first bit
	mov	#TPMINIT, TPM2SC	; 4, Reset the flag bit

	tst	TPM2C0SC		; 4, Preclear the bit period TOF flag bit
	mov	#TPM20INIT, TPM2C0SC	; 4, Reset the flag bit

; Receive byte loop

bits0_5:
	brclr	7, TPM2C0SC, bits0_5	; 5, Await the bit 0-5 period end TOF
	ldhx	TPM1C3VH				; 4, Read the capture register
	mov	#TPM20INIT, TPM2C0SC	; 4, Reset the flag bit
	
	cphx	bitThreshold		; 5, Compare with the threshold
	rola				; 1, Insert the new bit into A from the right

	dbnz	bitsLeft, bits0_5	; 7, Continue if bits 0-5 not received

bit6:
	brclr	7, TPM2C0SC, bit6	; 5, Await the bit 6 period end TOF
	ldhx	TPM1C3VH		; 4, Read the capture register
	mov	#TPM20INIT, TPM2C0SC	; 4, Reset the flag bit

	cphx	bitThreshold		; 5, Compare with the threshold
	rola				; 1, Insert the new bit into A from the right

	clrh				; 1, Stop transmission of RX start pulses
	clrx				; 1, after bit 7
	sthx	TPM2C0VH		; 4
bit7:
	brclr	7, TPM2C0SC, bit7	; 5, Await the bit 7 period end TOF

	ldhx	TPM1C3VH		; 4, Read the capture register

	cphx	bitThreshold		; 5, Compare with the threshold
	rola				; 1, Insert the new bit into A from the right

	ldhx	pBytes			; 4, Get the data ptr and store the data
	sta	,X			; 2

	aix	#1			; 2, Increment and save the data ptr
	sthx	pBytes			; 4, Save the data pointer

	dbnz	bytesLeft, enterRX	; 7, Continue if all bytes not received

; Reception of RX data is complete; do the exit delay, if requested

rxdone:

	lda	#BDMEXITDELAY		; Load the exit delay count, **!! never 0
exitdelay:
	brclr	7, TPM2SC, exitdelay	; Await the current bit period end
	mov	#TPMINIT, TPM2SC	; Reset the flag bit
	dbnza	exitdelay		; Wait until the exit delay is complete

txrxexit:
	clrh				; Set transceiver back to Input (constant B to A, Hi-Z)
	clrx				;
	sthx	TPM2C1VH		;
	sthx	TPM2C0VH		; 

	lda	ccr			; Restore original interrupt status
	tap				; to the condition code register
#endasm

	// if doing software reset to halt, reinit timers next time
	if(RxCount >= 0x80)
		TimersInited = FALSE;	// Ensure timers will be programmed before use

	return (status);
	}


//---------------------------------- EOF ------------------------------------

// BWJ TEMPORARY CODE TO TEST THE +12V GENERATOR
#if 0
	void
PulsePower (void)

	{
	BgndProgPower(1);

	BgndProgEnable(1);
	dly_10us(2);
	BgndProgEnable(0);
	wait_ms(1);

	BgndProgPower(0);
	wait_ms(20);
	}

	void
testgen (void)
	{
	for (;;)
        PulsePower();
	}
#endif
// BWJ END OF TEMPORARY CODE TO TEST THE +12V GENERATOR

//---------------------------------- EOF ------------------------------------
