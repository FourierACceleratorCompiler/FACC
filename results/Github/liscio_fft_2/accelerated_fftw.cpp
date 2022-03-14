/* Orignal skeleton is: 
Pre: SKELETON:

With the array index wrappers api_out,Annon
And (fromvars) []
Under dimensions [interface_len = N (=) ]
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers dir
And (fromvars) [Constant(1)]
Under dimensions []
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers interface_len
And (fromvars) [N]
Under dimensions []
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers api_in,im
And (fromvars) [x, im]
Under dimensions [interface_len = N (=) ]
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers api_in,re
And (fromvars) [x, re]
Under dimensions [interface_len = N (=) ]
With conversion function IdentityConversion
Post: SKELETON:

With the array index wrappers returnvar,im
And (fromvars) [api_out, im]
Under dimensions [interface_len = N (=) ]
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers returnvar,re
And (fromvars) [api_out, re]
Under dimensions [interface_len = N (=) ]
With conversion function IdentityConversion
*/

/* Typemap is :
 dir: int16
i38: int32
N: int32
api_in: array(_complex_float_: with dims interface_len (=) )
i37: int32
x: array(_complex_double_: with dims N (=) )
interface_len: int32
i40: int32
returnvar: array(_complex_double_: with dims N (=) )
i39: int32
api_out: array(_complex_float_: with dims interface_len (=) )
*/


extern "C" {
#include "../../benchmarks/Github/code/liscio_fft_2/self_contained_code.h"
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


void write_output(_complex_double_ * x, int N, _complex_double_ * returnvar) {

    json output_json;
std::vector<json> output_temp_69;
for (unsigned int i70 = 0; i70 < N; i70++) {
_complex_double_ output_temp_71 = returnvar[i70];
json output_temp_72;

output_temp_72["re"] = output_temp_71.re;

output_temp_72["im"] = output_temp_71.im;
output_temp_69.push_back(output_temp_72);
}
output_json["returnvar"] = output_temp_69;
std::ofstream out_str(output_file); 
out_str << std::setw(4) << output_json << std::endl;
}

_complex_double_ * FFT_wrapper_accel_internal(_complex_double_ * x,int N) {

if ((PRIM_EQUAL(N, 524288)) || ((PRIM_EQUAL(N, 262144)) || ((PRIM_EQUAL(N, 131072)) || ((PRIM_EQUAL(N, 65536)) || ((PRIM_EQUAL(N, 32768)) || ((PRIM_EQUAL(N, 16384)) || ((PRIM_EQUAL(N, 8192)) || ((PRIM_EQUAL(N, 4096)) || ((PRIM_EQUAL(N, 2048)) || ((PRIM_EQUAL(N, 1024)) || ((PRIM_EQUAL(N, 512)) || ((PRIM_EQUAL(N, 256)) || ((PRIM_EQUAL(N, 128)) || ((PRIM_EQUAL(N, 64)) || ((PRIM_EQUAL(N, 32)) || ((PRIM_EQUAL(N, 16)) || ((PRIM_EQUAL(N, 8)) || ((PRIM_EQUAL(N, 4)) || (PRIM_EQUAL(N, 2)))))))))))))))))))) {
	static short dir;;
	dir = 1;;
	static int interface_len;;
	interface_len = N;;
	_complex_float_ api_out[interface_len];;
	_complex_float_ api_in[interface_len];;
	for (int i37 = 0; i37 < interface_len; i37++) {
		api_in[i37].im = x[i37].im;
	};
	for (int i38 = 0; i38 < interface_len; i38++) {
		api_in[i38].re = x[i38].re;
	};
	fftwf_example_api(api_in, api_out, interface_len, dir);;
	StartAcceleratorTimer();;
	_complex_double_* returnvar = (_complex_double_*) facc_malloc (0, sizeof(_complex_double_)*N);;
	StopAcceleratorTimer();;
	for (int i39 = 0; i39 < interface_len; i39++) {
		returnvar[i39].im = api_out[i39].im;
	};
	for (int i40 = 0; i40 < interface_len; i40++) {
		returnvar[i40].re = api_out[i40].re;
	};
	
return returnvar;;
	
;
	
;
	
;
	

} else {

return FFT_wrapper(x, N);;
}
}
_complex_double_ * FFT_wrapper_accel(_complex_double_ * x, int N) {
return (_complex_double_ *)FFT_wrapper_accel_internal((_complex_double_ *) x, (int) N);
}
int main(int argc, char **argv) {
    char *inpname = argv[1]; 
    output_file = argv[2]; 

    std::ifstream ifs(inpname); 
    json input_json = json::parse(ifs);
std::vector<_complex_double_> x_vec;
for (auto& elem : input_json["x"]) {
double x_innerre = elem["re"];
double x_innerim = elem["im"];
_complex_double_ x_inner = { x_innerre, x_innerim};
x_vec.push_back(x_inner);
}
_complex_double_ *x = &x_vec[0];
int N = input_json["N"];

_complex_double_ * returnvar = nullptr;
for (int i = 0; i < TIMES; i ++) {
	if (returnvar != nullptr) {
		free(returnvar);
	}
	returnvar = FFT_wrapper_accel(x, N);
}


write_output(x, N, returnvar);
}
