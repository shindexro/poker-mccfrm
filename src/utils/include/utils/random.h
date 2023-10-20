#ifndef __RANDON_H__
#define __RANDON_H__

// random integer in range [low, high)
inline int randint(int low, int high)
{
    return low + (rand() % (high - low));
}

#endif
