/* Orignal skeleton is: 
Pre: SKELETON:

With the array index wrappers adi_acc_output,Annon
And (fromvars) []
Under dimensions [64]
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers adi_acc_n
And (fromvars) [Constant(64)]
Under dimensions []
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers adi_acc_input,re
And (fromvars) [Z, f32_2]
Under dimensions [64]
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers adi_acc_input,im
And (fromvars) [Z, f32_1]
Under dimensions [64]
With conversion function IdentityConversion
Post: SKELETON:

With the array index wrappers Z,f32_2
And (fromvars) [adi_acc_output, re]
Under dimensions [64]
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers Z,f32_1
And (fromvars) [adi_acc_output, im]
Under dimensions [64]
With conversion function IdentityConversion
*/

/* Typemap is :
 i12: int32
adi_acc_input: array(complex_float: with dims adi_acc_n (=) )
i13: int32
i14: int32
i15: int32
adi_acc_output: array(complex_float: with dims adi_acc_n (=) )
Z: array(facc_2xf32_t: with dims 64)
adi_acc_n: int32
*/


extern "C" {
#include "../../benchmarks/Github/code/tasimon_FFT/self_contained_code.c"
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

void write_output(float * Z) {

    json output_json;
std::vector<json> output_temp_85;
for (unsigned int i86 = 0; i86 < 128; i86++) {
float output_temp_87 = Z[i86];

output_temp_85.push_back(output_temp_87);
}
output_json["Z"] = output_temp_85;
std::ofstream out_str(output_file); 
out_str << std::setw(4) << output_json << std::endl;
}

void fft64_accel_internal(facc_2xf32_t * Z) {
static complex_float adi_acc_output[64]__attribute__((__aligned__(64)));;
	static int adi_acc_n;;
	adi_acc_n = 64;;
	static complex_float adi_acc_input[64]__attribute__((__aligned__(64)));;
	for (int i12 = 0; i12 < 64; i12++) {
		adi_acc_input[i12].re = Z[i12].f32_2;
	};
	for (int i13 = 0; i13 < 64; i13++) {
		adi_acc_input[i13].im = Z[i13].f32_1;
	};
	StartAcceleratorTimer();;
	accel_cfft_wrapper(adi_acc_input, adi_acc_output, adi_acc_n);;
	StopAcceleratorTimer();;
	for (int i14 = 0; i14 < 64; i14++) {
		Z[i14].f32_2 = adi_acc_output[i14].re;
	};
	for (int i15 = 0; i15 < 64; i15++) {
		Z[i15].f32_1 = adi_acc_output[i15].im;
	};
	
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
fft64_accel(Z);
clock_t end = clock();
std::cout << "Time: " << (double) (end - begin) / CLOCKS_PER_SEC << std::endl;
std::cout << "AccTime: " << (double) AcceleratorTotalNanos / CLOCKS_PER_SEC << std::endl;
write_output(Z);
}
