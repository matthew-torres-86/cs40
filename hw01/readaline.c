/****************************************************************************** 
                          Homework 1: FilesOfPix                           
               
    readline.c
   
    Authors: Matt Torres (mtorre07). Kim Nguyen (knguye21)

    Summary: This reads in a line from an opened file passed with 
    fp, and returns the contents of that line by updating datapp, which is 
    passed by reference, and returns the size of the line.
 
*******************************************************************************/
#include <readaline.h>
#include <stdlib.h>
#include <assert.h>


/*
 * readaline()
 * Args: Open file fp, pointer to char array datapp
 * Returns: size of line, contents of line
 *  
 * No errors
 */
size_t readaline(FILE *fp, char **datapp){
    if(feof(fp)){
        *datapp = NULL;
        return 0;
    }

    /*ensure file opened properly*/
    assert(fp != NULL);
    assert(datapp != NULL);

    int tocontinue = 1;
    int iteration = 0;
    int bytes = 1000;
    char *line = malloc(bytes);
    do{
        /*read in each character*/
        char c = fgetc(fp);

        /*realloc additional memory if needed*/
        if(iteration >= bytes){
            bytes *= 2;
            line = realloc(line, bytes);
        }

        /*adding newline and moving on for end of line*/
        if(c == '\n' || feof(fp))
        {    
            line[iteration] = '\n';
            tocontinue = 0;
            iteration++;
        }
        /*adding character to line*/
        else{
            line[iteration] = c;
            iteration++;
        }

    }while(tocontinue);

    /*passing line and size back to function*/
    *datapp = line;
    return iteration;
}