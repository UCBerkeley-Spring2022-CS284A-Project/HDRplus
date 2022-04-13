#include <cstdio>
#include <string>
#include <vector>
#include <utility> // std::pair
#include <opencv2/opencv.hpp> // all opencv header
#include "hdrplus/hdrplus_pipeline.h"
#include "hdrplus/burst.h"
#include "hdrplus/align.h"
#include "hdrplus/merge.h"
#include "hdrplus/finish.h"

namespace hdrplus
{
    
void hdrplus_pipeline::run_pipeline( \
    const std::string& burst_path, \
    const std::string& reference_image_path  )
{
    // Create burst of images
    burst burst_images( burst_path, reference_image_path );
    std::vector<std::vector<std::vector<std::pair<int, int>>>> alignments;
    
    // Run align
    align_module.process( burst_images, alignments );

    // Run merging
    merge_module.process( burst_images, alignments );

    // Run finishing
}

} // namespace hdrplus
