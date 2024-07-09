#include "read_instructions.h"

const int OP_LSB1 = 28;
uint64_t MAX = 4294967296;

//TODO: LOAD_PROGRAM SEG_LOAD SEG_STORE

/* conditional_move()
 * description:
 * input:
 * output:
*/
void conditional_move(int ra, int rb, int rc, UArray_T *registers){
    uint32_t *c = (uint32_t *)(uintptr_t)UArray_at(*registers, rc);
    uint32_t *b = (uint32_t *)(uintptr_t)UArray_at(*registers, rb);
    uint32_t *a = (uint32_t *)(uintptr_t)UArray_at(*registers, ra);
    if(*c != 0){
        *a = *b;
    }
}

/* conditional_move()
 * description:
 * input:
 * output:
*/
void seg_load(int ra, int rb, int rc, UArray_T *registers, Seq_T *memory){
    uint32_t *c = (uint32_t *)(uintptr_t)UArray_at(*registers, rc);
    uint32_t *b = (uint32_t *)(uintptr_t)UArray_at(*registers, rb);
    uint32_t *a = (uint32_t *)(uintptr_t)UArray_at(*registers, ra);
    segment curr = Seq_get(*memory, *b);

    word *index = Seq_get(curr, *c);
    *a = *index;
}

/* conditional_move()
 * description:
 * input:
 * output:
*/
void seg_store(int ra, int rb, int rc, UArray_T *registers, Seq_T *memory){
    uint32_t *c = (uint32_t *)(uintptr_t)UArray_at(*registers, rc);
    uint32_t *b = (uint32_t *)(uintptr_t)UArray_at(*registers, rb);
    uint32_t *a = (uint32_t *)(uintptr_t)UArray_at(*registers, ra);

    segment curr = (segment)Seq_get(*memory, *a);
    assert(curr != NULL);

    word *index = Seq_get(curr, *b);
    *index = *c;
}

/* conditional_move()
 * description:
 * input:
 * output:
*/
void addition(int ra, int rb, int rc, UArray_T *registers){
    uint32_t *c = (uint32_t *)(uintptr_t)UArray_at(*registers, rc);
    uint32_t *b = (uint32_t *)(uintptr_t)UArray_at(*registers, rb);
    printf("%u %u ", *b, *c);
    uint32_t *a = (uint32_t *)(uintptr_t)UArray_at(*registers, ra);
    *a = (*b + *c) % MAX;
}

/* conditional_move()
 * description:
 * input:
 * output:
*/
void multiplication(int ra, int rb, int rc, UArray_T *registers){
    uint32_t *c = (uint32_t *)(uintptr_t)UArray_at(*registers, rc);
    uint32_t *b = (uint32_t *)(uintptr_t)UArray_at(*registers, rb);
    uint32_t *a = (uint32_t *)(uintptr_t)UArray_at(*registers, ra);
    *a = (*b * *c) % MAX;
}

/* conditional_move()
 * description:
 * input:
 * output:
*/
void division(int ra, int rb, int rc, UArray_T *registers){

    uint32_t *c = (uint32_t *)(uintptr_t)UArray_at(*registers, rc);
    uint32_t *b = (uint32_t *)(uintptr_t)UArray_at(*registers, rb);
    uint32_t *a = (uint32_t *)(uintptr_t)UArray_at(*registers, ra);
    *a = *b / *c;
}

/* conditional_move()
 * description:
 * input:
 * output:
*/
void nand(int ra, int rb, int rc, UArray_T *registers){

    uint32_t *c = (uint32_t *)(uintptr_t)UArray_at(*registers, rc);
    uint32_t *b = (uint32_t *)(uintptr_t)UArray_at(*registers, rb);
    uint32_t *a = (uint32_t *)(uintptr_t)UArray_at(*registers, ra);
    *a = (uint32_t)(~(*b & *c));
}

void halt(UArray_T *registers, Seq_T *memory, Seq_T *addresses){

    int m_length = Seq_length(*memory);
    for(int i=0; i < m_length; i++){
        segment curr = Seq_get(*memory, i);
        Seq_free(&curr);
    }
    Seq_free(memory);
    UArray_free(registers);
    Seq_free(addresses);

}

