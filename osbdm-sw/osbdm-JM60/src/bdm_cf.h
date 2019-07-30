/* OSBDM-JM60 Target Interface Software Package
 * Copyright (C) 2009  Freescale
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


/******************************************************************************
*
*	Author:		Axiom Manufacturing
*
*	File:			bdm_cf.h
*
*	Purpose: 	header file for bdm_cf.c file
*
*
* 03/08/2011  : Modified by P&E Microcomputer Systems
*               http://www.pemicro.com/osbdm
*
*
*
******************************************************************************/

#ifndef _BDM_CF_H_
#define _BDM_CF_H_

//----------------------------------------------------------------------------
// BDM functions

extern word t_txrx(word spdata);	// sends a word of data out SPI port and receives a word or data 

int bdm_rbdreg(word cmd, dword *rptr);	// read Debug Module Register

extern byte bdm_wbdreg(word cmd, dword data); // write Debug Module Register

void cf_reset_nosync();					// target reset w/o resync

/*										
extern void jtag_goto_idle(void);	// reset JTAG state machine to RUN-TEST/IDLE stat
extern void jtag_goto_reset(void);		// reset JTAG state machine to TEST-LOGIC-RESET state

void jtag_tclk_pulse(void);						// issue a single TCLK pulse to target 
*/

void jtag_init(void);			// init JTAG state machine
unsigned char jtag_sel_shift(unsigned char mode);										/* JTAG - go from RUN-TEST/IDLE to SHIFT-DR (mode==0) or SHIFT-IR (mode!=0) state */
unsigned char jtag_sel_reset(void);													/* JTAG - go from RUN-TEST/IDLE to TEST-LOGIC-RESET state */
void jtag_write(unsigned char bit_count, unsigned char tap_transition, unsigned char * datap);
void jtag_read(unsigned char bit_count, unsigned char tap_transition, unsigned char * datap);

void jtag_transition_shift(unsigned char mode);
void jtag_transition_reset(void);
																			

#define RxPend		0x01				// Rx byte pending flag
#define Halted		0x02				// Target pending flag
#define VswEn		0x04				// VSW output enabled flag
#define VswFlt		0x08				// VSW output fault flag
#define VtrgEn		0x10				// VTRG output enabled flag
#define VtrgFlt	0x20				// VTRG output fault flag
#define max_num_retries 0x03  // maximum number of swap retries 
#define false   0x00
#define true    0x01							 
//----------------------------------------------------------------------------
// BDM commands 

#define BDMCF_RETRY         20   // how many times to retry before giving up 

#define BDMCF_CMD_RAREG     0x2180	// read AD register
#define BDMCF_CMD_WAREG     0x2080	// write AD register

#define BDMCF_CMD_READ8     0x1900
#define BDMCF_CMD_READ16    0x1940
#define BDMCF_CMD_READ32    0x1980

#define BDMCF_CMD_WRITE8    0x1800
#define BDMCF_CMD_WRITE16   0x1840
#define BDMCF_CMD_WRITE32   0x1880

#define BDMCF_CMD_DUMP8     0x1D00
#define BDMCF_CMD_DUMP16    0x1D40
#define BDMCF_CMD_DUMP32    0x1D80

#define BDMCF_CMD_FILL8     0x1C00
#define BDMCF_CMD_FILL16    0x1C40
#define BDMCF_CMD_FILL32    0x1C80

#define BDMCF_CMD_GO        0x0C00
#define BDMCF_CMD_NOP       0x0000

#define BDMCF_CMD_RCREG     0x2980
#define BDMCF_CMD_WCREG     0x2880

#define BDMCF_CMD_RDMREG    0x2D80
#define BDMCF_CMD_WDMREG    0x2C80


#define BDMCF_RDMREG_CSR	0x2D80
#define BDMCF_WDMREG_CSR	0x2C80
#define BDMCF_WDMREG_AATR	0x2C86
#define BDMCF_WDMREG_TDR	0x2C87
#define BDMCF_WDMREG_PBR	0x2C88
#define BDMCF_WDMREG_PBMR	0x2C89
#define BDMCF_WDMREG_ABHR	0x2C8C
#define BDMCF_WDMREG_ABHL	0x2C8D
#define BDMCF_WDMREG_DBR	0x2C8E
#define BDMCF_WDMREG_DBMR	0x2C8F

// Configuration Status Register Defines, CSR[23:17, 3:0] unused in V2
#define CSR_BSTAT3_MASK	0x80000000
#define CSR_BSTAT2_MASK	0x40000000
#define CSR_BSTAT1_MASK	0x20000000
#define CSR_BSTAT0_MASK	0x10000000
#define CSR_FOF_MASK		0x08000000
#define CSR_TRG_MASK		0x04000000
#define CSR_HALT_MASK	0x02000000
#define CSR_BKPT_MASK	0x01000000
#define CSR_IPW_MASK		0x00010000
#define CSR_MAP_MASK		0x00008000
#define CSR_TRC_MASK		0x00004000
#define CSR_EMU_MASK		0x00002000
#define CSR_DDC1_MASK	0x00001000
#define CSR_DDC0_MASK	0x00000800
#define CSR_UHE_MASK		0x00000400
#define CSR_BTB1_MASK	0x00000200
#define CSR_BTB0_MASK	0x00000100
#define CSR_NPL_MASK		0x00000040
#define CSR_IPI_MASK		0x00000020
#define CSR_SSM_MASK		0x00000010



