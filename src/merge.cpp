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
<<<<<<< HEAD
    std::tie(lambda_shot, lambda_read) = burst_images.bayer_images[burst_images.reference_image_idx].get_noise_params();

    // Obtain tiles
    std::vector<cv::Mat> reference_tiles;
    cv::Mat reference_image = burst_images.grayscale_images_pad[0];
    std::cout << reference_image.rows << " " << reference_image.cols << std::endl;
    for (int y = 0; y < reference_image.rows - 8; y += 8) {
        for (int x = 0; x < reference_image.cols - 8; x += 8) {
            cv::Mat tile = reference_image(cv::Rect(x, y, 16, 16));
            reference_tiles.push_back(tile);
        }
    }
=======
    std::tie(lambda_shot, lambda_read) = merge::getNoiseParams(ISO, white_level, black_level);
>>>>>>> Compute lambda_shot and lambda_read

    

    // cv::Mat outputImg = reference_image.clone();
    // cv::cvtColor(outputImg, outputImg, cv::COLOR_GRAY2RGB);
    // cv::imwrite("ref.jpg", outputImg);
    // cv::Mat outputImg1 = reference_tiles[0].clone();
    // cv::cvtColor(outputImg1, outputImg1, cv::COLOR_GRAY2RGB);
    // cv::imwrite("tile0.jpg", outputImg1);
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
