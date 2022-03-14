
#include "../../benchmarks/Github/code/xiahouzouxin_fft/self_contained_code.c"


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
double AcceleratorTotalNanos = 0;
void StartAcceleratorTimer() {
	AcceleratorStart = clock();
}

void StopAcceleratorTimer() {
	AcceleratorTotalNanos +=
		(double) ((clock()) - AcceleratorStart) / CLOCKS_PER_SEC;
}

void write_output(COMPLEX * x, int N) {

    json output_json;
std::vector<json> output_temp_65;
for (unsigned int i66 = 0; i66 < N; i66++) {
COMPLEX output_temp_67 = x[i66];
json output_temp_68;

output_temp_68["real"] = output_temp_67.real;

output_temp_68["imag"] = output_temp_67.imag;
output_temp_65.push_back(output_temp_68);
}
output_json["x"] = output_temp_65;
std::ofstream out_str(output_file); 
out_str << std::setw(4) << output_json << std::endl;
}

void fft_accel_internal(COMPLEX * x,int N) {
short dir;;
	dir = -1;;
	int interface_len;;
	interface_len = N;;
	_complex_float_ api_out[interface_len];;
	_complex_float_ api_in[interface_len];;
	for (int i2 = 0; i2 < interface_len; i2++) {
		api_in[i2].re = x[i2].real;
	};
	for (int i3 = 0; i3 < interface_len; i3++) {
		api_in[i3].im = x[i3].imag;
	};
	
if ((PRIM_EQUAL(dir, -1)) && ((PRIM_EQUAL(interface_len, 524288)) || ((PRIM_EQUAL(interface_len, 262144)) || ((PRIM_EQUAL(interface_len, 131072)) || ((PRIM_EQUAL(interface_len, 65536)) || ((PRIM_EQUAL(interface_len, 32768)) || ((PRIM_EQUAL(interface_len, 16384)) || ((PRIM_EQUAL(interface_len, 8192)) || ((PRIM_EQUAL(interface_len, 4096)) || ((PRIM_EQUAL(interface_len, 2048)) || ((PRIM_EQUAL(interface_len, 1024)) || ((PRIM_EQUAL(interface_len, 512)) || ((PRIM_EQUAL(interface_len, 256)) || ((PRIM_EQUAL(interface_len, 128)) || ((PRIM_EQUAL(interface_len, 64)) || ((PRIM_EQUAL(interface_len, 32)) || ((PRIM_EQUAL(interface_len, 16)) || ((PRIM_EQUAL(interface_len, 8)) || ((PRIM_EQUAL(interface_len, 4)) || ((PRIM_EQUAL(interface_len, 2)) || (PRIM_EQUAL(interface_len, 1)))))))))))))))))))))) {
StartAcceleratorTimer();;
	fftwf_example_api(api_in, api_out, interface_len, dir);;
	StopAcceleratorTimer();;
	for (int i4 = 0; i4 < N; i4++) {
		x[i4].real = api_out[i4].re;
	};
	for (int i5 = 0; i5 < N; i5++) {
		x[i5].imag = api_out[i5].im;
	}
} else {
fft(x, N);
}
}
void fft_accel(COMPLEX * x, int N) {
fft_accel_internal((COMPLEX *) x, (int) N);
}
int main(int argc, char **argv) {
    char *inpname = argv[1]; 
    output_file = argv[2]; 

    std::ifstream ifs(inpname); 
    json input_json = json::parse(ifs);
std::vector<COMPLEX> x_vec;
for (auto& elem : input_json["x"]) {
float x_innerreal = elem["real"];
float x_innerimag = elem["imag"];
COMPLEX x_inner = { x_innerreal, x_innerimag};
x_vec.push_back(x_inner);
}
COMPLEX *x = &x_vec[0];
int N = input_json["N"];
clock_t begin = clock();
for (int i = 0; i < TIMES; i ++) {
	fft_accel(x, N);
}
clock_t end = clock();
std::cout << "Time: " << (double) (end - begin) / CLOCKS_PER_SEC << std::endl;
std::cout << "AccTime: " << AcceleratorTotalNanos << std::endl;
write_output(x, N);
}
