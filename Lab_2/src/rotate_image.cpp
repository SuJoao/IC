#include <iostream>
#include <string>
#include <opencv4/opencv2/opencv.hpp> // Main OpenCV header

void rotate_90_pixel(int* x, int* y, int times, int img_width, int img_height) {
    // std::cout << "rotate_90_pixel called with x=" << *x << ", y=" << *y << ", times=" << times << std::endl;
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
    // --- 1. DEFINE FILENAMES ---
    std::string input_filename = "../images/anemone.ppm";   // Change this to your .ppm file
    std::string output_filename = "ROTATED.ppm"; // The file to save

    // --- 2. PARSE COMMAND LINE ARGUMENTS ---
    bool horizontal = false;
    bool vertical = false;
    
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " [<input_file>] [rotations ammount]" << std::endl;
        std::cerr << "At least one mirroring option must be specified." << std::endl;
        return -1;
    }

    input_filename = argv[1];

    int rotations = std::stoi(argv[2]) % 4; // Number of 90-degree rotations (0-3)
    
    if (rotations < 0) {
        std::cerr << "Error: Number of rotations must be non-negative." << std::endl;
        return -1;
    }

    // --- 2. LOAD THE IMAGE ---
    // cv::imread automatically handles the .ppm format
    cv::Mat image = cv::imread(input_filename);

    // --- 3. ERROR CHECKING ---
    if (image.empty()) {
        std::cerr << "Error: Could not open or find the image '" << input_filename << "'" << std::endl;
        return -1;
    }

    // --- 4. PREPARE THE RESULT IMAGE ---
    // Create a new empty image with the same size and type as the original
    int new_image_width = (rotations % 2 == 0) ? image.cols : image.rows;
    int new_image_height = (rotations % 2 == 0) ? image.rows : image.cols;
    cv::Mat result_image = cv::Mat(new_image_height, new_image_width, image.type());
    //cv::Mat result_image = cv::Mat(image.cols, image.rows, image.type());

    std::cout << "Processing image: " << image.cols << "x" << image.rows 
              << " with " << image.channels() << " channels." << std::endl;

    // --- 5. PIXEL ITERATION (Handles both Grayscale and Color) ---
    
    // Check if the image is 3-channel (color PPM)
    if (image.channels() == 3) {
        
        std::cout << "Processing as 3-channel (BGR) image." << std::endl;
        
        // Loop over all rows (y-coordinate)
        for (int y = 0; y < image.rows; ++y) {
            // Loop over all columns (x-coordinate)
            for (int x = 0; x < image.cols; ++x) {
                
                // Get the pixel at (y, x). cv::Vec3b = 3 unsigned chars (B, G, R)
                cv::Vec3b original_pixel = image.at<cv::Vec3b>(y, x);

                // --- YOUR LOGIC GOES HERE ---
                // Example: Invert the image (create a negative)
                cv::Vec3b modified_pixel;
                modified_pixel[0] = original_pixel[0]; // Blue
                modified_pixel[1] = original_pixel[1]; // Green
                modified_pixel[2] = original_pixel[2]; // Red
                // --- END OF YOUR LOGIC ---
                int *new_x = new int(x);
                int *new_y = new int(y);
                // std::cout << "image size: " << image.cols << "x" << image.rows << std::endl;
                // std::cout << "Center offset: " << image.cols/2 << "," << image.rows/2 << std::endl;
                rotate_90_pixel(new_x, new_y, rotations, image.cols, image.rows); // Rotate 90 degrees
                // std::cout << "Original: x=" << x << ", y=" << y << " -> New: x=" << *new_x << ", y=" << *new_y << std::endl;
                result_image.at<cv::Vec3b>(*new_y, *new_x) = modified_pixel;
                delete new_x;
                delete new_y;
            }
        }
    }
    // Check if the image is 1-channel (grayscale PPM)
    else if (image.channels() == 1) {

        std::cout << "Processing as 1-channel (grayscale) image." << std::endl;

        // Loop over all rows (y-coordinate)
        for (int y = 0; y < image.rows; ++y) {
            // Loop over all columns (x-coordinate)
            for (int x = 0; x < image.cols; ++x) {

                // Get the pixel. uchar = 1 unsigned char (0-255)
                uchar original_pixel = image.at<uchar>(y, x);

                // --- YOUR LOGIC GOES HERE ---
                // Example: Invert the image
                uchar modified_pixel = 255 - original_pixel;
                // --- END OF YOUR LOGIC ---

                // Set the pixel in the result image
                result_image.at<uchar>(y, x) = modified_pixel;
            }
        }
    }
    // Handle unsupported channel counts
    else {
        std::cerr << "Error: Unsupported number of channels: " << image.channels() << std::endl;
        std::cerr << "This code only supports 1-channel (grayscale) or 3-channel (color) images." << std::endl;
        return -1;
    }


    // --- 6. SAVE THE RESULT ---
    // cv::imwrite will also automatically handle the .ppm extension
    bool success = cv::imwrite(output_filename, result_image);

    if (!success) {
        std::cerr << "Error: Failed to save the image to '" << output_filename << "'" << std::endl;
        return -1;
    }

    std::cout << "Successfully processed and saved to '" << output_filename << "'" << std::endl;

    // --- 7. (Optional) DISPLAY THE IMAGES ---
    cv::imshow("Original Image", image);
    cv::imshow("Processed Image", result_image);

    std::cout << "Press any key in the image windows to exit." << std::endl;
    cv::waitKey(0); // Wait indefinitely for a key press

    return 0;
}