#ifndef __ENUM_BETTING_ROUND_H__
#define __ENUM_BETTING_ROUND_H__

#include <string>

namespace poker
{
    enum BettingRound
    {
        Preflop = 0,
        Flop,
        Turn,
        River
    };

    std::ostream &operator<<(std::ostream &out, const BettingRound &value)
    {
        std::string s = [value]
        {
#define PROCESS_VAL(p) \
    case (p):          \
        return #p;
            switch (value)
            {
                PROCESS_VAL(Preflop);
                PROCESS_VAL(Flop);
                PROCESS_VAL(Turn);
                PROCESS_VAL(River);
            }
            return "N/A";
#undef PROCESS_VAL
        }();
        return out << s;
    }

    BettingRound &operator++(BettingRound &round)
    {
        return round = static_cast<BettingRound>(static_cast<int>(round) + 1);
    }
} // namespace poker
#endif
