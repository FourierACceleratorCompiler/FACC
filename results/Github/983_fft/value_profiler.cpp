#include<vector>
#include<nlohmann/json.hpp>
#include<fstream>
#include<iomanip>
#include<clib/synthesizer.h>
#include<time.h>
#include<iostream>
extern "C" {
#include "../../../benchmarks/Github/code/983_fft/self_contained_code.c"
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

void write_output(struct_FFT * fft, float * re, float * im) {

    json output_json;
struct_FFT output_temp_1 = *fft;
json output_temp_2;

output_temp_2["n"] = output_temp_1.n;
std::vector<json> output_temp_3;
for (unsigned int i4 = 0; i4 < fft->n; i4++) {
int output_temp_5 = fft->reversed[i4];

output_temp_3.push_back(output_temp_5);
}
output_temp_2["reversed"] = output_temp_3;
std::vector<json> output_temp_6;
for (unsigned int i7 = 0; i7 < fft->n; i7++) {
float output_temp_8 = fft->c[i7];

output_temp_6.push_back(output_temp_8);
}
output_temp_2["c"] = output_temp_6;
std::vector<json> output_temp_9;
for (unsigned int i10 = 0; i10 < fft->n; i10++) {
float output_temp_11 = fft->s[i10];

output_temp_9.push_back(output_temp_11);
}
output_temp_2["s"] = output_temp_9;
output_json["fft"] = output_temp_2;
std::vector<json> output_temp_12;
for (unsigned int i13 = 0; i13 < fft->n; i13++) {
float output_temp_14 = re[i13];

output_temp_12.push_back(output_temp_14);
}
output_json["re"] = output_temp_12;
std::vector<json> output_temp_15;
for (unsigned int i16 = 0; i16 < fft->n; i16++) {
float output_temp_17 = im[i16];

output_temp_15.push_back(output_temp_17);
}
output_json["im"] = output_temp_15;
std::ofstream out_str(output_file); 
out_str << std::setw(4) << output_json << std::endl;
}
int main(int argc, char **argv) {
    char *inpname = argv[1]; 
    output_file = argv[2]; 

    std::ifstream ifs(inpname); 
    json input_json = json::parse(ifs);
std::vector<float> re_vec;
int n = input_json["n"];
for (auto& elem : input_json["re"]) {
float re_inner = elem;
re_vec.push_back(re_inner);
}
float *re = &re_vec[0];
std::vector<float> im_vec;
for (auto& elem : input_json["im"]) {
float im_inner = elem;
im_vec.push_back(im_inner);
}
float *im = &im_vec[0];
clock_t begin = clock();
struct_FFT fft;
fft_init(&fft, n);
fft_fft(&fft, re, im);
clock_t end = clock();
std::cout << "Time: " << (double) (end - begin) / CLOCKS_PER_SEC << std::endl;
std::cout << "AccTime: " << (double) AcceleratorTotalNanos / CLOCKS_PER_SEC << std::endl;
write_output(&fft, re, im);
}
