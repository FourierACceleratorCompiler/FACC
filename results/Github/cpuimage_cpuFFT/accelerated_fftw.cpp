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
And (fromvars) [input, imag]
Under dimensions [interface_len = n (=) ]
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers api_in,re
And (fromvars) [input, real]
Under dimensions [interface_len = n (=) ]
With conversion function IdentityConversion
Post: SKELETON:

With the array index wrappers output,imag
And (fromvars) [api_out, im]
Under dimensions [interface_len = n (=) ]
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers output,real
And (fromvars) [api_out, re]
Under dimensions [interface_len = n (=) ]
With conversion function IdentityConversion
*/

/* Typemap is :
 i30: int32
i28: int32
dir: int16
api_in: array(_complex_float_: with dims interface_len (=) )
i27: int32
input: array(cmplx: with dims n (=) )
i29: int32
output: array(cmplx: with dims n (=) )
interface_len: int32
api_out: array(_complex_float_: with dims interface_len (=) )
n: int32
*/


extern "C" {
#include "../../benchmarks/Github/code/cpuimage_cpuFFT/self_contained_code.c"
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

void write_output(cmplx * input, cmplx * output, int n) {

    json output_json;
std::vector<json> output_temp_69;
for (unsigned int i70 = 0; i70 < n; i70++) {
cmplx output_temp_71 = output[i70];
json output_temp_72;

output_temp_72["real"] = output_temp_71.real;

output_temp_72["imag"] = output_temp_71.imag;
output_temp_69.push_back(output_temp_72);
}
output_json["output"] = output_temp_69;
std::ofstream out_str(output_file); 
out_str << std::setw(4) << output_json << std::endl;
}

void FFT_accel_internal(cmplx * input,cmplx * output,int n) {

if ((PRIM_EQUAL(n, 524288)) || ((PRIM_EQUAL(n, 262144)) || ((PRIM_EQUAL(n, 131072)) || ((PRIM_EQUAL(n, 65536)) || ((PRIM_EQUAL(n, 32768)) || ((PRIM_EQUAL(n, 16384)) || ((PRIM_EQUAL(n, 8192)) || ((PRIM_EQUAL(n, 4096)) || ((PRIM_EQUAL(n, 2048)) || ((PRIM_EQUAL(n, 1024)) || ((PRIM_EQUAL(n, 512)) || ((PRIM_EQUAL(n, 256)) || ((PRIM_EQUAL(n, 128)) || ((PRIM_EQUAL(n, 64)) || ((PRIM_EQUAL(n, 32)) || ((PRIM_EQUAL(n, 16)) || ((PRIM_EQUAL(n, 8)) || ((PRIM_EQUAL(n, 4)) || (PRIM_EQUAL(n, 2)))))))))))))))))))) {
short dir;;
	dir = -1;;
	int interface_len;;
	interface_len = n;;
	_complex_float_ api_out[interface_len];;
	_complex_float_ api_in[interface_len];;
	for (int i27 = 0; i27 < interface_len; i27++) {
		api_in[i27].im = input[i27].imag;
	};
	for (int i28 = 0; i28 < interface_len; i28++) {
		api_in[i28].re = input[i28].real;
	};
	StartAcceleratorTimer();;
	fftwf_example_api(api_in, api_out, interface_len, dir);;
	StopAcceleratorTimer();;
	for (int i29 = 0; i29 < interface_len; i29++) {
		output[i29].imag = api_out[i29].im;
	};
	for (int i30 = 0; i30 < interface_len; i30++) {
		output[i30].real = api_out[i30].re;
	};
	
;
	
;
	
;
	

} else {
FFT(input, output, n);
}
}
void FFT_accel(cmplx * input, cmplx * output, int n) {
FFT_accel_internal((cmplx *) input, (cmplx *) output, (int) n);
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
for (int i = 0; i < TIMES; i ++)
	FFT_accel(input, output, n);
clock_t end = clock();
std::cout << "Time: " << (double) (end - begin) / CLOCKS_PER_SEC << std::endl;
std::cout << "AccTime: " << (double) AcceleratorTotalNanos / CLOCKS_PER_SEC << std::endl;
write_output(input, output, n);
}
