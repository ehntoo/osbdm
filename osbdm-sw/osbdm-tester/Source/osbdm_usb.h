
void osbdm_usb_init();
int osbdm_usb_find_devices();
unsigned char osbdm_usb_open(unsigned int device_no);
void osbdm_usb_close(void);
int osbdm_usb_send_ep1(int count, unsigned char *data);
int osbdm_usb_recv_ep2(int count, unsigned char * data);

extern int usbBulkWritten;	// number of bytes written by usb endpoint
extern int usbBulkRead;	// number of bytes read by usb endpoint

