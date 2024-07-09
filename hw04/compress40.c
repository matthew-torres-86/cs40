/****************************************************************************** 
                          Homework 4: arith                           
               
    compress40.c
   
    Authors: Matt Torres (mtorre07). Kim Nguyen (knguye21)

    Summary: 
 
*******************************************************************************/
#include <compress40.h>
#include <pnm.h>
#include <assert.h>
#include <stdio.h>
#include <a2methods.h>
#include <stdlib.h>
#include <string.h>
#include "a2plain.h"
#include "a2blocked.h"
#include "seq.h"
#include "bitpack.h"
#include <arith40.h>
#include <math.h>

const float DENOMINATOR = 255;
const float QUANTIZER = 50;
const unsigned PR_LSB = 0;
const unsigned PB_LSB = 4;
const unsigned D_LSB = 8; 
const unsigned C_LSB = 13; 
const unsigned B_LSB = 18; 
const unsigned A_LSB = 23;

typedef struct component_pix{
    float y;
    float pb;
    float pr;
}*component_pix;

typedef struct compressed_pix{
    float a, b, c, d;
    float pr, pb;
}*compressed_pix;

typedef struct component_img{
    int width;
    int height;
    A2Methods_UArray2 pixels;
    A2Methods_T methods;
} *component_img;

typedef struct compressing{
    Seq_T four_pix;
    component_img img;
}*compressing;

float no_negative(float pixel);
// extern void convert_to_RGB(component_img img, Pnm_ppm *decomp);
// void component_to_RGB(int i, int j, A2Methods_UArray2 pixmap, 
//                                     A2Methods_Object *ptr, void *cl);
void unpack_pixels(int i, int j, A2Methods_UArray2 pixmap, 
                                    A2Methods_Object *ptr, void *cl);

void nix_component(int i, int j, A2Methods_UArray2 pixmap, 
                                    A2Methods_Object *ptr, void *cl);

extern void reverse_component(component_img comp, Pnm_ppm *img);

void compressed_to_component(int i, int j, A2Methods_UArray2 pixmap, 
                                    A2Methods_Object *ptr, void *cl);

extern void print_RGB(Pnm_ppm pic){
    printf("PRINTING PIXELS:\n");
    for(int i = 0; i < (int)pic->height; i++){
        for(int j = 0; j < (int)pic->width; j++){
            Pnm_rgb pix = pic->methods->at(pic->pixels, j, i);
            printf("%u %u %u (%d, %d)\n", pix->red, pix->green, pix->blue, i, j);
        }
    }
    
}

/*****************COMPRESSION********************/

/*
* Trims off row or column if odd number of rows 
* or columns
*/
extern void even_dimensions(Pnm_ppm *img){
    
    int new_width = (*img)->width;
    int new_height = (*img)->height;

    /*Odd width, trim last column*/
    if((*img)->width % 2 != 0){
        new_width --;
    }

    /*Odd height, trim last row*/
    if((*img)->height % 2 != 0){
        new_height --;
    }

    A2Methods_UArray2 new_pixmap= (*img)->methods->
        new(new_width, new_height, sizeof(struct Pnm_rgb));
    
    for(int row = 0; row < new_height; row++){
        for(int col = 0; col < new_width; col++){

            Pnm_rgb curr = (*img)->methods->at((*img)->pixels, col, row);

            assert(curr);

            Pnm_rgb element = (*img)->methods->at(new_pixmap, col, row);

            *element = *curr;

        }
    }
    (*img)->pixels = new_pixmap;
    (*img)->width = new_width;
    (*img)->height = new_height;

}

