#include <opencv2/opencv.hpp> // all opencv header
#include "hdrplus/finish.h"
#include <type_traits>

namespace hdrplus
{
    void normalize(cv::Mat& A,int opencvType){
        std::cout<<A.size()<<std::endl;
        std::cout<<A.channels()<<std::endl;
        u_int16_t* ptr = (u_int16_t*)A.data;
        const double div = USHRT_MAX;
        for(int i=0;i<A.rows;i++){
            for(int j=0;j<A.cols;j++){
                for(int k=0;k<A.channels();k++){
                    *(ptr+(A.rows*i+j)*A.channels()+k)/=div;
                }
            }
        }
    }

    void convert16bit2_8bit_(cv::Mat& I){
        cv::MatIterator_<cv::Vec3w> it, end;
        for( it = I.begin<cv::Vec3w>(), end = I.end<cv::Vec3w>(); it != end; ++it)
        {
            // std::cout<<sizeof (*it)[0] <<std::endl;
            (*it)[0] *=(255.0/USHRT_MAX);
            (*it)[1] *=(255.0/USHRT_MAX);
            (*it)[2] *=(255.0/USHRT_MAX);
        }
    }

    void Finish::pipeline_finish(){
        // copy mergedBayer to rawReference
        std::cout<<"finish pipeline start ..."<<std::endl;

// read in ref img
        bayer_image* ref = new bayer_image(rawPathList[refIdx]);
        cv::Mat processedRefImage = postprocess(ref->libraw_processor,params.rawpyArgs);

// write reference image
        if(params.flags["writeReferenceImage"]){
            std::cout<<"process reference img ..."<<std::endl;
            
            convert16bit2_8bit_(processedRefImage);
            processedRefImage.convertTo(processedRefImage, CV_8UC3);
            cv::cvtColor(processedRefImage, processedRefImage, cv::COLOR_RGB2BGR);
            // cv::imshow("test",processedImage);
            cv::imwrite("processedRef.jpg", processedRefImage);
            // cv::waitKey(0);
        }

// write gamma reference


    }

    void Finish::copy_mat_16U(cv::Mat& A, cv::Mat B){
        u_int16_t* ptr_A = (u_int16_t*)A.data;
        u_int16_t* ptr_B = (u_int16_t*)B.data;
        for(int r = 0; r < A.rows; r++) {
            for(int c = 0; c < A.cols; c++) {
                *(ptr_A+r*A.cols+c) = *(ptr_B+r*B.cols+c);
            }
        }
    }

    void Finish::copy_rawImg2libraw(std::shared_ptr<LibRaw>& libraw_ptr, cv::Mat B){
        u_int16_t* ptr_A = (u_int16_t*)libraw_ptr->imgdata.rawdata.raw_image;
        u_int16_t* ptr_B = (u_int16_t*)B.data;
        for(int r = 0; r < B.rows; r++) {
            for(int c = 0; c < B.cols; c++) {
                *(ptr_A+r*B.cols+c) = *(ptr_B+r*B.cols+c);
            }
        }

    }

    // cv::Mat Finish::postprocess(std::shared_ptr<LibRaw>& libraw_ptr){
    //     std::cout<<"postprocessing..."<<std::endl;
    //     std::cout<<"conversion to 16 bit using black and white levels, demosaicking, white balance, color correction..."<<std::endl;

    //     libraw_ptr->dcraw_process();
    //     int errorcode;

    //     libraw_processed_image_t *ret_img = libraw_ptr->dcraw_make_mem_image(&errorcode);

    //     // std::cout<<"ret_img height = "<<ret_img->height<<std::endl;
    //     // std::cout<<"ret_img width = "<<ret_img->width<<std::endl;
    //     // std::cout<<"ret_img colors = "<<ret_img->colors<<std::endl;
    //     // std::cout<<"ret_img bits = "<<ret_img->bits<<std::endl;

    //     cv::Mat processedImg(ret_img->height,ret_img->width,CV_16UC3,ret_img->data);

    //     std::cout<<"postprocess finished!"<<std::endl;
    //     return processedImg;
    // }

    // void Finish::pipeline_finish(){ // post process test
    //     // copy mergedBayer to rawReference
    //     std::cout<<"finish pipeline start ..."<<std::endl;
    //     // copy_mat_16U(rawReference,mergedBayer);
    //     // copy_rawImg2libraw(refBayer->libraw_processor,mergedBayer);
    //     // rawReference = cv::Mat( rawReference.rows, rawReference.cols, CV_16U, refBayer->libraw_processor->imgdata.rawdata.raw_image );
    //     // showImg(rawReference);
    //     if(params.flags["writeReferenceImage"]){
    //         std::cout<<"process reference img ..."<<std::endl;
    //         cv::Mat processedImage = postprocess(refBayer->libraw_processor,params.rawpyArgs);
    //         //normalize(processedImage,CV_16UC1);
    //         std::cout<<"processedImage.rows = "<<processedImage.rows<<std::endl;
    //         // convert16bit2_8bit_(processedImage);
    //         // processedImage.convertTo(processedImage, CV_8UC3);
    //         cv::cvtColor(processedImage, processedImage, cv::COLOR_RGB2BGR);
    //         cv::imshow("test",processedImage);
    //         cv::imwrite("processedRef.png", processedImage);
    //         cv::waitKey(0);
    //     }
        
    //     // showImg(rawReference);

    // }

    
    
} // namespace hdrplus
