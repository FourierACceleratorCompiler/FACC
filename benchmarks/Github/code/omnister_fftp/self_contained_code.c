#include <math.h>
#include <stdio.h>
#include <stdlib.h>
typedef struct cplex {
   double re;
   double im;
} COMPLEX;
int snap2(int n) {	// snap to nearest power of two
    return (int)pow(2.0,(1.0+(int)(log((double)n-1.0)/log(2))));
}

double *new_double(int size) {
   return((double *)malloc(sizeof(double)*size));
}

COMPLEX *new_complex(int size) {
   return((COMPLEX *)malloc(sizeof(COMPLEX)*size));
}

// 1D Fast Fourier Transform (FFT).
// The array length must be a power of two
// from http://lux.vu/applets/frequency.pde

COMPLEX *fft_1d( COMPLEX *array, int n)
{
    double u_r, u_i, w_r, w_i, t_r, t_i;
    int ln, nv2, k, l, le, le1, j, ip, i;

    ln = (int) (log((double) n) / log(2.0) + 0.5);
    nv2 = n / 2;
    j = 1;
    for (i = 1; i < n; i++) {
	if (i < j) {
	    t_r = array[i-1].re;
	    t_i = array[i-1].im;
	    array[i-1].re = array[j-1].re;
	    array[i-1].im = array[j-1].im;
	    array[j-1].re = t_r;
	    array[j-1].im = t_i;
	}
	k = nv2;
	while (k < j) {
	    j = j-k;
	    k = k/2;
	}
	j = j + k;
    }

    for (l = 1; l <= ln; l++) {	/* loops thru stages */
	le = (int) (exp((double) l * log(2.0)) + 0.5);
	le1 = le / 2;
	u_r = 1.0;
	u_i = 0.0;
	w_r = cos(M_PI / (double) le1);
	w_i = -sin(M_PI / (double) le1);

	/* loops thru 1/2 twiddle values per stage */
	for (j = 1; j <= le1; j++) {	
	    /* loops thru points per 1/2 twiddle */
	    for (i = j; i <= n; i += le) {	
		ip = i + le1;
		t_r = array[ip-1].re * u_r - u_i * array[ip-1].im;
		t_i = array[ip-1].im * u_r + u_i * array[ip-1].re;

		array[ip-1].re = array[i-1].re - t_r;
		array[ip-1].im = array[i-1].im - t_i;

		array[i-1].re = array[i-1].re + t_r;
		array[i-1].im = array[i-1].im + t_i;
	    }
	    t_r = u_r * w_r - w_i * u_i;
	    u_i = w_r * u_i + w_i * u_r;
	    u_r = t_r;
	}
    }
    return array;
}				/* end of FFT_1d */
