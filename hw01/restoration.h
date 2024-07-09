/****************************************************************************** 
                          Homework 1: FilesOfPix                           
               
    restoration.h
   
    Authors: Matt Torres (mtorre07). Kim Nguyen (knguye21)

    Summary: This is the interface for restoration, which contains the 
    definition of the pic struct, as well as definitions for important
    helper functions. 
 
*******************************************************************************/


#ifndef RESTORATION_H_
#define RESTORATION_H_

#include <stdlib.h>
#include <stdio.h>
#include <readaline.h>
#include <seq.h>
#include <atom.h>

/*
 * Pic Struct
*/
struct pic{
    /*Values for  header*/
    int magic_num;
    int width;
    int height;
    int maxval;

    /*Repeated sequence of non-digit characters used throughout*/
    char *repeated_sequence;
    
    /*All of the below hold char * in their void * pointers*/
    /*Sequence of all lines in P2 image*/
    Seq_T lines;

    /*Sequence of all injected sequences*/
    Seq_T sequences; 

    /*Sequence of all remaining lines*/
    Seq_T rem_lines;

};

/*Functions for dealing with injected lines*/
char *compare_sequences(struct pic *mypic);

void determine_sequence(struct pic *mypic);

void remove_injections(struct pic *mypic);

int compare(const char* str1, const char* str2);


/*Functions for dealing with injected characters*/
void restore_whitespace(struct pic *mypic);

/*Functions for outputting finished*/

char *restore_header(struct pic mypic);

void convert_to_raw(struct pic *mypic);

// void print_pic(struct pic mypic);

int get_full_int(char *line, int *index);

void free_pic(struct pic mypic);

/*Called from main*/
void read_file(FILE *inputfd);



#endif