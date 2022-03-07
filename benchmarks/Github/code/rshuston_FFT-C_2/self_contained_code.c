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
void fftr_f(complex_f data[], unsigned log2_N, int direction)
{
    /*
     * fft(A[], N):
     *     if N == 1
     *         return
     *     for k = 0 to N/2-1
     *         e[k] = A[2*k]
     *         o[k] = A[2*k+1]
     *     fft(e, N/2)
     *     fft(o, N/2);
     *     WN = exp(−j2π/N)
     *     WNk = 1
     *     for k = 0 to N/2-1
     *         A[k]     = e[k] + WNk * o[k]
     *         A[k+N/2] = e[k] - WNk * o[k]
     *         WNk = WNk * WN
     *
     * For inverse FFT, use Wm = exp(+j2π/N)
     */

    if (log2_N > 0)
    {
        unsigned log2_Nd2;
        unsigned Nd2;
        unsigned k;
        unsigned kpNd2;
        complex_f *evn, *odd;
        double theta_pi;
        double theta;       /* Use double for precision */
        complex_d WN, WNk;  /* Use double for precision */
        complex_d u, t;     /* Use double for precision */

        log2_Nd2 = log2_N - 1;
        Nd2 = 1 << log2_Nd2;

        evn = malloc(Nd2 * sizeof(complex_f));
        odd = malloc(Nd2 * sizeof(complex_f));

        for (k = 0; k < Nd2; k++)
        {
            evn[k] = data[2*k];
            odd[k] = data[2*k+1];
        }

        fftr_f(evn, log2_Nd2, direction);
        fftr_f(odd, log2_Nd2, direction);

        theta_pi = (direction == FFT_FORWARD) ? -M_PI : M_PI;
        theta = theta_pi / Nd2;  /* - (2 * M_PI) / N */
        WN.re = cos(theta);
        WN.im = sin(theta);

        WNk.re = 1.f;
        WNk.im = 0.f;
        for (k = 0; k < Nd2; k++)
        {
            kpNd2 = k + Nd2;

            u.re = evn[k].re;
            u.im = evn[k].im;
            t.re = complex_mul_re(WNk.re, WNk.im, odd[k].re, odd[k].im);
            t.im = complex_mul_im(WNk.re, WNk.im, odd[k].re, odd[k].im);
            data[k].re = u.re + t.re;
            data[k].im = u.im + t.im;
            data[kpNd2].re = u.re - t.re;
            data[kpNd2].im = u.im - t.im;

            t.re = complex_mul_re(WNk.re, WNk.im, WN.re, WN.im);
            t.im = complex_mul_im(WNk.re, WNk.im, WN.re, WN.im);
            WNk = t;
        }

        free(evn);
        free(odd);
    }
}
