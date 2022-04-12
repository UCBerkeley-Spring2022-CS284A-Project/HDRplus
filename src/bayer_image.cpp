#include <string>
#include <opencv2/opencv.hpp> // all opencv header
#include <libraw/libraw.h>
#include "hdrplus/bayer_image.h"

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

    // Create CV mat
    // https://answers.opencv.org/question/105972/de-bayering-a-cr2-image/
    image = cv::Mat( width, height, CV_16U, libraw_processor.imgdata.rawdata.raw_image );
}

bayer_image::~bayer_image()
{
    libraw_processor.recycle();
}

}