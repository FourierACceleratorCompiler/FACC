#include<vector>
#include<nlohmann/json.hpp>
#include<fstream>
#include<iomanip>
#include<complex>
#include "interface.hpp"

namespace std {
	using json = nlohmann::json;
	int main(int argc, char **argv) {
		char *inpname = argv[1]; 
		char *outname = argv[2]; 
		std::ifstream ifs(inpname); 
		json input_json = json::parse(ifs);
		std::vector<_complex_double_> api_in_vec;
		for (auto& elem : input_json["api_in"]) {
			double api_in_innerre = elem["re"];
			double api_in_innerim = elem["im"];
			_complex_double_ api_in_inner(api_in_innerre, api_in_innerim);
			api_in_vec.push_back(api_in_inner);
		}
		_complex_double_ *api_in = &api_in_vec[0];
		int interface_len = input_json["interface_len"];
		_complex_double_ api_out[interface_len];
		fftw_example_api(api_in, api_out, interface_len);
		json output_json;
		std::vector<json> output_temp_1;
		for (unsigned int i2 = 0; i2 < interface_len; i2++) {
			_complex_double_ output_temp_3 = api_out[i2];
			json output_temp_4;

			output_temp_4["re"] = output_temp_3.re;

			output_temp_4["im"] = output_temp_3.im;
			output_temp_1.push_back(output_temp_4);
		}
		output_json["api_out"] = output_temp_1;
		std::ofstream out_str(outname); 
		out_str << std::setw(4) << output_json << std::endl;

	}
}
