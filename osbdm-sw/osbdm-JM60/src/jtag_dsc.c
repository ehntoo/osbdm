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
/*
* 03/08/2011  : Modified by P&E Microcomputer Systems
*               http://www.pemicro.com/osbdm
*
*
*/

#include <stdlib.h>
#include <string.h>
#include "derivative.h"				// include peripheral declarations
#include "typedef.h"			  	// Include peripheral declarations
#include "jtag_dsc.h"
#include "jtag_jm60.h"        // JTAG declarations for JM60 debug pod
#include "commands.h"			   	// BDM commands header file
#include "MCU.h"					    // MCU header
#include "timer.h"				    // timer functions
#include "targetAPI.h"				// target API include file
#include "board_id.h"		  // Board ID
#include "serial_io.h"        // Serial port handling/enable

//---------------------------------------------------------------------------
// Firmware Info
//---------------------------------------------------------------------------
  
volatile const byte TARGET_TYPE   =  eDSC;

#ifdef __EMBEDDED__ 
  volatile const byte FIRMWARE_TYPE   =  eEmbeddedGeneric;
#else
  volatile const byte FIRMWARE_TYPE  =  eGeneric;
#endif 
   

//---------------------------------------------------------------------------
// Sequence
//---------------------------------------------------------------------------

static const sequence_type dsc_write_r4[] = {
/* imm32    */ {32,1,{{3,{0x1C,0xE4,0x00,0x00,0x00,0x00,}},}},
};

static const sequence_type dsc_nop[] = {
/* null     */ {0,1,{{1,{0x00,0xE7,}},}},
};

static const sequence_type dsc_read_reg_sequence[HV2_CORE_REG_COUNT] = {
/* x0       */ {16,1,{{3,{0x7F,0xE7,0x7C,0xD4,0xFF,0xFF,}},}},
/* y0       */ {16,1,{{3,{0x7F,0xE7,0x7C,0xD5,0xFF,0xFF,}},}},
/* y1       */ {16,1,{{3,{0x7F,0xE7,0x7C,0xD7,0xFF,0xFF,}},}},
/* a0       */ {16,1,{{3,{0x7F,0xE7,0xFC,0xD6,0xFF,0xFF,}},}},
/* a1       */ {16,1,{{3,{0x7F,0xE7,0x7C,0xD0,0xFF,0xFF,}},}},
/* a2       */ {4,1,{{3,{0x7F,0xE7,0xFC,0xD4,0xFF,0xFF,}},}},
/* b0       */ {16,1,{{3,{0x7F,0xE7,0xFC,0xD7,0xFF,0xFF,}},}},
/* b1       */ {16,1,{{3,{0x7F,0xE7,0x7C,0xD1,0xFF,0xFF,}},}},
/* b2       */ {4,1,{{3,{0x7F,0xE7,0xFC,0xD5,0xFF,0xFF,}},}},
/* c0       */ {16,2,{{1,{0x20,0x7C,}},{3,{0x7F,0xE7,0xFC,0xD6,0xFF,0xFF,}},}},
/* c1       */ {16,2,{{1,{0x20,0x7C,}},{3,{0x7F,0xE7,0x7C,0xD0,0xFF,0xFF,}},}},
/* c2       */ {4,2,{{1,{0x20,0x7C,}},{3,{0x7F,0xE7,0xFC,0xD4,0xFF,0xFF,}},}},
/* d0       */ {16,2,{{1,{0x30,0x7C,}},{3,{0x7F,0xE7,0xFC,0xD6,0xFF,0xFF,}},}},
/* d1       */ {16,2,{{1,{0x30,0x7C,}},{3,{0x7F,0xE7,0x7C,0xD0,0xFF,0xFF,}},}},
/* d2       */ {4,2,{{1,{0x30,0x7C,}},{3,{0x7F,0xE7,0xFC,0xD4,0xFF,0xFF,}},}},
/* omr      */ {16,2,{{1,{0x9C,0x86,}},{3,{0x7F,0xE7,0xFC,0xD6,0xFF,0xFF,}},}},
/* sr       */ {16,2,{{1,{0x9D,0x86,}},{3,{0x7F,0xE7,0xFC,0xD6,0xFF,0xFF,}},}},
/* la       */ {24,2,{{1,{0x1F,0x8C,}},{3,{0x7F,0xE3,0x7D,0xDC,0xFF,0xFF,}},}},
/* la2      */ {24,2,{{3,{0x1C,0xE4,0xAD,0xDB,0xBA,0xFF,}},{3,{0x7F,0xE3,0x7D,0xDC,0xFF,0xFF,}},}},
/* lc       */ {24,2,{{1,{0x1E,0x8C,}},{3,{0x7F,0xE3,0x7D,0xDC,0xFF,0xFF,}},}},
/* lc2      */ {24,2,{{3,{0x1C,0xE4,0xAD,0xDB,0xBA,0xFF,}},{3,{0x7F,0xE3,0x7D,0xDC,0xFF,0xFF,}},}},
/* hws0     */ {24,2,{{3,{0x1C,0xE4,0xAD,0xDB,0xBA,0xFF,}},{3,{0x7F,0xE3,0x7D,0xDC,0xFF,0xFF,}},}},
/* hws1     */ {24,2,{{3,{0x1C,0xE4,0xAD,0xDB,0xBA,0xFF,}},{3,{0x7F,0xE3,0x7D,0xDC,0xFF,0xFF,}},}},
/* sp       */ {24,2,{{1,{0xBC,0x81,}},{3,{0x7F,0xE3,0x7D,0xDC,0xFF,0xFF,}},}},
/* n3       */ {16,2,{{1,{0x99,0x86,}},{3,{0x7F,0xE7,0xFC,0xD6,0xFF,0xFF,}},}},
/* m01      */ {16,2,{{1,{0x9A,0x86,}},{3,{0x7F,0xE7,0xFC,0xD6,0xFF,0xFF,}},}},
/* n        */ {24,1,{{3,{0x7F,0xE3,0x7D,0xDE,0xFF,0xFF,}},}},
/* r0       */ {24,1,{{3,{0x7F,0xE3,0x7D,0xD8,0xFF,0xFF,}},}},
/* r1       */ {24,1,{{3,{0x7F,0xE3,0x7D,0xD9,0xFF,0xFF,}},}},
/* r2       */ {24,1,{{3,{0x7F,0xE3,0x7D,0xDA,0xFF,0xFF,}},}},
/* r3       */ {24,1,{{3,{0x7F,0xE3,0x7D,0xDB,0xFF,0xFF,}},}},
/* r4       */ {24,1,{{3,{0x7F,0xE3,0x7D,0xDC,0xFF,0xFF,}},}},
/* r5       */ {24,1,{{3,{0x7F,0xE3,0x7D,0xDD,0xFF,0xFF,}},}},
/* shm01    */ {16,4,{{1,{0x06,0xE7,}},{1,{0x9A,0x86,}},{3,{0x7F,0xE7,0xFC,0xD6,0xFF,0xFF,}},{1,{0x06,0xE7,}},}},
/* shn      */ {24,5,{{1,{0x06,0xE7,}},{3,{0x7F,0xE3,0x7D,0xDE,0xFF,0xFF,}},{1,{0x00,0xE7,}},{1,{0x00,0xE7,}},{1,{0x06,0xE7,}},}},
/* shr0     */ {24,5,{{1,{0x06,0xE7,}},{3,{0x7F,0xE3,0x7D,0xD8,0xFF,0xFF,}},{1,{0x00,0xE7,}},{1,{0x00,0xE7,}},{1,{0x06,0xE7,}},}},
/* shr1     */ {24,5,{{1,{0x06,0xE7,}},{3,{0x7F,0xE3,0x7D,0xD9,0xFF,0xFF,}},{1,{0x00,0xE7,}},{1,{0x00,0xE7,}},{1,{0x06,0xE7,}},}},
/* pc       */ {21,2,{{1,{0x16,0xE7,}},{3,{0x7F,0xE3,0x7D,0xDC,0xFF,0xFF,}},}},
};

static const sequence_type dsc_write_reg_sequence[HV2_CORE_REG_COUNT] = {
/* x0       */ {16,1,{{1,{0x0C,0x84,}},}},
/* y0       */ {16,1,{{1,{0x0C,0x85,}},}},
/* y1       */ {16,1,{{1,{0x0C,0x87,}},}},
/* a0       */ {16,1,{{1,{0x8C,0x86,}},}},
/* a1       */ {16,1,{{1,{0x8C,0x80,}},}},
/* a2       */ {4,1,{{1,{0x8C,0x84,}},}},
/* b0       */ {16,1,{{1,{0x8C,0x87,}},}},
/* b1       */ {16,1,{{1,{0x8C,0x81,}},}},
/* b2       */ {4,1,{{1,{0x8C,0x85,}},}},
/* c0       */ {16,3,{{1,{0x20,0x7C,}},{1,{0x8C,0x86,}},{1,{0x00,0x7D,}},}},
/* c1       */ {16,3,{{1,{0x20,0x7C,}},{1,{0x8C,0x80,}},{1,{0x00,0x7D,}},}},
/* c2       */ {4,3,{{1,{0x20,0x7C,}},{1,{0x8C,0x84,}},{1,{0x00,0x7D,}},}},
/* d0       */ {16,3,{{1,{0x30,0x7C,}},{1,{0x8C,0x86,}},{1,{0x80,0x7D,}},}},
/* d1       */ {16,3,{{1,{0x30,0x7C,}},{1,{0x8C,0x80,}},{1,{0x80,0x7D,}},}},
/* d2       */ {4,3,{{1,{0x30,0x7C,}},{1,{0x8C,0x84,}},{1,{0x80,0x7D,}},}},
/* omr      */ {16,1,{{1,{0x8C,0x8C,}},}},
/* sr       */ {16,3,{{1,{0x8C,0x8D,}},{1,{0x00,0xE7,}},{1,{0x00,0xE7,}},}},
/* la       */ {24,1,{{1,{0x00,0xE7,}},}},
/* la2      */ {24,1,{{1,{0x00,0xE7,}},}},
/* lc       */ {16,1,{{1,{0x8C,0x8E,}},}},
/* lc2      */ {16,1,{{1,{0x00,0xE7,}},}},
/* hws0     */ {24,1,{{1,{0x00,0xE7,}},}},
/* hws0     */ {24,1,{{1,{0x00,0xE7,}},}},
/* sp       */ {24,1,{{1,{0xAB,0x81,}},}},
/* n3       */ {16,1,{{1,{0x8C,0x89,}},}},
/* m01      */ {16,1,{{1,{0x8C,0x8A,}},}},
/* n        */ {24,1,{{1,{0xAA,0x81,}},}},
/* r0       */ {24,1,{{1,{0xA0,0x81,}},}},
/* r1       */ {24,1,{{1,{0xA1,0x81,}},}},
/* r2       */ {24,1,{{1,{0xA2,0x81,}},}},
/* r3       */ {24,1,{{1,{0xA3,0x81,}},}},
/* r4       */ {24,1,{{1,{0x00,0xE7,}},}},
/* r5       */ {24,1,{{1,{0xA9,0x81,}},}},
/* shm01    */ {16,5,{{1,{0x06,0xE7,}},{1,{0x8C,0x8A,}},{1,{0x00,0xE7,}},{1,{0x00,0xE7,}},{1,{0x06,0xE7,}},}},
/* shn      */ {24,5,{{1,{0x06,0xE7,}},{1,{0xAA,0x81,}},{1,{0x00,0xE7,}},{1,{0x00,0xE7,}},{1,{0x06,0xE7,}},}},
/* shr0     */ {24,5,{{1,{0x06,0xE7,}},{1,{0xA0,0x81,}},{1,{0x00,0xE7,}},{1,{0x00,0xE7,}},{1,{0x06,0xE7,}},}},
/* shr1     */ {24,5,{{1,{0x06,0xE7,}},{1,{0xA1,0x81,}},{1,{0x00,0xE7,}},{1,{0x00,0xE7,}},{1,{0x06,0xE7,}},}},
/* pc       */ {21,1,{{1,{0x17,0xE7,}},}},
};

static const sequence_type dsc_read_xmem_sequence[5] = {
/* null     */ {0,1,{{1,{0x00,0xE7,}},}},
/* readxb   */ {8,2,{{1,{0xA0,0xF8,}},{3,{0x7F,0xE7,0x7C,0xD0,0xFF,0xFF,}},}},
/* readxw   */ {16,2,{{1,{0x80,0xF0,}},{3,{0x7F,0xE7,0x7C,0xD0,0xFF,0xFF,}},}},
/* null     */ {0,1,{{1,{0x00,0xE7,}},}},
/* readxl   */ {32,2,{{1,{0x20,0xF0,}},{3,{0x7F,0xE3,0x7D,0xD0,0xFF,0xFF,}},}},
};

static const sequence_type dsc_read_pmem_sequence[1] = {
/* read p   */ {16,2,{{1,{0x68,0x83,}},{3,{0x7F,0xE7,0x7C,0xD0,0xFF,0xFF,}},}},
};

static const sequence_type dsc_write_xmem_sequence[5] = {
/* null     */ {0,1,{{1,{0x00,0xE7,}},}},
/* write data in a1, address in r0 */
/* writexb  */ {8,1,{{1,{0xA0,0xD0,}},}},
/* write data in a1, address in r0 */
/* writexw  */ {16,1,{{1,{0x00,0xD0,}},}},
/* null     */ {0,1,{{1,{0x00,0xE7,}},}},
/* write lower word in a0, upper word in a1, address in r0 */
/* writexl  */ {32,1,{{1,{0x20,0xD0,}},}},
};

static const sequence_type dsc_write_pmem_sequence[1] = {
/* write data in a1, address in r0 */
/* write p  */ {16,1,{{1,{0x60,0x83,}},}},
};

static const sequence_type dsc_fill_xmem_sequence[5] = {
/* null     */ {32,1,{{1,{0x00,0xE7,}},}},
/* write data in a1, address in r0 */
/* fillxb   */ {8,1,{{1,{0xA0,0xD0,}},}},
/* write data in a1, address in r0 */
/* fillxw   */ {16,1,{{1,{0x00,0xD0,}},}},
/* null     */ {0,1,{{1,{0x00,0xE7,}},}},
/* write lower word in a0, upper word in a1, address in r0 */
/* fillxl   */ {32,1,{{1,{0x20,0xD0,}},}},
};

static const sequence_type dsc_fill_pmem_sequence[1] = {
/* write data in a1, address in r0 */
/* fill p   */ {16,1,{{1,{0x60,0x83,}},}},
};

