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
And (fromvars) [vector, re]
Under dimensions [adi_acc_n = N (=) ]
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers adi_acc_input,im
And (fromvars) [vector, im]
Under dimensions [adi_acc_n = N (=) ]
With conversion function IdentityConversion
Post: SKELETON:

With the array index wrappers vector,im
And (fromvars) [adi_acc_output, im]
Under dimensions [adi_acc_n = N (=) ]
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers vector,re
And (fromvars) [adi_acc_output, re]
Under dimensions [adi_acc_n = N (=) ]
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers returnv
And (fromvars) [Constant(0)]
Under dimensions []
With conversion function IdentityConversion
*/

/* Typemap is :
 i18: int32
i17: int32
vector: array(_float_complex_: with dims N (=) )
N: int64
adi_acc_n: int32
i20: int32
returnv: int32
adi_acc_input: array(complex_float: with dims adi_acc_n (=) )
adi_acc_output: array(complex_float: with dims adi_acc_n (=) )
i19: int32
*/


extern "C" {
#include "../../benchmarks/Github/code/xbarin02_uFFT_2/self_contained_code.c"
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

void write_output(_float_complex_ * vector, long int N, int returnv) {

    json output_json;
std::vector<json> output_temp_97;
for (unsigned int i98 = 0; i98 < N; i98++) {
_float_complex_ output_temp_99 = vector[i98];
json output_temp_100;

output_temp_100["re"] = output_temp_99.re;

output_temp_100["im"] = output_temp_99.im;
output_temp_97.push_back(output_temp_100);
}
output_json["vector"] = output_temp_97;

output_json["returnv"] = returnv;
std::ofstream out_str(output_file); 
out_str << std::setw(4) << output_json << std::endl;
}

int fft_wrapper_accel_internal(_float_complex_ * vector,long int N) {

if ((PRIM_EQUAL(N, 16384)) || ((PRIM_EQUAL(N, 8192)) || ((PRIM_EQUAL(N, 4096)) || ((PRIM_EQUAL(N, 2048)) || ((PRIM_EQUAL(N, 1024)) || ((PRIM_EQUAL(N, 512)) || ((PRIM_EQUAL(N, 256)) || ((PRIM_EQUAL(N, 128)) || (PRIM_EQUAL(N, 64)))))))))) {
static complex_float adi_acc_output[16384]__attribute__((__aligned__(64)));;
	static int adi_acc_n;;
	adi_acc_n = N;;
	static complex_float adi_acc_input[16384]__attribute__((__aligned__(64)));;
	for (int i17 = 0; i17 < adi_acc_n; i17++) {
		adi_acc_input[i17].re = vector[i17].re;
	};
	for (int i18 = 0; i18 < adi_acc_n; i18++) {
		adi_acc_input[i18].im = vector[i18].im;
	};
	StartAcceleratorTimer();;
	accel_cfft_wrapper(adi_acc_input, adi_acc_output, adi_acc_n);;
	StopAcceleratorTimer();;
	for (int i19 = 0; i19 < adi_acc_n; i19++) {
		vector[i19].im = adi_acc_output[i19].im;
	};
	for (int i20 = 0; i20 < adi_acc_n; i20++) {
		vector[i20].re = adi_acc_output[i20].re;
	};
	static int returnv;;
	returnv = 0;;
	
return returnv;;
	
;
	
;
	

} else {

return fft_wrapper(vector, N);;
}
}
int fft_wrapper_accel(_float_complex_ * vector, long int N) {
return (int)fft_wrapper_accel_internal((_float_complex_ *) vector, (long int) N);
}
int main(int argc, char **argv) {
    char *inpname = argv[1]; 
    output_file = argv[2]; 

    std::ifstream ifs(inpname); 
    json input_json = json::parse(ifs);
std::vector<_float_complex_> vector_vec;
for (auto& elem : input_json["vector"]) {
float vector_innerre = elem["re"];
float vector_innerim = elem["im"];
_float_complex_ vector_inner = { vector_innerre, vector_innerim};
vector_vec.push_back(vector_inner);
}
_float_complex_ *vector = &vector_vec[0];
long int N = input_json["N"];
clock_t begin = clock();
int returnv = fft_wrapper_accel(vector, N);
clock_t end = clock();
std::cout << "Time: " << (double) (end - begin) / CLOCKS_PER_SEC << std::endl;
std::cout << "AccTime: " << (double) AcceleratorTotalNanos / CLOCKS_PER_SEC << std::endl;
write_output(vector, N, returnv);
}
