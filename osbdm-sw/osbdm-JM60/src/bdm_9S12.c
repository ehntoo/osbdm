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
*	File:		bdm_9S12.c
*
*	Purpose: 	BDM 9S12 command formatter file
*				Contains commands specific to BGND devices
*				Supports 9S12 processors
*
*	History:	August 2008, Initial Development
*
*
* 03/08/2011  : Modified by P&E Microcomputer Systems
*               http://www.pemicro.com/osbdm
*
*
\*****************************************************************************/ 

#include "derivative.h"				// Include peripheral declarations  
#include "typedef.h"				// Include peripheral declarations  
#include "MCU.h"					// MCU header
#include "timer.h"					// timer functions
#include "targetAPI.h"				// target API include file
#include "bdm_9S12.h"				// BDM include file
#include "bdm_bgnd_driver.h"		// The driver interface
#include "board_id.h"		  // Board ID

//---------------------------------------------------------------------------
// Firmware Info
//---------------------------------------------------------------------------

volatile const byte TARGET_TYPE  =  eS12;
volatile const byte FIRMWARE_TYPE   =  eGeneric;

//---------------------------------------------------------------------------
// Internal definitions and variables
//---------------------------------------------------------------------------

static byte txBuff [8];				// Buffer for write bytes of BGND protocol
static byte rxBuff [8];				// Buffer for read  bytes of BGND protocol

byte   EnableBDM (void);			// Function to enable BDM mode

//---------------------------------------------------------------------------
//	Sync state machine definitions
//---------------------------------------------------------------------------

typedef enum
	{
	SyncReset = 0,	// Called when explicitly syncing, no future sync request
	SyncIfNeeded,	// Sync if needed; no future sync request
	SyncNow,		// Sync now; no future sync request
	SyncNext,		// Sync if needed; sync on next SyncIfNeeded request
	} SyncRequest;

//---------------------------------------------------------------------------
//	Data access macros - currently designed to swap bytes for Intel host
//---------------------------------------------------------------------------

#define GetDataByte32(pData, index) (pData[3 - index])
#define GetDataByte16(pData, index) (pData[1 - index])

//---------------------------------------------------------------------------
// BDM command codes
//---------------------------------------------------------------------------

#define CMDBDM_ACKENB      0xD5 //  d
#define CMDBDM_ACKDSB      0xD6 //  d

#define CMDBDM_BGND        0x90 //  d
#define CMDBDM_GO          0x08 //  d
#define CMDBDM_GO_UNTIL    0x0C //  d
#define CMDBDM_TRACE1      0x10 //  d
#define CMDBDM_TAGGO       0x18 //  d	(deprecated in the 9S12X family)

#define CMDBDM_READBD8     0xE4 //  A16/d/R16
#define CMDBDM_READBD16    0xEC //  A16/d/R16

#define CMDBDM_WRITEBD8    0xC4 //  A16/W16/d
#define CMDBDM_WRITEBD16   0xCC //  A16/W16/d

#define CMDBDM_READ8       0xE0 //  A16/d/R16
#define CMDBDM_READ16      0xE8 //  A16/d/R16

#define CMDBDM_WRITE8      0xC0 //  A16/W16/d
#define CMDBDM_WRITE16     0xC8 //  A16/W16/d

#define CMDBDM_READNEXT    0x62 //  d/R16 (X = X+2)
#define CMDBDM_WRITENEXT   0x42 //  W16/d (X = X+2)

#define CMDBDM_READPC      0x63 //  d/R16
#define CMDBDM_READD       0x64 //  d/R16
#define CMDBDM_READX       0x65 //  d/R16
#define CMDBDM_READY       0x66 //  d/R16
#define CMDBDM_READSP      0x67 //  d/R16

#define CMDBDM_WRITEPC     0x43 //  W16/d
#define CMDBDM_WRITED      0x44 //  W16/d
#define CMDBDM_WRITEX      0x45 //  W16/d
#define CMDBDM_WRITEY      0x46 //  W16/d
#define CMDBDM_WRITESP     0x47 //  W16/d

//----------------------------------------------------------------------------
// Enumerators for the debug registers
//----------------------------------------------------------------------------

