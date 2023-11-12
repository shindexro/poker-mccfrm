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

    // The next player to act in the round
    int State::GetNextPlayer(int player)
    {
        for (auto i = (community.playerToMove + 1) % Global::nofPlayers;
             i != (player + 1) % Global::nofPlayers;
             i = (i + 1) % Global::nofPlayers)
        {
            if (players[i].isStillInGame && players[i].lastAction != Action::ALLIN)
            {
                return i;
            }
        }
        return -1;
    }

    // The last remaining player to act in the round
    int State::GetLastPlayer(int playerWhoRaised)
    {
        int last = -1;
        for (auto i = (playerWhoRaised + 1) % Global::nofPlayers;
             i != playerWhoRaised % Global::nofPlayers;
             i = (i + 1) % Global::nofPlayers)
        {
            if (players[i].isStillInGame && players[i].lastAction != Action::ALLIN)
            {
                last = i;
            }
        }
        return last;
    }

    // Number of players still in the game and didn't all in
    int State::GetNumberOfPlayersThatNeedToAct()
    {
        int count = 0;
        for (auto i = 0; i < Global::nofPlayers; i++)
        {
            if (players[i].isStillInGame == true && players[i].lastAction != Action::ALLIN)
                count++;
        }
        return count;
    }

    int State::GetNumberOfActivePlayers()
    {
        return count_if(players.begin(), players.end(), [](PlayerInfo &p)
                        { return p.isStillInGame; });
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

    void State::PrettyPrintTree(int depth)
    {
        for (int i = 0; i < depth; i++)
            cout << "  ";

        cout << "└" << *this << endl;

        for (auto child : children)
        {
            child->PrettyPrintTree();
        }
    }

    ostream &operator<<(ostream &out, const State &state)
    {
        out << state.community << " ";
        for (auto &player : state.players)
            out << player << " ";
        out << " ";
        for (auto action : state.history)
            out << action << " ";
    }

}
