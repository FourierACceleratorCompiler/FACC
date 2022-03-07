#include<vector>
#include<nlohmann/json.hpp>
#include<fstream>
#include<iomanip>
#include<clib/synthesizer.h>
#include<chrono>
#include<iostream>
extern "C" {
#include "self_contained_code.c"
}
char *output_file; 
char *pre_accel_dump_file; // optional dump file. 
using json = nlohmann::json;


std::chrono::steady_clock::time_point AcceleratorStart;
long long AcceleratorTotalNanos = 0;
void StartAcceleratorTimer() {
	AcceleratorStart = std::chrono::steady_clock::now();
}

void StopAcceleratorTimer() {
	AcceleratorTotalNanos +=
		std::chrono::duration_cast<std::chrono::nanoseconds>(
			std::chrono::steady_clock::now() - AcceleratorStart
		).count();
}

void write_output(complex * x, int N, complex * returnv) {

    json output_json;
std::vector<json> output_temp_1;
for (unsigned int i2 = 0; i2 < N; i2++) {
complex output_temp_3 = returnv[i2];
json output_temp_4;

output_temp_4["re"] = output_temp_3.re;

output_temp_4["im"] = output_temp_3.im;
output_temp_1.push_back(output_temp_4);
}
output_json["returnv"] = output_temp_1;
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
clock_t begin = clock();
complex *returnv;
for (int i = 0; i < TIMES; i ++) {
	if (returnv)
		free(returnv);
	returnv = DFT_naive(x, N);
}
clock_t end = clock();
std::cout << "Time: " << (double) (end - begin) / CLOCKS_PER_SEC << std::endl;
std::cout << "AccTime: " << AcceleratorTotalNanos << std::endl;
write_output(x, N, returnv);
}
