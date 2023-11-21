#include "abstraction/player_info.h"

using namespace std;

namespace poker
{
    PlayerInfo::PlayerInfo() : stack{0},
                               bet{0},
                               reward{0},
                               isStillInGame{true},
                               cards(),
                               lastAction{Action::None}
    {
    }

    ulong PlayerInfo::GetCardBitmask() const
    {
        return get<0>(cards) | get<1>(cards);
    }

    ostream &PlayerInfo::PrettyPrint(ostream &out) const
    {
        out << Card(get<0>(cards)) << Card(get<1>(cards)) << " ";
        out << (isStillInGame ? "\033[1;32m" : "\033[1;31m");
        out << "stack: " << stack << "\t\t";
        out << "\033[0m";
        return out;
    }

    ostream &operator<<(ostream &out, const PlayerInfo &info)
    {
        out << "S:" << info.stack << " B:" << info.bet << " R:" << info.reward
            << " IG:" << info.isStillInGame << " A:" << info.lastAction
            << " H:" << Hand(info.GetCardBitmask()).ToString();
        return out;
    }
}
