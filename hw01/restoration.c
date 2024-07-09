/****************************************************************************** 
                          Homework 1: FilesOfPix                           
               
    Restoration.c
   
    Authors: Matt Torres (mtorre07). Kim Nguyen (knguye21)

    ISSUE FOR OFFICE HOURS>> PGM IMAGE HAS MORE THAN DESIRED NUMBER OF LINES?
    WHY? \n characters???

    WHAT ABOUT CHARACTERS THAT DO NOT SHOW UP IN TEXT EDITOR?
    
 
*******************************************************************************/

#include <restoration.h>

const char WHITESPACE = ' ';

/*
 * compare()
 *
 * Helper function for compare_sequences()
 * Args: two character arrays to compare
 * Structures: hanson's atom.h
 * Returns: size of line, contents of line
 *  
 * ERROR: Valgrind- Conditional jump or move depends on uninitialized value (23)
 * --ALLOWED BECAUSE OF LIMITATIONS OF HANSON'S ATOM--
 */
int compare(const char* str1, const char* str2){
    str1 = Atom_string(str1); 
    str2 = Atom_string(str2);
    if(str1 == NULL || str2 == NULL){
        return 0;
    }
    if (str1 == str2) { 
        
        return 1;
    }
    return 0;
}

/*
 * compare sequences()
 *
 * compares all of the char arrays in the Seq "mypic->sequences
 * Args: referenced pic struct
 * Returns: the repeated char sequence in the Seq "mypic->sequences"
 *  
 * No errors
 */
char *compare_sequences(struct pic *mypic){
    char *iseq;
    char *jseq;

    /*double for loop style compare algorithm*/
    for(int i = 0; i < Seq_length(mypic->sequences); i++){
        iseq = Seq_get(mypic->sequences, i);
        for(int j = i + 1; j < Seq_length(mypic->sequences); j++){
            jseq = Seq_get(mypic->sequences, j);
            
            /*compare helper funciton*/
            if(compare(iseq, jseq)){
            
                return iseq;
            }
        }
    }
    return '\0';
}

/*
 * determine sequence()
 *
 * Determines repeated char sequence in Seq sequences and stores in struct
 * Args: referenced pic struct
 * Returns: the repeated char string in the Seq "mypic->sequences"
 *  
 * ERROR: Valgrind- Conditional jump or move depends on uninitialized value
 */
void determine_sequence(struct pic *mypic)
{
    mypic->sequences = Seq_new(50);
    
    int seq_index;

    /*building non-int sequences by searching through
    each line for non-int chars*/
    for(int i = 0; i<mypic->height; i++){
        char *curr_sequence = malloc(10000);
        char *line = Seq_get(mypic->lines, i);
        seq_index = 0; 
        
        for(int j = 0; j < mypic->width; j++){
            
            if(line[j] < '0' || line[j] > '9'){
                curr_sequence[seq_index] = line[j];
                seq_index ++;
            }
        }
        curr_sequence[seq_index] = '\0';
        /*adding sequences to Seq of sequences*/
        Seq_addhi(mypic->sequences, curr_sequence);
    }

    /*comparing sequences from Seq of sequences*/
    mypic->repeated_sequence = compare_sequences(mypic);
}

/*
 * remove injections()
 *
 * creates new Seq with only the non-injected lines
 * Args: referenced pic struct
 * Returns: none, but stores data in struct
 *  
 * ERROR: Valgrind- Conditional jump or move depends on uninitialized value (71)
 */
void remove_injections(struct pic *mypic)
{
    mypic->height = 0;
    mypic->rem_lines = Seq_new(100);
    
    /**/
    for(int i = 0; i < Seq_length(mypic->sequences); i++){
        
        char *curr_seq = Seq_get(mypic->sequences, i);
        
        if(compare(curr_seq, mypic->repeated_sequence)){
           Seq_addhi(mypic->rem_lines, Seq_get(mypic->lines, i)); 
           mypic->height++;
        }
    }
}


/*
 * restore whitespace()
 *
 * replaces existing non-digit characters with whitespace
 * Args: referenced pic struct
 * Returns: none, but stores data in struct
 *  
 * ERROR: Valgrind- Conditional jump or move depends on uninitialized value (71)
 */
