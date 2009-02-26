// Copyright (c) 2009 Simon Pantzare <simon@spantz.org>
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "mult.h"

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
    
    machine->flags.n = 0;
    if (machine->ar < 0) {
        machine->flags.n = 1;
    }
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
    debug_print("  flags(C=%d, L=%d, N=%d, O=%d, ...)\n", 
            machine->flags.c, machine->flags.l, machine->flags.n, 
            machine->flags.o);
    m_bstring32(buf1, m_arhr(machine));
    debug_print("  ARHR=%s (%d)\n", buf1, m_arhr(machine));
    debug_print("  LC=%d\n", machine->lc);
}

static void m_add_ar(m_machine_t *machine, m_int16 a)
{
    m_int16 old_ar = machine->ar;
    machine->ar = machine->ar + a;

    // Update O flag (overflow) appropriately.
    machine->flags.o = 0;
    if (old_ar >= 0 && a >= 0) {
        if (machine->ar < 0) {
            machine->flags.o = 1;
        }
    }
    else if (old_ar < 0 && a < 0) {
        if (machine->ar >= 0) {
            machine->flags.o = 1;
        }
    }
}

static void m_mult16(m_machine_t *machine) // pm0 = X, grx = Y
{
    // Buffers to put binary numbers in.
    char buf1[64];
    char buf2[64];

    // Keep input values.
    m_int16 x = machine->pm0;
    m_int16 y = machine->grx;

    m_int32 should_be = machine->pm0 * machine->grx;

    // Initialize loop counter, L flag becomes 0.
    machine->lc = 15;
    machine->flags.l = 0;
    
    // Reset AR, put X in HR.    
    machine->ar = 0;
    machine->hr = machine->pm0;
    
    // Debugging.    
    debug_print("Entry.\n");
    m_debug_machine(machine);
    debug_print("\n");

    // ARHR arithmetic shift right.
    m_arhr_aritr(machine);
    
    int loop_count = 0;
    while (machine->flags.l != 1) {
        // Decrement loop counter.
        if (machine->lc == 0) {
            machine->flags.l = 1;
        }
        else {
            machine->lc -= 1;
        }

        m_add_ar(machine, 0); 
        if (machine->flags.c == 1) {
            if (machine->flags.l != 1) {
                m_add_ar(machine, machine->grx);
            }
            else {
                m_add_ar(machine, -machine->grx);
            }
        }
        // else { m_add_ar(machine, 0); } // not needed if we have it before
        
        // ARHR arithmetic shift right.
        m_arhr_aritr(machine);
        
        // Fix spill.        
        if (machine->flags.o == 1) {
            m_bstring16(buf1, machine->ar);
            if (m_bitn(machine->ar, 15) == 0) {
                machine->ar = (m_uint16)machine->ar | (m_uint16)0x8000;
            }
            else {
                machine->ar = (m_uint16)machine->ar & (m_uint16)0x7FFF;
            }
            m_bstring16(buf2, machine->ar);
        }

        debug_print("Bit %d processed.\n", loop_count++);
        m_debug_machine(machine);
        debug_print("\n");
    }

    if (m_arhr(machine) != should_be) {
        debug_print("Incorrect answer.\n");
        m_bstring32(buf1, m_arhr(machine));
        m_bstring32(buf2, should_be);
        debug_print("  %s (%d) is not\n  %s (%d)\n", 
                buf1, m_arhr(machine), buf2, should_be);
    }

    // Put back in program memory.
    machine->pm0 = machine->ar;
    machine->pm1 = machine->hr;
    m_bstring16(buf1, machine->pm0);
    m_bstring16(buf2, machine->pm1);
    debug_print("PM0: %s (%d)\nPM1: %s (%d)\n",
            buf1, machine->pm0, buf2, machine->pm1);
    debug_print("\n(Input was X=%d, Y=%d. Result is in last ARHR value.)\n", 
            x, y);
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
    if (argc != 3) {
        fprintf(stderr, "Usage: %s X Y\n", argv[0]);
        return -1;
    }

    m_machine_t M;
    m_init_machine(&M);

    M.pm0 = (m_int16)atoi(argv[1]);
    M.grx = (m_int16)atoi(argv[2]);
    m_mult16(&M);

    return 0;
}
