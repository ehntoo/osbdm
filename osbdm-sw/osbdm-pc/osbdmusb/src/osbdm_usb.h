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
//      osbdm_usb.h
// 
//
//  DESCRIPTION
//
//		OSBDM-JM60 USB communication
//
//
//----------------------------------------------------------------------------
//

#ifndef __OSBDM_USB_H__
#define __OSBDM_USB_H__

void			osbdm_usb_init(void);
int				osbdm_usb_find_devices(void);
unsigned char	osbdm_usb_open(unsigned int device_no);
void			osbdm_usb_close(void);
unsigned char	osbdm_usb_send_ep1(unsigned char count, unsigned char *data);
unsigned char	osbdm_usb_recv_ep2(unsigned char count, unsigned char *data);
int bootloader_usb_find_devices(void);
unsigned char osbdm_control(int requesttype, int request, int value, int index, unsigned char count, unsigned char *data);
unsigned char bootloader_usb_open(unsigned int device_no);

#endif  // __OSBDM_USB_H__
