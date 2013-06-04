/*
 * 4-bit 0x25AF BCH syndrome decoder
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 51
 * Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Based on:
 *    Generic binary BCH encoding/decoding library
 *    Copyright © 2011 Parrot S.A.
 *    Author: Ivan Djelic <ivan.djelic@parrot.com>
 */

#include <stdint.h>
#include <string.h>
#include "bch.h"

#define DIV_ROUND_UP(a, b) ((a + b - 1) / b)

#define GF_M                   13
#define GF_T                   4
#define GF_N                   8191

#define BCH_ECC_BITS           (GF_M * GF_T)
#define BCH_ECC_WORDS          DIV_ROUND_UP(BCH_ECC_BITS, 32)
#define BCH_ECC_BYTES          DIV_ROUND_UP(BCH_ECC_BITS, 8)

/*
 * represent a polynomial over GF(2^m)
 */
struct gf_poly {
	unsigned int deg;       /* polynomial degree */
	unsigned int c[8];      /* polynomial terms */
};

static uint16_t           a_pow_tab[8192];
static uint16_t           a_log_tab[8192];
static int                cache[8];
static struct gf_poly     elp;
static struct gf_poly     pelp;
static struct gf_poly     elp_copy;

static inline int modulo(unsigned int v)
{
	while (v >= GF_N) {
		v -= GF_N;
		v = (v & GF_N) + (v >> GF_M);
	}
	return v;
}

/*
 * shorter and faster modulo function, only works when v < 2N.
 */
static inline int mod_s(unsigned int v)
{
	return (v < GF_N) ? v : v-GF_N;
}

/* polynomial degree is the most-significant bit index */
static inline int deg(unsigned int poly)
{
	int n = 0;
	while (poly >>= 1)
		n++;
	return n;
}

/* Galois field basic operations: multiply, divide, inverse, etc. */

static inline unsigned int gf_mul(unsigned int a, unsigned int b)
{
	return (a && b) ? a_pow_tab[mod_s(a_log_tab[a]+
					       a_log_tab[b])] : 0;
}

static inline unsigned int gf_sqr(unsigned int a)
{
	return a ? a_pow_tab[mod_s(2*a_log_tab[a])] : 0;
}

static inline unsigned int gf_div(unsigned int a, unsigned int b)
{
	return a ? a_pow_tab[mod_s(a_log_tab[a]+GF_N-a_log_tab[b])] : 0;
}

static inline unsigned int a_pow(int i)
{
	return a_pow_tab[modulo(i)];
}

static inline int a_log(unsigned int x)
{
	return a_log_tab[x];
}

static int compute_error_locator_polynomial(const unsigned int *syn)
{
	unsigned int i, j, tmp, l, pd = 1, d = syn[0];
	int k, pp = -1;

	memset(&pelp, 0, sizeof(struct gf_poly));
	memset(&elp, 0, sizeof(struct gf_poly));

	pelp.deg = 0;
	pelp.c[0] = 1;
	elp.deg = 0;
	elp.c[0] = 1;

	/* use simplified binary Berlekamp-Massey algorithm */
	for (i = 0; (i < GF_T) && (elp.deg <= GF_T); i++) {
		if (d) {
			k = 2*i-pp;
			memcpy(&elp_copy, &elp, sizeof(struct gf_poly));
			/* e[i+1](X) = e[i](X)+di*dp^-1*X^2(i-p)*e[p](X) */
			tmp = a_log(d)+GF_N-a_log(pd);
			for (j = 0; j <= pelp.deg; j++) {
				if (pelp.c[j]) {
					l = a_log(pelp.c[j]);
					elp.c[j+k] ^= a_pow(tmp+l);
				}
			}
			/* compute l[i+1] = max(l[i]->c[l[p]+2*(i-p]) */
			tmp = pelp.deg+k;
			if (tmp > elp.deg) {
				elp.deg = tmp;
				memcpy(&pelp, &elp_copy, sizeof(struct gf_poly));
				pd = d;
				pp = 2*i;
			}
		}
		/* di+1 = S(2i+3)+elp[i+1].1*S(2i+2)+...+elp[i+1].lS(2i+3-l) */
		if (i < GF_T-1) {
			d = syn[2*i+2];
			for (j = 1; j <= elp.deg; j++)
				d ^= gf_mul(elp.c[j], syn[2*i+2-j]);
		}
	}
	return (elp.deg > GF_T) ? -1 : (int)elp.deg;
}

