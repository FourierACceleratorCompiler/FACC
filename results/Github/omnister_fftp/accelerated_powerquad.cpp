/* Orignal skeleton is: 
Pre: SKELETON:

With the array index wrappers power_quad_acc_output,Annon
And (fromvars) []
Under dimensions [power_quad_acc_n = n (=) ]
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers power_quad_acc_n
And (fromvars) [n]
Under dimensions []
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers power_quad_acc_input,im
And (fromvars) [array, im]
Under dimensions [power_quad_acc_n = n (=) ]
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers power_quad_acc_input,re
And (fromvars) [array, re]
Under dimensions [power_quad_acc_n = n (=) ]
With conversion function IdentityConversion
Post: SKELETON:

With the array index wrappers array,im
And (fromvars) [power_quad_acc_output, im]
Under dimensions [power_quad_acc_n = n (=) ]
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers array,re
And (fromvars) [power_quad_acc_output, re]
Under dimensions [power_quad_acc_n = n (=) ]
With conversion function IdentityConversion
*/

/* Typemap is :
 i5: int32
power_quad_acc_n: int32
power_quad_acc_input: array(complex_type: with dims power_quad_acc_n (=) )
i2: int32
i3: int32
array: array(COMPLEX: with dims n (=) )
i4: int32
n: int32
power_quad_acc_output: array(complex_type: with dims power_quad_acc_n (=) )
*/


extern "C" {
#include "../../benchmarks/Github/code/omnister_fftp/self_contained_code.c"
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

if ((PRIM_EQUAL(n, 2048)) || ((PRIM_EQUAL(n, 1024)) || ((PRIM_EQUAL(n, 512)) || ((PRIM_EQUAL(n, 256)) || ((PRIM_EQUAL(n, 128)) || (PRIM_EQUAL(n, 64))))))) {
int power_quad_acc_n;;
	power_quad_acc_n = n;;
	complex_type power_quad_acc_output[power_quad_acc_n]__attribute__((__aligned__(64)));
for (int i41 = 0; i41++; i41 < power_quad_acc_n) {
complex_type power_quad_acc_output_sub_element;

;
power_quad_acc_output[i41] = power_quad_acc_output_sub_element;
};
	complex_type power_quad_acc_input[power_quad_acc_n]__attribute__((__aligned__(64)));
for (int i42 = 0; i42++; i42 < power_quad_acc_n) {
complex_type power_quad_acc_input_sub_element;

;
power_quad_acc_input[i42] = power_quad_acc_input_sub_element;
};
	for (int i2 = 0; i2 < power_quad_acc_n; i2++) {
		power_quad_acc_input[i2].im = array[i2].im;
	};
	for (int i3 = 0; i3 < power_quad_acc_n; i3++) {
		power_quad_acc_input[i3].re = array[i3].re;
	};
	StartAcceleratorTimer();;
	fft_api(power_quad_acc_input, power_quad_acc_output, power_quad_acc_n);;
	StopAcceleratorTimer();;
	for (int i4 = 0; i4 < power_quad_acc_n; i4++) {
		array[i4].im = power_quad_acc_output[i4].im;
	};
	for (int i5 = 0; i5 < power_quad_acc_n; i5++) {
		array[i5].re = power_quad_acc_output[i5].re;
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
