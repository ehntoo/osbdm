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
 * File name   : UsbDesc.h
 *
 * Description : This software evaluates JM60 USB module 
 *
 * History     :
 * 04/01/2007  : Initial Development
 * 03/08/2011  : Modified by P&E Microcomputer Systems to have
 *                 (A) Serial Number Support (if programmed into memory)
 *                 (B) Multifunction Device adding Virtual Serial Port
 *               http://www.pemicro.com/osbdm
 * 
 *************************************************************************************************/
#ifndef USBDEFS_DSC_H
#define USBDEFS_DSC_H

#include "typedef.h"
#include "Usb_Config.h"
#include <stddef.h>
#include "Usb_Descriptor.h"

/* Descriptor Types*/
#define DSC_DEV     0x01
#define DSC_CFG     0x02
#define DSC_STR     0x03
#define DSC_INTF    0x04
#define DSC_EP      0x05


 /* USB standard endpoint address format: dir:X:X:X:EP3:EP2:EP1:EP0*/
#define _EP01_OUT   0x01
#define _EP01_IN    0x81
#define _EP02_OUT   0x02
#define _EP02_IN    0x82
#define _EP03_OUT   0x03
#define _EP03_IN    0x83
#define _EP04_OUT   0x04
#define _EP04_IN    0x84
#define _EP05_OUT   0x05
#define _EP05_IN    0x85
#define _EP06_OUT   0x06
#define _EP06_IN    0x86

/* Endpoint transfer type*/
#define _CTRL       0x00            //control transfer
#define _ISO        0x01            //isochronous transfer
#define _BULK       0x02            //bulk transfer
#define _INT        0x03            //interrupt transfer

/*isochronous endpoint synchronization type */
#define _NS         0x00<<2         //No Synchronization
#define _AS         0x01<<2         //Asynchronous
#define _AD         0x02<<2         //Adaptive
#define _SY         0x03<<2         //Synchronous

/*isochronous Endpoint Usage Type */
#define _DE         0x00<<4         //Data endpoint
#define _FE         0x01<<4         //Feedback endpoint
#define _IE         0x02<<4         //Implicit feedback Data endpoint

// endpoint EP1 definition
#define UEP1_CTL                EPCTL1
#define UEP1_BD                 Bdtmap.ep1Bio
#define UEP1_SIZE               64


// endpoint EP2 definition
#define UEP2_CTL                EPCTL2
#define UEP2_BD                 Bdtmap.ep2Bio
#define UEP2_SIZE               64


// endpoint EP3 definition
#define UEP3_CTL                EPCTL3
#define UEP3_BD                 Bdtmap.ep3Bio
#define UEP3_SIZE               32


// endpoint EP4 definition
#define UEP4_CTL                EPCTL4
#define UEP4_BD                 Bdtmap.ep4Bio
#define UEP4_SIZE               32

/*
// endpoint EP5 Even definition
#define UEP5_CTL                EPCTL5
#define UEP5_BD_EVEN            Bdtmap.ep5Bio_Even
#define UEP5_EVEN_SIZE          64


// endpoint EP5 Odd definition
#define UEP5_CTL                EPCTL5
#define UEP5_BD_ODD             Bdtmap.ep5Bio_Odd
#define UEP5_ODD_SIZE           64


// endpoint EP6 Even definition
#define UEP6_CTL                EPCTL6
#define UEP6_BD_EVEN            Bdtmap.ep6Bio_Even
#define UEP6_EVEN_SIZE          64


// endpoint EP6 Odd definition
#define UEP6_CTL                EPCTL6
#define UEP6_BD_ODD             Bdtmap.ep6Bio_Odd
#define UEP6_ODD_SIZE           64
*/

/*USB device descriptor structure*/
typedef struct _USB_DEV_DSC
{
    byte bLength;
    byte bDscType;
    word bcdUSB;
    byte bDevCls;
    byte bDevSubCls;
    byte bDevProtocol;
    byte bMaxPktSize0;
    word idVendor;
    word idProduct;
    word bcdDevice;
    byte iMFR;
    byte iProduct;
    byte iSerialNum;
    byte bNumCfg;
} USB_DEV_DSC;


/*USB configuration descriptor structure*/
typedef struct _USB_CFG_DSC
{
    byte bLength;
    byte bDscType;
    word wTotalLength;
    byte bNumIntf;
    byte bCfgValue;
    byte iCfg;
    byte bmAttributes;
    byte bMaxPower;
} USB_CFG_DSC;

/* USB interface descriptor structure*/
typedef struct _USB_INTF_DSC
{
    byte bLength;
    byte bDscType;
    byte bIntfNum;
    byte bAltSetting;
    byte bNumEPs;
    byte bIntfCls;
    byte bIntfSubCls;
    byte bIntfProtocol;
    byte iIntf;
} USB_INTF_DSC;

/*USB endpoint descriptor structure*/
typedef struct _USB_EP_DSC
{
    byte bLength;
    byte bDscType;
    byte bEPAdr;
    byte bmAttributes;
    word wMaxPktSize;
    byte bInterval;
} USB_EP_DSC;



/*
typedef struct _USB_CFG                            
{   
    USB_CFG_DSC     Cfg_Dsc_01;                  
    USB_INTF_DSC    Int1_Dsc_i00a00;                
    USB_EP_DSC      Ep1_Dsc_i00a00;          
    USB_EP_DSC      Ep2_Dsc_i00a00;
    USB_INTF_DSC    Int2_Dsc_i00a00;                
    USB_EP_DSC      Ep3_Dsc_i00a00;
    USB_EP_DSC      Ep4_Dsc_i00a00;
}USB_CFG;
*/

extern unsigned char* Str_Des[];
extern unsigned char* Cfg_Des[];

#define Device_Descriptor_Size 18
#define Device_Configuration_Size 0x62
extern const UINT8 Cfg_01[Device_Configuration_Size];//[0x6C]=
extern const UINT8 Device_Dsc_No_Serial_Number[Device_Descriptor_Size];
extern const UINT8 Device_Dsc_With_Serial_Number[Device_Descriptor_Size]; 

extern volatile byte _Serial3 @0x0000FFFA;
extern volatile byte _Serial2 @0x0000FFFB;
extern volatile byte _Serial1 @0x0000FFFC;
extern volatile byte _Serial0 @0x0000FFFD;

extern struct
{
	byte bLength;
	byte bDscType;
	byte string[14];
} OSSerialNum; 

#endif 