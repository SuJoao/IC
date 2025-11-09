#include "bit_stream/src/bit_stream.h"
#include <cmath>
#include <iostream>
#include <string>

enum NegativeHandling {
    ZIGZAG = 0,
    SIGN_MAGNITUDE = 1
};

class GolombUtils {
    public:
        GolombUtils(int m_value, NegativeHandling neg_handling_value)
            : m(m_value), neg_handling(neg_handling_value) {}
        
        void golomb_encode(BitStream *bs, int num);
        int golomb_decode(BitStream *bs);
    
    private:
        int m;
        NegativeHandling neg_handling;

        int decode_zigzag(BitStream *bs);
        void encode_zigzag(BitStream *bs, int num);
        int value_zigzag_to_signed(int num);
        unsigned int value_signed_to_zigzag(int num);
        
        void encode_sign_magnitude(BitStream *bs, int num);
        int decode_sign_magnitude(BitStream *bs);
        
        void encode_unsigned(BitStream *bs, unsigned int num);
        int decode_unsigned(BitStream *bs);
};