static const unsigned char dscxfer[] = {
    0xE4, 0x09,
    0xFF, 0xFE,
    0xE4, 0x0A,
    0xFF, 0xFF,
    0xE4, 0x0B,
    0xFF, 0xFD,
    0xE4, 0x1C,
    0xF4, 0x00,
    0x00, 0x00,
    0xF8, 0x35,
    0x8C, 0x43,
    0x00, 0x01,
    0xA0, 0x7D,
    0xF8, 0x35,
    0x8C, 0x43,
    0x00, 0x01,
    0xA0, 0x7D,
    0xF5, 0x15,
    0xF4, 0x16,
    0x5E, 0x81,
    0xE3, 0x68,
    0x00, 0x1C,
    0xE7, 0x00,
    0x5E, 0x82,
    0xE3, 0x68,
    0x00, 0x27,
    0xE7, 0x00,
    0x5E, 0x83,
    0xE3, 0x68,
    0x00, 0x42,
    0xE7, 0x00,
    0x5E, 0x84,
    0xE3, 0x68,
    0x00, 0x6A,
    0x8C, 0x43,
    0x00, 0x01,
    0xA0, 0x7D,
    0xF7, 0x35,
    0x72, 0x0B,
    0x85, 0x60,
    0xE3, 0x68,
    0x00, 0x8E,
    0x87, 0x60,
    0x72, 0x0B,
    0xA2, 0x75,
    0xE7, 0x00,
    0xE7, 0x00,
    0xE1, 0x6C,
    0x00, 0x87,
    0x8C, 0x43,
    0x00, 0x01,
    0xA0, 0x7D,
    0xF7, 0x35,
    0xD5, 0x00,
    0x72, 0x0B,
    0xE3, 0x68,
    0x00, 0x7F,
    0xD7, 0x00,
    0x72, 0x0B,
    0xA2, 0x75,
    0xE7, 0x00,
    0xE7, 0x00,
    0xE1, 0x6C,
    0x00, 0x78,
    0x8A, 0x43,
    0x01, 0x7E,
    0xE7, 0x00,
    0xE7, 0x00,
    0xF7, 0x35,
    0x85, 0x60,
    0x87, 0x60,
    0x86, 0x4C,
    0x00, 0x20,
    0x00, 0x14,
    0x86, 0x4C,
    0x00, 0x80,
    0x00, 0x13,
    0xE7, 0x00,
    0xE7, 0x00,
    0x8A, 0x4C,
    0x00, 0x13,
    0x80, 0x7B,
    0xE7, 0x00,
    0xE7, 0x00,
    0x72, 0x0B,
    0xE3, 0x68,
    0x00, 0x61,
    0xE7, 0x00,
    0xE7, 0x00,
    0x72, 0x0B,
    0xA2, 0x65,
    0xE7, 0x00,
    0xE7, 0x00,
    0xE1, 0x6C,
    0x00, 0x59,
    0x8A, 0x43,
    0x01, 0x7E,
    0xE7, 0x00,
    0xE7, 0x00,
    0xF7, 0x35,
    0xD5, 0x00,
    0x86, 0x4C,
    0x00, 0x20,
    0x00, 0x14,
    0x86, 0x4C,
    0x00, 0x80,
    0x00, 0x13,
    0xE7, 0x00,
    0xE7, 0x00,
    0x8A, 0x4C,
    0x00, 0x13,
    0x80, 0x7B,
    0xE7, 0x00,
    0xE7, 0x00,
    0x72, 0x0B,
    0xE3, 0x68,
    0x00, 0x43,
    0xE7, 0x00,
    0xE7, 0x00,
    0xD7, 0x00,
    0x86, 0x4C,
    0x00, 0x20,
    0x00, 0x14,
    0x86, 0x4C,
    0x00, 0x80,
    0x00, 0x13,
    0xE7, 0x00,
    0xE7, 0x00,
    0x8A, 0x4C,
    0x00, 0x13,
    0x80, 0x7B,
    0xE7, 0x00,
    0xE7, 0x00,
    0x72, 0x0B,
    0xA2, 0x58,
    0xE7, 0x00,
    0xE7, 0x00,
    0xE1, 0x6C,
    0x00, 0x2D,
    0x8A, 0x43,
    0x01, 0x7E,
    0xE7, 0x00,
    0xE7, 0x00,
    0xF7, 0x35,
    0x85, 0x60,
    0x86, 0x4C,
    0x00, 0x20,
    0x00, 0x14,
    0x86, 0x4C,
    0x00, 0x80,
    0x00, 0x13,
    0xE7, 0x00,
    0xE7, 0x00,
    0x8A, 0x4C,
    0x00, 0x13,
    0x80, 0x7B,
    0xE7, 0x00,
    0xE7, 0x00,
    0x72, 0x0B,
    0xE3, 0x68,
    0x00, 0x17,
    0xE7, 0x00,
    0xE7, 0x00,
    0x87, 0x60,
    0x86, 0x4C,
    0x00, 0x20,
    0x00, 0x14,
    0x86, 0x4C,
    0x00, 0x80,
    0x00, 0x13,
    0xE7, 0x00,
    0xE7, 0x00,
    0x8A, 0x4C,
    0x00, 0x13,
    0x80, 0x7B,
    0xE7, 0x00,
    0xE7, 0x00,
    0x72, 0x0B,
    0xA2, 0x58,
    0xE7, 0x00,
    0xE7, 0x00,
    0xE1, 0x6C,
    0x00, 0x01,
    0xE7, 0x00,
    0xE7, 0x00,
    0xE7, 0x01,
};
static const unsigned long dscxfer_len = 372;

static const eonce_type eonce[] = {
    {0x01,  8, "ocr"},
    {0x02, 24, "oscntr"},
    {0x03, 16, "osr"},
    {0x04, 16, "opdbr"}, /* 16, 32, or 48 */
    {0x05,  8, "obase"},
    {0x06,  8, "otxrxsr"},
    {0x07, 32, "otx"},
    {0x09, 16, "otx1"},
    {ORX_ADDR, 32, "orx"},
    {0x0d, 16, "orx1"},
    {0x0e, 16, "otbcr"},
    {0x0f,  8, "otbpr"},
    {0x10, 21, "otb"},

    {0x11, 24, "ob0cr"},
    {0x12, 24, "ob0ar1"},
    {0x13, 32, "ob0ar2"},
    {0x14, 32, "ob0msk"},
    {0x15, 16, "ob0cntr"},

    {0x17, 24, "ob1cr"},
    {0x18, 24, "ob1ar1"},
    {0x19, 32, "ob1ar2"},
    {0x1a, 32, "ob1msk"},
    {0x1b, 16, "ob1cntr"},

    {0x1d, 24, "ob2cr"},
    {0x1e, 24, "ob2ar1"},
    {0x20, 32, "ob2ar2"},
    {0x21, 32, "ob2msk"},
    {0x22, 16, "ob2cntr"},

    {0x24, 24, "ob3cr"},
    {0x25, 24, "ob3ar1"},
    {0x26, 32, "ob3ar2"},
    {0x27, 32, "ob3msk"},
    {0x28, 16, "ob3cntr"},
};

static cache_type Cache[1];
static int Nst;

static unsigned long SimCntlRegAddr;
static unsigned char output_buffer_enable_value = 0x00;
#define output_buffer_enable_mask 0x80

#define tms_tdi_transaction_compression_start 0x40
#define tms_tdi_transaction_compression_end 0x4F
#define tms_only_transaction_compression_start 0x50
#define tms_only_transaction_compression_end 0x5F

static unsigned short tms_only_transaction_compression_array_tmsval[tms_only_transaction_compression_end-tms_only_transaction_compression_start+1];
static unsigned char tms_only_transaction_compression_array_bitsval[tms_only_transaction_compression_end-tms_only_transaction_compression_start+1];
static unsigned short tms_tdi_transaction_compression_array_tmsval[tms_tdi_transaction_compression_end-tms_tdi_transaction_compression_start+1];
static unsigned short tms_tdi_transaction_compression_array_tdival[tms_tdi_transaction_compression_end-tms_tdi_transaction_compression_start+1];
static unsigned char tms_tdi_transaction_compression_array_bitsval[tms_tdi_transaction_compression_end-tms_tdi_transaction_compression_start+1];

/* local function declarations */
static osbdm_error read_status (osbdm_status * status);
static osbdm_error save_cache(int nst, int stepping_enabled);
static osbdm_error restore_cache(int nst, int stepping_enabled);
static osbdm_error run_from_pc (int nst);
static osbdm_error write_once_gx (unsigned char regaddr, unsigned long value, int length, int go, int exit);
static osbdm_error write_register (dscreg reg, unsigned long value);
static osbdm_error debug_request (void);
static osbdm_error wait_for_debug (unsigned long count);
static osbdm_error read_register (dscreg reg, unsigned long *value);
static osbdm_error write_register (dscreg reg, unsigned long value);
static osbdm_error execute_sequence (sequence_type * seq);
static unsigned long bigendian4_to_ulong (unsigned char * uc);
static unsigned long bigendian2_to_ulong (unsigned char * uc);
static void ulong_to_bigendian4(unsigned long ul,unsigned char * uc);
static void ulong_to_bigendian2(unsigned long ul,unsigned char * uc);
static unsigned long littleendian4_to_ulong (unsigned char * uc);
static unsigned long littleendian2_to_ulong (unsigned char * uc);
static void ulong_to_littleendian4(unsigned long ul,unsigned char * uc);
static void ulong_to_littleendian2(unsigned long ul,unsigned char * uc);
static osbdm_error step_one(int nst);
static osbdm_error step_n(int nst, int count);
static osbdm_error dsc_rtd_stat(int nst, unsigned long* read_count, unsigned long* write_count);
static int instr_match (unsigned short instr, instr_type cmpi);
/*static osbdm_error load_pram(int nst);*/
static osbdm_error activate_flash(int nst);
static osbdm_error deactivate_flash(int nst);

//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
// TODO:  these functions need to be defined to comply with common command processor

void t_assert_ta(word dly_cnt) { return; }
int t_write_ad(unsigned long addr, unsigned char *data) { return 0; }
int t_read_ad(unsigned long addr, unsigned char *data) { return 0; }
void t_debug_init(void){}

void  xchng16(unsigned char bitcount, 
                     UINT16 tdival,
                     UINT16 tmsval,
                     PUINT16 tdoval) {

	char b;

  PTED = (PTED & (output_buffer_enable_mask ^ 0xff)) | output_buffer_enable_value;

	*tdoval= 0; 
	
	// bang each bit out and receive a bit back each time
	for(b=0; b<bitcount; b++){
		if(tmsval & 0x0001 == 0x0001) {
		  TMS_SET();	        // bring TMS high
		}
		else {
		  
		  TMS_RESET();
		}
		if(tdival & 0x0001 == 0x0001) {
		  
				  TDI_OUT_SET();	        // bring TMS high
		}
		else {
		  
          TDI_OUT_RESET();
          
		}
		
	TCLK_SET();		// TCLK High     

		tdival >>=1;				// shift to next output bit
		tmsval >>=1;				// shift to next output bit
    *tdoval >>= 1;

	// return TDO status
	if(TDO_IN_SET){
    *tdoval= *tdoval | 0x8000;
	}
	  TCLK_RESET();	// TCLK Low
	}
}

void  send16(unsigned char bitcount, 
                     UINT16 tdival,
                     UINT16 tmsval) {

	char b;

  PTED = (PTED & (output_buffer_enable_mask ^ 0xff)) | output_buffer_enable_value;

	// bang each bit out and receive a bit back each time
	for(b=0; b<bitcount; b++){
		if(tmsval & 0x0001 == 0x0001) {
		  TMS_SET();	        // bring TMS high
		}
		else {
		  
		  TMS_RESET();
		}
		if(tdival & 0x0001 == 0x0001) {
		  
				  TDI_OUT_SET();	        // bring TMS high
		}
		else {
		  
          TDI_OUT_RESET();
          
		}

	TCLK_SET();		// TCLK High     

		tdival >>=1;				// shift to next output bit
		tmsval >>=1;				// shift to next output bit

  TCLK_RESET();	// TCLK Low
	}
}



/*
void eI1(unsigned short o1) {

    unsigned short junk_word;

    xchng16(16,0x440,0x1802,&junk_word);
    xchng16(12,(o1 << 4) & 0x0FF0,0x0002,&junk_word);
    xchng16(12,(o1 >> 8) & 0x00FF,0x0180,&junk_word);
}

void eI2(unsigned short o1, unsigned short o2) {
  
    unsigned short junk_word;

    xchng16(16,0x040,0x1802,&junk_word);
    xchng16(12,(o1 << 4) & 0x0FF0,0x0002,&junk_word);
    xchng16(12,(o1 >> 8) & 0x00FF,0x0180,&junk_word);

    eI1(o2);
}

void eI3(unsigned short o1, unsigned short o2, unsigned short o3) {
  
    unsigned short junk_word;

    xchng16(16,0x040,0x1802,&junk_word);
    xchng16(12,(o1 << 4) & 0x0FF0,0x0002,&junk_word);
    xchng16(12,(o1 >> 8) & 0x00FF,0x0180,&junk_word);


    eI2(o2, o3);
    
}
*/

void eI1(unsigned short o1) {

    send16(16,0x440,0x1802);
    send16(12,(o1 << 4) & 0x0FF0,0x0002);
    send16(12,(o1 >> 8) & 0x00FF,0x0180);
}

void eI2(unsigned short o1, unsigned short o2) {

    send16(16,0x040,0x1802);
    send16(12,(o1 << 4) & 0x0FF0,0x0002);
    send16(12,(o1 >> 8) & 0x00FF,0x0180);

    send16(16,0x440,0x1802);
    send16(12,(o2 << 4) & 0x0FF0,0x0002);
    send16(12,(o2 >> 8) & 0x00FF,0x0180);
}

void eI3(unsigned short o1, unsigned short o2, unsigned short o3) {

    send16(16,0x040,0x1802);
    send16(12,(o1 << 4) & 0x0FF0,0x0002);
    send16(12,(o1 >> 8) & 0x00FF,0x0180);


    send16(16,0x040,0x1802);
    send16(12,(o2 << 4) & 0x0FF0,0x0002);
    send16(12,(o2 >> 8) & 0x00FF,0x0180);

    send16(16,0x440,0x1802);
    send16(12,(o3 << 4) & 0x0FF0,0x0002);
    send16(12,(o3 >> 8) & 0x00FF,0x0180);

}



void dsc_send8_data(unsigned short data)
{
    send16(16,          // numbits
            data << 4,   // tdi
            0x1802); // tdo
}

void dsc_rcv_data16_to_buffer(PUINT16 data_pointer)
{

    send16(4,           // numbits
            0,           // tdi
            0x0002); // tdo

    xchng16(16,           // numbits
            0,           // tdi
            0x8000,      // tms
            data_pointer); // tdo

    send16(4,           // numbits
            0,           // tdi
            0x0001); // tdo
}



