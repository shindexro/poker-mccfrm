#ifndef __CLASS_INFOSET_H__
#define __CLASS_INFOSET_H__

#include "enums/betting_round.h"
#include <vector>
#include <string>
#include <cereal/archives/binary.hpp>

using namespace std;

namespace poker
{
    class Infoset
    {
    public:
        vector<int> regret;
        vector<int> actionCounter;

        Infoset();
        Infoset(int actions);
        Infoset(int actions, BettingRound round);

        vector<float> CalculateStrategy();
        vector<float> GetFinalStrategy();
    };
} // namespace poker
#endif
