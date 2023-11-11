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
}
