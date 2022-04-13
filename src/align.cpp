#include <vector>
#include <opencv2/opencv.hpp> // all opencv header
#include "hdrplus/align.h"
#include "hdrplus/burst.h"
#include "hdrplus/utility.h"

namespace hdrplus
{

void align::process( const hdrplus::burst& burst_images, \
                     std::vector<std::vector<std::vector<std::pair<int, int>>>>& aligements )
{
    /*
    // Build image pyramid
    std::vector<std::vector<cv::Mat>> grayscale_images_pyramid;
    grayscale_images_pyramid.resize( burst_images.grayscale_images_pad.size() );
    for ( int img_idx = 0; img_idx < burst_images.grayscale_images_pad.size(); ++img_idx )
    {
        grayscale_images_pyramid[ img_idx ].resize( inv_scale_factors.size() );

        // pyramid image are laied out from coarse to fine
        // level 0 is the original grayscale image 
        grayscale_images_pyramid[ img_idx ][ inv_scale_factors.size() - 1 ] = burst_images.grayscale_images_pad[ img_idx];

        // downsample and gaussian blur
        for ( int i = 1; i < inv_scale_factors.size(); ++i )
        {
            int inv_scale_factor_i = inv_scale_factors[ i ];
            cv::Mat blurred_image = \
                hdrplus::gaussian_blur( grayscale_images_pyramid[ img_idx ][ i - 1 ], inv_scale_factor_i * 0.5 );
            
            // Get around with template
            cv::Mat downsampled_blurred_image;
            if ( inv_scale_factor_i == 2 )
            {
                hdrplus::downsample_nearest_neighbour<uint16_t, 2>( blurred_image );
            }
            else if ( inv_scale_factor_i == 4 )
            {
                hdrplus::downsample_nearest_neighbour<uint16_t, 4>( blurred_image );
            }   

            grayscale_images_pyramid[ img_idx ][ inv_scale_factors.size() - i - 1 ] = downsampled_blurred_image;
        }
    }

    cv::GaussianBlur( src_image, dst_image, cv::Size(0, 0), sigma );

    */
}

} // namespace hdrplus