int t_special_feature(unsigned char sub_cmd_num,  // Special feature number (sub_cmd_num)
	                     PUINT16 pInputLength,      // Length of Input Command
	                     PUINT8  pInputBuffer,      // Input Command Buffer
	                     PUINT16 pOutputLength,     // Length of Output Response
	                     PUINT8  pOutputBuffer)
{          // Output Response Buffer 
int i,num_swaps, index_num, tempnum;
UINT32 temp_address;
UINT16 temp_word;
*pOutputLength = 0;

switch (sub_cmd_num)
        {
        case 0xAA:  // Test Case
          for (i=1;i<=*pInputLength;i++) 
            pOutputBuffer[i-1] = pInputBuffer[i-1] ^ 0xff;
          *pOutputLength = *pInputLength;
          return (0) ; //success
          break;

        case 0x00: // Get value of the TDO line
                   // Bit7  TDO
          if (TDO_IN_SET) 
             pOutputBuffer[0] = 0x90;
             else
             pOutputBuffer[0] = 0x00;
          pOutputBuffer[1] = pOutputBuffer[0];
          *pOutputLength=2;
          return (0) ; //success
          break;
        case 0x01: // Set values directly on JTAG Port (1)
                   // Bit0  TDI  
          if (pInputBuffer[0] & 0x01) 
             TDI_OUT_SET();
             else
             TDI_OUT_RESET();
                   // Bit7  TMS
          if (pInputBuffer[0] & 0x80)
             TMS_SET();
             else
             TMS_RESET();
                   // Bit1  TCLK 
          if (pInputBuffer[0] & 0x02)
             TCLK_SET();
             else
             TCLK_RESET();
                   // Bit3  RESET            
          if (pInputBuffer[0] & 0x08)
             tRSTO_DIR = 0;				// RSTO pin is an input
             else {
              tRSTO = 1;					// assert reset_out signal
            	tRSTO_DIR = 1;				// drive the signal out             
             };
          PTED = (PTED & (output_buffer_enable_mask ^ 0xff)) | output_buffer_enable_value;
          return (0) ; //success
          break;
        case 0x02: // Set value directly on JTAG Port (2)
                   // Bit0  JCOMP (Inverted) 
          if (pInputBuffer[0] & 0x01) 
             TRST_RESET();
             else
             TRST_SET();
          PTED = (PTED & (output_buffer_enable_mask ^ 0xff)) | output_buffer_enable_value;
          return (0) ; //success
          break;

        case 0x05: // Swap Bytes Right Justified
                   // 5 Bytes define one 1-16 bit exchange (non compressed)
                   // 3 OR 1 Bytes define on compressed 1-16 bit exchange

          num_swaps = *((PUINT16) pInputBuffer);
          pInputBuffer+=2;
          for (i=0; i<num_swaps; i++) {
             tempnum = *((PUINT8) (pInputBuffer));
             if (tempnum<17) {
                xchng16(tempnum,
                     *((PUINT16) (pInputBuffer+1)), // tdi
                     *((PUINT16) (pInputBuffer+3)), // tms
                     (PUINT16) (pOutputBuffer+i*2)); // tdo
             pInputBuffer+=5;
             } else 
             {
             if (tempnum<tms_only_transaction_compression_start) {
                xchng16(tms_tdi_transaction_compression_array_bitsval[tempnum-tms_tdi_transaction_compression_start], 
                     tms_tdi_transaction_compression_array_tdival[tempnum-tms_tdi_transaction_compression_start],
                     tms_tdi_transaction_compression_array_tmsval[tempnum-tms_tdi_transaction_compression_start],
                     (PUINT16) (pOutputBuffer+i*2)); // tdo                               
                pInputBuffer+=1;
                } else 
                {              
                xchng16(tms_only_transaction_compression_array_bitsval[tempnum-tms_only_transaction_compression_start],
                     *((PUINT16) (pInputBuffer+1)), // tdi
                     tms_only_transaction_compression_array_tmsval[tempnum-tms_only_transaction_compression_start],
                     (PUINT16) (pOutputBuffer+i*2)); // tdo
                pInputBuffer+=3;
                }
                        
             }
                     
          }
          *pOutputLength=num_swaps*2;
          return (0) ; //success
          break;

        case 0x010: // Set compressed byte stream encodings TMS, TDI, Bits into Bits
                    //;     CMD : X DDEEF DDEEF DDEEF DDEEF DDEEF DDEEF DDEEF DDEEF          
                    //;                      X = Array Index Start (0 or 8)
                    //;                      DD = set comp_tdival_w[X+0..7]   
                    //;                      EE = set comp_tmsval_w[X+0..7]  
                    //;                      F = set comp_numbits_b[X+0..7]
                    // 
                    // 
                    
          index_num = *((PUINT8) pInputBuffer); // index into array 0 or 8
          for (i=0; i<8; i++) {
              tms_tdi_transaction_compression_array_tdival[i+index_num] = 
                                                           *((PUINT16) (pInputBuffer+1+i*5)); // tdi
              tms_tdi_transaction_compression_array_tmsval[i+index_num] = 
                                                           *((PUINT16) (pInputBuffer+3+i*5)); // tms
              tms_tdi_transaction_compression_array_bitsval[i+index_num] = 
                                                           *((PUINT8) (pInputBuffer+5+i*5)); // bits
                     
          }
          return (0) ; //success
          break;

        case 0x011: // Set compressed byte stream encodings TMS, Bits into Bits
                    //;     CMD : X DDE DDE DDE DDE DDE DDE DDE DDE
                    //;                      X = Array Index Start (0 or 8)
                    //;                      DD = set comp_tmsval_w[X+0..7]  
                    //;                      E = set comp_numbits_b[X+0..7]
                    // 
                    // 
                    
          index_num = *((PUINT8) pInputBuffer); // index into array 0 or 8
          for (i=0; i<8; i++) {
              tms_only_transaction_compression_array_tmsval[i+index_num] = 
                                                           *((PUINT16) (pInputBuffer+1+i*3)); // tms
              tms_only_transaction_compression_array_bitsval[i+index_num] = 
                                                           *((PUINT8) (pInputBuffer+3+i*3)); // bits
                     
          }
          return (0) ; //success
          break;

          case 0x33: // BDM/serial signal shutdown 
          {
          if (!sci_virtual_serial_port_is_enabled) { 
            OUT_EN = 0; // tristate JTAG signals
            }
          }
          *pOutputLength = 0;
          return (0);
          break;
          
          case 0x34: // BDM/serial signal enable 
          *pOutputLength = 0;
          return (0);
          break;

          case 0x40:   //dsc_write_pblock
          num_swaps = *((PUINT16) pInputBuffer);
          pInputBuffer+=2;
          temp_address = *((PUINT32) pInputBuffer);

          pInputBuffer+=4;
          eI3(0xE41A,0xFFFF,0x00FF);
          eI3(0xE418,temp_address & 0xFFFF,(temp_address >> 16) & 0xFFFF);
          for (i = 0; i < num_swaps;i++) {
              temp_word = *((PUINT16) pInputBuffer);
              pInputBuffer+=2;
              eI2(0x8745, temp_word);             
              eI1(0x8560);                    
          }
          return (0);
          break;

          case 0x41:     //dsc_write_dblock
          num_swaps = *((PUINT16) pInputBuffer);
          pInputBuffer+=2;
          temp_address = *((PUINT32) pInputBuffer);

          pInputBuffer+=4;
          eI3(0xE41A,0xFFFF,0x00FF);
          eI3(0xE418,temp_address & 0xFFFF,(temp_address >> 16) & 0xFFFF);
          for (i = 0; i < num_swaps;i++) {
              temp_word = *((PUINT16) pInputBuffer);
              pInputBuffer+=2;
              eI2(0x8745, temp_word);             
              eI1(0xD500);                    
          }
          return (0);
          break;

          case 0x42: // dsc_read_pblock
          num_swaps = *((PUINT16) pInputBuffer);
          pInputBuffer+=2;
          temp_address = *((PUINT32) pInputBuffer);
          pInputBuffer+=4;

          eI3(0xE41A,0xFFFF,0x00FF);
          eI3(0xE418,temp_address & 0xFFFF,(temp_address >> 16) & 0xFFFF);

          for (i = 0; i < num_swaps;i++) {
              eI1(0x8568);
              eI1(0xD516);
              dsc_send8_data(0x0089);        //; OCMDR_OTX1 or OCMDR_READ
              dsc_rcv_data16_to_buffer((PUINT16) (pOutputBuffer+i*2));
          }
          *pOutputLength=num_swaps*2;
          return (0);
          break;

          case 0x43: // dsc_read_dblock
          num_swaps = *((PUINT16) pInputBuffer);
          pInputBuffer+=2;
          temp_address = *((PUINT32) pInputBuffer);
          pInputBuffer+=4;

          eI3(0xE41A,0xFFFF,0x00FF);
          eI3(0xE418,temp_address & 0xFFFF,(temp_address >> 16) & 0xFFFF);

          for (i = 0; i < num_swaps;i++) {
              eI1(0xF500);
              eI1(0xD516);
              dsc_send8_data(0x0089);       // ; OCMDR_OTX1 or OCMDR_READ
              dsc_rcv_data16_to_buffer((PUINT16) (pOutputBuffer+i*2));
          }
          *pOutputLength=num_swaps*2;
          return (0);
          break;




        }
return (1); // failure
}



Once_Send_Receive(char bitcount, char outval){
	char b, inbit, lastbit;
	unsigned long inval, inmask;

	lastbit = bitcount-1;
	inmask = 0x01;
	inmask <<= lastbit;
	inval=0;

	for(b=0; b<bitcount; b++){
		if(b == lastbit)
            inbit = TCLK_transition(TMS_HIGH, (outval & 0x01));		// Shift transition  (Exit1)
		else
            inbit = TCLK_transition(TMS_LOW,  (outval & 0x01));		// Shift transition

		if(b>0){
			inval >>=1;			// prepare for bit
		}
		if(inbit>0){			// add bit input byte
			inval |= inmask;
		}
		outval >>=1;						// shift to next output bit
	}
	return inval;	// return byte received
}

// jtag scan i/o routine - input 8 to 32 bit binary value, return same format
static unsigned long
ScanIO(char ir, char bits, unsigned long val){
	unsigned long ival;

	TCLK_transition(TMS_HIGH,0);	// Select Data Scan

	if(ir == INSTRUCTION_REGISTER){
		TCLK_transition(TMS_HIGH,0);	// Select Instr Scan
	}
	TCLK_transition(TMS_LOW,0);		// Capture
	TCLK_transition(TMS_LOW,0);		// Shift

	ival = Once_Send_Receive(bits, val);	  // Debug Request instruction
	TCLK_transition(TMS_HIGH,0);	// Update
	TCLK_transition(TMS_LOW,0);		// Run-Test/Idle
	return ival;
}

static osbdm_error
dsc_config(int nst, int config_reg, unsigned long config_data)
{
    HFMUnit *unit;

    Nst = nst;
    if (config_reg >= dsccfg_hfm_unit_address &&
        config_reg <= dsccfg_hfm_unit_interleaved &&
        Cache[nst].unitList == 0)
    {
        /* unit list not initialized */
        return osbdm_error_invalid_parameter;
    }
    unit = &(Cache[nst].unitList[Cache[nst].config[dsccfg_hfm_set_unit]]);

    switch (config_reg)
    {
        case dsccfg_hfm_units:
            // always 1 for OSBDM
            if (config_data != 1)
            {
                return osbdm_error_invalid_parameter;
            }
            Cache[nst].config[dsccfg_hfm_units] = config_data;
            memset(Cache[nst].unitList, 0, config_data * sizeof(HFMUnit));
            break;
        case dsccfg_hfm_set_unit:
            /* set current flash unit */
            if (config_data >= Cache[nst].config[dsccfg_hfm_units])
            {
                return osbdm_error_invalid_parameter;
            }
            Cache[nst].config[dsccfg_hfm_set_unit] = config_data;
            break;
            
        case dsccfg_hfm_unit_address:
            Cache[nst].config[dsccfg_hfm_unit_address] = 
            unit->startAddr = config_data;
            break;
            
        case dsccfg_hfm_unit_size:
            Cache[nst].config[dsccfg_hfm_unit_size] = (config_data + 1) / 2;
            unit->endAddr = unit->startAddr + ((config_data + 1) / 2) - 1;
            break;
            
        case dsccfg_hfm_unit_space:
            Cache[nst].config[dsccfg_hfm_unit_space] = 
            unit->progMem = config_data;
            break;
            
        case dsccfg_hfm_unit_bank:
            Cache[nst].config[dsccfg_hfm_unit_bank] = 
            unit->bank = config_data;
            break;
            
        case dsccfg_hfm_unit_pagesize:
            Cache[nst].config[dsccfg_hfm_unit_pagesize] = 
            unit->pageSize = config_data;
            break;
            
        case dsccfg_hfm_unit_interleaved:
            Cache[nst].config[dsccfg_hfm_unit_interleaved] = 
            unit->interleaved = (unsigned char)config_data;
            break;
            
        case dsccfg_hfm_pram:
            Cache[nst].fast_mem = 1;
            Cache[nst].config[dsccfg_hfm_pram] =
            Cache[nst].fast_mem_addr = config_data;
            break;
            
        case dsccfg_hfm_activate:
            Cache[nst].config[dsccfg_hfm_activate] = config_data;
            if (config_data)
            {
                return activate_flash(nst);
            }
            else
            {
                return deactivate_flash(nst);
            }
            break;
        default:
            if (config_reg < dsccfg_count)
            {
                Cache[nst].config[config_reg] = config_data;
            }
            break;
    }
    return osbdm_error_ok;
}

static osbdm_error
dsc_core_run_mode(int nst, osbdm_status * mode)
{
    osbdm_error error;

    Nst = nst;
    error = read_status (mode);
    if(error)
    {
        error = read_status (mode);
        return_if(error);
    }

    if (Cache[nst].running && (*mode == osbdm_status_debug))
    {
        error = save_cache(nst, 0);
        return_if(error);
        Cache[nst].running = 0;
    }
    else if (!Cache[nst].running && (*mode != osbdm_status_debug))
    {
        Cache[nst].running = 1;
    }
    return error;
}

static osbdm_error
dsc_run_core(int nst)
{
    osbdm_error error;

    Nst = nst;
    error = restore_cache(nst, 0);
    return_if(error);
    error = run_from_pc(nst);
    return_if(error);
    Cache[nst].running = 1;

    return error;
}
static osbdm_error
run_from_pc (int nst)
{
    osbdm_error error;

    Nst = nst;
    error = write_once_gx(EONCE_NOREG, 0, 1, 0, 1);
    return_if(error);

    Cache[nst].running = 1;

    return error;
}

static osbdm_error
dsc_stop_core(int nst)
{
    int error;
    osbdm_status status;

    Nst = nst;
    error = dsc_core_run_mode(nst, &status);
    return_if(error);
    
    if (status != osbdm_status_debug)
    {
        error = debug_request();
        return_if(error);
    }
    return_if(error);

    if (Cache[nst].running)
    {
        Cache[nst].running = 0;
        error = save_cache(nst, 0);
    }
    return_if(error);

    return error;
}


static osbdm_error
dsc_reset_occurred (int nst, int debug_entered)
{
    int i;
    osbdm_error error;
    static unsigned char enable_eonce[]  = {JTAG_ENABLE_EONCE};
    unsigned long otxrxsr;
    unsigned char hawk_tlm_select[] = {JTAG_CORE_TLM_SELECT};
    unsigned char select_chip[] = {TLM_SELECT_CHIP_TAP};

    Nst = nst;

    /* Clear hsst download info */
    Cache[nst].fast_mem_loaded = 0;

    if (debug_entered)
    {
        /* Wait for no more than 5 seconds */
        for (i=0;i<50;i++)
        {
            error = wait_for_debug(1);
            if (!error) break;
            wait_ms(100);
        }
        if (error)
            error = wait_for_debug(1);
        return_if(error);

        read_register(dscreg_otxrxsr, &otxrxsr);
        error = save_cache(nst, 0);
        return_if(error);
        Cache[nst].running = 0;

    }
    else
    {
        unsigned char tlm_select[] = {JTAG_CHIP_TLM_SELECT};
        unsigned char select_dsc[] = {TLM_SELECT_CORE_TAP};
        Jtag_ScanOut(INSTRUCTION_REGISTER, JTAG_CHIP_LENGTH_IR, tlm_select, RUN_TEST_IDLE);
        Jtag_ScanOut(DATA_REGISTER, JTAG_DEFAULT_LENGTH_DR, select_dsc, RUN_TEST_IDLE);
        Jtag_ScanOut(INSTRUCTION_REGISTER, JTAG_CORE_LENGTH_IR, enable_eonce, RUN_TEST_IDLE);
        read_register(dscreg_otxrxsr, &otxrxsr);
        Cache[nst].running = 1;
    }

    write_register(dscreg_ocr, OCR_PWU);

    return osbdm_error_ok;
}

static osbdm_error
dsc_step_core(int nst, int instruction_count)
{
    osbdm_error error = osbdm_error_ok;

    Nst = nst;
    if (!instruction_count)
    {
        return osbdm_error_ok;
    }

    switch (Cache[nst].config[dsccfg_fast_stepping])
    {
        case HV2CFG_FAST_STEP_DISABLED:
            while (instruction_count--)
            {
                error = step_one(nst);
            }
            break;
        case HV2CFG_FAST_STEP_ONE_DISABLED:
            if (instruction_count == 1)
            {
                error = step_one(nst);
            }
            else
            {
                error = step_n(nst, instruction_count);
            }
            break;
        case HV2CFG_FAST_STEP_ENABLED:
            /* fallthrough */
        default:
            error = step_n(nst, instruction_count);
            break;
    }

    return error;
}

