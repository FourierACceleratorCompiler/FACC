#include<vector>
#include<nlohmann/json.hpp>
#include<fstream>
#include<iomanip>
#include<clib/synthesizer.h>
extern "C" {
#include "algo.h"
};
char *output_file; 
char *pre_accel_dump_file; // optional dump file. 
using json = nlohmann::json;
void write_output(short * InRealData, short * InImagData, short * OutRealData, short * OutImagData, short DataSizeExponent, short * SineV, short * CosineV, short * BitRevInd) {

    json output_json;
std::vector<json> output_temp_1;
for (unsigned int i2 = 0; i2 < 256; i2++) {
short output_temp_3 = OutRealData[i2];

output_temp_1.push_back(output_temp_3);
}
output_json["OutRealData"] = output_temp_1;
std::vector<json> output_temp_4;
for (unsigned int i5 = 0; i5 < 256; i5++) {
short output_temp_6 = OutImagData[i5];

output_temp_4.push_back(output_temp_6);
}
output_json["OutImagData"] = output_temp_4;
std::ofstream out_str(output_file); 
out_str << std::setw(4) << output_json << std::endl;
}

int main(int argc, char **argv) {
    char *inpname = argv[1]; 
    output_file = argv[2]; 

    std::ifstream ifs(inpname); 
    json input_json = json::parse(ifs);
std::vector<short> InRealData_vec;
for (auto& elem : input_json["InRealData"]) {
short InRealData_inner = elem;
InRealData_vec.push_back(InRealData_inner);
}
short *InRealData = &InRealData_vec[0];
std::vector<short> InImagData_vec;
for (auto& elem : input_json["InImagData"]) {
short InImagData_inner = elem;
InImagData_vec.push_back(InImagData_inner);
}
short *InImagData = &InImagData_vec[0];
short DataSizeExponent = input_json["DataSizeExponent"];
std::vector<short> SineV_vec;
for (auto& elem : input_json["SineV"]) {
short SineV_inner = elem;
SineV_vec.push_back(SineV_inner);
}
short *SineV = &SineV_vec[0];
std::vector<short> CosineV_vec;
for (auto& elem : input_json["CosineV"]) {
short CosineV_inner = elem;
CosineV_vec.push_back(CosineV_inner);
}
short *CosineV = &CosineV_vec[0];
std::vector<short> BitRevInd_vec;
for (auto& elem : input_json["BitRevInd"]) {
short BitRevInd_inner = elem;
BitRevInd_vec.push_back(BitRevInd_inner);
}
short *BitRevInd = &BitRevInd_vec[0];
short OutRealData[256];
short OutImagData[256];
fxpfft(InRealData, InImagData, OutRealData, OutImagData, DataSizeExponent, SineV, CosineV, BitRevInd);
write_output(InRealData, InImagData, OutRealData, OutImagData, DataSizeExponent, SineV, CosineV, BitRevInd);
}
