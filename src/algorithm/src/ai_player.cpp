#include "algorithm/ai_player.h"

namespace poker
{
    AIPlayer::AIPlayer(int id, int stack) : Player(id, stack)
    {
    }

    Action AIPlayer::NextAction(shared_ptr<PlayState> state)
    {
        auto validActions = state->GetValidActions();
        if (!validActions.size())
        {
            throw invalid_argument("There are no valid actions.");
        }

        auto infoset = state->GetInfoset();
        if (state->BettingRound() == BettingRound::Preflop)
        {
            // use average strategy
            auto phi = infoset.GetFinalStrategy();
            auto actionIdx = utils::SampleDistribution(phi);
            return validActions[actionIdx];
        }
        else
        {
            // use real-time search
            auto trainer = Trainer(0);
            for (int i = 0; i < 10000; i++)
            {
                trainer.TraverseMCCFR(state, id, false);
            }

            auto sigma = infoset.CalculateStrategy();
            auto actionIdx = utils::SampleDistribution(sigma);
            return validActions[actionIdx];
        }
    }
} // namespace poker
