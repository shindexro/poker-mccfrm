#include "abstraction/state.h"

namespace poker
{
    State::State() : community(),
                     players(Global::nofPlayers, PlayerInfo()),
                     history()
    {
    }

    State::State(CommunityInfo &community,
                 vector<PlayerInfo> &players,
                 vector<Action> &history) : community(community),
                                            players(players),
                                            history(history),
                                            children(),
                                            infosetString(),
                                            infosetStringGenerated{false}
    {
    }

    int State::GetNextPlayer()
    {
        return GetNextPlayer(community.lastPlayer);
    }

    int State::GetNextPlayer(int lastToMoveTemp)
    {
        for (auto i = (community.playerToMove + 1) % Global::nofPlayers; i != (lastToMoveTemp + 1) % Global::nofPlayers;
             i = (i + 1) % Global::nofPlayers)
        {
            if (players[i].isStillInGame && players[i].lastAction != Action::ALLIN)
            {
                return i;
            }
        }
        return -1;
    }

    int State::GetLastPlayer(int playerThatRaised)
    {
        int last = -1;
        for (auto i = (playerThatRaised + 1) % Global::nofPlayers; i != playerThatRaised % Global::nofPlayers;
             i = (i + 1) % Global::nofPlayers)
        {
            if (players[i].isStillInGame && players[i].lastAction != Action::ALLIN)
            {
                last = i;
            }
        }
        return last;
    }

    int State::GetNumberOfPlayersThatNeedToAct()
    {
        // does not include all-in players
        int count = 0;
        for (auto i = 0; i < Global::nofPlayers; i++)
        {
            if (players[i].isStillInGame == true && players[i].lastAction != Action::ALLIN)
                count++;
        }
        return count;
    }

    int State::GetActivePlayers()
    {
        return count_if(players.begin(), players.end(), [](PlayerInfo &p)
                        { return p.isStillInGame == Action::ALLIN; });
    }

    int State::GetNumberOfAllInPlayers()
    {
        return count_if(players.begin(), players.end(), [](PlayerInfo &p)
                        { return p.lastAction == Action::ALLIN; });
    }

    int State::GetPot() const
    {
        int bets = 0;
        for (auto &player : players)
        {
            bets += player.bet;
        }
        return bets;
    }

    int State::MinimumCall() const
    {
        int call = -1;
        for (auto &player : players)
        {
            call = max({call, player.bet});
        }
        return call;
    }

    int State::BettingRound()
    {
        return community.bettingRound;
    }

}
