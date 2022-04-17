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

template <typename T>
cv::Mat box_filter_2x2( const cv::Mat& src_image )
{
    // https://stackoverflow.com/questions/34042112/opencv-mat-data-member-access
    const T* src_image_ptr = (T*)src_image.data;
    int src_height = src_image.size().height;
    int src_width  = src_image.size().width;
    int src_step   = src_image.step1();

    if ( src_height % 2 != 0 || src_width % 2 != 0 )
    {
        throw std::runtime_error( std::string( __FILE__ ) + "::" + __func__ + " source image need to have size multiplier of 2\n" );
    }

    cv::Mat dst_image( src_height / 2, src_width / 2, src_image.type() );
    T* dst_image_ptr = (T*)dst_image.data;
    int dst_step = dst_image.step1();

    // -03 should be enough to optimize below code
    for ( int row_i = 0; row_i < src_height; row_i += 2 )
    {
        for ( int col_i = 0; col_i < src_width; col_i += 2 )
        {
            T box_sum = src_image_ptr[ ( row_i + 0 ) * src_step + col_i + 0 ] + \
                        src_image_ptr[ ( row_i + 0 ) * src_step + col_i + 1 ] + \
                        src_image_ptr[ ( row_i + 1 ) * src_step + col_i + 0 ] + \
                        src_image_ptr[ ( row_i + 1 ) * src_step + col_i + 1 ];
            T box_avg = ( box_sum + T(3) ) / 4; // take ceiling
            dst_image_ptr[ ( row_i / 2 ) * dst_step + ( col_i / 2 ) ] = box_avg;
        }
    }

    // cv::Mat internally use reference count. Will not copy by value here
    return dst_image;
}

template <typename T, int kernel>
cv::Mat box_filter_kxk( cv::Mat src_image )
{
    const T* src_image_ptr = (T*)src_image.data;
    int src_height = src_image.size().height;
    int src_width  = src_image.size().width;
    int src_step   = src_image.step1();

    if ( src_height % kernel != 0 || src_width % kernel != 0 )
    {
        throw std::runtime_error( std::string( __FILE__ ) + "::" + __func__ + " source image need to have size multiplier of kernel\n" );
    }

    cv::Mat dst_image( src_height / kernel, src_width / kernel, src_image.type() );
    T* dst_image_ptr = (T*)dst_image.data;
    int dst_step = dst_image.step1();

    // -03 should be enough to optimize below code
    for ( int row_i = 0; row_i < src_height; row_i += kernel )
    {
        for ( int col_i = 0; col_i < src_width; col_i += kernel )
        {
            T box_sum = T(0);

            #pragma LOOP_UNROLL
            for ( int kernel_row_i = 0; kernel_row_i < kernel; ++kernel_row_i )
            {
                #pragma LOOP_UNROLL
                for ( int kernel_col_i = 0; kernel_col_i < kernel; ++kernel_col_i )
                {
                    box_sum += src_image_ptr[ ( row_i + kernel_row_i ) * src_step + ( col_i + kernel_col_i ) ];
                }
            }

            T box_avg = ( box_sum + T( kernel * kernel - 1 ) ) / ( kernel * kernel ); // take ceiling
            dst_image_ptr[ ( row_i / kernel ) * dst_step + ( col_i / kernel ) ] = box_avg;
        }
    }

    // cv::Mat internally use reference count. Will not copy by value here
    return dst_image;
}


template <typename T, int kernel>
cv::Mat downsample_nearest_neighbour( cv::Mat src_image )
{
    const T* src_image_ptr = (T*)src_image.data;
    int src_height = src_image.size().height;
    int src_width  = src_image.size().width;
    int src_step   = src_image.step1();

    if ( src_height % kernel != 0 || src_width % kernel != 0 )
    {
        throw std::runtime_error( std::string( __FILE__ ) + "::" + __func__ + " source image need to have size multiplier of kernel size\n" );
    }

    cv::Mat dst_image( src_height / kernel, src_width / kernel, src_image.type() );
    T* dst_image_ptr = (T*)dst_image.data;
    int dst_step = dst_image.step1();

    // -03 should be enough to optimize below code
    for ( int row_i = 0; row_i < src_height; row_i += kernel )
    {
        for ( int col_i = 0; col_i < src_width; col_i += kernel )
        {
            dst_image_ptr[ ( row_i / kernel ) * dst_step + ( col_i / kernel ) ] = \
                src_image_ptr[ row_i * src_step + col_i ];
        }
    }

    // cv::Mat internally use reference count. Will not copy by value here
    return dst_image;
}

} // namespace hdrplus
