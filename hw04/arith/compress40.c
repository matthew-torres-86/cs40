/****************************************************************************** 
                          Homework 4: arith                           
               
    compress40.c
   
    Authors: Matt Torres (mtorre07). Kim Nguyen (knguye21)

    Summary: compresses and decompresses an image
 
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

const float DENOMINATOR = 255.0;
const float QUANTIZER = 50.0;
const unsigned PR_LSB = 0;
const unsigned PB_LSB = 4;
const unsigned D_LSB = 8; 
const unsigned C_LSB = 14; 
const unsigned B_LSB = 20; 
const unsigned A_LSB = 26;

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
void unpack_pixels(int i, int j, A2Methods_UArray2 pixmap, 
                                    A2Methods_Object *ptr, void *cl);

void component_to_RGB(int i, int j, A2Methods_UArray2 pixmap, 
                                    A2Methods_Object *ptr, void *cl);

extern void reverse_component(component_img comp, Pnm_ppm *img);

void compressed_to_component(int i, int j, A2Methods_UArray2 pixmap, 
                                    A2Methods_Object *ptr, void *cl);


/*******************************************************************************
*                       Compression Helper Functions                           *
*                                                                              *
*******************************************************************************/


/* even_dimensions()
* Description: Trims off row or column if image has an odd number of rows or 
* columns
* Input: takes a pointer to a Pnm_ppm, which is expected to be in a valid format
* Output: modifies image to have an even number of rows and values.
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
    (*img)->methods->free(&((*img)->pixels));
    (*img)->pixels = new_pixmap;
    (*img)->width = new_width;
    (*img)->height = new_height;

}

/* RGB_to_component()
 * apply function used in convert to component. takes pixels rgb values
 * and converts them to y, pb and pr values for each pixel. 
 * Input: Takes the usual apply function parameters: int i and j
 * coordinates, the array of pixels, a pointer to the current pixel
 * and a void pointer for the closure
 * Output: places a new component_pix struct in the component_pix
 * pointed to by closure.  
*/
void RGB_to_component(int i, int j, A2Methods_UArray2 pixmap, 
                                    A2Methods_Object *ptr, void *cl)
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

/* RGB_to_component()
 * Description: Converts each pixel from the RGB space to Component 
 * space.
 * Input: Takes a PNM image and a pointer to an empty component_image
 * Output: modifies the component_image by filling with new component_pix
*/

extern void convert_to_component(Pnm_ppm img, component_img *comp){
    int width = img->width;
    int height = img->height;
    int size = sizeof(struct component_pix);
    
    (*comp)->methods = uarray2_methods_blocked;
    (*comp)->pixels = (*comp)->methods->new_with_blocksize
                                            (width, height, size, 2);

    (*comp)->width = width;
    (*comp)->height = height;

    img->methods->map_row_major(img->pixels, RGB_to_component, comp);
}


/* dct_pixels()
* Description: Combines 4 pixels into one blocked superpixel, using average pb 
* and pr values and a, b, c, d values as depicted in spec. Used as an apply 
* function in compress_pixels
* Input: Takes the usual apply function parameters: int i and j
* coordinates, the array of pixels, a pointer to the current pixel
* and a void pointer for the closure
* output: modifies the comp_struct (which contains a compressed image
* and a Seq of size 4).
*/
void dct_pixels(int i, int j, A2Methods_UArray2 pixmap, 
                                    A2Methods_Object *ptr, void *cl)
{
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

        compressed_pix curr_pix =
        (*comp_struct)->img->methods->at((*comp_struct)->img->pixels, (i-1)/2, (j-1)/2);

        *curr_pix = *new_pix; 
        (void)pixmap;
    }


}

/* dct_pixels()
* Description: Calls dct_pixels as an apply function to create a component image
* that holds all the compressed pixels. 
* Input: Takes a component image full of pixels in the component space.
* also takes an empty component image which will have 1/2 the width of the 
* original image
* output: fills the compressed image with superpixles
*/
extern void compress_pixels(component_img comp, component_img *compressed){
    int width = comp->width / 2;
    int height = comp->height / 2;
    int size = sizeof(struct compressed_pix);
    
    (*compressed)->methods = uarray2_methods_plain;
    (*compressed)->pixels = (*compressed)->methods->new(width, height, size);
    (*compressed)->width = width;
    (*compressed)->height = height;


    compressing comp_str = malloc(sizeof(struct compressing));
    comp_str->four_pix = Seq_new(4);
    comp_str->img = *compressed;
    comp->methods->map_block_major(comp->pixels, dct_pixels, &comp_str);
    Seq_free(&(comp_str->four_pix));
    free(comp_str);
}

