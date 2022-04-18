#pragma once

#include <opencv2/opencv.hpp> // all opencv header
#include <string>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <hdrplus/bayer_image.h>
#include <dirent.h>
#include <hdrplus/params.h>

namespace hdrplus
{

class Finish
{
    public:
        cv::Mat mergedBayer; // merged image from Merge Module
        std::string burstPath; // path to burst images
        std::vector<std::string> rawPathList; // a list or array of the path to all burst imgs under burst Path
        int refIdx; // index of the reference img
        Parameters params;
        cv::Mat rawReference;
        // LibRaw libraw_processor_finish;
        bayer_image* refBayer;

        Finish() = default;


        Finish(std::string burstPath, cv::Mat mergedBayer,int refIdx){
            this->refIdx = refIdx;
            this->burstPath = burstPath;
            this->mergedBayer = mergedBayer;
        }

        Finish(std::string burstPath, std::string mergedBayerPath,int refIdx){
            this->refIdx = refIdx;
            this->burstPath = burstPath;
            this->mergedBayer = loadFromCSV(mergedBayerPath, CV_16UC1);//
            load_rawPathList(burstPath);
            refBayer= new bayer_image(this->rawPathList[refIdx]);
            this->rawReference = refBayer->raw_image;//;grayscale_image

            // initialize parameters in libraw_processor_finish
            setLibRawParams();

            
            // cv::Mat rawRefGray = refBayer.grayscale_image;
            showParams();
            // showRawPathList();
            // showImg(rawRefGray);
            // showMat(rawReference);
            // showMat(mergedBayer);

            std::cout<<"Finish init() finished!"<<std::endl;
        }


        ~Finish() = default;

        // finish pipeline func
        void pipeline_finish();

        // replace Mat a with Mat b
        void copy_mat_16U(cv::Mat& A, cv::Mat B);
        void copy_rawImg2libraw(std::shared_ptr<LibRaw>& libraw_ptr, cv::Mat B);

        // postprocess
        // cv::Mat postprocess(std::shared_ptr<LibRaw>& libraw_ptr);

        void showImg(cv::Mat img)
        {
            int ch = CV_MAT_CN(CV_8UC1);

            // cv::Mat tmp(4208,3120,CV_16UC1);
            cv::Mat tmp(img);
            // u_int16_t* ptr_tmp = (u_int16_t*)tmp.data;
            // u_int16_t* ptr_img = (u_int16_t*)img.data;
            // // col major to row major
            // for(int r = 0; r < tmp.rows; r++) {
            //     for(int c = 0; c < tmp.cols; c++) {
            //         *(ptr_tmp+r*tmp.cols+c) = *(ptr_img+c*tmp.rows+r)/2048.0*255.0;
            //     }
            // }
            // std::cout<<"height="<<tmp.rows<<std::endl;
            // std::cout<<"width="<<tmp.cols<<std::endl;
            // cv::transpose(tmp, tmp);

            u_int16_t* ptr = (u_int16_t*)tmp.data;
            for(int r = 0; r < tmp.rows; r++) {
                for(int c = 0; c < tmp.cols; c++) {
                    *(ptr+r*tmp.cols+c) = *(ptr+r*tmp.cols+c)/2048.0*255.0;
                }
            }

            tmp = tmp.reshape(ch);
            tmp.convertTo(tmp, CV_8UC1);
            cv::imshow("test",tmp);
            cv::imwrite("test2.jpg", tmp);
            cv::waitKey(0);
            std::cout<< this->mergedBayer.size()<<std::endl;
        }

        void showMat(cv::Mat img){
            std::cout<<"size="<<img.size()<<std::endl;
            std::cout<<"type="<<img.type()<<std::endl;
        }

        void showParams()
        {
            std::cout<<"Parameters:"<<std::endl;
            std::cout<<"tuning_ltmGain = "<<this->params.tuning_ltmGain<<std::endl;
            std::cout<<"tuning_gtmContrast = "<<this->params.tuning_gtmContrast<<std::endl;
            for(auto key_val:this->params.flags){
                std::cout<<key_val.first<<","<<key_val.second<<std::endl;
            }
            std::cout<<"demosaic_algorithm = "<<refBayer->libraw_processor->imgdata.params.user_qual<<std::endl;
            std::cout<<"half_size = "<<refBayer->libraw_processor->imgdata.params.half_size<<std::endl;
            std::cout<<"use_camera_wb = "<<refBayer->libraw_processor->imgdata.params.use_camera_wb<<std::endl;
            std::cout<<"use_auto_wb = "<<refBayer->libraw_processor->imgdata.params.use_auto_wb<<std::endl;
            std::cout<<"no_auto_bright = "<<refBayer->libraw_processor->imgdata.params.no_auto_bright<<std::endl;
            std::cout<<"output_color = "<<refBayer->libraw_processor->imgdata.params.output_color <<std::endl;
            std::cout<<"gamma[0] = "<<refBayer->libraw_processor->imgdata.params.gamm[0]<<std::endl;
            std::cout<<"gamma[1] = "<<refBayer->libraw_processor->imgdata.params.gamm[1]<<std::endl;
            std::cout<<"output_bps = "<<refBayer->libraw_processor->imgdata.params.output_bps<<std::endl;

            // std::cout<<"demosaic_algorithm = "<<libraw_processor_finish.imgdata.params.user_qual<<std::endl;
            // std::cout<<"half_size = "<<libraw_processor_finish.imgdata.params.half_size<<std::endl;
            // std::cout<<"use_camera_wb = "<<libraw_processor_finish.imgdata.params.use_camera_wb<<std::endl;
            // std::cout<<"use_auto_wb = "<<libraw_processor_finish.imgdata.params.use_auto_wb<<std::endl;
            // std::cout<<"no_auto_bright = "<<libraw_processor_finish.imgdata.params.no_auto_bright<<std::endl;
            // std::cout<<"output_color = "<<libraw_processor_finish.imgdata.params.output_color <<std::endl;
            // std::cout<<"gamma[0] = "<<libraw_processor_finish.imgdata.params.gamm[0]<<std::endl;
            // std::cout<<"gamma[1] = "<<libraw_processor_finish.imgdata.params.gamm[1]<<std::endl;
            // std::cout<<"output_bps = "<<libraw_processor_finish.imgdata.params.output_bps<<std::endl;

            std::cout<<"===================="<<std::endl;
        }

