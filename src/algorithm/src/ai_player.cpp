#include "algorithm/ai_player.h"

namespace poker
{
    AIPlayer::AIPlayer(int id, int stack) : Player(id, stack)
    {
    }

    Action AIPlayer::NextAction(PlayState &state)
    {
        //// TODO: implement blueprint lookup, real-time search, action translation
        auto validActions = state.GetValidActions();
        if (!validActions.size())
        {
            throw invalid_argument("There are no valid actions.");
        }
        return validActions[0];
    }
} // namespace poker
