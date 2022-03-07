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

void write_output(complex_f * data, int log2_N, complex_f * direction) {

    json output_json;
std::vector<json> output_temp_1;
for (unsigned int i2 = 0; i2 < (1 << log2_N); i2++) {
complex_f output_temp_3 = data[i2];
json output_temp_4;

output_temp_4["re"] = output_temp_3.re;

output_temp_4["im"] = output_temp_3.im;
output_temp_1.push_back(output_temp_4);
}
output_json["data"] = output_temp_1;
std::ofstream out_str(output_file); 
out_str << std::setw(4) << output_json << std::endl;
}
int main(int argc, char **argv) {
    char *inpname = argv[1]; 
    output_file = argv[2]; 

    std::ifstream ifs(inpname); 
    json input_json = json::parse(ifs);
std::vector<complex_f> data_vec;
for (auto& elem : input_json["data"]) {
float data_innerre = elem["re"];
float data_innerim = elem["im"];
complex_f data_inner = { data_innerre, data_innerim};
data_vec.push_back(data_inner);
}
complex_f *data = &data_vec[0];
int log2_N = input_json["log2_N"];
std::vector<complex_f> direction_vec;
for (auto& elem : input_json["direction"]) {
float direction_innerre = elem["re"];
float direction_innerim = elem["im"];
complex_f direction_inner = { direction_innerre, direction_innerim};
direction_vec.push_back(direction_inner);
}
complex_f *direction = &direction_vec[0];
clock_t begin = clock();
for (int i = 0; i < TIMES; i ++) {
	fftr_f(data, log2_N, direction);
}
clock_t end = clock();
std::cout << "Time: " << (double) (end - begin) / CLOCKS_PER_SEC << std::endl;
std::cout << "AccTime: " << (double) AcceleratorTotalNanos / CLOCKS_PER_SEC << std::endl;
write_output(data, log2_N, direction);
}
