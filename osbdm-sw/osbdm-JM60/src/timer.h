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
*	File:			timer.h
*
*	Purpose: 	header file for timer.c
*
******************************************************************************/ 

#ifndef _TIMER_H_
#define _TIMER_H_

// Function Prototypes
void wait_ms(word ticks);			// adds delay in ms specified by ms_delay
void dly(word mod);		// add 3.33us delay
void dly_10us(word ticks);

#ifndef NULL
#define NULL (0)
#endif

#endif // _TIMER_H_
