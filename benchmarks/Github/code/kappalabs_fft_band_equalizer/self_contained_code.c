#include <math.h>
#include <stdlib.h>
typedef struct {
	double re;   // real part of this number
	double im;   // imaginary part of this number
} COMPLEX;

/*
 *  This structure represents array of complex numbers
 *   useful for one input sound track.
 */
typedef struct {
	COMPLEX *c;         // array of complex numbers
	unsigned int len;   // number of COMPLEX numbers in array (*c)
	unsigned int max;   // allocated length of the array (*c)
} C_ARRAY;
COMPLEX complexAdd(COMPLEX c1, COMPLEX c2) {
	COMPLEX c = {c1.re + c2.re, c1.im + c2.im};

	return c;
}

/*
 *  Returns COMPLEX number as a result of subtraction
 *   of two given COMPLEX numbers "c1" and "c2".
 */
COMPLEX complexSub(COMPLEX c1, COMPLEX c2) {
	c2.re *= -1; c2.im *= -1;
	COMPLEX c = complexAdd(c1, c2); 

	return c;
}

/*
 *  Returns COMPLEX number as a result of multiplication
 *   of two given COMPLEX numbers "c1" and "c2".
 */
COMPLEX complexMult(COMPLEX c1, COMPLEX c2) {
	double re = c1.re*c2.re - c1.im*c2.im;
	double im = c1.re*c2.im + c1.im*c2.re;
	COMPLEX c = {re, im};

	return c;
}
C_ARRAY *allocCA(unsigned int len) {
	C_ARRAY *n_arr;
	if ((n_arr = (C_ARRAY *) malloc(sizeof(C_ARRAY))) == NULL) {
		perror("malloc");
		return NULL;
	}

	if ((n_arr->c = (COMPLEX *) malloc(len * sizeof(COMPLEX))) == NULL) {
		perror("malloc");
		return NULL;
	}

	initCA(n_arr, len, 0);

	return n_arr;
}

/*
 *  Reallocate given C_ARRAY structure, so that it has "new_len"
 *   COMPLEX numbers in it.
 */
void reallocCA(C_ARRAY *ca, unsigned int new_len) {
	int olen = ca->max;					// save previous length
	
	if ((ca->c = (COMPLEX *) realloc(ca->c, new_len * sizeof(COMPLEX))) == NULL) {
		perror("malloc");
	}

	initCA(ca, new_len, olen);
}

/*
 *  Free allocated memory for given C_ARRAY structure and set
 *   its pointers to NULL.
 */
void freeCA(C_ARRAY *ca) {
	free(ca->c);
	ca->c = NULL;
	free(ca);
	ca = NULL;
}
/*
 *  Recursively computes Fourier transform performed on input array "ca"
 *  Operates in O(N*log(N)), where N = ca->len is length of input array.
 *  C_ARRAY *ca - input array of complex numbers i.e. sound track
 */
static C_ARRAY *recFFT(C_ARRAY *ca) {
	/*  Round the length of input array to the nearest power of 2 */
	int n = get_pow(ca->len, 2);
	C_ARRAY *cy = allocCA(n);
	cy->len = n;

	if (n == 1) {
		cy->c[0] = ca->c[0];
		return cy;
	}

	C_ARRAY *ca_s = allocCA(n/2);
	C_ARRAY *ca_l = allocCA(n/2);
	C_ARRAY *cy_s, *cy_l;
	
	/*
	 * Initialization of arrays:
	 *   "ca_s" - elements with even index from array "ca"
	 *   "ca_l" - elements with odd index from array "ca"
	 */
	int i;
	for (i=0; i<n/2; i++) {
		ca_s->c[ca_s->len++] = ca->c[2*i];
		ca_l->c[ca_l->len++] = ca->c[2*i + 1];
	}
	
	/* Recursion ready to run */
	cy_s = recFFT(ca_s);
	cy_l = recFFT(ca_l);
	
	/* Use data computed in recursion */
	for (i=0; i < n/2; i++) {
		/* Multiply entries of cy_l by the twiddle factors e^(-2*pi*i/N * k) */
		cy_l->c[i] = complexMult(polarToComplex(1, -2*M_PI*i/n), cy_l->c[i]);
	}
	for (i=0; i < n/2; i++) {
		cy->c[i] = complexAdd(cy_s->c[i], cy_l->c[i]);
		cy->c[i + n/2] = complexSub(cy_s->c[i], cy_l->c[i]);
	}

	/* Release unnecessary memory */
	freeCA(ca_s); freeCA(ca_l);
	freeCA(cy_s); freeCA(cy_l);

	return cy;
}
