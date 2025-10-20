#include <iostream>
#include <string>
#include <opencv4/opencv2/opencv.hpp> // Main OpenCV header

int main(int argc, char* argv[]) {
    // --- 1. DEFINE FILENAMES ---
    std::string input_filename = "../images/airplane.ppm";   // Change this to your .ppm file
    std::string output_filename = "mirrored.ppm"; // The file to save

    // --- 2. PARSE COMMAND LINE ARGUMENTS ---
    bool horizontal = false;
    bool vertical = false;
    
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " [<input_file>] [--horizontal] [--vertical]" << std::endl;
        std::cerr << "At least one mirroring option must be specified." << std::endl;
        return -1;
    }

    input_filename = argv[1];

    for (int i = 2; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--horizontal" || arg == "-h") {
            horizontal = true;
        } else if (arg == "--vertical" || arg == "-v") {
            vertical = true;
        } else {
            std::cerr << "Unknown argument: " << arg << std::endl;
            return -1;
        }
    }
    
    if (!horizontal && !vertical) {
        std::cerr << "Error: At least one mirroring option (--horizontal or --vertical) must be specified." << std::endl;
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
    cv::Mat result_image = cv::Mat(image.rows, image.cols, image.type());

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

                // Set the pixel in the result image
                if (vertical && horizontal)
                {
                    result_image.at<cv::Vec3b>(image.rows - y - 1, image.cols - x - 1) = modified_pixel;
                }else if (horizontal) {
                    result_image.at<cv::Vec3b>(y, image.cols - x - 1) = modified_pixel;
                }
                else if (vertical) {
                    result_image.at<cv::Vec3b>(image.rows - y - 1, x) = modified_pixel;
                }
                
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