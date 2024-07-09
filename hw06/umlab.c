/*
 * umlab.c
 *
 * Functions to generate UM unit tests. Once complete, this module
 * should be augmented and then linked against umlabwrite.c to produce
 * a unit test writing program.
 *  
 * A unit test is a stream of UM instructions, represented as a Hanson
 * Seq_T of 32-bit words adhering to the UM's instruction format.  
 * 
 * Any additional functions and unit tests written for the lab go
 * here. 
 *  
 */


#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include <seq.h>
#include "bitpack.h"

const int rc_LSB = 0;
const int rb_LSB = 3;
const int ra_LSB = 6;
const int op_LSB = 28;

typedef uint32_t Um_instruction;
typedef enum Um_opcode {
        CMOV = 0, SLOAD, SSTORE, ADD, MUL, DIV,
        NAND, HALT, ACTIVATE, INACTIVATE, OUT, IN, LOADP, LV
} Um_opcode;


/* Functions that return the two instruction types */

Um_instruction three_register(Um_opcode op, int ra, int rb, int rc){
        Um_instruction new_um = 0;
        new_um = Bitpack_newu(new_um, 3, rc_LSB, rc);
        new_um = Bitpack_newu(new_um, 3, rb_LSB, rb);
        new_um = Bitpack_newu(new_um, 3, ra_LSB, ra);
        new_um = Bitpack_newu(new_um, 4, op_LSB, op);
        
        return new_um;
}

Um_instruction loadval(unsigned ra, unsigned val){
        Um_instruction new_um = 0;
        new_um = Bitpack_newu(new_um, 25, 0, val);
        new_um = Bitpack_newu(new_um, 3, 25, ra);
        new_um = Bitpack_newu(new_um, 4, 28, LV);

        return new_um;
}


/* Wrapper functions for each of the instructions */

static inline Um_instruction halt(void) 
{
        return three_register(HALT, 0, 0, 0);
}

typedef enum Um_register { r0 = 0, r1, r2, r3, r4, r5, r6, r7 } Um_register;

static inline Um_instruction add(Um_register a, Um_register b, Um_register c) 
{
        return three_register(ADD, a, b, c);
}

static inline Um_instruction map(Um_register b, Um_register c) 
{
        return three_register(ACTIVATE, 0, b, c);
}

static inline Um_instruction unmap(Um_register c) 
{
        return three_register(INACTIVATE, 0, 0, c);
}

static inline Um_instruction mul(Um_register a, Um_register b, Um_register c) 
{
        return three_register(MUL, a, b, c);
}

static inline Um_instruction div(Um_register a, Um_register b, Um_register c) 
{
        return three_register(DIV, a, b, c);
}
static inline Um_instruction sload(Um_register a, Um_register b, Um_register c) 
{
        return three_register(SLOAD, a, b, c);
}
static inline Um_instruction loadp(Um_register b, Um_register c) 
{
        return three_register(LOADP, 0, b, c);
}
static inline Um_instruction store(Um_register a, Um_register b, Um_register c) 
{
        return three_register(SSTORE, a, b, c);
}

static inline Um_instruction cond(Um_register a, Um_register b, Um_register c) 
{
        return three_register(CMOV, a, b, c);
}

static inline Um_instruction nand(Um_register a, Um_register b, Um_register c) 
{
        return three_register(NAND, a, b, c);
}

Um_instruction output(Um_register c){
        return three_register(OUT, 0, 0, c);
}

Um_instruction input(Um_register c){
        return three_register(IN, 0, 0, c);
}

/* Functions for working with streams */

static inline void append(Seq_T stream, Um_instruction inst)
{
        assert(sizeof(inst) <= sizeof(uintptr_t));
        Seq_addhi(stream, (void *)(uintptr_t)inst);
}

const uint32_t Um_word_width = 32;

