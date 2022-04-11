#pragma once

#include <opencv2/opencv.hpp> // all opencv header
#include <hdrplus/burst.h>
#include <hdrplus/align.h>
#include <hdrplus/merge.h>
#include <hdrplus/finish.h>

namespace hdrplus
{

class hdrplus_pipeline
{
    private:
        hdrplus::align align;
        hdrplus::merge merge;
        hdrplus::finish finish;
    
    public:
        void run_pipeline();
};

} // namespace hdrplus
