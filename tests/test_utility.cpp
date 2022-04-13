#include <cstdio>
#include <vector>
#include <opencv2/opencv.hpp>
#include "hdrplus/utility.h"

template< typename T >
void print_cvmat( cv::Mat image )
{
    int height = image.size().height;
    int width = image.size().width;

    for ( int row_i = 0; row_i < height; ++row_i )
    {
        for ( int col_i = 0; col_i < width; ++col_i )
        {
            printf("%2.d ", int( image.at<T>( row_i, col_i ) ) );
        }
        printf("\n");
    }
}

void test_box_filter_2x2()
{
    printf("\n###Test test_box_filter_2x2()###\n");
    // Intialize input data
    int src_width = 10;
    int src_height = 6;
    std::vector<uint16_t> src_data( src_width, src_height );

    for ( int i = 0; i < src_width * src_height; ++i )
    {
        src_data[ i ] = i+1;
    }

    // Create input cv::mat
    cv::Mat src_image( src_height, src_width, CV_16U, src_data.data() );

    printf("src cv::Mat is \n");
    print_cvmat<uint16_t>( src_image );

    cv::Mat dst_image = hdrplus::box_filter_2x2<uint16_t>( src_image );

    printf("dst cv::Mat is \n");
    print_cvmat<uint16_t>( dst_image );
}

void test_box_filter_kxk()
{
    printf("\n###Test test_box_filter_kxk()###\n");
    // Intialize input data
    int src_width = 12;
    int src_height = 8;
    std::vector<uint16_t> src_data( src_width, src_height );

    for ( int i = 0; i < src_width * src_height; ++i )
    {
        src_data[ i ] = i+1;
    }

    // Create input cv::mat
    cv::Mat src_image( src_height, src_width, CV_16U, src_data.data() );

    printf("src cv::Mat is \n");
    print_cvmat<uint16_t>( src_image );

    cv::Mat dst_image = hdrplus::box_filter_kxk<uint16_t, 2>( src_image );

    printf("dst cv::Mat 2x2 is \n");
    print_cvmat<uint16_t>( dst_image );

    dst_image = hdrplus::box_filter_kxk<uint16_t, 4>( src_image );

    printf("dst cv::Mat 4x4 is \n");
    print_cvmat<uint16_t>( dst_image );
}


int main()
{
    test_box_filter_2x2();
    test_box_filter_kxk();
}