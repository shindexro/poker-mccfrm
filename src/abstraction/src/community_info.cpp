#include "abstraction/community_info.h"

namespace poker
{
    CommunityInfo::CommunityInfo() : bettingRound{BettingRound::Preflop},
                                     playerToMove{2 % Global::nofPlayers},
                                     lastPlayer{1},
                                     minRaise{Global::BB},
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
            << "pMove:" << info.playerToMove << " "
            << "pLast:" << info.lastPlayer << " "
            << "minR:" << info.minRaise << " "
            << Hand(info.GetCardBitmask()).ToString();
        return out;
    }
}
