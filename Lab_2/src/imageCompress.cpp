#include <filesystem>
#include <iostream>
#include <fstream>
#include <cstring>
#include "GolombUtils.h"
#include <opencv2/opencv.hpp>
using namespace std;

cv::Vec3b predictor_jpeg_ls(int x, int y, cv::Mat image, int channel=3) {
    cv::Vec3b left_pixel, top_pixel, top_left_pixel;

    if (x > 0) {
        left_pixel = image.at<cv::Vec3b>(y, x - 1);
    } else {
        left_pixel = cv::Vec3b(0, 0, 0);
    }
    
    if (y > 0) {
        top_pixel = image.at<cv::Vec3b>(y - 1, x);
    } else {
        top_pixel = cv::Vec3b(0, 0, 0);
    }

    if (x > 0 && y > 0) {
        top_left_pixel = image.at<cv::Vec3b>(y - 1, x - 1);
    } else {
        top_left_pixel = cv::Vec3b(0, 0, 0);
    }

    cv::Vec3b result_pixel;
    for (int c = 0; c < channel; ++c) {
        int p;
        if (top_left_pixel[c] >= std::max(left_pixel[c], top_pixel[c])) {
            p = std::min(left_pixel[c], top_pixel[c]);
        } else if (top_left_pixel[c] <= std::min(left_pixel[c], top_pixel[c])) {
            p = std::max(left_pixel[c], top_pixel[c]);
    } else {
            p = left_pixel[c] + top_pixel[c] - top_left_pixel[c];
        }
        result_pixel[c] = static_cast<uchar>(p);
    }

    return result_pixel;
}

cv::Vec3b predictor_linear_1(int x, int y, cv::Mat image, int channel=3) {
    cv::Vec3b left_pixel, top_pixel, top_left_pixel;

    if (x > 0) {
        left_pixel = image.at<cv::Vec3b>(y, x - 1);
    } else {
        left_pixel = cv::Vec3b(0, 0, 0);
    }

    cv::Vec3b result_pixel;
    for (int c = 0; c < channel; ++c) {
        
        result_pixel[c] = static_cast<uchar>(left_pixel[c]);
    }

    return result_pixel;
}

cv::Vec3b predictor_linear_2(int x, int y, cv::Mat image, int channel=3) {
    cv::Vec3b left_pixel, top_pixel, top_left_pixel;

    if (y > 0) {
        top_pixel = image.at<cv::Vec3b>(y - 1, x);
    } else {
        top_pixel = cv::Vec3b(0, 0, 0);
    }

    cv::Vec3b result_pixel;
    for (int c = 0; c < channel; ++c) {
        
        result_pixel[c] = static_cast<uchar>(top_pixel[c]);
    }

    return result_pixel;
}

cv::Vec3b predictor_linear_3(int x, int y, cv::Mat image, int channel=3) {
    cv::Vec3b left_pixel, top_pixel, top_left_pixel;

    if (x > 0 && y > 0) {
        top_left_pixel = image.at<cv::Vec3b>(y - 1, x - 1);
    } else {
        top_left_pixel = cv::Vec3b(0, 0, 0);
    }

    cv::Vec3b result_pixel;
    for (int c = 0; c < channel; ++c) {
        
        result_pixel[c] = static_cast<uchar>(top_left_pixel[c]);
    }

    return result_pixel;
}

cv::Vec3b predictor_linear_4(int x, int y, cv::Mat image, int channel=3) {
    cv::Vec3b left_pixel, top_pixel, top_left_pixel;

    if (x > 0) {
        left_pixel = image.at<cv::Vec3b>(y, x - 1);
    } else {
        left_pixel = cv::Vec3b(0, 0, 0);
    }
    
    if (y > 0) {
        top_pixel = image.at<cv::Vec3b>(y - 1, x);
    } else {
        top_pixel = cv::Vec3b(0, 0, 0);
    }

    if (x > 0 && y > 0) {
        top_left_pixel = image.at<cv::Vec3b>(y - 1, x - 1);
    } else {
        top_left_pixel = cv::Vec3b(0, 0, 0);
    }

    cv::Vec3b result_pixel;
    for (int c = 0; c < channel; ++c) {
        result_pixel[c] = static_cast<uchar>(left_pixel[c] + top_pixel[c] - top_left_pixel[c]);
    }

    return result_pixel;
}

