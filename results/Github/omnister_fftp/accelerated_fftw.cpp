/* Orignal skeleton is: 
Pre: SKELETON:

With the array index wrappers api_out,Annon
And (fromvars) []
Under dimensions [interface_len = n (=) ]
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers dir
And (fromvars) [Constant(1)]
Under dimensions []
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers interface_len
And (fromvars) [n]
Under dimensions []
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers api_in,im
And (fromvars) [array, re]
Under dimensions [interface_len = n (=) ]
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers api_in,re
And (fromvars) [array, im]
Under dimensions [interface_len = n (=) ]
With conversion function IdentityConversion
Post: SKELETON:

With the array index wrappers array,im
And (fromvars) [api_out, re]
Under dimensions [interface_len = n (=) ]
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers array,re
And (fromvars) [api_out, im]
Under dimensions [interface_len = n (=) ]
With conversion function IdentityConversion
*/

/* Typemap is :
 i30: int32
i28: int32
dir: int16
api_in: array(_complex_float_: with dims interface_len (=) )
i27: int32
i29: int32
array: array(COMPLEX: with dims n (=) )
interface_len: int32
api_out: array(_complex_float_: with dims interface_len (=) )
n: int32
*/


extern "C" {
#include "../../benchmarks/Github/code/omnister_fftp/self_contained_code.c"
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

void write_output(int n, COMPLEX * array) {

    json output_json;
std::vector<json> output_temp_89;
for (unsigned int i90 = 0; i90 < n; i90++) {
COMPLEX output_temp_91 = array[i90];
json output_temp_92;

output_temp_92["re"] = output_temp_91.re;

output_temp_92["im"] = output_temp_91.im;
output_temp_89.push_back(output_temp_92);
}
output_json["array"] = output_temp_89;
std::ofstream out_str(output_file); 
out_str << std::setw(4) << output_json << std::endl;
}

COMPLEX * fft_1d_accel_internal(COMPLEX * array,int n) {

if ((PRIM_EQUAL(n, 524288)) || ((PRIM_EQUAL(n, 262144)) || ((PRIM_EQUAL(n, 131072)) || ((PRIM_EQUAL(n, 65536)) || ((PRIM_EQUAL(n, 32768)) || ((PRIM_EQUAL(n, 16384)) || ((PRIM_EQUAL(n, 8192)) || ((PRIM_EQUAL(n, 4096)) || ((PRIM_EQUAL(n, 2048)) || ((PRIM_EQUAL(n, 1024)) || ((PRIM_EQUAL(n, 512)) || ((PRIM_EQUAL(n, 256)) || ((PRIM_EQUAL(n, 128)) || ((PRIM_EQUAL(n, 64)) || ((PRIM_EQUAL(n, 32)) || ((PRIM_EQUAL(n, 16)) || ((PRIM_EQUAL(n, 8)) || ((PRIM_EQUAL(n, 4)) || (PRIM_EQUAL(n, 2)))))))))))))))))))) {
short dir;;
	dir = 1;;
	int interface_len;;
	interface_len = n;;
	_complex_float_ api_out[interface_len];
for (int i87 = 0; i87++; i87 < interface_len) {
_complex_float_ api_out_sub_element;

;
api_out[i87] = api_out_sub_element;
};
	_complex_float_ api_in[interface_len];
for (int i88 = 0; i88++; i88 < interface_len) {
_complex_float_ api_in_sub_element;

;
api_in[i88] = api_in_sub_element;
};
	for (int i27 = 0; i27 < interface_len; i27++) {
		api_in[i27].im = array[i27].re;
	};
	for (int i28 = 0; i28 < interface_len; i28++) {
		api_in[i28].re = array[i28].im;
	};
	StartAcceleratorTimer();;
	fftwf_example_api(api_in, api_out, interface_len, dir);;
	StopAcceleratorTimer();;
	for (int i29 = 0; i29 < interface_len; i29++) {
		array[i29].im = api_out[i29].re;
	};
	for (int i30 = 0; i30 < interface_len; i30++) {
		array[i30].re = api_out[i30].im;
	};
	
return array;;
	
;
	
;
	
;
	

} else {

return fft_1d(array, n);;
}
}
COMPLEX * fft_1d_accel(COMPLEX * array, int n) {
return (COMPLEX *)fft_1d_accel_internal((COMPLEX *) array, (int) n);
}
int main(int argc, char **argv) {
    char *inpname = argv[1]; 
    output_file = argv[2]; 

    std::ifstream ifs(inpname); 
    json input_json = json::parse(ifs);
std::vector<COMPLEX> array_vec;
for (auto& elem : input_json["array"]) {
float array_innerre = elem["re"];
float array_innerim = elem["im"];
COMPLEX array_inner = { array_innerre, array_innerim};
array_vec.push_back(array_inner);
}
COMPLEX *array = &array_vec[0];
int n = input_json["n"];
clock_t begin = clock();
for (int i = 0; i < TIMES; i ++) {
	array = fft_1d_accel(array, n);
}
clock_t end = clock();
std::cout << "Time: " << (double) (end - begin) / CLOCKS_PER_SEC << std::endl;
std::cout << "AccTime: " << (double) AcceleratorTotalNanos / CLOCKS_PER_SEC << std::endl;
write_output(n, array);
}
