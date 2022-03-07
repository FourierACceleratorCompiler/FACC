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

void write_output(int n, int isgn, double * a, int * ip, double * w) {

    json output_json;

output_json["n"] = n;

output_json["isgn"] = isgn;
std::vector<json> output_temp_4;
for (unsigned int i5 = 0; i5 < n; i5++) {
int output_temp_6 = ip[i5];

output_temp_4.push_back(output_temp_6);
}
output_json["ip"] = output_temp_4;
std::vector<json> output_temp_7;
/* for (unsigned int i8 = 0; i8 < n; i8++) { */
/* double output_temp_9 = w[i8]; */

/* output_temp_7.push_back(output_temp_9); */
/* } */
/* output_json["w"] = output_temp_7; */
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
int ip[n]; // really could be sqrt n.
for (int i = 0; i < TIMES; i ++) {
	ip[0] = 0;
	// This benchmark has a special case for
	// where it needs the outputs to be written
	// just before input because cdft doesn't
	// leave ip unchanged --- this is technically
	// the correct place to do this anyway, I
	// just use the easier place outside the loop
	// normally.  Note that ip/w do not escape (and would
	// not escape in a normal usage pattern) so
	// are not liveout.
	write_output(n, isgn, a, ip, w);
	cdft_w(n, isgn, a, ip, w);
}
clock_t end = clock();
std::cout << "Time: " << (double) (end - begin) / CLOCKS_PER_SEC << std::endl;
std::cout << "AccTime: " << (double) AcceleratorTotalNanos / CLOCKS_PER_SEC << std::endl;
}
