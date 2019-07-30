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
 /*****************************************************************************
 * File name   : Usb_Drv.c
 * Project name: JM60 Evaluation code
 *
 * Description : This software evaluates JM60 USB module 
 *               
 *
 * History     :
 * 04/01/2007  : Initial Development
 * 03/08/2011  : Modified by P&E Microcomputer Systems
 *               http://www.pemicro.com/osbdm
 * 
 *****************************************************************************/
#include "Usb_Drv.h"
#include "hidef.h"

byte Usb_Device_State;				// device states
USB_DEVICE_STATUS Usb_Stat;		// global flags
byte Usb_Active_Cfg;					// value of current configuration
byte Usb_Alt_Intf[MAX_NUM_INTF];	// array to keep track of the current alternate
											// setting for each interface ID

// buffer descriptor table entry
//USB RAM  0x1860 - 0x195F (256 bytes)

BDTMAP Bdtmap @ 0x1860;

//endpoint 0 buffer definition
CTRL_TRANSFER_DATA CtrlTrf_Data @0x1880;
CTRL_TRANSFER_SETUP Setup_Pkt @0x1890;


void USB_Suspend(void);
void USB_WakeFrom_Suspend(void);
unsigned char USB_WakeUp(void );

void USB_Bus_Reset_Handler(void);
void USB_SOF_Handler(void);
void USB_Stall_Handler(void);
void USB_Error_Handler(void);


/******************************************************************************
 * Function:        void Initialize_USBModule(void)
 * Input:           None
 * Output:          None
 * Overview:        initialize the USB module
 *                  enable regulator, PHY, pull-up resistor, BDT initialization
 *
 * Note:            None
 *****************************************************************************/
void Initialize_USBModule(){
	/*char i;
	unsigned char *pUsbMem = (unsigned char *) &Bdtmap;*/

  
	Usb_Device_State = POWERED_STATE;
	#ifdef SELF_POWER
		Usb_Stat._byte = 0x01;		// SelfPower
	#else
		Usb_Stat._byte = 0x00;		// BusPower
	#endif
	Usb_Active_Cfg = 0x00;

	USBCTL0 = USB_RST;				// reset USB module
	while(USBCTL0_USBRESET);		// wait for USB reset to complete
  
  
	// Initialize the Buffer Descriptor Table (BDT)
	Bdtmap.ep0Bo.Stat._byte = _SIE|_DATA0|_DTS;	// init EP0 OUT
	Bdtmap.ep0Bo.Cnt = EP0_BUFF_SIZE;				// EP0 OUT packet size              
	Bdtmap.ep0Bo.Addr = EPADR0_OUT;					// EP0 OUT buffer address offset
  																// calculate this value using
  																//((byte)(((byte*) &Setup_Pkt)-0x1860)  )>> 2;    

	Bdtmap.ep0Bi.Stat._byte = _CPU;					// init EP0 IN
	Bdtmap.ep0Bi.Cnt = 0x00;							// host reports packet size
	Bdtmap.ep0Bi.Addr = EPADR0_IN;					// EP0 IN buffer address offset
  																// calculate this value using
  																//((byte)(((byte*) &Setup_Pkt)-0x1860)  )>> 2;

	USBCTL0 = 0x00;					// disable USB module
  
	EPCTL0 = 0x0D;						// configure and enable EP0

	INTENB = 0x01;						// enable USB RESET interrupt
}



/******************************************************************************
 * Function:        void Check_USBBus_Status(void)
 * Input:           None
 * Output:          None
 * Overview:        This function can be used to detect if the USB bus has 
 *                  attached on USB bus, we can use a GPIO, or KBI interrupt, 
 *                  it is disable here
 *
 *****************************************************************************/
void Check_USBBus_Status(void){

	if(CTL_USBEN == 0){			// if module disabled, then 
		EPCTL0 = 0x0D;
		INTSTAT = 0xBF;			// clear interrupt flags
		CTL = 0x00;					// disable module
		INTENB = 0x00;				// disable interrupt
		CTL_USBEN = 0x01;			// enable module
		USBCTL0 = UCFG_VAL;		// attach JM60 to the USB bus
		Usb_Device_State = ATTACHED_STATE;	// update state variable      
	}
   
	if(Usb_Device_State == ATTACHED_STATE){
		INTSTAT = 0xBF;				// clear interrupt flags
 		INTENB = 0xBB;					// enable interrupts
  											// SOFTOK interrupt not enabled
	}
  
	if(Usb_Device_State == USB_ENTER_SUSPEND){
		USB_Suspend();					// call suspend function 
	}	
}
/*
void ConfigureUSBDevice(){
		INTSTAT = 0xBF;			  // clear interrupt flags
		INTENB = 0x00;				// disable USB interrupt

  // this will be the only valid configuration
  Clear_Mem((byte*)&EPCTL1,6);                          
  Clear_Mem((byte*)&Usb_Alt_Intf,MAX_NUM_INTF);

 	Usb_Active_Cfg = 0x01;  
  Ctrl_Trf_Session_Owner = STANDARD_CLASS_ID;

  Usb_Device_State = CONFIGURED_STATE;
  Endpoint_Init(); 

	INTSTAT = 0xBF;			// clear interrupt flags
  INTENB = 0xBB;					// enable USB interrupts		
}
*/

