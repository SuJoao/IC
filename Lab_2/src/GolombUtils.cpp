#include "GolombUtils.h"

// Golomb Encoding and Decoding
void GolombUtils::golomb_encode(BitStream *bs, int num) {
    if (this->neg_handling == ZIGZAG) {
        encode_zigzag(bs, num);
    } else if (this->neg_handling == SIGN_MAGNITUDE) {
        encode_sign_magnitude(bs, num);
    } else {
        throw std::invalid_argument("Invalid NegativeHandling value");
    }
}

int GolombUtils::golomb_decode(BitStream *bs) {
    if (this->neg_handling == ZIGZAG) {
        return decode_zigzag(bs);
    } else if (this->neg_handling == SIGN_MAGNITUDE) {
        return decode_sign_magnitude(bs);
    } else {
        throw std::invalid_argument("Invalid NegativeHandling value");
    }
}


// Zigzag Encoding and Decoding
void GolombUtils::encode_zigzag(BitStream *bs, int num) {
    unsigned int zigzagged = value_signed_to_zigzag(num);
    encode_unsigned(bs, zigzagged);
}

int GolombUtils::decode_zigzag(BitStream *bs) {
    int val = decode_unsigned(bs);
    return value_zigzag_to_signed(val);
}

int GolombUtils::value_zigzag_to_signed(int num) {
    return (num>>1) ^ (-(num & 1));
}

unsigned int GolombUtils::value_signed_to_zigzag(int num) {
    return (num << 1) ^ (num >> 31);
}


// Sign-Magnitude Encoding and Decoding
void GolombUtils::encode_sign_magnitude(BitStream *bs, int num) {
    if (num == 0) {
        // Zero has no sign bit
        encode_unsigned(bs, 0);
    } else if (num > 0) {
        // Positive number
        encode_unsigned(bs, num);
        bs->write_bit(0);
    } else {
        // Negative number
        encode_unsigned(bs, -num);
        bs->write_bit(1);
    }
}

int GolombUtils::decode_sign_magnitude(BitStream *bs) {
    int magnitude = decode_unsigned(bs);
    
    if (magnitude == 0) {
        // Zero has no sign bit
        return 0;
    }
    
    // Read the sign bit
    int sign_bit = bs->read_bit();
    
    if (sign_bit == 0) {
        return magnitude;   // Positive
    } else {
        return -magnitude;  // Negative
    }
}


// Unsigned Golomb Encoding and Decoding
void GolombUtils::encode_unsigned(BitStream *bs, unsigned int num) {
    // golomb_encode(bs, zigzagged);
    int q = num / this->m;
    int r = num % this->m;
    int m_bits = 0;
    
    // calculate number of bits needed for m
    int temp = this->m;
    while (temp != 0)
    {
        m_bits++;
        temp >>= 1;
    }

    int cutoff = (1 << m_bits) - this->m;

    // Write unary code for quotient
    for (int i = 0; i < q; i++) {
        bs->write_bit(1);
    }
    bs->write_bit(0);
    
    // Write remainder in truncated binary form
    if (r < cutoff) {
        // shorter form
        for (int i = m_bits - 2; i >= 0; i--) {
            bs->write_bit((r >> i) & 1);
        }
    } else {
        // longer form
        r += cutoff;
        for (int i = m_bits - 1; i >= 0; i--) {
            bs->write_bit((r >> i) & 1);
        }
    }
}

int GolombUtils::decode_unsigned(BitStream *bs){
    int q = 0;

    while(bs->read_bit() != 0) {
        q++;
    }

    int m_bits = 0;
    int temp = this->m;

    while (temp > 0) {
        m_bits++;
        temp >>= 1;
    }
    
    int cutoff = (1 << m_bits) - this->m;

    // Read remainder in truncated binary form
    int r = 0;
    for (int i = m_bits - 2; i >= 0; i--) {
        r = (r << 1) | bs->read_bit();
    }

    if (r >= cutoff) {
        r = (r << 1) | bs->read_bit();
        r -= cutoff;
    }

    return q * this->m + r;
}
