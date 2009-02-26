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
