#pragma once

#include <vector>
#include <opencv2/opencv.hpp> // all opencv header
#include "hdrplus/burst.h"

namespace hdrplus
{

class merge
{
    public:
        merge() = default;
        ~merge() = default;

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
};

} // namespace hdrplus
