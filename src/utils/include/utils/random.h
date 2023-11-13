#ifndef __RANDOM_H__
#define __RANDOM_H__

#include <random>

// random integer in range [low, high)
inline int randint(int low, int high)
{
    static std::random_device dev;
    static std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> distribution(low, high - 1);
    return distribution(rng);
}

inline double randDouble()
{
    double lower_bound = 0;
    double upper_bound = 1;
    std::uniform_real_distribution<double> unif(lower_bound, upper_bound);
    std::default_random_engine re;
    return unif(re);
}

#endif
