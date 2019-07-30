/* OSBDM-JM60 Windows USB Library
 * Copyright (C) 2009  Freescale
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
/*
* 03/08/2011  : Modified by P&E Microcomputer Systems
*               http://www.pemicro.com/osbdm
*
*
*/

//----------------------------------------------------------------------------
//
//
//  FILE
//
//      osbdm_dll_base.c
// 
//
//  DESCRIPTION
//
//		OSBDM USB driver base dll
//
//
//
//----------------------------------------------------------------------------
//

#ifdef LINUX
#define  __declspec(dllexport)
#else
#include <windows.h>
#endif

#include <time.h>
#include <stdio.h>
#include "log.h"
#include "osbdm_usb.h"
#include "commands.h"
#include "osbdm_base.h"
#include "osbdm_utils.h"

/* define symbol "LOG" during compilation to produce a log file opensourcebdm_dll.log */

unsigned char usb_data[MAX_DATA_SIZE+2];  
unsigned char usb_data2[MAX_DATA_SIZE+2];
 
//
// local functions
//
unsigned char	packet_data_size(unsigned char hsize, unsigned char width, unsigned long size);
OsbdmErrT		osbdm_getreply(unsigned char cmd);
OsbdmErrT		osbdm_send1(unsigned char cmd);
void osbdm_mem_header(unsigned char cmd, unsigned char type, unsigned long addr,
					  unsigned char width, unsigned long len);



//----------------------------------------------------------------------
// OSBDM USB Driver APIs.
//----------------------------------------------------------------------

// returns version of the DLL in BCD format 
__declspec(dllexport) unsigned char osbdm_DLL_VERSION(void) {
	return(OSBDMUSB_VERSION);
}

// USB is initialized in DLLMAIN() (see "osbdm_dll.c": osbdm_usb_init())
// Find the USB devices and returns number of devices found
__declspec(dllexport) unsigned char osbdm_init(void) {
	unsigned char i;

#ifdef LINUX
	osbdm_usb_init();
#endif

	i = osbdm_usb_find_devices();	/* look for devices on all USB busses */
	if (i==255)
	{
		osbdm_usb_init();
		i = osbdm_usb_find_devices();	/* look for devices on all USB busses */
	}
	//print("osbdm_INIT: Usb initialised, found %d device(s)\r\n",i);
	return(i);					/* count the devices found and return the number */
}

/* opens a device with given number (0...) */
__declspec(dllexport) OsbdmErrT osbdm_open(unsigned char device_no) {
	//print("osbdm_OPEN: Trying to open device #%d\r\n",device_no);
	
	if (osbdm_usb_open(device_no))
		return osbdm_error_usb_transport;
	else
		return osbdm_error_ok;
}


/* opens a device with given number (0...) */
__declspec(dllexport) OsbdmErrT osbdm_bootloader_open_device(unsigned long device_no) {
	
	int tempint = bootloader_usb_find_devices();
		
	if ((device_no>=tempint)||(tempint>=20)||(tempint<1))
		return osbdm_error_usb_transport;
	if (bootloader_usb_open(device_no))
		return osbdm_error_usb_transport;
	else
		return osbdm_error_ok;
}

char control_data[8];

__declspec(dllexport) OsbdmErrT osbdm_bootloader_erase_flash_block(unsigned long start_address, unsigned long end_address) {
	
//	must stay within one block of flash

	if (osbdm_control(0x40, 0x82, end_address & 0xffff, start_address & 0xffff, 0, (unsigned char *) &control_data)<0)
		return osbdm_error_usb_transport;
	#ifdef LINUX
        usleep(10000);
        #else
	Sleep(10);
        #endif

	if (osbdm_control(0xC0, 0x8F, 0, 0, 1, (unsigned char *) &control_data)<0)
		return osbdm_error_usb_transport;

	if (control_data[0]!=1)
		return osbdm_error_usb_transport;

	return osbdm_error_ok;


}

