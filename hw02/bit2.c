#include "bit2.h"
#include "assert.h"

/*Struct for Bit2.c, hidden from client*/
struct B2{
    int width;
    int height;
    UArray_T buf;
};

/*
 * Bit2_new()
 * Description: Mallocs memory for a new Bit2.
 * input: ints width height and size of the elements
 * Expectations: all args are positive and nonzero.
 */ 
extern B2 Bit2_new(int width, int height){
    B2 b2 = malloc(10000);
    b2->width = width;
    b2->height = height;
    b2->buf = UArray_new(width, 8);
    for(int i = 0; i < width; i++){
        Bit_T col = Bit_new(height);
        Bit_T *temp = UArray_at(b2->buf, i);
        *temp = col;
    }
    return b2;
}

/*
 * Bit2_free()
 * Description: Frees memory allocated by the Bit2 
 * Input: pointer to bit2
 * Expectation: address is to a Bit2 that is currently stored in memory.
 */ 
extern void Bit2_free(B2 *set2){
    B2 temp_arr = *set2;
    for(int i = 0; i < temp_arr->width; i++){
        Bit_T *temp = UArray_at(temp_arr->buf, i);
        Bit_free(temp);
    }
    UArray_free(&temp_arr->buf);
} 

/*
 * Bit2_width()
 * Description: Returns the width of the Bit2
 * input: pointer to Bit2
 * Expectations: valid pointer to a Bit2 that exists in memory
 */ 
extern int Bit2_width(B2 set2){
    return set2->width;
}

/*
 * Bit2_height()
 * Description: Returns the height of the UArray
 * Input: pointer to Bit2
 * Expectations: valid pointer to a Bit2 that exists in memory
 */ 
extern int Bit2_height(B2 set2){
    return set2->height;
}

/*
 * Bit2_get()
 * Description: Returns a pointer to the element at a given position.
 * Position determined by [col, row].
 * input: Bit2, int col, int row
 * Expectations: Bit2 exists, col [0, width}, row [0, height}
 */ 
extern int Bit2_get(B2 set2, int col, int row){
    assert(col < set2->width);
    assert(row < set2->height);
    Bit_T *temp = UArray_at(set2->buf, col);
    return Bit_get(*temp, row);
}

/*
 * Bit2_put()
 * Description: Inserts an element into the Bit2 at a given position.
 * Position determined by [col, row].
 * input: Bit2, int col, int row, int bit
 * Expectations: Bit2 exists, col [0, width}, row [0, height}, bit [0, 1]
 */ 
extern int Bit2_put(B2 set2, int col, int row, int bit){
    assert(col < set2->width);
    assert(row < set2->height);
    Bit_T *temp = UArray_at(set2->buf, col);
    return Bit_put(*temp, row, bit);
}

/*
 * Bit2_map_row_major()
 * Description: Applies a given function to all of the elements in a Bit2 in 
 * row major order (traverses across rows then columns).
 * input: Bit2, function pointer with args int i, int j, Bit2 a, int b and a
 * void pointer, and void pointer cl.
 * Expectations: Bit2 exists, function passed as pointer has the same args as
 * apply.
 */ 
extern void Bit2_map_row_major(B2 set2, void apply(int i, int j, Bit2_T a, 
                                                   int b, void *p1), void *cl){

    for(int row = 0; row < set2->height; row++){
        for(int col = 0; col < set2->width; col++){
            int b = Bit2_get(set2, col, row);
            apply(col, row, set2, b, cl);
        }
    }
}

/*
 * Bit2_map_col_major()
 * Description: Applies a given function to all of the elements in a Bit2 in 
 * column major order (traverses across columns then rows).
 * input: Bit2, function pointer with args int i, int j, Bit2 a, an int b and  
 * void pointer p1, and void pointer cl.
 * Expectations: Bit2 exists, function passed as pointer has the same args as
 * apply.
 */ 
extern void Bit2_map_col_major(B2 set2, void apply(int i, int j, Bit2_T a, 
                                                   int b, void *p1), void *cl){
    for(int col = 0; col < set2->width; col++){
        for(int row = 0; row < set2->height; row++){
            int b = Bit2_get(set2, col, row);
            apply(col, row, set2, b, cl);
        }
    }
}