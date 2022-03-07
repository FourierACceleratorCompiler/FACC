/* 1024-point complex FFT (radix-2, in-place, decimation-in-time) */

#define NPOINTS 1024
#define NSTAGES 10

float data_real[1024];
float data_imag[1024];
float coef_real[1024];
float coef_imag[1024];

main()
{
  void fft();
  int i;
  int j=0;

  input_dsp (data_real, NPOINTS, 0);
 /* input_dsp (data_imag, NPOINTS, 0); */

  for (i=0 ; i<256 ; i++){
  *(data_imag + j) = 1;   
  *(coef_real + j) = 1;
  *(coef_imag + j) = 1;
  j++;
  }

  fft(data_real, data_imag, coef_real, coef_imag);
 
  output_dsp (data_real, NPOINTS, 0);
  output_dsp (data_imag, NPOINTS, 0);
  output_dsp (coef_real, NPOINTS, 0);
  output_dsp (coef_imag, NPOINTS, 0);

}

void fft(data_real, data_imag, coef_real, coef_imag)
float *data_real;		/* pointer to real data points */
float *data_imag;		/* pointer to imaginary data points */
float *coef_real;		/* pointer to real coefficient points */
float *coef_imag;		/* pointer to imaginary coefficient points */
{
  int i;
  int j;
  int k;
  float *A_real;
  float *A_imag;
  float *B_real;
  float *B_imag;
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
  for (i = 0; i < NSTAGES; i++) {
    A_real = data_real;
    A_imag = data_imag;
    B_real = data_real;
    B_real += buttersPerGroup;
    B_imag = data_imag;
    B_imag += buttersPerGroup;
    j = 0;
    do {
      Wr = *coef_real++;
      Wi = *coef_imag++;
      k = 0;
      do {
        Ar = *A_real;
        Ai = *A_imag;
        Br = *B_real;
        Bi = *B_imag;
        temp_real = Wr * Br - Wi * Bi;
        temp_imag = Wi * Br + Wr * Bi;
        *A_real++ = Ar + temp_real;
        *B_real++ = Ar - temp_real;
        *A_imag++ = Ai + temp_imag;
        *B_imag++ = Ai - temp_imag;
        k++;
      } while (k < buttersPerGroup);
      A_real += buttersPerGroup;
      A_imag += buttersPerGroup;
      B_real += buttersPerGroup;
      B_imag += buttersPerGroup;
      j++;
    } while (j < groupsPerStage);
    groupsPerStage <<= 1;
    buttersPerGroup >>= 1;
  }
}
