#include "mult.h"

m_machine_t M;

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

static void m_arhr_aritr(m_machine_t *machine)
{
    machine->flags.c = m_bitn(machine->hr, 0); 
    machine->hr = ((m_uint16)machine->hr) >> 1;
    machine->hr = machine->hr | ((machine->ar & 1) << 15);
    machine->ar = machine->ar >> 1;
}

static m_int32 m_arhr(m_machine_t *machine)
{
    m_int32 result = 0;
    result = machine->ar;
    result = result << 16;
    result |= (m_uint16)machine->hr;
    return result;
}

static void m_debug_machine(m_machine_t *machine)
{
    char buf1[64];
    m_bstring16(buf1, machine->ar);
    debug_print("  AR=%s (%d)\n", buf1, machine->ar);
    m_bstring16(buf1, machine->hr);
    debug_print("  HR=%s (%d)\n", buf1, machine->hr);
    debug_print("  flags(C=%d, L=%d ...)\n", 
            machine->flags.c, machine->flags.l);
    m_bstring32(buf1, m_arhr(machine));
    debug_print("  ARHR=%s (%d)\n", buf1, m_arhr(machine));
}

static m_int32 m_mult16(m_machine_t *machine) // pm0 = X, grx = Y
{
    // Ignore the m_bstring32 and debug_print calls. They are just for
    // debugging.
    
    // Buffers to put binary numbers in.
    char buf1[64];
    char buf2[64];

    m_int32 result = 0;
    int spill = 0;

    m_bstring16(buf1, machine->pm0); 
    m_bstring16(buf2, machine->grx); 
    debug_print("X:   %s\n     = %d\nY:   %s\n     = %d\n", 
            buf1, machine->pm0, buf2, machine->grx);
    m_bstring32(buf1, machine->pm0*machine->grx);
    debug_print("X*Y: %s\n     = %d\n\n", buf1, machine->pm0*machine->grx);

    // Initialize loop counter, L flag becomes 0.
    machine->lc = 16;
    machine->flags.l = 0;
    
    machine->ar = 0;
    machine->hr = machine->pm0;
    
    int loop_count = 0;
    while (machine->flags.l != 1) {
        debug_print("%d.\n", loop_count++);
        m_debug_machine(machine);

        // Decrement loop counter.
        if (machine->lc == 0) {
            machine->flags.l = 1;
        }
        else {
            machine->lc -= 1;
        }
        
        // ARHR arithmetic shift right.
        m_arhr_aritr(machine);

        if (machine->flags.c == 1) {
            machine->ar += machine->pm0;
        }
        
        debug_print("\n");
    }

    return result;
}

static void m_init_machine(m_machine_t *machine)
{
    machine->flags.z = 0;
    machine->flags.n = 0;
    machine->flags.o = 0;
    machine->flags.c = 0;
    machine->flags.l = 0;
    machine->ar = 0;
    machine->hr = 0;
    machine->pm0 = 0;
    machine->pm1 = 0;
    machine->grx = 0;
    machine->lc = 0;
}

int main(int argc, char **argv)
{
    m_init_machine(&M); // M is our global machine
    M.pm0 = 0xF0F0; // X
    M.grx = 5; // Y
    m_mult16(&M);
    /*
    debug_print("\n\n");
    m_mult16(-5, 0xF0);
    debug_print("\n\n");
    m_mult16(0x8000, 0x7FFF);
    */
    return 0;
}
