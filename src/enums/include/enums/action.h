#ifndef __ENUM_ACTION_H__
#define __ENUM_ACTION_H__

namespace poker
{
    enum Action
    {
        NONE,
        FOLD,
        // CHECK, combined with CALL, basically calling a 0 raise
        CALL,
        RAISE,
        RAISE1,
        RAISE2,
        RAISE3,
        RAISE4,
        RAISE5,
        RAISE6,
        ALLIN
    };
} // namespace poker

#endif
