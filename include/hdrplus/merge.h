#pragma once

#include <vector>
#include <opencv2/opencv.hpp> // all opencv header
#include "hdrplus/burst.h"

namespace hdrplus
{

class merge
{
    public:
        float baseline_lambda_shot = 3.24 * pow( 10, -4 );
        float baseline_lambda_read = 4.3 * pow( 10, -6 );

        merge() = default;
        ~merge() = default;

        /**
         * @brief Run alignment on burst of images
         * 
         * @param burst_images collection of burst images
         * @param alignments alignment in pixel value pair. 
         *      Outer most vector is per alternative image.
         *      Inner most two vector is for horizontal & vertical tiles 
         */
        void process( const hdrplus::burst& burst_images, \
                      std::vector<std::vector<std::vector<std::pair<int, int>>>>& alignments);
};

} // namespace hdrplus
