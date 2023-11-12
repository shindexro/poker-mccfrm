#include "abstraction/terminal_state.h"

using namespace std;

namespace poker
{
    const string TerminalState::type = "Terminal";

    TerminalState::TerminalState() : State()
    {
    }

    TerminalState::TerminalState(CommunityInfo &community,
                                 vector<PlayerInfo> &players,
                                 vector<Action> &history) : State(community, players, history),
                                                            rewardGenerated{false}
    {
    }

    void TerminalState::CreateRewards()
    {
        for (auto i = 0; i < Global::nofPlayers; ++i)
        {
            players[i].reward -= players[i].bet; // the bet amounts are considered lost
        }

        if (GetNumberOfActivePlayers() == 1)
        {
            for (auto i = 0; i < Global::nofPlayers; ++i)
            {
                if (players[i].isStillInGame)
                {
                    players[i].reward += GetPot();
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
                handValues[i] = Global::handEvaluator.Evaluate(cardsBitmask);
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
                players[playersWithBestHand[i]].reward += GetPot() / playersWithBestHand.size();
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
} // namespace poker