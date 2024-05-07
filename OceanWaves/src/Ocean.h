#pragma once


#include <vector>

#include "FFT.h"
#include "Height.h"
#include "Phillips.h"

class Ocean {

public:

    Ocean(const double, const double, const int, const int, const double);
    ~Ocean();

    const int get_lx() { return lx; }
    const int get_ly() { return ly; }
    const int get_nx() { return nx; }
    const int get_ny() { return ny; }

    void generate_height(Height* const);
    void main_computation(float);
    void init_gl_vertex_array_x(const int, double* const) const;
    void init_gl_vertex_array_y(const int, double* const) const;
    void gl_vertex_array_x(const int, double* const)      const;
    void gl_vertex_array_y(const int, double* const)      const;

    typedef std::vector<double>::iterator              vec_d_it;
    typedef std::vector<std::vector<double>>           vec_vec_d;
    typedef std::vector<std::vector<double>>::iterator vec_vec_d_it;

    void get_sine_amp(const int, const double, std::vector<double>* const, std::vector<double>* const) const;

    const double      lx;              /* actual width */
    const double      ly;              /* actual height */
    const int         nx;              /* nb of x points - must be a power of 2 */
    const int         ny;              /* nb of y points - must be a power of 2 */
    const double      motion_factor;

    vec_vec_d         height0R;        /* initial wave height field (spectrum) - real part */
    vec_vec_d         height0I;        /* initial wave height field (spectrum) - imaginary part */


    vec_vec_d         HR;              /* frequency domain, real part      - [y][x] */
    vec_vec_d         HI;              /* frequency domain, imaginary part - [y][x] */
    vec_vec_d         hr;              /* time domain, real part      - [y][x] */
    vec_vec_d         hi;              /* time domain, imaginary part - [y][x] */

    std::vector<FFT*> fftx;            /* fft structure to compute the FFT */
    std::vector<FFT*> ffty;            /* fft structure to compute the FFT */


};
