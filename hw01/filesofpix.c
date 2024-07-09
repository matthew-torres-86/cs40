/****************************************************************************** 
                          Homework 1: FilesOfPix                           
               
    filesofpix.c
   
    Authors: Matt Torres (mtorre07). Kim Nguyen (knguye21)

    Summary: This is the client function for the restoration interface, which
    takes a file from the command line and opens and prints it. 
 
*******************************************************************************/

#include <restoration.h>
#include <assert.h>

/*
 * main()
 *
 * open file and call functions to decrypt pix
 * Args: from command line
 *  
 * Errors: None
 */
int main(int argc, char **argv)
{
    /* No filename was entered */
    if (argc == 1) {
        printf("You need to enter a filename");
    }    
    else {
        assert(argc == 2);   
        FILE *fp = fopen(argv[1], "r");
        assert(fp != NULL);
       

        read_file(fp);
                        
        }
        return 0;
    }