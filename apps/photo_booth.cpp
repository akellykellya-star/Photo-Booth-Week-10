
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>

#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include "photo_booth/AppConfig.hpp"
#include "photo_booth/ImageCapture.hpp"
#include "photo_booth/ImageProcessing.hpp"

namespace {

struct ProcessingState {
    bool contrast_enabled{true};
    bool inversion_enabled{false};
    bool quantization_enabled{false};
    bool rotation_enabled{false};

    // Week 10 additions.
    bool blur_enabled{false};
    bool edge_detection_enabled{false};
    int quantization_levels{8};
    double rotation_angle{0.0};
    int spatial_preset{-1};
    int frequency_type{-1};
    int transformation_type{-1};
    bool high_pass{false};
};

cv::Mat processFrame(
    const cv::Mat& frame,
    const photo_booth::ProcessingConfig& config,
    const ProcessingState& state)
{
    cv::Mat processed_frame = frame.clone();

    if (config.channel_swap_enabled) {
        processed_frame =
            photo_booth::swapRedBlueChannels(
                processed_frame);
    }

    if (state.contrast_enabled) {
        processed_frame =
            photo_booth::dynamicContrast(
                processed_frame,
                1,
                99);
    }

    if (state.inversion_enabled) {
        processed_frame =
            photo_booth::invertImage(
                processed_frame);
    }

    if (state.blur_enabled) {
        processed_frame =
            photo_booth::blurImage(
                processed_frame);
    }

    if (state.edge_detection_enabled) {
        processed_frame =
            photo_booth::edgeDetect(
                processed_frame);
    }

    if (state.quantization_enabled) {
        processed_frame =
            photo_booth::quantizeImage(
                processed_frame,
                state.quantization_levels);
    }

    if (state.rotation_enabled) {
        processed_frame =
            photo_booth::rotateImage(
                processed_frame,
                state.rotation_angle);
    }
    if (state.spatial_preset >= 0) {
    processed_frame = photo_booth::spatialFilter(
        processed_frame,
        state.spatial_preset);
}

if (state.frequency_type >= 0) {
    processed_frame = photo_booth::frequencyFilter(
        processed_frame,
        state.frequency_type,
        state.high_pass,
        30.0,
        2);
    }

    if (state.transformation_type == 0) {
        processed_frame = photo_booth::affineTransform(processed_frame);
    }
    else if (state.transformation_type == 1) {
        processed_frame = photo_booth::perspectiveTransform(processed_frame);
    }
    else if (state.transformation_type == 2) {
        processed_frame = photo_booth::logPolarTransform(processed_frame);
    }
    return processed_frame;
}

void showPreviewFrame(
    const cv::Mat& frame,
    const photo_booth::PreviewConfig& config,
    const ProcessingState& state)
{
    cv::Mat preview_frame = frame.clone();

    // Preview orientation is separate from processing rotation.
    switch (config.rotation) {
    case 0:
        break;

    case 90:
        cv::rotate(
            preview_frame,
            preview_frame,
            cv::ROTATE_90_CLOCKWISE);
        break;

    case 180:
        cv::rotate(
            preview_frame,
            preview_frame,
            cv::ROTATE_180);
        break;

    case 270:
        cv::rotate(
            preview_frame,
            preview_frame,
            cv::ROTATE_90_COUNTERCLOCKWISE);
        break;
    }

    if (config.mirror) {
        cv::flip(
            preview_frame,
            preview_frame,
            1);
    }

    cv::putText(
        preview_frame,
        "PHOTO BOOTH - WEEK 10",
        cv::Point(10, 25),
        cv::FONT_HERSHEY_SIMPLEX,
        0.65,
        cv::Scalar(255, 255, 255),
        1);

    std::string status =
        "C: Contrast " +
        std::string(
            state.contrast_enabled ? "ON" : "OFF");

    status +=
        "   I: Invert " +
        std::string(
            state.inversion_enabled ? "ON" : "OFF");

    status +=
        "   N: Quantize " +
        std::string(
            state.quantization_enabled ? "ON" : "OFF");

    cv::putText(
        preview_frame,
        status,
        cv::Point(10, 50),
        cv::FONT_HERSHEY_SIMPLEX,
        0.42,
        cv::Scalar(255, 255, 255),
        1);

    std::string parameters =
        "Levels: " +
        std::to_string(
            state.quantization_levels);

    parameters +=
        "   R: Rotation " +
        std::string(
            state.rotation_enabled ? "ON" : "OFF");

    parameters +=
        " (" +
        std::to_string(
            static_cast<int>(state.rotation_angle)) +
        " deg)";

    cv::putText(
        preview_frame,
        parameters,
        cv::Point(10, 75),
        cv::FONT_HERSHEY_SIMPLEX,
        0.42,
        cv::Scalar(255, 255, 255),
        1);

    std::string new_effects =
        "B: Blur " +
        std::string(
            state.blur_enabled ? "ON" : "OFF");

    new_effects +=
        "   E: Edge Detection " +
        std::string(
            state.edge_detection_enabled ? "ON" : "OFF");

    cv::putText(
        preview_frame,
        new_effects,
        cv::Point(10, 100),
        cv::FONT_HERSHEY_SIMPLEX,
        0.42,
        cv::Scalar(255, 255, 255),
        1);

    cv::putText(
        preview_frame,
        "C Contrast  I Invert  N Quantize  R Rotate",
        cv::Point(10, 125),
        cv::FONT_HERSHEY_SIMPLEX,
        0.42,
        cv::Scalar(255, 255, 255),
        1);

    cv::putText(
        preview_frame,
        "B Blur  E Edges  [ ] Levels  , . Angle",
        cv::Point(10, 148),
        cv::FONT_HERSHEY_SIMPLEX,
        0.42,
        cv::Scalar(255, 255, 255),
        1);

    cv::putText(
        preview_frame,
        "SPACE Capture   Q Quit",
        cv::Point(10, 171),
        cv::FONT_HERSHEY_SIMPLEX,
        0.42,
        cv::Scalar(255, 255, 255),
        1);

    cv::imshow(
        config.window_name,
        preview_frame);
}

bool handleKey(
    const int key,
    ProcessingState& state,
    const cv::Mat& processed_frame,
    const photo_booth::CaptureConfig& capture_config)
{
    switch (key) {
    case 27:
    case 'q':
    case 'Q':
        return false;

    case 'c':
    case 'C':
        state.contrast_enabled =
            !state.contrast_enabled;

        std::cout
            << "Dynamic contrast: "
            << (state.contrast_enabled ? "ON" : "OFF")
            << '\n';
        break;

    case 'i':
    case 'I':
        state.inversion_enabled =
            !state.inversion_enabled;

        std::cout
            << "Image inversion: "
            << (state.inversion_enabled ? "ON" : "OFF")
            << '\n';
        break;

    case 'n':
    case 'N':
        state.quantization_enabled =
            !state.quantization_enabled;

        std::cout
            << "Quantization: "
            << (state.quantization_enabled ? "ON" : "OFF")
            << " (" << state.quantization_levels
            << " levels)\n";
        break;

    case 'b':
    case 'B':
        state.blur_enabled =
            !state.blur_enabled;

        std::cout
            << "Gaussian blur: "
            << (state.blur_enabled ? "ON" : "OFF")
            << '\n';
        break;

    case 'e':
    case 'E':
        state.edge_detection_enabled =
            !state.edge_detection_enabled;

        std::cout
            << "Edge detection: "
            << (state.edge_detection_enabled ? "ON" : "OFF")
            << '\n';
        break;

    case ']':
        if (state.quantization_levels < 256) {
            state.quantization_levels *= 2;

            if (state.quantization_levels > 256) {
                state.quantization_levels = 256;
            }
        }

        std::cout
            << "Quantization levels: "
            << state.quantization_levels
            << '\n';
        break;

    case '[':
        if (state.quantization_levels > 2) {
            state.quantization_levels /= 2;
        }

        std::cout
            << "Quantization levels: "
            << state.quantization_levels
            << '\n';
        break;

    case 'r':
    case 'R':
        state.rotation_enabled =
            !state.rotation_enabled;

        std::cout
            << "Rotation: "
            << (state.rotation_enabled ? "ON" : "OFF")
            << " (" << state.rotation_angle
            << " degrees)\n";
        break;

    case ',':
        state.rotation_angle -= 5.0;

        if (state.rotation_angle < -180.0) {
            state.rotation_angle = -180.0;
        }

        std::cout
            << "Rotation angle: "
            << state.rotation_angle
            << " degrees\n";
        break;

    case '.':
        state.rotation_angle += 5.0;

        if (state.rotation_angle > 180.0) {
            state.rotation_angle = 180.0;
        }

        std::cout
            << "Rotation angle: "
            << state.rotation_angle
            << " degrees\n";
        break;

    case ' ':
        if (processed_frame.empty()) {
            std::cerr
                << "Unable to capture: processed frame is empty.\n";
            break;
        }

        if (!cv::imwrite(
                capture_config.output_filename,
                processed_frame)) {
            std::cerr
                << "Unable to save image to "
                << capture_config.output_filename
                << '\n';
        }
        else {
            std::cout
                << "Captured image: "
                << capture_config.output_filename
                << '\n';
        }

        break;

    default:
        break;
    }

    return true;
}

} // namespace