/*
 * apply function used in convert to component. takes pixels rgb values
 * and converts them to y, pb and pr values for each pixel. 
*/
void RGB_to_component(int i, int j, A2Methods_UArray2 pixmap, 
                                    A2Methods_Object *ptr, 
                void *cl)
{
    Pnm_rgb curr = ptr;
    component_img *comp = cl;

    float r = curr->red / DENOMINATOR;
    float g = curr->green / DENOMINATOR;
    float b = curr->blue / DENOMINATOR;

    component_pix new_pix = malloc(sizeof(struct component_pix));
    new_pix->y = 0.299      * r + 0.587    * g + 0.114    * b;
    new_pix->pb = -0.168736 * r - 0.331264 * g + 0.5      * b;
    new_pix->pr = 0.5       * r - 0.418688 * g - 0.081312 * b;

    component_pix comp_pix = (*comp)->methods->at((*comp)->pixels, i, j);
    *comp_pix = *new_pix;
    (void)pixmap;
}

/*
 * converts all pixels from rgb space to component space with y, pb and pr 
 * values for each pixel
*/
extern void convert_to_component(Pnm_ppm img, component_img *comp){
    int width = img->width;
    int height = img->height;
    int size = sizeof(struct component_pix);
    
    (*comp)->methods = uarray2_methods_blocked;
    (*comp)->pixels = (*comp)->methods->new_with_blocksize
                                            (width, height, size, 2);
    // (*comp)->methods = uarray2_methods_plain;
    // (*comp)->pixels = (*comp)->methods->new(width, height, size);
    
    (*comp)->width = width;
    (*comp)->height = height;

    img->methods->map_row_major(img->pixels, RGB_to_component, comp);
}


/*
* Combines 4 pixels into one blocked megapixel, using average pb and pr values
* and a, b, c, d values as depicted in spec.
*/
//PROBLEM HERE??
void dct_pixels(int i, int j, A2Methods_UArray2 pixmap, 
                                    A2Methods_Object *ptr, void *cl)
{
    // printf("hi");
    component_pix curr = ptr;
    compressing *comp_struct = cl;

    Seq_addhi((*comp_struct)->four_pix, curr);

    if(Seq_length((*comp_struct)->four_pix) == 4){
        float avg_pb = 0.0;
        float avg_pr = 0.0;
        float y1, y2, y3, y4;
        component_pix pix1, pix2, pix3, pix4;
        compressed_pix new_pix = malloc(sizeof(struct compressed_pix));

        pix1 = Seq_remlo((*comp_struct)->four_pix);
        pix2 = Seq_remlo((*comp_struct)->four_pix);
        pix3 = Seq_remlo((*comp_struct)->four_pix);
        pix4 = Seq_remlo((*comp_struct)->four_pix);

        y1 = pix1->y;
        y2 = pix2->y;
        y3 = pix3->y;
        y4 = pix4->y;

        new_pix->a = (y4 + y3 + y2 + y1)/4.0;
        new_pix->b = (y4 + y3 - y2 - y1)/4.0;
        new_pix->c = (y4 - y3 + y2 - y1)/4.0;
        new_pix->d = (y4 - y3 - y2 + y1)/4.0;

        // printf("DCT YYYY %f %f %f %f \n", y1, y2, y3, y4);

        avg_pb += pix1->pb;
        avg_pb += pix2->pb;
        avg_pb += pix3->pb;
        avg_pb += pix4->pb;
        
        avg_pr += pix1->pr;
        avg_pr += pix2->pr;
        avg_pr += pix3->pr;
        avg_pr += pix4->pr;

        
        new_pix->pb = avg_pb / 4.0;
        new_pix->pr = avg_pr / 4.0;

        // printf("DCT ABCD %f %f %f %f %f %f\n", new_pix->a, new_pix->b, new_pix->c, new_pix->d, curr->pb, curr->pr);

        compressed_pix curr_pix =
        (*comp_struct)->img->methods->at((*comp_struct)->img->pixels, (i-1)/2, (j-1)/2);

        *curr_pix = *new_pix; 
        (void)pixmap;
    }


}

