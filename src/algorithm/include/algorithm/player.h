#ifndef __CLASS_PLAYER_H__
#define __CLASS_PLAYER_H__

#include "abstraction/state.h"
#include "abstraction/play_state.h"
#include "enums/action.h"

namespace poker
{
    class Player
    {
    public:
        const int id;

        Player(int id);

        // the whole state is passed for simlicity but a player should not access other player's hand
        virtual Action NextAction(PlayState &state) { throw invalid_argument("Not implemented"); };
    };
} // namespace poker

#endif
