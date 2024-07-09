#include "uarray2.h"
#include "assert.h"
#include <stdbool.h>

/*Struct for UArray2_T. Hidden from client*/
struct U{
        int width;
        int height;
        int size;
        /* UArray_T buf holds col UArrays */
        UArray_T buf;
};

/*
 * UArray2_new()
 * Description: Mallocs memory for a new UArray2.
 * input: ints width height and size of the elements
 * Expectations: all args are positive and nonzero.
 */ 
extern U UArray2_new(int width, int height, int size){
    assert(width > 0);
    assert(height > 0);
    assert(size > 0);
    U u2 = malloc(10000);
    assert(u2);
    u2->width = width;
    u2->height = height;
    u2->size = size;
    u2->buf = UArray_new(width, 8);
    for(int i = 0; i < width; i++){
        UArray_T col = UArray_new(height, size);
        UArray_T *temp = UArray_at(u2->buf, i);
        *temp = col;
    }
    return u2; 
}

/*
 * UArray2_free()
 * Description: Frees memory allocated by each col UArray in the UArray2 
 * Input: address of a UArray2 stored in memory
 * Expectation: address is to a UArray2 that is currently stored in memory.
 */ 
extern void UArray2_free(UArray2_T *u2){
    U temp_arr = *u2;
    assert(u2);
    for(int i = 0; i < temp_arr->width; i++){
        UArray_T *temp = UArray_at(temp_arr->buf, i);
        UArray_free(temp);
    }
    UArray_free(&temp_arr->buf);
}

/*
 * UArray2_width()
 * Description: Returns the width of the UArray
 * input: 
 * Expectations: valid pointer to a UArray2 that exists in memory
 */ 
extern int UArray2_width(UArray2_T u2){
    return u2->width;
}

/*
 * UArray2_height()
 * Returns the height of the UArray.
 * Expected args: valid pointer to a UArray2 that exists in memory
 */ 
extern int UArray2_height(UArray2_T u2){
    return u2->height;
}

/*
 * UArray2_size()
 * Returns the size of the elements in a given UArray2
 * Expected args: valid pointer to a UArray2 that exists in memory
 */ 
extern int UArray2_size (UArray2_T u2){
    return u2->size;
}

/*
 * UArray2_at()
 * Returns a pointer to the element at a given position.
 * Position determined by [col, row].
 * input: UArray2, int col, int row
 * Expectations: UArray2 exists, col [0, width}, row [0, height}
 */ 
extern void *UArray2_at(UArray2_T u2, int col, int row){
    assert(col < u2->width);
    assert(row < u2->height);
    UArray_T *temp = UArray_at(u2->buf, col);
    return UArray_at(*temp, row);
}

/*
 * UArray2_map_row_major()
 * Description: Applies a given function to all of the elements in a UArray2 in 
 * row major order (traverses across rows then columns).
 * input: Uarray2, function pointer with args int i, int j, UArray2 a, and 2 
 * void pointers, and void pointer cl.
 * Expectations: Uarray2 exists, function passed as pointer has the same args as
 * apply.
 */ 
extern void UArray2_map_row_major(U u2, void apply(int i, int j, UArray2_T a, 
                                                void *p1, void *p2), void *cl){
    for(int row = 0; row < u2->height; row++){
        for(int col = 0; col < u2->width; col++){
            void *p1 = UArray2_at(u2, col, row);
            apply(col, row, u2, p1, cl);
        }
    }
    
}

/*
 * UArray2_map_col_major()
 * Description: Applies a given function to all of the elements in a UArray2 in 
 * column major order (traverses across columns then rows).
 * input: Uarray2, function pointer with args int i, int j, UArray2 a, and 2 
 * void pointers, and void pointer cl.
 * Expectations: Uarray2 exists, function passed as pointer has the same args as
 * apply.
 */ 
extern void UArray2_map_col_major(U u2, void apply(int i, int j, UArray2_T a, 
                                                void *p1, void *p2), void *cl){
    // bool *clint = cl;
    // printf("Mapping Column");
    
    for(int col = 0; col < u2->width; col++){
        for(int row = 0; row < u2->height; row++){
            void *p1 = UArray2_at(u2, col, row);
            apply(col, row, u2, p1, cl);
        }
    }

}
