#include "photo_booth/ImageProcessing.hpp"
#include <opencv2/imgproc.hpp>
#include <cmath>
#include <stdexcept>
#include <string>
#include <algorithm>

namespace photo_booth {
namespace {

void validateImage(
    const cv::Mat& image,
    const char* function_name)
{
    if (image.empty()) {
        throw std::invalid_argument(
            std::string(function_name) +
            ": input image is empty");
    }
    if (image.type() != CV_8UC3) {
        throw std::invalid_argument(
            std::string(function_name) +
            ": input image must be CV_8UC3");
    }
}

} // namespace

cv::Mat swapRedBlueChannels(
    const cv::Mat& image)
{
    validateImage(image, "swapRedBlueChannels()");

    cv::Mat output;

    cv::cvtColor(
        image,
        output,
        cv::COLOR_BGR2RGB);

    return output;
}

cv::Mat invertImage(
    const cv::Mat& image)
{
    validateImage(image, "invertImage()");
    cv::Mat output;

    cv::bitwise_not(
        image,
        output);

    return output;
}

cv::Mat quantizeImage(
    const cv::Mat& image,
    int levels)
{
    validateImage(image, "quantizeImage()");

    if (levels <= 0 || levels > 256) {
        throw std::invalid_argument(
            "quantizeImage(): "
            "levels must be between 1 and 256");
    }

    cv::Mat output(
        image.size(),
        image.type());

    const double step =
        256.0 / static_cast<double>(levels);
    unsigned char lookup[256];

    for (int value = 0; value < 256; value++) {
        const int level =
            static_cast<int>(value / step);

        int quantized =
            static_cast<int>(
                level * step + step / 2.0);

        if (quantized < 0) {
            quantized = 0;
        }

        if (quantized > 255) {
            quantized = 255;
        }

        lookup[value] =
            static_cast<unsigned char>(quantized);
    }

    for (int row = 0; row < image.rows; row++) {
        for (int column = 0; column < image.cols; column++) {
            const cv::Vec3b& input_pixel =
                image.at<cv::Vec3b>(row, column);

            cv::Vec3b& output_pixel =
                output.at<cv::Vec3b>(row, column);

            output_pixel[0] = lookup[input_pixel[0]];
            output_pixel[1] = lookup[input_pixel[1]];
            output_pixel[2] = lookup[input_pixel[2]];
        }
    }

    return output;
}

cv::Mat dynamicContrast(
    const cv::Mat& image,
    int low_percentile,
    int high_percentile)
{
    validateImage(image, "dynamicContrast()");

    if (low_percentile < 0 || low_percentile >= 100) {
        throw std::invalid_argument(
            "dynamicContrast(): "
            "low percentile must be between 0 and 99");
    }

    if (high_percentile <= 0 || high_percentile > 100) {
        throw std::invalid_argument(
            "dynamicContrast(): "
            "high percentile must be between 1 and 100");
    }

    if (low_percentile >= high_percentile) {
        throw std::invalid_argument(
            "dynamicContrast(): "
            "low percentile must be less than high percentile");
    }

    cv::Mat output(
        image.size(),
        image.type());

    const int total_pixels =
        image.rows * image.cols;
    int histogram[3][256] = {};

    for (int row = 0; row < image.rows; row++) {
        for (int column = 0; column < image.cols; column++) {
            const cv::Vec3b& pixel =
                image.at<cv::Vec3b>(row, column);

            histogram[0][pixel[0]]++;
            histogram[1][pixel[1]]++;
            histogram[2][pixel[2]]++;
        }
    }

    unsigned char lookup[3][256];

    for (int channel = 0; channel < 3; channel++) {
        const int low_count =
            total_pixels * low_percentile / 100;

        const int high_count =
            total_pixels * high_percentile / 100;

        int count = 0;
        int low_value = 0;

        for (int value = 0; value < 256; value++) {
            count += histogram[channel][value];

            if (count > low_count) {
                low_value = value;
                break;
            }
        }

        count = 0;
        int high_value = 255;

        for (int value = 0; value < 256; value++) {
            count += histogram[channel][value];

            if (count >= high_count) {
                high_value = value;
                break;
            }
        }

        if (high_value <= low_value) {
            for (int value = 0; value < 256; value++) {
                lookup[channel][value] =
                    static_cast<unsigned char>(value);
            }
            continue;
        }

        for (int value = 0; value < 256; value++) {
            int output_value;

            if (value <= low_value) {
                output_value = 0;
            }
            else if (value >= high_value) {
                output_value = 255;
            }
            else {
                output_value =
                    static_cast<int>(
                        (value - low_value) * 255.0 /
                        (high_value - low_value));
            }

            if (output_value < 0) {
                output_value = 0;
            }

            if (output_value > 255) {
                output_value = 255;
            }

            lookup[channel][value] =
                static_cast<unsigned char>(output_value);
        }
    }

    for (int row = 0; row < image.rows; row++) {
        for (int column = 0; column < image.cols; column++) {
            const cv::Vec3b& input_pixel =
                image.at<cv::Vec3b>(row, column);

            cv::Vec3b& output_pixel =
                output.at<cv::Vec3b>(row, column);

            output_pixel[0] =
                lookup[0][input_pixel[0]];

            output_pixel[1] =
                lookup[1][input_pixel[1]];

            output_pixel[2] =
                lookup[2][input_pixel[2]];
        }
    }

    return output;
}

cv::Mat rotateImage(
    const cv::Mat& image,
    double angle)
{
    validateImage(image, "rotateImage()");

    cv::Mat output(
        image.size(),
        image.type(),
        cv::Scalar(0, 0, 0));

    const double radians =
        angle * CV_PI / 180.0;

    const double cosine = std::cos(radians);
    const double sine = std::sin(radians);

    const double center_x =
        (image.cols - 1) / 2.0;

    const double center_y =
        (image.rows - 1) / 2.0;

    for (int row = 0; row < output.rows; row++) {
        for (int column = 0; column < output.cols; column++) {
            const double x = column - center_x;
            const double y = row - center_y;

            const double source_x =
                cosine * x + sine * y + center_x;

            const double source_y =
                -sine * x + cosine * y + center_y;

            const int source_column =
                static_cast<int>(std::round(source_x));

            const int source_row =
                static_cast<int>(std::round(source_y));

            if (source_column < 0 ||
                source_column >= image.cols ||
                source_row < 0 ||
                source_row >= image.rows) {
                continue;
            }

            output.at<cv::Vec3b>(row, column) =
                image.at<cv::Vec3b>(
                    source_row,
                    source_column);
        }
    }

    return output;
}

cv::Mat blurImage(
    const cv::Mat& image)
{
    validateImage(image, "blurImage()");

    cv::Mat output;

    cv::GaussianBlur(
        image,
        output,
        cv::Size(5, 5),
        0.0);

    return output;
}

cv::Mat edgeDetect(
    const cv::Mat& image)
{
    validateImage(image, "edgeDetect()");

    cv::Mat gray;
    cv::Mat edges;
    cv::Mat output;

    cv::cvtColor(
        image,
        gray,
        cv::COLOR_BGR2GRAY);

    cv::Canny(
        gray,
        edges,
        100,
        200);

    cv::cvtColor(
        edges,
        output,
        cv::COLOR_GRAY2BGR);

    return output;
}

cv::Mat spatialFilter(const cv::Mat& image, int preset)
{
    validateImage(image, "spatialFilter()");

    cv::Mat output;
    cv::Mat kernel;

    switch (preset) {
    case 0:
        cv::blur(image, output, cv::Size(3, 3));
        break;

    case 1:
        cv::GaussianBlur(
            image,
            output,
            cv::Size(5, 5),
            1.0);
        break;

    case 2:
        kernel = (cv::Mat_<float>(3, 3) <<
            0, -1, 0,
            -1, 5, -1,
            0, -1, 0);

        cv::filter2D(image, output, -1, kernel);
        break;

    case 3:
        kernel = (cv::Mat_<float>(3, 3) <<
            -1, -1, -1,
            -1, 8, -1,
            -1, -1, -1);

        cv::filter2D(image, output, -1, kernel);
        break;

    case 4:
        kernel = (cv::Mat_<float>(3, 3) <<
            -2, -1, 0,
            -1, 1, 1,
            0, 1, 2);

        cv::filter2D(image, output, -1, kernel);
        break;

    default:
        throw std::invalid_argument(
            "Invalid spatial filter preset");
    }

    return output;
}

cv::Mat customSpatialFilter(
    const cv::Mat& image,
    const std::vector<double>& kernel,
    int kernel_size)
{
    validateImage(image, "customSpatialFilter()");

    if (kernel_size <= 0 ||
        kernel_size % 2 == 0 ||
        kernel.size() !=
            static_cast<std::size_t>(kernel_size * kernel_size)) {
        throw std::invalid_argument(
            "Kernel must have an odd size and matching coefficients");
    }

    cv::Mat kernel_matrix(
        kernel_size,
        kernel_size,
        CV_32F);

    for (int row = 0; row < kernel_size; ++row) {
        for (int column = 0; column < kernel_size; ++column) {
            kernel_matrix.at<float>(row, column) =
                static_cast<float>(
                    kernel[row * kernel_size + column]);
        }
    }

    cv::Mat output;

    cv::filter2D(
        image,
        output,
        -1,
        kernel_matrix);

    return output;
}

cv::Mat frequencyFilter(
    const cv::Mat& image,
    int filter_type,
    bool high_pass,
    double cutoff,
    int order)
{
    validateImage(image, "frequencyFilter()");

    if (filter_type < 0 || filter_type > 2 ||
        cutoff <= 0.0 || order <= 0) {
        throw std::invalid_argument(
            "Invalid frequency filter parameters");
    }

    // Convert to grayscale before preparing the DFT input.
    cv::Mat gray;
    cv::cvtColor(
        image,
        gray,
        cv::COLOR_BGR2GRAY);

    cv::Mat planes[] = {
        cv::Mat::zeros(image.size(), CV_32F),
        cv::Mat::zeros(image.size(), CV_32F)
    };

    gray.convertTo(
        planes[0],
        CV_32F);

    cv::Mat complex_image;

    cv::merge(
        planes,
        2,
        complex_image);

    cv::dft(
        complex_image,
        complex_image);

    cv::Mat mask(
        image.size(),
        CV_32F);

    for (int row = 0; row < image.rows; ++row) {
        for (int column = 0; column < image.cols; ++column) {
            const double dx = std::min(
                column,
                image.cols - column);

            const double dy = std::min(
                row,
                image.rows - row);

            const double distance =
                std::sqrt(dx * dx + dy * dy);

            double value = 0.0;

            if (filter_type == 0) {
                // Ideal filter.
                value = distance <= cutoff ? 1.0 : 0.0;
            }
            else if (filter_type == 1) {
                // Butterworth filter.
                value = 1.0 / (
                    1.0 +
                    std::pow(
                        distance / cutoff,
                        2 * order));
            }
            else {
                // Gaussian filter.
                value = std::exp(
                    -(distance * distance) /
                    (2.0 * cutoff * cutoff));
            }

            if (high_pass) {
                value = 1.0 - value;
            }

            mask.at<float>(row, column) =
                static_cast<float>(value);
        }
    }

    cv::Mat frequency_planes[2];

    cv::split(
        complex_image,
        frequency_planes);

    frequency_planes[0] =
        frequency_planes[0].mul(mask);

    frequency_planes[1] =
        frequency_planes[1].mul(mask);

    cv::merge(
        frequency_planes,
        2,
        complex_image);

    cv::Mat inverse;

    cv::idft(
        complex_image,
        inverse,
        cv::DFT_REAL_OUTPUT | cv::DFT_SCALE);

    cv::normalize(
        inverse,
        inverse,
        0,
        255,
        cv::NORM_MINMAX);

    inverse.convertTo(
        inverse,
        CV_8U);

    cv::Mat output;

    cv::cvtColor(
        inverse,
        output,
        cv::COLOR_GRAY2BGR);

    return output;
}

cv::Mat affineTransform(const cv::Mat& image)
{
    validateImage(image, "affineTransform()");

    cv::Point2f source[] = {
        cv::Point2f(0, 0),
        cv::Point2f(
            static_cast<float>(image.cols - 1),
            0),
        cv::Point2f(
            0,
            static_cast<float>(image.rows - 1))
    };

    cv::Point2f destination[] = {
        cv::Point2f(
            image.cols * 0.1f,
            image.rows * 0.1f),
        cv::Point2f(
            image.cols * 0.9f,
            image.rows * 0.05f),
        cv::Point2f(
            image.cols * 0.2f,
            image.rows * 0.9f)
    };

    cv::Mat matrix =
        cv::getAffineTransform(source, destination);

    cv::Mat output;

    cv::warpAffine(
        image,
        output,
        matrix,
        image.size());

    return output;
}

cv::Mat perspectiveTransform(const cv::Mat& image)
{
    validateImage(image, "perspectiveTransform()");

    cv::Point2f source[] = {
        cv::Point2f(0, 0),
        cv::Point2f(
            static_cast<float>(image.cols - 1),
            0),
        cv::Point2f(
            static_cast<float>(image.cols - 1),
            static_cast<float>(image.rows - 1)),
        cv::Point2f(
            0,
            static_cast<float>(image.rows - 1))
    };

    cv::Point2f destination[] = {
        cv::Point2f(
            image.cols * 0.1f,
            image.rows * 0.1f),
        cv::Point2f(
            image.cols * 0.9f,
            0),
        cv::Point2f(
            image.cols * 0.95f,
            image.rows * 0.9f),
        cv::Point2f(
            image.cols * 0.05f,
            static_cast<float>(image.rows - 1))
    };

    cv::Mat matrix =
        cv::getPerspectiveTransform(source, destination);

    cv::Mat output;

    cv::warpPerspective(
        image,
        output,
        matrix,
        image.size());

    return output;
}

cv::Mat logPolarTransform(const cv::Mat& image)
{
    validateImage(image, "logPolarTransform()");

    cv::Mat output;

    const cv::Point2f center(
        image.cols / 2.0f,
        image.rows / 2.0f);

    const double scale = image.cols /
        std::log(
            std::max(
                2.0,
                static_cast<double>(image.cols) / 2.0));

    cv::logPolar(
        image,
        output,
        center,
        scale,
        cv::INTER_LINEAR | cv::WARP_FILL_OUTLIERS);

    return output;
}

} // namespace photo_booth
