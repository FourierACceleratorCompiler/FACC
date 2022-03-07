/* Orignal skeleton is: 
Pre: SKELETON:

With the array index wrappers power_quad_acc_output,Annon
And (fromvars) []
Under dimensions [power_quad_acc_n = NumSamples (=) ]
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers power_quad_acc_n
And (fromvars) [NumSamples]
Under dimensions []
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers power_quad_acc_input,im
And (fromvars) [RealIn, Annon]
Under dimensions [power_quad_acc_n = NumSamples (=) ]
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers power_quad_acc_input,re
And (fromvars) [ImagIn, Annon]
Under dimensions [power_quad_acc_n = NumSamples (=) ]
With conversion function IdentityConversion
Post: SKELETON:

With the array index wrappers ImagOut,Annon
And (fromvars) [power_quad_acc_output, re]
Under dimensions [power_quad_acc_n = NumSamples (=) ]
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers RealOut,Annon
And (fromvars) [power_quad_acc_output, im]
Under dimensions [power_quad_acc_n = NumSamples (=) ]
With conversion function IdentityConversion
*/

/* Typemap is :
 i18: int32
i17: int32
ImagOut: array(float32: with dims NumSamples (=) )
power_quad_acc_n: int32
RealIn: array(float32: with dims NumSamples (=) )
power_quad_acc_input: array(complex_type: with dims power_quad_acc_n (=) )
i20: int32
InverseTransform: int32
NumSamples: int32
RealOut: array(float32: with dims NumSamples (=) )
ImagIn: array(float32: with dims NumSamples (=) )
power_quad_acc_output: array(complex_type: with dims power_quad_acc_n (=) )
i19: int32
*/


#include "../../benchmarks/Suites/MiBench/MiBench.cpp"


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

if ((PRIM_EQUAL(InverseTransform, 0)) && ((PRIM_EQUAL(NumSamples, 2048)) || ((PRIM_EQUAL(NumSamples, 1024)) || ((PRIM_EQUAL(NumSamples, 512)) || ((PRIM_EQUAL(NumSamples, 256)) || ((PRIM_EQUAL(NumSamples, 128)) || (PRIM_EQUAL(NumSamples, 64)))))))) {
int power_quad_acc_n;;
	power_quad_acc_n = NumSamples;;
	complex_type power_quad_acc_output[power_quad_acc_n]__attribute__((__aligned__(64)));
for (int i49 = 0; i49++; i49 < power_quad_acc_n) {
complex_type power_quad_acc_output_sub_element;

;
power_quad_acc_output[i49] = power_quad_acc_output_sub_element;
};
	complex_type power_quad_acc_input[power_quad_acc_n]__attribute__((__aligned__(64)));
for (int i50 = 0; i50++; i50 < power_quad_acc_n) {
complex_type power_quad_acc_input_sub_element;

;
power_quad_acc_input[i50] = power_quad_acc_input_sub_element;
};
	for (int i17 = 0; i17 < power_quad_acc_n; i17++) {
		power_quad_acc_input[i17].im = RealIn[i17];
	};
	for (int i18 = 0; i18 < power_quad_acc_n; i18++) {
		power_quad_acc_input[i18].re = ImagIn[i18];
	};
	StartAcceleratorTimer();;
	fft_api(power_quad_acc_input, power_quad_acc_output, power_quad_acc_n);;
	StopAcceleratorTimer();;
	for (int i19 = 0; i19 < power_quad_acc_n; i19++) {
		ImagOut[i19] = power_quad_acc_output[i19].re;
	};
	for (int i20 = 0; i20 < power_quad_acc_n; i20++) {
		RealOut[i20] = power_quad_acc_output[i20].im;
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
