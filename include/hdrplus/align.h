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
        const std::vector<int> inv_scale_factors = { 1, 2, 2, 4 };
};

} // namespace hdrplus
