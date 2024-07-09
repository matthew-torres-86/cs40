#include "bit2.h"
#include "assert.h"
#include <pnmrdr.h>
#include "stack.h"

/*Struct node used to simulate recursion in check_neighbors*/
struct node{
    int col;
    int row;
    int bit;
};

/*
 * check_neighbors()
 * Description: takes in coordinates of a bit. If the bit is 1, it changes it 
 * to 0 and checks its neighbors in a pseudo-recursive manner
 * Input: Bit2, col and row
 * Expectation: col < width of pgm, row < height of pgm
*/
void check_neighbors(Bit2_T pgm, int col, int row){
    Stack_T s; 
    if(Bit2_get(pgm, col, row)){
        s = Stack_new();
        struct node *np = malloc(12);
        np->col = col;
        np->row = row;
        np->bit = 1;
        Stack_push(s, np);
    }
    else{
        return;
    }
    while(!Stack_empty(s)){
        struct node *p = Stack_pop(s);
        Bit2_put(pgm, p->col, p->row, 0);
        

    if(p->col != 0){
        if(Bit2_get(pgm, p->col - 1, p->row)){
            struct node *np = malloc(12);
            np->col = p->col - 1;
            np->row = p->row;
            np->bit = 1;
            Stack_push(s, np);
        }
    }
    if(p->col != Bit2_width(pgm) - 1){
        if(Bit2_get(pgm, p->col + 1, p->row)){
            struct node *np = malloc(12);
            np->col = p->col + 1;
            np->row = p->row;
            np->bit = 1;
            Stack_push(s, np);
        }
    }  
    if(p->row != 0){
        if(Bit2_get(pgm, p->col, p->row -1)){
            struct node *np = malloc(12);
            np->col = p->col;
            np->row = p->row - 1;
            np->bit = 1;
            Stack_push(s, np);
        }

    }
    if(p->row != Bit2_height(pgm) - 1){
         if(Bit2_get(pgm, p->col, p->row + 1)){
            struct node *np = malloc(12);
            np->col = p->col;
            np->row = p->row + 1;
            np->bit = 1;
            Stack_push(s, np);
        }
    }  
        free(p);
    }
    Stack_free(&s);
    free(s);
}

/*
 * read_file()
 * Description: reads file, finds outer edges and calls check_neighbors()
 * for each. frees memory
 * Input: pointer to an open file
 * Expectation: file is a properly formatted pgm file.
*/
int read_file(FILE *fp){

    Pnmrdr_T reader = Pnmrdr_new(fp);
    unsigned width = Pnmrdr_data(reader).width;
    unsigned height = Pnmrdr_data(reader).height;

    Bit2_T pgm = Bit2_new(width, height);
    int c;
        for(int row = 0; row < (int)height; row ++){
            for(int col = 0; col < (int)width; col++){
                c = Pnmrdr_get(reader);
                Bit2_put(pgm, col, row, c);
            }
        }


    /*Find topmost and bottommost columns and check each of them for blackness*/
    for(int i = 0; i < (int)width; i++){
        check_neighbors(pgm, i, 0);

        check_neighbors(pgm, i, height - 1);
    }

    /*Find leftmost and rightmost columns and check each of them for blackness*/
    for(int i = 0; i < (int)height; i++){
        check_neighbors(pgm, 0, i);
        check_neighbors(pgm, width - 1, i);
    }

    /*print results*/
    printf("P1 %d %d\n", width, height);
    for(int row = 0; row < (int)height; row++){
        for(int col = 0; col < (int)width; col++){
            printf("%d ", Bit2_get(pgm, col, row));
        }
        printf("\n");
    }

    /*free memory*/
    Pnmrdr_free(&reader);
    Bit2_free(&pgm);
    free(pgm);
    return 0;

}

/*
 * main())
 * Description: opens file, calls readfile 
 * Input: command line args
 * Expectation: 1-2 command line args. opened file is a properly formatted
 * pgm file
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