#include "stdio.h"
#include "stdlib.h"
#include "uarray.h"
#include "seq.h"
#include "assert.h"
#include "stdint.h"
#include "bitpack.h"
#include "read_instructions.h"

const int OP_LSB = 28;
const int NUM_SEGMENTS = 200;
const int NUM_WORDS = 200;
const int NUM_REG = 8;

void command_center(FILE *fp){
    Seq_T memory = Seq_new(NUM_SEGMENTS);
    assert(memory);
    segment seg0 = Seq_new(NUM_WORDS);
    assert(seg0);
    Seq_T addresses = Seq_new(10);
    assert(addresses);

    Seq_addhi(memory, &seg0);

    while(!feof(fp)){
        uint32_t *new_word = malloc(sizeof(word));
        assert(new_word);
        *new_word = 0;
        char curr;
        for(int i = 3; i >= 0; i--){
            curr = fgetc(fp);
            if(curr == -1){
                break;
            }
            // printf("i: %d curr: %d\n", i, (uint8_t)curr);
            *new_word = Bitpack_newu(*new_word, 8, i * 8, (uint8_t)curr);
        }
        if(curr == -1){
            break;
        }
        // printf("%u\n",new_word);
        Seq_addhi(seg0, (void *)(uintptr_t)new_word); 
    }
    UArray_T registers = UArray_new(NUM_REG, sizeof(word));
    
    for(int i = 0; i < NUM_REG; i++){
        int * curr = UArray_at(registers, i);
        *curr = 0;
    }

    int num_instructions = Seq_length(seg0);
    uintptr_t prog_ctr;
        for(int i = 0; i < num_instructions; i++){
            printf("%d %d\n", i, num_instructions);
            prog_ctr = (uintptr_t)Seq_get(seg0, i);
            read_instruction(prog_ctr, &registers, &memory, &addresses, &i, 
            &num_instructions);
        }
}

int main(int argc, char *argv[]){
    if(argc !=2){
        fprintf(stderr, "Usage: Wrong number of arguments provided\nExpects ./um FILENAME.um");
    }
    FILE *fp = fopen(argv[1], "rb");
    assert(fp);
    command_center(fp);
}

