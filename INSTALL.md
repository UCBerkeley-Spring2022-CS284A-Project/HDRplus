
## Step by step installation

Platform: Ubuntu 20.x

1. [OpenCV](git@github.com:opencv/opencv.git) v4.x+ recommended
   `sudo apt install libopencv-dev`


2. LibRaw: install from source is recommended

   step 1: download libraw package here: https://www.libraw.org/download
   
   step 2: extract the pkg using `tar xzvf LibRaw-X.YY.tar.gz`
   
   step 3: 
   ```shell
      cd LibRaw-X.YY
      autoreconf -f -i
      ./configure # with optional args
      make -j 10
   ```

   step 4: `sudo make install`
   

3. Install `exiv2` : `apt install libexiv2-dev`


4. Run CMake to build
   ```shell
   mkdir build
   cd build
   # Default release build will be used
   cmake ..
   make -j 10
   ```
