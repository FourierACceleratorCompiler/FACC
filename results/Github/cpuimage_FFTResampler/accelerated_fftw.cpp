
#include "../../benchmarks/Github/code/cpuimage_FFTResampler/self_contained_code.c"


#include "../../benchmarks/Accelerators/FFTW/interface.hpp"
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
		((clock()) - AcceleratorStart);
}

void write_output(cmplx * input, cmplx * output, int n) {

    json output_json;
std::vector<json> output_temp_65;
for (unsigned int i66 = 0; i66 < n; i66++) {
cmplx output_temp_67 = output[i66];
json output_temp_68;

output_temp_68["real"] = output_temp_67.real;

output_temp_68["imag"] = output_temp_67.imag;
output_temp_65.push_back(output_temp_68);
}
output_json["output"] = output_temp_65;
std::ofstream out_str(output_file); 
out_str << std::setw(4) << output_json << std::endl;
}

void STB_FFT_accel_internal(cmplx * input,cmplx * output,int n) {
short dir;;
	dir = -1;;
	int interface_len;;
	interface_len = n;;
	_complex_float_ api_out[interface_len];;
	_complex_float_ api_in[interface_len];;
	for (int i7 = 0; i7 < interface_len; i7++) {
		api_in[i7].re = input[i7].real;
	};
	for (int i8 = 0; i8 < interface_len; i8++) {
		api_in[i8].im = input[i8].imag;
	};
	
if ((PRIM_EQUAL(dir, -1)) && ((PRIM_EQUAL(interface_len, 524288)) || ((PRIM_EQUAL(interface_len, 262144)) || ((PRIM_EQUAL(interface_len, 131072)) || ((PRIM_EQUAL(interface_len, 65536)) || ((PRIM_EQUAL(interface_len, 32768)) || ((PRIM_EQUAL(interface_len, 16384)) || ((PRIM_EQUAL(interface_len, 8192)) || ((PRIM_EQUAL(interface_len, 4096)) || ((PRIM_EQUAL(interface_len, 2048)) || ((PRIM_EQUAL(interface_len, 1024)) || ((PRIM_EQUAL(interface_len, 512)) || ((PRIM_EQUAL(interface_len, 256)) || ((PRIM_EQUAL(interface_len, 128)) || ((PRIM_EQUAL(interface_len, 64)) || ((PRIM_EQUAL(interface_len, 32)) || ((PRIM_EQUAL(interface_len, 16)) || ((PRIM_EQUAL(interface_len, 8)) || ((PRIM_EQUAL(interface_len, 4)) || ((PRIM_EQUAL(interface_len, 2)) || (PRIM_EQUAL(interface_len, 1)))))))))))))))))))))) {
StartAcceleratorTimer();;
	fftwf_example_api(api_in, api_out, interface_len, dir);;
	StopAcceleratorTimer();;
	for (int i9 = 0; i9 < n; i9++) {
		output[i9].imag = api_out[i9].im;
	};
	for (int i10 = 0; i10 < n; i10++) {
		output[i10].real = api_out[i10].re;
	}
} else {
STB_FFT(input, output, n);
}
}
void STB_FFT_accel(cmplx * input, cmplx * output, int n) {
STB_FFT_accel_internal((cmplx *) input, (cmplx *) output, (int) n);
}
int main(int argc, char **argv) {
    char *inpname = argv[1]; 
    output_file = argv[2]; 

    std::ifstream ifs(inpname); 
    json input_json = json::parse(ifs);
std::vector<cmplx> input_vec;
for (auto& elem : input_json["input"]) {
float input_innerreal = elem["real"];
float input_innerimag = elem["imag"];
cmplx input_inner = { input_innerreal, input_innerimag};
input_vec.push_back(input_inner);
}
cmplx *input = &input_vec[0];
int n = input_json["n"];
cmplx output[n];
clock_t begin = clock();
for (int i = 0; i < TIMES; i ++) {
	STB_FFT_accel(input, output, n);
}
clock_t end = clock();
std::cout << "Time: " << (double) (end - begin) / CLOCKS_PER_SEC << std::endl;
std::cout << "AccTime: " << (double) AcceleratorTotalNanos / CLOCKS_PER_SEC << std::endl;
write_output(input, output, n);
}
