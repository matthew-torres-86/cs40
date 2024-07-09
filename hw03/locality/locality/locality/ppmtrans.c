#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "cputiming.h"
#include "assert.h"
#include "a2methods.h"
#include "a2plain.h"
#include "a2blocked.h"
#include "pnm.h"


void openFile(int i, int argc, char *argv[], FILE **fp);
void rotate_0(int i, int j, A2Methods_UArray2 pixmap, A2Methods_Object *ptr, 
                void *cl);
void rotate_90(int i, int j, A2Methods_UArray2 pixmap, A2Methods_Object *ptr, 
                void *cl);
void rotate_180(int i, int j, A2Methods_UArray2 pixmap, A2Methods_Object *ptr, 
                void *cl);
void rotate_270(int i, int j, A2Methods_UArray2 pixmap, A2Methods_Object *ptr, 
                void *cl);
void command_center(int rotation, A2Methods_T methods, Pnm_ppm *data, A2Methods_mapfun *map);

const int MAX_FILE_SIZE = 256;

struct rotated {
        A2Methods_T methods;
        A2Methods_UArray2 array;
};

#define SET_METHODS(METHODS, MAP, WHAT) do {                    \
        methods = (METHODS);                                    \
        assert(methods != NULL);                                \
        map = methods->MAP;                                     \
        if (map == NULL) {                                      \
                fprintf(stderr, "%s does not support "          \
                                WHAT "mapping\n",               \
                                argv[0]);                       \
                exit(1);                                        \
        }                                                       \
} while (0)

static void
usage(const char *progname)
{
        fprintf(stderr, "Usage: %s [-rotate <angle>] "
                        "[-{row,col,block}-major] [filename]\n",
                        progname);
        exit(1);
}

int main(int argc, char *argv[]) 
{
        char *time_file_name = NULL;
        (void) time_file_name;
        int   rotation       = 0;
        int   i;

        /* default to UArray2 methods */
        A2Methods_T methods = uarray2_methods_plain; 
        assert(methods);

        /* default to best map */
        A2Methods_mapfun *map = methods->map_default; 
        assert(map);

        FILE *fp = NULL;

        if ( argc == 1 ) {
                openFile(0, argc, argv, &fp);
        }

        for (i = 1; i < argc; i++) {
                if (strcmp(argv[i], "-row-major") == 0) {
                        SET_METHODS(uarray2_methods_plain, map_row_major, 
                                    "row-major");
                } else if (strcmp(argv[i], "-col-major") == 0) {
                        SET_METHODS(uarray2_methods_plain, map_col_major, 
                                    "column-major");
                } else if (strcmp(argv[i], "-block-major") == 0) {
                        SET_METHODS(uarray2_methods_blocked, map_block_major,
                                    "block-major");
                } else if (strcmp(argv[i], "-rotate") == 0) {
                        if (!(i + 1 < argc)) {      /* no rotate value */
                                usage(argv[0]);
                        }
                        char *endptr;
                        rotation = strtol(argv[++i], &endptr, 10);
                        if (!(rotation == 0 || rotation == 90 ||
                            rotation == 180 || rotation == 270)) {
                                fprintf(stderr, 
                                        "Rotation must be 0, 90 180 or 270\n");
                                usage(argv[0]);
                        }
                        if (!(*endptr == '\0')) {    /* Not a number */
                                usage(argv[0]);
                        }
                } else if (strcmp(argv[i], "-time") == 0) {
                        time_file_name = argv[++i];
                } else if (*argv[i] == '-') {
                        fprintf(stderr, "%s: unknown option '%s'\n", argv[0],
                                argv[i]);
                        usage(argv[0]);
                } else if (argc - i > 1) {
                        fprintf(stderr, "Too many arguments\n");
                        usage(argv[0]);
                } else {

                        openFile(i, argc, argv, &fp);
                }
        }

        Pnm_ppm data = Pnm_ppmread(fp, methods);

        command_center(rotation, methods, &data, map);
}

