#include "bit_stream/src/bit_stream.h"
#include <cmath>

enum NegativeHandling {
    ZIGZAG = 0,
    SIGN_MAGNITUDE = 1
};

class GolombUtils {
    public:
        GolombUtils(int m_value, NegativeHandling neg_handling_value)
            : m(m_value), neg_handling(neg_handling_value) {}
        void golomb_encode(BitStream *bs, unsigned int num);
        void golomb_decode(BitStream *bs_in);

        int value_zigzag_to_signed(int num);
        unsigned int value_signed_to_zigzag(int num);
        int decode_zigzag(BitStream *bs);
        void encode_zigzag(int num, BitStream *bs);
        
        void encode_sign_magnitude(int num, BitStream *bs);
        
        void encode_unsigned(int num, BitStream *bs);
        int decode_unsigned(BitStream *bs);
    private:
        int m;
        NegativeHandling neg_handling;

};