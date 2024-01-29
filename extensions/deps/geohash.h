/* This is a public domain geohash implementation written by WEI Zhicheng. */

#ifndef __GEOHASH_H__
#define __GEOHASH_H__

#include <stdlib.h>

enum {GEOHASH_OK = 0, GEOHASH_INVALID};

int
geohash_encode(double latitude, double longitude, char *hash, size_t len);

int
geohash_decode(char *hash, double *latitude, double *longitude);

#endif /* __GEOHASH_H__ */
