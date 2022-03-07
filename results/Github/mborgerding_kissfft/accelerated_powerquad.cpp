/* Orignal skeleton is: 
Pre: SKELETON:

With the array index wrappers power_quad_acc_output,Annon
And (fromvars) []
Under dimensions [power_quad_acc_n = nfft (=) ]
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers power_quad_acc_n
And (fromvars) [nfft]
Under dimensions []
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers power_quad_acc_input,im
And (fromvars) [fin, i]
Under dimensions [power_quad_acc_n = nfft (=) ]
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers power_quad_acc_input,re
And (fromvars) [fin, r]
Under dimensions [power_quad_acc_n = nfft (=) ]
With conversion function IdentityConversion
Post: SKELETON:

With the array index wrappers fout,r
And (fromvars) [power_quad_acc_output, re]
Under dimensions [power_quad_acc_n = nfft (=) ]
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers fout,i
And (fromvars) [power_quad_acc_output, im]
Under dimensions [power_quad_acc_n = nfft (=) ]
With conversion function IdentityConversion
*/

/* Typemap is :
 fout: array(kiss_fft_cpx: with dims nfft (=) )
power_quad_acc_n: int32
i14: int32
power_quad_acc_input: array(complex_type: with dims power_quad_acc_n (=) )
fin: array(kiss_fft_cpx: with dims nfft (=) )
i12: int32
i13: int32
i15: int32
power_quad_acc_output: array(complex_type: with dims power_quad_acc_n (=) )
nfft: int32
*/


extern "C" {
#include "../../benchmarks/Github/code/mborgerding_kissfft/self_contained_code.c"
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

void write_output(int nfft, kiss_fft_cpx * fin, kiss_fft_cpx * fout) {

    json output_json;
std::vector<json> output_temp_33;
for (unsigned int i34 = 0; i34 < nfft; i34++) {
kiss_fft_cpx output_temp_35 = fout[i34];
json output_temp_36;

output_temp_36["r"] = output_temp_35.r;

output_temp_36["i"] = output_temp_35.i;
output_temp_33.push_back(output_temp_36);
}
output_json["fout"] = output_temp_33;
std::ofstream out_str(output_file); 
out_str << std::setw(4) << output_json << std::endl;
}

void kfc_fft_accel_internal(int nfft,kiss_fft_cpx * fin,kiss_fft_cpx * fout) {

if ((PRIM_EQUAL(nfft, 16384)) || ((PRIM_EQUAL(nfft, 8192)) || ((PRIM_EQUAL(nfft, 4096)) || ((PRIM_EQUAL(nfft, 2048)) || ((PRIM_EQUAL(nfft, 1024)) || ((PRIM_EQUAL(nfft, 512)) || ((PRIM_EQUAL(nfft, 256)) || ((PRIM_EQUAL(nfft, 128)) || (PRIM_EQUAL(nfft, 64)))))))))) {
int power_quad_acc_n;;
	power_quad_acc_n = nfft;;
	complex_type power_quad_acc_output[power_quad_acc_n]__attribute__((__aligned__(64)));;
	complex_type power_quad_acc_input[power_quad_acc_n]__attribute__((__aligned__(64)));;
	for (int i12 = 0; i12 < power_quad_acc_n; i12++) {
		power_quad_acc_input[i12].im = fin[i12].i;
	};
	for (int i13 = 0; i13 < power_quad_acc_n; i13++) {
		power_quad_acc_input[i13].re = fin[i13].r;
	};
	StartAcceleratorTimer();;
	fft_api(power_quad_acc_input, power_quad_acc_output, power_quad_acc_n);;
	StopAcceleratorTimer();;
	for (int i14 = 0; i14 < power_quad_acc_n; i14++) {
		fout[i14].r = power_quad_acc_output[i14].re;
	};
	for (int i15 = 0; i15 < power_quad_acc_n; i15++) {
		fout[i15].i = power_quad_acc_output[i15].im;
	};
	
;
	
;
	

} else {
kfc_fft(nfft, fin, fout);
}
}
void kfc_fft_accel(int nfft, kiss_fft_cpx * fin, kiss_fft_cpx * fout) {
kfc_fft_accel_internal((int) nfft, (kiss_fft_cpx *) fin, (kiss_fft_cpx *) fout);
}
int main(int argc, char **argv) {
    char *inpname = argv[1]; 
    output_file = argv[2]; 

    std::ifstream ifs(inpname); 
    json input_json = json::parse(ifs);
int nfft = input_json["nfft"];
std::vector<kiss_fft_cpx> fin_vec;
for (auto& elem : input_json["fin"]) {
float fin_innerr = elem["r"];
float fin_inneri = elem["i"];
kiss_fft_cpx fin_inner = { fin_innerr, fin_inneri};
fin_vec.push_back(fin_inner);
}
kiss_fft_cpx *fin = &fin_vec[0];
kiss_fft_cpx fout[nfft];
clock_t begin = clock();
kfc_fft_accel(nfft, fin, fout);
clock_t end = clock();
std::cout << "Time: " << (double) (end - begin) / CLOCKS_PER_SEC << std::endl;
std::cout << "AccTime: " << (double) AcceleratorTotalNanos / CLOCKS_PER_SEC << std::endl;
write_output(nfft, fin, fout);
}