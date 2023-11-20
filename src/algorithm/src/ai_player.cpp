#include "algorithm/ai_player.h"

namespace poker
{
    AIPlayer::AIPlayer(int id) : Player(id)
    {
    }

    Action AIPlayer::NextAction(PlayState &state)
    {
        //// TODO: implement blueprint lookup, real-time search, action translation
        return Action::Call;
    }
} // namespace poker
