/*
 *  MIT License
 *
 *  Copyright (c) 2021 David Turner
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in all
 *  copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *  SOFTWARE.
 */

#include "ripemd160.h"

/* Constants etc. taken directly from https://homes.esat.kuleuven.be/~bosselae/ripemd160/pdf/AB-9601/AB-9601.pdf */

uint32_t ripemd160_initial_digest[5] =
    { 0x67452301UL, 0xefcdab89UL, 0x98badcfeUL, 0x10325476UL, 0xc3d2e1f0UL };

uint8_t ripemd160_rho[16] =
    { 0x7, 0x4, 0xd, 0x1, 0xa, 0x6, 0xf, 0x3, 0xc, 0x0, 0x9, 0x5, 0x2, 0xe, 0xb, 0x8 };

uint8_t ripemd160_shifts[80] =
    { 11, 14, 15, 12, 5, 8, 7, 9, 11, 13, 14, 15, 6, 7, 9, 8
    , 12, 13, 11, 15, 6, 9, 9, 7, 12, 15, 11, 13, 7, 8, 7, 7
    , 13, 15, 14, 11, 7, 7, 6, 8, 13, 14, 13, 12, 5, 5, 6, 9
    , 14, 11, 12, 14, 8, 6, 5, 5, 15, 12, 15, 14, 9, 9, 8, 6
    , 15, 12, 13, 13, 9, 5, 8, 6, 14, 11, 12, 11, 8, 6, 5, 5
    };

uint32_t ripemd160_constants_left[5] =
    { 0x00000000UL, 0x5a827999UL, 0x6ed9eba1UL, 0x8f1bbcdcUL, 0xa953fd4eUL };

uint32_t ripemd160_constants_right[5] =
    { 0x50a28be6UL, 0x5c4dd124UL, 0x6d703ef3UL, 0x7a6d76e9UL, 0x00000000UL };

uint8_t ripemd160_fns_left[5]  = { 1, 2, 3, 4, 5 };

uint8_t ripemd160_fns_right[5] = { 5, 4, 3, 2, 1 };

#define ROL(x, n) (((x) << (n)) | ((x) >> (32 - (n))))

void ripemd160_compute_line(uint32_t* digest, uint32_t* words, uint32_t* chunk, uint8_t* index, uint8_t* shifts, uint32_t* ks, uint8_t* fns) {
    for (uint8_t i = 0; i < 5; i++) {
        words[i] = digest[i];
    }

    for (uint8_t round = 0; /* breaks out mid-loop */; round++) {
        uint32_t k  = ks[round];
        uint8_t  fn = fns[round];
        for (uint8_t i = 0; i < 16; i++) {
            uint32_t tmp;
            switch (fn) {
                case 1:
                    tmp = words[1] ^ words[2] ^ words[3];
                    break;
                case 2:
                    tmp = (words[1] & words[2]) | (~words[1] & words[3]);
                    break;
                case 3:
                    tmp = (words[1] | ~words[2]) ^ words[3];
                    break;
                case 4:
                    tmp = (words[1] & words[3]) | (words[2] & ~words[3]);
                    break;
                case 5:
                    tmp = words[1] ^ (words[2] | ~words[3]);
                    break;
            }
            tmp += words[0] + chunk[index[i]] + k;
            tmp = ROL(tmp, shifts[index[i]]) + words[4];
            words[0] = words[4];
            words[4] = words[3];
            words[3] = ROL(words[2], 10);
            words[2] = words[1];
            words[1] = tmp;
        }
        if (round == 4) {
            break;
        }
        shifts += 16;

        uint8_t index_tmp[16];
        for (uint8_t i = 0; i < 16; i++) {
            index_tmp[i] = ripemd160_rho[index[i]];
        }
        for (uint8_t i = 0; i < 16; i++) {
            index[i] = index_tmp[i];
        }
    }
}

void ripemd160_update_digest(uint32_t* digest, uint32_t* chunk)
{
    uint8_t index[16];

    /* initial permutation for left line is the identity */
    for (uint8_t i = 0; i < 16; i++) {
        index[i] = i;
    }
    uint32_t words_left[5];
    ripemd160_compute_line(digest, words_left, chunk, index, ripemd160_shifts, ripemd160_constants_left, ripemd160_fns_left);

    /* initial permutation for right line is 5+9i (mod 16) */
    index[0] = 5;
    for (uint8_t i = 1; i < 16; i++) {
        index[i] = (index[i-1] + 9) & 0x0f;
    }
    uint32_t words_right[5];
    ripemd160_compute_line(digest, words_right, chunk, index, ripemd160_shifts, ripemd160_constants_right, ripemd160_fns_right);

    /* update digest */
    digest[0] += words_left[1] + words_right[2];
    digest[1] += words_left[2] + words_right[3];
    digest[2] += words_left[3] + words_right[4];
    digest[3] += words_left[4] + words_right[0];
    digest[4] += words_left[0] + words_right[1];

    /* final rotation */
    words_left[0] = digest[0];
    digest[0] = digest[1];
    digest[1] = digest[2];
    digest[2] = digest[3];
    digest[3] = digest[4];
    digest[4] = words_left[0];
}

void ripemd160(const uint8_t* data, uint32_t data_len, uint8_t* digest_bytes)
{
    /* NB assumes correct endianness */
    uint32_t *digest = (uint32_t*)digest_bytes;
    for (uint8_t i = 0; i < 5; i++) {
        digest[i] = ripemd160_initial_digest[i];
    }

    const uint8_t *last_chunk_start = data + (data_len & (~0x3f));
    while (data < last_chunk_start) {
        ripemd160_update_digest(digest, (uint32_t*)data);
        data += 0x40;
    }

    uint8_t last_chunk[0x40];
    uint8_t leftover_size = data_len & 0x3f;
    for (uint8_t i = 0; i < leftover_size; i++) {
        last_chunk[i] = *data++;
    }

    /* append a single 1 bit and then zeroes, leaving 8 bytes for the length at the end */
    last_chunk[leftover_size] = 0x80;
    for (uint8_t i = leftover_size + 1; i < 0x40; i++) {
        last_chunk[i] = 0;
    }

    if (leftover_size >= 0x38) {
        /* no room for size in this chunk, add another chunk of zeroes */
        ripemd160_update_digest(digest, (uint32_t*)last_chunk);
        for (uint8_t i = 0; i < 0x38; i++) {
            last_chunk[i] = 0;
        }
    }

    uint32_t *length_lsw = (uint32_t *)(last_chunk + 0x38);
    *length_lsw = (data_len << 3);
    uint32_t *length_msw = (uint32_t *)(last_chunk + 0x3c);
    *length_msw = (data_len >> 29);

    ripemd160_update_digest(digest, (uint32_t*)last_chunk);
}
