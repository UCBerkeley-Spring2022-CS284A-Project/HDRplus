#include <string>
#include <cstdio>
#include <stdexcept> // std::runtime_error
#include <opencv2/opencv.hpp> // all opencv header
#include <libraw/libraw.h>
#include "hdrplus/bayer_image.h"
#include "hdrplus/utility.h" // box_filter_2x2
namespace hdrplus
{

bayer_image::bayer_image( const std::string& bayer_image_path )
{
    // Open RAW image file
    int return_code;
    if ( ( return_code = libraw_processor.open_file( bayer_image_path.c_str() ) ) != LIBRAW_SUCCESS )
    {
        libraw_processor.recycle();
        throw std::runtime_error("Error opening file " + bayer_image_path + libraw_strerror( return_code ));
    }

    // Unpack the raw image
    if ( ( return_code = libraw_processor.unpack() ) != LIBRAW_SUCCESS )
    {
        throw std::runtime_error("Error unpack file " + bayer_image_path + libraw_strerror( return_code ));
    }

    // Get image basic info
    width = size_t( libraw_processor.imgdata.rawdata.sizes.raw_width );
    height = size_t( libraw_processor.imgdata.rawdata.sizes.raw_height );
    white_level = size_t( libraw_processor.imgdata.rawdata.color.maximum );

    #ifndef NDEBUG
    printf("%s::%s read bayer image %d with width %zu height %zu", \
        __FILE__, __func__, bayer_image_path.c_str(), width, height );
    #endif

    // Create CV mat
    // https://answers.opencv.org/question/105972/de-bayering-a-cr2-image/
    // https://www.libraw.org/node/2141
    raw_image = cv::Mat( width, height, CV_16U, libraw_processor.imgdata.rawdata.raw_image );

    // 2x2 box filter
    grayscale_image = box_filter_2x2<uint16_t>( raw_image );
}

bayer_image::~bayer_image()
{
    libraw_processor.recycle();
}

}