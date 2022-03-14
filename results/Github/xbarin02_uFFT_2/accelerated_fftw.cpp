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
And (fromvars) [vector, im]
Under dimensions [interface_len = N (=) ]
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers api_in,re
And (fromvars) [vector, re]
Under dimensions [interface_len = N (=) ]
With conversion function IdentityConversion
Post: SKELETON:

With the array index wrappers vector,im
And (fromvars) [api_out, im]
Under dimensions [interface_len = N (=) ]
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers vector,re
And (fromvars) [api_out, re]
Under dimensions [interface_len = N (=) ]
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers returnv
And (fromvars) [Constant(0)]
Under dimensions []
With conversion function IdentityConversion
*/

/* Typemap is :
 dir: int16
vector: array(_float_complex_: with dims N (=) )
N: int64
api_in: array(_complex_float_: with dims interface_len (=) )
returnv: int32
interface_len: int32
i80: int32
i79: int32
i78: int32
i77: int32
api_out: array(_complex_float_: with dims interface_len (=) )
*/


extern "C" {
#include "../../benchmarks/Github/code/xbarin02_uFFT_2/self_contained_code.c"
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

void write_output(_float_complex_ * vector, long int N, int returnv) {

    json output_json;
std::vector<json> output_temp_197;
for (unsigned int i198 = 0; i198 < N; i198++) {
_float_complex_ output_temp_199 = vector[i198];
json output_temp_200;

output_temp_200["re"] = output_temp_199.re;

output_temp_200["im"] = output_temp_199.im;
output_temp_197.push_back(output_temp_200);
}
output_json["vector"] = output_temp_197;

output_json["returnv"] = returnv;
std::ofstream out_str(output_file); 
out_str << std::setw(4) << output_json << std::endl;
}

int fft_wrapper_accel_internal(_float_complex_ * vector,long int N) {

if ((PRIM_EQUAL(N, 524288)) || ((PRIM_EQUAL(N, 262144)) || ((PRIM_EQUAL(N, 131072)) || ((PRIM_EQUAL(N, 65536)) || ((PRIM_EQUAL(N, 32768)) || ((PRIM_EQUAL(N, 16384)) || ((PRIM_EQUAL(N, 8192)) || ((PRIM_EQUAL(N, 4096)) || ((PRIM_EQUAL(N, 2048)) || ((PRIM_EQUAL(N, 1024)) || ((PRIM_EQUAL(N, 512)) || ((PRIM_EQUAL(N, 256)) || ((PRIM_EQUAL(N, 128)) || ((PRIM_EQUAL(N, 64)) || ((PRIM_EQUAL(N, 32)) || ((PRIM_EQUAL(N, 16)) || ((PRIM_EQUAL(N, 8)) || ((PRIM_EQUAL(N, 4)) || (PRIM_EQUAL(N, 2)))))))))))))))))))) {
short dir;;
	dir = -1;;
	int interface_len;;
	interface_len = N;;
	_complex_float_ api_out[interface_len];;
	_complex_float_ api_in[interface_len];;
	for (int i77 = 0; i77 < interface_len; i77++) {
		api_in[i77].im = vector[i77].im;
	};
	for (int i78 = 0; i78 < interface_len; i78++) {
		api_in[i78].re = vector[i78].re;
	};
	StartAcceleratorTimer();;
	fftwf_example_api(api_in, api_out, interface_len, dir);;
	StopAcceleratorTimer();;
	for (int i79 = 0; i79 < interface_len; i79++) {
		vector[i79].im = api_out[i79].im;
	};
	for (int i80 = 0; i80 < interface_len; i80++) {
		vector[i80].re = api_out[i80].re;
	};
	int returnv;;
	returnv = 0;;
	
return returnv;;
	
;
	
;
	
;
	

} else {

return fft_wrapper(vector, N);;
}
}
int fft_wrapper_accel(_float_complex_ * vector, long int N) {
return (int)fft_wrapper_accel_internal((_float_complex_ *) vector, (long int) N);
}
int main(int argc, char **argv) {
    char *inpname = argv[1]; 
    output_file = argv[2]; 

    std::ifstream ifs(inpname); 
    json input_json = json::parse(ifs);
std::vector<_float_complex_> vector_vec;
for (auto& elem : input_json["vector"]) {
float vector_innerre = elem["re"];
float vector_innerim = elem["im"];
_float_complex_ vector_inner = { vector_innerre, vector_innerim};
vector_vec.push_back(vector_inner);
}
_float_complex_ *vector = &vector_vec[0];
long int N = input_json["N"];
clock_t begin = clock();
int returnv = 0;
for (int i = 0; i < TIMES; i ++)
	returnv = fft_wrapper_accel(vector, N);
clock_t end = clock();
std::cout << "Time: " << (double) (end - begin) / CLOCKS_PER_SEC << std::endl;
std::cout << "AccTime: " << (double) AcceleratorTotalNanos / CLOCKS_PER_SEC << std::endl;
write_output(vector, N, returnv);
}