cv::Vec3b predictor_linear_5(int x, int y, cv::Mat image, int channel=3) {
    cv::Vec3b left_pixel, top_pixel, top_left_pixel;

    if (x > 0) {
        left_pixel = image.at<cv::Vec3b>(y, x - 1);
    } else {
        left_pixel = cv::Vec3b(0, 0, 0);
    }
    
    if (y > 0) {
        top_pixel = image.at<cv::Vec3b>(y - 1, x);
    } else {
        top_pixel = cv::Vec3b(0, 0, 0);
    }

    if (x > 0 && y > 0) {
        top_left_pixel = image.at<cv::Vec3b>(y - 1, x - 1);
    } else {
        top_left_pixel = cv::Vec3b(0, 0, 0);
    }

    cv::Vec3b result_pixel;
    for (int c = 0; c < channel; ++c) {
        result_pixel[c] = static_cast<uchar>(left_pixel[c] + (top_pixel[c] - top_left_pixel[c])/2);
    }

    return result_pixel;
}

cv::Vec3b predictor_linear_6(int x, int y, cv::Mat image, int channel=3) {
    cv::Vec3b left_pixel, top_pixel, top_left_pixel;

    if (x > 0) {
        left_pixel = image.at<cv::Vec3b>(y, x - 1);
    } else {
        left_pixel = cv::Vec3b(0, 0, 0);
    }
    
    if (y > 0) {
        top_pixel = image.at<cv::Vec3b>(y - 1, x);
    } else {
        top_pixel = cv::Vec3b(0, 0, 0);
    }

    if (x > 0 && y > 0) {
        top_left_pixel = image.at<cv::Vec3b>(y - 1, x - 1);
    } else {
        top_left_pixel = cv::Vec3b(0, 0, 0);
    }

    cv::Vec3b result_pixel;
    for (int c = 0; c < channel; ++c) {
        result_pixel[c] = static_cast<uchar>(top_pixel[c] + (left_pixel[c] - top_left_pixel[c])/2);
    }

    return result_pixel;
}

cv::Vec3b predictor_linear_7(int x, int y, cv::Mat image, int channel=3) {
    cv::Vec3b left_pixel, top_pixel, top_left_pixel;

    if (x > 0) {
        left_pixel = image.at<cv::Vec3b>(y, x - 1);
    } else {
        left_pixel = cv::Vec3b(0, 0, 0);
    }
    
    if (y > 0) {
        top_pixel = image.at<cv::Vec3b>(y - 1, x);
    } else {
        top_pixel = cv::Vec3b(0, 0, 0);
    }

    if (x > 0 && y > 0) {
        top_left_pixel = image.at<cv::Vec3b>(y - 1, x - 1);
    } else {
        top_left_pixel = cv::Vec3b(0, 0, 0);
    }

    cv::Vec3b result_pixel;
    for (int c = 0; c < channel; ++c) {
        result_pixel[c] = static_cast<uchar>((left_pixel[c] + top_pixel[c])/2);
    }

    return result_pixel;
}

typedef cv::Vec3b (*PredictorFunc)(int, int, cv::Mat, int);

static PredictorFunc predictors[] = {
    predictor_jpeg_ls,
    predictor_linear_1,
    predictor_linear_2,
    predictor_linear_3,
    predictor_linear_4,
    predictor_linear_5,
    predictor_linear_6,
    predictor_linear_7
};

