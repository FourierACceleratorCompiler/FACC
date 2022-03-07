#include <adi_fft_wrapper.h>

// So, the idea here is to, as stated in the paper:
// Provide an API without too much needless sparsity.
// We could target the accel_cfft_large function directly,
// it would just require a bit more information in the
// apispec.json file --- it's just easier to target this
// this way.

// We could also support return values, but it just
// saves programming effort to not use the return values.
// (Have yet to be implemented in FACC at time of writing.)

void accel_cfft_wrapper(const complex_float dm input[],
						complex_float dm output[],
						int n) {
	if (n > 2048) {
		int stride;
		complex_float *twiddles;
		if (n <= 65536) {
			*twiddles = accel_twiddles_65526;
			stride = 65536 / n;
		} else {
			// Compute these --- TODO -- cache these.
			complex_float stack_twiddles[n];
			accel_twidfft(stack_twiddles, n);
			twiddles = stack_twiddles;
			stride = 1;
		}
		accel_cfft_large(input, output, twiddles, stride, 1.0, n);
	} else {
		accel_cfft_small(input, output, 1.0, n);
	}
}