/******************************************************************************
 * Function:        void USB_Soft_Detach(void)
 * Input:           None
 * Output:          None
 * Overview:        USB_Soft_Detach disconnects the device from USB bus. 
 *                  This is done by stop supplying Vusb voltage to
 *                  pull-up resistor. The pull-down resistors on the host
 *                  side will pull both differential signal lines low and
 *                  the host registers the event as a disconnect.
 *
 *                  Since the USB cable is not physically disconnected, the
 *                  power supply through the cable can still be sensed by
 *                  the device. The next time Check_USBBus_Status() function
 *                  is called, it will reconnect the device back to the bus.
 *****************************************************************************/
void USB_Soft_Detach(void){
	CTL = 0x00;							// disable module
	INTENB = 0x00;						// mask interrupts
	Usb_Device_State = POWERED_STATE; // update state variable      
	return;
}



/******************************************************************************
 * Function:        void USB_Suspend(void)
 * PreCondition:    None
 * Input:           None
 * Output:          None
 * Side Effects:    None
 * Overview:        After 3ms of USB bus idle status, the SLEEPF will be set,
 *                  The firmware should enable the resume function in 7ms before 
 *                  the USB enters into suspend state.       
 * Note:            None
 *****************************************************************************/
void USB_Suspend(void){
	unsigned char Result;
   
//	INTENB_RESUME = 1;				// enable resume interrupt
	USBCTL0_USBRESMEN = 1;			// enable resume from low-power
                     
	INTSTAT_SLEEPF = 1;				// clear SLEEP interrupt flag                    
	//RTC_DISBALE();					// disable the RTC
   
	Usb_Device_State = USB_SUSPEND;	// suspend the device
	do{
		Result = USB_WakeUp();		// wait until wake-up is received 
	}while(!Result);
      
	USBCTL0_USBRESMEN = 0;			// clear low-power resume flag  
    
	if(Result == 2) USB_Remote_Wakeup();	// enable Remote WakeUp
                          
//	RTC_ENABLE();						// restart RTC
	return;
}



/******************************************************************************
 * Function:        void USB_WakeFrom_Suspend(void)
 * Input:           None
 * Output:          None
 * Overview:        
 *****************************************************************************/
void USB_WakeFrom_Suspend(void)
{
	INTSTAT_RESUMEF = 1;				// clear resume flag
	INTENB_RESUME = 0;				// enable resume interrupt
	CTL_TSUSPEND = 0;					// enable SIE to continue processin packets
}
/******************************************************************************
 * Function:        void USB_Remote_Wakeup(void)
 * Input:           None
 * Output:          None
 * Overview:        This function is used to send wake-up signal from device to
 *                  host or hub
 *
 * Note:           According to USB 2.0 specification section 7.1.7.7,
 *                  "The remote wakeup device must hold the resume signaling
 *                  for at lest 1 ms but for no more than 15 ms."
 *****************************************************************************/
void USB_Remote_Wakeup(void){
	static word delay_count;

	USB_WakeFrom_Suspend();			// wake from suspend, see above    
     
	CTL_CRESUME = 1;					// enable REMOTE_WAKEUP signalling
       
	delay_count = 8000;				// REMOTE_WAKEUP signalling delay
	do{
		delay_count--;
		__RESET_WATCHDOG();
	} while(delay_count);        
        
	CTL_CRESUME = 0;					// disable REMOTE_WAKEUP signalling
}

/******************************************************************************
 * Function:        void USB_SOF_Handler(void)
 * Input:           None
 *
 * Output:          None
 * Overview:        The USB host sends out a SOF packet to full-speed devices
 *                  every 1 ms. This interrupt may be useful for isochronous
 *                  pipes. 
 *****************************************************************************/
void USB_SOF_Handler(void)
{
	// add code for isochronous transfer handling
    
	INTSTAT_SOFTOKF = 1;				// clear the interrupt flag
}



