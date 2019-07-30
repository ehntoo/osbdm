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
*	File:			bdm_cf.c
*
*	Purpose: 	ColdFire BDM command file.  Contains commands specific to 
*					V2/V3/V4 ColdFire devices
*
*	History:		April 2008, Initial Developement
*
* 03/08/2011  : Modified by P&E Microcomputer Systems
*               http://www.pemicro.com/osbdm
*
*
*
******************************************************************************/ 

#include "derivative.h"				// include peripheral declarations  
#include "typedef.h"				// Include peripheral declarations  
#include "targetAPI.h"				// target API include file
#include "bdm_cf.h"						// BDM include file
#include "board_id.h"			
#include "commands.h"				// BDM commands header file
#include "USB_User_API.h"			// USB EP configurations
#include "MCU.h"						// MCU header
#include "timer.h"				   // timer functions
#include "util.h"				// utility functions
#include "serial_io.h"        // Serial port handling/enable


//---------------------------------------------------------------------------
// Firmware Info
//---------------------------------------------------------------------------

volatile const byte TARGET_TYPE  =  eCFv234;

#ifdef __EMBEDDED__ 
  volatile const byte FIRMWARE_TYPE  =  eEmbeddedGeneric;
#else
  volatile const byte FIRMWARE_TYPE  =  eGeneric;
#endif 



// ---------------------------------------------------------------------------
// Function Prototypes
// ---------------------------------------------------------------------------


word t_txrx(word spdata);			// sends a word of data out SPI port and receives a word or data 

static byte txBuff [16];				// Buffer for write bytes of BGND protocol
static byte rxBuff [16];				// Buffer for read  bytes of BGND protocol


// Global Variables
word mod;								// SPI modulus count
byte sStatus;							// SPI transmit status
byte eStatus=0;							// BDMERROR_ status.  0 if no error
word rBuf;
word hold_sendword;
word hold_recvword;
word hold_block_length;
unsigned long hold_sendaddress;
unsigned long hold_senddata; 
unsigned int addrH;		// get high address word
unsigned int addrL;	// get low address word


/******************************************************************************
*	t_init(void)
*	
*	check target power status and enable Vout if target not self-powered, enable
* 	and initialized BDM signals to target, start internal timer, issue RSTO to 
*	target,and resynchronize to ensure communications
*
*	Input:	none
*	Return:	0 on success, 1 on failure
*
******************************************************************************/
byte t_init(void) {
		
	if(bdm_vtrg_en()) return 1;	// enable Vout to target if not self-powered

	// Enable BDM_CF interface to target
	// Under CW see: Target Setting, Compiler, Options, Language, Preprocessor Definitions
#ifdef __EMBEDDED__  
	OUT_EN = 1;		// init BDM signals for embedded CF BDM - OUT_EN is active high
#else
	OUT_EN = 0;		// init BDM signals for CF BDM cable - OUT_EN is active low 	
#endif
	PTEDD = 0xE4;						// TDSCLK_EN, DOUT, SCLK_OUT, OUT_EN all outputs

//	if(t_reset(0)) return 1;		// reset target and break

//	t_rsto();						// issue target reset, 300ms delay on return
//	if(t_resync()) return 1;	// init failed

 	return 0;						// return success
}

// Impliment a hard reset on the target
// reset_mode:  0 = reset to BDM mode, 1 = reset to Normal Mode
void t_hard_reset(byte reset_mode){

	tRSTO = 1;					// assert reset_out signal
	tRSTO_DIR = 1;				// drive the signal out
	if(reset_mode == 0){				// check requested RESET mode
		BRK_TMS = 0;			// assert BKPT signal to target
	}
	wait_ms(50);				// wait for 50ms
	tRSTO_DIR = 0;				// RSTO pin is an input
	
	wait_ms(100);				// wait for target to exit reset and see BRK signal asserted
		
	if(reset_mode == 0){
		BRK_TMS = 1;			// de-assert BKPT signal to target
	}
}

// Impliment a power on reset on the target
// reset_mode:  0 = reset to BDM mode, 1 = reset to Normal Mode
void t_power_reset(byte reset_mode){

	VTRG_EN = 0;			// disable BDM voltage switch
	if(reset_mode == 0){				// check requested RESET mode
		BRK_TMS = 0;			// assert BKPT signal to target
	}
	wait_ms(500);				// wait (in ms) for power to drain
	(void)bdm_vtrg_en();	// enable Vout to target if not self-powered
		
	if(reset_mode == 0){
		BRK_TMS = 1;			// de-assert BKPT signal to target
	}
}

void t_debug_init()
{

}

void t_serial_init()
{
if ((PTEDD & 0x80) == 0x00) {
  t_init();
}

}


/******************************************************************************
*	t_reset(mode)
*	
*	assert reset to target device.  tRSTO is inverted version of tRST to target
*	
*	Input:	
*				Mode:  0 soft reset to BDM mode, 
*	             1 soft reset to Running mode
*              2 hard reset to BDM mode, 
*	             3 hard reset to Running mode, 
*	             4 voltage reset to BDM mode (bdm must supply power)
*	             
*
*	Return:	0 on success, 1 on fail
*
******************************************************************************/
byte t_reset(ResetT mode){

//	byte cmd;

  switch(mode){
    case eSoftReset_to_DebugMode: // Soft Reset to BDM mode
/*
			BRK_TMS = 0;			// assert BKPT signal to target
			// write 0x80 to Reset Control Register (RCR)
			cmd = 0xC0;	// Soft reset command
			if(t_write_mem(MEM_RAM, 0x40110000, 8, 1, &cmd))	return 1;	// write soft reset command
			wait_ms(100);				// wait for target to exit reset
			BRK_TMS = 1;			// de-assert BKPT signal to target
*/
			// Soft reset command not working, use hard reset instead
      t_hard_reset(0);	// hard reset to BDM (halted) mode
      break;

    case eHardReset_to_NormalMode:  
      t_hard_reset(1);	// hard reset to normal (running) mode
      break;

    case eHardReset_to_DebugMode:
      t_hard_reset(0);	// hard reset to BDM (halted) mode
      break;

    case ePowerReset_to_DebugMode:
      t_power_reset(0);  // voltage reset to BDM (halted) mode
			// BDM not halting after voltage reset so we're using the hard reset to get it to halt on reset
      t_hard_reset(0);	// hard reset to BDM (halted) mode
      break;

    default:
/*
			// write 0x80 to Reset Control Register (RCR)
			cmd = 0xC0;	// Soft reset command
			if(t_write_mem(MEM_RAM, 0x40110000, 8, 1, &cmd))	return 1;	// write soft reset command
			wait_ms(100);				// wait for target to exit reset
*/
			// Soft reset command not working, use hard reset instead
      t_hard_reset(1);	// hard reset to normal (running) mode
      break;
  }

	if(t_resync()) return 1;	// reset failed
	
	return 0;					// return success
}

/******************************************************************************
*	t_txrx(type spdata)
*	
*	low level SPI send / receive data routine.  Sends 17bits of data to target and 
*	receives 17 bits of response.  Return status is captured in global variable 
*	sStatus[7].  
*	
*	Input:	17bit of SPI data to send to target
*
*	Output:	16bits of SPI data returned from target.  Return status bit captured
*				in global variable sStatus[7]
*
******************************************************************************/
word t_txrx(word spdata){
	
	word spi_rx_data;
	
	sStatus = 1;						// set status bit to fail
	SPI1C1_SPE = 0;					// disable SPI module

// create table for mod values based on baud rate
	mod = 0x20;							// load SPI mod count for 1/2 bit delay

	TDSCLK_EN = 1;						// enable TDSCLK for 1st clock to target
	dly(mod);							// add delay for 1/2 SPI bit time
	TDSCLK_EN = 0;						// clear TDSCLK 
	dly(mod);							// add delay for 1/2 SPI bit time
	SPI1C1_SPE = 1;					// enable SPI module
	TDSCLK_EN_DIR = 0;				// TDSCLK input  

	while (!(SPI1S_SPTEF));			// wait until SPI Tx Empty Flag is set
	SPI1D16 =  spdata;				// send data
	while (!(SPI1S_SPRF));			// wait for SPI Rx Full Flag to set to indicate transfer complete
	dly(mod);							// wait for final bit from target to settle
	
	spi_rx_data = SPI1D16;			// store SPI receive data

	TDSCLK_EN_DIR = 1;				// TDSCLK output
	SPI1C1_SPE = 0;					// disable SPI module

	sStatus = (spi_rx_data >> 8);	// SPI status bit in sStatus[8]
 	sStatus &= 0x80;
 		
	spi_rx_data <<= 1;				// shift data to make room for last data bit

	if(PTED_PTED4){
		spi_rx_data |= 0x01;		// get last SPI data bit
	}
	return(spi_rx_data);				// return data
}


/*------------------------------------------------------------------------------
  Transmit a command and receive a single byte or word of data (size of width) back
	width:8	 = receive 1 byte back
	width:16 = receive 2 bytes back

 	e:1 = store data Big-endian
 	e:0 = store data Little-endian
  
  returns 0 on success
  on return  global "eStatus" will have 0 on success or a BDMERROR_ status
*/
int bdmcf_rx(unsigned char width, char e, unsigned int cmd, unsigned char *data){

	word rval;
	int i;
	
	rval = t_txrx(cmd);	// transmit one word to the bdm

	for(i=0; i<20; i++){
		eStatus = 0;	// reset global error status
		// global sStatus has status
		if(sStatus != 0){
			// if an error was returned, set error flag
			switch(rval){
				case 0:		 eStatus = BDMERROR_NOT_READY; break;	// Not ready response from chip
				case 1:		 eStatus = BDMERROR_BERR; break;		// BERR Terminated bus cycle - Debugger Supplied DSACK
				case 0xFFFF: eStatus = BDMERROR_ILLEGAL; break;		// Illegal command error from CPU
			}
			if(rval != 0xFFFF){
				// -- delay before retry may be needed here
				rval = t_txrx(BDMCF_CMD_NOP);	// transmit NOP to the bdm
				continue;	// go back and retry 
			}
			return 1;	// return error, command wasn't sent correctly
		}else{
			switch(width){
				case 8:
					*data = (unsigned char) rval;
					break;
				default:	
					if(e==1){	// Store Big-endian
						*data++ = (unsigned char)(rval >> 8);
						*data	= (unsigned char)(rval & 0xFF);
					}else{		// Store Little-endian
						*data++ = (unsigned char)(rval & 0xFF);
						*data   = (unsigned char)(rval >> 8);
					}
					break;
			}
			return 0;	// return OK, command sent correctly
		}
	}		
	return 1;	// return error, command wasn't sent correctly
}

/*------------------------------------------------------------------------------
  Transmit a single data (size of width) unit back from the bdm
*/

void bdmcf_tx_msg(unsigned int data){
	rBuf = t_txrx(data);	// transmit one word to the bdm, ignore return value
}	

void bdmcf_tx_cmdaddr(unsigned int data){
	bdmcf_tx_msg(data);  			// send data
	bdmcf_tx_msg(addrH);  			 // send address
	bdmcf_tx_msg(addrL);  			
}	

// send a 32-bit value as 2 words from the buffer pointer
void bdmcf_tx_32(unsigned char *data){
	unsigned long val;
	val = getbuf4(data);		// get 32-bit value from the buffer
	bdmcf_tx_msg(val >> 16);	// send data - high word
	bdmcf_tx_msg(val & 0xFFFF);	// send data - low word
}	


#define	AD_BIT	0x0008;

// Write A/D register
int t_write_ad(unsigned long addr, unsigned char *data){
	unsigned char rbuf[4];	// holds unused return values
	unsigned int cmd;
		
	addr = 0x000000FF & addr;
	
	cmd = BDMCF_CMD_WAREG;	// comand
	if(addr > 7){
		addr -= 8;
		cmd += AD_BIT;	// if this is an ADDRESS register set AD bit
	}

	bdmcf_tx_msg(cmd+addr);		// send command + address
	bdmcf_tx_32(data);			// send data
	if(bdmcf_rx(16, 0, BDMCF_CMD_NOP, rbuf))	return 1;	// send NOP to get return value, return if error
	return 0;
}


// Read A/D register
int t_read_ad(unsigned long addr, unsigned char *data){
	unsigned int cmd;
		
	addr = 0x000000FF & addr;
	
	cmd = BDMCF_CMD_RAREG;	// command
	if(addr > 7){
		addr -= 8;
		cmd += AD_BIT;	// if this is an ADDRESS register set AD bit
	}
	cmd += addr;	// add register number

	bdmcf_tx_msg(cmd);  			// send command

	// we're reversed the order of the returned words for INTEL processors
	if(bdmcf_rx(16,0, BDMCF_CMD_NOP, data+2))	return 1;	// send NOP to get return value, return if error
	if(bdmcf_rx(16,0, BDMCF_CMD_NOP, data))	return 1;	// send NOP to get return value, return if error
	return 0;
}

