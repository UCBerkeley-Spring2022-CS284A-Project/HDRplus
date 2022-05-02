#include <cstdio>
#include <vector>
#include <cstdint>
#include <opencv2/opencv.hpp>
#include "hdrplus/utility.h"

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
    hdrplus::print_cvmat<uint16_t>( src_image );

    cv::Mat dst_image = hdrplus::box_filter_2x2<uint16_t>( src_image );

    printf("dst cv::Mat is \n");
    hdrplus::print_cvmat<uint16_t>( dst_image );

    printf("test_box_filter_2x2 finish\n"); fflush(stdout);
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
    hdrplus::print_cvmat<uint16_t>( src_image );

    cv::Mat dst_image = hdrplus::box_filter_kxk<uint16_t, 2>( src_image );

    printf("dst cv::Mat 2x2 is \n");
    hdrplus::print_cvmat<uint16_t>( dst_image );

    dst_image = hdrplus::box_filter_kxk<uint16_t, 4>( src_image );

    printf("dst cv::Mat 4x4 is \n");
    hdrplus::print_cvmat<uint16_t>( dst_image );

    printf("test_box_filter_kxk finish\n"); fflush(stdout);
}


void test_extract_rgb_from_bayer()
{
    printf("\n###Test test_extract_rgb_from_bayer()###\n");
    // Intialize input data
    int bayer_width = 20;
    int bayer_height = 12;
    std::vector<uint16_t> bayer_data( bayer_width, bayer_height );

    for ( int i = 0; i < bayer_width * bayer_height; ++i )
    {
        bayer_data[ i ] = i+1;
    }

    // Create input cv::mat
    cv::Mat bayer_img( bayer_height, bayer_width, CV_16U, bayer_data.data() );

    printf("\nbayer cv::Mat is \n");
    hdrplus::print_cvmat<uint16_t>( bayer_img );

    cv::Mat red_img, green_img, blue_img;
    hdrplus::extract_rgb_fmom_bayer<uint16_t>( bayer_img, red_img, green_img, blue_img );

    printf("\nRed cv::Mat is \n");
    hdrplus::print_cvmat<uint16_t>( red_img );

    printf("\nGreen cv::Mat is \n");
    hdrplus::print_cvmat<uint16_t>( green_img );

    printf("\nBlue cv::Mat is \n");
    hdrplus::print_cvmat<uint16_t>( blue_img );

    printf("test_extract_rgb_from_bayer finish\n"); fflush(stdout);
}


int main()
{
    //test_box_filter_2x2();
    //test_box_filter_kxk();
    test_extract_rgb_from_bayer();

    printf("\ntest_utility finish\n");
}