/******************************************************************************
 * Function:        void USB_Stall_Handler(void)
 * Input:           None
 * Output:          None
 * Overview:        The STALLIF is set anytime the SIE sends out a STALL
 *                  packet regardless of which endpoint causes it.
 *                  A Setup transaction overrides the STALL function. A stalled
 *                  endpoint stops stalling once it receives a setup packet.
 *
 *                  There are a few reasons for an endpoint to be stalled.
 *                  1. When a non-supported USB request is received.
 *                     Example: GET_DESCRIPTOR(DEVICE_QUALIFIER)
 *                  2. When an endpoint is currently halted.
 *                  3. When the device class specifies that an endpoint must
 *                     stall in response to a specific event.
 *
 *****************************************************************************/
void USB_Stall_Handler(void){

	if((STAT & 0xF0)== 0x00){		// EP0 STALL
      if(EPCTL0_EPSTALL == 1) 
      	EPCTL0_EPSTALL = 0;
      
		//Notice if is generated by BDTSTALL, the firmware should notice the IN or OUT DIR
		//we only set it for OUT direction in this demo
		USB_Prepare_Next_Trf();     
	} else {
      ;//Add processes for STALL generated by other endpoints (even or odd)
     }
    
    INTSTAT_STALLF = 1;				// clear interrupt flag
}



/******************************************************************************
 * Function:        void USB_Error_Handler(void)
 * Input:           None
 * Output:          None
 * Overview:        You can check error register to see which error occured
 * Note:            None
 *****************************************************************************/
void USB_Error_Handler(void){

	INTSTAT_ERRORF = 1;
	// add code to check the error source

	if((ERRSTAT_BUFERRF) && ((STAT & 0xF0)==0)){	// Bdt buffer overflow
		USB_Prepare_Next_Trf();  
	}
      
    /*check for other errors here */
      
	ERRSTAT = 0xBF;					// clear error flags 
	return;
}



/******************************************************************************
 * Function:        void USB_Bus_Reset_Handler(void)
 *
 * Input:           None
 * Output:          None
 *
 * Overview:        Once a USB bus reset is received from the host, this
 *                  function should be called. It resets the device address to
 *                  zero, disables all non-EP0 endpoints, initializes EP0 to
 *                  be ready for default communication, clears all USB
 *                  interrupt flags, unmasks applicable USB interrupts, and
 *                  reinitializes internal state-machine.
 *****************************************************************************/
void USB_Bus_Reset_Handler(void){

	ERRSTAT = 0xFF;					// clear USB error flags
	INTSTAT = 0xBF;					// clear USB interrupt flags
	ERRENB = 0xBF;						// enable all USB error interrupt sources
	INTENB = 0x9B;						// enable all interrupts except RESUME and SOFTOK
	
	ADDR = 0x00;						// reset to default address
	Clear_Mem((byte*)&EPCTL1,6);	// disable all endpoints
	EPCTL0 = EP_CTRL|HSHK_EN;		// enable endpoint 0*/

	while(INTSTAT_TOKDNEF)			// Flush pending transactions 
		INTSTAT = 0xBF;              

	USB_Prepare_Next_Trf();			// prepare to receive the setup packet 

  	
	Usb_Stat.BitCtl.RemoteWakeup = 0;	// default status flag to disable 
	Usb_Active_Cfg = 0;                    
	Usb_Device_State = DEFAULT_STATE;	// update state variable
}



/******************************************************************************
 * Function:        void USB_Buf_Rdy(buffer_dsc)
 *
 * PreCondition:    IN endpoint: Buffer is control by MCU.
 *                  OUT endpoint: Buffer is conrolled by SIE.
 *
 * Input:           byte buffer_dsc: one buffer descriptor in Bdtmap
 * Output:          None
 *
 * Overview:        This function turns the buffer ownership to SIE,
 *                  and toggles the DTS bit for synchronization.
 * Note:            This function should not be called by Endpoint 5 or 6
 *                  because they are pingpong buffer, DATA0/1 do not need 
 *                  to be switched
 *****************************************************************************/
void USB_Buf_Rdy(BUFF_DSC *buffer_dsc){

	buffer_dsc->Stat._byte &= _DATA1;												// set Status bits except DATA1          
	buffer_dsc->Stat.McuCtlBit.DATA = ~ buffer_dsc->Stat.McuCtlBit.DATA;	// toggle DATA0/1 bit*/
	buffer_dsc->Stat._byte |= _SIE|_DTS;											// SIE owns buffer
}



/******************************************************************************
 * Function:        void Clear_Mem(byte* startAdr,byte count)
 * Input:          
 * Output:          None
 * Overview:        None
 *****************************************************************************/
void Clear_Mem(byte* startAdr,byte count){

	byte i;
        
	for(i=0; i < count; i++){
		*(startAdr + i) = 0x00;		// write 0x00 to memory location
	}
	return;
}

