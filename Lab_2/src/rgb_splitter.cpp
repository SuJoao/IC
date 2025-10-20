#include <iostream>
#include <string>
#include <opencv4/opencv2/opencv.hpp> // Main OpenCV header

int main() {
    // --- 1. DEFINE FILENAMES ---
    std::string input_filename = "../images/airplane.ppm";   // Change this to your .ppm file
    std::string output_filenameR = "outputR.ppm"; // The file to save
    std::string output_filenameG = "outputG.ppm"; // The file to save
    std::string output_filenameB = "outputB.ppm"; // The file to save

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
    cv::Mat result_imageR = cv::Mat(image.rows, image.cols, image.type());
    cv::Mat result_imageG = cv::Mat(image.rows, image.cols, image.type());
    cv::Mat result_imageB = cv::Mat(image.rows, image.cols, image.type());

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
                modified_pixel[1] = 0; // Green
                modified_pixel[2] = 0; // Red
                // --- END OF YOUR LOGIC ---

                // Set the pixel in the result image
                result_imageR.at<cv::Vec3b>(y, x) = {original_pixel[2],0,0};
                result_imageG.at<cv::Vec3b>(y, x) = {0,original_pixel[1],0};
                result_imageB.at<cv::Vec3b>(y, x) = {0,0,original_pixel[0]};
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
                result_imageR.at<uchar>(y, x) = original_pixel;
                result_imageG.at<uchar>(y, x) = original_pixel;
                result_imageB.at<uchar>(y, x) = original_pixel;
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
    bool successR = cv::imwrite(output_filenameR, result_imageR);
    bool successG = cv::imwrite(output_filenameG, result_imageG);
    bool successB = cv::imwrite(output_filenameB, result_imageB);

    if (!successR) {
        std::cerr << "Error: Failed to save the image to '" << output_filenameR << "'" << std::endl;
        return -1;
    }

    std::cout << "Successfully processed and saved to '" << output_filenameG << "'" << std::endl;

    // --- 7. (Optional) DISPLAY THE IMAGES ---
    cv::imshow("Original Image", image);
    cv::imshow("Processed Image", result_imageR);

    std::cout << "Press any key in the image windows to exit." << std::endl;
    cv::waitKey(0); // Wait indefinitely for a key press

    return 0;
}