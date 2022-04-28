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

    // Obtain tiles
    cv::Mat reference_image = burst_images.grayscale_images_pad[0];
    std::vector<cv::Mat> reference_tiles = getReferenceTiles(reference_image);

    // cv::Mat outputImg = reference_image.clone();
    // cv::cvtColor(outputImg, outputImg, cv::COLOR_GRAY2RGB);
    // cv::imwrite("ref.jpg", outputImg);
    // cv::Mat outputImg1 = cosineWindow2D(reference_tiles[0].clone());
    // cv::cvtColor(outputImg1, outputImg1, cv::COLOR_GRAY2RGB);
    // cv::imwrite("tile0.jpg", outputImg1);
}

std::vector<cv::Mat> merge::getReferenceTiles(cv::Mat reference_image) {
    std::vector<cv::Mat> reference_tiles;
    for (int y = 0; y < reference_image.rows - 8; y += 8) {
        for (int x = 0; x < reference_image.cols - 8; x += 8) {
            cv::Mat tile = reference_image(cv::Rect(x, y, 16, 16));
            reference_tiles.push_back(tile);
        }
    }
    return reference_tiles;
}

} // namespace hdrplus
