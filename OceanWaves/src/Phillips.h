#pragma once


class Philipps {

public:

    Philipps(const double, const double, const int, const int, const double, const int, const double, const double);
    Philipps() {};
    ~Philipps() {}

    const double operator()();

    void init_fonctor(const int);

private:

    double lx;               /* actual width of the scene */
    double ly;               /* actual height of the scene */
    int    nx;               /* nb of x points - must be a power of 2 */
    int    ny;               /* nb of y points - must be a power of 2 */

    double wind_speed;       /* wind speed */
    int    wind_alignment;   /* the greater it is, the better waves are in the wind's direction */
    double min_wave_size;    /* waves are deleted if below this size */
    double A;                /* numeric constant to adjust the waves height */

    int x;                         /* equals i-nx/2 in a series of calls to the functor  */
    int y;                         /* goes in the range [-ny/2 ; ny/2] in a series of calls to the functor */

};

