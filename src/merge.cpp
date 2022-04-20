#include <opencv2/opencv.hpp> // all opencv header
#include <vector>
#include <utility>
#include "hdrplus/merge.h"
#include "hdrplus/burst.h"

namespace hdrplus
{

void merge::process( const hdrplus::burst& burst_images, \
                     std::vector<std::vector<std::vector<std::pair<int, int>>>>& alignments, \
                     int ISO, \
                     int white_level, \
                     double black_level )
{
    double lambda_shot, lambda_read;
    std::tie(lambda_shot, lambda_read) = merge::getNoiseParams(ISO, white_level, black_level);

    
}

std::pair<double, double> merge::getNoiseParams( int ISO, \
                                          int white_level, \
                                          double black_level ) 
{
    // Set ISO to 100 if not positive
    ISO = ISO <= 0 ? 100 : ISO;

    // Calculate shot noise and read noise parameters w.r.t ISO 100
    double lambda_shot_p = ISO / 100.0f * baseline_lambda_shot;
	double lambda_read_p = (ISO / 100.0f) * (ISO / 100.0f) * baseline_lambda_read;

    // Rescale shot and read noise to normal range
    double lambda_shot = lambda_shot_p * (white_level - black_level);
    double lambda_read = lambda_read_p * (white_level - black_level) * (white_level - black_level);

    // return pair
    return std::make_pair(lambda_shot, lambda_read);
}

} // namespace hdrplus
