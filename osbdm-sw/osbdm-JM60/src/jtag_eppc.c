/* OSBDM-JM60 Target Interface Software Package
 * Portions Copyright (C) 2011 P&E Microcomputer Systems, Inc.
 * Portions Copyright (C) 2009 Freescale Semiconductor
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

/*
* 03/08/2011  : Created by P&E Microcomputer Systems
*               http://www.pemicro.com/osbdm
*
*
*/


#include <stdlib.h>
#include <string.h>
#include "derivative.h"				// include peripheral declarations
#include "typedef.h"				// Include peripheral declarations
#include "jtag_eppc.h"
#include "jtag_jm60.h"              // JTAG declarations for JM60 debug pod
#include "commands.h"				// BDM commands header file
#include "MCU.h"					// MCU header
#include "timer.h"				    // timer functions
#include "targetAPI.h"				// target API include file
#include "board_id.h"				// read the hardware ID if available
#include "serial_io.h"      // Serial port handling/enable

//---------------------------------------------------------------------------
// Firmware Info
//---------------------------------------------------------------------------
  
volatile const byte TARGET_TYPE   =  ePPC;

volatile const byte FIRMWARE_TYPE  =  eEmbeddedGeneric;
   

//---------------------------------------------------------------------------
// Sequence
//---------------------------------------------------------------------------


//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
// TODO:  these functions need to be defined to comply with common command processor

void t_assert_ta(word dly_cnt) { return; }
int t_write_ad(unsigned long addr, unsigned char *data) { return osbdm_error_fail; }
int t_read_ad(unsigned long addr, unsigned char *data) { return osbdm_error_fail; }

#define tms_tdi_transaction_compression_start 0x40
#define tms_tdi_transaction_compression_end 0x4F
#define tms_only_transaction_compression_start 0x50
#define tms_only_transaction_compression_end 0x5F
      
static unsigned short tms_only_transaction_compression_array_tmsval[tms_only_transaction_compression_end-tms_only_transaction_compression_start+1];
static unsigned char tms_only_transaction_compression_array_bitsval[tms_only_transaction_compression_end-tms_only_transaction_compression_start+1];
static unsigned short tms_tdi_transaction_compression_array_tmsval[tms_tdi_transaction_compression_end-tms_tdi_transaction_compression_start+1];
static unsigned short tms_tdi_transaction_compression_array_tdival[tms_tdi_transaction_compression_end-tms_tdi_transaction_compression_start+1];
static unsigned char tms_tdi_transaction_compression_array_bitsval[tms_tdi_transaction_compression_end-tms_tdi_transaction_compression_start+1];
unsigned char osbdm_jtag_boardid = 0;
unsigned char output_buffer_enable_value = 0x00;

#define output_buffer_enable_mask 0x80

static osbdm_error
jtag_startup (void)
{

  output_buffer_enable_value = 0x80; // EMBEDDED TOWER
	
	PTED = output_buffer_enable_value ^ output_buffer_enable_mask;	// disable output enable (buffer)

  PTEDD = 0xED;	// set direction
	

	TMS_RESET();		// set TMS low
	TRST_RESET();
	TCLK_RESET();

	PTBDD |= 0x0C;	// TMS, TCLK signals output
	PTBD &= 0xF3;

  return osbdm_error_ok;
}

/******************************************************************************
*	t_halt(void)
*
*	asserts BKPT to target to HALT execution
*
*	Return:	0 on success, non-zero on fail
*
******************************************************************************/
byte t_halt()
{
return osbdm_error_fail;
}

/******************************************************************************
*	t_go(void)
*
*	resume target execution from current PC
*
*	Return:	0 on success, non-zero on fail
*
******************************************************************************/
byte t_go(void)
{
return osbdm_error_fail;
}

/******************************************************************************
*	t_step(void)
*
*	resume target execution from current PC
*
*	Return:	0 on success, non-zero on fail
*
******************************************************************************/
byte t_step()
{
return osbdm_error_fail;
//	return dsc_step_core(0, 1);	// single step
}