__declspec(dllexport) OsbdmErrT osbdm_bootloader_program_flash_block(unsigned long data_address, unsigned long data_length, char *data_array) {
	
	int temppos = 0;
	int amount = 0;
	if (data_length<1)
  	   return osbdm_error_ok;
	do
	{
	if (data_length>=8)
		amount = 8;
	else
		amount = data_length;
	if (osbdm_control(0x40, 0x81, data_address+temppos, data_address+temppos+amount-1, amount, (unsigned char *) &(data_array[temppos]))<0)
		return osbdm_error_usb_transport;
	#ifdef LINUX
        usleep(10000);
        #else
	Sleep(10);
        #endif
	if (osbdm_control(0xC0, 0x8F, 0, 0, 1, (unsigned char *) &control_data)<0)
		return osbdm_error_usb_transport;

	if (control_data[0]!=1)
		return osbdm_error_usb_transport;
	data_length -= amount;
	temppos +=amount;
	} while (data_length>0);

	return osbdm_error_ok;
}

__declspec(dllexport) OsbdmErrT osbdm_bootloader_verify_flash_block(unsigned long data_address, unsigned long data_length, char *data_array) {
	
	int temppos = 0;
	int amount = 0;
	if (data_length<1)
  	   return osbdm_error_ok;
	do
	{
	if (data_length>=8)
		amount = 8;   
	else
		amount = data_length;
	if (osbdm_control(0x40, 0x87, data_address+temppos, data_address+temppos+amount-1, amount, (unsigned char *) &(data_array[temppos]))<0)
		return osbdm_error_usb_transport;
	#ifdef LINUX
        usleep(10000);
        #else
	Sleep(10);
        #endif

	if (osbdm_control(0xC0, 0x8F, 0, 0, 1, (unsigned char *) &control_data)<0)
		return osbdm_error_usb_transport;

	if (control_data[0]!=1)
		return osbdm_error_usb_transport;
	data_length -= amount;
	temppos +=amount;
	} while (data_length>0);

	return osbdm_error_ok;
}







// initialize BDM 
__declspec(dllexport) OsbdmErrT osbdm_init_hardware() {
	usb_data[0] = CMD_BDM_INIT;	// initialize BDM
	if (osbdm_usb_send_ep1(1, usb_data)) // Tx data
		return osbdm_error_usb_transport;

	return osbdm_getreply(CMD_BDM_INIT);	// receive reply from command
}


/* closes currently open device */
__declspec(dllexport)void osbdm_close(void) {
	//print("osbdm_CLOSE: Trying to close the device\r\n");
	osbdm_usb_close();
}

/* gets version of the interface (HW and SW) in BCD format and other info */
__declspec(dllexport) OsbdmErrT  osbdm_get_version(unsigned char *version_info) {

	if(osbdm_send1(CMD_GET_VER)) 	  // send command
		return osbdm_error_usb_transport;	
	if(osbdm_usb_recv_ep2(VERSION_BLOCK_SIZE, version_info)) 	// receive result
		return osbdm_error_usb_transport;

	if(*version_info != CMD_GET_VER) 
		return osbdm_error_cmd_failed;

	return osbdm_error_ok;
}


/* resets the target*/
__declspec(dllexport) OsbdmErrT osbdm_target_reset(ResetT reset_mode) {
	usb_data[0]=CMD_RESET_TARGET;
	usb_data[1]=reset_mode;
	if(osbdm_usb_send_ep1(2, usb_data)) return 1;    // reset target
	return osbdm_getreply(CMD_RESET_TARGET);	// receive reply from command
}

/* fills user supplied structure with actual state of BDM communication channel */
/* returns 0 on success and non-zero on failure */
//__declspec(dllexport) unsigned char osbdm_bdm_sts(bdm_status_t *bdm_status) {
__declspec(dllexport) OsbdmErrT osbdm_status(unsigned char *data){

// NOTE:  this may need to change based on CW needs...

	if(osbdm_send1(CMD_GET_STATUS)) 	  // send command
		return osbdm_error_usb_transport;	
	if(osbdm_usb_recv_ep2(STATUS_BLOCK_SIZE, data)) 	// receive result
		return osbdm_error_usb_transport;

	if(*data != CMD_GET_STATUS) 
		return osbdm_error_cmd_failed;
	return osbdm_error_ok;

#ifdef TO_BE_IMPLIMENTED	// ???????

	usb_data[0]=3;	 /* get 3 bytes */
	usb_data[1]=CMD_GET_STATUS;
	osbdm_usb_recv_ep0(usb_data);
	if (usb_data[0]!=CMD_GET_STATUS) return(1);
	bdm_status->ackn_state = (ackn_state_e)(usb_data[1]&0x01);
	bdm_status->reset_state = (reset_state_e)((usb_data[1]&0x02)>>1);
	bdm_status->connection_state = (connection_state_e)((usb_data[1]&0x18)>>3);
#endif
}