// MCU Signal Defines
#define VSW_EN				PTBD_PTBD0
#define VSW_EN_MASK		PTBD_PTBD0_MASK
#define VSW_EN_DIR		PTBDD_PTBDD0
						  	
#define VTRG_EN			PTBD_PTBD1
#define VTRG_EN_MASK		PTBD_PTBD1_MASK
#define VTRG_EN_DIR		PTBDD_PTBDD1

#define TCLK_CTL			PTBD_PTBD2
#define TCLK_CTL_MASK	PTBD_PTBD2_MASK
#define TCLK_CTL_DIR		PTBDD_PTBDD2

#define BRK_TMS			PTBD_PTBD3
#define BRK_TMS_MASK		PTBD_PTBD3_MASK
#define BRK_TMS_DIR		PTBDD_PTBDD3

#define TA_OUT				PTBD_PTBD4
#define TA_OUT_MASK		PTBD_PTBD4_MASK
#define TA_OUT_DIR		PTBDD_PTBDD4

#define VTRG_IN			PTBD_PTBD5		// ADC input to monitor VTRG_IN
#define VTRG_IN_MASK		PTBD_PTBD5_MASK
#define VTRG_IN_DIR		PTBDD_PTBDD5

#define P4_IN				PTCD_PTCD0
#define P4_IN_MASK		PTCD_PTCD0_MASK
#define P4_IN_DIR			PTCDD_PTCDD0

#define P5_IN				PTCD_PTCD1
#define P5_IN_MASK		PTCD_PTCD1_MASK
#define P5_IN_DIR			PTCDD_PTCDD1

#define P6_IN				PTCD_PTCD2
#define P6_IN_MASK		PTCD_PTCD2_MASK
#define P6_IN_DIR			PTCDD_PTCDD2

#define P7_DE_IN			PTCD_PTCD3
#define P7_DE_IN_MASK	PTCD_PTCD3_MASK
#define P7_DE_IN_DIR		PTCDD_PTCDD3

#define tRSTO				PTCD_PTCD4
#define tRSTO_MASK		PTCD_PTCD4_MASK
#define tRSTO_DIR			PTCDD_PTCDD4

#define VPP_ON				PTCD_PTCD5
#define VPP_ON_MASK		PTCD_PTCD5_MASK
#define VPP_ON_DIR		PTCDD_PTCDD5

#define tPWR_LED			PTDD_PTDD0
#define tPWR_LED_MASK	PTDD_PTDD0_MASK
#define tPWR_LED_DIR		PTDDD_PTDDD0

#define STATUS_LED		PTDD_PTDD1
#define STATUS_LED_MASK	PTDD_PTDD1_MASK
#define STATUS_LED_DIR	PTDDD_PTDDD1

#define tRSTI				PTDD_PTDD2
#define tRSTI_MASK		PTDD_PTDD2_MASK
#define tRSTI_DIR			PTDDD_PTDDD2

#define TDSCLK_EN			PTED_PTED2
#define TDSCLK_EN_MASK	PTED_PTED2_MASK
#define TDSCLK_EN_DIR	PTEDD_PTEDD2

#define TCLK_EN			PTED_PTED3
#define TCLK_EN_MASK		PTED_PTED3_MASK
#define TCLK_EN_DIR		PTEDD_PTEDD3

#define DIN					PTED_PTED4
#define DIN_MASK			PTED_PTED4_MASK
#define DIN_DIR			PTEDD_PTEDD4

#define DOUT				PTED_PTED5
#define DOUT_MASK			PTED_PTED5_MASK
#define DOUT_DIR			PTEDD_PTEDD5

#define SCLK_OUT			PTED_PTED6
#define SCLK_OUT_MASK	PTED_PTED6_MASK
#define SCLK_OUT_DIR		PTEDD_PTEDD6

#define OUT_EN				PTED_PTED7
#define OUT_EN_MASK		PTED_PTED7_MASK
#define OUT_EN_DIR		PTEDD_PTEDD7

#define VPP_EN				PTFD_PTFD0
#define VPP_EN_MASK		PTFD_PTFD0_MASK
#define VP_EN_DIR			PTFDD_PTFDD0

#define VSW_FAULT			PTGD_PTGD0
#define VSW_FAULT_MASK	PTGD_PTGD0_MASK
#define VSW_FAULT_DIR	PTGDD_PTGDD0

#define VTRG_FAULT		PTGD_PTGD1
#define VTRG_FAULT_MASK	PTGD_PTGD1_MASK
#define VTRG_FAULT_DIR	PTGDD_PTGDD1

#define RTS					PTGD_PTGD2
#define RTS_MASK			PTGD_PTGD2_MASK
#define RTS_DIR			PTGDD_PTGDD2

#define CTS					PTGD_PTGD3
#define CTS_MASK			PTGD_PTGD3_MASK
#define CTS_DIR			PTGDD_PTGDD3

// JTAG Signal Macros

#define TRST_SET()        TDSCLK_EN = 1
#define TRST_RESET()      TDSCLK_EN = 0

#define TMS_SET()         BRK_TMS = 1
#define TMS_RESET()       BRK_TMS = 0
#define TMS_BRK_SET       BRK_TMS == 1

#define TCLK_SET()        TCLK_EN = 1
#define TCLK_RESET()      TCLK_EN = 0

#define TDI_OUT_SET()     DOUT = 1
#define TDI_OUT_RESET()   DOUT = 0

#define TDO_IN_SET        DIN==1

#endif // _BDM_CF_H_
