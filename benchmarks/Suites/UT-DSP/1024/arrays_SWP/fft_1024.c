/* 1024-point complex FFT (radix-2, in-place, decimation-in-time) */
/* Modified to use arrays - SMP */

#include "traps.h"

#define NPOINTS 1024
#define NSTAGES 10

float data_real[1024];
float data_imag[1024];
float coef_real[1024];
float coef_imag[1024];

void fft(float data_real[],float data_imag[], float coef_real[],
                 float coef_imag[]);

main()
{
  int i;

  input_dsp (data_real, NPOINTS, 0);

  for (i=0 ; i<NPOINTS ; i++){  /* was 256 - not ok? */
    data_imag[i] = 1;
    coef_real[i] = 1;
    coef_imag[i] = 1;
  }
  fft(data_real, data_imag, coef_real, coef_imag);

  output_dsp (data_real, NPOINTS, 0);
  output_dsp (data_imag, NPOINTS, 0);
  output_dsp (coef_real, NPOINTS, 0);
  output_dsp (coef_imag, NPOINTS, 0);

}

void fft(float data_real[],float data_imag[], float coef_real[],
                 float coef_imag[])
/* data_real:         real data points */
/* data_imag:         imaginary data points */
/* coef_real:         real coefficient points */
/* coef_imag:         imaginary coefficient points */
{
  int i;
  int j;
  int k;

  float temp_real;
  float temp_imag;
  float Ar;
  float Ai;
  float Br;
  float Bi;
  float Wr;
  float Wi;

  int groupsPerStage = 1;
  int buttersPerGroup = NPOINTS / 2;
  for (i = 0; i < NSTAGES; ++i) {
    for (j = 0; j < groupsPerStage; ++j) {
      Wr = coef_real[(1<<i)-1+j];
      Wi = coef_imag[(1<<i)-1+j];

      Ar = data_real[2*j*buttersPerGroup];
      Ai = data_imag[2*j*buttersPerGroup];
      Br = data_real[2*j*buttersPerGroup+buttersPerGroup];
      Bi = data_imag[2*j*buttersPerGroup+buttersPerGroup];
      temp_real = Wr * Br - Wi * Bi;
      temp_imag = Wi * Br + Wr * Bi;
      for (k = 1; k < buttersPerGroup; ++k) {
        data_real[2*j*buttersPerGroup+k-1] = Ar + temp_real;
        data_real[2*j*buttersPerGroup+buttersPerGroup+k-1] = Ar - temp_real;
        data_imag[2*j*buttersPerGroup+k-1] = Ai + temp_imag;
        data_imag[2*j*buttersPerGroup+buttersPerGroup+k-1] = Ai - temp_imag;

        Ar = data_real[2*j*buttersPerGroup+k];
        Ai = data_imag[2*j*buttersPerGroup+k];
        Br = data_real[2*j*buttersPerGroup+buttersPerGroup+k];
        Bi = data_imag[2*j*buttersPerGroup+buttersPerGroup+k];
        temp_real = Wr * Br - Wi * Bi;
        temp_imag = Wi * Br + Wr * Bi;
      } 
      data_real[2*j*buttersPerGroup+k-1] = Ar + temp_real;
      data_real[2*j*buttersPerGroup+buttersPerGroup+k-1] = Ar - temp_real;
      data_imag[2*j*buttersPerGroup+k-1] = Ai + temp_imag;
      data_imag[2*j*buttersPerGroup+buttersPerGroup+k-1] = Ai - temp_imag;
    }
    groupsPerStage <<= 1;
    buttersPerGroup >>= 1;
  }
}
