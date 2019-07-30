/*************************************************************************
 * DISCLAIMER *
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
/******************************************************************************
 * Copyright (c) 2007, Freescale Semiconductor
 *
 * File name   : Usb_Descriptor.c
 * Project name: JM60 Evaluation code
 *
 * Description : This software evaluates JM60 USB module 
 * History     :
 * 04/01/2007  : Initial Development
 * 03/08/2011  : Modified by P&E Microcomputer Systems to have
 *                 (A) Serial Number Support (if programmed into memory)
 *                 (B) Multifunction Device adding Virtual Serial Port
 *               http://www.pemicro.com/osbdm
 *****************************************************************************/
#include "Usb_Descriptor.h"

/********************************************************************
*       Device Descriptor
********************************************************************/
const UINT8 Device_Dsc_No_Serial_Number[Device_Descriptor_Size]= 
{

 	0x12,		      //blength
	0x01,		      //bDescriptor
	0x00,0x02,      //bcdUSB ver R=2.00 
  0xEF,		      //bDeviceClass
  0x02,		      //bDeviceSubClass			
  0x01,		      //bDeviceProtocol			
	EP0_BUFF_SIZE,		      //bMaxPacketSize0
	0xA2,0x15,	  //idVendor - 0x15A2(freescale Vendor ID)
	0x5E,0x00,	  //idProduct
	0x00,0x00,	  //bcdDevice - Version 1.00
	0x01,         //iManufacturer - Index to string Manufacturer descriptor
	0x02,	        //iProduct  - Index to string product descriptor 
	0x00,		      //iSerialNumber - Index to string serial number 
	0x01	        //bNumConfigurations - # of config. at current speed,
};

const UINT8 Device_Dsc_With_Serial_Number[Device_Descriptor_Size]= 
{

 	0x12,		      //blength
	0x01,		      //bDescriptor
	0x00,0x02,      //bcdUSB ver R=2.00 
  0xEF,		      //bDeviceClass
  0x02,		      //bDeviceSubClass			
  0x01,		      //bDeviceProtocol			
	EP0_BUFF_SIZE,		      //bMaxPacketSize0
	0xA2,0x15,	  //idVendor - 0x15A2(freescale Vendor ID)
	0x5E,0x00,	  //idProduct
	0x00,0x00,	  //bcdDevice - Version 1.00
	0x01,         //iManufacturer - Index to string Manufacturer descriptor
	0x02,	        //iProduct  - Index to string product descriptor 
	0x04,		      //iSerialNumber - Index to string serial number 
	0x01	        //bNumConfigurations - # of config. at current speed,
};


