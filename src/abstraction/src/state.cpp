#include "abstraction/state.h"

namespace poker
{
    State::State() : stacks(Global::nofPlayers),
                     bets(Global::nofPlayers),
                     history(),
                     playerCards(),
                     tableCards(),
                     lastActions(Global::nofPlayers),
                     isPlayerIn(Global::nofPlayers),
                     rewards(Global::nofPlayers),
                     children(),
                     playerToMove{2},
                     bettingRound{0},
                     playersInHand{0},
                     lastPlayer{1},
                     minRaise{Global::BB},
                     isBettingOpen{false},
                     actionCount{-1},
                     infosetString(),
                     infosetStringGenerated{false}
    {
    }

    State::State(vector<int> &stacks, vector<int> &bets, vector<Action> &history,
                 vector<tuple<ulong, ulong>> &playerCards, vector<ulong> &tableCards, vector<Action> &lastActions,
                 vector<bool> &isPlayerIn, int playersInHand, int bettingRound) : stacks(stacks),
                                                                                  bets(bets),
                                                                                  history(history),
                                                                                  playerCards(playerCards),
                                                                                  tableCards(tableCards),
                                                                                  lastActions(lastActions),
                                                                                  isPlayerIn(isPlayerIn),
                                                                                  rewards(Global::nofPlayers),
                                                                                  children(),
                                                                                  playerToMove{2},
                                                                                  bettingRound{bettingRound},
                                                                                  playersInHand{playersInHand},
                                                                                  lastPlayer{1},
                                                                                  minRaise{Global::BB},
                                                                                  isBettingOpen{false},
                                                                                  actionCount{-1},
                                                                                  infosetString(),
                                                                                  infosetStringGenerated{false}
    {
    }

    int State::GetNextPlayer()
    {
        return GetNextPlayer(lastPlayer);
    }

    int State::GetNextPlayer(int lastToMoveTemp)
    {
        for (int i = (playerToMove + 1) % Global::nofPlayers; i != (lastToMoveTemp + 1) % Global::nofPlayers;
             i = (i + 1) % Global::nofPlayers)
        {
            if (isPlayerIn[i] && lastActions[i] != Action::ALLIN)
            {
                return i;
            }
        }
        return -1;
    }

