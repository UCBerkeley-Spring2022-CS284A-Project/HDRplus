#include <string>
#include <cstdio>
#include <utility> // std::pair, std::makr_pair
#include <memory> // std::shared_ptr
#include <stdexcept> // std::runtime_error
#include <opencv2/opencv.hpp> // all opencv header
#include <libraw/libraw.h>
#include <exiv2/exiv2.hpp> // exiv2
#include "hdrplus/bayer_image.h"
#include "hdrplus/utility.h" // box_filter_kxk
namespace hdrplus
{

bayer_image::bayer_image( const std::string& bayer_image_path )
{
    libraw_processor = std::make_shared<LibRaw>();

    // Open RAW image file
    int return_code;
    if ( ( return_code = libraw_processor->open_file( bayer_image_path.c_str() ) ) != LIBRAW_SUCCESS )
    {
        libraw_processor->recycle();
        throw std::runtime_error("Error opening file " + bayer_image_path + " " + libraw_strerror( return_code ));
    }

    // Unpack the raw image
    if ( ( return_code = libraw_processor->unpack() ) != LIBRAW_SUCCESS )
    {
        throw std::runtime_error("Error unpack file " + bayer_image_path + " " + libraw_strerror( return_code ));
    }

    // Get image basic info
    width = int( libraw_processor->imgdata.rawdata.sizes.raw_width );
    height = int( libraw_processor->imgdata.rawdata.sizes.raw_height );

    // Read exif tags
    Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(bayer_image_path);
    assert(image.get() != 0);
    image->readMetadata();
    Exiv2::ExifData &exifData = image->exifData();
    if (exifData.empty()) {
        std::string error(bayer_image_path);
        error += ": No Exif data found in the file";
        std::cout << error << std::endl;
    }

    white_level = exifData["Exif.Image.WhiteLevel"].toLong();
    black_level_per_channel.resize( 4 );
    black_level_per_channel.at(0) = exifData["Exif.Image.BlackLevel"].toLong(0);
    black_level_per_channel.at(1) = exifData["Exif.Image.BlackLevel"].toLong(1);
    black_level_per_channel.at(2) = exifData["Exif.Image.BlackLevel"].toLong(2);
    black_level_per_channel.at(3) = exifData["Exif.Image.BlackLevel"].toLong(3);
    iso = exifData["Exif.Image.ISOSpeedRatings"].toLong();

    // Create CV mat
    // https://answers.opencv.org/question/105972/de-bayering-a-cr2-image/
    // https://www.libraw.org/node/2141
    raw_image = cv::Mat( height, width, CV_16U, libraw_processor->imgdata.rawdata.raw_image ).clone(); // changed the order of width and height

    // Adding salt and pepper noise
    cv::Mat saltpepper_noise = cv::Mat::zeros( raw_image.rows, raw_image.cols, CV_16UC3 );
    cv::randu( saltpepper_noise, 0, 255 );
    cv::Mat black = saltpepper_noise < 5;
    cv::Mat white = saltpepper_noise > 250;
    cv::Mat saltpepper_raw_image = raw_image.clone();
    saltpepper_raw_image.setTo(255,white);
    saltpepper_raw_image.setTo(0,black);

    raw_image = saltpepper_raw_image;

    // 2x2 box filter
    grayscale_image = box_filter_kxk<uint16_t, 2>( raw_image );

    #ifndef NDEBUG
    printf("%s::%s read bayer image %s with\n width %zu\n height %zu\n iso %.3f\n white level %d\n black level %d %d %d %d\n", \
        __FILE__, __func__, bayer_image_path.c_str(), width, height, iso, white_level, \
        black_level_per_channel[0], black_level_per_channel[1], black_level_per_channel[2], black_level_per_channel[3] );
    fflush( stdout );
    #endif
}

std::pair<double, double> bayer_image::get_noise_params() const
{
    // Set ISO to 100 if not positive
    double iso_ = iso <= 0 ? 100 : iso;

    // Calculate shot noise and read noise parameters w.r.t ISO 100
    double lambda_shot_p = iso_ / 100.0f * baseline_lambda_shot;
	double lambda_read_p = (iso_ / 100.0f) * (iso_ / 100.0f) * baseline_lambda_read;

    double black_level = (black_level_per_channel[0] + \
                       black_level_per_channel[1] + \
                       black_level_per_channel[2] + \
                       black_level_per_channel[3]) / 4.0;

    // Rescale shot and read noise to normal range
    double lambda_shot = lambda_shot_p * (white_level - black_level);
    double lambda_read = lambda_read_p * (white_level - black_level) * (white_level - black_level);

    // return pair
    return std::make_pair(lambda_shot, lambda_read);
}

}
