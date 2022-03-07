/* Orignal skeleton is: 
Pre: SKELETON:

With the array index wrappers adi_acc_output,Annon
And (fromvars) []
Under dimensions [adi_acc_n = NumSamples (=) ]
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers adi_acc_n
And (fromvars) [NumSamples]
Under dimensions []
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers adi_acc_input,re
And (fromvars) [ImagIn, Annon]
Under dimensions [adi_acc_n = NumSamples (=) ]
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers adi_acc_input,im
And (fromvars) [RealIn, Annon]
Under dimensions [adi_acc_n = NumSamples (=) ]
With conversion function IdentityConversion
Post: SKELETON:

With the array index wrappers ImagOut,Annon
And (fromvars) [adi_acc_output, re]
Under dimensions [adi_acc_n = NumSamples (=) ]
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers RealOut,Annon
And (fromvars) [adi_acc_output, im]
Under dimensions [adi_acc_n = NumSamples (=) ]
With conversion function IdentityConversion
*/

/* Typemap is :
 i7: int32
ImagOut: array(float32: with dims NumSamples (=) )
RealIn: array(float32: with dims NumSamples (=) )
adi_acc_n: int32
i8: int32
adi_acc_input: array(complex_float: with dims adi_acc_n (=) )
InverseTransform: int32
i9: int32
i10: int32
NumSamples: int32
adi_acc_output: array(complex_float: with dims adi_acc_n (=) )
RealOut: array(float32: with dims NumSamples (=) )
ImagIn: array(float32: with dims NumSamples (=) )
*/


#include "../../benchmarks/Suites/MiBench/MiBench.cpp"


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

void write_output(int NumSamples, int InverseTransform, float * RealIn, float * ImagIn, float * RealOut, float * ImagOut) {

    json output_json;
std::vector<json> output_temp_51;
for (unsigned int i52 = 0; i52 < NumSamples; i52++) {
float output_temp_53 = RealOut[i52];

output_temp_51.push_back(output_temp_53);
}
output_json["RealOut"] = output_temp_51;
std::vector<json> output_temp_54;
for (unsigned int i55 = 0; i55 < NumSamples; i55++) {
float output_temp_56 = ImagOut[i55];

output_temp_54.push_back(output_temp_56);
}
output_json["ImagOut"] = output_temp_54;
std::ofstream out_str(output_file); 
out_str << std::setw(4) << output_json << std::endl;
}

void fft_float_accel_internal(int NumSamples,int InverseTransform,float * RealIn,float * ImagIn,float * RealOut,float * ImagOut) {

if ((PRIM_EQUAL(InverseTransform, 0)) && ((PRIM_EQUAL(NumSamples, 16384)) || ((PRIM_EQUAL(NumSamples, 8192)) || ((PRIM_EQUAL(NumSamples, 4096)) || ((PRIM_EQUAL(NumSamples, 2048)) || ((PRIM_EQUAL(NumSamples, 1024)) || ((PRIM_EQUAL(NumSamples, 512)) || ((PRIM_EQUAL(NumSamples, 256)) || ((PRIM_EQUAL(NumSamples, 128)) || (PRIM_EQUAL(NumSamples, 64))))))))))) {
static complex_float adi_acc_output[16384]__attribute__((__aligned__(64)));
for (int i49 = 0; i49++; i49 < 16384) {
static complex_float adi_acc_output_sub_element;

;
adi_acc_output[i49] = adi_acc_output_sub_element;
};
	static int adi_acc_n;;
	adi_acc_n = NumSamples;;
	static complex_float adi_acc_input[16384]__attribute__((__aligned__(64)));
for (int i50 = 0; i50++; i50 < 16384) {
static complex_float adi_acc_input_sub_element;

;
adi_acc_input[i50] = adi_acc_input_sub_element;
};
	for (int i7 = 0; i7 < adi_acc_n; i7++) {
		adi_acc_input[i7].re = ImagIn[i7];
	};
	for (int i8 = 0; i8 < adi_acc_n; i8++) {
		adi_acc_input[i8].im = RealIn[i8];
	};
	StartAcceleratorTimer();;
	accel_cfft_wrapper(adi_acc_input, adi_acc_output, adi_acc_n);;
	StopAcceleratorTimer();;
	for (int i9 = 0; i9 < adi_acc_n; i9++) {
		ImagOut[i9] = adi_acc_output[i9].re;
	};
	for (int i10 = 0; i10 < adi_acc_n; i10++) {
		RealOut[i10] = adi_acc_output[i10].im;
	};
	
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
