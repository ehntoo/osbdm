osbdm directory description:
============================

1) osbdm-common:	Files shared between OSBDM PC software and JM60 firmware. 	
2) osbdm-JM60:		JM60 USB/OSBDM firmware for each supported target device. It converts OSBDM 
			USB commands from the PC into BDC or JTAG debug signals. 
3) osbdm-pc:		PC software consisting of OSBDM USB driver and Windows USB driver. OSBDM USB 
			driver interfaces with the OSBDM GDI driver, providing function calls to the 
			Windows USB driver.  The Windows USB driver (libusb.lib) interfaces with PC 
			USB ports.
4) osbdm-tester:	OSBDM USB command set tester which tests the interface between JM60 firmware 
			and OSBDM/USB drivers on the PC.

