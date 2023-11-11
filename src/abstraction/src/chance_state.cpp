#include "abstraction/chance_state.h"

namespace poker
{
    ChanceState::ChanceState()
    {
        for (auto i = 0; i < Global::nofPlayers; ++i)
        {
            players[i].isStillInGame = true;
            players[i].stack = Global::buyIn;
            players[i].lastAction = Action::NONE;
        }
        players[0].bet = Global::SB;
        players[1].bet = Global::BB;
        players[0].stack = Global::buyIn - Global::SB;
        players[1].stack = Global::buyIn - Global::BB;
        community.bettingRound = 0;
        community.lastPlayer = 1; // initially the BB player is last to act
        community.minRaise = Global::BB;
    }

    ChanceState::ChanceState(CommunityInfo &community,
                             vector<PlayerInfo> &players,
                             vector<Action> &history) : State(community, players, history)
    {
    }

    void ChanceState::CreateChildren()
    {
        if (children.size() != 0)
            return;

        auto newPlayers = vector<PlayerInfo>(players);
        auto newCommunity = CommunityInfo(community);

        // create one playstate child after chance
        int lastToMoveTemp = -1;
        int minRaiseTemp = Global::BB;
        int bettingRound = community.bettingRound + 1;
        if (community.bettingRound == 0)
        {
            for (auto i = 2 % Global::nofPlayers;; i = (i + 1) % Global::nofPlayers)
            {
                if (players[i].isStillInGame && players[i].lastAction != Action::ALLIN)
                {
                    lastToMoveTemp = i;
                }
                if (i == 1)
                    break;
            }
        }
        else if (community.bettingRound > 0)
        {
            for (auto i = 0; i < Global::nofPlayers; ++i)
            {
                if (players[i].isStillInGame && players[i].stack != 0)
                {
                    lastToMoveTemp = i;
                }
            }
        }

        switch (community.bettingRound)
        {
        case BettingRound::Preflop:
            Global::deck.Shuffle();
            for (auto i = 0; i < Global::nofPlayers; ++i)
            {
                newPlayers[i].cards = {Global::deck.Peek(i * 2), Global::deck.Peek(i * 2 + 1)};
            }
            break;
        case BettingRound::Flop:
            newCommunity.cards.push_back(Global::deck.Peek(Global::nofPlayers * 2 + 0));
            newCommunity.cards.push_back(Global::deck.Peek(Global::nofPlayers * 2 + 1));
            newCommunity.cards.push_back(Global::deck.Peek(Global::nofPlayers * 2 + 2));
            break;
        case BettingRound::Turn:
            newCommunity.cards.push_back(Global::deck.Peek(Global::nofPlayers * 2 + 3));
            break;
        case BettingRound::River:
            newCommunity.cards.push_back(Global::deck.Peek(Global::nofPlayers * 2 + 4));
            break;
        default:
            throw invalid_argument("Unknown betting round");
        }

        newCommunity.bettingRound = bettingRound;
        if (GetNumberOfPlayersThatNeedToAct() >= 2)
        {
            // there is someone left that plays
            newCommunity.isBettingOpen = true;
            newCommunity.lastPlayer = lastToMoveTemp;
            newCommunity.minRaise = minRaiseTemp;
            children.push_back(make_shared<PlayState>(newCommunity, newPlayers, history));
        }
        else
        {
            if (GetNumberOfPlayersThatNeedToAct() == 1)
            {
                throw invalid_argument("We just dealt new cards but only 1 player has any actions left");
            }
            if (community.bettingRound < BettingRound::River && GetNumberOfAllInPlayers() >= 2)
            {
                // directly go to next chance node
                children.push_back(make_shared<ChanceState>(newCommunity, newPlayers, history));
            }
            else
            {
                children.push_back(make_shared<TerminalState>(newCommunity, newPlayers, history));
            }
        }
    }

    /// <summary>
    /// Note: The single child was already randomly created
    /// </summary>
    /// <returns></returns>
    shared_ptr<State> ChanceState::DoRandomAction()
    {
        CreateChildren();
        return children[0];
    }

    vector<shared_ptr<PlayState>> ChanceState::GetFirstActionStates()
    {
        auto gameStates = vector<shared_ptr<PlayState>>();

        auto newCommunity = CommunityInfo(community);
        // create one playstate child after chance
        int lastToMoveTemp = -1;
        int minRaiseTemp = Global::BB;
        int bettingRound = community.bettingRound + 1;

        if (community.bettingRound == 0)
        {
            for (auto i = 2 % Global::nofPlayers;; i = (i + 1) % Global::nofPlayers)
            {
                if (players[i].isStillInGame && players[i].lastAction != Action::ALLIN)
                {
                    lastToMoveTemp = i;
                }
                if (i == 1)
                    break;
            }
        }
        else if (community.bettingRound > 0)
        {
            for (auto i = 0; i < Global::nofPlayers; ++i)
            {
                if (players[i].isStillInGame && players[i].stack != 0)
                {
                    lastToMoveTemp = i;
                }
            }
        }

        vector<Hand> startingHands = utils::GetStartingHandChart();

        for (auto i = 0; i < Global::RANKS * Global::RANKS; ++i)
        {
            auto newPlayers = vector<PlayerInfo>(players);
            players[0].cards = {startingHands[i].cards[0].Bitmask(), startingHands[i].cards[1].Bitmask()};
            newCommunity.cards = vector<ulong>();
            newCommunity.isBettingOpen = true;
            newCommunity.lastPlayer = lastToMoveTemp;
            newCommunity.minRaise = minRaiseTemp;
            newCommunity.bettingRound = bettingRound;
            gameStates.push_back(make_shared<PlayState>(newCommunity, newPlayers, history));
        }

        return gameStates;
    }

    bool ChanceState::IsPlayerInHand(int player)
    {
        return players[player].isStillInGame;
    }
} // namespace poker
