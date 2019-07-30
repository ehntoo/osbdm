/*************************************************************************************************
 * Copyright (c) 2007, Freescale Semiconductor
 *
 * File name   : typedef.h
 * Project name: JM60 Evaluation code
 *
 * Description : This software evaluates JM60 USB module 
 *               
 *
 * History     :
 * 04/01/2007  : Initial Development
 * 
 *************************************************************************************************/


#ifndef _TYPEDEF_H_
#define _TYPEDEF_H_


typedef unsigned char  UINT8,  *PUINT8;   /* unsigned 8 bit definition */
typedef unsigned short UINT16, *PUINT16;  /* unsigned 16 bit definition*/
typedef unsigned long  UINT32, *PUINT32;  /* unsigned 32 bit definition*/
typedef signed char    INT8,   *PINT8;    /* signed 8 bit definition   */
typedef short          INT16,  *PINT16;   /* signed 16 bit definition  */
typedef long int       INT32,  *PINT32;   /* signed 32 bit definition  */

typedef unsigned char   byte,  *Pbyte;    // 8-bit
typedef unsigned int    word,  *Pword;    // 16-bit
typedef unsigned long   dword, *Pdword;   // 32-bit

typedef union _BYTE
{
    byte _byte;
    struct
    {
        unsigned b0:1;
        unsigned b1:1;
        unsigned b2:1;
        unsigned b3:1;
        unsigned b4:1;
        unsigned b5:1;
        unsigned b6:1;
        unsigned b7:1;
    }Bit;
} BYTE, *PBYTE;

typedef union _WORD
{
    word _word;
    struct
    {
        byte byte1;
        byte byte0;
    }_byte;
    struct
    {
        BYTE HighB;
        BYTE LowB;
    }_Byte;
} WORD, *PWORD;

#define LSB(a)      ((a)._byte.byte0)
#define MSB(a)      ((a)._byte.byte1)

typedef union _DWORD
{
    dword _dword;
    struct
    {
        byte byte3;
        byte byte2;
        byte byte1;
        byte byte0;
    }_byte;
    struct
    {
        word word1;
        word word0;
    }_word;
    struct
    {
        BYTE Byte3;
        BYTE Byte2;
        BYTE Byte1;
        BYTE Byte0;
    }_Byte;
    struct
    {
        WORD Word1;
        WORD Word0;
    }_Word;
} DWORD, *PDWORD;

#define LOWER_LSB(a)    ((a)._byte.byte0)
#define LOWER_MSB(a)    ((a)._byte.byte1)
#define UPPER_LSB(a)    ((a)._byte.byte2)
#define UPPER_MSB(a)    ((a)._byte.byte3)

typedef void(* pFunc)(void);

typedef union _POINTER
{
    struct
    {
        byte bHigh;
        byte bLow;
    }_byte;
    
    word _word;                   // bLow & bHigh
    byte* bMem;                   // 2 bytes pointer pointing
    word* wMem;                   // 2 bytes poitner pointing
                                 
} PTR_TYPE;

#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE  1
#endif

#endif // _TYPEDEF_H_
