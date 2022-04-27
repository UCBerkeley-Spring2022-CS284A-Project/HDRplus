#pragma once

#include <string>
#include <vector>
#include <utility> // std::pair
#include <memory> // std::shared_ptr
#include <opencv2/opencv.hpp> // all opencv header
#include <libraw/libraw.h>

namespace hdrplus
{

class bayer_image
{
    public:
        explicit bayer_image( const std::string& bayer_image_path );
        ~bayer_image() = default;

        std::pair<double, double> get_noise_params() const;

        std::shared_ptr<LibRaw> libraw_processor;
        cv::Mat raw_image;
        cv::Mat grayscale_image;
        int width;
        int height;
        int white_level;
        std::vector<int> black_level_per_channel;
        float iso;

    private:
        float baseline_lambda_shot = 3.24 * pow( 10, -4 );
        float baseline_lambda_read = 4.3 * pow( 10, -6 );
};

} // namespace hdrplus