// Write Control Register
int t_write_creg(unsigned long addr, unsigned char *data){
	unsigned char rbuf[4];	// holds unused return values

	addrH = (addr >> 16);		// get high address word
	addrL = (addr & 0xFFFF);	// get low address word

    bdmcf_tx_cmdaddr(BDMCF_CMD_WCREG);	// send the command and address
	bdmcf_tx_32(data);			// send data
	if(bdmcf_rx(16,0, BDMCF_CMD_NOP, rbuf))	return 1;	// send NOP to get return value, return if error
	return 0;
}

// Read Control Register
int t_read_creg(unsigned long addr, unsigned char *data){

	addrH = (addr >> 16);		// get high address word
	addrL = (addr & 0xFFFF);	// get low address word

    bdmcf_tx_cmdaddr(BDMCF_CMD_RCREG);	// send the command and address
	
	if(bdmcf_rx(16,0, BDMCF_CMD_NOP, data+2))	return 1;	// send NOP to get return value, return if error
	if(bdmcf_rx(16,0, BDMCF_CMD_NOP, data))	return 1;	// send NOP to get return value, return if error
	return 0;
}

//---------------------------------------------------------------------------
// Write Debug Register
// addr is 5-bit DRc number (first register is 0)
#pragma MESSAGE DISABLE C5703
int t_write_dreg(unsigned long addr, unsigned char uWidth, unsigned char *data){

//	unsigned long val;
//	val = getbuf4(data);		// get 32-bit value from the buffer
//	return bdm_wbdreg( (word)(addr+BDMCF_WDMREG_CSR), val);

	unsigned char rbuf[4];	// holds unused return values

	bdmcf_tx_msg((word) (BDMCF_WDMREG_CSR + addr) );		// send command + address
	bdmcf_tx_32(data);			// send data
	if(bdmcf_rx(16, 0, BDMCF_CMD_NOP, rbuf))	return 1;	// send NOP to get return value, return if error
	return 0;
}
#pragma MESSAGE DEFAULT C5703

// Read Debug Register
// addr is 5-bit DRc number (first register is 0)
#pragma MESSAGE DISABLE C5703
int t_read_dreg(unsigned long addr, unsigned char uWidth, unsigned char *data){
//	dword val;
//	if(bdm_rbdreg( (word)(addr+BDMCF_RDMREG_CSR), &val)) return 1;
	return bdm_rbdreg( (word)(addr+BDMCF_RDMREG_CSR), (unsigned long *)data);
}
#pragma MESSAGE DEFAULT C5703

// transmit 4 bytes of data
// 	e:1 = data is Big-endian
// 	e:0 = data is Little-endian
void bdmcf_transmit4(char e, unsigned char *data){
	if(e==1){	// big endian
		bdmcf_tx_msg(getbuf2big(data));		// send data - high word
		bdmcf_tx_msg(getbuf2big(data+2));	// send data - low word
	}else{	// little endian
		bdmcf_tx_msg(getbuf2little(data+2));	// send data - high word
		bdmcf_tx_msg(getbuf2little(data));		// send data - low word
	}
}

/*------------------------------------------------------------------------------
	t_write_mem
	-------------
	Write data to target memory

	addr = target address to write to
	width = size of the writes (8/16/32)
	count = total number of BYTES to be written
	*data = pointer to data buffer containing the data

	returns 0 on success
*/
#pragma MESSAGE DISABLE C5703
static int cf_write_or_fill_mem(unsigned char type, unsigned long addr, unsigned char width, int count, unsigned char *data, unsigned char write){

	unsigned char rbuf[4];	// holds unused return values

	addrH = (addr >> 16);		// get high address word
	addrL = (addr & 0xFFFF);	// get low address word

	// write all the data differently based on the width requested
	switch(width){
		case 8:	// 8-bit writes
            bdmcf_tx_cmdaddr(BDMCF_CMD_WRITE8);	// send the command
            bdmcf_tx_msg(*data);  			// send data
			while(count > 1){
            	count-=1;						// adjust data counter
				if(write) data+=1;						// point to next data item
				if(bdmcf_rx(width,1, BDMCF_CMD_FILL8, rbuf))	return 1;	// send command to get return value, return if error
	            bdmcf_tx_msg(*data);  			// send data
            }
			break;
		case 16:	// 16-bit writes
            bdmcf_tx_cmdaddr(BDMCF_CMD_WRITE16); // send the command
//            bdmcf_tx_msg(getbuf2little(data));	// send data
            bdmcf_tx_msg(getbuf2big(data));	// send data
			while(count > 2){
	            count-=2;						// adjust data counter
				if(write) data+=2;						// point to next data item
				if(bdmcf_rx(width,1, BDMCF_CMD_FILL16, rbuf))	return 1;	// send command to get return value, return if error
//	            bdmcf_tx_msg(getbuf2little(data));	// send data
	            bdmcf_tx_msg(getbuf2big(data));	// send data
	        }
			break;
		case 32:	// 32-bit writes
            bdmcf_tx_cmdaddr(BDMCF_CMD_WRITE32); // send the command
//            bdmcf_transmit4(0, data);	// transmit 4-byte, Little-endian
            bdmcf_transmit4(1, data);	// transmit 4-byte, Big-endian

            while(count > 4){
	            count-=4;						// adjust data counter
				if(write) data+=4;						// point to next data item
				if(bdmcf_rx(width,1, BDMCF_CMD_FILL32, rbuf))	return 1;	// send command to get return value, return if error
//              bdmcf_transmit4(0, data);	// transmit 4-byte, Little-endian
	            bdmcf_transmit4(1, data);	// transmit 4-byte, Big-endian
            }
			break;
		default:
			return 1;	// error, unknown width
	}
	if(bdmcf_rx(width,1, BDMCF_CMD_NOP, rbuf))	return 1;	// send NOP to get return value, return if error
	return 0;
}
int t_write_mem(unsigned char type, unsigned long addr, unsigned char width, int count, unsigned char *data){
	return cf_write_or_fill_mem(type, addr, width, count, data, 1);
}
#pragma MESSAGE DEFAULT C5703

/*------------------------------------------------------------------------------
	t_read_mem
	------------
	Read data from the target memory

	addr = target address to read from
	width = size of the reads (8/16/32)
	count = total number of BYTES to read
	*data = pointer to data buffer to hold data that's read

	returns 0 on success
*/
#pragma MESSAGE DISABLE C5703
int t_read_mem(unsigned char type, unsigned long addr, unsigned char width, int count, unsigned char *data){

	unsigned char *ptr;	// for testing, delete when done
	ptr = data;

	addrH = (addr >> 16);		// get high address word
	addrL = (addr & 0xFFFF);	// get low address word

	if(count == 0)    return 1;
	

	// read all the data differently based on the width requested
	switch(width){
		case 8:	// 8-bit reads
            bdmcf_tx_cmdaddr(BDMCF_CMD_READ8);	// send the command
			// if this is the last one
			if(count < 2){
				if(bdmcf_rx(width,1, BDMCF_CMD_NOP, data))	return 1;	// send NOP to get return value, return if error
				return 0;
			}
            count-=1;						// adjust data counter
			break;
		case 16:	// 16-bit reads
            bdmcf_tx_cmdaddr(BDMCF_CMD_READ16); // send the command

			// if this is the last one
			if(count < 3){	// if last
				if(bdmcf_rx(width,1, BDMCF_CMD_NOP, data))	return 1;	// send NOP to get return value, return if error
				return 0;
			}
            count-=2;						// adjust data counter
			break;
		default:	// 32-bit reads
            bdmcf_tx_cmdaddr(BDMCF_CMD_READ32); // send the command

			// if this is the last word
			if(count < 5){
//				if(bdmcf_rx(width,0, BDMCF_CMD_NOP, data+2))	return 1;	// send NOP to get return value, return if error
//				if(bdmcf_rx(width,0, BDMCF_CMD_NOP, data))	return 1;	// send NOP to get return value, return if error
				if(bdmcf_rx(width,1, BDMCF_CMD_NOP, data))		return 1;	// send NOP to get return value, return if error
				if(bdmcf_rx(width,1, BDMCF_CMD_NOP, data+2))	return 1;	// send NOP to get return value, return if error
				return 0;
			}
			// otherwise get the first word of the readback
//			if(bdmcf_rx(width,0, BDMCF_CMD_NOP, data+2))	return 1;	// send NOP to get return value, return if error
//			if(bdmcf_rx(width,0, BDMCF_CMD_DUMP32, data))	return 1;	// send NOP to get return value, return if error
			if(bdmcf_rx(width,1, BDMCF_CMD_NOP, data))	return 1;	// send NOP to get return value, return if error
			if(bdmcf_rx(width,1, BDMCF_CMD_DUMP32, data+2))	return 1;	// send NOP to get return value, return if error
			data+=4;						// point to next data item
            count-=4;						// adjust data counter
			break;
	}

//	if(count == 0)	return 0;

	// use the DUMP command after the first read if count >0 at this point meaning multiple reads were requested 
	while(count){
		switch(width){
			case 8:	// 8-bit dump
				if(bdmcf_rx(width,1, BDMCF_CMD_DUMP8, data))	return 1;	// send command to get return value, return if error
				data+=1;						// point to next data item
	            count-=1;						// adjust data counter
				break;
			case 16:	// 16-bit dump
				if(bdmcf_rx(width,1, BDMCF_CMD_DUMP16, data))	return 1;	// send command to get return value, return if error
				data+=2;						// point to next data item
	            count-=2;						// adjust data counter
				break;
			default:	// 32-bit dump
//				if(bdmcf_rx(width,0, BDMCF_CMD_NOP, data+2))	return 1;	// send NOP to get return value, return if error
//				if(bdmcf_rx(width,0, BDMCF_CMD_DUMP32, data))	return 1;	// send NOP to get return value, return if error
				if(bdmcf_rx(width,1, BDMCF_CMD_NOP, data))	return 1;	// send NOP to get return value, return if error
				if(bdmcf_rx(width,1, BDMCF_CMD_DUMP32, data+2))	return 1;	// send NOP to get return value, return if error
				data+=4;						// point to next data item
	            count-=4;						// adjust data counter
				break;
		}
	}
//	if(bdmcf_rx(width,0, BDMCF_CMD_NOP, data))	return 1;	// send NOP to get return value, return if error
	if(bdmcf_rx(width,1, BDMCF_CMD_NOP, data))	return 1;	// send NOP to get return value, return if error
	return 0;
}
#pragma MESSAGE DEFAULT C5703

/*------------------------------------------------------------------------------
	t_fill_mem
	------------
	Fill target memory with data

	addr = target address to write to
	width = size of the writes (8/16/32)
	count = total number of BYTES to be written
	*data = pointer to data buffer containing the data

	returns 0 on success
*/
int t_fill_mem(unsigned char type, unsigned long addr, unsigned char width, int count, unsigned char *data){
	return cf_write_or_fill_mem(type, addr, width, count, data, 0);
}

/******************************************************************************
*	bdmcf_pst_status()
*	
*	get current state of PST inputs
*
*	Input:	none
*	Return:	current PST state, PST state is inverted from target
*			0 if part IS ed
*	
******************************************************************************/
byte bdmcf_pst_status(){
	byte pStat;

	pStat = PTCD;

	// if NOT embedded version
#ifndef __EMBEDDED__    	// Under CW see: Target Setting, Compiler, Options, Language, Preprocessor Definitions
	pStat ^= 0x0F;	// invert the bits#endif
#endif
	pStat &= 0x0F;
	return (pStat);			// Return PST status bits 
}

/*------------------------------------------------------------------------------
	t_stat
	--------
	Get bdm/target status information to be sent to the host
	
	data[] will be loaded with bytes as follows:
		0-4 = CSR register
		5 = last SPI error status (eStatus)
		6 = PST Status 
		7 = unused
		8 = unused
		9 = unused
		A = unused

	returns 0 on success
*/
int	t_stat(unsigned char *data){

//	unsigned long val;
//	unsigned char rbuf[4];	// holds return values
	

	if(bdm_rbdreg(BDMCF_RDMREG_CSR, (unsigned long *)data)) return 1;	// read CSR reg
	*(data+4) = eStatus;	// get most recent eStatus
	*(data+5) = bdmcf_pst_status();	// PST lines status 


/*
	// CSR reg is in data[]

	// a softwar HALT instruction was executed, incriment PC so it doesn't get stuck
//	if(CSReg & CSR_HALT_MASK){
//	if((data[0] & 0x0F) == 0x02){	// if software halt
	if((data[0] & 0x0F) == 0x04){	// if software halt
		if(t_read_creg(0x80F, rbuf))	return 1;	// read the current PC
		val = getbuf4(rbuf);		// get 32-bit value from the buffer
//		val += 2;	// move pc over the halt instruction
//		*(unsigned long *) rbuf = val;
//		if(t_write_creg(0x80F, rbuf)) return 1;	// write new PC

	}
*/
	return 0;
}


