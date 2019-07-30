/******************************************************************************
  Copyright (c) 2007 Freescale Semiconductor
  Freescale Confidential Proprietary
  \file     	USB_User_API.c
  \brief    	Functions for the USB API
  \author   	Freescale Semiconductor
  \author     Jose Ruiz
  \author   	Guadalajara Applications Laboratory RTAC Americas
  \version    0.3
  \date     	17/October/2007
  \warning    (If needed)

  * History:
  
  30/July/2007      Start of bridge coding
  15/October/2007   Modifications for new version of bridges
  17/October/2007   Macros added and funcion names changed
  06/December/2007  Added variables to easily know the                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          
*
* 03/08/2011  : Modified by P&E Microcomputer Systems
*               Support for multiple interfaces added
*               http://www.pemicro.com/osbdm
*
*
*
******************************************************************************/

#include "USB_User_API.h"

// Global Variables 

// User Buffers
UINT8 EP1_Buffer[UEP1_SIZE];
UINT8 EP2_Buffer[UEP2_SIZE];
UINT8 EP3_Buffer[UEP3_SIZE];
UINT8 EP4_Buffer[UEP4_SIZE];

UINT8 EP1_BufferLastReceivedDataSize;
UINT8 EP2_BufferLastReceivedDataSize;
UINT8 EP3_BufferLastReceivedDataSize;
UINT8 EP4_BufferLastReceivedDataSize;


// User Out buffer recieved data size
UINT8 EP1_OUT_SIZE;
UINT8 EP2_OUT_SIZE;
UINT8 EP3_OUT_SIZE;
UINT8 EP4_OUT_SIZE;

// Shared Buffers (SIE & Core)
UINT8 EP1_BDT_Buffer[UEP1_SIZE] @ 0x18A0;	// 64 bytes
UINT8 EP2_BDT_Buffer[UEP2_SIZE] @ 0x18E0;	// 64 bytes
UINT8 EP3_BDT_Buffer[UEP3_SIZE] @ 0x1920;	// 32 bytes
UINT8 EP4_BDT_Buffer[UEP4_SIZE] @ 0x1940;	// 32 bytes



/*****************************************************************************/
void Endpoint_Init(void)
{   
	// EndPoint 1 Configuration
	EPCTL1 = EP_OUT|HSHK_EN;		// EP is OUT w/ handshake enabled                
	EP1_Set.Cnt = UEP1_SIZE; 		// max data size in bytes
	EP1_Set.Addr = EPADR1_OUT;		// EP buffer offset address
	EP1_Set.Stat._byte = _SIE|_DATA0|_DTS;		// SIE owns buffer, set DATA0, enable DTS
    
	// EndPoint 2 Configuration
	EPCTL2 = EP_IN|HSHK_EN;			// EP is IN w/ handshake enabled    
	EP2_Set.Addr = EPADR2_IN;		// EP buffer offset address
	EP2_Set.Stat._byte = _CPU|_DATA1;	// MCU owns buffer, set DATA1
    
    // EndPoint 3 Configuration
	EPCTL3 = EP_OUT|HSHK_EN;		// EP is OUT w/ handshake enabled               
	EP3_Set.Cnt = UEP3_SIZE; 		// max data size in bytes   
	EP3_Set.Addr = EPADR3_OUT;		// EP buffer offset address
	EP3_Set.Stat._byte = _SIE|_DATA0|_DTS;		// SIE owns buffer, set DATA0, enable DTS
    
	// EndPoint 4 Configuration
	EPCTL4 = EP_IN|HSHK_EN;			// EP is IN w/ handshake enabled                 
	EP4_Set.Addr = EPADR4_IN;		// EP buffer offset address
	EP4_Set.Stat._byte = _CPU|_DATA1;	// MCU owns buffer, set DATA1

	// EndPoint 4 Configuration
	EPCTL5 = EP_IN|HSHK_EN;			// EP is IN w/ handshake enabled                 
	EP5Even_Set.Addr = EPADR5_IN;		// EP buffer offset address
	EP5Even_Set.Stat._byte = _CPU|_DATA1;	// MCU owns buffer, set DATA1

}



/*****************************************************************************/


