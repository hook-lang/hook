#include <limits.h>

#include "base58.h"

static const char alphabet[] =
    "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";

static const char alphamap[] = {
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1,  0,  1,  2,  3,  4,  5,  6,  7,  8, -1, -1, -1, -1, -1, -1,
    -1,  9, 10, 11, 12, 13, 14, 15, 16, -1, 17, 18, 19, 20, 21, -1,
    22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, -1, -1, -1, -1, -1,
    -1, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, -1, 44, 45, 46,
    47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, -1, -1, -1, -1, -1
};

int base58_encode(const char *in, size_t in_len, char *out, size_t *out_len) {
    if (!in_len) {
        *out_len = 0;
        return 0;
    }
    if (!(*out_len)) {
        *out_len = 0;
        return -1;
    }

    // leading zeroes
    size_t total = 0;
    for (size_t i = 0; i < in_len && !in[i]; ++i) {
        if (total == *out_len) {
            *out_len = 0;
            return -1;
        }
        out[total++] = alphabet[0];
    }
    in += total;
    in_len -= total;
    out += total;

    // encoding
    size_t idx = 0;
    for (size_t i = 0; i < in_len; ++i) {
        unsigned int carry = (unsigned char)in[i];
        for (size_t j = 0; j < idx; ++j) {
            carry += (unsigned int)out[j] << 8;
            out[j] = (unsigned char)(carry % 58);
            carry /= 58;
        }
        while (carry > 0) {
            if (total == *out_len) {
                *out_len = 0;
                return -1;
            }
            total++;
            out[idx++] = (unsigned char)(carry % 58);
            carry /= 58;
        }
    }

    // apply alphabet and reverse
    size_t c_idx = idx >> 1;
    for (size_t i = 0; i < c_idx; ++i) {
        char s = alphabet[(unsigned char)out[i]];
        out[i] = alphabet[(unsigned char)out[idx - (i + 1)]];
        out[idx - (i + 1)] = s;
    }
    if ((idx & 1)) {
        out[c_idx] = alphabet[(unsigned char)out[c_idx]];
    }
    *out_len = total;
    return 0;
}

int base58_decode(const char *in, size_t in_len, char *out, size_t *out_len) {
    if (!in_len) {
        *out_len = 0;
        return 0;
    }
    if (!(*out_len)) {
        *out_len = 0;
        return -1;
    }

    // leading ones
    size_t total = 0;
    for (size_t i = 0; i < in_len && in[i] == '1'; ++i) {
        if (total == *out_len) {
            *out_len = 0;
            return -1;
        }
        out[total++] = 0;
    }
    in += total;
    in_len -= total;
    out += total;

    // decoding
    size_t idx = 0;
    for (size_t i = 0; i < in_len; ++i) {
        unsigned int carry = (unsigned int)alphamap[(unsigned char)in[i]];
        if (carry == UINT_MAX) {
            *out_len = 0;
            return -1;
        }
        for (size_t j = 0; j < idx; j++) {
            carry += (unsigned char)out[j] * 58;
            out[j] = (unsigned char)(carry & 0xff);
            carry >>= 8;
        }
        while (carry > 0) {
            if (total == *out_len) {
                *out_len = 0;
                return -1;
            }
            total++;
            out[idx++] = (unsigned char)(carry & 0xff);
            carry >>= 8;
        }
    }

    // apply simple reverse
    size_t c_idx = idx >> 1;
    for (size_t i = 0; i < c_idx; ++i) {
        char s = out[i];
        out[i] = out[idx - (i + 1)];
        out[idx - (i + 1)] = s;
    }
    *out_len = total;
    return 0;
}
