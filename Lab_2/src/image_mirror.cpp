#include <iostream>
#include <string>
#include <opencv2/opencv.hpp>

using namespace std;

int main(int argc, char* argv[]) {

    if (argc < 4) {
        cerr << "Usage: " << argv[0] << " <input ppm file> <output ppm file> <mode>\n";
        cerr << "Modes:\n";
        cerr << "  horizontal or h  - Mirror horizontally\n";
        cerr << "  vertical or v    - Mirror vertically\n";
        return 1;
    }

    string input_filename = argv[1];
    string output_filename = argv[2];
    string mode = argv[3];

    bool horizontal = false;
    bool vertical = false;

    if (mode == "horizontal" || mode == "h") {
        horizontal = true;
    } else if (mode == "vertical" || mode == "v") {
        vertical = true;
    } else {
        cerr << "Error: Invalid mode '" << mode << "'\n";
        cerr << "Valid modes: horizontal, h, vertical, v\n";
        return 1;
    }

    cv::Mat image = cv::imread(input_filename);

    if (image.empty()) {
        std::cerr << "Error: Could not open or find the image '" << input_filename << "'" << std::endl;
        return -1;
    }

    // Create a new empty image with the same size and type as the original
    cv::Mat result_image = cv::Mat(image.rows, image.cols, image.type());

    std::cout << "Processing image: " << image.cols << "x" << image.rows
              << " with " << image.channels() << " channels." << std::endl;


    if (image.channels() == 3) {
        for (int y = 0; y < image.rows; ++y) {
            for (int x = 0; x < image.cols; ++x) {

                // Get the pixel at (y, x)
                cv::Vec3b original_pixel = image.at<cv::Vec3b>(y, x);

                cv::Vec3b modified_pixel;
                modified_pixel[0] = original_pixel[0]; // Blue
                modified_pixel[1] = original_pixel[1]; // Green
                modified_pixel[2] = original_pixel[2]; // Red

                if (horizontal) {
                    result_image.at<cv::Vec3b>(y, image.cols - x - 1) = modified_pixel;
                } else if (vertical) {
                    result_image.at<cv::Vec3b>(image.rows - y - 1, x) = modified_pixel;
                }
            }
        }
    } else {
        std::cerr << "Error: Unsupported number of channels: " << image.channels() << std::endl;
        std::cerr << "This code only supports 3-channel (color) images." << std::endl;
        return -1;
    }

    bool success = cv::imwrite(output_filename, result_image);

    if (!success) {
        std::cerr << "Error: Failed to save the image to '" << output_filename << "'" << std::endl;
        return -1;
    }

    std::cout << "Successfully processed and saved to '" << output_filename << "'" << std::endl;

    return 0;
}