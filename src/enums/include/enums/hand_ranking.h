#ifndef __ENUM_HAND_RANKING_H__
#define __ENUM_HAND_RANKING_H__

#include <ostream>
#include <string>

namespace poker
{
    enum HandRanking
    {
        HighCard,
        Pair,
        TwoPair,
        ThreeOfAKind,
        Straight,
        Flush,
        FullHouse,
        FourOfAKind,
        StraightFlush,
    };

    std::ostream &operator<<(std::ostream &out, const HandRanking &value);
} // namespace poker
#endif