enum
	{
	BDCSCR_ENBDM  = 0x80, // R/W
	BDCSCR_BDMACT = 0x40, // RONLY
	BDCSCR_ENTAG  = 0x20, // R/W
	BDCSCR_SDV    = 0x10, // DO NOT ALTER
	BDCSCR_TRACE  = 0x08, // DO NOT ALTER
	BDCSCR_CLKSW  = 0x04, // R/W
	BDCSCR_UNSEC  = 0x02, // RONLY
	BDCSCR_rfu    = 0x01  // RONLY
	};

// Debug register indices

#define DEBUG_BDMSTS (0xFF01)
#define DEBUG_BDMCCR (0xFF06)
#define DEBUG_BDMINR (0xFF07)

//---------------------------------------------------------------------------
//	Sync manager state machine (internal use only)
//---------------------------------------------------------------------------
	void
SyncCheck (
	SyncRequest  request)			// Requested sync machine action

	{
static byte  synconcheck = FALSE;	// TRUE if requesting a future sync

	switch (request)
        {
        case SyncReset:
			synconcheck = FALSE;
            break;

        case SyncIfNeeded:
			if (synconcheck)
				(void) t_sync();
			synconcheck = FALSE;
            break;

        case SyncNow:
			(void) t_sync();
			synconcheck = FALSE;
            break;

        case SyncNext:
			if (synconcheck)
				(void) t_sync();
			synconcheck = TRUE;
			break;
        }
	}

//---------------------------------------------------------------------------
//	Check target power status and enable Vout if target not self-powered,
//  enable and initialize BDM signals to target, issue reset to target
//---------------------------------------------------------------------------
/*	byte				// Returns 0 for success, 1 for failure
t_init (void)
	{
	BgndInit();						// Initialize the BGND driver

	if (bdm_vtrg_en())				// Enable Vout to target if not self-powered
		return (1);					// Failed, return error
	wait_ms(10);

  if (EnableBDM())      // Enable BDM
    return (1);	

	return (0);						// Return success
	}
*/

//---------------------------------------------------------------------------
//	Check target power status and enable Vout if target not self-powered,
//  enable and initialize BDM signals to target, issue reset to target
//---------------------------------------------------------------------------
	byte				// Returns 0 for success, 1 for failure
t_init (void)
	{
	BgndInit();						// Initialize the BGND driver

	if (bdm_vtrg_en())				// Enable Vout to target if not self-powered
		return (1);					// Failed, return error
	wait_ms(10);

  if (EnableBDM())      // Enable BDM
    return (1);	
  

	return (0);						// Return success
	}

	void t_debug_init()
{

}

void t_serial_init()
{

}


//---------------------------------------------------------------------------
//	Assert reset to target device
//---------------------------------------------------------------------------
	byte				// Returns 0 for success, 1 for failure
t_reset (
	ResetT  mode)	// Mode:  0 soft reset to BDM mode, 
	              //      1 hard reset to Running mode, 
	              //      2 hard reset to BDM mode, 
	              //      3 voltage reset to BDM mode (bdm must supply power)
	              //			4 soft reset to Running mode
	{
		 		
	SyncCheck(SyncReset);
	SyncCheck(SyncNext);
	
  switch(mode){
    case eSoftReset_to_DebugMode: // Soft Reset to BDM mode
		  // NOTE: the DBG module cannot cause a soft reset on HCS12 - using hard reset instead
      (void) BgndReset(0);  // hard reset to BDM (halted) mode
      break;

    case eHardReset_to_NormalMode:   
      return BgndReset(1);  // hard reset to normal (running) mode

    case eHardReset_to_DebugMode:
      (void) BgndReset(0);  // hard reset to BDM (halted) mode
      break;

    case ePowerReset_to_DebugMode:
      (void) BgndReset(2);  // voltage reset to BDM (halted) mode
      break;

     case eSoftReset_to_NormalMode:
     default:
		  // NOTE: the DBG module cannot cause a soft reset on HCS12 - using hard reset instead
      return BgndReset(1);  // hard reset to normal (running) mode
  }
}


//---------------------------------------------------------------------------
//	Perform a sync operation on the target
//---------------------------------------------------------------------------
	word		// Returns the sync interval in 60 MHz. clocks, or 0 for timeout
t_sync (void)

	{
	UINT16  clock;			// The returned clock value

	SyncCheck(SyncReset);	// Reset the sync state machine
	clock = BgndSync();		// Sync the target
	if (clock != 0)			// If there is a target,
		t_enable_ack(1);	// ACK is required by the 9S12 driver
	return (clock);
	}

