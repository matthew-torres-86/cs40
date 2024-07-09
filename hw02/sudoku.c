#include "assert.h"
#include "pnmrdr.h"
#include "uarray2.h"


const int WIDTH = 9;
const int HEIGHT = 9;
/*
 * validate()
 * Description: takes mapped arrays and ensures that there are no duplicates in 
 * each set of 9 elements
 * Input: 1D UArray
 * Expectation: UArray of size 81, with no null elements.
*/
int validate(UArray_T arr){
    int *curr_p;
    for(int i = 0; i < WIDTH; i++){
        int subarr[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};

        for(int j = WIDTH*i; j < WIDTH*i + 8; j++){
            curr_p = UArray_at(arr, j); 

            /*If there are no duplicates, keep looking*/
            if(subarr[*curr_p-1] ==0){
                subarr[*curr_p-1] = *curr_p;
            }

            /*if there are duplicates, return false*/
            else{
                return 0;
            }
        }

    }
    /*all the way through with no duplicates, return true*/
    return 1;
}


/*
 * load_elements()
 * Description: the function called by apply() which adds each of the elements 
 * to an array
 * Input: ints i and j for col and row respectively, UArray2, and 2 void 
 * pointers
 * Expectation: i < 9, j < 9. UArray of size 81, with no null elements. 
 * void pointer p1 points to an int, and p2 points to a UArray.
*/
void load_elements(int i, int j, UArray2_T sudoku, void *p1, void *p2){
    int *curr = p1;
    UArray_T map = p2;
    assert(p1 == UArray2_at(sudoku, i, j));
    int *temp = UArray_at(map, WIDTH*i + j);
    *temp = *curr;
}

/*
 * load_rows()
 * Description: creates a 1D Uarray from the 2D uarray in row major
 * order. 
 * Input: 2D UArray
 * Expectation: UArray2 with width 9 and height 9.
*/
int load_rows(UArray2_T sudoku){
    UArray_T map = UArray_new(81, 8);
    UArray2_map_row_major(sudoku, load_elements, map);
    int result = validate(map);
    UArray_free(&map);
    return result;
}

/*
 * load_cols()
 * Description: creates a 1D Uarray from the 2D uarray in column major
 * order. 
 * Input: 2D UArray
 * Expectation: UArray2 with width 9 and height 9.
*/
int load_cols(UArray2_T sudoku){
    UArray_T map = UArray_new(81, 8);
    UArray2_map_col_major(sudoku, load_elements, map);
    int result = validate(map);
    UArray_free(&map);
    return result;
}

/*
 * load_submaps()
 * Description: creates a 1D Uarray from the 2D uarray in the order of each 
 * submap in column major order.
 * Input: 2D UArray
 * Expectation: UArray2 with width 9 and height 9.
*/
int load_submaps(UArray2_T sudoku){
    UArray_T map = UArray_new(81, 8);
    
    /*To map each of the submaps*/
    for(int i = 0; i < 3; i++){
        for(int j = 0; j < WIDTH; j++){
            for(int k = 0; k < 3; k++){
                int *curr = UArray2_at(sudoku, i * 3 + k, j);
                int *temp = UArray_at(map, i * 27 + j * 3 + k);
                *temp = *curr; 
            }
        }
    }
    int result = validate(map);
    UArray_free(&map);
    return result;
}

/*
 * load_rows()
 * Description: called from main, reads file and checks validity of sudoku
 * found in pgm file
 * Input: pointer to an open file.
 * output: exits with a code of 0 if sudoku is valid
 * exits with a code of 1 if sudoku is not valid.
 * Expectation: properly formatted pgm file with header
 * and raster of 9x9 elements.
*/
int read_file(FILE *fp)
{
    UArray2_T sudoku = UArray2_new(WIDTH, HEIGHT, sizeof(0));

    int c = 0;
    int *data;
    Pnmrdr_T reader = Pnmrdr_new(fp);
    unsigned width = Pnmrdr_data(reader).width;
    unsigned height = Pnmrdr_data(reader).height;

    assert((int)width == WIDTH);
    assert((int)height == WIDTH);

    for(int row = 0; row < (int)height; row ++){
        for(int col = 0; col < (int)width; col++){
            c = Pnmrdr_get(reader);
            data = UArray2_at(sudoku, col, row);
            *data = c;
        }
    }

    if(load_rows(sudoku) && load_cols(sudoku) && load_submaps(sudoku)){
        UArray2_free(&sudoku);
        free(sudoku);
        exit(0);
    }
    else{
        UArray2_free(&sudoku);
        free(sudoku);
        exit(1);
    }
    
}

/*
 * main()
 * Description: opens file, calls read(File)
 * Input: command line arguments
 * Expectation: [1, 2] command line arguments, second arg
 * if provided is the name of a properly formatted pgm file 
*/
int main(int argc, char **argv)
{
    if (argc == 1) {
        read_file(stdin);
    }    
    else {
        assert(argc == 2);   
        FILE *fp = fopen(argv[1], "r");
        assert(fp != NULL);
       
        read_file(fp);           
    }
}