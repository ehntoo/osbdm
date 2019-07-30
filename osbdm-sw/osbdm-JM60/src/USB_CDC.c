#include"Usb_Ep0_Handler.h"
#include"USB_CDC.h"
#include"serial_io.h"
#include"usb_drv.h"


/* CDC Global Structures */
CDC_Line_Coding LineCoding;
unsigned char u8CDCState=0;
//UINT8 CDC_OUT_Data[CDC_BUFFER_SIZE];


/* USB Variables & Flags */
//extern UINT8 gu8USB_Flags; 
//extern UINT8 gu8USB_State;              
//extern tUSB_Setup *_Setup_Pkt;
//extern UINT8 gu8EP2_Buffer[];
//extern UINT8 gu8EP3_Buffer[];
//extern tBDT tBDTtable[];
//extern UINT8 gu8Interface;


/* cHeck */
//UINT8 u8RecData;

/**********************************************************/
void CDC_Init()
{
    
    /* Line Coding Initialization */
    LineCoding.DTERate=LWordSwap(115200);
    LineCoding.CharFormat=0;
    LineCoding.ParityType=0;
    LineCoding.Databits=0x08;
   
}


/**********************************************************/
void CDC_Engine(void)
{
    /* control Stage */
  
    switch(u8CDCState)
    {
        case SET_LINE_CODING:
            if (Total_Received_data>=7) {
            u8CDCState=0;
            // The Line Coding has been set. Initialize serial port
            SCI_Init_CDC(LineCoding);
            }
            break;
      
    }
  


	// If serial data is available from PC then send it to the SCI!!!
	if (
	   (sci_virtual_serial_port_is_enabled) &&
		 (Bdtmap.ep3Bio.Stat.McuCtlBit.OWN == 0) // Data Available from PC
     ) 
     {
       if (EP3_BufferLastReceivedDataSize>0){
         SCI_SendBuffer(EP3_BufferLastReceivedDataSize, &(EP3_Buffer[0]));
     }    
	   USB_Buf_Rdy(&EP3_Set);					// return EP control to SIE
     }


	               
	// If serial data is available from SCI then send it to the PC!!!
	if (
	   (sci_virtual_serial_port_is_enabled) &&
		 (Bdtmap.ep4Bio.Stat.McuCtlBit.OWN == 0) &&    // Is USB transmit empty AND
		 (SCI_CharReady()==TRUE)           // Serial Data Available 
     ) {
	  
	     int numbytes = 0;
	     do {
  	       EP4_Buffer[numbytes] = SCI_GetChar();
	         numbytes++;
	     }
	     while ((SCI_CharReady()==TRUE)&&(numbytes<UEP4_SIZE));
     EndPoint_IN(EP4,numbytes);
     }

}



/**********************************************************/
unsigned long LWordSwap(unsigned long u32DataSwap)
{
    unsigned long u32Temp;
    u32Temp= (u32DataSwap & 0xFF000000) >> 24;
    u32Temp+=(u32DataSwap & 0xFF0000)   >> 8;
    u32Temp+=(u32DataSwap & 0xFF00)     << 8;
    u32Temp+=(u32DataSwap & 0xFF)       << 24;
    return(u32Temp);    
}
