/** @file ascii85.h
 *
 * @brief Ascii85 encoder and decoder
 *
 * @par
 * @copyright Copyright © 2017 Doug Currie, Londonderry, NH, USA. All rights reserved.
 *
 * @par
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 * and associated documentation files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 * BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
**/

#ifndef SLI_ASCII85_H
#define SLI_ASCII85_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

enum ascii85_errs_e
{
    ascii85_err_out_buf_too_small = -255,
    ascii85_err_in_buf_too_large,
    ascii85_err_bad_decode_char,
    ascii85_err_decode_overflow
};

int32_t encode_ascii85 (const uint8_t *inp, int32_t in_length, uint8_t *outp, int32_t out_max_length);

int32_t decode_ascii85 (const uint8_t *inp, int32_t in_length, uint8_t *outp, int32_t out_max_length);

int32_t ascii85_get_max_encoded_length (int32_t in_length);

int32_t ascii85_get_max_decoded_length (int32_t in_length);


#ifdef __cplusplus
}
#endif

#endif /* SLI_ASCII85_H */
