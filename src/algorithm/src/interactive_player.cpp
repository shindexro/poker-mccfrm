#include "algorithm/interactive_player.h"

namespace poker
{
    InteractivePlayer::InteractivePlayer(int id, int stack) : Player(id, stack)
    {
    }

    Action InteractivePlayer::NextAction(shared_ptr<PlayState> state)
    {
        auto validActions = state->GetValidActions();

        std::cout << "Enter a integer bet size or fold (-1): ";
        int bet;
        std::cin >> bet;

        auto action = TranslateAction(state, bet);
        std::cout << "Mapped to abstraction as " << action << std::endl;
        return action;
    }

    Action InteractivePlayer::TranslateAction(shared_ptr<PlayState> state, int actualBet)
    {
        auto abstractActions = state->GetValidActions();
        auto abstractBets = state->GetBetSizes();

        for (auto i = 0ul; i < abstractBets.size() - 1; i++)
        {
            if (abstractBets[i] >= abstractBets[i + 1])
            {
                throw invalid_argument("Abstract bets should be in ascending order.");
            }
        }

        auto it = lower_bound(abstractBets.begin(), abstractBets.end(), actualBet);
        auto idx = it - abstractBets.begin();
        if (*it == actualBet)
        {
            return abstractActions[idx];
        }

        float lowBet = abstractBets[idx - 1];
        float highBet = abstractBets[idx];

        // random pseudo-harmonic action translation
        float lowBetProb = (highBet - actualBet) * (1 + lowBet) / (highBet - lowBet) / (1 + actualBet);
        if (randDouble() < lowBetProb)
        {
            return abstractActions[idx - 1];
        }
        else
        {
            return abstractActions[idx];
        }
    }
} // namespace poker
