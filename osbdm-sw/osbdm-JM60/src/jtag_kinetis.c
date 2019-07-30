/* OSBDM-JM60 Target Interface Software Package for Kinetis
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
#include "derivative.h"		// include peripheral declarations
#include "typedef.h"			// Include peripheral declarations
#include "jtag_kinetis.h"
#include "jtag_jm60.h"    // JTAG declarations for JM60 debug pod
#include "commands.h"			// BDM commands header file
#include "MCU.h"					// MCU header
#include "timer.h"				// timer functions
#include "targetAPI.h"		// target API include file
#include "board_id.h"		  // Board ID
#include "serial_io.h"    // Serial port handling/enable

//---------------------------------------------------------------------------
// Firmware Info
//---------------------------------------------------------------------------

volatile const byte TARGET_TYPE   =  eKinetis;

volatile const byte FIRMWARE_TYPE   =  eEmbeddedGeneric;
   

//---------------------------------------------------------------------------
// Sequence
//---------------------------------------------------------------------------

/**************************************************/
//Local function declarations
void write_jtag_IR_register(UINT16 value);
void write_jtag_DR_register(UINT32 value_L, UINT32 value_H, UINT8 numBits, PUINT32 tdoval_L, PUINT32 tdoval_H);
void write_AP_register(UINT8 AP_select, UINT8 address, UINT32 value);
UINT32 read_AP_register(UINT8 AP_select, UINT8 address);
void  xchng16(unsigned char bitcount, UINT16 tdival,UINT16 tmsval, PUINT16 tdoval);
void write_data_long (UINT32 addr, UINT32 datum);
void write_data_word(UINT32 addr, UINT16 datum);
void write_data_byte(UINT32 addr, UINT16 datum);
UINT32 read_data_long(UINT32 addr);
UINT16 read_data_word(UINT32 addr);
UINT8 read_data_byte(UINT32 addr);
unsigned long remove_ACK_bits_from_35_exchange(UINT32 upper_word, UINT32 lower_word);
void set_reset(void);
void clear_reset(void);

//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
                                               
/**************************************************/


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

  PTEDD = 0xED; 	// set direction

	TMS_RESET();		// set TMS low
	TRST_RESET();
	TCLK_RESET();

	PTBDD |= 0x0C;	// TMS, TCLK signals output
	PTBD &= 0xF3;

  return osbdm_error_ok;
}

void t_assert_ta(word dly_cnt) { return; }

/******************************************************************************
*	t_write_ad
*
*	for addresses 0 - 95 -  Write Core Registers
*
*	for address == 1000  -  Write AP MDM Status Register
*
* for address == 1001  -  Write AP MDM Control Register
*
******************************************************************************/

int t_write_ad(unsigned long addr, unsigned char *data)  
{
   unsigned long reg = 0;
   
   reg = (reg | data[3]) << 8;
   reg = (reg | data[2]) << 8;
   reg = (reg | data[1]) << 8;
   reg = reg | data[0];

   if(addr == 1000) {           // write AP MDM Status register
     write_AP_register(AP_MDM, Reg_MDM_Status_address, reg);
   } 
   else if(addr == 1001) {      // write AP MDM Control register
     write_AP_register(AP_MDM, Reg_MDM_Control_address, reg);
   } 
   else if(addr <= 95) {        // write core registers
      
     if((read_data_long(Reg_CoreDebug_DHCSR_address)&0x00020003)!=0x00020003)
     {
       return osbdm_error_fail; // error --  core running   
     }
     write_data_long(Reg_CoreDebug_DCRDR_address, reg);
     write_data_long(Reg_CoreDebug_DCRSR_address, ((addr & 0x7F) | 0x00010000));
   }
   else {
     return osbdm_error_invalid_parameter; // writing outside range
   }

   return osbdm_error_ok;       // success
}

/******************************************************************************
*	t_write_ad
*
*	for addresses 0 - 95 -  Read Core Registers
*
*	for address == 1000  -  Read AP MDM Status Register
*
* for address == 1001  -  Read AP MDM Control Register
*
******************************************************************************/

