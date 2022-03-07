#include<vector>
#include<nlohmann/json.hpp>
#include<fstream>
#include<iomanip>
#include<clib/synthesizer.h>
#include<time.h>
#include<iostream>
extern "C" {
#include "../../../benchmarks/Github/code/biotrump_OouraFFT/self_contained_code.c"
}
char *output_file; 
char *pre_accel_dump_file; // optional dump file. 
using json = nlohmann::json;


clock_t AcceleratorStart;
clock_t AcceleratorTotalNanos = 0;
void StartAcceleratorTimer() {
	AcceleratorStart = clock();
}

void StopAcceleratorTimer() {
	AcceleratorTotalNanos +=
		(clock()) - AcceleratorStart;
}

void write_output(int n, int isgn, double * a, int * ip, double * w) {

    json output_json;
std::vector<json> output_temp_1;
for (unsigned int i2 = 0; i2 < n; i2++) {
double output_temp_3 = a[i2];

output_temp_1.push_back(output_temp_3);
}
output_json["a"] = output_temp_1;
std::ofstream out_str(output_file); 
out_str << std::setw(4) << output_json << std::endl;
}
int main(int argc, char **argv) {
    char *inpname = argv[1]; 
    output_file = argv[2]; 

    std::ifstream ifs(inpname); 
    json input_json = json::parse(ifs);
int n = input_json["n"];
std::vector<double> a_vec;
for (auto& elem : input_json["a"]) {
double a_inner = elem;
a_vec.push_back(a_inner);
}
double *a = &a_vec[0];
std::vector<int> ip_vec;
clock_t begin = clock();
double w[n * 5 / 4];
int isgn = 1;
int ip[64]; // should be sqrt n, number from example
ip[0] = 0;
for (int i = 0; i < TIMES; i ++) {
	cdft_w(n, isgn, a, ip, w);
}
clock_t end = clock();
std::cout << "Time: " << (double) (end - begin) / CLOCKS_PER_SEC << std::endl;
std::cout << "AccTime: " << (double) AcceleratorTotalNanos / CLOCKS_PER_SEC << std::endl;
write_output(n, isgn, a, ip, w);
}
