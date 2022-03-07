// See notes - this is to re-sugar the enum types without
// having to implement them in FACC>
// This is basically emulating what the compiler would do
// as it deconstructs the program.
typedef struct {
	int N;
	int Ncvec;
	int *ifac;
	int transform;
	v4sf *data;
	float *e;
	float *twiddle;
} PFFFT_Setup_Desugar;
