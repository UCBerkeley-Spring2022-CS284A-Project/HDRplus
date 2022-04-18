
## Step by step installation

1. [OpenCV](git@github.com:opencv/opencv.git) v3.4.0 build from source through CMake
   1. v4.x+ version have some MacOS compatable issue
   2. v4.x+ version work on Ubuntu


2. LibRaw
   1. MacOS : `brew install libraw libpng libjpeg`
   2. Ubuntu : 
   
      step 1: download libraw package here: https://www.libraw.org/download
      
      step 2: extract the pkg using `tar xzvf LibRaw-X.YY.tar.gz`
      
      step 3: 
      ```shell
         cd LibRaw-X.YY
         ./configure # with optional args
         make
      ```
               
      step 4: `sudo make install`
      
      step 5: change the target_link_libraries in cmakelist file to the following:
      
      ```shell
      target_link_libraries(${PROJECT_NAME} 
              ${OpenCV_LIBS}
              ${LIBRAW_LIBRARY})
              ```

3. Run CMake to build
   ```shell
   mkdir build
   cd build
   # Default release build will be used
   cmake ..
   make -j 10
   ```
