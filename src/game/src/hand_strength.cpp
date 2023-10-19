#include "game/hand_strength.h"

int HandStrength::Compare(const HandStrength &other) const
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

bool HandStrength::operator<(const HandStrength &rhs) const
{
    return this->Compare(rhs) < 0;
}
