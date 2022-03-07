/* Orignal skeleton is: 
Pre: SKELETON:

With the array index wrappers power_quad_acc_output,Annon
And (fromvars) []
Under dimensions [power_quad_acc_n = setup.N (=) ]
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers power_quad_acc_n
And (fromvars) [setup.N]
Under dimensions []
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers power_quad_acc_input,im
And (fromvars) [input, f32_2]
Under dimensions [power_quad_acc_n = setup.N (=) ]
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers power_quad_acc_input,re
And (fromvars) [input, f32_1]
Under dimensions [power_quad_acc_n = setup.N (=) ]
With conversion function IdentityConversion
Post: SKELETON:

With the array index wrappers output,f32_1
And (fromvars) [power_quad_acc_output, re]
Under dimensions [power_quad_acc_n = setup.N (=) ]
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers output,f32_2
And (fromvars) [power_quad_acc_output, im]
Under dimensions [power_quad_acc_n = setup.N (=) ]
With conversion function IdentityConversion
*/

/* Typemap is :
 power_quad_acc_n: int32
i218: int32
i220: int32
direction: int32
power_quad_acc_input: array(complex_type: with dims power_quad_acc_n (=) )
input: array(facc_2xf32_t: with dims setup.N (=) )
i217: int32
output: array(facc_2xf32_t: with dims setup.N (=) )
setup: pointer(PFFFT_Setup_Desugar)
i219: int32
work: array(facc_2xf32_t: with dims setup.N (=) )
power_quad_acc_output: array(complex_type: with dims power_quad_acc_n (=) )
*/


extern "C" {
#include "../../benchmarks/Github/code/marton78_pffft/self_contained_code.c"
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

void write_output(PFFFT_Setup_Desugar * setup, float * input, float * output, float * work, int direction) {

    json output_json;
std::vector<json> output_temp_12;
for (unsigned int i13 = 0; i13 < setup->N* 2; i13++) {
float output_temp_14 = output[i13];

output_temp_12.push_back(output_temp_14);
}
output_json["output"] = output_temp_12;
std::ofstream out_str(output_file); 
out_str << std::setw(4) << output_json << std::endl;
}

void desugared_transform_ordered_accel_internal(PFFFT_Setup_Desugar * setup,facc_2xf32_t * input,facc_2xf32_t * output,facc_2xf32_t * work,int direction) {

if ((PRIM_EQUAL(direction, 1)) || (PRIM_EQUAL(direction, 0))) {
int power_quad_acc_n;;
	power_quad_acc_n = setup->N;;
	complex_type power_quad_acc_output[power_quad_acc_n]__attribute__((__aligned__(64)));
for (int i10 = 0; i10++; i10 < power_quad_acc_n) {
complex_type power_quad_acc_output_sub_element;

;
power_quad_acc_output[i10] = power_quad_acc_output_sub_element;
};
	complex_type power_quad_acc_input[power_quad_acc_n]__attribute__((__aligned__(64)));
for (int i11 = 0; i11++; i11 < power_quad_acc_n) {
complex_type power_quad_acc_input_sub_element;

;
power_quad_acc_input[i11] = power_quad_acc_input_sub_element;
};
	for (int i217 = 0; i217 < power_quad_acc_n; i217++) {
		power_quad_acc_input[i217].im = input[i217].f32_2;
	};
	for (int i218 = 0; i218 < power_quad_acc_n; i218++) {
		power_quad_acc_input[i218].re = input[i218].f32_1;
	};
	StartAcceleratorTimer();;
	fft_api(power_quad_acc_input, power_quad_acc_output, power_quad_acc_n);;
	StopAcceleratorTimer();;
	for (int i219 = 0; i219 < power_quad_acc_n; i219++) {
		output[i219].f32_1 = power_quad_acc_output[i219].re;
	};
	for (int i220 = 0; i220 < power_quad_acc_n; i220++) {
		output[i220].f32_2 = power_quad_acc_output[i220].im;
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
for (int i = 0; i < TIMES; i ++) {
	desugared_transform_ordered_accel(setup, input, output, work, direction);
}
clock_t end = clock();
std::cout << "Time: " << (double) (end - begin) / CLOCKS_PER_SEC << std::endl;
std::cout << "AccTime: " << (double) AcceleratorTotalNanos / CLOCKS_PER_SEC << std::endl;
write_output(setup, input, output, work, direction);
}
