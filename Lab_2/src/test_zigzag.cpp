#include "GolombUtils.h"
#include "bit_stream/src/bit_stream.h"
#include <iostream>
#include <cassert>
#include <vector>
#include <fstream>
#include <cstring>

using namespace std;

int test_zigzag_encoding_decoding() {
    cout << "========================================" << endl;
    cout << "Testing Zigzag Encoding and Decoding" << endl;
    cout << "========================================" << endl;

    // Test cases: pairs of (signed_value, expected_zigzag_encoded)
    vector<pair<int, unsigned int>> test_cases = {
        {0, 0},           // 0 -> 0
        {-1, 1},          // -1 -> 1
        {1, 2},           // 1 -> 2
        {-2, 3},          // -2 -> 3
        {2, 4},           // 2 -> 4
        {-3, 5},          // -3 -> 5
        {3, 6},           // 3 -> 6
        {-64, 127},       // -64 -> 127
        {64, 128},        // 64 -> 128
    };

    GolombUtils golomb(8, ZIGZAG);

    cout << "\nTest 1: Zigzag Encoding/Decoding Round-trip" << endl;
    cout << "--------------------------------------------" << endl;
    
    int passed = 0;
    int failed = 0;
    
    for (const auto& test : test_cases) {
        int original_signed = test.first;
        unsigned int expected_zigzag = test.second;
        
        // Create temporary file for testing
        string temp_file = "test_zigzag_temp.bin";
        
        // Encode phase
        {
            fstream fs(temp_file, ios::out | ios::binary);
            BitStream bs(fs, STREAM_WRITE);
            golomb.encode_zigzag(original_signed, &bs);
            bs.close();
        }
        
        // Decode phase
        {
            fstream fs(temp_file, ios::in | ios::binary);
            BitStream bs(fs, STREAM_READ);
            int decoded_signed = golomb.decode_zigzag(&bs);
            bs.close();
            
            if (decoded_signed == original_signed) {
                cout << "✓ PASS: " << original_signed 
                     << " (zigzag: " << expected_zigzag << ") "
                     << "-> Decoded: " << decoded_signed << endl;
                passed++;
            } else {
                cout << "✗ FAIL: " << original_signed 
                     << " -> Got: " << decoded_signed << endl;
                failed++;
            }
        }
        
        // Cleanup
        remove(temp_file.c_str());
    }

    cout << "\nTest 2: Additional Edge Cases" << endl;
    cout << "------------------------------" << endl;
    
    vector<int> edge_cases = {
        0, 1, -1, 127, -127, 255, -255, 
        32767, -32767, 65535, -65535
    };

    for (int val : edge_cases) {
        string temp_file = "test_zigzag_temp.bin";
        
        // Encode
        {
            fstream fs(temp_file, ios::out | ios::binary);
            BitStream bs(fs, STREAM_WRITE);
            golomb.encode_zigzag(val, &bs);
            bs.close();
        }
        
        // Decode
        {
            fstream fs(temp_file, ios::in | ios::binary);
            BitStream bs(fs, STREAM_READ);
            int decoded = golomb.decode_zigzag(&bs);
            bs.close();
            
            if (decoded == val) {
                cout << "✓ PASS: Edge case " << val << " decoded correctly" << endl;
                passed++;
            } else {
                cout << "✗ FAIL: Edge case " << val 
                     << " -> Got: " << decoded << endl;
                failed++;
            }
        }
        
        remove(temp_file.c_str());
    }

    cout << "\n========================================" << endl;
    cout << "Test Results:" << endl;
    cout << "  Passed: " << passed << endl;
    cout << "  Failed: " << failed << endl;
    cout << "========================================" << endl;
    
    if (failed > 0) {
        return 1;
    }
}

int main() {
    try {
        return test_zigzag_encoding_decoding();
    } catch (const exception& e) {
        cerr << "Test failed with exception: " << e.what() << endl;
        return 1;
    }
    return 0;
}
