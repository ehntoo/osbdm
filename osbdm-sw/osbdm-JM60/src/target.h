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
*	File:		target.h
*
*	Purpose: 	Automate the inclusion of target specific header file
*
*
* 03/08/2011  : Modified by P&E Microcomputer Systems
*               http://www.pemicro.com/osbdm
*
*
*
\*****************************************************************************/ 

#ifndef _TARGET_H_
#define _TARGET_H_


#if   defined __CFV2__
#include "bdm_cf.h"

#elif defined __DSC__  
#include "jtag_dsc.h"
#include "jtag_jm60.h"

#elif defined __KINETIS__  
#include "jtag_kinetis.h"
#include "jtag_jm60.h"

#elif defined __EPPC__  
#include "jtag_eppc.h"
#include "jtag_jm60.h"

#elif defined __9S08__
#include "bdm_9S08.h"

#elif defined __9RS08__
#include "bdm_9RS08.h"

#elif defined __9S12__
#include "bdm_9S12.h"

#elif defined __9S12Z__
#include "bdm_9s12z.h"

#elif defined __CFV1__
#include "bdm_CFV1.h"

#endif


#endif // _TARGET_H_
