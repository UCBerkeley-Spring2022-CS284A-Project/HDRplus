
## Step by step installation

1. [OpenCV](git@github.com:opencv/opencv.git) v4.5.5 build from source through CMake


2. LibRaw
   1. MacOS : `brew install libraw libpng libjpeg`
   2. Ubuntu : TODO

3. Run CMake to build
   ```shell
   mkdir build
   cd build
   cmake ..
   make -j 10
   ```