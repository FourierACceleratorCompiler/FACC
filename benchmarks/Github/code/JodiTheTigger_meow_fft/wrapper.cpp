#include<vector>
#include<nlohmann/json.hpp>
#include<fstream>
#include<iomanip>
#include<clib/synthesizer.h>
#include<time.h>
#include<iostream>
extern "C" {
#include "self_contained_code.c"
}
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
std::vector<json> output_temp_1;
for (unsigned int i2 = 0; i2 < data->N; i2++) {
Meow_FFT_Complex output_temp_3 = out[i2];
json output_temp_4;

output_temp_4["r"] = output_temp_3.r;

output_temp_4["j"] = output_temp_3.j;
output_temp_1.push_back(output_temp_4);
}
output_json["out"] = output_temp_1;
std::ofstream out_str(output_file); 
out_str << std::setw(4) << output_json << std::endl;
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
for (int i = 0; i < TIMES; i ++) {
	meow_fft(data, in, out);
}
clock_t end = clock();
std::cout << "Time: " << (double) (end - begin) / CLOCKS_PER_SEC << std::endl;
std::cout << "AccTime: " << (double) AcceleratorTotalNanos / CLOCKS_PER_SEC << std::endl;
write_output(data, in, out);
}
