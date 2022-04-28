#include <vector>
#include <string>
#include <limits>
#include <stdexcept> // std::runtime_error
#include <opencv2/opencv.hpp> // all opencv header
#include "hdrplus/align.h"
#include "hdrplus/burst.h"
#include "hdrplus/utility.h"

namespace hdrplus
{

// static function only visible within file
static void build_per_grayimg_pyramid( \
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
    std::vector<std::vector<std::pair<int, int>>>& dst_alignment )
{
    int src_height = src_alignment.size();
    int src_width = src_alignment[ 0 ].size();

    int dst_height = src_height * stride;
    int dst_width = src_width * stride;

    // Allocate data for dst_alignment
    dst_alignment.resize( dst_height, std::vector<std::pair<int, int>>( dst_width ) );

    // Upsample alignment
    for ( int row_i = 0; row_i < src_height; row_i++ )
    {
        for ( int col_i = 0; col_i < src_width; col_i++ )
        {
            // Scale alignment
            std::pair<int, int> align_i = src_alignment[ row_i ][ col_i ];
            align_i.first *= scale_factor;
            align_i.second *= scale_factor;

            // repeat
            for ( int stride_row_i = 0; stride_row_i < stride; ++stride_row_i )
            {
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
    const std::vector<std::vector<std::pair<int, int>>>& reftiles_start, \
    std::vector<std::vector<std::pair<int, int>>>& prev_aligement, \
    std::vector<std::vector<std::pair<int, int>>>& alignment, \
    int scale_factor_prev_curr, \
    int tile_size, \
    int prev_tile_size, \
    int search_radiou, \
    int distance )
{
    #ifndef NDEBUG
    printf("%s::%s align_image_level : ", __FILE__, __func__ );
    printf("scale_factor_prev_curr %d, tile_size %d, prev_tile_size %d, search_radiou %d, distance %d", \
        scale_factor_prev_curr, tile_size, prev_tile_size, search_radiou, distance );
    printf("\n");
    #endif

    /* Basic infos */
    int num_tiles_h = reftiles_start.size();
    int num_tiles_w = reftiles_start.at( 0 ).size();

    /* Upsample pervious layer alignment */
    std::vector<std::vector<std::pair<int, int>>> upsampled_prev_aligement;

    // Coarsest level
    // prev_alignment is invalid / empty, construct alignment as (0,0)
    if ( prev_tile_size == -1 )
    {
        upsampled_prev_aligement.resize( num_tiles_h, std::vector<std::pair<int, int>>( num_tiles_w, std::pair<int, int>(0, 0) ) );
    }
    // Upsample previous level alignment 
    else
    {
        if ( scale_factor_prev_curr == 2 )
        {
            // TODO: add choose from 3 neighbour
            upsample_alignment_stride<2>( prev_aligement, upsampled_prev_aligement );
        }
        else if ( scale_factor_prev_curr == 4 )
        {
            upsample_alignment_stride<4>( prev_aligement, upsampled_prev_aligement );
        }
        else
        {
            throw std::runtime_error("Invalid scale factor" + std::to_string( scale_factor_prev_curr ) );
        }
    }

    /* Pad alternative image */
    cv::Mat alt_img_pad;
    cv::copyMakeBorder( alt_img, \
                        alt_img_pad, \
                        search_radiou, search_radiou, search_radiou, search_radiou, \
                        cv::BORDER_CONSTANT, std::numeric_limits<char16_t>::max );

    /* Iterate through all reference tile & compute distance */
    for ( int ref_tile_row = 0; ref_tile_row < num_tile_h; ref_tile_row++ )
    {
        for ( int ref_tile_col = 0; ref_tile_col < num_tile_w; ref_tile_col++ )
        {
            // Upper left index of reference tile
            int ref_tile_idx_x = reftiles_start.at( ref_tile_row ).at( ref_tile_col );
            int ref_tile_idx_y = reftiles_start.at( ref_tile_row ).at( ref_tile_col );

            // Upsampled alignment at this tile
            int 
        }
    }

}


static void build_per_pyramid_reftiles_start( \
    std::vector<std::vector<std::vector<std::pair<int, int>>>>& per_pyramid_reftiles_start, \
    const std::vector<std::vector<cv::Mat>>& per_grayimg_pyramid, \
    const std::vector<int>& grayimg_tile_sizes )
{
    per_pyramid_reftiles_start.resize( per_grayimg_pyramid.size() );

    // Every image pyramid level
    for ( int level_i = 0; level_i < per_grayimg_pyramid.size(); level_i++ )
    {
        int level_i_img_h = per_grayimg_pyramid.at( level_i ).size().height;
        int level_i_img_w = per_grayimg_pyramid.at( level_i ).size().width;

        int level_i_tile_size = grayimg_tile_sizes.at( level_i );

        int num_tiles_h = level_i_img_h / (level_i_tile_size / 2) - 1;
        int num_tiles_w = level_i_img_w / (level_i_tile_size / 2) - 1;

        // Allocate memory
        per_pyramid_reftiles_start.at( level_i ).resize( num_tiles_h, std::vector<std::pair<int, int>>( num_tiles_w ) );

        for ( int tile_col_i = 0; tile_col_i < num_tiles_h; tile_col_i++ )
        {
            for ( int tile_row_j = 0; tile_row_j < num_tiles_w; tile_row_j++ )
            {
                per_pyramid_reftiles_start.at( level_i ).at( tile_col_i ).at( tile_row_j ) \
                    = std::make_pair<int, int>( tile_col_i * level_i_tile_size, tile_col_j * level_i_tile_size );
            }
        }
    }
}


void align::process( const hdrplus::burst& burst_images, \
                     std::vector<std::vector<std::vector<std::pair<int, int>>>>& images_alignment )
{
    // image pyramid per image, per pyramid level
    std::vector<std::vector<cv::Mat>> per_grayimg_pyramid;

    per_grayimg_pyramid.resize( burst_images.num_images );
    for ( int img_idx = 0; img_idx < burst_images.num_images; ++img_idx )
    {
        // per_grayimg_pyramid[ img_idx ][ 0 ] is the original image
        // per_grayimg_pyramid[ img_idx ][ 3 ] is the coarsest image
        build_per_grayimg_pyramid( per_grayimg_pyramid[ img_idx ], \
                                   burst_images.grayscale_images_pad[ img_idx ], \
                                   inv_scale_factors );
    }

    #ifndef NDEBUG
    printf("%s::%s build image pyramid of size : ", __FILE__, __func__ );
    for ( int level_i = 0; level_i < num_levels; ++level_i )
    {
        printf("(%d, %d) ", per_grayimg_pyramid[ 0 ][ level_i ].size().height,
                            per_grayimg_pyramid[ 0 ][ level_i ].size().width );
    }
    printf("\n");
    #endif


    // Tile starting location for each tile level
    std::vector<std::vector<std::vector<std::pair<int, int>>>> per_pyramid_reftiles_start;

    build_per_pyramid_reftiles_start( \
        per_pyramid_reftiles_start, \
        per_grayimg_pyramid, \
        grayimg_tile_sizes );

    // Align every image
    const std::vector<cv::Mat>& ref_grayimg_pyramid = per_grayimg_pyramid[ burst_images.reference_image_idx ];
    for ( int img_idx = 0; img_idx < burst_images.num_images; ++img_idx )
    {
        // Do not align with reference image
        if ( img_idx == burst_images.reference_image_idx )
            continue;

        const std::vector<cv::Mat>& alt_grayimg_pyramid = per_grayimg_pyramid[ img_idx ];

        // Align every level from coarse to grain
        // level 0 : finest level, the original image
        // level 3 : coarsest level
        std::vector<std::pair<int, int>> curr_alignment;
        std::vector<std::pair<int, int>> prev_alignment;
        for ( int level_i = num_levels - 1; level_i >= 0; level_i-- )
        {
            align_image_level(
                ref_grayimg_pyramid[ level_i ],    // reference image at current level
                alt_grayimg_pyramid[ level_i ],    // alternative image at current level
                per_pyramid_reftiles_start[ level_i ] // reference tile start location for current level
                prev_alignment,                    // previous layer alignment
                curr_alignment,                    // current layer alignment
                ( level_i == ( num_levels - 1 ) ? -1 : inv_scale_factors[ level_i ] ),                 // scale factor between previous layer and current layer. -1 if current layer is the coarsest layer
                grayimg_tile_sizes[ level_i ],     // current level tile size
                ( level_i == ( num_levels - 1 ) ? -1 : tile_sizes[ level_i + 1 ] ), // previous level tile size
                grayimg_search_radious[ level_i ], // search radious
                distances[ level_i ] );            // L1/L2 distance
           
            // make curr alignment as previous alignment
            prev_alignment.swap( curr_alignment );
            curr_alignment.clear();
        } // for pyramid level


    } // for alternative image

}

} // namespace hdrplus
