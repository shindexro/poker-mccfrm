#include "abstraction/play_state.h"

namespace poker
{
    PlayState::PlayState(CommunityInfo &community,
                         vector<PlayerInfo> &players,
                         vector<Action> &history) : State(community, players, history)
    {
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

    void PlayState::CreateChildren()
    {
        if (children.size() != 0)
            return;

        int pot = GetPot();
        int currentCall = MinimumCall();

        if (community.isBettingOpen)
        {
            // raises
            for (auto i = 0UL; i < Global::raiseRatios.size(); ++i)
            {
                auto nextState = make_shared<PlayState>(community, players, history);

                // we add <raise> chips to our current bet
                int raise = (int)(Global::raiseRatios[i] * pot);
                int actualRaise = (raise + players[community.playerToMove].bet) - currentCall;
                if (actualRaise < community.minRaise || raise >= players[community.playerToMove].stack)
                    continue;

                // valid raise, if stack is equal it would be an all in
                // TODO: dont hardcode this
                if (i == 0)
                    nextState->history.push_back(Action::RAISE1);
                if (i == 1)
                    nextState->history.push_back(Action::RAISE2);
                if (i == 2)
                    nextState->history.push_back(Action::RAISE3);
                // if (i == 3)
                //     newHistory.push_back(Action::RAISE4);
                // if (i == 4)
                //     newHistory.push_back(Action::RAISE5);
                // if (i == 5)
                //     newHistory.push_back(Action::RAISE6);

                nextState->players[community.playerToMove].stack -= raise;
                nextState->players[community.playerToMove].bet += raise;
                nextState->players[community.playerToMove].lastAction = Action::RAISE;

                nextState->community.lastPlayer = GetLastPlayer(community.playerToMove);
                nextState->community.playerToMove = GetNextPlayer(nextState->community.lastPlayer);

                nextState->community.isBettingOpen = true;
                nextState->community.minRaise = actualRaise;

                if (nextState->community.playerToMove != -1)
                {
                    children.push_back(nextState);
                }
                else
                {
                    throw invalid_argument("Someone raised but there is no one left to play next");
                }
            }
            /*

            // all-in
            if (players[community.playerToMove].stack > 0)
            {
                //(currently, multiple all-ins in a row dont accumulate the raises and re-open betting round but probably they should)
                int raise = players[community.playerToMove].stack;
                int actualRaise = (raise + players[community.playerToMove].bet) - currentCall;

                auto newHistory = vector<Action>(history);
                auto newStacks = vector<int>(stacks);
                auto newBets = vector<int>(bets);

                auto newLastActions = vector<Action>(lastActionOfPlayer);
                auto newIsPlayerIn = vector<bool>(isPlayerIn);

                if (actualRaise >= minRaise)
                {
                    // re-open betting if raise high enough
                    newHistory.push_back(Action::ALLIN);
                    newLastActions[community.playerToMove] = Action::ALLIN;

                    newBets[community.playerToMove] += raise;
                    newStacks[community.playerToMove] = 0;

                    int newLastPlayer = GetLastPlayer(community.playerToMove);
                    int nextPlayer = GetNextPlayer(newLastPlayer);

                    // check if there is any player that has to play..
                    if (nextPlayer != -1)
                    {
                        children.push_back(make_shared<PlayState>(community.bettingRound, nextPlayer, newLastPlayer,
                                                                  actualRaise, playersInHand, newStacks, newBets, newHistory,
                                                                  playerCards, community.cards, newLastActions, newIsPlayerIn, true));
                    }
                    else
                    {
                        // ...otherwise go to chance
                        if (community.bettingRound != 4)
                        {
                            children.push_back(make_shared<ChanceState>(community.bettingRound, GetActivePlayers(newIsPlayerIn), newStacks,
                                                                        newBets, newHistory, playerCards, community.cards, newLastActions, newIsPlayerIn));
                        }
                        else
                        {
                            children.push_back(make_shared<TerminalState>(newStacks, newBets, newHistory,
                                                                          playerCards, community.cards, newLastActions, newIsPlayerIn));
                        }
                    }
                }
                else
                {
                    // all in possible but not re-open betting
                    newHistory.push_back(Action::ALLIN);
                    newLastActions[community.playerToMove] = Action::ALLIN;

                    newBets[community.playerToMove] += raise;
                    newStacks[community.playerToMove] = 0;

                    int newLastPlayer = GetLastPlayer(community.playerToMove);
                    int nextPlayer = GetNextPlayer(newLastPlayer);

                    // check if there is any player that has to play..
                    if (nextPlayer != -1)
                    {
                        children.push_back(make_shared<PlayState>(community.bettingRound, nextPlayer, newLastPlayer,
                                                                  minRaise, playersInHand, newStacks, newBets, newHistory,
                                                                  playerCards, community.cards, newLastActions, newIsPlayerIn, isBettingOpen));
                    }
                    else
                    {
                        // ...otherwise go to chance
                        if (community.bettingRound != 4)
                        {
                            children.push_back(make_shared<ChanceState>(community.bettingRound, GetActivePlayers(newIsPlayerIn), newStacks,
                                                                        newBets, newHistory, playerCards, community.cards, newLastActions, newIsPlayerIn));
                        }
                        else
                        {
                            children.push_back(make_shared<TerminalState>(newStacks, newBets, newHistory,
                                                                          playerCards, community.cards, newLastActions, newIsPlayerIn));
                        }
                    }
                }
            }
        }
        if (currentCall > players[community.playerToMove].bet)
        {
            // fold
            auto newHistory = vector<Action>(history);
            auto newStacks = vector<int>(stacks);
            auto newBets = vector<int>(bets);
            auto newLastActions = vector<Action>(lastActionOfPlayer);
            auto newIsPlayerIn = vector<bool>(isPlayerIn);

            newHistory.push_back(Action::FOLD);
            newLastActions[community.playerToMove] = Action::FOLD;
            newIsPlayerIn[community.playerToMove] = false;

            int nextPlayer = GetNextPlayer();

            if (GetActivePlayers(newIsPlayerIn) == 1)
            {
                // terminal state
                children.push_back(make_shared<TerminalState>(newStacks, newBets, newHistory,
                                                              playerCards, community.cards, newLastActions, newIsPlayerIn));
            }
            else if (nextPlayer != -1)
            {
                children.push_back(make_shared<PlayState>(community.bettingRound, nextPlayer, lastPlayer,
                                                          minRaise, playersInHand, newStacks, newBets, newHistory,
                                                          playerCards, community.cards, newLastActions, newIsPlayerIn, isBettingOpen));
            }
            else
            {
                // here the betting round is over, there is more than 1 player left
                if (community.bettingRound != 4)
                {
                    // chance
                    children.push_back(make_shared<ChanceState>(community.bettingRound, GetActivePlayers(newIsPlayerIn), newStacks,
                                                                newBets, newHistory, playerCards, community.cards, newLastActions, newIsPlayerIn));
                }
                else
                {
                    children.push_back(make_shared<TerminalState>(newStacks, newBets, newHistory,
                                                                  playerCards, community.cards, newLastActions, newIsPlayerIn));
                }
            }
        }
        if (currentCall - players[community.playerToMove] < players[community.playerToMove].bet.stack)
        {
            // call possible if needed chips is LESS (otherwise its all in), if same its a check
            auto newHistory = vector<Action>(history);
            auto newStacks = vector<int>(stacks);
            auto newBets = vector<int>(bets);
            auto newLastActions = vector<Action>(lastActionOfPlayer);
            auto newIsPlayerIn = vector<bool>(isPlayerIn);

            newHistory.push_back(Action::CALL);
            newLastActions[community.playerToMove] = Action::CALL;

            newBets[community.playerToMove] += currentCall - players[community.playerToMove].bet;
            newStacks[community.playerToMove] -= currentCall - players[community.playerToMove].bet;

            int nextPlayer = GetNextPlayer();

            if (nextPlayer != -1) // the round isnt over
            {
                children.push_back(make_shared<PlayState>(community.bettingRound, nextPlayer, lastPlayer,
                                                          minRaise, playersInHand, newStacks, newBets, newHistory,
                                                          playerCards, community.cards, newLastActions, newIsPlayerIn, isBettingOpen));
            }
            else
            {
                // all players have moved
                if (community.bettingRound < 4)
                {
                    // some cards are missing still, chance
                    children.push_back(make_shared<ChanceState>(community.bettingRound, GetActivePlayers(newIsPlayerIn), newStacks,
                                                                newBets, newHistory, playerCards, community.cards, newLastActions, newIsPlayerIn));
                }
                else if (community.bettingRound == 4)
                {
                    // terminal, all cards are already dealt
                    children.push_back(make_shared<TerminalState>(newStacks, newBets, newHistory,
                                                                  playerCards, community.cards, lastActionOfPlayer, isPlayerIn));
                }
            }
        }

        int totalStack = accumulate(stacks.begin(), stacks.end(), 0);
        int totalBet = accumulate(bets.begin(), bets.end(), 0);
        if (totalStack + totalBet != Global::buyIn * Global::nofPlayers)
        {
            throw invalid_argument("Impossible chip counts");
        }
        */
        }
    }
} // namespace poker
