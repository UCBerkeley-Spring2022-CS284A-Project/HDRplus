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


// Set tilesize as template argument for better compiler optimization result.
template< typename T, int tile_size >
static T l1_distance( const cv::Mat& img1, const cv::Mat& img2, \
    int img1_tile_row_start_idx, int img1_tile_col_start_idx, \
    int img2_tile_row_start_idx, int img2_tile_col_start_idx )
{
    #define CUSTOME_ABS( x ) ( x ) > 0 ? ( x ) : - ( x )

    const T* img1_ptr = (const T*)img1.data;
    const T* img2_ptr = (const T*)img2.data;

    int img1_step = img1.step1();
    int img2_step = img2.step1();

    int img1_width = img1.size().width;
    int img1_height = img1.size().height;

    int img2_width = img2.size().width;
    int img2_height = img2.size().height;

    // Range check for safety
    if ( img1_tile_row_start_idx < 0 || img1_tile_row_start_idx > img1_height - tile_size )
    {
        throw std::runtime_error("l1 distance img1_tile_row_start_idx out of valid range\n");
    }

    if ( img1_tile_col_start_idx < 0 || img1_tile_col_start_idx > img1_width - tile_size )
    {
        throw std::runtime_error("l1 distance img1_tile_col_start_idx out of valid range\n");
    }

    if ( img2_tile_row_start_idx < 0 || img2_tile_row_start_idx > img2_height - tile_size )
    {
        throw std::runtime_error("l1 distance img2_tile_row_start_idx out of valid range\n");
    }

    if ( img2_tile_col_start_idx < 0 || img2_tile_col_start_idx > img2_width - tile_size )
    {
        throw std::runtime_error("l1 distance img2_tile_col_start_idx out of valid range\n");
    }

    T sum(0);
    // TODO: add pragma unroll here
    for ( int row_i = 0; row_i < tile_size; ++row_i )
    {
        const T* img1_ptr_row_i = img1_ptr + img1_tile_row_start_idx * img1_step + img1_tile_col_start_idx;
        const T* img2_ptr_row_i = img2_ptr + img2_tile_row_start_idx * img2_step + img2_tile_col_start_idx;

        for ( int col_i = 0; col_i < tile_size; ++col_i )
        {
            sum += CUSTOME_ABS( img1_ptr_row_i[ col_i ] - img2_ptr_row_i[ col_i ] );
        }
    }

    #undef CUSTOME_ABS

    return sum;
}