static osbdm_error
dsc_read_reg (int nst, unsigned long reg, unsigned long *value)
{
    osbdm_status status;
    osbdm_error error;

    Nst = nst;
    if (reg == 0x1000)
    {
        *value = Cache[nst].chiplevel_idcode;
        return osbdm_error_ok;
    }
    else if (reg == 0x1001)
    {
        unsigned char enable_eonce = JTAG_ENABLE_EONCE;
        unsigned char ir;
        Jtag_ScanIO(INSTRUCTION_REGISTER,JTAG_CORE_LENGTH_IR,&enable_eonce,&ir,RUN_TEST_IDLE);
        *value = ir;
        return osbdm_error_ok;
    }

    if (reg > dscreg_end)
    {
        return osbdm_error_invalid_parameter;
    }

    /* Not allowed to read, ever */
    switch (reg)
    {
        case dscreg_opdbr:
        case dscreg_orx:
        case dscreg_orx1:
        case dscreg_otx:
        case dscreg_otx1:
            *value = 0xbadbad;
            return osbdm_error_ok;
        default:
            break;
    }

    error = dsc_core_run_mode(nst, &status);
    return_if(error);

    if (status == osbdm_status_debug)
    {
        /* Not allowed to read during debug */
        switch (reg)
        {
            default:
                break;
        }
    }
    else
    {
        /* Allowed to read while running */
        switch (reg)
        {
            case dscreg_idcode:
            case dscreg_otxrxsr:
            case dscreg_osr:
            case dscreg_oscntr:
            case dscreg_ocr:
            case dscreg_obase:
            case dscreg_otbcr:
            case dscreg_otbpr:
            case dscreg_otb:
                break;
            default:
                return osbdm_error_core_not_responding;
        }
    }

#if CACHE_ENABLED
    /* Registers stored in the cache */
    switch (reg)
    {
        case dscreg_r0:
            *value = Cache[nst].r0;
            return osbdm_error_ok;
        case dscreg_r1:
            *value = Cache[nst].r1;
            return osbdm_error_ok;
        case dscreg_r2:
            *value = Cache[nst].r2;
            return osbdm_error_ok;
        case dscreg_r3:
            *value = Cache[nst].r3;
            return osbdm_error_ok;
        case dscreg_r4:
            *value = Cache[nst].r4;
            return osbdm_error_ok;
        case dscreg_a0:
            *value = Cache[nst].a0;
            return osbdm_error_ok;
        case dscreg_a1:
            *value = Cache[nst].a1;
            return osbdm_error_ok;
        case dscreg_a2:
            *value = Cache[nst].a2;
            return osbdm_error_ok;
        case dscreg_pc:
            *value = Cache[nst].pc;
            return osbdm_error_ok;
        case dscreg_omr:
            *value = Cache[nst].omr;
            return osbdm_error_ok;
        case dscreg_m01:
            *value = Cache[nst].m01;
            return osbdm_error_ok;
        case dscreg_x0:
            *value = Cache[nst].x0;
            return osbdm_error_ok;
        case dscreg_y0:
            *value = Cache[nst].y0;
            return osbdm_error_ok;
        case dscreg_y1:
            *value = Cache[nst].y1;
            return osbdm_error_ok;
        case dscreg_sr:
            *value = Cache[nst].sr;
            return osbdm_error_ok;
        default:
            break;
    }
#endif
    return read_register ((int)reg, value);
}

static osbdm_error
dsc_write_reg(int nst, unsigned long reg, unsigned long value)
{
    osbdm_error error;
    osbdm_status status;

    Nst = nst;
    if (reg > dscreg_end)
    {
        return osbdm_error_invalid_parameter;
    }

    /* Not allowed to written, ever */

    switch (reg)
    {
        case dscreg_idcode:
        case dscreg_opdbr:
        case dscreg_orx:
        case dscreg_orx1:
        case dscreg_otx:
        case dscreg_otx1:
            return osbdm_error_ok;
        default:
            break;
    }

    error = dsc_core_run_mode(nst, &status);
    return_if(error);

    if (status == osbdm_status_debug)
    {
        /* Not allowed to written during debug */
        switch (reg)
        {
            default:
                break;
        }
    }
    else
    {
        /* Allowed to write while running */
        switch (reg)
        {
            case dscreg_ocr:
            case dscreg_otbcr:
            case dscreg_otbpr:
            case dscreg_otb:
                break;
            default:
                return osbdm_error_core_not_responding;
        }
    }

#if CACHE_ENABLED
    /* Registers stored in the cache */
    switch (reg)
    {
        case dscreg_r0:
            Cache[nst].r0 = value;
            return osbdm_error_ok;
        case dscreg_r1:
            Cache[nst].r1 = value;
            return osbdm_error_ok;
        case dscreg_r2:
            Cache[nst].r2 = value;
            return osbdm_error_ok;
        case dscreg_r3:
            Cache[nst].r3 = value;
            return osbdm_error_ok;
        case dscreg_r4:
            Cache[nst].r4 = value;
            return osbdm_error_ok;
        case dscreg_a0:
            Cache[nst].a0 = value;
            return osbdm_error_ok;
        case dscreg_a1:
            Cache[nst].a1 = value;
            return osbdm_error_ok;
        case dscreg_a2:
            Cache[nst].a2 = value;
            return osbdm_error_ok;
        case dscreg_pc:
            if (Cache[nst].pc != value)
            {
                Cache[nst].pc = value;
                Cache[nst].pc_modified = 1;
            }
            return osbdm_error_ok;
        case dscreg_ocr:
            Cache[nst].ocr = value | OCR_PWU;
            return osbdm_error_ok;
            break;
        case dscreg_omr:
            Cache[nst].omr = value;
            return write_register (dscreg_omr, Cache[nst].omr & OMR_SA_MASK);
        case dscreg_m01:
            Cache[nst].m01 = value;
            return osbdm_error_ok;
        case dscreg_x0:
            Cache[nst].x0 = value;
            return osbdm_error_ok;
        case dscreg_y0:
            Cache[nst].y0 = value;
            return osbdm_error_ok;
        case dscreg_y1:
            Cache[nst].y1 = value;
            return osbdm_error_ok;
        case dscreg_sr:
            Cache[nst].sr = value;
            return osbdm_error_ok;
        default:
            break;
    }
#endif
    return write_register ((int)reg, value);
}

static osbdm_error
slow_read_mem (mem_address addr, unsigned long count, unsigned char mem[])
{
    osbdm_error error;
    unsigned long i;
    unsigned long data;
    dscreg otx;

    sequence_type * memseq;

    if (count % addr.size)
    {
        return osbdm_error_invalid_parameter;
    }

    switch (addr.space)
    {
        case dscmem_p:
            if (addr.size != 2)
            {
                return osbdm_error_invalid_parameter;
            }
            memseq = (sequence_type *)&(dsc_read_pmem_sequence[0]);
            break;
        case dscmem_x:  
            if (addr.size == 1  || addr.size == 2 || addr.size == 4)
            {
                memseq = (sequence_type *)&(dsc_read_xmem_sequence[addr.size]);
            }
            else
            {
                return osbdm_error_invalid_parameter;
            }
            break;
        default:
            return osbdm_error_invalid_parameter;
    }

    otx = memseq->datasize > 16 ? dscreg_otx : dscreg_otx1;

    error = write_register (dscreg_r0, addr.address);
    return_if(error);
    for (i=0;i<count;i+=addr.size)
    {
        error = execute_sequence (memseq);
        return_if(error);
        error = read_register (otx, &data);
        return_if(error);

        switch (addr.size)
        {
            case 1: mem[i] = (unsigned char)data; break;
            case 2: ulong_to_bigendian2 (data, mem+i); break;
            case 4: ulong_to_bigendian4 (data, mem+i); break;
            default: break;
        }

    }

    return error;
}

static osbdm_error
fast_read_mem(mem_address addr, unsigned long bcount, unsigned char mem[])
{
    unsigned long i, j;
    unsigned long cache_start = Cache[Nst].fast_mem_addr;
    unsigned long cache_end = Cache[Nst].fast_mem_addr + MEMCACHESIZE_WORDS;
    unsigned long cache_len = MEMCACHESIZE_BYTES;
    unsigned long wcount = bcount/2;
    osbdm_error error;
    /* starting address falls within range of cache */
    if (addr.address >= cache_start && addr.address <= cache_end)
    {
        i = 2*(addr.address - cache_start); /* how far into our cache it begins */
        memcpy(mem, Cache[Nst].fast_mem_cache+i, cache_len - i);
        for (j = 0;
             bcount && (addr.address < cache_end);
             i += addr.size, bcount -= addr.size, wcount--, addr.address += 1, j += addr.size)
        {
        }
        error = slow_read_mem(addr,bcount,mem+j);
        return_if(error);
    }
    /* ending address falls within range of cache */
    else if (addr.address + wcount >= cache_start && addr.address + wcount <= cache_end)
    {
        /* where in their memory we need to start filling */
        i = 2*(cache_start - addr.address);

        for ( j=0 ; i < bcount; i+=addr.size, j+=addr.size)
        {
            mem[j] = Cache[Nst].fast_mem_cache[i];
        }
        /* reduce count by amount we overlapped cache */
        wcount -= (addr.address + wcount) - cache_start;
        bcount = wcount*2;
        error = slow_read_mem(addr,bcount,mem);
    }
    /* completely overlaps our range of cache */
    else if (addr.address < cache_start && addr.address+wcount > cache_end)
    {
        mem_address lo_mem = addr;
        mem_address hi_mem = addr;

        hi_mem.address = cache_end + 1;

        i = 2*(cache_start - addr.address);
        for (j=0; j < cache_len; j++, i++)
        {
            mem[i] = Cache[Nst].fast_mem_cache[j];
        }

        error = slow_read_mem (lo_mem, 2*(cache_start - addr.address), mem);
        return_if(error);
        error = slow_read_mem (hi_mem, bcount-2*(cache_start-addr.address)-cache_len, mem + i);
        return_if(error);
    }
    /* doesn't have anything to do with our cache */
    else
    {
        error = slow_read_mem(addr, bcount, mem);
        return_if(error);
    }
    return error;
}

static osbdm_error
dsc_read_mem (int nst, mem_address addr, unsigned long count, unsigned char mem[])
{
    osbdm_error error;

    Nst = nst;
    if (Cache[nst].fast_mem_loaded)
    {
        if (addr.space == dscmem_p)
        {
            error = fast_read_mem(addr, count, mem);
        }
        else
        {
            if (addr.space != dscmem_x)
            {
                addr.space = dscmem_p;
            }
            error = slow_read_mem(addr, count, mem);
        }
    }
    else
    {
        if (addr.space > dscmem_x)
        {
            addr.space -= 2;
        }

        error = slow_read_mem(addr, count, mem);
    }
    return error;
}

static osbdm_error
slow_write_mem (mem_address addr, unsigned long count, unsigned char mem[])
{
    osbdm_error error;
    unsigned long i;
    unsigned long data;

    sequence_type * memseq;

    if (count % addr.size)
    {
        return osbdm_error_invalid_parameter;
    }

    switch (addr.space)
    {
        case dscmem_p:
            if (addr.size != 2)
            {
                return osbdm_error_invalid_parameter;
            }
            memseq = (sequence_type *)&(dsc_write_pmem_sequence[0]);
            break;
        case dscmem_x:
            if (!(addr.size == 1 || addr.size == 2 || addr.size == 4))
            {
                return osbdm_error_invalid_parameter;
            }
            memseq = (sequence_type *)&(dsc_write_xmem_sequence[addr.size]);
            break;
        default:
            return osbdm_error_invalid_parameter;
    }

    error = write_register (dscreg_r0, addr.address);
    return_if(error);
    for (i=0; i < count; i += addr.size)
    {
        switch (addr.size)
        {
            case 1:
                data = (unsigned long) mem[i];
                error = write_register (dscreg_a1, data);
                return_if(error);
                break;
            case 2:
                data = bigendian2_to_ulong (mem+i);
                error = write_register (dscreg_a1, data);
                return_if(error);
                break;
            case 4:
                data = bigendian4_to_ulong (mem+i);
                error = write_register (dscreg_a0, data & 0xFFFF);
                return_if(error);
                error = write_register (dscreg_a1, data >> 16);
                return_if(error);
                break;
        }
        error = execute_sequence (memseq);
        return_if(error);
    }

    return error;
    
}

static osbdm_error
start_fast_write(unsigned long address, unsigned long space, unsigned long wcount, HFMUnit *unit)
{
    osbdm_error error;
    unsigned long orx;

    if (!wcount) return osbdm_error_ok;

    if (space == dscmem_p_flash && !unit->interleaved)
    {
        space += 2;     /* indicate non-interleaved memory */
    }

    /* Begin executing routine */
    error = write_register(dscreg_pc, Cache[Nst].fast_mem_addr);
    return_if(error);
    error = write_once_gx(EONCE_NOREG, 0, 1, 0, 1);
    return_if(error);

    orx = address;
    error = write_register(dscreg_orx, orx);
    return_if(error);

    orx = wcount << 16 | space;
    error = write_register(dscreg_orx, orx);
    return_if(error);
    
    return osbdm_error_ok;
}

static osbdm_error
do_fast_write (mem_address addr, unsigned long bcount, unsigned char mem[])
{
    osbdm_error error;
    unsigned long wcount = bcount/2;
    unsigned long orx;
    unsigned long osr;
    unsigned long i;
    unsigned long j;
    unsigned char odd_addr = 0;
    unsigned char ocmdr_write_orx = 0x0B | OCMDR_WR;
    unsigned char orx_scandata[4];
    HFMUnit *unit = 0;

    if (!bcount)
        return osbdm_error_ok;

    if (addr.size != 2)
    {
        return slow_write_mem(addr,bcount,mem);
    }
    
    if (addr.space == dscmem_p_flash)
    {
        for (i = 0; i < Cache[Nst].config[dsccfg_hfm_units]; i++)
        {
            unit = &(Cache[Nst].unitList[i]);
            if (addr.space == unit->progMem &&
                addr.address >= unit->startAddr &&
                addr.address <= unit->endAddr)
                break;
        }
    }

    if (addr.space == dscmem_p_flash && unit->interleaved &&
        addr.address & 1)
    {
        odd_addr = 1;
        addr.address -= 1;
        wcount += 1;
    }
        
    error = start_fast_write(addr.address, addr.space, wcount, unit);
    return_if(error);

    if (odd_addr)
    {
        orx = (bigendian2_to_ulong(mem) << 16) | 0xFFFF;
        write_register(dscreg_orx, orx);
        i = 2;
    }
    else
    {
        i = 0;
    }
        
    for (; i < bcount-2; i+=4)
    {
    #if 1
        orx_scandata[0] = mem[i+1];
        orx_scandata[1] = mem[i+0];
        orx_scandata[2] = mem[i+3];
        orx_scandata[3] = mem[i+2];
        Jtag_ScanOut(DATA_REGISTER,  8, &ocmdr_write_orx, RUN_TEST_IDLE);
        Jtag_ScanOut(DATA_REGISTER, 32, orx_scandata,     RUN_TEST_IDLE);
    #else
        orx = (bigendian2_to_ulong(mem+i+2)<<16) | bigendian2_to_ulong(mem+i);
        write_register(dscreg_orx, orx);
    #endif
        /* Wait for orx to clear */
        j = 100;
        do
        {
            if (!j--)
                return osbdm_error_core_not_responding;
            read_register(dscreg_osr, &osr);
        } while (osr & 0x40);
    }
    /*
       We use an XOR here because if one condition is true,
       the other must be false in order to execute the
       bracketed code.
     */
    if (!!(bcount%4) ^ odd_addr)
    {
        if (addr.space == dscmem_p_flash || addr.space == dscmem_x_flash)
        {
            orx = (0xFFFF<<16) | bigendian2_to_ulong(mem+i);
        }
        else
        {
            orx = (bigendian2_to_ulong(mem+i)<<16) | bigendian2_to_ulong(mem+i);
        }
        write_register(dscreg_orx, orx);
    }
    error = wait_for_debug(WAIT_FOR_DEBUG_TIME);
    if (error)
    {
        debug_request();
        return error;
    }

    return error;
}

