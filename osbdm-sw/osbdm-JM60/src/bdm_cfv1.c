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
*	File:		bdm_CFV1.c
*
*	Purpose: 	BDM CFV1 command formatter file.
*				Contains commands specific to BGND devices
*				Supports CFV1 (51JM128 series) processors
*
*	History:	August 2008, Initial Development
*
* 03/08/2011  : Modified by P&E Microcomputer Systems
*               http://www.pemicro.com/osbdm
*
*
*
\*****************************************************************************/

#include "derivative.h"				// Include peripheral declarations  
#include "typedef.h"				// Include peripheral declarations  
#include "MCU.h"					// MCU header
#include "timer.h"					// timer functions
#include "targetAPI.h"				// target API include file
#include "bdm_CFV1.h"				// BDM include file
#include "bdm_bgnd_driver.h"		// The driver interface
#include "board_id.h"		  // Board ID

//---------------------------------------------------------------------------
// Firmware Info
//---------------------------------------------------------------------------

#define  TARGET_TYPE    eCFv1;
#define  FIRMWARE_TYPE  eGeneric;

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

#define CMDBDM_ACKENB        0x02 //  d
#define CMDBDM_ACKDSB        0x03 //  d
#define CMDBDM_BGND          0x04 //  d

#define CMDBDM_DUMPMEM8      0x32 //  d/R8
#define CMDBDM_DUMPMEM16     0x36 //  d/R16
#define CMDBDM_DUMPMEM32     0x3A //  d/R32

#define CMDBDM_DUMPMEM8WS    0x33 //  d/SS/R8
#define CMDBDM_DUMPMEM16WS   0x37 //  d/SS/R16
#define CMDBDM_DUMPMEM32WS   0x3B //  d/SS/R32

#define CMDBDM_FILLMEM8      0x12 //  W8/d/SS
#define CMDBDM_FILLMEM16     0x16 //  W16/d/SS
#define CMDBDM_FILLMEM32     0x1A //  W32/d/SS

#define CMDBDM_FILLMEM8WS    0x13 //  W8/d/SS
#define CMDBDM_FILLMEM16WS   0x17 //  W16/d/SS
#define CMDBDM_FILLMEM32WS   0x1B //  W32/d/SS

#define CMDBDM_GO            0x08 //  d
#define CMDBDM_NOP           0x00 //  d

#define CMDBDM_READCREG      0xE0 //  d/R32 (cmd += reg number)
#define CMDBDM_READDREG      0xA0 //  d/R32 (cmd += reg number)

#define CMDBDM_READMEM8      0x30 //  A24/d/R8
#define CMDBDM_READMEM16     0x34 //  A24/d/R16
#define CMDBDM_READMEM32     0x38 //  A24/d/R32

#define CMDBDM_READMEM8WS    0x31 //  A24/d/SS/R8
#define CMDBDM_READMEM16WS   0x35 //  A24/d/SS/R16
#define CMDBDM_READMEM32WS   0x39 //  A24/d/SS/R32

#define CMDBDM_READPSTB      0x50 //  d/R32 (cmd += reg number)

#define CMDBDM_READREG       0x60 //  d/R32 (cmd += reg number)

#define CMDBDM_READXCSR      0x2D //  d/R8
#define CMDBDM_READCSR2      0x2E //  d/R8
#define CMDBDM_READCSR3      0x2F //  d/R8

#define CMDBDM_SYNCPC        0x01 //  d

#define CMDBDM_WRITECREG     0xC0 //  W32/d (cmd += reg number)
#define CMDBDM_WRITEDREG     0x80 //  W32/d (cmd += reg number)

#define CMDBDM_WRITEMEM8     0x10 //  A24/W8/d
#define CMDBDM_WRITEMEM16    0x14 //  A24/W16/d
#define CMDBDM_WRITEMEM32    0x18 //  A24/W32/d

#define CMDBDM_WRITEMEM8WS   0x11 //  A24/W8/d/SS
#define CMDBDM_WRITEMEM16WS  0x15 //  A24/W16/d/SS
#define CMDBDM_WRITEMEM32WS  0x19 //  A24/W32/d/SS

#define CMDBDM_WRITEREG      0x40 //  W32/d (cmd += reg number)

