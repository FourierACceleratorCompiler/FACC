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
void write_output(COMPLEX * x, int N) {

    json output_json;
std::vector<json> output_temp_1;
for (unsigned int i2 = 0; i2 < N; i2++) {
COMPLEX output_temp_3 = x[i2];
json output_temp_4;

output_temp_4["real"] = output_temp_3.real;

output_temp_4["imag"] = output_temp_3.imag;
output_temp_1.push_back(output_temp_4);
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
std::vector<COMPLEX> x_vec;
for (auto& elem : input_json["x"]) {
float x_innerreal = elem["real"];
float x_innerimag = elem["imag"];
COMPLEX x_inner = { x_innerreal, x_innerimag};
x_vec.push_back(x_inner);
}
COMPLEX *x = &x_vec[0];
int N = input_json["N"];
clock_t begin = clock();
for (int i = 0; i < TIMES; i ++) {
	fft(x, N);
}
clock_t end = clock();
std::cout << "Time: " << (double) (end - begin) / CLOCKS_PER_SEC << std::endl;
std::cout << "AccTime: 0" << std::endl;
write_output(x, N);
}