//---------------------------------------------------------------------------
//	Return the bdm/target status information to host
//	
//	pData[] will be loaded with bytes as follows:
//		0    = VERSION_HW
//		1    = VERSION_SW
//		2    = BUILD_VER
//		3    = TARGET_TYPE
//    4    = FIRMWARE_TYPE
//		5..9 = unused
//---------------------------------------------------------------------------
int					// Returns 0 for success, 1 for failure
t_get_ver (
	PUINT8  pData)		// Ptr to return status buffer

{
    pData[0] = VERSION_HW;	// Hardware Version
    pData[1] = VERSION_SW;	// Firmware Version
    pData[2] = BUILD_VER;
  	pData[3] = TARGET_TYPE;
  	pData[4] = FIRMWARE_TYPE;
  	pData[5] = BUILD_VER_REV;
  	pData[6] = board_id; 
  	pData[7] = osbdm_id;
  	pData[8] = 0;
  	pData[9] =  0;

 	return (0);
}

/******************************************************************************
*	t_halt(void)
*	
*	asserts BKPT to target to HALT execution
*	
*	Return:	0 on success, 1 on fail
*
******************************************************************************/
byte t_halt(){

	unsigned char rbuf[4];	// holds unused return values

//	byte n;
	
	BRK_TMS = 0;					// assert BKPT to halt target in BDM mode if 
	wait_ms(100);					// wait for command to execute and HALT processor
	BRK_TMS = 1;					// if BDM mode selected, de-assert BKPT

/*
	for(n=0; n<5; n++){				// check status 5 times
		if(bdmcf_pst_status() != 0)	return 1;	// fail if not halted
		wait_ms(1);					// wait 1ms to allow update
	}
*/
//	return 0;						// success

		// The description of the HALT ColdFire instruction states that the execution
		// is resumed at the next instruction after the HALT when a "go" signal is received. 
		//Instead the debug session just gets stuck at HALT and the only way to resume it is
		// to manually change the PC.
//		wait_ms(100);					// wait for command to execute
//		return(t_step());	// added 7/3/08, see note above


	return (byte) (bdmcf_rx(16, 0, BDMCF_CMD_NOP, rbuf));	// send NOP to get return value, return if error

// from the tblcf firmware: 
/* it is a workaround for a strange problem: CF CPU V2 seems to ignore the first transfer after a halt */
/* I do not admit I know why it happens, but the extra NOP command fixes the problem... */
/* the problem has nothing to do with the delay: adding up to 400ms of delay between the halt nad the read did not fix it */

}


/******************************************************************************
*	t_go(void)
*	
*	resume target execution from current PC
*	
*	Return:	0 on success, 1 on fail
*
******************************************************************************/
byte t_go(void){
	dword CSReg;							// temp var for CSR from target

	unsigned long val;
	unsigned char rbuf[4];	// holds return values
	
	if(bdm_rbdreg(BDMCF_RDMREG_CSR, &CSReg)) return 1;	// read CSR reg

	CSReg &= ~CSR_SSM_MASK;			// clear step mode bit 

	if(bdm_wbdreg(BDMCF_WDMREG_CSR, CSReg)) return 1;	// command failed

  /* read and write PC, ostensibly to clear instruction pipeline */
	if(t_read_creg(0x80F, rbuf))	return 1;	// read the current PC
		val = getbuf4(rbuf);		// get 32-bit value from the buffer
    if(t_write_creg(0x80F, rbuf)) return 1;	// write PC
	
	CSReg = t_txrx(BDMCF_CMD_GO);		// start target execution 
	CSReg = t_txrx(BDMCF_CMD_NOP);	// get status
	if(sStatus) return 1;			// return fail
	return 0;							// command successful
}

/******************************************************************************
*	t_step(void)
*	
*	resume target execution from current PC
*	
*	Return:	0 on success, 1 on fail
*
******************************************************************************/
byte t_step(){
	dword CSReg;							// temp var for CSR from target

	unsigned long val;
	unsigned char rbuf[4];	// holds return values

 
	if(bdm_rbdreg(BDMCF_RDMREG_CSR, &CSReg)) return 1;	// read CSR reg

	CSReg |= CSR_SSM_MASK;			// set step mode bit 

	if(bdm_wbdreg(BDMCF_WDMREG_CSR, CSReg)) return 1;	// command failed
	
  /* read and write PC, ostensibly to clear instruction pipeline */
	if(t_read_creg(0x80F, rbuf))	return 1;	// read the current PC
		val = getbuf4(rbuf);		// get 32-bit value from the buffer
    if(t_write_creg(0x80F, rbuf)) return 1;	// write PC
    

	CSReg = t_txrx(BDMCF_CMD_GO);		// start target execution 
	CSReg = t_txrx(BDMCF_CMD_NOP);	// get status
	if(sStatus) return 1;			// return fail
	return 0;							// command successful
}


/******************************************************************************
*	bdm_wdbreg(word cmd, long data)
*	
*	write Debug Module register indicated in CMD
*	
*	Return:	0 on success, 1 on fail
*
******************************************************************************/
byte bdm_wbdreg(word cmd, dword data){

	word result, dataH, dataL;				// temp data variables
	
	dataH = (data >> 16);			// get high data word 
	dataL = (data & 0x0000FFFF);	// get low data word
	
	result = t_txrx(cmd);			// send command to target, dummy read 
	result = t_txrx(dataH);			// send high data word
	if (result == 0xFFFF){			// illegal command
		result = t_txrx(BDMCF_CMD_NOP);	// flush the pipeline, dummy read
		return 1;						// command failed
	}
	result = t_txrx(dataL);			// send low data word
	result = t_txrx(BDMCF_CMD_NOP);	// dummy read to get status
	if (sStatus) return 1;			// command failed

	return 0;							// command successful
}



/******************************************************************************
*	bdm_rdbreg(word cmd)
*	
*	read Debug Module register indicated in CMD.  Debug Module Register reads
*	return 32b data
*	
*	Return:	CSR data on success, bdrStatus = 1 on fail
*
******************************************************************************/
int bdm_rbdreg(word cmd, dword *rptr){

	word resultH, resultL;
	dword dmreg;
		
	resultH = t_txrx(cmd);			// send command to target, dummy read 
	resultH = t_txrx(BDMCF_CMD_NOP);	// get high data word from target DB reg
	if(sStatus){
		if ((resultH & 0xFFFF) == 0xFFFF){
			resultH = t_txrx(BDMCF_CMD_NOP);	// flush the pipeline, dummy read
			return 1;	// error
		}
	}
	resultL = t_txrx(BDMCF_CMD_NOP);	// get low data word from target DB reg
	dmreg = resultH;					// move high word data
	dmreg <<= 16;						// shift result in to high word
	dmreg += resultL;					// get high word read value
	
	*rptr = dmreg;						// save value read
	return 0;
}


/******************************************************************************
*	t_assert_ta(word dly_cnt)
*	
*	asserts TA to target for user delay in 10us increments
*	
*	INPUT:	# of 10us units to assert TA
*
******************************************************************************/
void t_assert_ta(word dly_cnt){

	byte n;
	
	TA_OUT = 1;							// assert TA
	TA_OUT_DIR = 1;					// pin is an output
	for(n=0; n<dly_cnt; n++){
		dly_10us(1);						// wait for 10us
	}
	TA_OUT_DIR = 0;					// remove TA
}



/******************************************************************************
*	t_resync(void)
*	
*	resynchronize host and target if comm is disrupted
*	
******************************************************************************/
byte t_resync(void){

	byte n;
	word d;
	
	for(n=0; n<20; n++){
		d=t_txrx(BDMCF_CMD_NOP);	// send NOP to target, dummy read
		if(!(sStatus)) return 0;	// loop until BDM Status bit clears
	}
	return 1;							// resynch failed
}


/******************************************************************************
*	cf_reset_nosync()
*	
*	resets target CF device to normal mode w/o resync.  tRSTO is inverted version 
*	of RSTO to target
*
******************************************************************************/
void cf_reset_nosync(){

	tRSTO = 1;					// assert reset_out signal
	tRSTO_DIR = 1;				// drive the signal out

	wait_ms(10);				// wait for 50ms
	tRSTO_DIR = 0;				// RSTO pin is an input
	
	wait_ms(10);				// wait for target to exit reset and see BRK signal asserted
}

/*********************** JTAG Support ****************************************/