int t_read_ad(unsigned long addr, unsigned char *data) 
{ 
   unsigned long reg;
  
   if(addr == 1000) {           // read AP MDM Status register
     reg = read_AP_register(AP_MDM, Reg_MDM_Status_address);
   } 
   else if(addr == 1001) {      // read AP MDM Control register
     reg = read_AP_register(AP_MDM, Reg_MDM_Control_address);
   } 
   else if(addr <= 95) {        // read core registers
   
     if((read_data_long(Reg_CoreDebug_DHCSR_address)&0x00020003)!=0x00020003)
     {
       return osbdm_error_fail; // error --  core running   
     }
     write_data_long(Reg_CoreDebug_DCRSR_address, (addr & 0x7F));
     reg = read_data_long(Reg_CoreDebug_DCRDR_address);
   } 
   else {
     data[0] = 0;               // return 0 in case of error
     data[1] = 0;
     data[2] = 0;
     data[3] = 0;
     return osbdm_error_invalid_parameter; // reading outside range
   }
   
   data[0] =  reg & 0x000000FF;
   data[1] = (reg & 0x0000FF00) >> 8;
   data[2] = (reg & 0x00FF0000) >> 16;
   data[3] = (reg & 0xFF000000) >> 24;
   
   return osbdm_error_ok;     // success
 
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
     unsigned long i;   
     write_data_long(Reg_CoreDebug_DHCSR_address, 0xA05F0003);
     
     for(i = 0; i < 10; i++)
     {
        if((read_data_long(Reg_CoreDebug_DHCSR_address)&0x00020003) == 0x00020003)     // wait for core to halt
        {
           return osbdm_error_ok; // success  -- core halted
        }
     }    
     return osbdm_error_fail;     // timeout 
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
    write_data_long(Reg_CoreDebug_DHCSR_address, 0xA05F0001);   
    return osbdm_error_ok;
}

/******************************************************************************
*	t_step(void)
*
*	single step from current PC
*
*	Return:	0 on success, non-zero on fail
*
******************************************************************************/
byte t_step()
{
     unsigned long DHCSR_value;
     unsigned long i;
     
     DHCSR_value = read_data_long(Reg_CoreDebug_DHCSR_address);
     if ((DHCSR_value & 0x00020000) != 0x00020000)
     {
         return osbdm_error_fail; // cannot single step. Core is not halted
     }

     DHCSR_value = DHCSR_value | 0x00000004;    // Set the C_STEP bit
     write_data_long(Reg_CoreDebug_DHCSR_address, DHCSR_value | 0xA05F0000);
     
     // Clear C_Halt bit
     write_data_long(Reg_CoreDebug_DHCSR_address, (DHCSR_value | 0xA05F0000) & 0xFFFFFFFD);
     
     for(i=0; i < 10; i++){
       if((read_data_long(Reg_CoreDebug_DHCSR_address)&0x00020003) == 0x00020003){
          return osbdm_error_ok; // success 
       }
     }
     
     return osbdm_error_fail;    // timeout     
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
    unsigned long current_buffer_address;
    unsigned long read_value;
    unsigned long num_bytes_already_read;
    unsigned long current_address_to_read;
    unsigned char read_type;
    
    if (count <= 0)
    {
      return osbdm_error_invalid_parameter; // 0 bytes to read
    }
     
    read_type = width / 8;
     
    if  (!((read_type==1)|(read_type==2)|(read_type==4)))
    {
      return osbdm_error_invalid_parameter; // not a supported type of read
    }   

    current_buffer_address = addr;
    current_address_to_read = addr;
    num_bytes_already_read = 0;

    while (num_bytes_already_read < count)
    {
        
        switch (read_type)
        {
        case 1 :
             read_value = read_data_byte(current_address_to_read);
        
             num_bytes_already_read = num_bytes_already_read + read_type;
             current_address_to_read = current_address_to_read + read_type;
       
                  
             *(data+current_buffer_address-addr) = read_value & 0x000000FF;

             current_buffer_address++;
        break;
        case 2 :
             read_value = read_data_word(current_address_to_read);
        
             num_bytes_already_read = num_bytes_already_read + read_type;
             current_address_to_read = current_address_to_read + read_type;
       
             *(data+current_buffer_address-addr) = (read_value & 0x000000FF);
             current_buffer_address++;

             *(data+current_buffer_address-addr) = (read_value & 0x0000FF00) >> 8;
             current_buffer_address++;
        break;
        case 4 :
             read_value = read_data_long(current_address_to_read);
        
             num_bytes_already_read = num_bytes_already_read + read_type;
             current_address_to_read = current_address_to_read + read_type;
       
       
             *(data+current_buffer_address-addr) = (read_value & 0x000000FF);
             current_buffer_address++;

             *(data+current_buffer_address-addr) = (read_value & 0x0000FF00) >> 8;
             current_buffer_address++;

             *(data+current_buffer_address-addr) = (read_value & 0x00FF0000) >> 16;
             current_buffer_address++;

             *(data+current_buffer_address-addr) = (read_value & 0xFF000000) >> 24;
             current_buffer_address++;
        break;
        } //endcase            
    }
    
    return osbdm_error_ok;  //success 
}

