#include<vector>
#include<nlohmann/json.hpp>
#include<fstream>
#include<iomanip>
#include<clib/synthesizer.h>
#include<time.h>
#include<iostream>
extern "C" {
#include "../../../benchmarks/Github/code/marton78_pffft/self_contained_code.c"
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

void write_output(PFFFT_Setup * setup, float * input, float * output, float * work, pffft_direction_t direction) {

    json output_json;
PFFFT_Setup output_temp_1 = *setup;
json output_temp_2;

output_temp_2["N"] = output_temp_1.N;

output_temp_2["Ncvec"] = output_temp_1.Ncvec;
std::vector<json> output_temp_3;
for (unsigned int i4 = 0; i4 < 15; i4++) {
int output_temp_5 = setup->ifac[i4];

output_temp_3.push_back(output_temp_5);
}
output_temp_2["ifac"] = output_temp_3;

output_temp_2["transform"] = (output_temp_1.transform == PFFFT_COMPLEX ? 1 : 0);
std::vector<json> output_temp_6;
for (unsigned int i7 = 0; i7 < 2 * output_temp_1.N; i7++) {
v4sf output_temp_8 = setup->data[i7];
output_temp_6.push_back(output_temp_8);
}
output_temp_2["data"] = output_temp_6;
std::vector<json> output_temp_10;
for (unsigned int i11 = 0; i11 < 2 * output_temp_1.N; i11++) {
float output_temp_12 = setup->e[i11];

output_temp_10.push_back(output_temp_12);
}
output_temp_2["e"] = output_temp_10;
std::vector<json> output_temp_13;
for (unsigned int i14 = 0; i14 < 2 * output_temp_1.N; i14++) {
float output_temp_15 = setup->twiddle[i14];

output_temp_13.push_back(output_temp_15);
}
output_temp_2["twiddle"] = output_temp_13;
output_json["setup"] = output_temp_2;
std::vector<json> output_temp_22;
for (unsigned int i23 = 0; i23 < 2 * output_temp_1.N; i23++) {
float output_temp_24 = work[i23];

output_temp_22.push_back(output_temp_24);
}
output_json["work"] = output_temp_22;

output_json["direction"] = (direction == PFFFT_FORWARD ? 0 : 1);
std::ofstream out_str(output_file); 
out_str << std::setw(4) << output_json << std::endl;
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
n ++;
}
float*input = &input_vec[0];
float work[n];
float output[n];
n /= 2;

clock_t begin = clock();
PFFFT_Setup *setup = pffft_new_setup(n, PFFFT_COMPLEX);
pffft_direction_t direction = PFFFT_FORWARD;
for (int i = 0; i < TIMES; i ++) {
	pffft_transform_ordered(setup, input, output, work, direction);
}
clock_t end = clock();
std::cout << "Time: " << (double) (end - begin) / CLOCKS_PER_SEC << std::endl;
std::cout << "AccTime: " << (double) AcceleratorTotalNanos / CLOCKS_PER_SEC << std::endl;
write_output(setup, input, output, work, direction);
}
