#include<vector>
#include<nlohmann/json.hpp>
#include<fstream>
#include "ddcmath.h"
#include "fourier.h"
#include<iomanip>
#include<iostream>
#include<chrono>
#include<time.h>
char *pre_accel_dump_file;
using json = nlohmann::json;
int main(int argc, char **argv) {
    char *inpname = argv[1]; 
    char *outname = argv[2]; 
	pre_accel_dump_file = argv[3];
    std::ifstream ifs(inpname); 
    json input_json = json::parse(ifs);
int NumSamples = input_json["NumSamples"];
int InverseTransform = input_json["InverseTransform"];
std::vector<float> RealIn_vec;
for (auto& elem : input_json["RealIn"]) {
float RealIn_inner = elem;
RealIn_vec.push_back(RealIn_inner);
}
float *RealIn = &RealIn_vec[0];
std::vector<float> ImagIn_vec;
for (auto& elem : input_json["ImagIn"]) {
float ImagIn_inner = elem;
ImagIn_vec.push_back(ImagIn_inner);
}
float *ImagIn = ImagIn_vec.data();
std::vector<float> RealOut_vec;
for (auto& elem : input_json["RealIn"]) {
float RealOut_inner = elem;
RealOut_vec.push_back(RealOut_inner);
}
float *RealOut = RealOut_vec.data();
std::vector<float> ImagOut_vec;
for (auto& elem : input_json["ImagIn"]) {
float ImagOut_inner = elem;
ImagOut_vec.push_back(ImagOut_inner);
}
std::cout << std::endl;
float *ImagOut = ImagOut_vec.data();
    using std::chrono::high_resolution_clock;
    using std::chrono::duration_cast;
    using std::chrono::duration;
    using std::chrono::milliseconds;

clock_t begin = clock();
for (int i = 0; i < TIMES; i ++) {
	fft_float(NumSamples, InverseTransform, RealIn, ImagIn, RealOut, ImagOut);
}
clock_t end = clock();
std::cout << "Time: " << (double) (end - begin) / CLOCKS_PER_SEC << std::endl;
std::cout << "AccTime: " << (double) 0 / CLOCKS_PER_SEC << std::endl;

    json output_json;
std::vector<json> output_temp_1;
for (unsigned int i2 = 0; i2 < NumSamples; i2++) {
float output_temp_3 = RealOut[i2];

output_temp_1.push_back(output_temp_3);
}
output_json["RealOut"] = output_temp_1;
std::vector<json> output_temp_4;
// std::cout << "Imag out is " << std::endl;
for (unsigned int i5 = 0; i5 < NumSamples; i5++) {
float output_temp_6 = ImagOut[i5];
	// std::cout << output_temp_6 << ",";

output_temp_4.push_back(output_temp_6);
}
std::cout << std::endl;
output_json["ImagOut"] = output_temp_4;
std::ofstream out_str(outname); 
out_str << std::setw(4) << output_json << std::endl;
}

/*============================================================================

    fourierf.c  -  Don Cross <dcross@intersrv.com>

    http://www.intersrv.com/~dcross/fft.html

    Contains definitions for doing Fourier transforms
    and inverse Fourier transforms.

    This module performs operations on arrays of 'float'.

    Revision history:

1998 September 19 [Don Cross]
    Updated coding standards.
    Improved efficiency of trig calculations.

============================================================================*/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#define CHECKPOINTER(p)  CheckPointer(p,#p)

static void CheckPointer ( void *p, char *name )
{
    if ( p == NULL )
    {
        fprintf ( stderr, "Error in fft_float():  %s == NULL\n", name );
        exit(1);
    }
}


void fft_float (
    unsigned  NumSamples,
    int       InverseTransform,
    float    *RealIn,
    float    *ImagIn,
    float    *RealOut,
    float    *ImagOut )
{
    unsigned NumBits;    /* Number of bits needed to store indices */
    unsigned i, j, k, n;
    unsigned BlockSize, BlockEnd;

    double angle_numerator = 2.0 * DDC_PI;
    double tr, ti;     /* temp real, temp imaginary */

    if ( !IsPowerOfTwo(NumSamples) )
    {
        fprintf (
            stderr,
            "Error in fft():  NumSamples=%u is not power of two\n",
            NumSamples );

        exit(1);
    }

    if ( InverseTransform )
        angle_numerator = -angle_numerator;

    CHECKPOINTER ( RealIn );
    CHECKPOINTER ( RealOut );
    CHECKPOINTER ( ImagOut );

    NumBits = NumberOfBitsNeeded ( NumSamples );

    /*
    **   Do simultaneous data copy and bit-reversal ordering into outputs...
    */

    for ( i=0; i < NumSamples; i++ )
    {
        j = ReverseBits ( i, NumBits );
        RealOut[j] = RealIn[i];
        ImagOut[j] = (ImagIn == NULL) ? 0.0 : ImagIn[i];
    }

    /*
    **   Do the FFT itself...
    */

    BlockEnd = 1;
    for ( BlockSize = 2; BlockSize <= NumSamples; BlockSize <<= 1 )
    {
        double delta_angle = angle_numerator / (double)BlockSize;
        double sm2 = sin ( -2 * delta_angle );
        double sm1 = sin ( -delta_angle );
        double cm2 = cos ( -2 * delta_angle );
        double cm1 = cos ( -delta_angle );
        double w = 2 * cm1;
        double ar[3], ai[3];
        double temp;

        for ( i=0; i < NumSamples; i += BlockSize )
        {
            ar[2] = cm2;
            ar[1] = cm1;

            ai[2] = sm2;
            ai[1] = sm1;

            for ( j=i, n=0; n < BlockEnd; j++, n++ )
            {
                ar[0] = w*ar[1] - ar[2];
                ar[2] = ar[1];
                ar[1] = ar[0];

                ai[0] = w*ai[1] - ai[2];
                ai[2] = ai[1];
                ai[1] = ai[0];

                k = j + BlockEnd;
                tr = ar[0]*RealOut[k] - ai[0]*ImagOut[k];
                ti = ar[0]*ImagOut[k] + ai[0]*RealOut[k];

                RealOut[k] = RealOut[j] - tr;
                ImagOut[k] = ImagOut[j] - ti;

                RealOut[j] += tr;
                ImagOut[j] += ti;
            }
        }

        BlockEnd = BlockSize;
    }

    /*
    **   Need to normalize if inverse transform...
    */

    if ( InverseTransform )
    {
        double denom = (double)NumSamples;

        for ( i=0; i < NumSamples; i++ )
        {
            RealOut[i] /= denom;
            ImagOut[i] /= denom;
        }
    }
}


/*--- end of file fourierf.c ---*/