//
//	unit of speed is KHz.
// 
__declspec(dllexport) OsbdmErrT osbdm_get_speed(unsigned long *speed) {
    unsigned long val;
		
	if(osbdm_send1(CMD_GET_SPEED)) return osbdm_error_usb_transport;	
	if(osbdm_usb_recv_ep2(6, usb_data2)) return osbdm_error_usb_transport;	// receive result
	if (usb_data2[0] != CMD_GET_SPEED) return osbdm_error_cmd_failed;

	val = BufToUlong(usb_data2+2);
	if (val == 0){	
		*speed =0;
		return osbdm_error_fail;
	}

	*speed = (unsigned long) ( 60.0 * 2 * 128 *1000.0 /(unsigned long)(val&0xFFFF) );
	return osbdm_error_ok;
}


//	unit of speed is KHz.
__declspec(dllexport) OsbdmErrT osbdm_set_speed(unsigned long speed) {
    unsigned long val;
		
	// Example - to set oscillator at 4Mhz (which is 4000Khz):
	
	// (1 / 4,000) * 256 / (1/60,000) = freq to set in 60Mhz ticks
	// (1 / 4,000) * 15360000 = 3840 (0xF00, which is the value sent from the PC)

	//val = (unsigned long) (1.0 / speed) * 15360.0;  // speed in MHz
	val = (unsigned long) (1.0 / speed * 256 * 60.0 * 1000.0);  // speed in KHz

	usb_data[0] = CMD_SET_SPEED;
	*(unsigned long *) (usb_data+1) =  val;	
	if(osbdm_usb_send_ep1(5, usb_data)) return osbdm_error_usb_transport;	// tx data
	if(osbdm_usb_recv_ep2(2, usb_data2)) return osbdm_error_usb_transport;	// receive result
	if (usb_data2[0] != CMD_SET_SPEED) return osbdm_error_cmd_failed;
	return osbdm_error_ok;
}

// configure paramater 
__declspec(dllexport) 
OsbdmErrT osbdm_config(unsigned char config_type, unsigned char config_param, unsigned long param_value) {

	usb_data[0] = CMD_CONFIG;
	usb_data[1] = config_type;
	usb_data[2] = config_param;
	*(unsigned long *) (usb_data+3) =  param_value;	

	if(osbdm_usb_send_ep1(7, usb_data)) return osbdm_error_usb_transport;	// tx data
	if(osbdm_usb_recv_ep2(2, usb_data2)) {  
		// try once more
		#ifdef LINUX
		usleep(500000);
                #else
		Sleep(500);
                #endif 
		if(osbdm_usb_send_ep1(7, usb_data)) return osbdm_error_usb_transport;	// tx data
		if(osbdm_usb_recv_ep2(2, usb_data2))
			return osbdm_error_usb_transport;	// receive result
	}
	if (usb_data2[0] != CMD_CONFIG) return osbdm_error_cmd_failed;
	return osbdm_error_ok;
}

/* starts target execution from current PC address */
__declspec(dllexport) OsbdmErrT osbdm_target_go(void) {
	if(osbdm_send1(CMD_GO)) return osbdm_error_usb_transport;
	return osbdm_getreply(CMD_GO);	// receive reply from command
}

/* steps over a single target instruction */
__declspec(dllexport) OsbdmErrT osbdm_target_step(void) {
	if(osbdm_send1(CMD_STEP)) return osbdm_error_usb_transport;
	return osbdm_getreply(CMD_STEP);	// receive reply from command
}

/* brings the target into active background mode */
__declspec(dllexport) OsbdmErrT osbdm_target_halt(void) {
	if(osbdm_send1(CMD_HALT)) return osbdm_error_usb_transport;
	return osbdm_getreply(CMD_HALT);	// receive reply from command
}



