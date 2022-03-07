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

void write_output(COMPLEX * Y, int N) {

    json output_json;
std::vector<json> output_temp_1;
for (unsigned int i2 = 0; i2 < N; i2++) {
COMPLEX output_temp_3 = Y[i2];
json output_temp_4;

output_temp_4["real"] = output_temp_3.real;

output_temp_4["imag"] = output_temp_3.imag;
output_temp_1.push_back(output_temp_4);
}
output_json["Y"] = output_temp_1;
std::ofstream out_str(output_file); 
out_str << std::setw(4) << output_json << std::endl;
}
int main(int argc, char **argv) {
    char *inpname = argv[1]; 
    output_file = argv[2]; 

    std::ifstream ifs(inpname); 
    json input_json = json::parse(ifs);
std::vector<COMPLEX> Y_vec;
for (auto& elem : input_json["Y"]) {
float Y_innerreal = elem["real"];
float Y_innerimag = elem["imag"];
COMPLEX Y_inner = { Y_innerreal, Y_innerimag};
Y_vec.push_back(Y_inner);
}
COMPLEX *Y = &Y_vec[0];
int N = input_json["N"];
clock_t begin = clock();
for (int i = 0; i < TIMES; i ++) {
	FFT(Y, N);
}
clock_t end = clock();
std::cout << "Time: " << (double) (end - begin) / CLOCKS_PER_SEC << std::endl;
std::cout << "AccTime: " << (double) AcceleratorTotalNanos / CLOCKS_PER_SEC << std::endl;
write_output(Y, N);
}
