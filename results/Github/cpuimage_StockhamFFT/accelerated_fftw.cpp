/* Orignal skeleton is: 
Pre: SKELETON:

With the array index wrappers api_out,Annon
And (fromvars) []
Under dimensions [interface_len = n (=) ]
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers dir
And (fromvars) [Constant(-1)]
Under dimensions []
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers interface_len
And (fromvars) [n]
Under dimensions []
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers api_in,im
And (fromvars) [x, imag]
Under dimensions [interface_len = n (=) ]
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers api_in,re
And (fromvars) [x, real]
Under dimensions [interface_len = n (=) ]
With conversion function IdentityConversion
Post: SKELETON:

With the array index wrappers x,real
And (fromvars) [api_out, re]
Under dimensions [interface_len = n (=) ]
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers x,imag
And (fromvars) [api_out, im]
Under dimensions [interface_len = n (=) ]
With conversion function IdentityConversion
*/

/* Typemap is :
 dir: int16
i25: int32
api_in: array(_complex_float_: with dims interface_len (=) )
i22: int32
x: array(complex_t: with dims n (=) )
i23: int32
i24: int32
interface_len: int32
api_out: array(_complex_float_: with dims interface_len (=) )
n: int32
*/

#include <clib/fft_synth/lib.h>
extern "C" {
#include "../../benchmarks/Github/code/cpuimage_StockhamFFT/self_contained_code.c"
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

void write_output(complex_t * x, int n) {

    json output_json;
std::vector<json> output_temp_69;
for (unsigned int i70 = 0; i70 < n; i70++) {
complex_t output_temp_71 = x[i70];
json output_temp_72;

output_temp_72["real"] = output_temp_71.real;

output_temp_72["imag"] = output_temp_71.imag;
output_temp_69.push_back(output_temp_72);
}
output_json["x"] = output_temp_69;
std::ofstream out_str(output_file); 
out_str << std::setw(4) << output_json << std::endl;
}

void fft_accel_internal(complex_t * x,int n) {

if ((PRIM_EQUAL(n, 524288)) || ((PRIM_EQUAL(n, 262144)) || ((PRIM_EQUAL(n, 131072)) || ((PRIM_EQUAL(n, 65536)) || ((PRIM_EQUAL(n, 32768)) || ((PRIM_EQUAL(n, 16384)) || ((PRIM_EQUAL(n, 8192)) || ((PRIM_EQUAL(n, 4096)) || ((PRIM_EQUAL(n, 2048)) || ((PRIM_EQUAL(n, 1024)) || ((PRIM_EQUAL(n, 512)) || ((PRIM_EQUAL(n, 256)) || ((PRIM_EQUAL(n, 128)) || ((PRIM_EQUAL(n, 64)) || ((PRIM_EQUAL(n, 32)) || ((PRIM_EQUAL(n, 16)) || ((PRIM_EQUAL(n, 8)) || ((PRIM_EQUAL(n, 4)) || (PRIM_EQUAL(n, 2)))))))))))))))))))) {
short dir;;
	dir = -1;;
	int interface_len;;
	interface_len = n;;
	_complex_float_ api_out[interface_len];;
	_complex_float_ api_in[interface_len];;
	for (int i22 = 0; i22 < interface_len; i22++) {
		api_in[i22].im = x[i22].imag;
	};
	for (int i23 = 0; i23 < interface_len; i23++) {
		api_in[i23].re = x[i23].real;
	};
	StartAcceleratorTimer();;
	fftwf_example_api(api_in, api_out, interface_len, dir);;
	StopAcceleratorTimer();;
	for (int i24 = 0; i24 < interface_len; i24++) {
		x[i24].real = api_out[i24].re;
	};
	for (int i25 = 0; i25 < interface_len; i25++) {
		x[i25].imag = api_out[i25].im;
	};
	
;
	
;
	
;
	
;
	
if (GREATER_THAN(n, -1)) {
ARRAY_NORM_POSTIND(x, real, n);;
	ARRAY_NORM_POSTIND(x, imag, n);
} else {
;
}
} else {
fft(x, n);
}
}
void fft_accel(complex_t * x, int n) {
fft_accel_internal((complex_t *) x, (int) n);
}
int main(int argc, char **argv) {
    char *inpname = argv[1]; 
    output_file = argv[2]; 

    std::ifstream ifs(inpname); 
    json input_json = json::parse(ifs);
std::vector<complex_t> x_vec;
for (auto& elem : input_json["x"]) {
float x_innerreal = elem["real"];
float x_innerimag = elem["imag"];
complex_t x_inner = { x_innerreal, x_innerimag};
x_vec.push_back(x_inner);
}
complex_t *x = &x_vec[0];
int n = input_json["n"];
clock_t begin = clock();
for (int i = 0; i < TIMES; i ++) {
	fft_accel(x, n);
}
clock_t end = clock();
std::cout << "Time: " << (double) (end - begin) / CLOCKS_PER_SEC << std::endl;
std::cout << "AccTime: " << (double) AcceleratorTotalNanos / CLOCKS_PER_SEC << std::endl;
write_output(x, n);
}
