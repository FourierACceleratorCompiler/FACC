/* Orignal skeleton is: 
Pre: SKELETON:

With the array index wrappers power_quad_acc_output,Annon
And (fromvars) []
Under dimensions [power_quad_acc_n = N (=) ]
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers power_quad_acc_n
And (fromvars) [N]
Under dimensions []
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers power_quad_acc_input,im
And (fromvars) [x, im]
Under dimensions [power_quad_acc_n = N (=) ]
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers power_quad_acc_input,re
And (fromvars) [x, re]
Under dimensions [power_quad_acc_n = N (=) ]
With conversion function IdentityConversion
Post: SKELETON:

With the array index wrappers returnv,im
And (fromvars) [power_quad_acc_output, im]
Under dimensions [power_quad_acc_n = N (=) ]
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers returnv,re
And (fromvars) [power_quad_acc_output, re]
Under dimensions [power_quad_acc_n = N (=) ]
With conversion function IdentityConversion
*/

/* Typemap is :
 i18: int32
i17: int32
N: int32
power_quad_acc_n: int32
x: array(complex: with dims N (=) )
power_quad_acc_input: array(complex_type: with dims power_quad_acc_n (=) )
i20: int32
returnv: array(complex: with dims N (=) )
power_quad_acc_output: array(complex_type: with dims power_quad_acc_n (=) )
i19: int32
*/


extern "C" {
#include "../../benchmarks/Github/code/jtfell_c-fft/self_contained_code.c"
}



extern "C" {
#include "../../benchmarks/Accelerators/PowerQuad/powerquad_emulation.c"
}



#include<vector>
#include<nlohmann/json.hpp>
#include<fstream>
#include<iomanip>
#include<clib/synthesizer.h>
#include<time.h>
#include<iostream>
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

void write_output(complex * x, int N, complex * returnv) {

    json output_json;
std::vector<json> output_temp_33;
for (unsigned int i34 = 0; i34 < N; i34++) {
complex output_temp_35 = returnv[i34];
json output_temp_36;

output_temp_36["re"] = output_temp_35.re;

output_temp_36["im"] = output_temp_35.im;
output_temp_33.push_back(output_temp_36);
}
output_json["returnv"] = output_temp_33;
std::ofstream out_str(output_file); 
out_str << std::setw(4) << output_json << std::endl;
}

complex * DFT_naive_accel_internal(complex * x,int N) {

if ((PRIM_EQUAL(N, 2048)) || ((PRIM_EQUAL(N, 1024)) || ((PRIM_EQUAL(N, 512)) || ((PRIM_EQUAL(N, 256)) || ((PRIM_EQUAL(N, 128)) || (PRIM_EQUAL(N, 64))))))) {
int power_quad_acc_n;;
	power_quad_acc_n = N;;
	complex_type power_quad_acc_output[power_quad_acc_n]__attribute__((__aligned__(64)));;
	complex_type power_quad_acc_input[power_quad_acc_n]__attribute__((__aligned__(64)));;
	for (int i17 = 0; i17 < power_quad_acc_n; i17++) {
		power_quad_acc_input[i17].im = x[i17].im;
	};
	for (int i18 = 0; i18 < power_quad_acc_n; i18++) {
		power_quad_acc_input[i18].re = x[i18].re;
	};
	StartAcceleratorTimer();;
	fft_api(power_quad_acc_input, power_quad_acc_output, power_quad_acc_n);;
	StopAcceleratorTimer();;
	complex* returnv = (complex*) facc_malloc (0, sizeof(complex)*N);;
	for (int i19 = 0; i19 < power_quad_acc_n; i19++) {
		returnv[i19].im = power_quad_acc_output[i19].im;
	};
	for (int i20 = 0; i20 < power_quad_acc_n; i20++) {
		returnv[i20].re = power_quad_acc_output[i20].re;
	};
	
return returnv;;
	
;
	
;
	

} else {

return DFT_naive(x, N);;
}
}
complex * DFT_naive_accel(complex * x, int N) {
return (complex *)DFT_naive_accel_internal((complex *) x, (int) N);
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
complex * returnv = DFT_naive_accel(x, N);
clock_t end = clock();
std::cout << "Time: " << (double) (end - begin) / CLOCKS_PER_SEC << std::endl;
std::cout << "AccTime: " << (double) AcceleratorTotalNanos / CLOCKS_PER_SEC << std::endl;
write_output(x, N, returnv);
}