//---------------------------------------------------------------------------
//	Resynchronize host and target if comm is disrupted
//---------------------------------------------------------------------------
	byte				// Returns 0 for success, 1 for failure
t_resync (void)

	{
	return (t_sync() == 0);
	}

//---------------------------------------------------------------------------
//	Get the current BDM clock value of the target
//---------------------------------------------------------------------------
int t_clear_array (PUINT8 data, int data_length) 
{
  int i;
  
  for (i=1; i<= data_length; i++)
    data[i-1] = 0;
   return(0);
}

	UINT32
t_get_clock (void)

	{
	return (UINT32)(BgndGetClock());
	}

int t_special_feature(unsigned char sub_cmd_num,  // Special feature number (sub_cmd_num)
	                     PUINT16 pInputLength,      // Length of Input Command
	                     PUINT8  pInputBuffer,               // Input Command Buffer
	                     PUINT16 pOutputLength,     // Length of Output Response
	                     PUINT8  pOutputBuffer)    {          // Output Response Buffer 
int i;
*pOutputLength = 0;
// modify for CFV1
switch (sub_cmd_num)
        {
        case 0xAA:
          for (i=1;i<=*pInputLength;i++) 
            pOutputBuffer[i-1] = pInputBuffer[i-1] ^ 0xff;
          *pOutputLength = *pInputLength;
          return (0) ; //success
          break;
        case 0x01: // send sync request
          SyncCheck(SyncNow); // allow PC to sync to the part
          return (0) ; // success
          break;
        case 0xE5:
          return (0); // success
          break;
        case 0x33: // BDM/serial signal shutdown 
          TBGND_EN = 0; // tristate BGND signal
          *pOutputLength = 0;
          return (0);
          break;
          
        case 0x34: // BDM/serial signal enable 
          *pOutputLength = 0;
          return (0);
          break;
        

        }

return (1); // failure
}

//---------------------------------------------------------------------------
//	Set the BDM clock value of the target
//---------------------------------------------------------------------------
	void
t_set_clock (
	UINT32 clock)

	{
	SyncCheck(SyncReset);			// Reset the sync state machine
	BgndSetClock((UINT16)(clock));	// Will also sync to the target
	(void) t_enable_ack(1);			// ACK mode is required by the 9S12 driver
	}

//---------------------------------------------------------------------------
//	Enable/disable the programming voltage power supply
//---------------------------------------------------------------------------
	int
t_flash_power (
	byte enable)		// 1 to enable, 0 to disable

	{
	return (0);			// Not used for 9S12 targets
	}

//---------------------------------------------------------------------------
//	Enable/disable the programming voltage to the target chip
//---------------------------------------------------------------------------
	int
t_flash_enable (
	byte enable)		// 1 to enable, 0 to disable

	{
	return (0);			// Not used for 9S12 targets
	}

//---------------------------------------------------------------------------
//	Execute flash programming algo
//---------------------------------------------------------------------------
	int
t_flash_prog (
	PUINT8  pData)	// ptr to data buffer

	{
	return (0);			// Not used for CFV1 targets
	}

//---------------------------------------------------------------------------
//	Enable/disable the target ACK protocol
//---------------------------------------------------------------------------
	int					// Returns 0 for success, 1 for failure
t_enable_ack (
	byte  enable)		// 1 to enable, 0 to disable

	{
	byte  status;		// Returned status

	if ( ! BgndAckSupported())
		return (enable != 0);	// The driver does not support ACK

	SyncCheck(SyncIfNeeded);

	txBuff[0] = ((enable) ? CMDBDM_ACKENB : CMDBDM_ACKDSB);
	status    = BgndTxRx(txBuff, NULL, 1, 0);

	return (status != 0);
	}

//---------------------------------------------------------------------------
//	Get the current state of the PST inputs
//---------------------------------------------------------------------------
	byte				// Returns the current PST state
bdmcf_pst_status (void)

	{
	// Not required for a BDM connection (signals not used), but required to link
	return (0);
	}

//---------------------------------------------------------------------------
//	Asserts TA to target for user delay in 10us increments
//---------------------------------------------------------------------------
	void
t_assert_ta (
	word dly_cnt)

	{
	// Not required for a BDM connection (signal not used), but required to link
	}

//---------------------------------------------------------------------------
//	Mass erase and unsecure target FLASH.
//	Wait_ms() provides delay to allow FLASH erase to complete before exit
//---------------------------------------------------------------------------
#pragma MESSAGE DISABLE C5703
	char
