/* OSBDM-JM60 bdm_9s12z.c 
 * Copyright (C) 2011 P&E Microcomputer Systems, Inc.
 * http://www.pemicro.com
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
*	Author:		P&E Microcomputer Systems
*           www.pemicro.com/osbdm
*
*	File:	   	bdm_9s12z.c
*
*	Purpose: 	BDM 9S12Z command formatter file.
*				    Contains commands specific to BGND devices
*				    Supports 9S12Z processors
*
* 08/22/2011  : Modified by P&E Microcomputer Systems
*               http://www.pemicro.com/osbdm
*
*
*
\*****************************************************************************/

#include "derivative.h"				// Include peripheral declarations  
#include "typedef.h"				  // Include peripheral declarations  
#include "MCU.h"					    // MCU header
#include "timer.h"					  // timer functions
#include "targetAPI.h"				// target API include file
#include "bdm_9s12z.h"				  // BDM include file
#include "bdm_bgnd_driver.h"	// The driver interface
#include "board_id.h"		      // Board ID

//---------------------------------------------------------------------------
// Firmware Info
//---------------------------------------------------------------------------

#define  TARGET_TYPE    eS12Z;
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
			cmd = 0x03;	// Soft reset to BDM mode command
		  (void) t_write_dreg(2, 8, &cmd);	// Issue the soft reset command
		  wait_ms(20);							// Allow the reset to complete
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

// modified for S12Z
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
              //send byte and keep bkgd low
              case 0x02:
              i++;
              BgndTxRx(pInputBuffer++, NULL, 1, 0x0);
              BgndLow();
              break;            
            
              case 0x00:
              i++;
              BgndTxRx(pInputBuffer++, NULL, 1, 0);
              BgndDelay();
              break;
              
              case 0x07:
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
	return osbdm_error_fail;
	//return (0);			// Not used for S12Z targets
	}

//---------------------------------------------------------------------------
//	Enable/disable the programming voltage to the target chip
//---------------------------------------------------------------------------
	int
t_flash_enable (
	byte enable)		// 1 to enable, 0 to disable

	{
	return osbdm_error_fail;
	}

//---------------------------------------------------------------------------
//	Execute flash programming algo
//---------------------------------------------------------------------------
	int
t_flash_prog (
	PUINT8  pData)	// ptr to data buffer

	{
	return osbdm_error_fail;
	}
	
//---------------------------------------------------------------------------
//	Enable/disable the target ACK protocol
//---------------------------------------------------------------------------
	int					// Returns 0 for success, 1 for failure
t_enable_ack (
	byte  enable)		// 1 to enable, 0 to disable

	{
	return osbdm_error_fail;
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

  return osbdm_error_fail;
	
	}

//---------------------------------------------------------------------------
//	Read any X register (local implementation function)
//---------------------------------------------------------------------------
	int					// Returns 0 for success, 1 for failure
bdm_read_xreg (
	UINT32   addr,		// Register number; translate to command code
	PUINT8  pData)		// Ptr to 8b register value

	{
	
	return osbdm_error_fail;
	
	}

//---------------------------------------------------------------------------
//	Write any register (local implementation function)
//---------------------------------------------------------------------------
	int					// Returns 0 for success, 1 for failure
bdm_write_reg (
	byte     cmd,		// The command
	PUINT8  pData)		// Ptr to host-Endian 32b register value

	{
	
	return osbdm_error_fail;
		
	}

//---------------------------------------------------------------------------
//	Read any register (local implementation function)
//---------------------------------------------------------------------------
	int					// Returns 0 for success, 1 for failure
bdm_read_reg (
	byte     cmd,		// The command
	PUINT8  pData)		// Ptr to host-Endian 32b register buffer

	{
	return osbdm_error_fail;
	
	}

//---------------------------------------------------------------------------
//	Write a control register
//---------------------------------------------------------------------------
	int					// Returns 0 for success, 1 for failure
t_write_creg (
	UINT32   addr,		// Register number
	PUINT8  pData)		// Ptr to host-Endian 32b register value

	{
	
	return osbdm_error_fail;
	
	}

//---------------------------------------------------------------------------
//	Read control register
//---------------------------------------------------------------------------
	int					// Returns 0 for success, 1 for failure
t_read_creg (
	UINT32   addr,		// Register number
	PUINT8  pData)		// Ptr to host-Endian 32b register buffer

	{
	
	return osbdm_error_fail;
	
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
	return osbdm_error_fail;
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
	
	return osbdm_error_fail;

	}

//---------------------------------------------------------------------------
//	Write a CPU register
//---------------------------------------------------------------------------
	int					// Returns 0 for success, 1 for failure
t_write_ad (
	UINT32    addr,		// Register number
	PUINT8  pData)		// Ptr to host-Endian 32b register value

	{
	return osbdm_error_fail;
	
	}

//---------------------------------------------------------------------------
//	Read a CPU register
//---------------------------------------------------------------------------
	int					// Returns 0 for success, 1 for failure
t_read_ad (
	UINT32    addr,		// Register number
	PUINT8  pData)		// Ptr to host-Endian 32b register buffer

	{
	
	return osbdm_error_fail;
	
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
	return osbdm_error_fail;

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
	
	return osbdm_error_fail;

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
	
	return osbdm_error_fail;
	
	}

//---------------------------------------------------------------------------
//	Enable BDM processing
//---------------------------------------------------------------------------
	byte				// Returns 0 for success, 1 for failure
EnableBDM (void)

	{
	byte  status;		// Returned status

	SyncCheck(SyncNow);
	
	return osbdm_return_success;
	
	}

//---------------------------------------------------------------------------
//	Halt target execution
//---------------------------------------------------------------------------
	byte				// Returns 0 for success, 1 for failure
t_halt (void)

	{
	
	return osbdm_return_success;
	}

//---------------------------------------------------------------------------
//	Resume target execution from the current PC
//---------------------------------------------------------------------------
	byte				// Returns 0 for success, 1 for failure
t_go (void)

	{
	
	return osbdm_error_fail;

	}

//---------------------------------------------------------------------------
//	Execute one instruction from the current PC
//---------------------------------------------------------------------------
	byte				// Returns 0 for success, 1 for failure
t_step (void)

	{
	return osbdm_error_fail;
	
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
  return osbdm_error_fail;			
}

//---------------------------------- EOF ------------------------------------
