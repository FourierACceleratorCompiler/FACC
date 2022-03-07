#include<fftw3.h>
#include<complex>

namespace std {
// simple call to FFTW --- we don't handle double array generation yet, but it has the same memory
// layout as this struct so that seems OK to just treat
// as the struct.
void fftw_example_api(complex<double> *api_in, complex<double> *api_out, int interface_len, int direction) {
	fftw_plan p;
	p = fftw_plan_dft_1d(interface_len, reinterpret_cast<fftw_complex*>(api_in), reinterpret_cast<fftw_complex*>(api_out), direction, FFTW_ESTIMATE);
    fftw_execute(p); /* repeat as needed */
    fftw_destroy_plan(p);
}
}
