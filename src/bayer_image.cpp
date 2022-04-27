#include <string>
#include <cstdio>
#include <utility> // std::pair, std::makr_pair
#include <memory> // std::shared_ptr
#include <stdexcept> // std::runtime_error
#include <opencv2/opencv.hpp> // all opencv header
#include <libraw/libraw.h>
#include "hdrplus/bayer_image.h"
#include "hdrplus/utility.h" // box_filter_2x2
namespace hdrplus
{

bayer_image::bayer_image( const std::string& bayer_image_path )
{
    libraw_processor = std::make_shared<LibRaw>();

    // Open RAW image file
    int return_code;
    if ( ( return_code = libraw_processor->open_file( bayer_image_path.c_str() ) ) != LIBRAW_SUCCESS )
    {
        libraw_processor->recycle();
        throw std::runtime_error("Error opening file " + bayer_image_path + " " + libraw_strerror( return_code ));
    }

    // Unpack the raw image
    if ( ( return_code = libraw_processor->unpack() ) != LIBRAW_SUCCESS )
    {
        throw std::runtime_error("Error unpack file " + bayer_image_path + " " + libraw_strerror( return_code ));
    }

    // Get image basic info
    width = int( libraw_processor->imgdata.rawdata.sizes.raw_width );
    height = int( libraw_processor->imgdata.rawdata.sizes.raw_height );
    white_level = int( libraw_processor->imgdata.rawdata.color.maximum );
    black_level_per_channel.resize( 4 );
    black_level_per_channel.at( 0 ) = int( libraw_processor->imgdata.rawdata.color.cblack[ 0 ] + libraw_processor->imgdata.rawdata.color.black );
    black_level_per_channel.at( 1 ) = int( libraw_processor->imgdata.rawdata.color.cblack[ 1 ] + libraw_processor->imgdata.rawdata.color.black );
    black_level_per_channel.at( 2 ) = int( libraw_processor->imgdata.rawdata.color.cblack[ 2 ] + libraw_processor->imgdata.rawdata.color.black );
    black_level_per_channel.at( 3 ) = int( libraw_processor->imgdata.rawdata.color.cblack[ 3 ] + libraw_processor->imgdata.rawdata.color.black );
    iso = float( libraw_processor->imgdata.other.iso_speed );

    // Create CV mat
    // https://answers.opencv.org/question/105972/de-bayering-a-cr2-image/
    // https://www.libraw.org/node/2141
    raw_image = cv::Mat( height, width, CV_16U, libraw_processor->imgdata.rawdata.raw_image ).clone(); // changed the order of width and height

    // 2x2 box filter
    grayscale_image = box_filter_2x2<uint16_t>( raw_image );

    #ifndef NDEBUG
    printf("%s::%s read bayer image %s with\n width %zu\n height %zu\n iso %.3f\n white level %d\n black level %d %d %d %d\n", \
        __FILE__, __func__, bayer_image_path.c_str(), width, height, iso, white_level, \
        black_level_per_channel[0], black_level_per_channel[1], black_level_per_channel[2], black_level_per_channel[3] );
    fflush( stdout );
    #endif
}

std::pair<double, double> bayer_image::get_noise_params() const
{
    double iso100_lambdas = 3.24 * 0.0001;
    double iso100_lambdar = 4.3 * 0.000001;

    double lambdas = iso / 100 * iso100_lambdas;
    double lambdar = ( iso / 100 ) * ( iso / 100 ) * iso100_lambdar;

    return std::make_pair<int, int>(lambdas, lambdar);
}

}
