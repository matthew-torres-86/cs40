/****************************************************************************** 
                          Homework 4: arith                           
               
    bitpack.c
   
    Authors: Matt Torres (mtorre07). Kim Nguyen (knguye21)

    Summary: 
 
*******************************************************************************/
#include "bitpack.h"
#include <math.h>
#include <assert.h>
#include <stdio.h>
#include "except.h"
Except_T Bitpack_Overflow = { "Overflow packing bits" };


bool Bitpack_fitsu(uint64_t n, unsigned width){
    assert(width <= 64);
    if(width == 0){
        return false;
    }
    uint64_t mask = ~0;
    mask = mask >> (64 - width - 1);
    return (n <= mask);
}

bool Bitpack_fitss(int64_t n, unsigned width){
    assert(width <= 64);
    if(width == 0){
        return false;
    }

    int64_t mask_h = 1;
    mask_h = mask_h << (width - 1);
    int64_t mask_l = ~(mask_h);

    return (n > mask_l) && (n < mask_h);
}

uint64_t Bitpack_getu(uint64_t word, unsigned width, unsigned lsb){
    assert(width <= 64);
    assert(width + lsb <= 64);
    uint64_t mask = ~0;
    mask = mask >> (64 - width);
    word = word >> lsb;
    return word & mask;
}

int64_t Bitpack_gets(uint64_t word, unsigned width, unsigned lsb){
    assert(width <= 64);
    assert(width + lsb <= 64);

    uint64_t mask = ~0;
    mask = mask >> (64 - width);
    mask = mask << lsb;
    word = word & mask;
    // printf("%li\n", mask);
    int64_t new_word = ~0;
    word = word << (64 - width - lsb);
    new_word = word & new_word;
    // printf("%li\n", new_word);
    new_word = new_word >> (64 - width);
    // printf("%li\n", new_word);

    return new_word;
    
}

uint64_t Bitpack_newu(uint64_t word, unsigned width, unsigned lsb, 
                      uint64_t value){
    assert(width <= 64);
    assert(width + lsb <= 64);
    
    if(!Bitpack_fitsu(value, width)){
        RAISE(Bitpack_Overflow);
    }
    
    /*clearing values*/
    uint64_t mask = ~0;
    mask = mask >> (64 - width);
    mask = mask << lsb;
    mask = ~mask;
    word = word & mask;

    /*Adding new values*/
    value = value << lsb;
    word = word | value;
    return word;
}

uint64_t Bitpack_news(uint64_t word, unsigned width, unsigned lsb,  
                      int64_t value){
    assert(width <= 64);
    assert(width + lsb <= 64);
    // printf("%u %li\n", width, value);
    if(!Bitpack_fitss(value, width)){
        RAISE(Bitpack_Overflow);
    }

    uint64_t mask = ~0;
    mask = mask >> (64 - width);
    
    /*CLEARING VALUE IN RANGE MSB TO LSB*/
    mask = mask << lsb;

    uint64_t new_value = 0;
    value = value << lsb;
    new_value = new_value | value;
    new_value = new_value & mask;

    mask = ~mask;
    word = word & mask;

    /*Adding new values*/
    word = word | new_value;
    return word;
}
