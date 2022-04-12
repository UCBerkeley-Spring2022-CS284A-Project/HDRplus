#pragma once

#include <string>
#include <opencv2/opencv.hpp> // all opencv header
#include <libraw/libraw.h>

namespace hdrplus
{

class bayer_image
{
    public:
        explicit bayer_image( const std::string& bayer_image_path );
        ~bayer_image();

        LibRaw libraw_processor;
        cv::Mat raw_image;
        cv::Mat grayscale_image;
        size_t width;
        size_t height;
        size_t white_level;
};

} // namespace hdrplus
