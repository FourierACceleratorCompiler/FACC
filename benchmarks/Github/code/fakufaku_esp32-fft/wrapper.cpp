#include<vector>
#include<nlohmann/json.hpp>
#include<fstream>
#include<iomanip>
#include<clib/synthesizer.h>
#include<chrono>
#include<iostream>
extern "C" {
#include "self_contained_code.c"
}
char *output_file; 
char *pre_accel_dump_file; // optional dump file. 
using json = nlohmann::json;
void write_output(float * input, float * output, float * twiddle_factors, int n) {

    json output_json;
std::vector<json> output_temp_1;
for (unsigned int i2 = 0; i2 < n; i2++) {
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
std::vector<float> input_vec;
for (auto& elem : input_json["input"]) {
float input_inner = elem;
input_vec.push_back(input_inner);
}
float *input = &input_vec[0];
std::vector<float> twiddle_factors_vec;
for (auto& elem : input_json["twiddle_factors"]) {
float twiddle_factors_inner = elem;
twiddle_factors_vec.push_back(twiddle_factors_inner);
}
float *twiddle_factors = &twiddle_factors_vec[0];
int n = input_json["n"];
float output[n];
clock_t begin = clock();
fft(input, output, twiddle_factors, n);
clock_t end = clock();
std::cout << "Time: " << (double) (end - begin) / CLOCKS_PER_SEC << std::endl;
std::cout << "AccTime: 0" << std::endl;
write_output(input, output, twiddle_factors, n);
}
