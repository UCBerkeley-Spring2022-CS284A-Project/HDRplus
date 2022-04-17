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


template< int stride >
static void upsample_alignment_stride( \
    std::vector<std::vector<std::pair<int, int>>>& src_alignment, \
    std::vector<std::vector<std::pair<int, int>>>& dst_alignment,
    int scale_factor )
{
    int src_height = src_alignment.size();
    int src_width = src_alignment[ 0 ].size();

    int dst_height = src_height * stride;
    int dst_width = src_width * stride;

    // Allocate data for dst_alignment
    dst_alignment.resize( dst_height, std::vector<std::pair<int, int>>( dst_width ) );

    for ( int row_i = 0; row_i < src_height; row_i++ )
    {
        for ( int col_i = 0; col_i < src_width; col_i++ )
        {
            // Scale alignment
            std::pair<int, int> align_i = src_alignment[ row_i ][ col_i ];
            align_i.first *= scale_factor;
            align_i.second *= scale_factor;

            // repeat
            #pragma LOOP_UNROLL
            for ( int stride_row_i = 0; stride_row_i < stride; ++stride_row_i )
            {
                #pragma LOOP_UNROLL
                for ( int stride_col_i = 0; stride_col_i < stride; ++stride_col_i )
                {
                    dst_alignment[ row_i + stride_row_i ][ col_i + stride_col_i ] = align_i;
                }
            }
        }
    }
}


static void align_image_level( \
    const cv::Mat& ref_img, \
    const cv::Mat& alt_img, \
    const std::pair<int, int>& num_tiles, \
    std::vector<std::vector<std::pair<int, int>>>& prev_aligement, \
    std::vector<std::vector<std::pair<int, int>>>& alignment, \
    int inv_scale_factor, \
    int tile_size, \
    int prev_tile_size, \
    int search_radiou, \
    int distance )
{

    // Upsample pervious layer alignment

    // Coarsest level
    // prev_alignment is invalid / empty, construct alignment as (0,0)
    if ( prev_tile_size == -1 )
    {
        alignment.resize( num_tiles.first, std::vector<std::pair<int, int>>( num_tiles.second, std::pair<int, int>(0, 0) ) );
    }
    // Upsample previous level alignment 
    else
    {
        if ( inv_scale_factor == 2 )
        {
            upsample_alignment_stride<2>( prev_aligement, alignment, inv_scale_factor );
        }
        else if ( inv_scale_factor == 4 )
        {
            upsample_alignment_stride<4>( prev_aligement, alignment, inv_scale_factor );
        }
        else
        {
            throw std::runtime_error("Invalid scale factor" + std::to_string( inv_scale_factor ) );
        }
    }


}

void align::process( const hdrplus::burst& burst_images, \
                     std::vector<std::vector<std::vector<std::pair<int, int>>>>& images_alignment )
{
    // Build image pyramid

    // image pyramid per image, per pyramid level
    std::vector<std::vector<cv::Mat>> grayscale_images_pyramid;

    grayscale_images_pyramid.resize( burst_images.num_images );
    for ( int img_idx = 0; img_idx < burst_images.num_images; ++img_idx )
    {
        build_image_pyramid( grayscale_images_pyramid[ img_idx ], \
                             burst_images.grayscale_images_pad[ img_idx ], \
                             inv_scale_factors );
    }

    #ifndef NDEBUG
    printf("%s::%s build image pyramid of size : ", __FILE__, __func__ );
    for ( int level_i = 0; level_i < num_levels; ++level_i )
    {
        printf("(%d, %d) ", grayscale_images_pyramid[ 0 ][ level_i ].size().height,
                            grayscale_images_pyramid[ 0 ][ level_i ].size().width );
    }
    printf("\n");
    #endif

    // number of tiles per pyramid level
    // this is shared across all image at particular level
    // `num_tiles_pyramid[0]` represent number of tiles in level 0 (finest level, original image)
    std::vector<std::pair<int, int>> num_tiles_pyramid( num_levels );

    for ( int level_i = 0; level_i < num_levels; ++level_i )
    {
        cv::Size image_size_level_i = grayscale_images_pyramid[ 0 ][ level_i ].size();
        int half_tile_size_level_i = tile_sizes[ level_i ] / 2;

        num_tiles_pyramid[ level_i ] = std::make_pair<int, int>( \
            image_size_level_i.height / half_tile_size_level_i - 1, \
            image_size_level_i.width / half_tile_size_level_i - 1 );
    }

    #ifndef NDEBUG
    printf("%s::%s each pyramid tile size : ", __FILE__, __func__ );
    for ( int level_i = 0; level_i < num_levels; ++level_i )
    {
        printf("(%d, %d) ", num_tiles_pyramid[ level_i ].first,
                            num_tiles_pyramid[ level_i ].second );
    }
    printf("\n");
    #endif

    // Align every image
    const std::vector<cv::Mat>& ref_imgs_pyramid = grayscale_images_pyramid[ burst_images.reference_image_idx ];
    for ( int img_idx = 0; img_idx < burst_images.num_images; ++img_idx )
    {
        // Do not align with reference image
        if ( img_idx == burst_images.reference_image_idx )
            continue;

        const std::vector<cv::Mat>& alt_imgs_pyramid = grayscale_images_pyramid[ img_idx ];

        // Align every level from coarse to grain
        // level 0 : finest level, the original image
        // level 3 : coarsest level
        std::vector<std::pair<int, int>> curr_alignment;
        std::vector<std::pair<int, int>> prev_alignment;
        for ( int level_i = num_levels - 1; level_i >= 0; level_i-- )
        {
            /*
            align_image_level(
                ref_imgs_pyramid[ level_i ], // reference image at current level
                alt_imgs_pyramid[ level_i ], // alternative image at current level
                num_tiles_pyramid[ level_i ], // number of tiles at this level
                prev_alignment, // previous layer alignment
                curr_alignment, // current layer alignment
                inv_scale_factors[ level_i ], // ?
                tile_sizes[ level_i ],       // current level tile size
                ( level_i == ( num_levels - 1 ) ? -1 : tile_sizes[ level_i + 1] ), // previous level tile size
                search_radious[ level_i ], // search radious
                distances[ level_i ] ); // L1/L2 distance
            */
           
            // make curr alignment as previous alignment
            prev_alignment.swap( curr_alignment );
            curr_alignment.clear();
        } // for pyramid level


    } // for alternative image

}

} // namespace hdrplus
