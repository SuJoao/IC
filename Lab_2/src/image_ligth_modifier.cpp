#include <iostream>
#include <string>
#include <opencv2/opencv.hpp>

using namespace std;

cv::Vec3b iluminate_pixel(const cv::Vec3b& pixel, double ilumination) {
    cv::Vec3b clamped_pixel;
    double pixel_int[3];
    pixel_int[0] = ((double)pixel[0] * ilumination); // Blue
    pixel_int[1] = ((double)pixel[1] * ilumination); // Green
    pixel_int[2] = ((double)pixel[2] * ilumination); // Red

    for (int i = 0; i < 3; ++i) {
        if (pixel_int[i] > 255) {
            clamped_pixel[i] = 255;
        } else if (pixel_int[i] < 0) {
            clamped_pixel[i] = 0;
        } else {
            clamped_pixel[i] = (uchar)((int)pixel_int[i]);
        }
    }
    return clamped_pixel;
}

int main(int argc, char* argv[]) {

    if (argc < 4) {
        cerr << "Usage: " << argv[0] << " <input ppm file> <output ppm file> <brightness factor>\n";
        cerr << "Brightness factor:\n";
        cerr << "  < 1.0 - Darken the image\n";
        cerr << "  = 1.0 - No change\n";
        cerr << "  > 1.0 - Brighten the image\n";
        return 1;
    }

    string input_filename = argv[1];
    string output_filename = argv[2];
    float ilumination = stof(argv[3]);

    if (ilumination < 0) {
        cerr << "Error: Brightness factor must be non-negative\n";
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
    std::cout << "Brightness factor: " << ilumination << std::endl;

    if (image.channels() == 3) {
        for (int y = 0; y < image.rows; ++y) {
            for (int x = 0; x < image.cols; ++x) {

                cv::Vec3b original_pixel = image.at<cv::Vec3b>(y, x);

                // Apply illumination to the pixel
                result_image.at<cv::Vec3b>(y, x) = iluminate_pixel(original_pixel, ilumination);
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