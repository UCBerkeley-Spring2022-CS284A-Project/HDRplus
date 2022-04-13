#include <cstdio>
#include <string>
#include "hdrplus/burst.h"

int main( int argc, char** argv )
{
    if ( argc != 3 )
    {
        printf("Usage: ./test_burst BURST_FOLDER_PATH(no / at end) REFERENCE_IMAGE_PATH\n");
        exit(1);
    }

    hdrplus::burst burst_image( argv[1], argv[2] );
    printf("number of image in burst %d\n", burst_image.bayer_images.size() );
    printf("grayscale image shape (h=%d, w=%d)\n", \
        burst_image.bayer_images[ 0 ].grayscale_image.size().height,
        burst_image.bayer_images[ 0 ].grayscale_image.size().width );
    printf("grayscale image pad shape (h=%d, w=%d)\n", 
        burst_image.grayscale_images_pad[ 0 ].size().height, \
        burst_image.grayscale_images_pad[ 0 ].size().width );
}