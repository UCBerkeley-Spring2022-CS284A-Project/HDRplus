#include <opencv2/opencv.hpp> // all opencv header
#include "hdrplus/finish.h"
#include <cmath>

// #include <type_traits>

namespace hdrplus
{
    // void normalize(cv::Mat& A,int opencvType){
    //     std::cout<<A.size()<<std::endl;
    //     std::cout<<A.channels()<<std::endl;
    //     u_int16_t* ptr = (u_int16_t*)A.data;
    //     const double div = USHRT_MAX;
    //     for(int i=0;i<A.rows;i++){
    //         for(int j=0;j<A.cols;j++){
    //             for(int k=0;k<A.channels();k++){
    //                 *(ptr+(A.rows*i+j)*A.channels()+k)/=div;
    //             }
    //         }
    //     }
    // }

    cv::Mat convert16bit2_8bit_(cv::Mat ans){
        // cv::Mat ans(I);
        cv::MatIterator_<cv::Vec3w> it, end;
        for( it = ans.begin<cv::Vec3w>(), end = ans.end<cv::Vec3w>(); it != end; ++it)
        {
            // std::cout<<sizeof (*it)[0] <<std::endl;
            (*it)[0] *=(255.0/USHRT_MAX);
            (*it)[1] *=(255.0/USHRT_MAX);
            (*it)[2] *=(255.0/USHRT_MAX);
        }
        ans.convertTo(ans, CV_8UC3);
        return ans;
    }

    uint16_t uGammaCompress_1pix(float x, float threshold,float gainMin,float gainMax,float exponent){
        // Normalize pixel val
        x/=USHRT_MAX;
        // check the val against the threshold
        if(x<=threshold){
            x =gainMin*x;
        }else{
            x = gainMax* pow(x,exponent)-gainMax+1;
        }
        // clip
        if(x<0){
            x=0;
        }else{
            if(x>1){
                x = 1;
            }
        }

        x*=USHRT_MAX;
        
        return (uint16_t)x;
    }

    uint16_t uGammaDecompress_1pix(float x, float threshold,float gainMin,float gainMax,float exponent){
        // Normalize pixel val
        x/=65535.0;
        // check the val against the threshold
        if(x<=threshold){
            x = x/gainMin;
        }else{
            x = pow((x+gainMax-1)/gainMax,exponent);
        }
        // clip
        if(x<0){
            x=0;
        }else{
            if(x>1){
                x = 1;
            }
        }
        x*=65535;
        
        return (uint16_t)x;
    }

    cv::Mat uGammaCompress_(cv::Mat m,float threshold,float gainMin,float gainMax,float exponent){
        cv::MatIterator_<cv::Vec3w> it, end;
        for( it = m.begin<cv::Vec3w>(), end = m.end<cv::Vec3w>(); it != end; ++it)
        {
            (*it)[0] =uGammaCompress_1pix((*it)[0],threshold,gainMin,gainMax,exponent);
            (*it)[1] =uGammaCompress_1pix((*it)[1],threshold,gainMin,gainMax,exponent);
            (*it)[2] =uGammaCompress_1pix((*it)[2],threshold,gainMin,gainMax,exponent);
        }
        return m;
    }

    cv::Mat uGammaDecompress_(cv::Mat m,float threshold,float gainMin,float gainMax,float exponent){
        cv::MatIterator_<cv::Vec3w> it, end;
        for( it = m.begin<cv::Vec3w>(), end = m.end<cv::Vec3w>(); it != end; ++it)
        {
            (*it)[0] =uGammaDecompress_1pix((*it)[0],threshold,gainMin,gainMax,exponent);
            (*it)[1] =uGammaDecompress_1pix((*it)[1],threshold,gainMin,gainMax,exponent);
            (*it)[2] =uGammaDecompress_1pix((*it)[2],threshold,gainMin,gainMax,exponent);
        }
        return m;
    }

    cv::Mat gammasRGB(cv::Mat img, bool mode){
        if(mode){// compress
            return uGammaCompress_(img,0.0031308, 12.92, 1.055, 1. / 2.4);
        }else{ // decompress
            return uGammaDecompress_(img, 0.04045, 12.92, 1.055, 2.4);
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
            std::cout<<"writing reference img ..."<<std::endl;
            cv::Mat outputImg = convert16bit2_8bit_(processedRefImage.clone());
            cv::cvtColor(outputImg, outputImg, cv::COLOR_RGB2BGR);
            // cv::imshow("test",processedImage);
            cv::imwrite("processedRef.jpg", outputImg);
            // cv::waitKey(0);
        }

// write gamma reference
        if(params.flags["writeGammaReference"]){
            std::cout<<"writing Gamma reference img ..."<<std::endl;
            cv::Mat outputImg = gammasRGB(processedRefImage.clone(),true);
            outputImg = convert16bit2_8bit_(outputImg);
            cv::cvtColor(outputImg, outputImg, cv::COLOR_RGB2BGR);
            cv::imwrite("processedRefGamma.jpg", outputImg);
        }

// get the bayer_image of the merged image
        bayer_image* mergedImg = new bayer_image(rawPathList[refIdx]);
        copy_mat_16U(mergedImg->raw_image,this->mergedBayer);
        cv::Mat processedMerge = postprocess(mergedImg->libraw_processor,params.rawpyArgs);

// write merged image
        if(params.flags["writeMergedImage"]){
            std::cout<<"writing Merged img ..."<<std::endl;
            cv::Mat outputImg = convert16bit2_8bit_(processedMerge.clone());
            cv::cvtColor(outputImg, outputImg, cv::COLOR_RGB2BGR);
            cv::imwrite("mergedImg.jpg", outputImg);
        }

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
