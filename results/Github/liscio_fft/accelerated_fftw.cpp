
extern "C" {
#include "../../../benchmarks/Github/code/liscio_fft/self_contained_code.h"
}



#include "../../../benchmarks/Accelerators/FFTW/interface.hpp"
#include "complex"


#include<vector>
#include<nlohmann/json.hpp>
#include<fstream>
#include<iomanip>
#include<clib/synthesizer.h>
#include<time.h>
#include<iostream>
char *output_file; 
char *pre_accel_dump_file; // optional dump file. 
using json = nlohmann::json;
const char* __asan_default_options() { return "detect_leaks=0"; }


clock_t AcceleratorStart;
clock_t AcceleratorTotalNanos = 0;
void StartAcceleratorTimer() {
	AcceleratorStart = clock();
}

void StopAcceleratorTimer() {
	AcceleratorTotalNanos +=
		(clock()) - AcceleratorStart;
}

void write_output(_complex_double_ * x, int N, _complex_double_ * returnvar) {

    json output_json;
std::vector<json> output_temp_65;
for (unsigned int i66 = 0; i66 < N; i66++) {
_complex_double_ output_temp_67 = returnvar[i66];
json output_temp_68;

output_temp_68["re"] = output_temp_67.re;

output_temp_68["im"] = output_temp_67.im;
output_temp_65.push_back(output_temp_68);
}
output_json["returnvar"] = output_temp_65;
std::ofstream out_str(output_file); 
out_str << std::setw(4) << output_json << std::endl;
}

_complex_double_ * FFT_wrapper_accel_internal(_complex_double_ * x,int N) {
short dir;;
	dir = 1;;
	int interface_len;;
	interface_len = N;;
	_complex_float_ api_out[interface_len];;
	_complex_float_ api_in[interface_len];;
	for (int i20 = 0; i20 < interface_len; i20++) {
		api_in[i20].re = x[i20].re;
	};
	for (int i21 = 0; i21 < interface_len; i21++) {
		api_in[i21].im = x[i21].im;
	};
	
if ((PRIM_EQUAL(dir, 1)) && ((PRIM_EQUAL(interface_len, 524288)) || ((PRIM_EQUAL(interface_len, 262144)) || ((PRIM_EQUAL(interface_len, 131072)) || ((PRIM_EQUAL(interface_len, 65536)) || ((PRIM_EQUAL(interface_len, 32768)) || ((PRIM_EQUAL(interface_len, 16384)) || ((PRIM_EQUAL(interface_len, 8192)) || ((PRIM_EQUAL(interface_len, 4096)) || ((PRIM_EQUAL(interface_len, 2048)) || ((PRIM_EQUAL(interface_len, 1024)) || ((PRIM_EQUAL(interface_len, 512)) || ((PRIM_EQUAL(interface_len, 256)) || ((PRIM_EQUAL(interface_len, 128)) || ((PRIM_EQUAL(interface_len, 64)) || ((PRIM_EQUAL(interface_len, 32)) || ((PRIM_EQUAL(interface_len, 16)) || ((PRIM_EQUAL(interface_len, 8)) || ((PRIM_EQUAL(interface_len, 4)) || ((PRIM_EQUAL(interface_len, 2)) || (PRIM_EQUAL(interface_len, 1)))))))))))))))))))))) {
StartAcceleratorTimer();;
	fftwf_example_api(api_in, api_out, interface_len, dir);;
	StopAcceleratorTimer();;
	_complex_double_* returnvar = (_complex_double_*) malloc (sizeof(_complex_double_)*N);;;
	for (int i23 = 0; i23 < N; i23++) {
		returnvar[i23].im = api_out[i23].im;
	};
	for (int i24 = 0; i24 < N; i24++) {
		returnvar[i24].re = api_out[i24].re;
	};
	
return returnvar;
} else {

return FFT_wrapper(x, N);;
}
}
_complex_double_ * FFT_wrapper_accel(_complex_double_ * x, int N) {
return (_complex_double_ *)FFT_wrapper_accel_internal((_complex_double_ *) x, (int) N);
}
int main(int argc, char **argv) {
    char *inpname = argv[1]; 
    output_file = argv[2]; 

    std::ifstream ifs(inpname); 
    json input_json = json::parse(ifs);
std::vector<_complex_double_> x_vec;
for (auto& elem : input_json["x"]) {
double x_innerre = elem["re"];
double x_innerim = elem["im"];
_complex_double_ x_inner = { x_innerre, x_innerim};
x_vec.push_back(x_inner);
}
_complex_double_ *x = &x_vec[0];
int N = input_json["N"];
clock_t begin = clock();
_complex_double_ * returnvar = nullptr;
for (int i = 0; i< TIMES; i ++) {
	if (returnvar != nullptr) {
		free(returnvar);
	}
	returnvar = FFT_wrapper_accel(x, N);
}
clock_t end = clock();
std::cout << "Time: " << (double) (end - begin) / CLOCKS_PER_SEC << std::endl;
std::cout << "AccTime: " << (double) AcceleratorTotalNanos / CLOCKS_PER_SEC << std::endl;
write_output(x, N, returnvar);
}
