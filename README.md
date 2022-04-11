# HDRplus
[Under Develop] HDR+ Paper reimplementation, UC Berkeley Spring 2022 CS184/284A Final Project

Please create a proposal webpage with the following sections.
Title, Summary and Team Members
Provide us a descriptive title, 2-3 sentences that summarize your project, and list your three team members.
Title: HDR+ Denoising Pipeline
Summary: Taking multi-shot / burst sequences of images as input, we aim to implement the HDR+ denoising algorithm to align and merge the sequence to produce a temporally and spatially denoised image, which in turn become suitable for HDR image generation. 
Team members: Haohua Lyu, Xiao Song, Siming Liu, Cyrus Vachha
Problem Description
Here you should provide the context for your idea.  
Describe the problem that you are trying to solve, 
Give us a general idea on how you are going to solve it.: We take the HDR+ paper as reference and try to implement the generation pipeline in C++, with emphasis on the denoising algorithm. 
Why it is important: Smartphones have limited camera hardware, making computational photography an important part of smartphones. Taking photos on smartphones can result in parts of the image being too bright or too dark given the limited range. HDR+ allows us to create an image that has a higher amount of visual detail in more lighting conditions.
Where it is challenging: The HDR pipeline is composed of multiple steps, which make it a time consuming process. Balancing the quality of each step and the runtime speed for each part is a challenging task. Part of the generation process requires hardware (camera) experiment data for tuning, which could also be time consuming and hardware dependent. 
Goals and Deliverables
This is the most important part of your proposal. You should carefully think through what you are trying to accomplish, what results you are going for, and why you think you can accomplish those goals. For example:
Since this is a graphics class you will likely define the kind of images you will create (e.g. including a photo of a new lighting effect you will simulate).
If you are working on an interactive system, describe what demo you will create.
Define how you will measure the quality / performance of your system (e.g. graphs showing speedup, or quantifying accuracy). It may not be possible to define precise target metrics at this time, but we encourage you to try.
What questions do you plan to answer with your analysis?
You should break this section into two parts: (1) what you plan to deliver, and (2) what you hope to deliver. In (1), describe what you believe you must accomplish to have a successful project and achieve the grade you expect (i.e. your baseline plan -- planning for some unexpected problems would make sense). In (2), describe what you hope to achieve if things go well and you get ahead of schedule (your aspirational plan).


Deliverable / Goal: A complete HDR pipeline that takes a burst of bayer raw image, and outputs a denoised HDR image. 
Future work: modify the HDR+ algorithm a little to enable RGB input support. So that user can use our pipeline with their own burst of RGB input. The initial algorithm is designed to take raw bayer images as input. In order to use RGB as input, we need to make some modifications to the pipeline. 
Measure Performance: compare HDR image produced by our pipeline with raw bayer image converted to RGB directly. Compare our HDR image result with the result given in the HDR+ paper \cite{hdr+}. 
Schedule
In this section you should organize and plan the tasks and subtasks that your team will execute. Since presentations are ~4 weeks from the due-date of the proposal, you should include a set of tasks for every week.
4-5 page paper.
First two week: working on the coding part. Xiao work on the 
Third week: debugging the code and validating functionality.
Fourth week : run experiment & write report. 
Resources
List what resources, e.g. books, papers and/or online resources that are references for your project. List the computing platform, hardware and software resources that you will use for your project. You have a wide latitude here to use what you have access to, but be aware that you will have to support and trouble-shoot on your platform yourselves. If you are starting from an existing piece of code or system, describe and provide a pointer to it here.


Burst photography for high dynamic range and low-light imaging on mobile cameras
An Analysis and Implementation of the HDR+ Burst Denoising Method
Burst photography for high dynamic range and low-light imaging on mobile cameras Supplemental Material


Ubuntu 18, OpenCV, CMake, LibRaw