    int State::GetLastPlayer(int playerThatRaised)
    {
        int last = -1;
        for (int i = (playerThatRaised + 1) % Global::nofPlayers; i != (playerThatRaised) % Global::nofPlayers;
             i = (i + 1) % Global::nofPlayers)
        {
            if (isPlayerIn[i] && lastActions[i] != Action::ALLIN)
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
        for (int i = 0; i < Global::nofPlayers; i++)
        {
            if (isPlayerIn[i] == true && lastActions[i] != Action::ALLIN)
                count++;
        }
        return count;
    }

    int State::GetActivePlayers(vector<bool> &newIsPlayerIn)
    {
        return count(newIsPlayerIn.begin(), newIsPlayerIn.end(), true);
    }

    int State::GetNumberOfAllInPlayers()
    {
        return count_if(lastActions.begin(), lastActions.end(), [](Action &a)
                        { return a == Action::ALLIN; });
    }

    int State::BettingRound()
    {
        return bettingRound;
    }

    TerminalState::TerminalState(vector<int> &stacks, vector<int> &bets, vector<Action> &history,
                                 vector<tuple<ulong, ulong>> &playerCards, vector<ulong> &tableCards, vector<Action> &lastActions,
                                 vector<bool> &isPlayerIn) : State(stacks, bets, history, playerCards, tableCards, lastActions, isPlayerIn, 0, 0),
                                                             rewardGenerated{false}

    {
    }

    float TerminalState::GetReward(int player)
    {
        if (!rewardGenerated)
            CreateRewards();
        if (accumulate(rewards.begin(), rewards.end(), 0) != 0)
            throw invalid_argument("Wrong reward calculation");
        return rewards[player];
    }

    void TerminalState::CreateRewards()
    {
        for (int i = 0; i < Global::nofPlayers; ++i)
        {
            rewards[i] -= bets[i]; // the bet amounts are considered lost
        }
        playersInHand = GetActivePlayers(isPlayerIn);

        if (playersInHand == 1)
        {
            for (int i = 0; i < Global::nofPlayers; ++i)
            {
                if (isPlayerIn[i])
                {
                    rewards[i] += accumulate(bets.begin(), bets.end(), 0);
                }
            }
        }
        else
        {
            // at least 2 players are in
            auto handValues = vector<int>(Global::nofPlayers, -1);
            for (int i = 0; i < Global::nofPlayers; ++i)
            {
                if (!isPlayerIn[i])
                    continue;

                ulong cardsBitmask = get<0>(playerCards[i]) + get<1>(playerCards[i]);
                for (int k = 0; k < tableCards.size(); ++k)
                {
                    cardsBitmask |= tableCards[k];
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
            for (int i = 0; i < indicesWithBestHands.size(); ++i)
            {
                rewards[indicesWithBestHands[i]] += accumulate(bets.begin(), bets.end(), 0) / indicesWithBestHands.size();
            }
        }
        rewardGenerated = true;
    }

    ChanceState::ChanceState()
    {
        for (int i = 0; i < Global::nofPlayers; ++i)
        {
            isPlayerIn[i] = true;
            stacks[i] = Global::buyIn;
            lastActions[i] = Action::NONE;
        }
        bets[0] = Global::SB;
        bets[1] = Global::BB;
        stacks[0] = Global::buyIn - Global::SB;
        stacks[1] = Global::buyIn - Global::BB;
        bettingRound = 0;
        lastPlayer = 1; // initially the BB player is last to act
        minRaise = Global::BB;
        playersInHand = Global::nofPlayers;
    }

    ChanceState::ChanceState(int bettingRound, int playersInHand, vector<int> &stacks, vector<int> &bets, vector<Action> &history,
                             vector<tuple<ulong, ulong>> &playerCards, vector<ulong> &tableCards, vector<Action> &lastActions,
                             vector<bool> &isPlayerIn) : State(stacks, bets, history, playerCards, tableCards, lastActions, isPlayerIn, playersInHand, bettingRound)
    {
    }

    void ChanceState::CreateChildren()
    {
        if (children.size() != 0)
            return;

        // create one playstate child after chance
        int lastToMoveTemp = -1;
        int minRaiseTemp = Global::BB;
        int newBettingRound = bettingRound + 1;
        if (bettingRound == 0)
        {
            for (int i = 2 % Global::nofPlayers;; i = (i + 1) % Global::nofPlayers)
            {
                if (isPlayerIn[i] && lastActions[i] != Action::ALLIN)
                {
                    lastToMoveTemp = i;
                }
                if (i == 1)
                    break;
            }
        }
        else if (bettingRound > 0)
        {
            for (int i = 0; i < Global::nofPlayers; ++i)
            {
                if (isPlayerIn[i] && stacks[i] != 0)
                {
                    lastToMoveTemp = i;
                }
            }
        }

        // todo: wouldnt need to always copy
        auto playerCardsNew = vector<tuple<ulong, ulong>>(playerCards); // original c# list capacity is playerCards
        auto tableCardsNew = vector<ulong>(tableCards);                 // original c# list capacity is tableCards

        switch (bettingRound)
        {
        case 0: // preflop, deal player hands
            Global::deck.Shuffle();
            for (int i = 0; i < Global::nofPlayers; ++i)
            {
                playerCardsNew.push_back({Global::deck.Peek(i), Global::deck.Peek(i + 1)});
            }
            break;
        case 1: // deal flop
            tableCardsNew.push_back(Global::deck.Peek(Global::nofPlayers * 2 + 0));
            tableCardsNew.push_back(Global::deck.Peek(Global::nofPlayers * 2 + 1));
            tableCardsNew.push_back(Global::deck.Peek(Global::nofPlayers * 2 + 2));
            break;
        case 2: // deal turn
            tableCardsNew.push_back(Global::deck.Peek(Global::nofPlayers * 2 + 3));
            break;
        case 3: // deal river
            tableCardsNew.push_back(Global::deck.Peek(Global::nofPlayers * 2 + 4));
            break;
        }
        if (GetNumberOfPlayersThatNeedToAct() >= 2 && bettingRound < 4)
        {
            // there is someone left that plays
            children.push_back(make_shared<PlayState>(newBettingRound, playerToMove, lastToMoveTemp, minRaiseTemp, playersInHand,
                                                      stacks, bets, history, playerCardsNew, tableCardsNew, lastActions, isPlayerIn, true));
        }
        else
        {
            if (GetNumberOfPlayersThatNeedToAct() == 1)
            {
                throw invalid_argument("We just dealt new cards but only 1 player has any actions left");
            }
            if (bettingRound < 3 && GetNumberOfAllInPlayers() >= 2)
            {
                // directly go to next chance node
                children.push_back(make_shared<ChanceState>(newBettingRound, GetNumberOfAllInPlayers(), stacks,
                                                            bets, history, playerCardsNew, tableCardsNew, lastActions, isPlayerIn));
            }
            else
            {
                children.push_back(make_shared<TerminalState>(stacks, bets, history,
                                                              playerCardsNew, tableCardsNew, lastActions, isPlayerIn));
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

        // create one playstate child after chance
        int lastToMoveTemp = -1;
        int minRaiseTemp = Global::BB;
        int newBettingRound = bettingRound + 1;

        if (bettingRound == 0)
        {
            for (int i = 2 % Global::nofPlayers;; i = (i + 1) % Global::nofPlayers)
            {
                if (isPlayerIn[i] && lastActions[i] != Action::ALLIN)
                {
                    lastToMoveTemp = i;
                }
                if (i == 1)
                    break;
            }
        }
        else if (bettingRound > 0)
        {
            for (int i = 0; i < Global::nofPlayers; ++i)
            {
                if (isPlayerIn[i] && stacks[i] != 0)
                {
                    lastToMoveTemp = i;
                }
            }
        }

        vector<Hand> startingHands = utils::GetStartingHandChart();

        for (int i = 0; i < Global::RANKS * Global::RANKS; ++i)
        {
            auto playerCardsNew = vector<tuple<ulong, ulong>>();
            auto tableCardsNew = vector<ulong>();

            playerCardsNew.push_back({startingHands[i].cards[0].Bitmask(), startingHands[i].cards[1].Bitmask()});
            gameStates.push_back(make_shared<PlayState>(newBettingRound, playerToMove, lastToMoveTemp, minRaiseTemp, playersInHand,
                                                        stacks, bets, history, playerCardsNew, tableCardsNew, lastActions, isPlayerIn, true));
        }

        return gameStates;
    }

    bool ChanceState::IsPlayerInHand(int player)
    {
        return isPlayerIn[player];
    }

    PlayState::PlayState(int bettingRound, int playerToMove, int lastToMove, int minRaise,
                         int playersInHand, vector<int> &stacks, vector<int> &bets, vector<Action> &history,
                         vector<tuple<ulong, ulong>> &playerCards, vector<ulong> &tableCards, vector<Action> &lastActions,
                         vector<bool> &isPlayerIn, bool isBettingOpen) : State(stacks, bets, history, playerCards, tableCards, lastActions, isPlayerIn, playersInHand, bettingRound)
    {
        this->lastPlayer = lastToMove;
        this->minRaise = minRaise;
        this->playerToMove = playerToMove;
        this->isBettingOpen = isBettingOpen;
    }

    void PlayState::CreateChildren()
    {
        if (children.size() != 0)
            return;

        int pot = accumulate(bets.begin(), bets.end(), 0);
        int currentCall = *max_element(bets.begin(), bets.end());

        if (isBettingOpen)
        {
            // raises
            for (int i = 0; i < Global::raises.size(); ++i)
            {
                auto newHistory = vector<Action>(history);
                auto newStacks = vector<int>(stacks);
                auto newBets = vector<int>(bets);

                auto newLastActions = vector<Action>(lastActions);
                auto newIsPlayerIn = vector<bool>(isPlayerIn);

                // we add <raise> chips to our current bet
                int raise = (int)(Global::raises[i] * pot);
                int actualRaise = (raise + bets[playerToMove]) - currentCall;
                if (actualRaise < minRaise || raise >= stacks[playerToMove])
                    continue;

                // valid raise, if stack is equal it would be an all in
                // TODO: dont hardcode this
                // if (i == 0)
                //     newHistory.push_back(Action::RAISE1);
                // if (i == 1)
                //     newHistory.push_back(Action::RAISE2);
                // if (i == 2)
                //     newHistory.push_back(Action::RAISE3);
                // if (i == 3)
                //     newHistory.push_back(Action::RAISE4);
                // if (i == 4)
                //     newHistory.push_back(Action::RAISE5);
                // if (i == 5)
                //     newHistory.push_back(Action::RAISE6);

                newStacks[playerToMove] -= raise;
                newBets[playerToMove] += raise;
                newLastActions[playerToMove] = Action::RAISE;

                int newLastPlayer = GetLastPlayer(playerToMove);
                int nextPlayer = GetNextPlayer(newLastPlayer);

                if (nextPlayer != -1)
                {
                    children.push_back(make_shared<PlayState>(bettingRound, nextPlayer, newLastPlayer,
                                                              actualRaise, playersInHand, newStacks, newBets, newHistory,
                                                              playerCards, tableCards, newLastActions, newIsPlayerIn, true));
                }
                else
                {
                    throw invalid_argument("Someone raised but there is noone left to play next");
                }
            }

            // all-in
            if (stacks[playerToMove] > 0)
            {
                //(currently, multiple all-ins in a row dont accumulate the raises and re-open betting round but probably they should)
                int raise = stacks[playerToMove];
                int actualRaise = (raise + bets[playerToMove]) - currentCall;

                auto newHistory = vector<Action>(history);
                auto newStacks = vector<int>(stacks);
                auto newBets = vector<int>(bets);

                auto newLastActions = vector<Action>(lastActions);
                auto newIsPlayerIn = vector<bool>(isPlayerIn);

                if (actualRaise >= minRaise)
                {
                    // re-open betting if raise high enough
                    newHistory.push_back(Action::ALLIN);
                    newLastActions[playerToMove] = Action::ALLIN;

                    newBets[playerToMove] += raise;
                    newStacks[playerToMove] = 0;

                    int newLastPlayer = GetLastPlayer(playerToMove);
                    int nextPlayer = GetNextPlayer(newLastPlayer);

                    // check if there is any player that has to play..
                    if (nextPlayer != -1)
                    {
                        children.push_back(make_shared<PlayState>(bettingRound, nextPlayer, newLastPlayer,
                                                                  actualRaise, playersInHand, newStacks, newBets, newHistory,
                                                                  playerCards, tableCards, newLastActions, newIsPlayerIn, true));
                    }
                    else
                    {
                        // ...otherwise go to chance
                        if (bettingRound != 4)
                        {
                            children.push_back(make_shared<ChanceState>(bettingRound, GetActivePlayers(newIsPlayerIn), newStacks,
                                                                        newBets, newHistory, playerCards, tableCards, newLastActions, newIsPlayerIn));
                        }
                        else
                        {
                            children.push_back(make_shared<TerminalState>(newStacks, newBets, newHistory,
                                                                          playerCards, tableCards, newLastActions, newIsPlayerIn));
                        }
                    }
                }
                else
                {
                    // all in possible but not re-open betting
                    newHistory.push_back(Action::ALLIN);
                    newLastActions[playerToMove] = Action::ALLIN;

                    newBets[playerToMove] += raise;
                    newStacks[playerToMove] = 0;

                    int newLastPlayer = GetLastPlayer(playerToMove);
                    int nextPlayer = GetNextPlayer(newLastPlayer);

                    // check if there is any player that has to play..
                    if (nextPlayer != -1)
                    {
                        children.push_back(make_shared<PlayState>(bettingRound, nextPlayer, newLastPlayer,
                                                                  minRaise, playersInHand, newStacks, newBets, newHistory,
                                                                  playerCards, tableCards, newLastActions, newIsPlayerIn, isBettingOpen));
                    }
                    else
                    {
                        // ...otherwise go to chance
                        if (bettingRound != 4)
                        {
                            children.push_back(make_shared<ChanceState>(bettingRound, GetActivePlayers(newIsPlayerIn), newStacks,
                                                                        newBets, newHistory, playerCards, tableCards, newLastActions, newIsPlayerIn));
                        }
                        else
                        {
                            children.push_back(make_shared<TerminalState>(newStacks, newBets, newHistory,
                                                                          playerCards, tableCards, newLastActions, newIsPlayerIn));
                        }
                    }
                }
            }
        }
        if (currentCall > bets[playerToMove])
        {
            // fold
            auto newHistory = vector<Action>(history);
            auto newStacks = vector<int>(stacks);
            auto newBets = vector<int>(bets);
            auto newLastActions = vector<Action>(lastActions);
            auto newIsPlayerIn = vector<bool>(isPlayerIn);

            newHistory.push_back(Action::FOLD);
            newLastActions[playerToMove] = Action::FOLD;
            newIsPlayerIn[playerToMove] = false;

            int nextPlayer = GetNextPlayer();

            if (GetActivePlayers(newIsPlayerIn) == 1)
            {
                // terminal state
                children.push_back(make_shared<TerminalState>(newStacks, newBets, newHistory,
                                                              playerCards, tableCards, newLastActions, newIsPlayerIn));
            }
            else if (nextPlayer != -1)
            {
                children.push_back(make_shared<PlayState>(bettingRound, nextPlayer, lastPlayer,
                                                          minRaise, playersInHand, newStacks, newBets, newHistory,
                                                          playerCards, tableCards, newLastActions, newIsPlayerIn, isBettingOpen));
            }
            else
            {
                // here the betting round is over, there is more than 1 player left
                if (bettingRound != 4)
                {
                    // chance
                    children.push_back(make_shared<ChanceState>(bettingRound, GetActivePlayers(newIsPlayerIn), newStacks,
                                                                newBets, newHistory, playerCards, tableCards, newLastActions, newIsPlayerIn));
                }
                else
                {
                    children.push_back(make_shared<TerminalState>(newStacks, newBets, newHistory,
                                                                  playerCards, tableCards, newLastActions, newIsPlayerIn));
                }
            }
        }
        if (currentCall - bets[playerToMove] < stacks[playerToMove])
        {
            // call possible if needed chips is LESS (otherwise its all in), if same its a check
            auto newHistory = vector<Action>(history);
            auto newStacks = vector<int>(stacks);
            auto newBets = vector<int>(bets);
            auto newLastActions = vector<Action>(lastActions);
            auto newIsPlayerIn = vector<bool>(isPlayerIn);

            newHistory.push_back(Action::CALL);
            newLastActions[playerToMove] = Action::CALL;

            newBets[playerToMove] += currentCall - bets[playerToMove];
            newStacks[playerToMove] -= currentCall - bets[playerToMove];

            int nextPlayer = GetNextPlayer();

            if (nextPlayer != -1) // the round isnt over
            {
                children.push_back(make_shared<PlayState>(bettingRound, nextPlayer, lastPlayer,
                                                          minRaise, playersInHand, newStacks, newBets, newHistory,
                                                          playerCards, tableCards, newLastActions, newIsPlayerIn, isBettingOpen));
            }
            else
            {
                // all players have moved
                if (bettingRound < 4)
                {
                    // some cards are missing still, chance
                    children.push_back(make_shared<ChanceState>(bettingRound, GetActivePlayers(newIsPlayerIn), newStacks,
                                                                newBets, newHistory, playerCards, tableCards, newLastActions, newIsPlayerIn));
                }
                else if (bettingRound == 4)
                {
                    // terminal, all cards are already dealt
                    children.push_back(make_shared<TerminalState>(newStacks, newBets, newHistory,
                                                                  playerCards, tableCards, lastActions, isPlayerIn));
                }
            }
        }

        int totalStack = accumulate(stacks.begin(), stacks.end(), 0);
        int totalBet = accumulate(bets.begin(), bets.end(), 0);
        if (totalStack + totalBet != Global::buyIn * Global::nofPlayers)
        {
            throw invalid_argument("Impossible chip counts");
        }
    }

    int PlayState::GetValidActionsCount()
    {
        if (actionCount != -1)
            return actionCount;

        if (children.size() != 0)
        {
            actionCount = children.size();
        }
        else
            actionCount = GetValidActions().size();
        return actionCount;
    }

    vector<Action> PlayState::GetValidActions()
    {
        auto validActions = vector<Action>();
        int pot = accumulate(bets.begin(), bets.end(), 0);
        int currentCall = *max_element(bets.begin(), bets.end());

        if (playersInHand == 0)
        {
            throw invalid_argument("There must always be >= one player in hand");
        }
        if (playersInHand == 1)
        {
            return validActions; // no valid actions
        }

        // raises
        if (isBettingOpen)
        {
            int raise;
            for (int i = 0; i < Global::raises.size(); ++i)
            {
                raise = (int)(Global::raises[i] * pot);
                int actualRaise = raise - (currentCall - bets[playerToMove]);

                if (actualRaise < minRaise || raise >= stacks[playerToMove])
                    continue;

                // valid raise, if stack is equal it would be an all in
                if (i == 0)
                    validActions.push_back(Action::RAISE1);
                if (i == 1)
                    validActions.push_back(Action::RAISE2);
                if (i == 2)
                    validActions.push_back(Action::RAISE3);
            }

            raise = stacks[playerToMove];
            // all-in
            if (raise > 0)
            {
                //(currently, multiple all-ins in a row dont accumulate the raises and reopen betting round but probably they should)
                // int actualRaise = (raise + bets[playerToMove]) - currentCall;
                validActions.push_back(Action::ALLIN);
            }
        }
        if (currentCall > bets[playerToMove])
        {
            // fold
            validActions.push_back(Action::FOLD);
        }
        if (currentCall - bets[playerToMove] < stacks[playerToMove])
        {
            // call
            validActions.push_back(Action::CALL);
        }

        return validActions;
    }

    bool PlayState::IsPlayerTurn(int player)
    {
        return playerToMove == player;
    }

    bool PlayState::IsPlayerInHand(int player)
    {
        return isPlayerIn[player];
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
                Card::GetIndexFromBitmask(get<0>(playerCards[playerToMove])),
                Card::GetIndexFromBitmask(get<1>(playerCards[playerToMove]))};
            for (int i = 0; i < tableCards.size(); ++i)
            {
                cards.push_back(Card::GetIndexFromBitmask(tableCards[i]));
            }

            string cardString = "";
            if (tableCards.size() == 0)
            {
                long index = Global::indexer_2.IndexLastRound(cards);
                cardString += "P" + to_string(index);
            }
            else if (tableCards.size() == 3)
            {
                long index = EMDTable::flopIndices[Global::indexer_2_3.IndexLastRound(cards)];
                cardString += "F" + to_string(index);
            }
            else if (tableCards.size() == 4)
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

            auto cards = vector<int>({Card::GetIndexFromBitmask(get<0>(playerCards[playerToMove])),
                                      Card::GetIndexFromBitmask(get<1>(playerCards[playerToMove]))});
            for (int i = 0; i < tableCards.size(); ++i)
            {
                cards.push_back(Card::GetIndexFromBitmask(tableCards[i]));
            }

            string cardString = "";
            if (tableCards.size() == 0)
            {
                long index = Global::indexer_2.IndexLastRound(cards);
                cardString += "Preflop" + to_string(index);
            }
            else if (tableCards.size() == 3)
            {
                long index = EMDTable::flopIndices[Global::indexer_2_3.IndexLastRound(cards)];
                cardString += "Flop" + to_string(index);
            }
            else if (tableCards.size() == 4)
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
}