t_unsecure (
	byte lockr,
	byte lrcnt,
	byte clkdiv)

	{
	BgndUnsecure();	// (Not implemented)
	wait_ms(1);
	return 0;
	}
#pragma MESSAGE DEFAULT C5703

//---------------------------------------------------------------------------
//	Write a control register (unsupported for 9S12)
//---------------------------------------------------------------------------
#pragma MESSAGE DISABLE C5703
	int					// Returns 0 for success, 1 for failure
t_write_creg (
	UINT32   addr,		// Register number
	PUINT8  pData)		// Ptr to value buffer

	{
	return (1);			// Return error for unsupported
	}
#pragma MESSAGE DEFAULT C5703

//---------------------------------------------------------------------------
//	Read control register (unsupported for 9S12)
//---------------------------------------------------------------------------
#pragma MESSAGE DISABLE C5703
	int					// Returns 0 for success, 1 for failure
t_read_creg (
	UINT32   addr,		// Register number
	PUINT8  pData)		// Ptr to return buffer

	{
	return (1);			// Return error for unsupported
	}
#pragma MESSAGE DEFAULT C5703

//---------------------------------------------------------------------------
//	Write Debug Register (8 bit)
//---------------------------------------------------------------------------
	int					// Returns 0 for success, 1 for failure
t_write_dreg (
	UINT32   addr,		// Register number (FF0n)
	UINT8    uWidth,	// Register width (ignored)
	PUINT8  pData)		// Ptr to single byte value buffer

	{
	int      status;	// The returned status

	SyncCheck(SyncIfNeeded);

	txBuff[0] = CMDBDM_WRITEBD8;
	txBuff[1] = (addr >> 8) & 0xFF;			// Addr MSB first
	txBuff[2] = (addr     ) & 0xFF;
	txBuff[3] = *pData;	// Write to both bytes,
	txBuff[4] = *pData;	// to be odd/even insensitive
	status    = BgndTxRx(txBuff, NULL, 5, 0);

	return (status != 0);
	}

//---------------------------------------------------------------------------
//	Read Debug Register (8 bit)
//---------------------------------------------------------------------------
	int					// Returns 0 for success, 1 for failure
t_read_dreg (
	UINT32   addr,		// Register address (FF0n)
	UINT8    uWidth,	// Register width (ignored)
	PUINT8  pData)		// Ptr to single byte return buffer

	{
	byte     status;	// The returned status

	SyncCheck(SyncIfNeeded);

	txBuff[0] = CMDBDM_READBD8;
	txBuff[1] = (addr >> 8) & 0xFF;			// Addr MSB first
	txBuff[2] = (addr     ) & 0xFF;
	status    = BgndTxRx(txBuff, rxBuff, 3, 2);
	pData[0]  = GetDataByte16(rxBuff, !(addr & 1)); // odd registers returned in [1], even in [0]

	return (status != 0);
	}

//---------------------------------------------------------------------------
//	Write a CPU register
//---------------------------------------------------------------------------
	int					// Returns 0 for success, 1 for failure
t_write_ad (
	UINT32    addr,		// Register number: [0:4]
	PUINT8  pData)		// Ptr to four byte Little Endian register value

	{
	byte     status;	// The returned status
  addr = 0x000000FF & addr;
  
	SyncCheck(SyncIfNeeded);

	switch (addr)
        {
        case BGNDREG_D:  txBuff[0] = CMDBDM_WRITED;  break;
        case BGNDREG_X:  txBuff[0] = CMDBDM_WRITEX;  break;
        case BGNDREG_Y:  txBuff[0] = CMDBDM_WRITEY;  break;
        case BGNDREG_SP: txBuff[0] = CMDBDM_WRITESP; break;
        case BGNDREG_PC: txBuff[0] = CMDBDM_WRITEPC; break;
		default:         return (1);
        }

	txBuff[1] = GetDataByte16(pData, 0);	// from Host-Endian order
	txBuff[2] = GetDataByte16(pData, 1);
	status    =	BgndTxRx(txBuff, NULL, 3, 0);

	return (status != 0);
	}

//---------------------------------------------------------------------------
//	Read a CPU register
//---------------------------------------------------------------------------
	int					// Returns 0 for success, 1 for failure
