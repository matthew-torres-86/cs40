/*
 * Alexander Zsikla (azsikl01)
 * Partner: Ann Marie Burke (aburke04)
 * um_driver.c
 * COMP40 HW6
 * Fall 2019
 *
 * Optimized and Adapted by Alexander Chanis and Matt Torres
 * 
 * Driver file for UM Implementation.
 * The driver opens the provided .um file,
 * creates a UM_T struct,
 * reads in all instructions from given file,
 * and populates segment zero with all instructions.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include <sys/stat.h>

#include "um.h"
#include "bitpack.h"

#define W_SIZE 32
#define CHAR_SIZE 8
#define CHAR_PER_WORD 4

int main(int argc, char *argv[]) 
{
    if (argc != 2) {
        fprintf(stderr, "Usage: ./um <Um file>\n");
        return EXIT_FAILURE;
    }

    FILE *fp = fopen(argv[1], "r");
    assert(fp != NULL);

    struct stat file_info;
    stat(argv[1], &file_info);
    uint32_t size = file_info.st_size / CHAR_PER_WORD;
    
    um_execute(fp, size);
    fclose(fp);

    return EXIT_SUCCESS;
}
