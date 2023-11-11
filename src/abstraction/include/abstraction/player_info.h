#ifndef __CLASS_PLAYER_INFO_H__
#define __CLASS_PLAYER_INFO_H__

#include "enums/action.h"

#include <tuple>

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

        ulong GetCardBitmask();
    };
}

#endif