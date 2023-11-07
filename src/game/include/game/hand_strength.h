#ifndef __CLASS_HAND_STRENGTH_H__
#define __CLASS_HAND_STRENGTH_H__

#include "game/hand.h"
#include "enums/hand_ranking.h"
#include <vector>
#include <string>

using namespace std;

class HandStrength
{
public:
    HandRanking handRanking;
    vector<Rank> kickers;

    HandStrength();

    int Compare(const HandStrength &other) const;
    bool operator<(const HandStrength &rhs) const;
    bool operator==(const HandStrength &rhs) const;
};

#endif
