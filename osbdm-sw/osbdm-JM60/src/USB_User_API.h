/**
Copyright (c) 2007 Freescale Semiconductor
Freescale Confidential Proprietary
\file     USB_User_API.h
\brief    Header for USB_User_API.c
\author   Freescale Semiconductor
\author   Guadalajara Applications Laboratory RTAC Americas
\author   Jose Ruiz
\version  1.0
\date     17/Oct/2007
\warning  
* History:

  30/July/2007      Start of bridge coding
  15/October/2007   Modifications for new version of bridges
  17/October/2007   Macros added and funcion names changed
  29/April/2008     Added defines to modularize loction of EP memory in BDT
*
* 03/08/2011  : Modified by P&E Microcomputer Systems
*               http://www.pemicro.com/osbdm
*
  
*/

#ifndef _USB_USER_API_H_
#define _USB_USER_API_H_

/* Includes */
#include "derivative.h"
#include "Usb_Bdt.h"
#include "Usb_Drv.h"
#include "Usb_Descriptor.h"
#include "typedef.h"

/*Defines */

#define EPADR1_OUT	0x10			// EP1 OUT offset address - 64 bytes
#define EPADR2_IN		0x20			// EP2 IN offset address - 64 bytes
#define EPADR3_OUT	0x30			// EP3 OUT offset address - 32 bytes
#define EPADR4_IN		0x38			// EP4 IN offset address - 32 bytes
#define EPADR5_IN		0x08			// EP5 GARBAGE (Address offet $20)

#define EP1_Set Bdtmap.ep1Bio
#define EP2_Set Bdtmap.ep2Bio
#define EP3_Set Bdtmap.ep3Bio
#define EP4_Set Bdtmap.ep4Bio
#define EP5Even_Set Bdtmap.ep5Bio_Even
#define EP5Odd_Set Bdtmap.ep5Bio_Odd
#define EP6Even_Set Bdtmap.ep6Bio_Even
#define EP6Odd_Set Bdtmap.ep6Bio_Odd


/* Use these definitions for the API functions EndPoint_IN and CheckEndPointOUT */
#define EP0   0
#define EP1   1
#define EP2   2
#define EP3   3
#define EP4   4
/***********************/

/* External Variables Declaration */
extern UINT8 EP1_Buffer[UEP1_SIZE];
extern UINT8 EP2_Buffer[UEP2_SIZE];
extern UINT8 EP3_Buffer[UEP3_SIZE];
extern UINT8 EP4_Buffer[UEP4_SIZE];

extern UINT8 EP1_BufferLastReceivedDataSize;
extern UINT8 EP2_BufferLastReceivedDataSize;
extern UINT8 EP3_BufferLastReceivedDataSize;
extern UINT8 EP4_BufferLastReceivedDataSize;

extern UINT8 EP1_OUT_SIZE;
extern UINT8 EP2_OUT_SIZE;
extern UINT8 EP3_OUT_SIZE;
extern UINT8 EP4_OUT_SIZE;

/* Function Prototypes */
void Endpoint_Init(void);
void EndPoint_OUT(UINT8);
void EndPoint_IN(UINT8,UINT8);


#endif /* _USB_USER_API_H_ */