void command_center(int rotation, A2Methods_T methods, Pnm_ppm *data, A2Methods_mapfun *map)
{
        Pnm_ppm info = *data;
        A2Methods_UArray2 pixmap = info->pixels;
        A2Methods_UArray2 newArray;

        void (*rotate)(int, int, void *, void *, void *);

        if ( rotation == 90 ) {

                newArray = methods->new(info->height, info->width, 
                                        sizeof(struct Pnm_rgb));

                int width = info->width;
                info->width = info->height;
                info->height = width;

                rotate = (*rotate_90);

        } else if ( rotation == 180 ) {

                newArray = methods->new(info->width, info->height, 
                                        sizeof(struct Pnm_rgb));

                rotate = (*rotate_180);

        } else if ( rotation == 270 ) {

                newArray = methods->new(info->height, info->width, 
                                        sizeof(struct Pnm_rgb));

                int width = info->width;
                info->width = info->height;
                info->height = width;

                rotate = (*rotate_270);

        } else {

                newArray = methods->new(info->width, info->height, 
                                        sizeof(struct Pnm_rgb));

                rotate = (*rotate_0);
        }

        struct rotated *thing = malloc(sizeof(thing));
        thing->methods = methods;
        thing->array = newArray;

        CPUTime_T timer = CPUTime_New();
        CPUTime_Start(timer);

        map(pixmap, (*rotate), thing);

        double time_used = CPUTime_Stop(timer);

        (void) time_used;

        

        info->pixels = thing->array;

        Pnm_ppmwrite(stdout, info);
}

void rotate_0(int i, int j, A2Methods_UArray2 pixmap, A2Methods_Object *ptr, 
                void *cl)
{
        struct rotated *r = cl;

        Pnm_rgb curr = r->methods->at(r->array, i, j);

        assert(curr);

        Pnm_rgb element = ptr;

        *curr = *element;

        (void) pixmap;
}

void rotate_90(int i, int j, A2Methods_UArray2 pixmap, A2Methods_Object *ptr, 
                void *cl)
{
        struct rotated r = *(struct rotated *) cl;

        int height = r.methods->height(pixmap);

        Pnm_rgb curr = r.methods->at(r.array, height - j - 1, i);

        assert(curr);

        Pnm_rgb element = ptr;

        *curr = *element;
}

void rotate_180(int i, int j, A2Methods_UArray2 pixmap, A2Methods_Object *ptr, 
                void *cl)
{
        struct rotated r = *(struct rotated *) cl;

        int width = r.methods->width(pixmap);
        int height = r.methods->height(pixmap);

        Pnm_rgb curr = r.methods->at(r.array, width - i - 1, 
                                        height - j - 1);

        assert(curr);

        Pnm_rgb element = ptr;

        *curr = *element;
}

void rotate_270(int i, int j, A2Methods_UArray2 pixmap, A2Methods_Object *ptr, 
                void *cl)
{
        struct rotated r = *(struct rotated *) cl;

        int width = r.methods->width(pixmap);
        
        Pnm_rgb curr = r.methods->at(r.array, j, width - i - 1);

        assert(curr);

        Pnm_rgb element = ptr;

        *curr = *element;
}

/*
 * openFile
 *    Purpose: uses the given file pointer and opens a command line file or
 *             stdin file with it 
 * Parameters: a pointer to a file pointer, an int argc equal to 1 or 2, and
 *             a pointer to an argv char array with 1 or 2 arguments. file
               argument must be a pgm file
 *    Effects: asserts that the given file can be opened and sets the file 
 *             pointer to the file
 */
void openFile(int i, int argc, char *argv[], FILE **fp)
{
        if ( argc == 1 ) {

                char fileName[MAX_FILE_SIZE];
                fgets(fileName, MAX_FILE_SIZE, stdin);

                /* strcspn removes the \n from the entered fileName */
                fileName[strcspn(fileName, "\n")] = 0;

                assert(*fp = fopen(fileName, "r"));

        } else {
                assert(*fp = fopen(argv[i], "r"));
        }
}
