#ifndef __ENUM_BETTING_ROUND_H__
#define __ENUM_BETTING_ROUND_H__

#include <string>

enum BettingRound
{
    Preflop,
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
#undef PROCESS_VAL
    }();
    return out << s;
}

#endif
