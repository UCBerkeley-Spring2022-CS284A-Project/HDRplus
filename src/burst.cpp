#include <cstdio>
#include <string>
#include <opencv2/opencv.hpp> // all opencv header
#include "hdrplus/burst.h"

namespace hdrplus
{

burst::burst( const std::string& burst_path, const std::string& reference_image_path ) 
    : reference_image_path( reference_image_path ), burst_path( burst_path )
{
    // Search through the input path directory to get all input image path
    cv::glob( burst_path + "/*.dng", bayer_image_paths, false );

    // Number of images
    num_images = bayer_image_paths.size();

    // Find reference image path in input directory
    // reference image path need to be absolute path
    reference_image_idx = -1;
    for ( int i = 0; i < bayer_image_paths.size(); ++i )
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

    // Pad gray scale image
    int pad_height = bayer_images[ 0 ].height % 32;
    int pad_width  = bayer_images[ 0 ].width  % 32;
    // https://docs.opencv.org/3.4/dc/da3/tutorial_copyMakeBorder.html
    for ( const auto& bayer_image_i : bayer_images )
    {
        cv::Mat grayscale_image_pad_i;
        cv::copyMakeBorder( bayer_image_i.grayscale_image, \
                            grayscale_image_pad_i, \
                            0, pad_height, 0, pad_width, \
                            cv::BORDER_REFLECT );
        // cv::Mat use internal reference count
        grayscale_images_pad.emplace_back( grayscale_image_pad_i );
    } 

    #ifndef NDEBUG
    printf("%s::%s Pad image from (%d, %d) -> (%d, %d)\n", \
        __FILE__, __func__, \
        bayer_images[ 0 ].grayscale_image.size().height, \
        bayer_images[ 0 ].grayscale_image.size().width, \
        grayscale_images_pad[ 0 ].size().height, \
        grayscale_images_pad[ 0 ].size().width );
    #endif
}

} // namespace hdrplus
