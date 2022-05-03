#include <cstdio>
#include "hdrplus/align.h"
#include "hdrplus/bayer_image.h"
#include "hdrplus/burst.h"
#include "hdrplus/utility.h"

void test_align_one_level(int argc, char** argv)
{
    if ( argc != 3 )
    {
        printf("Usage ./test_align BUTST_PATH REF_PATH");
        exit(-1);
    }

    printf("Burst img dir %s\n", argv[1]);
    printf("Ref img path %s\n", argv[2]);

    hdrplus::burst burst_images( argv[1], argv[2] );
    std::vector<std::vector<std::vector<std::pair<int, int>>>> alignments;

    hdrplus::align align_module;
    align_module.process( burst_images, alignments );

    // Access alternative image tile in each channel
    // Below code can be use in merging part
    for ( int img_idx = 0; img_idx < burst_images.num_images; ++img_idx )
    {
        if ( img_idx == burst_images.reference_image_idx )
        {
            continue;
        }

        //const auto& grayscale_image_pad = burst_images.grayscale_images_pad.at( img_idx );
        const auto& bayer_image_pad = burst_images.bayer_images_pad.at( img_idx );
        const auto& alignment = alignments.at( img_idx );

        // Create RGB channel
        std::vector<cv::Mat> rggb_imgs( 4 );
        hdrplus::extract_rgb_fmom_bayer<uint16_t>( bayer_image_pad, rggb_imgs.at(0), rggb_imgs.at(1), rggb_imgs.at(2), rggb_imgs.at(3) );

        // Get tile of each channel with the alignments
        int tilesize = 16; // tile size of grayscale image
        int num_tiles_h = rggb_imgs.at(0).size().height / ( tilesize / 2 ) - 1;
        int num_tiles_w = rggb_imgs.at(0).size().width / ( tilesize / 2 ) - 1;

        for ( int img_channel = 0; img_channel < rggb_imgs.size(); ++img_channel )
        {
            for ( int tile_row_i = 0; tile_row_i < num_tiles_h; ++tile_row_i )
            {
                for ( int tile_col_i = 0; tile_col_i < num_tiles_w; ++tile_col_i )
                {
                    int ref_tile_row_start_idx_i = tile_row_i * tilesize / 2;
                    int ref_tile_col_start_idx_i = tile_col_i * tilesize / 2;

                    int alignment_row_i = alignment.at( tile_row_i ).at( tile_col_i ).first;
                    int alignment_col_i = alignment.at( tile_row_i ).at( tile_col_i ).second;

                    // Alternative image tile i (tile_row_i, tile_col_i) left top pixel index (tile start location)
                    int alt_tile_row_start_idx_i = ref_tile_row_start_idx_i + alignment_row_i;
                    int alt_tile_col_start_idx_i = ref_tile_col_start_idx_i + alignment_col_i;

                    printf("\nAlt img align channel %d tile [%d, %d]\n", img_channel, tile_row_i, tile_col_i );
                    hdrplus::print_tile<uint16_t>( rggb_imgs.at( img_channel ), tilesize, alt_tile_row_start_idx_i, alt_tile_col_start_idx_i );
                }
            }
        }
    }

} // end of test_align_one_level


int main(int argc, char** argv)
{
    test_align_one_level(argc, argv);
}