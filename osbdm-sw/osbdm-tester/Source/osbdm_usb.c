/*
*/

#include <stdio.h>
#include "osbdm_usb.h"
#include "osbdm_hwdesc.h"
#include "libusb.h"


// These determine what USB ID information is verified for the connection to be established
// Vendor ID, Vendor ID and Product ID, or Nothing.  Uncomment the one you want
//#define CHECK_VID_PID
//#define CHECK_VID
#define CHECK_NOTHING


struct usb_device *osbdm_devs[10]={NULL};	// array pointing to all OSBDM devices found terminated by NULL pointer
usb_dev_handle *osbdm_devh=NULL;

int usbBulkWritten;	// number of bytes written by usb endpoint
int usbBulkRead;	// number of bytes read by usb endpoint


void osbdm_usb_init(){
	usb_init();				/* init LIBUSB */
}

/* find all OSBDM devices attached to the computer */
// returns number of devices found
// or -1 if driver not running
int osbdm_usb_find_devices() {
	struct usb_bus *libusb_bus;
	struct usb_device *libusb_dev;
	struct usb_version *version;
	unsigned int i=0;
	osbdm_devs[0]=NULL;				/* terminate the list */
	version = usb_get_version();
	if (version==NULL) {
		return -1;	/* something quite wrong... */
	}
	if (version->driver.major==-1) {
		/* driver not running! */
		//print("USB: Driver not running\r\n");
		return -1;
	}
	//print("USBLIB Driver version: %i.%i.%i.%i\r\n",version->driver.major, version->driver.minor,version->driver.micro, version->driver.nano);
	usb_find_busses();		/* enumerate all busses */
	usb_find_devices();		/* enumerate all devices */
	for (libusb_bus = usb_get_busses(); libusb_bus; libusb_bus = libusb_bus->next) {		/* scan through all busses */
		for (libusb_dev = libusb_bus->devices; libusb_dev; libusb_dev = libusb_dev->next) {	/* scan through all devices */
			// Check Vendor ID & Product ID
			#ifdef CHECK_VID_PID
			if ((libusb_dev->descriptor.idVendor==OSBDM_VID)&&(libusb_dev->descriptor.idProduct==product_id)) {
			#endif
			// Just check Vendor ID
			#ifdef CHECK_VID
			if ((libusb_dev->descriptor.idVendor==OSBDM_VID)) {
			#endif
			// Don't check any hardware ID
			#ifdef CHECK_NOTHING
			if (1) {
			#endif
				/* found a device */
				osbdm_devs[i++]=libusb_dev;	/* add pointer to the device */
				osbdm_devs[i]=NULL;			/* terminate the list again */
				if (i>=(sizeof(osbdm_devs)/sizeof(struct usb_device*))) return i;	/* too many devices! */
			}
		}
	}
	return i;
}

/* open connection to device enumerated by osbdm_usb_find_devices
   returns 0 on success and 1 on error

	The libusb API ties an open device to a specific interface.
	This means that if you want to claim multiple interfaces on a device, you should open
	the device multiple times to receive one usb_dev_handle for each interface you want to
	communicate with
*/
unsigned char osbdm_usb_open(unsigned int device_no) {
	osbdm_devh = usb_open(osbdm_devs[device_no]);
	if (osbdm_devh==NULL) return (1);
	if (usb_set_configuration(osbdm_devh,1)) return(2);		// only one valid configuration
	if (usb_claim_interface(osbdm_devh,0)) return(3);		// only 1 interface
	return (0);
}
// closes connection to the currently open device
void osbdm_usb_close(void) {
	if (osbdm_devh!=NULL) {
		usb_release_interface(osbdm_devh,0);		// release the interface
		usb_close(osbdm_devh);						// close the device
		osbdm_devh=NULL;							// indicate that no device is open
	}
}



//-----------------------------------------------------------------------------------------
/*
  sends a message to the OSBDM device over EP1
	global "usbBulkWritten" is set with the number of bytes that were sent
	returns 0 on success and 1 on error
*/
int osbdm_usb_send_ep1(int count, unsigned char *data) {
	if (osbdm_devh==NULL) 	return 1;	// ignore if no device handler
	usbBulkWritten=usb_bulk_write(osbdm_devh, 0x01, data, count, TIMEOUT);
	if (usbBulkWritten<0) return 1; else return 0;
}
/*
  receives data from EP2
	global "usbBulkRead" is set with the number of bytes read
	returns 0 on success and 1 on error
*/
int osbdm_usb_recv_ep2(int count, unsigned char * data) {
	if (osbdm_devh==NULL) 	return 1;	// ignore if no device handler
	usbBulkRead=usb_bulk_read(osbdm_devh, 0x82, data, count, TIMEOUT);
	if (usbBulkRead<0) return 1; else return 0;
}
