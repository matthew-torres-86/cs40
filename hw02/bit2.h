#ifndef _BIT2_H__
#define _BIT2_H__

#include "bit.h"
#include "uarray.h"
#include <stdlib.h>
#include <stdio.h>

#define B2 Bit2_T
typedef struct B2 *B2;

/*
 * Bit2_new()
 * Description: Mallocs memory for a new Bit2.
 * input: ints width height and size of the elements
 * Expectations: all args are positive and nonzero.
 */ 
extern B2 Bit2_new(int width, int height); 

/*
 * Bit2_free()
 * Description: Frees memory allocated by the Bit2 
 * Input: pointer to bit2
 * Expectation: address is to a Bit2 that is currently stored in memory.
 */ 
extern void Bit2_free(B2 *set2); 

/*
 * Bit2_width()
 * Description: Returns the width of the Bit2
 * input: pointer to Bit2
 * Expectations: valid pointer to a Bit2 that exists in memory
 */ 
extern int Bit2_width(B2 set2); 

/*
 * Bit2_height()
 * Description: Returns the height of the UArray
 * Input: pointer to Bit2
 * Expectations: valid pointer to a Bit2 that exists in memory
 */ 
extern int Bit2_height(B2 set2); 

/*
 * Bit2_get()
 * Description: Returns a pointer to the element at a given position.
 * Position determined by [col, row].
 * input: Bit2, int col, int row
 * Expectations: Bit2 exists, col [0, width}, row [0, height}
 */ 
extern int Bit2_get(B2 set2, int col, int row);

/*
 * Bit2_put()
 * Description: Inserts an element into the Bit2 at a given position.
 * Position determined by [col, row].
 * input: Bit2, int col, int row, int bit
 * Expectations: Bit2 exists, col [0, width}, row [0, height}, bit [0, 1]
 */ 
extern int Bit2_put(B2 set2, int col, int row, int bit);

/*
 * Bit2_map_row_major()
 * Description: Applies a given function to all of the elements in a Bit2 in 
 * row major order (traverses across rows then columns).
 * input: Bit2, function pointer with args int i, int j, Bit2 a, int b and a
 * void pointer, and void pointer cl.
 * Expectations: Bit2 exists, function passed as pointer has the same args as
 * apply.
 */ 
extern void Bit2_map_row_major(Bit2_T set2, void apply(int i, int j, Bit2_T a, 
                                                int b, void *p1), void *cl);

/*
 * Bit2_map_col_major()
 * Description: Applies a given function to all of the elements in a Bit2 in 
 * column major order (traverses across columns then rows).
 * input: Bit2, function pointer with args int i, int j, Bit2 a, an int b and  
 * void pointer p1, and void pointer cl.
 * Expectations: Bit2 exists, function passed as pointer has the same args as
 * apply.
 */ 
extern void Bit2_map_col_major(Bit2_T set2, void apply(int i, int j, Bit2_T a, 
                                                int b, void *p1), void *cl);

#undef T2
#endif