#pragma once

#include <vector>
#include <string>
#include <opencv2/opencv.hpp> // all opencv header
#include "hdrplus/bayer_image.h"

namespace hdrplus
{

class burst
{
    public:
        explicit burst( const std::string& burst_path, const std::string& reference_image_path );
        ~burst() = default;

        std::string reference_image_path;
        std::string burst_path;
        int reference_image_idx;
        std::vector<std::string> bayer_image_paths;

        std::vector<hdrplus::bayer_image> bayer_images;
        std::vector<cv::Mat> grayscale_images_pad;

};

} // namespace hdrplus
