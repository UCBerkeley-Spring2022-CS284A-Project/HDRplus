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
            T box_sum = T( kernel * kernel - 1 );
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

} // namespace hdrplus
