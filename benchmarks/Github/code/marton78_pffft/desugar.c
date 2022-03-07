#include "self_contained_code.h"

void desugared_transform_ordered(PFFFT_Setup_Desugar *setup, const float *input, float *output, float *work, int direction) {
	struct PFFFT_Setup setup_struct = {
		setup->N, setup->Ncvec,
		setup->ifac[1],
		setup->ifac[2],
		setup->ifac[3],
		setup->ifac[4],
		setup->ifac[5],
		setup->ifac[6],
		setup->ifac[7],
		setup->ifac[8],
		setup->ifac[9],
		setup->ifac[10],
		setup->ifac[11],
		setup->ifac[12],
		setup->ifac[13],
		setup->ifac[14],
		setup->ifac[15],
		(setup->transform == 0 ? PFFFT_REAL : PFFFT_COMPLEX),
		setup->data, setup->e, setup->twiddle
	};

	pffft_transform_ordered(&setup_struct, input, output, work, (direction == 0 ? PFFFT_FORWARD : PFFFT_BACKWARD));
}

void desugar_setup(PFFFT_Setup *orig, PFFFT_Setup_Desugar *news) {
	news->N = orig->N;
	news->Ncvec = orig->Ncvec;
	news->ifac = orig->ifac;
	news->transform = (orig->transform == PFFFT_REAL ? 0 : 1);
	news->data = orig->data;
	news->e = orig->e;
	news->twiddle = orig->twiddle;
}
