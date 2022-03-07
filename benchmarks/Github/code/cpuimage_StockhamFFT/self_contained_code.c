
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <stdbool.h>
//ref: http://wwwa.pikara.ne.jp/okojisan/otfft-en/optimization1.html
typedef struct {
   float real, imag;
} complex_t;

void fft0(int n, int s, bool eo, complex_t *x, complex_t *y)
// n  : sequence length
// s  : stride
// eo : x is output if eo == 0, y is output if eo == 1
// x  : input sequence(or output sequence if eo == 0)
// y  : work area(or output sequence if eo == 1)
{
   const int m = n / 2;
   const float theta0 = (float) (2 * M_PI) / n;

   if (n == 2) {
       complex_t *z = eo ? y : x;
       for (int q = 0; q < s; q++) {
           const complex_t a = x[q + 0];
           const complex_t b = x[q + s];
           z[q + 0].real = a.real + b.real;
           z[q + 0].imag = a.imag + b.imag;
           z[q + s].real = a.real - b.real;
           z[q + s].imag = a.imag - b.imag;
       }
   } else if (n >= 4) {
       for (int p = 0; p < m; p++) {
           complex_t wp;
           wp.real = cosf(p * theta0);
           wp.imag = -sinf(p * theta0);
           for (int q = 0; q < s; q++) {
               const complex_t a = x[q + s * (p + 0)];
               const complex_t b = x[q + s * (p + m)];
               y[q + s * (2 * p + 0)].real = a.real + b.real;
               y[q + s * (2 * p + 0)].imag = a.imag + b.imag;
               complex_t t;
               t.real = a.real - b.real;
               t.imag = a.imag - b.imag;
               y[q + s * (2 * p + 1)].real = t.real * wp.real - t.imag * wp.imag;
               y[q + s * (2 * p + 1)].imag = t.real * wp.imag + t.imag * wp.real;
           }
       }
       fft0(n / 2, 2 * s, !eo, y, x);
   }
}

void fft(complex_t *x, int n)
// Fourier transform
// n : sequence length
// x : input/output sequence
{
   complex_t *y = (complex_t *) calloc(sizeof(complex_t), n);
   fft0(n, 1, 0, x, y);
   free(y);
   for (int k = 0; k < n; k++) {
       x[k].real /= n;
       x[k].imag /= n;
   }
}
