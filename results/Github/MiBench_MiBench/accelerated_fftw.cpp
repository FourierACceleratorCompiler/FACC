/* Orignal skeleton is: 
Pre: SKELETON:

With the array index wrappers api_out,Annon
And (fromvars) []
Under dimensions [interface_len = NumSamples (=) ]
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers dir
And (fromvars) [Constant(-1)]
Under dimensions []
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers interface_len
And (fromvars) [NumSamples]
Under dimensions []
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers api_in,im
And (fromvars) [RealIn, Annon]
Under dimensions [interface_len = NumSamples (=) ]
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers api_in,re
And (fromvars) [ImagIn, Annon]
Under dimensions [interface_len = NumSamples (=) ]
With conversion function IdentityConversion
Post: SKELETON:

With the array index wrappers ImagOut,Annon
And (fromvars) [api_out, re]
Under dimensions [interface_len = NumSamples (=) ]
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers RealOut,Annon
And (fromvars) [api_out, im]
Under dimensions [interface_len = NumSamples (=) ]
With conversion function IdentityConversion
*/

/* Typemap is :
 dir: int16
i38: int32
ImagOut: array(float32: with dims NumSamples (=) )
api_in: array(_complex_float_: with dims interface_len (=) )
i37: int32
RealIn: array(float32: with dims NumSamples (=) )
InverseTransform: int32
interface_len: int32
i40: int32
i39: int32
api_out: array(_complex_float_: with dims interface_len (=) )
NumSamples: int32
RealOut: array(float32: with dims NumSamples (=) )
ImagIn: array(float32: with dims NumSamples (=) )
*/


#include "../../benchmarks/Suites/MiBench/MiBench.cpp"


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

void write_output(int NumSamples, int InverseTransform, float * RealIn, float * ImagIn, float * RealOut, float * ImagOut) {

    json output_json;
std::vector<json> output_temp_107;
for (unsigned int i108 = 0; i108 < NumSamples; i108++) {
float output_temp_109 = RealOut[i108];

output_temp_107.push_back(output_temp_109);
}
output_json["RealOut"] = output_temp_107;
std::vector<json> output_temp_110;
for (unsigned int i111 = 0; i111 < NumSamples; i111++) {
float output_temp_112 = ImagOut[i111];

output_temp_110.push_back(output_temp_112);
}
output_json["ImagOut"] = output_temp_110;
std::ofstream out_str(output_file); 
out_str << std::setw(4) << output_json << std::endl;
}

void fft_float_accel_internal(int NumSamples,int InverseTransform,float * RealIn,float * ImagIn,float * RealOut,float * ImagOut) {

if ((PRIM_EQUAL(InverseTransform, 0)) && ((PRIM_EQUAL(NumSamples, 16384)) || ((PRIM_EQUAL(NumSamples, 8192)) || ((PRIM_EQUAL(NumSamples, 4096)) || ((PRIM_EQUAL(NumSamples, 2048)) || ((PRIM_EQUAL(NumSamples, 1024)) || ((PRIM_EQUAL(NumSamples, 512)) || ((PRIM_EQUAL(NumSamples, 256)) || ((PRIM_EQUAL(NumSamples, 128)) || ((PRIM_EQUAL(NumSamples, 64)) || ((PRIM_EQUAL(NumSamples, 32)) || ((PRIM_EQUAL(NumSamples, 16)) || ((PRIM_EQUAL(NumSamples, 8)) || ((PRIM_EQUAL(NumSamples, 4)) || (PRIM_EQUAL(NumSamples, 2)))))))))))))))) {
short dir;;
	dir = -1;;
	int interface_len;;
	interface_len = NumSamples;;
	_complex_float_ api_out[interface_len];
for (int i105 = 0; i105++; i105 < interface_len) {
_complex_float_ api_out_sub_element;

;
api_out[i105] = api_out_sub_element;
};
	_complex_float_ api_in[interface_len];
for (int i106 = 0; i106++; i106 < interface_len) {
_complex_float_ api_in_sub_element;

;
api_in[i106] = api_in_sub_element;
};
	for (int i37 = 0; i37 < interface_len; i37++) {
		api_in[i37].im = RealIn[i37];
	};
	for (int i38 = 0; i38 < interface_len; i38++) {
		api_in[i38].re = ImagIn[i38];
	};
	StartAcceleratorTimer();;
	fftwf_example_api(api_in, api_out, interface_len, dir);;
	StopAcceleratorTimer();;
	for (int i39 = 0; i39 < interface_len; i39++) {
		ImagOut[i39] = api_out[i39].re;
	};
	for (int i40 = 0; i40 < interface_len; i40++) {
		RealOut[i40] = api_out[i40].im;
	};
	
;
	
;
	
;
	

} else {
fft_float(NumSamples, InverseTransform, RealIn, ImagIn, RealOut, ImagOut);
}
}
void fft_float_accel(int NumSamples, int InverseTransform, float * RealIn, float * ImagIn, float * RealOut, float * ImagOut) {
fft_float_accel_internal((int) NumSamples, (int) InverseTransform, (float *) RealIn, (float *) ImagIn, (float *) RealOut, (float *) ImagOut);
}
int main(int argc, char **argv) {
    char *inpname = argv[1]; 
    output_file = argv[2]; 

    std::ifstream ifs(inpname); 
    json input_json = json::parse(ifs);
int NumSamples = input_json["NumSamples"];
int InverseTransform = input_json["InverseTransform"];
std::vector<float> RealIn_vec;
for (auto& elem : input_json["RealIn"]) {
float RealIn_inner = elem;
RealIn_vec.push_back(RealIn_inner);
}
float *RealIn = &RealIn_vec[0];
std::vector<float> ImagIn_vec;
for (auto& elem : input_json["ImagIn"]) {
float ImagIn_inner = elem;
ImagIn_vec.push_back(ImagIn_inner);
}
float *ImagIn = &ImagIn_vec[0];
float RealOut[NumSamples];
float ImagOut[NumSamples];
clock_t begin = clock();
for (int i = 0; i < TIMES; i ++) {
	fft_float_accel(NumSamples, InverseTransform, RealIn, ImagIn, RealOut, ImagOut);
}
clock_t end = clock();
std::cout << "Time: " << (double) (end - begin) / CLOCKS_PER_SEC << std::endl;
std::cout << "AccTime: " << (double) AcceleratorTotalNanos / CLOCKS_PER_SEC << std::endl;
write_output(NumSamples, InverseTransform, RealIn, ImagIn, RealOut, ImagOut);
}
