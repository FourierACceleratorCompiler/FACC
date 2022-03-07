/* Orignal skeleton is: 
Pre: SKELETON:

With the array index wrappers power_quad_acc_output,Annon
And (fromvars) []
Under dimensions [power_quad_acc_n = num (=) ]
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers power_quad_acc_n
And (fromvars) [num]
Under dimensions []
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers power_quad_acc_input,im
And (fromvars) [x, imag]
Under dimensions [power_quad_acc_n = num (=) ]
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers power_quad_acc_input,re
And (fromvars) [x, real]
Under dimensions [power_quad_acc_n = num (=) ]
With conversion function IdentityConversion
Post: SKELETON:

With the array index wrappers x,real
And (fromvars) [power_quad_acc_output, re]
Under dimensions [power_quad_acc_n = num (=) ]
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers x,imag
And (fromvars) [power_quad_acc_output, im]
Under dimensions [power_quad_acc_n = num (=) ]
With conversion function IdentityConversion
*/

/* Typemap is :
 i115: int32
power_quad_acc_n: int32
num: int32
i113: int32
i112: int32
x: array(j_complex_t: with dims num (=) )
power_quad_acc_input: array(complex_type: with dims power_quad_acc_n (=) )
w: array(j_complex_t: with dims num (=) )
i114: int32
power_quad_acc_output: array(complex_type: with dims power_quad_acc_n (=) )
*/


extern "C" {
#include "../../benchmarks/Github/code/akw0088_fft/self_contained_code.c"
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
int power_quad_acc_n;;
	power_quad_acc_n = num;;
	complex_type power_quad_acc_output[power_quad_acc_n]__attribute__((__aligned__(64)));;
	complex_type power_quad_acc_input[power_quad_acc_n]__attribute__((__aligned__(64)));;
	for (int i112 = 0; i112 < power_quad_acc_n; i112++) {
		power_quad_acc_input[i112].im = x[i112].imag;
	};
	for (int i113 = 0; i113 < power_quad_acc_n; i113++) {
		power_quad_acc_input[i113].re = x[i113].real;
	};
	StartAcceleratorTimer();;
	accel_cfft_wrapper(power_quad_acc_input, power_quad_acc_output, power_quad_acc_n);;
	StopAcceleratorTimer();;
	for (int i114 = 0; i114 < power_quad_acc_n; i114++) {
		x[i114].real = power_quad_acc_output[i114].re;
	};
	for (int i115 = 0; i115 < power_quad_acc_n; i115++) {
		x[i115].imag = power_quad_acc_output[i115].im;
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