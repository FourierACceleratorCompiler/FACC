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

void write_output(float * x, int NC, int forward) {

    json output_json;
std::vector<json> output_temp_1;
for (unsigned int i2 = 0; i2 < NC; i2++) {
float output_temp_3 = x[i2];

output_temp_1.push_back(output_temp_3);
}
output_json["x"] = output_temp_1;
std::ofstream out_str(output_file); 
out_str << std::setw(4) << output_json << std::endl;
}
int main(int argc, char **argv) {
    char *inpname = argv[1]; 
    output_file = argv[2]; 

    std::ifstream ifs(inpname); 
    json input_json = json::parse(ifs);
std::vector<float> x_vec;
for (auto& elem : input_json["x"]) {
float x_inner = elem;
x_vec.push_back(x_inner);
}
float *x = &x_vec[0];
int NC = input_json["NC"];
int forward = input_json["forward"];
clock_t begin = clock();
fftease_cfft(x, NC, forward);
clock_t end = clock();
std::cout << "Time: " << (double) (end - begin) / CLOCKS_PER_SEC << std::endl;
std::cout << "AccTime: " << (double) AcceleratorTotalNanos / CLOCKS_PER_SEC << std::endl;
write_output(x, NC, forward);
}