/*------------------------------------------------------------------------------------
	Write Memory

	type:  kind of memory to be written (see commands.h)

		MEM_RAM			Normal Memory
		MEM_REG			Normal Register
		MEM_CREG		Control Register
		MEM_DREG		Debug Register


	width:	width of writes (8/16/32)
	addr:	address to start writing to
	data:	pointer to the data
	size:   number of BYTES total to be written

*/
__declspec(dllexport) OsbdmErrT osbdm_write_block(unsigned char type, unsigned char width, unsigned long addr, unsigned char *data, unsigned long size) {
	unsigned int i, blkcount;
	unsigned char hsize = MEM_HEADER_SIZE;	// first block header size
	unsigned char count;

	blkcount =0;
	count = packet_data_size(hsize, width, size);	// determine size of this data block
	osbdm_mem_header(CMD_WRITE_MEM, type, addr, width, count);	// prepare usb header for this memory command

	// send all data packets
	for( ; ; ){
		// fill remaining buffer with data
		for (i=0;i<count;i++){
			usb_data[hsize+i] = *(data++);
		}
		if(osbdm_usb_send_ep1(hsize+count, usb_data)) return osbdm_error_usb_transport;	// Tx data
		if(usb_data[0] == CMD_WRITE_MEM) {
		    if(osbdm_getreply(CMD_WRITE_MEM) != osbdm_error_ok) return osbdm_error_usb_transport;
        } else {
		    if(osbdm_getreply(CMD_WRITE_NEXT) != osbdm_error_ok) return osbdm_error_usb_transport;
        }

		size -= count;	// take number sent off count
		if(size < 1)	break;	// stop if all data sent

		hsize = 2;	// new header size for remaining packets
		count = packet_data_size(hsize, width, size);	// determine size of this data block

		usb_data[0] = CMD_WRITE_NEXT;	// use this command for all remaining packets
		usb_data[1] = count;
		blkcount++;
	}
	if (blkcount==0){
		if (usb_data2[0] != CMD_WRITE_MEM) return osbdm_error_cmd_failed;
	}
	else {
		if (usb_data2[0] != CMD_WRITE_NEXT) return osbdm_error_cmd_failed;
	}
	return osbdm_error_ok;
}

 
/*------------------------------------------------------------------------------------
	Fill Memory

	type:  kind of memory to be read (see commands.h)

		MEM_RAM			Normal Memory

	addr:	address to start filling from
	width:	width of Fills (8/16/32)
	data:	pointer to hold data
	count:  number of units to be filled

*/
 __declspec(dllexport) OsbdmErrT osbdm_write_fill(unsigned char type, unsigned char width, unsigned long addr, unsigned char *data, unsigned long count){
 	osbdm_mem_header(CMD_FILL, type, addr, width, count);	// prepare usb header for this memory command
 	switch (width) {
        case 8:
            *(usb_data+MEM_HEADER_SIZE) = data[0];
            break;
        case 16:
            *(usb_data+MEM_HEADER_SIZE)   = data[0];
            *(usb_data+MEM_HEADER_SIZE+1) = data[1];
            break;
        case 32:
        default:
            *(usb_data+MEM_HEADER_SIZE)   = data[0];
            *(usb_data+MEM_HEADER_SIZE+1) = data[1];
            *(usb_data+MEM_HEADER_SIZE+2) = data[2];
            *(usb_data+MEM_HEADER_SIZE+3) = data[3];
            break;
    }

	if(osbdm_usb_send_ep1(MEM_HEADER_SIZE+(width/8), usb_data)) return osbdm_error_usb_transport;	// Tx data
	return osbdm_getreply(CMD_FILL);	// receive reply from command
}


