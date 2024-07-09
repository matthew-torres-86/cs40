#include "stdio.h"
#include "stdlib.h"
#include "uarray.h"
#include "seq.h"
#include "assert.h"
#include "stdint.h"
#include "bitpack.h"

typedef Seq_T segment;
typedef uint32_t word;

void read_instruction(uintptr_t curr, UArray_T *registers, Seq_T *memory,
    Seq_T *addresses, int *i, int *num_instructions);