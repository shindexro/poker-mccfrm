#ifndef __CLASS_COMMUNITY_INFO_H__
#define __CLASS_COMMUNITY_INFO_H__

#include "abstraction/global.h"

#include <string>

typedef unsigned long ulong;

using namespace std;

namespace poker
{
    class CommunityInfo
    {
    public:
        
        int bettingRound;
        int playerToMove;
        int lastPlayer;
        int minRaise;
        bool isBettingOpen;
        int actionCount;
        vector<ulong> cards;

        CommunityInfo();

        ulong GetCardBitmask() const;
    };

}

#endif