t_read_ad (
	UINT32    addr,		// Register number: [0:4]
	PUINT8  pData)		// Ptr to four byte Little Endian register value

	{
	byte     status;	// The returned status
  addr = 0x000000FF & addr;
  
	SyncCheck(SyncIfNeeded);

	pData[0] = 0;		// Preclear the return buffer
	pData[1] = 0;
	pData[2] = 0;
	pData[3] = 0;

	switch (addr)
        {
        case BGNDREG_D:  txBuff[0] = CMDBDM_READD;  break;
        case BGNDREG_X:  txBuff[0] = CMDBDM_READX;  break;
        case BGNDREG_Y:  txBuff[0] = CMDBDM_READY;  break;
        case BGNDREG_SP: txBuff[0] = CMDBDM_READSP; break;
        case BGNDREG_PC: txBuff[0] = CMDBDM_READPC; break;
		default:         return (1);
        }

	status   = BgndTxRx(txBuff, rxBuff, 1, 2);
	pData[0] = GetDataByte16(rxBuff, 0);	// to Host-Endian order
	pData[1] = GetDataByte16(rxBuff, 1);

	return (status != 0);
	}

//---------------------------------------------------------------------------
//	Write data to target memory (internal implementation)
//---------------------------------------------------------------------------
	int					// Returns 0 for success, 1 for failure
bdm_write_mem (
	UINT32   addr,		// Target memory starting write address
	int      count,		// Total number of BYTEs
	PUINT8  pData)		// Ptr to Little-Endian origination data

	{
	// Receives 8-bit data, using 16-bit writes, preswapped as needed

	if (addr & 0x0001)	// Check for an odd initial address
		{
		txBuff[0] = CMDBDM_WRITE8;		// Yes, write one initial odd byte
		txBuff[1] = (addr >> 8) & 0xFF;	// Addr MSB first
		txBuff[2] = (addr     ) & 0xFF;	// only 16 bits used
//		txBuff[3] = 0;
		txBuff[4] = pData[0];
		if (BgndTxRx(txBuff, NULL, 5, 0) != 0)
			return (1);
		addr     += 1;
		pData    += 1;
		count    -= 1;
		}

	txBuff[0] = CMDBDM_WRITE16;
	while (count >= 2)	// Write all possible byte pairs
		{
		txBuff[1] = (addr >> 8) & 0xFF;
		txBuff[2] = (addr     ) & 0xFF;
		txBuff[3] = pData[0];
		txBuff[4] = pData[1];
		if (BgndTxRx(txBuff, NULL, 5, 0) != 0)
			return (1);
		addr     += 2;
		pData    += 2;
		count    -= 2;
		}

	if (count != 0)		// Check for a final odd byte
		{
		txBuff[0] = CMDBDM_WRITE8;		// Yes, write one trailing odd byte
		txBuff[1] = (addr >> 8) & 0xFF;
		txBuff[2] = (addr     ) & 0xFF;
		txBuff[3] = pData[0];
//		txBuff[4] = 0;
		if (BgndTxRx(txBuff, NULL, 5, 0) != 0)
			return (1);
		}

	return (0);
	}

//---------------------------------------------------------------------------
//	Write data to target memory
//---------------------------------------------------------------------------
	int					// Returns 0 for success, 1 for failure
t_write_mem (
	byte     type,		// Memory type (ignored)
	UINT32   addr,		// Target memory starting write address
	UINT8    width,		// Object width (8/16/32)
	int      count,		// Total number of BYTEs
	PUINT8  pData)		// Ptr to Little-Endian origination data

	{
static byte  buffer[4];	// Used to byte swap as required

	if (count <= 0)		// Error if invalid request length
		return (1);

	SyncCheck(SyncIfNeeded);

	if (width == 8)				// 8-bit writes
        {
		return (bdm_write_mem(addr, count, pData));
		}

	else if (width == 16)		// 16-bit writes
		{
		while (count >= 2)
            {
			buffer[0] = GetDataByte16(pData, 0);
			buffer[1] = GetDataByte16(pData, 1);

			if (bdm_write_mem(addr, 2, buffer))
				return (1);
			addr     += 2;
			pData    += 2;
			count    -= 2;
            }
		}

	else /* (width == 32) */	// 32-bit writes
		{
		while (count >= 4)
            {
			buffer[0] = GetDataByte32(pData, 0);
			buffer[1] = GetDataByte32(pData, 1);
			buffer[2] = GetDataByte32(pData, 2);
			buffer[3] = GetDataByte32(pData, 3);

			if (bdm_write_mem(addr, 4, buffer))
				return (1);
			addr     += 4;
			pData    += 4;
			count    -= 4;
            }
		}

	return (0);
	}

