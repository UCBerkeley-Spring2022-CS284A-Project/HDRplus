#pragma once

#include <vector>
#include <opencv2/opencv.hpp> // all opencv header
#include "hdrplus/bayer_image.h"

namespace hdrplus
{

class burst
{
    std::vector<hdrplus::bayer_image> bayer_images;
};

} // namespace hdrplus
