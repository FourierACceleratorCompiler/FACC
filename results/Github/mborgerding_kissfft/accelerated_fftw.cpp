
#include "../../benchmarks/Github/code/mborgerding_kissfft/self_contained_code.c"


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
double AcceleratorTotalNanos = 0;
void StartAcceleratorTimer() {
	AcceleratorStart = clock();
}

void StopAcceleratorTimer() {
	AcceleratorTotalNanos +=
		(double) ((clock()) - AcceleratorStart) / CLOCKS_PER_SEC;
}

void write_output(int nfft, kiss_fft_cpx * fin, kiss_fft_cpx * fout) {

    json output_json;
std::vector<json> output_temp_65;
for (unsigned int i66 = 0; i66 < nfft; i66++) {
kiss_fft_cpx output_temp_67 = fout[i66];
json output_temp_68;

output_temp_68["r"] = output_temp_67.r;

output_temp_68["i"] = output_temp_67.i;
output_temp_65.push_back(output_temp_68);
}
output_json["fout"] = output_temp_65;
std::ofstream out_str(output_file); 
out_str << std::setw(4) << output_json << std::endl;
}

void kfc_fft_accel_internal(int nfft,kiss_fft_cpx * fin,kiss_fft_cpx * fout) {
short dir;;
	dir = -1;;
	int interface_len;;
	interface_len = nfft;;
	_complex_float_ api_out[interface_len];;
	_complex_float_ api_in[interface_len];;
	for (int i2 = 0; i2 < interface_len; i2++) {
		api_in[i2].re = fin[i2].r;
	};
	for (int i3 = 0; i3 < interface_len; i3++) {
		api_in[i3].im = fin[i3].i;
	};
	
if ((PRIM_EQUAL(dir, -1)) && ((PRIM_EQUAL(interface_len, 524288)) || ((PRIM_EQUAL(interface_len, 262144)) || ((PRIM_EQUAL(interface_len, 131072)) || ((PRIM_EQUAL(interface_len, 65536)) || ((PRIM_EQUAL(interface_len, 32768)) || ((PRIM_EQUAL(interface_len, 16384)) || ((PRIM_EQUAL(interface_len, 8192)) || ((PRIM_EQUAL(interface_len, 4096)) || ((PRIM_EQUAL(interface_len, 2048)) || ((PRIM_EQUAL(interface_len, 1024)) || ((PRIM_EQUAL(interface_len, 512)) || ((PRIM_EQUAL(interface_len, 256)) || ((PRIM_EQUAL(interface_len, 128)) || ((PRIM_EQUAL(interface_len, 64)) || ((PRIM_EQUAL(interface_len, 32)) || ((PRIM_EQUAL(interface_len, 16)) || ((PRIM_EQUAL(interface_len, 8)) || ((PRIM_EQUAL(interface_len, 4)) || ((PRIM_EQUAL(interface_len, 2)) || (PRIM_EQUAL(interface_len, 1)))))))))))))))))))))) {
StartAcceleratorTimer();;
	fftwf_example_api(api_in, api_out, interface_len, dir);;
	StopAcceleratorTimer();;
	for (int i4 = 0; i4 < nfft; i4++) {
		fout[i4].r = api_out[i4].re;
	};
	for (int i5 = 0; i5 < nfft; i5++) {
		fout[i5].i = api_out[i5].im;
	}
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
for (int i = 0; i < TIMES; i ++) {
	kfc_fft_accel(nfft, fin, fout);
}
clock_t end = clock();
std::cout << "Time: " << (double) (end - begin) / CLOCKS_PER_SEC << std::endl;
std::cout << "AccTime: " << AcceleratorTotalNanos << std::endl;
write_output(nfft, fin, fout);
}
