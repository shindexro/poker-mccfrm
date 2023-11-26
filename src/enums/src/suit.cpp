#include "enums/suit.h"

namespace poker
{
    std::ostream &operator<<(std::ostream &out, const Suit &value)
    {
        std::string s = [value]
        {
#define PROCESS_VAL(p) \
    case (p):          \
        return #p;
            switch (value)
            {
                PROCESS_VAL(Spades);
                PROCESS_VAL(Hearts);
                PROCESS_VAL(Diamonds);
                PROCESS_VAL(Clubs);
            }
            return "N/A";
#undef PROCESS_VAL
        }();
        return out << s;
    }
} // namespace poker
