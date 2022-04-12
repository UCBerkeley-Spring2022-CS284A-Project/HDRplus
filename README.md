# HDRplus
[Under Develop] HDR+ Paper reimplementation, UC Berkeley Spring 2022 CS184/284A Final Project


Title: HDR+ Denoising Pipeline

Team members: Haohua Lyu, Xiao Song, Siming Liu, Cyrus Vachha

Summary: Taking multi-shot / burst sequences of images as input, we aim to implement the HDR+ denoising algorithm to align and merge the sequence to produce a temporally and spatially denoised image, which in turn become suitable for HDR image generation. 


1. Problem Description

(1) main idea: We take the HDR+ paper as reference and try to implement the generation pipeline in C++, with emphasis on the denoising algorithm. 
Why it is important: Smartphones have limited camera hardware, making computational photography an important part of smartphones. Taking photos on smartphones can result in parts of the image being too bright or too dark given the limited range. HDR+ allows us to create an image that has a higher amount of visual detail in more lighting conditions.

2) Where it is challenging: The HDR pipeline is composed of multiple steps, which make it a time consuming process. Balancing the quality of each step and the runtime speed for each part is a challenging task. Part of the generation process requires hardware (camera) experiment data for tuning, which could also be time consuming and hardware dependent. 

2. Goals and Deliverables

1) Deliverable / Goal: A complete HDR pipeline that takes a burst of bayer raw image, and outputs a denoised HDR image. 
2) Future work: modify the HDR+ algorithm a little to enable RGB input support. So that user can use our pipeline with their own burst of RGB input. The initial algorithm is designed to take raw bayer images as input. In order to use RGB as input, we need to make some modifications to the pipeline. 
3) Measure Performance: compare HDR image produced by our pipeline with raw bayer image converted to RGB directly. Compare our HDR image result with the result given in the HDR+ paper \cite{hdr+}. 


3. Schedule
1) First two week: working on the coding part. Xiao work on the 
2) Third week: debugging the code and validating functionality.
3) Fourth week : run experiment & write report. 


4. Resources
1) related works
Burst photography for high dynamic range and low-light imaging on mobile cameras
An Analysis and Implementation of the HDR+ Burst Denoising Method
Burst photography for high dynamic range and low-light imaging on mobile cameras Supplemental Material

2) software and hardware support
Ubuntu 18, OpenCV, CMake, LibRaw

