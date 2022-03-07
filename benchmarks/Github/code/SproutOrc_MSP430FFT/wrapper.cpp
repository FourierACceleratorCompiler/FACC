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

void write_output(Complex * data, Complex * factor, int num, int series) {

    json output_json;
std::vector<json> output_temp_1;
for (unsigned int i2 = 0; i2 < num; i2++) {
Complex output_temp_3 = data[i2];
json output_temp_4;

output_temp_4["real"] = output_temp_3.real;

output_temp_4["imag"] = output_temp_3.imag;
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
std::vector<Complex> data_vec;
for (auto& elem : input_json["data"]) {
float data_innerreal = elem["real"];
float data_innerimag = elem["imag"];
Complex data_inner = { data_innerreal, data_innerimag};
data_vec.push_back(data_inner);
}
Complex *data = &data_vec[0];
std::vector<Complex> factor_vec;
for (auto& elem : input_json["factor"]) {
float factor_innerreal = elem["real"];
float factor_innerimag = elem["imag"];
Complex factor_inner = { factor_innerreal, factor_innerimag};
factor_vec.push_back(factor_inner);
}
Complex *factor = &factor_vec[0];
int num = input_json["num"];
int series = input_json["series"];
clock_t begin = clock();
for (int i = 0; i < TIMES; i ++) {
	FFT(data, factor, num, series);
}
clock_t end = clock();
std::cout << "Time: " << (double) (end - begin) / CLOCKS_PER_SEC << std::endl;
std::cout << "AccTime: " << (double) AcceleratorTotalNanos / CLOCKS_PER_SEC << std::endl;
write_output(data, factor, num, series);
}