/*------------------------------------------------------------------------------
	t_read_mem
	------------
	Read data from the target memory

	type = type of memory
	addr = target address to read from
	width = size of the reads (8/16/32)
	count = total number of BYTES to read
	*data = pointer to data buffer to hold data that's read

	returns 0 on success, non-zero on fail
*/
int t_read_mem(unsigned char type, unsigned long addr, unsigned char width, int count, unsigned char *data)
{
return osbdm_error_fail;
}

/*------------------------------------------------------------------------------
	t_write_mem
	-------------
	Write data to target memory

	type = type of memory
	addr = target address to write to
	width = size of the writes (8/16/32)
	count = total number of BYTES to be written
	*data = pointer to data buffer containing the data

	returns 0 on success
*/
int t_write_mem(unsigned char type, unsigned long addr, unsigned char width, int count, unsigned char *data)
{
return osbdm_error_fail;
}

/*------------------------------------------------------------------------------
	t_fill_mem
	-------------
	Fill data to target memory

	type = type of memory
	addr = target address to fill to
	width = size of the fill units (8/16/32)
	count = total number of units to be filled
	*data = pointer to data buffer containing the data

	returns 0 on success
*/
int t_fill_mem(unsigned char type, unsigned long addr, unsigned char width, int count, unsigned char *data)
{
return osbdm_error_fail;
}

//-------------------------------------------------------------
// in this version addr is a register index number
int t_write_creg(unsigned long addr, unsigned char *data)
{
  return osbdm_error_fail;	
}

//-------------------------------------------------------------
int t_read_creg(unsigned long addr, unsigned char *data)
{
  return osbdm_error_fail;	
}

/******************************************************************************
*	t_reset(mode)
*
*	assert reset to target device.  tRSTO is inverted version of tRST to target
*
*	Input:	0 = reset to ONcE mode, 1 = reset to Normal Mode
*	Return:	0 on success, 1 on fail
*
******************************************************************************/
byte t_reset(ResetT  mode)
{
  return osbdm_error_fail;	
}



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
byte t_init(void)
{

    
  if (jtag_startup() != osbdm_error_ok)
        return 1;
       
    return 0;
}

void t_debug_init()
{
	PTBPE_PTBPE2 = 1;   // Pull up Board ID pins
	PTBPE_PTBPE4 = 1;   // Pull up Board ID pins
	wait_ms(10);                 // 10ms
	osbdm_jtag_boardid = (PTBD_PTBD2) | (PTBD_PTBD4 << 1);
    jtag_startup();
}

void t_serial_init()
{
if ((PTEDD & 0x80) == 0x00) {
  t_init();
}

}


