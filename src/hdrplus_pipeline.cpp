#include <cstdio>
#include <opencv2/opencv.hpp> // all opencv header
#include "hdrplus/hdrplus_pipeline.h"
#include <string>

namespace hdrplus
{
    
void hdrplus_pipeline::run_pipeline()
{
    printf("Run pipeline\n");

    // finish part
    std::string burstPath = "../test_data/";
    std::string mergedBayerPath = "../test_data/_merged_bayer.csv";

    this->finish = hdrplus::Finish(burstPath,mergedBayerPath,0);
}

} // namespace hdrplus
