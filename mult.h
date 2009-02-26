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

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

// Let us use the `m_` prefix for micro code stuff.
typedef int16_t m_int16; 
typedef uint16_t m_uint16; 
typedef int32_t m_int32; 
typedef uint32_t m_uint32; 

typedef struct m_flags {
    int z; // AR = 0
    int n; // MSB(AR) = 1
    int o; // arithmetic op spilled
    int c; // memory digit from add or sub, or bit shifted out
    int l; // LC (loop counter) = 0
} m_flags_t;

typedef struct m_machine {
    m_flags_t flags;
    m_int16 ar;
    m_int16 hr;
    m_int16 pm0; // generic memory addresses
    m_int16 pm1;
    m_int16 grx;
    m_int16 lc; // loop counter
} m_machine_t;

static void debug_print(const char *fmt, ...);
static void m_arhr_aritr(m_machine_t *machine);
static m_int32 m_arhr(m_machine_t *machine);
static void m_debug_machine(m_machine_t *machine);
static void m_add_ar(m_machine_t *machine, m_int16 a);
static void m_mult16(m_machine_t *machine);
static void m_init_machine(m_machine_t *machine);
