#pragma once

#include <vector>
#include <utility> // std::pair
#include <opencv2/opencv.hpp> // all opencv header

namespace hdrplus
{

class align
{
    public:
        align() = default;
        void process( std::vector<cv::Mat>& bayer_images, \
                      int reference_frame_idx,\
                      std::vector<std::vector<std::vector<std::pair<int, int>>>>& aligements );
};

} // namespace hdrplus
