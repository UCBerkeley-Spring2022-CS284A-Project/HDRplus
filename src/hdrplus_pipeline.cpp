#include <cstdio>
#include <string>
#include <vector>
#include <utility> // std::pair
#include <opencv2/opencv.hpp> // all opencv header
#include <exiv2/exiv2.hpp> // exiv2
#include "hdrplus/hdrplus_pipeline.h"
#include "hdrplus/burst.h"
#include "hdrplus/align.h"
#include "hdrplus/merge.h"
#include "hdrplus/finish.h"

namespace hdrplus
{
    
void hdrplus_pipeline::run_pipeline( \
    const std::string& burst_path, \
    const std::string& reference_image_path  )
{
    // Create burst of images
    burst burst_images( burst_path, reference_image_path );
    std::vector<std::vector<std::vector<std::pair<int, int>>>> alignments;
    
    // Read exif tags
    Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(reference_image_path);
    assert(image.get() != 0);
    image->readMetadata();
    Exiv2::ExifData &exifData = image->exifData();
    if (exifData.empty()) {
        std::string error(reference_image_path);
        error += ": No Exif data found in the file";
        std::cout << error << std::endl;
    }
    int ISO = exifData["Exif.Image.ISOSpeedRatings"].toLong();
    int white_level = exifData["Exif.Image.WhiteLevel"].toLong();
    double black_level = exifData["Exif.Image.BlackLevel"].toFloat();

    // Run align
    align_module.process( burst_images, alignments );

    // Run merging
    merge_module.process( burst_images, alignments, ISO, white_level, black_level );

    // Run finishing
}

} // namespace hdrplus
