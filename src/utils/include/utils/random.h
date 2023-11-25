#ifndef __RANDOM_H__
#define __RANDOM_H__

#include <random>
#include <iostream>

// random integer in range [low, high)
inline int randint(int low, int high)
{
    static std::random_device dev;
    static std::seed_seq seed{dev(), dev(), dev(), dev()};
    static std::mt19937 rng(seed);
    std::uniform_int_distribution<std::mt19937::result_type> distribution(low, high - 1);
    return distribution(rng);
}

inline double randDouble()
{
    static const double lower_bound = 0;
    static const double upper_bound = 1;
    static std::default_random_engine re;
    std::uniform_real_distribution<double> unif(lower_bound, upper_bound);
    return unif(re);
}

#endif
