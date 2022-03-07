 #define t_float float
/* fft64.c  complex data stored  re, im, re, im, re, ... im */

void fft64(float Z[128]) /* input data points and output  [0] to [127] */
{
  static float W[128];    /* scratch vector, used many times, */
  static float E[66] =    /* constants for FFT algorithm */
    {1.0,      0.0,       0.995185,  0.0980171,
     0.980785, 0.19509,   0.95694,   0.290285,
     0.92388,  0.382683,  0.881921,  0.471397,
     0.83147,  0.55557,   0.77301,   0.634393,
     0.707107, 0.707107,  0.634393,  0.77301,
     0.55557,  0.83147,   0.471397,  0.881921,
     0.382683, 0.92388,   0.290285,  0.95694,
     0.19509,  0.980785,  0.0980171, 0.995185,
     0.0,      1.0,      -0.0980171, 0.995185,
    -0.19509,  0.980785, -0.290285,  0.95694,
    -0.382683, 0.92388,  -0.471397,  0.881921,
    -0.55557,  0.83147,  -0.634393,  0.77301,
    -0.707107, 0.707107, -0.77301,   0.634393,
    -0.83147,  0.55557,  -0.881921,  0.471397,
    -0.92388,  0.382683, -0.95694,   0.290285,
    -0.980785, 0.19509,  -0.995185,  0.0980171,
    -1.0, 0.0};
  float Tre, Tim;
  int i, j, k, l, m;

   m = 32;
   l = 1;
   while(1)
   { 
      k = 0;
      j = l;
      i = 0;
      while(1)
      { 
         while(1)
	 {
	   /* W[i+k] = Z[i] + Z[m+i]; complex */
           W[2*(i+k)]   = Z[2*i]   + Z[2*(m+i)];
           W[2*(i+k)+1] = Z[2*i+1] + Z[2*(m+i)+1];

           /* W[i+j] = E[k] * (Z[i] - Z[m+i]); complex */
           Tre = Z[2*i]   - Z[2*(m+i)];
           Tim = Z[2*i+1] - Z[2*(m+i)+1];
           W[2*(i+j)]   = E[2*k] * Tre - E[2*k+1] * Tim;
           W[2*(i+j)+1] = E[2*k] * Tim + E[2*k+1] * Tre; 
           i++;
           if(i >= j) break;
	 }
         k = j;
         j = k+l;
         if(j > m) break;
      }
      l = l+l;
                  /* work back other way without copying */
      k = 0;
      j = l;
      i = 0;
      while(1)
      {
        while(1)
	{
          /* Z[i+k] = W[i] + W[m+i]; complex */
          Z[2*(i+k)]   = W[2*i]   + W[2*(m+i)];
          Z[2*(i+k)+1] = W[2*i+1] + W[2*(m+i)+1];

          /* Z[i+j] = E[k] * (W[i] - W[m+i]); complex */
          Tre = W[2*i]   - W[2*(m+i)];
          Tim = W[2*i+1] - W[2*(m+i)+1];
          Z[2*(i+j)]   = E[2*k] * Tre - E[2*k+1] * Tim;
          Z[2*(i+j)+1] = E[2*k] * Tim + E[2*k+1] * Tre;
          i++;
          if(i >= j) break;
	}
        k = j;
        j = k+l;
        if(j > m) break;
      }
      l = l+l;
      if(l > m) break; // result is in Z
   }
} /* end fft64 */
