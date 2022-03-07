#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <complex.h>
typedef enum
{
  FFT_REAL,
  FFT_COMPLEX
} fft_type_t;

typedef enum
{
  FFT_FORWARD,
  FFT_BACKWARD
} fft_direction_t;

#define FFT_OWN_INPUT_MEM 1
#define FFT_OWN_OUTPUT_MEM 2

typedef struct
{
  int size;  // FFT size
  float *input;  // pointer to input buffer
  float *output; // pointer to output buffer
  float *twiddle_factors;  // pointer to buffer holding twiddle factors
  fft_type_t type;   // real or complex
  fft_direction_t direction; // forward or backward
  unsigned int flags; // FFT flags
} fft_config_t;
#define TWO_PI 6.28318530
fft_config_t *fft_init(int size, fft_type_t type, fft_direction_t direction, float *input, float *output)
{
  /*
   * Prepare an FFT of correct size and types.
   *
   * If no input or output buffers are provided, they will be allocated.
   */
  int k,m;

  fft_config_t *config = (fft_config_t *)malloc(sizeof(fft_config_t));

  // Check if the size is a power of two
  if ((size & (size-1)) != 0)  // tests if size is a power of two
    return NULL;

  // start configuration
  config->flags = 0;
  config->type = type;
  config->direction = direction;
  config->size = size;

  // Allocate and precompute twiddle factors
  config->twiddle_factors = (float *)malloc(2 * config->size * sizeof(float));

  float two_pi_by_n = TWO_PI / config->size;

  for (k = 0, m = 0 ; k < config->size ; k++, m+=2)
  {
    config->twiddle_factors[m] = cosf(two_pi_by_n * k);    // real
    config->twiddle_factors[m+1] = sinf(two_pi_by_n * k);  // imag
  }

  // Allocate input buffer
  if (input != NULL)
    config->input = input;
  else 
  {
    if (config->type == FFT_REAL)
      config->input = (float *)malloc(config->size * sizeof(float));
    else if (config->type == FFT_COMPLEX)
      config->input = (float *)malloc(2 * config->size * sizeof(float));

    config->flags |= FFT_OWN_INPUT_MEM;
  }

  if (config->input == NULL)
    return NULL;

  // Allocate output buffer
  if (output != NULL)
    config->output = output;
  else
  {
    if (config->type == FFT_REAL)
      config->output = (float *)malloc(config->size * sizeof(float));
    else if (config->type == FFT_COMPLEX)
      config->output = (float *)malloc(2 * config->size * sizeof(float));

    config->flags |= FFT_OWN_OUTPUT_MEM;
  }

  if (config->output == NULL)
    return NULL;

  return config;
}

void fft_primitive(float *x, float *y, int n, int stride, float *twiddle_factors, int tw_stride)
{
  /*
   * This code will compute the FFT of the input vector x
   *
   * The input data is assumed to be real/imag interleaved
   *
   * The size n should be a power of two
   *
   * y is an output buffer of size 2n to accomodate for complex numbers
   *
   * Forward fast Fourier transform
   * DIT, radix-2, out-of-place implementation
   *
   * For a complex FFT, call first stage as:
   * fft(x, y, n, 2, 2);
   *
   * Parameters
   * ----------
   *  x (float *)
   *    The input array containing the complex samples with
   *    real/imaginary parts interleaved [Re(x0), Im(x0), ..., Re(x_n-1), Im(x_n-1)]
   *  y (float *)
   *    The output array containing the complex samples with
   *    real/imaginary parts interleaved [Re(x0), Im(x0), ..., Re(x_n-1), Im(x_n-1)]
   *  n (int)
   *    The FFT size, should be a power of 2
   *  stride (int)
   *    The number of elements to skip between two successive samples
   *  tw_stride (int)
   *    The number of elements to skip between two successive twiddle factors
   */
  int k;
  float t;

#if LARGE_BASE_CASE
  // End condition, stop at n=8 to avoid one trivial recursion
  if (n == 8)
  {
    fft8(x, stride, y, 2);
    return;
  }
#else
  // End condition, stop at n=2 to avoid one trivial recursion
  if (n == 2)
  {
	  std::cout << "Stride " << stride << std::endl;
    y[0] = x[0] + x[stride];
    y[1] = x[1] + x[stride + 1];
    y[2] = x[0] - x[stride];
    y[3] = x[1] - x[stride + 1];
    return;
  }
#endif

  // Recursion -- Decimation In Time algorithm
  fft_primitive(x, y, n / 2, 2 * stride, twiddle_factors, 2 * tw_stride);             // even half
  fft_primitive(x + stride, y+n, n / 2, 2 * stride, twiddle_factors, 2 * tw_stride);  // odd half

  // Stitch back together

  // We can a few multiplications in the first step
  t = y[0];
  y[0] = t + y[n];
  y[n] = t - y[n];

  t = y[1];
  y[1] = t + y[n+1];
  y[n+1] = t - y[n+1];

  for (k = 1 ; k < n / 2 ; k++)
  {
    float x1r, x1i, x2r, x2i, c, s;
    c = twiddle_factors[k * tw_stride];
    s = twiddle_factors[k * tw_stride + 1];

    x1r = y[2 * k];
    x1i = y[2 * k + 1];
    x2r =  c * y[n + 2 * k] + s * y[n + 2 * k + 1];
    x2i = -s * y[n + 2 * k] + c * y[n + 2 * k + 1];

    y[2 * k] = x1r + x2r;
    y[2 * k + 1] = x1i + x2i;

    y[n + 2 * k] = x1r - x2r;
    y[n + 2 * k + 1] = x1i - x2i;
  }

}
void fft(float *input, float *output, float *twiddle_factors, int n)
{
  /*
   * Forward fast Fourier transform
   * DIT, radix-2, out-of-place implementation
   *
   * Parameters
   * ----------
   *  input (float *)
   *    The input array containing the complex samples with
   *    real/imaginary parts interleaved [Re(x0), Im(x0), ..., Re(x_n-1), Im(x_n-1)]
   *  output (float *)
   *    The output array containing the complex samples with
   *    real/imaginary parts interleaved [Re(x0), Im(x0), ..., Re(x_n-1), Im(x_n-1)]
   *  n (int)
   *    The FFT size, should be a power of 2
   */

#if USE_SPLIT_RADIX
  split_radix_fft(input, output, n, 2, twiddle_factors, 2);
#else
  fft_primitive(input, output, n, 2, twiddle_factors, 2);
#endif
}