//---------------------------------------------------------------------------
//	Read data from the target memory (internal implementation)
//---------------------------------------------------------------------------
	int					// Returns 0 for success, 1 for failure
bdm_read_mem (
	UINT32   addr,		// Target memory starting read address
	int      count,		// Total number of BYTEs
	PUINT8  pData)		// Ptr to Little-Endian destination buffer

	{
	// Delivers 8-bit data, using 16-bit reads, to be postswapped as needed

	if (addr & 0x0001)	// Check for an odd initial address
		{
		txBuff[0] = CMDBDM_READ8;		// Yes, read one initial odd byte
		txBuff[1] = (addr >> 8) & 0xFF;	// Addr MSB first
		txBuff[2] = (addr     ) & 0xFF;	// only 16 bits used
		if (BgndTxRx(txBuff, rxBuff, 3, 2) != 0)
			return (1);
//		pData[X]  = rxBuff[0];
		pData[0]  = rxBuff[1];
		addr     += 1;
		pData    += 1;
		count    -= 1;
		}

	txBuff[0] = CMDBDM_READ16;	// Read all possible byte pairs
	while (count >= 2)
		{
		txBuff[1] = (addr >> 8) & 0xFF;
		txBuff[2] = (addr     ) & 0xFF;
		if (BgndTxRx(txBuff, rxBuff, 3, 2) != 0)
			return (1);
		pData[0]  = rxBuff[0];
		pData[1]  = rxBuff[1];
		addr     += 2;
		pData    += 2;
		count    -= 2;
		}

	if (count != 0)		// Check for a final odd byte
		{
		txBuff[0] = CMDBDM_READ8;		// Yes, read one trailing odd byte
		txBuff[1] = (addr >> 8) & 0xFF;
		txBuff[2] = (addr     ) & 0xFF;
		if (BgndTxRx(txBuff, rxBuff, 3, 2) != 0)
			return (1);
		pData[0]  = rxBuff[0];
//		pData[X]  = rxBuff[1];
		}

	return (0);
	}

//---------------------------------------------------------------------------
//	Read data from the target memory
//---------------------------------------------------------------------------
	int					// Returns 0 for success, 1 for failure
t_read_mem (
	byte     type,		// Memory type (ignored)
	UINT32   addr,		// Target memory starting read address
	UINT8    width,		// Object width (8/16/32)
	int      count,		// Total number of BYTEs
	PUINT8  pData)		// Ptr to Little-Endian destination buffer

	{
static byte  buffer[4];	// Used to byte swap as required

	if (count <= 0)		// Error if invalid request length
		return (1);

	SyncCheck(SyncIfNeeded);

	if (width == 8)				// 8-bit reads
        {
		return (bdm_read_mem(addr, count, pData));
		}

	else if (width == 16)		// 16-bit reads
		{
		while (count >= 2)
            {
			if (bdm_read_mem(addr, 2, buffer))
				return (1);

			pData[0] = GetDataByte16(buffer, 0);
			pData[1] = GetDataByte16(buffer, 1);
			addr    += 2;
			pData   += 2;
			count   -= 2;
            }
		}

	else /* (width == 32) */	// 32-bit reads
		{
		while (count >= 4)
            {
			if (bdm_read_mem(addr, 4, buffer))
				return (1);

			pData[0] = GetDataByte32(buffer, 0);
			pData[1] = GetDataByte32(buffer, 1);
			pData[2] = GetDataByte32(buffer, 2);
			pData[3] = GetDataByte32(buffer, 3);
			addr    += 4;
			pData   += 4;
			count   -= 4;
            }
		}

	return (0);
	}

//---------------------------------------------------------------------------
//	Fill target memory with data
//---------------------------------------------------------------------------
	int					// Returns 0 for success, 1 for failure
t_fill_mem (
	byte     type,		// Memory type (ignored)
	UINT32   addr,		// Target memory starting write address
	UINT8    width,		// Object width (8/16/32)
	int      count,		// Total number of 8/16/32 bit OBJECTs
	PUINT8  pData)		// Ptr to Little-Endian 8/16/32 bit fill object

	{
	int      size;		// Number bytes per write	

	if (count <= 0)		// Error if invalid request length
		return (1);

 	if      (width ==  8)
		size = 1;
	else if (width == 16)
		size = 2;
	else /* (width == 32) */
		size = 4;

	while (count-- > 0)
		{
		if (t_write_mem(type, addr, width, size, pData) != 0)
			return (1);
		addr += size;
		}

	return (0);
	}

