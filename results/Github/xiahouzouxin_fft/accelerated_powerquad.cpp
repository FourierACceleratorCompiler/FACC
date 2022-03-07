/* Orignal skeleton is: 
Pre: SKELETON:

With the array index wrappers power_quad_acc_output,Annon
And (fromvars) []
Under dimensions [power_quad_acc_n = N (=) ]
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers power_quad_acc_n
And (fromvars) [N]
Under dimensions []
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers power_quad_acc_input,im
And (fromvars) [x, imag]
Under dimensions [power_quad_acc_n = N (=) ]
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers power_quad_acc_input,re
And (fromvars) [x, real]
Under dimensions [power_quad_acc_n = N (=) ]
With conversion function IdentityConversion
Post: SKELETON:

With the array index wrappers x,real
And (fromvars) [power_quad_acc_output, re]
Under dimensions [power_quad_acc_n = N (=) ]
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers x,imag
And (fromvars) [power_quad_acc_output, im]
Under dimensions [power_quad_acc_n = N (=) ]
With conversion function IdentityConversion
*/

/* Typemap is :
 N: int32
power_quad_acc_n: int32
i14: int32
x: array(COMPLEX: with dims N (=) )
power_quad_acc_input: array(complex_type: with dims power_quad_acc_n (=) )
i12: int32
i13: int32
i15: int32
power_quad_acc_output: array(complex_type: with dims power_quad_acc_n (=) )
*/


extern "C" {
#include "../../benchmarks/Github/code/xiahouzouxin_fft/self_contained_code.c"
}



extern "C" {
#include "../../benchmarks/Accelerators/PowerQuad/powerquad_emulation.c"
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

if ((PRIM_EQUAL(N, 512)) || ((PRIM_EQUAL(N, 256)) || ((PRIM_EQUAL(N, 128)) || (PRIM_EQUAL(N, 64))))) {
int power_quad_acc_n;;
	power_quad_acc_n = N;;
	complex_type power_quad_acc_output[power_quad_acc_n]__attribute__((__aligned__(64)));;
	complex_type power_quad_acc_input[power_quad_acc_n]__attribute__((__aligned__(64)));;
	for (int i12 = 0; i12 < power_quad_acc_n; i12++) {
		power_quad_acc_input[i12].im = x[i12].imag;
	};
	for (int i13 = 0; i13 < power_quad_acc_n; i13++) {
		power_quad_acc_input[i13].re = x[i13].real;
	};
	StartAcceleratorTimer();;
	fft_api(power_quad_acc_input, power_quad_acc_output, power_quad_acc_n);;
	StopAcceleratorTimer();;
	for (int i14 = 0; i14 < power_quad_acc_n; i14++) {
		x[i14].real = power_quad_acc_output[i14].re;
	};
	for (int i15 = 0; i15 < power_quad_acc_n; i15++) {
		x[i15].imag = power_quad_acc_output[i15].im;
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