static osbdm_error
fast_write_mem (mem_address addr, unsigned long bcount, unsigned char mem[])
{
    unsigned long i, j;
    unsigned long cache_start = Cache[Nst].fast_mem_addr;
    unsigned long cache_end = Cache[Nst].fast_mem_addr + MEMCACHESIZE_WORDS;
    unsigned long cache_len = MEMCACHESIZE_BYTES;
    unsigned long cache_overlap;
    unsigned long wcount = bcount/2;
    osbdm_error error = osbdm_error_ok;

    if (addr.space != dscmem_p)
    {
        error = do_fast_write(addr, bcount, mem);
        return_if(error);
    }
    /* starting address falls within range of cache */
    else if (addr.address >= cache_start && addr.address <= cache_end)
    {
        i = 2*(addr.address - cache_start); /* how far into our cache it begins */
        cache_overlap = bcount < cache_len - i ? bcount : cache_len - i;
        memcpy(Cache[Nst].fast_mem_cache+i,mem,cache_overlap);
        for (j = 0;
             bcount && (addr.address < cache_end);
             i += addr.size, bcount -= addr.size, wcount-- , addr.address += 1, j += addr.size)
        {
        }
        error = do_fast_write(addr,bcount,mem+j);
        return_if(error);
    }
    /* ending address falls within range of cache */
    else if (addr.address + wcount >= cache_start && addr.address + wcount <= cache_end)
    {
        i = 2*(cache_start - addr.address);

        for ( j=0 ; i < bcount; i++, j++)
        {
            Cache[Nst].fast_mem_cache[j] = mem[i];
        }
        /* reduce count by amount we overlapped cache */
        wcount -= (addr.address + wcount) - cache_start;
        bcount = wcount*2;
        error = do_fast_write(addr,bcount,mem);
    }
    /* completely overlaps our range of cache */
    else if (addr.address < cache_start && addr.address+wcount > cache_end)
    {
        mem_address lo_mem = addr;
        mem_address hi_mem = addr;

        hi_mem.address = cache_end + 1;

        i = 2*(cache_start - addr.address);
        for (j=0; j < cache_len; j++, i++)
        {
            Cache[Nst].fast_mem_cache[j] = mem[i];
        }

        error = do_fast_write (lo_mem, cache_end - lo_mem.address, mem);
        return_if(error);
        error = do_fast_write (hi_mem, lo_mem.address + wcount - cache_end, mem + cache_end - lo_mem.address + 1);
        return_if(error);
    }
    /* doesn't have anything to do with our cache */
    else
    {
        error = do_fast_write(addr, bcount, mem);
        return_if(error);
    }
    return error;
}

static osbdm_error
fast_flush_cache(void)
{
    mem_address cache_mem;
    if(Cache[Nst].fast_mem_loaded)
    {
        cache_mem.address = Cache[Nst].fast_mem_addr;
        cache_mem.size = 2;
        cache_mem.space = dscmem_p;
        Cache[Nst].fast_mem_loaded = 0;
        return slow_write_mem(cache_mem,MEMCACHESIZE_BYTES,Cache[Nst].fast_mem_cache);
    }
    return osbdm_error_ok;
}

static osbdm_error
dsc_write_flash (int nst, mem_address addr, unsigned long count, unsigned char mem[])
{
    osbdm_error error;
    char regData[2];
    unsigned long hfmustat;
    unsigned long hfm_base = Cache[nst].config[dsccfg_hfm_base];
    HFMUnit *unit = &(Cache[nst].unitList[0]);
    unsigned long timeoutCount;
    mem_address hfm;
    int i;

    Nst = nst;
    if (!hfm_base)
    {
        return osbdm_error_invalid_parameter;
    }
    
    if (!Cache[nst].config[dsccfg_hfm_unit_size]     ||
        !Cache[nst].config[dsccfg_hfm_unit_pagesize] ||
        !Cache[nst].config[dsccfg_hfm_clock_divider])
    
    {
        return osbdm_error_invalid_parameter;
    }
   
    
    for (i = 0; i < Cache[nst].config[dsccfg_hfm_units]; i++)
    {
        unit = &(Cache[nst].unitList[i]);
        if (addr.space == unit->progMem &&
            addr.address >= unit->startAddr &&
            addr.address <= unit->endAddr)
            break;
    }
    if (i >= Cache[nst].config[dsccfg_hfm_units])
    {
        return osbdm_error_invalid_parameter;
    }

    ulong_to_bigendian2(unit->bank & HFMCR_BKSEL, regData);
    hfm.size = 2;
    hfm.space = dscmem_x;
    hfm.address = hfm_base + HFMCR_OFFSET;
    error = slow_write_mem(hfm, 2, regData);
    return_if(error);
    
    hfm.address = hfm_base + HFMUSTAT_OFFSET;
    error = slow_read_mem(hfm, 2, regData);
    return_if(error);
    hfmustat = bigendian2_to_ulong(regData);

    /* check for previous errors and clear them if necessary */
    if (hfmustat & HFMUSTAT_PVIOL)
    {
        ulong_to_bigendian2(HFMUSTAT_PVIOL, regData);
        error = slow_write_mem(hfm, 2, regData);
        return_if(error);
    }
    
    if (hfmustat & HFMUSTAT_ACCERR)
    {
        ulong_to_bigendian2(HFMUSTAT_ACCERR, regData);
        error = slow_write_mem(hfm, 2, regData);
        return_if(error);
    }
    
    if (hfmustat & HFMUSTAT_BLANK)
    {
        ulong_to_bigendian2(HFMUSTAT_BLANK, regData);
        error = slow_write_mem(hfm, 2, regData);
        return_if(error);
    }

    /* make sure flash unit is ready */
    timeoutCount = 0;
    while (timeoutCount < HFM_MAX_WRITE_COUNT)
    {
        if (hfmustat & HFMUSTAT_CBEIF)
            break;
            
        wait_ms(100);
        
        error = slow_read_mem(hfm, 2, regData);
        return_if(error);
        hfmustat = bigendian2_to_ulong(regData);

        timeoutCount++;
    }
    
    if (timeoutCount == HFM_MAX_WRITE_COUNT)
    {
        return osbdm_error_core_not_responding;
    }

    error = write_register(dscreg_r4, hfm_base);
    return_if(error);
    error = do_fast_write(addr, count, mem);
    return_if(error);

    /* check if programming is complete before continuing */
    timeoutCount = 0;
    while (timeoutCount < HFM_MAX_WRITE_COUNT)
    {
        error = slow_read_mem(hfm, 2, regData);
        return_if(error);
        hfmustat = bigendian2_to_ulong(regData);

        if (hfmustat & HFMUSTAT_CBEIF)
            break;
            
        wait_ms(100);
        
        timeoutCount++;
    }
    if (timeoutCount == HFM_MAX_WRITE_COUNT)
    {
        return osbdm_error_core_not_responding;
    }

    /* check for errors */
    if ((hfmustat & HFMUSTAT_PVIOL) ||
        (hfmustat & HFMUSTAT_ACCERR))
    {
        return osbdm_error_internal_failure;
    }

    if (error != osbdm_error_ok)
        return error;

    return osbdm_error_ok;
}

static osbdm_error
dsc_write_mem (int nst, mem_address addr, unsigned long count, unsigned char mem[])
{
    osbdm_error error;

    Nst = nst;
    if (addr.space == dscmem_p_flash || addr.space == dscmem_x_flash)
    {
        if (Cache[nst].fast_mem &&
            !Cache[nst].fast_mem_loaded)
        {
            mem_address pram;
            pram.size = 2;
            pram.space = dscmem_p;
            pram.address = Cache[nst].config[dsccfg_hfm_pram];
            error = slow_read_mem(pram, MEMCACHESIZE_BYTES, Cache[nst].fast_mem_cache);
            return_if(error);
            error = slow_write_mem(pram, dscxfer_len, (unsigned char *)dscxfer);
            return_if(error);
            Cache[nst].fast_mem_loaded = 1;
        }
        return dsc_write_flash(nst, addr, count, mem);
    }

    /* write is big enough, p memory & not already loaded */
    if (Cache[nst].config[dsccfg_hsst_download] &&
        !Cache[nst].fast_mem_loaded &&
        (count >= MEMCACHESIZE_BYTES) &&
        (addr.size ==  2) &&
        (addr.space == dscmem_p))
    {
        mem_address fastmem = addr;

        error = slow_write_mem(fastmem, dscxfer_len, (unsigned char *)dscxfer);
        return_if(error);
        Cache[nst].fast_mem_loaded = 1;
        Cache[nst].fast_mem_addr = addr.address;
        fast_write_mem(addr, MEMCACHESIZE_BYTES, mem);
        count -= MEMCACHESIZE_BYTES;
        addr.address += MEMCACHESIZE_WORDS;
        error = do_fast_write(addr, count, mem+MEMCACHESIZE_BYTES);
        return error;
    }

    if (Cache[nst].config[dsccfg_hsst_download] &&
        Cache[nst].fast_mem_loaded &&
        (addr.space == dscmem_p || addr.space == dscmem_x))
    {
        error = fast_write_mem (addr, count, mem);
    }
    else
    {
        if (addr.space > 1)
        {
            addr.space -= 2;
        }
        error = slow_write_mem (addr, count, mem);
    }
    return error;
}

static osbdm_error
dsc_erase_flash (int nst, mem_address addr, unsigned long count, unsigned char mem[])
{
    osbdm_error error;
    char regData[2];
    unsigned long hfmustat;
    unsigned long hfm_base = Cache[nst].config[dsccfg_hfm_base];
    HFMUnit *unit = &(Cache[nst].unitList[0]);
    unsigned long timeoutCount;
    unsigned long readCount = 2;
    mem_address hfm;
    mem_address dummy = addr;
    int i;

    QUIET(mem);
    Nst = nst;
    if (!hfm_base)
    {
        return osbdm_error_invalid_parameter;
    }
    
    if (!Cache[nst].config[dsccfg_hfm_unit_size]     ||
        !Cache[nst].config[dsccfg_hfm_unit_pagesize] ||
        !Cache[nst].config[dsccfg_hfm_clock_divider])
    
    {
        return osbdm_error_invalid_parameter;
    }
 
       
    for (i = 0; i < Cache[nst].config[dsccfg_hfm_units]; i++)
    {
        unit = &(Cache[nst].unitList[i]);
        if (addr.space == unit->progMem &&
            addr.address >= unit->startAddr &&
            addr.address <= unit->endAddr)
            break;
    }
    if (i >= Cache[nst].config[dsccfg_hfm_units])
    {
        return osbdm_error_invalid_parameter;
    }

    ulong_to_bigendian2(unit->bank & HFMCR_BKSEL, regData);
    hfm.size = 2;
    hfm.space = dscmem_x;
    hfm.address = hfm_base + HFMCR_OFFSET;
    error = slow_write_mem(hfm, 2, regData);
    return_if(error);
        
    /* make sure flash unit is ready */
    timeoutCount = 0;
    while (timeoutCount < HFM_MAX_ERASE_COUNT)
    {
        hfm.address = hfm_base + HFMUSTAT_OFFSET;
        error = slow_read_mem(hfm, readCount, regData);
        return_if(error);
        hfmustat = bigendian2_to_ulong(regData);
            
        /* check for previous errors and clear them if necessary */
        if (hfmustat & HFMUSTAT_PVIOL)
        {
            ulong_to_bigendian2(HFMUSTAT_PVIOL, regData);
            error = slow_write_mem(hfm, 2, regData);
            return_if(error);
        }
        
        if (hfmustat & HFMUSTAT_ACCERR)
        {
            ulong_to_bigendian2(HFMUSTAT_ACCERR, regData);
            error = slow_write_mem(hfm, 2, regData);
            return_if(error);
        }
        
        if (hfmustat & HFMUSTAT_BLANK)
        {
            ulong_to_bigendian2(HFMUSTAT_BLANK, regData);
            error = slow_write_mem(hfm, 2, regData);
            return_if(error);
        }
            
        if (hfmustat & HFMUSTAT_CBEIF)
            break;
            
        wait_ms(100);
        timeoutCount++;
    }
    
    if (timeoutCount == HFM_MAX_ERASE_COUNT)
        return osbdm_error_core_not_responding;

    /* write dummy data to beginning of flash */
    dummy.space -= 2;
    error = slow_write_mem(dummy, 2, regData);
    return_if(error);
        
    /* write erase command */
    hfm.space = dscmem_x;
    hfm.address = hfm_base + HFMCMD_OFFSET;
    if (addr.address == unit->startAddr &&
        (addr.address + ((count + 1) / 2) - 1) == unit->endAddr)
    {
        ulong_to_bigendian2(HFM_CMD_MASS_ERASE, regData);
    }
    else
    {
        ulong_to_bigendian2(HFM_CMD_PAGE_ERASE, regData);
    }
    error = slow_write_mem(hfm, 2, regData);
    return_if(error);
        
    /* start operation */
    ulong_to_bigendian2(HFMUSTAT_CBEIF, regData);
    hfm.address = hfm_base + HFMUSTAT_OFFSET;
    error = slow_write_mem(hfm, 2, regData);
    return_if(error);
        
    /* check if erase is complete before continuing */
    timeoutCount = 0;
    while (timeoutCount < HFM_MAX_ERASE_COUNT)
    {
        error = slow_read_mem(hfm, readCount, regData);
        return_if(error);
        hfmustat = bigendian2_to_ulong(regData);
            
        if (hfmustat & HFMUSTAT_CCIF)
            break;
            
        wait_ms(100);
        timeoutCount++;
    }
    if (timeoutCount == HFM_MAX_ERASE_COUNT)
        return osbdm_error_core_not_responding;
        
    /* check for errors */
    if ((hfmustat & HFMUSTAT_PVIOL) ||
        (hfmustat & HFMUSTAT_ACCERR))
    {
        return osbdm_error_internal_failure;
    }

    if (error != osbdm_error_ok)
        return error;
        
    return osbdm_error_ok;
}

