#include <opencv2/opencv.hpp> // all opencv header
// #include <opencv2/photo.hpp>
#include "hdrplus/finish.h"
#include <cmath>

// #include <type_traits>

namespace hdrplus
{
    

    cv::Mat convert16bit2_8bit_(cv::Mat ans){
        if(ans.type()==CV_16UC3){
            cv::MatIterator_<cv::Vec3w> it, end;
            for( it = ans.begin<cv::Vec3w>(), end = ans.end<cv::Vec3w>(); it != end; ++it)
            {
                // std::cout<<sizeof (*it)[0] <<std::endl;
                (*it)[0] *=(255.0/USHRT_MAX);
                (*it)[1] *=(255.0/USHRT_MAX);
                (*it)[2] *=(255.0/USHRT_MAX);
            }
            ans.convertTo(ans, CV_8UC3);
        }else if(ans.type()==CV_16UC1){
            u_int16_t* ptr = (u_int16_t*)ans.data;
            int end = ans.rows*ans.cols;
            for(int i=0;i<end;i++){
                *(ptr+i) *=(255.0/USHRT_MAX);
            }
            ans.convertTo(ans, CV_8UC1);
        }else{
            std::cout<<"Unsupported Data Type"<<std::endl;
        }
        return ans;
    }

    cv::Mat convert8bit2_16bit_(cv::Mat ans){
        if(ans.type()==CV_8UC3){
            ans.convertTo(ans, CV_16UC3);
            cv::MatIterator_<cv::Vec3w> it, end;
            for( it = ans.begin<cv::Vec3w>(), end = ans.end<cv::Vec3w>(); it != end; ++it)
            {
                // std::cout<<sizeof (*it)[0] <<std::endl;
                (*it)[0] *=(65535.0/255.0);
                (*it)[1] *=(65535.0/255.0);
                (*it)[2] *=(65535.0/255.0);
            }
            
        }else if(ans.type()==CV_8UC1){
            ans.convertTo(ans, CV_16UC1);
            u_int16_t* ptr = (u_int16_t*)ans.data;
            int end = ans.rows*ans.cols;
            for(int i=0;i<end;i++){
                *(ptr+i) *=(65535.0/255.0);
            }
            
        }else{
            std::cout<<"Unsupported Data Type"<<std::endl;
        }
        return ans;
    }

    cv::Mat convert8bit2_12bit_(cv::Mat ans){
        // cv::Mat ans(I);
        cv::MatIterator_<cv::Vec3w> it, end;
        for( it = ans.begin<cv::Vec3w>(), end = ans.end<cv::Vec3w>(); it != end; ++it)
        {
            // std::cout<<sizeof (*it)[0] <<std::endl;
            (*it)[0] *=(2048.0/255.0);
            (*it)[1] *=(2048.0/255.0);
            (*it)[2] *=(2048.0/255.0);
        }
        ans.convertTo(ans, CV_16UC3);
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
        if(m.type()==CV_16UC3){
            cv::MatIterator_<cv::Vec3w> it, end;
            for( it = m.begin<cv::Vec3w>(), end = m.end<cv::Vec3w>(); it != end; ++it)
            {
                (*it)[0] =uGammaCompress_1pix((*it)[0],threshold,gainMin,gainMax,exponent);
                (*it)[1] =uGammaCompress_1pix((*it)[1],threshold,gainMin,gainMax,exponent);
                (*it)[2] =uGammaCompress_1pix((*it)[2],threshold,gainMin,gainMax,exponent);
            }
        }else if(m.type()==CV_16UC1){
            u_int16_t* ptr = (u_int16_t*)m.data;
            int end = m.rows*m.cols;
            for(int i=0;i<end;i++){
                *(ptr+i) = uGammaCompress_1pix(*(ptr+i),threshold,gainMin,gainMax,exponent);
            }

        }else{
            std::cout<<"Unsupported Data Type"<<std::endl;
        }
        return m;
    }

