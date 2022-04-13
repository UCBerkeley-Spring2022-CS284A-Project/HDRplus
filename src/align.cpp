#include <vector>
#include <string>
#include <stdexcept> // std::runtime_error
#include <opencv2/opencv.hpp> // all opencv header
#include "hdrplus/align.h"
#include "hdrplus/burst.h"
#include "hdrplus/utility.h"

namespace hdrplus
{

// static function only visible within file
static void build_image_pyramid( \
    std::vector<cv::Mat>& images_pyramid, \
    const cv::Mat& src_image, \
    const std::vector<int>& inv_scale_factors )
{
    images_pyramid.resize( inv_scale_factors.size() );
    
    for ( int i = 0; i < inv_scale_factors.size(); ++i )
    {
        cv::Mat blur_image;
        cv::Mat downsample_image;

        switch ( inv_scale_factors[ i ] )
        {
        case 1:
            images_pyramid[ images_pyramid.size() - i - 1 ] = src_image;
            // cv::Mat use reference count, will not create deep copy
            downsample_image = src_image;
            break;
        case 2:
            // Gaussian blur
            cv::GaussianBlur( downsample_image, blur_image, cv::Size(0, 0), 1.0 );

            // Downsample
            downsample_image = downsample_nearest_neighbour<uint16_t, 2>( blur_image );

            // Add
            images_pyramid[ images_pyramid.size() - i - 1 ] = downsample_image;

            break;
        case 4:
            cv::GaussianBlur( downsample_image, blur_image, cv::Size(0, 0), 2.0 );
            downsample_image = downsample_nearest_neighbour<uint16_t, 4>( blur_image );
            images_pyramid[ images_pyramid.size() - i - 1 ] = downsample_image;
            break;
        default:
            throw std::runtime_error("inv scale factor " + std::to_string( inv_scale_factors[ i ]) + "invalid" );
        }
    }
}

void align::process( const hdrplus::burst& burst_images, \
                     std::vector<std::vector<std::vector<std::pair<int, int>>>>& aligements )
{
    // Build image pyramid
    std::vector<std::vector<cv::Mat>> grayscale_images_pyramid;
    grayscale_images_pyramid.resize( burst_images.grayscale_images_pad.size() );
    for ( int img_idx = 0; img_idx < burst_images.grayscale_images_pad.size(); ++img_idx )
    {
        build_image_pyramid( grayscale_images_pyramid[ img_idx ], \
                             burst_images.grayscale_images_pad[ img_idx ], \
                             inv_scale_factors );
    }



}

} // namespace hdrplus
