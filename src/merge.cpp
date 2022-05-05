#include <opencv2/opencv.hpp> // all opencv header
#include <vector>
#include <utility>
#include "hdrplus/merge.h"
#include "hdrplus/burst.h"
#include "hdrplus/utility.h"

namespace hdrplus
{

    void merge::process(hdrplus::burst& burst_images, \
        std::vector<std::vector<std::vector<std::pair<int, int>>>>& alignments)
    {
        // 4.1 Noise Parameters and RMS
        // Noise parameters calculated from baseline ISO noise parameters
        double lambda_shot, lambda_read;
        std::tie(lambda_shot, lambda_read) = burst_images.bayer_images[burst_images.reference_image_idx].get_noise_params();

        // 4.2-4.4 Denoising and Merging

        // Get padded bayer image
        cv::Mat reference_image = burst_images.bayer_images_pad[burst_images.reference_image_idx];
        cv::imwrite("ref.jpg", reference_image);

        // Get raw channels
        std::vector<cv::Mat> channels(4);
        hdrplus::extract_rgb_fmom_bayer<uint16_t>(reference_image, channels[0], channels[1], channels[2], channels[3]);

        std::vector<cv::Mat> processed_channels(4);
        // For each channel, perform denoising and merge
        for (int i = 0; i < 4; ++i) {
            // Get channel mat
            cv::Mat channel_i = channels[i];
            // cv::imwrite("ref" + std::to_string(i) + ".jpg", channel_i);
            
            //we should be getting the individual channel in the same place where we call the processChannel function with the reference channel in its arguments
            //possibly we could add another argument in the processChannel function which is the channel_i for the alternate image. maybe using a loop to cover all the other images

            //create list of channel_i of alternate images:
            std::vector<cv::Mat> alternate_channel_i_list;
            for (int j = 0; j < burst_images.num_images; j++) {
                if (j != burst_images.reference_image_idx) {

                    //get alternate image
                    cv::Mat alt_image = burst_images.bayer_images_pad[j];
                    std::vector<cv::Mat> alt_channels(4);
                    hdrplus::extract_rgb_fmom_bayer<uint16_t>(alt_image, alt_channels[0], alt_channels[1], alt_channels[2], alt_channels[3]);

                    alternate_channel_i_list.push_back(alt_channels[i]);
                }
            }

            // Apply merging on the channel
            cv::Mat merged_channel = processChannel(burst_images, alignments, channel_i, alternate_channel_i_list, lambda_shot, lambda_read);
            // cv::imwrite("merged" + std::to_string(i) + ".jpg", merged_channel);

            // Put channel raw data back to channels
            merged_channel.convertTo(processed_channels[i], CV_16U);
        }

        // Write all channels back to a bayer mat
        cv::Mat merged(reference_image.rows, reference_image.cols, CV_16U);
        int x, y;
        for (y = 0; y < reference_image.rows; ++y){
            uint16_t* row = merged.ptr<uint16_t>(y);
            if (y % 2 == 0){
                uint16_t* i0 = processed_channels[0].ptr<uint16_t>(y / 2);
                uint16_t* i1 = processed_channels[1].ptr<uint16_t>(y / 2);

                for (x = 0; x < reference_image.cols;){
                    //R
                    row[x] = i0[x / 2];
                    x++;

                    //G1
                    row[x] = i1[x / 2];
                    x++;
                }
            }
            else {
                uint16_t* i2 = processed_channels[2].ptr<uint16_t>(y / 2);
                uint16_t* i3 = processed_channels[3].ptr<uint16_t>(y / 2);

                for(x = 0; x < reference_image.cols;){
                    //G2
                    row[x] = i2[x / 2];
                    x++;

                    //B
                    row[x] = i3[x / 2];
                    x++;
                }
            }
        }

        // Remove padding
        std::vector<int> padding = burst_images.padding_info_bayer;
        cv::Range horizontal = cv::Range(padding[2], reference_image.cols - padding[3]);
        cv::Range vertical = cv::Range(padding[0], reference_image.rows - padding[1]);
        burst_images.merged_bayer_image = merged(vertical, horizontal);
        cv::imwrite("merged.jpg", burst_images.merged_bayer_image);
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

    cv::Mat merge::mergeTiles(std::vector<cv::Mat> tiles, int num_rows, int num_cols) {
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

    cv::Mat merge::processChannel(hdrplus::burst& burst_images, \
        std::vector<std::vector<std::vector<std::pair<int, int>>>>& alignments, \
        cv::Mat channel_image, \
        std::vector<cv::Mat> alternate_channel_i_list,\
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
            cv::dft(ref_tile_DFT, ref_tile_DFT, cv::DFT_SCALE | cv::DFT_COMPLEX_OUTPUT);
            reference_tiles_DFT.push_back(ref_tile_DFT);
        }

        // Acquire alternate tiles and apply FFT on them as well
        std::vector<std::vector<cv::Mat>> alt_tiles_list(reference_tiles.size());
        int num_tiles_row = alternate_channel_i_list[0].rows / offset - 1;
        int num_tiles_col = alternate_channel_i_list[0].cols / offset - 1;
        for (int y = 0; y < num_tiles_row; ++y) {
            for (int x = 0; x < num_tiles_col; ++x) {
                std::vector<cv::Mat> alt_tiles;
                // Get reference tile location
                int top_left_y = y * offset;
                int top_left_x = x * offset;

                for (int i = 0; i < alternate_channel_i_list.size(); ++i) {
                    // Get alignment displacement
                    int displacement_y, displacement_x;
                    std::tie(displacement_y, displacement_x) = alignments[i + 1][y][x];
                    // Get tile
                    cv::Mat alt_tile = alternate_channel_i_list[i](cv::Rect(top_left_x + displacement_x, top_left_y + displacement_y, TILE_SIZE, TILE_SIZE));
                    // Apply FFT
                    cv::Mat alt_tile_DFT;
                    alt_tile.convertTo(alt_tile_DFT, CV_32F);
                    cv::dft(alt_tile_DFT, alt_tile_DFT, cv::DFT_SCALE | cv::DFT_COMPLEX_OUTPUT);
                    alt_tiles.push_back(alt_tile_DFT);
                }
                alt_tiles_list[y * num_tiles_col + x] = alt_tiles;
            }
        }

        // 4.2 Temporal Denoising
        reference_tiles_DFT = temporal_denoise(reference_tiles_DFT, alt_tiles_list, noise_variance, TEMPORAL_FACTOR);
        
        // 4.3 Spatial Denoising
        reference_tiles_DFT = spatial_denoise(reference_tiles_DFT, alternate_channel_i_list.size(), noise_variance, SPATIAL_FACTOR);
        //now reference tiles are temporally and spatially denoised

        // Apply IFFT on reference tiles (frequency to spatial)
        std::vector<cv::Mat> denoised_tiles;
        for (auto dft_tile : reference_tiles_DFT) {
            cv::Mat denoised_tile;
            cv::dft(dft_tile, denoised_tile, cv::DFT_INVERSE | cv::DFT_REAL_OUTPUT);
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
        
    std::vector<cv::Mat> merge::temporal_denoise(std::vector<cv::Mat> tiles, std::vector<std::vector<cv::Mat>> alt_tiles, std::vector<float> noise_variance, float temporal_factor) {
        // goal: temporially denoise using the weiner filter
        // input:
        // 1. array of 2D dft tiles of the reference image
        // 2. array of 2D dft tiles of the aligned alternate image
        // 3. estimated noise variance
        // 4. temporal factor
        // return: merged image patches dft

        // calculate noise scaling
        double temporal_noise_scaling = ((2.0 / 16)) * TEMPORAL_FACTOR;
        
        // loop across tiles
        std::vector<cv::Mat> denoised;
        for (int i = 0; i < tiles.size(); ++i) {
            // sum of pairwise denoising
            cv::Mat tile_sum = cv::Mat::zeros(TILE_SIZE, TILE_SIZE, CV_32FC2);
            double coeff = temporal_noise_scaling * noise_variance[i];

            // Ref tile
            cv::Mat tile = tiles[i];
            // Alt tiles
            std::vector<cv::Mat> alt_tiles_i = alt_tiles[i];

            for (int j = 0; j < alt_tiles_i.size(); ++j) {
                // Alt tile
                cv::Mat alt_tile = alt_tiles_i[j];
                // Tile difference
                cv::Mat diff = tile - alt_tile;

                // Calculate absolute difference
                cv::Mat complexMats[2];
                cv::split(diff, complexMats);               // planes[0] = Re(DFT(I)), planes[1] = Im(DFT(I))
                cv::magnitude(complexMats[0], complexMats[1], complexMats[0]); // planes[0] = magnitude
                cv::Mat absolute_diff = complexMats[0].mul(complexMats[0]);

                // find shrinkage operator A
                cv::Mat shrinkage;
                cv::divide(absolute_diff, absolute_diff + coeff, shrinkage);
                cv::merge(std::vector<cv::Mat>{shrinkage, shrinkage}, shrinkage);
                
                // Interpolation
                tile_sum += alt_tile + diff.mul(shrinkage);
            }
            // Average by num of frames
            cv::divide(tile_sum, alt_tiles_i.size() + 1, tile_sum);
            denoised.push_back(tile_sum);
        }

        return denoised;
    }

    std::vector<cv::Mat> merge::spatial_denoise(std::vector<cv::Mat> tiles, int num_alts, std::vector<float> noise_variance, float spatial_factor) {
        
        double spatial_noise_scaling = ((1.0 / 16)) * spatial_factor;

        // Calculate |w| using ifftshift
        cv::Mat row_distances = cv::Mat::zeros(1, TILE_SIZE, CV_32F);
        for(int i = 0; i < TILE_SIZE; ++i) {
            row_distances.at<float>(i) = i - offset;
        }
        row_distances = cv::repeat(row_distances.t(), 1, TILE_SIZE);
        cv::Mat col_distances = row_distances.t();
        cv::Mat distances;
        cv::sqrt(row_distances.mul(row_distances) + col_distances.mul(col_distances), distances);
        ifftshift(distances);
        
        std::vector<cv::Mat> denoised;
        // Loop through all tiles
        for (int i = 0; i < tiles.size(); ++i) {
            cv::Mat tile = tiles[i];
            float coeff = noise_variance[i] / (num_alts + 1) * spatial_noise_scaling;

            // Calculate absolute difference
            cv::Mat complexMats[2];
            cv::split(tile, complexMats);               // planes[0] = Re(DFT(I)), planes[1] = Im(DFT(I))
            cv::magnitude(complexMats[0], complexMats[1], complexMats[0]); // planes[0] = magnitude
            cv::Mat absolute_diff = complexMats[0].mul(complexMats[0]);
            
            // Division
            cv::Mat scale;
            cv::divide(absolute_diff, absolute_diff + distances * coeff, scale);
            cv::merge(std::vector<cv::Mat>{scale, scale}, scale);
            denoised.push_back(tile.mul(scale));
        }
        return denoised;
    }
    

} // namespace hdrplus