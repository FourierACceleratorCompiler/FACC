// This is a C simulation of the behaviour of the accel_cfft_small function.
// Written using FFTW.

#include<fftw3>
#include "../exec_adi.c"

#define ispow2(n) (((n - 1) & n) == 0)

// This is a simulation of what the accelerator actually does so that you don't
// have to have the whole accelerator.
complex_float *accel_cfft_small(const complex_float input[],
		complex_float output[],
		float scale,
		int n) {
	// Accelerator supports smaller/greater values,
	// but not via this function call, and doesn't promise speedup
	// for values less than 1024.
	if (!ispow2(n) && (n < 1024 || n > 2048)) {
		return exec_adi_fft(input, output, n);
	} else {
		return NULL;
	}
}