int main(
    int argc,
    char* argv[])
{
    try {
        std::filesystem::path config_path{
            "config.toml"};

        if (argc > 2) {
            std::cerr
                << "Usage: "
                << argv[0]
                << " [config.toml]\n";

            return EXIT_FAILURE;
        }

        if (argc == 2) {
            const std::string argument{
                argv[1]};

            if (argument == "-h" ||
                argument == "--help") {
                std::cout
                    << "Usage: "
                    << argv[0]
                    << " [config.toml]\n\n"
                    << "Runs the semester photo-booth "
                    << "image-processing application.\n";

                return EXIT_SUCCESS;
            }

            config_path = argument;
        }

        const auto config =
            photo_booth::loadConfig(
                config_path);

        photo_booth::ImageCapture camera(
            photo_booth::makeImageCaptureConfiguration(
                config.camera));

        if (!camera.open()) {
            std::cerr
                << camera.errorMessage()
                << '\n';

            return EXIT_FAILURE;
        }

        std::cout
            << camera
            << '\n';

        cv::namedWindow(
            config.preview.window_name,
            cv::WINDOW_AUTOSIZE);

        ProcessingState processing_state;

        while (true) {
            if (!camera.read()) {
                std::cerr
                    << camera.errorMessage()
                    << '\n';

                return EXIT_FAILURE;
            }

            cv::Mat processed_frame =
                processFrame(
                    camera.image(),
                    config.processing,
                    processing_state);

            showPreviewFrame(
                processed_frame,
                config.preview,
                processing_state);

            const int key =
                cv::waitKey(1);

            if (!handleKey(
                    key,
                    processing_state,
                    processed_frame,
                    config.capture)) {
                break;
            }
        }

        cv::destroyAllWindows();
    }
    catch (const cv::Exception& error) {
        std::cerr
            << "OpenCV error: "
            << error.what()
            << '\n';

        return EXIT_FAILURE;
    }
    catch (const std::exception& error) {
        std::cerr
            << "Error: "
            << error.what()
            << '\n';

        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
