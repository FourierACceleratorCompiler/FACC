/* Orignal skeleton is: 
Pre: SKELETON:

With the array index wrappers adi_acc_output,Annon
And (fromvars) []
Under dimensions [adi_acc_n = setup.N]
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers adi_acc_n
And (fromvars) [setup.N]
Under dimensions []
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers adi_acc_input,re
And (fromvars) [input, f32_1]
Under dimensions [adi_acc_n = setup.N]
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers adi_acc_input,im
And (fromvars) [input, f32_2]
Under dimensions [adi_acc_n = setup.N]
With conversion function IdentityConversion
Post: SKELETON:

With the array index wrappers output,f32_1
And (fromvars) [adi_acc_output, re]
Under dimensions [setup.N = adi_acc_n]
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers output,f32_2
And (fromvars) [adi_acc_output, im]
Under dimensions [setup.N = adi_acc_n]
With conversion function IdentityConversion
*/


extern "C" {
#include "../../benchmarks/Github/code/marton78_pffft/self_contained_code.c"
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

void write_output(PFFFT_Setup_Desugar * setup, float * input, float * output, float * work, int direction) {

    json output_json;
std::vector<json> output_temp_197;
for (unsigned int i198 = 0; i198 < setup->N; i198++) {
float output_temp_199 = output[i198];

output_temp_197.push_back(output_temp_199);
}
output_json["output"] = output_temp_197;
std::ofstream out_str(output_file); 
out_str << std::setw(4) << output_json << std::endl;
}

void desugared_transform_ordered_accel_internal(PFFFT_Setup_Desugar * setup,facc_2xf32_t * input,facc_2xf32_t * output,facc_2xf32_t * work,int direction) {

if ((PRIM_EQUAL(direction, 1)) || (PRIM_EQUAL(direction, 0))) {
static complex_float adi_acc_output[16384]__attribute__((__aligned__(64)));;
	static int adi_acc_n;;
	adi_acc_n = setup->N;;
	static complex_float adi_acc_input[16384]__attribute__((__aligned__(64)));;
	for (int i82 = 0; i82 < adi_acc_n; i82++) {
		adi_acc_input[i82].re = input[i82].f32_1;
	};
	for (int i83 = 0; i83 < adi_acc_n; i83++) {
		adi_acc_input[i83].im = input[i83].f32_2;
	};
	StartAcceleratorTimer();;
	accel_cfft_wrapper(adi_acc_input, adi_acc_output, adi_acc_n);;
	StopAcceleratorTimer();;
	for (int i84 = 0; i84 < setup->N; i84++) {
		output[i84].f32_1 = adi_acc_output[i84].re;
	};
	for (int i85 = 0; i85 < setup->N; i85++) {
		output[i85].f32_2 = adi_acc_output[i85].im;
	};
	
;
	
;
	

} else {
desugared_transform_ordered(setup, (float *)input, (float *)output, (float *)work, direction);
}
}
void desugared_transform_ordered_accel(PFFFT_Setup_Desugar * setup, float * input, float * output, float * work, int direction) {
desugared_transform_ordered_accel_internal((PFFFT_Setup_Desugar *) setup, (facc_2xf32_t *) input, (facc_2xf32_t *) output, (facc_2xf32_t *) work, (int) direction);
}
int main(int argc, char **argv) {
    char *inpname = argv[1]; 
    output_file = argv[2]; 

    std::ifstream ifs(inpname); 
    json input_json = json::parse(ifs);
int setup_pointerN = input_json["setup"]["N"];
int setup_pointerNcvec = input_json["setup"]["Ncvec"];
std::vector<int> setup_pointerifac_vec;
for (auto& elem : input_json["setup"]["ifac"]) {
int setup_pointerifac_inner = elem;
setup_pointerifac_vec.push_back(setup_pointerifac_inner);
}
int *setup_pointerifac = &setup_pointerifac_vec[0];
int setup_pointertransform = input_json["setup"]["transform"];
std::vector<float> setup_pointerdata_vec;
for (auto& elem : input_json["setup"]["data"]) {
float setup_pointerdata_inner = elem;
setup_pointerdata_vec.push_back(setup_pointerdata_inner);
}
float *setup_pointerdata = &setup_pointerdata_vec[0];
std::vector<float> setup_pointere_vec;
for (auto& elem : input_json["setup"]["e"]) {
float setup_pointere_inner = elem;
setup_pointere_vec.push_back(setup_pointere_inner);
}
float *setup_pointere = &setup_pointere_vec[0];
std::vector<float> setup_pointertwiddle_vec;
for (auto& elem : input_json["setup"]["twiddle"]) {
float setup_pointertwiddle_inner = elem;
setup_pointertwiddle_vec.push_back(setup_pointertwiddle_inner);
}
float *setup_pointertwiddle = &setup_pointertwiddle_vec[0];
PFFFT_Setup_Desugar setup_pointer = { setup_pointerN, setup_pointerNcvec, setup_pointerifac, setup_pointertransform, setup_pointerdata, setup_pointere, setup_pointertwiddle};
PFFFT_Setup_Desugar* setup = &setup_pointer;
std::vector<float> input_vec;
for (auto& elem : input_json["input"]) {
float input_inner = elem;
input_vec.push_back(input_inner);
}
float *input = &input_vec[0];
std::vector<float> work_vec;
for (auto& elem : input_json["work"]) {
float work_inner = elem;
work_vec.push_back(work_inner);
}
float *work = &work_vec[0];
int direction = input_json["direction"];
float output[setup->N* 2];
clock_t begin = clock();
desugared_transform_ordered_accel(setup, input, output, work, direction);
clock_t end = clock();
std::cout << "Time: " << (double) (end - begin) / CLOCKS_PER_SEC << std::endl;
std::cout << "AccTime: " << (double) AcceleratorTotalNanos / CLOCKS_PER_SEC << std::endl;
write_output(setup, input, output, work, direction);
}