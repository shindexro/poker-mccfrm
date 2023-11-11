#include "abstraction/player_info.h"

namespace poker
{
    PlayerInfo::PlayerInfo() : stack{0},
                               bet{0},
                               reward{0},
                               isStillInGame{true},
                               cards(),
                               lastAction{Action::NONE}
    {
    }

    ulong PlayerInfo::GetCardBitmask()
    {
        return get<0>(cards) | get<1>(cards);
    }
}
