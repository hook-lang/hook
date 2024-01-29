/* This is a public domain geohash implementation written by WEI Zhicheng. */

#include <math.h>
#include <float.h>
#include <string.h>

#include "geohash.h"

/* BASE 32 encode table */
static char base32en[] = {
	'0', '1', '2', '3', '4', '5', '6', '7',
	'8', '9', 'b', 'c', 'd', 'e', 'f', 'g',
	'h', 'j', 'k', 'm', 'n', 'p', 'q', 'r',
	's', 't', 'u', 'v', 'w', 'x', 'y', 'z'
};

#define BASE32DE_FIRST	'0'
#define BASE32DE_LAST	'z'
/* ASCII order for BASE 32 decode,from '0' to 'z', -1 in unused character */
static signed char base32de[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	10, 11, 12, 13, 14, 15, 16, -1, 17, 18,
	-1, 19, 20, -1, 21, 22, 23, 24, 25, 26,
	27, 28, 29, 30, 31
};

#define PRECISION	.00000000001	/* XXX not validated */

static int
max(int x, int y)
{
	if (x < y)
		return y;
	return x;
}

static double
pround(double x, int precision)
{
	double div = pow(10, precision);
	return round(x * div) / div;
}

/* Quick and Dirty Floating-Point to Fractional Precision */
static double
fprec(double x)
{
        int i;
        double int_part, frac_part;

        frac_part = modf(x, &int_part);

	/* check fractional is really close 0.0 or 1.0 */
	for (i = 0; fabs(frac_part - 0.0) > FLT_EPSILON && 
		    fabs(frac_part - 1.0) > FLT_EPSILON; i++) {
                frac_part *= 10;
                frac_part = modf(frac_part, &int_part);
        }
	return pow(10, -i);
}

int
geohash_encode(double latitude, double longitude, char *hash, size_t len)
{
	double lat[] = {-90.0, 90.0};
	double lon[] = {-180.0, 180.0};
	double mid;

	double precision;

	char mask[] = {16, 8, 4, 2, 1};

	size_t i, n;
	int idx;

	int even = 0;
	int right;

	/* check input latitude/longitude is ok or invalid */
	if (latitude < lat[0] || latitude > lat[1] ||
	    longitude < lon[0] || longitude > lon[1])
		return GEOHASH_INVALID;

	precision = fmin(fprec(latitude), fprec(longitude));
	precision = fmax(PRECISION, precision);

	/* save the last space for '\0' when len is not enough */
	len -= 1;
	for (i = 0; i < len; i++) {
		/* break when precision is enough for input latitude/longitude */
		if ((lat[1] - lat[0]) / 2.0 < precision && 
		    (lon[1] - lon[0]) / 2.0 < precision)
			break;

		for (n = idx = 0; n <= 4; n++) {
			if ((even = !even)) {
				mid = (lon[0] + lon[1]) / 2.0;
				right = (longitude > mid);
				if (right)
					idx |= mask[n];
				lon[!right] = mid;
			} else {
				mid = (lat[0] + lat[1]) / 2.0;
				right = (latitude > mid);
				if (right)
					idx |= mask[n];
				lat[!right] = mid;
			}
		}
		hash[i] = base32en[idx];
	}
	hash[i] = 0;
	return GEOHASH_OK;
}

int
geohash_decode(char *hash, double *latitude, double *longitude)
{
	size_t len = strlen(hash);

	double lat[] = {-90.0, 90.0};
	double lon[] = {-180.0, 180.0};
	double mid;

	double lat_err, lon_err;

	char mask[] = {16, 8, 4, 2, 1};

	size_t i, n;
	int idx = 0;

	int even = 0;

	int right;

	for (i = 0; i < len; i++) {
		if (hash[i] < BASE32DE_FIRST || hash[i] > BASE32DE_LAST ||
		    (idx = base32de[hash[i] - BASE32DE_FIRST]) == -1)
			return GEOHASH_INVALID;

		for (n = 0; n <= 4; n++) {
			if ((even = !even)) {
				mid = (lon[0] + lon[1]) / 2.0;
				right = (idx & mask[n]);
				lon[!right] = mid;
			} else {
				mid = (lat[0] + lat[1]) / 2.0;
				right = (idx & mask[n]);
				lat[!right] = mid;
			}
		}
	}
	lat_err = (lat[1] - lat[0]) / 2.0;
	*latitude = pround((lat[0] + lat[1]) / 2.0,
			max(1, (int)round(-log10(lat_err))) - 1);
	
	lon_err = (lon[1] - lon[0]) / 2.0;
	*longitude = pround((lon[0] + lon[1]) / 2.0, 
			max(1, (int)round(-log10(lon_err))) - 1);

	return GEOHASH_OK;
}
