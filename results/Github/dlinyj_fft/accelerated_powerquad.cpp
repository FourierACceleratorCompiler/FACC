/* Orignal skeleton is: 
Pre: SKELETON:

With the array index wrappers power_quad_acc_output,Annon
And (fromvars) []
Under dimensions [power_quad_acc_n = p (^2) ]
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers power_quad_acc_n
And (fromvars) [p]
Under dimensions []
With conversion function PowerOfTwoConversion

>(new binding): 

With the array index wrappers power_quad_acc_input,im
And (fromvars) [in, f32_2]
Under dimensions [power_quad_acc_n = p (^2) ]
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers power_quad_acc_input,re
And (fromvars) [in, f32_1]
Under dimensions [power_quad_acc_n = p (^2) ]
With conversion function IdentityConversion
Post: SKELETON:

With the array index wrappers out,f32_1
And (fromvars) [power_quad_acc_output, re]
Under dimensions [power_quad_acc_n = p (^2) ]
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers out,f32_2
And (fromvars) [power_quad_acc_output, im]
Under dimensions [power_quad_acc_n = p (^2) ]
With conversion function IdentityConversion
*/

/* Typemap is :
 out: array(facc_2xf32_t: with dims p (^2) )
c: array(facc_2xf32_t: with dims p (^2) )
power_quad_acc_n: int32
i52: int32
i54: int32
power_quad_acc_input: array(complex_type: with dims power_quad_acc_n (=) )
i55: int32
norm: int32
i53: int32
p: int32
in: array(facc_2xf32_t: with dims p (^2) )
bi_1: int64
power_quad_acc_output: array(complex_type: with dims power_quad_acc_n (=) )
*/

#include <clib/fft_synth/lib.h>
extern "C" {
#include "../../benchmarks/Github/code/dlinyj_fft/self_contained_code.c"
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

void write_output(int p, float * c, float * in, float * out, int norm) {

    json output_json;
std::vector<json> output_temp_197;
for (unsigned int i198 = 0; i198 < (1 << (p))* 2; i198++) {
float output_temp_199 = out[i198];

output_temp_197.push_back(output_temp_199);
}
output_json["out"] = output_temp_197;
std::ofstream out_str(output_file); 
out_str << std::setw(4) << output_json << std::endl;
}

void fft_calc_accel_internal(int p,facc_2xf32_t * c,facc_2xf32_t * in,facc_2xf32_t * out,int norm) {

if ((PRIM_EQUAL(norm, 1)) && ((PRIM_EQUAL(p, 11)) || ((PRIM_EQUAL(p, 10)) || ((PRIM_EQUAL(p, 9)) || ((PRIM_EQUAL(p, 8)) || ((PRIM_EQUAL(p, 7)) || (PRIM_EQUAL(p, 6)))))))) {
int power_quad_acc_n;;
	power_quad_acc_n = Pow2(p);;;
	complex_type power_quad_acc_output[power_quad_acc_n]__attribute__((__aligned__(64)));;
	complex_type power_quad_acc_input[power_quad_acc_n]__attribute__((__aligned__(64)));;
	for (int i52 = 0; i52 < power_quad_acc_n; i52++) {
		power_quad_acc_input[i52].im = in[i52].f32_2;
	};
	for (int i53 = 0; i53 < power_quad_acc_n; i53++) {
		power_quad_acc_input[i53].re = in[i53].f32_1;
	};
	StartAcceleratorTimer();;
	fft_api(power_quad_acc_input, power_quad_acc_output, power_quad_acc_n);;
	StopAcceleratorTimer();;
	for (int i54 = 0; i54 < power_quad_acc_n; i54++) {
		out[i54].f32_1 = power_quad_acc_output[i54].re;
	};
	for (int i55 = 0; i55 < power_quad_acc_n; i55++) {
		out[i55].f32_2 = power_quad_acc_output[i55].im;
	};
	
;
	
;
	
;
	
if (GREATER_THAN(norm, -1)) {
long int bi_1;;
	bi_1 = Pow2(p);;;
	ARRAY_HALF_NORM_POSTIND(out, f32_1, bi_1);;
	ARRAY_HALF_NORM_POSTIND(out, f32_2, bi_1);
} else {
;
}
} else {
fft_calc(p, (float *)c, (float *)in, (float *)out, norm);
}
}
void fft_calc_accel(int p, float * c, float * in, float * out, int norm) {
fft_calc_accel_internal((int) p, (facc_2xf32_t *) c, (facc_2xf32_t *) in, (facc_2xf32_t *) out, (int) norm);
}
int main(int argc, char **argv) {
    char *inpname = argv[1]; 
    output_file = argv[2]; 

    std::ifstream ifs(inpname); 
    json input_json = json::parse(ifs);
int p = input_json["p"];
std::vector<float> c_vec;
for (auto& elem : input_json["c"]) {
float c_inner = elem;
c_vec.push_back(c_inner);
}
float *c = &c_vec[0];
std::vector<float> in_vec;
for (auto& elem : input_json["in"]) {
float in_inner = elem;
in_vec.push_back(in_inner);
}
float *in = &in_vec[0];
int norm = input_json["norm"];
float out[(1 << (p))* 2];
clock_t begin = clock();
fft_calc_accel(p, c, in, out, norm);
clock_t end = clock();
std::cout << "Time: " << (double) (end - begin) / CLOCKS_PER_SEC << std::endl;
std::cout << "AccTime: " << (double) AcceleratorTotalNanos / CLOCKS_PER_SEC << std::endl;
write_output(p, c, in, out, norm);
}
