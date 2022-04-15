#pragma once

#include <opencv2/opencv.hpp> // all opencv header
#include <string>
#include <fstream>
#include <sstream>

namespace hdrplus
{

class Finish
{
    public:
        cv::Mat mergedBayer; // merged image from Merge Module
        std::string burstPath; // path to burst images
        //std::string[] rawPathList; // a list or array of the path to all burst imgs under burst Path
        int refIdx; // index of the reference img

        Finish() = default;


        Finish(std::string burstPath, cv::Mat mergedBayer,int refIdx){
            this->refIdx = refIdx;
            this->burstPath = burstPath;
            this->mergedBayer = mergedBayer;
        }

        Finish(std::string burstPath, std::string mergedBayerPath,int refIdx){
            this->refIdx = refIdx;
            this->burstPath = burstPath;
            this->mergedBayer = loadFromCSV(mergedBayerPath, CV_8UC1);
            showImg();
        }

        ~Finish() = default;

        void showImg(){
            cv::imshow("test",this->mergedBayer);
            cv::waitKey(0);
            std::cout<< this->mergedBayer.size()<<std::endl;
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
                while (getline(ss, val, ','))
                {
                    dvals.push_back(stod(val)*255.0/2048.0);
                    
                }

                cv::Mat mline(dvals, true);
                cv::transpose(mline, mline);

                m.push_back(mline);
            }
            int ch = CV_MAT_CN(opencv_type);

            m = m.reshape(ch);
            m.convertTo(m, opencv_type);

            return m;
        }

        
};

} // namespace hdrplus