/********************************************************************
*       Configuration Descriptor
********************************************************************/
const UINT8 Cfg_01[Device_Configuration_Size]=
{
	
	0x09,		    //blength
	0x02,		    //bDescriptor
	0x62,0x00,
	0x03,		    //bNumInterfaces - at least 1 data interface
	0x01,		    //bConfigurationValue - 
	0x00,		    //iConfiguration - index to string descriptor	
	0xC0,		    //bmAttributes - 	bit 7- Compatibility with USB 1.0
				      //						    bit 6 if 1 self powered else Bus powered
				      //						    bit 5-remote wakeup
				      //						    bit 4-0-reserved
	0xDC,		    //bMaxPower - 200mA


/********************************************************************
*       OSBDM Interface Descriptor (Interface 0)
********************************************************************/	

	0x09,   // bLength - Size of this descriptor in bytes
	0x04,   // bDescriptorType - INTERFACE descriptor type
	0x00,   // bInterfaceNumber
	0x00,   // bAlternateSetting
	0x02,   // bNumEndpoints - # of endpoints in this interface
	0x00,   // bInterfaceClass
  0x00,   // bInterfaceSubClass
  0x00,   // bInterfaceProtocol
  0x02,   // iInterface - index of string descriptor for this intf

/********************************************************************
*       OSBDM Endpoint Descriptors
********************************************************************/

	0x07,   // bLength
	0x05,   // bDescriptorType - Endpoint
	0x01,   // bEndpointAddress (OUT)
	0x02,   // bmAttributes - Bulk
	0x40,0x00, //wMaxPacketSize 
	0x00,   // bInterval
	
	0x07,   // bLength
	0x05,   // bDescriptorType - Endpoint
	0x82,   // bEndpointAddress (IN)
	0x02,   // bmAttributes - Bulk
	0x40,0x00, //wMaxPacketSize 
	0x00,   // bInterval


/********************************************************************
*       Interface Association Descriptor (CDC Function)
********************************************************************/
 
  0x08,       //bLength
  0x0B,       //bDescriptortype - IAD
  0x01,       //bFirstInterface                        
  0x02,       //bInterfaceCount
  0x02,       //bFunctionClass - Should match Class of first interface in function
  0x02,       //bFunctionSubClass - Should match SubClass of first interface in function
  0x01,       //bFunctionProtocol - Should match Protocol of first interface in function
  0x03,       //iFunction - Index of string describing the function
	
/********************************************************************
*       Interface Descriptor (CDC Control - Interface 1)
********************************************************************/

	0x09,	      //blength
	0x04,		    //bDescriptorType - Interface descriptor
	0x01,		    //bInterfaceNumber - Zero based value identifying the index of the config.
	0x00,		    //bAlternateSetting
	0x01,		    //bNumEndpoints
	0x02,		    //bInterfaceClass - Communications Interface Class 
	0x02,		    //bInterfaceSubClass - Abstract Control Model
	0x01,		    //bInterfaceProtocol - AT Commands: V.250 etc 
	0x03,		    //iInterface - Index to String descriptor

/********************************************************************
*       Functional Descriptors  (CDC Class specific)
********************************************************************/

  0x05,       //bFunctionLength
  0x24,       //bDescriptorType - CS_Interface
  0x00,       //bDescriptorSubtype - Header Functional Descriptor
  0x10,0x01,  //bcdCDC - Revision of USB CDC Spec
  
  0x05,       //bFunctionLength
  0x24,       //bDescriptorType - CS_Interface
  0x01,       //bDescriptorSubtype - Call Management
  0x00,       //bmCapabilities - D1=0: Device sends/receives info only over Com Class interface
              //                 D0=0: Device does not handle call management itself 
  0x02,       //bDataInterface - Multiplexed commands handled via data interface 02h
  
  0x04,       //bFunctionLength
  0x24,       //bDescriptorType - CS_Interface
  0x02,       //bDescriptorSubtype - Abstract Control Management
  0x00,       //bmCapabilities - No Commands supported 
  
  0x05,       //bFunctionLength
  0x24,       //bDescriptorType - CS_Interface
  0x06,       //bDescriptorSubtype - Union Functional Descriptor
  0x01,       //bControlInterface - Interface controlling the union
  0x02,       //bSubordinateInterface - Subordinate of the union 
    
/********************************************************************
*       Endpoint  Descriptor
********************************************************************/
	0x07,           //blength
	0x05,           //bDescriptorType - EndPoint
	0x85,           //bEndpointAddress    
	0x03,           //bmAttributes
	0x20,0x00,      //wMaxPacketSize           
	0x02,           //bInterval

/********************************************************************
*       Interface Descriptor (CDC Data - Interface 2)
********************************************************************/
	0x09,           //blength
	0x04,           //bDescriptorType - Interface descriptor
	0x02,           //bInterfaceNumber - Zero based value identifying the index of the config.
	0x00,           //bAlternateSetting;
	0x02,           //bNumEndpoints
	0x0A,           //bInterfaceClass - Data Interface Class 
	0x00,           //bInterfaceSubClass - Field is un-used for Data Class interfaces and should have a value of 00h
	0x00,           //bInterfaceProtocol - No class specific protocol required 
	0x03,           //iInterface - Index to String descriptor
  
/********************************************************************
*       Endpoint __IN Descriptor
********************************************************************/

	0x07,           //blength
	0x05,           //bDescriptorType - EndPoint
	0x84,           //bEndpointAddress    
	0x02,           //bmAttributes
	0x20,0x00,      //wMaxPacketSize           
	0x00,           //bInterval

/********************************************************************
*       Endpoint __OUT Descriptor
********************************************************************/
	0x07,           //blength
	0x05,           //bDescriptorType - EndPoint
	0x03,           //bEndpointAddress    
	0x02,           //bmAttributes
	0x20,0x00,      //wMaxPacketSize           
	0x00            //bInterval         

};