static osbdm_error
dsc_fill_mem (int nst, mem_address addr, unsigned long count, unsigned char mem[])
{
    osbdm_error error;
    unsigned long i;
    unsigned long data;
    sequence_type *memseq;

    Nst = nst;
    if (addr.space == dscmem_p_flash || addr.space == dscmem_x_flash)
    {
        if (Cache[nst].fast_mem &&
            !Cache[nst].fast_mem_loaded)
        {
            mem_address pram;
            pram.size = 2;
            pram.space = dscmem_p;
            pram.address = Cache[nst].config[dsccfg_hfm_pram];
            error = slow_read_mem(pram, MEMCACHESIZE_BYTES, Cache[nst].fast_mem_cache);
            return_if(error);
            error = slow_write_mem(pram, dscxfer_len, (unsigned char *)dscxfer);
            return_if(error);
            Cache[nst].fast_mem_loaded = 1;
        }
        return dsc_erase_flash(nst, addr, count, mem);
    }

    if (count % addr.size)
    {
        return osbdm_error_invalid_parameter;
    }

#if 0
    switch (addr.size)
    {
        case 1: data = (unsigned long) *mem; break;
        case 2: data = bigendian2_to_ulong (mem); break;
        case 4: data = bigendian4_to_ulong (mem); break;
        default: data = 0; break;
    }

    switch (addr.space)
    {
        case dscmem_p:
            if (addr.size != 2)
            {
                return osbdm_error_invalid_parameter;
            }
            memseq = (sequence_type *)&(dsc_fill_pmem_sequence[0]);
            break;
        case dscmem_x:
            if (!(addr.size == 1 || addr.size == 2 || addr.size == 4))
            {
                return osbdm_error_invalid_parameter;
            }
            memseq = (sequence_type *)&(dsc_fill_xmem_sequence[addr.size]);
            break;
        default:
            return osbdm_error_invalid_parameter;
    }
    if (memseq->datasize > 16)
    {
        error = write_register (dscreg_orx, data);
        error = execute_sequence ((sequence_type *)dsc_fill_xmem_sequence); /*small hack... xmem[0] writes reg a10*/
    }
    else
    {
        error = write_register (dscreg_a1, data);
        return_if(error);
    }

    error = write_register (dscreg_r0, addr.address);
    return_if(error);

#else

    switch (addr.space)
    {
        case dscmem_p:
            if (addr.size != 2)
            {
                return osbdm_error_invalid_parameter;
            }
            memseq = (sequence_type *)&(dsc_write_pmem_sequence[0]);
            break;
        case dscmem_x:
            if (!(addr.size == 1 || addr.size == 2 || addr.size == 4))
            {
                return osbdm_error_invalid_parameter;
            }
            memseq = (sequence_type *)&(dsc_write_xmem_sequence[addr.size]);
            break;
        default:
            return osbdm_error_invalid_parameter;
    }

    error = write_register (dscreg_r0, addr.address);
    return_if(error);

    switch (addr.size)
    {
        case 1:
            data = (unsigned long) mem[0];
            error = write_register (dscreg_a1, data);
            return_if(error);
            break;
        case 2:
            data = bigendian2_to_ulong (mem);
            error = write_register (dscreg_a1, data);
            return_if(error);
            break;
        case 4:
            data = bigendian4_to_ulong (mem);
            error = write_register (dscreg_a0, data & 0xFFFF);
            return_if(error);
            error = write_register (dscreg_a1, data >> 16);
            return_if(error);
            break;
    }
#endif

    for (i=0;i<count;i+=addr.size)
    {
        error = execute_sequence (memseq);
        return_if(error);
    }

    return error;
}

/*****************************************************************/

/* BEGIN STUPID HAWKV2 STEPPING WORKAROUND */
typedef struct nonint_info_tag {
    instr_type instr;
    int length;
    int skid;
    int chof;
    int nonint;
} nonint_info;


static instr_type swi = {0xE600, 0xFFFC}; /* SWI #x */

static instr_type addw = {0x4040, 0xFC40}; /* ADD.W EEE,X:(SP-xx) */

static nonint_info addw_nonint[] = {
    {{0xE702, 0xFFFF}, 2, 0, 0, 0}, /* ADD.W EEE,X:(SP-xx) */
    {{0x0000, 0x0000}, 1, 1, 0, 1}, /* ADD.W X:(SP-xx),EEE */
};

static int addw_nonint_length = sizeof(addw_nonint)/sizeof(addw_nonint[0]);

static instr_type prefix = {0xE030, 0xF8B0}; /* Prefix word */

static nonint_info prefix_nonint[] = {
    {{0xE36C, 0xFFFF}, 3, 2, 1, 0}, /* BRAD xxxxxx - interruptable */
    {{0xE26C, 0xFFFF}, 3, 1, 1, 0}, /* BSR xxxxxx - interruptable */
    {{0xE16C, 0xFFFF}, 3, 1, 1, 0}, /* BRA xxxxxx - interruptable */
    {{0x8A54, 0xFFFF}, 4, 2, 1, 1}, /* BRCLR #MASK8,X:xxxxxx,Aa */
    {{0x8E54, 0xFFFF}, 4, 2, 1, 1}, /* BRSET #MASK8,X:xxxxxx,Aa */
    {{0xE354, 0xFFFF}, 3, 2, 1, 0}, /* JMPD xxxxxx - interruptale */
    {{0xE254, 0xFFFF}, 3, 1, 1, 0}, /* JSR xxxxxx - interruptable */
    {{0xE154, 0xFFFF}, 3, 1, 1, 0}, /* JMP xxxxxx - interruptable */
    {{0xE068, 0xF8FB}, 3, 1, 1, 1}, /* Bcc xxxxxx */
    {{0xE050, 0xF8FB}, 3, 1, 1, 1}, /* Jcc xxxxxx */
};

int prefix_nonint_length = sizeof(prefix_nonint)/sizeof(prefix_nonint[0]);

static nonint_info short_nonint[] = {
    {{0x8A54, 0xFFFF}, 3, 2, 1, 1}, /* BRCLR #MASK8,X:xxxx,Aa */
    {{0x8E54, 0xFFFF}, 3, 2, 1, 1}, /* BRSET #MASK8,X:xxxx,Aa */
    {{0xE36C, 0xFFFC}, 2, 2, 1, 0}, /* BRAD xxxxx - interruptable */
    {{0xE26C, 0xFFFC}, 2, 0, 1, 0}, /* BSR xxxxx - interruptable */
    {{0xE16C, 0xFFFC}, 2, 1, 1, 0}, /* BRA xxxxx - interruptable */
    {{0x8A50, 0xFFFC}, 2, 2, 1, 1}, /* BRCLR #MASK8,dd,Aa */
    {{0x8E50, 0xFFFC}, 2, 2, 1, 1}, /* BRSET #MASK8,dd,Aa */
    {{0x8A44, 0xFFF4}, 3, 2, 1, 1}, /* BRCLR #MASK8,X:(Rn+xxxx),Aa */
    {{0x8A40, 0xFFF4}, 2, 2, 1, 1}, /* BRCLR #MASK8,X:(Rn),Aa */
    {{0x8E40, 0xFFF4}, 2, 2, 1, 1}, /* BRSET #MASK8,X:(Rn),Aa */
    {{0x8E44, 0xFFF4}, 3, 2, 1, 1}, /* BRSET #MASK8,X:(Rn+xxxx),Aa */
    {{0xE354, 0xFFF4}, 2, 2, 1, 0}, /* JMPD xxxxx - interruptable */
    {{0xE254, 0xFFF4}, 2, 0, 1, 0}, /* JSR xxxxx - interruptable */
    {{0xE154, 0xFFF4}, 2, 1, 1, 0}, /* JMP xxxxx - interruptable */
    {{0x706F, 0xF87F}, 1, 1, 0, 1}, /* Tcc ~F,F R0,R1 */
    {{0x8B40, 0xFFE0}, 2, 2, 1, 1}, /* BRCLR #MASK8,DDDDD,Aa */
    {{0x8F40, 0xFFE0}, 2, 2, 1, 1}, /* BRSET #MASK8,DDDDD,Aa */
    {{0xE068, 0xF8F8}, 2, 1, 1, 1}, /* Bcc xxxxx */
    {{0xAAC0, 0xFFC0}, 2, 2, 1, 1}, /* BRCLR #MASK8,X:(SP-xx),Aa */
    {{0xABC0, 0xFFC0}, 2, 2, 1, 1}, /* BRCLR #MASK8,X:Ppp,Aa */
    {{0xAEC0, 0xFFC0}, 2, 2, 1, 1}, /* BRSET #MASK8,X:(SP-xx),Aa */
    {{0xAFC0, 0xFFC0}, 2, 2, 1, 1}, /* BRSET #MASK8,X:Ppp,Aa */
    {{0x704F, 0xF84F}, 1, 1, 0, 1}, /* Tcc DD,F R0,R1 */
    {{0xAB00, 0xFF80}, 1, 2, 1, 0}, /* BRAD <aa> - interruptable */
    {{0xA900, 0xFF80}, 1, 0, 1, 0}, /* BRA <aa> - interruptable */
    {{0xE050, 0xF8F0}, 2, 1, 1, 1}, /* Jcc xxxxx */
    {{0x4040, 0xFC40}, 1, 1, 0, 0}, /* ADD.W EEE,X:(SP-xx) */
    {{0x4040, 0xFC40}, 1, 1, 0, 1}, /* ADD.W X:(SP-xx),EEE */
    {{0xA000, 0xF080}, 1, 2, 1, 1}, /* Bcc <aa> */
};
static int short_nonint_length = sizeof(short_nonint)/sizeof(short_nonint[0]);

static int
instr_match (unsigned short instr, instr_type cmpi)
{
    return ((instr & cmpi.mask) == cmpi.word);
}

static void
get_instr_info(unsigned short * instr, int * nonint, int * length, int * skid, int * chof)
{
    unsigned short iword;
    nonint_info * nonint_table;
    int entries;
    int i;

    *nonint = 0;
    *length = 0;
    *skid = 0;
    *chof = 0;
    
    if (instr_match(instr[0], prefix))
    {
        /* this is a prefix word, use next short as instruction */
        iword = instr[1];
        /* use the prefix noninterruptable instruction table */
        nonint_table = prefix_nonint;
        entries = prefix_nonint_length;
    }
    else if (instr_match(instr[0], addw))
    {
        /* this is an addw use next short as instruction */
        iword = instr[1];
        /* use the addw noninterruptable instruction table */
        nonint_table = addw_nonint;
        entries = addw_nonint_length;
    }
    else
    {
        /* this is a normal instruction */
        iword = instr[0];
        /* use the short noninterruptable instruction table */
        nonint_table = short_nonint;
        entries = short_nonint_length;
    }

    /* find an entry in the selected table */
    for (i=0; i < entries; i++)
    {
        if(instr_match(iword, nonint_table[i].instr))
        {
            *nonint = nonint_table[i].nonint;
            *length = nonint_table[i].length;
            *skid = nonint_table[i].skid;
            *chof = nonint_table[i].chof;
            break;
        }
    }
}

#define TB_FULL_NOACTION 0x00
#define TB_FULL_HALT 0x01
#define TB_FULL_DEBUG 0x02
#define TB_FULL_INTERRUPT 0x03
#define TB_HALT 0x04
#define TB_TRACE_COF_NOT_TAKEN 0x08
#define TB_TRACE_COF_INTERRUPT 0x10
#define TB_TRACE_COF_SUBROUTINE 0x20
#define TB_TRACE_COF_FORWARD 0x40
#define TB_TRACE_COF_BACKWARD 0x80

static void
setup_trace_buffer_for_branch_detect(void)
{
    write_register(dscreg_otbpr, 0);
    write_register(dscreg_otbcr, TB_FULL_DEBUG|TB_TRACE_COF_FORWARD|TB_TRACE_COF_BACKWARD);
}

static int
branch_taken(void)
{
    unsigned long otbpr;
    read_register(dscreg_otbpr, &otbpr);
    write_register(dscreg_otbpr, 0);
    write_register(dscreg_otbcr, 0);
    return (int)otbpr;
}


static osbdm_error
step_one(int nst)
{
    osbdm_error error;
    mem_address pc;
    mem_address nopmem;
    mem_address bkpmem;
    int i;
    int length;
    int nonint;
    int skid;
    int chof;
    unsigned char nops[] = {0xE7,0x00,0xE7,0x00};
    unsigned char bkpt[] = {0xE7,0x00,0xE7,0x00,0xE7,0x01};
    unsigned char instruction_bytes[12];
    unsigned char readtest[12];
    unsigned char in_ram = 1;
    unsigned short instr[sizeof(instruction_bytes)/2];
    osbdm_status mode;

    Nst = nst;
    pc.address = Cache[nst].pc;
    pc.space = dscmem_p;
    pc.size = 2;
    
    /* read current instruction and up to the next 3 instructions which may be skidded over */
    error = dsc_read_mem(nst, pc, sizeof(instruction_bytes), instruction_bytes);
    return_if(error);
    for (i=0;i<(sizeof(instruction_bytes)/2);i++)
    {
        instr[i] = (unsigned short)bigendian2_to_ulong(instruction_bytes+(i*2));
    }

    if (instr_match(instr[0],swi))
    {
        /* for SWI instructions need to set a breakpoint instead of hardware stepping */
        bkpmem.address = pc.address + 1;
        bkpmem.space = pc.space;
        bkpmem.size = pc.size;
        error = dsc_write_mem(nst, bkpmem, sizeof(bkpt)/sizeof(bkpt[0]), bkpt);
        return_if(error);
        
        /* verify that the write took - if it didn't we're in ROM/Flash */
        error = dsc_read_mem(nst,bkpmem, sizeof(bkpt)/sizeof(bkpt[0]), readtest);
        return_if(error);
        for (i=0;i<sizeof(bkpt)/sizeof(bkpt[0]);i++)
        {
            if (bkpt[i] != readtest[i])
            {
                in_ram = 0;
                break;
            }
        }
        error = dsc_run_core(nst);
        return_if(error);
        error = wait_for_debug(100);
        return_if(error);
        error = dsc_core_run_mode(nst, &mode);
        return_if(error);
        if (in_ram && (Cache[nst].pc == (pc.address + 4)))
        {
            /* if pc is here, then we are not at an ISR breakpoint so back up PC*/
            Cache[nst].pc = pc.address + 1;
        }
        error = dsc_write_mem(nst, bkpmem, sizeof(bkpt)/sizeof(bkpt[0]), instruction_bytes+2);
        return_if(error);
    }
    else
    {
        get_instr_info (instr, &nonint, &length, &skid, &chof);
        if (nonint)
        {
            /* fill instructions skidded over with NOPs */
            nopmem.address = pc.address + length;
            nopmem.space = pc.space;
            nopmem.size = pc.size;
            error = dsc_write_mem(nst, nopmem, (unsigned long)(skid*2), nops);
            return_if(error);

            /* verify that the write took - if it didn't we're in ROM/Flash */
            error = dsc_read_mem(nst,nopmem, (unsigned long)(skid*2), readtest);
            return_if(error);
            for (i=0;i<(skid*2);i++)
            {
                if (nops[i] != readtest[i])
                {
                    in_ram = 0;
                    break;
                }
            }

            if (in_ram && chof)
            {
                setup_trace_buffer_for_branch_detect();
            }

            error = step_n(nst, 1);
            return_if(error);

            if (in_ram)
            {
                if (chof)
                {
                    if(!branch_taken())
                    {
                        /* if a branch is not taken, there is a skid */
                        /* back up pc to before the skid */
                        Cache[nst].pc = pc.address + length;
                    }
                }
                else
                {
                    /* back up pc to before the skid */
                    Cache[nst].pc = pc.address + length;
                }
            }
            /* restore skidded over instructions */
            error = dsc_write_mem(nst, nopmem, (unsigned long)(skid*2), instruction_bytes+(length*2));
            return_if(error);

        }
        else
        {
            /* easy case - no skidding */
            error = step_n(nst, 1);
            return_if(error);
        }
    }

    return error;
}

static osbdm_error
step_n(int nst, int count)
{
    osbdm_error error;
    osbdm_status mode;
    unsigned long save_ocr;

    Nst = nst;
    save_ocr = Cache[nst].ocr;
    Cache[nst].ocr = OCR_PWU|OCR_ISC_SINGLE_STEP;
    error = write_register (dscreg_oscntr, (unsigned long)count-1);
    return_if(error);
    error = dsc_run_core(nst);
    return_if(error);
    error = wait_for_debug((unsigned long)count);
    return_if(error);
    error = dsc_core_run_mode(nst, &mode);
    Cache[nst].ocr = save_ocr;
    return_if(error);
    return error;
}
/* END STUPID HAWKV2 STEPPING WORKAROUND */

