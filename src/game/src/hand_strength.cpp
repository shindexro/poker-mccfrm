#include "game/hand_strength.h"

int HandStrength::Compare(HandStrength &other)
{
    if (this->handRanking > other.handRanking)
        return 1;
    else if (this->handRanking < other.handRanking)
        return -1;

    for (auto i = 0; i < this->kickers.size(); i++)
    {
        if (this->kickers[i] > other.kickers[i])
            return 1;
        else if (this->kickers[i] < other.kickers[i])
            return -1;
    }
    return 0;
}
