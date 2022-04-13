#pragma once

#include <string>
#include <opencv2/opencv.hpp> // all opencv header
#include "hdrplus/burst.h"
#include "hdrplus/align.h"
#include "hdrplus/merge.h"
#include "hdrplus/finish.h"

namespace hdrplus
{

class hdrplus_pipeline
{
    private:
        hdrplus::align align_module;
        hdrplus::merge merge_module;
        hdrplus::finish finish_module;
    
    public:
        void run_pipeline( const std::string& burst_path, const std::string& reference_image_path  );
        hdrplus_pipeline() = default;
        ~hdrplus_pipeline() = default;
};

} // namespace hdrplus
