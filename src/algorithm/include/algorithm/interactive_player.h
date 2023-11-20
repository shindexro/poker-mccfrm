#ifndef __CLASS_INTERACTIVE_PLAYER_H__
#define __CLASS_INTERACTIVE_PLAYER_H__

#include "abstraction/state.h"
#include "enums/action.h"
#include "algorithm/player.h"

namespace poker
{
    class InteractivePlayer : public Player
    {
    public:
        InteractivePlayer(int id);

        Action NextAction(PlayState &state) override;
    };
} // namespace poker

#endif
