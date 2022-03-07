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

void write_output(PFFFT_Setup_Desugar * setup, float * input, float * output, float * work, int direction) {

    json output_json;
std::vector<json> output_temp_1;
for (unsigned int i2 = 0; i2 < 2 * setup->N; i2++) {
float output_temp_3 = output[i2];

output_temp_1.push_back(output_temp_3);
}
output_json["output"] = output_temp_1;
std::ofstream out_str(output_file); 
out_str << std::setw(4) << output_json << std::endl;
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
std::vector<v4sf> setup_pointerdata_vec;
for (auto& elem : input_json["setup"]["data"]) {
float setup_pointerdata_innera = elem;
v4sf setup_pointerdata_inner = { setup_pointerdata_innera};
setup_pointerdata_vec.push_back(setup_pointerdata_inner);
}
v4sf *setup_pointerdata = &setup_pointerdata_vec[0];
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
int n = 0;
for (auto& elem : input_json["input"]) {
float input_inner = elem;
input_vec.push_back(input_inner);
n += 1;
}
float *input = &input_vec[0];
std::vector<float> work_vec;
for (auto& elem : input_json["work"]) {
float work_inner = elem;
work_vec.push_back(work_inner);
}
float *work = &work_vec[0];
int direction = input_json["direction"];
float output[n];
clock_t begin = clock();
for (int i = 0; i < TIMES; i ++) {
	desugared_transform_ordered(setup, input, output, work, direction);
}
clock_t end = clock();
std::cout << "Time: " << (double) (end - begin) / CLOCKS_PER_SEC << std::endl;
std::cout << "AccTime: " << (double) AcceleratorTotalNanos / CLOCKS_PER_SEC << std::endl;
write_output(setup, input, output, work, direction);
}
