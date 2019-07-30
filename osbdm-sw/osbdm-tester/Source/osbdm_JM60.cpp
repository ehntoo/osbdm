/*---------------------------------------------------------------------------
	Interface to Axiom JM60 OSBDM functions

---------------------------------------------------------------------------*/

// Define constants for which version you're building for
// --- this is defined in borland Project Options / Directories and Conditionals
// #define HCS0812

#include <stdlib.h>

#include "datatypes.h"

extern "C" {
#include "osbdm_usb.h"
#include "commands.h"
#include "auth.h"
}
#include "osbdm_JM60.h"


unsigned char usb_data[MAX_DATA_SIZE+2];
unsigned char usb_data2[MAX_DATA_SIZE+2];




int osbdm_getreply(unsigned char cmd){
	for(int i=0; i< 4; i++){
		if(osbdm_usb_recv_ep2(2, usb_data2) == 0){	// got a good reply
			if(usb_data2[0] != cmd) return 1;	// error if wrong reply
			return 0;	// good
		}
	}
	return 1;
}

/*------------------------------------------------------------------------------------
	osbdm_connnect
	--------------
	initialises USB interface and search for an osbdm device

	returns:
		if successful, the number of osbdm devices found
		-1 if there's a problem with the usb driver
 */
int osbdm_connect() {
	int i;
	osbdm_usb_init();			// initialise USB
	i=osbdm_usb_find_devices();	// look for devices on all USB busses
	return(i);
}
/* opens a device with given number (0...) */
/* returns 0 on success and 1 on error */
int osbdm_open(unsigned char device_no) {
	return(osbdm_usb_open(device_no));
}
int osbdm_init() {
	usb_data[0] = CMD_BDM_INIT;	// initialize BDM
	if(osbdm_usb_send_ep1(1, usb_data)) return 1;	// Tx data
	return osbdm_getreply(CMD_BDM_INIT);	// receive reply from command
}

/* closes currently open device */
void osbdm_close() {
	osbdm_usb_close();
}

// send a single byte out ep1
int osbdm_send1(unsigned char cmd){
	usb_data[0]=cmd;
	usb_data[1]=0;	// break on reset
//	usb_data[1]=1;	// NO break on reset
	return(osbdm_usb_send_ep1(1, usb_data));
}
// send count bytes from usb_data[] to ep1
int osbdm_send(unsigned char count){
	return(osbdm_usb_send_ep1(count, usb_data));
}

// get bdm/target status information
int osbdm_status(char *data){
	if(osbdm_send1(CMD_GET_STATUS)) return 1;		// send command
	if(osbdm_usb_recv_ep2(STATUS_BLOCK_SIZE, data)) return 1;	// receive result
	return 0;
}

// Reset Target
// 	return 0 on success
int osbdm_target_reset(unsigned char mode){
/*	unsigned char rval;
	rval = tblcf_target_reset(BDM_MODE);
	ExecuteResetScript();	// execute target initialization script
	return rval;
*/
//	if(osbdm_send1(CMD_ENABLE_BDM_VOLTAGE)) return 1;	// enable bdm voltage
//	if(osbdm_send1(CMD_ENABLE_TRG_VOLTAGE)) return 1;	// enable target voltage

	usb_data[0]=CMD_RESET_TARGET;
	usb_data[1]=mode;
	if(osbdm_usb_send_ep1(2, usb_data)) return 1;    // reset target
	return osbdm_getreply(CMD_RESET_TARGET);	// receive reply from command
}
int osbdm_target_halt(){
	if(osbdm_send1(CMD_HALT)) return 1;
	return osbdm_getreply(CMD_HALT);	// receive reply from command
}
int osbdm_target_go(){
	if(osbdm_send1(CMD_GO)) return 1;
	return osbdm_getreply(CMD_GO);	// receive reply from command
}
int osbdm_target_step(){
	if(osbdm_send1(CMD_STEP)) return 1;
	return osbdm_getreply(CMD_STEP);	// receive reply from command
}

/*
  Set Target BDM speed.  Speed is in MHz (floating point decimal)

	Example - to set oscillator at 4Mhz:

	(1 / 4,000,000) * 256 / (1/60,000,000) = freq to set in 60Mhz ticks
	(1 / 4,000,000) * 15360000000 = 3840 (0xF00, which is the value sent from the PC)
*/
int osbdm_target_set_speed(float speed){

	float val = 1 / speed * (float) 15360;

	usb_data[0]=CMD_SET_SPEED;
	*(unsigned long *) (usb_data+1) = (unsigned long) val;
	if(osbdm_usb_send_ep1(5, usb_data)) return 1;    // set target clock speed
	return osbdm_getreply(CMD_SET_SPEED);	// receive reply from command

}

int osbdm_target_get_speed(unsigned char *data){
	if(osbdm_send1(CMD_GET_SPEED)) return 1;
	if(osbdm_usb_recv_ep2(6, data)) return 1;	// receive result
}


