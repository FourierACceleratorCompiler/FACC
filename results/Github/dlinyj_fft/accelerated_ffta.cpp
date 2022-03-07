/* Orignal skeleton is: 
Pre: SKELETON:

With the array index wrappers adi_acc_output,Annon
And (fromvars) []
Under dimensions [adi_acc_n = p (^2) ]
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers adi_acc_n
And (fromvars) [p]
Under dimensions []
With conversion function PowerOfTwoConversion

>(new binding): 

With the array index wrappers adi_acc_input,re
And (fromvars) [in, f32_1]
Under dimensions [adi_acc_n = p (^2) ]
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers adi_acc_input,im
And (fromvars) [in, f32_2]
Under dimensions [adi_acc_n = p (^2) ]
With conversion function IdentityConversion
Post: SKELETON:

With the array index wrappers out,f32_1
And (fromvars) [adi_acc_output, re]
Under dimensions [adi_acc_n = p (^2) ]
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers out,f32_2
And (fromvars) [adi_acc_output, im]
Under dimensions [adi_acc_n = p (^2) ]
With conversion function IdentityConversion
*/

/* Typemap is :
 out: array(facc_2xf32_t: with dims p (^2) )
c: array(facc_2xf32_t: with dims p (^2) )
i25: int32
i22: int32
i23: int32
adi_acc_n: int32
i24: int32
norm: int32
adi_acc_input: array(complex_float: with dims adi_acc_n (=) )
p: int32
in: array(facc_2xf32_t: with dims p (^2) )
bi_1: int64
adi_acc_output: array(complex_float: with dims adi_acc_n (=) )
*/

#include <clib/fft_synth/lib.h>
extern "C" {
#include "../../benchmarks/Github/code/dlinyj_fft/self_contained_code.c"
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

if ((PRIM_EQUAL(norm, 1)) && ((PRIM_EQUAL(p, 14)) || ((PRIM_EQUAL(p, 13)) || ((PRIM_EQUAL(p, 12)) || ((PRIM_EQUAL(p, 11)) || ((PRIM_EQUAL(p, 10)) || ((PRIM_EQUAL(p, 9)) || ((PRIM_EQUAL(p, 8)) || ((PRIM_EQUAL(p, 7)) || (PRIM_EQUAL(p, 6))))))))))) {
static complex_float adi_acc_output[16384]__attribute__((__aligned__(64)));;
	static int adi_acc_n;;
	adi_acc_n = Pow2(p);;;
	static complex_float adi_acc_input[16384]__attribute__((__aligned__(64)));;
	for (int i22 = 0; i22 < adi_acc_n; i22++) {
		adi_acc_input[i22].re = in[i22].f32_1;
	};
	for (int i23 = 0; i23 < adi_acc_n; i23++) {
		adi_acc_input[i23].im = in[i23].f32_2;
	};
	StartAcceleratorTimer();;
	accel_cfft_wrapper(adi_acc_input, adi_acc_output, adi_acc_n);;
	StopAcceleratorTimer();;
	for (int i24 = 0; i24 < adi_acc_n; i24++) {
		out[i24].f32_1 = adi_acc_output[i24].re;
	};
	for (int i25 = 0; i25 < adi_acc_n; i25++) {
		out[i25].f32_2 = adi_acc_output[i25].im;
	};
	
;
	
;
	
;
	
if (GREATER_THAN(norm, -1)) {
static long int bi_1;;
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
std::vector<float> in_vec;
for (auto& elem : input_json["in"]) {
float in_inner = elem;
in_vec.push_back(in_inner);
}
float *in = &in_vec[0];
float c[1 << (p + 1)];
fft_make(p, c);
int norm = input_json["norm"];
float out[(1 << (p))* 2];
clock_t begin = clock();
for (int i = 0 ; i < TIMES; i ++) {
	fft_calc_accel(p, c, in, out, norm);
}
clock_t end = clock();
std::cout << "Time: " << (double) (end - begin) / CLOCKS_PER_SEC << std::endl;
std::cout << "AccTime: " << (double) AcceleratorTotalNanos / CLOCKS_PER_SEC << std::endl;
write_output(p, c, in, out, norm);
}