extern void compress_pixels(component_img comp, component_img *compressed){
    int width = comp->width / 2;
    int height = comp->height / 2;
    int size = sizeof(struct compressed_pix);

    // printf("%d %d\n", width, height);
    
    (*compressed)->methods = uarray2_methods_plain;
    (*compressed)->pixels = (*compressed)->methods->new(width, height, size);
    (*compressed)->width = width;
    (*compressed)->height = height;


    compressing comp_str = malloc(sizeof(struct compressing));
    comp_str->four_pix = Seq_new(4);
    comp_str->img = *compressed;
    // // // // printf("width: %d\n", comp_str->img->width);
    // printf("hi 1");
    comp->methods->map_block_major(comp->pixels, dct_pixels, &comp_str);
}

void pack_pixels(int i, int j, A2Methods_UArray2 pixmap, 
                                    A2Methods_Object *ptr, void *cl){
    // // // // printf("%d %d\n", i, j);
    compressed_pix curr = ptr;
    A2Methods_UArray2 *packs = cl;
    A2Methods_T methods = uarray2_methods_plain; 
    uint64_t *index = methods->at(*packs, i, j);
    // // // // printf("%d %d\n", i, j);

    uint64_t word = 0;

    unsigned a = curr->a * 511;
    signed b;
    signed c;
    signed d;

    

    if(curr->b <= 0.3 && curr->b >= -0.3){
        b = QUANTIZER * curr->b;
    }
    else if(curr->b < 0){
        b = QUANTIZER * -0.3;
    }
    else{
        b = QUANTIZER * 0.3;
    }

    if(curr->c <= 0.3 && curr->c >= -0.3){
        c = QUANTIZER * curr->c;
    }
    else if(curr->c < 0){
        c = QUANTIZER * -0.3;
    }
    else{
        c = QUANTIZER * 0.3;
    }

    if(curr->d <= 0.3 && curr->d >= -0.3){
        d = QUANTIZER * curr->d;
    }
    else if(curr->d < 0){
        d = QUANTIZER * -0.3;
    }
    else{
        d = QUANTIZER * 0.3;
    }
    
    
    
    unsigned pb =  Arith40_index_of_chroma(curr->pb);
    unsigned pr = Arith40_index_of_chroma(curr->pr);

    // printf("PACK ABCD %d %d %d %d %u %u\n", a, b, c, d, pb, pr);

    // // // printf("indexes %u %u\n", pb, pr);

    word = Bitpack_newu(word, 4, PR_LSB, pr);
    // // // // printf("%lu ", word);
    word = Bitpack_newu(word, 4, PB_LSB, pb);
    // // // // printf("%lu ", word);
    word = Bitpack_news(word, 5, D_LSB, d);
    // // // // printf("%lu ", word);
    word = Bitpack_news(word, 5, C_LSB, c);
    // // // // printf("%lu ", word);
    word = Bitpack_news(word, 5, B_LSB, b);
    // // // // printf("%lu ", word);
    word = Bitpack_newu(word, 9, A_LSB, a);
    // // // // printf("%lu \n", word);
    
    *index = word;
    (void)pixmap;
}


void write_codewords(int i, int j, A2Methods_UArray2 pixmap, 
                                    A2Methods_Object *ptr, void *cl){
    uint64_t *word = ptr;
    putchar(Bitpack_getu(*word, 8, 24));
    putchar(Bitpack_getu(*word, 8, 16));
    putchar(Bitpack_getu(*word, 8, 8));
    putchar(Bitpack_getu(*word, 8, 0));
    
    (void)pixmap;
    (void)cl;
    (void)i;
    (void)j;
}

////////////////////////////////////////////////////////////////////////////////

extern void compress40  (FILE *input){
    

    A2Methods_T methods = uarray2_methods_plain; 

    /*READ IMAGE*/
    Pnm_ppm img = Pnm_ppmread(input, methods);
    assert(img && img->width > 1 && img->height > 1);
    
    

    /*Make dimensions even*/
    if((img->width % 2 != 0) || (img->height % 2 != 0)){
        even_dimensions(&img);
    }



    /*Convert RGB pixels to component*/
    component_img comp_blocked = malloc(sizeof(struct component_img));
    convert_to_component(img, &comp_blocked); 

    /*Convert component pixels to 4-pixel blocks, effectively compressing them*/
    component_img compressed = malloc(sizeof(struct component_img));
    compress_pixels(comp_blocked, &compressed);

    /*Pack pixels */
    // printf("PACKING PIXELS\n");
    A2Methods_UArray2 packed_array = methods->new(compressed->width, 
                            compressed->height, 8);
    methods->map_row_major(compressed->pixels, pack_pixels, &packed_array);

    /*Print header and codewords*/
    printf("COMP40 Compressed image format 2\n%u %u\n", compressed->width, compressed->height);
    methods->map_row_major(packed_array, write_codewords, &img);

    /*END COMPRESSION*/   
    
}



