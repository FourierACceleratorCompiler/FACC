typedef struct Meow_FFT_Complex
{
    float r;
    float j;
}
Meow_FFT_Complex;
typedef const Meow_FFT_Complex Complex;
typedef struct Meow_Fft_Stages
{
    unsigned  count;
    unsigned* radix;
    unsigned* remainder;
    unsigned* offsets;
}
Meow_Fft_Stages;

typedef struct Meow_FFT_Workset
{
    int               N;

    Meow_FFT_Complex* wn;
    // Non-null only defined if there is a slow-dft as one of the radix stages.

    Meow_FFT_Complex* wn_ordered;
    // Sequentially ordered per stage, will be duplicates between stages.

    Meow_Fft_Stages   stages;
}
Meow_FFT_Workset;
inline Meow_FFT_Complex meow_add
(
      const Meow_FFT_Complex lhs
    , const Meow_FFT_Complex rhs
)
{
    Meow_FFT_Complex result =
    {
          lhs.r + rhs.r
        , lhs.j + rhs.j
    };

    return result;
}

inline Meow_FFT_Complex meow_sub
(
      const Meow_FFT_Complex lhs
    , const Meow_FFT_Complex rhs
)
{
    Meow_FFT_Complex result =
    {
          lhs.r - rhs.r
        , lhs.j - rhs.j
    };

    return result;
}

inline Meow_FFT_Complex meow_negate(const Meow_FFT_Complex lhs)
{
    Meow_FFT_Complex result =
    {
          -lhs.r
        , -lhs.j
    };

    return result;
}

inline Meow_FFT_Complex meow_conjugate(const Meow_FFT_Complex lhs)
{
    Meow_FFT_Complex result =
    {
           lhs.r
        , -lhs.j
    };

    return result;
}

inline Meow_FFT_Complex meow_mul
(
      const Meow_FFT_Complex lhs
    , const Meow_FFT_Complex rhs
)
{
    Meow_FFT_Complex result =
    {
          (lhs.r * rhs.r) - (lhs.j * rhs.j)
        , (lhs.r * rhs.j) + (lhs.j * rhs.r)
    };

    return result;
}

inline Meow_FFT_Complex meow_mul_by_conjugate
(
      const Meow_FFT_Complex lhs
    , const Meow_FFT_Complex rhs
)
{
    Meow_FFT_Complex result =
    {
          (lhs.r * rhs.r) + (lhs.j * rhs.j)
        , (lhs.j * rhs.r) - (lhs.r * rhs.j)
    };

    return result;
}

inline Meow_FFT_Complex meow_mul_by_j(const Meow_FFT_Complex lhs)
{
    Meow_FFT_Complex result =
    {
          -lhs.j
        ,  lhs.r
    };

    return result;
}

inline Meow_FFT_Complex meow_mulf
(
      const Meow_FFT_Complex lhs
    ,       float            rhs
)
{
    Meow_FFT_Complex result =
    {
          lhs.r * rhs
        , lhs.j * rhs
    };

    return result;
}
void meow_dft_n_dit
(
      const Meow_FFT_Complex* w_n
    , Meow_FFT_Complex*       out
    , unsigned                count
    , unsigned                w_multiplier
    , unsigned                radix
    , unsigned                N
    , unsigned                reverse
)
{
    // Can I do something with the knowledge that n is always odd?

    Meow_FFT_Complex scratch[radix];

    for (unsigned butterfly = 0; butterfly < count; ++butterfly)
    {
        for (unsigned i = 0; i < radix; i++)
        {
            scratch[i] = out[i * count + butterfly];
        }

        for (unsigned i = 0 ; i < radix ; ++i)
        {
            const unsigned  index_out = i * count + butterfly;

            // W0 is always 1
            Meow_FFT_Complex sum = scratch[0];

            for (unsigned j = 1; j < radix; ++j )
            {
                const unsigned wi = (j * w_multiplier * index_out) % N;
                Complex        w  = w_n[wi];
                Complex        in = scratch[j];

                float rr;
                float jj;

                if (reverse)
                {
                    rr = (in.r * w.r) - (in.j * w.j);
                    jj = (in.r * w.j) + (in.j * w.r);
                }
                else
                {
                    rr = (in.r * w.r) + (in.j * w.j);
                    jj = (in.j * w.r) - (in.r * w.j);
                }

                sum.r += rr;
                sum.j += jj;
            }

            out[index_out] = sum;
        }
    }
}

