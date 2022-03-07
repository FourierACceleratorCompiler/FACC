// FACC currently doesn't support appropriately-complex
// length specifiers to support this benchmark.  
// This is a wrapper that enables performance profiling before
// the implementation is complete.

void cdft_w(int n, int sign, double *a, int *ip, double *w) {
	cdft(n * 2, sign, a, ip, w);
}
