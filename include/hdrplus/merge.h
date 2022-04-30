#pragma once

#include <vector>
#include <opencv2/opencv.hpp> // all opencv header
#include <cmath>
#include "hdrplus/burst.h"

#define TILE_SIZE 16

namespace hdrplus
{

class merge
{
    public:
        int offset = TILE_SIZE / 2;
        float baseline_lambda_shot = 3.24 * pow( 10, -4 );
        float baseline_lambda_read = 4.3 * pow( 10, -6 );

        merge() = default;
        ~merge() = default;

        /**
         * @brief Run alignment on burst of images
         * 
         * @param burst_images collection of burst images
         * @param alignments alignment in pixel value pair. 
         *      Outer most vector is per alternative image.
         *      Inner most two vector is for horizontal & vertical tiles 
         */
        void process( const hdrplus::burst& burst_images, \
                      std::vector<std::vector<std::vector<std::pair<int, int>>>>& alignments);
        
    private:
        cv::Mat cosineWindow1D(cv::Mat input, int window_size = TILE_SIZE) {
            cv::Mat output = input.clone();
            for (int i = 0; i < input.cols; ++i) {
                output.at<float>(0, i) = 1. / 2. - 1. / 2. * cos(2 * M_PI * (input.at<float>(0, i) + 1 / 2.) / window_size);
            }
            return output;
        }

        cv::Mat cosineWindow2D(cv::Mat tile) {
            int window_size = tile.rows; // Assuming square tile
            cv::Mat output_tile = tile.clone();

            cv::Mat window = cv::Mat::zeros(1, window_size, CV_32F);
            for(int i = 0; i < window_size; ++i) {
                window.at<float>(i) = i;
            }

            cv::Mat window_x = cosineWindow1D(window, window_size);
            window_x = cv::repeat(window_x, window_size, 1);
            cv::Mat window_2d = window_x.mul(window_x.t());

            cv::Mat window_applied;
            cv::multiply(tile, window_2d, window_applied, 1, CV_32F);
            return window_applied;
        }

        cv::Mat cat2Dtiles(std::vector<std::vector<cv::Mat>> tiles) {
            std::vector<cv::Mat> rows;
            for (auto row_tiles : tiles) {
                cv::Mat row;
                cv::hconcat(row_tiles, row);
                rows.push_back(row);
            }
            cv::Mat img;
            cv::vconcat(rows, img);
            return img;
        }

        std::vector<cv::Mat> getReferenceTiles(cv::Mat reference_image);

        cv::Mat mergeTiles(std::vector<cv::Mat> tiles, int rows, int cols);

        cv::Mat processChannel( const hdrplus::burst& burst_images, \
                      std::vector<std::vector<std::vector<std::pair<int, int>>>>& alignments, \
                      cv::Mat channel_image);
};

} // namespace hdrplus
