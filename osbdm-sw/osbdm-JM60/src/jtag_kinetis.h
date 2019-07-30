/* OSBDM-JM60 Target Interface Software Package
 * Portions Copyright (C) 2011 P&E Microcomputer Systems, Inc.
 * Portions Copyright (C) 2009 Freescale Semiconductor
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
/*
* 03/08/2011  : Created by P&E Microcomputer Systems
*               http://www.pemicro.com/osbdm
*
*
*/


#ifndef _JTAG_KINETIS_H_
#define _JTAG_KINETIS_H_

#define OUT_EN			PTED_PTED7
#define OUT_EN_MASK		PTED_PTED7_MASK
#define OUT_EN_DIR		PTEDD_PTEDD7

#define RTS				PTGD_PTGD2
#define RTS_MASK		PTGD_PTGD2_MASK
#define RTS_DIR			PTGDD_PTGDD2

#define Reg_Core_PC_index         0x0F        //Same as R15
#define Reg_Core_xPSR_index       0x10
#define Reg_Core_MSP_index        0x11
#define Reg_Core_PSP_index        0x12

#define Reg_DP_CTRLSTAT_address   0x04
#define Reg_DP_SELECT_address     0x08
#define Reg_DP_RDBUFF_address     0x0C

//Freescale Pioneer ONLY
#define Reg_MDM_Status_address    0x00
#define Reg_MDM_Control_address   0x04
#define Reg_MDM_IDR_address       0xFC

#define DPACC_APACC_READ          0x01
#define DPACC_APACC_WRITE         0x00

#define AP_MDM                    0x01
#define AP_AHB                    0x00
#define AP_APB                    0x01

#define Reg_AHB_CSW_address       0x00
#define Reg_AHB_TAR_address       0x04
#define Reg_AHB_DRW_address       0x0C
#define Reg_AHB_BD0_address       0x10
#define Reg_AHB_BD1_address       0x14
#define Reg_AHB_BD2_address       0x18
#define Reg_AHB_BD3_address       0x1C
#define Reg_AHB_ROM_address       0xF8
#define Reg_AHB_IDR_address       0xFC

#define Reg_CoreDebug_DHCSR_address      0xE000EDF0
#define Reg_CoreDebug_DCRSR_address      0xE000EDF4
#define Reg_CoreDebug_DCRDR_address      0xE000EDF8
#define Reg_CoreDebug_DEMCR_address      0xE000EDFC
//---------------//

typedef enum osbdm_error_tag {
    osbdm_error_ok,
    osbdm_error_fail,
    osbdm_error_invalid_parameter,
    osbdm_error_internal_failure,
    osbdm_error_undefined
} osbdm_error;


#endif
