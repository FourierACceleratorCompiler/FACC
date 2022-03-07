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

void write_output(struct_FFT * fft, float * re, float * im) {

    json output_json;
std::vector<json> output_temp_1;
for (unsigned int i2 = 0; i2 < fft->n; i2++) {
float output_temp_3 = re[i2];

output_temp_1.push_back(output_temp_3);
}
output_json["re"] = output_temp_1;
std::vector<json> output_temp_4;
for (unsigned int i5 = 0; i5 < fft->n; i5++) {
float output_temp_6 = im[i5];

output_temp_4.push_back(output_temp_6);
}
output_json["im"] = output_temp_4;
std::ofstream out_str(output_file); 
out_str << std::setw(4) << output_json << std::endl;
}
int main(int argc, char **argv) {
    char *inpname = argv[1]; 
    output_file = argv[2]; 

    std::ifstream ifs(inpname); 
    json input_json = json::parse(ifs);
int fft_pointern = input_json["fft"]["n"];
std::vector<int> fft_pointerreversed_vec;
for (auto& elem : input_json["fft"]["reversed"]) {
int fft_pointerreversed_inner = elem;
fft_pointerreversed_vec.push_back(fft_pointerreversed_inner);
}
int *fft_pointerreversed = &fft_pointerreversed_vec[0];
std::vector<float> fft_pointerc_vec;
for (auto& elem : input_json["fft"]["c"]) {
float fft_pointerc_inner = elem;
fft_pointerc_vec.push_back(fft_pointerc_inner);
}
float *fft_pointerc = &fft_pointerc_vec[0];
std::vector<float> fft_pointers_vec;
for (auto& elem : input_json["fft"]["s"]) {
float fft_pointers_inner = elem;
fft_pointers_vec.push_back(fft_pointers_inner);
}
float *fft_pointers = &fft_pointers_vec[0];
struct_FFT fft_pointer = { fft_pointern, fft_pointerreversed, fft_pointerc, fft_pointers};
struct_FFT* fft = &fft_pointer;
std::vector<float> re_vec;
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
for (int i = 0; i < TIMES; i ++) {
	fft_fft(fft, re, im);
}
clock_t end = clock();
std::cout << "Time: " << (double) (end - begin) / CLOCKS_PER_SEC << std::endl;
std::cout << "AccTime: " << (double) AcceleratorTotalNanos / CLOCKS_PER_SEC << std::endl;
write_output(fft, re, im);
}
