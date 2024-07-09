/*
 * Alexander Zsikla (azsikl01)
 * Partner: Ann Marie Burke (aburke04)
 * um.c
 * COMP40 HW6
 * Fall 2019
 * 
 * Optimized and adapted by Alexander Chanis and Matt Torres
 *
 * Implementation of the full UM, which includes functions to
 * allocate/deallocate memory associated with the UM_T struct,
 * function to populate segment zero with an instruction,
 * functions to execute instructions in segment zero,
 * and functions to perform all UM instructions
 *
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "um.h"
#include "uarray.h"
#include "seq.h"
#include "bitpack.h"
#include "memory.h"

#define WORD_SIZE 32
#define OP_WIDTH 4
#define R_WIDTH 3
#define RA_LSB 6
#define RB_LSB 3
#define RC_LSB 0
#define NUM_REGISTERS 8

/* Struct definition of a Memory_T which 
   contains two sequences: 
   - one holding pointers to UArray_T's representing segments
   - one holding pointers to uint32_t's representing free segments */
struct Memory_T {
        Seq_T segments;
        Seq_T free;
};

struct segment {
        uint32_t *array;
        uint32_t length;
};

const uint32_t value_size = 25;

const uint32_t a_mask = 448;
const uint32_t a_valmask = 234881024;
const uint32_t b_mask = 56;
const uint32_t c_mask = 7;
const uint32_t val_mask = 33554431;
const uint32_t op_mask = 4026531840;

const uint32_t a_shift = 6;
const uint32_t b_shift = 3;

#define W_SIZE 32
#define CHAR_SIZE 8

uint32_t construct_word(FILE *fp)
{
    assert(fp != NULL);

    uint32_t c = 0, word = 0;
    int bytes = W_SIZE / CHAR_SIZE;

    /* Reads in a char and creates word in big endian order */
    for (int c_loop = 0; c_loop < bytes; c_loop++) {
        c = getc(fp);
        assert(!feof(fp));

        unsigned lsb = W_SIZE - (CHAR_SIZE * c_loop) - CHAR_SIZE;
        word = Bitpack_newu(word, CHAR_SIZE, lsb, c);
    }

    return word;
}


/* Name: um_execute
 * Input: a UM_T struct
 * Output: N/A
 * Does: Executes all instructions in segment zero until
 *       there is no instruction left or until there is a halt instruction
 *       Calls instruction_call to execute all instructions except
 *       load program and load value
 * Error: Asserts if UM_T struct is NULL
 *        Asserts if segment zero is NULL at any point
 */
