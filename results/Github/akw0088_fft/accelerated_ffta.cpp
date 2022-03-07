/* Orignal skeleton is: 
Pre: SKELETON:

With the array index wrappers adi_acc_output,Annon
And (fromvars) []
Under dimensions [adi_acc_n = num (=) ]
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers adi_acc_n
And (fromvars) [num]
Under dimensions []
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers adi_acc_input,re
And (fromvars) [x, real]
Under dimensions [adi_acc_n = num (=) ]
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers adi_acc_input,im
And (fromvars) [x, imag]
Under dimensions [adi_acc_n = num (=) ]
With conversion function IdentityConversion
Post: SKELETON:

With the array index wrappers x,real
And (fromvars) [adi_acc_output, re]
Under dimensions [adi_acc_n = num (=) ]
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers x,imag
And (fromvars) [adi_acc_output, im]
Under dimensions [adi_acc_n = num (=) ]
With conversion function IdentityConversion
*/

/* Typemap is :
 num: int32
x: array(j_complex_t: with dims num (=) )
i82: int32
adi_acc_n: int32
w: array(j_complex_t: with dims num (=) )
i84: int32
i83: int32
adi_acc_input: array(complex_float: with dims adi_acc_n (=) )
adi_acc_output: array(complex_float: with dims adi_acc_n (=) )
i85: int32
*/


extern "C" {
#include "../../benchmarks/Github/code/akw0088_fft/self_contained_code.c"
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

void write_output(int num, j_complex_t * x, j_complex_t * w) {

    json output_json;
std::vector<json> output_temp_193;
for (unsigned int i194 = 0; i194 < num; i194++) {
j_complex_t output_temp_195 = x[i194];
json output_temp_196;

output_temp_196["real"] = output_temp_195.real;

output_temp_196["imag"] = output_temp_195.imag;
output_temp_193.push_back(output_temp_196);
}
output_json["x"] = output_temp_193;
std::ofstream out_str(output_file); 
out_str << std::setw(4) << output_json << std::endl;
}

void fft_c_accel_internal(int num,j_complex_t * x,j_complex_t * w) {

if ((PRIM_EQUAL(num, 16384)) || ((PRIM_EQUAL(num, 8192)) || ((PRIM_EQUAL(num, 4096)) || ((PRIM_EQUAL(num, 2048)) || ((PRIM_EQUAL(num, 1024)) || ((PRIM_EQUAL(num, 512)) || ((PRIM_EQUAL(num, 256)) || ((PRIM_EQUAL(num, 128)) || (PRIM_EQUAL(num, 64)))))))))) {
static complex_float adi_acc_output[16384]__attribute__((__aligned__(64)));;
	static int adi_acc_n;;
	adi_acc_n = num;;
	static complex_float adi_acc_input[16384]__attribute__((__aligned__(64)));;
	for (int i82 = 0; i82 < adi_acc_n; i82++) {
		adi_acc_input[i82].re = x[i82].real;
	};
	for (int i83 = 0; i83 < adi_acc_n; i83++) {
		adi_acc_input[i83].im = x[i83].imag;
	};
	StartAcceleratorTimer();;
	accel_cfft_wrapper(adi_acc_input, adi_acc_output, adi_acc_n);;
	StopAcceleratorTimer();;
	for (int i84 = 0; i84 < adi_acc_n; i84++) {
		x[i84].real = adi_acc_output[i84].re;
	};
	for (int i85 = 0; i85 < adi_acc_n; i85++) {
		x[i85].imag = adi_acc_output[i85].im;
	};
	
;
	
;
	

} else {
fft_c(num, x, w);
}
}
void fft_c_accel(int num, j_complex_t * x, j_complex_t * w) {
fft_c_accel_internal((int) num, (j_complex_t *) x, (j_complex_t *) w);
}
int main(int argc, char **argv) {
    char *inpname = argv[1]; 
    output_file = argv[2]; 

    std::ifstream ifs(inpname); 
    json input_json = json::parse(ifs);
int num = input_json["num"];
std::vector<j_complex_t> x_vec;
for (auto& elem : input_json["x"]) {
float x_innerreal = elem["real"];
float x_innerimag = elem["imag"];
j_complex_t x_inner = { x_innerreal, x_innerimag};
x_vec.push_back(x_inner);
}
j_complex_t *x = &x_vec[0];
std::vector<j_complex_t> w_vec;
for (auto& elem : input_json["w"]) {
float w_innerreal = elem["real"];
float w_innerimag = elem["imag"];
j_complex_t w_inner = { w_innerreal, w_innerimag};
w_vec.push_back(w_inner);
}
j_complex_t *w = &w_vec[0];
clock_t begin = clock();
fft_c_accel(num, x, w);
clock_t end = clock();
std::cout << "Time: " << (double) (end - begin) / CLOCKS_PER_SEC << std::endl;
std::cout << "AccTime: " << (double) AcceleratorTotalNanos / CLOCKS_PER_SEC << std::endl;
write_output(num, x, w);
}
