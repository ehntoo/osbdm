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
/*************************************************************************************************
 * Copyright (c) 2007, Freescale Semiconductor
 *
 * File name   : Usb_Bdt.h
 * Project name: JM60 Evaluation code
 *
 *
 * Description : The BDT definition for USB module 
 * History     :
 * 04/01/2007  : Initial Development
 * 
 *************************************************************************************************/

#ifndef _USBBDT_H
#define _USBBDT_H

#include "typedef.h"
#include "Usb_Config.h"
#include "Usb_Ep0_Handler.h"


/* Buffer Descriptor Status Register Initialization Parameters */
#define _BDTSTALL      0x04                /*Buffer Stall enable*/
#define _DATA0         0x00                /*DATA0 packet expected next*/
#define _DATA1         0x40                /*DATA1 packet expected next*/
#define _DTS           0x08                /*DTS Mask*/
#define _SIE           0x80                /*SIE owns buffer*/
#define _CPU           0x00                /*CPU owns buffer*/

 
typedef union _USB_DEVICE_STATUS
{
    byte _byte;
    struct
    {
        unsigned Self_Power  :1;
        unsigned RemoteWakeup:1;        /*[0]Disabled [1]Enabled: */
    }BitCtl;
} USB_DEVICE_STATUS;

typedef union _BD_STAT
{
    byte _byte;
    struct{
        unsigned :1;
        unsigned :1;
        unsigned BDTSTALL:1;            /*Buffer Stall Enable*/
        unsigned DTS:1;                 /*Data Toggle Synch Enable*/
        unsigned :1;                    /*Address Increment Disable*/
        unsigned :1;                    /*BD Keep Enable*/
        unsigned DATA:1;                /*Data Toggle Synch Value*/
        unsigned OWN:1;                 /*USB Ownership*/
    }McuCtlBit;
    struct{
        unsigned :1;
        unsigned :1;
        unsigned PID0:1;
        unsigned PID1:1;
        unsigned PID2:1;
        unsigned PID3:1;
        unsigned :1;
        unsigned OWN:1;
    }SieCtlBit;
    struct{
        unsigned    :2;
        unsigned PID:4;                 /*Packet Identifier*/
        unsigned    :2;
    }RecPid;
} BD_STAT;                              /*Buffer Descriptor Status Register*/

typedef struct _BUFF_DSC
{
        BD_STAT Stat;
        byte Cnt;
        byte Addr;                      /*Buffer Address */
} BUFF_DSC;                                  /*Buffer Descriptor Table*/


typedef struct _BDTMAP
{ 
  BUFF_DSC ep0Bi;              /*Endpoint 0 BD In*/
  BUFF_DSC ep0Bo;              /*Endpoint 1 BD Out*/

  BUFF_DSC ep1Bio;             /*Endpoint 1 BD IN or OUT*/
  BUFF_DSC ep2Bio;             /*Endpoint 2 BD IN or OUT*/
  BUFF_DSC ep3Bio;             /*Endpoint 3 BD IN or OUT*/
  BUFF_DSC ep4Bio;             /*Endpoint 4 BD IN or OUT*/
  BUFF_DSC ep5Bio_Even;        /*Endpoint 5 BD IN or OUT  Even*/
  BUFF_DSC ep5Bio_Odd;         /*Endpoint 5 BD IN or OUT  Odd */
  BUFF_DSC ep6Bio_Even;        /*Endpoint 6 BD IN or OUT  Even */
  BUFF_DSC ep6Bio_Odd;         /*Endpoint 6 BD IN or OUT  Odd*/

  WORD Reserved;
}BDTMAP; 


#define CAL_BD_OFFSET(address)       ((byte)((address - 0x1860) >> 2))

#endif 