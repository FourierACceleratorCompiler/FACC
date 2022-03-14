/* Orignal skeleton is: 
Pre: SKELETON:

With the array index wrappers api_out,Annon
And (fromvars) []
Under dimensions [interface_len = setup.N]
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers dir
And (fromvars) [Constant(-1)]
Under dimensions []
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers interface_len
And (fromvars) [setup.N]
Under dimensions []
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers api_in,im
And (fromvars) [input, f32_2]
Under dimensions [interface_len = setup.N]
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers api_in,re
And (fromvars) [input, f32_1]
Under dimensions [interface_len = setup.N]
With conversion function IdentityConversion
Post: SKELETON:

With the array index wrappers output,f32_1
And (fromvars) [api_out, re]
Under dimensions [setup.N = interface_len]
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers output,f32_2
And (fromvars) [api_out, im]
Under dimensions [setup.N = interface_len]
With conversion function IdentityConversion
*/


extern "C" {
#include "../../../benchmarks/Github/code/marton78_pffft/self_contained_code.c"
}



#include "../../../benchmarks/Accelerators/FFTW/interface.hpp"
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


void write_output(PFFFT_Setup_Desugar * setup, float * input, float * output, float * work, int direction) {

    json output_json;
std::vector<json> output_temp_1051;
for (unsigned int i1052 = 0; i1052 < setup->N; i1052++) {
float output_temp_1053 = output[i1052];

output_temp_1051.push_back(output_temp_1053);
}
output_json["output"] = output_temp_1051;
std::ofstream out_str(output_file); 
out_str << std::setw(4) << output_json << std::endl;
}

void desugared_transform_ordered_accel_internal(PFFFT_Setup_Desugar * setup,facc_2xf32_t * input,facc_2xf32_t * output,facc_2xf32_t * work,int direction) {
short dir;;
	dir = -1;;
	int interface_len;;
	interface_len = setup->N;;
	_complex_float_ api_out[interface_len];;
	_complex_float_ api_in[interface_len];;
	for (int i582 = 0; i582 < interface_len; i582++) {
		api_in[i582].im = input[i582].f32_2;
	};
	for (int i583 = 0; i583 < interface_len; i583++) {
		api_in[i583].re = input[i583].f32_1;
	};
	
if ((PRIM_EQUAL(dir, -1)) && ((PRIM_EQUAL(interface_len, 524288)) || ((PRIM_EQUAL(interface_len, 262144)) || ((PRIM_EQUAL(interface_len, 131072)) || ((PRIM_EQUAL(interface_len, 65536)) || ((PRIM_EQUAL(interface_len, 32768)) || ((PRIM_EQUAL(interface_len, 16384)) || ((PRIM_EQUAL(interface_len, 8192)) || ((PRIM_EQUAL(interface_len, 4096)) || ((PRIM_EQUAL(interface_len, 2048)) || ((PRIM_EQUAL(interface_len, 1024)) || ((PRIM_EQUAL(interface_len, 512)) || ((PRIM_EQUAL(interface_len, 256)) || ((PRIM_EQUAL(interface_len, 128)) || ((PRIM_EQUAL(interface_len, 64)) || ((PRIM_EQUAL(interface_len, 32)) || ((PRIM_EQUAL(interface_len, 16)) || ((PRIM_EQUAL(interface_len, 8)) || ((PRIM_EQUAL(interface_len, 4)) || ((PRIM_EQUAL(interface_len, 2)) || (PRIM_EQUAL(interface_len, 1)))))))))))))))))))))) {
	StartAcceleratorTimer();
fftwf_example_api(api_in, api_out, interface_len, dir);;
	StopAcceleratorTimer();;
	for (int i584 = 0; i584 < setup->N; i584++) {
		output[i584].f32_1 = api_out[i584].re;
	};
	for (int i585 = 0; i585 < setup->N; i585++) {
		output[i585].f32_2 = api_out[i585].im;
	}
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
std::vector<float> input_vec;
int n = 0;
for (auto& elem : input_json["input"]) {
float input_inner = elem;
input_vec.push_back(input_inner);
n += 1;
}
float *input = &input_vec[0];
float work[n];
float output[n];
n /= 2;

PFFFT_Setup *setup = pffft_new_setup(n, PFFFT_COMPLEX);
PFFFT_Setup_Desugar desugared;
// Desugar
desugar_setup(setup, &desugared);
// pffft_direction_t direction = PFFFT_FORWARD;
// Desugar
int direction = 1;
clock_t begin = clock();
for (int i = 0; i < TIMES; i ++) {
	desugared_transform_ordered_accel(&desugared, input, output, work, direction);
}

clock_t end = clock();
std::cout << "Time: " << (double) (end - begin) / CLOCKS_PER_SEC << std::endl;
std::cout << "AccTime: " << (double) AcceleratorTotalNanos / CLOCKS_PER_SEC << std::endl;

write_output(&desugared, input, output, work, direction);
}
