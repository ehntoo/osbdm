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


#ifndef _JTAG_DSC_H_
#define _JTAG_DSC_H_


#define SIM_CNTL_ADDR_UNDEFINED 0xFFFFFFFF

/* shut up compiler warnings about unused parameters */
#define QUIET(arg) if(!&arg)return osbdm_error_invalid_parameter

#define return_if(err) if(err)return(err)

#define CACHE_ENABLED 1

#define JTAG_CHIP_LENGTH_IR 8
#define JTAG_CORE_LENGTH_IR 4
#define JTAG_DEFAULT_LENGTH_DR 4
#define JTAG_IDCODE 0x2
#define JTAG_ENABLE_EONCE 0x6
#define JTAG_DEBUG_REQUEST 0x7
#define JTAG_CHIP_TLM_SELECT 0x05
#define JTAG_CORE_TLM_SELECT 0x8
#define JTAG_CHIP_BYPASS 0xFF
#define JTAG_CORE_BYPASS 0xF
#define JTAG_BYPASS_LENGTH 1
#define TLM_SELECT_CHIP_TAP 1
#define TLM_SELECT_CORE_TAP 2

#define OCR_ERLO  (1<<7)
#define OCR_PWU   (1<<5)
#define OCR_DEVEN (1<<4)
#define OCR_LTE   (1<<3)
#define OCR_ISC_0 (0x00)
#define OCR_ISC_1 (0x01)
#define OCR_ISC_2 (0x02)
#define OCR_ISC_3 (0x03)
#define OCR_ISC_4 (0x04)
#define OCR_ISC_SINGLE_STEP (0x05)
#define OCR_ISC_6 (0x06)
#define OCR_ISC_7 (0x07)

#define IR_MODE_DEBUG 0xD
#define IR_MODE_USER 0x1
#define IR_MODE_EXACC 0x9
#define IR_MODE_STOP 0x5
#define IR_MODE_DISCONNECTED 0xF

#define OPDBR_ADDR  0x04
#define OCMDR_GO    0x40
#define OCMDR_EX    0x20
#define OCMDR_WR    0x00
#define OCMDR_RE    0x80
#define OCMDR_LENGTH 8

#define ORX_ADDR 0x0b

#define EONCE_NOREG 0x1f

#define OTXRXSR_RDF (1<<0)
#define OTXRXSR_TDF (1<<1)
#define OTXRXSR_RIE (1<<2)
#define OTXRXSR_TIE (1<<3)

#define WAIT_FOR_DEBUG_TIME 100

#define OMR_SA_MASK (~(1<<4))
#define M01_NO_MODULO (0xFFFFL)
#define SR_DISABLE_INTERRUPTS (3<<8)

#define RTD_BUFFER_SIZE ((CCS_MAX_BYTE_DATA/sizeof(unsigned long))+2)

/* HFM commands */
#define HFM_CMD_ERASE_VERIFY    0x05
#define HFM_CMD_PROGRAM         0x20
#define HFM_CMD_PAGE_ERASE      0x40
#define HFM_CMD_MASS_ERASE      0x41

/* HFM Registers */
#define HFMCLKD_OFFSET      0x00
#define HFMCR_OFFSET        0x01
#define HFMSECH_OFFSET      0x03
#define HFMSECL_OFFSET      0x04
#define HFMPROT_OFFSET      0x10
#define HFMPROTB_OFFSET     0x11
#define HFMUSTAT_OFFSET     0x13
#define HFMCMD_OFFSET       0x14

/* HFM Config Locations */
#define SECL_VALUE          0x00
#define SECH_VALUE          0x01
#define PROTB_VALUE         0x02
#define PROT_BANK0_VALUE    0x03
#define PROT_BANK1_VALUE    0x04
#define BACK_KEY_0_VALUE    0x05
#define BACK_KEY_1_VALUE    0x06
#define BACK_KEY_2_VALUE    0x07
#define BACK_KEY_3_VALUE    0x08

/* Register bits */
#define HFMCLKD_DIVLD       0x80

#define HFMCR_BKSEL         0x0003

#define HFMUSTAT_BLANK      0x04
#define HFMUSTAT_ACCERR     0x10
#define HFMUSTAT_PVIOL      0x20
#define HFMUSTAT_CCIF       0x40
#define HFMUSTAT_CBEIF      0x80

