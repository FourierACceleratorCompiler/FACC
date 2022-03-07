#include <stddef.h>

// Conditions
#define POWER_OF_TWO(x) ((x & (x - 1)) == 0)
#define GREATER_THAN(x, y) x > y
#define GREATER_THAN_OR_EQUAL(x, y) x >= y
#define LESS_THAN(x, y) x < y
#define LESS_THAN_OR_EQUAL(x, y) x <= y
#define PRIM_EQUAL(x, y) x == y
/* TODO --- would like to make this better.  */
#define FLOAT_EQUAL(x, y) ((x < y + x / 1000.0) && (x > y - x / 1000.0))

// Operations
#define Pow2(x) (1 << x)
#define IntDivide(x,y) (x / y)

// Builtin types
typedef struct { float f32_1; float f32_2; } facc_2xf32_t;
typedef struct { float f64_1; float f64_2; } facc_2xf64_t;

// FACC aligned memory functions
void *facc_malloc(size_t alignment, size_t size);
void facc_free(void* pointer);
