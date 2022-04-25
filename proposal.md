---
title: Proposal
layout: default
---

## HDR+ Denoising Pipeline

Summary: Taking multi-shot / burst sequences of images as input, we aim to implement a burst denoising pipeline (based on Google's HDR+) to produce a temporally and spatially denoised image, which in turn become suitable for HDR image generation. 

### Team members
Haohua Lyu ([haohua@berkeley.edu](mailto:haohua@berkeley.edu))

Xiao Song ([xiaosx@berkeley.edu](mailto:xiaosx@berkeley.edu))

Cyrus Vachha ([cvachha@berkeley.edu](mailto:cvachha@berkeley.edu))

Siming Liu ([liusm220036@berkeley.edu](mailto:liusm220036@berkeley.edu))


<img src="img/pipeline.png">

---

## Problem Description

* **Problem & Importance**
Smartphones have limited camera hardware, making computational photography an important part of smartphones. Taking photos on smartphones can result in parts of the image being over- or under-exposed given the limited range. The HDR+ algorithm allows us to create an image that has a higher amount of visual detail in more lighting conditions and remove noise at the same time.

* **Main idea**
We take [Google's HDR+ paper](https://static.googleusercontent.com/media/hdrplusdata.org/en//hdrplus.pdf) as reference and try to implement the generation pipeline in C++, with emphasis on the denoising algorithm. The algorithmic insight is to take a burst of raw frames, all under-exposed to keep the details captured, and then align and merge them to remove noise. Once merged, the image would have an improved range and good details; we can obtain a high quality HDR image through post-processing.
 
* **Challenges**
The HDR+ pipeline is composed of multiple steps, which make it a time consuming process. Balancing the quality of each step and the runtime speed for each part is a challenging task. Part of the generation process requires hardware (camera) experiment data for tuning, which could also be time consuming and hardware dependent. 

---
## Goals and Deliverables

* **Planned Deliverable / Goal**: A complete HDR pipeline implemented in C++ that takes a burst of bayer raw image as inputs and outputs a denoised HDR image. 
  
* **Implementation Pipeline**: We would first slice images in the burst sequence into tiles, align the movement of tiles, remove noise from the tiles, and merge them into a single image. We would apply additional post-processing steps like tone-mapping to achieve the HDR effect. 

* **Potential Deliverable / Future Work**: If time allows, we would like to modify the HDR+ algorithm to support RGB images as inputs, as the initial algorithm is designed to only take raw bayer images. Users can then use our pipeline with their own burst of RGB inputs. In order to do so, we would need to modify the APIs to account for the different channels, as well as applying additional merging. We could also implement [the exposure bracketing technique](https://ai.googleblog.com/2021/04/hdr-with-bracketing-on-pixel-phones.html) to overcome some drawbacks of the original HDR+ algorithm.

* **Measure Performance**: We would compare HDR images produced by our pipeline with raw bayer image converted to RGB directly. We could also compare our HDR image result with the result given in the HDR+ paper, using metrics like Peak Signal-to-Noise Ratio (PSNR).  

---
## Schedule

- First two week (Apr 11 - Apr 24): We aim to do additional research and implement the code in C++. 

- Third week (Apr 25 - May 1): We would focus on debugging the code, tuning coefficients, and validating functionality.

- Fourth week and after (May 2 - EOS): We will run experiments, prepare the presentation, and write final report. 

---
## Resources

### Related works
1. Burst photography for high dynamic range and low-light imaging on mobile cameras, [Google Research](https://static.googleusercontent.com/media/hdrplusdata.org/en//hdrplus.pdf)

2. An Analysis and Implementation of the HDR+ Burst Denoising Method, [GoPro](https://arxiv.org/abs/2110.09354)

3. Burst photography for high dynamic range and low-light imaging on mobile cameras, [Supplemental Material](https://static.googleusercontent.com/media/hdrplusdata.org/en//hdrplus_supp.pdf)

### Software and Hardware support
Ubuntu 18, OpenCV, CMake, LibRaw