#define HFM_MAX_ERASE_COUNT         100
#define HFM_MAX_WRITE_COUNT         50
#define HFM_MAX_VERIFY_COUNT        50

#ifndef MIN
#define MIN(a,b)        (((a)<(b))?(a):(b))
#endif
#ifndef MAX
#define MAX(a,b)        (((a)>(b))?(a):(b))
#endif

#define HV2MAXDEV 1

#define HV2CFG_FAST_STEP_DISABLED     0 /* RAM Only: Accurate stepping, no skids. */
#define HV2CFG_FAST_STEP_ONE_DISABLED 1 /* RAM Only: No skid on 1 instruction step, may skid for > 1 instructions */
#define HV2CFG_FAST_STEP_ENABLED      2 /* RAM/ROM: May skid for any instruction step */

#define HV2_CORE_REG_COUNT            38

typedef enum dscreg_tag {
    dscreg_x0,
    dscreg_y0,
    dscreg_y1,
    dscreg_a0,
    dscreg_a1,
    dscreg_a2,
    dscreg_b0,
    dscreg_b1,
    dscreg_b2,
    dscreg_c0,
    dscreg_c1,
    dscreg_c2,
    dscreg_d0,
    dscreg_d1,
    dscreg_d2,
    dscreg_omr,
    dscreg_sr,
    dscreg_la,
    dscreg_la2, /* read only */
    dscreg_lc,
    dscreg_lc2, /* read only */
    dscreg_hws0,
    dscreg_hws1,
    dscreg_sp,
    dscreg_n3,
    dscreg_m01,
    dscreg_n,
    dscreg_r0,
    dscreg_r1,
    dscreg_r2,
    dscreg_r3,
    dscreg_r4,
    dscreg_r5,
    dscreg_shm01,
    dscreg_shn,
    dscreg_shr0,
    dscreg_shr1,
    dscreg_pc,
    dscreg_idcode,
    dscreg_ocr,
    dscreg_oscntr,
    dscreg_osr,
    dscreg_opdbr,
    dscreg_obase,
    dscreg_otxrxsr,
    dscreg_otx,
    dscreg_otx1,
    dscreg_orx,
    dscreg_orx1,
    dscreg_otbcr,
    dscreg_otbpr,
    dscreg_otb,
    dscreg_ob0cr,
    dscreg_ob0ar1,
    dscreg_ob0ar2,
    dscreg_ob0msk,
    dscreg_ob0cntr,
    /* only 1 breakpoint unit, so the following are not used */
    dscreg_ob1cr,
    dscreg_ob1ar1,
    dscreg_ob1ar2,
    dscreg_ob1msk,
    dscreg_ob1cntr,
    dscreg_ob2cr,
    dscreg_ob2ar1,
    dscreg_ob2ar2,
    dscreg_ob2msk,
    dscreg_ob2cntr,
    dscreg_ob3cr,
    dscreg_ob3ar1,
    dscreg_ob3ar2,
    dscreg_ob3msk,
    dscreg_ob3cntr

} dscreg;

#define dscreg_once_first   (dscreg_ocr)
#define dscreg_once_last    (dscreg_ob0cntr)
#define dscreg_once_count   ((dscreg_once_last-dscreg_once_first)+1)
#define dscreg_core_first   (dscreg_x0)
#define dscreg_core_last    (dscreg_pc)
#define dscreg_core_count   ((dscreg_core_last-dscreg_core_first)+1)
#define dscreg_end          (dscreg_ob0cntr)
#define dscreg_count        (dscreg_end+1)
#define dscreg_once_output  (dscreg_otx0)
#define dscreg_once_input   (dscreg_orx0)

#define dscreg_otx0 dscreg_otx

typedef enum dscmem_tag {
    dscmem_p,
    dscmem_x,
    dscmem_p_flash,
    dscmem_x_flash
} dscmem_type;

typedef enum
{
    eHFMEraseModeAll,
    eHFMEraseModeUnits,
    eHFMEraseModePages,
} HFMEraseMode;

