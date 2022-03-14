/* Orignal skeleton is: 
Pre: SKELETON:

With the array index wrappers api_out,Annon
And (fromvars) []
Under dimensions [interface_len = N (=) ]
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers dir
And (fromvars) [Constant(-1)]
Under dimensions []
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers interface_len
And (fromvars) [N]
Under dimensions []
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers api_in,im
And (fromvars) [Y, imag]
Under dimensions [interface_len = N (=) ]
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers api_in,re
And (fromvars) [Y, real]
Under dimensions [interface_len = N (=) ]
With conversion function IdentityConversion
Post: SKELETON:

With the array index wrappers Y,real
And (fromvars) [api_out, re]
Under dimensions [interface_len = N (=) ]
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers Y,imag
And (fromvars) [api_out, im]
Under dimensions [interface_len = N (=) ]
With conversion function IdentityConversion
*/

/* Typemap is :
 dir: int16
Y: array(COMPLEX: with dims N (=) )
N: int32
i25: int32
api_in: array(_complex_float_: with dims interface_len (=) )
i22: int32
i23: int32
i24: int32
interface_len: int32
api_out: array(_complex_float_: with dims interface_len (=) )
*/


extern "C" {
#include "../../benchmarks/Github/code/mozanunal_SimpleDSP/self_contained_code.c"
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

void write_output(COMPLEX * Y, int N) {

    json output_json;
std::vector<json> output_temp_69;
for (unsigned int i70 = 0; i70 < N; i70++) {
COMPLEX output_temp_71 = Y[i70];
json output_temp_72;

output_temp_72["real"] = output_temp_71.real;

output_temp_72["imag"] = output_temp_71.imag;
output_temp_69.push_back(output_temp_72);
}
output_json["Y"] = output_temp_69;
std::ofstream out_str(output_file); 
out_str << std::setw(4) << output_json << std::endl;
}

void FFT_accel_internal(COMPLEX * Y,int N) {

if (PRIM_EQUAL(N, 64)) {
short dir;;
	dir = -1;;
	int interface_len;;
	interface_len = N;;
	_complex_float_ api_out[interface_len];;
	_complex_float_ api_in[interface_len];;
	for (int i22 = 0; i22 < interface_len; i22++) {
		api_in[i22].im = Y[i22].imag;
	};
	for (int i23 = 0; i23 < interface_len; i23++) {
		api_in[i23].re = Y[i23].real;
	};
	StartAcceleratorTimer();;
	fftwf_example_api(api_in, api_out, interface_len, dir);;
	StopAcceleratorTimer();;
	for (int i24 = 0; i24 < interface_len; i24++) {
		Y[i24].real = api_out[i24].re;
	};
	for (int i25 = 0; i25 < interface_len; i25++) {
		Y[i25].imag = api_out[i25].im;
	};
	
;
	
;
	
;
	

} else {
FFT(Y, N);
}
}
void FFT_accel(COMPLEX * Y, int N) {
FFT_accel_internal((COMPLEX *) Y, (int) N);
}
int main(int argc, char **argv) {
    char *inpname = argv[1]; 
    output_file = argv[2]; 

    std::ifstream ifs(inpname); 
    json input_json = json::parse(ifs);
std::vector<COMPLEX> Y_vec;
for (auto& elem : input_json["Y"]) {
float Y_innerreal = elem["real"];
float Y_innerimag = elem["imag"];
COMPLEX Y_inner = { Y_innerreal, Y_innerimag};
Y_vec.push_back(Y_inner);
}
COMPLEX *Y = &Y_vec[0];
int N = input_json["N"];
clock_t begin = clock();
for (int i =0 ; i < TIMES; i ++) {
	FFT_accel(Y, N);
}
clock_t end = clock();
std::cout << "Time: " << (double) (end - begin) / CLOCKS_PER_SEC << std::endl;
std::cout << "AccTime: " << (double) AcceleratorTotalNanos / CLOCKS_PER_SEC << std::endl;
write_output(Y, N);
}
