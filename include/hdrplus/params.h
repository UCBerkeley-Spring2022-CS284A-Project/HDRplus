#pragma once

#include <string>
#include <memory> // std::shared_ptr
#include <opencv2/opencv.hpp> // all opencv header
#include <libraw/libraw.h>


namespace hdrplus
{
class RawpyArgs{
    public:
        int demosaic_algorithm = 3;// 3 - AHD interpolation <->int user_qual
        bool half_size = false;
        bool use_camera_wb = true;
        bool use_auto_wb = false;
        bool no_auto_bright = true;
        int output_color = LIBRAW_COLORSPACE_sRGB;
        int gamma[2] = {1,1}; //# gamma correction not applied by rawpy (not quite understand)
        int output_bps = 16;
};

class Parameters{
    public:
        std::string tuning_ltmGain = "auto";
        double tuning_gtmContrast = 0.075;

        std::unordered_map<std::string,bool> flags{
            {"writeReferenceImage",true},
            {"writeGammaReference", true},
            {"writeMergedImage", false},
            {"writeGammaMerged", true},
            {"writeShortExposure", false},
            {"writeLongExposure", false},
            {"writeFusedExposure", false},
            {"writeLTMImage", false},
            {"writeLTMGamma", false},
            {"writeGTMImage", false},
            {"writeReferenceFinal", true},
            {"writeFinalImage", true}
        };

        RawpyArgs rawpyArgs;

        Parameters()= default;

};

cv::Mat postprocess(std::shared_ptr<LibRaw>& libraw_ptr, RawpyArgs rawpyArgs);
void setParams(std::shared_ptr<LibRaw>& libraw_ptr, RawpyArgs rawpyArgs);

} // namespace hdrplus