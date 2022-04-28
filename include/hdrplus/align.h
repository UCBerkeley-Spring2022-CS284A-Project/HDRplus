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
        ~align() = default;

        /**
         * @brief Run alignment on burst of images
         * 
         * @param burst_images collection of burst images
         * @param aligements alignment in pixel value pair. 
         *      Outer most vector is per alternative image.
         *      Inner most two vector is for horizontle & verticle tiles 
         */
        void process( const hdrplus::burst& burst_images, \
                      std::vector<std::vector<std::vector<std::pair<int, int>>>>& aligements );

    private:
        // From original image to coarse image
        const std::vector<int> inv_scale_factors = { 1, 2, 4, 4 };
        const std::vector<int> distances = { 1, 2, 2, 2 }; // L1 / L2 distance
        const std::vector<int> grayimg_search_radious = { 1, 4, 4, 4 };
        const std::vector<int> grayimg_tile_sizes = { 16, 16, 16, 8 };
        const int num_levels = 4;
};

void align_image_level( \
    const cv::Mat& ref_img, \
    const cv::Mat& alt_img, \
    const std::vector<std::vector<std::pair<int, int>>>& reftiles_start, \
    std::vector<std::vector<std::pair<int, int>>>& prev_aligement, \
    std::vector<std::vector<std::pair<int, int>>>& alignment, \
    int scale_factor_prev_curr, \
    int tile_size, \
    int prev_tile_size, \
    int search_radiou, \
    int distance );


} // namespace hdrplus
