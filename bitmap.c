#include <stdio.h>
#include <stdlib.h>
#include "bitmap.h"


/*
 * Read in the location of the pixel array, the image width, and the image 
 * height in the given bitmap file.
 * Use fseek to move the file position. Don't read the whole file.
 */
void read_bitmap_metadata(FILE *image, int *pixel_array_offset, int *width, int *height) {
    //byte 10
    fseek(image, 10, SEEK_SET); 
    fread(pixel_array_offset, 4, 1, image);
     //byte 18
    fseek(image, 18, SEEK_SET);
    fread(width, 4, 1, image);
 //byte 22
    fseek(image, 22, SEEK_SET);
    fread(height, 4, 1, image);
}

/*
 * Read in pixel array by following these instructions:
 *
 * 1. First, allocate space for m `struct pixel *` values, where m is the 
 *    height of the image.  Each pointer will eventually point to one row of 
 *    pixel data.
 * 2. For each pointer you just allocated, initialize it to point to 
 *    heap-allocated space for an entire row of pixel data.
 * 3. Use the given file and pixel_array_offset to fill in the rows of pixels 
 *    you allocated with the image's corresponding pixel values.
 * 4. Hint: Try reading a whole row of pixels in one fread call
 * 5. Return the address of the first `struct pixel *` you initialized.
 *
 * Note: this makes height + 1 calls to malloc. A single malloc of
 * width * height pixels, indexed as pixels[i * width + j], would be
 * more efficient.
 */
struct pixel **read_pixel_array(FILE *image, int pixel_array_offset, int width, int height) {
    struct pixel **pixels = malloc(sizeof(struct pixel *) * height);
    //malloc pointers one per row
    fseek(image, pixel_array_offset, SEEK_SET);

    for (int i = 0; i <  height;  i++) { //full row
        pixels[i] = malloc(sizeof(struct pixel) * width);
        //read the whole row
        fread(pixels[i], sizeof(struct pixel), width, image);
    }

    return pixels;
}


/*
 * Print the blue, green, and red colour values of a pixel.
 * You don't need to change this function.
 */
void print_pixel(struct pixel p) {
    printf("(%u, %u, %u)\n", p.blue, p.green, p.red);
}