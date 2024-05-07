#pragma once

#include <iostream>
#include <vector>

#include "Phillips.h"

class Height {

public:

    static const double gaussian();

    Height(const int, const int);
    ~Height() {}

    const double operator()();

    void init_fonctor(const int);
    void generate_philipps(Philipps* const);

private:

    typedef std::vector<std::vector<double>> vec_vec_d;

    const int nx;        /* nb of x points - must be a power of 2 */
    const int ny;        /* nb of y points - must be a power of 2 */
    vec_vec_d philipps;  /* Philips spectrum _philipps[y][x] */
    int       x;
    int       y;

};

