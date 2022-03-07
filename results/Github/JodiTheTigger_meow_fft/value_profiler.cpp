#include<vector>
#include<nlohmann/json.hpp>
#include<fstream>
#include<iomanip>
#include<clib/synthesizer.h>
#include<time.h>
#include<iostream>
extern "C" {
#include "../../../benchmarks/Github/code/JodiTheTigger_meow_fft/context_code.c"
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

void write_output(Meow_FFT_Workset * data, Meow_FFT_Complex * in, Meow_FFT_Complex * out) {

    json output_json;
Meow_FFT_Workset output_temp_1 = *data;
json output_temp_2;

output_temp_2["N"] = output_temp_1.N;
std::vector<json> output_temp_3;
if (data->wn != NULL) {
	for (unsigned int i4 = 0; i4 < output_temp_1.stages.count; i4++) {
	Meow_FFT_Complex output_temp_5 = data->wn[i4];
	json output_temp_6;

	output_temp_6["r"] = output_temp_5.r;

	output_temp_6["j"] = output_temp_5.j;
	output_temp_3.push_back(output_temp_6);
	}
output_temp_2["wn"] = output_temp_3;
} else {
	output_temp_2["wn"] = output_temp_3;
}
std::vector<json> output_temp_7;
if (data->wn_ordered != NULL) {
	for (unsigned int i8 = 0; i8 < output_temp_1.N; i8++) {
	Meow_FFT_Complex output_temp_9 = data->wn_ordered[i8];
	json output_temp_10;

	output_temp_10["r"] = output_temp_9.r;

	output_temp_10["j"] = output_temp_9.j;
	output_temp_7.push_back(output_temp_10);
	}
	output_temp_2["wn_ordered"] = output_temp_7;
} else {
	output_temp_2["wn_ordered"] = NULL;
}
json output_temp_11;

output_temp_11["count"] = data->stages.count;
std::vector<json> output_temp_12;
for (unsigned int i13 = 0; i13 < data->stages.count; i13++) {
unsigned int output_temp_14 = data->stages.radix[i13];

output_temp_12.push_back(output_temp_14);
}
output_temp_11["radix"] = output_temp_12;
std::vector<json> output_temp_15;
for (unsigned int i16 = 0; i16 < data->stages.count; i16++) {
unsigned int output_temp_17 = data->stages.remainder[i16];

output_temp_15.push_back(output_temp_17);
}
output_temp_11["remainder"] = output_temp_15;
std::vector<json> output_temp_18;
for (unsigned int i19 = 0; i19 < data->stages.count; i19++) {
unsigned int output_temp_20 = data->stages.offsets[i19];

output_temp_18.push_back(output_temp_20);
}
output_temp_11["offsets"] = output_temp_18;
output_temp_2["stages"] = output_temp_11;
output_json["data"] = output_temp_2;
/* Don't wnat to profile input data.  */
/* std::vector<json> output_temp_21; */
/* for (unsigned int i22 = 0; i22 < N; i22++) { */
/* Meow_FFT_Complex output_temp_23 = in[i22]; */
/* json output_temp_24; */

/* output_temp_24["r"] = output_temp_23.r; */

/* output_temp_24["j"] = output_temp_23.j; */
/* output_temp_21.push_back(output_temp_24); */
/* } */
/* output_json["in"] = output_temp_21; */
std::ofstream out_str(output_file); 
out_str << std::setw(4) << output_json << std::endl;
}
int main(int argc, char **argv) {
    char *inpname = argv[1]; 
    output_file = argv[2]; 

    std::ifstream ifs(inpname); 
    json input_json = json::parse(ifs);
std::vector<Meow_FFT_Complex> in_vec;
int n = 0;
for (auto& elem : input_json["in"]) {
float in_innerr = elem["r"];
float in_innerj = elem["j"];
Meow_FFT_Complex in_inner = { in_innerr, in_innerj};
in_vec.push_back(in_inner);
n += 1;
}


// begin typical use (hand-written)
// Typical use from the github example: https://github.com/JodiTheTigger/meow_fft
size_t workset_bytes = meow_fft_generate_workset(n, NULL);
Meow_FFT_Workset* fft =
	(Meow_FFT_Workset *) malloc(workset_bytes);
meow_fft_generate_workset(n, fft);
Meow_FFT_Complex *in = &in_vec[0];
Meow_FFT_Complex out[n];
clock_t begin = clock();
meow_fft(fft, in, out);
clock_t end = clock();
// End hand-written typical use


std::cout << "Time: " << (double) (end - begin) / CLOCKS_PER_SEC << std::endl;
std::cout << "AccTime: " << (double) AcceleratorTotalNanos / CLOCKS_PER_SEC << std::endl;
write_output(fft, in, out);
}
