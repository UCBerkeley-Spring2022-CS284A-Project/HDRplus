#pragma once

#include <string>
#include <memory> // std::shared_ptr
#include <opencv2/opencv.hpp> // all opencv header
#include <libraw/libraw.h>

namespace hdrplus
{

class bayer_image
{
    public:
        explicit bayer_image( const std::string& bayer_image_path );
        ~bayer_image() = default;

        std::shared_ptr<LibRaw> libraw_processor;
        cv::Mat raw_image;
        cv::Mat grayscale_image;
        int width;
        int height;
        int white_level;
};

} // namespace hdrplus
