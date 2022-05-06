#include <cstdio>
#include <string>
#include <omp.h>
#include <opencv2/opencv.hpp> // all opencv header
#include "hdrplus/burst.h"
#include "hdrplus/utility.h"

namespace hdrplus
{

burst::burst( const std::string& burst_path, const std::string& reference_image_path ) 
{
    std::vector<cv::String> bayer_image_paths;
    // Search through the input path directory to get all input image path
    if ( burst_path.at( burst_path.size() - 1) == '/')
        cv::glob( burst_path + "*.dng", bayer_image_paths, false );
    else
        cv::glob( burst_path + "/*.dng", bayer_image_paths, false );

    #ifndef NDEBUG
    for ( const auto& bayer_img_path_i : bayer_image_paths )
    {
        printf("img i path %s\n", bayer_img_path_i.c_str()); fflush(stdout);
    }
    printf("ref img path %s\n", reference_image_path.c_str()); fflush(stdout);
    #endif

    // Number of images
    num_images = bayer_image_paths.size();

    // Find reference image path in input directory
    // reference image path need to be absolute path
    reference_image_idx = -1;
    for ( size_t i = 0; i < bayer_image_paths.size(); ++i )
    {
        if ( bayer_image_paths[ i ] == reference_image_path )
        {
            reference_image_idx = i;
        }
    }

    if ( reference_image_idx == -1 )
    {
        throw std::runtime_error("Error unable to locate reference image " + reference_image_path );
    }

    #ifndef NDEBUG
    for ( const auto& bayer_image_path_i : bayer_image_paths )
    {
        printf("%s::%s Find image %s\n", \
            __FILE__, __func__, bayer_image_path_i.c_str());
    }

    printf("%s::%s reference image idx %d\n", \
        __FILE__, __func__, reference_image_idx );
    #endif

    // Get source bayer image
    // Downsample original bayer image by 2x2 box filter
    for ( const auto& bayer_image_path_i : bayer_image_paths )
    {
        bayer_images.emplace_back( bayer_image_path_i );
    }

    // Pad information
    int tile_size_bayer = 32;
    int padding_top = tile_size_bayer / 2;
    int padding_bottom = tile_size_bayer / 2 + \
        ( (bayer_images[ 0 ].height % tile_size_bayer) == 0 ? \
        0 : tile_size_bayer - bayer_images[ 0 ].height % tile_size_bayer );
    int padding_left = tile_size_bayer / 2;
    int padding_right = tile_size_bayer / 2 + \
        ( (bayer_images[ 0 ].width % tile_size_bayer) == 0 ? \
        0 : tile_size_bayer - bayer_images[ 0 ].width % tile_size_bayer );
    padding_info_bayer = std::vector<int>{ padding_top, padding_bottom, padding_left, padding_right };

    // Pad bayer image
    bayer_images_pad.resize( bayer_images.size() );
    grayscale_images_pad.resize( bayer_images.size() );

    #pragma omp parallel for
    for ( size_t img_i = 0; img_i < bayer_images.size(); ++img_i )
    {
        cv::Mat bayer_image_pad_i;
        cv::copyMakeBorder( bayer_images.at( img_i ).raw_image, \
                            bayer_image_pad_i, \
                            padding_top, padding_bottom, padding_left, padding_right, \
                            cv::BORDER_REFLECT );
        
        bayer_images_pad.at( img_i ) = bayer_image_pad_i;
        grayscale_images_pad.at( img_i ) = box_filter_kxk<uint16_t, 2>( bayer_image_pad_i );
    }

    #ifndef NDEBUG
    printf("%s::%s Pad bayer image from (%d, %d) -> (%d, %d)\n", \
        __FILE__, __func__, \
        bayer_images[ 0 ].height, \
        bayer_images[ 0 ].width, \
        bayer_images_pad[ 0 ].size().height, \
        bayer_images_pad[ 0 ].size().width );
    printf("%s::%s pad top %d, buttom %d, left %d, right %d\n", \
        __FILE__, __func__, \
        padding_top, padding_bottom, padding_left, padding_right );
    #endif
}

} // namespace hdrplus
