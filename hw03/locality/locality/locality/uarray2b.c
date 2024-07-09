#include <stdlib.h>
#include <uarray2b.h>
#include <uarray2.h>
#include <uarray.h>
#include <assert.h>
#include <stdio.h>
#include <pnm.h>

struct UArray2b_T {

        int width;
        int height;
        int blockWidth;
        int blockHeight;
        int size;
        int blocksize;
        UArray2_T array;
};

/*
 * new blocked 2d array
 * blocksize = square root of # of cells in block.
 * blocksize < 1 is a checked runtime error
 */
extern struct UArray2b_T *UArray2b_new (int width, int height, int size, 
                                int blocksize)
{
        assert((width > 0) && (height > 0) && (size > 0) && (blocksize > 0));

        int newWidth = width / blocksize;
        int newHeight = height / blocksize;

        if ((width % blocksize) > 0) {
                newWidth += width % blocksize;
        }

        if ((height % blocksize) > 0) {
                newHeight += height % blocksize;
        }

        UArray2_T array = UArray2_new(newWidth, newHeight, sizeof(void*));

        for ( int r = 0; r < newHeight; r++ ) {
                for ( int c = 0; c < newWidth; c++ ) {

                        int length = blocksize * blocksize;
                        UArray_T newBlock = UArray_new(length, size);
                        UArray_T *curr = UArray2_at(array, c, r);
                        *curr = newBlock;
                }
        }
        
        UArray2b_T blocked = malloc(sizeof(*blocked));

        blocked->width = width;
        blocked->height = height;
        blocked->blockWidth = newWidth;
        blocked->blockHeight = newHeight;
        blocked->size = sizeof(void*);
        blocked->blocksize = blocksize;
        blocked->array = array;

        return blocked;
}

/* new blocked 2d array: blocksize as large as possible provided
 * block occupies at most 64KB (if possible)
 */
extern struct UArray2b_T *UArray2b_new_64K_block(int width, int height, 
                                                int size)
{
        assert((width > 0) && (height > 0) && (size > 0));

        int MAX_SIZE = 1024 * 64;

        if ( size >= MAX_SIZE ) {

                return UArray2b_new(width, height, size, 1);
        }

        int blocksize = 1;

        /* !!! this is slow, look for better algorithm */
        while ( (blocksize * blocksize * size) < MAX_SIZE ) {

                blocksize++;
        }

        if ( (blocksize * blocksize * size) > MAX_SIZE ) {
                blocksize--;
        }

        return UArray2b_new(width, height, size, blocksize);
}

extern void UArray2b_free(UArray2b_T *array2b)
{
        assert(array2b && *array2b);

        UArray2_T data = (*array2b)->array;

        UArray2_free(&data);

        // for (i = 0; i < (*array2)->height; i++) {
        //         UArray_T p = row(*array2, i);
        //         UArray_free(&p);
        // }
        // UArray_free(&(*array2)->rows);
        free(*array2b);
}

extern int UArray2b_width(UArray2b_T array2b)
{
        assert(array2b);

        return array2b->width;
}

extern int UArray2b_height(UArray2b_T array2b)
{
        assert(array2b);

        return array2b->height;
}

extern int UArray2b_size(UArray2b_T array2b)
{
        assert(array2b);

        return array2b->size;
}

extern int UArray2b_blocksize(UArray2b_T array2b)
{
        assert(array2b);

        return array2b->blocksize;
}

/* return a pointer to the cell in the given column and row.
 * index out of range is a checked run-time error
 */
extern void *UArray2b_at(UArray2b_T array2b, int column, int row)
{
        assert(array2b);
        assert((column > -1) && (row > -1));

        int blockCol = column / array2b->blocksize;
        int blockRow = row / array2b->blocksize;

        // printf("col: %d row: %d\n", blockCol, blockRow);

        UArray_T *block = UArray2_at(array2b->array, blockCol, blockRow);

        int blockIndex = array2b->blocksize * (column % array2b->blocksize) 
                        + (row % array2b->blocksize);

        return UArray_at(*block, blockIndex);

}

/* visits every cell in one block before moving to another block */
extern void  UArray2b_map(UArray2b_T array2b,
                          void apply(int col, int row, UArray2b_T array2b,
                                     void *elem, void *cl), void *cl)
{
        assert(array2b);

        int height = array2b->height;
        int width = array2b->width;
        // printf("height: %d width: %d \n", height, width);
        // int map_c = 0;
        // int map_r = 0;
        // for ( int r = 0; r < height; r++ ) {
        //         for ( int c = 0; c < width; c++ ) {
                        
        //                 // printf("row: %d col: %d \n", r, c);
        //                 UArray_T *curr = UArray2_at(array2b->array, c, r);


        //                 for ( int i = 0; i < UArray_length(*curr); i++ ) {
                                
        //                         if(map_c >= width){
        //                                 map_c = 0;
        //                                 map_r++;
        //                         }
        //                         if(map_r >= array2b->height){
        //                                 break;
        //                         }
        //                         // printf("%d %d\n", map_c, map_r);
        //                         apply(map_c, map_r, array2b, UArray_at(*curr, i), cl);
        //                         map_c++;
        //                 }
        //         }
        // }
        int blocksize = array2b->blocksize;
        for(int r = 0; r < height; r++){
                for(int c = 0; c < width; c++){
                        
                      UArray_T *curr = UArray2_at(array2b->array, c / blocksize, r / blocksize);

                        int i = blocksize * (c % blocksize) + (r % blocksize);
                      apply(c, r, array2b, UArray_at(*curr, i), cl);
                }
        }

}
