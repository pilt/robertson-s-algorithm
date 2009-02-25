#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdarg.h>

// Let us use the `m_` prefix for micro code stuff.
typedef int16_t m_int16; 
typedef int32_t m_int32; 

// We are missing a conversion letter for the `bool` type. With this macro we
// can do `printf("Boolean value is %c.\n", bv(some_bool))`.
#define bv(b) ((b) ? 'T' : 'F')

static void
debug_print(const char *fmt, ...)
{
#ifndef NDEBUG
    va_list args;
    va_start(args, fmt);
    if (vfprintf(stderr, fmt, args) != 0) {
        // Ignore error.
    }
    va_end(args);
#endif
}

static bool m_same_sign16(m_int16 a, m_int16 b)
{
    if (a >= 0 && b >= 0) {
        return true;
    }
    else if (a < 0 && b < 0) {
        return true;
    }
    return false;
}

// Construct functions for making binary represantions of integers.
#define CONSTRUCT_M_BSTRINGN(bits) \
    static void m_bstring ## bits (char* buf, m_int ## bits n) \
    { \
        buf[0] = '0'; \
        buf[1] = 'b'; \
        for (size_t i = 0; i != bits; ++i) { \
            if (((uint ## bits ## _t)((1 << (bits-1))) \
                    & (uint ## bits ## _t)(n << i)) != 0) { \
                buf[i + 2] = '1'; \
            } \
            else { \
                buf[i + 2] = '0'; \
            } \
        } \
        buf[bits + 2] = '\0'; \
    }
CONSTRUCT_M_BSTRINGN(16)
CONSTRUCT_M_BSTRINGN(32)

// Get the nth bit from bits. 
#define m_bitn(bits, n) (((uint32_t)(bits) & (uint32_t)(1 << (n))) ? 1 : 0)

static m_int32 m_mult16(m_int16 x, m_int16 y)
{
    // Ignore the m_bstring32 and debug_print calls. They are just for
    // debugging.
    
    // Buffers to put binary numbers in.
    char buf1[64];
    char buf2[64];

    m_int32 result = 0;
    int spill = 0;

    m_bstring16(buf1, x); 
    m_bstring16(buf2, y); 
    debug_print("X:   %s\n     = %d\nY:   %s\n     = %d\n", 
            buf1, x, buf2, y);
    m_bstring32(buf1, x*y);
    debug_print("X*Y: %s\n     = %d\n\n", buf1, x*y);

    for (int i = 0; i != 16; ++i) {
        m_bstring32(buf1, result);

        result = result >> 1;
        if (m_bitn(x, i) == 1) {
            if (i == 15) { // last iteration, MSB is 1
                result -= y << 15;
                m_bstring32(buf2, -(y << i));
            }
            else {
                result += y << 15;
                m_bstring32(buf2, (y << i));
            }
        }
        else {
            m_bstring32(buf2, 0);
        }

        debug_print("%2d:  %s\n   + %s %cX_%d*Y\n",
                i + 1, buf1, buf2, i == 15 ? '-' : ' ', i);
    }

    m_bstring32(buf1, result);
    debug_print("\n   = %s\n", buf1);
    debug_print("   = %d\n", result);

    return result;
}

int main(int argc, char **argv)
{
    m_mult16(5, 0x7FF0);
    debug_print("\n\n");
    m_mult16(-5, 0xF0);
    debug_print("\n\n");
    m_mult16(0x8000, 0x7FFF);
    return 0;
}