/*------------------------------------------------------------------------------
	t_write_mem
	----------------
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
     unsigned long current_buffer_address;
     unsigned long temp_long;
     unsigned long num_bytes_already_written;
     unsigned long temp_val;
     unsigned char write_type;
     
     if (count <= 0)
     {
         return osbdm_error_invalid_parameter;
     }
     
     write_type = width / 8;
     
     if  (!((write_type==1)|(write_type==2)|(write_type==4)))
     {
        return osbdm_error_invalid_parameter; // not a supported type of write
     }
     
     current_buffer_address = addr;
     num_bytes_already_written = 0;     
          
     while (num_bytes_already_written < count)
     {
         
        switch (write_type)
        {
            case 1 :
                 temp_val = 0x000000FF & *(data+current_buffer_address-addr);
                 temp_long = temp_val;
                 current_buffer_address++;
                 
                 write_data_byte(current_buffer_address - 1, temp_long);

                 num_bytes_already_written = num_bytes_already_written + write_type;
                 
            break;
            case 2 :
                 temp_val = 0x000000FF & *(data+current_buffer_address-addr);
                 temp_long = temp_val;
                 current_buffer_address++;
                 temp_val = 0x000000FF & *(data+current_buffer_address-addr);
                 temp_long = temp_long | (temp_val << 8);
                 current_buffer_address++;
                 
                 write_data_word(current_buffer_address - 2, temp_long);

                 num_bytes_already_written = num_bytes_already_written + write_type;
                 
            break;
            case 4 :
                 temp_val = 0x000000FF & *(data+current_buffer_address-addr);
                 temp_long = temp_val;
                 current_buffer_address++;
                 temp_val = 0x000000FF & *(data+current_buffer_address-addr);
                 temp_long = temp_long | (temp_val << 8);
                 current_buffer_address++;
                 temp_val = 0x000000FF & *(data+current_buffer_address-addr);
                 temp_long = temp_long | (temp_val << 16);
                 current_buffer_address++;
                 temp_val = 0x000000FF & *(data+current_buffer_address-addr);
                 temp_long = temp_long | (temp_val << 24);
                 current_buffer_address++;

                 write_data_long(current_buffer_address - 4, temp_long);

                 num_bytes_already_written = num_bytes_already_written + write_type;
            break;
        } //endcase
     }
     
     return osbdm_error_ok;  //success 
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
     //Use only on aligned addreses to ensure the right write type
     
     unsigned long current_buffer_address;
     unsigned char write_type;
     unsigned long num_bytes_already_written;
     unsigned long tmp_data = 0;
     
     if (count <= 0)
     {
         return osbdm_error_invalid_parameter;
     }
     
     write_type = width / 8;
     
     if  (!((write_type==1)|(write_type==2)|(write_type==4)))
     {
        return osbdm_error_invalid_parameter; // not a supported type of write
     }
     
     
     current_buffer_address = addr;
     num_bytes_already_written = 0; 
     
     switch (write_type) 
     {
        case 1 :
          tmp_data = *(data);
        break;
        
        case 2 :
          tmp_data = *(data+1);
          tmp_data = tmp_data << 8;
          tmp_data = tmp_data | *(data);
        break;
        
        case 4 :
          tmp_data = *(data+3);
          tmp_data = tmp_data << 8;
          tmp_data = tmp_data | *(data+2);          
          tmp_data = tmp_data << 8;
          tmp_data = tmp_data | *(data+1);          
          tmp_data = tmp_data << 8;
          tmp_data = tmp_data | *(data);
        break;
      
     }
          
     while (num_bytes_already_written < count)
     {
         
        switch (write_type)
        {
            case 1 :
                 current_buffer_address++;
                 
                 write_data_byte(current_buffer_address - 1, tmp_data);

                 num_bytes_already_written = num_bytes_already_written + write_type;
                 
            break;
            case 2 :
                 current_buffer_address++;
                 current_buffer_address++;
                 
                 write_data_word(current_buffer_address - 2, tmp_data);

                 num_bytes_already_written = num_bytes_already_written + write_type;
                 
            break;
            case 4 :
                 current_buffer_address++;
                 current_buffer_address++;
                 current_buffer_address++;
                 current_buffer_address++;

                 write_data_long(current_buffer_address - 4, tmp_data);

                 num_bytes_already_written = num_bytes_already_written + write_type;
            break;
        } //endcase
     }
     
     return osbdm_error_ok;  //success 
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
  unsigned short tempword=0;
  unsigned long temp=0;
  unsigned long temp2=0;
  unsigned long value_to_write=0;
  unsigned long CTRLSTAT_value;
  
  // mode parameter ignored  
  clear_reset();
  
  //Reset JTAG state machine to Test Logic Reset and go to Run Test Idle
  xchng16(6,0,0x001F,&tempword);
  
  //Request debug mode on
  write_jtag_IR_register(0xA);  
  value_to_write = 0x10000000;
  write_jtag_DR_register((value_to_write<<3)|((Reg_DP_CTRLSTAT_address/4)<<1)|DPACC_APACC_WRITE, 
                         ((value_to_write & 0xE0000000) >> 29),
                         35,
                         &temp,
                         &temp2);
                       
  
  //Read CTRLSTAT to make sure request is acknowledged                      
  value_to_write = 0;
  write_jtag_DR_register((value_to_write<<3)|((Reg_DP_CTRLSTAT_address/4)<<1)|DPACC_APACC_READ, 
                         ((value_to_write & 0xE0000000) >> 29),
                         35,
                         &temp,
                         &temp2);
  write_jtag_DR_register((value_to_write<<3)|((Reg_DP_CTRLSTAT_address/4)<<1)|DPACC_APACC_READ, 
                         ((value_to_write & 0xE0000000) >> 29),
                         35,
                         &temp,
                         &temp2);
            
                         
  CTRLSTAT_value = remove_ACK_bits_from_35_exchange(temp2, temp);
  
  if (CTRLSTAT_value == 0xFFFFFFFF) 
  {
    set_reset();      //Reset high
    return osbdm_error_fail;  
  }
  if ((CTRLSTAT_value & 0x20000000) != 0x20000000) 
  {
    set_reset();      //Reset high
    return osbdm_error_fail;
  }
  
  //Clear sticky errors in CTRLSTAT
  value_to_write = 0x10000032;
  write_jtag_DR_register((value_to_write<<3)|((Reg_DP_CTRLSTAT_address/4)<<1)|DPACC_APACC_WRITE, 
                         ((value_to_write & 0xE0000000) >> 29),
                         35,
                         &temp,
                         &temp2);
                         
  //Configure for 32-bit AHB accesses by default
  write_AP_register(AP_AHB, Reg_AHB_CSW_address, 0x23000042);

  //Reset AP_TAR address to 0
  write_AP_register(AP_AHB, Reg_AHB_TAR_address, 0x0);
  
  //Set "T" bit in PSR
  temp = 0x01000000;
  t_write_ad(Reg_Core_xPSR_index,(char *) &temp);  
  
  write_data_long(Reg_CoreDebug_DHCSR_address, 0xA05F0001);   //CDEBUGEN = 1
  write_data_long(Reg_CoreDebug_DEMCR_address, 0x01000001);   //VC_CORERESET = 1
  
  set_reset();      //Reset high
     
  return osbdm_error_ok;
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
       return osbdm_error_fail;
       
  return osbdm_error_ok;
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
// // Pullup on PTA0 will give an issue with this
// if (TARGET_TYPE != ePPC)
// PTED = (PTED & (output_buffer_enable_mask ^ 0xff)) | (output_buffer_enable_value ^ output_buffer_enable_mask);
}



//--------------------------------------------------------------------------

void clear_reset(void){
     tRSTO = 1;					  
     tRSTO_DIR = 1;				// drive the signal out 
}

void set_reset(void){
     tRSTO = 0;					  
     tRSTO_DIR = 1;				// drive the signal out 
}

UINT8 read_data_byte(UINT32 addr)
{
     unsigned long CSW_value;
     unsigned long value_to_write;
     unsigned long temp=0;
     unsigned long temp2=0;
     unsigned long result;
     unsigned long num_bytes_misaligned;
     
     CSW_value = read_AP_register(AP_AHB, Reg_AHB_CSW_address);
     write_AP_register(AP_AHB, Reg_AHB_CSW_address, (CSW_value & 0xFFFFFFF8)); //Set SIZE to 8-bits
     
     result = 0;
          
     write_AP_register(AP_AHB, Reg_AHB_TAR_address, addr);     
     
     num_bytes_misaligned = addr - (addr & 0xFFFFFFFC);
     switch ( num_bytes_misaligned )
     {
         case 0 :
             result = read_AP_register(AP_AHB, Reg_AHB_DRW_address) & 0x000000FF;
             break;
         case 1 :
             result = (read_AP_register(AP_AHB, Reg_AHB_DRW_address) & 0x0000FF00) >> 8;
             break;
         case 2 :
             result = (read_AP_register(AP_AHB, Reg_AHB_DRW_address) & 0x00FF0000) >> 16;
             break;
         case 3 :
             result = (read_AP_register(AP_AHB, Reg_AHB_DRW_address) & 0xFF000000) >> 24;
             break;
     }
     
     //Clear sticky errors in CTRLSTAT
     value_to_write = 0x10000032;
     write_jtag_IR_register(0xA);
     write_jtag_DR_register((value_to_write<<3)|((Reg_DP_CTRLSTAT_address/4)<<1)|DPACC_APACC_WRITE, 
                         ((value_to_write & 0xE0000000) >> 29),
                         35,
                         &temp,
                         &temp2);       
     
     return result;
}

UINT16 read_data_word(UINT32 addr)
{
     unsigned long CSW_value;
     unsigned long temp=0;
     unsigned long temp2=0;
     unsigned long result;
     unsigned long num_bytes_misaligned;
     
     CSW_value = read_AP_register(AP_AHB, Reg_AHB_CSW_address);
     write_AP_register(AP_AHB, Reg_AHB_CSW_address, (CSW_value & 0xFFFFFFF8) | 0x00000001); //Set SIZE to 16-bits

     result = 0;
          
     write_AP_register(AP_AHB, Reg_AHB_TAR_address, addr);     
     
     num_bytes_misaligned = addr - (addr & 0xFFFFFFFC);
     switch ( num_bytes_misaligned )
     {
         case 0 :
             result = read_AP_register(AP_AHB, Reg_AHB_DRW_address) & 0x0000FFFF;
             break;
         case 2 :
             result = (read_AP_register(AP_AHB, Reg_AHB_DRW_address) & 0xFFFF0000) >> 16;
             break;         
     }

     return result;
}

UINT32 read_data_long(UINT32 addr)
{
     unsigned long CSW_value;
     unsigned long value_to_write;
     unsigned long temp=0;
     unsigned long temp2=0;
     unsigned long result;
     
     CSW_value = read_AP_register(AP_AHB, Reg_AHB_CSW_address);
     write_AP_register(AP_AHB, Reg_AHB_CSW_address, (CSW_value & 0xFFFFFFF8) | 0x00000002); //Set SIZE to 32-bits
     
     result = 0;
     
     write_AP_register(AP_AHB, Reg_AHB_TAR_address, addr);
     result = read_AP_register(AP_AHB, Reg_AHB_DRW_address);
     
     //Clear sticky errors in CTRLSTAT
     value_to_write = 0x10000032;
     write_jtag_IR_register(0xA);
     write_jtag_DR_register((value_to_write<<3)|((Reg_DP_CTRLSTAT_address/4)<<1)|DPACC_APACC_WRITE, 
                         ((value_to_write & 0xE0000000) >> 29),
                         35,
                         &temp,
                         &temp2);     

    return result;
}

void write_data_byte(UINT32 addr, UINT16 datum)
{
     unsigned long CSW_value;
     unsigned long value_to_write;
     unsigned long datum_to_write;
     unsigned long datum_long;
     unsigned long temp=0;
     unsigned long temp2=0;
     
     datum_long = datum & 0x000000FF;
     datum_to_write = (datum_long << 24) | (datum_long << 16) | (datum_long << 8) | datum_long;
     
     CSW_value = read_AP_register(AP_AHB, Reg_AHB_CSW_address);
     write_AP_register(AP_AHB, Reg_AHB_CSW_address, (CSW_value & 0xFFFFFFF8)); //Set SIZE to 8-bits
             
     write_AP_register(AP_AHB, Reg_AHB_TAR_address, addr);
     write_AP_register(AP_AHB, Reg_AHB_DRW_address, datum_to_write);
    
     //Clear sticky errors in CTRLSTAT
     value_to_write = 0x10000032;
     write_jtag_IR_register(0xA);
     write_jtag_DR_register((value_to_write<<3)|((Reg_DP_CTRLSTAT_address/4)<<1)|DPACC_APACC_WRITE, 
                         ((value_to_write & 0xE0000000) >> 29),
                         35,
                         &temp,
                         &temp2);
}

void write_data_word(UINT32 addr, UINT16 datum)
{     
     unsigned long CSW_value;
     unsigned long value_to_write;
     unsigned long datum_to_write;
     unsigned long datum_long;
     unsigned long temp=0;
     unsigned long temp2=0;
     
     datum_long = datum;
     datum_to_write = (datum_long << 16) | datum_long;
     
     CSW_value = read_AP_register(AP_AHB, Reg_AHB_CSW_address);
     write_AP_register(AP_AHB, Reg_AHB_CSW_address, (CSW_value & 0xFFFFFFF8) | 0x00000001); //Set SIZE to 16-bits
             
     write_AP_register(AP_AHB, Reg_AHB_TAR_address, addr);
     write_AP_register(AP_AHB, Reg_AHB_DRW_address, datum_to_write);
    
     //Clear sticky errors in CTRLSTAT
     value_to_write = 0x10000032;
     write_jtag_IR_register(0xA);
     write_jtag_DR_register((value_to_write<<3)|((Reg_DP_CTRLSTAT_address/4)<<1)|DPACC_APACC_WRITE, 
                         ((value_to_write & 0xE0000000) >> 29),
                         35,
                         &temp,
                         &temp2);     
}

void write_data_long (UINT32 addr, UINT32 datum)
{    
     unsigned long CSW_value;
     unsigned long value_to_write;
     unsigned long temp=0;
     unsigned long temp2=0;
     
     CSW_value = read_AP_register(AP_AHB, Reg_AHB_CSW_address);
     write_AP_register(AP_AHB, Reg_AHB_CSW_address, (CSW_value & 0xFFFFFFF8) | 0x00000002); //Set SIZE to 32-bits
    
     write_AP_register(AP_AHB, Reg_AHB_TAR_address, addr);
     write_AP_register(AP_AHB, Reg_AHB_DRW_address, datum);
    
     //Clear sticky errors in CTRLSTAT
     value_to_write = 0x10000032;
     write_jtag_IR_register(0xA);
     write_jtag_DR_register((value_to_write<<3)|((Reg_DP_CTRLSTAT_address/4)<<1)|DPACC_APACC_WRITE, 
                         ((value_to_write & 0xE0000000) >> 29),
                         35,
                         &temp,
                         &temp2);    
}

unsigned long remove_ACK_bits_from_35_exchange(UINT32 upper_word, UINT32 lower_word)
{
     unsigned long result;
     
     result = lower_word >> 3;
     result = result | ((upper_word & 0x0007) << 29);
     
     return result;
}


void write_jtag_IR_register(UINT16 value)
{     
     unsigned short temp=0;     
     
     xchng16(4,0,0x0003,&temp);       //Get to Shift IR
     xchng16(4,value,0x0008,&temp);   
     xchng16(2,0,0x0001,&temp);       //Go back to Run Test Idle     
}

void write_jtag_DR_register(UINT32 value_L, UINT32 value_H, UINT8 numBits, PUINT32 tdoval_L, PUINT32 tdoval_H) 
{
    unsigned short temp=0;
    unsigned short temp2=0;
    unsigned long result=0;
    
    *tdoval_L = 0;
    *tdoval_H = 0;
    
    xchng16(3,0,0x0001,&temp);       //Get to Shift DR
    
    if (numBits <= 16) 
    {
        xchng16(numBits,value_L,1<<(numBits-1), &temp);   
        *tdoval_L = temp >> (16-numBits);        
        xchng16(2,0,0x0001,&temp);                          //Go back to Run Test Idle                 
    } 
    else if (numBits <= 32)
    {
        xchng16(16,(value_L & 0xFFFF),0x0000, &temp);
        xchng16(numBits-16,(value_L >> 16),1 << (numBits-16-1), &temp2);
        temp2 = temp2 >> (32-numBits);
        *tdoval_L = (unsigned long)temp | ((unsigned long)temp2 << 16);                   //Concatenate results  
        xchng16(2,0,0x0001,&temp);                          //Go back to Run Test Idle         
    } 
    else 
    {
        xchng16(16,(value_L & 0xFFFF),0x0000, &temp);
        xchng16(16,(value_L >> 16),0x0000, &temp2);
        *tdoval_L = (unsigned long)temp | ((unsigned long)temp2 << 16);                   //Concatenate results  
        
        xchng16(numBits-32,(value_H & 0xFFFF),1 << (numBits-32-1), &temp);
        *tdoval_H = temp >> (48-numBits);
        xchng16(2,0,0x0001,&temp);                          //Go back to Run Test Idle           
    }        
}

void write_AP_register(UINT8 AP_select, UINT8 address, UINT32 value)
{
     unsigned char APBANKSEL, AP_index;
     unsigned long AP_select_long = 0, APBANKSEL_long = 0;
     unsigned long temp=0;
     unsigned long temp2=0;
     unsigned long value_to_write=0;
     
     APBANKSEL = (address & 0xF0) >> 4;
     AP_index = ((address & 0x0F) / 4) & 0x03;
     
     AP_select_long |= AP_select;
     APBANKSEL_long |= APBANKSEL;
     
     
     write_jtag_IR_register(0x0A);  //Select DPACC
     value_to_write = (AP_select_long << 24) | (APBANKSEL_long << 4);
     write_jtag_DR_register((value_to_write << 3)|((Reg_DP_SELECT_address/4)<<1)|DPACC_APACC_WRITE,
                            (value_to_write & 0xE0000000) >> 29,
                            35,
                            &temp,
                            &temp2);
     
     write_jtag_IR_register(0x0B);  //Select APACC
     value_to_write = value;
     write_jtag_DR_register((value_to_write << 3)|(AP_index<<1)|DPACC_APACC_WRITE,
                            (value_to_write & 0xE0000000) >> 29,
                            35,
                            &temp,
                            &temp2);
}

UINT32 read_AP_register(UINT8 AP_select, UINT8 address)
{
    unsigned long APBANKSEL, AP_index;
    unsigned long result;
    unsigned long temp=0;
    unsigned long temp2=0;
    unsigned long tdoval_H=0;
    unsigned long tdoval_L=0;
    unsigned long temp_AP_select=0; 
    unsigned long value_to_write=0; 
    
    temp_AP_select = (unsigned long)AP_select;
    
    APBANKSEL = (address & 0xF0) >> 4;
    AP_index = ((address & 0x0F) / 4) & (0x03);
    
    write_jtag_IR_register(0x0A);  //Select DPACC
    value_to_write = (temp_AP_select << 24) | (APBANKSEL << 4);
    write_jtag_DR_register((value_to_write << 3)|((Reg_DP_SELECT_address/4)<<1)|DPACC_APACC_WRITE,
                            (value_to_write & 0xE0000000) >> 29,
                            35,
                            &temp,
                            &temp2);
    

    write_jtag_IR_register(0x0B);  //Select APACC
    write_jtag_DR_register((value_to_write << 3)|(AP_index<<1)|DPACC_APACC_READ,
                            (value_to_write & 0xE0000000) >> 29,
                            35,
                            &temp,
                            &temp2);
                            
    write_jtag_DR_register((value_to_write << 3)|(AP_index<<1)|DPACC_APACC_READ,
                            (value_to_write & 0xE0000000) >> 29,
                            35,
                            &temp,
                            &temp2);                           
    
    
    result = remove_ACK_bits_from_35_exchange(temp2, temp);
    
    return result;
}


//--------------------------------------------------------------------------


int t_special_feature(unsigned char sub_cmd_num,   // Special feature number (sub_cmd_num)
	                     PUINT16 pInputLength,       // Length of Input Command
	                     PUINT8  pInputBuffer,       // Input Command Buffer
	                     PUINT16 pOutputLength,      // Length of Output Response
	                     PUINT8  pOutputBuffer)    { // Output Response Buffer 
int i,num_swaps, index_num, tempnum;
char *temp_pointer;
unsigned long delay_long;
unsigned long data_from_cpu2, data_from_cpu;
unsigned long int cycle_start;
unsigned long int cycle_end;
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
            	tRSTO_DIR = 1;			// drive the signal out             
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

          case 0x70: 
          {
          delay_long = ((unsigned int)(pInputBuffer[4])); // * 1000;
          write_AP_register(AP_AHB, Reg_AHB_TAR_address, *((PUINT32) (pInputBuffer)));
          write_AP_register(AP_AHB, Reg_AHB_DRW_address, 0x00);
          
          cycle_start = read_AP_register(AP_AHB, Reg_AHB_DRW_address);
          
          wait_ms(delay_long);
                    
          cycle_end = read_AP_register(AP_AHB, Reg_AHB_DRW_address);

          *pOutputLength = 8;

          pOutputBuffer[0] = ((cycle_end >> 24) & 0xFF);
          pOutputBuffer[1] = ((cycle_end >> 16) & 0xFF);
          pOutputBuffer[2] = ((cycle_end >> 8) & 0xFF);
          pOutputBuffer[3] = ((cycle_end) & 0xFF);
          
          pOutputBuffer[4] = ((cycle_start >> 24) & 0xFF);
          pOutputBuffer[5] = ((cycle_start >> 16) & 0xFF);
          pOutputBuffer[6] = ((cycle_start >> 8) & 0xFF);
          pOutputBuffer[7] = ((cycle_start) & 0xFF);
           
          }
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


