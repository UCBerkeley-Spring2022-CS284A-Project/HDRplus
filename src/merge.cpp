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

    // Call merge on each channel
    cv::Mat reference_image = burst_images.bayer_images_pad[burst_images.reference_image_idx];
    reference_image.convertTo(reference_image, CV_32F);
    
    // Get Channels
    // cv::Mat channel_0(reference_image.rows, reference_image.cols, CV_32F, (uchar*)reference_image.data, 2 * sizeof(float));
    // cv::cvtColor(outputImg, outputImg, cv::COLOR_GRAY2RGB);

    //cv::Mat merged_channel = processChannel(burst_images, alignments, channel_0);
    
}

std::vector<cv::Mat> merge::getReferenceTiles(cv::Mat reference_image) {
    std::vector<cv::Mat> reference_tiles;
    for (int y = 0; y < reference_image.rows - offset; y += offset) {
        for (int x = 0; x < reference_image.cols - offset; x += offset) {
            cv::Mat tile = reference_image(cv::Rect(x, y, TILE_SIZE, TILE_SIZE));
            reference_tiles.push_back(tile);
        }
    }
    return reference_tiles;
}

cv::Mat merge::mergeTiles(std::vector<cv::Mat> tiles, int num_rows, int num_cols){
    // 1. get all four subsets: original (evenly split), horizontal overlapped,
    // vertical overlapped, 2D overlapped
    std::vector<std::vector<cv::Mat>> tiles_original;
    for (int y = 0; y < num_rows / offset - 1; y += 2) {
        std::vector<cv::Mat> row;
        for (int x = 0; x < num_cols / offset - 1; x += 2) {
            row.push_back(tiles[y * (num_cols / offset - 1) + x]);
        }
        tiles_original.push_back(row);
    }

    std::vector<std::vector<cv::Mat>> tiles_horizontal;
    for (int y = 0; y < num_rows / offset - 1; y += 2) {
        std::vector<cv::Mat> row;
        for (int x = 1; x < num_cols / offset - 1; x += 2) {
            row.push_back(tiles[y * (num_cols / offset - 1) + x]);
        }
        tiles_horizontal.push_back(row);
    }

    std::vector<std::vector<cv::Mat>> tiles_vertical;
    for (int y = 1; y < num_rows / offset - 1; y += 2) {
        std::vector<cv::Mat> row;
        for (int x = 0; x < num_cols / offset - 1; x += 2) {
            row.push_back(tiles[y * (num_cols / offset - 1) + x]);
        }
        tiles_vertical.push_back(row);
    }

    std::vector<std::vector<cv::Mat>> tiles_2d;
    for (int y = 1; y < num_rows / offset - 1; y += 2) {
        std::vector<cv::Mat> row;
        for (int x = 1; x < num_cols / offset - 1; x += 2) {
            row.push_back(tiles[y * (num_cols / offset - 1) + x]);
        }
        tiles_2d.push_back(row);
    }

    // 2. Concatenate the four subsets
    cv::Mat img_original = cat2Dtiles(tiles_original);
    cv::Mat img_horizontal = cat2Dtiles(tiles_horizontal);
    cv::Mat img_vertical = cat2Dtiles(tiles_vertical);
    cv::Mat img_2d = cat2Dtiles(tiles_2d);

    // 3. Add the four subsets together
    img_original(cv::Rect(offset, 0, num_cols - TILE_SIZE, num_rows)) += img_horizontal;
    img_original(cv::Rect(0, offset, num_cols, num_rows - TILE_SIZE)) += img_vertical;
    img_original(cv::Rect(offset, offset, num_cols - TILE_SIZE, num_rows - TILE_SIZE)) += img_2d;

    return img_original;
}

cv::Mat merge::processChannel( const hdrplus::burst& burst_images, \
                      std::vector<std::vector<std::vector<std::pair<int, int>>>>& alignments, \
                      cv::Mat channel_image) {

    std::vector<cv::Mat> reference_tiles = getReferenceTiles(channel_image);

    // Temporal Denoising

    // Spatial Denoising

    // Process tiles through 2D cosine window
    std::vector<cv::Mat> windowed_tiles;
    for (auto tile : reference_tiles) {
        windowed_tiles.push_back(cosineWindow2D(tile));
    }

    // Merge tiles
    cv::Mat merged = mergeTiles(windowed_tiles, channel_image.rows, channel_image.cols);

    // cv::Mat outputImg = channel_image.clone();
    // cv::cvtColor(outputImg, outputImg, cv::COLOR_GRAY2RGB);
    // cv::imwrite("ref.jpg", outputImg);
    // cv::Mat outputImg1 = reference_tiles[0].clone();
    // cv::Mat outputImg1 = cosineWindow2D(reference_tiles[0].clone());
    // cv::Mat outputImg1 = cat2Dtiles(tiles_2D);
    // cv::cvtColor(merged, merged, cv::COLOR_GRAY2RGB);
    // cv::imwrite("tile0.jpg", merged);
    return merged;
}

} // namespace hdrplus
