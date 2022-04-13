#include "hdrplus/hdrplus_pipeline.h"

int main( int argc, char** argv )
{
    if ( argc != 3 )
    {
        printf("Usage: ./demo BURST_FOLDER_PATH(no / at end) REFERENCE_IMAGE_PATH\n");
        exit(1);
    }

    hdrplus::hdrplus_pipeline pipeline;
    pipeline.run_pipeline( argv[1], argv[ 2 ]);
}