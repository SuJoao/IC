#include "GolombUtils.h"
#include <iostream>
#include <fstream>
#include <cstring>

using namespace std;

void print_usage(const char* prog_name) {
    cout << "Usage: " << prog_name << " <m> <method> <number> <output_file>\n\n";
    cout << "Parameters:\n";
    cout << "  <m>       - Golomb parameter (positive integer)\n";
    cout << "  <method>  - Encoding method: 'zigzag' or 'sign_magnitude'\n";
    cout << "  <number>  - Integer to encode (can be negative)\n";
    cout << "  <output_file> - Output file path\n\n";
    cout << "Example:\n";
    cout << "  " << prog_name << " 4 zigzag 10 output.bin\n";
}

int main(int argc, char* argv[]) {
    if (argc != 5) {
        print_usage(argv[0]);
        return 1;
    }

    int m = atoi(argv[1]);
    NegativeHandling method = parse_method(argv[2]);
    int number = atoi(argv[3]);
    string output_file = argv[4];

    if (m <= 0) {
        cerr << "Error: m must be a positive integer\n";
        return 1;
    }

    cout << "m: " << m << endl;
    cout << "method: " << argv[2] << endl;
    cout << "number: " << number << endl;
    cout << "output: " << output_file << endl;
    cout << endl;

    // ENCODE
    cout << "=== ENCODING ===" << endl;
    {
        fstream file(output_file, ios::out | ios::binary | ios::trunc);
        if (!file.is_open()) {
            cerr << "Error: Cannot create output file\n";
            return 1;
        }

        BitStream bs(file, STREAM_WRITE);
        GolombUtils golomb(m, method);
        
        golomb.golomb_encode(&bs, number);
        
        bs.close();
        file.close();
        cout << "Encoded to " << output_file << endl;
    }
    
    // READ BITS
    cout << "\n=== READING BITS FROM FILE ===" << endl;
    {
        fstream file(output_file, ios::in | ios::binary);
        if (!file.is_open()) {
            cerr << "Error: Cannot open input file\n";
            return 1;
        }

        BitStream bs(file, STREAM_READ);
        
        cout << "Binary: ";
        int bit;
        int count = 0;
        while ((bit = bs.read_bit()) != EOF) {
            cout << bit;
            count++;
        }
        cout << endl;
        cout << "Total bits: " << count << endl;
        
        bs.close();
        file.close();
    }
    
    // DECODE
    cout << "\n=== DECODING ===" << endl;
    {
        fstream file(output_file, ios::in | ios::binary);
        if (!file.is_open()) {
            cerr << "Error: Cannot open input file\n";
            return 1;
        }

        BitStream bs(file, STREAM_READ);
        GolombUtils golomb(m, method);
        
        int decoded = golomb.golomb_decode(&bs);
        
        cout << "Decoded number: " << decoded << endl;
        
        bs.close();
        file.close();
    }

    return 0;
}
