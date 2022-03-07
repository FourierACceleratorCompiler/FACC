/* Orignal skeleton is: 
Pre: SKELETON:

With the array index wrappers power_quad_acc_output,Annon
And (fromvars) []
Under dimensions [power_quad_acc_n = n (=) ]
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers power_quad_acc_n
And (fromvars) [n]
Under dimensions []
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers power_quad_acc_input,im
And (fromvars) [input, imag]
Under dimensions [power_quad_acc_n = n (=) ]
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers power_quad_acc_input,re
And (fromvars) [input, real]
Under dimensions [power_quad_acc_n = n (=) ]
With conversion function IdentityConversion
Post: SKELETON:

With the array index wrappers output,imag
And (fromvars) [power_quad_acc_output, im]
Under dimensions [power_quad_acc_n = n (=) ]
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers output,real
And (fromvars) [power_quad_acc_output, re]
Under dimensions [power_quad_acc_n = n (=) ]
With conversion function IdentityConversion
*/

/* Typemap is :
 i18: int32
i17: int32
power_quad_acc_n: int32
power_quad_acc_input: array(complex_type: with dims power_quad_acc_n (=) )
input: array(cmplx: with dims n (=) )
i20: int32
output: array(cmplx: with dims n (=) )
n: int32
power_quad_acc_output: array(complex_type: with dims power_quad_acc_n (=) )
i19: int32
*/


extern "C" {
#include "../../benchmarks/Github/code/cpuimage_FFTResampler/self_contained_code.c"
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

void write_output(cmplx * input, cmplx * output, int n) {

    json output_json;
std::vector<json> output_temp_33;
for (unsigned int i34 = 0; i34 < n; i34++) {
cmplx output_temp_35 = output[i34];
json output_temp_36;

output_temp_36["real"] = output_temp_35.real;

output_temp_36["imag"] = output_temp_35.imag;
output_temp_33.push_back(output_temp_36);
}
output_json["output"] = output_temp_33;
std::ofstream out_str(output_file); 
out_str << std::setw(4) << output_json << std::endl;
}

void STB_FFT_accel_internal(cmplx * input,cmplx * output,int n) {

if ((PRIM_EQUAL(n, 16384)) || ((PRIM_EQUAL(n, 8192)) || ((PRIM_EQUAL(n, 4096)) || ((PRIM_EQUAL(n, 2048)) || ((PRIM_EQUAL(n, 1024)) || ((PRIM_EQUAL(n, 512)) || ((PRIM_EQUAL(n, 256)) || ((PRIM_EQUAL(n, 128)) || (PRIM_EQUAL(n, 64)))))))))) {
int power_quad_acc_n;;
	power_quad_acc_n = n;;
	complex_type power_quad_acc_output[power_quad_acc_n]__attribute__((__aligned__(64)));;
	complex_type power_quad_acc_input[power_quad_acc_n]__attribute__((__aligned__(64)));;
	for (int i17 = 0; i17 < power_quad_acc_n; i17++) {
		power_quad_acc_input[i17].im = input[i17].imag;
	};
	for (int i18 = 0; i18 < power_quad_acc_n; i18++) {
		power_quad_acc_input[i18].re = input[i18].real;
	};
	StartAcceleratorTimer();;
	fft_api(power_quad_acc_input, power_quad_acc_output, power_quad_acc_n);;
	StopAcceleratorTimer();;
	for (int i19 = 0; i19 < power_quad_acc_n; i19++) {
		output[i19].imag = power_quad_acc_output[i19].im;
	};
	for (int i20 = 0; i20 < power_quad_acc_n; i20++) {
		output[i20].real = power_quad_acc_output[i20].re;
	};
	
;
	
;
	

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
STB_FFT_accel(input, output, n);
clock_t end = clock();
std::cout << "Time: " << (double) (end - begin) / CLOCKS_PER_SEC << std::endl;
std::cout << "AccTime: " << (double) AcceleratorTotalNanos / CLOCKS_PER_SEC << std::endl;
write_output(input, output, n);
}