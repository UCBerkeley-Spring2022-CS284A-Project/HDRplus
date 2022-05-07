#include <cstdio>
#include <vector>
#include <cstdint>
#include <opencv2/opencv.hpp>
#include "hdrplus/utility.h"

void test_downsample_nearest_neighbour( )
{
    printf("\n###Test test_box_filter_kxk()###\n");
    // Intialize input data
    int src_width = 10;
    int src_height = 6;
    std::vector<uint16_t> src_data( src_width * src_height );

    for ( int i = 0; i < src_width * src_height; ++i )
    {
        src_data[ i ] = i+1;
    }

    // Create input cv::mat
    cv::Mat src_image( src_height, src_width, CV_16U, src_data.data() );

    printf("src cv::Mat is \n");
    hdrplus::print_cvmat<uint16_t>( src_image );

    cv::Mat dst_image = hdrplus::downsample_nearest_neighbour<uint16_t, 2>( src_image );

    printf("dst cv::Mat downsample nn 2x2 is \n");
    hdrplus::print_cvmat<uint16_t>( dst_image );

    dst_image = hdrplus::downsample_nearest_neighbour<uint16_t, 4>( src_image );

    printf("dst cv::Mat downsample nn 4x4 is \n");
    hdrplus::print_cvmat<uint16_t>( dst_image );

    printf("test_downsample_nearest_neighbour finish\n"); fflush(stdout);
}


void test_box_filter_kxk()
{
    printf("\n###Test test_box_filter_kxk()###\n");
    // Intialize input data
    int src_width = 12;
    int src_height = 8;
    std::vector<uint16_t> src_data( src_width * src_height );

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
    int bayer_width = 24;
    int bayer_height = 16;
    std::vector<uint16_t> bayer_data( bayer_width * bayer_height );

    for ( int i = 0; i < bayer_width * bayer_height; ++i )
    {
        bayer_data[ i ] = i+1;
    }

    // Create input cv::mat
    cv::Mat bayer_img = cv::Mat( bayer_height, bayer_width, CV_16U, bayer_data.data() );
    cv::Mat red_img, green_img1, green_img2, blue_img;

    printf("\nbayer cv::Mat is \n");
    hdrplus::print_cvmat<uint16_t>( bayer_img );

    hdrplus::extract_rgb_from_bayer<uint16_t>( bayer_img, red_img, green_img1, green_img2, blue_img );

    printf("\nRed cv::Mat is \n");
    hdrplus::print_cvmat<uint16_t>( red_img );

    printf("\nGreen 1 cv::Mat is \n");
    hdrplus::print_cvmat<uint16_t>( green_img1 );

    printf("\nGreen 2 cv::Mat is \n");
    hdrplus::print_cvmat<uint16_t>( green_img2 );

    printf("\nBlue cv::Mat is \n");
    hdrplus::print_cvmat<uint16_t>( blue_img );

    printf("test_extract_rgb_from_bayer finish\n"); fflush(stdout);
}


void test_rgb_2_gray()
{
    printf("\n###Test test_rgb_2_gray()###\n");
    // Intialize input data
    int img_width = 10;
    int img_height = 14;
    int img_chns = 3;
    std::vector<uint16_t> rgb_data( img_width * img_height * img_chns );

    for ( int i = 0; i < img_width * img_height * img_chns; ++i )
    {
        rgb_data[ i ] = i+1;
    }

    // Create input cv::mat
    cv::Mat rgb_img = cv::Mat( img_height, img_width, CV_16UC3, rgb_data.data() );

    printf("\nrgb cv::Mat is \n");
    hdrplus::print_cvmat<uint16_t>( rgb_img );

    cv::Mat gray_img = hdrplus::rgb_2_gray<uint16_t, uint16_t, CV_16U>( rgb_img );

    printf("\nGray cv::Mat is \n");
    hdrplus::print_cvmat<uint16_t>( gray_img );

    printf("test_rgb_2_gray finish\n"); fflush(stdout);
}


int main()
{
    //test_downsample_nearest_neighbour();
    //test_box_filter_kxk();
    //test_extract_rgb_from_bayer();
    test_rgb_2_gray();

    printf("\ntest_utility finish\n");
}