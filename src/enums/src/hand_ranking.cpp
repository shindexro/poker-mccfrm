#include "enums/hand_ranking.h"

namespace poker
{
    std::ostream &operator<<(std::ostream &out, const HandRanking &value)
    {
        std::string s = [value]
        {
#define PROCESS_VAL(p) \
    case (p):          \
        return #p;
            switch (value)
            {
                PROCESS_VAL(HighCard);
                PROCESS_VAL(HandRanking::Pair);
                PROCESS_VAL(TwoPair);
                PROCESS_VAL(ThreeOfAKind);
                PROCESS_VAL(Straight);
                PROCESS_VAL(Flush);
                PROCESS_VAL(FullHouse);
                PROCESS_VAL(FourOfAKind);
                PROCESS_VAL(StraightFlush);
            }
            return "N/A";
#undef PROCESS_VAL
        }();
        return out << s;
    }
} // namespace poker