template <typename T>
void print_tile( const cv::Mat& img, int tile_size, int start_idx_row, int start_idx_col )
{
    const T* img_ptr = (T*)img.data;
    int src_height = img.size().height;
    int src_width  = img.size().width;
    int src_step   = img.step1();

    for ( int row = start_idx_row; row < tile_size + start_idx_row; ++row )
    {
        const T* img_ptr_row = img_ptr + row * src_step;
        for ( int col = start_idx_col; col < tile_size + start_idx_col; ++col )
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

    // Every align image level share the same distance function. 
    uint16_t(*)(const cv::Mat&, const cv::Mat&, int, int, int, int) distance_func_ptr = nullptr;

    #ifndef NDEBUG
    printf("%s::%s start: \n", __FILE__, __func__ );
    printf("    scale_factor_prev_curr %d, tile_size %d, prev_tile_size %d, search_radiou %d, distance L%d, \n", \
        scale_factor_prev_curr, tile_size, prev_tile_size, search_radiou, distance );
    printf("    ref img size h=%d w=%d, alt img size h=%d w=%d, \n", \
        ref_img.size().height, ref_img.size().width, alt_img.size().height, alt_img.size().width );
    printf("    num tile h %d, num tile w %d\n", num_tiles_h, num_tiles_w);
    #endif

    printf("Reference image h=%d, w=%d: \n", ref_img.size().height, ref_img.size().width );
    print_img<uint16_t>( ref_img );

    /* Upsample pervious layer alignment */
    std::vector<std::vector<std::pair<int, int>>> upsampled_prev_aligement;

    // Coarsest level
    // prev_alignment is invalid / empty, construct alignment as (0,0)
    if ( prev_tile_size == -1 )
    {
        printf("create empty prev alignment\n");
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

    printf("Alter image pad h=%d, w=%d: \n", alt_img_pad.size().height, alt_img_pad.size().width );
    print_img<uint16_t>( alt_img_pad );

    printf("!! enlarged tile size %d\n", tile_size + 2 * search_radiou );

    int alt_tile_row_idx_max = alt_img_pad.size().height - ( tile_size + 2 * search_radiou );
    int alt_tile_col_idx_max = alt_img_pad.size().width  - ( tile_size + 2 * search_radiou );

    /* Iterate through all reference tile & compute distance */
    for ( int ref_tile_row_i = 0; ref_tile_row_i < num_tiles_h; ref_tile_row_i++ )
    {
        for ( int ref_tile_col_i = 0; ref_tile_col_i < num_tiles_w; ref_tile_col_i++ )
        {
            // Upper left index of reference tile
            int ref_tile_row_start_idx_i = ref_tile_row_i * tile_size / 2;
            int ref_tile_col_start_idx_i = ref_tile_col_i * tile_size / 2;

            printf("Ref img tile [%d, %d] -> start idx [%d, %d] (row, col)\n", \
                ref_tile_row_i, ref_tile_col_i, ref_tile_row_start_idx_i, ref_tile_col_start_idx_i );

            print_tile<uint16_t>( ref_img, 8, ref_tile_row_start_idx_i, ref_tile_col_start_idx_i );

            // Upsampled alignment at this tile
            int prev_alignment_row = upsampled_prev_aligement.at( ref_tile_row_i ).at( ref_tile_col_i ).first;
            int prev_alignment_col = upsampled_prev_aligement.at( ref_tile_row_i ).at( ref_tile_col_i ).second;

            // Alternative image tile start idx
            int alt_tile_row_start_idx_i = ref_tile_row_start_idx_i + prev_alignment_row;
            int alt_tile_col_start_idx_i = ref_tile_col_start_idx_i + prev_alignment_col;

            // Ensure alternative image tile within range
            if ( alt_tile_row_start_idx_i < 0 )
                alt_tile_row_start_idx_i = 0;
            if ( alt_tile_col_start_idx_i < 0 )
                alt_tile_col_start_idx_i = 0;
            if ( alt_tile_row_start_idx_i > alt_tile_row_idx_max )
            {
                int before = alt_tile_row_start_idx_i;
                alt_tile_row_start_idx_i = alt_tile_row_idx_max;
                printf("@@ change start x from %d to %d\n", before, alt_tile_row_idx_max);
            }
            if ( alt_tile_col_start_idx_i > alt_tile_col_idx_max )
            {
                int before = alt_tile_col_start_idx_i;
                alt_tile_col_start_idx_i = alt_tile_col_idx_max;
                printf("@@ change start y from %d to %d\n", before, alt_tile_col_idx_max );
            }

            // Because alternative image is padded with search radious. 
            // Using same coordinate with reference image will automatically considered search radious * 2
            printf("Alt image tile [%d, %d]-> start idx [%d, %d]\n", 
                ref_tile_row_i, ref_tile_col_i, alt_tile_row_start_idx_i, alt_tile_col_start_idx_i );
            print_tile<uint16_t>( alt_img_pad, 16, alt_tile_row_start_idx_i, alt_tile_col_start_idx_i );

            // Search based on L1/L2 distance
            uint16_t min_distance = UINT_LEAST16_MAX;
            int min_distance_row_i = -1;
            int min_distance_col_i = -1;
            for ( int search_row_i = 0; search_row_i < search_radiou * 2; search_row_i++ )
            {
                for ( int search_col_i = 0; search_col_i < search_radiou * 2; search_col_i++ )
                {
                    uint16_t distance_i = l1_distance<uint16_t, 8>( ref_img, alt_img_pad, \
                        ref_tile_row_start_idx_i, ref_tile_col_start_idx_i, \
                        alt_tile_row_start_idx_i, alt_tile_col_start_idx_i );

                    // If this is smaller distance
                    if ( distance_i < min_distance )
                    {
                        min_distance = distance_i;
                        min_distance_col_i = search_col_i;
                        min_distance_row_i = search_row_i;
                    }
                }
            }

            // Add min_distance's corresbonding idx as min

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

    // for ( int level_i; level_i < num_levels; ++level_i )
    // {
    //     printf("level %d img : \n" , level_i );
    //     print_img<uint16_t>( per_grayimg_pyramid[ burst_images.reference_image_idx ][ level_i], 100, 100 );
    // }

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
