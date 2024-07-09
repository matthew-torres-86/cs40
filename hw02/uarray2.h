
#ifndef _UARRAY2_H__
#define _UARRAY2_H__

#include "uarray.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#define U UArray2_T
typedef struct U *U;


/*
 * UArray2_new()
 * Description: Mallocs memory for a new UArray2.
 * input: ints width height and size of the elements
 * Expectations: all args are positive and nonzero.
 */ 
extern U UArray2_new(int width, int height, int size); 
    

/*
 * UArray2_free()
 * Description: Frees memory allocated by the UArray2 
 * Input: pointer to UArray2
 * Expectation: address is to a UArray2 that is currently stored in memory.
 */ 
extern void UArray2_free(UArray2_T *u2);


/*
 * UArray2_width()
 * Description: Returns the width of the UArray
 * Input: pointer to UArray2
 * Expectations: valid pointer to a UArray2 that exists in memory
 */ 
extern int UArray2_width(UArray2_T u2); 


/*
 * UArray2_height()
 * Description: Returns the height of the UArray
 * Input: pointer to UArray2
 * Expectations: valid pointer to a UArray2 that exists in memory
 */ 
extern int UArray2_height(UArray2_T u2); 


/*
 * UArray2_size()
 * Returns the size of the elements in a given UArray2
 * Expected args: valid UArray2
 */ 
extern int UArray2_size (UArray2_T u2); 


/*
 * UArray2_at()
 * Returns a pointer to the element at a given position.
 * Position determined by [col, row].
 * input: UArray2, int col, int row
 * Expectations: UArray2 exists, col [0, width}, row [0, height}
 */ 
extern void *UArray2_at(UArray2_T u2, int col, int row); 


/*
 * UArray2_map_row_major()
 * Applies a given function to all of the elements in a UArray2 in row major 
 * order
 * Expected args: valid UArray2, function pointer, and a void pointer used
 * as an arg in the apply function
 */ 
extern void UArray2_map_row_major(UArray2_T u2, void apply(int i, int j, 
                            UArray2_T a, void *p1, void *p2), void *cl);


/*
 * UArray2_map_col_major()
 * Applies a given function to all of the elements in a UArray2 in row major 
 * order
 * Expected args: valid UArray2, function pointer, and a void pointer used as 
 * an arg in the apply function
 */ 
extern void UArray2_map_col_major(UArray2_T u2, void apply(int i, int j, 
                        UArray2_T a, void *p1, void *p2), void *cl);


#undef T2
#endif
