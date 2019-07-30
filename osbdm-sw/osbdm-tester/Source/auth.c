/*
==============================================================================

    auth.c			Device Authentication Code 

    Version:		1.0
    Author:			Spencer Ruggles, Axiom Manufacturing (www.axman.com)
    Compiler:		CodeWarrior, V5.1


==============================================================================
*/

#include "auth.h"

unsigned char ident (unsigned char data1) {
	unsigned char result;
	
	result = data1 + DEV_KEY;			// add DEV_KEY to input interrogation value
	result = result ^ DEV_KEY;			// XOR result w/ DEV_KEY
	result ^= 0xFF;						// negate result
	return (result);						// return result
}