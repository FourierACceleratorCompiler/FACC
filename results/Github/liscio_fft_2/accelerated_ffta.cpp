/* Orignal skeleton is: 
Pre: SKELETON:

With the array index wrappers adi_acc_output,Annon
And (fromvars) []
Under dimensions [adi_acc_n = N (=) ]
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers adi_acc_n
And (fromvars) [N]
Under dimensions []
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers adi_acc_input,re
And (fromvars) [x, im]
Under dimensions [adi_acc_n = N (=) ]
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers adi_acc_input,im
And (fromvars) [x, re]
Under dimensions [adi_acc_n = N (=) ]
With conversion function IdentityConversion
Post: SKELETON:

With the array index wrappers returnvar,im
And (fromvars) [adi_acc_output, re]
Under dimensions [adi_acc_n = N (=) ]
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers returnvar,re
And (fromvars) [adi_acc_output, im]
Under dimensions [adi_acc_n = N (=) ]
With conversion function IdentityConversion
*/

/* Typemap is :
 N: int32
i14: int32
x: array(_complex_double_: with dims N (=) )
adi_acc_n: int32
i12: int32
adi_acc_input: array(complex_float: with dims adi_acc_n (=) )
i13: int32
returnvar: array(_complex_double_: with dims N (=) )
i15: int32
adi_acc_output: array(complex_float: with dims adi_acc_n (=) )
*/


extern "C" {
#include "../../benchmarks/Github/code/liscio_fft_2/self_contained_code.h"
}



extern "C" {
#include "../../benchmarks/Accelerators/FFTA/adi_emulation.c"
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

void write_output(_complex_double_ * x, int N, _complex_double_ * returnvar) {

    json output_json;
std::vector<json> output_temp_33;
for (unsigned int i34 = 0; i34 < N; i34++) {
_complex_double_ output_temp_35 = returnvar[i34];
json output_temp_36;

output_temp_36["re"] = output_temp_35.re;

output_temp_36["im"] = output_temp_35.im;
output_temp_33.push_back(output_temp_36);
}
output_json["returnvar"] = output_temp_33;
std::ofstream out_str(output_file); 
out_str << std::setw(4) << output_json << std::endl;
}

_complex_double_ * FFT_wrapper_accel_internal(_complex_double_ * x,int N) {

if ((PRIM_EQUAL(N, 16384)) || ((PRIM_EQUAL(N, 8192)) || ((PRIM_EQUAL(N, 4096)) || ((PRIM_EQUAL(N, 2048)) || ((PRIM_EQUAL(N, 1024)) || ((PRIM_EQUAL(N, 512)) || ((PRIM_EQUAL(N, 256)) || ((PRIM_EQUAL(N, 128)) || (PRIM_EQUAL(N, 64)))))))))) {
static complex_float adi_acc_output[16384]__attribute__((__aligned__(64)));;
	static int adi_acc_n;;
	adi_acc_n = N;;
	static complex_float adi_acc_input[16384]__attribute__((__aligned__(64)));;
	for (int i12 = 0; i12 < adi_acc_n; i12++) {
		adi_acc_input[i12].re = x[i12].im;
	};
	for (int i13 = 0; i13 < adi_acc_n; i13++) {
		adi_acc_input[i13].im = x[i13].re;
	};
	StartAcceleratorTimer();;
	accel_cfft_wrapper(adi_acc_input, adi_acc_output, adi_acc_n);;
	StopAcceleratorTimer();;
	_complex_double_* returnvar = (_complex_double_*) facc_malloc (0, sizeof(_complex_double_)*N);;
	for (int i14 = 0; i14 < adi_acc_n; i14++) {
		returnvar[i14].im = adi_acc_output[i14].re;
	};
	for (int i15 = 0; i15 < adi_acc_n; i15++) {
		returnvar[i15].re = adi_acc_output[i15].im;
	};
	
return returnvar;;
	
;
	
;
	

} else {

return FFT_wrapper(x, N);;
}
}
_complex_double_ * FFT_wrapper_accel(_complex_double_ * x, int N) {
return (_complex_double_ *)FFT_wrapper_accel_internal((_complex_double_ *) x, (int) N);
}
int main(int argc, char **argv) {
    char *inpname = argv[1]; 
    output_file = argv[2]; 

    std::ifstream ifs(inpname); 
    json input_json = json::parse(ifs);
std::vector<_complex_double_> x_vec;
for (auto& elem : input_json["x"]) {
double x_innerre = elem["re"];
double x_innerim = elem["im"];
_complex_double_ x_inner = { x_innerre, x_innerim};
x_vec.push_back(x_inner);
}
_complex_double_ *x = &x_vec[0];
int N = input_json["N"];

_complex_double_ *returnvar;
clock_t begin = clock();
for (int i = 0; i < TIMES; i ++) {
	if (returnvar)
		free(returnvar);
	returnvar = FFT_wrapper_accel(x, N);
}

clock_t end = clock();
std::cout << "Time: " << (double) (end - begin) / CLOCKS_PER_SEC << std::endl;
std::cout << "AccTime: " << (double) AcceleratorTotalNanos / CLOCKS_PER_SEC << std::endl;

// write_output(x, N, returnvar);
}
