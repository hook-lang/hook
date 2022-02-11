#ifndef BASE58_H
#define BASE58_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

extern int base58_encode(const char *in, size_t in_len, char *out, size_t *out_len);
extern int base58_decode(const char *in, size_t in_len, char *out, size_t *out_len);

#ifdef __cplusplus
}
#endif

#endif /* BASE58_H */
