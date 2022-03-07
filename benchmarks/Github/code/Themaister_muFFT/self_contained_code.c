typedef void (*mufft_1d_func)(void * MUFFT_RESTRICT output, const void * MUFFT_RESTRICT input,
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h>
#define MUFFT_FORWARD (-1)
/// The inverse FFT transform.
#define MUFFT_INVERSE  (1)

/// \addtogroup MUFFT_FLAG Planning options
/// @{

/// muFFT will use any SIMD instruction set it can if supported by the CPU.
#define MUFFT_FLAG_CPU_ANY (0)
/// muFFT will not use any SIMD instruction set.
#define MUFFT_FLAG_CPU_NO_SIMD ((1 << 16) - 1)
/// muFFT will not use the AVX instruction set.
#define MUFFT_FLAG_CPU_NO_AVX (1 << 0)
/// muFFT will not use the SSE3 instruction set.
#define MUFFT_FLAG_CPU_NO_SSE3 (1 << 1)
/// muFFT will not use the SSE instruction set.
#define MUFFT_FLAG_CPU_NO_SSE (1 << 2)
/// The real-to-complex 1D transform will also output the redundant conjugate values X(N - k) = X(k)*.
#define MUFFT_FLAG_FULL_R2C (1 << 16)
/// The second/upper half of the input array is assumed to be 0 and will not be read and memory for the second half of the input array does not have to be allocated.
/// This is mostly useful when you want to do zero-padded FFTs which are very common for convolution-type operations, see \ref MUFFT_CONV. This flag is only recognized for 1D transforms.
#define MUFFT_FLAG_ZERO_PAD_UPPER_HALF (1 << 17)
#define MUFFT_CONV_METHOD_FLAG_MONO_MONO 0

/// The convolution will convolve stereo data with a real filter or stereo filter with single channel data.
/// The stereo channel should be interleaved so that the real part of the input represents the first channel, and the imaginary channel represents the other channel.
/// This format matches perfectly to the commonly used interleaved format used to represent stereo audio of (L, R, L, R, L, R, ...).
/// The first block is a stereo channel (complex) and second block is single channel (real) input.
#define MUFFT_CONV_METHOD_FLAG_STEREO_MONO 1

/// The first block is assumed to be zero padded as defined by \ref MUFFT_FLAG_ZERO_PAD_UPPER_HALF.
#define MUFFT_CONV_METHOD_FLAG_ZERO_PAD_UPPER_HALF_FIRST (1 << 2)

/// The second block is assumed to be zero padded as defined by \ref MUFFT_FLAG_ZERO_PAD_UPPER_HALF.
#define MUFFT_CONV_METHOD_FLAG_ZERO_PAD_UPPER_HALF_SECOND (1 << 3)
/// @}

/// The first block.
#define MUFFT_CONV_BLOCK_FIRST 0

/// The second block.
#define MUFFT_CONV_BLOCK_SECOND 1
struct mufft_step_1d
{
    mufft_1d_func func; ///< Function pointer to a 1D partial FFT.
    unsigned radix; ///< Radix of the FFT step. 2, 4 or 8.
    unsigned p; ///< The current p factor of the FFT. Determines butterfly stride. It is equal to prev_step.p * prev_step.radix. Initial value is 1.
    unsigned twiddle_offset; ///< Offset into twiddle factor table.
};
/// Represents a complete plan for a 1D FFT.
struct mufft_plan_1d
{
    struct mufft_step_1d *steps; ///< A list of steps to take to complete a full N-tap FFT.
    unsigned num_steps; ///< Number of steps contained in mufft_plan_1d::steps.
    unsigned N; ///< Size of the 1D transform.

    cfloat *tmp_buffer; ///< A temporary buffer used during intermediate steps of the FFT.
    cfloat *twiddles; ///< Buffer holding twiddle factors used in the FFT.

    mufft_r2c_resolve_func r2c_resolve; ///< If non-NULL, a function to turn a N / 2 complex transform into a N-tap real transform.
    mufft_r2c_resolve_func c2r_resolve; ///< If non-NULL, a function to turn a N real inverse transform into a N / 2 complex transform.
    cfloat *r2c_twiddles; ///< Special twiddle factors used in mufft_plan_1d::r2c_resolve or mufft_plan_1d::c2r_resolve.
};
/// \brief Computes the twiddle factor exp(pi * I * direction * k / p)
static cfloat twiddle(int direction, int k, int p)
{
    double phase = (M_PI * direction * k) / p;
    return cfloat_create((float)cos(phase), (float)sin(phase));
}

/// \brief Builds a table of twiddle factors.
/// The table is built for a DIT transform with increasing butterfly strides.
/// The table is suitable for any FFT radix.
/// @param N Transform size
/// @param direction Direction of transform. See \ref MUFFT_FORWARD and \ref MUFFT_INVERSE.
/// @returns Newly allocated twiddle factor table.
static cfloat *build_twiddles(unsigned N, int direction)
{
    cfloat *twiddles = mufft_alloc(N * sizeof(cfloat));
    if (twiddles == NULL)
    {
        return NULL;
    }

    cfloat *pt = twiddles;

    for (unsigned p = 1; p < N; p <<= 1)
    {
        for (unsigned k = 0; k < p; k++)
        {
            pt[k] = twiddle(direction, k, p);
        }
        pt += p == 2 ? 3 : p; // Make sure that twiddles for p == 4 and up are aligned properly for AVX.
    }

    return twiddles;
}
/// ABI compatible base struct for \ref fft_step_1d and \ref fft_step_2d.
struct fft_step_base
{
    void (*func)(void); ///< Generic function pointer.
    unsigned radix; ///< Radix of the FFT. 2, 4 or 8.
};

/// Represents a single step of the complete 1D/horizontal FFT with requirements on use.
struct fft_step_1d
{
    mufft_1d_func func; ///< Function pointer to a 1D partial FFT.
    unsigned radix; ///< Radix of the FFT step. 2, 4 or 8.
    unsigned minimum_elements; ///< Minimum transform size for which this function can be used.
    unsigned fixed_p; ///< Non-zero if this can only be used with a fixed value for mufft_step_base::p.
    unsigned minimum_p; ///< Minimum p-factor for which this can be used. Set to -1u if it can only be used with fft_step_1d::fixed_p.
    unsigned flags; ///< Flags which determine under which conditions this function can be used.
};
static const struct fft_step_1d fft_1d_table[] = {
#define STAMP_CPU_1D(arch, ext, min_x) \
    { .flags = arch | MUFFT_FLAG_DIRECTION_FORWARD | MUFFT_FLAG_NO_ZERO_PAD_UPPER_HALF, \
        .func = mufft_forward_radix8_p1_ ## ext, .minimum_elements = 8 * min_x, .radix = 8, .fixed_p = 1, .minimum_p = ~0u }, \
    { .flags = arch | MUFFT_FLAG_DIRECTION_FORWARD | MUFFT_FLAG_NO_ZERO_PAD_UPPER_HALF, \
        .func = mufft_forward_radix4_p1_ ## ext, .minimum_elements = 4 * min_x, .radix = 4, .fixed_p = 1, .minimum_p = ~0u }, \
    { .flags = arch | MUFFT_FLAG_DIRECTION_ANY | MUFFT_FLAG_NO_ZERO_PAD_UPPER_HALF, \
        .func = mufft_radix2_p1_ ## ext, .minimum_elements = 2 * min_x, .radix = 2, .fixed_p = 1, .minimum_p = ~0u }, \
    { .flags = arch | MUFFT_FLAG_DIRECTION_FORWARD | MUFFT_FLAG_ZERO_PAD_UPPER_HALF, \
        .func = mufft_forward_half_radix8_p1_ ## ext, .minimum_elements = 8 * min_x, .radix = 8, .fixed_p = 1, .minimum_p = ~0u }, \
    { .flags = arch | MUFFT_FLAG_DIRECTION_FORWARD | MUFFT_FLAG_ZERO_PAD_UPPER_HALF, \
        .func = mufft_forward_half_radix4_p1_ ## ext, .minimum_elements = 4 * min_x, .radix = 4, .fixed_p = 1, .minimum_p = ~0u }, \
    { .flags = arch | MUFFT_FLAG_DIRECTION_ANY | MUFFT_FLAG_ZERO_PAD_UPPER_HALF, \
        .func = mufft_radix2_half_p1_ ## ext, .minimum_elements = 2 * min_x, .radix = 2, .fixed_p = 1, .minimum_p = ~0u }, \
    { .flags = arch | MUFFT_FLAG_DIRECTION_FORWARD, \
        .func = mufft_forward_radix2_p2_ ## ext, .minimum_elements = 2 * min_x, .radix = 2, .fixed_p = 2, .minimum_p = ~0u }, \
    { .flags = arch | MUFFT_FLAG_DIRECTION_INVERSE, \
        .func = mufft_inverse_radix8_p1_ ## ext, .minimum_elements = 8 * min_x, .radix = 8, .fixed_p = 1, .minimum_p = ~0u }, \
    { .flags = arch | MUFFT_FLAG_DIRECTION_INVERSE, \
        .func = mufft_inverse_radix4_p1_ ## ext, .minimum_elements = 4 * min_x, .radix = 4, .fixed_p = 1, .minimum_p = ~0u }, \
    { .flags = arch | MUFFT_FLAG_DIRECTION_INVERSE, \
        .func = mufft_inverse_radix2_p2_ ## ext, .minimum_elements = 2 * min_x, .radix = 2, .fixed_p = 2, .minimum_p = ~0u }, \
    { .flags = arch | MUFFT_FLAG_DIRECTION_ANY, \
        .func = mufft_radix8_generic_ ## ext, .minimum_elements = 8 * min_x, .radix = 8, .minimum_p = 8 }, \
    { .flags = arch | MUFFT_FLAG_DIRECTION_ANY, \
        .func = mufft_radix4_generic_ ## ext, .minimum_elements = 4 * min_x, .radix = 4, .minimum_p = 4 }, \
    { .flags = arch | MUFFT_FLAG_DIRECTION_ANY, \
        .func = mufft_radix2_generic_ ## ext, .minimum_elements = 2 * min_x, .radix = 2, .minimum_p = 4 }

#ifdef MUFFT_HAVE_AVX
    STAMP_CPU_1D(MUFFT_FLAG_CPU_AVX, avx, 4),
#endif
#ifdef MUFFT_HAVE_SSE3
    STAMP_CPU_1D(MUFFT_FLAG_CPU_SSE3, sse3, 2),
#endif
#ifdef MUFFT_HAVE_SSE
    STAMP_CPU_1D(MUFFT_FLAG_CPU_SSE, sse, 2),
#endif
    STAMP_CPU_1D(0, c, 1),
};
/// \brief Adds a new FFT step to either \ref mufft_step_1d or \ref mufft_step_2d.
static bool add_step(struct mufft_step_base **steps, unsigned *num_steps,
        const struct fft_step_base *step, unsigned p)
{
    unsigned twiddle_offset = 0;
    if (*num_steps != 0)
    {
        struct mufft_step_base prev = (*steps)[*num_steps - 1];
        twiddle_offset = prev.twiddle_offset +
            (prev.p == 2 ? 3 : (prev.p * (prev.radix - 1)));

        // We skipped radix2 kernels, we have to add the padding twiddle here.
        if (p >= 4 && prev.p == 1)
        {
            twiddle_offset++;
        }
    }

    struct mufft_step_base *new_steps = realloc(*steps, (*num_steps + 1) * sizeof(*new_steps));
    if (new_steps == NULL)
    {
        return false;
    }

    *steps = new_steps;
    (*steps)[*num_steps] = (struct mufft_step_base) {
        .func = step->func,
        .radix = step->radix,
        .p = p,
        .twiddle_offset = twiddle_offset,
    };
    (*num_steps)++;
    return true;
}

/// \brief Builds a plan for a horizontal transform.
static bool build_plan_1d(struct mufft_step_1d **steps, unsigned *num_steps, unsigned N, int direction, unsigned flags)
{
    unsigned radix = N;
    unsigned p = 1;

    unsigned step_flags = 0;
    switch (direction)
    {
        case MUFFT_FORWARD:
            step_flags |= MUFFT_FLAG_DIRECTION_FORWARD;
            break;

        case MUFFT_INVERSE:
            step_flags |= MUFFT_FLAG_DIRECTION_INVERSE;
            break;
    }

    // Add CPU flags. Just accept any CPU for now, but mask out flags we don't want.
    step_flags |= mufft_get_cpu_flags() & ~(MUFFT_FLAG_CPU_NO_SIMD & flags);
    step_flags |= (flags & MUFFT_FLAG_ZERO_PAD_UPPER_HALF) != 0 ?
        MUFFT_FLAG_ZERO_PAD_UPPER_HALF : MUFFT_FLAG_NO_ZERO_PAD_UPPER_HALF;

    while (radix > 1)
    {
        bool found = false;

        // Find first (optimal?) routine which can do work.
        for (unsigned i = 0; i < ARRAY_SIZE(fft_1d_table); i++)
        {
            const struct fft_step_1d *step = &fft_1d_table[i];

            if (radix % step->radix == 0 &&
                    N >= step->minimum_elements &&
                    (step_flags & step->flags) == step->flags &&
                    (p >= step->minimum_p || p == step->fixed_p))
            {
                // Ugly casting, but add_step_1d and add_step_2d are ABI-wise exactly the same, and we don't have templates :(
                if (add_step((struct mufft_step_base**)steps, num_steps, (const struct fft_step_base*)step, p))
                {
                    found = true;
                    radix /= step->radix;
                    p *= step->radix;
                    break;
                }
            }
        }

        if (!found)
        {
            return false;
        }
    }

    return true;
}
void *mufft_alloc(size_t size)
{
#if defined(_ISOC11_SOURCE)
    return aligned_alloc(MUFFT_ALIGNMENT, size);
#elif (_POSIX_C_SOURCE >= 200112L) || (_XOPEN_SOURCE >= 600)
    void *ptr = NULL;
    if (posix_memalign(&ptr, MUFFT_ALIGNMENT, size) < 0)
    {
        return NULL;
    }
    return ptr;
#else
    // Align stuff ourselves. Kinda ugly, but will work anywhere.
    void **place;
    uintptr_t addr = 0;
    void *ptr = malloc(MUFFT_ALIGNMENT + size + sizeof(uintptr_t));

    if (ptr == NULL)
    {
        return NULL;
    }

    addr = ((uintptr_t)ptr + sizeof(uintptr_t) + MUFFT_ALIGNMENT)
        & ~(MUFFT_ALIGNMENT - 1);
    place = (void**)addr;
    place[-1] = ptr;

    return (void*)addr;
#endif
}

void *mufft_calloc(size_t size)
{
    void *ptr = mufft_alloc(size);
    if (ptr != NULL)
    {
        memset(ptr, 0, size);
    }
    return ptr;
}

void mufft_free(void *ptr)
{
#if !defined(_ISOC11_SOURCE) && !((_POSIX_C_SOURCE >= 200112L) || (_XOPEN_SOURCE >= 600))
    if (ptr != NULL)
    {
        void **p = (void**)ptr;
        free(p[-1]);
    }
#else
    free(ptr);
#endif
}
void mufft_free_plan_1d(mufft_plan_1d *plan)
{
    if (plan == NULL)
    {
        return;
    }
    free(plan->steps);
    mufft_free(plan->tmp_buffer);
    mufft_free(plan->twiddles);
    mufft_free(plan->r2c_twiddles);
    mufft_free(plan);
}
mufft_plan_1d *mufft_create_plan_1d_c2c(unsigned N, int direction, unsigned flags)
{
    if ((N & (N - 1)) != 0 || N == 1)
    {
        return NULL;
    }

    mufft_plan_1d *plan = mufft_calloc(sizeof(*plan));
    if (plan == NULL)
    {
        goto error;
    }

    plan->twiddles = build_twiddles(N, direction);
    if (plan->twiddles == NULL)
    {
        goto error;
    }

    plan->tmp_buffer = mufft_alloc(N * sizeof(cfloat));
    if (plan->tmp_buffer == NULL)
    {
        goto error;
    }

    if (!build_plan_1d(&plan->steps, &plan->num_steps, N, direction, flags))
    {
        goto error;
    }

    plan->N = N;
    return plan;

error:
    mufft_free_plan_1d(plan);
    return NULL;
}
void mufft_execute_plan_1d(mufft_plan_1d *plan, void * MUFFT_RESTRICT output, const void * MUFFT_RESTRICT input)
{
    const cfloat *pt = plan->twiddles;
    cfloat *out = output;
    cfloat *in = plan->tmp_buffer;
    unsigned N = plan->N;

    // If we're doing real-to-complex, we need an extra step.
    unsigned steps = plan->num_steps + (plan->r2c_resolve != NULL);

    // We want final step to write to output.
    if ((steps & 1) == 1)
    {
        SWAP(out, in);
    }

    const struct mufft_step_1d *first_step = &plan->steps[0];
    if (plan->c2r_resolve != NULL)
    {
        plan->c2r_resolve(out, input, plan->r2c_twiddles, N);
        first_step->func(in, out, pt, 1, N);
    }
    else
    {
        first_step->func(in, input, pt, 1, N);
    }

    for (unsigned i = 1; i < plan->num_steps; i++)
    {
        const struct mufft_step_1d *step = &plan->steps[i];
        step->func(out, in, pt + step->twiddle_offset, step->p, N);
        SWAP(out, in);
    }

    // Do Real-to-complex butterfly resolve.
    if (plan->r2c_resolve != NULL)
    {
        plan->r2c_resolve(out, in, plan->r2c_twiddles, N);
    }
}
