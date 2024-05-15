#include "algorithm/ai_player.h"

namespace poker
{
    AIPlayer::AIPlayer(int id, int stack) : Player(id, stack)
    {
    }

    Action AIPlayer::NextAction(shared_ptr<PlayState> state, shared_ptr<PlayState> roundStartState)
    {
        auto validActions = state->GetValidActions();
        if (!validActions.size())
        {
            throw invalid_argument("There are no valid actions.");
        }

        if (state->BettingRound() == BettingRound::Preflop)
        {
            // use average strategy
            auto infoset = state->GetInfoset();
            auto phi = infoset.GetFinalStrategy();
            auto actionIdx = SampleDistribution(phi);
            return validActions[actionIdx];
        }

        // use real-time search based on blue-print strategy
        auto trainer = Trainer(0);
        /// TOOD: make number for iterations more dynamic?
        for (int i = 0; i < 10000; i++)
        {
            trainer.TraverseMCCFR(roundStartState, id, false);
        }

        auto infoset = state->GetInfoset();
        auto sigma = infoset.CalculateStrategy();
        auto actionIdx = SampleDistribution(sigma);
        return validActions[actionIdx];
    }
} // namespace poker