typedef enum osbdm_status_tag {
    osbdm_status_debug,
    osbdm_status_execute,
    osbdm_status_sleep,
    osbdm_status_waitstate,
    osbdm_status_secure,
    osbdm_status_undefined
} osbdm_status;

typedef enum osbdm_error_tag {
    osbdm_error_ok,
    osbdm_error_core_not_responding,
    osbdm_error_invalid_parameter,
    osbdm_error_internal_failure,
    osbdm_error_undefined
} osbdm_error;

typedef enum dsccfg_tag {
    dsccfg_dakar_enable_lockout_recovery,/* 0: */
    dsccfg_dakar_hfm_divisor,   /* 1: */
    dsccfg_fast_stepping,       /* 2:  See dscCFG_FAST_STEP_* */
    dsccfg_hsst_download,       /* 3:  set to 1 for hsst downloads (when possible) */
    dsccfg_hsst_poll_interval,  /* 4:  defaults to 1 poll before sleep */
    dsccfg_hsst_poll_disable,   /* 5:  set to 1 to disable, defaults to 0 */

    dsccfg_hfm_units,           /* 6:  number of flash block units */
    dsccfg_hfm_set_unit,        /* 7:  establish current flash unit */
    dsccfg_hfm_unit_address,    /* 8:  current flash unit address */
    dsccfg_hfm_unit_size,       /* 9:  current flash unit size in bytes */
    dsccfg_hfm_unit_space,      /* 10: current flash unit memory space */
    dsccfg_hfm_unit_bank,       /* 11: current flash unit memory bank */
    dsccfg_hfm_unit_interleaved,/* 12: current flash unit interleave flag */
    dsccfg_hfm_unit_pagesize,   /* 13: current flash unit page size */
    dsccfg_hfm_base,            /* 14: flash module base address */
    dsccfg_hfm_clock_divider,   /* 15: flash module clock divider */
    dsccfg_hfm_pram,            /* 16: flash algorithm location */
    dsccfg_hfm_activate,        /* 17: flash activation/deactivation flag */
    
  	dsccfg_sim_cntl_address,    /* 18: SIM CNTL memory mapped register address */ 

    dsccfg_count
} dsccfg_type;

typedef struct instr_type_tag {
    unsigned short word;
    unsigned short mask;
} instr_type;

typedef struct mem_address_tag {
    unsigned long space;
    unsigned long size;
    unsigned long address;
} mem_address;

typedef struct HFMUnit_tag
{
    unsigned long   startAddr;
    unsigned long   endAddr;
    unsigned long   bank;
    unsigned long   numSectors;
    unsigned long   numPages;
    unsigned long   pageSize;   /* in words */
    unsigned long   progMem;    /* memory space */
    unsigned char   boot;
    unsigned char   interleaved;
    unsigned char   erased;     /* only used when erase pages option is not in use */
    unsigned char   initialized;/* unit initialized? */
} HFMUnit;

#define MEMCACHESIZE_WORDS (200)
#define MEMCACHESIZE_BYTES (MEMCACHESIZE_WORDS*2)
typedef struct cache_tag {
    unsigned long r0;
    unsigned long r1;
    unsigned long r2;
    unsigned long r3;
    unsigned long r4;
    unsigned long a0;
    unsigned long a1;
    unsigned long a2;
    unsigned long pc;
    unsigned long omr;
    unsigned long m01;
    unsigned long ocr;
    unsigned long sr;
    unsigned long y0;
    unsigned long y1;
    unsigned long x0;
    unsigned long chiplevel_idcode;
    unsigned long pc_modified;
    unsigned long running;
    unsigned long config[dsccfg_count];
    unsigned long fast_mem;
    unsigned long fast_mem_addr;
    unsigned long fast_mem_loaded;
    unsigned char fast_mem_cache[MEMCACHESIZE_BYTES];
    HFMUnit unitList[1];
} cache_type;

typedef struct eonce_tag {
    unsigned char address;
    unsigned long length;
    char * name;
} eonce_type;

typedef struct corer_tag {
    unsigned long mask;
    char * name;
} corer_type;

typedef struct instruction_tag {
    int size;
    unsigned char bytes[6];
} instruction_type;


typedef struct sequence_tag {
    int datasize;
    int count;
    instruction_type instr[6];
} sequence_type;

#endif