void Um_write_sequence(FILE *output, Seq_T stream)
{
        assert(output != NULL && stream != NULL);
        int stream_length = Seq_length(stream);
        for (int i = 0; i < stream_length; i++) {
                Um_instruction inst = (uintptr_t)Seq_remlo(stream);
                for (int lsb = Um_word_width - 8; lsb >= 0; lsb -= 8) {
                        fputc(Bitpack_getu(inst, 8, lsb), output);
                }
        }
      
}


/* Unit tests for the UM */

void build_halt_test(Seq_T stream)
{
        append(stream, halt());
}

void build_verbose_halt_test(Seq_T stream)
{
        append(stream, halt());
        append(stream, loadval(r1, 'B'));
        append(stream, output(r1));
        append(stream, loadval(r1, 'a'));
        append(stream, output(r1));
        append(stream, loadval(r1, 'd'));
        append(stream, output(r1));
        append(stream, loadval(r1, '!'));
        append(stream, output(r1));
        append(stream, loadval(r1, '\n'));
        append(stream, output(r1));
}

void build_add_test(Seq_T stream){
        append(stream, loadval(r2, 8));
        append(stream, loadval(r3, 7));
        append(stream, add(r1, r2, r3));
        append(stream, output(r1));
        append(stream, halt());
}

void build_sload_map_test(Seq_T stream){
        append(stream, loadval(r1, 1));
        append(stream, loadval(r2, 1));
        append(stream, loadval(r3, 97));
        append(stream, map(r1, r2));
        append(stream, loadval(r5, 0));
        append(stream, store(r1, r5, r3));
        append(stream, sload(r4, r1, r5));
        append(stream, output(r4));
        append(stream, halt());
}

void build_loadp_test(Seq_T stream){
        append(stream, loadval(r0, 0)); // b = 1
        append(stream, loadval(r1, 1)); // c = 3
        append(stream, loadval(r2, 2)); // b = 1
        append(stream, loadval(r3, 3));
        append(stream, loadval(r4, 4)); // c = 3
        append(stream, map(r1, r4)); //map segment with 4 words
        
        append(stream, store(r1, 0, 3));
        printf("1"); //
        append(stream, store(r1, 1, 2)); 
        printf("2"); //
        append(stream, store(r1, 2, 1));
        printf("3"); 
        append(stream, store(r1, 3, 0));
        printf("4"); 
        append(stream, loadp(r1, 0));
        append(stream, halt());
}

void build_map_test(Seq_T stream){
        append(stream, loadval(r3, 8));
        append(stream, map(r2, r3));
        append(stream, output(r2));
        append(stream, halt());
}

void build_unmap_test(Seq_T stream){
        append(stream, loadval(r3, 8));
        append(stream, map(r2, r3));
        append(stream, unmap(r2));
        append(stream, output(r2));
        append(stream, halt());
}

void build_mult_test(Seq_T stream){
        append(stream, loadval(r2, 8));
        append(stream, loadval(r3, 7));
        append(stream, mul(r1, r2, r3));
        append(stream, output(r1));
        append(stream, halt());
}

void build_io_test(Seq_T stream){
        append(stream, input(r1));
        append(stream, output(r1));
        append(stream, halt());
}

void build_div_test(Seq_T stream){
        append(stream, loadval(r2, 8));
        append(stream, loadval(r3, 7));
        append(stream, div(r1, r2, r3));
        append(stream, output(r1));
        append(stream, halt());
}

void build_digit_test(Seq_T stream){
        append(stream, loadval(r1, 48));
        append(stream, loadval(r2, 6));
        append(stream, add(r3, r1, r2));
        append(stream, output(r3));
        append(stream, halt());
}

void build_cond_test(Seq_T stream){
        append(stream, loadval(r3, 0));
        append(stream, loadval(r2, 0));
        append(stream, cond(r1, r2, r3));
        append(stream, output(r1));
        append(stream, halt());
}

void build_nand_test(Seq_T stream){
        append(stream, loadval(r2, 1));
        append(stream, loadval(r3, 1));
        append(stream, nand(r1, r2, r3));
        append(stream, output(r1));
        append(stream, halt());
}
