#include "algorithm/ai_player.h"

namespace poker
{
    AIPlayer::AIPlayer(int id, int stack) : Player(id, stack)
    {
    }

    Action AIPlayer::NextAction(PlayState &state)
    {
        auto validActions = state.GetValidActions();
        if (!validActions.size())
        {
            throw invalid_argument("There are no valid actions.");
        }

        auto infoset = state.GetInfoset();
        if (state.BettingRound() == BettingRound::Preflop)
        {
            // use average strategy
            auto phi = infoset.GetFinalStrategy();
            auto actionIdx = utils::SampleDistribution(phi);
            return validActions[actionIdx];
        }
        else
        {
            /// TODO: use real-time search
            auto sigma = infoset.CalculateStrategy();
            auto actionIdx = utils::SampleDistribution(sigma);
            return validActions[actionIdx];
        }
    }
} // namespace poker