static osbdm_error
read_status (osbdm_status * status)
{
    unsigned char enable_eonce[] = {JTAG_ENABLE_EONCE};
    unsigned char core_status;
    osbdm_error error=osbdm_error_ok;

    Jtag_ScanIO (INSTRUCTION_REGISTER, JTAG_CORE_LENGTH_IR, enable_eonce, &core_status, RUN_TEST_IDLE);

    core_status &= 0xF;

    switch (core_status)
    {
        case    IR_MODE_DEBUG:
            *status = osbdm_status_debug;
            break;
        case    IR_MODE_USER:
            *status = osbdm_status_execute;
            break;
        case    IR_MODE_EXACC:
            *status = osbdm_status_waitstate;
            break;
        case    IR_MODE_STOP:
            *status = osbdm_status_sleep;
            break;
        case    IR_MODE_DISCONNECTED:
            *status = osbdm_status_undefined;
            error = osbdm_error_core_not_responding;
            break;
        default:
            *status = osbdm_status_undefined;
            error = osbdm_error_core_not_responding;
            break;
    }

    return error;
}

#if 0
/* This method doesn't work for Angila and beyond */
static osbdm_error
execute_instruction (instruction_type * instr)
{
    osbdm_error error;
    unsigned char ocmdrgo[] = {OPDBR_ADDR | OCMDR_GO | OCMDR_WR};
    unsigned char * ocmdr = ocmdrgo;
    int words = instr->size;

    Jtag_ScanOut (DATA_REGISTER, OCMDR_LENGTH, ocmdr, RUN_TEST_IDLE);
    Jtag_ScanOut (DATA_REGISTER, words*16, instr->bytes, RUN_TEST_IDLE);

    error = wait_for_debug(WAIT_FOR_DEBUG_TIME);
    return_if(error);

    return osbdm_error_ok;
}
#else
/* This method should work for everything (old and new) */
static osbdm_error
execute_instruction (instruction_type * instr)
{
    osbdm_error error;
    unsigned char ocmdr[] = {OPDBR_ADDR | OCMDR_WR};
    unsigned char * data;
    int i;

    for (i = 0; i < instr->size; i++)
    {
        if ((i+1) == (instr->size))
        {
            *ocmdr |= OCMDR_GO;
        }

        data = (unsigned char *)instr->bytes + (i*2);

        Jtag_ScanOut (DATA_REGISTER, OCMDR_LENGTH, ocmdr, RUN_TEST_IDLE);
        Jtag_ScanOut (DATA_REGISTER, 16, data, RUN_TEST_IDLE);
    }

    error = wait_for_debug(WAIT_FOR_DEBUG_TIME);
    return_if(error);

    return osbdm_error_ok;
}
#endif

static osbdm_error
wait_for_debug (unsigned long count)
{
    osbdm_status status;
    osbdm_error error;

    if (!count)
    {
        count=1;
    }
    
    /* 
     *  dsc is broken here. You must read status twice and discard the results
     *  before you read status for the real time the 3rd time.
     */
    read_status(&status);
    read_status(&status);

    while (count--)
    {
        error = read_status(&status);
        return_if(error);

        if (status == osbdm_status_debug)
        {
            return osbdm_error_ok;
        }
    }
    return osbdm_error_core_not_responding;
}

static osbdm_error
execute_sequence (sequence_type * seq)
{
    int i;
    osbdm_error error = osbdm_error_ok;

    for (i=0; i < seq->count; i++)
    {
        error = execute_instruction ((instruction_type *)&(seq->instr[i]));
        return_if(error);
    }

    return (error);

}

static osbdm_error
save_cache(int nst, int stepping_enabled)
{
    osbdm_error error;
#if CACHE_ENABLED
    #define save_cache_reg(x) error = read_register(dscreg_##x, &Cache[nst].x); return_if(error)
    
    Nst = nst;

    save_cache_reg(ocr);
    Cache[nst].ocr |= OCR_PWU;
    return_if(error);
    write_register(dscreg_ocr, OCR_PWU);

    save_cache_reg(r4);
    save_cache_reg(r0);
    save_cache_reg(r1);
    save_cache_reg(r2);
    save_cache_reg(r3);
    save_cache_reg(pc);
    save_cache_reg(a0);
    save_cache_reg(a1);
    save_cache_reg(a2);
    save_cache_reg(omr);
    save_cache_reg(m01);
    save_cache_reg(x0);
    save_cache_reg(y0);
    save_cache_reg(y1);
    save_cache_reg(sr);

    error = write_register (dscreg_omr, Cache[nst].omr & OMR_SA_MASK);
    return_if(error);
    error = write_register (dscreg_m01, M01_NO_MODULO);
    return_if(error);
    error = write_register (dscreg_sr, Cache[nst].sr | SR_DISABLE_INTERRUPTS);
    return_if(error);
    Cache[nst].pc_modified = 0;

#endif
    QUIET(stepping_enabled);

    return osbdm_error_ok;
}

static osbdm_error
restore_cache(int nst, int stepping_enabled)
{
    osbdm_error error;

#if CACHE_ENABLED
    #define restore_cache_reg(x) error = write_register(dscreg_##x, Cache[nst].x); return_if(error)

    Nst = nst;
    fast_flush_cache();

    restore_cache_reg(sr);
    restore_cache_reg(y1);
    restore_cache_reg(y0);
    restore_cache_reg(x0);
    restore_cache_reg(m01);
    restore_cache_reg(omr);
    restore_cache_reg(a2);
    restore_cache_reg(a1);
    restore_cache_reg(a0);
    restore_cache_reg(pc);
    restore_cache_reg(r3);
    restore_cache_reg(r2);
    restore_cache_reg(r1);
    restore_cache_reg(r0);
    restore_cache_reg(r4);
    restore_cache_reg(ocr);

#endif
    QUIET(stepping_enabled);
    return osbdm_error_ok;
}

static osbdm_error
write_once_gx (unsigned char regaddr, unsigned long value, int length, int go, int exit)
{
    unsigned char ocmdr;
    unsigned char val[4];

    ocmdr = (unsigned char)(regaddr | OCMDR_WR);
    if (go)
    {
        ocmdr |= OCMDR_GO;
    }
    if (exit)
    {
        ocmdr |= OCMDR_EX;
    }

    ulong_to_littleendian4(value, val);

    Jtag_ScanOut(DATA_REGISTER, OCMDR_LENGTH, &ocmdr, RUN_TEST_IDLE);
    if (regaddr != EONCE_NOREG)
    {
        Jtag_ScanOut(DATA_REGISTER, length, val, RUN_TEST_IDLE);
    }

    return osbdm_error_ok;
}

static osbdm_error
write_once_register (dscreg reg, unsigned long value)
{
    int index = reg - dscreg_once_first;
    return write_once_gx(eonce[index].address, value, (int)eonce[index].length, 0, 0);
}

static osbdm_error
read_once_register (dscreg reg, unsigned long * value)
{
    int index = reg - dscreg_once_first;
    unsigned char ocmdr;
    unsigned char val[4];

    ocmdr = (unsigned char)(OCMDR_RE | eonce[index].address);

    Jtag_ScanOut(DATA_REGISTER, OCMDR_LENGTH, &ocmdr, RUN_TEST_IDLE);
    Jtag_ScanIn(DATA_REGISTER, (int)eonce[index].length, val, RUN_TEST_IDLE);

    *value = littleendian4_to_ulong(val);
    *value &= 0xFFFFFFFFL >> (unsigned char)(32 - eonce[index].length);

    return osbdm_error_ok;
}

static osbdm_error
write_core_register (dscreg reg, unsigned long value)
{
    osbdm_error error;
    int index = reg - dscreg_core_first;
    instruction_type write_r4;
    unsigned char bytes[6];

    write_r4.size = 3;
    bytes[0] = 0x1C;
    bytes[1] = 0xE4;
    bytes[2] = bytes[3] = bytes[4] = bytes[5] = 0;
    /* total evil magic ... stuff value into the opcode */
    ulong_to_littleendian4(value, &bytes[2]);
    if(value & 0xFF800000)
    {
        bytes[5] = 0xFF;
    }
    memcpy(write_r4.bytes, bytes, 6);
    error = execute_instruction(&write_r4);
    return_if(error);

    if (reg != dscreg_r4)
    {
        error = execute_sequence ((sequence_type *)&(dsc_write_reg_sequence[index]));
        return_if(error);
    }

    return error;
}
static osbdm_error
read_core_register (dscreg reg, unsigned long *value)
{
    osbdm_error error;
    int index = reg - dscreg_core_first;
    dscreg otx;

    error = execute_sequence ((sequence_type *)&(dsc_read_reg_sequence[index]));
    return_if(error);

    otx = dsc_read_reg_sequence[index].datasize > 16 ? dscreg_otx : dscreg_otx1;

    error = read_once_register (otx, value);
    return_if(error);

    *value &= 0xFFFFFFFFL >> (unsigned char)(32 - dsc_read_reg_sequence[index].datasize);

    return error;
}
static osbdm_error
write_register (dscreg reg, unsigned long value)
{
    if (reg >= dscreg_core_first && reg <= dscreg_core_last)
    {
        return write_core_register (reg, value);
    }
    else if (reg >= dscreg_once_first && reg <= dscreg_once_last)
    {
        return write_once_register (reg, value);
    }
    return osbdm_error_invalid_parameter;
}

static osbdm_error
read_register (dscreg reg, unsigned long *value)
{
    if (reg == dscreg_idcode)
    {
        unsigned char idcode[] = {0x2};
        unsigned char val[4];
        unsigned char enable_eonce[] = {JTAG_ENABLE_EONCE};

        Jtag_ScanOut(INSTRUCTION_REGISTER, JTAG_CORE_LENGTH_IR, idcode, RUN_TEST_IDLE);
        Jtag_ScanIn(DATA_REGISTER, 32, val, RUN_TEST_IDLE);
        Jtag_ScanOut(INSTRUCTION_REGISTER, JTAG_CORE_LENGTH_IR, enable_eonce, RUN_TEST_IDLE);
        *value = littleendian4_to_ulong(val);
        return osbdm_error_ok;
    }
    else if (reg >= dscreg_core_first && reg <= dscreg_core_last)
    {
        return read_core_register (reg, value);
    }
    else if (reg >= dscreg_once_first && reg <= dscreg_once_last)
    {
        return read_once_register (reg, value);
    }
    return osbdm_error_invalid_parameter;
}

static osbdm_error
debug_request (void)
{
    osbdm_error error;
    unsigned char debug_request[] = {JTAG_DEBUG_REQUEST};
    unsigned char enable_eonce[] = {JTAG_ENABLE_EONCE};

    Jtag_ScanOut (INSTRUCTION_REGISTER, JTAG_CORE_LENGTH_IR, debug_request, RUN_TEST_IDLE);

    error = wait_for_debug(WAIT_FOR_DEBUG_TIME);
    return_if(error);

    Jtag_ScanOut (INSTRUCTION_REGISTER, JTAG_CORE_LENGTH_IR, enable_eonce, RUN_TEST_IDLE);

    return error;
}


static osbdm_error
activate_flash(int nst)
{
    osbdm_error error;
    unsigned long hfm_base = Cache[nst].config[dsccfg_hfm_base];
    unsigned long clock_divider = Cache[nst].config[dsccfg_hfm_clock_divider];
    mem_address hfm;
    unsigned long hfmustat;
    char regData[2];

    /* Basic configuration parameters must be set */
    if (hfm_base == 0 || clock_divider == 0)
        return osbdm_error_invalid_parameter;

    Nst = nst;
    /* set up flash module clock divider */
    hfm.size = 2;
    hfm.space = dscmem_x;
    hfm.address = hfm_base + HFMCLKD_OFFSET;
    error = slow_read_mem(hfm, 2, regData);
    return_if(error);
    hfmustat = bigendian2_to_ulong(regData);
    if (!(hfmustat & HFMCLKD_DIVLD))
    {
        ulong_to_bigendian2(clock_divider, regData);
        error = slow_write_mem(hfm, 2, regData);
        return_if(error);
    }

    /* disable write/erase protection */
    hfm.address = hfm_base + HFMCR_OFFSET;
    regData[0] = regData[1] = 0;
    error = slow_write_mem(hfm, 2, regData);
    return_if(error);
    hfm.address = hfm_base + HFMPROT_OFFSET;
    error = slow_write_mem(hfm, 2, regData);
    return_if(error);
    hfm.address = hfm_base + HFMCR_OFFSET;
    ulong_to_bigendian2(1, regData);
    error = slow_write_mem(hfm, 2, regData);
    return_if(error);
    hfm.address = hfm_base + HFMPROT_OFFSET;
    regData[0] = regData[1] = 0;
    error = slow_write_mem(hfm, 2, regData);
    return_if(error);
    hfm.address = hfm_base + HFMCR_OFFSET;
    error = slow_write_mem(hfm, 2, regData);
    return_if(error);
    hfm.address = hfm_base + HFMPROTB_OFFSET;
    error = slow_write_mem(hfm, 2, regData);
    return_if(error);

    return osbdm_error_ok;
}

static osbdm_error
deactivate_flash(int nst)
{
    QUIET(nst);
    return osbdm_error_ok;
}

static unsigned long
bigendian4_to_ulong (unsigned char * uc)
{
    return ((((unsigned long)((uc)[0]))<<24)|
            (((unsigned long)((uc)[1]))<<16)|
            (((unsigned long)((uc)[2]))<<8) |
            (((unsigned long)((uc)[3]))));
}

static unsigned long
bigendian2_to_ulong (unsigned char * uc)
{
    return ((((unsigned long)((uc)[0]))<<8) |
            (((unsigned long)((uc)[1]))));
}

static void
ulong_to_bigendian4(unsigned long ul,unsigned char * uc)
{
    (uc)[0] = (unsigned char)((ul)>>24);
    (uc)[1] = (unsigned char)((ul)>>16);
    (uc)[2] = (unsigned char)((ul)>>8);
    (uc)[3] = (unsigned char)(ul);
}
static void
ulong_to_bigendian2(unsigned long ul,unsigned char * uc)
{
    (uc)[0] = (unsigned char)((ul)>>8);
    (uc)[1] = (unsigned char)(ul);
}

static unsigned long
littleendian4_to_ulong (unsigned char * uc)
{
    return ((((unsigned long)((uc)[3]))<<24)|
            (((unsigned long)((uc)[2]))<<16)|
            (((unsigned long)((uc)[1]))<<8) |
            (((unsigned long)((uc)[0]))));
}

static unsigned long
littleendian2_to_ulong (unsigned char * uc)
{
    return ((((unsigned long)((uc)[1]))<<8) |
            (((unsigned long)((uc)[0]))));
}

static void
ulong_to_littleendian4(unsigned long ul,unsigned char * uc)
{
    (uc)[3] = (unsigned char)((ul)>>24);
    (uc)[2] = (unsigned char)((ul)>>16);
    (uc)[1] = (unsigned char)((ul)>>8);
    (uc)[0] = (unsigned char)(ul);
}
static void
ulong_to_littleendian2(unsigned long ul,unsigned char * uc)
{
    (uc)[1] = (unsigned char)((ul)>>8);
    (uc)[0] = (unsigned char)(ul);
}

