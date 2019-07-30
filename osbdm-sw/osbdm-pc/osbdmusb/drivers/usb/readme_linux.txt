OSBDM-JM60 USB Driver Manual Installation:
==========================================

Supported Devices:
	OSBDM-JM60 

Supported Operating Systems:
	Red Hat Linux 5.0

Driver Type:
	libusb Version 0.1.12

Installation Instructions

	Please note that the following procedures assume that the user
        has the Administrator privileges.

	1.  Copy OSBDM-JM60 USB shared library ("libosbdm-jm60.so")  to 
            /usr/local/lib. 
        
            "libosbdm-jm60.so" can be found from:

            {OSBDMDistRootDir}/osbdm-sw/osbdm-pc/osbdmusb/drivers/usb/linux.

	2.  Install udev rules

	    Copy "00-osbdm-jm60.rules" to /etc/udev/rules.d

            "00-osbdm-jm60.rules" can be found from:

            {OSBDMDistRootDir}/osbdm-sw/osbdm-pc/osbdmusb/drivers/usb/linux.
	
        3.  Activate new udev rules

	    Run the command 'udevcontrol reload_rules'


	4.  Plug in OSBDM-JM60

	    NOTE: If you already had your OSBDM-JM60 plugged in prior to performing
	    the above modifications, you will need to unplug them and plug them in
	    again in order for the changes to take effect. 


