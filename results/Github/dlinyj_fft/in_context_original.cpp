#include<vector>
#include<nlohmann/json.hpp>
#include<fstream>
#include<iomanip>
#include<clib/synthesizer.h>
#include<time.h>
#include<iostream>
extern "C" {
#include "../../../benchmarks/Github/code/dlinyj_fft/self_contained_code.c"
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

void write_output(int p, float * c, float * in, float * out, int norm) {

    json output_json;
std::vector<json> output_temp_1;
for (unsigned int i2 = 0; i2 < 1 << (p + 1); i2++) {
float output_temp_3 = out[i2];

output_temp_1.push_back(output_temp_3);
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
int p = input_json["p"];
std::vector<float> in_vec;
for (auto& elem : input_json["in"]) {
float in_inner = elem;
in_vec.push_back(in_inner);
}
float *in = &in_vec[0];
int norm = input_json["norm"];
float c[1 << (p + 1)];
fft_make(p, c);
float out[1 << (p + 1)];
clock_t begin = clock();
for (int i = 0; i < TIMES; i ++) {
	fft_calc(p, c, in, out, norm);
}
clock_t end = clock();
std::cout << "Time: " << (double) (end - begin) / CLOCKS_PER_SEC << std::endl;
std::cout << "AccTime: " << (double) AcceleratorTotalNanos / CLOCKS_PER_SEC << std::endl;
write_output(p, c, in, out, norm);
}