std::ifstream::pos_type compress_image(const string& input_filename, const string& output_filename, int predictor_idx) {
    // Compression logic here
    PredictorFunc predictor = predictors[predictor_idx];
    cv::Mat image = cv::imread(input_filename);
        fstream file(output_filename, ios::out | ios::binary | ios::trunc);
        if (!file.is_open()) {
            cerr << "Error: Cannot create output file\n";
            return 1;
        }
        BitStream bs(file, STREAM_WRITE);
    
        if (image.empty()) {
            std::cerr << "Error: Could not open or find the image '" << input_filename << "'" << std::endl;
            return -1;
        }
    
        // Create a new empty image with the same size and type as the original
        cv::Mat result_image = cv::Mat(image.cols, image.rows, image.type());

        cout << "image type : "<<image.type() << endl;
        cout << "size of type var is " <<sizeof(image.type()) << endl;
    
        std::cout << "Processing image: " << image.cols << "x" << image.rows
                << " with " << image.channels() << " channels." << std::endl;
        std::ifstream::pos_type file_size = 0;
        long long total_difference = 0;
        if (image.channels() == 3) {
            for (int y = 0; y < image.rows; ++y) {
                for (int x = 0; x < image.cols; ++x) {
    
                    cv::Vec3b original_pixel = image.at<cv::Vec3b>(y, x);
    
                    cv::Vec3b modified_pixel;
                    
                    //std::cout << "predicting" << ", ";
                
                    modified_pixel = predictor(x, y, image, 3);
                    
                    
                    for (size_t i = 0; i < 3; i++)
                    {   
                        total_difference += abs((int)modified_pixel[i] - (int)original_pixel[i]);
                        // std::cout << (int)modified_pixel[i] - (int)original_pixel[i] << ", ";
                        modified_pixel[i] = (uchar)abs((int)modified_pixel[i] - (int)original_pixel[i]);
                    }
                    
                    //std::cout << std::endl;
    
                    result_image.at<cv::Vec3b>(y, x) = modified_pixel;
    
                }
            }

            int pixel_count = image.rows * image.cols * image.channels();
            int aprox_m = (int)((total_difference+ (pixel_count/2)) / pixel_count);
            std::cout << "Suggested m value for Golomb coding: " << aprox_m << std::endl;

            GolombUtils golomb(aprox_m, ZIGZAG);

            fetch_4B_value(&bs, aprox_m); 
            fetch_4B_value(&bs, (int)image.cols);
            fetch_4B_value(&bs, (int)image.rows);
            fetch_4B_value(&bs, image.type());
            fetch_4B_value(&bs, image.channels());
            fetch_4B_value(&bs, predictor_idx);

            for (int y = 0; y < image.rows; ++y) {
                for (int x = 0; x < image.cols; ++x) {
                    
                    cv::Vec3b original_pixel = image.at<cv::Vec3b>(y, x);
                    
                    cv::Vec3b modified_pixel;

                    // std::cout << "predicting" << ", ";
                    
                    modified_pixel = predictor(x, y, image, 3);
                    
                    
                    for (size_t i = 0; i < 3; i++)
                    {   
                        golomb.golomb_encode(&bs,(int)modified_pixel[i] - (int)original_pixel[i]);
                    }
                    
                    //std::cout << std::endl;
                    
                    result_image.at<cv::Vec3b>(y, x) = modified_pixel;
                    
                }
            }

            
            
            bs.close();
            
            file.close();
            std::filesystem::path file_path = "./" + output_filename;
            std::filesystem::path file_path2 = "./" + input_filename;
            file_size = std::filesystem::file_size(file_path);
            double file_size2 = (double)std::filesystem::file_size(file_path2);
            
            cout << "----------------------------\n";
            cout<< "m : " << aprox_m << "\n";
            cout << "method used : " << predictor_idx << "\n";
            cout << "Compression Ratio: " << ((double)file_size)/file_size2 << " bytes" << std::endl;
            cout<< "width : " << image.rows << "\n";
            cout<< "height : " << image.cols << "\n";
            cout<< "pixel number : " << image.rows * image.cols<< "\n";
            cout << "----------------------------\n";
            
        } else {
            std::cerr << "Error: Unsupported number of channels: " << image.channels() << std::endl;
            std::cerr << "This code only supports 3-channel (color) images." << std::endl;
            return -1;
        }
        
        auto output_path = "image_" + output_filename + ".ppm";
        
        bool success = cv::imwrite(output_path, result_image);
        
        if (!success) {
            std::cerr << "Error: Failed to save the image to '" << output_path << "'" << std::endl;
            return -1;
        }
        
    std::cout << "Successfully processed and saved to '" << output_path << "'" << std::endl;
        
    return file_size;
}

