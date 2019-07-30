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


// utility functions

#include "util.h"


/*
unsigned int ByteSwap16(unsigned int val){
	unsigned char h = (val >> 8);	// shift high to low and save
	val = (val & 0xFF);	// mask high
	val <<= 8;			// shift low to high
	return(val + h);	// return them merged
}
unsigned int ByteSwap32(unsigned long val){
	unsigned long a, b, c, d;

	a = val << 24;
	b = (val << 8) & 0x00FF0000;
	c = (val >> 8) & 0x0000FF00;
	d = val >> 24;

	return(a + b + c + d);
}
*/
/*------------------------------------------------------------------------------------
   return a 32-bit value taking 4 bytes from an 8-bit buffer -- LSB first
   so INTEL processors can send us data their way
*/
unsigned long getbuf4(unsigned char *buf){
	unsigned long val=0;
	int i;
	buf+=3;	// point to last byte
	for(i=0; i<4; i++){
		val <<=8;
		val += *(buf--);
	}
	
	return val;	
}
/*------------------------------------------------------------------------------------
	return a 16-bit value taking 2 bytes from an 8-bit buffer 
	data is Little-endian
*/
unsigned int getbuf2little(unsigned char *buf){
	unsigned int val;
	val = *(buf+1);	// get high byte
	val <<= 8;
	val += *buf;	// add low byte
	return val;	
}
/*------------------------------------------------------------------------------------
    return a 16-bit value taking 2 bytes from an 8-bit buffer 
	data is Big-endian
*/
unsigned int getbuf2big(unsigned char *buf){
	unsigned int val;
	val = *(buf);	// get high byte
	val <<= 8;
	val += *(buf+1);	// add low byte
	return val;	
}

// todo: use these instead, I think they're more efficient:

unsigned long bigendian4_to_ulong (unsigned char * uc)
{
    return ((((unsigned long)((uc)[0]))<<24)|
            (((unsigned long)((uc)[1]))<<16)|
            (((unsigned long)((uc)[2]))<<8) |
            (((unsigned long)((uc)[3]))));
}

unsigned long bigendian2_to_ulong (unsigned char * uc)
{
    return ((((unsigned long)((uc)[0]))<<8) |
            (((unsigned long)((uc)[1]))));
}
unsigned long littleendian2_to_ulong (unsigned char * uc)
{
    return ((((unsigned long)((uc)[1]))<<8) |
            (((unsigned long)((uc)[0]))));
}

void ulong_to_bigendian4(unsigned long ul,unsigned char * uc)
{
    (uc)[0] = (unsigned char)((ul)>>24);
    (uc)[1] = (unsigned char)((ul)>>16);
    (uc)[2] = (unsigned char)((ul)>>8);
    (uc)[3] = (unsigned char)(ul);
}
void ulong_to_littleendian4(unsigned long ul,unsigned char * uc)
{
    (uc)[3] = (unsigned char)((ul)>>24);
    (uc)[2] = (unsigned char)((ul)>>16);
    (uc)[1] = (unsigned char)((ul)>>8);
    (uc)[0] = (unsigned char)(ul);
}
void ulong_to_bigendian2(unsigned long ul,unsigned char * uc)
{
    (uc)[0] = (unsigned char)((ul)>>8);
    (uc)[1] = (unsigned char)(ul);
}
void ulong_to_littleendian2(unsigned long ul,unsigned char * uc)
{
    (uc)[1] = (unsigned char)((ul)>>8);
    (uc)[0] = (unsigned char)(ul);
}
