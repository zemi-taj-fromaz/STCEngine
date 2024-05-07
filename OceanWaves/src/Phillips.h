#pragma once


class Philipps {

public:

    Philipps(const double, const double, const int, const int, const double, const int, const double, const double);
    ~Philipps() {}

    const double operator()();

    void init_fonctor(const int);

private:

    const double lx;               /* actual width of the scene */
    const double ly;               /* actual height of the scene */
    const int    nx;               /* nb of x points - must be a power of 2 */
    const int    ny;               /* nb of y points - must be a power of 2 */

    const double wind_speed;       /* wind speed */
    const int    wind_alignment;   /* the greater it is, the better waves are in the wind's direction */
    const double min_wave_size;    /* waves are deleted if below this size */
    const double A;                /* numeric constant to adjust the waves height */

    int x;                         /* equals i-nx/2 in a series of calls to the functor  */
    int y;                         /* goes in the range [-ny/2 ; ny/2] in a series of calls to the functor */

};

