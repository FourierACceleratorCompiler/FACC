/* Orignal skeleton is: 
Pre: SKELETON:

With the array index wrappers api_out,Annon
And (fromvars) []
Under dimensions [interface_len = n (=) ]
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers dir
And (fromvars) [forward]
Under dimensions []
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers interface_len
And (fromvars) [n]
Under dimensions []
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers api_in,im
And (fromvars) [invec, re]
Under dimensions [interface_len = n (=) ]
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers api_in,re
And (fromvars) [invec, im]
Under dimensions [interface_len = n (=) ]
With conversion function IdentityConversion
Post: SKELETON:

With the array index wrappers outvec,im
And (fromvars) [api_out, re]
Under dimensions [interface_len = n (=) ]
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers outvec,re
And (fromvars) [api_out, im]
Under dimensions [interface_len = n (=) ]
With conversion function IdentityConversion
*/

/* Typemap is :
 dir: int16
i5: int32
api_in: array(_complex_float_: with dims interface_len (=) )
i2: int32
i3: int32
interface_len: int32
i4: int32
api_out: array(_complex_float_: with dims interface_len (=) )
outvec: array(_complex_double_: with dims n (=) )
n: uint32
invec: array(_complex_double_: with dims n (=) )
forward: bool
*/


extern "C" {
#include "../../benchmarks/Github/code/gregfjohnson_fft/self_contained_code.c"
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

void write_output(_complex_double_ * outvec, _complex_double_ * invec, unsigned int n, bool forward) {

    json output_json;
std::vector<json> output_temp_97;
for (unsigned int i98 = 0; i98 < n; i98++) {
_complex_double_ output_temp_99 = outvec[i98];
json output_temp_100;

output_temp_100["re"] = output_temp_99.re;

output_temp_100["im"] = output_temp_99.im;
output_temp_97.push_back(output_temp_100);
}
output_json["outvec"] = output_temp_97;
std::ofstream out_str(output_file); 
out_str << std::setw(4) << output_json << std::endl;
}

void recFFT_wrapper_accel_internal(_complex_double_ * outvec,_complex_double_ * invec,unsigned int n,bool forward) {

if (((PRIM_EQUAL((short)forward, 1)) || (PRIM_EQUAL((short)forward, -1))) && ((PRIM_EQUAL(n, 524288)) || ((PRIM_EQUAL(n, 262144)) || ((PRIM_EQUAL(n, 131072)) || ((PRIM_EQUAL(n, 65536)) || ((PRIM_EQUAL(n, 32768)) || ((PRIM_EQUAL(n, 16384)) || ((PRIM_EQUAL(n, 8192)) || ((PRIM_EQUAL(n, 4096)) || ((PRIM_EQUAL(n, 2048)) || ((PRIM_EQUAL(n, 1024)) || ((PRIM_EQUAL(n, 512)) || ((PRIM_EQUAL(n, 256)) || ((PRIM_EQUAL(n, 128)) || ((PRIM_EQUAL(n, 64)) || ((PRIM_EQUAL(n, 32)) || ((PRIM_EQUAL(n, 16)) || ((PRIM_EQUAL(n, 8)) || ((PRIM_EQUAL(n, 4)) || (PRIM_EQUAL(n, 2))))))))))))))))))))) {
short dir;;
	dir = forward;;
	int interface_len;;
	interface_len = n;;
	_complex_float_ api_out[interface_len];;
	_complex_float_ api_in[interface_len];;
	for (int i2 = 0; i2 < interface_len; i2++) {
		api_in[i2].im = invec[i2].re;
	};
	for (int i3 = 0; i3 < interface_len; i3++) {
		api_in[i3].re = invec[i3].im;
	};
	StartAcceleratorTimer();;
	fftwf_example_api(api_in, api_out, interface_len, dir);;
	StopAcceleratorTimer();;
	for (int i4 = 0; i4 < interface_len; i4++) {
		outvec[i4].im = api_out[i4].re;
	};
	for (int i5 = 0; i5 < interface_len; i5++) {
		outvec[i5].re = api_out[i5].im;
	};
	
;
	
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
for (int i = 0; i < n; i ++) {
	recFFT_wrapper_accel(outvec, invec, n, forward);
}
clock_t end = clock();
std::cout << "Time: " << (double) (end - begin) / CLOCKS_PER_SEC << std::endl;
std::cout << "AccTime: " << (double) AcceleratorTotalNanos / CLOCKS_PER_SEC << std::endl;
write_output(outvec, invec, n, forward);
}