/* quantize()
* Description: a helper function that ensures a float is within range
* [-0.3, 0.3]
* Input: Takes a float
* output: returns the quantized float
*/
float quantize(float num){
    if(num <= 0.3 && num >= -0.3){
        return QUANTIZER * num;
    }
    else if(num < 0){
        return QUANTIZER * -0.3;
    }
    else{
        return QUANTIZER * 0.3;
    }
}

/* pack_pixels()
* Description: utilizes bitpack to convert each of the compressed pixels
* into a word
* Input: Takes the usual apply function parameters: int i and j
* coordinates, the array of pixels, a pointer to the current pixel
* and a void pointer for the closure
* output: returns via closure a pointer to a codeword.
*/
void pack_pixels(int i, int j, A2Methods_UArray2 pixmap, 
                                    A2Methods_Object *ptr, void *cl){

    compressed_pix curr = ptr;
    A2Methods_UArray2 *packs = cl;
    A2Methods_T methods = uarray2_methods_plain; 
    uint64_t *index = methods->at(*packs, i, j);
    uint64_t word = 0;

    unsigned a = curr->a * 63;
    signed b = quantize(curr->b);
    signed c = quantize(curr->c);
    signed d = quantize(curr->d);
    unsigned pb =  Arith40_index_of_chroma(curr->pb);
    unsigned pr = Arith40_index_of_chroma(curr->pr);

    word = Bitpack_newu(word, 4, PR_LSB, pr);
    word = Bitpack_newu(word, 4, PB_LSB, pb);
    word = Bitpack_news(word, 6, D_LSB, d);
    word = Bitpack_news(word, 6, C_LSB, c);
    word = Bitpack_news(word, 6, B_LSB, b);
    word = Bitpack_newu(word, 6, A_LSB, a);
    
    *index = word;
    (void)pixmap;
}

/* dct_pixels()
* Description: an apply function which prints each of the codewords to a file
* Input: Takes the usual apply function parameters: int i and j
* coordinates, the array of pixels, a pointer to the current pixel
* and a void pointer for the closure
* output: prints to stdout
*/
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


/* compress40()
* Description: Calls helper functions to compress a PPM image, and 
* prints the Comp40 compressed image to stdout
* Input: Takes a pointer to an open file, which is expected to be
* in the Pnm_ppm standard format with a width and height greater than 1
* output: prints compressed image to standard out.
*/
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
    A2Methods_UArray2 packed_array = methods->new(compressed->width, 
                            compressed->height, 8);
    methods->map_row_major(compressed->pixels, pack_pixels, &packed_array);

    /*Print header and codewords*/
    printf("COMP40 Compressed image format 2\n%u %u\n", compressed->width, compressed->height);
    methods->map_row_major(packed_array, write_codewords, &img);

    /*END COMPRESSION*/   
    comp_blocked->methods->free((&comp_blocked->pixels));
    free(comp_blocked);

    methods->free(&(compressed->pixels));
    free(compressed);

    methods->free(&packed_array);

    Pnm_ppmfree(&img);
}



/*******************************************************************************
*                      Deompression Helper Functions                           *
*                                                                              *
*******************************************************************************/
/* unpack_pixels()
* Description: utilizes bitpack to unpack each word and convert to a compressed
* super pixel
* Input: Takes the usual apply function parameters: int i and j
* coordinates, the array of pixels, a pointer to the current pixel
* and a void pointer for the closure
* output: returns via closure a pointer to an unpacked superpixel.
*/
void unpack_pixels(int i, int j, A2Methods_UArray2 pixmap, 
                                    A2Methods_Object *ptr, void *cl){
    uint64_t *word = ptr;
    A2Methods_UArray2 *unpacks = cl;
    A2Methods_T methods = uarray2_methods_plain; 
    compressed_pix pix = methods->at(*unpacks, i, j);

    compressed_pix new_pix = malloc(sizeof(struct compressed_pix));
    new_pix->pr = Arith40_chroma_of_index(Bitpack_getu(*word, 4, PR_LSB));
    new_pix->pb = Arith40_chroma_of_index(Bitpack_getu(*word, 4, PB_LSB));
    new_pix->d  = Bitpack_gets(*word, 6, D_LSB) / QUANTIZER;
    new_pix->c  = Bitpack_gets(*word, 6, C_LSB) / QUANTIZER;
    new_pix->b  = Bitpack_gets(*word, 6, B_LSB) / QUANTIZER;
    new_pix->a = Bitpack_getu(*word, 6, A_LSB) / 63.0;

    *pix = *new_pix;
    (void)pixmap;
}