/******************************************************************************
*	jtag_goto_reset()
*	
*	transition JTAG TAP state machine to TEST-LOGIC-RESET state.  Must re-init 
*	JTAG TAP to transition to RUN-TEST/IDLE state.  TMS remains high on exit
*	Must call jtag_init() to exit from this state
*
******************************************************************************/
/*
void jtag_goto_reset(){
	
	byte n;
	
	TMS_SET;								// assert TMS to target
	for(n=0; n<5; n++){				// strobe TCLK for 5 pulses to return TAP state machine to TEST-LOGIC-RESET state
		jtag_tclk_pulse();			// issue TCLK pulse
	}
}
*/
/******************************************************************************
*	jtag_init()
*	
*	setup BDM for JTAG operation and transition TAP to RUN-TEST/IDLE state. 
*	returns 0 for compatibility with t_init() routine
*
******************************************************************************/
/*
void jtag_init(){
	
	TMS_CLR;								// set TMS low
	TCLK_CTL = 0;						// TCLK driver output enable, active low
   PTED |= 0x04;						// set TDSCLK_CTL high
   PTED &= 0x57;						// OUT_EN, TDI, TCLK_EN, low
	
	PTBDD |= 0x0C;						// TMS, TCLK signals output
   PTEDD = 0xAC;						// OUT_EN, TDI, TCLK_EN, TDSCLK_EN output, SCLK_OUT, TDO input

	wait_ms(1);							// wait for input signals to settle	
	jtag_goto_reset();				// ensure TAP is in TEST-LOGIC-RESET state
	TMS_CLR;								// de-assert TMS to target
	jtag_tclk_pulse();				// clock TAP into RUN-TEST/IDLE state
}
*/
/******************************************************************************
*	jtag_goto_idle()
*	
*	transition TAP into RUN-TEST/IDLE state ready for next JTAG command.  Must
*  be in EXIT1-DR/IR state.  
*
******************************************************************************/
/*
void jtag_goto_idle(){
	
	TMS_SET;								// assert TMS to target
	jtag_tclk_pulse();				// transition to UPDATE-DR state
	TMS_CLR;								// de-assert TMS
	jtag_tclk_pulse();				// update DR and return to RUN-TEST/IDLE state
											// TCLK and TMS are left low
}
*/
/******************************************************************************
*	jtag_tclk_pulse()
*	
*	issue a single pulse on TCLK to target.  Currently set to ~150 kHz baud rate
*	Leaves TCLK low on exit
*
******************************************************************************/
/*
void jtag_tclk_pulse(){
	dly(45);								// wait  
	TCLK_SET;							// set TCLK
	dly(45);								// wait  
	TCLK_CLR;							// clear TCLK
											// leaves TCLK in low state
}
*/
/******************************************************************************
*	jtag_goto_shift()
*	
*	transitions TAP from RUN-TEST/IDLE to SHIFT-DR state or SHIFT-IR state 
*	based on mode input.  Assumes TAP is in RUN-TEST/IDLE state.  TCLK and 
*	TMS remain low on exit
*
*	INPUT:	mode = 0, transition to DR path
*					  = 1, transition to IR path
*
******************************************************************************/
/*
void jtag_goto_shift(byte mode){
	TMS_SET;								// set TMS high
	jtag_tclk_pulse();				// transition to SELECT DR-SCAN
	if (mode) jtag_tclk_pulse();	// in moded = 1, then transition to SELECT IR-SCAN
	TMS_CLR;								// set TMS low
	jtag_tclk_pulse();				// transition to CAPTURE-DR or CAPTURE-IR state
	jtag_tclk_pulse();				// transition to SHIFT-DR or SHIFT-IR state
}
*/
/******************************************************************************
*	jtag_write()
*	
*	writes a bit stream into the data/instruction path of JTAG TAP.  Must 
*	be in SHIFT-DR or SHIFT-IR state (use jtag_goto_shift()) before calling this 
*	function.  Data shifts to/from TAP LSB 1st 
*
*	INPUT:	tap_exit - 0 leaves TAP state unchanged, 1 transitions TAP back to RUN-TEST/IDLE state
*				bit_count - number of data/instruction bits to write to TAP
*				pdata - pointer to location of data to write to TAP
*
******************************************************************************/
/*
void jtag_write(byte tap_exit, byte bit_count, byte * pdata){
	
	byte bit_num = 0;
	byte data;							// holds current data byte being shited
	
	pdata += (bit_count >> 3);		// pdata points to last byte of data
	if((bit_count & 0x07) == 0) pdata--;	// adjust pointer if data occupies exact number of bytes 
	
	while(bit_num < bit_count){
		if((bit_num & 0x07) == 0) data = *(pdata--);	// fetch new byte and update pointer
		if(data & 0x01) TDI_OUT_SET;	// if data bit is a 1 then set TDI_OUT to send a 1
			else TDI_OUT_CLR;		// else clear TDI_OUT to send a 0
		data >>= 1; 					// shift data 1 bit
		bit_num++;						// increment bit num
		if(bit_num == bit_count) TMS_SET;	// if last bit then set TMS to exit SHIFT-DR/IR state 
		jtag_tclk_pulse();			// clock TAP, leaves TCLK low
	}
	if(tap_exit){						// if exit is selected
		jtag_goto_idle();				// return TAP to RUN-TEST/IDLE state
	}
}
*/
/******************************************************************************
*	jtag_read()
*	
*	reads a bit stream from the data/instruction path of JTAG TAP.  Must 
*	be in SHIFT-DR or SHIFT-IR state (use jtag_goto_shift()) before calling this 
*	function.  Data shifts to/from TAP LSB 1st 
*
*	INPUT:	tap_exit - 0 leaves TAP state unchanged, 1 transitions TAP back to RUN-TEST/IDLE state
*				bit_count - number of data/instruction bits to read to TAP
*				pdata - pointer to location to store data read from TAP
*
******************************************************************************/
/*
void jtag_read(byte tap_exit, byte bit_count, byte * pdata){
	
	byte bit_num = 0;
	byte data = 0;						// temp data var, holds current data being shifted
	
	pdata += (bit_count >> 3);		// pdata points to last byte of data
	if((bit_count & 0x07) == 0) pdata--;	// adjust pointer if data occupies exact number of bytes 
	
   while(bit_num < bit_count){
   	jtag_tclk_pulse();			// clock 1st data bit out of TAP
   	data >>= 1;						// shift current data down
   	if(TDO_IN_SET) data |= 0x80;	// capture current data bit 
   	bit_num++;						// update bit number
   	if((bit_num & 0x07) == 0) {
   		*(pdata--) = data;		// store recieved byte into data buffer and update pointer
   		data = 0;					// clear temp data var
   	}
   }
	if((bit_num & 0x07) != 0) {	// remaining bits to capture
		data >>= (8 - (bit_num & 0x07));	// shift bits into correct bit position
		*(pdata) = data;				// save last data bits in buffer
	}
	if(tap_exit){						// if exit is selected
		jtag_goto_idle();				// return TAP to RUN-TEST/IDLE state
	}
}
*/
/******************************************************************************
*	t_unsecure()
*	
*	Mass erase and unsecure target FLASH.  TAP must be in RUN-TEST/IDLE mode
*	when this function is called.  Mass erase is executed when TAP transitions
*	from UPDATE-DR to RUN-TEST/IDLE state at end of routine.  Wait_ms() provides
* 	delay to allow FLASH erase to complete before exit
*
******************************************************************************/
/*
void org_jtag_unsecure(){
	
//	byte lockr = 0x0B;				// LOCKOUT_RECOVERY command
//	byte clkdiv = 0x6D;				// CFM clock divider value for LOCKOUT_RECOVERY
//	byte lrcnt = 0x04;				// num of LOCKOUT_RECOVERY instruction bits to write

	byte lockr = 0x17;				// LOCKOUT_RECOVERY command
	byte clkdiv = 0x5F;				// CFM clock divider value for LOCKOUT_RECOVERY
	byte lrcnt = 5;					// num of LOCKOUT_RECOVERY instruction bits to write

	byte exit = 0x01;					// exit from write after shifting data 
	byte divcnt = 0x07;				// num of clkdiv data bits to write
   
	tRSTO = 0;							// de-assert reset_out signal
	tRSTO_DIR = 1;						// drive the signal out
	wait_ms(2);							// wait for signal to stabilize

	cf_reset_nosync();				// reset target

	jtag_init();						// init port signals, reset TAP, and go to RUN-TEST/IDLE state
	wait_ms(2);							// add delay
	
	jtag_goto_shift(1);				// go to SHIFT-IR state
	jtag_write(exit, lrcnt, &lockr);	//write LOCKOUT_RECOVERY command to TAP and exit to RUN-TEST/IDLE
	wait_ms(2);							// add delay

	jtag_goto_shift(0);				// go to SHIFT-DR state
	jtag_write(exit, divcnt, &clkdiv);	// write TFM_CLKDIV value to data register

	wait_ms(1000);						// wait for flash to erase	

	jtag_goto_reset();				// reset TAP
//	cf_reset_nosync();				// reset target

	tRSTO_DIR = 1;						// leave target in RESET state
}
*/

/*
// JTAG support 
*/


#define jdly()		      dly(45)


void jdly_loop(int i){
	while(i > 0){
		jdly();
		--i;
	}
}


char t_unsecure(byte lockr, byte lrcnt, byte clkdiv){

//	unsigned char idcode[4]={0,0,0,1};	/* IDCODE instruction */
	unsigned char idcode[5]={0,0,0,1,4};	// nonsence code
	

	unsigned char part_id[4];
	unsigned char unsec_instr[4];
	unsigned long int unsec_instruction;
	byte divcnt = 0x07;				// num of clkdiv data bits to write
	

	// test values
	lockr = 0x17;				// LOCKOUT_RECOVERY command
	clkdiv = 0x5F;				// CFM clock divider value for LOCKOUT_RECOVERY
	lrcnt = 0x05;					// num of LOCKOUT_RECOVERY instruction bits to write

//	tRSTO = 0;							// de-assert reset_out signal
//	tRSTO_DIR = 1;						// drive the signal out

//	wait_ms(2);							// wait for signal to stabilize

	cf_reset_nosync();				// reset target

					// NOTE:  this was done by tblcf_set_target_type(JTAG)
	jtag_init();	// init port signals, reset TAP, and go to RUN-TEST/IDLE state
					
					
	jdly_loop(20);	// about 200us delay
//	wait_ms(2);							// add delay
/*

	jtag_goto_shift(1);				// go to SHIFT-IR state
	jtag_write(1, lrcnt, &lockr);	//write LOCKOUT_RECOVERY command to TAP and exit to RUN-TEST/IDLE
	wait_ms(2);							// add delay

	jtag_goto_shift(0);				// go to SHIFT-DR state
	jtag_write(1, divcnt, &clkdiv);	// write TFM_CLKDIV value to data register

	wait_ms(1000);						// wait for flash to erase	

	jtag_goto_reset();				// reset TAP
//	cf_reset_nosync();				// reset target

	tRSTO_DIR = 1;						// leave target in RESET state
*/

	// NOTE: doing it this way is stupid, it should be changed after you see it works...
	// create unsecure instruction string
	unsec_instruction = lockr;
	unsec_instr[0]=unsec_instruction&0xff;
	unsec_instr[1]=(unsec_instruction>>8)&0xff;
	unsec_instr[2]=(unsec_instruction>>16)&0xff;
	unsec_instr[3]=(unsec_instruction>>24)&0xff;


//	tblcf_open(device_no);
//	tblcf_set_target_type(JTAG);	// select JTAG target 

	jtag_transition_shift(1);		// select instruction path 
	jtag_write(lrcnt,1,idcode+4-((lrcnt-1)>>3));	// shift the IDCODE instruction in 

	jdly_loop(20);	// about 200us delay

	jtag_transition_shift(0);		// select data path 
	jtag_read(32,1,part_id);		// get the ID 
//	printf("Part ID: %02X%02X%02X%02X\r\n",part_id[0],part_id[1],part_id[2],part_id[3]);
	jdly_loop(20);	// about 200us delay

	jtag_transition_shift(1);		// select instruction path 
	jtag_write(lrcnt,1,unsec_instr);	// shift the LOCKOUT_RECOVERY instruction in 
	jdly_loop(20);	// about 200us delay

	jtag_transition_shift(0);		// select data path 
	jtag_write(7,1,&clkdiv);				// shift the flash clk div in 
	jdly_loop(20);	// about 200us delay

	jtag_transition_reset();			// go to TEST-LOGIC-RESET 
	return 0;
}



// initialises the JTAG TAP and brings the TAP into RUN-TEST/IDLE state 
void jtag_init(void) {

	unsigned char i;
/*
	PTA  = JTAG_IDLE;    // preload idle state into port A data register 

	DDRA = TDI_OUT_MASK | TCLK_OUT_MASK | TRST_OUT_MASK | TMS_OUT_MASK | DDRA_DDRA7; // PTA7 is unused when not debugging, make sure it is output in such case 

	PTC  = 0;
	DDRC = DDRC_DDRC1;    // make pin PTC1 output (it is not bonded out on the 20 pin package anyway) 
	POCR = POCR_PTE20P;   // enable pull-ups on PTE0-2 (unused pins) 
*/

	PTED = 0x80;	// disable output enable (buffer)
	PTEDD = 0xED;	// set direction

	TMS_RESET();						// set TMS low
	TRST_RESET();

	PTBDD |= 0x0C;						// TMS, TCLK signals output
	PTBD &= 0xF3;

	PTED = 0x00;	// clear and enable JTAG port

	

//	tRSTO = 0;							// de-assert reset_out signal
//	tRSTO_DIR = 1;						// drive the signal out



//	PTED |= 0x04;						// set TDSCLK_CTL high
//	PTED &= 0x57;						// OUT_EN, TDI, TCLK_EN, low

//	TCLK_CTL = 0;						// TCLK driver output enable, active low
	
	
//	PTBDD |= 0x0C;						// TMS, TCLK signals output
//	PTEDD = 0xAC;						// OUT_EN, TDI, TCLK_EN, TDSCLK_EN output, SCLK_OUT, TDO input
	
	wait_ms(1);							// wait for input signals to settle	

	// now the JTAG pins are in their default states 
	TRST_RESET();                 // assert TRST 
	wait_ms(50);                 // 50ms 
	TDSCLK_EN = 1;                   // de-assert TRST 
	wait_ms(10);                 // 10ms 
	TMS_SET();                    // bring TMS high 

	for (i=10;i>0;i--) {	  		  // create 10 pulses on the TCLK line in case TRST is not connected to bring TAP to TEST-LOGIC-RESET (but it better should be as it is BKPT...) 
		TCLK_SET();
		//asm (NOP); asm (NOP); asm (NOP);  // nop padding to ensure that TCLK frequency is <0.5MHz, TCLK freq must be <= system clock / 4 and min crystal freq for 52235 is 2MHz... 
		jdly();		
		TCLK_RESET();
		jdly();		
	}
	TMS_RESET();                  // take TMS low 
	jdly();		
	TCLK_SET();										// transition TAP to RUN-TEST/IDLE 
	jdly();		
	TCLK_RESET();
	jdly();		
}


/******************************************************************************
*	jtag_init()
*	
*	setup BDM for JTAG operation and transition TAP to RUN-TEST/IDLE state. 
*	returns 0 for compatibility with t_init() routine
*
******************************************************************************/
/*
void jtag_init(){
	
	TMS_RESET();						// set TMS low
	TCLK_CTL = 0;						// TCLK driver output enable, active low
	PTED |= 0x04;						// set TDSCLK_CTL high
	PTED &= 0x57;						// OUT_EN, TDI, TCLK_EN, low
	
	PTBDD |= 0x0C;						// TMS, TCLK signals output
	PTEDD = 0xAC;						// OUT_EN, TDI, TCLK_EN, TDSCLK_EN output, SCLK_OUT, TDO input
	
	wait_ms(1);							// wait for input signals to settle	

	jtag_transition_reset();			// ensure TAP is in TEST-LOGIC-RESET state

	TMS_RESET();                  // take TMS low 
	TCLK_SET();										// transition TAP to RUN-TEST/IDLE 
	jdly();		
	TCLK_RESET();
	jdly();		
}
*/