void restore_whitespace(struct pic *mypic)
{
    for(int i = 0; i<mypic->height; i++){
        
        char *line = Seq_get(mypic->rem_lines, i);

        for(int j = 0; j<mypic->width; j++){
            
            if(line[j] < '0' || line[j] > '9'){
                line[j] = WHITESPACE;
            }
        }
    }
        
}
    

/*
 * get full int()
 *
 * creates new Seq with only the non-injected lines
 * Args: referenced pic struct
 * Returns: none, but stores data in struct
 *  
 * ERROR: Valgrind- Conditional jump or move depends on uninitialized value (71)
 */
int get_full_int(char *line, int *index)
{
    char* num = malloc(12);
    num[0] = '0';
    int num_index = 0;

    while(line[*index] >= '0' && line[*index] <= '9'){
        num[num_index] = line[*index];
        num_index ++;
        *index = *index + 1;
    }
    int num_int = atoi(num); //unsure whether num is garbage value
    free(num);
    return num_int;
}

/*
 * restore header()
 *
 * builds and returns header string
 * Args:  pic struct
 * Returns: char *header with values from pic struct
 *  
 */
char *restore_header(struct pic mypic)
{
    char *header = malloc(255);
    snprintf(header, 255, "P%d %d %d %d ", mypic.magic_num, mypic.width, mypic.
    height, mypic.maxval);
    return header;
}

/*
 * convert to raw
 *
 * converts de-corrupted P2 image to P5 image
 * Args: referenced pic struct
 * Returns: none, but stores new data in struct
 *  
 * ERROR: Valgrind- Conditional jump or move depends on uninitialized value
 */
void convert_to_raw(struct pic *mypic)
{
    /*Gathering data for header*/

    int new_width = 0;
    int old_width = mypic->width;
    
    for(int i = 0; i < old_width; i++){
        char *curr_line = Seq_get(mypic->rem_lines, 0);
        if(curr_line[i] >= '0' && curr_line[i] <= '9'){
            while(curr_line[i] >= '0' && curr_line[i] <= '9'){
                i++;
            }
            new_width ++;
        }
    }
    mypic->width = new_width;
    mypic->height = Seq_length(mypic->rem_lines);
    mypic->magic_num = 5;
    char* header = restore_header(*mypic);
    printf("%s", header);
    free(header);

    /* building raster of image by converting chars to ints and back again*/
    int curr_int;
    int new_index;

    for(int i = 0; i < Seq_length(mypic->rem_lines); i++){
        
        char *curr_line = Seq_get(mypic->rem_lines, i);
        
        new_index = 0;

        for(int j = 0; j<old_width; j++){
            if(curr_line[j] >= '0' && curr_line[j] <= '9'){
                curr_int = get_full_int(curr_line, &j);

                printf("%c", curr_int);
                while(curr_line[j] >= '0' && curr_line[j] <= '9'){
                    j++;
                }


                new_index++;
            }
            
        }
    }
    
}

/*
 * pic free()
 *
 * frees heap-allocated memory
 * Args: referenced pic struct
 * Returns: none, but removes heap-allocated memory
 *  
 * No errors!
 */
void pic_free(struct pic *mypic)
{
    for(int i = 0; i<Seq_length(mypic->lines); i++)
    {
        free(Seq_get(mypic->lines, i));
    }
    Seq_free(&mypic->lines);
    for(int i = 0; i<Seq_length(mypic->sequences); i++)
    {
        free(Seq_get(mypic->sequences, i));
    }
    Seq_free(&mypic->sequences);
    Seq_free(&mypic->rem_lines);
}

/*
 * read_file()
 *
 * called from main, decrypts and prints images in phases
 * Args: opened input file
 *  
 */
void read_file(FILE *inputfd)
{
    char *datapp;
    int size=-1;
    int width = 0;
    struct pic mypic;
    mypic.lines = Seq_new(50);
    mypic.magic_num = 2;
    mypic.maxval=255;

    while(size != 0){
        size = readaline(inputfd, &datapp);
        if(size > width){
            width = size;
        }
        if(size == 0){
            break;
        }
        Seq_addhi(mypic.lines, datapp);
    }
    mypic.width = width;
    mypic.height = Seq_length(mypic.lines);
    determine_sequence(&mypic);
    remove_injections(&mypic);
    restore_whitespace(&mypic);
    convert_to_raw(&mypic);
    pic_free(&mypic);
}

