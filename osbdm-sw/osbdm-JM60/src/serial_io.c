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

/*
* 03/08/2011  : Modified by P&E Microcomputer Systems
*               http://www.pemicro.com/osbdm
*
*
*/


// serial i/o routines

#include "serial_io.h"
#include "USB_CDC.h"

char sci_rx_buf[SCI_RX_BUFSIZE];    // receive buffer
char sci_rx_sptr = 0;     // receive data start pointer
char sci_rx_eptr = 0;     // receive data end pointer
char sci_rx_overflow_occured;
char sci_remember_SCI1C2_REI;
char sci_virtual_serial_port_is_enabled = FALSE;

void SCI_Init(unsigned sbr, char scic1val) {
  
  SCI1BD = sbr;
  SCI1C1 = scic1val;

  sci_rx_sptr = 0;
  sci_rx_eptr = 0;
  sci_rx_overflow_occured = FALSE;

 /* enable serial output/input */
  PTGDD = 0x04;       // set PTG2 output, rest input
  PTGD_PTGD2 = 1;     // enable serial bridge

  SCI1C2 = 0x2C;			// enable transmit and receive and receive interrupt
  SCI1C3 = 0x0F;			// error interrupts enabled
 
  sci_remember_SCI1C2_REI = SCI1C2_RIE;
  
  sci_virtual_serial_port_is_enabled = TRUE;
  
  //SCI1C1_LOOPS = 1; // TEST!!!!
  
}

void SCI_Init_CDC(CDC_Line_Coding LineCoding) {
  
  unsigned char paramsval;
  
  unsigned long u32TempBauds = LWordSwap(LineCoding.DTERate);
  u32TempBauds = SCI1_BAUDRATE(u32TempBauds);
  
  paramsval = 0;
  if (LineCoding.ParityType==1)
    paramsval = paramsval | 0x03;
  if (LineCoding.ParityType==2)
    paramsval = paramsval | 0x02;
  if (LineCoding.Databits==9) 
    paramsval = paramsval | 0x10;

  SCI1BD = u32TempBauds & 0xffff;
	SCI1C1 = paramsval;                               //8 Bit, No parity, normal operation		

  sci_rx_sptr = 0;
  sci_rx_eptr = 0;
  sci_rx_overflow_occured = FALSE;

 /* enable serial output/input */
  PTGDD = 0x04;       // set PTG2 output, rest input
  PTGD_PTGD2 = 1;     // enable serial bridge

  SCI1C2 = 0x2C;			// enable transmit and receive and receive interrupt
  SCI1C3 = 0x0F;			// error interrupts enabled
 
  sci_remember_SCI1C2_REI = SCI1C2_RIE;
  
  sci_virtual_serial_port_is_enabled = TRUE;
  
  t_serial_init();
  
  // SCI1C1_LOOPS = 1; // TEST!!!!
  
}


void SCI_Close(void){
  
  SCI1C2 = 0x00;			// Disable transmit and receive and receive interrupt
  sci_rx_sptr = 0;
  sci_rx_eptr = 0;    // Clear buffer
  sci_rx_overflow_occured = FALSE;
  sci_virtual_serial_port_is_enabled = FALSE;
  }
      


void SCI_save_and_disable_interrupt_settings(void) {
  sci_remember_SCI1C2_REI = SCI1C2_RIE;
  SCI1C2_RIE = 0; 
}

void SCI_restore_interrupt_settings(void){
  SCI1C2_RIE = sci_remember_SCI1C2_REI;
}


// transmit data out serial port
// this waits for TX complete before returning
void SCI_Transmit(char sData){
    while (!SCI1S1_TDRE);  // wait for Tx data buffer empty
    SCI1D = sData; // load data to SCI register 
    while (!SCI1S1_TC); // wait for Tx complete 
}


// transmit count bytes from data buffer out serial port
void SCI_SendBuffer(int count, char *data){
	while(count > 0){		
		SCI_Transmit(*data++);
		--count;
	}
}

// if UART serial data receive put it in the receive buffer

	char temp_sci_stat;
	char temp_sci_data;

void SCI_get_pending_chars_during_an_interrupt(void) {
  
	temp_sci_stat = SCI1S1;	// get status
	if((temp_sci_stat & 0x20) == 0) { 
	  return;	// exit if byte not received
	}
	
	temp_sci_data = SCI1D;	// read data
	if( (temp_sci_stat & 0x03) == 0 ) {
  	sci_rx_buf[sci_rx_eptr++] = temp_sci_data;			// put in buffer
  	if(sci_rx_eptr >= SCI_RX_BUFSIZE) sci_rx_eptr=0;	// reset if at end

    if ((sci_rx_eptr==sci_rx_sptr)||(temp_sci_stat&0x08!=0)) // overflow!
      sci_rx_overflow_occured = TRUE;  
	}
}

void interrupt SCI_ReceiveISR(void){
  SCI_get_pending_chars_during_an_interrupt();	
}

void interrupt SCI_OverrunISR(void){
	char stat;
	char data;

	stat = SCI1S1; 
	data = SCI1D;	 // force read data to clear overrun flag
  if (stat&0x08!=0) // overflow!
    sci_rx_overflow_occured = TRUE;  
  
}

char SCI_GetOverflowStatus(void){
  char tempval;
  tempval = sci_rx_overflow_occured;
  sci_rx_overflow_occured = 0;
  return(tempval); 
}

char SCI_CharReady(void) {
 
 if (sci_rx_eptr!=sci_rx_sptr)
    return TRUE;
 return FALSE;
}             

char SCI_GetChar(void) {
 char tempbyte;
 
 SCI_save_and_disable_interrupt_settings(); // prevent corruption
 tempbyte = sci_rx_buf[sci_rx_sptr++];
	if(sci_rx_sptr >= SCI_RX_BUFSIZE) sci_rx_sptr=0;
 SCI_restore_interrupt_settings();	
 return tempbyte;
}