//---------------------------------------------------------------------------
//	Return the bdm/target status information to host
//
//	pData[] will be loaded with bytes as follows:
//		0    = unused
//		1    = unused
//		2    = unused
//		3    = unused
//		4    = unused
//		3..9 = unused
//
//  Returns 0 for success, 1 for failure
//---------------------------------------------------------------------------
int	t_stat(unsigned char *data) { 

	return (osbdm_error_fail);
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


//-------------------------------------------------------------
char t_unsecure(byte lockr, byte lrcnt, byte clkdiv)
{
return osbdm_error_fail;
}

//-------------------------------------------------------------
int t_flash_power (byte enable)
{
return osbdm_error_fail;
}

//-------------------------------------------------------------
unsigned long t_get_clock (void)
{
return osbdm_error_fail;
}

//---------------------------------------------------------------------------
//	Execute flash programming algorithm
//---------------------------------------------------------------------------
int t_flash_prog (PUINT8 pData)
{
return osbdm_error_fail;
}

//-------------------------------------------------------------
int t_write_dreg   (UINT32 addr, UINT8 uWidth, PUINT8 pData)
{
return osbdm_error_fail;
}

//-------------------------------------------------------------
int t_read_dreg    (UINT32 addr, UINT8 uWidth, PUINT8 pData)
{
return osbdm_error_fail;
}
//---------------------------------------------------------------------------
//	Set the BDM clock value of the target
//---------------------------------------------------------------------------
void t_set_clock (UINT32 clock){
}


void  xchng16(unsigned char bitcount, 
                     UINT16 tdival,
                     UINT16 tmsval,
                     PUINT16 tdoval) {

	char b;

  PTED = (PTED & (output_buffer_enable_mask ^ 0xff)) | output_buffer_enable_value;

	*tdoval= 0; 
	
	// bang each bit out and receive a bit back each time
	for(b=0; b<bitcount; b++){
		if(tmsval & 0x0001 == 0x0001) {
		  TMS_SET();	        // bring TMS high
		}
		else {
		  
		  TMS_RESET();
		}
		if(tdival & 0x0001 == 0x0001) {
		  
				  TDI_OUT_SET();	        // bring TMS high
//				  TDI_OUT_SET();	        // bring TMS high
//				  TDI_OUT_SET();	        // bring TMS high
		}
		else {
		  
          TDI_OUT_RESET();
//          TDI_OUT_RESET();
//          TDI_OUT_RESET();
          
		}
		
	TCLK_SET();		// TCLK High     

		tdival >>=1;				// shift to next output bit
		tmsval >>=1;				// shift to next output bit
    *tdoval >>= 1;

	// return TDO status
	if(TDO_IN_SET){
    *tdoval= *tdoval | 0x8000;
	}
	  TCLK_RESET();	// TCLK Low
	}
  if (TARGET_TYPE != ePPC)
   PTED = (PTED & (output_buffer_enable_mask ^ 0xff)) | (output_buffer_enable_value ^ output_buffer_enable_mask);
}


int t_special_feature(unsigned char sub_cmd_num,  // Special feature number (sub_cmd_num)
	                     PUINT16 pInputLength,      // Length of Input Command
	                     PUINT8  pInputBuffer,               // Input Command Buffer
	                     PUINT16 pOutputLength,     // Length of Output Response
	                     PUINT8  pOutputBuffer)    {          // Output Response Buffer 
int i,num_swaps, index_num, tempnum;
char *temp_pointer;
*pOutputLength = 0;

switch (sub_cmd_num)
        {
        case 0xAA:  // Test Case
          for (i=1;i<=*pInputLength;i++) 
            pOutputBuffer[i-1] = pInputBuffer[i-1] ^ 0xff;
          *pOutputLength = *pInputLength;
          return (0) ; //success
          break;
        case 0xA0:  // Write Block in Cable - Untested
          temp_pointer = (char *) ((((unsigned int ) pInputBuffer[0]) << 8) + pInputBuffer[1]);
          for (i=1;i<=*pInputLength-2;i++) 
            *temp_pointer++ = pInputBuffer[i+1];
          return (0) ; //success
          break;
        case 0xA1:  // Read Block in Cable - Untested
          temp_pointer = (char *) ((((unsigned int ) pInputBuffer[0]) << 8) + pInputBuffer[1]);
          for (i=1;i<=*pInputLength-2;i++) 
            pOutputBuffer[i-1] = *temp_pointer++;
          *pOutputLength = *pInputLength-2;
          return (0) ; //success
          break;
        case 0x00: // Get value of the TDO line
                   // Bit7  TDO
          if (TDO_IN_SET) 
             pOutputBuffer[0] = 0x90;
             else
             pOutputBuffer[0] = 0x00;
          pOutputBuffer[1] = pOutputBuffer[0];
          *pOutputLength=2;
          return (0) ; //success
          break;
        case 0x01: // Set values directly on JTAG Port (1)
                   // Bit0  TDI  
          if (pInputBuffer[0] & 0x01) 
             TDI_OUT_SET();
             else
             TDI_OUT_RESET();
                   // Bit7  TMS
          if (pInputBuffer[0] & 0x80)
             TMS_SET();
             else
             TMS_RESET();
                   // Bit1  TCLK 
          if (pInputBuffer[0] & 0x02)
             TCLK_SET();
             else
             TCLK_RESET();
                   // Bit3  RESET            
          if (pInputBuffer[0] & 0x08)
             tRSTO_DIR = 0;				// RSTO pin is an input
             else {
              tRSTO = 1;					// assert reset_out signal
            	tRSTO_DIR = 1;				// drive the signal out             
             };
          PTED = (PTED & (output_buffer_enable_mask ^ 0xff)) | output_buffer_enable_value;
          return (0) ; //success
          break;
        case 0x02: // Set value directly on JTAG Port (2)
                   // Bit0  JCOMP (Inverted) 
          if (pInputBuffer[0] & 0x01) 
             TRST_RESET();
             else
             TRST_SET();
          PTED = (PTED & (output_buffer_enable_mask ^ 0xff)) | output_buffer_enable_value;
          return (0) ; //success
          break;

        case 0x05: // Swap Bytes Right Justified
                   // 5 Bytes define one 1-16 bit exchange (non compressed)
                   // 3 OR 1 Bytes define on compressed 1-16 bit exchange

          num_swaps = *((PUINT16) pInputBuffer);
          pInputBuffer+=2;
          for (i=0; i<num_swaps; i++) {
             tempnum = *((PUINT8) (pInputBuffer));
             if (tempnum<17) {
                xchng16(tempnum,
                     *((PUINT16) (pInputBuffer+1)), // tdi
                     *((PUINT16) (pInputBuffer+3)), // tms
                     (PUINT16) (pOutputBuffer+i*2)); // tdo
             pInputBuffer+=5;
             } else 
             {
             if (tempnum<tms_only_transaction_compression_start) {
                xchng16(tms_tdi_transaction_compression_array_bitsval[tempnum-tms_tdi_transaction_compression_start], 
                     tms_tdi_transaction_compression_array_tdival[tempnum-tms_tdi_transaction_compression_start],
                     tms_tdi_transaction_compression_array_tmsval[tempnum-tms_tdi_transaction_compression_start],
                     (PUINT16) (pOutputBuffer+i*2)); // tdo                               
                pInputBuffer+=1;
                } else 
                {              
                xchng16(tms_only_transaction_compression_array_bitsval[tempnum-tms_only_transaction_compression_start],
                     *((PUINT16) (pInputBuffer+1)), // tdi
                     tms_only_transaction_compression_array_tmsval[tempnum-tms_only_transaction_compression_start],
                     (PUINT16) (pOutputBuffer+i*2)); // tdo
                pInputBuffer+=3;
                }
                        
             }
                     
          }
          *pOutputLength=num_swaps*2;
          return (0) ; //success
          break;

        case 0x010: // Set compressed byte stream encodings TMS, TDI, Bits into Bits
                    //;     CMD : X DDEEF DDEEF DDEEF DDEEF DDEEF DDEEF DDEEF DDEEF          
                    //;                      X = Array Index Start (0 or 8)
                    //;                      DD = set comp_tdival_w[X+0..7]   
                    //;                      EE = set comp_tmsval_w[X+0..7]  
                    //;                      F = set comp_numbits_b[X+0..7]
                    // 
                    // 
                    
          index_num = *((PUINT8) pInputBuffer); // index into array 0 or 8
          for (i=0; i<8; i++) {
              tms_tdi_transaction_compression_array_tdival[i+index_num] = 
                                                           *((PUINT16) (pInputBuffer+1+i*5)); // tdi
              tms_tdi_transaction_compression_array_tmsval[i+index_num] = 
                                                           *((PUINT16) (pInputBuffer+3+i*5)); // tms
              tms_tdi_transaction_compression_array_bitsval[i+index_num] = 
                                                           *((PUINT8) (pInputBuffer+5+i*5)); // bits
                     
          }
          return (0) ; //success
          break;

        case 0x011: // Set compressed byte stream encodings TMS, Bits into Bits
                    //;     CMD : X DDE DDE DDE DDE DDE DDE DDE DDE
                    //;                      X = Array Index Start (0 or 8)
                    //;                      DD = set comp_tmsval_w[X+0..7]  
                    //;                      E = set comp_numbits_b[X+0..7]
                    // 
                    // 
                    
          index_num = *((PUINT8) pInputBuffer); // index into array 0 or 8
          for (i=0; i<8; i++) {
              tms_only_transaction_compression_array_tmsval[i+index_num] = 
                                                           *((PUINT16) (pInputBuffer+1+i*3)); // tms
              tms_only_transaction_compression_array_bitsval[i+index_num] = 
                                                           *((PUINT8) (pInputBuffer+3+i*3)); // bits
                     
          }
          return (0) ; //success
          break;

          case 0x33: // BDM/serial signal shutdown 
          if (!sci_virtual_serial_port_is_enabled) { 
          OUT_EN = 0; // tristate JTAG signals
          }
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
//	Configure parameters 
//---------------------------------------------------------------------------
int t_config (byte configType, byte configParam, UINT32 paramVal)
{
  return osbdm_error_fail;

}