static osbdm_error
dsc_startup (int nst)
{
    unsigned char enable_once[] = {JTAG_ENABLE_EONCE};
    unsigned char tlm_select[] = {JTAG_CHIP_TLM_SELECT};
    unsigned char select_dsc[] = {TLM_SELECT_CORE_TAP};
    int i;
    unsigned long osr;
    char stat;
    HFMUnit *unit;

  output_buffer_enable_value = 0x80; // EMBEDDED TOWER

	PTED = 0x00;	// disable output enable (buffer)

	PTEDD = 0xED;	// set direction

	TMS_RESET();		// set TMS low
	TRST_RESET();
	TCLK_RESET();

	PTBDD |= 0x08;	// TMS, TCLK signals output
	PTBD &= 0xF7;

	PTED = 0x80;	// enable JTAG port, output enable (buffer

	wait_ms(1);		// wait for input signals to settle

	// now the JTAG pins are in their default states
	TRST_RESET();                 // assert TRST
	wait_ms(50);                 // 50ms
	TDSCLK_EN = 1;                   // de-assert TRST
	wait_ms(10);                 // 10ms

	// Force into Reset (normal operation) mode
	// create 10 pulses on the TCLK line
	for (i=0; i<10; i++) TCLK_transition(TMS_HIGH,0);		// Transition CLK line: low_high

	TCLK_transition(TMS_LOW,0);		// Run-Test/Idle

	jdly_loop(20);	// about 200us delay

	// Read Chip ID Code
	// 	MC56F8013 reads 0x01f2401d
	//	MC56F8006 reads 0x01c0601d
    Cache[nst].chiplevel_idcode = ScanIO(DATA_REGISTER, 32, 0);	// read chip id code

/*
NEED FOR dsc CHIPS - DSP5685x series
On the dsc series, there are two TAP controllers, but to get from
the default TAP (chip TAP) to the core TAP you need to traverse the
newly installed logic of the IEEE compliant Tap Linking Module (a.k.a.
TLM).
-TAP reset (either the TAP reset pin, or traverse the state machine to
Test Logic Reset State)
-shift in 0x05 into 8 bit Instruction Register
(This selects the 4 bit TLM Data Register)
-shift in 0010 into the 4 bit TLM Data Register
Now you are talking to the core TAP

note-
0001 selects chip TAP
0010 selects core TAP
0011 selects both TAPs
0000 reserved
11xx reserved

Important Note-
Any time you assert the external TAP reset pin ...OR... traverse the
Test Logic Reset State, you go back to square one, i.e. you're back to
talking to the default (chip) TAP.
*/

    // select the TLM Data Register
  	stat = (char) ScanIO(INSTRUCTION_REGISTER, 8, 0x05);	// select TLM

    // select the Core TAP
  	stat = (char) ScanIO(DATA_REGISTER, 4, 0x02);	// select Core tap

  	// debug request
  	// stat = (char) ScanIO(INSTRUCTION_REGISTER, 4, 0x07);	// Debug Request

  	// enable EOnCE and get status - do it twice per example/spec
  	stat = (char) ScanIO(INSTRUCTION_REGISTER, 4, 0x6);	  // Enable EOnCE and get status
  	stat = (char) ScanIO(INSTRUCTION_REGISTER, 4, 0x6);	  // Enable EOnCE and get status
  	// stat should be: 	1101 = Debug - Core halted and in Debug mode

  	write_register(dscreg_ocr, OCR_PWU);	// fully Power Up EOnCE to enable all features

  	read_once_register(dscreg_osr, &osr);	// get EOnCE status
    // osr should be 0x4030 indicating we're in debug status, entered externally (EDB)


    // clear all configuration registers
    for (i = 0; i < dsccfg_count; i++)
    {
        Cache[nst].config[i] = 0;
    }

    // set up default flash configuration values
    Cache[nst].config[dsccfg_hfm_units]    = 1;
    Cache[nst].config[dsccfg_hfm_set_unit] = 0;
    unit = &(Cache[nst].unitList[Cache[nst].config[dsccfg_hfm_set_unit]]);
    
/*
    Cache[nst].config[dsccfg_hfm_unit_address]     = unit->startAddr   = 0;
                                                     unit->endAddr = 0x3fff;           
    Cache[nst].config[dsccfg_hfm_unit_size]        = unit->endAddr + 1;
*/

    Cache[nst].config[dsccfg_hfm_unit_space]       = unit->progMem     = 2;
    Cache[nst].config[dsccfg_hfm_unit_bank]        = unit->bank        = 0;
    Cache[nst].config[dsccfg_hfm_unit_interleaved] = unit->interleaved = 0;

/*
    Cache[nst].config[dsccfg_hfm_unit_pagesize]    = unit->pageSize    = 0x100;
    Cache[nst].config[dsccfg_hfm_base]             = 0xf400;
    Cache[nst].config[dsccfg_hfm_clock_divider]    = 39;
 */
    Cache[nst].fast_mem = 1;
//  Cache[nst].config[dsccfg_hfm_pram] = Cache[nst].fast_mem_addr      = 0x8346;

    Cache[nst].running = 1;
    Cache[nst].pc_modified = 0;

    return osbdm_error_ok;
}

/******************************************************************************
*	t_halt(void)
*
*	asserts BKPT to target to HALT execution
*
*	Return:	0 on success, non-zero on fail
*
******************************************************************************/
byte t_halt()
{
	return dsc_stop_core(0);	// halt processor
}

/******************************************************************************
*	t_go(void)
*
*	resume target execution from current PC
*
*	Return:	0 on success, non-zero on fail
*
******************************************************************************/
byte t_go(void)
{
	return dsc_run_core(0);
}

/******************************************************************************
*	t_step(void)
*
*	resume target execution from current PC
*
*	Return:	0 on success, non-zero on fail
*
******************************************************************************/
byte t_step()
{
	return dsc_step_core(0, 1);	// single step
}

/*------------------------------------------------------------------------------
	t_read_mem
	------------
	Read data from the target memory

	type = type of memory
	addr = target address to read from
	width = size of the reads (8/16/32)
	count = total number of BYTES to read
	*data = pointer to data buffer to hold data that's read

	returns 0 on success, non-zero on fail
*/
int t_read_mem(unsigned char type, unsigned long addr, unsigned char width, int count, unsigned char *data)
{
	mem_address mem;

    mem.size = width / 8;

	switch(type){
		case MEM_RAM:
		case MEM_X:
            mem.space = dscmem_x;
			break;
		case MEM_X_FLASH:
            mem.space = dscmem_x_flash;
			break;

		case MEM_P:
            mem.space = dscmem_p;
			break;
		case MEM_P_FLASH:
            mem.space = dscmem_p_flash;
			break;
		default:
			return osbdm_error_invalid_parameter;
	}

    mem.address = addr;

	switch(width){
		case 8:
		case 16:
		case 32:
            mem.size = width / 8;
			break;
		default:
			return osbdm_error_invalid_parameter;
	}

	return dsc_read_mem (0, mem, count, data);
}

/*------------------------------------------------------------------------------
	t_write_mem
	-------------
	Write data to target memory

	type = type of memory
	addr = target address to write to
	width = size of the writes (8/16/32)
	count = total number of BYTES to be written
	*data = pointer to data buffer containing the data

	returns 0 on success
*/
int t_write_mem(unsigned char type, unsigned long addr, unsigned char width, int count, unsigned char *data)
{
	mem_address mem;

    mem.size = width / 8;

	switch(type){
		case MEM_RAM:
		case MEM_X:
            mem.space = dscmem_x;
			break;
		case MEM_X_FLASH:
            mem.space = dscmem_x_flash;
			break;

		case MEM_P:
            mem.space = dscmem_p;
			break;
		case MEM_P_FLASH:
            mem.space = dscmem_p_flash;
			break;
		default:
			return osbdm_error_invalid_parameter;
	}

    mem.address = addr;

	switch(width){
		case 8:
		case 16:
		case 32:
            mem.size = width / 8;
			break;
		default:
			return osbdm_error_invalid_parameter;
	}

	return dsc_write_mem (0, mem, count, data);
}

/*------------------------------------------------------------------------------
	t_fill_mem
	-------------
	Fill data to target memory

	type = type of memory
	addr = target address to fill to
	width = size of the fill units (8/16/32)
	count = total number of units to be filled
	*data = pointer to data buffer containing the data

	returns 0 on success
*/
int t_fill_mem(unsigned char type, unsigned long addr, unsigned char width, int count, unsigned char *data)
{
	mem_address mem;

    mem.size = width / 8;

	switch(type){
		case MEM_RAM:
		case MEM_X:
            mem.space = dscmem_x;
			break;
		case MEM_X_FLASH:
            mem.space = dscmem_x_flash;
			break;

		case MEM_P:
            mem.space = dscmem_p;
			break;
		case MEM_P_FLASH:
            mem.space = dscmem_p_flash;
			break;
		default:
			return osbdm_error_invalid_parameter;
	}

    mem.address = addr;

	switch(width){
		case 8:
		case 16:
		case 32:
            mem.size = width / 8;
			break;
		default:
			return osbdm_error_invalid_parameter;
	}

	return dsc_fill_mem(0, mem, count, data);
}

//-------------------------------------------------------------
// in this version addr is a register index number
int t_write_creg(unsigned long addr, unsigned char *data)
{
//	return write_register (hv2reg reg, unsigned long value);
	return dsc_write_reg(0, addr, littleendian4_to_ulong(data));
}

//-------------------------------------------------------------
int t_read_creg(unsigned long addr, unsigned char *data)
{
	unsigned long val;
	char error;

	error = dsc_read_reg(0, addr, &val);
	ulong_to_littleendian4(val, data);

	return error;
}

/******************************************************************************
*	t_reset(mode)
*
*	assert reset to target device.  tRSTO is inverted version of tRST to target
*
*	Input:	0 = reset to ONcE mode, 1 = reset to Normal Mode
*	Return:	0 on success, 1 on fail
*
******************************************************************************/
byte t_reset(ResetT  mode)
{
	mem_address mem;
	unsigned char reset[2];
	char error;
	
	unsigned long simCntl_SWRST=0x10;

	
	// DSC OSBDM-JM60 firmware only supports software reset
	// Set SIM_CNTL memory mapped register bit 4 to 1 (i.e., 0x0010).
 	// e.g., write 0x0010 to 0xF240 (56F8006) for software reset
	
	if (SimCntlRegAddr == SIM_CNTL_ADDR_UNDEFINED)
	  return (1);

	  //  mem.address = 0xF240;
    mem.address = SimCntlRegAddr;
    mem.size = 2;
    mem.space = dscmem_x;
    
    ulong_to_bigendian2(simCntl_SWRST,reset);
 
	  error = slow_write_mem(mem, 2, reset);
    return_if(error);
   
 	  wait_ms(100);				// wait for target to exit reset

    if (mode == eSoftReset_to_DebugMode ||
        mode == eHardReset_to_DebugMode ||
        mode == ePowerReset_to_DebugMode) {
        
  	  error = dsc_reset_occurred(0, 1);	    // re-initialize JTAG and ONcE
  
    }
    else
		  error = dsc_reset_occurred(0, 0);

	return error;
}



/******************************************************************************
*	t_init(void)
*
*	check target power status and enable Vout if target not self-powered, enable
* 	and initialized BDM signals to target, start internal timer, issue RSTO to
*	target,and resynchronize to ensure communications
*
*	Input:	none
*	Return:	0 on success, 1 on failure
*
******************************************************************************/
byte t_init(void)
{
/*
	if(bdm_vtrg_en()) return 1;	// enable Vout to target if not self-powered

	// Enable BDM_CF interface to target
	// Under CW see: Target Setting, Compiler, Options, Language, Preprocessor Definitions
#ifdef __EMBEDDED__
	OUT_EN = 1;		// init BDM signals for embedded CF BDM - OUT_EN is active high
#else
	OUT_EN = 0;		// init BDM signals for CF BDM cable - OUT_EN is active low
#endif
	PTEDD = 0xE4;						// TDSCLK_EN, DOUT, SCLK_OUT, OUT_EN all outputs
*/

//	if(t_reset(0)) return 1;		// reset target and break

    SimCntlRegAddr = SIM_CNTL_ADDR_UNDEFINED;
    
    if (dsc_startup(0) != osbdm_error_ok)
        return 1;
    
    activate_flash(0);
    
    return 0;
}

void t_serial_init()
{
if ((PTEDD & 0x80) == 0x00) {
  // We have not yet enabled the debug signals including the virtual serial signals

	PTED = 0x00;	// disable output enable (buffer)

	PTEDD = 0xED;	// set direction

	TMS_RESET();		// set TMS low
	TRST_RESET();
	TCLK_RESET();



	PTBDD |= 0x08;	// TMS, TCLK signals output
	PTBD &= 0xF7;

	PTED = 0x80;	// enable JTAG port
  }


}


//---------------------------------------------------------------------------
//	Return the bdm/target status information to host
//
//	pData[] will be loaded with bytes as follows:
//		0    = unused
//		1    = unused
//		2    = unused
//		3    = unused
//		4    = unused
//		3..9 = unused
//
//  Returns 0 for success, 1 for failure
//---------------------------------------------------------------------------
int	t_stat(unsigned char *data) { 

	data[0] = 0;
	data[1] = 0;
	data[2] = 0;
	data[3] = 
	data[4] = 
	data[5] =
	data[6] =
	data[7] =
	data[8] =
	data[9] = 0;

	return (0);
}

//---------------------------------------------------------------------------
//	Return the bdm/target status information to host
//	
//	pData[] will be loaded with bytes as follows:
//		0    = VERSION_HW
//		1    = VERSION_SW
//		2    = BUILD_VER
//		3    = TARGET_TYPE
//    4    = FIRMWARE_TYPE
//		5    = BUILD_VER_REV
//    6    = board_id
//    7    = osbdm_id
//    8..9 = unused
//---------------------------------------------------------------------------

int					// Returns 0 for success, 1 for failure
t_get_ver (
	PUINT8  pData)		// Ptr to return status buffer

{
    pData[0] = VERSION_HW;	// Hardware Version
    pData[1] = VERSION_SW;	// Firmware Version
    pData[2] = BUILD_VER;
  	pData[3] = TARGET_TYPE;
  	pData[4] = FIRMWARE_TYPE;
  	pData[5] = BUILD_VER_REV;
  	pData[6] = board_id; 
  	pData[7] = osbdm_id;
  	pData[8] = 0;
  	pData[9] =  0;

 	return (0);
}


//-------------------------------------------------------------
char t_unsecure(byte lockr, byte lrcnt, byte clkdiv)
{
	// unsecure the part using the mass erase and verify method
	mem_address mem;
    unsigned char erase[2];

    mem.address = 0;
    mem.size = 2;
    mem.space = dscmem_p_flash;
    erase[0] = erase[1] = 0xFF;

	return dsc_fill_mem(0, mem, 0x2000, erase);
}

//-------------------------------------------------------------
int t_flash_power (byte enable)
{
	return(0);
}

//-------------------------------------------------------------
unsigned long t_get_clock (void)
{
	return (0);
}

//---------------------------------------------------------------------------
//	Execute flash programming algorithm
//---------------------------------------------------------------------------
int t_flash_prog (PUINT8 pData)
{
	return (0);			// Not used for this target
}

//-------------------------------------------------------------
int t_write_dreg   (UINT32 addr, UINT8 uWidth, PUINT8 pData)
{
	return (0);			// Not used for this target
}

//-------------------------------------------------------------
int t_read_dreg    (UINT32 addr, UINT8 uWidth, PUINT8 pData)
{
	return (0);			// Not used for this target
}
//---------------------------------------------------------------------------
//	Set the BDM clock value of the target
//---------------------------------------------------------------------------
void t_set_clock (UINT32 clock){
	return;			    // Not used for this target
}


//---------------------------------------------------------------------------
//	Configure parameters 
//---------------------------------------------------------------------------
int t_config (byte configType, byte configParam, UINT32 paramVal)
{
    int nst;
  
  
    if (configParam == dsccfg_sim_cntl_address) {
        SimCntlRegAddr = paramVal;
        return (0);
    }
    
    nst = 0;
    return dsc_config(nst, configParam, paramVal);
}