int osbdm_flash_unsecure(unsigned char unsec_instruction, unsigned char unsec_instruction_len,
						unsigned char flash_clock_divisor){

/*
		i=osbdm_open(0);		// open first device found
		if(i>0) return 0x200 + i;
		if(CL_InitBDM){	// if not overridden by command line
			if(osbdm_init())	return 0x300;	// initialize bdm
		}

	tblcf_open(device_no);
	tblcf_set_target_type(JTAG);

*/

	usb_data[0]=CMD_JTAG_UNSECURE;
	usb_data[1]=unsec_instruction;		// unsecure command
	usb_data[2]=unsec_instruction_len;	// number of bits in the command
	usb_data[3]=flash_clock_divisor;	// clock divider

	if(osbdm_usb_send_ep1(4, usb_data)) return 1;    // send commad
	return osbdm_getreply(CMD_JTAG_UNSECURE);	// receive reply from command
}


/*------------------------------------------------------------------------------------
  osbdm_mem_header
  ----------------
  Prepare usb header for a memory command
	cmd = 8-bit Read or Write
	type = 8-bit Memory Type
	addr = 32-bit address
	width = 8-bit Bit Width (8/16/32)
	len = 32-bit length (number of BYTES to read or write)
*/
void osbdm_mem_header(unsigned char cmd, unsigned char type, unsigned long addr,
					  unsigned char width, unsigned long len){

	usb_data[0] = cmd;
	usb_data[1] = type;
	usb_data[2] = (addr & 0xFF);		// LSB
	usb_data[3] = (addr >>= 8) & 0xFF;
	usb_data[4] = (addr >>= 8) & 0xFF;
	usb_data[5] = (addr >>= 8);			// MSB
	usb_data[6] = width;
	usb_data[7] = (len & 0xFF);			// LSB
	usb_data[8] = (len >>= 8) & 0xFF;
	usb_data[9] = (len >>= 8) & 0xFF;
	usb_data[10] =(len >>= 8);			// MSB
	// note:  change MEM_HEADER_SIZE if you add to this
}

// write a single 8-bit value to target memory
int osbdm_write8(unsigned char type, unsigned long address, unsigned char data){
	osbdm_mem_header(CMD_WRITE_MEM, type, address, 8, 1);	// prepare usb header for this memory command
	usb_data[MEM_HEADER_SIZE] = data;	// data to write
	if(osbdm_usb_send_ep1(MEM_HEADER_SIZE+1, usb_data)) return 1;	// Tx data
	if(osbdm_usb_recv_ep2(2, usb_data2)) return 1;	// receive result
	return 0;
}


// write a single 32-bit value to target memory
int osbdm_write32(unsigned char type, unsigned long address, unsigned long data){

	osbdm_mem_header(CMD_WRITE_MEM, type, address, 32, 4);	// prepare usb header for this memory command
	*(unsigned long *) (usb_data+MEM_HEADER_SIZE) = data;	// data to write
	if(osbdm_usb_send_ep1(MEM_HEADER_SIZE+4, usb_data)) return 1;	// Tx data
	if(osbdm_usb_recv_ep2(2, usb_data2)) return 1;	// receive result
	return 0;
}