/********************************************************************
*       Device Qualifier Descriptor
********************************************************************/
const UINT8 Device_Qualifier[10]=
{
    0x0A,         // bLength
    0x06,         // bDescriptorType - Dev Qualifier Type
    0x00,0x02,    // bcdUSB - USB Spec Rev: V2.00                                        
    0x00,         // bDeviceClass
    0x00,         // bDeviceSubClass
    0x00,         // bDeviceProtocol
    0x10,         // bMaxPacketSize0 - Max packet size for other speed
    0x01,         // bNumConfigurations - Num of other speed configs
    0x00          // bReserved
};


struct
{
	byte bLength;
	byte bDscType;
	word string[1];
} const sd000 = {sizeof(sd000),DSC_STR,0x0904};

struct
{
	byte bLength;
	byte bDscType;
	byte string[54];
} const sd001 = {
	sizeof(sd001),DSC_STR,
	{
	'F', 0,
	'r', 0,
	'e', 0,
	'e', 0,
	's', 0,
	'c', 0,
	'a', 0,
	'l', 0,
	'e', 0,
	' ', 0,
	'S', 0,
	'e', 0,
	'm', 0,
	'i', 0,
	'c', 0,
	'o', 0,
	'n', 0,
	'd', 0,
	'u', 0,
	'c', 0,
	't', 0,
	'o', 0,
	'r', 0,
	' ', 0,
	'I', 0,
	'n', 0,
	'c', 0
	}
};
       

struct
{
	byte bLength;
	byte bDscType;
	byte string[14];
} 
  OSSerialNum = {
	sizeof(OSSerialNum),DSC_STR,
	{
	'O', 0,
	'S', 0,
	'0', 0,
	'0', 0,
	'0', 0,
	'0', 0,
	}
}; 


volatile byte _Serial3 @0x0000FFFA;
volatile byte _Serial2 @0x0000FFFB;
volatile byte _Serial1 @0x0000FFFC;
volatile byte _Serial0 @0x0000FFFD;


struct
{
	byte bLength;
	byte bDscType;
	byte string[36];
} 
  const sd002 = {
	sizeof(sd002),DSC_STR,
	{
	'O', 0,
	'S', 0,
	'B', 0,
	'D', 0,
	'M', 0,
	' ', 0,
	'-', 0,
	' ', 0,
	'D', 0,
	'e', 0,
	'b', 0,
	'u', 0,
	'g', 0,
	' ', 0, 
	'P', 0,
	'o', 0,
	'r', 0,
	't', 0,
	}
}; 

struct
{
	byte bLength;
	byte bDscType;
	byte string[46];
} 
  const sd003 = {
	sizeof(sd003),DSC_STR,
	{
	'O', 0,
	'S', 0,
	'B', 0,
	'D', 0,
	'M', 0,
	' ', 0,
	'-', 0,
	' ', 0,
	'C', 0,
	'D', 0,
	'C', 0,
	' ', 0,
	'S', 0,
	'e', 0,
	'r', 0,
	'i', 0,
	'a', 0,
	'l', 0,
	' ', 0,
	'P', 0,
	'o', 0,
	'r', 0,
	't', 0,
	}
}; 

    
unsigned char* Str_Des[] = 
	{ (unsigned char*)&sd000,(unsigned char*)&sd001,(unsigned char*)&sd002,(unsigned char*)&sd003, (unsigned char*)&OSSerialNum};   

unsigned char* Cfg_Des[] =
	{(unsigned char*)&Cfg_01,(unsigned char*)&Cfg_01 };
    
pFunc Std_Class_Req_Handler[1] = { NULL };


