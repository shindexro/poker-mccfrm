#include "algorithm/ai_player.h"

namespace poker
{
    AIPlayer::AIPlayer(int id, int stack) : Player(id, stack)
    {
    }

    Action AIPlayer::NextAction(PlayState &state)
    {
        //// TODO: implement blueprint lookup, real-time search, action translation
        return Action::Call;
    }
} // namespace poker
