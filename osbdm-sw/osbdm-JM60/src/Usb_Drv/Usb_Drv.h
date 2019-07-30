/*************************************************************************
 * DISCLAIMER                                                            *
 * Services performed by FREESCALE in this matter are performed          *
 * AS IS and without any warranty. CUSTOMER retains the final decision   *
 * relative to the total design and functionality of the end product.    *
 * FREESCALE neither guarantees nor will be held liable by CUSTOMER      *
 * for the success of this project. FREESCALE disclaims all warranties,  *
 * express, implied or statutory including, but not limited to,          *
 * implied warranty of merchantability or fitness for a particular       *
 * purpose on any hardware, software ore advise supplied to the project  *
 * by FREESCALE, and or any product resulting from FREESCALE services.   *
 * In no event shall FREESCALE be liable for incidental or consequential *
 * damages arising out of this agreement. CUSTOMER agrees to hold        *
 * FREESCALE harmless against any and all claims demands or actions      *
 * by anyone on account of any damage, or injury, whether commercial,    *
 * contractual, or tortuous, rising directly or indirectly as a result   *
 * of the advise or assistance supplied CUSTOMER in connection with      *
 * product, services or goods supplied under this Agreement.             *
 *************************************************************************/
/*************************************************************************************************
 * Copyright (c) 2007, Freescale Semiconductor
 *
 * File name   : Usb_Drv.h
 *
 *
 * Description : This program is used for evaluation of JM60 USB module 
 *               
 *
 * History     :
 * 04/01/2007  : Initial Development
 * 
 *************************************************************************************************/

#ifndef _USBDRV_H
#define _USBDRV_H

#include "USB_User_API.h"
#include "typedef.h"
#include "Usb_Bdt.h"
#include "derivative.h"
#include "Usb_Config.h"
#include "Usb_Ep0_Handler.h"


#define USB_BUS_ATTACHED    1
#define USB_BUS_DETACHED    0

/* USBCTL0 Initialization Parameters */
#define _USBPHYEN           0x01            /* Use internal transceiver*/
#define _USBPUEN            0x40            /* Use internal pull-up resistor*/
#define _USBREGEN           0x04            /* Use the internal regulator*/
#define _USBRESET           0x80

#define USB_RST             _USBRESET
#define UCFG_VAL            _USBPUEN|_USBPHYEN|_USBREGEN

/******************************************************************************
 * USB - STAT Register
 *       bit 7 6 5 4 3 2 1 0
 *           | | | | | | | |
 *           | | | | | |
 *           | | | | | |_ODD
 *           | | | | |___IN
 *           |_|_|_|_____ENDP 
 *
 *****************************************************************************/
#define EP_CTRL     0x0C            /* Cfg Control pipe for this endpoint */
#define EP_OUT      0x08            /* Cfg OUT only pipe for this endpoint*/
#define EP_IN       0x04            /* Cfg IN only pipe for this endpoint */
#define HSHK_EN     0x01            /* Enable handshake packet              */
                                    /* Handshake should be disable for isochorous transfer*/

#define OUT         0
#define IN          1

#define EP_NUM_MASK   0b11110000
#define EP_DIR_MASK   0b00001000
#define EP_EVEN_MASK  0b00000100

#define EPADR0_IN		0x08				// EP0 IN offset address
#define EPADR0_OUT	0x0C				// EP0 OUT offset address

#define EP00_OUT    ((0x00<<4)|(OUT<<3))
#define EP00_IN     ((0x00<<4)|(IN<<3))
#define EP01_OUT    ((0x01<<4)|(OUT<<3))
#define EP01_IN     ((0x01<<4)|(IN<<3))
#define EP02_OUT    ((0x02<<4)|(OUT<<3))
#define EP02_IN     ((0x02<<4)|(IN<<3))
#define EP03_OUT    ((0x03<<4)|(OUT<<3))
#define EP03_IN     ((0x03<<4)|(IN<<3))
#define EP04_OUT    ((0x04<<4)|(OUT<<3))
#define EP04_IN     ((0x04<<4)|(IN<<3))
#define EP05_OUT    ((0x05<<4)|(OUT<<3))
#define EP05_IN     ((0x05<<4)|(IN<<3))
#define EP06_OUT    ((0x06<<4)|(OUT<<3))
#define EP06_IN     ((0x06<<4)|(IN<<3))



extern byte Usb_Device_State;
extern USB_DEVICE_STATUS Usb_Stat;
extern byte Usb_Active_Cfg;
extern byte Usb_Alt_Intf[MAX_NUM_INTF];

extern  CTRL_TRANSFER_SETUP Setup_Pkt;
extern  CTRL_TRANSFER_DATA CtrlTrf_Data;
extern  BDTMAP Bdtmap;
extern void  Initialize_USBModule(void);
extern void  USB_Buf_Rdy(BUFF_DSC *buffer_dsc);

void Check_USBBus_Status(void);
void USB_Remote_Wakeup(void);
void USB_Soft_Detach(void); 

void Clear_Mem(byte* startAdr,byte count);

void ConfigureUSBDevice(void);

#endif