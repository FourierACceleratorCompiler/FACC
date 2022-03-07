#include<vector>
#include<nlohmann/json.hpp>
#include<fstream>
#include<iomanip>
#include "1024/arrays/fft_1024.c"
char *output_file; 
char *pre_accel_dump_file; // optional dump file. 
using json = nlohmann::json;
void write_output(float * data_real, float * data_imag, float * coef_real, float * coef_imag) {

    json output_json;
std::vector<json> output_temp_1;
for (unsigned int i2 = 0; i2 < 1024; i2++) {
float output_temp_3 = data_real[i2];

output_temp_1.push_back(output_temp_3);
}
output_json["data_real"] = output_temp_1;
std::vector<json> output_temp_4;
for (unsigned int i5 = 0; i5 < 1024; i5++) {
float output_temp_6 = data_imag[i5];

output_temp_4.push_back(output_temp_6);
}
output_json["data_imag"] = output_temp_4;
std::ofstream out_str(output_file); 
out_str << std::setw(4) << output_json << std::endl;
}

int main(int argc, char **argv) {
    char *inpname = argv[1]; 
    output_file = argv[2]; 

    std::ifstream ifs(inpname); 
    json input_json = json::parse(ifs);
std::vector<float> data_real_vec;
for (auto& elem : input_json["data_real"]) {
float data_real_inner = elem;
data_real_vec.push_back(data_real_inner);
}
float *data_real = &data_real_vec[0];
std::vector<float> data_imag_vec;
for (auto& elem : input_json["data_imag"]) {
float data_imag_inner = elem;
data_imag_vec.push_back(data_imag_inner);
}
float *data_imag = &data_imag_vec[0];
std::vector<float> coef_real_vec;
for (auto& elem : input_json["coef_real"]) {
float coef_real_inner = elem;
coef_real_vec.push_back(coef_real_inner);
}
float *coef_real = &coef_real_vec[0];
std::vector<float> coef_imag_vec;
for (auto& elem : input_json["coef_imag"]) {
float coef_imag_inner = elem;
coef_imag_vec.push_back(coef_imag_inner);
}
float *coef_imag = &coef_imag_vec[0];
fft(data_real, data_imag, coef_real, coef_imag);
write_output(data_real, data_imag, coef_real, coef_imag);
}
