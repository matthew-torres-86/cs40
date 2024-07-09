/*
 * Alexander Zsikla (azsikl01)
 * Partner: Ann Marie Burke (aburke04)
 * um.h
 * COMP40 HW6
 * Fall 2019
 * 
 * Optimized and adapted by Alexander Chanis and Matt Torres
 *
 * Interface for the UM implementation
 *
 */

#include <stdint.h>

#ifndef UM_H_
#define UM_H_

/* Pointer to a struct that contains the data structure for this module */
typedef struct UM_T *UM_T;
typedef struct segment segment;

/* Contains the indices associated with specific opcodes */
typedef enum Um_opcode {
    CMOV = 0, SLOAD, SSTORE, ADD, MUL, DIV,
    NAND, HALT, MAP, UNMAP, OUT, IN, LOADP, LV
} Um_opcode;

/* Executes passed in program */
void um_execute(FILE *fp, uint32_t size);

#endif