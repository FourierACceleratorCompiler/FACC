#include <math.h>
#include <stdint.h>
typedef struct {
    float real;
	float imag;
} COMPLEX;
typedef unsigned int                      uint32_t;
#define TYPE_FFT_E     float    /* Type is the same with COMPLEX member */     
typedef COMPLEX TYPE_FFT;  /* Define COMPLEX in Config.h */

int ones_32(uint32_t n)
{
    unsigned int c =0 ;
    for (c = 0; n; ++c)
    {
        n &= (n -1) ; // 清除最低位的1
    }
    return c ;
}
uint32_t floor_log2_32(uint32_t x)
{
    x |= (x>>1);
    x |= (x>>2);
    x |= (x>>4);
    x |= (x>>8);
    x |= (x>>16);

    return (ones_32(x>>1));
}
const float sin_tb[] = {  // 精度(PI PI/2 PI/4 PI/8 PI/16 ... PI/(2^k))
0.000000, 1.000000, 0.707107, 0.382683, 0.195090, 0.098017, 
0.049068, 0.024541, 0.012272, 0.006136, 0.003068, 0.001534, 
0.000767, 0.000383, 0.000192, 0.000096, 0.000048, 0.000024, 
0.000012, 0.000006, 0.000003 
};


const float cos_tb[] = {  // 精度(PI PI/2 PI/4 PI/8 PI/16 ... PI/(2^k))
-1.000000, 0.000000, 0.707107, 0.923880, 0.980785, 0.995185, 
0.998795, 0.999699, 0.999925, 0.999981, 0.999995, 0.999999, 
1.000000, 1.000000, 1.000000, 1.000000 , 1.000000, 1.000000, 
1.000000, 1.000000, 1.000000
};
int fft(TYPE_FFT *x, uint32_t N)
{
	int i,j,l,k,ip;
	static uint32_t M = 0;
	static int le,le2;
	static TYPE_FFT_E sR,sI,tR,tI,uR,uI;

	M = floor_log2_32(N);

	/*
	 * bit reversal sorting
	 */
	l = N >> 1;
	j = l;
    ip = N-2;
    for (i=1; i<=ip; i++) {
        if (i < j) {
            tR = x[j].real;
			tI = x[j].imag;
            x[j].real = x[i].real;
			x[j].imag = x[i].imag;
            x[i].real = tR;
			x[i].imag = tI;
		}
		k = l;
		while (k <= j) {
            j = j - k;
			k = k >> 1;
		}
		j = j + k;
	}

	/*
	 * For Loops
	 */
	for (l=1; l<=M; l++) {   /* loop for ceil{log2(N)} */
		//le = (int)pow(2,l);
		le  = (int)(1 << l);
		le2 = (int)(le >> 1);
		uR = 1;
		uI = 0;

        k = floor_log2_32(le2);
        sR = cos_tb[k]; //cos(PI / le2);
        sI = -sin_tb[k];  // -sin(PI / le2)
		for (j=1; j<=le2; j++) {   /* loop for each sub DFT */
			//jm1 = j - 1;
			for (i=j-1; i<N; i+=le) {  /* loop for each butterfly */
				ip = i + le2;
				tR = x[ip].real * uR - x[ip].imag * uI;
				tI = x[ip].real * uI + x[ip].imag * uR;
				x[ip].real = x[i].real - tR;
				x[ip].imag = x[i].imag - tI;
				x[i].real += tR;
				x[i].imag += tI;
			}  /* Next i */
			tR = uR;
			uR = tR * sR - uI * sI;
			uI = tR * sI + uI *sR;
		} /* Next j */
	} /* Next l */

	return 0;
}
