/* Orignal skeleton is: 
Pre: SKELETON:

With the array index wrappers adi_acc_output,Annon
And (fromvars) []
Under dimensions [adi_acc_n = n (=) ]
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers adi_acc_n
And (fromvars) [n]
Under dimensions []
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers adi_acc_input,re
And (fromvars) [array, re]
Under dimensions [adi_acc_n = n (=) ]
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers adi_acc_input,im
And (fromvars) [array, im]
Under dimensions [adi_acc_n = n (=) ]
With conversion function IdentityConversion
Post: SKELETON:

With the array index wrappers array,im
And (fromvars) [adi_acc_output, im]
Under dimensions [adi_acc_n = n (=) ]
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers array,re
And (fromvars) [adi_acc_output, re]
Under dimensions [adi_acc_n = n (=) ]
With conversion function IdentityConversion
*/

/* Typemap is :
 i14: int32
adi_acc_n: int32
i12: int32
array: array(COMPLEX: with dims n (=) )
adi_acc_input: array(complex_float: with dims adi_acc_n (=) )
i13: int32
n: int32
i15: int32
adi_acc_output: array(complex_float: with dims adi_acc_n (=) )
*/


extern "C" {
#include "../../benchmarks/Github/code/omnister_fftp/self_contained_code.c"
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

void write_output(int n, COMPLEX * array) {

    json output_json;
std::vector<json> output_temp_43;
for (unsigned int i44 = 0; i44 < n; i44++) {
COMPLEX output_temp_45 = array[i44];
json output_temp_46;

output_temp_46["re"] = output_temp_45.re;

output_temp_46["im"] = output_temp_45.im;
output_temp_43.push_back(output_temp_46);
}
output_json["array"] = output_temp_43;
std::ofstream out_str(output_file); 
out_str << std::setw(4) << output_json << std::endl;
}

COMPLEX * fft_1d_accel_internal(COMPLEX * array,int n) {

if ((PRIM_EQUAL(n, 16384)) || ((PRIM_EQUAL(n, 8192)) || ((PRIM_EQUAL(n, 4096)) || ((PRIM_EQUAL(n, 2048)) || ((PRIM_EQUAL(n, 1024)) || ((PRIM_EQUAL(n, 512)) || ((PRIM_EQUAL(n, 256)) || ((PRIM_EQUAL(n, 128)) || (PRIM_EQUAL(n, 64)))))))))) {
static complex_float adi_acc_output[16384]__attribute__((__aligned__(64)));
for (int i41 = 0; i41++; i41 < 16384) {
static complex_float adi_acc_output_sub_element;

;
adi_acc_output[i41] = adi_acc_output_sub_element;
};
	static int adi_acc_n;;
	adi_acc_n = n;;
	static complex_float adi_acc_input[16384]__attribute__((__aligned__(64)));
for (int i42 = 0; i42++; i42 < 16384) {
static complex_float adi_acc_input_sub_element;

;
adi_acc_input[i42] = adi_acc_input_sub_element;
};
	for (int i12 = 0; i12 < adi_acc_n; i12++) {
		adi_acc_input[i12].re = array[i12].re;
	};
	for (int i13 = 0; i13 < adi_acc_n; i13++) {
		adi_acc_input[i13].im = array[i13].im;
	};
	StartAcceleratorTimer();;
	accel_cfft_wrapper(adi_acc_input, adi_acc_output, adi_acc_n);;
	StopAcceleratorTimer();;
	for (int i14 = 0; i14 < adi_acc_n; i14++) {
		array[i14].im = adi_acc_output[i14].im;
	};
	for (int i15 = 0; i15 < adi_acc_n; i15++) {
		array[i15].re = adi_acc_output[i15].re;
	};
	
return array;;
	
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