void decompress_image(const string& input_filename, const string& output_filename) {
    fstream file(input_filename, ios::in | ios::binary);
        
    if (!file.is_open()) {
            cerr << "Error: Cannot create output file\n";
            return;
        }
        BitStream bs(file, STREAM_READ);
        
        int m = retrieve_4B_value(&bs);
        int width = retrieve_4B_value(&bs);
        int height = retrieve_4B_value(&bs);
        int type = retrieve_4B_value(&bs);
        int channels = retrieve_4B_value(&bs);
        int predictor_idx = retrieve_4B_value(&bs);

        PredictorFunc predictor = predictors[predictor_idx];
        
        cv::Mat image = cv::Mat(width, height, type);
        
        cout<< "m : " << m << "\n";
        cout<< "width : " << width << "\n";
        cout<< "height : " << height << "\n";
        cout<< "type : " << type << "\n";
        cout<< "channels : " << channels << "\n";

        GolombUtils golomb(m, ZIGZAG);
        

        for (int y = 0; y < image.rows; ++y) {
                for (int x = 0; x < image.cols; ++x) {
                    
                    // cv::Vec3b original_pixel = image.at<cv::Vec3b>(y, x);
                    
                    cv::Vec3b modified_pixel;

                    // std::cout << "predicting" << ", ";
                    
                    modified_pixel = predictor(x, y, image, channels);
                    
                    
                    for (size_t i = 0; i < channels; i++)
                    {   
                        int diff= golomb.golomb_decode(&bs);
                        modified_pixel[i] = (uchar)((int)modified_pixel[i] - diff);
                    }
                    
                    // std::cout << std::endl;
                    
                    image.at<cv::Vec3b>(y, x) = modified_pixel;
                    
                }
            }
        bs.close();
        file.close();
        bool success = cv::imwrite(output_filename, image);
        cout << "Decompressed image saved to " << output_filename << endl;
}


int main(int argc, char* argv[]) {
    cv::Vec3b (*predictor)(int, int, cv::Mat, int) = predictors[0];
    if (argc < 4) {
        cout << "Usage:\n";
        cout << "../bin/image_compressor <image_to_compress> compress <output_file>";
		cout << "\nOR\n";
		cout << "../bin/image_compressor <compressed_file> decompress <output_image>";
        return 1;
    }

    string input_filename = argv[1];

    string operation = argv[2];

    string output_filename = "output.comp"; // Default output filename

    if (argc > 3)
    {
        output_filename = argv[3];
    }
    
    
    string predictor_type = "zigzag"; // Default predictor

    if (argc > 4)
    {
        predictor_type = argv[4];
    }
    

    if (operation == "compress" ) {
        cout << "compressing ...";
        cout << "write point : "<<(int) compress_image(input_filename, output_filename, 0) << endl;
        int s = 0;
        int choice = 0;
        
        for (int i = 0; i < 8; ++i) {
            cout << "Predictor " << i << ": ";
            int t = compress_image(input_filename, output_filename, i);
            if (s > t)
            {
                s = t;
                choice = i;
            }
        }
        compress_image(input_filename, output_filename, choice);
    }
    else if (operation == "decompress")
    {   
        cout << "decompressing ...";
        decompress_image(input_filename, output_filename);

    }else {
        cerr << "Error: Unknown operation '" << operation << "'. Use 'compress' or 'decompress'.\n";
        return 1;
    }
    


    return 0;
}
