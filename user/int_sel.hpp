#ifndef _INT_SEL_HPP_
#define _INT_SEL_HPP_

#include <stdint.h>

//get integer type with at least `b` bytes
template <int b> struct int_bytes;
#define _INT_SEL(I, U, S) \
    template <> \
    struct int_bytes <I> { \
        typedef U u; \
        typedef S s; \
    }
_INT_SEL(1, uint8_t, int8_t);
_INT_SEL(2, uint16_t, int16_t);
_INT_SEL(3, uint32_t, int32_t);
_INT_SEL(4, uint32_t, int32_t);
#undef _INT_SEL

//get integer type with at least `k` bits
template <int k>
struct int_bits {
    typedef typename int_bytes<(k+7)/8>::u u;
    typedef typename int_bytes<(k+7)/8>::s s;
};

#endif//_INT_SEL_HPP_
