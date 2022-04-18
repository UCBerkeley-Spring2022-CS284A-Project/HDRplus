#include <opencv2/opencv.hpp> // all opencv header
#include <hdrplus/params.h>

namespace hdrplus
{

void setParams(std::shared_ptr<LibRaw>& libraw_ptr, RawpyArgs rawpyArgs){
    libraw_ptr->imgdata.params.user_qual = rawpyArgs.demosaic_algorithm;
    libraw_ptr->imgdata.params.half_size = rawpyArgs.half_size;
    libraw_ptr->imgdata.params.use_camera_wb = rawpyArgs.use_camera_wb;
    libraw_ptr->imgdata.params.use_auto_wb = rawpyArgs.use_auto_wb;
    libraw_ptr->imgdata.params.no_auto_bright = rawpyArgs.no_auto_bright;
    libraw_ptr->imgdata.params.output_color = rawpyArgs.output_color;
    libraw_ptr->imgdata.params.gamm[0] = rawpyArgs.gamma[0];
    libraw_ptr->imgdata.params.gamm[1] = rawpyArgs.gamma[1];
    libraw_ptr->imgdata.params.output_bps = rawpyArgs.output_bps;
}

cv::Mat postprocess(std::shared_ptr<LibRaw>& libraw_ptr, RawpyArgs rawpyArgs){
    std::cout<<"postprocessing..."<<std::endl;
    // set parameters
    setParams(libraw_ptr,rawpyArgs);

    std::cout<<"conversion to 16 bit using black and white levels, demosaicking, white balance, color correction..."<<std::endl;

    libraw_ptr->dcraw_process();
    int errorcode;

    libraw_processed_image_t *ret_img = libraw_ptr->dcraw_make_mem_image(&errorcode);

    cv::Mat processedImg(ret_img->height,ret_img->width,CV_16UC3,ret_img->data);

    std::cout<<"postprocess finished!"<<std::endl;
    return processedImg;
}

}