        void showRawPathList(){
            std::cout<<"RawPathList:"<<std::endl;
            for(auto pth:this->rawPathList){
                std::cout<<pth<<std::endl;
            }
            std::cout<<"===================="<<std::endl;
        }




    private:
        cv::Mat loadFromCSV(const std::string& path, int opencv_type)
        {
            cv::Mat m;
            std::ifstream csvFile (path);

            std::string line;

            while (getline(csvFile, line))
            {
                std::vector<int> dvals;
                std::stringstream ss(line);
                std::string val;
                // int count=0;
                while (getline(ss, val, ','))
                {
                    dvals.push_back(stod(val));//*255.0/2048.0
                    // count++;
                }
                // std::cout<<count<<std::endl;
                cv::Mat mline(dvals, true);
                cv::transpose(mline, mline);

                m.push_back(mline);
            }
            int ch = CV_MAT_CN(opencv_type);

            m = m.reshape(ch);
            m.convertTo(m, opencv_type);
            
            return m;
        }

        void load_rawPathList(std::string burstPath){
            DIR *pDir; // pointer to root
            struct dirent *ptr;
            if (!(pDir = opendir(burstPath.c_str()))) {
                std::cout<<"root dir not found!"<<std::endl;
                return;
            }
            while ((ptr = readdir(pDir)) != nullptr) {
                // ptr will move to the next file automatically
                std::string sub_file = burstPath + "/" + ptr->d_name; // current filepath that ptr points to
                if (ptr->d_type != 8 && ptr->d_type != 4) { // not normal file or dir
                    return;
                }
                // only need normal files
                if (ptr->d_type == 8) {
                    if (strcmp(ptr->d_name, ".") != 0 && strcmp(ptr->d_name, "..") != 0) {
                        if (strstr(ptr->d_name, ".dng")) {
                            rawPathList.emplace_back(sub_file);
                        }
                    }
                }
            }
            // close root dir
            closedir(pDir);
        }

        void setLibRawParams(){
            refBayer->libraw_processor->imgdata.params.user_qual = params.rawpyArgs.demosaic_algorithm;
            refBayer->libraw_processor->imgdata.params.half_size = params.rawpyArgs.half_size;
            refBayer->libraw_processor->imgdata.params.use_camera_wb = params.rawpyArgs.use_camera_wb;
            refBayer->libraw_processor->imgdata.params.use_auto_wb = params.rawpyArgs.use_auto_wb;
            refBayer->libraw_processor->imgdata.params.no_auto_bright = params.rawpyArgs.no_auto_bright;
            refBayer->libraw_processor->imgdata.params.output_color = params.rawpyArgs.output_color;
            refBayer->libraw_processor->imgdata.params.gamm[0] = params.rawpyArgs.gamma[0];
            refBayer->libraw_processor->imgdata.params.gamm[1] = params.rawpyArgs.gamma[1];
            refBayer->libraw_processor->imgdata.params.output_bps = params.rawpyArgs.output_bps;

            // libraw_processor_finish.imgdata.params.user_qual = params.rawpyArgs.demosaic_algorithm;
            // libraw_processor_finish.imgdata.params.half_size = params.rawpyArgs.half_size;
            // libraw_processor_finish.imgdata.params.use_camera_wb = params.rawpyArgs.use_camera_wb;
            // libraw_processor_finish.imgdata.params.use_auto_wb = params.rawpyArgs.use_auto_wb;
            // libraw_processor_finish.imgdata.params.no_auto_bright = params.rawpyArgs.no_auto_bright;
            // libraw_processor_finish.imgdata.params.output_color = params.rawpyArgs.output_color;
            // libraw_processor_finish.imgdata.params.gamm[0] = params.rawpyArgs.gamma[0];
            // libraw_processor_finish.imgdata.params.gamm[1] = params.rawpyArgs.gamma[1];
            // libraw_processor_finish.imgdata.params.output_bps = params.rawpyArgs.output_bps;
        }

        
};

} // namespace hdrplus
