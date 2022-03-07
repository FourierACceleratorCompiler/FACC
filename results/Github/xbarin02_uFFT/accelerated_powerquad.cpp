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
And (fromvars) [vector, im]
Under dimensions [power_quad_acc_n = N (=) ]
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers power_quad_acc_input,re
And (fromvars) [vector, re]
Under dimensions [power_quad_acc_n = N (=) ]
With conversion function IdentityConversion
Post: SKELETON:

With the array index wrappers vector,im
And (fromvars) [power_quad_acc_output, im]
Under dimensions [power_quad_acc_n = N (=) ]
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers vector,re
And (fromvars) [power_quad_acc_output, re]
Under dimensions [power_quad_acc_n = N (=) ]
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers returnv
And (fromvars) [Constant(0)]
Under dimensions []
With conversion function IdentityConversion
*/

/* Typemap is :
 i47: int32
vector: array(_float_complex_: with dims N (=) )
N: int64
power_quad_acc_n: int32
power_quad_acc_input: array(complex_type: with dims power_quad_acc_n (=) )
i50: int32
returnv: int32
i49: int32
i48: int32
power_quad_acc_output: array(complex_type: with dims power_quad_acc_n (=) )
*/


extern "C" {
#include "../../benchmarks/Github/code/xbarin02_uFFT/self_contained_code.c"
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
int power_quad_acc_n;;
	power_quad_acc_n = N;;
	complex_type power_quad_acc_output[power_quad_acc_n]__attribute__((__aligned__(64)));;
	complex_type power_quad_acc_input[power_quad_acc_n]__attribute__((__aligned__(64)));;
	for (int i47 = 0; i47 < power_quad_acc_n; i47++) {
		power_quad_acc_input[i47].im = vector[i47].im;
	};
	for (int i48 = 0; i48 < power_quad_acc_n; i48++) {
		power_quad_acc_input[i48].re = vector[i48].re;
	};
	StartAcceleratorTimer();;
	fft_api(power_quad_acc_input, power_quad_acc_output, power_quad_acc_n);;
	StopAcceleratorTimer();;
	for (int i49 = 0; i49 < power_quad_acc_n; i49++) {
		vector[i49].im = power_quad_acc_output[i49].im;
	};
	for (int i50 = 0; i50 < power_quad_acc_n; i50++) {
		vector[i50].re = power_quad_acc_output[i50].re;
	};
	int returnv;;
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