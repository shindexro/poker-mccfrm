#ifndef __CLASS_INTERACTIVE_PLAYER_H__
#define __CLASS_INTERACTIVE_PLAYER_H__

#include "abstraction/state.h"
#include "enums/action.h"
#include "algorithm/player.h"
#include "utils/random.h"

#include <stdexcept>

namespace poker
{
    class InteractivePlayer : public Player
    {
    public:
        InteractivePlayer(int id, int stack);

        Action NextAction(shared_ptr<PlayState> state) override;
        Action NextAction(shared_ptr<PlayState> state, shared_ptr<PlayState>) override;

    private:
        Action TranslateAction(shared_ptr<PlayState> state, int actualBet);
    };
} // namespace poker

#endif