#define CMDBDM_WRITEXCSR     0x0D //  W8/d
#define CMDBDM_WRITECSR2     0x0E //  W8/d
#define CMDBDM_WRITECSR3     0x0F //  W8/d

//----------------------------------------------------------------------------
// Enumerators for the XCSR and CSR registers
//----------------------------------------------------------------------------

enum	// XCSR register high byte significant bits
	{
	XCSR_CPUHALT  = 0x80, // RONLY
	XCSR_CPUSTOP  = 0x40, // RONLY
	XCSR_BDMSTAT  = 0x38, // R/W
	XCSR_CLKSW    = 0x04, // R/W
	XCSR_SECERASE = 0x02, // R/W
	XCSR_ENBDM    = 0x01, // R/W
	};

enum	// CSR register significant bits
	{
	CSR_SSM       = 0x10, // R/W
	};

enum	// Debug register indices
	{
	DEBUG_CSR     = 0x00,
	DEBUG_XCSR    = 0x01
	};

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
	byte				// Returns 0 for success, 1 for failure
t_init (void)
	{
#if 0
	if (bdm_vtrg_en())				// Enable Vout to target if not self-powered
		return (1);					// Failed, return error

	if (t_reset(0))					// Reset target and leave stopped
		return (1);					// Failed, return error

	BgndInit();						// Initialize the BGND driver
	wait_ms(1);	
	(void) t_sync();				// Synchronize the BGND driver

	return (0);						// Return success
#else
	BgndInit();						// Initialize the BGND driver

	if (bdm_vtrg_en())				// Enable Vout to target if not self-powered
		return (1);					// Failed, return error
	wait_ms(10);

  if (EnableBDM())
    return (1);	
  
	return (0);						// Return success
#endif
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
	ResetT mode)	// Mode:  0 soft reset to BDM mode, 
	              //      1 hard reset to Running mode, 
	              //      2 hard reset to BDM mode, 
	              //      3 voltage reset to BDM mode (bdm must supply power)
	              //			4 soft reset to Running mode
	{

	byte  cmd;
	byte  status = 0;		// Returned status

	SyncCheck(SyncReset);
	SyncCheck(SyncNext);
	
  switch(mode){
    case eSoftReset_to_DebugMode: // Soft Reset to BDM mode
			cmd = 0x03;	// Soft reset to BDM mode command
		  (void) t_write_dreg(2, 8, &cmd);	// Issue the soft reset command
		  wait_ms(2);							// Allow the reset to complete
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
//			cmd = 0x01;	// Soft reset to NORMAL (run) mode command
			cmd = 0x03;	// Soft reset to BDM mode command
		  (void) t_write_dreg(2, 8, &cmd);	// Issue the soft reset command
		  wait_ms(20);							// Allow the reset to complete
//			status += EnableBDM();		// Initialize cumulative status
			status += t_go();			// enter normal (running) mode
			return status;		  
  }
	return t_halt();					// Enable BDM and Halt target
}

//---------------------------------------------------------------------------
//	Perform a sync operation on the target
//---------------------------------------------------------------------------
	word			// Returns the sync interval in 60 MHz. clocks, or 0 for timeout
