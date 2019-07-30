/* OSBDM-JM60 Target Interface Software Package
 * Portions Copyright (C) 2009  Freescale
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

#ifndef _SCI_H_
#define _SCI_H_

#include "derivative.h"				// include peripheral declarations  
#include "typedef.h"				// include peripheral declarations  
#include "USB_CDC.h"				// include peripheral declarations  

extern void SCI_Init(unsigned sbr, char scic1val);
extern void SCI_Init_CDC(CDC_Line_Coding LineCoding);
extern void SCI_SendBuffer(int count, char *data);
extern char SCI_CharReady(void);
extern char SCI_GetChar(void);
extern char SCI_GetOverflowStatus(void);
extern void interrupt SCI_ReceiveISR(void);
extern void interrupt SCI_OverrunISR(void);
extern void SCI_save_and_disable_interrupt_settings(void);
extern void SCI_restore_interrupt_settings(void);
extern void SCI_Close(void);

// size of serial receive buffer
#define SCI_RX_BUFSIZE 256

extern char sci_rx_buf[SCI_RX_BUFSIZE];    // receive buffer
extern char sci_rx_sptr;     // receive data start pointer
extern char sci_rx_eptr;     // receive data end pointer
extern char sci_rx_overflow_occured;
extern char sci_virtual_serial_port_is_enabled;

#endif // _SCI_H_
