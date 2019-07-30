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



// JTAG functions for Freescale JM60 OSBDM

#include "derivative.h"				// include peripheral declarations
#include "typedef.h"				// Include peripheral declarations
#include "jtag_jm60.h"              // JTAG declarations for JM60 debug pod
#include "MCU.h"					// MCU header
#include "timer.h"				    // timer functions


void Jtag_UlongToScanData4 (unsigned long ulong_data, unsigned char * scandata)
{
    scandata[0] = (unsigned char)(ulong_data >>  0);
    scandata[1] = (unsigned char)(ulong_data >>  8);
    scandata[2] = (unsigned char)(ulong_data >> 16);
    scandata[3] = (unsigned char)(ulong_data >> 24);
}

unsigned long Jtag_ScanData4ToUlong (unsigned char * scandata)
{
    return (((unsigned long)(scandata[0])      ) |
            ((unsigned long)(scandata[1]) <<  8) |
            ((unsigned long)(scandata[2]) << 16) |
            ((unsigned long)(scandata[3]) << 24));
}

void Jtag_UlongToScanData3 (unsigned long ulong_data, unsigned char * scandata)
{
    scandata[0] = (unsigned char)(ulong_data >>  0);
    scandata[1] = (unsigned char)(ulong_data >>  8);
    scandata[2] = (unsigned char)(ulong_data >> 16);
}

unsigned long Jtag_ScanData3ToUlong (unsigned char * scandata)
{
    return (((unsigned long)(scandata[0])      ) |
            ((unsigned long)(scandata[1]) <<  8) |
            ((unsigned long)(scandata[2]) << 16));
}

void Jtag_UlongToScanData2 (unsigned long ulong_data, unsigned char * scandata)
{
    scandata[0] = (unsigned char)(ulong_data >>  0);
    scandata[1] = (unsigned char)(ulong_data >>  8);
}

unsigned long Jtag_ScanData2ToUlong (unsigned char * scandata)
{
    return (((unsigned long)(scandata[0])      ) |
            ((unsigned long)(scandata[1]) <<  8));
}

//----------------------------------------------------------------------------------------

#define jdly()		      dly(45)

void jdly_loop(int i)
{
	while(i > 0){
		jdly();
		--i;
	}
}

/*
	- Transition CLK line: low_high
	- Set or clear TDI_OUT for outgoing data
	- Read back TDO_IN for incoming data

	Returns the polarity of TDO_IN bit (readback from last write)

*/

//----------------------------------------------------------------------------------------
char TCLK_transition(char tms, char tdi)
{
	// set or clear TMS
	if(tms == 0)
        TMS_RESET();		// take TMS low
	else
        TMS_SET();	        // bring TMS high

	// set or clear TDI
	if(tdi == 0)
        TDI_OUT_RESET();	// Data out Low
	else
    	TDI_OUT_SET();		// Data out High

	jdly();				// delay
	TCLK_RESET();	// TCLK Low
	jdly();				// delay

	// return TDO status
	if(TDO_IN_SET){
		TCLK_SET();		// TCLK High
		return 1;	// return high
	}
	TCLK_SET();		// TCLK High
	return 0;	// return low
}

//-------------------------------------------------------------
// jtag scan i/o routine - input and output bytes using array pointers
//-------------------------------------------------------------
//	maximum transfer = 4 bytes
//	jtag_register:	DATA_REGISTER or INSTRUCTION_REGISTER

void Jtag_ScanIO (char jtag_register, char bitcount, unsigned char *out_data,
                  unsigned char *in_data, JTAG_STATE_TYPE state)
{
	char b, b8, inbit, lastbit;
	unsigned char outval;
	unsigned long inval, inmask;

	//bail out if invalid/empty scan
	if ((bitcount == 0) || (out_data == 0))
        return;

	// create input bitmask (data received msb first)
	lastbit = bitcount-1;
	inmask = 0x01;
	inmask <<= lastbit;
	inval=0;

	outval = *out_data;  // move to local variable so we don't change the original data
	b8=0;	             // reset byte counter

	// prepare JTAG for next data or instruction byte
	TCLK_transition(TMS_HIGH,0);	// Select Data Scan
	if (jtag_register == INSTRUCTION_REGISTER){
		TCLK_transition(TMS_HIGH,0);	// Select Instr Scan
	}
	TCLK_transition(TMS_LOW,0);		// Capture
	TCLK_transition(TMS_LOW,0);		// Shift

	// bang each bit out and receive a bit back each time
	for(b=0; b<bitcount; b++){
		if(b == lastbit)
            inbit = TCLK_transition(TMS_HIGH, (outval & 0x01));		// Shift transition  (Exit1)
		else
            inbit = TCLK_transition(TMS_LOW, (outval & 0x01));		// Shift transition

		if(b>0)
            inval >>=1;				// if not first, prepare for next input bit
		if(inbit>0)
            inval |= inmask;	    // add this bit to input byte
		outval >>=1;				// shift to next output bit

		// if we're done with this byte prep for next
		if(++b8 > 7){
			++out_data;	// point to next byte
			outval = *out_data;	// move to local variable so we don't change the original data
			b8=0;	// reset byte counter
		}

	}
	if(bitcount < 9)
         *(unsigned char  *)in_data = inval;	// return data as byte
	else if(bitcount < 17)
         //*(unsigned short *)in_data = inval;	// return data as short
         Jtag_UlongToScanData2(inval, in_data);
	else if(bitcount < 25)
         //*(unsigned short *)in_data = inval;	// return data as short
         Jtag_UlongToScanData3(inval, in_data);
	else if(bitcount < 33)
         //*(unsigned long  *)in_data = inval;	// return data as long
         Jtag_UlongToScanData4(inval, in_data);

	TCLK_transition(TMS_HIGH,0);	// Update
	TCLK_transition(TMS_LOW,0);		// Run-Test/Idle
}

//-------------------------------------------------------------
void Jtag_ScanIn(char jtag_register, char bitcount,
                  unsigned char *in_data, JTAG_STATE_TYPE state)
{
	unsigned char dummy_out[4];
	dummy_out[0] = dummy_out[1] = dummy_out[2] = dummy_out[3] = 0;
    Jtag_ScanIO(jtag_register, bitcount, dummy_out, in_data, state);
}

//-------------------------------------------------------------
void Jtag_ScanOut(char jtag_register, char bitcount,
                  unsigned char *out_data, JTAG_STATE_TYPE state)
{
	unsigned char dummy_in[4];	// ignore return
    Jtag_ScanIO(jtag_register, bitcount, out_data, dummy_in, state);
}
