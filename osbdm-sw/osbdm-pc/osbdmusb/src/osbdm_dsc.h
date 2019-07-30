/* OSBDM-JM60 Windows USB Library
 * Copyright (C) 2009  Freescale
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

//----------------------------------------------------------------------------
//
//
//  FILE
//
//      osbdm_dsc.h
// 
//
//  DESCRIPTION
//
//		OSBDM DSC Specific header
//
//
//----------------------------------------------------------------------------
//

#ifndef __OSBDM_DSC_H__
#define __OSBDM_DSC_H__

//
// definitions
//
typedef enum  {
    dscmem_p,
    dscmem_x,
    dscmem_p_flash,
    dscmem_x_flash
} dscmem;


//
// DSC (568600E) register numbers
//
typedef enum dscregIDTag {
    dscregID_x0,		// 0
    dscregID_y0,
    dscregID_y1,
    dscregID_a0,
    dscregID_a1,
    dscregID_a2,
    dscregID_b0,
    dscregID_b1,
    dscregID_b2,
    dscregID_c0,
    dscregID_c1,		// 10
    dscregID_c2,
    dscregID_d0,
    dscregID_d1,
    dscregID_d2,
    dscregID_omr,
    dscregID_sr,
    dscregID_la,
    dscregID_la2, /* read only */
    dscregID_lc,
    dscregID_lc2, /* read only */ // 20
    dscregID_hws0,
    dscregID_hws1,
    dscregID_sp,
    dscregID_n3,
    dscregID_m01,
    dscregID_n,
    dscregID_r0,
    dscregID_r1,
    dscregID_r2,
    dscregID_r3,		// 30
    dscregID_r4,
    dscregID_r5,
    dscregID_shm01,
    dscregID_shn,
    dscregID_shr0,
    dscregID_shr1,
    dscregID_pc,		// 37
    dscregID_idcode,	// 38
    dscregID_ocr,
    dscregID_oscntr,	// 40
    dscregID_osr,		// 41
    dscregID_opdbr,
    dscregID_obase,
    dscregID_otxrxsr,
    dscregID_otx,
    dscregID_otx1,
    dscregID_orx,
    dscregID_orx1,
    dscregID_otbcr,
    dscregID_otbpr,	// 50
    dscregID_otb,
    dscregID_ob0cr,
    dscregID_ob0ar1,
    dscregID_ob0ar2,
    dscregID_ob0msk,
    dscregID_ob0cntr,
    /* only 1 breakpoint unit, so the following are not used */
    dscregID_ob1cr,
    dscregID_ob1ar1,
    dscregID_ob1ar2,
    dscregID_ob1msk,	// 60
    dscregID_ob1cntr,
    dscregID_ob2cr,
    dscregID_ob2ar1,
    dscregID_ob2ar2,
    dscregID_ob2msk,
    dscregID_ob2cntr,
    dscregID_ob3cr,
    dscregID_ob3ar1,
    dscregID_ob3ar2,
    dscregID_ob3msk,	// 70
    dscregID_ob3cntr,

	dscregID_chiplevel_idcode = 0x1000

} dscregID;

typedef enum dsccfgTag {
    dsccfg_dakar_enable_lockout_recovery,
    dsccfg_dakar_hfm_divisor,
    dsccfg_fast_stepping,   /* See dscCFG_FAST_STEP_* */
    dsccfg_hsst_download,   /* set to 1 for hsst downloads (when possible) */
    dsccfg_hsst_poll_interval,  /* defaults to 1 poll before sleep */
    dsccfg_hsst_poll_disable,   /* set to 1 to disable, defaults to 0 */

    dsccfg_hfm_units,           /* number of flash block units */
    dsccfg_hfm_set_unit,        /* establish current flash unit */
    dsccfg_hfm_unit_address,    /* current flash unit address */
    dsccfg_hfm_unit_size,       /* current flash unit size in bytes */
    dsccfg_hfm_unit_space,      /* current flash unit memory space */
    dsccfg_hfm_unit_bank,       /* current flash unit memory bank */
    dsccfg_hfm_unit_interleaved,/* current flash unit interleave flag */
    dsccfg_hfm_unit_pagesize,   /* current flash unit page size */
    dsccfg_hfm_base,            /* flash module base address */
    dsccfg_hfm_clock_divider,   /* flash module clock divider */
    dsccfg_hfm_pram,            /* flash algorithm location */
    dsccfg_hfm_activate,        /* flash activation/deactivation flag */
 
	dsccfg_sim_cntl_address,    /* 18: SIM CNTL memory mapped register address */ 
    dsccfg_count
} dsccfgType;

#endif //__OSBDM_DSC_H__

