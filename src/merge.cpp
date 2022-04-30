#include <opencv2/opencv.hpp> // all opencv header
#include <vector>
#include <utility>
#include "hdrplus/merge.h"
#include "hdrplus/burst.h"

namespace hdrplus
{

void merge::process( hdrplus::burst& burst_images, \
                     std::vector<std::vector<std::vector<std::pair<int, int>>>>& alignments)
{
    // 4.1 Noise Parameters and RMS
    // Noise parameters calculated from baseline ISO noise parameters
    double lambda_shot, lambda_read;
    std::tie(lambda_shot, lambda_read) = burst_images.bayer_images[burst_images.reference_image_idx].get_noise_params();

    // 4.2-4.4 Denoising and Merging
    // Get padded bayer image
    cv::Mat reference_image = burst_images.bayer_images_pad[burst_images.reference_image_idx];
    // cv::imwrite("ref.jpg", reference_image);
    
    // Get raw channels
    std::vector<ushort> channels[4];

    for (int y = 0; y < reference_image.rows; ++y) {
        for (int x = 0; x < reference_image.cols; ++x) {
            if (y % 2 == 0) {
                if (x % 2 == 0) {
                    channels[0].push_back(reference_image.at<ushort>(y, x));
                } else {
                    channels[1].push_back(reference_image.at<ushort>(y, x));
                }
            } else {
                if (x % 2 == 0) {
                    channels[2].push_back(reference_image.at<ushort>(y, x));
                } else {
                    channels[3].push_back(reference_image.at<ushort>(y, x));
                }
            }
        }
    }

    // For each channel, perform denoising and merge
    for (int i = 0; i < 4; ++i) {
        // Get channel mat
        cv::Mat channel_i(reference_image.rows / 2, reference_image.cols / 2, CV_16U, channels[i].data());
        // cv::imwrite("ref" + std::to_string(i) + ".jpg", channel_i);

        // Apply merging on the channel
        cv::Mat merged_channel = processChannel(burst_images, alignments, channel_i, lambda_shot, lambda_read);
        // cv::imwrite("merged" + std::to_string(i) + ".jpg", merged_channel);

        // Put channel raw data back to channels
        channels[i] = merged_channel.reshape(1, merged_channel.total());
    }

    // Write all channels back to a bayer mat
    std::vector<ushort> merged_raw;

    for (int y = 0; y < reference_image.rows; ++y) {
        for (int x = 0; x < reference_image.cols; ++x) {
            if (y % 2 == 0) {
                if (x % 2 == 0) {
                    merged_raw.push_back(channels[0][(y/2)*(reference_image.cols/2) + (x/2)]);
                } else {
                    merged_raw.push_back(channels[1][(y/2)*(reference_image.cols/2) + (x/2)]);
                }
            } else {
                if (x % 2 == 0) {
                    merged_raw.push_back(channels[2][(y/2)*(reference_image.cols/2) + (x/2)]);
                } else {
                    merged_raw.push_back(channels[3][(y/2)*(reference_image.cols/2) + (x/2)]);
                }
            }
        }
    }
    
    // Create merged mat
    cv::Mat merged(reference_image.rows, reference_image.cols, CV_16U, merged_raw.data());
    // cv::imwrite("merged.jpg", merged);

    // Remove padding
    std::vector<int> padding = burst_images.padding_info_bayer;
    cv::Range horizontal = cv::Range(padding[2], reference_image.cols - padding[3]);
    cv::Range vertical = cv::Range(padding[0], reference_image.rows - padding[1]);
    burst_images.merged_bayer_image = merged(vertical, horizontal);
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

cv::Mat merge::processChannel( hdrplus::burst& burst_images, \
                      std::vector<std::vector<std::vector<std::pair<int, int>>>>& alignments, \
                      cv::Mat channel_image, \
                      float lambda_shot, \
                      float lambda_read) {
    // Get tiles of the reference image
    std::vector<cv::Mat> reference_tiles = getReferenceTiles(channel_image);

    // Get noise variance (sigma**2 = lambda_shot * tileRMS + lambda_read)
    std::vector<float> noise_variance = getNoiseVariance(reference_tiles, lambda_shot, lambda_read);

    // Apply FFT on reference tiles (spatial to frequency)
    std::vector<cv::Mat> reference_tiles_DFT;
    for (auto ref_tile : reference_tiles) {
        cv::Mat ref_tile_DFT;
        ref_tile.convertTo(ref_tile_DFT, CV_32F);
        cv::dft(ref_tile_DFT, ref_tile_DFT, cv::DFT_SCALE|cv::DFT_COMPLEX_OUTPUT);
        reference_tiles_DFT.push_back(ref_tile_DFT);
    }

    // TODO: 4.2 Temporal Denoising

    // TODO: 4.3 Spatial Denoising

    // Apply IFFT on reference tiles (frequency to spatial)
    std::vector<cv::Mat> denoised_tiles;
    for (auto dft_tile : reference_tiles_DFT) {
        cv::Mat denoised_tile;
        cv::dft(dft_tile, denoised_tile, cv::DFT_INVERSE|cv::DFT_REAL_OUTPUT);
        denoised_tile.convertTo(denoised_tile, CV_16U);
        denoised_tiles.push_back(denoised_tile);
    }
    reference_tiles = denoised_tiles;

    // 4.4 Cosine Window Merging
    // Process tiles through 2D cosine window
    std::vector<cv::Mat> windowed_tiles;
    for (auto tile : reference_tiles) {
        windowed_tiles.push_back(cosineWindow2D(tile));
    }

    // Merge tiles
    return mergeTiles(windowed_tiles, channel_image.rows, channel_image.cols);
}

} // namespace hdrplus