// -----------------------------------------------------------------------------

// Algorithms taken from
// http://www.briangough.com/fftalgorithms.pdf
// (equations 135 to 146)
// in, out and twiddle indicies taken from kiss_fft
// All twiddles are assumed to be ifft calculated. Conjugation is done in the
// maths.
// All twiddle input arrays are assumed to be sequentiall accessed. Twiddle
// indicies are pre-calculated.

// -----------------------------------------------------------------------------
// Forward
// -----------------------------------------------------------------------------

void meow_radix_2_dit
(
      const Meow_FFT_Complex* w_n
    , Meow_FFT_Complex*       out
    , unsigned                count
)
{
    // butteryfly 0 always has the twiddle factor == 1.0f
    // so special case that one.
    {
        Complex z0  = out[0];
        Complex z1  = out[count];

        out[0]     = meow_add(z0, z1);
        out[count] = meow_sub(z0, z1);
    }

    for (unsigned butterfly = 1; butterfly < count; ++butterfly)
    {
        Complex  w   = w_n[butterfly - 1];

        const unsigned i0  = butterfly;
        const unsigned i1  = butterfly + count;

        Complex z0  = out[i0];
        Complex z1  = meow_mul_by_conjugate(out[i1], w);

        out[i0] = meow_add(z0, z1);
        out[i1] = meow_sub(z0, z1);
        // Equation 135
    }
}

#define MEOW_SIN_PI_3 -0.866025403784438646763723170752936183471402626905190314f

void meow_radix_3_dit
(
      const Meow_FFT_Complex* w_n
    , Meow_FFT_Complex*       out
    , unsigned                count
)
{
    // W[0] is always 1.0f;
    {
        const unsigned i0  = 0 * count;
        const unsigned i1  = 1 * count;
        const unsigned i2  = 2 * count;

        Complex z0  = out[i0];
        Complex z1  = out[i1];
        Complex z2  = out[i2];

        Complex t1  = meow_add(z1, z2);
        Complex t2  = meow_sub(z0, meow_mulf(t1, 0.5));
        Complex t3j = meow_mul_by_j(meow_mulf(meow_sub(z1, z2), MEOW_SIN_PI_3));

        out[i0] = meow_add(z0, t1);
        out[i1] = meow_add(t2, t3j);
        out[i2] = meow_sub(t2, t3j);
    }

    unsigned wi = 0;
    for (unsigned butterfly = 1; butterfly < count; butterfly++, wi+=2)
    {
        Complex w1  = w_n[wi + 0];
        Complex w2  = w_n[wi + 1];

        const unsigned i0  = butterfly;
        const unsigned i1  = butterfly + count;
        const unsigned i2  = butterfly + 2 * count;

        Complex z0  = out[i0];
        Complex z1  = meow_mul_by_conjugate(out[i1], w1);
        Complex z2  = meow_mul_by_conjugate(out[i2], w2);

        Complex t1  = meow_add(z1, z2);
        Complex t2  = meow_sub(z0, meow_mulf(t1, 0.5));
        Complex t3j = meow_mul_by_j(meow_mulf(meow_sub(z1, z2), MEOW_SIN_PI_3));
        // Equation 136

        out[i0] = meow_add(z0, t1);
        out[i1] = meow_add(t2, t3j);
        out[i2] = meow_sub(t2, t3j);
        // Equation 137
    }
}

