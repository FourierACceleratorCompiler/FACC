/* Orignal skeleton is: 
Pre: SKELETON:

With the array index wrappers adi_acc_output,Annon
And (fromvars) []
Under dimensions [adi_acc_n = n (=) ]
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers adi_acc_n
And (fromvars) [n]
Under dimensions []
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers adi_acc_input,re
And (fromvars) [invec, im]
Under dimensions [adi_acc_n = n (=) ]
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers adi_acc_input,im
And (fromvars) [invec, re]
Under dimensions [adi_acc_n = n (=) ]
With conversion function IdentityConversion
Post: SKELETON:

With the array index wrappers outvec,im
And (fromvars) [adi_acc_output, re]
Under dimensions [adi_acc_n = n (=) ]
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers outvec,re
And (fromvars) [adi_acc_output, im]
Under dimensions [adi_acc_n = n (=) ]
With conversion function IdentityConversion
*/

/* Typemap is :
 i14: int32
adi_acc_n: int32
i12: int32
adi_acc_input: array(complex_float: with dims adi_acc_n (=) )
i13: int32
outvec: array(_complex_double_: with dims n (=) )
n: uint32
i15: int32
adi_acc_output: array(complex_float: with dims adi_acc_n (=) )
invec: array(_complex_double_: with dims n (=) )
forward: bool
*/


extern "C" {
#include "../../benchmarks/Github/code/gregfjohnson_fft/self_contained_code.c"
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

void write_output(_complex_double_ * outvec, _complex_double_ * invec, unsigned int n, bool forward) {

    json output_json;
std::vector<json> output_temp_33;
for (unsigned int i34 = 0; i34 < n; i34++) {
_complex_double_ output_temp_35 = outvec[i34];
json output_temp_36;

output_temp_36["re"] = output_temp_35.re;

output_temp_36["im"] = output_temp_35.im;
output_temp_33.push_back(output_temp_36);
}
output_json["outvec"] = output_temp_33;
std::ofstream out_str(output_file); 
out_str << std::setw(4) << output_json << std::endl;
}

void recFFT_wrapper_accel_internal(_complex_double_ * outvec,_complex_double_ * invec,unsigned int n,bool forward) {

if ((PRIM_EQUAL(n, 16384)) || ((PRIM_EQUAL(n, 8192)) || ((PRIM_EQUAL(n, 4096)) || ((PRIM_EQUAL(n, 2048)) || ((PRIM_EQUAL(n, 1024)) || ((PRIM_EQUAL(n, 512)) || ((PRIM_EQUAL(n, 256)) || ((PRIM_EQUAL(n, 128)) || (PRIM_EQUAL(n, 64)))))))))) {
static complex_float adi_acc_output[16384]__attribute__((__aligned__(64)));;
	static int adi_acc_n;;
	adi_acc_n = n;;
	static complex_float adi_acc_input[16384]__attribute__((__aligned__(64)));;
	for (int i12 = 0; i12 < adi_acc_n; i12++) {
		adi_acc_input[i12].re = invec[i12].im;
	};
	for (int i13 = 0; i13 < adi_acc_n; i13++) {
		adi_acc_input[i13].im = invec[i13].re;
	};
	StartAcceleratorTimer();;
	accel_cfft_wrapper(adi_acc_input, adi_acc_output, adi_acc_n);;
	StopAcceleratorTimer();;
	for (int i14 = 0; i14 < adi_acc_n; i14++) {
		outvec[i14].im = adi_acc_output[i14].re;
	};
	for (int i15 = 0; i15 < adi_acc_n; i15++) {
		outvec[i15].re = adi_acc_output[i15].im;
	};
	
;
	
;
	

} else {
recFFT_wrapper(outvec, invec, n, forward);
}
}
void recFFT_wrapper_accel(_complex_double_ * outvec, _complex_double_ * invec, unsigned int n, bool forward) {
recFFT_wrapper_accel_internal((_complex_double_ *) outvec, (_complex_double_ *) invec, (unsigned int) n, (bool) forward);
}
int main(int argc, char **argv) {
    char *inpname = argv[1]; 
    output_file = argv[2]; 

    std::ifstream ifs(inpname); 
    json input_json = json::parse(ifs);
std::vector<_complex_double_> invec_vec;
for (auto& elem : input_json["invec"]) {
double invec_innerre = elem["re"];
double invec_innerim = elem["im"];
_complex_double_ invec_inner = { invec_innerre, invec_innerim};
invec_vec.push_back(invec_inner);
}
_complex_double_ *invec = &invec_vec[0];
unsigned int n = input_json["n"];
bool forward = input_json["forward"];
_complex_double_ outvec[n];
clock_t begin = clock();
recFFT_wrapper_accel(outvec, invec, n, forward);
clock_t end = clock();
std::cout << "Time: " << (double) (end - begin) / CLOCKS_PER_SEC << std::endl;
std::cout << "AccTime: " << (double) AcceleratorTotalNanos / CLOCKS_PER_SEC << std::endl;
write_output(outvec, invec, n, forward);
}
