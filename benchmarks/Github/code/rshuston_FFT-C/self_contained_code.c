 #define FFT_FORWARD 0 // Desugar enums
 #define FFT_INVERSE 1
 #define FFT_INVERSE 1
typedef struct complex_f {
    float re;
    float im;
} complex_f;

typedef struct complex_d {
    double re;
    double im;
} complex_d;
#define complex_mul_re(a_re, a_im, b_re, b_im)  (a_re * b_re - a_im * b_im)
#define complex_mul_im(a_re, a_im, b_re, b_im)  (a_re * b_im + a_im * b_re)


/* In-place FFT */
void ffti_f(complex_f data[], unsigned log2_N, int direction);

/* ... In-place FFT stage functions ... */
void ffti_copy_shuffle_f(complex_f src[], complex_f dst[], unsigned log2_N);
void ffti_shuffle_f(complex_f data[], unsigned log2_N);
void ffti_evaluate_f(complex_f data[], unsigned log2_N, int direction);

/* Recursive FFT */
void fftr_f(complex_f data[], unsigned log2_N, int direction);

/* Recursive FFT, user-supplied scratchpad buffer */
void fftrb_f(complex_f data[], unsigned log2_N, int direction, complex_f scratch[]);
#include <math.h>

#include <stdio.h>
#include <stdlib.h>



/* ===== In-Place FFT ======================================================= */



void ffti_f(complex_f data[], unsigned log2_N, int direction)
{
    ffti_shuffle_f(data, log2_N);
    ffti_evaluate_f(data, log2_N, direction);
}



void ffti_copy_shuffle_f(complex_f src[], complex_f dst[], unsigned log2_N)
{
    /*
     * Basic Bit-Reversal Scheme:
     *
     * The incrementing pattern operations used here correspond
     * to the logic operations of a synchronous counter.
     *
     * Incrementing a binary number simply flips a sequence of
     * least-significant bits, for example from 0111 to 1000.
     * So in order to compute the next bit-reversed index, we
     * have to flip a sequence of most-significant bits.
     */

    unsigned N = 1 << log2_N;   /* N */
    unsigned Nd2 = N >> 1;      /* N/2 = number range midpoint */
    unsigned Nm1 = N - 1;       /* N-1 = digit mask */
    unsigned i;                 /* index for source element */
    unsigned j;                 /* index for next destination element */

    for (i = 0, j = 0; i < N; i++) {
        dst[j] = src[i];

        /*
         * Find least significant zero bit
         */

        unsigned lszb = ~i & (i + 1);

        /*
         * Use division to bit-reverse the single bit so that we now have
         * the most significant zero bit
         *
         * N = 2^r = 2^(m+1)
         * Nd2 = N/2 = 2^m
         * if lszb = 2^k, where k is within the range of 0...m, then
         *     mszb = Nd2 / lszb
         *          = 2^m / 2^k
         *          = 2^(m-k)
         *          = bit-reversed value of lszb
         */

        unsigned mszb = Nd2 / lszb;

        /*
         * Toggle bits with bit-reverse mask
         */

        unsigned bits = Nm1 & ~(mszb - 1);
        j ^= bits;
    }
}



void ffti_shuffle_f(complex_f data[], unsigned log2_N)
{
    /*
     * Basic Bit-Reversal Scheme:
     *
     * The incrementing pattern operations used here correspond
     * to the logic operations of a synchronous counter.
     *
     * Incrementing a binary number simply flips a sequence of
     * least-significant bits, for example from 0111 to 1000.
     * So in order to compute the next bit-reversed index, we
     * have to flip a sequence of most-significant bits.
     */

    unsigned N = 1 << log2_N;   /* N */
    unsigned Nd2 = N >> 1;      /* N/2 = number range midpoint */
    unsigned Nm1 = N - 1;       /* N-1 = digit mask */
    unsigned i;                 /* index for array elements */
    unsigned j;                 /* index for next element swap location */

    for (i = 0, j = 0; i < N; i++) {
        if (j > i) {
            complex_f tmp = data[i];
            data[i] = data[j];
            data[j] = tmp;
        }

        /*
         * Find least significant zero bit
         */

        unsigned lszb = ~i & (i + 1);

        /*
         * Use division to bit-reverse the single bit so that we now have
         * the most significant zero bit
         *
         * N = 2^r = 2^(m+1)
         * Nd2 = N/2 = 2^m
         * if lszb = 2^k, where k is within the range of 0...m, then
         *     mszb = Nd2 / lszb
         *          = 2^m / 2^k
         *          = 2^(m-k)
         *          = bit-reversed value of lszb
         */

        unsigned mszb = Nd2 / lszb;

        /*
         * Toggle bits with bit-reverse mask
         */

        unsigned bits = Nm1 & ~(mszb - 1);
        j ^= bits;
    }
}



void ffti_evaluate_f(complex_f data[], unsigned log2_N, int direction)
{
    /*
     * In-place FFT butterfly algorithm
     *
     * input:
     *     A[] = array of N shuffled complex values where N is a power of 2
     * output:
     *     A[] = the DFT of input A[]
     *
     * for r = 1 to log2(N)
     *     m = 2^r
     *     Wm = exp(−j2π/m)
     *     for n = 0 to N-1 by m
     *         Wmk = 1
     *         for k = 0 to m/2 - 1
     *             u = A[n + k]
     *             t = Wmk * A[n + k + m/2]
     *             A[n + k]       = u + t
     *             A[n + k + m/2] = u - t
     *             Wmk = Wmk * Wm
     *
     * For inverse FFT, use Wm = exp(+j2π/m)
     */

    unsigned N;
    unsigned r;
    unsigned m, md2;
    unsigned n, k;
    unsigned i_e, i_o;
    double theta_2pi;
    double theta;       /* Use double for precision */
    complex_d Wm, Wmk;  /* Use double for precision */
    complex_d u, t;     /* Use double for precision */

    N = 1 << log2_N;
    theta_2pi = (direction == FFT_FORWARD) ? -M_PI : M_PI;
    theta_2pi *= 2;

    for (r = 1; r <= log2_N; r++)
    {
        m = 1 << r;
        md2 = m >> 1;
        theta = theta_2pi / m;
        Wm.re = cos(theta);
        Wm.im = sin(theta);
        for (n = 0; n < N; n += m)
        {
            Wmk.re = 1.f;
            Wmk.im = 0.f;
            for (k = 0; k < md2; k++)
            {
                i_e = n + k;
                i_o = i_e + md2;
                u.re = data[i_e].re;
                u.im = data[i_e].im;
                t.re = complex_mul_re(Wmk.re, Wmk.im, data[i_o].re, data[i_o].im);
                t.im = complex_mul_im(Wmk.re, Wmk.im, data[i_o].re, data[i_o].im);
                data[i_e].re = u.re + t.re;
                data[i_e].im = u.im + t.im;
                data[i_o].re = u.re - t.re;
                data[i_o].im = u.im - t.im;
                t.re = complex_mul_re(Wmk.re, Wmk.im, Wm.re, Wm.im);
                t.im = complex_mul_im(Wmk.re, Wmk.im, Wm.re, Wm.im);
                Wmk = t;
            }
        }
    }
}
