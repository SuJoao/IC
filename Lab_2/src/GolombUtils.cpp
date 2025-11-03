#include "GolombUtils.h"

int GolombUtils::value_zigzag_to_signed(int num) {
    return (num>>1) ^ (-(num & 1));
}

unsigned int GolombUtils::value_signed_to_zigzag(int num) {
    return (num << 1) ^ (num >> 31);
}

void GolombUtils::encode_zigzag(int num,BitStream *bs) {
    unsigned int zigzagged = value_signed_to_zigzag(num);

    encode_unsigned(zigzagged, bs);
}

void GolombUtils::encode_unsigned(int num, BitStream *bs) {
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

    int output = 0;
    // Write unary code for quotient
    for (int i = 0; i < q; i++) {
        bs->write_bit(1);
    }
    bs->write_bit(0);
    
    
    // truncação
    temp = (1<<m_bits) - this->m;
    
    if (r < temp)
    {
        m_bits -= 1;
    }else {
        r += temp;
    }
    
    // Write binary code for remainder
    for (int i = m_bits - 1; i >= 0; i--)
    {
        bs->write_bit((r >> i) & 1);
    }
}

int GolombUtils::decode_unsigned(BitStream *bs){
    // Implementation not shown in the original snippets
    int q = 0;

    while(bs->read_bit() != 0){
        q++;
    }

    int m_bits = 0;
    int r = this->m;

    while (r > 0)
    {
        m_bits++;
        r >>= 1;
    }
    

    for (int i = 0; i < m_bits-1; i++) {
        r = (r << 1) | bs->read_bit();
    }

    if (r >= (1<<m_bits) - this->m)
    {
        r = (r << 1) | bs->read_bit();
    }   

    return q * this->m + r;
}

int GolombUtils::decode_zigzag(BitStream *bs) {
    int val = decode_unsigned(bs);
    return value_zigzag_to_signed(val);
}

void GolombUtils::golomb_encode(BitStream *bs, unsigned int num) {
    
}

