#include "binary/state.h"

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
    return count_if(lastActions.begin(), lastActions.end(), [](auto a)
                    { a == Action::ALLIN; });
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
    auto playerCardsNew = vector<tuple<ulong, ulong>>(); // original c# list capacity is playerCards
    auto tableCardsNew = vector<ulong>();                // original c# list capacity is tableCards

    switch (bettingRound)
    {
    case 0: // preflop, deal player hands
        Global::Deck.Value.Shuffle();
        for (int i = 0; i < Global::nofPlayers; ++i)
        {
            playerCardsNew.push_back({Global::Deck.Value.Draw(i * 2), Global::Deck.Value.Draw(i * 2 + 1)});
        }
        break;
    case 1: // deal flop
        // Global::Deck.Value.Shuffle(Global::nofPlayers * 2); // not necessarily needed, check
        tableCardsNew.push_back(Global::Deck.Value.Draw(Global::nofPlayers * 2 + 0));
        tableCardsNew.push_back(Global::Deck.Value.Draw(Global::nofPlayers * 2 + 1));
        tableCardsNew.push_back(Global::Deck.Value.Draw(Global::nofPlayers * 2 + 2));
        break;
    case 2: // deal turn
        Global::Deck.Value.Shuffle(Global::nofPlayers * 2 + 3);
        tableCardsNew.push_back(Global::Deck.Value.Draw(Global::nofPlayers * 2 + 3));
        break;
    case 3: // deal river
        Global::Deck.Value.Shuffle(Global::nofPlayers * 2 + 4);
        tableCardsNew.push_back(Global::Deck.Value.Draw(Global::nofPlayers * 2 + 4));
        break;
    }
    if (GetNumberOfPlayersThatNeedToAct() >= 2 && bettingRound < 4)
    {
        // there is someone left that plays
        children.push_back(PlayState(newBettingRound, playerToMove, lastToMoveTemp, minRaiseTemp, playersInHand,
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
            children.push_back(ChanceState(newBettingRound, GetNumberOfAllInPlayers(), stacks,
                                           bets, history, playerCardsNew, tableCardsNew, lastActions, isPlayerIn));
        }
        else
        {
            children.push_back(TerminalState(stacks, bets, history,
                                             playerCardsNew, tableCardsNew, lastActions, isPlayerIn));
        }
    }
}
/// <summary>
/// Note: The single child was already randomly created
/// </summary>
/// <returns></returns>
public
override State DoRandomAction()
{
    CreateChildren();
    return children[0];
}
public
List<PlayState> GetFirstActionStates()
{
    List<PlayState> gameStates = new List<PlayState>();

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

    List<Hand> startingHands = Utilities.GetStartingHandChart();

    for (int i = 0; i < 169; ++i)
    {
        List<Tuple<ulong, ulong>> playerCardsNew = new List<Tuple<ulong, ulong>>();
        List<ulong> tableCardsNew = new List<ulong>();

        playerCardsNew.Add(Tuple.Create(startingHands[i].Cards[0].GetBit(), startingHands[i].Cards[1].GetBit()));
        gameStates.Add(new PlayState(newBettingRound, playerToMove, lastToMoveTemp, minRaiseTemp, playersInHand,
                                     stacks, bets, history, playerCardsNew, tableCardsNew, lastActions, isPlayerIn, true));
    }

    return gameStates;
}
public
override bool IsPlayerInHand(int player)
{
    return isPlayerIn[player];
}
}
class PlayState : State
{
public
    PlayState(int bettingRound, int playerToMove, int lastToMove, int minRaise,
              int playersInHand, int[] stacks, int[] bets, List<Action> history,
              List<Tuple<ulong, ulong>> playerCards, List<ulong> tableCards, Action[] lastActions,
              bool[] isPlayerIn, bool isBettingOpen)
    {
        this.lastPlayer = lastToMove;
        this.minRaise = minRaise;
        this.stacks = stacks;
        this.playerCards = playerCards;
        this.tableCards = tableCards;
        this.lastActions = lastActions;
        this.isPlayerIn = isPlayerIn;
        this.bets = bets;
        this.stacks = stacks;
        this.history = history;
        this.playerToMove = playerToMove;
        this.bettingRound = bettingRound;
        this.playersInHand = playersInHand;
        this.isBettingOpen = isBettingOpen;
    }
public
    override void CreateChildren()
    {
        if (children.Count != 0)
            return;

        int pot = bets.Sum();
        int currentCall = bets.Max();

        if (isBettingOpen)
        {
            // raises
            for (int i = 0; i < Global::raises.Count; ++i)
            {
                List<Action> newHistory = new List<Action>(history);
                int[] newStacks = new int[Global::nofPlayers];
                Array.Copy(stacks, newStacks, Global::nofPlayers);
                int[] newBets = new int[Global::nofPlayers];
                Array.Copy(bets, newBets, Global::nofPlayers);

                Action[] newLastActions = new Action[Global::nofPlayers];
                Array.Copy(lastActions, newLastActions, Global::nofPlayers);
                bool[] newIsPlayerIn = new bool[Global::nofPlayers];
                Array.Copy(isPlayerIn, newIsPlayerIn, Global::nofPlayers);

                // we add <raise> chips to our current bet
                int raise = (int)(Global::raises[i] * pot);
                int actualRaise = (raise + bets[playerToMove]) - currentCall;
                if (actualRaise < minRaise || raise >= stacks[playerToMove])
                    continue;

                // valid raise, if stack is equal it would be an all in
                // TODO: dont hardcode this
                if (i == 0)
                    newHistory.Add(Action::RAISE1);
                if (i == 1)
                    newHistory.Add(Action::RAISE2);
                if (i == 2)
                    newHistory.Add(Action::RAISE3);
                // if (i == 3)
                //     newHistory.Add(Action::RAISE4);
                // if (i == 4)
                //     newHistory.Add(Action::RAISE5);
                // if (i == 5)
                //     newHistory.Add(Action::RAISE6);

                newStacks[playerToMove] -= raise;
                newBets[playerToMove] += raise;
                newLastActions[playerToMove] = Action::RAISE;

                int newLastPlayer = GetLastPlayer(playerToMove);
                int nextPlayer = GetNextPlayer(newLastPlayer);

                if (nextPlayer != -1)
                {
                    children.Add(new PlayState(bettingRound, nextPlayer, newLastPlayer,
                                               actualRaise, playersInHand, newStacks, newBets, newHistory,
                                               playerCards, tableCards, newLastActions, newIsPlayerIn, true));
                }
                else
                {
                    throw new Exception("Someone raised but there is noone left to play next");
                }
            }

            // all-in
            if (stacks[playerToMove] > 0)
            {
                //(currently, multiple all-ins in a row dont accumulate the raises and re-open betting round but probably they should)
                int raise = stacks[playerToMove];
                int actualRaise = (raise + bets[playerToMove]) - currentCall;

                List<Action> newHistory = new List<Action>(history);
                Action[] newLastActions = new Action[Global::nofPlayers];
                Array.Copy(lastActions, newLastActions, Global::nofPlayers);

                int[] newStacks = new int[Global::nofPlayers];
                Array.Copy(stacks, newStacks, Global::nofPlayers);
                int[] newBets = new int[Global::nofPlayers];
                Array.Copy(bets, newBets, Global::nofPlayers);

                bool[] newIsPlayerIn = new bool[Global::nofPlayers];
                Array.Copy(isPlayerIn, newIsPlayerIn, Global::nofPlayers);

                if (actualRaise >= minRaise)
                {
                    // re-open betting if raise high enough
                    newHistory.Add(Action::ALLIN);
                    newLastActions[playerToMove] = Action::ALLIN;

                    newBets[playerToMove] += raise;
                    newStacks[playerToMove] = 0;

                    int newLastPlayer = GetLastPlayer(playerToMove);
                    int nextPlayer = GetNextPlayer(newLastPlayer);

                    // check if there is any player that has to play..
                    if (nextPlayer != -1)
                    {
                        children.Add(new PlayState(bettingRound, nextPlayer, newLastPlayer,
                                                   actualRaise, playersInHand, newStacks, newBets, newHistory,
                                                   playerCards, tableCards, newLastActions, newIsPlayerIn, true));
                    }
                    else
                    {
                        // ...otherwise go to chance
                        if (bettingRound != 4)
                        {
                            children.Add(new ChanceState(bettingRound, GetActivePlayers(newIsPlayerIn), newStacks,
                                                         newBets, newHistory, playerCards, tableCards, newLastActions, newIsPlayerIn));
                        }
                        else
                        {
                            children.Add(new TerminalState(newStacks, newBets, newHistory,
                                                           playerCards, tableCards, newLastActions, newIsPlayerIn));
                        }
                    }
                }
                else
                {
                    // all in possible but not re-open betting
                    newHistory.Add(Action::ALLIN);
                    newLastActions[playerToMove] = Action::ALLIN;

                    newBets[playerToMove] += raise;
                    newStacks[playerToMove] = 0;

                    int newLastPlayer = GetLastPlayer(playerToMove);
                    int nextPlayer = GetNextPlayer(newLastPlayer);

                    // check if there is any player that has to play..
                    if (nextPlayer != -1)
                    {
                        children.Add(new PlayState(bettingRound, nextPlayer, newLastPlayer,
                                                   minRaise, playersInHand, newStacks, newBets, newHistory,
                                                   playerCards, tableCards, newLastActions, newIsPlayerIn, isBettingOpen));
                    }
                    else
                    {
                        // ...otherwise go to chance
                        if (bettingRound != 4)
                        {
                            children.Add(new ChanceState(bettingRound, GetActivePlayers(newIsPlayerIn), newStacks,
                                                         newBets, newHistory, playerCards, tableCards, newLastActions, newIsPlayerIn));
                        }
                        else
                        {
                            children.Add(new TerminalState(newStacks, newBets, newHistory,
                                                           playerCards, tableCards, newLastActions, newIsPlayerIn));
                        }
                    }
                }
            }
        }
        if (currentCall > bets[playerToMove])
        {
            // fold
            List<Action> newHistory = new List<Action>(history);
            Action[] newLastActions = new Action[Global::nofPlayers];
            Array.Copy(lastActions, newLastActions, Global::nofPlayers);
            int[] newStacks = new int[Global::nofPlayers];
            Array.Copy(stacks, newStacks, Global::nofPlayers);
            int[] newBets = new int[Global::nofPlayers];
            Array.Copy(bets, newBets, Global::nofPlayers);
            bool[] newIsPlayerIn = new bool[Global::nofPlayers];
            Array.Copy(isPlayerIn, newIsPlayerIn, Global::nofPlayers);

            newHistory.Add(Action::FOLD);
            newLastActions[playerToMove] = Action::FOLD;
            newIsPlayerIn[playerToMove] = false;

            int nextPlayer = GetNextPlayer();

            if (GetActivePlayers(newIsPlayerIn) == 1)
            {
                // terminal state
                children.Add(new TerminalState(newStacks, newBets, newHistory,
                                               playerCards, tableCards, newLastActions, newIsPlayerIn));
            }
            else if (nextPlayer != -1)
            {
                children.Add(new PlayState(bettingRound, nextPlayer, lastPlayer,
                                           minRaise, playersInHand, newStacks, newBets, newHistory,
                                           playerCards, tableCards, newLastActions, newIsPlayerIn, isBettingOpen));
            }
            else
            {
                // here the betting round is over, there is more than 1 player left
                if (bettingRound != 4)
                {
                    // chance
                    children.Add(new ChanceState(bettingRound, GetActivePlayers(newIsPlayerIn), newStacks,
                                                 newBets, newHistory, playerCards, tableCards, newLastActions, newIsPlayerIn));
                }
                else
                {
                    children.Add(new TerminalState(newStacks, newBets, newHistory,
                                                   playerCards, tableCards, newLastActions, newIsPlayerIn));
                }
            }
        }
        if (currentCall - bets[playerToMove] < stacks[playerToMove])
        {
            // call possible if needed chips is LESS (otherwise its all in), if same its a check
            List<Action> newHistory = new List<Action>(history);
            Action[] newLastActions = new Action[Global::nofPlayers];
            Array.Copy(lastActions, newLastActions, Global::nofPlayers);
            int[] newStacks = new int[Global::nofPlayers];
            Array.Copy(stacks, newStacks, Global::nofPlayers);
            int[] newBets = new int[Global::nofPlayers];
            Array.Copy(bets, newBets, Global::nofPlayers);
            bool[] newIsPlayerIn = new bool[Global::nofPlayers];
            Array.Copy(isPlayerIn, newIsPlayerIn, Global::nofPlayers);

            newHistory.Add(Action::CALL);
            newLastActions[playerToMove] = Action::CALL;

            newBets[playerToMove] += currentCall - bets[playerToMove];
            newStacks[playerToMove] -= currentCall - bets[playerToMove];

            int nextPlayer = GetNextPlayer();

            if (nextPlayer != -1) // the round isnt over
            {
                children.Add(new PlayState(bettingRound, nextPlayer, lastPlayer,
                                           minRaise, playersInHand, newStacks, newBets, newHistory,
                                           playerCards, tableCards, newLastActions, newIsPlayerIn, isBettingOpen));
            }
            else
            {
                // all players have moved
                if (bettingRound < 4)
                {
                    // some cards are missing still, chance
                    children.Add(new ChanceState(bettingRound, GetActivePlayers(newIsPlayerIn), newStacks,
                                                 newBets, newHistory, playerCards, tableCards, newLastActions, newIsPlayerIn));
                }
                else if (bettingRound == 4)
                {
                    // terminal, all cards are already dealt
                    children.Add(new TerminalState(newStacks, newBets, newHistory,
                                                   playerCards, tableCards, lastActions, isPlayerIn));
                }
            }
        }

        if (stacks.Sum() + bets.Sum() != Global::buyIn * Global::nofPlayers)
        {
            throw new Exception("Impossible chip counts");
        }
    }
public
    int GetValidActionsCount()
    {
        if (actionCount != -1)
            return actionCount;

        if (children.Count != 0)
        {
            actionCount = children.Count;
        }
        else
            actionCount = GetValidActions().Count();
        return actionCount;
    }
public
    List<Action> GetValidActions()
    {
        List<Action> validActions = new List<Action>();
        int pot = bets.Sum();
        int currentCall = bets.Max();

        if (playersInHand == 0)
        {
            throw new Exception("There must always be >= one player in hand");
        }
        if (playersInHand == 1)
        {
            return validActions; // no valid actions
        }

        // raises
        if (isBettingOpen)
        {
            int raise;
            for (int i = 0; i < Global::raises.Count; ++i)
            {
                raise = (int)(Global::raises[i] * pot);
                int actualRaise = raise - (currentCall - bets[playerToMove]);

                if (actualRaise < minRaise || raise >= stacks[playerToMove])
                    continue;

                // valid raise, if stack is equal it would be an all in
                if (i == 0)
                    validActions.Add(Action::RAISE1);
                if (i == 1)
                    validActions.Add(Action::RAISE2);
                if (i == 2)
                    validActions.Add(Action::RAISE3);
            }

            raise = stacks[playerToMove];
            // all-in
            if (raise > 0)
            {
                //(currently, multiple all-ins in a row dont accumulate the raises and reopen betting round but probably they should)
                // int actualRaise = (raise + bets[playerToMove]) - currentCall;
                validActions.Add(Action::ALLIN);
            }
        }
        if (currentCall > bets[playerToMove])
        {
            // fold
            validActions.Add(Action::FOLD);
        }
        if (currentCall - bets[playerToMove] < stacks[playerToMove])
        {
            // call
            validActions.Add(Action::CALL);
        }

        return validActions;
    }
public
    override bool IsPlayerTurn(int player)
    {
        return playerToMove == player;
    }
public
    override bool IsPlayerInHand(int player)
    {
        return isPlayerIn[player];
    }
public
    override Infoset GetInfoset()
    {
        // Betting history R, A, CH, C, F
        // Player whose turn it is // not needed?
        // Cards of player whose turn it is
        // community cards

        if (infosetStringGenerated == false)
        {
            string historyString = string.Join("", history);

            List<int> cards = new List<int>{
                Card.GetIndexFromBitmask(playerCards[playerToMove].Item1),
                Card.GetIndexFromBitmask(playerCards[playerToMove].Item2)};
            for (int i = 0; i < tableCards.Count; ++i)
            {
                cards.Add(Card.GetIndexFromBitmask(tableCards[i]));
            }
            int[] cardArray = cards.ToArray();

            string cardString = "";
            if (tableCards.Count == 0)
            {
                long index = Global::indexer_2.indexLast(cardArray);
                cardString += "P" + index;
            }
            else if (tableCards.Count == 3)
            {
                long index = EMDTable.flopIndices[Global::indexer_2_3.indexLast(cardArray)];
                cardString += "F" + index;
            }
            else if (tableCards.Count == 4)
            {
                long index = EMDTable.turnIndices[Global::indexer_2_4.indexLast(cardArray)];
                cardString += "T" + index;
            }
            else
            {
                long index = OCHSTable.riverIndices[Global::indexer_2_5.indexLast(cardArray)];
                cardString += "R" + index;
            }
            infosetString = historyString + cardString;
            infosetStringGenerated = true;
        }

        Global::nodeMap.TryGetValue(infosetString, out Infoset infoset);
        if (infoset != null)
        {
            return infoset;
        }
        else
        {
            infoset = new Infoset(GetValidActionsCount());
            Infoset infosetRet = Global::nodeMap.GetOrAdd(infosetString, infoset);
            return infosetRet;
        }
    }
public
    override Infoset GetInfosetSecondary()
    {
        // Betting history R, A, CH, C, F
        // Player whose turn it is // not needed?
        // Cards of player whose turn it is
        // community cards

        if (infosetStringGenerated == false)
        {
            string historyString = string.Join(",", history.ToArray());

            List<int> cards = new List<int>();
            cards.Add(Card.GetIndexFromBitmask(playerCards[playerToMove].Item1));
            cards.Add(Card.GetIndexFromBitmask(playerCards[playerToMove].Item2));
            for (int i = 0; i < tableCards.Count; ++i)
            {
                cards.Add(Card.GetIndexFromBitmask(tableCards[i]));
            }
            int[] cardArray = cards.ToArray();

            string cardString = "";
            if (tableCards.Count == 0)
            {
                long index = Global::indexer_2.indexLast(cardArray);
                cardString += "Preflop" + index.ToString();
            }
            else if (tableCards.Count == 3)
            {
                long index = EMDTable.flopIndices[Global::indexer_2_3.indexLast(cardArray)];
                cardString += "Flop" + index.ToString();
            }
            else if (tableCards.Count == 4)
            {
                long index = EMDTable.turnIndices[Global::indexer_2_4.indexLast(cardArray)];
                cardString += "Turn" + index.ToString();
            }
            else
            {
                long index = OCHSTable.riverIndices[Global::indexer_2_5.indexLast(cardArray)];
                cardString += "River" + index.ToString();
            }
            infosetString = historyString + cardString;
        }

        Global::nodeMapBaseline.TryGetValue(infosetString, out Infoset infoset);
        if (infoset != null)
        {
            return infoset;
        }
        else
        {
            infoset = new Infoset(GetValidActionsCount());
            Infoset infosetRet = Global::nodeMapBaseline.GetOrAdd(infosetString, infoset);
            return infosetRet;
        }
    }
