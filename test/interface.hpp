#include<fftw3.h>
#include<complex>

class complex_double_ {
	public:
		double re;
		double im;
};


void fft_example_api(complex_double_ *api_in, complex_double_ *api_out, int interface_len) {
	fftw_plan p;
	p = fftw_plan_dft_1d(interface_len, reinterpret_cast<fftw_complex*>(api_in), reinterpret_cast<fftw_complex*>(api_out), FFTW_FORWARD, FFTW_ESTIMATE);
    fftw_execute(p); /* repeat as needed */
    fftw_destroy_plan(p);
    fftw_destroy_plan(p);
}