/*------------------------------------------------------------------------------------
	Read Memory

	type:  kind of memory to be read (see commands.h)

		MEM_RAM			Normal Memory
		MEM_REG			Normal Register
		MEM_CREG		Control Register
		MEM_DREG		Debug Register

	addr:	address to start reading from
	width:	width of Reads (8/16/32)
	data:	pointer to hold data
	size:   number of BYTES total to be read

*/
__declspec(dllexport) OsbdmErrT osbdm_read_block(unsigned char type, unsigned char width, unsigned long addr, unsigned char *data, unsigned long size){

	unsigned int i, blkcount;
	unsigned char hsize = MEM_HEADER_SIZE;	// first block header size
	unsigned char count;

	blkcount =0;
	count = packet_data_size(hsize, width, size);	// determine size of this data block
	osbdm_mem_header(CMD_READ_MEM, type, addr, width, count);	// prepare usb header for this memory command

	// receive all data packets
	for( ; ; ){
		if(osbdm_usb_send_ep1(hsize, usb_data)) return osbdm_error_usb_transport;	// Tx data
		if(osbdm_usb_recv_ep2(count+2, usb_data2)) return osbdm_error_usb_transport;	// get data back

		// copy data so we don't send back header info
		for(i=0; i<count; i++){
			*data++ = usb_data2[i+2];
		}

		size -= count;	// take number received off total count
		if(size < 1)	break;	// stop if all data received

		hsize = 2;	// new header size for remaining packets
		count = packet_data_size(hsize, width, size);	// determine size of this data block

		usb_data[0] = CMD_READ_NEXT;	// use this command for all remaining packets
		usb_data[1] = count;
		blkcount++;
	}
	if (blkcount==0){
		if (usb_data2[0] != CMD_READ_MEM) return osbdm_error_cmd_failed;
	}
	else {
		if (usb_data2[0] != CMD_READ_NEXT) return osbdm_error_cmd_failed;
	}
	return osbdm_error_ok;
}


 
// write a single 32-bit value to target memory
 __declspec(dllexport) OsbdmErrT	osbdm_write_32(unsigned char type, unsigned long address, unsigned long data){
 	osbdm_mem_header(CMD_WRITE_MEM, type, address, 32, 4);	// prepare usb header for this memory command
	*(unsigned long *) (usb_data+MEM_HEADER_SIZE) = data;	// data to write

	if(osbdm_usb_send_ep1(MEM_HEADER_SIZE+4, usb_data)) return osbdm_error_usb_transport;	// Tx data
	if(osbdm_usb_recv_ep2(2, usb_data2)) return osbdm_error_usb_transport;	// receive result

	if (usb_data2[0] != CMD_WRITE_MEM) return osbdm_error_cmd_failed;
	return osbdm_error_ok;
}

// write a single 16-bit value to target memory
 __declspec(dllexport) OsbdmErrT	osbdm_write_16(unsigned char type, unsigned long address, unsigned short data){
  	osbdm_mem_header(CMD_WRITE_MEM, type, address, 16, 2);	// prepare usb header for this memory command
	*(unsigned short *) (usb_data+MEM_HEADER_SIZE) = data;	// data to write

	if(osbdm_usb_send_ep1(MEM_HEADER_SIZE+2, usb_data)) return osbdm_error_usb_transport;	// Tx data
	if(osbdm_usb_recv_ep2(2, usb_data2)) return osbdm_error_usb_transport;	// receive result

	if (usb_data2[0] != CMD_WRITE_MEM) return osbdm_error_cmd_failed;
	return osbdm_error_ok;	
}

// write a single 8-bit value to target memory
 __declspec(dllexport) OsbdmErrT	osbdm_write_8(unsigned char type, unsigned long address, unsigned char data){
 	osbdm_mem_header(CMD_WRITE_MEM, type, address, 8, 1);	// prepare usb header for this memory command
	*(unsigned char *) (usb_data+MEM_HEADER_SIZE) = data;	// data to write

	if(osbdm_usb_send_ep1(MEM_HEADER_SIZE+1, usb_data)) return osbdm_error_usb_transport;	// Tx data
	if(osbdm_usb_recv_ep2(2, usb_data2)) return osbdm_error_usb_transport;  // receive result

	if (usb_data2[0] != CMD_WRITE_MEM) return osbdm_error_cmd_failed;
	return osbdm_error_ok;	
}

 
// read a single 32-bit value from target memory/register
 __declspec(dllexport) OsbdmErrT	osbdm_read_32(unsigned char type, unsigned long address, unsigned long *data){
	osbdm_mem_header(CMD_READ_MEM, type, address, 32, 4);	// prepare usb header for this memory command

	if(osbdm_usb_send_ep1(MEM_HEADER_SIZE, usb_data)) return osbdm_error_usb_transport;	// Tx data
	if(osbdm_usb_recv_ep2(6, usb_data2)) return osbdm_error_usb_transport;	// receive result

	*data = (*(unsigned long *) (usb_data2+2));	// return received data
	if (usb_data2[0] != CMD_READ_MEM) return osbdm_error_cmd_failed;
	return osbdm_error_ok;	
}

