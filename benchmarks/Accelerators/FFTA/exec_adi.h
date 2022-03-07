// This is a fake header for the ADI components --- the idea is that code
// compiled using the defs here can just be copied into the proper
// analog devices environments and compile/work without issue :)
#include "adi_header_emulation.h"

complex_float exec_adi(const complex_float *in, complex_float *out, int n);
