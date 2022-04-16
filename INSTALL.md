
## Step by step installation

1. [OpenCV](git@github.com:opencv/opencv.git) v4.5.5 build from source through CMake


2. LibRaw
   1. MacOS : `brew install libraw libpng libjpeg`
   2. Ubuntu : 
      step 1: download libraw package here: https://www.libraw.org/download
      step 2: extract the pkg using `tar xzvf LibRaw-X.YY.tar.gz`
      step 3: cd LibRaw-X.YY
               ./configure # with optional args
               make
      step 4: sudo make install
      step 5: change the target_link_libraries in cmakelist file to the following:
      target_link_libraries(${PROJECT_NAME} 
              ${OpenCV_LIBS}
              ${LIBRAW_LIBRARY})

3. Run CMake to build
   ```shell
   mkdir build
   cd build
   cmake ..
   make -j 10
   ```
