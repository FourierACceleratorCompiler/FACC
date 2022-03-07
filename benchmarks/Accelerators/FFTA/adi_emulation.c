// This is a fake header for the ADI components --- the idea is that code
// compiled using the defs here can just be copied into the proper
// analog devices environments and compile/work without issue :)
#include "adi_header_emulation.h"
#include<fftw3.h>

// Note that we use a combined API --- that is something I wrote
// up that targets the multiple differnt sizes in the ADI API.
// This isn't because we can't target it, but because it just
// makes it a bit clearer to evaluate IMO.
// Lots of decisions surrounding what programming
// model should be presented to FACC.

void accel_cfft_wrapper(const complex_float *in,
						complex_float *out,
						int n) {
		fftwf_complex fftw_in[n], fftw_out[n];
		for (int i = 0; i < n; i ++) {
			fftw_in[i][0] = in[i].re;
			fftw_in[i][1] = in[i].im;
		}
		fftwf_plan p = fftwf_plan_dft_1d(n, fftw_in, fftw_out, FFTW_FORWARD, FFTW_ESTIMATE);
		fftwf_execute(p);
		for (int i = 0; i < n; i ++) {
			out[i].re = fftw_out[i][0];
			out[i].im = fftw_out[i][1];
		}
}
