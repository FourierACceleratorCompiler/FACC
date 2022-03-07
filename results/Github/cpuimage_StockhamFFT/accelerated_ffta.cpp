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
And (fromvars) [x, real]
Under dimensions [adi_acc_n = n (=) ]
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers adi_acc_input,im
And (fromvars) [x, imag]
Under dimensions [adi_acc_n = n (=) ]
With conversion function IdentityConversion
Post: SKELETON:

With the array index wrappers x,real
And (fromvars) [adi_acc_output, re]
Under dimensions [adi_acc_n = n (=) ]
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers x,imag
And (fromvars) [adi_acc_output, im]
Under dimensions [adi_acc_n = n (=) ]
With conversion function IdentityConversion
*/

/* Typemap is :
 i5: int32
x: array(complex_t: with dims n (=) )
i2: int32
adi_acc_n: int32
i3: int32
adi_acc_input: array(complex_float: with dims adi_acc_n (=) )
i4: int32
n: int32
adi_acc_output: array(complex_float: with dims adi_acc_n (=) )
*/

#include <clib/fft_synth/lib.h>
extern "C" {
#include "../../benchmarks/Github/code/cpuimage_StockhamFFT/self_contained_code.c"
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

void write_output(complex_t * x, int n) {

    json output_json;
std::vector<json> output_temp_33;
for (unsigned int i34 = 0; i34 < n; i34++) {
complex_t output_temp_35 = x[i34];
json output_temp_36;

output_temp_36["real"] = output_temp_35.real;

output_temp_36["imag"] = output_temp_35.imag;
output_temp_33.push_back(output_temp_36);
}
output_json["x"] = output_temp_33;
std::ofstream out_str(output_file); 
out_str << std::setw(4) << output_json << std::endl;
}

void fft_accel_internal(complex_t * x,int n) {

if ((PRIM_EQUAL(n, 16384)) || ((PRIM_EQUAL(n, 8192)) || ((PRIM_EQUAL(n, 4096)) || ((PRIM_EQUAL(n, 2048)) || ((PRIM_EQUAL(n, 1024)) || ((PRIM_EQUAL(n, 512)) || ((PRIM_EQUAL(n, 256)) || ((PRIM_EQUAL(n, 128)) || (PRIM_EQUAL(n, 64)))))))))) {
static complex_float adi_acc_output[16384]__attribute__((__aligned__(64)));;
	static int adi_acc_n;;
	adi_acc_n = n;;
	static complex_float adi_acc_input[16384]__attribute__((__aligned__(64)));;
	for (int i2 = 0; i2 < adi_acc_n; i2++) {
		adi_acc_input[i2].re = x[i2].real;
	};
	for (int i3 = 0; i3 < adi_acc_n; i3++) {
		adi_acc_input[i3].im = x[i3].imag;
	};
	StartAcceleratorTimer();;
	accel_cfft_wrapper(adi_acc_input, adi_acc_output, adi_acc_n);;
	StopAcceleratorTimer();;
	for (int i4 = 0; i4 < adi_acc_n; i4++) {
		x[i4].real = adi_acc_output[i4].re;
	};
	for (int i5 = 0; i5 < adi_acc_n; i5++) {
		x[i5].imag = adi_acc_output[i5].im;
	};
	
;
	
;
	
;
	
if (GREATER_THAN(n, -1)) {
ARRAY_NORM_POSTIND(x, real, n);;
	ARRAY_NORM_POSTIND(x, imag, n);
} else {
;
}
} else {
fft(x, n);
}
}
void fft_accel(complex_t * x, int n) {
fft_accel_internal((complex_t *) x, (int) n);
}
int main(int argc, char **argv) {
    char *inpname = argv[1]; 
    output_file = argv[2]; 

    std::ifstream ifs(inpname); 
    json input_json = json::parse(ifs);
std::vector<complex_t> x_vec;
for (auto& elem : input_json["x"]) {
float x_innerreal = elem["real"];
float x_innerimag = elem["imag"];
complex_t x_inner = { x_innerreal, x_innerimag};
x_vec.push_back(x_inner);
}
complex_t *x = &x_vec[0];
int n = input_json["n"];
clock_t begin = clock();
fft_accel(x, n);
clock_t end = clock();
std::cout << "Time: " << (double) (end - begin) / CLOCKS_PER_SEC << std::endl;
std::cout << "AccTime: " << (double) AcceleratorTotalNanos / CLOCKS_PER_SEC << std::endl;
write_output(x, n);
}
