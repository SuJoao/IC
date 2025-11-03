#include <iostream>
#include <string>
#include <opencv4/opencv2/opencv.hpp> // Main OpenCV header

cv::Vec3b iluminate_pixel(const cv::Vec3b& pixel, double ilumination) {
    cv::Vec3b clamped_pixel;
    double pixel_int[3];
    pixel_int[0] = ((double)pixel[0] * ilumination); // Blue
    pixel_int[1] = ((double)pixel[1] * ilumination); // Green
    pixel_int[2] = ((double)pixel[2] * ilumination); // Red
    // fprintf(stderr, "Ilumination factor: %.2f\n", ilumination);
    // fprintf(stderr, "Original pixel: B=%d, G=%d, R=%d\n", pixel_int[0], pixel_int[1], pixel_int[2]);
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

    float ilumination = std::stof(argv[2]); // Number of 90-degree rotations (0-3)

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
    
    cv::Mat result_image = cv::Mat(image.cols, image.rows, image.type());

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
                
                // --- END OF YOUR LOGIC ---
                result_image.at<cv::Vec3b>(y, x) = iluminate_pixel(original_pixel, ilumination);
                
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