// read a single 16-bit value to target memory
__declspec(dllexport) OsbdmErrT osbdm_read_16(unsigned char type, unsigned long address, unsigned short *data){
	osbdm_mem_header(CMD_READ_MEM, type, address, 16, 2);	// prepare usb header for this memory command

	if(osbdm_usb_send_ep1(MEM_HEADER_SIZE, usb_data)) return osbdm_error_usb_transport;	// Tx data
	if(osbdm_usb_recv_ep2(4, usb_data2)) return osbdm_error_usb_transport;	// receive result

	*data =  (*(unsigned short *) (usb_data2+2));	// return received data
	if (usb_data2[0] != CMD_READ_MEM) return osbdm_error_cmd_failed;
	return osbdm_error_ok;	

}
// read a single 8-bit value to target memory
__declspec(dllexport) OsbdmErrT	osbdm_read_8(unsigned char type, unsigned long address, unsigned char *data){
	osbdm_mem_header(CMD_READ_MEM, type, address, 8, 1);	// prepare usb header for this memory command
	if(osbdm_usb_send_ep1(MEM_HEADER_SIZE, usb_data)) return osbdm_error_usb_transport;	// Tx data
	if(osbdm_usb_recv_ep2(3, usb_data2)) return osbdm_error_usb_transport;	// receive result

	*data = (*(unsigned char *) (usb_data2+2));	// return received data
	if (usb_data2[0] != CMD_READ_MEM) return osbdm_error_cmd_failed;
	return osbdm_error_ok;	
}

/***************************************/
/* RS08 Specific Flash Programming API */
/***************************************/


/*-------------------------------------------------------------------------------------------------------------------
 Below is the summary of the steps that the debugger and run control GDI DLL should perform to program flash:

1) CW debugger starts the FP task by calling opensourcebdm_flash_dlstart()
2) CW debugger writes the FP algorithm into target RAM
3) CW debugger calls opensourcebdm_flash_prog() with address of FP algo, data to be written and size of data as arguments.
4) JM60 firmware:
	- writes data to fast access ram starting ad address 0
	- enables programming voltage to the target
	- executes the FP algo on the target

5) FP algorithm does the operations for writing the 8 bytes of data from page 0
         - set PGM
         - writes something to the address in X to select the flash page (handles PAGESEL)
         - set HVEN
         - writes data from page 0 to the address in X
         - clear PGM
         - clear HVEN
         - put the target on BDM

6) JM60 firmware polls for BDM - and disables programming voltage when no longer running
		- a safety timeout removes programming voltage automatically if BDM mode never entered, returns TIMEOUT status

7) CW debugger polls for the status of the FP algorithm by calling opensourcebdm_bdm_sts()
		- value returned in (bdm_status->flash_stat) as: ((DONE = 0, RUNNING = 1, TIMEOUT = 2)

8) CW debugger repeats from step 3 for the next 8 bytes until the whole FP task completes

9) CW debugger ends the FP task by calling opensourcebdm_flash_dlend()

-----------------------------------------------------------------------------------------------------------*/

__declspec(dllexport) OsbdmErrT osbdm_flash_dlstart(void){
	//print("CMD_FLASH_DLSTART...... \r\n");
	usb_data[0] = CMD_FLASH_DLSTART;	// Start Flash programming sequence
	if(osbdm_usb_send_ep1(1, usb_data)) return osbdm_error_usb_transport;		// Tx data
	return osbdm_getreply(CMD_FLASH_DLSTART);	// receive reply from command
}

// this will turn OFF Flash Programming voltage
__declspec(dllexport) OsbdmErrT osbdm_flash_dlend(void) {
	//print("CMD_FLASH_DLEND......  \r\n");
	usb_data[0] = CMD_FLASH_DLEND;	// End Flash programming sequence
	if(osbdm_usb_send_ep1(1, usb_data)) return osbdm_error_usb_transport;	// Tx data
	return osbdm_getreply(CMD_FLASH_DLEND);	// receive reply from command
}

