/* Orignal skeleton is: 
Pre: SKELETON:

With the array index wrappers adi_acc_output,Annon
And (fromvars) []
Under dimensions [adi_acc_n = nfft]
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers adi_acc_n
And (fromvars) [nfft]
Under dimensions []
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers adi_acc_input,re
And (fromvars) [fin, r]
Under dimensions [adi_acc_n = nfft]
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers adi_acc_input,im
And (fromvars) [fin, i]
Under dimensions [adi_acc_n = nfft]
With conversion function IdentityConversion
Post: SKELETON:

With the array index wrappers fout,r
And (fromvars) [adi_acc_output, re]
Under dimensions [nfft = adi_acc_n]
With conversion function IdentityConversion

>(new binding): 

With the array index wrappers fout,i
And (fromvars) [adi_acc_output, im]
Under dimensions [nfft = adi_acc_n]
With conversion function IdentityConversion
*/


extern "C" {
#include "../../benchmarks/Github/code/mborgerding_kissfft/self_contained_code.c"
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

if ((PRIM_EQUAL(nfft, 16384)) || ((PRIM_EQUAL(nfft, 8192)) || ((PRIM_EQUAL(nfft, 4096)) || ((PRIM_EQUAL(nfft, 2048)) || ((PRIM_EQUAL(nfft, 1024)) || ((PRIM_EQUAL(nfft, 512)) || ((PRIM_EQUAL(nfft, 256)) || ((PRIM_EQUAL(nfft, 128)) || ((PRIM_EQUAL(nfft, 64)) || ((PRIM_EQUAL(nfft, 32)) || ((PRIM_EQUAL(nfft, 16)) || ((PRIM_EQUAL(nfft, 8)) || ((PRIM_EQUAL(nfft, 4)) || ((PRIM_EQUAL(nfft, 2)) || (PRIM_EQUAL(nfft, 1)))))))))))))))) {
static complex_float adi_acc_output[16384]__attribute__((__aligned__(64)));;
	static int adi_acc_n;;
	adi_acc_n = nfft;;
	static complex_float adi_acc_input[16384]__attribute__((__aligned__(64)));;
	for (int i2 = 0; i2 < adi_acc_n; i2++) {
		adi_acc_input[i2].re = fin[i2].r;
	};
	for (int i3 = 0; i3 < adi_acc_n; i3++) {
		adi_acc_input[i3].im = fin[i3].i;
	};
	StartAcceleratorTimer();;
	accel_cfft_wrapper(adi_acc_input, adi_acc_output, adi_acc_n);;
	StopAcceleratorTimer();;
	for (int i4 = 0; i4 < nfft; i4++) {
		fout[i4].r = adi_acc_output[i4].re;
	};
	for (int i5 = 0; i5 < nfft; i5++) {
		fout[i5].i = adi_acc_output[i5].im;
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