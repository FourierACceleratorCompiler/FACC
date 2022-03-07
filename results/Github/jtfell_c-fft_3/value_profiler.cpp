#include<vector>
#include<nlohmann/json.hpp>
#include<fstream>
#include<iomanip>
#include<clib/synthesizer.h>
#include<time.h>
#include<iostream>
extern "C" {
#include "../../../benchmarks/Github/code/jtfell_c-fft_3/self_contained_code.c"
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

void write_output(int N, int N1, int N2) {

    json output_json;
std::vector<json> output_temp_1;

output_json["N"] = N;

output_json["N1"] = N1;

output_json["N2"] = N2;
std::vector<json> output_temp_5;
std::ofstream out_str(output_file); 
out_str << std::setw(4) << output_json << std::endl;
}
int main(int argc, char **argv) {
    char *inpname = argv[1]; 
    output_file = argv[2]; 

    std::ifstream ifs(inpname); 
    json input_json = json::parse(ifs);
std::vector<complex> x_vec;
for (auto& elem : input_json["x"]) {
double x_innerre = elem["re"];
double x_innerim = elem["im"];
complex x_inner = { x_innerre, x_innerim};
x_vec.push_back(x_inner);
}
complex *x = &x_vec[0];
int N = input_json["N"];
// These must be coprime, which is admittedly a bit challenging
// for the current power-of-two evaluation environment.
int N1 = N / 2;
int N2 = 2;
clock_t begin = clock();
complex *returnv;
for (int i = 0; i < TIMES; i ++) {
	returnv = FFT_CooleyTukey(x, N, N1, N2);
}
clock_t end = clock();
std::cout << "Time: " << (double) (end - begin) / CLOCKS_PER_SEC << std::endl;
std::cout << "AccTime: " << (double) AcceleratorTotalNanos / CLOCKS_PER_SEC << std::endl;
write_output(N, N1, N2);
free(returnv);
}
