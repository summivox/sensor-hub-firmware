#ifndef _CIC_HPP_
#define _CIC_HPP_

#include "int_sel.hpp"

#include <stdlib.h>
#include <string.h>

template <uint8_t input_bits, uint8_t decimation_bits, uint8_t rank>
struct CIC {
    enum {
        decimation = 1<<decimation_bits,
        gain_bits = decimation_bits*rank,
    };

    typedef typename int_bits<input_bits>::s input_t;
    typedef typename int_bits<input_bits + gain_bits>::s state_t;

    state_t I[rank], C[rank];

    CIC() {
        memset(this, 0, sizeof(*this));
    }
    input_t operator() (input_t* x, int skip = 1) {
        // integrate sections
        for (int i = 0 ; i < decimation ; ++i) {
            int r = rank - 1, rr = rank - 2;
            while (rr >= 0) {
                I[r] += I[rr];
                r = rr--;
            }
            I[0] += *x;
            x += skip;
        }
        // comb sections
        state_t y = I[rank-1];
        for (int r = 0 ; r < rank ; ++r) {
            state_t z = y - C[r];
            C[r] = y;
            y = z;
        }
        // scale back the gain
        return y >> gain_bits;
    }
};


#endif//_CIC_HPP_
