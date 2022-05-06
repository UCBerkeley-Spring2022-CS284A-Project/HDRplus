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
#include <fstream>

namespace hdrplus
{
void writeCSV(std::string filename, cv::Mat m)
{
    std::ofstream myfile;
    myfile.open(filename.c_str());
    myfile<< cv::format(m, cv::Formatter::FMT_CSV) << std::endl;
    myfile.close();
}

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

    cv::Mat mergedBayer = burst_images.merged_bayer_image.clone();

    std::cout<<std::endl<<"size: "<<mergedBayer.rows<<"*"<<mergedBayer.cols<<std::endl;

    // for(int i=0;i<20;i++){
    //     u_int16_t* ptr = (u_int16_t*)mergedBayer.data;
    //     for(int j=0;j<20;j++){
    //         std::cout<<*(ptr+i*mergedBayer.cols+j)<<", ";
    //     }
    //     std::cout<<std::endl;
    // }

    hdrplus::writeCSV("merged.csv",mergedBayer);


    // Run finishing
    finish_module.process( burst_path, burst_images.merged_bayer_image, burst_images.reference_image_idx);
}

} // namespace hdrplus
