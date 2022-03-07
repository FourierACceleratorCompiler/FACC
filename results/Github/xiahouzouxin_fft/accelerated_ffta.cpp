/* Orignal skeleton is: 
Pre: SKELETON:

With the array index wrappers adi_acc_output,Annon
And (fromvars) []
Under dimensions [adi_acc_n = N]
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers adi_acc_n
And (fromvars) [N]
Under dimensions []
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers adi_acc_input,re
And (fromvars) [x, real]
Under dimensions [adi_acc_n = N]
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers adi_acc_input,im
And (fromvars) [x, imag]
Under dimensions [adi_acc_n = N]
With conversion function IdentityConversion
Post: SKELETON:

With the array index wrappers x,real
And (fromvars) [adi_acc_output, re]
Under dimensions [N = adi_acc_n]
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers x,imag
And (fromvars) [adi_acc_output, im]
Under dimensions [N = adi_acc_n]
With conversion function IdentityConversion
*/


extern "C" {
#include "../../benchmarks/Github/code/xiahouzouxin_fft/self_contained_code.c"
}



extern "C" {
#include "../../benchmarks/Accelerators/FFTA/adi_emulation.c"
}



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

void write_output(COMPLEX * x, int N) {

    json output_json;
std::vector<json> output_temp_33;
for (unsigned int i34 = 0; i34 < N; i34++) {
COMPLEX output_temp_35 = x[i34];
json output_temp_36;

output_temp_36["real"] = output_temp_35.real;

output_temp_36["imag"] = output_temp_35.imag;
output_temp_33.push_back(output_temp_36);
}
output_json["x"] = output_temp_33;
std::ofstream out_str(output_file); 
out_str << std::setw(4) << output_json << std::endl;
}

void fft_accel_internal(COMPLEX * x,int N) {

if ((PRIM_EQUAL(N, 16384)) || ((PRIM_EQUAL(N, 8192)) || ((PRIM_EQUAL(N, 4096)) || ((PRIM_EQUAL(N, 2048)) || ((PRIM_EQUAL(N, 1024)) || ((PRIM_EQUAL(N, 512)) || ((PRIM_EQUAL(N, 256)) || ((PRIM_EQUAL(N, 128)) || ((PRIM_EQUAL(N, 64)) || ((PRIM_EQUAL(N, 32)) || ((PRIM_EQUAL(N, 16)) || ((PRIM_EQUAL(N, 8)) || ((PRIM_EQUAL(N, 4)) || ((PRIM_EQUAL(N, 2)) || (PRIM_EQUAL(N, 1)))))))))))))))) {
static complex_float adi_acc_output[16384]__attribute__((__aligned__(64)));;
	static int adi_acc_n;;
	adi_acc_n = N;;
	static complex_float adi_acc_input[16384]__attribute__((__aligned__(64)));;
	for (int i2 = 0; i2 < adi_acc_n; i2++) {
		adi_acc_input[i2].re = x[i2].real;
	};
	for (int i3 = 0; i3 < adi_acc_n; i3++) {
		adi_acc_input[i3].im = x[i3].imag;
	};
	StartAcceleratorTimer();;
	accel_cfft_wrapper(adi_acc_input, adi_acc_output, adi_acc_n);;
	StopAcceleratorTimer();;
	for (int i4 = 0; i4 < N; i4++) {
		x[i4].real = adi_acc_output[i4].re;
	};
	for (int i5 = 0; i5 < N; i5++) {
		x[i5].imag = adi_acc_output[i5].im;
	};
	
;
	
;
	

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
fft_accel(x, N);
clock_t end = clock();
std::cout << "Time: " << (double) (end - begin) / CLOCKS_PER_SEC << std::endl;
std::cout << "AccTime: " << (double) AcceleratorTotalNanos / CLOCKS_PER_SEC << std::endl;
write_output(x, N);
}