/*
#define max_debug_buf 16
unsigned char pedebug_last_stat[max_debug_buf];
unsigned char pedebug_last_intstat[max_debug_buf];
unsigned char pedebug_last_errstat[max_debug_buf];
unsigned char pedebug_last_stat_before_clear_tokdnef[max_debug_buf];
unsigned char pedebug_last_stat_after_clear_tokdnef[max_debug_buf];
unsigned char pedebug_end_intstat[max_debug_buf];
unsigned char debug_last_current_loop = 0;
*/


/******************************************************************************
 * Function:       void interrupt  USB_ISR()
 * Input:          
 * Output:          None
 * Overview:        The USB stat interrupt service routine
 * Note:            None
 *****************************************************************************/
//void interrupt 7 USB_ISR(){
void interrupt USB_ISR(){

  SCI_get_pending_chars_during_an_interrupt();	

/*
  pedebug_last_stat[debug_last_current_loop] = STAT;
  pedebug_last_intstat[debug_last_current_loop] = INTSTAT;
  pedebug_last_errstat[debug_last_current_loop] = ERRSTAT;
*/
   
	if((USBCTL0_LPRESF) && (Usb_Device_State == USB_SUSPEND)){	// LPRESF interrupt
      USBCTL0_USBRESMEN = 0;		// resume from low-power suspend
	}
  
	if(INTSTAT_RESUMEF && INTENB_RESUME){	// RESUME interrupt
		USB_WakeFrom_Suspend();
	}
     	
	if(INTSTAT_USBRSTF && INTENB_USBRST){	// USB RESET interrupt 
		USB_Bus_Reset_Handler();
	}

	if(INTSTAT_SOFTOKF && INTENB_SOFTOK){	// SOF Token interrupt
		USB_SOF_Handler();
	}
    
	if(INTSTAT_STALLF && INTENB_STALL ){	// STALL interrupt
		USB_Stall_Handler();
	}
    
	if(INTSTAT_ERRORF && INTENB_ERROR){		// ERROR interrupt
		USB_Error_Handler(); 
	}
     
	if((INTSTAT_SLEEPF && INTENB_SLEEP)&&(Usb_Device_State >= CONFIGURED_STATE)){	// SLEEP interrupt  
		Usb_Device_State = USB_ENTER_SUSPEND;	// update state variable 
		INTSTAT_SLEEPF = 1;							// clear SLEEP flag
	}
                
// pedebug_last_stat_before_clear_tokdnef[debug_last_current_loop] = STAT;

	if(INTSTAT_TOKDNEF && INTENB_TOKDNE){	// Token Complete interrupt
		USB_Transaction_Handler();
    SCI_get_pending_chars_during_an_interrupt();	
		INTSTAT_TOKDNEF = 1;						// clear interrupt flag
		//if
	}
/*
pedebug_last_stat_after_clear_tokdnef[debug_last_current_loop] = STAT;
pedebug_end_intstat[debug_last_current_loop] = INTSTAT;
  
debug_last_current_loop = (debug_last_current_loop + 1) % max_debug_buf;
*/
  	return;
}



/******************************************************************************
 * Function:       void interrupt  IRQ_ISR()
 * Input:          
 * Output:          None
 * Overview:        The IRQ is used to wakup the USB and MCU.
 * Note:            None
 *****************************************************************************/
//void interrupt 2 IRQ_ISR(){
void interrupt IRQ_ISR(){
   IRQSC_IRQACK = 1;					// ACK interrupt
   return;
}



/******************************************************************************
 * Function:        unsigned char USB_WakeUp()
 * Input:          
 * Output:          0: noise, re-entern into sleep
 *                  1: Bus wake-up
 *                  2: remote wake up
 * Overview:        The USB sleep and wake up
 * Note:            None
 *****************************************************************************/
 unsigned char USB_WakeUp(void )
 {
	int delay_count;
   
	asm STOP;							// enter into stop3
    
	Usb_Device_State = CONFIGURED_STATE;	// update state variable
    
	if((! USBCTL0_LPRESF) /*&& (Kbi_Stat)*/){	// is the KBI interrupt bring the MCU out of STOP3
		if(Usb_Stat.BitCtl.RemoteWakeup == 1)	// is remote wakeup supported
			return 2;									// will enable remote wakeup
		else
			return 0;
	} else {								//add delay to filter make sure input is real
		delay_count = 50;               
  		do {
			delay_count--;
			__RESET_WATCHDOG();
		} while(delay_count);        
        
	  	if(INTSTAT_RESUMEF){			// if input set hte RESUMEF bit, then 
			return 1;					// USB resume
		} else return 0;				// input is caused by noise
	}
}      