/* conditional_move()
 * description:
 * input:
 * output:
*/
void map_seg(int rb, int rc, UArray_T *registers, Seq_T *memory, 
                            Seq_T *addresses){
    uint32_t *c = (uint32_t *)(uintptr_t)UArray_at(*registers, rc);
    uint32_t *b = (uint32_t *)(uintptr_t)UArray_at(*registers, rb);
    segment new_seg = Seq_new(*c);
    
    for(int i = 0; i < (int)*c; i++){
        uint32_t *zero = malloc(sizeof(word));
        *zero = 0;
        Seq_addhi(new_seg, zero);
    }


    if(Seq_length(*addresses) == 0){

        uint32_t id = Seq_length(*memory);
        Seq_addhi(*memory, new_seg);
        *b = id;
        
    }
    else{
        uint32_t *addr = Seq_remlo(*addresses);
        segment *curr = Seq_get(*memory, (int)*addr);
        *curr = new_seg;
        *b = *addr;
    }

}

/* conditional_move()
 * description:
 * input:
 * output:
*/
void unmap_seg(int rc, UArray_T *registers, Seq_T *memory,
                            Seq_T *addresses){
    
    uint32_t *c = (uint32_t *)(uintptr_t)UArray_at(*registers, rc);

    Seq_addhi(*addresses, c);
    segment *curr = (segment *)Seq_get(*memory, *c);
    *curr = NULL;
}

/* conditional_move()
 * description:
 * input:
 * output:
*/
void output(int rc, UArray_T *registers){
    char *c = (char *)UArray_at(*registers, rc);
    putchar(*c);
}

/* conditional_move()
 * description:
 * input:
 * output:
*/
void input(int rc, UArray_T *registers){
    char x = getc(stdin);
    if(x == EOF){
        uint32_t *t = (uint32_t *)UArray_at(*registers, rc);
        *t = ~0;
        return;
    }
    char *c = (char *)UArray_at(*registers, rc);
    *c = x;
}

/* conditional_move()
 * description:
 * input:
 * output:
*/
uintptr_t load_program(int rb, int rc, UArray_T *registers, Seq_T *memory, int *i, int *num_instructions){
    printf("load prog\n");
    uint32_t *c = (uint32_t *)(uintptr_t)UArray_at(*registers, rc);
    uint32_t *b = (uint32_t *)(uintptr_t)UArray_at(*registers, rb);
    segment seg0 = Seq_get(*memory, 0);
    uintptr_t ctr = (uintptr_t)Seq_get(seg0, *c);
    if(*b == 0){
        return ctr;
    }
    segment curr = Seq_get(*memory, *b);
    int curr_length = Seq_length(curr);
    
    segment copy = Seq_new(curr_length);
    //copy sequence into new sequence
    for(int i = 0; i < curr_length; i++){
        word *new_word = malloc(sizeof(word));
        assert(new_word);
        *new_word = *(uint32_t *)Seq_get(curr, i);
        Seq_addhi(copy, new_word);
    }
    
    seg0 = copy;
    ctr = (uintptr_t)Seq_get(seg0, *c);
    *i = *c - 1;
    *num_instructions = Seq_length(seg0);
    printf("NUM INS: %d\n", *num_instructions);
    return ctr;
}

/* conditional_move()
 * description:
 * input:
 * output:
*/
void load_value(int ra, uint32_t val, UArray_T *registers){
    // printf("load val %u\n", val);
    uint32_t *a = (uint32_t *)(uintptr_t)UArray_at(*registers, ra);
    *a = val;
}

/* conditional_move()
 * description:
 * input:
 * output:
*/
void read_instruction(uintptr_t ctr, UArray_T *registers, Seq_T *memory, 
    Seq_T *addresses, int *i, int *num_instructions){
    word curr = *(uint32_t *)ctr;
    int curr_op = Bitpack_getu(curr, 4, OP_LSB1);
    int ra, rb, rc;
    uint32_t val;
    if(curr_op == 13){
        val = Bitpack_getu(curr, 25, 0);
        ra = Bitpack_getu(curr, 3, 25);
        load_value(ra, val, registers);
    }
    else{
        ra = Bitpack_getu(curr, 3, 6);
        rb = Bitpack_getu(curr, 3, 3);
        rc = Bitpack_getu(curr, 3, 0);
    switch(curr_op){
        case 0: conditional_move(ra, rb, rc, registers); break;
        case 1: seg_load(ra, rb, rc, registers, memory); break;
        case 2: seg_store(ra, rb, rc, registers, memory); break;
        case 3: addition(ra, rb, rc, registers); break;
        case 4: multiplication(ra, rb, rc, registers); break;
        case 5: division(ra, rb, rc, registers); break;
        case 6: nand(ra, rb, rc, registers); break;
        case 7: halt(registers, memory, addresses); break;
        case 8: map_seg(rb, rc, registers, memory, addresses); break;
        case 9: unmap_seg(rc, registers, memory, addresses); break;
        case 10: output(rc, registers); break;
        case 11: input(rc, registers); break;
        case 12: ctr = load_program(rb, rc, registers, memory, i, 
        num_instructions); break;
    }
    }
}