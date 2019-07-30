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
//      osbdm_usb.c
// 
//
//  DESCRIPTION
//
//		OSBDM-JM60 USB communication
//
//
//----------------------------------------------------------------------------
//



#include <stdio.h>
#include "osbdm_usb.h"
#include "log.h"
#include "osbdm_hwdesc.h"
#include "usb.h"


struct usb_device *osbdm_devs[10]={NULL};	/* array pointing to all Open Source BDM devices found terminated by NULL pointer */
usb_dev_handle *osbdm_devh=NULL;

int usbBulkWritten;	// number of bytes written by usb endpoint
int usbBulkRead;	// number of bytes read by usb endpoint


/* provides low level USB functions which talk to opensourcebdm hardware */

/* initialisation */
void osbdm_usb_init(void) {
	usb_init();				/* init LIBUSB */
}


/* find all opensourcebdm devices attached to the computer */
// returns number of devices found
// or -1 if driver not running
int internal_usb_find_devices(int interfacenum, int the_VID, int the_PID1, int the_PID2, int the_PID3, int the_PID4, int the_PID5, int the_PID6) {
	struct usb_bus *libusb_bus;
	struct usb_device *libusb_dev;

// no version support in Linux
#ifndef LINUX
	struct usb_version *version;
#endif

  	unsigned int i=0;
	osbdm_devs[0]=NULL;				/* terminate the list */


#ifndef LINUX
	version = (struct usb_version *)usb_get_version();
	if (version==NULL) {
		return -1;	/* something quite wrong... */
	}
    //print("USBLIB DLL version: %i.%i.%i.%i\r\n",version->dll.major,version->dll.minor,version->dll.micro,version->dll.nano);

	if (version->driver.major==-1) {
		/* driver not running! */
		//print("USB: Driver not running\r\n");
	    return -1;	
	} 

	//print("USBLIB Driver version: %i.%i.%i.%i\r\n",version->driver.major, version->driver.minor,version->driver.micro, version->driver.nano);
#endif
	usb_find_busses();		/* enumerate all busses */
	usb_find_devices();		/* enumerate all devices */
	for (libusb_bus = usb_get_busses(); libusb_bus; libusb_bus = libusb_bus->next) {		/* scan through all busses */
    	for (libusb_dev = libusb_bus->devices; libusb_dev; libusb_dev = libusb_dev->next) {	/* scan through all devices */
  			if ((libusb_dev->descriptor.idVendor==the_VID)
				&&((libusb_dev->descriptor.idProduct==the_PID1)||(libusb_dev->descriptor.idProduct==the_PID2)||(libusb_dev->descriptor.idProduct==the_PID3)||(libusb_dev->descriptor.idProduct==the_PID4)||(libusb_dev->descriptor.idProduct==the_PID5)||(libusb_dev->descriptor.idProduct==the_PID6))
				&&(libusb_dev->config->interface->altsetting->bInterfaceNumber==interfacenum)) {
				/* found a device */
				osbdm_devs[i++]=libusb_dev;	/* add pointer to the device */
				osbdm_devs[i]=NULL;			/* terminate the list again */
				if (i>=(sizeof(osbdm_devs)/sizeof(struct usb_device*))) return i;	/* too many devices! */
			}
		}
	}
	return i;
}

int last_opened_interface = 0;

/* open connection to device enumerated by find_osbdm_devices */
/* returns 0 on success and 1 on error */
unsigned char internal_usb_open(unsigned int device_no, unsigned int interface_no) {
	last_opened_interface = interface_no;
	osbdm_devh = usb_open(osbdm_devs[device_no]);
	//This doesn't work for Vista, but it's ok since we only have 1 configuration and interface
	if (osbdm_devh==NULL) return (1);
	//print("USB Device open\r\n");
	if (usb_set_configuration(osbdm_devh,1)) 
            {
            #ifndef LINUX
             ERROR TEST REMOVING ME IN WINDOWS
             return(1);  // opensourcebdm has only one valid configuration 
            #endif
            } 
	//print("USB Configuration set\r\n");
	if (usb_claim_interface(osbdm_devh,interface_no)) 
            {
            #ifndef LINUX
             ERROR TEST REMOVING ME IN WINDOWS
             return(1);  // opensourcebdm has only one valid configuration 
            #endif
            } 
	//print("USB Interface claimed\r\n");

	return (0);
}

