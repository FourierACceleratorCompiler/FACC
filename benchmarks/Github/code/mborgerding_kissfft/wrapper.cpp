#include<vector>
#include<nlohmann/json.hpp>
#include<fstream>
#include<iomanip>
#include<clib/synthesizer.h>
#include<chrono>
#include<iostream>
extern "C" {
#include "self_contained_code.c"
}
char *output_file; 
char *pre_accel_dump_file; // optional dump file. 
using json = nlohmann::json;
void write_output(int nfft, kiss_fft_cpx * fin, kiss_fft_cpx * fout) {

    json output_json;
std::vector<json> output_temp_1;
for (unsigned int i2 = 0; i2 < nfft; i2++) {
kiss_fft_cpx output_temp_3 = fout[i2];
json output_temp_4;

output_temp_4["r"] = output_temp_3.r;

output_temp_4["i"] = output_temp_3.i;
output_temp_1.push_back(output_temp_4);
}
output_json["fout"] = output_temp_1;
std::ofstream out_str(output_file); 
out_str << std::setw(4) << output_json << std::endl;
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
	kfc_fft(nfft, fin, fout);
}
clock_t end = clock();
std::cout << "Time: " << (double) (end - begin) / CLOCKS_PER_SEC << std::endl;
std::cout << "AccTime: 0" << std::endl;
write_output(nfft, fin, fout);
}
