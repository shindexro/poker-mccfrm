#include "enums/rank.h"

namespace poker
{
    std::ostream &operator<<(std::ostream &out, const Rank &value)
    {
        std::string s = [value]
        {
#define PROCESS_VAL(p) \
    case (p):          \
        return #p;
            switch (value)
            {
                PROCESS_VAL(Two);
                PROCESS_VAL(Three);
                PROCESS_VAL(Four);
                PROCESS_VAL(Five);
                PROCESS_VAL(Six);
                PROCESS_VAL(Seven);
                PROCESS_VAL(Eight);
                PROCESS_VAL(Nine);
                PROCESS_VAL(Ten);
                PROCESS_VAL(Jack);
                PROCESS_VAL(Queen);
                PROCESS_VAL(King);
                PROCESS_VAL(Ace);
            }
            return "N/A";
#undef PROCESS_VAL
        }();
        return out << s;
    }
} // namespace poker
