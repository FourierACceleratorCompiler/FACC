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

void write_output(_complex_double_ * invec, _complex_double_ * outvec, unsigned int n, bool forward) {

    json output_json;
std::vector<json> output_temp_1;
for (unsigned int i2 = 0; i2 < n; i2++) {
_complex_double_ output_temp_3 = outvec[i2];
json output_temp_4;

output_temp_4["re"] = output_temp_3.re;

output_temp_4["im"] = output_temp_3.im;
output_temp_1.push_back(output_temp_4);
}
output_json["outvec"] = output_temp_1;

std::ofstream out_str(output_file); 
out_str << std::setw(4) << output_json << std::endl;
}
int main(int argc, char **argv) {
    char *inpname = argv[1]; 
    output_file = argv[2]; 

    std::ifstream ifs(inpname); 
    json input_json = json::parse(ifs);
std::vector<_complex_double_> invec_vec;
for (auto& elem : input_json["invec"]) {
double invec_innerre = elem["re"];
double invec_innerim = elem["im"];
_complex_double_ invec_inner = { invec_innerre, invec_innerim};
invec_vec.push_back(invec_inner);
}
_complex_double_ *invec = &invec_vec[0];
unsigned int n = input_json["n"];
bool forward = input_json["forward"];
_complex_double_ outvec[n];
clock_t begin = clock();
for (int i = 0; i < TIMES; i ++) {
	recFFT_wrapper(outvec, invec, n, forward);
}
clock_t end = clock();
std::cout << "Time: " << (double) (end - begin) / CLOCKS_PER_SEC << std::endl;
std::cout << "AccTime: " << (double) AcceleratorTotalNanos / CLOCKS_PER_SEC << std::endl;
write_output(invec, outvec, n, forward);
}
