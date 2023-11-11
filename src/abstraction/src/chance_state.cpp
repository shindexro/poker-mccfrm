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
