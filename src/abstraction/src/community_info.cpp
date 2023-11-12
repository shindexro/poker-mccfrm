#include "abstraction/community_info.h"

namespace poker
{
    CommunityInfo::CommunityInfo() : bettingRound{0},
                                     playerToMove{2 % Global::nofPlayers},
                                     lastPlayer{1},
                                     minRaise{Global::BB},
                                     isBettingOpen{false},
                                     actionCount{-1},
                                     cards()
    {
    }

    ulong CommunityInfo::GetCardBitmask() const
    {
        ulong bitmask = 0ul;
        for (auto card : cards)
        {
            bitmask |= card;
        }
        return bitmask;
    }

    ostream &operator<<(ostream &out, const CommunityInfo &info)
    {
        out << info.bettingRound << " "
            << info.playerToMove << " "
            << info.lastPlayer << " "
            << info.minRaise << " "
            << info.isBettingOpen << " "
            << info.actionCount << " "
            << Hand(info.GetCardBitmask()).ToString();
        return out;
    }
}