/*
 * build monic, log-based representation of a polynomial
 */
static void gf_poly_logrep(const struct gf_poly *a, int *rep)
{
	int i, d = a->deg, l = GF_N-a_log(a->c[a->deg]);

	/* represent 0 values with -1; warning, rep[d] is not set to 1 */
	for (i = 0; i < d; i++)
		rep[i] = a->c[i] ? mod_s(a_log(a->c[i])+l) : -1;
}

/*
 * exhaustive root search (Chien) implementation
 */
static int chien_search(unsigned int len, unsigned int *roots)
{
	int m;
	unsigned int i, j, syn, syn0, count = 0;
	const unsigned int k = 8*len+BCH_ECC_BITS;

	/* use a log-based representation of polynomial */
	gf_poly_logrep(&elp, cache);
	cache[elp.deg] = 0;
	syn0 = gf_div(elp.c[0], elp.c[elp.deg]);

	for (i = GF_N-k+1; i <= GF_N; i++) {
		/* compute elp(a^i) */
		for (j = 1, syn = syn0; j <= elp.deg; j++) {
			m = cache[j];
			if (m >= 0)
				syn ^= a_pow(m+j*i);
		}
		if (syn == 0) {
			roots[count++] = GF_N-i;
			if (count == elp.deg)
				break;
		}
	}
	return (count == elp.deg) ? count : 0;
}

/**
 * bch_decode - decode received codeword and find bit error locations
 * @len:      data length in bytes, must always be provided
 * @syn:      hw computed syndrome data
 * @errloc:   output array of error locations
 *
 * Returns:
 *  The number of errors found, or -1 if decoding failed
 *
 * Once decode_bch() has successfully returned with a positive value, error
 * locations returned in array @errloc should be interpreted as follows -
 *
 * if (errloc[n] >= 8*len), then n-th error is located in ecc (no need for
 * data correction)
 *
 * if (errloc[n] < 8*len), then n-th error is located in data and can be
 * corrected with statement data[errloc[n]/8] ^= 1 << (errloc[n] % 8);
 *
 * Note that this function does not perform any data correction by itself, it
 * merely indicates error locations.
 */
int bch_decode(unsigned int len, unsigned int *syn, unsigned int *errloc)
{
	unsigned int nbits;
	int i, err, nroots;

	/* v(a^(2j)) = v(a^j)^2 */
	for (i = 0; i < GF_T; i++)
		syn[2*i+1] = gf_sqr(syn[i]);

	err = compute_error_locator_polynomial(syn);
	if (err > 0) {
		nroots = chien_search(len, errloc);
		if (err != nroots)
			err = -1;
	}
	if (err > 0) {
		/* post-process raw error locations for easier correction */
		nbits = (len*8)+BCH_ECC_BITS;
		for (i = 0; i < err; i++) {
			if (errloc[i] >= nbits) {
				err = -1;
				break;
			}
			errloc[i] = nbits-1-errloc[i];
			errloc[i] = (errloc[i] & ~7)|(7-(errloc[i] & 7));
		}
	}
	return (err >= 0) ? err : -1;
}

/**
 * bch_init - initialize a BCH decoder
 */
void bch_init(void)
{
	unsigned int i, x = 1;
	const unsigned int poly = 0x25AF;
	const unsigned int k = 1 << deg(poly);

	for (i = 0; i < GF_N; i++) {
		a_pow_tab[i] = x;
		a_log_tab[x] = i;
		x <<= 1;
		if (x & k)
			x ^= poly;
	}
	a_pow_tab[GF_N] = 1;
	a_log_tab[0] = 0;
}

