#pragma once

#include <string>
#include <stdexcept> // std::runtime_error
#include <opencv2/opencv.hpp> // all opencv header
// TODO: add openmp support

#if defined(__clang__)
    #define LOOP_UNROLL unroll
#elif defined(__GNUC__) || defined(__GNUG__)
    #define LOOP_UNROLL GCC unroll
#elif defined(_MSC_VER)
    #define LOOP_UNROLL unroll
#endif

namespace hdrplus
{



template <typename T, int kernel>
cv::Mat box_filter_kxk( const cv::Mat& src_image )
{
    const T* src_image_ptr = (T*)src_image.data;
    int src_height = src_image.size().height;
    int src_width  = src_image.size().width;
    int src_step   = src_image.step1();

    if ( kernel <= 0 )
    {
        throw std::runtime_error(std::string( __FILE__ ) + "::" + __func__ + " box filter only support kernel size >= 1");
    }

    //  int(src_height / kernel) = floor(src_height / kernel)
    // When input size is not multiplier of kernel, take floor
    cv::Mat dst_image( src_height / kernel, src_width / kernel, src_image.type() );
    T* dst_image_ptr = (T*)dst_image.data;
    int dst_height = dst_image.size().height;
    int dst_width  = dst_image.size().width; 
    int dst_step = dst_image.step1();

    for ( int row_i = 0; row_i < dst_height; ++row_i )
    {
        for ( int col_i = 0; col_i < dst_width; col_i++ )
        {
            // Take ceiling for rounding
            T box_sum = T( 0 );
            //#pragma LOOP_UNROLL
            for ( int kernel_row_i = 0; kernel_row_i < kernel; ++kernel_row_i )
            {
                //#pragma LOOP_UNROLL
                for ( int kernel_col_i = 0; kernel_col_i < kernel; ++kernel_col_i )
                {
                    box_sum += src_image_ptr[ ( row_i * kernel + kernel_row_i ) * src_step + ( col_i * kernel + kernel_col_i ) ];
                }
            }

            // Average by taking ceiling 
            T box_avg = box_sum / T( kernel * kernel );
            dst_image_ptr[ row_i * dst_step + col_i ] = box_avg;
        }
    }

    return dst_image;
}


template <typename T, int kernel>
cv::Mat downsample_nearest_neighbour( const cv::Mat& src_image )
{
    const T* src_image_ptr = (T*)src_image.data;
    int src_height = src_image.size().height;
    int src_width  = src_image.size().width;
    int src_step   = src_image.step1();

    //  int(src_height / kernel) = floor(src_height / kernel)
    // When input size is not multiplier of kernel, take floor
    cv::Mat dst_image = cv::Mat( src_height / kernel, src_width / kernel, src_image.type() );
    T* dst_image_ptr = (T*)dst_image.data;
    int dst_height = dst_image.size().height;
    int dst_width  = dst_image.size().width; 
    int dst_step = dst_image.step1();

    // -03 should be enough to optimize below code
    for ( int row_i = 0; row_i < dst_height; row_i++ )
    {
        for ( int col_i = 0; col_i < dst_width; col_i++ )
        {
            dst_image_ptr[ row_i * dst_step + col_i ] = \
                src_image_ptr[ (row_i * kernel) * src_step + (col_i * kernel) ];
        }
    }

    return dst_image;
}


template< typename T >
void print_cvmat( cv::Mat image )
{
    const T* img_ptr = (const T*)image.data;
    int height = image.size().height;
    int width = image.size().width;
    int step = image.step1();

    printf("print_cvmat()::Image of size height = %d, width = %d, step = %d\n", \
        height, width, step );

    for ( int row_i = 0; row_i < height; ++row_i )
    {
        int row_i_offset = row_i * step;
        for ( int col_i = 0; col_i < width; ++col_i )
        {
            printf("%3.d ", img_ptr[ row_i_offset + col_i ]);
            //printf("%3.d ", int( image.at<T>( row_i, col_i ) ) );
        }
        printf("\n");
    }
}


/**
 * @brief Extract RGB channel seprately from bayer image
 * 
 * @tparam T data tyoe of bayer image.
 * @return vector of RGB image. OpenCV internally maintain reference count. 
 *      Thus this step won't create deep copy overhead. 
 * 
 * @example extract_rgb_fmom_bayer<uint16_t>( bayer_img, rgb_vector_container );
 */
template <typename T>
void extract_rgb_fmom_bayer( const cv::Mat& bayer_img, \
    cv::Mat& red_img, cv::Mat& green_img1, cv::Mat& green_img2, cv::Mat& blue_img )
{
    const T* bayer_img_ptr = (const T*)bayer_img.data;
    int bayer_width = bayer_img.size().width;
    int bayer_height = bayer_img.size().height;
    int bayer_step = bayer_img.step1();

    if ( bayer_width % 2 != 0 || bayer_height % 2 != 0 )
    {
        throw std::runtime_error("Bayer image data size incorrect, must be multiplier of 2\n");
    }

    // RGB image is half the size of bayer image
    int rgb_width = bayer_width / 2;
    int rgb_height = bayer_height / 2;
    red_img.create( rgb_height, rgb_width, bayer_img.type() );
    green_img1.create( rgb_height, rgb_width, bayer_img.type() );
    green_img2.create( rgb_height, rgb_width, bayer_img.type() );
    blue_img.create( rgb_height, rgb_width, bayer_img.type() );
    int rgb_step = red_img.step1();

    T* r_img_ptr = (T*)red_img.data;
    T* g1_img_ptr = (T*)green_img1.data;
    T* g2_img_ptr = (T*)green_img2.data;
    T* b_img_ptr = (T*)blue_img.data;

    for ( int rgb_row_i = 0; rgb_row_i < rgb_height; rgb_row_i++ )
    {
        int rgb_row_i_offset = rgb_row_i * rgb_step;

        // Every RGB row corresbonding to two Bayer image row
        int bayer_row_i_offset1 = ( rgb_row_i * 2 + 0 ) * bayer_step; // For RG
        int bayer_row_i_offset2 = ( rgb_row_i * 2 + 1 ) * bayer_step; // For GB

        for ( int rgb_col_j = 0; rgb_col_j < rgb_width; rgb_col_j++ )
        {   
            r_img_ptr[  rgb_row_i_offset + rgb_col_j ] = bayer_img_ptr[ bayer_row_i_offset1 + ( rgb_col_j * 2 + 0 ) ];
            g1_img_ptr[ rgb_row_i_offset + rgb_col_j ] = bayer_img_ptr[ bayer_row_i_offset1 + ( rgb_col_j * 2 + 1 ) ];
            g2_img_ptr[ rgb_row_i_offset + rgb_col_j ] = bayer_img_ptr[ bayer_row_i_offset2 + ( rgb_col_j * 2 + 0 ) ];
            b_img_ptr[  rgb_row_i_offset + rgb_col_j ] = bayer_img_ptr[ bayer_row_i_offset2 + ( rgb_col_j * 2 + 1 ) ];
        }
    }
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


} // namespace hdrplus
