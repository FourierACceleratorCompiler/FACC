#include<vector>
#include<nlohmann/json.hpp>
#include<fstream>
#include<iomanip>
#include<clib/synthesizer.h>
#include<time.h>
#include<iostream>
extern "C" {
#include "adi_emulation.c"
}
char *output_file; 
char *pre_accel_dump_file; // optional dump file. 
using json = nlohmann::json;
const char* __asan_default_options() { return "detect_leaks=0"; }


clock_t AcceleratorStart;
long long AcceleratorTotalNanos = 0;
void StartAcceleratorTimer() {
	AcceleratorStart = clock();
}

void StopAcceleratorTimer() {
	AcceleratorTotalNanos +=
		(double) ((clock()) - AcceleratorStart) / CLOCKS_PER_SEC;
}

void write_output(complex_float * adi_acc_input, complex_float * adi_acc_output, int adi_acc_n) {

    json output_json;
std::vector<json> output_temp_1;
for (unsigned int i2 = 0; i2 < adi_acc_n; i2++) {
complex_float output_temp_3 = adi_acc_output[i2];
json output_temp_4;

output_temp_4["re"] = output_temp_3.re;

output_temp_4["im"] = output_temp_3.im;
output_temp_1.push_back(output_temp_4);
}
output_json["adi_acc_output"] = output_temp_1;
std::ofstream out_str(output_file); 
out_str << std::setw(4) << output_json << std::endl;
}
int main(int argc, char **argv) {
    char *inpname = argv[1]; 
    output_file = argv[2]; 

    std::ifstream ifs(inpname); 
    json input_json = json::parse(ifs);
std::vector<complex_float> adi_acc_input_vec;
for (auto& elem : input_json["adi_acc_input"]) {
float adi_acc_input_innerre = elem["re"];
float adi_acc_input_innerim = elem["im"];
complex_float adi_acc_input_inner = { adi_acc_input_innerre, adi_acc_input_innerim};
adi_acc_input_vec.push_back(adi_acc_input_inner);
}
complex_float *adi_acc_input = &adi_acc_input_vec[0];
int adi_acc_n = input_json["adi_acc_n"];
complex_float adi_acc_output[adi_acc_n];
clock_t begin = clock();
accel_cfft_wrapper(adi_acc_input, adi_acc_output, adi_acc_n);
clock_t end = clock();
std::cout << "Time: " << (double) (end - begin) / CLOCKS_PER_SEC << std::endl;
std::cout << "AccTime: " << AcceleratorTotalNanos << std::endl;
write_output(adi_acc_input, adi_acc_output, adi_acc_n);
}
