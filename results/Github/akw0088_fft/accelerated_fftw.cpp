/* Orignal skeleton is: 
Pre: SKELETON:

With the array index wrappers api_out,Annon
And (fromvars) []
Under dimensions [interface_len = num (=) ]
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers dir
And (fromvars) [Constant(-1)]
Under dimensions []
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers interface_len
And (fromvars) [num]
Under dimensions []
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers api_in,im
And (fromvars) [x, imag]
Under dimensions [interface_len = num (=) ]
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers api_in,re
And (fromvars) [x, real]
Under dimensions [interface_len = num (=) ]
With conversion function IdentityConversion
Post: SKELETON:

With the array index wrappers x,real
And (fromvars) [api_out, re]
Under dimensions [interface_len = num (=) ]
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers x,imag
And (fromvars) [api_out, im]
Under dimensions [interface_len = num (=) ]
With conversion function IdentityConversion
*/

/* Typemap is :
 dir: int16
num: int32
api_in: array(_complex_float_: with dims interface_len (=) )
i222: int32
x: array(j_complex_t: with dims num (=) )
i223: int32
w: array(j_complex_t: with dims num (=) )
interface_len: int32
i224: int32
i225: int32
api_out: array(_complex_float_: with dims interface_len (=) )
*/


extern "C" {
#include "../../benchmarks/Github/code/akw0088_fft/self_contained_code.c"
}



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
		(clock()) - AcceleratorStart;
}

void write_output(int num, j_complex_t * x, j_complex_t * w) {

    json output_json;
std::vector<json> output_temp_389;
for (unsigned int i390 = 0; i390 < num; i390++) {
j_complex_t output_temp_391 = x[i390];
json output_temp_392;

output_temp_392["real"] = output_temp_391.real;

output_temp_392["imag"] = output_temp_391.imag;
output_temp_389.push_back(output_temp_392);
}
output_json["x"] = output_temp_389;
std::ofstream out_str(output_file); 
out_str << std::setw(4) << output_json << std::endl;
}

void fft_c_accel_internal(int num,j_complex_t * x,j_complex_t * w) {

if ((PRIM_EQUAL(num, 524288)) || ((PRIM_EQUAL(num, 262144)) || ((PRIM_EQUAL(num, 131072)) || ((PRIM_EQUAL(num, 65536)) || ((PRIM_EQUAL(num, 32768)) || ((PRIM_EQUAL(num, 16384)) || ((PRIM_EQUAL(num, 8192)) || ((PRIM_EQUAL(num, 4096)) || ((PRIM_EQUAL(num, 2048)) || ((PRIM_EQUAL(num, 1024)) || ((PRIM_EQUAL(num, 512)) || ((PRIM_EQUAL(num, 256)) || ((PRIM_EQUAL(num, 128)) || ((PRIM_EQUAL(num, 64)) || ((PRIM_EQUAL(num, 32)) || ((PRIM_EQUAL(num, 16)) || ((PRIM_EQUAL(num, 8)) || ((PRIM_EQUAL(num, 4)) || (PRIM_EQUAL(num, 2)))))))))))))))))))) {
short dir;;
	dir = -1;;
	int interface_len;;
	interface_len = num;;
	_complex_float_ api_out[interface_len];;
	_complex_float_ api_in[interface_len];;
	for (int i222 = 0; i222 < interface_len; i222++) {
		api_in[i222].im = x[i222].imag;
	};
	for (int i223 = 0; i223 < interface_len; i223++) {
		api_in[i223].re = x[i223].real;
	};
	StartAcceleratorTimer();;
	fftwf_example_api(api_in, api_out, interface_len, dir);;
	StopAcceleratorTimer();;
	for (int i224 = 0; i224 < interface_len; i224++) {
		x[i224].real = api_out[i224].re;
	};
	for (int i225 = 0; i225 < interface_len; i225++) {
		x[i225].imag = api_out[i225].im;
	};
	
;
	
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
for (int i = 0; i < TIMES; i ++)
	fft_c_accel(num, x, w);
clock_t end = clock();
std::cout << "Time: " << (double) (end - begin) / CLOCKS_PER_SEC << std::endl;
std::cout << "AccTime: " << (double) AcceleratorTotalNanos / CLOCKS_PER_SEC << std::endl;
write_output(num, x, w);
}