t_sync (void)

	{
	SyncCheck(SyncReset);		// Reset the sync state machine
	return (BgndSync());		// Sync the target
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
	UINT32
t_get_clock (void)

	{
	SyncCheck(SyncNow);
	return (UINT32)(BgndGetClock());
	}

	int t_special_feature(unsigned char sub_cmd_num,  // Special feature number (sub_cmd_num)
	                     PUINT16 pInputLength,      // Length of Input Command
	                     PUINT8  pInputBuffer,               // Input Command Buffer
	                     PUINT16 pOutputLength,     // Length of Output Response
	                     PUINT8  pOutputBuffer)    {          // Output Response Buffer 
int i;
UINT8  temp_cmd;
UINT32 hold_address;
hold_address = 0;
*pOutputLength = 0;

// modify for CFV1
switch (sub_cmd_num)
        {
        case 0xAA:
          //for (i=1;i<=*pInputLength;i++) 
          //  pOutputBuffer[i-1] = pInputBuffer[i-1] ^ 0xff;
          //*pOutputLength = *pInputLength;
          return (0) ; //success
          break;
        case 0x01: // send sync request
          SyncCheck(SyncNow); // allow PC to sync to the part
          //wait_ms(25);
          return (0) ; // success
          break;

        case 0x07: // Set BKGD low if pInputBuffer[0] = 0, high-z if pInputBuffer[0] = FF
          if (pInputBuffer[0] == 0xff) 
          {
            BgndHiz();  
          }
          if (pInputBuffer[0] == 0x00) 
          {
            BgndLow();
          }
          *pOutputLength = 0; 
          return (0);
          break;
        case 0x08:
          if (pInputBuffer[0] == 0xff) 
          {  
            ResetHiz();
          }
          if (pInputBuffer[0] == 0x00) 
          {
            ResetLow(); 
          }
          *pOutputLength = 0;
          return (0);
          break;
           
            
            

        case 0x1D: 
          //for (i=1;i<=*pInputLength;i++) 
          //  {
          //  pOutputBuffer[i-1] = pInputBuffer[i-1] ^ 0xff;
          //  }
          *pOutputLength = 0;
          for (i=0; i<*pInputLength-1; i++)
          {
            temp_cmd = *pInputBuffer++;
            switch(temp_cmd) 
            {
              case 0x02:
              i++;
              BgndTxRx(pInputBuffer++, NULL, 1, 0x0);
              BgndLow();
              break;  
            
              case 0x00:
              i++;
              BgndTxRx(pInputBuffer++, NULL, 1, 0);
              //BgndDelay();
              break;
              
              case 0x07:
              i++;
              pInputBuffer++;
              BgndTxRx(NULL, rxBuff, 0, 1);
              pOutputBuffer[*pOutputLength]=rxBuff[0];
              (*pOutputLength)++;
              //BgndDelay();
              break;

              default:
                i++;
                pInputBuffer++;
              break;
                
            }
              
          }
          return (0);
          break;
          
        case 0x20: /* turn off power to target */
          VTRG_EN = 0;
          *pOutputLength = 0;
          return (0);
          break;
        case 0x21: /* turn on power to target */
          VTRG_EN = 1;
          *pOutputLength = 0;
          return (0);
          break;           
        case 0x2D: 
          *pOutputLength = 0;
          pInputBuffer++; // remove dummy character
          for (i=0; i<*pInputLength-2; i++)
          {
            temp_cmd = *pInputBuffer++;
            switch(temp_cmd) 
            {
              case 0x00:
              i++;
              BgndTxRx(pInputBuffer++, NULL, 1, 0);
              BgndDelay();
              break;
              
              case 0x01:
              i++;
              pInputBuffer++;
              BgndTxRx(NULL, rxBuff, 0, 1);
              pOutputBuffer[*pOutputLength]=rxBuff[0];
              (*pOutputLength)++;
              BgndDelay();
              break;

              default:
                i++;
                pInputBuffer++;
              break;               
            }              
          }
          return (0);
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
        
          
        }  // switch (sub_cmd_num)
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
	}

//---------------------------------------------------------------------------
//	Enable/disable the programming voltage power supply
//---------------------------------------------------------------------------
	int
t_flash_power (
	byte enable)		// 1 to enable, 0 to disable

	{
	return (0);			// Not used for CFV1 targets
	}

//---------------------------------------------------------------------------
//	Enable/disable the programming voltage to the target chip
//---------------------------------------------------------------------------
	int
t_flash_enable (
	byte enable)		// 1 to enable, 0 to disable

	{
	return (0);			// Not used for CFV1 targets
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
  // for this to work bdm must power the target
  (void) t_reset(3); // voltage reset to BDM mode

	BgndUnsecure();	
	wait_ms(1);
	return 0;
	}
#pragma MESSAGE DEFAULT C5703

//---------------------------------------------------------------------------
//	Write any X register (local implementation function)
//---------------------------------------------------------------------------
	int					// Returns 0 for success, 1 for failure
bdm_write_xreg (
	UINT32   addr,		// Register number; translate to command code
	PUINT8  pData)		// Ptr to 8b register value

	{
	int      status;	// The returned status

	SyncCheck(SyncIfNeeded);  // BWJ was SyncNow during fix testing

	if      (addr == 1)
		txBuff[0] = CMDBDM_WRITEXCSR;
	else if (addr == 2)
		txBuff[0] = CMDBDM_WRITECSR2;
	else if (addr == 3)
		txBuff[0] = CMDBDM_WRITECSR3;
	else
		return (1);		// Invalid register address

	txBuff[1] = *pData;
	status    = BgndTxRx(txBuff, NULL, 2, 0);

	return (status != 0);
	}

//---------------------------------------------------------------------------
//	Read any X register (local implementation function)
//---------------------------------------------------------------------------
	int					// Returns 0 for success, 1 for failure
bdm_read_xreg (
	UINT32   addr,		// Register number; translate to command code
	PUINT8  pData)		// Ptr to 8b register value

	{
	int      status;	// The returned status

	SyncCheck(SyncIfNeeded);  // BWJ was SyncNow during fix testing

	if      (addr == 1)
		txBuff[0] = CMDBDM_READXCSR;
	else if (addr == 2)
		txBuff[0] = CMDBDM_READCSR2;
	else if (addr == 3)
		txBuff[0] = CMDBDM_READCSR3;
	else
		return (1);		// Invalid register address

	status    = BgndTxRx(txBuff, rxBuff, 1, 1);
	*pData    = rxBuff[0];

	return (status != 0);
	}

//---------------------------------------------------------------------------
//	Write any register (local implementation function)
//---------------------------------------------------------------------------
	int					// Returns 0 for success, 1 for failure
bdm_write_reg (
	byte     cmd,		// The command
	PUINT8  pData)		// Ptr to host-Endian 32b register value

	{
	int      status;	// The returned status

	SyncCheck(SyncIfNeeded);  // BWJ was SyncNow during fix testing

	txBuff[0] = cmd;
	txBuff[1] = GetDataByte32(pData, 0);
	txBuff[2] = GetDataByte32(pData, 1);
	txBuff[3] = GetDataByte32(pData, 2);
	txBuff[4] = GetDataByte32(pData, 3);
	status    = BgndTxRx(txBuff, NULL, 5, 0);

	return (status != 0);
	}

//---------------------------------------------------------------------------
//	Read any register (local implementation function)
//---------------------------------------------------------------------------
	int					// Returns 0 for success, 1 for failure
bdm_read_reg (
	byte     cmd,		// The command
	PUINT8  pData)		// Ptr to host-Endian 32b register buffer

	{
	int      status;	// The returned status

	SyncCheck(SyncIfNeeded);  // BWJ was SyncNow during fix testing

	txBuff[0] = cmd;
	status    = BgndTxRx(txBuff, rxBuff, 1, 4);
	pData[0]  = GetDataByte32(rxBuff, 0);
	pData[1]  = GetDataByte32(rxBuff, 1);
	pData[2]  = GetDataByte32(rxBuff, 2);
	pData[3]  = GetDataByte32(rxBuff, 3);

	return (status != 0);
	}

//---------------------------------------------------------------------------
//	Write a control register
//---------------------------------------------------------------------------
	int					// Returns 0 for success, 1 for failure
t_write_creg (
	UINT32   addr,		// Register number
	PUINT8  pData)		// Ptr to host-Endian 32b register value

	{
// BWJ PUT SANITY CHECK IN
	SyncCheck(SyncIfNeeded);  // BWJ was SyncNow during fix testing

	return (bdm_write_reg(CMDBDM_WRITECREG + addr, pData));
	}

//---------------------------------------------------------------------------
//	Read control register
//---------------------------------------------------------------------------
	int					// Returns 0 for success, 1 for failure
t_read_creg (
	UINT32   addr,		// Register number
	PUINT8  pData)		// Ptr to host-Endian 32b register buffer

	{
	pData[0] = 0;		// Preclear the return buffer
	pData[1] = 0;
	pData[2] = 0;
	pData[3] = 0;

// BWJ PUT SANITY CHECK IN
	SyncCheck(SyncIfNeeded);  // BWJ was SyncNow during fix testing
  
	return (bdm_read_reg(CMDBDM_READCREG + addr, pData));
	}

//---------------------------------------------------------------------------
//	Write Debug Register
//---------------------------------------------------------------------------
	int					// Returns 0 for success, 1 for failure
t_write_dreg (
	UINT32   addr,		// Register number
	UINT8    uWidth,	// Register width
	PUINT8  pData)		// Ptr to host-Endian 32b register value

	{
	SyncCheck(SyncIfNeeded);  // BWJ was SyncNow during fix testing

	if (uWidth == 8)
		return (bdm_write_xreg(addr, pData));

	return (bdm_write_reg(CMDBDM_WRITEDREG + addr, pData));
	}

//---------------------------------------------------------------------------
//	Read Debug Register
//---------------------------------------------------------------------------
	int					// Returns 0 for success, 1 for failure
t_read_dreg (
	UINT32   addr,		// Register number
	UINT8    uWidth,	// Register width
	PUINT8  pData)		// Ptr to host-Endian 32b register buffer

	{
	pData[0] = 0;		// Preclear the return buffer
	pData[1] = 0;
	pData[2] = 0;
	pData[3] = 0;

	SyncCheck(SyncIfNeeded);  // BWJ was SyncNow during fix testing

	if (uWidth == 8)
		return (bdm_read_xreg(addr, pData));

	return (bdm_read_reg(CMDBDM_READDREG + addr, pData));
	}

//---------------------------------------------------------------------------
//	Write a CPU register
//---------------------------------------------------------------------------
	int					// Returns 0 for success, 1 for failure
t_write_ad (
	UINT32    addr,		// Register number
	PUINT8  pData)		// Ptr to host-Endian 32b register value

	{
	SyncCheck(SyncIfNeeded);  // BWJ was SyncNow during fix testing
  
  addr = 0x000000FF & addr;
  
	if (addr > 0x10)
		return (1);
	if (addr == 0x10) 	// Support PC access through this command also
		return (t_write_creg(0x0F, pData));
	return (bdm_write_reg(CMDBDM_WRITEREG + addr, pData));
	}

//---------------------------------------------------------------------------
//	Read a CPU register
//---------------------------------------------------------------------------
	int					// Returns 0 for success, 1 for failure
t_read_ad (
	UINT32    addr,		// Register number
	PUINT8  pData)		// Ptr to host-Endian 32b register buffer

	{
	pData[0] = 0;		// Preclear the return buffer
	pData[1] = 0;
	pData[2] = 0;
	pData[3] = 0;
	
	addr = 0x000000FF & addr;

	if (addr > 0x10)
		return (1);
	
	SyncCheck(SyncIfNeeded);  // BWJ was SyncNow during fix testing

	if (addr == 0x10) 	// Support PC access through this command also
		return (t_read_creg(0x0F, pData));
	return (bdm_read_reg(CMDBDM_READREG + addr, pData));
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
	if (count <= 0)		// Error if invalid request length
		return (1);

	SyncCheck(SyncIfNeeded);

	txBuff[1] = (addr >> 16) & 0xFF;	// Addr MSB first
	txBuff[2] = (addr >>  8) & 0xFF;	// Addr MSB first
	txBuff[3] = (addr      ) & 0xFF;	// Only 24 bits used

	if (width == 8)
        {
		txBuff[0] = CMDBDM_WRITEMEM8;
		txBuff[4] = pData[0];
		if (BgndTxRx(txBuff, NULL, 5, 0) != 0)
			return (1);
		count    -= 1;

		txBuff[0] = CMDBDM_FILLMEM8;
		while (count >= 1)
			{
			pData    += 1;
			txBuff[1] = pData[0];
			if (BgndTxRx(txBuff, NULL, 2, 0) != 0)
				return (1);
			count    -= 1;
			}
		}
		
	else if (width == 16)
		{
		if (count >= 2)
            {
			txBuff[0] = CMDBDM_WRITEMEM16;
			txBuff[4] = GetDataByte16(pData, 0);
			txBuff[5] = GetDataByte16(pData, 1);
			if (BgndTxRx(txBuff, NULL, 6, 0) != 0)
				return (1);
			count    -= 2;
			}

		txBuff[0] = CMDBDM_FILLMEM16;
		while (count >= 2)
			{
			pData    += 2;
			txBuff[1] = GetDataByte16(pData, 0);
			txBuff[2] = GetDataByte16(pData, 1);
			if (BgndTxRx(txBuff, NULL, 3, 0) != 0)
				return (1);
			count    -= 2;
			}
		}

	else /* (width == 32) */
		{
		if (count >= 4)
            {
			txBuff[0] = CMDBDM_WRITEMEM32;
			txBuff[4] = GetDataByte32(pData, 0);
			txBuff[5] = GetDataByte32(pData, 1);
			txBuff[6] = GetDataByte32(pData, 2);
			txBuff[7] = GetDataByte32(pData, 3);
			if (BgndTxRx(txBuff, NULL, 8, 0) != 0)
				return (1);
			count    -= 4;
			}

		txBuff[0] = CMDBDM_FILLMEM32;
		while (count >= 4)
			{
			pData    += 4;
			txBuff[1] = GetDataByte32(pData, 0);
			txBuff[2] = GetDataByte32(pData, 1);
			txBuff[3] = GetDataByte32(pData, 2);
			txBuff[4] = GetDataByte32(pData, 3);
			if (BgndTxRx(txBuff, NULL, 5, 0) != 0)
				return (1);
			count    -= 4;
			}
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
	if (count <= 0)		// Error if invalid request length
		return (1);

	SyncCheck(SyncIfNeeded);

	txBuff[1] = (addr >> 16) & 0xFF;	// Addr MSB first
	txBuff[2] = (addr >>  8) & 0xFF;
	txBuff[3] = (addr      ) & 0xFF;	// Only 24 bits used

	if (width == 8)
        {
		txBuff[0] = CMDBDM_READMEM8;
		if (BgndTxRx(txBuff, rxBuff, 4, 1) != 0)
			return (1);
		*pData++  = rxBuff[0];
		count    -= 1;

		txBuff[0] = CMDBDM_DUMPMEM8;
		while (count >= 1)
			{
			if (BgndTxRx(txBuff, rxBuff, 1, 1) != 0)
				return (1);
			*pData++ = rxBuff[0];
			count   -= 1;
			}
		}

	else if (width == 16)
		{
		if (count >= 2)
			{
			txBuff[0] = CMDBDM_READMEM16;
			if (BgndTxRx(txBuff, rxBuff, 4, 2) != 0)
				return (1);
			*pData++  = GetDataByte16(rxBuff, 0);
			*pData++  = GetDataByte16(rxBuff, 1);
			count    -= 2;
			}

		txBuff[0] = CMDBDM_DUMPMEM16;
		while (count >= 2)
			{
			if (BgndTxRx(txBuff, rxBuff, 1, 2) != 0)
				return (1);
			*pData++ = GetDataByte16(rxBuff, 0);
			*pData++ = GetDataByte16(rxBuff, 1);
			count   -= 2;
			}
		}

	else /* (width == 32) */
		{
		if (count >= 4)
			{
			txBuff[0] = CMDBDM_READMEM32;
			if (BgndTxRx(txBuff, rxBuff, 4, 4) != 0)
				return (1);
			*pData++  = GetDataByte32(rxBuff, 0);
			*pData++  = GetDataByte32(rxBuff, 1);
			*pData++  = GetDataByte32(rxBuff, 2);
			*pData++  = GetDataByte32(rxBuff, 3);
			count    -= 4;
			}

		txBuff[0] = CMDBDM_DUMPMEM32;
		while (count >= 4)
			{
			if (BgndTxRx(txBuff, rxBuff, 1, 4) != 0)
				return (1);
			*pData++ = GetDataByte32(rxBuff, 0);
			*pData++ = GetDataByte32(rxBuff, 1);
			*pData++ = GetDataByte32(rxBuff, 2);
			*pData++ = GetDataByte32(rxBuff, 3);
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
	byte  status;		// Returned status

	SyncCheck(SyncNow);

	txBuff[0] = CMDBDM_READXCSR;
	status    = BgndTxRx(txBuff, &txBuff[1], 1, 1);
	if ((txBuff[1] & XCSR_ENBDM) == 0)		// Is BDM enabled?
		{
		txBuff[1] |= XCSR_ENBDM;			// No, enable it
		txBuff[0]  = CMDBDM_WRITEXCSR;		// and write it back
		status    |= BgndTxRx(txBuff, NULL, 2, 0);
		}
	return (status != 0);
	}

//---------------------------------------------------------------------------
//	Halt target execution
//---------------------------------------------------------------------------
	byte				// Returns 0 for success, 1 for failure
t_halt (void)

	{
	byte  status = EnableBDM();		// Initialize cumulative status

	txBuff[0] = CMDBDM_BGND;				// Issue the HALT command
	status   |= BgndTxRx(txBuff, NULL, 1, 0);

	wait_ms(1);								// Wait for it to happen

	txBuff[0] = CMDBDM_NOP;					// Do a NOP in case the first one is ignored
	status   |= BgndTxRx(txBuff, NULL, 1, 0);

	txBuff[0] = CMDBDM_READXCSR;			// Check if succeeded (stat shows halted)
	status   |= BgndTxRx(txBuff, rxBuff, 1, 1);
	if ((rxBuff[0] & XCSR_CPUHALT) == 0)
		status = 1;							// Did not halt

	return (status != 0);
	}

//---------------------------------------------------------------------------
//	Resume target execution from the current PC
//---------------------------------------------------------------------------
	byte				// Returns 0 for success, 1 for failure
t_go (void)

	{
	byte  status;		// Cumulative status

	SyncCheck(SyncNext);  // BWJ was SyncNow during fix testing

	// Make sure the step mode bit is clear
	txBuff[0] = CMDBDM_READDREG + DEBUG_CSR;
	status    = BgndTxRx(txBuff, &txBuff[1], 1, 4);
	if ((txBuff[4] & CSR_SSM) != 0)			// Check if it is off
		{
		txBuff[4] &= ~CSR_SSM;				// No, turn it off
		txBuff[0]  = CMDBDM_WRITEDREG + DEBUG_CSR;	// and write it back
		status    |= BgndTxRx(txBuff, NULL, 5, 0);
		}

	txBuff[0] = CMDBDM_GO;					// Issue the GO command
	status   |= BgndTxRx(txBuff, NULL, 1, 0);

	wait_ms(1);								// Wait for it to take effect

	txBuff[0] = CMDBDM_NOP;					// Do a NOP in case the first cmd is ignored
	status   |= BgndTxRx(txBuff, NULL, 1, 0);

	txBuff[0] = CMDBDM_READXCSR;			// Check if succeeded (stat shows not halted)
	status   |= BgndTxRx(txBuff, rxBuff, 1, 1);
	if ((rxBuff[0] & XCSR_CPUHALT) != 0)
		status = 1;							// Did not start

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

	// Make sure the step mode bit is set
	txBuff[0] = CMDBDM_READDREG + DEBUG_CSR;
	status    = BgndTxRx(txBuff, &txBuff[1], 1, 4);
	if ((txBuff[4] & CSR_SSM) == 0)			// Check if it is on
		{
		txBuff[4] |= CSR_SSM;				// No, turn it on
		txBuff[0]  = CMDBDM_WRITEDREG + DEBUG_CSR;
		status    |= BgndTxRx(txBuff, NULL, 5, 0);	// and write it back
		}

	txBuff[0] = CMDBDM_GO;					// Issue the GO command
	status   |= BgndTxRx(txBuff, NULL, 1, 0);

	wait_ms(1);								// Wait for it to take effect

	txBuff[0] = CMDBDM_NOP;					// Do a NOP in case the first cmd is ignored
	status   |= BgndTxRx(txBuff, NULL, 1, 0);

	return (status != 0);
	}

//---------------------------------------------------------------------------
//	Return the bdm/target status information to host
//	
//	pData[] will be loaded with bytes as follows:
//		0    = ACK support (No = 0,  Yes = 1) (8b)
//		1    = Target reset detect (No = 0, Yes = 1)  (8b)
//		2    = Target detection state (NONE = 0, SYNC = 1, Manual = 3) (8b)
//		3..4 = HW/FW version information
//		5..9 = unused
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
 	return (0);			// Not used for CFV1 targets			
}

//---------------------------------- EOF ------------------------------------
