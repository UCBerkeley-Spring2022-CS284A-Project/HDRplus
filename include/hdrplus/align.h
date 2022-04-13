#pragma once

#include <vector>
#include <utility> // std::pair
#include <opencv2/opencv.hpp> // all opencv header
#include "hdrplus/burst.h"

namespace hdrplus
{

class align
{
    public:
        align() = default;
        void process( hdrplus::burst& burst_images, \
                      std::vector<std::vector<std::vector<std::pair<int, int>>>>& aligements );

        const std::vector<int> inv_scale_factors = { 1, 2, 2, 4 };
};

} // namespace hdrplus
