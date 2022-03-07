 #define t_float float
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
 #define TWOPI 2.0 * M_PI // don't know where this was meant to come from
 #define TWOPI 2.0 * M_PI // don't know where this was meant to come from

/* bitreverse places t_float array x containing N/2 complex values
   into bit-reversed order */

void fftease_bitreverse( t_float *x, int N )

{
  t_float   rtemp,itemp;
  int       i,j,
        m;

    for ( i = j = 0; i < N; i += 2, j += m ) {
    if ( j > i ) {
        rtemp = x[j]; itemp = x[j+1]; /* complex exchange */
        x[j] = x[i]; x[j+1] = x[i+1];
        x[i] = rtemp; x[i+1] = itemp;
    }
    for ( m = N>>1; m >= 2 && j >= m; m >>= 1 )
        j -= m;
    }
}
/* cfft replaces t_float array x containing NC complex values
   (2*NC t_float values alternating real, imagininary, etc.)
   by its Fourier transform if forward is true, or by its
   inverse Fourier transform if forward is false, using a
   recursive Fast Fourier transform method due to Danielson
   and Lanczos.  NC MUST be a power of 2. */

void fftease_cfft( t_float *x, int NC, int forward )

{
  t_float   wr,wi,
        wpr,wpi,
        theta,
        scale;
  int       mmax,
        ND,
        m,
        i,j,
        delta;

 ;

    ND = NC<<1;
    fftease_bitreverse( x, ND );
    for ( mmax = 2; mmax < ND; mmax = delta ) {
    delta = mmax<<1;
    theta = TWOPI/( forward? mmax : -mmax );
    wpr = -2.*pow( sin( 0.5*theta ), 2. );
    wpi = sin( theta );
    wr = 1.;
    wi = 0.;
    for ( m = 0; m < mmax; m += 2 ) {
     register t_float rtemp, itemp;
        for ( i = m; i < ND; i += delta ) {
        j = i + mmax;
        rtemp = wr*x[j] - wi*x[j+1];
        itemp = wr*x[j+1] + wi*x[j];
        x[j] = x[i] - rtemp;
        x[j+1] = x[i+1] - itemp;
        x[i] += rtemp;
        x[i+1] += itemp;
        }
        wr = (rtemp = wr)*wpr - wi*wpi + wr;
        wi = wi*wpr + rtemp*wpi + wi;
    }
    }

/* scale output */

    scale = forward ? 1./ND : 2.;
    { register t_float *xi=x, *xe=x+ND;
    while ( xi < xe )
        *xi++ *= scale;
    }
}