void meow_radix_4_dit
(
      const Meow_FFT_Complex* w_n
    , Meow_FFT_Complex*       out
    , unsigned                count
)
{
    // W[0] is always 1.0f;
    {
        const unsigned i0  = 0 * count;
        const unsigned i1  = 1 * count;
        const unsigned i2  = 2 * count;
        const unsigned i3  = 3 * count;

        Complex z0  = out[i0];
        Complex z1  = out[i1];
        Complex z2  = out[i2];
        Complex z3  = out[i3];

        Complex t1  = meow_add(z0, z2);
        Complex t2  = meow_add(z1, z3);
        Complex t3  = meow_sub(z0, z2);
        Complex t4j = meow_negate(meow_mul_by_j(meow_sub(z1, z3)));

        out[i0] = meow_add(t1, t2);
        out[i1] = meow_add(t3, t4j);
        out[i2] = meow_sub(t1, t2);
        out[i3] = meow_sub(t3, t4j);
    }

    unsigned wi = 0u;
    for (unsigned butterfly = 1; butterfly < count; ++butterfly, wi+=3)
    {
        Complex w1  = w_n[wi + 0];
        Complex w2  = w_n[wi + 1];
        Complex w3  = w_n[wi + 2];

        const unsigned i0  = butterfly + 0 * count;
        const unsigned i1  = butterfly + 1 * count;
        const unsigned i2  = butterfly + 2 * count;
        const unsigned i3  = butterfly + 3 * count;

        Complex z0  = out[i0];
        Complex z1  = meow_mul_by_conjugate(out[i1], w1);
        Complex z2  = meow_mul_by_conjugate(out[i2], w2);
        Complex z3  = meow_mul_by_conjugate(out[i3], w3);

        Complex t1  = meow_add(z0, z2);
        Complex t2  = meow_add(z1, z3);
        Complex t3  = meow_sub(z0, z2);
        Complex t4j = meow_negate(meow_mul_by_j(meow_sub(z1, z3)));
        // Equations 138
        // Also instead of conjugating the input and multplying with the
        // twiddles for the ifft, we invert the twiddles instead. This works
        // fine except here, the mul_by_j is assuming that it's the forward
        // fft twiddle we are multiplying with, not the conjugated one we
        // actually have. So we have to conjugate it _back_ if we are doing the
        // ifft.
        // Also, had to multiply by -j, not j for reasons I am yet to grasp.

        out[i0] = meow_add(t1, t2);
        out[i1] = meow_add(t3, t4j);
        out[i2] = meow_sub(t1, t2);
        out[i3] = meow_sub(t3, t4j);
        // Equations 139
    }
}

#define MEOW_SQR_5_DIV_4 0.5590169943749474241022934171828190588601545899028814f
#define MEOW_SIN_2PI_5  -0.9510565162951535721164393333793821434056986341257502f
#define MEOW_SIN_2PI_10 -0.5877852522924731291687059546390727685976524376431459f

void meow_radix_5_dit
(
      const Meow_FFT_Complex* w_n
    , Meow_FFT_Complex*       out
    , unsigned              count
)
{
    // W[0] is always 1.0f;
    {
        const unsigned i0   = 0 * count;
        const unsigned i1   = 1 * count;
        const unsigned i2   = 2 * count;
        const unsigned i3   = 3 * count;
        const unsigned i4   = 4 * count;

        Complex z0   = out[i0];
        Complex z1   = out[i1];
        Complex z2   = out[i2];
        Complex z3   = out[i3];
        Complex z4   = out[i4];

        Complex t1   = meow_add(z1, z4);
        Complex t2   = meow_add(z2, z3);
        Complex t3   = meow_sub(z1, z4);
        Complex t4   = meow_sub(z2, z3);
        // Equations 140

        Complex t5   = meow_add(t1, t2);
        Complex t6   = meow_mulf(meow_sub(t1, t2), MEOW_SQR_5_DIV_4);
        Complex t7   = meow_sub(z0, meow_mulf(t5, 0.25f));
        // Equation 141

        Complex t8   = meow_add(t7, t6);
        Complex t9   = meow_sub(t7, t6);
        // Equation 142

        Complex t10j = meow_mul_by_j
        (
            meow_add
            (
                  meow_mulf(t3, MEOW_SIN_2PI_5)
                , meow_mulf(t4, MEOW_SIN_2PI_10)
            )
        );

        Complex t11j = meow_mul_by_j
        (
            meow_sub
            (
                  meow_mulf(t3, MEOW_SIN_2PI_10)
                , meow_mulf(t4, MEOW_SIN_2PI_5)
            )
        );
        // Equation 143

        out[i0] = meow_add(z0, t5);
        // Equation 144

        out[i1] = meow_add(t8, t10j);
        out[i2] = meow_add(t9, t11j);
        // Equation 145

        out[i3] = meow_sub(t9, t11j);
        out[i4] = meow_sub(t8, t10j);
        // Equation 146
    }

    unsigned wi = 0u;
    for (unsigned butterfly = 1; butterfly < count; ++butterfly, wi+=4)
    {
        Complex w1  = w_n[wi + 0];
        Complex w2  = w_n[wi + 1];
        Complex w3  = w_n[wi + 2];
        Complex w4  = w_n[wi + 3];

        unsigned i0   = butterfly + 0 * count;
        unsigned i1   = butterfly + 1 * count;
        unsigned i2   = butterfly + 2 * count;
        unsigned i3   = butterfly + 3 * count;
        unsigned i4   = butterfly + 4 * count;

        Complex z0   = out[i0];
        Complex z1   = meow_mul_by_conjugate(out[i1], w1);
        Complex z2   = meow_mul_by_conjugate(out[i2], w2);
        Complex z3   = meow_mul_by_conjugate(out[i3], w3);
        Complex z4   = meow_mul_by_conjugate(out[i4], w4);

        Complex t1   = meow_add(z1, z4);
        Complex t2   = meow_add(z2, z3);
        Complex t3   = meow_sub(z1, z4);
        Complex t4   = meow_sub(z2, z3);
        // Equations 140

        Complex t5   = meow_add(t1, t2);
        Complex t6   = meow_mulf(meow_sub(t1, t2), MEOW_SQR_5_DIV_4);
        Complex t7   = meow_sub(z0, meow_mulf(t5, 0.25f));
        // Equation 141

        Complex t8   = meow_add(t7, t6);
        Complex t9   = meow_sub(t7, t6);
        // Equation 142

        Complex t10j = meow_mul_by_j
        (
            meow_add
            (
                  meow_mulf(t3, MEOW_SIN_2PI_5)
                , meow_mulf(t4, MEOW_SIN_2PI_10)
            )
        );

        Complex t11j = meow_mul_by_j
        (
            meow_sub
            (
                  meow_mulf(t3, MEOW_SIN_2PI_10)
                , meow_mulf(t4, MEOW_SIN_2PI_5)
            )
        );
        // Equation 143

        out[i0] = meow_add(z0, t5);
        // Equation 144

        out[i1] = meow_add(t8, t10j);
        out[i2] = meow_add(t9, t11j);
        // Equation 145

        out[i3] = meow_sub(t9, t11j);
        out[i4] = meow_sub(t8, t10j);
        // Equation 146
    }
}

