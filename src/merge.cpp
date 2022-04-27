#include <opencv2/opencv.hpp> // all opencv header
#include <vector>
#include <utility>
#include "hdrplus/merge.h"
#include "hdrplus/burst.h"

namespace hdrplus
{

void merge::process( const hdrplus::burst& burst_images, \
                     std::vector<std::vector<std::vector<std::pair<int, int>>>>& alignments)
{
    double lambda_shot, lambda_read;
    std::tie(lambda_shot, lambda_read) = burst_images.bayer_images[burst_images.reference_image_idx].get_noise_params();
}

} // namespace hdrplus