/****************DECOMPRESSION**********************/

void unpack_pixels(int i, int j, A2Methods_UArray2 pixmap, 
                                    A2Methods_Object *ptr, void *cl){
    // // // // printf("%d %d\n", i, j);
    uint64_t *word = ptr;
    A2Methods_UArray2 *unpacks = cl;
    A2Methods_T methods = uarray2_methods_plain; 
    compressed_pix pix = methods->at(*unpacks, i, j);

    compressed_pix new_pix = malloc(sizeof(struct compressed_pix));
    //ISSUE WITH PB AND PRRRRRRR///
    new_pix->pr = Arith40_chroma_of_index(Bitpack_getu(*word, 4, PR_LSB));
    new_pix->pb = Arith40_chroma_of_index(Bitpack_getu(*word, 4, PB_LSB));
    new_pix->d  = Bitpack_gets(*word, 5, D_LSB) / QUANTIZER;
    new_pix->c  = Bitpack_gets(*word, 5, C_LSB) / QUANTIZER;
    new_pix->b  = Bitpack_gets(*word, 5, B_LSB) / QUANTIZER;
    new_pix->a = Bitpack_getu(*word, 9, A_LSB) / 511;
    
    // printf("%d %d %f %f %f %f %f %f\n ", i, j, new_pix->a, new_pix->b, new_pix->c, new_pix->d, new_pix->pr, new_pix->pb);

    *pix = *new_pix;
    (void)pixmap;
}

void nix_component(int i, int j, A2Methods_UArray2 pixmap, 
                                    A2Methods_Object *ptr, void *cl)
{
    component_pix comp = ptr;
    Pnm_ppm *img = cl;
    Pnm_rgb curr = (*img)->methods->at((*img)->pixels, i, j);
    // printf("%d %d\n", i, j);
    float y = comp->y;
    float pb = comp->pb;
    float pr = comp->pr;
    curr->red = no_negative((1.0 * y + 0.0 * pb + 1.402 * pr) * DENOMINATOR) ;
    curr->green = no_negative((1.0 * y - 0.344136 * pb - 0.714136 * pr) * DENOMINATOR);
    curr->blue = no_negative((1.0 * y + 1.772 * pb + 0.0 * pr) * DENOMINATOR);
    (void)pixmap;
}

extern void reverse_component(component_img comp, Pnm_ppm *img){
    int width = comp->width;
    int height = comp->height;
    int size = sizeof(struct Pnm_rgb);
    
    (*img)->methods = uarray2_methods_plain;
    (*img)->pixels = (*img)->methods->new(width, height, size);
    
    (*img)->width = width;
    (*img)->height = height;
    (*img)->denominator = DENOMINATOR;

    (*img)->methods->map_row_major(comp->pixels, nix_component, img);
}

float no_negative(float pixel){
    if(pixel < 0.0){
        return 0.0;
    }
    if(pixel > DENOMINATOR){
        return DENOMINATOR;
    }
    return pixel;
}

void load_component(float y, float pb, float pr, component_pix *pix){
    
    (*pix)->y = y;
    (*pix)->pb = pb;
    (*pix)->pr = pr;
}