/* component_to_RGB()
* Description: serves as an apply function which converts individual
* pixels from a component space to a RGB space
* Input: Takes the usual apply function parameters: int i and j
* coordinates, the array of pixels, a pointer to the current pixel
* and a void pointer for the closure
* output: returns via closure a pointer to a Pnm_RGB struct.
*/
void component_to_RGB(int i, int j, A2Methods_UArray2 pixmap, 
                                    A2Methods_Object *ptr, void *cl)
{
    component_pix comp = ptr;
    Pnm_ppm *img = cl;
    Pnm_rgb curr = (*img)->methods->at((*img)->pixels, i, j);

    float y = comp->y;
    float pb = comp->pb;
    float pr = comp->pr;
    curr->red = no_negative((1.0 * y + 0.0 * pb + 1.402 * pr) * DENOMINATOR) ;
    curr->green = no_negative((1.0 * y - 0.344136 * pb - 0.714136 * pr) * DENOMINATOR);
    curr->blue = no_negative((1.0 * y + 1.772 * pb + 0.0 * pr) * DENOMINATOR);
    (void)pixmap;
}

/* reverse_component()
* Description: utilizes component_to_rgb as an apply function which converts
* an image of component pix to an image of RGB pix.
* Input: takes an image of component pix, and a pointer to an empty Pnm_ppm
* output: fills the Pnm_ppm with converted RGB pixels
*/
extern void reverse_component(component_img comp, Pnm_ppm *img){
    int width = comp->width;
    int height = comp->height;
    int size = sizeof(struct Pnm_rgb);
    
    (*img)->methods = uarray2_methods_plain;
    (*img)->pixels = (*img)->methods->new(width, height, size);
    
    (*img)->width = width;
    (*img)->height = height;
    (*img)->denominator = DENOMINATOR;

    (*img)->methods->map_row_major(comp->pixels, component_to_RGB, img);
}

/* no_negative()
* Description: ensures that there is less error, by converting all of the
* RGB values so that they are in the range [0, 255]
* Input: Takes a float
* output: returns a float that is in the range [0, 255]
*/
float no_negative(float pixel){
    if(pixel < 0.0){
        return 0.0;
    }
    if(pixel > DENOMINATOR){
        return DENOMINATOR;
    }
    return pixel;
}

/* load component()
* Description: a helper function that loads y, pb, and pr values 
* into a pixel 
* Input: floats for the y, pb, and pr values as well as a pointer
* to a component pix to load with them
* output: loads the pix with the component values
*/
void load_component(float y, float pb, float pr, component_pix *pix){
    
    (*pix)->y = y;
    (*pix)->pb = pb;
    (*pix)->pr = pr;
}

/* compressed_to_component()
* Description: takes the compressed super pixels and converts them
* each to 4 pixels using calculations specified in the spec. Serves as
* an apply function for decompress_40.
* Input: Takes the usual apply function parameters: int i and j
* coordinates, the array of pixels, a pointer to the current pixel
* and a void pointer for the closure
* output: returns via closure a pointer to a component_pix
*/
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

/* decompress40()
* Description: takes a compressed comp40 image and converts it
* to a Pnm_ppm by calling multiple helper functions. prints 
* decompressed Pnm_ppm to stdout
* Input: takes a pointer to an open file, which is expected 
* to be in the valid Comp40 compressed image format.
* output: prints decompressed Pnm_ppm to standard out
*/
extern void decompress40(FILE *input){
    
    unsigned height, width;
    int read = fscanf(input, "COMP40 Compressed image format 2\n%u %u", &width, &height);
    assert(read == 2); 
    int c = getc(input);
    assert(c == '\n');
    A2Methods_T methods = uarray2_methods_plain;
    A2Methods_UArray2 packed_array = methods->new(width, height, 8);
    for(int r = 0; r < (int)height; r ++){
        for(int c = 0; c < (int)width; c++){
            
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

    methods->free(&(decomp->pixels));
    free(decomp);

    methods->free(&packed_array);
    
    methods->free(&unpacked_array);

    Pnm_ppmfree(&new_img);   
}