#ifndef __CLASS_PLAYER_INFO_H__
#define __CLASS_PLAYER_INFO_H__

#include "enums/action.h"
#include "game/hand.h"

#include <tuple>
#include <iostream>

typedef unsigned long ulong;

using namespace std;

namespace poker
{
    class PlayerInfo
    {
    public:
        int stack;
        int bet;
        int reward;
        bool isStillInGame;
        tuple<ulong, ulong> cards;
        Action lastAction;

        PlayerInfo();

        ulong GetCardBitmask() const;

        ostream &PrettyPrint(ostream &out) const;
        friend ostream &operator<<(ostream &out, const PlayerInfo &info);
    };
}

#endif