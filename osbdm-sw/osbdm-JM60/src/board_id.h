/* OSBDM-JM60 Read Board ID (If Exists)
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


#ifndef _BOARD_ID_H_
#define _BOARD_ID_H_


unsigned char read_board_id(void); // also places result in "board_id"
unsigned char read_osbdm_id(void); // also places result in "osbdm_id"

extern unsigned char board_id;
extern unsigned char osbdm_id;


#endif // _BDM_CF_H_
