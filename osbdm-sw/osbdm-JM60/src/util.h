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


#ifndef _UTIL_H_
#define _UTIL_H_

unsigned long getbuf4(unsigned char *buf);
unsigned int getbuf2big(unsigned char *buf);
unsigned int getbuf2little(unsigned char *buf);

unsigned long bigendian4_to_ulong (unsigned char * uc);
unsigned long bigendian2_to_ulong (unsigned char * uc);
void ulong_to_bigendian4(unsigned long ul,unsigned char * uc);
void ulong_to_bigendian2(unsigned long ul,unsigned char * uc);
void ulong_to_littleendian4(unsigned long ul,unsigned char * uc);
void ulong_to_littleendian2(unsigned long ul,unsigned char * uc);

#endif // _UTIL_H_
