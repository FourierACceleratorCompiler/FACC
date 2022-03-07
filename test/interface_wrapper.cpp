#include<vector>
#include<nlohmann/json.hpp>
#include<fstream>
#include<iomanip>
#include "cmplx.hpp"
#include<complex.h>

typedef double complex cplx;
double PI = 3.1415926;

using json = nlohmann::json;

#include <fftw3.h>

void fft(

int main(int argc, char **argv) {
    char *inpname = argv[1]; 
    char *outname = argv[2]; 
    std::ifstream ifs(inpname); 
    json input_json = json::parse(ifs);
std::vector<Complex> cpx_vec;
for (auto& elem : input_json["cpx"]) {
float cpx_innerre = elem["re"];
float cpx_innerim = elem["im"];
Complex cpx_inner = { cpx_innerre, cpx_innerim};
cpx_vec.push_back(cpx_inner);
}
Complex *cpx = &cpx_vec[0];
short len = input_json["len"];
fft(cpx, len);
    json output_json;
std::vector<json> output_temp_1;
for (unsigned int i2 = 0; i2 < len; i2++) {
Complex output_temp_3 = cpx[i2];
json output_temp_4;

output_temp_4["re"] = output_temp_3.re;

output_temp_4["im"] = output_temp_3.im;
output_temp_1.push_back(output_temp_4);
}
output_json["cpx"] = output_temp_1;
std::ofstream out_str(outname); 
out_str << std::setw(4) << output_json << std::endl;
}
