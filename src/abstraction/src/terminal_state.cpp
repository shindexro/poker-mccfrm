#include "abstraction/terminal_state.h"

using namespace std;

namespace poker
{
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

        if (GetActivePlayers() == 1)
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

                ulong cardsBitmask = players[i].GetCardBitmask();
                for (auto k = 0UL; k < community.cards.size(); ++k)
                {
                    cardsBitmask |= community.cards[k];
                }
                handValues[i] = Global::handEvaluator.Evaluate(cardsBitmask);
            }
            // temphandval contains values of each players hand who is in
            auto indicesWithBestHands = vector<int>();

            auto maxIt = max_element(handValues.begin(), handValues.end());
            int maxVal = *maxIt;
            int maxIndex = maxIt - handValues.begin();
            while (maxIt != handValues.end())
            {
                indicesWithBestHands.push_back(maxIndex);
                *maxIt = -1;
                maxIt = find(handValues.begin(), handValues.end(), maxVal);
            }
            for (auto i = 0UL; i < indicesWithBestHands.size(); ++i)
            {
                players[indicesWithBestHands[i]].reward += GetPot() / indicesWithBestHands.size();
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
} // namespace poker