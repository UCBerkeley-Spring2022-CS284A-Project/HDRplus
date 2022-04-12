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

int main()
{
    test_box_filter_2x2();
}