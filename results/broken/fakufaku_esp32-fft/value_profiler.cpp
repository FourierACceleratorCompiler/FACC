#include<vector>
#include<nlohmann/json.hpp>
#include<fstream>
#include<iomanip>
#include<clib/synthesizer.h>
#include<chrono>
#include<iostream>
extern "C" {
#include "context_code.c"
}
char *output_file; 
char *pre_accel_dump_file; // optional dump file. 
using json = nlohmann::json;
void write_output(float * twiddle_factors, int n) {

    json output_json;
std::vector<json> output_temp_1;
for (unsigned int i2 = 0; i2 < 2 * n; i2++) {
float output_temp_3 = twiddle_factors[i2];

output_temp_1.push_back(output_temp_3);
}
output_json["twiddle_factors"] = output_temp_1;
output_json["n"] = n;
std::ofstream out_str(output_file); 
out_str << std::setw(4) << output_json << std::endl;
}

int main(int argc, char **argv) {
    char *inpname = argv[1]; 
    output_file = argv[2]; 

    std::fstream ifs(inpname); 
	std::cout << "failed: " << ifs.fail() << std::endl;
    json input_json = json::parse(ifs);
	ifs.close();
std::vector<float> input_vec;
for (auto& elem : input_json["input"]) {
float input_inner = elem;
input_vec.push_back(input_inner);
}
float *input = &input_vec[0];
int n = input_json["n"];
std::cout << "N is "<< n << " " << input_vec.size() << std::endl;
float *twiddle_factors = (float*) malloc(sizeof(float) * 2 * n);
float output[n];
std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
fft(input, output, twiddle_factors, n);
std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
std::cout << "Time: " << std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count() << std::endl;
write_output(twiddle_factors, n);
}