    cv::Mat uGammaDecompress_(cv::Mat m,float threshold,float gainMin,float gainMax,float exponent){
        if(m.type()==CV_16UC3){
            cv::MatIterator_<cv::Vec3w> it, end;
            for( it = m.begin<cv::Vec3w>(), end = m.end<cv::Vec3w>(); it != end; ++it)
            {
                (*it)[0] =uGammaDecompress_1pix((*it)[0],threshold,gainMin,gainMax,exponent);
                (*it)[1] =uGammaDecompress_1pix((*it)[1],threshold,gainMin,gainMax,exponent);
                (*it)[2] =uGammaDecompress_1pix((*it)[2],threshold,gainMin,gainMax,exponent);
            }
        }else if(m.type()==CV_16UC1){
            u_int16_t* ptr = (u_int16_t*)m.data;
            int end = m.rows*m.cols;
            for(int i=0;i<end;i++){
                *(ptr+i) = uGammaDecompress_1pix(*(ptr+i),threshold,gainMin,gainMax,exponent);
            }

        }else{
            std::cout<<"Unsupported Data Type"<<std::endl;
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

    void copy_mat_16U_2(u_int16_t* ptr_A, cv::Mat B){
        // u_int16_t* ptr_A = (u_int16_t*)A.data;
        u_int16_t* ptr_B = (u_int16_t*)B.data;
        for(int r = 0; r < B.rows; r++) {
            for(int c = 0; c < B.cols; c++) {
                *(ptr_A+r*B.cols+c) = *(ptr_B+r*B.cols+c);
            }
        }
    }

    cv::Mat mean_(cv::Mat img){
        // initialize processedImg
        int H = img.rows;
        int W = img.cols;
        cv::Mat processedImg = cv::Mat(H,W,CV_16UC1);
        u_int16_t* ptr = (u_int16_t*)processedImg.data;
        
        // traverse img
        int idx = 0;
        cv::MatIterator_<cv::Vec3w> it, end;
        for( it = img.begin<cv::Vec3w>(), end = img.end<cv::Vec3w>(); it != end; ++it)
        {
            uint32_t tmp = (*it)[0]+(*it)[1]+(*it)[2];
            uint16_t avg_val = tmp/3;
            *(ptr+idx) = avg_val;
            idx++;
        }

        return processedImg;
    }

    double getMean(cv::Mat img){
        u_int16_t* ptr = (u_int16_t*)img.data;
        int max_idx = img.rows*img.cols*img.channels();
        double sum=0;
        for(int i=0;i<max_idx;i++){
            sum += *(ptr+i);
        }
        sum/=max_idx;
        sum/=USHRT_MAX;
        return sum;
    }

    cv::Mat matMultiply_scalar(cv::Mat img,float gain){
        u_int16_t* ptr = (u_int16_t*)img.data;
        int max_idx = img.rows*img.cols*img.channels();
        for(int i=0;i<max_idx;i++){
            double tmp = *(ptr+i)*gain;
            if(tmp<0){
                *(ptr+i)=0;
            }else if(tmp>USHRT_MAX){
                *(ptr+i) = USHRT_MAX;
            }else{
                *(ptr+i)=(u_int16_t)tmp;
            }
        }
        return img;
    }

    double getSaturated(cv::Mat img, double threshold){
        threshold *= USHRT_MAX;
        double count=0;
        u_int16_t* ptr = (u_int16_t*)img.data;
        int max_idx = img.rows*img.cols*img.channels();
        for(int i=0;i<max_idx;i++){
            if(*(ptr+i)>threshold){
                count++;
            }
        }
        return count/(double)max_idx;

    }

    cv::Mat meanGain_(cv::Mat img,int gain){
        if(img.channels()!=3){
            std::cout<<"unsupport img type in meanGain_()"<<std::endl;
            return cv::Mat();
        }else{ // RGB img
            int H = img.rows;
            int W = img.cols;
            cv::Mat processedImg = cv::Mat(H,W,CV_16UC1);
            u_int16_t* ptr = (u_int16_t*)processedImg.data;
            int idx=0;

            cv::MatIterator_<cv::Vec3w> it, end;
            for( it = img.begin<cv::Vec3w>(), end = img.end<cv::Vec3w>(); it != end; ++it)
            {
                double sum = 0;
                // R
                double tmp = (*it)[0]*gain;
                if(tmp<0) tmp=0;
                if(tmp>USHRT_MAX) tmp = USHRT_MAX;
                sum+=tmp;

                // G
                tmp = (*it)[1]*gain;
                if(tmp<0) tmp=0;
                if(tmp>USHRT_MAX) tmp = USHRT_MAX;
                sum+=tmp;

                // B
                tmp = (*it)[2]*gain;
                if(tmp<0) tmp=0;
                if(tmp>USHRT_MAX) tmp = USHRT_MAX;
                sum+=tmp;

                // put into processedImg
                uint16_t avg_val = sum/3;
                *(ptr+idx) = avg_val;
                idx++;
            }
            return processedImg;
        }

    }

    cv::Mat applyScaling_(cv::Mat mergedImage, cv::Mat shortGray, cv::Mat fusedGray){
        cv::Mat result = mergedImage.clone();
        u_int16_t* ptr_shortg = (u_int16_t*)shortGray.data;
        u_int16_t* ptr_fusedg = (u_int16_t*)fusedGray.data;
        int count = 0;
        cv::MatIterator_<cv::Vec3w> it, end;
        for( it = result.begin<cv::Vec3w>(), end = result.end<cv::Vec3w>(); it != end; ++it)
        {
            double s = 1;
            if(*(ptr_shortg+count)!=0){
                s = *(ptr_fusedg+count);
                s/=*(ptr_shortg+count);
            }
            for(int c=0;c<mergedImage.channels();c++){
                double tmp = (*it)[c]*s;
                if(tmp<0){
                    (*it)[c] = 0;
                }else if(tmp>USHRT_MAX){
                    (*it)[c] = USHRT_MAX;
                }else{
                    (*it)[c] = tmp;
                }
            }
        }
        return result;
    }

    void localToneMap(cv::Mat& mergedImage, Options options, cv::Mat& shortg,
         cv::Mat& longg, cv::Mat& fusedg, int& gain){
        std::cout<<"HDR Tone Mapping..."<<std::endl;
        // # Work with grayscale images
        cv::Mat shortGray = mean_(mergedImage);
        std::cout<<"--- Compute grayscale image"<<std::endl;

        // compute gain
        gain = 0;
        if(options.ltmGain==-1){
            double dsFactor = 25;
            int down_height = round(shortGray.rows/dsFactor);
            int down_width = round(shortGray.cols/dsFactor);
            cv::Mat shortS;
            cv::resize(shortGray,shortS,cv::Size(down_height,down_width),cv::INTER_LINEAR);
            shortS = shortS.reshape(1,1);

            bool bestGain = false;
            double compression = 1.0;
            double saturated = 0.0;
            cv::Mat shortSg = gammasRGB(shortS.clone(), true);
            double sSMean = getMean(shortSg);

            while((compression < 1.9 && saturated < .95)||((!bestGain) && (compression < 6) && (gain < 30) && (saturated < 0.33))){
                gain += 2;
                cv::Mat longSg = gammasRGB(shortS.clone()*gain, true);
                double lSMean = getMean(longSg);
                compression = lSMean / sSMean;
                bestGain = lSMean > (1 - sSMean) / 2;  // only works if burst underexposed
                saturated = getSaturated(longSg,0.95);
                if(options.verbose==4){

                }
            }    

        }else{
            if(options.ltmGain>0){
                gain = options.ltmGain;
            }
        }
        std::cout<<"--- Compute gain"<<std::endl;
        // create a synthetic long exposure
        cv::Mat longGray = meanGain_(mergedImage.clone(),gain);
        std::cout<<"--- Synthetic long expo"<<std::endl;
        // apply gamma correction to both
        longg = gammasRGB(longGray.clone(), true);
        shortg = gammasRGB(shortGray.clone(),true);
        std::cout<<"--- Apply Gamma correction"<<std::endl;
        // perform tone mapping by exposure fusion in grayscale
        cv::Ptr<cv::MergeMertens> mergeMertens = cv::createMergeMertens();
        std::cout<<"--- Create Mertens"<<std::endl;
        // hack: cv2 mergeMertens expects inputs between 0 and 255
	    // but the result is scaled between 0 and 1 (some values can actually be greater than 1!)
        std::vector<cv::Mat> src_expos;
        src_expos.push_back(convert16bit2_8bit_(shortg.clone()));
        src_expos.push_back(convert16bit2_8bit_(longg.clone()));
        mergeMertens->process(src_expos, fusedg);
        fusedg = fusedg*USHRT_MAX;
        fusedg.convertTo(fusedg, CV_16UC1);
        std::cout<<"--- Apply Mertens"<<std::endl;
        // undo gamma correction
        cv::Mat fusedGray = gammasRGB(fusedg.clone(), false);
        // cv::imwrite("fusedg_degamma.png", fusedGray);
        std::cout<<"--- Un-apply Gamma correction"<<std::endl;
        // scale each RGB channel of the short exposure accordingly
        mergedImage = applyScaling_(mergedImage, shortGray, fusedGray);
        std::cout<<"--- Scale channels"<<std::endl;
    }

    u_int16_t enhanceContrast_1pix(u_int16_t pix_val,double gain){
        double x = pix_val;
        x/=USHRT_MAX;
        x = x - gain*sin(2*M_PI*x);
        if(x<0){
            x = 0;
        }else if(x>1){
            x = 1;
        }
        u_int16_t result = x*USHRT_MAX;
        return result;
    }

    cv::Mat enhanceContrast(cv::Mat image, Options options){
        if(options.gtmContrast>=0 && options.gtmContrast<=1){
            u_int16_t* ptr = (u_int16_t*)image.data;
            int end = image.rows*image.cols*image.channels();
            for(int idx = 0;idx<end;idx++){
                *(ptr+idx) = enhanceContrast_1pix(*(ptr+idx),options.gtmContrast);
            }
        }else{
            std::cout<<"GTM ignored, expected a contrast enhancement ratio between 0 and 1"<<std::endl;
        }
        return image;
    }

    cv::Mat distL1_(cv::Mat X, cv::Mat Y){
        int end_x = X.rows*X.cols*X.channels();
        int end_y = Y.rows*Y.cols*Y.channels();
        cv::Mat result = cv::Mat(X.rows,X.cols,X.type());
        if(end_x==end_y){
            u_int16_t* ptr_x = (u_int16_t*)X.data;
            u_int16_t* ptr_y = (u_int16_t*)Y.data;
            u_int16_t* ptr_r = (u_int16_t*)result.data;
            for(int i=0;i<end_x;i++){
                if(*(ptr_x+i)<*(ptr_y+i)){
                    *(ptr_r+i) = *(ptr_y+i) - *(ptr_x+i);
                }else{
                    *(ptr_r+i) = *(ptr_x+i) - *(ptr_y+i);
                }
            }
        }else{
            std::cout<<"Mat size not match. distL1_ failed!"<<std::endl;
        }
        return result;
    }

    cv::Mat sharpenTriple_(cv::Mat image,
        cv::Mat blur0, cv::Mat low0, float th0, float k0,
        cv::Mat blur1, cv::Mat low1, float th1, float k1,
        cv::Mat blur2, cv::Mat low2, float th2, float k2){
            // create result mat
            cv::Mat result = cv::Mat(image.rows,image.cols,image.type());
            // initialize iteraters
            u_int16_t* ptr_r = (u_int16_t*)result.data;
            u_int16_t* ptr_img = (u_int16_t*)image.data;
            u_int16_t* ptr_blur0 = (u_int16_t*)blur0.data;
            u_int16_t* ptr_low0 = (u_int16_t*)low0.data;
            u_int16_t* ptr_blur1 = (u_int16_t*)blur1.data;
            u_int16_t* ptr_low1 = (u_int16_t*)low1.data;
            u_int16_t* ptr_blur2 = (u_int16_t*)blur2.data;
            u_int16_t* ptr_low2 = (u_int16_t*)low2.data;
            int n_channels = image.channels();
            int end = image.rows*image.cols*n_channels;
            // traverse Image
            for(int idx = 0;idx<end;idx++){
                double r, r0, r1, r2;
                double x = *(ptr_img+idx);
                double l0 = *(ptr_low0+idx)/(double)USHRT_MAX;
                double l1 = *(ptr_low1+idx)/(double)USHRT_MAX;
                double l2 = *(ptr_low2+idx)/(double)USHRT_MAX;
                double b0 = *(ptr_blur0+idx);
                double b1 = *(ptr_blur1+idx);
                double b2 = *(ptr_blur2+idx);
                r0 = l0<th0? x:x+k0*(x-b0);
                r1 = l1<th1? x:x+k1*(x-b1);
                r2 = l2<th2? x:x+k2*(x-b2);
                r = (r0+r1+r2)/3.0;
                if(r<0) r=0;
                if(r>USHRT_MAX) r = USHRT_MAX;
                *(ptr_r+idx) = (u_int16_t)r;
            }
            return result;
        }

    cv::Mat sharpenTriple(cv::Mat image, Tuning tuning, Options options){
        // sharpen the image using unsharp masking
        std::vector<float> amounts = tuning.sharpenAmount;
        std::vector<float> sigmas = tuning.sharpenSigma;
        std::vector<float> thresholds = tuning.sharpenThreshold;
        // Compute all Gaussian blur
        cv::Mat blur0,blur1,blur2;
        cv::GaussianBlur(image,blur0,cv::Size(0,0),sigmas[0]);
        cv::GaussianBlur(image,blur1,cv::Size(0,0),sigmas[1]);
        cv::GaussianBlur(image,blur2,cv::Size(0,0),sigmas[2]);
        std::cout<<" --- gaussian blur"<<std::endl;
        // cv::imwrite("blur2.png", blur2);
        // Compute all low contrast images
        cv::Mat low0 = distL1_(blur0, image);
        cv::Mat low1 = distL1_(blur1, image);
        cv::Mat low2 = distL1_(blur2, image);
        std::cout<<" --- low contrast"<<std::endl;
        // cv::imwrite("low2.png", low2);
        // Compute the triple sharpen
        cv::Mat sharpImage = sharpenTriple_(image,
         blur0, low0, thresholds[0], amounts[0],
         blur1, low1, thresholds[1], amounts[1],
         blur2, low2, thresholds[2], amounts[2]);
        std::cout<<" --- sharpen"<<std::endl;
        return sharpImage;
    }

    void finish::process(std::string burstPath, cv::Mat mergedBayer,int refIdx){
        // copy mergedBayer to rawReference
        std::cout<<"finish pipeline start ..."<<std::endl;

        this->refIdx = refIdx;
        this->burstPath = burstPath;
        this->mergedBayer = mergedBayer;//loadFromCSV(mergedBayerPath, CV_16UC1);
        load_rawPathList(burstPath);

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
        copy_mat_16U_2(mergedImg->libraw_processor->imgdata.rawdata.raw_image,this->mergedBayer);
        cv::Mat processedMerge = postprocess(mergedImg->libraw_processor,params.rawpyArgs);

// write merged image
        if(params.flags["writeMergedImage"]){
            std::cout<<"writing Merged img ..."<<std::endl;
            cv::Mat outputImg = convert16bit2_8bit_(processedMerge.clone());
            cv::cvtColor(outputImg, outputImg, cv::COLOR_RGB2BGR);
            cv::imwrite("mergedImg.jpg", outputImg);
        }

// write gamma merged image
        if(params.flags["writeMergedImage"]){
            std::cout<<"writing Gamma Merged img ..."<<std::endl;
            cv::Mat outputImg = gammasRGB(processedMerge.clone(),true);
            outputImg = convert16bit2_8bit_(outputImg);
            cv::cvtColor(outputImg, outputImg, cv::COLOR_RGB2BGR);
            cv::imwrite("mergedImgGamma.jpg", outputImg);
        }

// step 5. HDR tone mapping
// processedImage, gain, shortExposure, longExposure, fusedExposure = localToneMap(burstPath, processedImage, options)
        int gain;
        if(params.options.ltmGain){
            cv::Mat shortExposure, longExposure, fusedExposure;
            
            localToneMap(processedMerge, params.options,shortExposure,longExposure,fusedExposure,gain);
            std::cout<<"gain="<< gain<<std::endl;
            if(params.flags["writeShortExposure"]){
                std::cout<<"writing ShortExposure img ..."<<std::endl;
                cv::Mat outputImg = convert16bit2_8bit_(shortExposure);
                cv::imwrite("shortg.jpg", outputImg);
            }
            if(params.flags["writeLongExposure"]){
                std::cout<<"writing LongExposure img ..."<<std::endl;
                cv::Mat outputImg = convert16bit2_8bit_(longExposure);
                cv::imwrite("longg.jpg", outputImg);
            }
            if(params.flags["writeFusedExposure"]){
                std::cout<<"writing FusedExposure img ..."<<std::endl;
                cv::Mat outputImg = convert16bit2_8bit_(fusedExposure);
                cv::imwrite("fusedg.jpg", outputImg);
            }
            if(params.flags["writeLTMImage"]){
                std::cout<<"writing LTMImage ..."<<std::endl;
                cv::Mat outputImg = convert16bit2_8bit_(processedMerge.clone());
                cv::cvtColor(outputImg, outputImg, cv::COLOR_RGB2BGR);
                cv::imwrite("ltmGain.jpg", outputImg);
            }
            if(params.flags["writeLTMGamma"]){
                std::cout<<"writing LTMImage Gamma ..."<<std::endl;
                cv::Mat outputImg = gammasRGB(processedMerge.clone(),true);
                outputImg = convert16bit2_8bit_(outputImg);
                cv::cvtColor(outputImg, outputImg, cv::COLOR_RGB2BGR);
                cv::imwrite("ltmGain_gamma.jpg", outputImg);
            }
        }

// step 6 GTM: contrast enhancement / global tone mapping
        if(params.options.gtmContrast){
            processedMerge = enhanceContrast(processedMerge, params.options);
            std::cout<<"STEP 6 -- Apply GTM"<<std::endl;
        }

        // apply the final sRGB gamma curve
        processedMerge = gammasRGB(processedMerge.clone(),true);
        std::cout<<"-- Apply Gamma"<<std::endl;

        if(params.flags["writeGTMImage"]){
            std::cout<<"writing GTMImage ..."<<std::endl;
            cv::Mat outputImg = convert16bit2_8bit_(processedMerge.clone());
            cv::cvtColor(outputImg, outputImg, cv::COLOR_RGB2BGR);
            cv::imwrite("GTM_gamma.jpg", outputImg);
        }

// Step 7: sharpen
        cv::Mat processedImage = sharpenTriple(processedMerge.clone(), params.tuning, params.options);
        if(params.flags["writeFinalImage"]){
            std::cout<<"writing FinalImage ..."<<std::endl;
            cv::Mat outputImg = convert16bit2_8bit_(processedImage.clone());
            cv::cvtColor(outputImg, outputImg, cv::COLOR_RGB2BGR);
            cv::imwrite("FinalImage.jpg", outputImg);
        }
// write final ref
        if(params.flags["writeReferenceFinal"]){
            std::cout<<"writing Final Ref Image ..."<<std::endl;
            if(params.options.ltmGain){
                params.options.ltmGain = gain;
            }
            cv::Mat shortExposureRef, longExposureRef, fusedExposureRef;
            localToneMap(processedRefImage, params.options,shortExposureRef,longExposureRef,fusedExposureRef,gain);
            if(params.options.gtmContrast){ // contrast enhancement / global tone mapping
                processedRefImage = enhanceContrast(processedRefImage, params.options);
            }
            processedRefImage = gammasRGB(processedRefImage.clone(),true);
            // sharpen
            processedRefImage = sharpenTriple(processedRefImage.clone(), params.tuning, params.options);
            cv::Mat outputImg = convert16bit2_8bit_(processedRefImage.clone());
            cv::cvtColor(outputImg, outputImg, cv::COLOR_RGB2BGR);
            cv::imwrite("FinalReference.jpg", outputImg);
        }
// End of finishing
    }

    void finish::copy_mat_16U(cv::Mat& A, cv::Mat B){
        u_int16_t* ptr_A = (u_int16_t*)A.data;
        u_int16_t* ptr_B = (u_int16_t*)B.data;
        for(int r = 0; r < A.rows; r++) {
            for(int c = 0; c < A.cols; c++) {
                *(ptr_A+r*A.cols+c) = *(ptr_B+r*B.cols+c);
            }
        }
    }

    

    void finish::copy_rawImg2libraw(std::shared_ptr<LibRaw>& libraw_ptr, cv::Mat B){
        u_int16_t* ptr_A = (u_int16_t*)libraw_ptr->imgdata.rawdata.raw_image;
        u_int16_t* ptr_B = (u_int16_t*)B.data;
        for(int r = 0; r < B.rows; r++) {
            for(int c = 0; c < B.cols; c++) {
                *(ptr_A+r*B.cols+c) = *(ptr_B+r*B.cols+c);
            }
        }

    }
    
    
} // namespace hdrplus
