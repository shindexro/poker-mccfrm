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

    int PlayState::GetValidActionsCount()
    {
        if (community.actionCount != -1)
            return community.actionCount;

        if (children.size() != 0)
            community.actionCount = children.size();
        else
            community.actionCount = GetValidActions().size();

        return community.actionCount;
    }

    vector<Action> PlayState::GetValidActions()
    {
        auto validActions = vector<Action>();
        int pot = GetPot();
        int currentCall = MinimumCall();

        if (GetActivePlayers() == 0)
        {
            throw invalid_argument("There must always be >= one player in hand");
        }
        if (GetActivePlayers() == 1)
        {
            return validActions; // no valid actions
        }

        // raises
        if (community.isBettingOpen)
        {
            int raise;
            for (auto i = 0UL; i < Global::raiseRatios.size(); ++i)
            {
                raise = (int)(Global::raiseRatios[i] * pot);
                int actualRaise = raise - (currentCall - players[community.playerToMove].bet);

                if (actualRaise < community.minRaise || raise >= players[community.playerToMove].stack)
                    continue;

                // valid raise, if stack is equal it would be an all in
                if (i == 0)
                    validActions.push_back(Action::RAISE1);
                if (i == 1)
                    validActions.push_back(Action::RAISE2);
                if (i == 2)
                    validActions.push_back(Action::RAISE3);
            }

            raise = players[community.playerToMove].stack;
            // all-in
            if (raise > 0)
            {
                //(currently, multiple all-ins in a row dont accumulate the raises and reopen betting round but probably they should)
                // int actualRaise = (raise + players[community.playerToMove].bet) - currentCall;
                validActions.push_back(Action::ALLIN);
            }
        }
        if (currentCall > players[community.playerToMove].bet)
        {
            // fold
            validActions.push_back(Action::FOLD);
        }
        if (currentCall - players[community.playerToMove].bet < players[community.playerToMove].stack)
        {
            // call
            validActions.push_back(Action::CALL);
        }

        return validActions;
    }

    bool PlayState::IsPlayerTurn(int player)
    {
        return community.playerToMove == player;
    }

    bool PlayState::IsPlayerInHand(int player)
    {
        return players[player].isStillInGame;
    }

    Infoset PlayState::GetInfoset()
    {
        // Betting history R, A, CH, C, F
        // Player whose turn it is // not needed?
        // Cards of player whose turn it is
        // community cards

        if (infosetStringGenerated == false)
        {
            string historyString = "";
            for (auto h : history)
            {
                historyString += h;
            }

            auto cards = vector<int>{
                Card::GetIndexFromBitmask(get<0>(players[community.playerToMove].cards)),
                Card::GetIndexFromBitmask(get<1>(players[community.playerToMove].cards))};
            for (auto i = 0UL; i < community.cards.size(); ++i)
            {
                cards.push_back(Card::GetIndexFromBitmask(community.cards[i]));
            }

            string cardString = "";
            if (community.cards.size() == 0)
            {
                long index = Global::indexer_2.IndexLastRound(cards);
                cardString += "P" + to_string(index);
            }
            else if (community.cards.size() == 3)
            {
                long index = EMDTable::flopIndices[Global::indexer_2_3.IndexLastRound(cards)];
                cardString += "F" + to_string(index);
            }
            else if (community.cards.size() == 4)
            {
                long index = EMDTable::turnIndices[Global::indexer_2_4.IndexLastRound(cards)];
                cardString += "T" + to_string(index);
            }
            else
            {
                long index = OCHSTable::riverIndices[Global::indexer_2_5.IndexLastRound(cards)];
                cardString += "R" + to_string(index);
            }
            infosetString = historyString + cardString;
            infosetStringGenerated = true;
        }

        NodeMapAccessor accessor;
        bool nodeMapEntryExists = Global::nodeMap.find(accessor, infosetString);
        if (!nodeMapEntryExists)
        {
            Infoset infoset = Infoset(GetValidActionsCount());
            Global::nodeMap.insert({infosetString, infoset});
        }
        Global::nodeMap.find(accessor, infosetString);
        return accessor->second;
    }

    Infoset PlayState::GetInfosetSecondary()
    {
        // Betting history R, A, CH, C, F
        // Player whose turn it is // not needed?
        // Cards of player whose turn it is
        // community cards

        if (infosetStringGenerated == false)
        {
            string historyString = "";
            for (auto h : history)
            {
                historyString += h;
                historyString += ",";
            }

            auto cards = vector<int>{
                Card::GetIndexFromBitmask(get<0>(players[community.playerToMove].cards)),
                Card::GetIndexFromBitmask(get<1>(players[community.playerToMove].cards))};
            for (auto i = 0UL; i < community.cards.size(); ++i)
            {
                cards.push_back(Card::GetIndexFromBitmask(community.cards[i]));
            }

            string cardString = "";
            if (community.cards.size() == 0)
            {
                long index = Global::indexer_2.IndexLastRound(cards);
                cardString += "Preflop" + to_string(index);
            }
            else if (community.cards.size() == 3)
            {
                long index = EMDTable::flopIndices[Global::indexer_2_3.IndexLastRound(cards)];
                cardString += "Flop" + to_string(index);
            }
            else if (community.cards.size() == 4)
            {
                long index = EMDTable::turnIndices[Global::indexer_2_4.IndexLastRound(cards)];
                cardString += "Turn" + to_string(index);
            }
            else
            {
                long index = OCHSTable::riverIndices[Global::indexer_2_5.IndexLastRound(cards)];
                cardString += "River" + to_string(index);
            }
            infosetString = historyString + cardString;
        }

        tbb::concurrent_hash_map<string, Infoset>::accessor accessor;
        if (Global::nodeMap.find(accessor, infosetString))
        {
            return accessor->second;
        }
        else
        {
            Infoset infoset = Infoset(GetValidActionsCount());
            Global::nodeMap.insert({infosetString, infoset});
            return infoset;
        }
    }

    void PlayState::UpdateInfoset(Infoset &infoset)
    {
        NodeMapAccessor accessor;
        Global::nodeMap.insert(accessor, infosetString);
        accessor->second = infoset;
    }
}