unsigned char internal_bootloader_usb_open(unsigned int device_no, unsigned int interface_no) {
	last_opened_interface = interface_no;
	osbdm_devh = usb_open(osbdm_devs[device_no]);
	//This doesn't work for Vista, but it's ok since we only have 1 configuration and interface
	if (osbdm_devh==NULL) return (1);
	//print("USB Device open\r\n");
#ifdef LINUX
	if (usb_set_configuration(osbdm_devh,1)) 
           {
           }
	//print("USB Configuration set\r\n");

	if (usb_claim_interface(osbdm_devh,interface_no)) 
           {
           }
#endif
	return (0);
}

/* find all opensourcebdm devices attached to the computer */
// returns number of devices found
// or -1 if driver not running
int osbdm_usb_find_devices(void) {
	return internal_usb_find_devices(0,opensourcebdm_VID,opensourcebdm_PID1,opensourcebdm_PID2,opensourcebdm_PID3,opensourcebdm_PID4,opensourcebdm_PID5,opensourcebdm_PID6);  // Debug port is Interface 0
}

int virtual_serial_usb_find_devices(void) {
	return internal_usb_find_devices(1,opensourcebdm_VID,opensourcebdm_PID1,opensourcebdm_PID1,opensourcebdm_PID1,opensourcebdm_PID1,opensourcebdm_PID1,opensourcebdm_PID1);  // Virtual Serial Port is Interface 1
}

int bootloader_usb_find_devices(void) {
	return internal_usb_find_devices(0,opensourcebdm_VID,opensourcebdm_bootloader_PID,opensourcebdm_bootloader_PID,opensourcebdm_bootloader_PID,opensourcebdm_bootloader_PID,opensourcebdm_bootloader_PID,opensourcebdm_bootloader_PID);  // Virtual Serial Port is Interface 1
}


/* open connection to device enumerated by find_osbdm_devices */
/* returns 0 on success and 1 on error */
unsigned char osbdm_usb_open(unsigned int device_no) {
	return (internal_usb_open(device_no, 0));
}
unsigned char virtual_serial_usb_open(unsigned int device_no) {
	return (internal_usb_open(device_no, 1));
}

unsigned char bootloader_usb_open(unsigned int device_no) {
	return (internal_bootloader_usb_open(device_no, 0));
}

/* closes connection to the currently open device */
void osbdm_usb_close(void) {
	if (osbdm_devh!=NULL) {
		usb_release_interface(osbdm_devh,last_opened_interface);		/* release the interface */
		//print("USB Interface released\r\n");
		usb_close(osbdm_devh);						/* close the device */
		//print("USB Device closed\r\n");
		osbdm_devh=NULL;							/* indicate that no device is open */
	}
}


//-----------------------------------------------------------------------------------------
/*
  sends a message to the OSBDM device over EP1
	global "usbBulkWritten" is set with the number of bytes that were sent
	returns 0 on success and 1 on error
*/
unsigned char osbdm_usb_send_ep1(unsigned char count, unsigned char *data) {
	if (osbdm_devh==NULL){
		//print("USB EP1 send: device not open\r\n");
		return 1;	// ignore if no device handler
	}
	//print("  USB EP1 send:  ");
	//print_dump(data,count);
	usbBulkWritten=usb_bulk_write(osbdm_devh, 0x01, (char *)data, count, TIMEOUT);
	if (usbBulkWritten<0) return 1; else return 0;
}
/*
  receives data from EP2
	global "usbBulkRead" is set with the number of bytes read
	returns 0 on success and 1 on error
*/
unsigned char osbdm_usb_recv_ep2(unsigned char count, unsigned char * data) {
	if (osbdm_devh==NULL){
		//print("USB EP2 receive request: device not open\r\n");
		return 1;	// ignore if no device handler
	}
	//print("  USB EP2 receive request:  ");
//	//print_dump(data,6);
	usbBulkRead=usb_bulk_read(osbdm_devh, 0x82, (char *)data, count, TIMEOUT);
	//print_dump(data,count);
	if (usbBulkRead<0) return 1; else return 0;
}

unsigned char osbdm_control(int requesttype, int request, int value, int index, unsigned char count, unsigned char *data) {
	if (osbdm_devh==NULL){
		//print("USB EP1 send: device not open\r\n");
		return 1;	// ignore if no device handler
	}

    if (usb_control_msg(osbdm_devh, requesttype, request, value, index, (char *)data, count, TIMEOUT)<0)
		return 1; else return 0;

}