/* fills block to memory

	count = number of BTYES, we're sending the bdm the number of units
	returns 0 on success and 1 on error
*/
int osbdm_write_fill(unsigned char type, unsigned char width, unsigned long addr, unsigned char *data, unsigned long count) {
	osbdm_mem_header(CMD_FILL, type, addr, width, count);	// prepare usb header for this memory command

	if(osbdm_usb_send_ep1(MEM_HEADER_SIZE+count, data)) return 1;	// Tx data
	if(osbdm_usb_recv_ep2(2, usb_data2)) return 1;
	return 0;
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
	Write Memory

	type:  kind of memory to be written (see commands.h)

		MEM_RAM			Normal Memory
		MEM_REG			Normal Register
		MEM_CREG		Control Register
		MEM_DREG		Debug Register


	width:	width of writes (8/16/32)
	addr:	address to start writing to
	data:	pointer to the data
	size:   number of BTYES total to be written

	returns 0 on success and 1 on error
*/
int osbdm_write_block(unsigned char type, unsigned char width, unsigned long addr, unsigned char *data, unsigned long size) {
	unsigned int i;
	unsigned char hsize = MEM_HEADER_SIZE;	// first block header size
	unsigned char count;

	count = packet_data_size(hsize, width, size);	// determine size of this data block
	osbdm_mem_header(CMD_WRITE_MEM, type, addr, width, count);	// prepare usb header for this memory command

	// send all data packets
	for( ; ; ){

		// fill remaining buffer with data
		for (i=0;i<count;i++){
			usb_data[hsize+i] = *(data++);
		}
		if(osbdm_usb_send_ep1(hsize+count, usb_data)) return 1;	// Tx data
		if(osbdm_usb_recv_ep2(2, usb_data2)) return 1;

		size -= count;	// take number sent off count
		if(size < 1)	break;	// stop if all data sent

		hsize = 2;	// new header size for remaining packets
		count = packet_data_size(hsize, width, size);	// determine size of this data block

		usb_data[0] = CMD_WRITE_NEXT;	// use this command for all remaining packets
		usb_data[1] = count;
	}
	return 0;

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
	size:   number of BTYES total to be read

	returns 0 on success and 1 on error
*/
int osbdm_read_block(unsigned char type, unsigned char width, unsigned long addr, unsigned char *data, unsigned long size){

	unsigned int i;
	unsigned char hsize = MEM_HEADER_SIZE;	// first block header size
	unsigned char count;

	count = packet_data_size(hsize, width, size);	// determine size of this data block
	osbdm_mem_header(CMD_READ_MEM, type, addr, width, count);	// prepare usb header for this memory command

	// receive all data packets
	for( ; ; ){
		if(osbdm_usb_send_ep1(hsize, usb_data)) return 1;	// Tx data
		if(osbdm_usb_recv_ep2(count+2, usb_data2)) return 1;	// get data back

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
	}
	return 0;
}

// read a single 32-bit value to target memory
unsigned long osbdm_read32(unsigned char type, unsigned long address){
	osbdm_mem_header(CMD_READ_MEM, type, address, 32, 4);	// prepare usb header for this memory command
	if(osbdm_usb_send_ep1(MEM_HEADER_SIZE, usb_data)) return 0xDEAD;	// Tx data
	if(osbdm_usb_recv_ep2(6, usb_data2)) return 0xDEAD;	// receive result
	return (*(unsigned long *) (usb_data2+2));	// return received data
}

// read data from the serial port
// returns data[] with command in 0, count in 1, data in the rest
int osbdm_sci_read(unsigned char *data){
	usb_data[0] = CMD_SCI_READ;	// read serial data
	if(osbdm_usb_send_ep1(1, usb_data)) return 1;	// Tx data
	if(osbdm_usb_recv_ep2(MAX_DATA_SIZE, data)) return 1;	// receive result
	if(data[0] != CMD_SCI_READ) return 1;	// error if wrong reply
	return 0;
}
// send data to the serial port
int osbdm_sci_write(char count, char *data){
	usb_data[0] = CMD_SCI_WRITE;	// write serial data command
	usb_data[1] = count;			// data count
	for(char i=0; i<count; i++){	// copy data to buffer
		usb_data[i+2] = *data++;
	}
	if(osbdm_usb_send_ep1(count+2, usb_data)) return 1;	// Tx data
	return osbdm_getreply(CMD_SCI_WRITE);	// receive reply from command
}
int osbdm_sci_write1(unsigned char data){
	usb_data[0] = CMD_SCI_WRITE;	// write serial data command
	usb_data[1] = 1;			// data count
	usb_data[2] = data;
	if(osbdm_usb_send_ep1(3, usb_data)) return 1;	// Tx data
	return osbdm_getreply(CMD_SCI_WRITE);	// receive reply from command
}

// RS08 Flash programming commands
int osbdm_flash_dlstart(void){
	usb_data[0] = CMD_FLASH_DLSTART;	// Start Flash programming sequence
	if(osbdm_usb_send_ep1(1, usb_data)) return 1;		// Tx data
	return osbdm_getreply(CMD_FLASH_DLSTART);	// receive reply from command
}

// this will turn OFF Flash Programming voltage
int osbdm_flash_dlend(void) {
	usb_data[0] = CMD_FLASH_DLEND;	// End Flash programming sequence
	if(osbdm_usb_send_ep1(1, usb_data)) return 1;	// Tx data
	return osbdm_getreply(CMD_FLASH_DLEND);	// receive reply from command
}

// This will cause the bdm firmware to enable programming voltage to the chip and then
// execute the flash programming algorithm at address: (faddr) with (size) bytes of data at (*data)
// NOTE:  Before calling this the debugger must have already loaded the flash algo into target ram at (faddr).
int osbdm_flash_prog(unsigned int faddr, unsigned char size, unsigned char *data){

	unsigned char *ptr;
	unsigned char i;

	usb_data[0] = CMD_FLASH_PROG;	// Execute flash programming algo
	usb_data[1] = size;	// number of data bytes to program
	usb_data[2] = (unsigned char) (faddr & 0xFF);		// LSB Flash programming algo address
	usb_data[3] = (unsigned char) (faddr >>= 8) & 0xFF;	// MSB Flash programming algo address

	// load the data to be programmed into usb transmit buffer
	for( i=size, ptr = usb_data+4; i > 0; i--){
		*ptr++ = *data++;
	}
	if(osbdm_usb_send_ep1(4+size, usb_data)) return 1;	// Tx data
	return osbdm_getreply(CMD_FLASH_PROG);	// receive reply from command
}