//PROBLEM HERE?s
void compressed_to_component(int i, int j, A2Methods_UArray2 pixmap, 
                                    A2Methods_Object *ptr, void *cl)
{
    compressed_pix curr = ptr;
    component_img *decomp = cl;

    float a = curr->a; 
    float b = curr->b; 
    float c = curr->c; 
    float d = curr->d; 
    float pr = curr->pr;
    float pb = curr ->pb;
    float y;

    // printf("COMP ABCD %f %f %f %f\n ", a, b, c, d);
    
    component_pix pix1 = malloc(sizeof(struct Pnm_rgb));
    component_pix pix2 = malloc(sizeof(struct Pnm_rgb));
    component_pix pix3 = malloc(sizeof(struct Pnm_rgb));
    component_pix pix4 = malloc(sizeof(struct Pnm_rgb));

    y = a - b - c + d;
    load_component(y, pb, pr, &pix1);
    
    y = a - b + c - d;
    load_component(y, pb, pr, &pix2);

    y = a + b - c - d;
    load_component(y, pb, pr, &pix3);

    y = a + b + c + d;
    load_component(y, pb, pr, &pix4);

    component_pix idx1 = (*decomp)->methods->at((*decomp)->pixels, i * 2, j * 2);
    component_pix idx2 = (*decomp)->methods->at((*decomp)->pixels, i * 2, j * 2 + 1);
    component_pix idx3 = (*decomp)->methods->at((*decomp)->pixels, i * 2 + 1, j * 2);
    component_pix idx4 = (*decomp)->methods->at((*decomp)->pixels, i * 2+ 1, j * 2 + 1);
    *idx1 = *pix1;
    *idx2 = *pix2;
    *idx3 = *pix3;
    *idx4 = *pix4;
    (void)pixmap;
} 


extern void decompress40(FILE *input){
    
    unsigned height, width;
    int read = fscanf(input, "COMP40 Compressed image format 2\n%u %u", &width, &height);
    assert(read == 2); 
    int c = getc(input);
    assert(c == '\n');

    A2Methods_T methods = uarray2_methods_plain;
    A2Methods_UArray2 packed_array = methods->new(width, height, 8);
    //  printf("%u %u\n", width, height);
    for(int r = 0; r < (int)height; r ++){
        for(int c = 0; c < (int)width; c++){
            // printf("%d %d\n", c, r);
            uint64_t *idx = methods->at(packed_array, c, r);
            uint64_t curr = 0;
            curr = Bitpack_newu(curr, 8, 24, getc(input));
            curr = Bitpack_newu(curr, 8, 16, getc(input));
            curr = Bitpack_newu(curr, 8, 8, getc(input));
            curr = Bitpack_newu(curr, 8, 0, getc(input));
            *idx = curr;
        }
    }

    A2Methods_UArray2 unpacked_array = methods->new(width, 
                            height, sizeof(struct compressed_pix));
    methods->map_row_major(packed_array, unpack_pixels, &unpacked_array);

    width *= 2;
    height *= 2;

    /*Reverting compressed blocks to individual component pixels*/
    component_img decomp = malloc(sizeof(struct component_img));
    decomp->methods = uarray2_methods_plain;
    decomp->pixels = decomp->methods->new(width, height, sizeof(struct component_pix));
    decomp->width = width;
    decomp->height = height;
    methods->map_row_major(unpacked_array, compressed_to_component, &decomp);

    /*Conerting component space to RGB space*/
    Pnm_ppm new_img = malloc(sizeof(struct Pnm_ppm));
    reverse_component(decomp, &new_img); 

    /*Write decompressed image*/
    Pnm_ppmwrite(stdout, new_img);
    
    // component_img compressed = malloc(sizeof(struct component_img));
    // compressed->pixels = unpacked_array;
    // compressed->methods = uarray2_methods_plain;
    // compressed->width = width;
    // compressed->height = height;

    // Pnm_ppm decomp = malloc(sizeof(struct Pnm_ppm));
    // // convert_to_RGB(compressed, &decomp);


    // // comp->methods->map_row_major(comp->pixels, reverse_component, &img);

    
    // decomp->width = compressed->width * 2;
    // decomp->height = compressed->height * 2;
    // // print_RGB(decomp);

    // Pnm_ppmwrite(stdout, decomp);
}