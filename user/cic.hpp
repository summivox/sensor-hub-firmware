#ifndef _CIC_HPP_
#define _CIC_HPP_

#include "int_sel.hpp"

#include <stdlib.h>
#include <string.h>

// generic
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
        for (int i = decimation - 1 ; i >= 0 ; --i) {
            for (int r = rank - 1 ; r >= 1 ; --r) {
                I[r] += I[r-1];
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

// hard coded
template <uint8_t input_bits, uint8_t decimation_bits>
struct CIC <input_bits, decimation_bits, 2> {
    enum {
        decimation = 1<<decimation_bits,
        gain_bits = decimation_bits*2,
    };

    typedef typename int_bits<input_bits>::s input_t;
    typedef typename int_bits<input_bits + gain_bits>::s state_t;

    state_t I0, I1, C0, C1;

    CIC() {
        I0 = I1 = C0 = C1 = 0;
    }
    input_t operator() (input_t* x, int skip = 1) {
        // integrate sections
        for (int i = decimation - 1 ; i >= 0 ; --i) {
            I1 += I0;
            I0 += *x;
            x += skip;
        }
        // comb sections
        state_t y = I1, z;
        z = y - C0; C0 = y; y = z;
        z = y - C1; C1 = y; y = z;
        
        // scale back the gain
        return y >> gain_bits;
    }
};

template <uint8_t input_bits, uint8_t decimation_bits>
struct CIC <input_bits, decimation_bits, 3> {
    enum {
        decimation = 1<<decimation_bits,
        gain_bits = decimation_bits*3,
    };

    typedef typename int_bits<input_bits>::s input_t;
    typedef typename int_bits<input_bits + gain_bits>::s state_t;

    state_t I0, I1, I2, C0, C1, C2;

    CIC() {
        I0 = I1 = I2 = C0 = C1 = C2 = 0;
    }
    input_t operator() (input_t* x, int skip = 1) {
        // integrate sections
        for (int i = decimation - 1 ; i >= 0 ; --i) {
            I2 += I1;
            I1 += I0;
            I0 += *x;
            x += skip;
        }
        // comb sections
        state_t y = I2, z;
        z = y - C0; C0 = y; y = z;
        z = y - C1; C1 = y; y = z;
        z = y - C2; C2 = y; y = z;
        
        // scale back the gain
        return y >> gain_bits;
    }
};

#endif//_CIC_HPP_
