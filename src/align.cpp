#include <vector>
#include <string>
#include <limits>
#include <cstdio>
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
    #ifndef NDEBUG
    printf("%s::%s build_per_grayimg_pyramid start with scale factor : ", __FILE__, __func__ );
    for ( int i = 0; i < inv_scale_factors.size(); ++i )
    {
        printf("%d ", inv_scale_factors.at( i ));
    }
    printf("\n");
    #endif

    images_pyramid.resize( inv_scale_factors.size() );

    for ( int i = 0; i < inv_scale_factors.size(); ++i )
    {
        cv::Mat blur_image;
        cv::Mat downsample_image;

        switch ( inv_scale_factors[ i ] )
        {
        case 1:
            images_pyramid[ i ] = src_image.clone();
            // cv::Mat use reference count, will not create deep copy
            downsample_image = src_image;
            break;
        case 2:
            // Gaussian blur
            cv::GaussianBlur( images_pyramid.at( i-1 ), blur_image, cv::Size(0, 0), inv_scale_factors[ i ] / 2 );
        
            // Downsample
            downsample_image = downsample_nearest_neighbour<uint16_t, 2>( blur_image );

            // Add
            images_pyramid.at( i ) = downsample_image.clone();

            break;
        case 4:
            cv::GaussianBlur( images_pyramid.at( i-1 ), blur_image, cv::Size(0, 0), inv_scale_factors[ i ] / 2 );
            downsample_image = downsample_nearest_neighbour<uint16_t, 4>( blur_image );
            images_pyramid.at( i ) = downsample_image.clone();
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
            align_i.first *= stride;
            align_i.second *= stride;

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


template <typename T>
void print_tile( const cv::Mat& img, int tile_size, int start_idx_x, int start_idx_y )
{
    const T* img_ptr = (T*)img.data;
    int src_height = img.size().height;
    int src_width  = img.size().width;
    int src_step   = img.step1();

    for ( int row = start_idx_x; row < tile_size + start_idx_x; ++row )
    {
        const T* img_ptr_row = img_ptr + row * src_step;
        for ( int col = start_idx_y; col < tile_size + start_idx_y; ++col )
        {
            printf("%u ", img_ptr_row[ col ] );
        }
        printf("\n");
    }
    printf("\n");
}


template< typename T>
void print_img( const cv::Mat& img, int img_height = -1, int img_width = -1 )
{
    const T* img_ptr = (T*)img.data;
    if ( img_height == -1 && img_width == -1 )
    {
        img_height = img.size().height;
        img_width = img.size().width;
    }
    else
    {
        img_height = std::min( img.size().height, img_height );
        img_width = std::min( img.size().width, img_width );
    }
    printf("Image size (h=%d, w=%d), Print range (h=0-%d, w=0-%d)]\n", \
        img.size().height, img.size().width, img_height, img_width );

    int img_step = img.step1();

    for ( int row = 0; row < img_height; ++row )
    {
        const T* img_ptr_row = img_ptr + row * img_step;
        for ( int col = 0; col < img_width; ++col )
        {
            printf("%u ", img_ptr_row[ col ]);
        }
        printf("\n");
    }
    printf("\n");
}


void align_image_level( \
    const cv::Mat& ref_img, \
    const cv::Mat& alt_img, \
    std::vector<std::vector<std::pair<int, int>>>& prev_aligement, \
    std::vector<std::vector<std::pair<int, int>>>& alignment, \
    int scale_factor_prev_curr, \
    int tile_size, \
    int prev_tile_size, \
    int search_radiou, \
    int distance )
{
    /* Basic infos */
    int num_tiles_h = ref_img.size().height / (tile_size / 2) - 1;
    int num_tiles_w = ref_img.size().width / (tile_size / 2 ) - 1 ;

    #ifndef NDEBUG
    printf("%s::%s start: \n", __FILE__, __func__ );
    printf("    scale_factor_prev_curr %d, tile_size %d, prev_tile_size %d, search_radiou %d, distance L%d, \n", \
        scale_factor_prev_curr, tile_size, prev_tile_size, search_radiou, distance );
    printf("    ref img size h=%d w=%d, alt img size h=%d w=%d, \n", \
        ref_img.size().height, ref_img.size().width, alt_img.size().height, alt_img.size().width );
    printf("    num tile h %d, num tile w %d\n", num_tiles_h, num_tiles_w);
    #endif

    printf("Reference image : \n");
    print_img<uint16_t>( ref_img );

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
            // TODO: add choose from 3 neighbour
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
                        cv::BORDER_CONSTANT, cv::Scalar( UINT_LEAST16_MAX ) );

    /* Iterate through all reference tile & compute distance */
    for ( int ref_tile_row = 0; ref_tile_row < num_tiles_h; ref_tile_row++ )
    {
        for ( int ref_tile_col = 0; ref_tile_col < num_tiles_w; ref_tile_col++ )
        {
            // Upper left index of reference tile
            int ref_tile_idx_x = ref_tile_row * tile_size / 2;
            int ref_tile_idx_y = ref_tile_col * tile_size / 2;

            // Upsampled alignment at this tile
            // int prev_alignment_x = upsampled_prev_aligement.at( ref_tile_row ).at( ref_tile_col ).first;
            // int prev_alignment_y = upsampled_prev_aligement.at( ref_tile_row ).at( ref_tile_col ).second;

            // int alt_tile_idx_x = ref_tile_idx_x + prev_alignment_x;
            // int alt_tile_idx_y = ref_tile_idx_y + prev_alignment_y;

            printf("Ref img tile [%d, %d] -> start [%d, %d]\n", \
                ref_tile_row, ref_tile_col, ref_tile_idx_x, ref_tile_idx_y );

            print_tile<uint16_t>( ref_img, 8, ref_tile_idx_x, ref_tile_idx_y );
        }
    }

}


static void build_per_pyramid_reftiles_start( \
    std::vector<std::vector<std::vector<std::pair<int, int>>>>& per_pyramid_reftiles_start, \
    const std::vector<std::vector<cv::Mat>>& per_grayimg_pyramid, \
    const std::vector<int>& grayimg_tile_sizes )
{
    per_pyramid_reftiles_start.resize( per_grayimg_pyramid.at(0).size() );

    // Every image pyramid level
    for ( int level_i = 0; level_i < per_grayimg_pyramid.at(0).size(); level_i++ )
    {
        int level_i_img_h = per_grayimg_pyramid.at(0).at( level_i ).size().height;
        int level_i_img_w = per_grayimg_pyramid.at(0).at( level_i ).size().width;

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
                    = std::make_pair<int, int>( tile_col_i * level_i_tile_size, tile_row_j * level_i_tile_size );
            }
        }
    }
}


