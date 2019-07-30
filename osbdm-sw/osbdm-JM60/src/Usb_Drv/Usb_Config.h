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
 * File name   : Usb_Config.h
 *
 * Description : This program is used for the evaluation for JM60 USB module 
 *               
 *
 * History     :
 * 04/01/2007  : Initial Development
 * 
 *************************************************************************************************/

#ifndef _USBCFG_H
#define _USBCFG_H

/* Configuration Attributes */
#define _SELFPOWER       0x01<<6         /*Self-powered  */
#define _REMOTEWAKEUP    0x01<<5         /*Remote Wakeup */
#define _DEFAULT         0x01<<7         /*Default Value */

#define EP0_BUFF_SIZE           8   /* can be 8, 16, 32, or 64 for full speed*/
#define MAX_NUM_INTF            1   /* maximum interface number*/


#define CLASS_NULL              0
#define STANDARD_CLASS_ID       1
#define HID_CLASS_ID            2


/*USB state machine*/
#define POWERED_STATE           0
#define ATTACHED_STATE          1
#define DEFAULT_STATE           2
#define ADDR_PENDING_STATE      3
#define ADDRESS_STATE           4
#define CONFIGURED_STATE        5
#define USB_ENTER_SUSPEND       6
#define USB_SUSPEND             7


#endif 