/*****************************************************************************/
void EndPoint_IN(UINT8 u8Endp,UINT8 u8EPSize)
{
	BUFF_DSC	*pu8BDT;
	UINT8		*pu8UserBuffer;
	UINT8		*pu8SIEBuffer;
	UINT8		u8Counter;
    
	switch(u8Endp) 
	{
		case 1:
			pu8BDT=&EP1_Set;
			pu8UserBuffer=&EP1_Buffer[0];
			pu8SIEBuffer=&EP1_BDT_Buffer[0];
			break;  

		case 2:
			pu8BDT=&EP2_Set;
			pu8UserBuffer=&EP2_Buffer[0];
			pu8SIEBuffer=&EP2_BDT_Buffer[0];
			break;  
        
		case 3:
			pu8BDT=&EP3_Set;
			pu8UserBuffer=&EP3_Buffer[0];
			pu8SIEBuffer=&EP3_BDT_Buffer[0];
			break;
        
		case 4:
			pu8BDT=&EP4_Set;
			pu8UserBuffer=&EP4_Buffer[0];
			pu8SIEBuffer=&EP4_BDT_Buffer[0];
			break;
	}
      
	/* Copy buffer to USB RAM */
	for (u8Counter = 0; u8Counter < u8EPSize; u8Counter++)
		*pu8SIEBuffer++ = *pu8UserBuffer++;
    
	/* Set Packet Size and activate EP */
	pu8BDT->Cnt = u8EPSize;
	USB_Buf_Rdy(pu8BDT);

}



/*****************************************************************************/
void EndPoint_OUT(UINT8 u8Endp)
{
	BUFF_DSC	*pu8BDT;
	UINT8		*pu8UserBuffer;
	UINT8		*pu8SIEBuffer;
	UINT8		u8Counter;
  byte Ep_Out_Size;
    
  SCI_get_pending_chars_during_an_interrupt();	
	switch(u8Endp) 
	{
		case 1:
			pu8BDT = &EP1_Set;
			pu8UserBuffer = &EP1_Buffer[0];
			pu8SIEBuffer = &EP1_BDT_Buffer[0];			
			Ep_Out_Size = pu8BDT->Cnt;
			EP1_BufferLastReceivedDataSize = pu8BDT->Cnt;
			pu8BDT->Cnt = UEP1_SIZE;	// reset max data size  
			break;

		case 2:
			pu8BDT = &EP2_Set;
			pu8UserBuffer = &EP2_Buffer[0];
			pu8SIEBuffer = &EP2_BDT_Buffer[0];
  			Ep_Out_Size = pu8BDT->Cnt;
			EP2_BufferLastReceivedDataSize = pu8BDT->Cnt;
			pu8BDT->Cnt = UEP2_SIZE;	// reset max data size  
  			break;
        
		case 3:
			pu8BDT = &EP3_Set;
			pu8UserBuffer = &EP3_Buffer[0];
			pu8SIEBuffer = &EP3_BDT_Buffer[0];
  			Ep_Out_Size = pu8BDT->Cnt;
			EP3_BufferLastReceivedDataSize = pu8BDT->Cnt;
			pu8BDT->Cnt = UEP3_SIZE;	// reset max data size  
  			break;
        
		case 4:
			pu8BDT = &EP4_Set;
			pu8UserBuffer = &EP4_Buffer[0];
  			pu8SIEBuffer = &EP4_BDT_Buffer[0];
			Ep_Out_Size = pu8BDT->Cnt;
			EP4_BufferLastReceivedDataSize = pu8BDT->Cnt;
			pu8BDT->Cnt = UEP4_SIZE;	// reset max data size  
			break;
	}
    
  SCI_get_pending_chars_during_an_interrupt();	
	if(!pu8BDT->Stat.McuCtlBit.OWN)
	{
		/* Copy USB buffer to User RAM */
		for (u8Counter = 0; u8Counter < Ep_Out_Size; u8Counter++) {
		  *pu8UserBuffer++ = *pu8SIEBuffer++;	// copy EP buffer to RAM
      SCI_get_pending_chars_during_an_interrupt();	
		}

	if (u8Endp!=3) {
	  // We do not re-enable Endpoint 3 since we need to make sure we
	  // can process the information (outside of an interrupt) before
	  // a new packet comes in from the PC
	  USB_Buf_Rdy(pu8BDT);					// return EP control to SIE
	}
	}
}


