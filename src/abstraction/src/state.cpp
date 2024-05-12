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
                                            infosetString()
    {
    }

    // The next player to act in the round
    int State::NextActivePlayer()
    {
        for (auto i = (community.playerToMove + 1) % Global::nofPlayers;
             i != (community.lastPlayer + 1) % Global::nofPlayers;
             i = (i + 1) % Global::nofPlayers)
        {
            if (players[i].IsAlive() && players[i].lastAction != Action::Allin)
            {
                return i;
            }
        }
        return -1;
    }

    // The last remaining player to act in the round
    int State::PrevActivePlayer()
    {
        int last = -1;
        for (auto i = (community.playerToMove + 1) % Global::nofPlayers;
             i != community.playerToMove;
             i = (i + 1) % Global::nofPlayers)
        {
            if (players[i].IsAlive() && players[i].lastAction != Action::Allin)
            {
                last = i;
            }
        }
        return last;
    }

    // The first player to act in the round, starting from SB
    int State::FirstActivePlayer()
    {
        for (int i = 0; i < Global::nofPlayers; i++)
        {
            if (players[i].IsAlive() && players[i].lastAction != Action::Allin)
            {
                return i;
            }
        }
        return -1;
    }

    // Number of players still in the game and didn't all in
    int State::GetNumberOfPlayersThatNeedToAct()
    {
        int count = 0;
        for (auto i = 0; i < Global::nofPlayers; i++)
        {
            if (players[i].IsAlive() && players[i].lastAction != Action::Allin)
                count++;
        }
        return count;
    }

    int State::GetNumberOfActivePlayers()
    {
        return count_if(players.begin(), players.end(), [](PlayerInfo &p)
                        { return p.IsAlive(); });
    }

    int State::GetNumberOfAllInPlayers()
    {
        return count_if(players.begin(), players.end(), [](PlayerInfo &p)
                        { return p.lastAction == Action::Allin; });
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
            std::cout << "│ ";

        if (children.size())
            std::cout << "├─┬─";
        else
            std::cout << "├───";

        std::cout << *this << std::endl;

        for (auto child : children)
        {
            child->PrettyPrintTree(depth + 1);
        }
    }

    ostream &State::Print(ostream &out) const
    {
        out << community << " | ";
        for (auto &player : players)
            out << player << " ";
        out << " ";
        for (auto action : history)
            out << action << " ";
        return out;
    }

    ostream &State::PrettyPrint(ostream &out) const
    {
        for (auto card : community.cards)
        {
            out << Card(card);
        }
        out << std::endl;

        out << "Pot: " << GetPot() << std::endl;

        for (auto &player : players)
            player.PrettyPrint(out);
        return out;
    }

    ostream &operator<<(ostream &out, const State &state)
    {
        return state.Print(out);
    }
}
