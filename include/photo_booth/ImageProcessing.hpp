
#ifndef PHOTO_BOOTH_IMAGE_PROCESSING_HPP
#define PHOTO_BOOTH_IMAGE_PROCESSING_HPP

#include <opencv2/core.hpp>
#include <vector>

namespace photo_booth {

cv::Mat swapRedBlueChannels(const cv::Mat& image);
cv::Mat invertImage(const cv::Mat& image);
cv::Mat quantizeImage(const cv::Mat& image, int levels);
cv::Mat dynamicContrast(const cv::Mat& image, int low_percentile, int high_percentile);
cv::Mat rotateImage(const cv::Mat& image, double angle);
cv::Mat grayscaleImage(const cv::Mat& image);
cv::Mat blurImage(const cv::Mat& image);
cv::Mat spatialFilter(const cv::Mat& image, int preset);
cv::Mat customSpatialFilter(const cv::Mat& image, const std::vector<double>& kernel, int kernel_size);
cv::Mat frequencyFilter(const cv::Mat& image, int filter_type, bool high_pass, double cutoff, int order);
cv::Mat affineTransform(const cv::Mat& image);
cv::Mat perspectiveTransform(const cv::Mat& image);
cv::Mat logPolarTransform(const cv::Mat& image);

}

#endif

