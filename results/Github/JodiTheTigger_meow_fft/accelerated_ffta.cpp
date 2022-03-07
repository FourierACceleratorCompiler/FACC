/* Orignal skeleton is: 
Pre: SKELETON:

With the array index wrappers adi_acc_output,Annon
And (fromvars) []
Under dimensions [adi_acc_n = data.N]
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers adi_acc_n
And (fromvars) [data.N]
Under dimensions []
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers adi_acc_input,re
And (fromvars) [in, r]
Under dimensions [adi_acc_n = data.N]
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers adi_acc_input,im
And (fromvars) [in, j]
Under dimensions [adi_acc_n = data.N]
With conversion function IdentityConversion
Post: SKELETON:

With the array index wrappers out,j
And (fromvars) [adi_acc_output, im]
Under dimensions [data.N = adi_acc_n]
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers out,r
And (fromvars) [adi_acc_output, re]
Under dimensions [data.N = adi_acc_n]
With conversion function IdentityConversion
*/


extern "C" {
#include "../../benchmarks/Github/code/JodiTheTigger_meow_fft/self_contained_code.c"
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

void write_output(Meow_FFT_Workset * data, Meow_FFT_Complex * in, Meow_FFT_Complex * out) {

    json output_json;
std::vector<json> output_temp_133;
for (unsigned int i134 = 0; i134 < data->N; i134++) {
Meow_FFT_Complex output_temp_135 = out[i134];
json output_temp_136;

output_temp_136["r"] = output_temp_135.r;

output_temp_136["j"] = output_temp_135.j;
output_temp_133.push_back(output_temp_136);
}
output_json["out"] = output_temp_133;
std::ofstream out_str(output_file); 
out_str << std::setw(4) << output_json << std::endl;
}

void meow_fft_accel_internal(Meow_FFT_Workset * data,Meow_FFT_Complex * in,Meow_FFT_Complex * out) {
static complex_float adi_acc_output[16384]__attribute__((__aligned__(64)));;
	static int adi_acc_n;;
	adi_acc_n = data->N;;
	static complex_float adi_acc_input[16384]__attribute__((__aligned__(64)));;
	for (int i47 = 0; i47 < adi_acc_n; i47++) {
		adi_acc_input[i47].re = in[i47].r;
	};
	for (int i48 = 0; i48 < adi_acc_n; i48++) {
		adi_acc_input[i48].im = in[i48].j;
	};
	StartAcceleratorTimer();;
	accel_cfft_wrapper(adi_acc_input, adi_acc_output, adi_acc_n);;
	StopAcceleratorTimer();;
	for (int i49 = 0; i49 < data->N; i49++) {
		out[i49].j = adi_acc_output[i49].im;
	};
	for (int i50 = 0; i50 < data->N; i50++) {
		out[i50].r = adi_acc_output[i50].re;
	};
	
;
	
;
	

}
void meow_fft_accel(Meow_FFT_Workset * data, Meow_FFT_Complex * in, Meow_FFT_Complex * out) {
meow_fft_accel_internal((Meow_FFT_Workset *) data, (Meow_FFT_Complex *) in, (Meow_FFT_Complex *) out);
}
int main(int argc, char **argv) {
    char *inpname = argv[1]; 
    output_file = argv[2]; 

    std::ifstream ifs(inpname); 
    json input_json = json::parse(ifs);
int data_pointerN = input_json["data"]["N"];
std::vector<Meow_FFT_Complex> data_pointerwn_vec;
for (auto& elem : input_json["data"]["wn"]) {
float data_pointerwn_innerr = elem["r"];
float data_pointerwn_innerj = elem["j"];
Meow_FFT_Complex data_pointerwn_inner = { data_pointerwn_innerr, data_pointerwn_innerj};
data_pointerwn_vec.push_back(data_pointerwn_inner);
}
Meow_FFT_Complex *data_pointerwn = &data_pointerwn_vec[0];
std::vector<Meow_FFT_Complex> data_pointerwn_ordered_vec;
for (auto& elem : input_json["data"]["wn_ordered"]) {
float data_pointerwn_ordered_innerr = elem["r"];
float data_pointerwn_ordered_innerj = elem["j"];
Meow_FFT_Complex data_pointerwn_ordered_inner = { data_pointerwn_ordered_innerr, data_pointerwn_ordered_innerj};
data_pointerwn_ordered_vec.push_back(data_pointerwn_ordered_inner);
}
Meow_FFT_Complex *data_pointerwn_ordered = &data_pointerwn_ordered_vec[0];
unsigned int data_pointerstagescount = input_json["data"]["stages"]["count"];
std::vector<unsigned int> data_pointerstagesradix_vec;
for (auto& elem : input_json["data"]["stages"]["radix"]) {
unsigned int data_pointerstagesradix_inner = elem;
data_pointerstagesradix_vec.push_back(data_pointerstagesradix_inner);
}
unsigned int *data_pointerstagesradix = &data_pointerstagesradix_vec[0];
std::vector<unsigned int> data_pointerstagesremainder_vec;
for (auto& elem : input_json["data"]["stages"]["remainder"]) {
unsigned int data_pointerstagesremainder_inner = elem;
data_pointerstagesremainder_vec.push_back(data_pointerstagesremainder_inner);
}
unsigned int *data_pointerstagesremainder = &data_pointerstagesremainder_vec[0];
std::vector<unsigned int> data_pointerstagesoffsets_vec;
for (auto& elem : input_json["data"]["stages"]["offsets"]) {
unsigned int data_pointerstagesoffsets_inner = elem;
data_pointerstagesoffsets_vec.push_back(data_pointerstagesoffsets_inner);
}
unsigned int *data_pointerstagesoffsets = &data_pointerstagesoffsets_vec[0];
Meow_Fft_Stages data_pointerstages = { data_pointerstagescount, data_pointerstagesradix, data_pointerstagesremainder, data_pointerstagesoffsets};
Meow_FFT_Workset data_pointer = { data_pointerN, data_pointerwn, data_pointerwn_ordered, data_pointerstages};
Meow_FFT_Workset* data = &data_pointer;
std::vector<Meow_FFT_Complex> in_vec;
for (auto& elem : input_json["in"]) {
float in_innerr = elem["r"];
float in_innerj = elem["j"];
Meow_FFT_Complex in_inner = { in_innerr, in_innerj};
in_vec.push_back(in_inner);
}
Meow_FFT_Complex *in = &in_vec[0];
Meow_FFT_Complex out[data->N];
clock_t begin = clock();
meow_fft_accel(data, in, out);
clock_t end = clock();
std::cout << "Time: " << (double) (end - begin) / CLOCKS_PER_SEC << std::endl;
std::cout << "AccTime: " << (double) AcceleratorTotalNanos / CLOCKS_PER_SEC << std::endl;
write_output(data, in, out);
}