#define MEOW_1_DIV_SQR_2 0.707106781186547524400844362104849039284835938f

static void meow_radix_8_dit
(
      const Meow_FFT_Complex* w_n
    , Meow_FFT_Complex*       out
    , unsigned                count
)
{
    const float* W = &w_n[0].r;

    {
        float T3;
        float T23;
        float T18;
        float T38;
        float T6;
        float T37;
        float T21;
        float T24;
        float T13;
        float T49;
        float T35;
        float T43;
        float T10;
        float T48;
        float T30;
        float T42;
        {
            float T1;
            float T2;
            float T19;
            float T20;
            T1 = out[0].r;
            T2 = out[count * 4].r;
            T3 = T1 + T2;
            T23 = T1 - T2;
            {
                float T16;
                float T17;
                float T4;
                float T5;
                T16 = out[0].j;
                T17 = out[count * 4].j;
                T18 = T16 + T17;
                T38 = T16 - T17;
                T4 = out[count * 2].r;
                T5 = out[count * 6].r;
                T6 = T4 + T5;
                T37 = T4 - T5;
            }
            T19 = out[count * 2].j;
            T20 = out[count * 6].j;
            T21 = T19 + T20;
            T24 = T19 - T20;
            {
                float T11;
                float T12;
                float T31;
                float T32;
                float T33;
                float T34;
                T11 = out[count * 7].r;
                T12 = out[count * 3].r;
                T31 = T11 - T12;
                T32 = out[count * 7].j;
                T33 = out[count * 3].j;
                T34 = T32 - T33;
                T13 = T11 + T12;
                T49 = T32 + T33;
                T35 = T31 - T34;
                T43 = T31 + T34;
            }
            {
                float T8;
                float T9;
                float T26;
                float T27;
                float T28;
                float T29;
                T8 = out[count * 1].r;
                T9 = out[count * 5].r;
                T26 = T8 - T9;
                T27 = out[count * 1].j;
                T28 = out[count * 5].j;
                T29 = T27 - T28;
                T10 = T8 + T9;
                T48 = T27 + T28;
                T30 = T26 + T29;
                T42 = T29 - T26;
            }
        }
        {
            float T7;
            float T14;
            float T51;
            float T52;
            T7 = T3 + T6;
            T14 = T10 + T13;
            out[count * 4].r = T7 - T14;
            out[0].r = T7 + T14;
            T51 = T18 + T21;
            T52 = T48 + T49;
            out[count * 4].j = T51 - T52;
            out[0].j = T51 + T52;
        }
        {
            float T15;
            float T22;
            float T47;
            float T50;
            T15 = T13 - T10;
            T22 = T18 - T21;
            out[count * 2].j = T15 + T22;
            out[count * 6].j = T22 - T15;
            T47 = T3 - T6;
            T50 = T48 - T49;
            out[count * 6].r = T47 - T50;
            out[count * 2].r = T47 + T50;
        }
        {
            float T25;
            float T36;
            float T45;
            float T46;
            T25 = T23 + T24;
            T36 = MEOW_1_DIV_SQR_2 * (T30 + T35);
            out[count * 5].r = T25 - T36;
            out[count * 1].r = T25 + T36;
            T45 = T38 - T37;
            T46 = MEOW_1_DIV_SQR_2 * (T42 + T43);
            out[count * 5].j = T45 - T46;
            out[count * 1].j = T45 + T46;
        }
        {
            float T39;
            float T40;
            float T41;
            float T44;
            T39 = T37 + T38;
            T40 = MEOW_1_DIV_SQR_2 * (T35 - T30);
            out[count * 7].j = T39 - T40;
            out[count * 3].j = T39 + T40;
            T41 = T23 - T24;
            T44 = MEOW_1_DIV_SQR_2 * (T42 - T43);
            out[count * 7].r = T41 - T44;
            out[count * 3].r = T41 + T44;
        }
    }

    out = out + 1;

    {
        unsigned m;
        for (m = 1; m < count; m = m + 1, out = out + 1, W = W + 14)
        {
            float T7;
            float T76;
            float T43;
            float T71;
            float T41;
            float T65;
            float T53;
            float T56;
            float T18;
            float T77;
            float T46;
            float T68;
            float T30;
            float T64;
            float T48;
            float T51;
            {
                float T1;
                float T70;
                float T6;
                float T69;
                T1 = out[0].r;
                T70 = out[0].j;
                {
                    float T3;
                    float T5;
                    float T2;
                    float T4;
                    T3 = out[count * 4].r;
                    T5 = out[count * 4].j;
                    T2 = W[6];
                    T4 = W[7];
                    T6 = (T2 * T3) + (T4 * T5);
                    T69 = (T2 * T5) - (T4 * T3);
                }
                T7 = T1 + T6;
                T76 = T70 - T69;
                T43 = T1 - T6;
                T71 = T69 + T70;
            }
            {
                float T35;
                float T54;
                float T40;
                float T55;
                {
                    float T32;
                    float T34;
                    float T31;
                    float T33;
                    T32 = out[count * 7].r;
                    T34 = out[count * 7].j;
                    T31 = W[12];
                    T33 = W[13];
                    T35 = (T31 * T32) + (T33 * T34);
                    T54 = (T31 * T34) - (T33 * T32);
                }
                {
                    float T37;
                    float T39;
                    float T36;
                    float T38;
                    T37 = out[count * 3].r;
                    T39 = out[count * 3].j;
                    T36 = W[4];
                    T38 = W[5];
                    T40 = (T36 * T37) + (T38 * T39);
                    T55 = (T36 * T39) - (T38 * T37);
                }
                T41 = T35 + T40;
                T65 = T54 + T55;
                T53 = T35 - T40;
                T56 = T54 - T55;
            }
            {
                float T12;
                float T44;
                float T17;
                float T45;
                {
                    float T9;
                    float T11;
                    float T8;
                    float T10;
                    T9 = out[count * 2].r;
                    T11 = out[count * 2].j;
                    T8 = W[2];
                    T10 = W[3];
                    T12 = (T8 * T9) + (T10 * T11);
                    T44 = (T8 * T11) - (T10 * T9);
                }
                {
                    float T14;
                    float T16;
                    float T13;
                    float T15;
                    T14 = out[count * 6].r;
                    T16 = out[count * 6].j;
                    T13 = W[10];
                    T15 = W[11];
                    T17 = (T13 * T14) + (T15 * T16);
                    T45 = (T13 * T16) - (T15 * T14);
                }
                T18 = T12 + T17;
                T77 = T12 - T17;
                T46 = T44 - T45;
                T68 = T44 + T45;
            }
            {
                float T24;
                float T49;
                float T29;
                float T50;
                {
                    float T21;
                    float T23;
                    float T20;
                    float T22;
                    T21 = out[count * 1].r;
                    T23 = out[count * 1].j;
                    T20 = W[0];
                    T22 = W[1];
                    T24 = (T20 * T21) + (T22 * T23);
                    T49 = (T20 * T23) - (T22 * T21);
                }
                {
                    float T26;
                    float T28;
                    float T25;
                    float T27;
                    T26 = out[count * 5].r;
                    T28 = out[count * 5].j;
                    T25 = W[8];
                    T27 = W[9];
                    T29 = (T25 * T26) + (T27 * T28);
                    T50 = (T25 * T28) - (T27 * T26);
                }
                T30 = T24 + T29;
                T64 = T49 + T50;
                T48 = T24 - T29;
                T51 = T49 - T50;
            }
            {
                float T19;
                float T42;
                float T73;
                float T74;
                T19 = T7 + T18;
                T42 = T30 + T41;
                out[count * 4].r = T19 - T42;
                out[0].r = T19 + T42;
                {
                    float T67;
                    float T72;
                    float T63;
                    float T66;
                    T67 = T64 + T65;
                    T72 = T68 + T71;
                    out[0].j = T67 + T72;
                    out[count * 4].j = T72 - T67;
                    T63 = T7 - T18;
                    T66 = T64 - T65;
                    out[count * 6].r = T63 - T66;
                    out[count * 2].r = T63 + T66;
                }
                T73 = T41 - T30;
                T74 = T71 - T68;
                out[count * 2].j = T73 + T74;
                out[count * 6].j = T74 - T73;
                {
                    float T59;
                    float T78;
                    float T62;
                    float T75;
                    float T60;
                    float T61;
                    T59 = T43 - T46;
                    T78 = T76 - T77;
                    T60 = T51 - T48;
                    T61 = T53 + T56;
                    T62 = MEOW_1_DIV_SQR_2 * (T60 - T61);
                    T75 = MEOW_1_DIV_SQR_2 * (T60 + T61);
                    out[count * 7].r = T59 - T62;
                    out[count * 5].j = T78 - T75;
                    out[count * 3].r = T59 + T62;
                    out[count * 1].j = T75 + T78;
                }
                {
                    float T47;
                    float T80;
                    float T58;
                    float T79;
                    float T52;
                    float T57;
                    T47 = T43 + T46;
                    T80 = T77 + T76;
                    T52 = T48 + T51;
                    T57 = T53 - T56;
                    T58 = MEOW_1_DIV_SQR_2 * (T52 + T57);
                    T79 = MEOW_1_DIV_SQR_2 * (T57 - T52);
                    out[count * 5].r = T47 - T58;
                    out[count * 7].j = T80 - T79;
                    out[count * 1].r = T47 + T58;
                    out[count * 3].j = T79 + T80;
                }
            }
        }
    }
}
void meow_recursive_fft_mixed_meow_radix_dit
(
      const Meow_FFT_Workset* fft
    , unsigned                stage
    , Complex* in
    , Meow_FFT_Complex*       out
    , unsigned                w_mul
)
{
    const unsigned radix = fft->stages.radix[stage];
    const unsigned count = fft->stages.remainder[stage];

    Complex* w              = fft->wn;
    const unsigned w_offset = fft->stages.offsets[stage];
    Complex* w_sequential   = &fft->wn_ordered[w_offset];

    if (count == 1)
    {
        for (unsigned i = 0; i < radix; i++)
        {
            out[i] = in[i * w_mul];
        }
    }
    else
    {
        const unsigned new_w_multiplier = w_mul * radix;

        for (unsigned i = 0; i < radix; ++i)
        {
            meow_recursive_fft_mixed_meow_radix_dit
            (
                  fft
                , stage + 1
                , &in[w_mul * i]
                , &out[count * i]
                , new_w_multiplier
            );
        }
    }

    switch (radix)
    {
        case  2: meow_radix_2_dit(w_sequential, out, count); break;
        case  3: meow_radix_3_dit(w_sequential, out, count); break;
        case  4: meow_radix_4_dit(w_sequential, out, count); break;
        case  5: meow_radix_5_dit(w_sequential, out, count); break;
        case  8: meow_radix_8_dit(w_sequential, out, count); break;

        default: meow_dft_n_dit(w, out, count, w_mul, radix, fft->N, 0); break;
    }
}
void meow_fft
(
      const Meow_FFT_Workset* data
    , const Meow_FFT_Complex* in
    , Meow_FFT_Complex* out
)
{
    meow_recursive_fft_mixed_meow_radix_dit
    (
          data
        , 0
        , in
        , out
        , 1
    );
}
