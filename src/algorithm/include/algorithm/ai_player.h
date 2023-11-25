#ifndef __CLASS_AI_PLAYER_H__
#define __CLASS_AI_PLAYER_H__

#include "abstraction/state.h"
#include "enums/action.h"
#include "algorithm/player.h"
#include "utils/random.h"

#include <stdexcept>

namespace poker
{
    class AIPlayer : public Player
    {
    public:
        AIPlayer(int id, int stack);

        Action NextAction(PlayState &state) override;
    };
} // namespace poker

#endif
