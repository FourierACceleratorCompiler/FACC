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

void write_output(_float_complex_ * vector, long int N, int returnv) {

    json output_json;
std::vector<json> output_temp_1;
for (unsigned int i2 = 0; i2 < N; i2++) {
_float_complex_ output_temp_3 = vector[i2];
json output_temp_4;

output_temp_4["re"] = output_temp_3.re;

output_temp_4["im"] = output_temp_3.im;
output_temp_1.push_back(output_temp_4);
}
output_json["vector"] = output_temp_1;

output_json["returnv"] = returnv;
std::ofstream out_str(output_file); 
out_str << std::setw(4) << output_json << std::endl;
}
int main(int argc, char **argv) {
    char *inpname = argv[1]; 
    output_file = argv[2]; 

    std::ifstream ifs(inpname); 
    json input_json = json::parse(ifs);
std::vector<_float_complex_> vector_vec;
for (auto& elem : input_json["vector"]) {
float vector_innerre = elem["re"];
float vector_innerim = elem["im"];
_float_complex_ vector_inner = { vector_innerre, vector_innerim};
vector_vec.push_back(vector_inner);
}
_float_complex_ *vector = &vector_vec[0];
long int N = input_json["N"];
clock_t begin = clock();
int returnv;
for (int i = 0; i < TIMES; i ++) {
	returnv = fft_wrapper(vector, N);
}
clock_t end = clock();
std::cout << "Time: " << (double) (end - begin) / CLOCKS_PER_SEC << std::endl;
std::cout << "AccTime: " << (double) AcceleratorTotalNanos / CLOCKS_PER_SEC << std::endl;
write_output(vector, N, returnv);
}
