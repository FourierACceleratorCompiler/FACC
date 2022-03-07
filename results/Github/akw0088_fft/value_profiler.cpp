#include<vector>
#include<nlohmann/json.hpp>
#include<fstream>
#include<iomanip>
#include<clib/synthesizer.h>
#include<time.h>
#include<iostream>
extern "C" {
#include "../../../benchmarks/Github/code/akw0088_fft/self_contained_code.c"
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

void write_output(int num, j_complex_t * x, j_complex_t * w) {

    json output_json;

output_json["num"] = num;
std::vector<json> output_temp_5;
for (unsigned int i6 = 0; i6 < num; i6++) {
j_complex_t output_temp_7 = w[i6];
json output_temp_8;

output_temp_8["real"] = output_temp_7.real;

output_temp_8["imag"] = output_temp_7.imag;
output_temp_5.push_back(output_temp_8);
}
output_json["w"] = output_temp_5;
std::ofstream out_str(output_file); 
out_str << std::setw(4) << output_json << std::endl;
}
int main(int argc, char **argv) {
    char *inpname = argv[1]; 
    output_file = argv[2]; 

    std::ifstream ifs(inpname); 
    json input_json = json::parse(ifs);
int num = input_json["num"];
std::vector<j_complex_t> x_vec;
for (auto& elem : input_json["x"]) {
float x_innerreal = elem["real"];
float x_innerimag = elem["imag"];
j_complex_t x_inner = { x_innerreal, x_innerimag};
x_vec.push_back(x_inner);
}
j_complex_t *x = &x_vec[0];
j_complex_t w[num];

clock_t begin = clock();
init_w(num, w);
for (int i = 0; i < TIMES; i ++) {
	fft_c(num, x, w);
}
clock_t end = clock();
std::cout << "Time: " << (double) (end - begin) / CLOCKS_PER_SEC << std::endl;
std::cout << "AccTime: " << (double) AcceleratorTotalNanos / CLOCKS_PER_SEC << std::endl;
write_output(num, x, w);
}
