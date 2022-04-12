#include <cstdio>
#include "hdrplus/bayer_image.h"

int main( int argc, char** argv )
{
    if ( argc != 2 )
    {
        printf("Usage: ./test_bayer_image RAW_IMAGE_PATH\n");
        exit( -1 );
    }

    hdrplus::bayer_image raw_bayer_image( argv[1] );

    printf("Image of shape h=%d w=%d\n", raw_bayer_image.height, raw_bayer_image.width );
}