//---------------------------------------------------------------------------
//	Enable BDM processing
//---------------------------------------------------------------------------
	byte				// Returns 0 for success, 1 for failure
EnableBDM (void)

	{
	byte  dreg;			// Temporary debug register buffer
	byte  status;		// Returned status

	SyncCheck(SyncNow);

	status = (byte)(t_read_dreg(DEBUG_BDMSTS, 8, &dreg));
	if ((dreg & BDCSCR_ENBDM) == 0)						// Is BDM enabled?
		{
		dreg   |= BDCSCR_ENBDM;							// No, enable it
		status |= t_write_dreg(DEBUG_BDMSTS, 8, &dreg);	// and write it back
		}
	return (status != 0);
	}

//---------------------------------------------------------------------------
//	Halt target execution
//---------------------------------------------------------------------------
	byte				// Returns 0 for success, 1 for failure
t_halt (void)

	{
	byte  dreg;			// Temporary debug register buffer
	byte  status = EnableBDM();		// Initialize cumulative status

	wait_ms(1);											// Wait for it to happen

	txBuff[0] = CMDBDM_BGND;							// Issue the HALT command
	status   |= BgndTxRx(txBuff, NULL, 1, 0);

	wait_ms(1);											// Wait for it to happen

	status |= t_read_dreg(DEBUG_BDMSTS, 8, &dreg);		// Check if succeeded
	if ((dreg & BDCSCR_BDMACT) == 0)
		status = 1;										// Did not halt

	return (status != 0);
	}

//---------------------------------------------------------------------------
//	Resume target execution from the current PC
//---------------------------------------------------------------------------
	byte				// Returns 0 for success, 1 for failure
t_go (void)

	{
	byte  dreg;			// Temporary debug register buffer
	byte  status = 0;	// Cumulative status

	SyncCheck(SyncNext);

	txBuff[0] = CMDBDM_GO;							// Issue the GO command
	status   |= BgndTxRx(txBuff, NULL, 1, 0);

	wait_ms(1);										// Wait for it to happen

	status |= t_read_dreg(DEBUG_BDMSTS, 8, &dreg);	// Check if succeeded
	if ((dreg & BDCSCR_BDMACT) != 0)
		status = 1;									// Did not start

	return (status != 0);
	}

//---------------------------------------------------------------------------
//	Execute one instruction from the current PC
//---------------------------------------------------------------------------
	byte				// Returns 0 for success, 1 for failure
t_step (void)

	{
	byte  status;		// Cumulative status

	SyncCheck(SyncNext);

	txBuff[0] = CMDBDM_TRACE1;				// Issue the STEP command
	status    = BgndTxRx(txBuff, NULL, 1, 0);

	return (status != 0);
	}

//---------------------------------------------------------------------------
//	Return the bdm/target status information to host
//	
//	pData[] will be loaded with bytes as follows:
//		0    = ACK support (No = 0,  Yes = 1) (8b)
//		1    = Target reset detect (No = 0, Yes = 1)  (8b)
//		2    = Target detection state (NONE = 0, SYNC = 1, Manual = 3) (8b)
//		3..9 = unused
//---------------------------------------------------------------------------
	int					// Returns 0 for success, 1 for failure
t_stat (
	PUINT8  pData)		// Ptr to return status buffer

{

   	SyncCheck(SyncIfNeeded);
  	pData[0] = BgndAckSupported();
	  pData[1] = BgndResetDetect();
  	pData[2] = BgndClockState();
    pData[3] = VERSION_HW;	// Hardware Version
    pData[4] = VERSION_SW;	// Firmware Version
    pData[5] =
  	pData[6] =
  	pData[7] =
  	pData[8] =
  	pData[9] =  0;

 	  return (0);
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
//		5    = BUILD_VER_REV
//    6    = board_id
//    7    = osbdm_id
//    8..9 = unused
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


 //---------------------------------------------------------------------------
//	Configure parameters 
//---------------------------------------------------------------------------
int t_config (byte configType, byte configParam, UINT32 paramVal)
{
 	return (0);			// Not used for S12 targets			
}

//---------------------------------- EOF ------------------------------------