// This will cause the bdm firmware to enable programming voltage to the chip and then 
// execute the flash programming algorithm at address: (faddr) with (size) bytes of data at (*data)
// NOTE:  Before calling this the debugger must have already loaded the flash algo into target ram at (faddr).
__declspec(dllexport) OsbdmErrT osbdm_flash_prog(unsigned int faddr, unsigned char size, unsigned char *data){

	unsigned char *ptr;
	unsigned char i;

	//print("CMD_FLASH_PROG...... \r\n");
	usb_data[0] = CMD_FLASH_PROG;	// Execute flash programming algo
	usb_data[1] = size;	// number of data bytes to program
	usb_data[2] = (unsigned char) (faddr & 0xFF);		// LSB Flash programming algo address
	usb_data[3] = (unsigned char) (faddr >>= 8) & 0xFF;	// MSB Flash programming algo address

	// load the data to be programmed into usb transmit buffer
	i=size;
	for(ptr = usb_data+4; i > 0; i--){
		*ptr++ = *data++;
	}
	if(osbdm_usb_send_ep1(4+size, usb_data)) return osbdm_error_usb_transport;	// Tx data
	return osbdm_getreply(CMD_FLASH_PROG);	// receive reply from command
}







/************************************/
/* To-Be-Deprecated			        */
/************************************/

// reads from the BDM area of memory 
// returns value read 
__declspec(dllexport) unsigned long osbdm_read_bd(unsigned int address) {
	osbdm_read_block(MEM_DREG, 32, address, usb_data2, 4);	// read 1, 32-bit value
	return(BufToUlong(usb_data2));
}

// writes to the BDM area of memory
// returns 0 on success and non-zero on failure
__declspec(dllexport) unsigned char osbdm_write_bd(unsigned int address, unsigned long data) {
	return osbdm_write32(MEM_DREG, address, data);	// write debug register
}

// write a single 32-bit value to target memory
 __declspec(dllexport) unsigned char osbdm_write32(unsigned char type, unsigned long address, unsigned long data){
	osbdm_mem_header(CMD_WRITE_MEM, type, address, 32, 4);	// prepare usb header for this memory command
	*(unsigned long *) (usb_data+MEM_HEADER_SIZE) = data;	// data to write
	if(osbdm_usb_send_ep1(MEM_HEADER_SIZE+4, usb_data)) return 1;	// Tx data
	if(osbdm_usb_recv_ep2(2, usb_data2)) return 1;	// receive result
	return 0;
}
// write a single 16-bit value to target memory
 __declspec(dllexport) unsigned char osbdm_write16(unsigned char type, unsigned long address, unsigned short data){
 	osbdm_mem_header(CMD_WRITE_MEM, type, address, 16, 2);	// prepare usb header for this memory command
	*(unsigned short *) (usb_data+MEM_HEADER_SIZE) = data;	// data to write
	if(osbdm_usb_send_ep1(MEM_HEADER_SIZE+2, usb_data)) return 1;	// Tx data
	if(osbdm_usb_recv_ep2(2, usb_data2)) return 1;	// receive result
	return 0;
}
// write a single 8-bit value to target memory
 __declspec(dllexport) unsigned char osbdm_write8(unsigned char type, unsigned long address, unsigned char data){
 	osbdm_mem_header(CMD_WRITE_MEM, type, address, 8, 1);	// prepare usb header for this memory command
	*(unsigned char *) (usb_data+MEM_HEADER_SIZE) = data;	// data to write
	if(osbdm_usb_send_ep1(MEM_HEADER_SIZE+1, usb_data)) return 1;	// Tx data
	if(osbdm_usb_recv_ep2(2, usb_data2)) return 1;	// receive result
	return 0;
}