// trasitions the state machine from RUN-TEST/IDLE to SHIFT-DR or SHIFT-IR state 
// if mode == 0 DR path is used, if mode != 0 IR path is used 
// leaves TCLK high and TMS low on exit 
void jtag_transition_shift(unsigned char mode) {
	
	TMS_SET();			// bring TMS high 
	TCLK_RESET();		// just in case TCLK was left high 
	jdly();		

	TCLK_SET();			// transition TAP to SELECT DR-SCAN 
	jdly();		
	TCLK_RESET();
	jdly();		

	if (mode!=0) {		// select the IR path 
		TCLK_SET(); 	// transition TAP to SELECT IR-SCAN 
		jdly();		
		TCLK_RESET();
		jdly();		
	}
	TMS_RESET();        // take TMS low 
	jdly();		
	TCLK_SET(); 		// transition TAP to CAPTURE-DR/IR 
	jdly();		
	TCLK_RESET();
	jdly();		

	TCLK_SET(); 		// transition TAP to SHIFT-DR/IR 
	jdly();		
	TCLK_RESET();

	jdly_loop(20);	// about 200us delay
}

// transitions the JTAG state machine to the TEST-LOGIC-REST state 
// the jtag must be re-initialised after this has happened since no other routine knows how to get out of this state 
void jtag_transition_reset(void) {
	unsigned char i;
	TMS_SET();               // bring TMS high 
	for (i=10;i>0;i--) {	 // create 10 pulses on the TCLK line 
		TCLK_SET();
		jdly();		
		TCLK_RESET();
		jdly();		
	}
}

// writes given bit stream into data/instruction path of JTAG 
// bit_count specifies the number of bits to write 
// data are transmitted starting with LSB of the LAST byte in the supplied buffer (for easier readability of code which uses JTAG) 
// expects to find the TAP in SHIFT-DR or SHIFT-IR state 
// if tap_transition == 0 leaves the TAP state unchanged, if tap_transition != 0 leaves the tap in RUN-TEST/IDLE 
void jtag_write(unsigned char bit_count, unsigned char tap_transition, unsigned char * datap) {
	unsigned char bit_no=0;
	unsigned char data;
	datap += (bit_count>>3);                                  // each byte has 8 bits, point to the last byte in the buffer 
	if ((bit_count&0x07)==0) datap--;                         // the size fits into the bytes precisely 
	while (bit_no<bit_count) {
		if ((bit_no&0x07)==0){
			data=*(datap--);                  // fetch new byte from the buffer and update the pointer 
		}
		if (data&0x01){
			TDI_OUT_SET(); 
		}
		else{ 
			TDI_OUT_RESET();     // assign new bit value to TDI 
		}
		TCLK_RESET();                                           // take TCLK low 
		jdly();		
		data>>=1;                                               // shift the data down 
		bit_no++;                                               // update the bit number 
		if (bit_no==bit_count){
			 if (tap_transition!=0){
			 	TMS_SET();// bring TMS high if exit from the SHIFT state is required 
				jdly();		
		 	}
		}
		TCLK_SET();                                             // take TCLK high 
		jdly();		
	}
	if (tap_transition) {                                     // the TAP is now in EXIT1-DR/IR if exit was requested 
		TCLK_RESET();                                           // take TCLK low 
		jdly();		
		TCLK_SET();                                             // bring TCLK high  // transition TAP to UPDATE-DR/IR 
		jdly();		
		TMS_RESET();                                            // take TMS low 
		TCLK_RESET();                                           // take TCLK low 
		jdly();		
		TCLK_SET();                                             // bring TCLK high  // transition TAP to RUN-TEST/IDLE 
		jdly();		
		TCLK_RESET();                                           // take TCLK low 
	}                                                         // now the tap is in RUN-TEST/IDLE if exit was requested 
}

// reads bitstream out of JTAG 
// bit_count specifies the number of bits to read 
// data are stored starting with LSB of the LAST byte in the supplied buffer (for easier readability of code which uses JTAG) 
// expects to find the TAP in SHIFT-DR or SHIFT-IR state 
// if tap_transition == 0 leaves the TAP state unchanged, if tap_transition != 0 leaves the tap in RUN-TEST/IDLE 
void jtag_read(unsigned char bit_count, unsigned char tap_transition, unsigned char * datap) {
	unsigned char bit_no=0;
	unsigned char data=0;
	datap += (bit_count>>3);                                  // each byte has 8 bits, point to the last byte in the buffer 
	if ((bit_count&0x07)==0) datap--;                         // the size fits into the bytes precisely 
	while (bit_no<bit_count) {
		TCLK_RESET();                                           // take TCLK low 
		jdly();		
		data>>=1;                                               // shift the data down 
		if (TDO_IN_SET) data|=0x80;                             // move the current bit into the data variable 
		bit_no++;                                               // update the bit number 
		if ((bit_no&0x07)==0) {
			*(datap--)=data;                                      // store the received byte into the buffer and update the pointer 
			data=0;                                               // clear the receiving variable 
		}
		if (bit_no==bit_count){
			if (tap_transition!=0){
				TMS_SET();// bring TMS high if exit from the SHIFT state is required 
				jdly();		
			}
		}
		TCLK_SET();                                             // take TCLK high 
		jdly();		
	}
	if ((bit_no&0x07)!=0) {                                   // there are bits in the data variable to be stored into the buffer 
		data>>=(8-(bit_no&0x07));                               // shift the bits into the right place 
		*(datap)=data;                                          // store the last bits into the buffer 
	}
	if (tap_transition) {                                     // the TAP is now in EXIT1-DR/IR if exit was requested 
		TCLK_RESET();                                           // take TCLK low 
		jdly();		
		TCLK_SET();                                             // bring TCLK high  // transition TAP to UPDATE-DR/IR 
		jdly();		
		TMS_RESET();                                            // take TMS low 
		TCLK_RESET();                                           // take TCLK low 
		jdly();		
		TCLK_SET();                                             // bring TCLK high  // transition TAP to RUN-TEST/IDLE 
		jdly();		
		TCLK_RESET();                                           // take TCLK low 
	}                                                         // now the tap is in RUN-TEST/IDLE if exit was requested 
}

//-------------------------------------------------------------
#pragma MESSAGE DISABLE C5703
int t_flash_power (byte enable){
	return(0);
}
#pragma MESSAGE DEFAULT C5703
//-------------------------------------------------------------
unsigned long t_get_clock (void){
	return (0);
}

void delay_492ns(int count) 
{
int i;
for (i = 1; i <= count; i++) 
{
_asm {
     nop; /* at 24Mhz each NOP takes 41ns to execute */
     nop;
     nop;
     nop;
     nop;
     nop;
     nop;
     nop;
     nop;
     nop;
     nop;
     nop;
}
}
}

unsigned long t_swap17(unsigned long dataout, byte *result) 
{
  unsigned long datain;
  byte hold_bkpt_mask;
  int i;
  datain = 0;
  // new 
  if (TMS_BRK_SET)
    hold_bkpt_mask = 0x04;
  else
    hold_bkpt_mask = 0;
  outp(((byte)((dataout>>16) & 0xFF | hold_bkpt_mask))); /* make sure BKPT is set, configure output port */
  TRST_SET();      /* set DSCLK high */
  delay_492ns(1);
  TRST_RESET();    /* set DSCLK low */
  delay_492ns(1);
    
  for (i=1; i<=16; i++) 
  {
  /* send out bit */
  if (dataout & 0x8000) 
  {
    TDI_OUT_SET(); /* DSI high */  
  } 
  else 
  {
    TDI_OUT_RESET(); /* DSI low */
  }
  dataout<<=1; 
  /* receive bit */
  datain <<=1; /* shift the data from previous bit swap, has to preceed the reading
                  not to overshift by  a bit */
  
  if (TDO_IN_SET) /* check DSO input from target */
  {
  datain |= 0x00001;
  }
  else 
  {
  datain &= 0x1FFFE;
  }
    
  TRST_SET();      /* set DSCLK high */
  delay_492ns(1);
  TRST_RESET();    /* set DSCLK low */
  TDI_OUT_RESET(); /* DSI low */
  } /* 16 bit swap is complete */
  datain <<=1;
  if (TDO_IN_SET) /* check DSO input from target to receive last status bit */
  {  
  datain |= 0x0001;
  }
  else 
  {
  datain &= 0x1FFFE;
  }
  delay_492ns(2);
  
  *result++ = (byte)((datain >> 16) & 0xFF);
  *result++ = (byte)((datain >> 8) & 0xFF);
  *result++ = (byte)(datain & 0xFF);
  
  return datain;
  
}

void cf_do_dsack() 
{
//    TA_OUT_ENABLE();

	TA_OUT = 1;							// assert TA
	TA_OUT_DIR = 1;					// pin is an output
  delay_492ns(2);
	TA_OUT_DIR = 0;					// remove TA
        delay_492ns(2);	

}

unsigned long t_xchange(unsigned long xdataout,byte *xresult) 
{
UINT8 swap_count = 0;
UINT8 hold_status = 1; 
UINT8 hold_testforready;
UINT32 hold_port_state;
UINT8 hold_swap_results[8];
unsigned long hold_temp_result = 0;



//NEW XCHANGE START

//int illegal_command_error = false;
//int data_xchng_error = 0;
int retries = 0;
int recov_cnt;
unsigned long tempdata = xdataout;
retry:
xdataout = tempdata;

hold_testforready = ((xdataout >> 16) & 0xFF);
hold_port_state = 0x00F80000;
hold_temp_result = t_swap17(((xdataout & 0x0000FFFF) | hold_port_state),&hold_swap_results[0] );
if (hold_swap_results[0] & 0x01 != 0) 
{
   retries++;
   switch (hold_temp_result & 0x0000FFFF) 
   {
    case 0x0000:
      if (hold_testforready) 
      {
         if (retries < max_num_retries)
          goto retry;
         
         cf_do_dsack();
         delay_492ns(200);
         xdataout = tempdata & 0x0000FFFF;
         hold_temp_result = t_swap17(((xdataout & 0x0000FFFF) | hold_port_state),&hold_swap_results[0] );
         
         if (hold_swap_results[0] == 0) 
         {
           hold_temp_result = (hold_temp_result & 0x0000FFFF) | 0x00020000;
           *xresult++ = (byte)((hold_temp_result >> 16) & 0xFF);
           *xresult++ = (byte)((hold_temp_result >> 8) & 0xFF);
           *xresult++ = (byte)(hold_temp_result & 0xFF);
           cf_do_dsack();
      
           return hold_temp_result;   
         } 
         else 
         {
           hold_temp_result = (hold_temp_result & 0x0000FFFF) | 0x00010000;
           *xresult++ = (byte)((hold_temp_result >> 16) & 0xFF);
           *xresult++ = (byte)((hold_temp_result >> 8) & 0xFF);
           *xresult++ = (byte)(hold_temp_result & 0xFF);
           cf_do_dsack();
           
           return hold_temp_result; 
         }
        /* test_for_ready */
      } else {
        hold_temp_result = (hold_temp_result & 0x0000FFFF);        
      }
      break;
    case 0x0001:
      hold_temp_result = (hold_temp_result & 0x0000FFFF) | 0x00020000;
      *xresult++ = (byte)((hold_temp_result >> 16) & 0xFF);
      *xresult++ = (byte)((hold_temp_result >> 8) & 0xFF);
      *xresult++ = (byte)(hold_temp_result & 0xFF);
      cf_do_dsack();
      
      return hold_temp_result;   
      break;
      
    default:
      cf_do_dsack();
      hold_temp_result = t_swap17((BDMCF_CMD_NOP | hold_port_state),&hold_swap_results[0] );
      if ((hold_swap_results[0] == 1) && ((hold_temp_result & 0x0000FFFF) > 0x0001)) 
      { // still an illegal command error 
        for (recov_cnt=1; recov_cnt<= 17; recov_cnt++) 
        {
          TDI_OUT_RESET(); 
          TRST_SET();  
          delay_492ns(200); // delay 100us
          TRST_RESET(); 
          delay_492ns(200); // delay 100us
          hold_temp_result = t_swap17((BDMCF_CMD_NOP | hold_port_state),&hold_swap_results[0] );
          if ((hold_swap_results[0] == 0) && ((hold_temp_result & 0x0000FFFF) == 0xFFFF)) 
          { // recovered
            *xresult++ = (byte)((hold_temp_result >> 16) & 0xFF);
            *xresult++ = (byte)((hold_temp_result >> 8) & 0xFF);
            *xresult++ = (byte)(hold_temp_result & 0xFF);
      
            return hold_temp_result;
            break;          
          }          
        }
      }      
      hold_temp_result = (hold_temp_result & 0x0000FFFF) | 0x00040000;       
      break;
   } /* error switch */
}/* if error */

  *xresult++ = (byte)((hold_temp_result >> 16) & 0xFF);
  *xresult++ = (byte)((hold_temp_result >> 8) & 0xFF);
  *xresult++ = (byte)(hold_temp_result & 0xFF);
      
  return hold_temp_result;  
//NEW XCHANGE END

}

