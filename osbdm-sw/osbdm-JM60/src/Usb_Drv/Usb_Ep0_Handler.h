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
 * File name   : Usb_Setup_Pkt.h
 * Project name: JM60 Evaluation code
 *
 *
 * Description : This program is used for evaluation of JM60 USB module 
 *
 * History     :
 * 04/01/2007  : Initial Development
 * 
 ************************s*************************************************************************/
#ifndef USB_EP0_HANDLER_H
#define USB_EP0_HANDLER_H

#include "typedef.h"
#include "Usb_Config.h"

/******************************************************************************
 * Standard Request Codes
 * USB 2.0 Spec Ref Table 9-4
 *****************************************************************************/
#define GET_STATUS  0
#define CLR_FEATURE 1
#define SET_FEATURE 3
#define SET_ADR     5
#define GET_DSC     6
#define SET_DSC     7
#define GET_CFG     8
#define SET_CFG     9
#define GET_INTF    10
#define SET_INTF    11
#define SYNCH_FRAME 12

/* USB_CDC Definitions */
#define SET_LINE_CODING         0x20
#define GET_LINE_CODING         0x21
#define SET_CONTROL_LINE_STATE  0x22

/* Standard Feature Selectors */
#define DEVICE_REMOTE_WAKEUP    0x01
#define ENDPOINT_HALT           0x00

/* USB PID: Token Types - See chapter 8 in the USB specification*/
#define SETUP_TOKEN         0b00001101
#define OUT_TOKEN           0b00000001
#define IN_TOKEN            0b00001001

/* bmRequestType Definitions */
#define HOST_TO_DEV         0
#define DEV_TO_HOST         1

#define STANDARD            0x00
#define CLASS               0x01
#define VENDOR              0x02

#define RCPT_DEV            0
#define RCPT_INTF           1
#define RCPT_EP             2
#define RCPT_OTH            3

/*Control transfer state machine*/
#define WAIT_SETUP_TOKEN        0
#define CTL_TRF_DATA_TX         1
#define CTL_TRF_DATA_RX         2

/******************************************************************************
 * CTRL_TRANSFER_SETUP:
 *
 * There are 8 bytes for each setup packet. 
 * The buffer size can be  8, 16, 32, or 64, it is determine by EP0_BUFF_SIZE in
 * this siftware.
 *
 * These 8 bytes are defined to be directly addressable to improve access speed
 * and reduce code size.
 *
 *****************************************************************************/
typedef union _CTRL_TRANSFR_SETUP
{
    struct
    {
        byte _byte[EP0_BUFF_SIZE];
    };
    
    /* Standard Device Requests */
    struct
    {
        byte bmRequestType;
        byte bRequest;    
        word wValue;
        word wIndex;
        word wLength;
    }StdCtl;
    
    struct
    {
        unsigned :8;
        unsigned :8;
        WORD W_Value;
        WORD W_Index;
        WORD W_Length;
    }CtlPara;
    
    struct
    {
        unsigned Recipient:5;           /*Device,Interface,Endpoint,Other*/
        unsigned RequestType:2;         /*Standard,Class,Vendor,Reserved*/
        unsigned DataDir:1;             /*Host-to-device,Device-to-host*/
        unsigned :8;
        byte bFeature;                  /*DEVICE_REMOTE_WAKEUP,ENDPOINT_HALT*/
        unsigned :8;
        unsigned :8;
        unsigned :8;
        unsigned :8;
        unsigned :8;
    }CtlReqT;
    
    struct
    {
        unsigned :8;
        unsigned :8;
        byte bDscIndex;                 /*For Configuration and String DSC Only*/
        byte bDscType;                  /*Device,Configuration,String*/
        word wLangID;                   /*Language ID*/
        unsigned :8;
        unsigned :8;
    }ReqWval;
    
    struct
    {
        unsigned :8;
        unsigned :8;
        BYTE bDevADR;                   /*Device Address 0-127*/
        byte bDevADRH;                  /*Must equal zero*/
        unsigned :8;
        unsigned :8;
        unsigned :8;
        unsigned :8;
    }CtlAdd;
    
    struct
    {
        unsigned :8;
        unsigned :8;
        byte bCfgValue;                 /*Configuration Value 0-255*/
        byte bCfgRSD;                   /*Must equal zero (Reserved)*/
        unsigned :8;
        unsigned :8;
        unsigned :8;
        unsigned :8;
    }CtrCfg;
    
    struct
    {
        unsigned :8;
        unsigned :8;
        byte bAltID;                    /*Alternate Setting Value 0-255*/
        byte bAltID_H;                  /*Must equal zero*/
        byte bIntfID;                   /*Interface Number Value 0-255*/
        byte bIntfID_H;                 /*Must equal zero*/
        unsigned :8;
        unsigned :8;
    }CtlIntf;
    struct
    {
        unsigned :8;
        unsigned :8;
        unsigned :8;
        unsigned :8;
        byte bEPID;                     /*Endpoint ID (Number & Direction)*/
        byte bEPID_H;                   /*Must equal zero*/
        unsigned :8;
        unsigned :8;
    }CtlEp;
    
    struct
    {
        unsigned :8;
        unsigned :8;
        unsigned :8;
        unsigned :8;
        unsigned EPNum:4;               /*Endpoint Number 0-15*/
        unsigned :3;
        unsigned EPDir:1;               /*Endpoint Direction: 0-OUT, 1-IN*/
        unsigned :8;
        unsigned :8;
        unsigned :8;
    }EpND;
    
} CTRL_TRANSFER_SETUP;

/******************************************************************************
 * CTRL_TRANSFER_DATA:
 * The size can be 8, 16, 32, or 64. It is specified by the value of EP0_BUFF_SIZE 
 *
 * These 8 bytes are defined to be directly addressable to improve access speed
 * and reduce code size.
 *****************************************************************************/
typedef union _CTRL_TRANSFER_DATA
{
    /*Array for indirect addressing*/
    struct
    {
        byte _byte[EP0_BUFF_SIZE];
    }ByteArray;
    
    /*First 8-byte direct addressing */
    struct
    {
        byte _byte0;
        byte _byte1;
        byte _byte2;
        byte _byte3;
        byte _byte4;
        byte _byte5;
        byte _byte6;
        byte _byte7;
    }EachByte;
    
    struct
    {
        word _word0;
        word _word1;
        word _word2;
        word _word3;
    }Eachword;

} CTRL_TRANSFER_DATA;


extern byte Ctrl_Trf_Session_Owner;   
extern byte *pSrc; 
extern byte *pObj;
extern int Transfer_Cnt;
extern int Total_Received_data;


extern void USB_Transaction_Handler(void);
extern void USB_Prepare_Next_Trf(void);

#endif
