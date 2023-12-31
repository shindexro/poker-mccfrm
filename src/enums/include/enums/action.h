#ifndef __ENUM_ACTION_H__
#define __ENUM_ACTION_H__

#include <string>
#include <ostream>

namespace poker
{
    enum Action
    {
        None,
        Fold,
        // Check, combined with Call, basically calling a 0 Raise
        Call,
        Raise,
        Raise1,
        Raise2,
        Raise3,
        Raise4,
        Raise5,
        Raise6,
        Raise7,
        Raise8,
        Raise9,
        Allin
    };

    std::ostream &operator<<(std::ostream &out, const Action &value);
} // namespace poker

#endif