void um_execute(FILE *fp, uint32_t seg_zero_len)
{
        /* Create Memory */
        Memory_T memory = malloc(sizeof(*memory));
        assert(memory != NULL);

        /* Creating the segments */
        memory->segments = Seq_new(0);
        // assert(m_new->segments != NULL);

        /* Creating the sequence to keep track of free segments */
        memory->free = Seq_new(0);


        /*Populate Segment 0*/
        segment *seg0 = malloc(sizeof(*seg0));
        uint32_t array0[seg_zero_len];
        
        for (uint32_t index = 0; index < seg_zero_len; ++index) {
                uint32_t word = construct_word(fp);

                array0[index] = word;
        }
        seg0->array = array0;
        seg0->length = seg_zero_len;

        /* Mapping seg 0 */
        // uint32_t index = Seq_length(memory->segments);
        Seq_addhi(memory->segments, seg0);
        
        /* Registers and program counter */
        uint32_t registers[8] = {0, 0, 0, 0, 0, 0, 0, 0};

        uint32_t prog_counter = 0;
        uint32_t op, ra, rb, rc, word;

    /* Execute words in segment zero until there are none left */
    while (prog_counter < seg_zero_len) {

        word = seg0->array[prog_counter];
        op = (word & op_mask) >> 28;
        prog_counter++;

        /* Load value */
        if (op == 13) {

            ra = (word & a_valmask) >> 25;
            uint32_t value = (word & val_mask);
            registers[ra] = value;
        } 
        
        /* Every other instruction */
        else {
        ra = (word & a_mask) >> a_shift;
        rb = (word & b_mask) >> b_shift;
        rc = word & c_mask;
        
        assert(op < 14);
        assert(ra < NUM_REGISTERS && rb < NUM_REGISTERS && rc < 
        NUM_REGISTERS);

        /* Load Program */
        if (op == 12) {

                uint32_t rb_val = registers[rb];

                /* If rb value is 0, 0 is already loaded into segment 0 */
                if (rb_val == 0) {

                        prog_counter = registers[rc];
                } else {
                        /* Get the segment to copy */
                segment *to_copy = Seq_get(memory->segments, rb_val);

                /* Creating a copy with the same specifications */
                int seg_len = to_copy->length;
                uint32_t *copy = malloc(sizeof(uint32_t) * seg_len);
                // assert(copy != NULL);

                /* Deep copying */
                for (int i = 0; i < seg_len; i++){
                   copy[i] = to_copy->array[i];
                 }
                segment *copy_seg = malloc(sizeof(*copy_seg));
                copy_seg->array = copy;
                copy_seg->length = seg_len;

                /* Freeing segment 0 and inserting the copy */
                segment *seg_zero = Seq_get(memory->segments, 0);
                free(seg_zero);
                Seq_put(memory->segments, 0, copy_seg);

                prog_counter = registers[rc];
                }

                seg0 = Seq_get(memory->segments, 0);

                seg_zero_len = seg0->length;

        } 
        /* Conditional Move */
        if (op == CMOV) {
                if (registers[rc] != 0) {
                        registers[ra] = registers[rb];
                }  
        } 
        /* Segmented Load */
        else if (op == SLOAD) {

                uint32_t rb_val = registers[rb];
                uint32_t rc_val = registers[rc];

                segment *queried_segment = Seq_get(memory->segments, rb_val);
                registers[ra] = queried_segment->array[rc_val];

        } 
        /* Segmented Store */
        else if (op == SSTORE) {

                uint32_t ra_val = registers[ra];
                uint32_t rb_val = registers[rb];

                segment *queried_segment = Seq_get(memory->segments, ra_val);

                queried_segment->array[rb_val] = registers[rc];

        } 
        /* Addition */
        else if (op == ADD) {

                uint32_t rb_val = registers[rb];
                uint32_t rc_val = registers[rc];

                registers[ra] = rb_val + rc_val;

        } 
        /* Multiplication */
        else if (op == MUL) {

                uint32_t rb_val = registers[rb];
                uint32_t rc_val = registers[rc];

                registers[ra] = rb_val * rc_val;

        } 
        /* Division */
        else if (op == DIV) {

                uint32_t rb_val = registers[rb];
                uint32_t rc_val = registers[rc];

                registers[ra] = rb_val / rc_val;

        } 
        /* Nand */
        else if (op == NAND) {

                uint32_t rb_val = registers[rb];
                uint32_t rc_val = registers[rc];

                registers[ra] = ~(rb_val & rc_val);

        } 
        /* Halt */
        else if (op == HALT) {

                /* Freeing the UArray_T segments */
                int seg_len = Seq_length(memory->segments);
                for (int seg_num = 0; seg_num < seg_len; ++seg_num) {
                        segment *aux = Seq_remhi(memory->segments);
                        
                        /* If the segment is unmapped, there is nothing to free */
                        if (aux == NULL) {
                                continue;
                        } else {
                                
                                free(aux->array);
                                free(aux);
                        }
                }

                /* Freeing the uint32_t pointers */
                int free_len = Seq_length(memory->free);
                for (int seg_num = 0; seg_num < free_len; ++seg_num) {
                        uint32_t *integer = (uint32_t *)Seq_remhi(memory->free);
                        free(integer);
                }

                /* Freeing everything else */
                Seq_free(&(memory->segments));
                Seq_free(&(memory->free));
                free(memory);
                exit(EXIT_SUCCESS);
        }
        /* Map Segment */
        else if (op == MAP) {

                uint32_t rc_val = registers[rc];

                segment *seg = malloc(sizeof(*seg));
                seg->length = rc_val;
                uint32_t *array = malloc(sizeof(uint32_t) * rc_val);


                /* Setting values in new segment to 0 */
                for (uint32_t arr_index = 0; arr_index < rc_val; ++arr_index) {
                        array[arr_index] = 0;
                }

                seg->array = array;

                /* Mapping a segment */
                uint32_t index = Seq_length(memory->segments);
                if (Seq_length(memory->free) == 0) {
                        /* If there are no free segments, 
                        put UArray_T at end of sequence */
                        Seq_addhi(memory->segments, seg);
                } else {
                        /* If there is a free segment, 
                        get the index and put the UArray_T at that index */
                        uint32_t *free_seg_num = (uint32_t *)Seq_remlo
                        (memory->free);
                        index = *free_seg_num;
                        free(free_seg_num);
                        Seq_put(memory->segments, index, seg);
                }

                registers[rb] = index;
        } 
        /* Unmap Segment */
        else if (op == UNMAP) {

                uint32_t rc_val = registers[rc];

        uint32_t *unmap = Seq_get(memory->segments, rc_val);
        assert(unmap != NULL);

        free(unmap);

        uint32_t *free_seg = malloc(sizeof(uint32_t));
        assert(free_seg != NULL);

        *free_seg = rc_val;
        Seq_addhi(memory->free, free_seg);

        Seq_put(memory->segments, rc_val, NULL);
        } 
        /* Output */
        else if (op == OUT) {

                uint32_t rc_val = registers[rc];
                putchar(rc_val);

        } 
        /* Input */
        else if (op == IN) {

                int character = fgetc(stdin);

                if (character == EOF) {

                registers[rc] = ~0;
                }

                registers[rc] = character;
        }
        }
        }
}