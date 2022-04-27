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

        // Reference image index in the array
        int reference_image_idx;

        // Source bayer images & grayscale unpadded image
        std::vector<hdrplus::bayer_image> bayer_images;

        // Image padded to upper level tile size (16*2)
        // Use for alignment, merging, and finishing
        std::vector<cv::Mat> bayer_images_pad;

        // Padding information
        std::vector<int> padding_info_bayer;

        // Image padded to upper level tile size (16)
        // Use for alignment, merging, and finishing
        std::vector<cv::Mat> grayscale_images_pad;
    
        // number of image (including reference) in burst
        int num_images;
};

} // namespace hdrplus