// read a single 32-bit value to target memory
__declspec(dllexport) unsigned long osbdm_read32(unsigned char type, unsigned long address){
	osbdm_mem_header(CMD_READ_MEM, type, address, 32, 4);	// prepare usb header for this memory command
	if(osbdm_usb_send_ep1(MEM_HEADER_SIZE, usb_data)) return 0xDEAD;	// Tx data
	if(osbdm_usb_recv_ep2(6, usb_data2)) return 0xDEAD;	// receive result
	return (*(unsigned long *) (usb_data2+2));	// return received data
}
// read a single 16-bit value to target memory
__declspec(dllexport) unsigned short osbdm_read16(unsigned char type, unsigned long address){
	osbdm_mem_header(CMD_READ_MEM, type, address, 16, 2);	// prepare usb header for this memory command
	if(osbdm_usb_send_ep1(MEM_HEADER_SIZE, usb_data)) return 0xDEAD;	// Tx data
	if(osbdm_usb_recv_ep2(4, usb_data2)) return 0xDEAD;	// receive result
	return (*(unsigned short *) (usb_data2+2));	// return received data
}
// read a single 8-bit value to target memory
__declspec(dllexport) unsigned char osbdm_read8(unsigned char type, unsigned long address){
	osbdm_mem_header(CMD_READ_MEM, type, address, 8, 1);	// prepare usb header for this memory command
	if(osbdm_usb_send_ep1(MEM_HEADER_SIZE, usb_data)) return 0xDE;	// Tx data
	if(osbdm_usb_recv_ep2(3, usb_data2)) return 0xDE;	// receive result
	return (*(unsigned char *) (usb_data2+2));	// return received data
}


//---------------------------------------------------------------
// local functions
//---------------------------------------------------------------


// standard wait for reply routine for single byte commands
OsbdmErrT osbdm_getreply(unsigned char cmd){
	int i;
	for(i=0; i < 10; i++){
		if(osbdm_usb_recv_ep2(2, usb_data2) == 0){	// got a good reply
			if(usb_data2[0] != cmd) return osbdm_error_cmd_failed;	// error if wrong reply
			return osbdm_error_ok;	// good
		}
	}
	return osbdm_error_usb_transport;
}


// send a single byte out ep1
OsbdmErrT osbdm_send1(unsigned char cmd){
	usb_data[0]=cmd;
	usb_data[1]=0;	// break on reset
//	usb_data[1]=1;	// NO break on reset

	if (osbdm_usb_send_ep1(1, usb_data))
		return osbdm_error_usb_transport;
	else 
		return osbdm_error_ok;
}


 /*------------------------------------------------------------------------------------
  packet_data_size
  ----------------
	determine the number of bytes to send for a packet based on:
		header size, data width, number of data bytes left to send
*/
unsigned char packet_data_size(unsigned char hsize, unsigned char width, unsigned long size){
	unsigned char count, r, d;

	count = (MAX_DATA_SIZE - hsize);	// maximum data size possible

	// if total left to send is less than maximum, that's how many we'll send
	if(size < count){
		return (unsigned char) size;
	}
	// if 8-bit writes then send the maximum as is
	if(width <= 8)	return count;

	// otherwise we have to take width into account so we don't send an odd number on 16 or 32 bit writes
	d = width / 8;	// get divisor
	r = count % d;	// get remainder

	if(r)	return count - r;	// subtract remainder if not evenly divisible by width
	return count;				// or it's even so just return count
}

/*------------------------------------------------------------------------------------
  osbdm_mem_header
  ----------------
  Prepare usb header for a memory command
	cmd = 8-bit Read or Write
	type = 8-bit Memory Type
	addr = 32-bit address
	width = 8-bit Bit Width (8/16/32)
	len = 32-bit length (number to read or write)
*/
void osbdm_mem_header(unsigned char cmd, unsigned char type, unsigned long addr,
					  unsigned char width, unsigned long len){

	usb_data[0] = cmd;
	usb_data[1] = type;
	usb_data[2] = (unsigned char) (addr & 0xFF);		// LSB
	usb_data[3] = (unsigned char) (addr >>= 8) & 0xFF;
	usb_data[4] = (unsigned char) (addr >>= 8) & 0xFF;
	usb_data[5] = (unsigned char) (addr >>= 8);			// MSB
	usb_data[6] = width;
	usb_data[7] = (unsigned char) (len & 0xFF);			// LSB
	usb_data[8] = (unsigned char) (len >>= 8) & 0xFF;
	usb_data[9] = (unsigned char) (len >>= 8) & 0xFF;
	usb_data[10] =(unsigned char) (len >>= 8);			// MSB
	// note:  change MEM_HEADER_SIZE if you add to this
}