void align::process( const hdrplus::burst& burst_images, \
                     std::vector<std::vector<std::vector<std::pair<int, int>>>>& images_alignment )
{
    #ifndef NDEBUG
    printf("%s::%s align::process start\n", __FILE__, __func__ ); fflush(stdout);
    #endif

    // image pyramid per image, per pyramid level
    std::vector<std::vector<cv::Mat>> per_grayimg_pyramid;

    per_grayimg_pyramid.resize( burst_images.num_images );
    for ( int img_idx = 0; img_idx < burst_images.num_images; ++img_idx )
    {
        // per_grayimg_pyramid[ img_idx ][ 0 ] is the original image
        // per_grayimg_pyramid[ img_idx ][ 3 ] is the coarsest image
        build_per_grayimg_pyramid( per_grayimg_pyramid.at( img_idx ), \
                                   burst_images.grayscale_images_pad.at( img_idx ), \
                                   this->inv_scale_factors );
    }

    #ifndef NDEBUG
    printf("%s::%s build image pyramid of size : ", __FILE__, __func__ );
    for ( int level_i = 0; level_i < num_levels; ++level_i )
    {
        printf("(%d, %d) ", per_grayimg_pyramid[ 0 ][ level_i ].size().height,
                            per_grayimg_pyramid[ 0 ][ level_i ].size().width );
    }
    printf("\n"); fflush(stdout);
    #endif

    for ( int level_i; level_i < num_levels; ++level_i )
    {
        printf("level %d img : \n" , level_i );
        print_img<uint16_t>( per_grayimg_pyramid[ burst_images.reference_image_idx ][ level_i], 100, 100 );
    }

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
        std::vector<std::vector<std::pair<int, int>>> curr_alignment;
        std::vector<std::vector<std::pair<int, int>>> prev_alignment;
        for ( int level_i = num_levels - 1; level_i >= 0; level_i-- )
        {
            align_image_level(
                ref_grayimg_pyramid[ level_i ],    // reference image at current level
                alt_grayimg_pyramid[ level_i ],    // alternative image at current level
                prev_alignment,                    // previous layer alignment
                curr_alignment,                    // current layer alignment
                ( level_i == ( num_levels - 1 ) ? -1 : inv_scale_factors[ level_i ] ),                 // scale factor between previous layer and current layer. -1 if current layer is the coarsest layer
                grayimg_tile_sizes[ level_i ],     // current level tile size
                ( level_i == ( num_levels - 1 ) ? -1 : grayimg_tile_sizes[ level_i + 1 ] ), // previous level tile size
                grayimg_search_radious[ level_i ], // search radious
                distances[ level_i ] );            // L1/L2 distance
           
            // make curr alignment as previous alignment
            prev_alignment.swap( curr_alignment );
            curr_alignment.clear();

            break;
        } // for pyramid level


    } // for alternative image

}

} // namespace hdrplus
