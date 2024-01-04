#include "abstraction/terminal_state.h"

using namespace std;

namespace poker
{
    TerminalState::TerminalState() : State()
    {
        evaluator = Global::handEvaluator;
    }

    TerminalState::TerminalState(CommunityInfo &community,
                                 vector<PlayerInfo> &players,
                                 vector<Action> &history) : State(community, players, history),
                                                            rewardGenerated{false}
    {
        evaluator = Global::handEvaluator;
    }

    void TerminalState::CreateRewards()
    {
        for (auto i = 0; i < Global::nofPlayers; ++i)
        {
            players[i].reward -= players[i].bet; // the bet amounts are considered lost
        }

        auto pot = GetPot();
        pot = (1.0f - Global::rake) * pot;

        if (GetNumberOfActivePlayers() == 1)
        {
            for (auto i = 0; i < Global::nofPlayers; ++i)
            {
                if (players[i].isStillInGame)
                {
                    players[i].reward += pot;
                }
            }
        }
        else
        {
            // at least 2 players are in
            auto handValues = vector<int>(Global::nofPlayers, -1);
            for (auto i = 0; i < Global::nofPlayers; ++i)
            {
                if (!players[i].isStillInGame)
                    continue;

                ulong cardsBitmask = players[i].GetCardBitmask() | community.GetCardBitmask();
                handValues[i] = evaluator->Evaluate(cardsBitmask);
            }
            auto playersWithBestHand = vector<int>();
            auto maxHandValue = *max_element(handValues.begin(), handValues.end());

            for (int i = 0; i < Global::nofPlayers; i++)
            {
                if (handValues[i] == maxHandValue)
                {
                    playersWithBestHand.push_back(i);
                }
            }

            // winners chop the pot
            for (auto i = 0UL; i < playersWithBestHand.size(); ++i)
            {
                players[playersWithBestHand[i]].reward += pot / playersWithBestHand.size();
            }
        }
        rewardGenerated = true;
    }

    float TerminalState::GetReward(int player)
    {
        if (!rewardGenerated)
            CreateRewards();
        // TODO: add this check to test
        // if (accumulate(rewards.begin(), rewards.end(), 0) != 0)
        //     throw invalid_argument("Wrong reward calculation");
        return players[player].reward;
    }

    void TerminalState::CreateChildren()
    {
        // do nothing
    }

    ostream &TerminalState::Print(ostream &out) const
    {
        out << "Terminal | ";
        return State::Print(out);
    }

} // namespace poker