int outp(byte port_write) 
{
    /* bit0 - DSI
    bit1 - CLK
    bit2 - BREAK
    bit3 - RESET 
    */
    if (port_write & 0x01) 
    {
    TDI_OUT_SET();
//    TDI_OUT_ENABLE();  
    } 
    else 
    {
    TDI_OUT_RESET(); 
//    TDI_OUT_ENABLE();  
    }
        
    if (port_write & 0x02) 
    {
    TRST_SET(); 
//    TDSCLK_ENABLE();
    } 
    else 
    {
    TRST_RESET(); 
//    TDSCLK_ENABLE();
    }
        
    if (port_write & 0x04) 
    {
    TMS_SET();
//    TMS_BRK_ENABLE();
    } 
    else
    {
    TMS_RESET();
//    TMS_BRK_ENABLE();
    }
        

    if (port_write & 0x08) 
    {
    tRSTO_DIR = 0;		  // set reset signal as an input 
    } 
    else 
    {
    tRSTO = 1;					// assert reset_out signal
    tRSTO_DIR = 1;		  // drive the signal out
    }

    if (port_write & 0x40) 
    {
	  TA_OUT_DIR = 0;					// pin is an input
    } 
    else 
    {
  	TA_OUT = 1;							// assert TA
	  TA_OUT_DIR = 1;					// pin is an output
    }  
}

int cfinp(void) 
{
    int hold_port_status = 0;
    hold_port_status = 0x10; /* OSBDM is powered */
//    TDO_INPUT_ENABLE(); /* set TDO bit to input */

    if (PTED_PTED4 != 1)  {
        hold_port_status |= 0x80;
     }

    if ((bdmcf_pst_status() & 0x0F) == 0x0F)
        {
          hold_port_status |= 0x40;
        }

return hold_port_status;
}


