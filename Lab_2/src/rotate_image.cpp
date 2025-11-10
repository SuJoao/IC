#include <iostream>
#include <string>
#include <opencv2/opencv.hpp>

using namespace std;

void rotate(int* x, int* y, int times, int img_width, int img_height) {
    *x -= img_width / 2;
    *y -= img_height / 2;

    for (int i = 0; i < times; ++i) {
        // Perform a 90-degree rotation
        int tmpSize = img_width;
        img_width = img_height;
        img_height = tmpSize;

        int temp = *x;
        *x = -(*y);
        *y = temp;
    }

    *x += img_width / 2;
    *y += img_height / 2;
}

int main(int argc, char* argv[]) {
    
    if (argc < 4) {
        cerr << "Usage: " << argv[0] << " <input ppm file> <output ppm file> <rotation>\n";
        cerr << "Rotations (clockwise):\n";
        cerr << "  0 - No rotation\n";
        cerr << "  1 - 90 degrees\n";
        cerr << "  2 - 180 degrees\n";
        cerr << "  3 - 270 degrees\n";
        return 1;
    }

    string input_filename = argv[1];
    string output_filename = argv[2];
    int rotation_input = stoi(argv[3]);
    
    if (rotation_input < 0 || rotation_input > 3) {
        cerr << "Error: Rotation must be 0, 1, 2, or 3\n";
        return 1;
    }
    
    int rotations = rotation_input;

    cv::Mat image = cv::imread(input_filename);

    if (image.empty()) {
        std::cerr << "Error: Could not open or find the image '" << input_filename << "'" << std::endl;
        return -1;
    }

    // Create a new empty image with the same size and type as the original
    int new_image_width = (rotations % 2 == 0) ? image.cols : image.rows;
    int new_image_height = (rotations % 2 == 0) ? image.rows : image.cols;
    cv::Mat result_image = cv::Mat(new_image_height, new_image_width, image.type());

    std::cout << "Processing image: " << image.cols << "x" << image.rows 
              << " with " << image.channels() << " channels." << std::endl;

    
    if (image.channels() == 3) {        
        for (int y = 0; y < image.rows; ++y) {
            for (int x = 0; x < image.cols; ++x) {

                cv::Vec3b original_pixel = image.at<cv::Vec3b>(y, x);

                cv::Vec3b modified_pixel;
                modified_pixel[0] = original_pixel[0]; // Blue
                modified_pixel[1] = original_pixel[1]; // Green
                modified_pixel[2] = original_pixel[2]; // Red

                int *new_x = new int(x);
                int *new_y = new int(y);

                rotate(new_x, new_y, rotations, image.cols, image.rows);
                result_image.at<cv::Vec3b>(*new_y, *new_x) = modified_pixel;
                delete new_x;
                delete new_y;
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
