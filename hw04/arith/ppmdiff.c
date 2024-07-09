#include <pnm.h>
#include <assert.h>
#include <stdio.h>
#include <a2methods.h>
#include <stdlib.h>
#include <string.h>
#include "a2plain.h"
#include <math.h>

double diff(Pnm_ppm *i1, Pnm_ppm *i2, int smwidth, int smheight){
    Pnm_ppm img1 = *i1;
    Pnm_ppm img2 = *i2;
    A2Methods_UArray2 pix1 = img1->pixels;
    A2Methods_UArray2 pix2 = img2->pixels;
    double reddiff =0;
    double greendiff = 0;
    double bluediff = 0;
    double difftotal = 0;
    double denom = 3 * smwidth * smheight;
    // printf("%f", denom);
    
    
    for(int i = 0; i< smheight; i++){
        for(int j = 0; j < smwidth; j++){
            
            Pnm_rgb pixel1 = img1->methods->at(pix1, j, i);
            Pnm_rgb pixel2 = img2->methods->at(pix2, j, i);

            reddiff = (((double)pixel1->red - (double)pixel2->red)/(double)img1->denominator) * (((double)pixel1->red - (double)pixel2->red)/(double)img1->denominator);

            greendiff = (((double)pixel1->green - (double)pixel2->green) / (double)img1->denominator) * (((double)pixel1->green - (double)pixel2->green) / (double)img1->denominator);
            
            bluediff = (((double)pixel1->blue - (double)pixel2->blue) / (double)img1->denominator) * (((double)pixel1->blue - (double)pixel2->blue) / (double)img1->denominator);

            difftotal += (reddiff + greendiff + bluediff);
        }
    }
    difftotal = difftotal / denom;
    return sqrt( difftotal);
}


int main(int argc, char **argv){
    assert(argc == 3);
    FILE *f1;
    FILE *f2;
    if(!strcmp(argv[1], "-")){
        f1 = stdin;

        if(!strcmp(argv[2], "-")){
            fprintf(stderr, "%s does not support 2 \'-\' args\n",
                                argv[0]); 
            exit(EXIT_FAILURE);
        }
    }
    if(!strcmp(argv[2], "-")){
        f2 = stdin;
    }

    f1 = fopen(argv[1], "r");
    assert(f1);
    f2 = fopen(argv[2], "r");
    assert(f2);
    
    
    A2Methods_T methods = uarray2_methods_plain; 

    Pnm_ppm img1 = Pnm_ppmread(f1, methods);
    Pnm_ppm img2 = Pnm_ppmread(f2, methods);

    

    if(((int)(img1->width - img2->width) > 1) 
         || ((int)(img1->width - img2->width) < -1)){
        fprintf(stderr, "Usage: Image widths should differ by at most 1");
        printf("1.0\n");
    }
    else if(((int)(img1->height - img2->height) > 1) 
         || ((int)(img1->height - img2->height) < -1)){
        fprintf(stderr, "Usage: Image heights should differ by at most 1");
        printf("1.0\n");
    }
    else{
        int smwidth = 0;
        int smheight = 0;

        if(img1->width < img2->width){
            smwidth = img1->width;
        }
        else{
            smwidth = img2->width;
        }
        if(img1->height < img2->height){
            smheight = img1->height;
        }
        else{
            smheight = img2->height;
        }

        printf("%0.4f\n", diff(&img1, &img2, smwidth, smheight));
    }
    
}