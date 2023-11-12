#include "abstraction/player_info.h"

using namespace std;

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

    ulong PlayerInfo::GetCardBitmask() const
    {
        return get<0>(cards) | get<1>(cards);
    }

    ostream &operator<<(ostream &out, const PlayerInfo &info)
    {
        cout << info.stack << " " << info.bet << " " << info.reward << " "
             << info.isStillInGame << " " << info.lastAction << " "
             << Hand(info.GetCardBitmask()).ToString();
    }
}