int t_special_feature(unsigned char sub_cmd_num,  // Special feature number (sub_cmd_num)
	                     PUINT16 pInputLength,      // Length of Input Command
	                     PUINT8  pInputBuffer,               // Input Command Buffer
	                     PUINT16 pOutputLength,     // Length of Output Response
	                     PUINT8  pOutputBuffer)    {          // Output Response Buffer 
int i;
int status;
unsigned long temp_new_address = 0;
int hold_errorval = 0;
word bytes_read = 0;
word recv_index = 0;
word bytes_sent = 0;
word send_index = 0;
char ztest_array [] = { 0,0,0,0,0,0,0,0 };
char *pData = &ztest_array[0];
UINT16 temp_word;
UINT32 temp_longword; 
*pOutputLength = 0;
switch (sub_cmd_num)
        {
        case 0x0A: // re_sync to device 
          t_resync; 
          return (0) ; // success
          break;
        case 0x00: // get a port status from device
          rxBuff[0] = (byte)cfinp();
          *pOutputLength = 2;
          pOutputBuffer[0] = rxBuff[0];
          pOutputBuffer[1] = 0;
          return 0;  
          break;         
        case 0x01: // output byte to target port 
       
        /* bit0 - DSI
           bit1 - CLK
           bit2 - BREAK
           bit3 - RESET 
        */
        
        //hold_sendword = pInputBuffer[0] & 0x00FF;
          outp(pInputBuffer[0]);
          return 0;
          break;
        case 0x02: // swap - get 17 bits with status 0000000X 00000000 XXXXXXXX XXXXXXXXS
          hold_senddata = 0;
          hold_senddata |= (((long)pInputBuffer[0]<<16) & 0xFF0000);
          hold_senddata |= (((long)pInputBuffer[1]<<8) & 0x00FF00);
          hold_senddata |= ((long)pInputBuffer[2] & 0x0000FF);
          t_swap17(hold_senddata,rxBuff);
          *pOutputLength = 4;
          pOutputBuffer[0] = rxBuff[0];
          pOutputBuffer[1] = rxBuff[1]; /* pc expects to get 2 bytes of error */
          pOutputBuffer[2] = rxBuff[2];
          pOutputBuffer[3] = 0; /* this byte is not used */
          return 0;        
          break;
        case 0x03: // do_sack, drive dsack low (TEA BDM pin 26), wait for 1us, tristate dsack line, wait for 1us
          cf_do_dsack();
          return 0;
          break;
        
        case 0x10: // data_xchange 
          hold_senddata = 0;
          hold_senddata |= (((long)pInputBuffer[0]<<16) & 0xFF0000);
          hold_senddata |= (((long)pInputBuffer[1]<<8) & 0x00FF00);
          hold_senddata |= ((long)pInputBuffer[2] & 0x0000FF);
          t_xchange(hold_senddata,rxBuff);
          *pOutputLength = 4;
	        pOutputBuffer[0] = rxBuff[0];
	        pOutputBuffer[1] = rxBuff[1];
	        pOutputBuffer[2] = rxBuff[2];
	        pOutputBuffer[3] = 0; // unused
	        return 0; 
          break;
        case 0x11: // do read sequence - 16 bit opcode - 32 bit address - 
                   // get back 32 bit data and 8 bit error code, 8 bit error code
          hold_sendword = (word)pInputBuffer[0]<<8 & 0xFF00;
          hold_sendword = (hold_sendword | (word)pInputBuffer[1]) & 0xFFFF;
          hold_sendaddress = ((long) pInputBuffer[2]<<24) & 0xFF000000;
          hold_sendaddress = ((hold_sendaddress | ((long)pInputBuffer[3]<<16)) & 0xFFFF0000);
          hold_sendaddress = ((hold_sendaddress | ((long)pInputBuffer[4]<<8)) & 0xFFFFFF00);
          hold_sendaddress = ((hold_sendaddress | ((long)pInputBuffer[5])) & 0xFFFFFFFF);
        
          t_xchange(hold_sendword,&rxBuff[0]); /* send command */
          t_xchange(((hold_sendaddress>>16) & 0xFFFF), &rxBuff[0]); /* send high address */
          t_xchange((hold_sendaddress & 0xFFFF), &rxBuff[0]); /* send low address */
        
          delay_492ns(4);
        
          t_xchange((0xFF0000 | BDMCF_CMD_NOP), &rxBuff[0]); /* read high 16 bits of data */
          if (rxBuff[0] == 0) 
          {
            t_xchange(BDMCF_CMD_NOP, &rxBuff[3]); /* read low 16 bits of data */            
          } 
          else 
          {
            cf_do_dsack();        
            t_xchange(BDMCF_CMD_NOP,&rxBuff[3]);
            rxBuff[1] = 0;
            rxBuff[2] = 0;
            rxBuff[4] = 0;
            rxBuff[5] = 0;
            rxBuff[3] = rxBuff[0];
          }
          
	      *pOutputLength = 6;
	      pOutputBuffer[0] = rxBuff[1];
	      pOutputBuffer[1] = rxBuff[2];
	      pOutputBuffer[2] = rxBuff[4];
	      pOutputBuffer[3] = rxBuff[5];
	      pOutputBuffer[4] = rxBuff[3];
	      pOutputBuffer[5] = rxBuff[3]; 	      
	      return 0; 
        break;
        case 0x12: // test_for_freeze, read a byte of data 
          rxBuff[0] = cfinp();

          if ((rxBuff[0] & 0x40) != 0x40) 
          {
            rxBuff[0] = 0;
          } else {
            rxBuff[0] = 0xff;          
          }
            
          *pOutputLength = 2;
          pOutputBuffer[0] = rxBuff[0];
          pOutputBuffer[1] = rxBuff[0];
          return 0;  
          break;
         
        case 0x13: // get 32 bit data, no address (return 32 bits of data and 8 bits of error)
	      hold_sendword = ((word)pInputBuffer[0]<<8) & 0xFF00;
              hold_sendword = (hold_sendword | (word)pInputBuffer[1]) & 0xFFFF;
	      t_xchange(hold_sendword,&rxBuff[0]);
	      delay_492ns(4);
	      t_xchange(BDMCF_CMD_NOP,&rxBuff[0]);  
	      /* check for bus error */
	      if (rxBuff[0] == 0) 
	      {
	      t_xchange(BDMCF_CMD_NOP,&rxBuff[3]);
	      t_xchange(BDMCF_CMD_NOP,&rxBuff[6]); /* this is not necessary */
	      pOutputBuffer[0] = rxBuff[1]; /* results of first swap */
	      pOutputBuffer[1] = rxBuff[2];
	      pOutputBuffer[2] = rxBuff[4];
	      pOutputBuffer[3] = rxBuff[5]; /* results of second swap */
	      pOutputBuffer[4] = 0; /* status byte of the second swap */
	      pOutputBuffer[5] = 0; // we try to get 16 bytes even though command should only return 5 
	      } 
	      else 
	      {  
	      pOutputBuffer[0] = 0;
	      pOutputBuffer[1] = 0;
	      pOutputBuffer[2] = 0;
	      pOutputBuffer[3] = 0;
	      pOutputBuffer[4] = rxBuff[0]; /* status byte of the second swap */
	      pOutputBuffer[5] = rxBuff[0]; // we try to get 16 bytes even though command should only return 5 
	      }
	      
	      *pOutputLength = 6;
	      return 0; 
        break;
        case 0x14: // write 32 bit sequence at an address - 16 bit opcode, 32 bit address, 32 bit data
                   // return 8 bit status in high byte of a 16 bit word
        hold_sendword = (word)pInputBuffer[0]<<8 & 0xFF00;
        hold_sendword = (hold_sendword | (word)pInputBuffer[1]) & 0xFFFF;
        hold_sendaddress = ((long)pInputBuffer[2]<<24) & 0xFF000000;
        hold_sendaddress = hold_sendaddress | (((long)pInputBuffer[3]<<16) & 0x00FF0000);
        hold_sendaddress = hold_sendaddress | (((long)pInputBuffer[4]<<8) & 0x0000FF00);
        hold_sendaddress = hold_sendaddress | (((long)pInputBuffer[5]) & 0x000000FF); 
        hold_senddata = ((long)pInputBuffer[6]<<24) & 0xFF000000;
        hold_senddata = hold_senddata | (((long)pInputBuffer[7]<<16) & 0x00FF0000);
        hold_senddata = hold_senddata | (((long)pInputBuffer[8]<<8) & 0x0000FF00);
        hold_senddata = hold_senddata | (((long)pInputBuffer[9]) & 0x000000FF);        
        
        t_xchange(hold_sendword,&rxBuff[0]); /* send command */
        t_xchange(((hold_sendaddress>>16) & 0xFFFF), &rxBuff[0]); /* send high address */
        t_xchange((hold_sendaddress & 0xFFFF), &rxBuff[0]); /* send low address */
        
        t_xchange(((hold_senddata>>16) & 0xFFFF), &rxBuff[0]); /* send high word */
        t_xchange((hold_senddata & 0xFFFF), &rxBuff[0]); /* send low word */
        
	      delay_492ns(4);
	      
	      t_xchange((BDMCF_CMD_NOP | 0xFF0000),&rxBuff[0]);
	      if (rxBuff[0] != 0) 
	      {
          cf_do_dsack();          
   	      t_xchange((BDMCF_CMD_NOP | 0xFF0000),&rxBuff[3]);
          //rxBuff[0] = 0xAD; /* pass the actual error through to software */          
	      }
        
        
        *pOutputLength = 2;
	      pOutputBuffer[0] = rxBuff[0];
	      pOutputBuffer[1] = rxBuff[0];
	      return 0;
        break;
        case 0x15: // write 16 bit sequence
        hold_sendword = (word)pInputBuffer[0]<<8 & 0xFF00;
        hold_sendword = (hold_sendword | (word)pInputBuffer[1]) & 0xFFFF;
        hold_sendaddress = ((long)pInputBuffer[2]<<24) & 0xFF000000;
        hold_sendaddress = hold_sendaddress | (((long)pInputBuffer[3]<<16) & 0x00FF0000);
        hold_sendaddress = hold_sendaddress | (((long)pInputBuffer[4]<<8) & 0x0000FF00);
        hold_sendaddress = hold_sendaddress | (((long)pInputBuffer[5]) & 0x000000FF); 
        hold_senddata = ((long)pInputBuffer[6]<<8) & 0xFF00;
        hold_senddata = hold_senddata | ((long)pInputBuffer[7] & 0x00FF); 
        
        t_xchange(hold_sendword,&rxBuff[0]); /* send command */
        t_xchange(((hold_sendaddress>>16) & 0xFFFF), &rxBuff[0]); /* send high address */
        t_xchange((hold_sendaddress & 0xFFFF), &rxBuff[0]); /* send low address */        
        t_xchange((hold_senddata & 0xFFFF), &rxBuff[0]); /* send word */
        
	      delay_492ns(4);
	      
        t_xchange((BDMCF_CMD_NOP | 0xFF0000),&rxBuff[0]);
	      if (rxBuff[0] != 0) 
	      {
              cf_do_dsack();
   	      t_xchange((BDMCF_CMD_NOP | 0xFF0000),&rxBuff[3]);
              }
        
        
        *pOutputLength = 2;
	      pOutputBuffer[0] = rxBuff[0];
	      pOutputBuffer[1] = rxBuff[0];
	      return 0;	      
        break;
        case 0x16: // write 32 bit data, no address, 16 bit opcode, 32 bit data, read 16 bits of status
        hold_sendword = (word)pInputBuffer[0]<<8 & 0xFF00;
        hold_sendword = (hold_sendword | (word)pInputBuffer[1]) & 0xFFFF;
        hold_senddata = ((long)pInputBuffer[2]<<24) & 0xFF000000;
        hold_senddata = hold_senddata | (((long)pInputBuffer[3]<<16) & 0x00FF0000);
        hold_senddata = hold_senddata | (((long)pInputBuffer[4]<<8) & 0x0000FF00);
        hold_senddata = hold_senddata | (((long)pInputBuffer[5]) & 0x000000FF);
        
        t_xchange(hold_sendword,&rxBuff[0]); /* send command */
        t_xchange(((hold_senddata>>16) & 0xFFFF), &rxBuff[0]); /* send high data word */
        if (rxBuff[0] != 0) 
        {
	      pOutputBuffer[0] = rxBuff[0];
	      pOutputBuffer[1] = rxBuff[0];
        } 
        else 
        {
        t_xchange((hold_senddata & 0xFFFF), &rxBuff[0]); /* send high data word */
        delay_492ns(4);
	      pOutputBuffer[0] = 0;
	      pOutputBuffer[1] = 0;
        }
        *pOutputLength = 2;
	      return 0;
        break;
        case 0x17: 
        hold_sendword = BDMCF_CMD_READ32;
        hold_sendaddress = ((long)pInputBuffer[0]<<24) & 0xFF000000;
        hold_sendaddress = hold_sendaddress | (((long)pInputBuffer[1]<<16) & 0x00FF0000);
        hold_sendaddress = hold_sendaddress | (((long)pInputBuffer[2]<<8) & 0x0000FF00);
        hold_sendaddress = hold_sendaddress | (((long)pInputBuffer[3]) & 0x000000FF); 
        
        hold_block_length = ((long)pInputBuffer[4]<<8) & 0xFF00;
        hold_block_length = hold_block_length | ((long)pInputBuffer[5] & 0x00FF);
        
        t_xchange(hold_sendword,&rxBuff[0]); /* send command */
        t_xchange(((hold_sendaddress>>16) & 0xFFFF), &rxBuff[0]); /* send high address */
        t_xchange((hold_sendaddress & 0xFFFF), &rxBuff[0]); /* send low address */
        
        delay_492ns(4);
        
        t_xchange((BDMCF_CMD_NOP | 0xFF0000), &rxBuff[0]); /* read high 16 bits of data */
        if (rxBuff[0] == 0) 
        {
        t_xchange(BDMCF_CMD_NOP, &rxBuff[3]); /* read low 16 bits of data */            
        pOutputBuffer[recv_index] = rxBuff[1];
        pOutputBuffer[recv_index+1] = rxBuff[2];
        pOutputBuffer[recv_index+2] = rxBuff[4];
        pOutputBuffer[recv_index+3] = rxBuff[5];
        pOutputBuffer[recv_index+4] = rxBuff[3];
        pOutputBuffer[recv_index+5] = rxBuff[3];

        } 
        else 
        {
          cf_do_dsack();
          t_xchange(BDMCF_CMD_NOP,&rxBuff[3]);
          pOutputBuffer[recv_index] = 0;
          pOutputBuffer[recv_index+1] = 0;
          pOutputBuffer[recv_index+2] = 0;
          pOutputBuffer[recv_index+3] = 0;
          pOutputBuffer[recv_index+4] = rxBuff[0];
          pOutputBuffer[recv_index+5] = rxBuff[0]; 
        }

        bytes_read +=4;
        recv_index +=6;
        *pOutputLength =6;
        while (bytes_read < hold_block_length) 
        {
          t_xchange(BDMCF_CMD_DUMP32, &rxBuff[0]); /* send DUMP command */
          delay_492ns(4);
          t_xchange((0xFF0000|BDMCF_CMD_NOP),&rxBuff[0]);  /* read high 16 bits of data */
          if (rxBuff[0] == 0) 
          {
            t_xchange(BDMCF_CMD_NOP,&rxBuff[3]); /* read low 16 bits of data */
            pOutputBuffer[recv_index] = rxBuff[1];
            pOutputBuffer[recv_index+1] = rxBuff[2];
            pOutputBuffer[recv_index+2] = rxBuff[4];
            pOutputBuffer[recv_index+3] = rxBuff[5];
            pOutputBuffer[recv_index+4] = 00;
            pOutputBuffer[recv_index+5] = 00; 
          } 
          else 
          {
            cf_do_dsack();
            delay_492ns(2);
            t_xchange(BDMCF_CMD_NOP,&rxBuff[3]);    
        
            // reread data from the same location 
            temp_new_address = hold_sendaddress + (recv_index/6)*4;
          
            t_xchange(BDMCF_CMD_READ32,&rxBuff[0]); /* send command */
            t_xchange(((temp_new_address>>16) & 0xFFFF), &rxBuff[0]); /* send high address */
            t_xchange((temp_new_address & 0xFFFF), &rxBuff[0]); /* send low address */
        
            delay_492ns(4);
        
            t_xchange((0xFF0000|BDMCF_CMD_NOP), &rxBuff[0]); /* read high 16 bits of data */
            if (rxBuff[0] == 0) 
            {
            t_xchange(BDMCF_CMD_NOP, &rxBuff[3]); /* read low 16 bits of data */            
            pOutputBuffer[recv_index] = rxBuff[1];
            pOutputBuffer[recv_index+1] = rxBuff[2];
            pOutputBuffer[recv_index+2] = rxBuff[4];
            pOutputBuffer[recv_index+3] = rxBuff[5];
            pOutputBuffer[recv_index+4] = rxBuff[3];
            pOutputBuffer[recv_index+5] = rxBuff[3]; 
            } 
            else 
            {
              cf_do_dsack();          
              delay_492ns(2);
              t_xchange(BDMCF_CMD_NOP,&rxBuff[3]);
              pOutputBuffer[recv_index] = 0;
              pOutputBuffer[recv_index+1] = 0;
              pOutputBuffer[recv_index+2] = 0;
              pOutputBuffer[recv_index+3] = 0;
              pOutputBuffer[recv_index+4] = rxBuff[0];
              pOutputBuffer[recv_index+5] = rxBuff[0]; 
            }
          }
          recv_index+=6; 
          bytes_read +=4;
          *pOutputLength+=6;
        } /* while */
      return 0;
      break;
  
      case 0x1A: 
        hold_sendword = BDMCF_CMD_READ16;
        hold_sendaddress = ((long)pInputBuffer[0]<<24) & 0xFF000000;
        hold_sendaddress = hold_sendaddress | (((long)pInputBuffer[1]<<16) & 0x00FF0000);
        hold_sendaddress = hold_sendaddress | (((long)pInputBuffer[2]<<8) & 0x0000FF00);
        hold_sendaddress = hold_sendaddress | (((long)pInputBuffer[3]) & 0x000000FF); 
        
        hold_block_length = ((long)pInputBuffer[4]<<8) & 0xFF00;
        hold_block_length = hold_block_length | ((long)pInputBuffer[5] & 0x00FF);
        
        t_xchange(hold_sendword,&rxBuff[0]); /* send command */
        t_xchange(((hold_sendaddress>>16) & 0xFFFF), &rxBuff[0]); /* send high address */
        t_xchange((hold_sendaddress & 0xFFFF), &rxBuff[0]); /* send low address */
        
        delay_492ns(4);
        
        t_xchange(0xFF0000|BDMCF_CMD_NOP, &rxBuff[0]); /* read high 16 bits of data */
        if (rxBuff[0] == 0) 
        {
        t_xchange(BDMCF_CMD_NOP, &rxBuff[3]); /* read high 16 bits of data */
        } 
        else 
        {
          cf_do_dsack();        
          t_xchange(BDMCF_CMD_NOP,&rxBuff[3]);
          rxBuff[1] = 0x00;
          rxBuff[2] = 0x00;
          rxBuff[4] = 0x00;
          rxBuff[5] = 0x00;
          rxBuff[3] = rxBuff[0];
          //rxBuff[0] = 0x01; /* Bus error occured */
        }        
        pOutputBuffer[recv_index] = rxBuff[1];
        pOutputBuffer[recv_index+1] = rxBuff[2];
        pOutputBuffer[recv_index+2] = rxBuff[4];
        pOutputBuffer[recv_index+3] = rxBuff[5];
        pOutputBuffer[recv_index+4] = rxBuff[3];
        pOutputBuffer[recv_index+5] = rxBuff[3]; 

        bytes_read +=2;
        recv_index +=6;
        *pOutputLength =6;  
        while (bytes_read < hold_block_length) 
        {
          t_xchange(BDMCF_CMD_DUMP16, &rxBuff[0]); /* send dump16 command */
          delay_492ns(4);
          t_xchange((0xFF0000|BDMCF_CMD_NOP),&rxBuff[0]); /* read high 16 bits of data */
          if (rxBuff[0] == 0) 
          {

          } 
          else 
          {
          cf_do_dsack();
          t_xchange(BDMCF_CMD_NOP,&rxBuff[0]);    
        
          // reread data from the same location 
          temp_new_address = hold_sendaddress + (recv_index/6)*2;
          
          t_xchange(BDMCF_CMD_READ16,&rxBuff[0]); /* send command */
          t_xchange(((temp_new_address>>16) & 0xFFFF), &rxBuff[0]); /* send high address */
          t_xchange((temp_new_address & 0xFFFF), &rxBuff[0]); /* send low address */
        
          delay_492ns(4);
        
          t_xchange(0xFF0000|BDMCF_CMD_NOP, &rxBuff[0]); /* read high 16 bits of data */
          if (rxBuff[0] == 0) 
            {
            t_xchange(BDMCF_CMD_NOP, &rxBuff[3]); /* read high 16 bits of data */
            } 
          else 
            {
            cf_do_dsack();        
            t_xchange(BDMCF_CMD_NOP,&rxBuff[3]);
            rxBuff[1] = 0x00;
            rxBuff[2] = 0x00;
            rxBuff[4] = 0x00;
            rxBuff[5] = 0x00;
            rxBuff[3] = rxBuff[0];
            }
          }
          pOutputBuffer[recv_index] = rxBuff[1];
          pOutputBuffer[recv_index+1] = rxBuff[2];
          pOutputBuffer[recv_index+2] = rxBuff[0];
          pOutputBuffer[recv_index+3] = rxBuff[0];
          recv_index+=4; /* only the first word has DDXXEX format, all following words
                            have DDEX format */ 
          bytes_read +=2;
          *pOutputLength+=4;
        } /* while */        
        return 0;
        break;
      case 0x1B: /* read byte block */ 
        // This routine is currently unfixed and does not match Multilink (close enough?)
        hold_sendword = BDMCF_CMD_READ8;
        hold_sendaddress = ((long)pInputBuffer[0]<<24) & 0xFF000000;
        hold_sendaddress = hold_sendaddress | (((long)pInputBuffer[1]<<16) & 0x00FF0000);
        hold_sendaddress = hold_sendaddress | (((long)pInputBuffer[2]<<8) & 0x0000FF00);
        hold_sendaddress = hold_sendaddress | (((long)pInputBuffer[3]) & 0x000000FF); 
        
        hold_block_length = ((long)pInputBuffer[4]<<8) & 0xFF00;
        hold_block_length = hold_block_length | ((long)pInputBuffer[5] & 0x00FF);
        
        t_xchange(hold_sendword,&rxBuff[0]); /* send command */
        t_xchange(((hold_sendaddress>>16) & 0xFFFF), &rxBuff[0]); /* send high address */
        t_xchange((hold_sendaddress & 0xFFFF), &rxBuff[0]); /* send low address */
        
        delay_492ns(4);
        
        t_xchange(BDMCF_CMD_NOP, &rxBuff[0]); /* read high 16 bits of data */
        if (rxBuff[0] == 0) 
        {

        } 
        else 
        {
          cf_do_dsack();        
          t_xchange(BDMCF_CMD_NOP,&rxBuff[0]);
          rxBuff[1] = 0xAD;
          rxBuff[2] = 0xAD;
          rxBuff[4] = 0;
          rxBuff[5] = 0;
          //rxBuff[0] = 0x01; /* Bus error occured */
        }        
        pOutputBuffer[recv_index] = 0; /* data */; /* first byte sent back as XDXXEX, followed by ED ED ... */
        pOutputBuffer[recv_index+1] = rxBuff[2]; /* data */
        pOutputBuffer[recv_index+2] = 0xFF;
        pOutputBuffer[recv_index+3] = 0xFF;
        pOutputBuffer[recv_index+4] = rxBuff[0]; /* error */
        pOutputBuffer[recv_index+5] = rxBuff[0]; 

        bytes_read +=1;
        recv_index +=6;
        *pOutputLength =6;  
        while (bytes_read < hold_block_length) 
        {
          t_xchange(BDMCF_CMD_DUMP8, &rxBuff[0]); /* send dump8 command */
          delay_492ns(4);
          t_xchange((0xFF0000|BDMCF_CMD_NOP),&rxBuff[0]);
          if (rxBuff[0] == 0) 
          {
          //  t_xchange(BDMCF_CMD_NOP,&rxBuff[6]);
          } 
          else 
          {
          cf_do_dsack();
          t_xchange(BDMCF_CMD_NOP,&rxBuff[6]);    
        
          // reread data from the same location 
          temp_new_address = hold_sendaddress + (recv_index/6)*1;
          
          t_xchange(BDMCF_CMD_READ16,&rxBuff[0]); /* send command */
          t_xchange(((temp_new_address>>16) & 0xFFFF), &rxBuff[0]); /* send high address */
          t_xchange((temp_new_address & 0xFFFF), &rxBuff[0]); /* send low address */
        
          delay_492ns(4);
        
          t_xchange(BDMCF_CMD_NOP, &rxBuff[0]); /* read high 16 bits of data */
          if (rxBuff[0] == 0) 
          {
          //t_xchange(BDMCF_CMD_NOP, &rxBuff[3]); /* read low 16 bits of data */            
          } 
          else 
          {
          cf_do_dsack();
          t_xchange(BDMCF_CMD_NOP,&rxBuff[0]);
          rxBuff[1] = 0xAD;   /* first byte sent back as XDXXEX, followed by ED ED ... */
          rxBuff[2] = 0xAD;
          rxBuff[0] = 0x01; /* Bus error occured */
          }
          }
          pOutputBuffer[recv_index] = rxBuff[0]; /* error */
          pOutputBuffer[recv_index+1] = rxBuff[2]; /* data */
          recv_index+=2; 
          bytes_read +=1;
          *pOutputLength+=2;
        } /* while */        
      return 0;
      break;

      case 0x19: /* write block long */     
      hold_sendword = BDMCF_CMD_WRITE32;

      hold_block_length = ((long)pInputBuffer[0]<<8) & 0xFF00;
      hold_block_length = hold_block_length | ((long)pInputBuffer[1] & 0x00FF);      

      hold_sendaddress = ((long)pInputBuffer[2]<<24) & 0xFF000000;
      hold_sendaddress = hold_sendaddress | (((long)pInputBuffer[3]<<16) & 0x00FF0000);
      hold_sendaddress = hold_sendaddress | (((long)pInputBuffer[4]<<8) & 0x0000FF00);
      hold_sendaddress = hold_sendaddress | (((long)pInputBuffer[5]) & 0x000000FF); 
        
      hold_senddata = ((long)pInputBuffer[6]<<24) & 0xFF000000;
      hold_senddata = hold_senddata | (((long)pInputBuffer[7]<<16) & 0x00FF0000);
      hold_senddata = hold_senddata | (((long)pInputBuffer[8]<<8) & 0x0000FF00);
      hold_senddata = hold_senddata | (((long)pInputBuffer[9]) & 0x000000FF);

      t_xchange(hold_sendword,&rxBuff[0]); /* send command */
      t_xchange(((hold_sendaddress>>16) & 0xFFFF), &rxBuff[0]); /* send high address */
      t_xchange((hold_sendaddress & 0xFFFF), &rxBuff[0]); /* send low address */
        
      t_xchange(((hold_senddata>>16) & 0xFFFF), &rxBuff[0]); /* send high word */
      t_xchange((hold_senddata & 0xFFFF), &rxBuff[0]); /* send low word */
        
	    delay_492ns(4);
	      
	    t_xchange((BDMCF_CMD_NOP | 0xFF0000),&rxBuff[0]);
	    if (rxBuff[0] != 0) 
	    {
        cf_do_dsack();        
   	    t_xchange(BDMCF_CMD_NOP,&rxBuff[0]);
        rxBuff[0] = 0xAD;           
	    }
	    pOutputBuffer[0] = rxBuff[0];
	    pOutputBuffer[1] = rxBuff[0];
	    *pOutputLength=2;
	    bytes_sent+=4;
      send_index = 10; /* place of next data longword in pInputBuffer */
      while (bytes_sent < hold_block_length) 
      {
      hold_senddata = ((long)pInputBuffer[send_index]<<24) & 0xFF000000;
      hold_senddata = hold_senddata | (((long)pInputBuffer[send_index+1]<<16) & 0x00FF0000);
      hold_senddata = hold_senddata | (((long)pInputBuffer[send_index+2]<<8) & 0x0000FF00);
      hold_senddata = hold_senddata | (((long)pInputBuffer[send_index+3]) & 0x000000FF);
      t_xchange(BDMCF_CMD_FILL32,&rxBuff[0]); /* send command */
      t_xchange(((hold_senddata>>16) & 0xFFFF), &rxBuff[0]); /* send high word */
      /* quit with an error */
      if (rxBuff[0] != 0) 
      {
	      pOutputBuffer[0] = rxBuff[0];
	      pOutputBuffer[1] = rxBuff[0];
        return 0;          
      } 
      t_xchange((hold_senddata & 0xFFFF), &rxBuff[0]); /* send low word */
      delay_492ns(4);
	    t_xchange((BDMCF_CMD_NOP | 0xFF0000),&rxBuff[0]);
      if (rxBuff[0] != 0) 
      {
        cf_do_dsack();
      }
      pOutputBuffer[0] = rxBuff[0];
	    pOutputBuffer[1] = rxBuff[0];
      
      bytes_sent+=4;
      send_index+=4; /* place of next data longword in pInputBuffer */
      } /* while */
	    
      return(0);
      break;

      case 0x1E: /* write block byte */
      hold_sendword = BDMCF_CMD_WRITE8;
      
      hold_errorval = 0;

      hold_block_length = ((long)pInputBuffer[0]<<8) & 0xFF00;
      hold_block_length = hold_block_length | ((long)pInputBuffer[1] & 0x00FF);      

      hold_sendaddress = ((long)pInputBuffer[2]<<24) & 0xFF000000;
      hold_sendaddress = hold_sendaddress | (((long)pInputBuffer[3]<<16) & 0x00FF0000);
      hold_sendaddress = hold_sendaddress | (((long)pInputBuffer[4]<<8) & 0x0000FF00);
      hold_sendaddress = hold_sendaddress | (((long)pInputBuffer[5]) & 0x000000FF); 
        
      hold_senddata = (pInputBuffer[6] << 8) | pInputBuffer[6]; //put in low byte

      t_xchange(hold_sendword,&rxBuff[0]); /* send command */
      t_xchange(((hold_sendaddress>>16) & 0xFFFF), &rxBuff[0]); /* send high address */
      t_xchange((hold_sendaddress & 0xFFFF), &rxBuff[0]); /* send low address */
        
      t_xchange(hold_senddata , &rxBuff[0]); /* send word */
        
	    delay_492ns(4);
	      
	    t_xchange((BDMCF_CMD_NOP | 0xFF0000),&rxBuff[0]);
	    if (rxBuff[0] != 0) 
	    {
        cf_do_dsack();
   	    t_xchange((BDMCF_CMD_NOP | 0xFF0000),&rxBuff[3]);
	    }

	    hold_errorval |= rxBuff[0];    
	    bytes_sent+=1;
      send_index = 7; /* place of next data longword in pInputBuffer */


//	    pOutputBuffer[0] = rxBuff[0];
//	    pOutputBuffer[1] = rxBuff[0];
//	    *pOutputLength=2;
	    
	    
      while (bytes_sent < hold_block_length) 
      {
      hold_senddata = pInputBuffer[send_index];
      t_xchange(BDMCF_CMD_FILL8,&rxBuff[0]); /* send command */
      t_xchange(((hold_senddata) & 0x00FF), &rxBuff[0]); /* send high word */
      /* quit with an error */
      if (rxBuff[0] != 0) 
      {
	      pOutputBuffer[0] = rxBuff[0];
	      pOutputBuffer[1] = rxBuff[0];
  	    *pOutputLength=2;
        return 0;          
      } 
      delay_492ns(4);
	    t_xchange((BDMCF_CMD_NOP | 0xFF0000),&rxBuff[0]);
      if (rxBuff[0] != 0) 
      {
        cf_do_dsack();        
      }

	    hold_errorval |= rxBuff[0];    
      
      bytes_sent+=1;
      send_index+=1; /* place of next data longword in pInputBuffer */      
      } /* while */
      
  
	    pOutputBuffer[0] = hold_errorval;
	    pOutputBuffer[1] = hold_errorval;
	    *pOutputLength=2;
  
      return(0);
      break;
  
      case 0x1F: /* write block word */
      hold_sendword = BDMCF_CMD_WRITE16;

      hold_block_length = ((long)pInputBuffer[0]<<8) & 0xFF00;
      hold_block_length = hold_block_length | ((long)pInputBuffer[1] & 0x00FF);      

      hold_sendaddress = ((long)pInputBuffer[2]<<24) & 0xFF000000;
      hold_sendaddress = hold_sendaddress | (((long)pInputBuffer[3]<<16) & 0x00FF0000);
      hold_sendaddress = hold_sendaddress | (((long)pInputBuffer[4]<<8) & 0x0000FF00);
      hold_sendaddress = hold_sendaddress | (((long)pInputBuffer[5]) & 0x000000FF); 
        
      hold_senddata = ((long)pInputBuffer[6]<<24) & 0xFF000000;
      hold_senddata = hold_senddata | (((long)pInputBuffer[7]<<16) & 0x00FF0000);

      t_xchange(hold_sendword,&rxBuff[0]); /* send command */
      t_xchange(((hold_sendaddress>>16) & 0xFFFF), &rxBuff[0]); /* send high address */
      t_xchange((hold_sendaddress & 0xFFFF), &rxBuff[0]); /* send low address */
        
      t_xchange(((hold_senddata>>16) & 0xFFFF), &rxBuff[0]); /* send word */
        
	    delay_492ns(4);
	      
	    t_xchange((BDMCF_CMD_NOP | 0xFF0000),&rxBuff[0]);
	    if (rxBuff[0] != 0) 
	    {
        cf_do_dsack();
   	    t_xchange(BDMCF_CMD_NOP,&rxBuff[0]);
        rxBuff[0] = 0xAD;           
	    }
	    pOutputBuffer[0] = rxBuff[0];
	    pOutputBuffer[1] = rxBuff[0];
	    *pOutputLength=2;
	    bytes_sent+=2;
      send_index = 8; /* place of next data longword in pInputBuffer */
      while (bytes_sent < hold_block_length) 
      {
      hold_senddata = ((long)pInputBuffer[send_index]<<24) & 0xFF000000;
      hold_senddata = hold_senddata | (((long)pInputBuffer[send_index+1]<<16) & 0x00FF0000);
      t_xchange(BDMCF_CMD_FILL16,&rxBuff[0]); /* send command */
      t_xchange(((hold_senddata>>16) & 0xFFFF), &rxBuff[0]); /* send high word */
      /* quit with an error */
      if (rxBuff[0] != 0) 
      {
	      pOutputBuffer[0] = rxBuff[0];
	      pOutputBuffer[1] = rxBuff[0];
        return 0;          
      } 
      delay_492ns(4);
	    t_xchange((BDMCF_CMD_NOP | 0xFF0000),&rxBuff[0]);
      if (rxBuff[0] != 0) 
      {
        cf_do_dsack();        
      }
      pOutputBuffer[0] = rxBuff[0];
	    pOutputBuffer[1] = rxBuff[0];
      
      bytes_sent+=2;
      send_index+=2; /* place of next data longword in pInputBuffer */      
      } /* while */
     return(0); 
     break;
   } /* switch */
return (1); // failure
}


//---------------------------------------------------------------------------
//	Execute flash programming algo
//---------------------------------------------------------------------------
	int
t_flash_prog (
	PUINT8  pData)	// ptr to data buffer
{
	return (0);			// Not used for CFV targets
}
//---------------------------------------------------------------------------
//	Set the BDM clock value of the target
//---------------------------------------------------------------------------
	void
t_set_clock (
	UINT32 clock)
{
	return;			// Not used for CFV targets
}


//---------------------------------------------------------------------------
//	Configure parameters 
//---------------------------------------------------------------------------
int t_config (byte configType, byte configParam, UINT32 paramVal)
{
 	return (0);			// Not used for CFV targets			
}


//-------------------------------------------------------------
