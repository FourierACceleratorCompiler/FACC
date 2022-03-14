/* Orignal skeleton is: 
Pre: SKELETON:

With the array index wrappers api_out,Annon
And (fromvars) []
Under dimensions [interface_len = p (^2) ]
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers dir
And (fromvars) [norm]
Under dimensions []
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers interface_len
And (fromvars) [p]
Under dimensions []
With conversion function PowerOfTwoConversion

>(new binding): 

With the array index wrappers api_in,im
And (fromvars) [in, f32_1]
Under dimensions [interface_len = p (^2) ]
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers api_in,re
And (fromvars) [in, f32_2]
Under dimensions [interface_len = p (^2) ]
With conversion function IdentityConversion
Post: SKELETON:

With the array index wrappers out,f32_1
And (fromvars) [api_out, im]
Under dimensions [interface_len = p (^2) ]
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers out,f32_2
And (fromvars) [api_out, re]
Under dimensions [interface_len = p (^2) ]
With conversion function IdentityConversion
*/

/* Typemap is :
 out: array(facc_2xf32_t: with dims p (^2) )
dir: int16
i427: int32
c: array(facc_2xf32_t: with dims p (^2) )
api_in: array(_complex_float_: with dims interface_len (=) )
i429: int32
i430: int32
norm: int32
interface_len: int32
p: int32
in: array(facc_2xf32_t: with dims p (^2) )
bi_1: int64
api_out: array(_complex_float_: with dims interface_len (=) )
i428: int32
*/

#include <clib/fft_synth/lib.h>
extern "C" {
#include "../../benchmarks/Github/code/dlinyj_fft/self_contained_code.c"
}



#include "../../benchmarks/Accelerators/FFTW/interface.hpp"
#include "complex"


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
std::vector<json> output_temp_1485;
for (unsigned int i1486 = 0; i1486 < (1 << (p))* 2; i1486++) {
float output_temp_1487 = out[i1486];

output_temp_1485.push_back(output_temp_1487);
}
output_json["out"] = output_temp_1485;
std::ofstream out_str(output_file); 
out_str << std::setw(4) << output_json << std::endl;
}

void fft_calc_accel_internal(int p,facc_2xf32_t * c,facc_2xf32_t * in,facc_2xf32_t * out,int norm) {

if ((PRIM_EQUAL(norm, 1)) && ((PRIM_EQUAL(p, 15)) || ((PRIM_EQUAL(p, 14)) || ((PRIM_EQUAL(p, 13)) || ((PRIM_EQUAL(p, 12)) || ((PRIM_EQUAL(p, 11)) || ((PRIM_EQUAL(p, 10)) || ((PRIM_EQUAL(p, 9)) || ((PRIM_EQUAL(p, 8)) || ((PRIM_EQUAL(p, 7)) || ((PRIM_EQUAL(p, 6)) || ((PRIM_EQUAL(p, 5)) || ((PRIM_EQUAL(p, 4)) || ((PRIM_EQUAL(p, 3)) || ((PRIM_EQUAL(p, 2)) || (PRIM_EQUAL(p, 1))))))))))))))))) {
	static short dir;;
	dir = norm;;
	static int interface_len;;
	interface_len = Pow2(p);;;
	_complex_float_ api_out[interface_len];;
	_complex_float_ api_in[interface_len];;
	for (int i427 = 0; i427 < interface_len; i427++) {
		api_in[i427].im = in[i427].f32_1;
	};
	for (int i428 = 0; i428 < interface_len; i428++) {
		api_in[i428].re = in[i428].f32_2;
	};
	StartAcceleratorTimer();;
	fftwf_example_api(api_in, api_out, interface_len, dir);;
	StopAcceleratorTimer();;
	for (int i429 = 0; i429 < interface_len; i429++) {
		out[i429].f32_1 = api_out[i429].im;
	};
	for (int i430 = 0; i430 < interface_len; i430++) {
		out[i430].f32_2 = api_out[i430].re;
	};
	
;
	
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
int norm = input_json["norm"];
float out[(1 << (p))* 2];

float c[1 << (p + 1)];
fft_make(p, c);
clock_t begin = clock();
for (int i = 0; i < TIMES; i ++) {
	fft_calc_accel(p, c, in, out, norm);
}
clock_t end = clock();

std::cout << "Time: " << (double) (end - begin) / CLOCKS_PER_SEC << std::endl;
std::cout << "AccTime: " << (double) AcceleratorTotalNanos / CLOCKS_PER_SEC << std::endl;

write_output(p, c, in, out, norm);
}
