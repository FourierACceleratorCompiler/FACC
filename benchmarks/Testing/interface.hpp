#include<fftw3.h>
#include<complex>
#include<iostream>
#include "types.h"

int i_last_interface_len = 0;
int i_last_dir = -1000000;
fftwf_plan i_last_plan = nullptr;

// simple call to FFTW --- we don't handle array generation yet, but it has the same memory
// layout as this struct so that seems OK to just treat
// as the struct.
void i_fftwf_example_api(float *api_in, float *api_out, int interface_len) {
	if (interface_len == i_last_interface_len) {
		// nothing
	} else {
		// Need to create a new plan.
		i_last_interface_len = interface_len;
		if (i_last_plan != nullptr) {
			fftwf_destroy_plan(i_last_plan);
		}
		i_last_plan = fftwf_plan_dft_1d(interface_len / 2, reinterpret_cast<fftwf_complex*>(api_in), reinterpret_cast<fftwf_complex*>(api_out), -1, FFTW_ESTIMATE);
	}
	std::cout << "Starting execution!" << std::endl;;
    fftwf_execute(i_last_plan); /* repeat as needed */
	std::cout << "Finished execution" << std::endl;;
}
