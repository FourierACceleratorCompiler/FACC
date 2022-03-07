#include<fftw3.h>
#include<complex>
#include<iostream>

class _complex_float_ {
	public:
		float re;
		float im;

		_complex_float_ (float re, float im): re(re), im(im) {

		}
		_complex_float_ () {}

		void print() {
			std::cout << "CPX type is re:" << re << ", imag:" << im << std::endl;
		}
};
int last_interface_len = 0;
int last_dir = -1000000;
fftwf_plan last_plan = nullptr;

// simple call to FFTW --- we don't handle array generation yet, but it has the same memory
// layout as this struct so that seems OK to just treat
// as the struct.
void fftwf_example_api(_complex_float_ *api_in, _complex_float_ *api_out, int interface_len, int dir) {
	if (interface_len == last_interface_len && last_dir == dir) {
		// nothing
	} else {
		// Need to create a new plan.
		last_interface_len = interface_len;
		last_dir = dir;
		if (last_plan != nullptr) {
			fftwf_destroy_plan(last_plan);
		}
		last_plan = fftwf_plan_dft_1d(interface_len, reinterpret_cast<fftwf_complex*>(api_in), reinterpret_cast<fftwf_complex*>(api_out), dir, FFTW_ESTIMATE);
	}
	// std::cout << "Starting execution!" << std::endl;;
    fftwf_execute(last_plan); /* repeat as needed */
	// std::cout << "Finished execution" << std::endl;;
}
