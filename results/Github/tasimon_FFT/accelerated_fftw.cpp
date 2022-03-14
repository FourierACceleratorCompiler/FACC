/* Orignal skeleton is: 
Pre: SKELETON:

With the array index wrappers api_out,Annon
And (fromvars) []
Under dimensions [64]
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers dir
And (fromvars) [Constant(1)]
Under dimensions []
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers interface_len
And (fromvars) [Constant(64)]
Under dimensions []
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers api_in,im
And (fromvars) [Z, f32_2]
Under dimensions [64]
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers api_in,re
And (fromvars) [Z, f32_1]
Under dimensions [64]
With conversion function IdentityConversion
Post: SKELETON:

With the array index wrappers Z,f32_2
And (fromvars) [api_out, im]
Under dimensions [64]
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers Z,f32_1
And (fromvars) [api_out, re]
Under dimensions [64]
With conversion function IdentityConversion
*/

/* Typemap is :
 dir: int16
api_in: array(_complex_float_: with dims interface_len (=) )
interface_len: int32
i80: int32
i79: int32
i78: int32
i77: int32
api_out: array(_complex_float_: with dims interface_len (=) )
Z: array(facc_2xf32_t: with dims 64)
*/


extern "C" {
#include "../../benchmarks/Github/code/tasimon_FFT/self_contained_code.c"
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

void write_output(float * Z) {

    json output_json;
std::vector<json> output_temp_228;
for (unsigned int i229 = 0; i229 < 128; i229++) {
float output_temp_230 = Z[i229];

output_temp_228.push_back(output_temp_230);
}
output_json["Z"] = output_temp_228;
std::ofstream out_str(output_file); 
out_str << std::setw(4) << output_json << std::endl;
}

void fft64_accel_internal(facc_2xf32_t * Z) {
short dir;;
	dir = 1;;
	int interface_len;;
	interface_len = 64;;
	_complex_float_ api_out[interface_len];;
	_complex_float_ api_in[interface_len];;
	for (int i77 = 0; i77 < 64; i77++) {
		api_in[i77].im = Z[i77].f32_2;
	};
	for (int i78 = 0; i78 < 64; i78++) {
		api_in[i78].re = Z[i78].f32_1;
	};
	StartAcceleratorTimer();;
	fftwf_example_api(api_in, api_out, interface_len, dir);;
	StopAcceleratorTimer();;
	for (int i79 = 0; i79 < 64; i79++) {
		Z[i79].f32_2 = api_out[i79].im;
	};
	for (int i80 = 0; i80 < 64; i80++) {
		Z[i80].f32_1 = api_out[i80].re;
	};
	
;
	
;
	
;
	

}
void fft64_accel(float * Z) {
fft64_accel_internal((facc_2xf32_t *) Z);
}
int main(int argc, char **argv) {
    char *inpname = argv[1]; 
    output_file = argv[2]; 

    std::ifstream ifs(inpname); 
    json input_json = json::parse(ifs);
std::vector<float> Z_vec;
for (auto& elem : input_json["Z"]) {
float Z_inner = elem;
Z_vec.push_back(Z_inner);
}
float *Z = &Z_vec[0];
clock_t begin = clock();
for (int i = 0; i < TIMES; i ++)
	fft64_accel(Z);
clock_t end = clock();
std::cout << "Time: " << (double) (end - begin) / CLOCKS_PER_SEC << std::endl;
std::cout << "AccTime: " << (double) AcceleratorTotalNanos / CLOCKS_PER_SEC << std::endl;
write_output(Z);
}
