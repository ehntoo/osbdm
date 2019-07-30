/* OSBDM-JM60 Read Board ID 
 * Copyright (C) 2011 P&E Microcomputer Systems, Inc.
 * http://www.pemicro.com
 *
 * This software package is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this software; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */


/* Overview
 * 
 * Newer OSBDM/OSJTAG Designs reserve the ADP4:ADP2 ADC channels for reading
 * an analog board ID on the JM60 processor. The idea is that if all development
 * boards are uniquely ID'd, we can make changes in the firmware to account for
 * discrepancies between them. If a board is designed without the desire for a 
 * board ID, then these lines (if unused) should be pulled to Vss or Vdd. A board
 * ID may be requested from Freescale/P&E and should be included in new tower cards
 * where possible.
 */

#include "derivative.h"				// include peripheral declarations  
#include "typedef.h"				// Include peripheral declarations  

unsigned char board_id = 0;
unsigned char osbdm_id = 0;   // used to identify version of OSBDM hardware
unsigned char bid1 = 0;
unsigned char bid0 = 0;

static const unsigned char  adc_conv[] = 
    {
    // convert high 4 bits of the ADC result into a number 0-8
    // board ID is decimal equivalent of 9*HighVal + LowVal resulting in a
    // board ID of 0-81
    0,         
    1,1,
    2,2,
    3,3,
    4,4,
    5,5,
    6,6,
    7,7,
    8
    };

unsigned char read_board_id(void) // also places result in "board_id"
{

bid0 = 0;
bid1 = 0;
board_id = 0;

#if defined __CFV2__
   // CFV234 do not support the version mechanism
#else
ADCCFG = 0x13;   // Internal ADC Clock  (00000011)

ADCSC1 = 0x02;   // initiate an ADC conversion Channel 2
while ((ADCSC1 & 0x80) != 0x80) ;
bid0 = ADCRL;

ADCSC1 = 0x04;   // initiate an ADC conversion Channel 4 
while ((ADCSC1 & 0x80) != 0x80) ;
bid1 = ADCRL;

board_id = adc_conv[((bid1&0xF0)>>4)]*9 + adc_conv[((bid0&0xF0)>>4)];
#endif

return board_id;
}

unsigned char read_osbdm_id(void) // also places result in osbdm_id
{ 

  char hold_current_value = 0;

  osbdm_id = 0;
#if defined __CFV2__
   // CFV234 do not support the version mechanism
#else 
   hold_current_value = PTCPE; 
   
   PTCPE |= 0x07;
   wait_ms(2);
   
   if ((PTCD & 0x07) == 0x07) {
      /* nothing is connected to the OSBDM_ID pins */ 
      osbdm_id = 0;
   }
   else {
   osbdm_id = PTCD & 0x07;
   }

   /* restore pull-up value */ 
   PTCPE = hold_current_value;
   wait_ms(2);

 
#endif  
   return osbdm_id; 
}

