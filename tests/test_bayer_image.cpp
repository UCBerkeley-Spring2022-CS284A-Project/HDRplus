#include <cstdio>
#include <utility> // std::pair
#include "hdrplus/bayer_image.h"

int main( int argc, char** argv )
{
    if ( argc != 2 )
    {
        printf("Usage: ./test_bayer_image RAW_IMAGE_PATH\n");
        exit( -1 );
    }

    hdrplus::bayer_image raw_bayer_image( argv[1] );

    printf("Raw image of shape h=%d w=%d\n", \
        raw_bayer_image.raw_image.size().height, \
        raw_bayer_image.raw_image.size().width );

    printf("Gray image of shape h=%d, w=%d\n", \
        raw_bayer_image.grayscale_image.size().height, \
        raw_bayer_image.grayscale_image.size().width );
    
    // 1143
    printf("Image ISO level %.3f\n", raw_bayer_image.iso );

    // 0.00370332, 0.0005617730699999999
    // 3.55148388, 516.6520187906699
    std::pair<double, double> noise_param = raw_bayer_image.get_noise_params();
    printf("Image Noise Param lambda_s %.10f lambda_r %.10f\n", \
        noise_param.first, noise_param.second );

    // 1023
    printf("Image white level %d\n", raw_bayer_image.white_level );

    // [64, 64, 64, 64]
    printf("Image black level %d %d %d %d\n", \
        raw_bayer_image.black_level_per_channel[0], \
        raw_bayer_image.black_level_per_channel[1], \
        raw_bayer_image.black_level_per_channel[2], \
        raw_bayer_image.black_level_per_channel[3] );
}