/* Orignal skeleton is: 
Pre: SKELETON:

With the array index wrappers power_quad_acc_output,Annon
And (fromvars) []
Under dimensions [64]
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers power_quad_acc_n
And (fromvars) [Constant(64)]
Under dimensions []
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers power_quad_acc_input,im
And (fromvars) [Z, f32_1]
Under dimensions [64]
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers power_quad_acc_input,re
And (fromvars) [Z, f32_2]
Under dimensions [64]
With conversion function IdentityConversion
Post: SKELETON:

With the array index wrappers Z,f32_2
And (fromvars) [power_quad_acc_output, re]
Under dimensions [64]
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers Z,f32_1
And (fromvars) [power_quad_acc_output, im]
Under dimensions [64]
With conversion function IdentityConversion
*/

/* Typemap is :
 i5: int32
i3: int32
i4: int32
power_quad_acc_n: int32
power_quad_acc_output: array(complex_type: with dims power_quad_acc_n (=) )
Z: array(facc_2xf32_t: with dims 64)
power_quad_acc_input: array(complex_type: with dims power_quad_acc_n (=) )
i2: int32
*/


extern "C" {
#include "../../benchmarks/Github/code/tasimon_FFT/self_contained_code.c"
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
int power_quad_acc_n;;
	power_quad_acc_n = 64;;
	complex_type power_quad_acc_output[power_quad_acc_n]__attribute__((__aligned__(64)));;
	complex_type power_quad_acc_input[power_quad_acc_n]__attribute__((__aligned__(64)));;
	for (int i2 = 0; i2 < 64; i2++) {
		power_quad_acc_input[i2].im = Z[i2].f32_1;
	};
	for (int i3 = 0; i3 < 64; i3++) {
		power_quad_acc_input[i3].re = Z[i3].f32_2;
	};
	StartAcceleratorTimer();;
	fft_api(power_quad_acc_input, power_quad_acc_output, power_quad_acc_n);;
	StopAcceleratorTimer();;
	for (int i4 = 0; i4 < 64; i4++) {
		Z[i4].f32_2 = power_quad_acc_output[i4].re;
	};
	for (int i5 = 0; i5 < 64; i5++) {
		Z[i5].f32_1 = power_quad_acc_output[i5].im;
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