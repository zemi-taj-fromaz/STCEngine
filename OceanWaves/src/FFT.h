#pragma once

#include <iostream>
#include <vector>

class FFT {

public:

    FFT(const int, std::vector<double>* const, std::vector<double>* const);

    void direct() { sort(); radix_direct(); }
    void reverse() { sort(); radix_reverse(); }

private:

    typedef std::vector<double>* vec_d_p;

    void radix_direct();
    void radix_reverse();
    void sort();

    const int n;      /* power of two, the size of the vector */
    const int p;      /* so that n = 2^p */
    vec_d_p   real;   /* data vector, real values */
    vec_d_p   imag;   /* data vectorn imaginary values */

};
