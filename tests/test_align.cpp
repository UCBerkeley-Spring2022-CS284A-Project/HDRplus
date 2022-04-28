#include "hdrplus/align.h"
#include "hdrplus/bayer_image.h"
#include "hdrplus/burst.h"
#include <cstdio>

void test_align_one_level(int argc, char** argv)
{
    if ( argc != 3 )
    {
        printf("Usage ./test_align BUTST_PATH REF_PATH");
    }
    hdrplus::burst burst_images( argv[1], argv[2] );
    std::vector<std::vector<std::vector<std::pair<int, int>>>> alignments;

    hdrplus::align align_module;
    align_module.process( burst_images, alignments );

}


int main(int argc, char** argv)
{
    test_align_one_level(argc, argv);
}