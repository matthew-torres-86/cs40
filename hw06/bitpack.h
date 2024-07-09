#include <inttypes.h>
#include <stdio.h>
#include <stdbool.h>
#include "except.h"
#include "assert.h"

/* 
 * What makes things hellish is that C does not define the effects of
 * a 64-bit shift on a 64-bit value, and the Intel hardware computes
 * shifts mod 64, so that a 64-bit shift has the same effect as a
 * 0-bit shift.  The obvious workaround is to define new shift functions
 * that can shift by 64 bits.
 */

#ifndef _BITPACK_H__
#define _BITPACK_H__

bool Bitpack_fitss( int64_t n, unsigned width);

bool Bitpack_fitsu(uint64_t n, unsigned width);

/****************************************************************/

int64_t Bitpack_gets(uint64_t word, unsigned width, unsigned lsb);

uint64_t Bitpack_getu(uint64_t word, unsigned width, unsigned lsb);

/****************************************************************/
uint64_t Bitpack_newu(uint64_t word, unsigned width, unsigned lsb,
                      uint64_t value);

uint64_t Bitpack_news(uint64_t word, unsigned width, unsigned lsb,
                      int64_t value);

#endif